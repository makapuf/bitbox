/* flashit : aynchronous writing to flash from a SD card 
(c) makapuf2@gmail.com */


#include <stdint.h>
#include <string.h>
#include "bitbox.h" // vga_line, frame
#include "flashit.h"


volatile enum State flash_state=state_idle;
volatile int current_sector;
volatile int errno;


#ifdef EMULATOR

// flash stubs
int frame_started; // simulate a flash delay
void flash_init() {
	frame_started=vga_frame;
}

int flash_start_write(FIL *file) {
	frame_started = vga_frame;
	flash_state = state_writing;
	return 0;
}

void flash_frame() {
	if (frame_started && vga_frame-frame_started>3*60) {
		flash_state = state_done;
	} else {
		current_sector = (vga_frame-frame_started)/18;
	}
}

#else

#include "stm32f4xx.h" // CMSIS
#define PROGSIZE uint32_t
#define FLASH_PSIZE_WORD 0x200
#define CR_PSIZE_MASK ((uint32_t)0xFFFFFCFF)

// 8k buffer here. 16k must be divideable by it, better be readable in a frame on the SD
#define BUFFERSIZE (8*1024)

#define FLASH_KEY1 ((uint32_t)0x45670123)
#define FLASH_KEY2 ((uint32_t)0xCDEF89AB)
#define FLASH_LOCK ((uint32_t)1<<31)
#define FLASH_BUSY ((uint32_t)1<<16)

#define FLASH_ERRORS ((uint32_t)0x000000F2)
#define FLASH_EOP 	((uint32_t)0x00000001)
#define SECTOR_MASK ~((uint32_t)0xf<<3)

#define END_LINE 360 // line where to end writing

PROGSIZE buffer[BUFFERSIZE/sizeof(PROGSIZE)];

struct Sector {
	uint16_t sector_id;
	PROGSIZE *addr;
};

struct Sector sectors[]= {
	// { 0x00, 0x08000000}, // 16 Kbytes, sector 0 (reserved for bootloader), never written
	{ 0x08, (PROGSIZE *)0x08004000}, // 16 Kbytes, sector 1 
	{ 0x10, (PROGSIZE *)0x08008000}, // 16 Kbytes, sector 2
	{ 0x18, (PROGSIZE *)0x0800C000}, // 16 Kbytes, sector 3
	{ 0x20, (PROGSIZE *)0x08010000}, // 64 Kbytes, sector 4
	{ 0x28, (PROGSIZE *)0x08020000}, // 128 Kbytes, sector 5
	{ 0x30, (PROGSIZE *)0x08040000}, // 128 Kbytes, sector 6
	{ 0x38, (PROGSIZE *)0x08060000}, // 128 Kbytes, sector 7
	{ 0x40, (PROGSIZE *)0x08080000}, // 128 Kbytes, sector 8
	{ 0x48, (PROGSIZE *)0x080A0000}, // 128 Kbytes, sector 9
	{ 0x50, (PROGSIZE *)0x080C0000}, // 128 Kbytes, sector 10
	{ 0x58, (PROGSIZE *)0x080E0000} , // 128 kbytes, sector 11

 	{0xffff,(PROGSIZE *)0x08100000}, // 0 Kbytes, sector "12" : extra sector to mark the end of the flash
}; 

#define NB_SECTORS (sizeof(sectors)/sizeof(Sector))



static PROGSIZE *src, *dst;

static FIL *file_to_write;
static unsigned int bytes_to_write; // bytes to write on this buffer.
static int eof;

//-------------------------------------------------------

void flash_init()
{
    FLASH->CR &= CR_PSIZE_MASK;
    FLASH->CR |= FLASH_PSIZE_WORD;
    file_to_write=0;
    flash_state = state_idle;

	// verify BOR / voltage ? 
}

int flash_start_write(FIL *file) 
// async write the content of file to the start of flash.
{
	// check not busy
	if (file_to_write) return 0;

	file_to_write = file;
	current_sector=-1;
	dst=sectors[0].addr; 
	eof=0;

	// Unlock flash
	FLASH->KEYR = FLASH_KEY1;
	FLASH->KEYR = FLASH_KEY2;

	// verify unlocked
	if (FLASH->CR & (1<<31)) {
		flash_state = state_unlock_error;
	}

	// launch it
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

			r = f_read(file_to_write,buffer,BUFFERSIZE,&bytes_to_write); // bytes read  = to write
			if ( r != FR_OK ) {
				errno = r;
				flash_state = state_error_reading;
				return;
			}

			eof = bytes_to_write!=BUFFERSIZE;

			// if must erase before writing, start to do it
			if (dst >= sectors[current_sector+1].addr) { 
	   			current_sector++;
	   			if (sectors[current_sector].sector_id==0xffff)
	   			{
	   				flash_state = state_overflow;
	   				return;
	   			}

			    FLASH->CR &= SECTOR_MASK;
			    FLASH->CR |= FLASH_CR_SER | sectors[current_sector].sector_id;
	    		FLASH->CR |= FLASH_CR_STRT;
			}
			
			// after : erasing state (either already finished = not needed) or busy erasing
			flash_state = state_erasing;
			break;

		case state_erasing :
			r=FLASH->SR;	
			if (r & FLASH_BUSY) {
				set_led(vga_frame & 0x40);
			} else if (!r) { // finished ?
			    /* if the erase operation is completed, disable the SER Bit */
			    FLASH->CR &= (~FLASH_CR_SER);
	    		FLASH->CR &= SECTOR_MASK;
				// nb bytes and source already set by read action
				flash_state = state_writing;
			} else { // error
				errno=r;
				flash_state = state_error_erasing;
			} 
			break;

		case state_writing :
		    FLASH->CR &= CR_PSIZE_MASK;
    		FLASH->CR |= FLASH_PSIZE_WORD;
			FLASH->CR |= FLASH_CR_PG;
			while (vga_line != END_LINE && bytes_to_write>0 ) {
				set_led(vga_frame & 0x10);
				while (FLASH->SR & FLASH_BUSY);
				r=FLASH->SR;
				if (!r) { // finished, write next word
					*(__IO uint32_t*)dst++ = *src++; 
					bytes_to_write -= sizeof(PROGSIZE); 
				} else if (r&0xf0) { // Error ?
					errno = r;
					flash_state = state_error_writing;
				} // busy ? wait a bit.
			}
			FLASH->CR &= (~FLASH_CR_PG); // disable programming

			// finished writing ? read more if needed, stop if finished
			if (bytes_to_write==0) 
			{
				if (eof) {
					flash_state = state_done;
					file_to_write = 0;
					FLASH->CR |= FLASH_CR_LOCK; // relock flash
				} else {
					flash_state = state_must_read;
				}
			}

			break;

		case state_error_writing : 
		case state_error_reading : 
		case state_error_erasing : 
		case state_overflow : 
		case state_idle : 
		case state_done : 
		case state_unlock_error:
			break;
		
	} // switch
} 

/*

	See for sample code :  https://github.com/nabilt/STM32F4-Discovery-Firmware/tree/master/Project/Peripheral_Examples/FLASH_Program
 	see also "stm32f4xx_flash.h"  + .c ! 

*/
#endif // emulator
