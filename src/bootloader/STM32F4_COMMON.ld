/*
Linker script for STM32F40x Common - sourcer32@gmail.com
*/

/* default stack sizes.

These are used by the startup in order to allocate stacks for the different modes.
*/

__Stack_Size = 8192 ;

PROVIDE ( _Stack_Size = __Stack_Size ) ;

__Stack_Init = _estack  - __Stack_Size ;

/*"PROVIDE" allows to easily override these values from an object file or the commmand line.*/
PROVIDE ( _Stack_Init = __Stack_Init ) ;

/*
There will be a link error if there is not this amount of RAM free at the end.
*/
_Minimum_Stack_Size = 0x100 ;



/*
this sends all unreferenced IRQHandlers to reset
*/


PROVIDE (   Undefined_Handler = 0 ) ;
PROVIDE (   SWI_Handler = 0 ) ;
PROVIDE (   IRQ_Handler = 0 ) ;
PROVIDE (   Prefetch_Handler = 0 ) ;
PROVIDE (   Abort_Handler = 0 ) ;
PROVIDE (   FIQ_Handler = 0 ) ;

PROVIDE (   NMIException = 0 ) ;
PROVIDE (   HardFaultException = 0 ) ;
PROVIDE (   MemManageException = 0 ) ;
PROVIDE (   BusFaultException = 0 ) ;
PROVIDE (   UsageFaultException = 0 ) ;
PROVIDE (   SVCHandler = 0 ) ;
PROVIDE (   DebugMonitor = 0 ) ;
PROVIDE (   PendSVC = 0 ) ;
PROVIDE (   SysTickHandler = 0 ) ;

PROVIDE (   WWDG_IRQHandler = 0 ) ;
PROVIDE (   PVD_IRQHandler = 0 ) ;
PROVIDE (   TAMP_STAMP_IRQHandler = 0 ) ;
PROVIDE (   RTC_WKUP_IRQHandler = 0 ) ;
PROVIDE (   FLASH_IRQHandler = 0 ) ;
PROVIDE (   RCC_IRQHandler = 0 ) ;
PROVIDE (   EXTI0_IRQHandler = 0 ) ;
PROVIDE (   EXTI1_IRQHandler = 0 ) ;
PROVIDE (   EXTI2_IRQHandler = 0 ) ;
PROVIDE (   EXTI3_IRQHandler = 0 ) ;
PROVIDE (   EXTI4_IRQHandler = 0 ) ;
PROVIDE (   DMA1_Stream0_IRQHandler = 0 ) ;
PROVIDE (   DMA1_Stream1_IRQHandler = 0 ) ;
PROVIDE (   DMA1_Stream2_IRQHandler = 0 ) ;
PROVIDE (   DMA1_Stream3_IRQHandler = 0 ) ;
PROVIDE (   DMA1_Stream4_IRQHandler = 0 ) ;
PROVIDE (   DMA1_Stream5_IRQHandler = 0 ) ;
PROVIDE (   DMA1_Stream6_IRQHandler = 0 ) ;
PROVIDE (   ADC_IRQHandler = 0 ) ;
PROVIDE (   CAN1_TX_IRQHandler = 0 ) ;
PROVIDE (   CAN1_RX0_IRQHandler = 0 ) ;
PROVIDE (   CAN1_RX1_IRQHandler = 0 ) ;
PROVIDE (   CAN1_SCE_IRQHandler = 0 ) ;
PROVIDE (   EXTI9_5_IRQHandler = 0 ) ;
PROVIDE (   TIM1_BRK_TIM9_IRQHandler = 0 ) ;
PROVIDE (   TIM1_UP_TIM10_IRQHandler = 0 ) ;
PROVIDE (   TIM1_TRG_COM_TIM11_IRQHandler = 0 ) ;
PROVIDE (   TIM1_CC_IRQHandler = 0 ) ;
PROVIDE (   TIM2_IRQHandler = 0 ) ;
PROVIDE (   TIM3_IRQHandler = 0 ) ;
PROVIDE (   TIM4_IRQHandler = 0 ) ;
PROVIDE (   I2C1_EV_IRQHandler = 0 ) ;
PROVIDE (   I2C1_ER_IRQHandler = 0 ) ;
PROVIDE (   I2C2_EV_IRQHandler = 0 ) ;
PROVIDE (   I2C2_ER_IRQHandler = 0 ) ;
PROVIDE (   SPI1_IRQHandler = 0 ) ;
PROVIDE (   SPI2_IRQHandler = 0 ) ;
PROVIDE (   USART1_IRQHandler = 0 ) ;
PROVIDE (   USART2_IRQHandler = 0 ) ;
PROVIDE (   USART3_IRQHandler = 0 ) ;
PROVIDE (   EXTI15_10_IRQHandler = 0 ) ;
PROVIDE (   RTC_Alarm_IRQHandler = 0 ) ;
PROVIDE (   OTG_FS_WKUP_IRQHandler = 0 ) ;
PROVIDE (   TIM8_BRK_TIM12_IRQHandler = 0 ) ;
PROVIDE (   TIM8_UP_TIM13_IRQHandler = 0 ) ;
PROVIDE (   TIM8_TRG_COM_TIM14_IRQHandler = 0 ) ;
PROVIDE (   TIM8_CC_IRQHandler = 0 ) ;
PROVIDE (   DMA1_Stream7_IRQHandler = 0 ) ;
PROVIDE (   FSMC_IRQHandler = 0 ) ;
PROVIDE (   SDIO_IRQHandler = 0 ) ;
PROVIDE (   TIM5_IRQHandler = 0 ) ;
PROVIDE (   SPI3_IRQHandler = 0 ) ;
PROVIDE (   UART4_IRQHandler = 0 ) ;
PROVIDE (   UART5_IRQHandler = 0 ) ;
PROVIDE (   TIM6_DAC_IRQHandler = 0 ) ;
PROVIDE (   TIM7_IRQHandler = 0 ) ;
PROVIDE (   DMA2_Stream0_IRQHandler = 0 ) ;
PROVIDE (   DMA2_Stream1_IRQHandler = 0 ) ;
PROVIDE (   DMA2_Stream2_IRQHandler = 0 ) ;
PROVIDE (   DMA2_Stream3_IRQHandler = 0 ) ;
PROVIDE (   DMA2_Stream4_IRQHandler = 0 ) ;
PROVIDE (   ETH_IRQHandler = 0 ) ;
PROVIDE (   ETH_WKUP_IRQHandler = 0 ) ;
PROVIDE (   CAN2_TX_IRQHandler = 0 ) ;
PROVIDE (   CAN2_RX0_IRQHandler = 0 ) ;
PROVIDE (   CAN2_RX1_IRQHandler = 0 ) ;
PROVIDE (   CAN2_SCE_IRQHandler = 0 ) ;
PROVIDE (   OTG_FS_IRQHandler = 0 ) ;
PROVIDE (   DMA2_Stream5_IRQHandler = 0 ) ;
PROVIDE (   DMA2_Stream6_IRQHandler = 0 ) ;
PROVIDE (   DMA2_Stream7_IRQHandler = 0 ) ;
PROVIDE (   USART6_IRQHandler = 0 ) ;
PROVIDE (   I2C3_EV_IRQHandler = 0 ) ;
PROVIDE (   I2C3_ER_IRQHandler = 0 ) ;
PROVIDE (   OTG_HS_EP1_OUT_IRQHandler = 0 ) ;
PROVIDE (   OTG_HS_EP1_IN_IRQHandler = 0 ) ;
PROVIDE (   OTG_HS_WKUP_IRQHandler = 0 ) ;
PROVIDE (   OTG_HS_IRQHandler = 0 ) ;
PROVIDE (   DCMI_IRQHandler = 0 ) ;
PROVIDE (   CRYP_IRQHandler = 0 ) ;
PROVIDE (   HASH_RNG_IRQHandler = 0 ) ;
PROVIDE (   FPU_IRQHandler = 0 ) ;

