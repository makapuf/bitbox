// kernel functions for Bitbox micro
// inspired, and simplified, from bitbox new vga.
// This code is under GPL V3, makapuf2@gmail.com

// profiler ?

#include "cmsis/stm32f4xx.h"
#include "kconf_micro.h"

#if KERNEL!=MICRO // Micro interface is the only supported on micro hardware
    #error only MICRO interface is suported on MICRO hardware (set KERNEL=MICRO on makefile. See lib/bitbox.mk)
#endif 


// does not include microx.h since all definitions change (const access, volatile ...)

/* 
 *  Hardware setup : 

      - Video :  RRRGGBBL DAC, on PA0-7
            - targeted by DMA1 Stream6 CH2 
            - triggered by Tim4 UP
            - TIM4 gated by TIM3 on CH2 (p352 refman)
      - HSYNC on PB1 ( TIM3 CH4 )
      - VSYNC on PB15 ( GPIO OUT )
      - 

 */


 /* DEBUG 

    TIM1 (pixel clk ) : p *((TIM_TypeDef *) ((0x40000000UL) + 0x00010000 + 0x00000000)) 
    TIM3 (line sync ) : p *((TIM_TypeDef *) ((0x40000000UL) + 0x00000400)) 

    RCC :  p /x *(RCC_TypeDef*) 0x40021000UL
    CYCCNT : disp *(uint32_t *)0xE0001004
    */

uint32_t line_time,max_line_time, max_line; // maximum time of line 

int vga_line;
volatile int vga_frame; 

#ifdef VGA_SKIPLINE
volatile int vga_odd; // only defined in "skipline" modes
#endif 

// align to 1kb since we're using DMA bursts
static uint8_t LineBuffer1[VGA_H_PIXELS+16] __attribute__((aligned (1024))); 
static uint8_t LineBuffer2[VGA_H_PIXELS+16] __attribute__((aligned (1024))); 

static uint8_t *display_buffer; // will be sent to display 
uint8_t *draw_buffer; // will be drawn (bg already drawn)

#define TIMER_CYCL (SYSCLK/VGA_VFREQ)
#define SYNC_END (VGA_H_SYNC*TIMER_CYCL/(VGA_H_PIXELS+VGA_H_SYNC+VGA_H_FRONTPORCH+VGA_H_BACKPORCH))
#define BACKPORCH_END ((VGA_H_SYNC+VGA_H_BACKPORCH)*TIMER_CYCL/(VGA_H_PIXELS+VGA_H_SYNC+VGA_H_FRONTPORCH+VGA_H_BACKPORCH))

// default implmentations of graph_line and graph_frame
void __attribute__((weak)) graph_frame(void) {}
void __attribute__((weak)) graph_line(void)  {}

static inline void output_black(void)
{
    GPIOA->BSRR |= GPIO_BSRR_BR_0*0xff; // output black pins 0-7
}

void vga_setup() 
{
    // software state
    vga_line=0; vga_frame=0;
    display_buffer=LineBuffer1;
    draw_buffer=LineBuffer2;

    memset(LineBuffer1,0,sizeof(LineBuffer1));
    memset(LineBuffer1,0,sizeof(LineBuffer2));

    // --- GPIO ---------------------------------------------------------------------------------------
    // GPIOB->BSRR |= (1<<1) | (1<<0);     // drive sync pins high
    /*
    GPIO Setup : 
      - PA0-7 : VGA OUT, RRRGGBBL 
      - PB1 : HSync (TIM2 CH3)
      - PB15 : VSync

    */

    // Enable clocks for peripherals
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOBEN;

    // PA0-7 out, pushpull(default), 50MHz, pulldown
    GPIOA->MODER   |= 0b0101010101010101; // 00:IN 01:OUT 10:ALT 11:ANALOG x16 bits
    GPIOA->OSPEEDR |= 0b1111111111111111; // SPEED=50MHz 
    GPIOA->PUPDR   |= 0b1010101010101010; // 10 = Pull Down

    // PB1 : HSync (TIM2 CH3) - 31kHz
    // PB15 : VSync - 60Hz
    GPIOB->MODER  |= GPIO_MODER_MODER15_0 * 0b01 | GPIO_MODER_MODER1_0 * 0b10 ; // PB15: OUT PB1:ALT 
    GPIOB->AFR[0] |= 2<<4; // PB1 alt func nb 2 (datasheet p45)
    GPIOB->PUPDR  |=  GPIO_PUPDR_PUPDR15_0 * 0b01 | GPIO_PUPDR_PUPDR1_0 * 0b01 ; // PB1, PB15 pullup 01

    output_black();

    // --- TIMERS ---------------------------------------------------------------------------------------
    
    // TIMER 3

    // -- Configure Timer 3 as the HSync timer. Timer 3 runs at full frequency (div=0)
    // CC4 is used to generate the HSync pulse, using PWM mode and driving the pin directly (channel 4 timer 3 drives gpio pin)
    // CC3 is used to generate a trigger signal for TIM1, which drives the pixel DMA, though ITR1
    
    // -- Global setup

    RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;    // enable timer 3
    
    TIM3->CR1=TIM_CR1_ARPE; // autoreload preload enable, no other function on
    TIM3->DIER=TIM_DIER_UIE; // Enable update interrupt.
    TIM3->CCER=0; // Disable CC, so we can program it.
    TIM3->ARR=TIMER_CYCL-1; 

    // -- Channel 4  : Hsync pulse PWM

    // On CNT==0: sync pulse start
    // set to PWM mode active level on reload, passive level after CC4-match + enable preload
    TIM3->CCMR2|=6*TIM_CCMR2_OC4M_0 | TIM_CCMR2_OC4PE;
    // output enabled, reversed polarity (active low).
    TIM3->CCER|=TIM_CCER_CC4E|TIM_CCER_CC4P; 
    TIM3->CCR4|=SYNC_END; 

    // -- Channel 3 : Trigger signal for TIM1 - will start on ITR0 (refman table p )

    // Master mode selection OC3REF signal is used as trigger output (TRGO)
    TIM3->CR2 |= 0b110*TIM_CR2_MMS_0; 

    // Channel 3 set to passive level on reload, active level after CC3-match.
    TIM3->CCMR2 |= 7*TIM_CCMR2_OC3M_0;

    TIM3->CCR3=BACKPORCH_END-10;

    
    // Enable HSync timer interrupt and set highest priority. TIM3_IRQhandler is defined below
    NVIC_EnableIRQ(TIM3_IRQn);
    NVIC_SetPriority(TIM3_IRQn,0);

    // Enable HSync timer.
    TIM3->CNT=-11; // Make sure it hits ARR. 
    TIM3->BDTR |= TIM_BDTR_MOE; // enable main output 
    TIM3->CR1 |= TIM_CR1_CEN; // Go.
    TIM3->EGR |= TIM_EGR_UG;  // Transfer modes by setting the UG bit (refman 396)

    // --- TIMER 1 : DMA2 pixel clock, gated mode from timer3 (see time sinc ref352) 
    
    RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;    // enable it (default no clock div)

    TIM1->ARR=VGA_PIXELCLOCK-1;  // loop each "pixelclock"
    TIM1->CR1=TIM_CR1_ARPE;  // autoreload preload enable, no other function on
    TIM1->DIER=TIM_DIER_UDE;  // Enable update DMA request interrupt
    
    // Only run when TIM3 trigger-out is high
    // SMS=5 : set Gated mode (clk enabled on input),
    // TS: selection ITR2 which for TIM1 selects TIM3 as master (ref289)
    TIM1->SMCR=(0b101 * TIM_SMCR_SMS_0) | (0b010 * TIM_SMCR_TS_0); 

    // --- DMA -----------------------------------------------------------------------------------------

    // TIM1_UP drives the DMA2 on stream 5 channel 6 (ref. manual p170)
    
    RCC->AHB1ENR |= RCC_AHB1ENR_DMA2EN; // DMA2 enable

    // Stop it and configure interrupts.
    DMA2_Stream5->CR&=~DMA_SxCR_EN;
    
    NVIC_DisableIRQ(DMA2_Stream5_IRQn);
    NVIC_EnableIRQ(DMA2_Stream5_IRQn);
    NVIC_SetPriority(DMA2_Stream5_IRQn,0);


}

static void prepare_pixel_DMA(void)
{
    // Visible line. Configure and enable pixel DMA.
    // defaults : PSIZE = MSIZE = 8 bit, dont increase periph addr, 


    DMA2_Stream5->CR=(6*DMA_SxCR_CHSEL_0)| // Channel 6
        (3*DMA_SxCR_PL_0)| // Priority 3
        (0*DMA_SxCR_PSIZE_0)| // PSIZE = 8 bit
        (0*DMA_SxCR_MSIZE_0)| // MSIZE = 16 bit
        DMA_SxCR_MINC| // Increase memory address
        (1*DMA_SxCR_DIR_0)| // Memory to peripheral
        DMA_SxCR_TCIE |  // Transfer complete interrupt 
        (2*DMA_SxCR_MBURST_0); // burst on the memory-side - 4beats
        
    // Enable FIFO (see p190 of ref manual)
    // XXX DMA FIFO (as MBURST/ref p236) ?
    DMA2_Stream5->FCR |= DMA_SxFCR_DMDIS; 

    DMA2_Stream5->NDTR=VGA_H_PIXELS+1; // transfer N pixels + one last _black_ pixel
    DMA2_Stream5->PAR=((uint32_t)&(GPIOA->ODR));
    DMA2_Stream5->M0AR=(uint32_t)display_buffer; 


    // Enable pixel clock. Clock will only start once TIM3 allows it.

    TIM1->DIER = 0; // Update DMA request has to be disabled while zeroing the counter.
    TIM1->EGR = TIM_EGR_UG; // Force an update event to reset counter. Setting CNT is not reliable.
    TIM1->DIER = TIM_DIER_UDE; // Re-enable update DMA request.
    TIM1->CR1 |= TIM_CR1_CEN; // go .. when activated by TIM3

    DMA2_Stream5->CR |= DMA_SxCR_EN; // Go .. when timer 1 will clock it.
}

// --- Interrupt Handlers ------------------------------------------------------------------------------


// HSYNC interrupt handler
void  __attribute__ ((used)) TIM3_IRQHandler(void) // attribute used neded if called from ASM
{

    TIM3->SR=0; // clear status 

    // double-line modes
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
        // swap display & draw buffers, effectively draws line-1. does not swap on odd lines if not needed
        {
            uint8_t *t;
            t=display_buffer;
            display_buffer = draw_buffer;
            draw_buffer = t;
        }        

        prepare_pixel_DMA(); // will be triggered after

        #ifdef PROFILE
        line_time = DWT->CYCCNT; // reset the perf counter
        #endif
        
        graph_line(); // Game graph callback !   
        draw_buffer[0]=0; 
           
        #ifdef PROFILE
        line_time = DWT->CYCCNT - line_time; // read the counter 
        line_time /= VGA_PIXELCLOCK; // scale it to screen width 

        // plot from 0 to VGA_V_PIXELS a green or red element
        uint8_t c=0b11001; //green
        if (line_time>VGA_H_PIXELS) // start over but red
        {
            line_time-=VGA_H_PIXELS;
            c=0b11100000; // red
        }
        draw_buffer[line_time-1]=0;
        draw_buffer[line_time]=c;
        draw_buffer[line_time+1]=0;
        #endif

    }  else {
        #ifdef VGA_SKIPLINE
        if (!vga_odd)  // only once
        #endif
        {
            if (vga_line== VGA_V_PIXELS) {
                vga_frame++; // new frame sync now. 
                graph_frame(); 
            } else if (vga_line==VGA_V_PIXELS+VGA_V_FRONTPORCH+1) {
                GPIOB->BSRR |= GPIO_BSRR_BR_15; // lower VSync line 
            } else if(vga_line==VGA_V_PIXELS+1+VGA_V_FRONTPORCH+VGA_V_SYNC) {
                GPIOB->BSRR |= GPIO_BSRR_BS_15; // raise VSync line
            } 
        }

        if(vga_line==VGA_V_PIXELS+VGA_V_FRONTPORCH+VGA_V_SYNC+VGA_V_BACKPORCH+1) {
            vga_line=0;
            graph_line();  // first line next frame!
        }

    }
}

// DMA Stream 5 : pixel DMA interrupt, DMA complete
void __attribute__ ((used)) DMA2_Stream5_IRQHandler(void)
{
    output_black();

    // Clear Transfer complete interrupt flag of stream 5
    do {
        DMA2->HIFCR|= DMA_HIFCR_CTCIF5; 
    } while (DMA2->HISR & DMA_HISR_TCIF5);

    // stop DMA
    TIM1->CR1 &= ~TIM_CR1_CEN; // Stop pixel clock.
    DMA2_Stream5->CR = 0; // Disable pixel DMA.

    // this will trigger a new interrupt. need to get rid of it !
    NVIC_ClearPendingIRQ (DMA2_Stream5_IRQn);    // clear pending DMA IRQ from the NVIC
}
