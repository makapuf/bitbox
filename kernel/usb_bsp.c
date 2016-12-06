#include "kconf.h"

#include "system.h"

#include "stm32f4xx.h"
#include "usb_bsp.h"
#include "usbh_core.h"
#include "usbh_hid_core.h"
#include "usb_hcd_int.h"


ErrorStatus HSEStartUpStatus;

#ifdef USE_USB_OTG_FS  
USB_OTG_CORE_HANDLE USB_OTG_FS_Core;
USBH_HOST USB_FS_Host;

void OTG_FS_IRQHandler(void)
{
  USBH_OTG_ISR_Handler(&USB_OTG_FS_Core);
}
#endif


#ifdef USE_USB_OTG_HS  
USB_OTG_CORE_HANDLE USB_OTG_Core;
USBH_HOST USB_Host;

void OTG_HS_IRQHandler(void)
{
  USBH_OTG_ISR_Handler(&USB_OTG_Core);
}
#endif

void TIM4_IRQHandler()
{
  if (TIM4->SR & TIM_SR_UIF) // no reason not to
  {
    TIM4->SR &= ~TIM_SR_UIF; // clear UIF flag

      // process USB
      #ifdef USE_USB_OTG_FS
        USBH_Process(&USB_OTG_FS_Core, &USB_FS_Host);
      #endif

      #ifdef USE_USB_OTG_HS
        // set_led(USB_Host.gState == HOST_DEV_DISCONNECTED); // XXX DEBUG
        USBH_Process(&USB_OTG_Core, &USB_Host);
      #endif
  }
}


void setup_usb_timer()
{
  // Enable 125Hz USB timer timer 4
  RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;

  TIM4->PSC=999;  // Prescaler = 1000
  TIM4->ARR = SYSCLK/APB1_DIV/1000/125; // ~ 168MHz /2 / 1000 / 125Hz (65536 max, typically 704)
  /* XXX verify bInterval ?
    For low- and full-speed interrupt endpoints, the descriptor's bInterval value indicates 
    the requested maximum number of milliseconds between transaction attempts. 
    http://janaxelson.com/usbfaq.htm
  */
  TIM4->CR1 = TIM_CR1_ARPE; // autoreload preload enable, no other function on

  //InstallInterruptHandler(TIM4_IRQn,TIM4_IRQHandler);
  NVIC_EnableIRQ(TIM4_IRQn); 
  NVIC_SetPriority(TIM4_IRQn,15); // low priority

  TIM4->DIER = TIM_DIER_UIE; // enable interrupt
  TIM4->CR1 |= TIM_CR1_CEN; // go timer 7
}


void setup_usb_hardware(void)
{

  #ifdef USE_USB_OTG_FS
  /* Configure VBUS:PA9 ID:PA10 DM:PA11 DP:PA12 Pins  - STM32F405 here ! */  
  RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;  
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
  GPIOA->MODER |= GPIO_MODER_MODER9_0 * 0b10101010; // mode = AF(0b10)
  GPIOA->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR9_0 * 0b11111111; // SPEED=100MHz 
  GPIOA->PUPDR   &= ~(GPIO_PUPDR_PUPDR9_0*0b11111111); // set to 0 = no pull
  GPIOA->AFR[1] |= 0x000AAAA0 ; // PA9-12 alt func nb 10 (datasheet p45 + refman p153)
  RCC->AHB2ENR |= RCC_AHB2ENR_OTGFSEN;
  #endif 
  

  #ifdef  USE_USB_OTG_HS 
  // OTG HS - pins PB12:id 13:vbus 14:dm 15:dp / 100MHz / Alt / USB_OTG2_FS
  RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;  
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
  GPIOB->MODER |= GPIO_MODER_MODER12_0 * 0b10101010; // mode = AF(0b10)
  GPIOB->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR12_0 * 0b11111111; // SPEED=100MHz 
  GPIOB->PUPDR   &= ~(GPIO_PUPDR_PUPDR12_0*0b11111111); // set to 0 = no pull
  GPIOB->AFR[1] |= 0xCCCC0000UL ; // PB12-15 alt func nb 12 (datasheet p45 + refman p153)
  RCC->AHB1ENR |= RCC_AHB1ENR_OTGHSEN; 
  #endif
}

void setup_usb()
{
  /* Init FS/HS Cores */
  setup_usb_hardware();

  #ifdef USE_USB_OTG_HS
  USBH_DeInit(&USB_OTG_Core,&USB_Host);    /* Host de-initializations */
  USB_Host.class_cb = &HID_cb;
  HCD_Init(&USB_OTG_Core, USB_OTG_HS_CORE_ID); // Start the USB OTG core

  SetInterruptPriority(OTG_HS_IRQn,3); // enable interrupt
  EnableInterrupt(OTG_HS_IRQn);
  #endif

  #ifdef USE_USB_OTG_FS

  USBH_DeInit(&USB_OTG_FS_Core,&USB_FS_Host);    /* Host de-initializations */
  USB_FS_Host.class_cb = &HID_cb;
  HCD_Init(&USB_OTG_FS_Core, USB_OTG_FS_CORE_ID); // Start the USB OTG FS core
  
  SetInterruptPriority(OTG_FS_IRQn,3);  // Enable Interrupts
  EnableInterrupt(OTG_FS_IRQn);
  #endif

  setup_usb_timer();
}



// erased because vbus is always high by hardware
void USB_OTG_BSP_DriveVBUS(USB_OTG_CORE_HANDLE *pdev, uint8_t state) {}
void USB_OTG_BSP_ConfigVBUS(USB_OTG_CORE_HANDLE *pdev) {}


/**
  * @brief  USB_OTG_BSP_uDelay
  *         This function provides delay time in micro sec
  * @param  usec : Value of delay required in micro sec
  * @retval None
  */
void USB_OTG_BSP_uDelay (const uint32_t usec)
{

  __IO uint32_t count = 0;
  const uint32_t utime = (120 * usec / 7);
  do
  {
    if ( ++count > utime )
    {
      return ;
    }
  }
  while (1);

}

/**
  * @brief  USB_OTG_BSP_mDelay
  *          This function provides delay time in milli sec
  * @param  msec : Value of delay required in milli sec
  * @retval None
  */
void USB_OTG_BSP_mDelay (const uint32_t msec)
{
  USB_OTG_BSP_uDelay(msec * 1000);
}

