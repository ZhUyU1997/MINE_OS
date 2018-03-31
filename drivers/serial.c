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
void uart0_init(void)
{
    GPHCON  |= 0xa0;    // GPH2,GPH3用作TXD0,RXD0
    GPHUP   = 0x0c;     // GPH2,GPH3内部上拉

    ULCON0  = 0x03;     // 8N1(8个数据位，无较验，1个停止位)
    UCON0   = 0x05;     // 查询方式，UART时钟源为PCLK
    UFCON0  = 0x00;     // 不使用FIFO
    UMCON0  = 0x00;     // 不使用流控
    UBRDIV0 = UART_BRD; // 波特率为115200
}



/*
 * 发送一个字符
 */
static void __serial_putc(unsigned char c)
{
    while (!(UTRSTAT0 & TXD0READY));/* 等待，直到发送缓冲区中的数据已经全部发送出去 */
    UTXH0 = c;/* 向UTXH0寄存器中写入数据，UART即自动将它发送出去 */
}

/*
 * 接收字符
 */
static unsigned char __serial_getc(void)
{
    while (!(UTRSTAT0 & RXD0READY));/* 等待，直到接收缓冲区中的有数据 */
    return URXH0;/* 直接读取URXH0寄存器，即可获得接收到的数据 */
}

void serial_putc(unsigned char c)
{
    __serial_putc(c);
	if(c=='\b'){
		__serial_putc(' ');
		__serial_putc('\b');
	}
		
}

unsigned char serial_getc(void)
{
    return __serial_getc();
}