/*

	Async flash : asychronously write a uSD file to flash memory.
	Assumes we wan write DWORD by DWORD

	Usage : 
		- flash_init
		- flash_start_write : call to launch a write of an open file to flash mem.
		- flash_frame : call each frame. Can be long, do whatever is needed before.
		- char flash_message[] : progress / error message 
	
	Statically allocs a buffer of memory (8k)

*/
#include <stdint.h>
#include <string.h>
#include "stm32f4xx.h" // CMSIS
#include "kernel.h" // vga_line, 

#include "flashit.h"

// must be a subsize of 16k, readable in a frame.
#define BUFFERSIZE (8*1024)

#define FLASH_KEY1 ((uint32_t)0x45670123)
#define FLASH_KEY2 ((uint32_t)0xCDEF89AB)
#define FLASH_LOCK ((uint32_t)1<<31)
#define FLASH_BUSY ((uint32_t)1<<16)

#define FLASH_PSIZE_DOUBLE_WORD ((uint32_t)0x00000300)
#define CR_PSIZE_MASK ((uint32_t)0xFFFFFCFF)
#define FLASH_ERRORS ((uint32_t)0x000000F2)
#define FLASH_EOP 	((uint32_t)0x00000001)
#define SECTOR_MASK ~((uint32_t)0xf<<3)

#define END_LINE 480 // line where to end writing

uint64_t buffer[BUFFERSIZE/8]; // u64 are 8 bytes

struct Sector {
	uint16_t sector_id;
	uint64_t *addr;
};

struct Sector sectors[]= {
	// { 0x00, 0x08000000}, // 16 Kbytes, sector 0 (reserved for bootloader), never written
	{ 0x08, (uint64_t *)0x08004000}, // 16 Kbytes, sector 1 (reserved for data ?)
	{ 0x10, (uint64_t *)0x08008000}, // 16 Kbytes, sector 2
	{ 0x18, (uint64_t *)0x0800C000}, // 16 Kbytes, sector 3
	{ 0x20, (uint64_t *)0x08010000}, // 64 Kbytes, sector 4
	{ 0x28, (uint64_t *)0x08020000}, // 128 Kbytes, sector 5
	{ 0x30, (uint64_t *)0x08040000}, // 128 Kbytes, sector 6
	{ 0x38, (uint64_t *)0x08060000}, // 128 Kbytes, sector 7
	{ 0x40, (uint64_t *)0x08080000}, // 128 Kbytes, sector 8
	{ 0x48, (uint64_t *)0x080A0000}, // 128 Kbytes, sector 9
	{ 0x50, (uint64_t *)0x080C0000}, // 128 Kbytes, sector 10
	{ 0x58, (uint64_t *)0x080E0000} , // 128 kbytes, sector 11

 	{0xffff,(uint64_t *)0x08100000}, // 0 Kbytes, sector "12" : extra sector to mark the end of the flash
}; 

#define NB_SECTORS (sizeof(sectors)/sizeof(Sector))

enum State {
	state_error, 
	state_erasing, 
	state_must_read, 
	state_writing, 
	state_idle
};

char flash_message[32];
char *HEX_Digits="0123456789ABCDEF";

// private
static int flash_state=state_idle;
static FIL *file_to_write;
static int current_sector;
static uint64_t *src, *dst;
static int bytes_to_write; // bytes to write on this buffer.
static int eof;

//-------------------------------------------------------

void flash_init()
{
	// we'll use dword (u64)
    FLASH->CR &= CR_PSIZE_MASK;
    FLASH->CR |= FLASH_PSIZE_DOUBLE_WORD;
    file_to_write=0;

	// verify BOR / voltage ? 
}

int flash_start_write(FIL *file) 
// async write the content of file to the start of flash.
{
	// check pas deja en train 
	if (file_to_write) return 0;

	file_to_write = file;
	current_sector=-1;
	dst=sectors[0].addr; 
	eof=0;

	// Unlock flash
	FLASH->KEYR = FLASH_KEY1;
	FLASH->KEYR = FLASH_KEY2;

	// launch it
	strcpy(flash_message,"Starting");
	flash_state = state_must_read;
	return 1;
}

void flash_frame()
/* will take as much time as needed for reading a buffer or reach line X */
{
	uint32_t r;

	switch (flash_state) {

		case state_must_read : 
			// needs reading (reading of a buffer must be done within a frame)
			src = &buffer[0];
			f_read(file_to_write,buffer,BUFFERSIZE,&bytes_to_write); // bytes read  = to write
			eof = bytes_to_write!=BUFFERSIZE;

			// if must erase before writing, start to do it
			if (dst >= sectors[current_sector+1].addr) { 
	   			current_sector++;
	   			if (sectors[current_sector].sector_id==0xffff)
	   			{
	   				strcpy(flash_message,"ERROR : FLASH overflow");
	   				return;
	   			}

			    FLASH->CR &= SECTOR_MASK;
			    FLASH->CR |= FLASH_CR_SER | sectors[current_sector].sector_id;
	    		FLASH->CR |= FLASH_CR_STRT;

	   			// update display
				strcpy(flash_message,"Erasing ...");
			}
			
			// after : erasing state (either already finished = not needed) or busy erasing
			flash_state = state_erasing;
			break;

		case state_erasing :
			r=FLASH->SR;	
			if (r & FLASH_BUSY) {
				set_led(vga_frame & 0x40);
			} else if (r & FLASH_EOP) {
			    /* if the erase operation is completed, disable the SER Bit */
			    FLASH->CR &= (~FLASH_CR_SER);
	    		FLASH->CR &= SECTOR_MASK;
				// nb bytes and source already set by read action
				strcpy(flash_message,"Writing ...");
				flash_state = state_writing;
			} else { // error
				strcpy(flash_message,"ERROR Erasing :");
				flash_message[16] = HEX_Digits[r & 0xf0 ];
				flash_message[17] = HEX_Digits[r & 0xf  ];

				flash_state == state_error;
			} 
			break;

		case state_writing :
			FLASH->CR |= FLASH_CR_PG;
			while (vga_line<END_LINE && bytes_to_write>0 ) {
				r=FLASH->SR;
				set_led(vga_frame & 0x40);
				if (r & FLASH_EOP) {
					(*dst++) = *src++; // write next element
					bytes_to_write -= sizeof(*src); 
				} else  {
					strcpy(flash_message,"Error Writing : ");
					flash_message[16] = HEX_Digits[r & 0xf0 ];
					flash_message[17] = HEX_Digits[r & 0xf  ];

					flash_state = state_error;
				}
			}
			FLASH->CR &= (~FLASH_CR_PG); // disable programming

			// finished writing ? read more if needed, stop if finished
			if (bytes_to_write==0 && !eof) 
				flash_state = state_must_read;
			else {
				flash_state = state_idle;
				strcpy(flash_message,"Done.");	
				FLASH->CR |= FLASH_CR_LOCK; // lock flash
			}
			
			break;

		case state_error : 
			set_led(vga_frame & 0x20);
			break;

		case state_idle : 
			break;
		
	} // switch
} 

/*

	See for sample code :  https://github.com/nabilt/STM32F4-Discovery-Firmware/tree/master/Project/Peripheral_Examples/FLASH_Program
 	see also "stm32f4xx_flash.h"  + .c ! 

*/