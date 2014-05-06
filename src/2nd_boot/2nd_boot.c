// bootloader

// 2 - second stage bootloader from SD can load a larger bootloader 
// with customizable menu, UI, USB, whatever.


/*
TODO

Share as many possible files/libs with other projects (included button)

Include full kernel: video & USB
star demo/logo ?
read SD card data, text funcs / menu  : from engine
display image / text as selected when button pushed

write to Flash & jump to it.



 */

#define BOOTFILE "bitbox.bin"


#define START_RAM 0x20000000 // standard SRAM2, second - not CCRAM (see bitbox memories spreadsheet)
#define MAX_FILESIZE (112*1024) // 112k MAX !

#include <string.h>
#include "stm32f4xx.h" 
#include "fatfs/ff.h"
//#include "fatfs/stm32f4_discovery_sdio_sd.h"
#include "system.h" // InitializeSystem
#include "kernel.h"

enum {INIT =0, MOUNT=1, OPEN=2, READ=3}; // where we died

void die(int where, int cause);

void led_on() 
{
    GPIOA->BSRRL = 1<<2; 
}

void led_off() 
{
    GPIOA->BSRRH = 1<<2; 
}

void led_init() {
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
	led_init();
    

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
	blink(2,1);

	// read 128k max to RAM ! 
	r=f_read (&bootfile, (void*)START_RAM, MAX_FILESIZE, &bytes_read);
	if (r != FR_OK) die(READ,r);

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

    // set stack pointer as in application's vector table
    __set_MSP((u32)(ApplicationAddress[0]));
    Jump_To_Application();
}


// ---------------- die : should be integrated to kerne + emulator (as window title by example)

#define WAIT_TIME 168000000/128 // quick ticks 
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

extern uint8_t font_data [256][16];
char vram_char  [30][80];
char vram_attrs [30][80];

void print_at(int column, int line, const char *msg)
{
	strcpy(&vram_char[line][column],msg); 
}

// draws an empty window at this position, asserts x1<x2 && y1<y2
void window (int x1, int y1, int x2, int y2 )
{
	for (int i=x1+1;i<x2;i++) 
	{
		vram_char[y1][i]='\xCD';
		vram_char[y2][i]='\xCD';
	}
	for (int i=y1+1;i<y2;i++) 
	{
		vram_char[i][x1]='\xBA';
		vram_char[i][x2]='\xBA';
	}
	vram_char[y1][x1] ='\xC9';
	vram_char[y1][x2] ='\xBB'; 
	vram_char[y2][x1] ='\xC8'; 
	vram_char[y2][x2] ='\xBC'; 

}

void game_init() {
	window(2,2,60,5);
	print_at(10,3, "\x01 Bonjour, amis de la Bitbox !");
    // init FatFS
	/*
	memset(&fs32, 0, sizeof(FATFS));
	FRESULT r = f_mount(&fs32,"",1); //mount now
	if (r != FR_OK) die(MOUNT,r); 
	*/
}

int x =5,y=10 , dir_x=1, dir_y=1;
void game_frame() {
	if (vga_frame%2 != 0 ) return;
	// do something each ~ 1/4 seconds

	vram_char[y][x]=' ';
	if (x==79) dir_x = -1;
	if (x==0)  dir_x = 1;

	if (y==6)  dir_y = 1;
	if (y==29) dir_y = -1;

	x += dir_x;
	y += dir_y;
	vram_char[y][x] = '\x02';
}


void game_line() 
{

	// text mode

	uint16_t *dst = draw_buffer;
	char c;
	for (int i=0;i<80;i++) // column char
	{
		c = font_data[vram_char[vga_line / 16][i]][vga_line%16];
		// draw a character on this line
		for (int j=0x80;j>0;j>>=1)
		{
			// FIXME : attrs ?
			*dst++ = (c & j) ? 0x7fff:0; // attrs ...
		}
	}
}

void game_snd_buffer(uint16_t *buffer, int len) {}

/*
void main(void) {
	init();
	while(1) {
		wait(4);
		blink(16,1);
	}
	load();
	jump(START_RAM);
}
*/