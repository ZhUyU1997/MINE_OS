#ifndef __INTERRUPT_H__
#define __INTERRUPT_H__
enum INT_NUM{
	EINT0=0,
	EINT1,
	EINT2,
	EINT3,
	EINT4_7,
	EINT8_23,
	INT_CAM,
	nBATT_FLT,
	INT_TICK,
	INT_WDT_AC97,
	INT_TIMER0,
	INT_TIMER1,
	INT_TIMER2,
	INT_TIMER3,
	INT_TIMER4,
	INT_UART2,
	INT_LCD,
	INT_DMA0,
	INT_DMA1,
	INT_DMA2,
	INT_DMA3,
	INT_SDI,
	INT_SPI0,
	INT_UART1,
	INT_NFCON,
	INT_USBD,
	INT_USBH,
	INT_IIC,
	INT_UART0,
	INT_SPI1,
	INT_RTC,
	INT_ADC
};
void EINT_Handle();
void enable_irq(void);
void disable_irq(void);
void INTMS_set(unsigned int offset);
void INTMS_clr(unsigned int offset);
void set_irq_handler(int offset,int (*hander)(void));
#endif