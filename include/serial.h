#ifndef __SERIAL_H__
#define __SERIAL_H__
void uart0_init(void);
void serial_putc(unsigned char c);
unsigned char serial_getc(void);
#endif
