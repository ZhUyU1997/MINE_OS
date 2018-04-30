#include <stdio.h>
#include <assert.h>
#include <interrupt.h>
#include <s3c24xx.h>
#define DMA0_BASE_ADDR  0x4B000000
#define DMA1_BASE_ADDR  0x4B000040
#define DMA2_BASE_ADDR  0x4B000080
#define DMA3_BASE_ADDR  0x4B0000C0

struct s3c_dma_regs {
	unsigned long disrc;
	unsigned long disrcc;
	unsigned long didst;
	unsigned long didstc;
	unsigned long dcon;
	unsigned long dstat;
	unsigned long dcsrc;
	unsigned long dcdst;
	unsigned long dmasktrig;
};
volatile enum DMA_STATUS{
	DMA_STOP,
	DMA_READY,
	DMA_INIT,
	DMA_RUN,
	DMA_FINISH
} dma_status = DMA_INIT;

#define dma_regs (*(volatile struct s3c_dma_regs *)DMA2_BASE_ADDR)

void dma_init()
{
	//TODO:等待DMA传输完毕，
	while(!dma_can_run());
	struct s3c_dma_regs temp = {0};
	dma_regs = temp;
	dma_status = DMA_INIT;
	//printf("temp.dstat = %X, dma_regs.dstat = %X\n", temp.dstat, dma_regs.dstat);
}
void dma_reset()
{
	struct s3c_dma_regs temp = {0};
	dma_regs = temp;
	dma_status = DMA_INIT;
}

void dma_set(unsigned int src, unsigned int len)
{
	assert(len < 0x100000);
	dma_regs.dstat		= 0;
	dma_regs.disrc      = src;				/* 源的物理地址 */
	dma_regs.disrcc     = (0<<1) | (0<<0);	/* 源位于AHB总线, 源地址递增 */
	dma_regs.didst      = 0x55000010;		/* 目的的物理地址 */
	dma_regs.didstc     = (0<<2) | (1<<1) | (1<<0);	/* 目的位于APB总线, 目的地址不变 */
	dma_regs.dcon       = (1<<31)|(0<<30)|(1<<29)|(0<<28)|(0<<27)|(0<<24)|(1<<23)|(1<<20)|(len/2);	/* 使能中断,单个传输,硬件触发 */
	dma_status = DMA_READY;
}

void dma_stop(void)
{
	dma_regs.dmasktrig  &= ~(1<<1);
	INTMSK_clr(INT_DMA2);
	dma_status = DMA_STOP;
}

void DMA2IntHandle(void)
{
	dma_stop();
	dma_status = DMA_FINISH;
	//putc('#');
}

void dma_start(void)
{
	set_irq_handler(INT_DMA2, DMA2IntHandle);
	INTMSK_set(INT_DMA2);
	dma_regs.dmasktrig  = (1<<1);
	dma_status = DMA_RUN;
}

int dma_can_run(){
	return dma_status != DMA_RUN;
}

