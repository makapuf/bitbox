// bootloader

// auto load+startup from RAM, no GUI given 
// check if sdcard present, if not run from flash sector 2
// beware : boot.box must be compiled to run from ram !

// 1 - load bootloader from SD to RAM. Ultra minimal, always the same name, boot from it.
// first stage boootloader MUST be compiled & linked to be located in SRAM

// 2 - second stage bootloader from SD can load a larger bootloader 
// with customizable menu, UI, USB, whatever.

// maybe defrag EEPROM DATA there.

//config for ff_conf : no long filenames, no dirs, only read (minimal)


#define BOOTFILE "bitbox.bin"
#define START_RAM 0x2000000 // standard SRMA, not CCRAM, which is at 0x1000 0000
#define MAX_FILESIZE (128*1024) // 128k

#include "ff.h"

enum {INIT, OPEN, READ}; // where we died
void die(int where, int cause);

void led_on() 
{
    GPIOA->ODR |= 1<<8; 
}

void led_off() 
{
    GPIOA->ODR &= ~(1<<8); 
}


FIL bootfile;
// load from sd to RAM
// flash LED, boot

void init()
{
	// init LED GPIO
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN; // enable gpioA
    GPIOA->MODER |= (1 << 16) ; // set pin 8 to be general purpose output

    // init FatFS
	FRESULT r=init_ff();
	if (r != E_OK) die(INIT,r); 
}


void load(void)
{
	int bytes_read;

	// check presence & size of file or die
	FRESULT r=f_open (&bootfile,BOOTFILE,FA_OPEN_EXISTING);
	// check result
	if (r != F_OK) die(OPEN,r);

	// Ack blink OK : 2 times
	blink(2,2);

	// read 128k max to RAM ! 

	r=f_read (&bootfile, (void*)START_RAM, MAX_FILESIZE, &bytes_read);
	if (r != F_OK) die(READ,r);

	// check file content ? checksum =4last bytes ?

);

// Code stolen from "matis"
// http://forum.chibios.org/phpbb/viewtopic.php?f=2&t=338
void jump(uint32_t address)
{
    typedef void (*pFunction)(void);

    pFunction Jump_To_Application;

    // variable that will be loaded with the start address of the application
    uint32_t* JumpAddress;
    const uint32_t* ApplicationAddress = (uint32_t*) address;

    // get jump address from application vector table
    JumpAddress = (uint32_t*) ApplicationAddress[1];

    // load this address into function pointer
    Jump_To_Application = (pFunction) JumpAddress;

    // reset all interrupts to default
    // chSysDisable();

    // Clear pending interrupts just to be on the save side
    SCB_ICSR = ICSR_PENDSVCLR;

    // Disable all interrupts
    int i;
    for (i = 0; i < 8; i++)
            NVIC->ICER[i] = NVIC->IABR[i];

    // set stack pointer as in application's vector table
    __set_MSP((u32)(ApplicationAddress[0]));
    Jump_To_Application();
}


void main(void) {
	init();
	load();
	jump(START_RAM);
}


// ---------------- die

#define WAIT_TIME 168000000/4 // 1/4s ticks 
void wait(int k) {

	for (int i=0;i<k*WAIT_TIME;i++) {};
}

void blink(int times, int speed)
{
	for (int i=0;i<times;i++) 
		{
			led_on();wait(1);
			led_off();wait(1);
		}
}

void die(int where, int cause)
{
	for (;;)
	{
		blink(where,2);
		blink(cause,1);
		wait(4);
	}
}