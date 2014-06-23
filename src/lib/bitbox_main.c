#include "../lib/system.h"
#include "../lib/kernel.h"

#ifdef GAMEPAD
#include "gamepad.h"
#include "usb_bsp.h"
#include "usbh_core.h"
#include "usbh_usr.h"
#include "usbh_hid_core.h"
#include "usbh_hid_gamepad.h"
#endif

#ifdef GAMEPAD
/** @defgroup USBH_USR_MAIN_Private_Variables
* @{
*/
#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
  #if defined ( __ICCARM__ ) /*!< IAR Compiler */
    #pragma data_alignment=4   
  #endif
#endif /* USB_OTG_HS_INTERNAL_DMA_ENABLED */
__ALIGN_BEGIN USB_OTG_CORE_HANDLE           USB_OTG_Core __ALIGN_END ;

#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
  #if defined ( __ICCARM__ ) /*!< IAR Compiler */
    #pragma data_alignment=4   
  #endif
#endif /* USB_OTG_HS_INTERNAL_DMA_ENABLED */
__ALIGN_BEGIN USBH_HOST                     USB_Host __ALIGN_END ;

#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
  #if defined ( __ICCARM__ ) /*!< IAR Compiler */
    #pragma data_alignment=4   
  #endif
#endif /* USB_OTG_HS_INTERNAL_DMA_ENABLED */
__ALIGN_BEGIN USB_OTG_CORE_HANDLE           USB_OTG_FS_Core __ALIGN_END ;

#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
  #if defined ( __ICCARM__ ) /*!< IAR Compiler */
    #pragma data_alignment=4   
  #endif
#endif /* USB_OTG_HS_INTERNAL_DMA_ENABLED */
__ALIGN_BEGIN USBH_HOST                     USB_FS_Host __ALIGN_END ;
/**
* @}
*/ 
#endif

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

	#ifdef GAMEPAD
  /* Init Host Library */
#ifdef USE_USB_OTG_HS
    USBH_Init(&USB_OTG_Core, 
            USB_OTG_HS_CORE_ID,
            &USB_Host,
            &HID_cb, 
            &USR_Callbacks);
#endif

  /* Init FS Core */
#ifdef USE_USB_OTG_FS
    USBH_Init(&USB_OTG_FS_Core, 
            USB_OTG_FS_CORE_ID,
            &USB_FS_Host,
            &HID_cb, 
            &USR_Callbacks);
#endif
	// gamepad_init();
	#endif

        /*
        #ifdef USE_USB_OTG_HS
        do { // while (frame < 1000) {
           USBH_Process(&USB_OTG_Core, &USB_Host);
	   USBH_Process(&USB_OTG_FS_Core, &USB_FS_Host);
	 } while (USBH_Usr_InputDone == 0);
        #endif
        */

	#ifdef AUDIO
	audio_init();
	#endif
	

  uint32_t oframe;
  __IO uint32_t oline;

  game_init();

  // start vga after game setup
	vga640_setup();

	while (1)
	{


		game_frame();
		
		// wait next frame
    gamepad1 = 0;
		oframe=vga_frame;

    do 
    {
        #ifdef GAMEPAD
            oline = vga_line;
            #ifdef USE_USB_OTG_HS
                 USBH_Process(&USB_OTG_Core, &USB_Host);
                 gamepad1 |= HID_GAMEPAD_Data[0].button;
            #endif
            #ifdef USE_USB_OTG_FS
                 USBH_Process(&USB_OTG_FS_Core, &USB_FS_Host);
                 gamepad1 |= HID_GAMEPAD_Data[1].button;
            #endif
		        // while (oline == line); 
                // } while (line < 450);
        #endif
		} while (oframe==vga_frame);

    set_led(button_state());

	}; // all work done inside interrupts
}
