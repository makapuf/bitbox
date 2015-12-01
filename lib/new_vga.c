/*

mode as defined in kconf.h values 
- output is on PORT E, lower 15 bits
- sync is on PORT A, pin0 (vsync), pin1 (hsync)

*/


/*
  TODO : audio, exclude headers, ...
  soft reflash doesnt enable output
*/

/* 
	PA1 (HSYNC) output is driven by Timer5 with CC 2 (see Table9 of datasheet, p60) using pwm mode
	Timer 1 (PIXEL DMA) is started as a slave of Timer5 CC1 via ITR1 (see config options table 76, p463 of refman)
   	TIM1_UP drives the DMA2 on stream 5 channel 6 (ref manual p164)
	DMA2 outputs to gpio PE0-15 ( refman p48 - DMA1 cannot drive AHB1 )
*/


/* debug with gdb : use 

	disp *(uint32_t *)0xE0001004 - line_time

	*(DMA_Stream_TypeDef*)0x40026488 (DMA2stream5 )
	*(DMA_TypeDef *)0x40026400  DMA2 status
	*(TIM_TypeDef*) 0x40010000 TIM1
	*(TIM_TypeDef*) 0x40000c00 TIM5
*/

#include <stdint.h>

#include "system.h" // interrupts 
#include "kconf.h"
#include "stm32f4xx.h" // ports, timers, profile

// --- local
#define TIMER_CYCL (SYSCLK/VGA_VFREQ/APB1_DIV)
#define SYNC_END (VGA_H_SYNC*TIMER_CYCL/(VGA_H_PIXELS+VGA_H_SYNC+VGA_H_FRONTPORCH+VGA_H_BACKPORCH))
#define BACKPORCH_END ((VGA_H_SYNC+VGA_H_BACKPORCH)*TIMER_CYCL/(VGA_H_PIXELS+VGA_H_SYNC+VGA_H_FRONTPORCH+VGA_H_BACKPORCH))


#ifdef PROFILE
// from http://forums.arm.com/index.php?/topic/13949-cycle-count-in-cortex-m3/
// also average ?
uint32_t line_time; // maximum time of line 
// gdb : disp *(uint32_t *)0xE0001004
#endif 


#define MIN(x,y) ((x)<(y)?x:y) 

extern void graph_line(void);
extern void graph_frame(void);

// public interface
uint32_t vga_line;
volatile uint32_t vga_frame; 

#ifdef VGA_SKIPLINE
volatile int vga_odd; // only in skipline modes
#endif 

// aligned on a 1kb boundary , see http://blog.frankvh.com/2011/08/18/stm32f2xx-dma-controllers/

uint16_t LineBuffer1[1024] __attribute__((aligned (1024))); // __attribute__ ((section (".sram")))
uint16_t LineBuffer2[1024] __attribute__((aligned (1024)));

uint16_t *display_buffer = LineBuffer1; // will be sent to display 
uint16_t *draw_buffer = LineBuffer2; // will be drawn (bg already drawn)


static inline void vga_output_black()
{
    GPIOE->BSRR |= GPIO_BSRR_BR_0*0x7fff; // Set signal to black. 
} 

static inline void vga_raise_vsync() 
{
	GPIOA->BSRR |= GPIO_BSRR_BS_0; // raise VSync line
}

static inline void vga_lower_vsync() 
{
	GPIOA->BSRR |= GPIO_BSRR_BR_0; // raise VSync line
}



void vga_setup()
{

	for (int i=0;i<1024;i++)
	{
		LineBuffer1[i]=0;
		LineBuffer2[i]=0;
	}

	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

	// initialize software state.
	vga_line=0;	vga_frame=0;
	
	display_buffer=LineBuffer1;
	draw_buffer=LineBuffer2;
	
	
	// --- GPIO ---------------------------------------------------------------------------------------
	// init vga gpio ports A0 vsync, A1 hsync B0-11 dac
	// XXX here port PC11 is used for vsync !!!  

	// GPIO A pins 0 (vsync- not bitbox prototype) & 1 (hsync) and GPIO E for pixel DAC
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN; // enable gpio E

	// GPIO A pins 0 (vsync- not bitbox prototype) & 1 (hsync) and GPIO E for pixel DAC

	// - Configure DAC pins in GPIOE 0-14
	// PA0-7 out, pushpull(default), 50MHz, pulldown
	
	// clear bits 0-14 (bit15 being User Button)
    GPIOE->MODER    |= 0b00010101010101010101010101010101; // 00:IN 01:OUT 10:ALT 11:ANALOG x15 bits
    GPIOE->OSPEEDR  |= 0b00101010101010101010101010101010; // 10 : SPEED=50MHz 
    GPIOE->PUPDR    |= 0b00101010101010101010101010101010; // 10 = Pull Down x 15

	vga_output_black();

	// - Configure sync pins as GPIOA 0 (vsync - out) , 1 (hsync : alt - PWM) high speed each
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN; // enable gpioA
	GPIOA->MODER |= GPIO_MODER_MODER0_0 * 0b01 | GPIO_MODER_MODER1_0 * 0b10; // PA0 out, PA1 alternate 
    GPIOA->OSPEEDR  |= 0b1010; // 10 : SPEED=50MHz 

	GPIOA->AFR[0] |= 2<<4; // TIM 5 CH 2, see Table9 of datasheet, p60 : alt func 2 is PA1
    GPIOA->PUPDR  |=  GPIO_PUPDR_PUPDR0_0 * 0b01 | GPIO_PUPDR_PUPDR1_0 * 0b01 ; // PB1, PB15 pullup 01

	// drive them high
	//GPIOA->BSRR |= GPIO_BSRR_BS_0 | GPIO_BSRR_BS_1; // raise VSync line
	GPIOA->BSRR=(1<<1) | (1<<0);

	// --- TIMERS ---------------------------------------------------------------------------------------
	
	// TIMER 5

	// -- Configure timer 5 as the HSync timer. Timer 5 runs at half frequency
	// CC2 is used to generate the HSync pulse, using PWM mode and driving the pin directly (channel 2 timer 5 drives gpio pin ..)
	// CC3 is used to generate a trigger signal for TIM1, which drives the pixel DMA, though ITR1
	// CC4 is used to trigger a soft interrupt
	
	// -- Global setup

	// enable timer 5
	RCC->APB1ENR |= RCC_APB1ENR_TIM5EN;

	TIM5->PSC=0; // XXX debug Prescaler = 1 : PSC=0
	TIM5->CR1=TIM_CR1_ARPE; // autoreload preload enable, no other function on
	TIM5->DIER=TIM_DIER_UIE; // Enable update interrupt.
	TIM5->CCER=0; // Disable CC, so we can program it.
	// TIM5->ARR=2796-1; // 88 MHz (OC) / 31.46875 kHz = 2796.42502483 (88 = 176MHz / 2 )
	// TIM5->ARR=3050-1; // 96 MHz (OC) / 31.46875 kHz = 3 050.64548 (96 = 192MHz / 2 )
	TIM5->ARR=TIMER_CYCL-1; // 96 MHz (OC) / 31.46875 kHz = 3 050.64548 (96 = 192MHz / 2 )

	// -- Channel 2  : Hsync pulse

	// On CNT==0: sync pulse start
	// set to PWM mode active level on reload, passive level after CC2-match.
	TIM5->CCMR1=6*TIM_CCMR1_OC2M_0;
	// output enabled, reversed polarity (active low).
	TIM5->CCER=TIM_CCER_CC2E|TIM_CCER_CC2P; 
	// 88 MHz * 3.813 microseconds = 335.544 - sync pulse end
	// TIM5->CCR2=336; 
    // 96 MHz * 3.813 microseconds = 366.048 - sync pulse end

	TIM5->CCR2=SYNC_END; 

	// -- Channel 3 : Trigger signal for TIM1 - will start on ITR1

	// Master mode selection OC3REF signal is used as trigger output (TRGO)
	TIM5->CR2=(0b110*TIM_CR2_MMS_0); 

	// Channel 3 set to passive level on reload, active level after CC3-match.
	TIM5->CCMR2=7*TIM_CCMR2_OC3M_0; 

	// 88 MHz * (3.813 + 1.907) microseconds = 503.36 - back porch end, start pixel clock
    // -14 is a kludge to account for slow start of timer.
	// TIM5->CCR3=503-14;

	// 96 MHz * (3.813 + 1.907) microseconds = 549.12 - back porch end, start pixel clock
    // -14 is a kludge to account for slow start of timer.
	TIM5->CCR3=BACKPORCH_END-14;

	// Enable HSync timer.

	// -- Channel 4 : software Hsync interrupt
	// 88 MHz * (3.813 + 1.907) microseconds = 503.36 - back porch end, start pixel clock
	//TIM5->CCER=TIM_CCER_CC4E|TIM_CCER_CC4P; // Channel 4 enabled, reversed polarity (active low).
	// TIM5->CCR4=503;
	// 96 MHz * (3.813 + 1.907) microseconds = 549.12 - back porch end, start pixel clock
	//TIM5->CCER=TIM_CCER_CC4E|TIM_CCER_CC4P; // Channel 4 enabled, reversed polarity (active low).
	TIM5->CCR4=BACKPORCH_END;
	// wait for last line ? 

	// Enable HSync timer interrupt and set highest priority.
	//InstallInterruptHandler(TIM5_IRQn,TIM5_IRQHandler);
	NVIC_EnableIRQ(TIM5_IRQn);
	NVIC_SetPriority(TIM5_IRQn,0);

	TIM5->CNT=-10; // Make sure it hits ARR. 
	TIM5->CR1|=TIM_CR1_CEN; // Go.


	// --- TIMER 1 : DMA2 pixel clock
	
	// enable it
	RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;
	// Prescaler = 1.
	TIM1->PSC=0; 
	// loop each "pixelclock"
	TIM1->ARR=VGA_PIXELCLOCK-1; 
	// autoreload preload enable, no other function on
	TIM1->CR1=TIM_CR1_ARPE;
	// Enable update DMA request interrupt
	TIM1->DIER=TIM_DIER_UDE; 
	// Only run TIM1 when TIM5 trigger-out is high

	// SMS=5 : set Gated mode (clk enabled on input), 
	// TS: selection ITR0 : for timer 1 this is timer 5 (ref464)
	TIM1->SMCR=(5*TIM_SMCR_SMS_0)|(0*TIM_SMCR_TS_0); 

	// --- DMA -----------------------------------------------------------------------------------------

	// TIM1_UP drives the DMA2 on stream 5 channel 6 (ref. manual p168)
	
	RCC->AHB1ENR |= RCC_AHB1ENR_DMA2EN; // DMA2 enable

	// Stop it and configure interrupts.
	DMA2_Stream5->CR&=~DMA_SxCR_EN;
	
	//NVIC_DisableIRQ(DMA2_Stream5_IRQn);
	//InstallInterruptHandler(DMA2_Stream5_IRQn,DMACompleteHandler);
	NVIC_EnableIRQ(DMA2_Stream5_IRQn);
	NVIC_SetPriority(DMA2_Stream5_IRQn,0);

}


static void prepare_pixel_DMA()
{
	// Visible line. Configure and enable pixel DMA.
	DMA2_Stream5->CR=(6*DMA_SxCR_CHSEL_0)| // Channel 6
	(3*DMA_SxCR_PL_0)| // Priority 3
	(1*DMA_SxCR_PSIZE_0)| // PSIZE = 16 bit
	(1*DMA_SxCR_MSIZE_0)| // MSIZE = 16 bit
	DMA_SxCR_MINC| // Increase memory address
	(1*DMA_SxCR_DIR_0)| // Memory to peripheral
	DMA_SxCR_TCIE |  // Transfer complete interrupt 
	(1*DMA_SxCR_MBURST_0); // burst on the memory-side

	DMA2_Stream5->NDTR=VGA_H_PIXELS+1; // transfer N pixels + one last _black_ pixel
	DMA2_Stream5->PAR=((uint32_t)&(GPIOE->ODR));
	DMA2_Stream5->M0AR=(uint32_t)display_buffer; // XXX +MARGIN*2;

	// Enable FIFO (see p190 of ref manual)
	// XXX DMA FIFO (as MBURST/ref p236) ?
	DMA2_Stream5->FCR |= DMA_SxFCR_DMDIS; 

	// Enable pixel clock. Clock will only start once TIM2 allows it.

	TIM1->DIER=0; // Update DMA request has to be disabled while zeroing the counter.
	TIM1->EGR=TIM_EGR_UG; // Force an update event to reset counter. Setting CNT is not reliable.
	TIM1->DIER=TIM_DIER_UDE; // Re-enable update DMA request.

	TIM1->CR1|=TIM_CR1_CEN; // go .. when slave active

	DMA2_Stream5->CR|=DMA_SxCR_EN; // Go .. when timer 3 will start.

}


#ifdef MICROKERNEL
// simulates MICRO interface through palette expansion
extern const uint16_t palette_flash[256]; // microX palette in bitbox pixels
static inline void expand_line( void )
{
	uint8_t  * restrict drawbuf8=(uint8_t *) draw_buffer;
	// expand in place buffer from 8bits RRRGGBBL to 15bits RRRrrGGLggBBLbb 
	// XXX unroll loop, read 4 by 4 pixels src, write 2 pixels out by two ... 
	for (int i=VGA_H_PIXELS-1;i>=0;i--)
		draw_buffer[i] = palette_flash[drawbuf8[i]]; 
}
#endif 
 void __attribute__ ((used)) TIM5_IRQHandler() // Hsync Handler
{
	TIM5->SR=0; // clear pending interrupts 

	#ifdef VGA_SKIPLINE
	vga_line+=vga_odd;
	vga_odd=1-vga_odd;
	#else 
	vga_line++;
	#endif 

       // starting from line #1, line #0 already in drawbuffer
	if (vga_line < VGA_V_PIXELS) {

		#ifdef VGA_SKIPLINE
		if (!vga_odd) 
		#endif 
		// swap display & draw buffers, effectively draws line-1
		{
			uint16_t *t;
			t=display_buffer;
			display_buffer = draw_buffer;
			draw_buffer = t;
		}		

		prepare_pixel_DMA(); // will be triggered 

		#ifdef PROFILE
		line_time = DWT->CYCCNT; // reset the perf counter
		#endif

		graph_line(); // Game callback !

        #ifdef PROFILE
        line_time = DWT->CYCCNT - line_time; // read the counter 
        line_time /= VGA_PIXELCLOCK; // scale it to screen width 

        // plot from 0 to VGA_V_PIXELS a green or red element
        uint16_t c=0b1111100000; //green
        if (line_time>VGA_H_PIXELS) // start over but red
        {
            line_time-=VGA_H_PIXELS;
            c=0b111110000000000; // red
        }
        draw_buffer[line_time-1]=0;
        draw_buffer[line_time]=c;
        draw_buffer[line_time+1]=0;
        #endif
		#if MICROKERNEL // Micro interface to bitbox hardware
		if (vga_odd)
			expand_line();
		#endif

	}  else {
		if (vga_line== VGA_V_PIXELS) {
			vga_frame++; // new frame sync now. 
			graph_frame(); 
		}

		if (vga_line==VGA_V_PIXELS+VGA_V_FRONTPORCH+1) {
			vga_lower_vsync();
		} else if(vga_line==VGA_V_PIXELS+1+VGA_V_FRONTPORCH+VGA_V_SYNC)	{
			vga_raise_vsync();
		} else if(vga_line==VGA_V_PIXELS+VGA_V_FRONTPORCH+VGA_V_SYNC+VGA_V_BACKPORCH) {
			vga_line=0;
            graph_line();  // first line next frame!
		}
	}
}

void __attribute__ ((used)) DMA2_Stream5_IRQHandler() // DMA handler
{
	vga_output_black(); // This should not be necessary, as timing is wrong in sw. 

	// Clear Transfer complete interrupt flag of stream 5
	do {
		DMA2->HIFCR|= DMA_HIFCR_CTCIF5;
	} while (DMA2->HISR & DMA_HISR_TCIF5);

	// stop DMA
	TIM1->CR1&=~TIM_CR1_CEN; // Stop pixel clock.
	DMA2_Stream5->CR=0; // Disable pixel DMA.
	// this will trigger a new interrupt. need to get rid of it !
	NVIC_ClearPendingIRQ (DMA2_Stream5_IRQn);    // clear pending DMA IRQ from the NVIC

}

