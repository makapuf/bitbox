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

// TODO : check CRC ; if no card, boot from flash ... 

/* See Bitbox memories to see what happens ! */

#define BOOTFILE "2nd_boot.bin" // this is the standard filename of the firmware, to be located on the uSD root.

#define START_RAM 0x20000000 // standard SRAM2, second - not CCRAM (see bitbox memories spreadsheet)
#define START_FLASH_PGM 0x08004000 // standard flash (excepted this bootloader)
#define MAX_FILESIZE (112*1024) // 112k MAX !

#include <string.h>
#include "stm32f4xx.h"
#include "stm32f4_discovery_sdio_sd.h"
#include "system.h"

#include "ff.h"

enum {INIT =0, MOUNT=1, OPEN=2, READ=3}; // where we died
void die(int where, int cause);


int button_state()
{
	return GPIOE->IDR & GPIO_IDR_IDR_15;
}

void led_on() 
{
    GPIOA->BSRRL = 1<<2; 
}

void led_off() 
{
    GPIOA->BSRRH = 1<<2; 
}

void GPIO_init() {
	// button is PE15
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN; // enable GPIO 
	GPIOE->PUPDR |= GPIO_PUPDR_PUPDR15_0; // set input / pullup 
	
	// init LED GPIO
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN; // enable gpioA (+)
    GPIOA->MODER |= (1 << 4) ; // set pin 8 to be general purpose output
}

void blink(int times, int speed);

FATFS fs32;
// load from sd to RAM
// flash LED, boot

void NVIC_Configuration(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;

  /* Configure the NVIC Preemption Priority Bits */
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);

  NVIC_InitStructure.NVIC_IRQChannel = SDIO_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}

void init()
{
	InitializeSystem(); // now we're using system.c 
	NVIC_Configuration(); /* Interrupt Config */
	GPIO_init();
}

void init_fatfs()
{
	memset(&fs32, 0, sizeof(FATFS));
	FRESULT r = f_mount(&fs32,"",1); //mount now
	if (r != FR_OK) die(MOUNT,r); 
}

void load(void)
{
	FIL bootfile;
	memset(&bootfile, 0, sizeof(FIL));
	int bytes_read;

	// check presence & size of file or die
	FRESULT r=f_open (&bootfile,BOOTFILE,FA_READ | FA_OPEN_EXISTING);
	// check result
	if (r != FR_OK) die(OPEN,r);

	// Ack : blink OK : 2 quick times
	blink(1,1);

	// read to RAM 
	r=f_read (&bootfile, (void*)START_RAM, MAX_FILESIZE, &bytes_read);
	if (r != FR_OK) die(READ,r);
	
	blink(1,1);

	// check file content ? checksum =4last bytes ?
}

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

    // Clear pending interrupts just to be on the safe side
    //SCB_ICSR = ICSR_PENDSVCLR;

    // Disable all interrupts
    int i;
    for (i = 0; i < 8; i++)
            NVIC->ICER[i] = NVIC->IABR[i];
	blink(1,1);

    // set stack pointer as in application's vector table
    __set_MSP((u32)(ApplicationAddress[0]));
    Jump_To_Application();
}

void loop_check_sense(void)
{
	// SDIO sense is PC7
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN; // enable GPIO 
	GPIOC->PUPDR |= GPIO_PUPDR_PUPDR7_0; // set input / pullup 
	while(1) {
		GPIOC->IDR & GPIO_IDR_IDR_7 ? led_on() : led_off();
	}
}


void main(void) {
	init();
	// activate bootloader if button pressed.
	if (!button_state()) { 
		init_fatfs();
		load(); // blocks if NOK
		jump(START_RAM);
	} else {
		jump(START_FLASH_PGM);
	}
}


// ---------------- die

#define WAIT_TIME 168000000/64 // 1/4s ticks 
void wait(int k) {

	for (volatile int i=0;i<k*WAIT_TIME;i++) {};
}

void blink(int times, int speed)
{
	for (int i=0;i<times;i++) 
		{
			led_on();wait(speed);
			led_off();wait(speed);
		}
}

void die(int where, int cause)
{
	for (;;)
	{
		blink(where,2);
		wait(4);
		blink(cause,1);
		wait(4);
	}
}