#include "system.h"
#include "bitbox.h"

// very simple profiling statistics 
#define PROFILE_DECL(name) uint32_t name##_time,name##_min=0xffffffff, name##_max, name##_tot, name##_nb; // stats defs
#define PROFILE_TIC(name) name##_time= DWT->CYCCNT; 
#define PROFILE_TOC(name) name##_time = DWT->CYCCNT - name##_time; \
		if (name##_time>name##_max) name##_max=name##_time; \
		if (name##_time<name##_min) name##_min=name##_time; \
		name##_tot += name##_time; \
		name##_nb++; 



#ifndef NO_USB
#include "usb_bsp.h"
#include "usbh_core.h"
#include "usbh_hid_core.h"

__ALIGN_BEGIN USB_OTG_CORE_HANDLE           USB_OTG_Core __ALIGN_END ;
__ALIGN_BEGIN USBH_HOST                     USB_Host __ALIGN_END ;
__ALIGN_BEGIN USB_OTG_CORE_HANDLE           USB_OTG_FS_Core __ALIGN_END ;
__ALIGN_BEGIN USBH_HOST                     USB_FS_Host __ALIGN_END ;

PROFILE_DECL(usb);
#endif

PROFILE_DECL(sound);

// vga init, not public, it's done automatically
void vga_setup();
extern uint16_t *audio_write; // draw sound buffer. accessible by mùain thread.

void board_init()
{
	// User LED  
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN; // enable gpioA
	GPIOA->MODER |= (1 << 4) ; // set pin 2 to be general purpose output

	// button is PE15
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN; // enable GPIO 
	GPIOE->PUPDR |= GPIO_PUPDR_PUPDR15_0; // set input / pullup 
	
	// SDIO sense is PC7
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN; // enable GPIO 
	GPIOC->PUPDR |= GPIO_PUPDR_PUPDR7_0; // set input / pullup 

	// Profiling
	CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
	DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk ; // enable the cycle counter
}



void toggle_led()
{
	GPIOA->ODR ^= 1<<2; // led on/off
}

void set_led(int value)
{
	if (value)
		GPIOA->BSRRL |= 1<<2;
	else
		GPIOA->BSRRH |= 1<<2;
}

int button_state()
{
	return GPIOE->IDR & GPIO_IDR_IDR_15;
}

int sdio_sense_state()
{
	return GPIOC->IDR & GPIO_IDR_IDR_7;
}


void message(const char * msg, ...) {
	// do nothing on bitbox (UART ? write to flash ? to RAM )
}

int main(void)
{
	InitializeSystem();
	board_init();
	audio_init();

	/* Init Host Library */
	#ifndef NO_USB

	#ifdef USE_USB_OTG_HS
	USBH_Init(
		&USB_OTG_Core, 
		USB_OTG_HS_CORE_ID,
		&USB_Host, 
		&HID_cb);
	#endif 

	#ifdef USE_USB_OTG_FS
	/* Init FS Core */
	USBH_Init(
		&USB_OTG_FS_Core, 
		USB_OTG_FS_CORE_ID,
		&USB_FS_Host,
		&HID_cb);
	#endif
	#endif 

	uint32_t oframe;

	game_init();

	/* 
	start vga after game setup so that we can 
	setup data that will be used in line callbacks
	*/
	vga_setup();

	while (1)
	{

		game_frame();
		
		// must be finished by the end of frame 
		PROFILE_TIC(sound);
		game_snd_buffer(audio_write,BITBOX_SNDBUF_LEN); 
		PROFILE_TOC(sound);

		oframe=vga_frame;

		// wait next frame.
		do {
			#ifndef NO_USB
			#ifdef USE_USB_OTG_FS
				USBH_Process(&USB_OTG_FS_Core, &USB_FS_Host);
			#endif

			#ifdef USE_USB_OTG_HS
				set_led(USB_Host.gState == HOST_DEV_DISCONNECTED); 
				PROFILE_TIC(usb);
				USBH_Process(&USB_OTG_Core, &USB_Host);
				PROFILE_TOC(usb);	
			#endif
			#endif 
		} while (oframe==vga_frame);


			
		set_led(button_state());

	} 
}

// die : standard blinking to sgnal on the user led where we died and why

#define WAIT_TIME 168000000/128 // quick ticks, random number, good enough
void wait(int k) {
	for (volatile int i=0;i<k*WAIT_TIME;i++) {};
}

void blink(int times, int speed) {
	for (int i=0;i<times;i++) 
		{
			set_led(1);wait(speed);
			set_led(0);wait(speed);
		}
}

void die(int where, int cause)
{
	for (;;)
	{
		blink(where,3);
		wait(4);
		blink(cause,2);
		wait(4);
	}
}
