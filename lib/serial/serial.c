// bitbox implementation of serial protocol
#include "serial.h"


#ifdef EMULATOR
// emulation on stdout

#include <unistd.h>
#include <sys/select.h>
#include <stdio.h>

int kbhit()
{
    struct timeval tv = { 0L, 0L };
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(0, &fds);
    return select(1, &fds, NULL, NULL, &tv);
}
void serial_init() {}
int  serial_tx_ready(void) { return 1; }
int  serial_rx_ready(void) {return kbhit();}
char serial_getchar() {return getchar();}
void serial_putchar(char c) { putchar(c); fflush(stdout);}
int serial_write(const char *buf, int size) {return fwrite(buf,1,size,stdout);}
int serial_read(char *buf, int size) {return fread(buf,1,size,stdin); }

#else 

#include "stm32f4xx.h"

/* ---------------tx buffer----------------
 *             ^              ^
 * sending --> |==============| --> writing   
 *            tail           head 
 */
static char tx_buffer[SERIAL_BUFSIZE];
static char rx_buffer[SERIAL_BUFSIZE];
static int tx_head, tx_tail; 
static int rx_head, rx_tail; 


// Minimal Serial config and run for USART3
void serial_hw_init() {
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN; // enable GPIO D for UART3 TX=PD8, TX=PD9 (with AF7)
    GPIOD->MODER    |= 0b1010<<16; // 10 : ALT
    GPIOD->OSPEEDR  |= 0b1010<<16; // 10 : SPEED=50MHz
    GPIOD->PUPDR |= 01<<(8*2); // Tx on pullup=01 / Rx floating=00
    GPIOD->AFR[1] |= 7 | 7<<4 ; // Tx/Rx = UART3 = AF7 for pins 8 and 9

    RCC->APB1ENR |= RCC_APB1ENR_USART3EN; // clock for USART3 (on APB1)
    USART3->BRR = (SYSCLK/APB1_PRE)/SERIAL_BAUDRATE; // PCLK1=42MHz / (16*115200) = 22.78 16*0.78~13 

    USART3->CR1 |= USART_CR1_RE | USART_CR1_TE; // enable it
    USART3->CR1 |= USART_CR1_UE; // enable it
}

void serial_init() {
    serial_hw_init();

    tx_head = tx_tail = 0;
    rx_head = rx_tail = 0;
}


int serial_tx_ready(void)
{
    return USART3->SR & USART_SR_TXE;
}

int serial_rx_ready(void)
{
    return USART3->SR & USART_SR_RXNE;
}

// Receive single character (blocking)
char serial_getchar(void) 
{
    // wait for RXNE
    while (!serial_rx_ready()) {}
    return USART3->DR;
}

// Sends single character (blokcing)
void serial_putchar(const char c) 
{
    while (!serial_tx_ready()) {}
    USART3->DR = c;
}

int serial_write(const char *buf, int size)
{
    int n;
    for (n=0;n<size && (tx_head-tx_tail+1)%SERIAL_BUFSIZE; n++) {
        tx_buffer[tx_head]=*buf++;
        tx_head = (tx_head+1)%SERIAL_BUFSIZE;
    }

    return n;
}

int serial_read(char *buf, int size)
{
    int n;
    for (n=0;n<size && rx_tail != rx_tail; n++) { 
        *buf++=rx_buffer[rx_tail];
        rx_tail = (rx_tail+1)%SERIAL_BUFSIZE;
    }

    return n;
}

void serial_line_cb()
{
    // if not empty && can send, send one
    if (tx_head != tx_tail && serial_tx_ready())
        serial_putchar(tx_buffer[tx_head++]); 

    // if not full & can receive, receive one
    if ((rx_head-rx_tail+1)%SERIAL_BUFSIZE && serial_rx_ready())
        rx_buffer[rx_head++]=serial_getchar(); 
}

#ifndef SERIAL_NO_IMPLEMENT_MESSAGE
#include <stdarg.h>

int mini_vsnprintf(char* buffer, unsigned int buffer_len, char *fmt, va_list va);
static char msgbuffer[SERIAL_BUFSIZE];
void message (char *fmt, ...)
{
    va_list argptr;
    va_start(argptr, fmt);
    mini_vsnprintf(msgbuffer,SERIAL_BUFSIZE, fmt, argptr);
    va_end(argptr);
    serial_write(msgbuffer,SERIAL_BUFSIZE);  
}
#endif

#endif 
