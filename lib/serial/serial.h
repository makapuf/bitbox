// bitbox implementation of serial protocol
#pragma once

#ifndef SERIAL_SPEED 
#define SERIAL_SPEED 9600
#define SERIAL_BUFFER 64
#endif 

void serial_init();

int serial_tx_ready(void);
int serial_rx_ready(void);

// Send / Receive single character (blocking)
char serial_getchar(void);
void serial_putchar(const char c);

