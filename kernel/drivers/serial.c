#include <interrupt.h>
#include "s3c24xx.h"
#include "serial.h"
#include "framebuffer.h"
#define TXD0READY   (1<<2)
#define RXD0READY   (1)

#define PCLK            50000000    // init.c中的clock_init函数设置PCLK为50MHz
#define UART_CLK        PCLK        //  UART0的时钟源设为PCLK
#define UART_BAUD_RATE  115200      // 波特率
#define UART_BRD        ((UART_CLK  / (UART_BAUD_RATE * 16)) - 1)

/*
 * 初始化UART0
 * 115200,8N1,无流控
 */
void uart0_init(void) {
	GPHCON  |= 0xa0;    // GPH2,GPH3用作TXD0,RXD0
	GPHUP   = 0x0c;     // GPH2,GPH3内部上拉

	ULCON0  = 0x03;     // 8N1(8个数据位，无较验，1个停止位)
	UCON0   = 0x05;     // 查询方式，UART时钟源为PCLK
	UFCON0  = 0x00;     // 不使用FIFO
	UMCON0  = 0x00;     // 不使用流控
	UBRDIV0 = UART_BRD; // 波特率为115200
}
static void UART0_RX(unsigned long nr, unsigned long parameter){//接收中断
	unsigned char buf;
	buf = URXH0;  //将接收到的字符存放在buf中
	printf("%02X ", buf&0xff);
}
static void UART0_TX(unsigned long nr, unsigned long parameter){//清除发送中断
}
static void UART0_ERR(unsigned long nr, unsigned long parameter){//清除发送中断
}
void uart0_interrupt_init(void)
{ 
	//request_irq(INT_UART0, UART0_UART0_ISR);
	request_irq(IRQ_S3CUART_RX0, UART0_RX);
	request_irq(IRQ_S3CUART_TX0, UART0_TX);
	request_irq(IRQ_S3CUART_ERR0, UART0_ERR);
}

/*
 * 发送一个字符
 */
static void __serial_putc(unsigned char c) {
	while (!(UTRSTAT0 & TXD0READY));/* 等待，直到发送缓冲区中的数据已经全部发送出去 */
	UTXH0 = c;/* 向UTXH0寄存器中写入数据，UART即自动将它发送出去 */
}

/*
 * 接收字符
 */
static unsigned char __serial_getc(void) {
	while (!(UTRSTAT0 & RXD0READY));/* 等待，直到接收缓冲区中的有数据 */
	return URXH0;/* 直接读取URXH0寄存器，即可获得接收到的数据 */
}

unsigned char serial_getc_async(void) {
	if (!(UTRSTAT0 & RXD0READY))
		return 0;
	return URXH0;/* 直接读取URXH0寄存器，即可获得接收到的数据 */
}

void serial_putc(unsigned char c) {
	__serial_putc(c);
	if (c == '\b') {
		__serial_putc(' ');
		__serial_putc('\b');
	}else if (c == '\n') {
		__serial_putc('\r');
	}
}

unsigned char serial_getc(void) {
	return __serial_getc();
}
