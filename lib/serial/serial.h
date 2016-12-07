// bitbox implementation of serial protocol
// redefines message(...) to output to serial. Needs to compile mini-printf.c
#pragma once

// defaults, redefine all or none
#ifndef SERIAL_BAUDRATE
  #define SERIAL_BAUDRATE 9600
  #define SERIAL_BUFSIZE 64 // most preferably a power of two. Can send BAUDRATE/60/8 bytes in a frame.
#endif 
// #define SERIAL_NO_REDEFINE_MESSAGE // avoid redefining message()

void serial_init();

// -- high level interface
int serial_write(const char *buf, int bufsize); // returns number of written chars
int serial_read(char *buf, int bufsize); // returns number of received chars

/* Call this in you line_cb and/or your vsync callbacks if you're using 
  high level interface. We're using a poll interface to avoid interrupts.
  */
void serial_line_cb(); 

// -- low-level interface 
int serial_tx_ready(void);
int serial_rx_ready(void);
// send/receives single character (blocking)
char serial_getchar(void);
void serial_putchar(const char c);

