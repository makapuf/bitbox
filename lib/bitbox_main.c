#include "system.h"
#include "bitbox.h"

#ifndef NO_USB
#include "usb_bsp.h"
#include "usbh_core.h"
#include "usbh_hid_core.h"

__ALIGN_BEGIN USB_OTG_CORE_HANDLE           USB_OTG_Core __ALIGN_END ;
__ALIGN_BEGIN USBH_HOST                     USB_Host __ALIGN_END ;
__ALIGN_BEGIN USB_OTG_CORE_HANDLE           USB_OTG_FS_Core __ALIGN_END ;
__ALIGN_BEGIN USBH_HOST                     USB_FS_Host __ALIGN_END ;
#endif

// vga init, not public, it's done automatically
void vga640_setup();
void audio_frame(); // will call audio callback


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


void message(char *msg) {};

void main()
{
	InitializeSystem();
	board_init();
	audio_init();

	/* Init Host Library */
	#ifndef NO_USB
	USBH_Init(
		&USB_OTG_Core, 
		USB_OTG_HS_CORE_ID,
		&USB_Host, 
		&HID_cb);
	    // ,&USR_Callbacks);

	/* Init FS Core */
	USBH_Init(
		&USB_OTG_FS_Core, 
		USB_OTG_FS_CORE_ID,
		&USB_FS_Host,
		&HID_cb);
		// &USR_Callbacks);
	#endif

				/*
				#ifdef USE_USB_OTG_HS
				do { // while (frame < 1000) {
					 USBH_Process(&USB_OTG_Core, &USB_Host);
		 USBH_Process(&USB_OTG_FS_Core, &USB_FS_Host);
	 } while (USBH_Usr_InputDone == 0);
				#endif
				*/

	
	uint32_t oframe;
	__IO uint32_t oline; // used for debugging

	game_init();

	/* 
	start vga after game setup so that we can 
	setup data that will be used in line callbacks
	*/
	vga640_setup();

	while (1)
	{
		game_frame();
		
		oframe=vga_frame;

		do {
			oline = vga_line;
			#ifdef USE_USB_OTG_HS
				USBH_Process(&USB_OTG_Core, &USB_Host);
			#endif
			#ifdef USE_USB_OTG_FS
				USBH_Process(&USB_OTG_FS_Core, &USB_FS_Host);
			#endif
		} while (oframe==vga_frame);
		// wait next frame.

		set_led(button_state());

	} 
}
