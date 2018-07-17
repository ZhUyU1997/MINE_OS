#include <stdio.h>
#include <sys/types.h>
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
static volatile enum DMA_STATUS {
	DMA_STOP,
	DMA_READY,
	DMA_INIT,
	DMA_RUN,
	DMA_FINISH
}
dma_status = DMA_INIT;

#define dma_regs (*(volatile struct s3c_dma_regs *)DMA2_BASE_ADDR)

static int (*dma_end_func)() = NULL;

void set_dma_end_func(int (*f)()) {
	assert(f && ((dma_status == DMA_INIT) || dma_status == DMA_READY));
	if (f && ((dma_status == DMA_INIT) || dma_status == DMA_READY))
		dma_end_func = f;
	else
		dma_end_func = 0;
}

void dma_init() {
	//TODO:等待DMA传输完毕，
	while (!dma_can_run());
	if (dma_status == DMA_INIT)
		return;
	struct s3c_dma_regs temp = {0};
	dma_regs = temp;
	dma_end_func = NULL;
	dma_status = DMA_INIT;
	//printf("temp.dstat = %X, dma_regs.dstat = %X\n", temp.dstat, dma_regs.dstat);
}
void dma_exit() {
	if (dma_status == DMA_FINISH) {
		dma_end_func = NULL;
		dma_regs.dmasktrig  &= ~(1 << 1);
		free_irq(INT_DMA2);
		dma_init();
	}
}

void dma_set(unsigned int src, unsigned int len) {
	assert(src && (len < 0x100000) && (dma_status != DMA_RUN));
	if (src && (len < 0x100000) && (dma_status != DMA_RUN)) {
		dma_regs.dstat		= 0;
		dma_regs.disrc      = src;				/* 源的物理地址 */
		dma_regs.disrcc     = (0 << 1) | (0 << 0);	/* 源位于AHB总线, 源地址递增 */
		dma_regs.didst      = 0x55000010;		/* 目的的物理地址 */
		dma_regs.didstc     = (0 << 2) | (1 << 1) | (1 << 0);	/* 目的位于APB总线, 目的地址不变 */
		dma_regs.dcon       = (1 << 31) | (0 << 30) | (1 << 29) | (0 << 28) | (0 << 27) | (0 << 24) | (1 << 23) | (1 << 23) | (1 << 22) | (1 << 20) | (len / 2);	/* 使能中断,单个传输,硬件触发 */
		dma_status = DMA_READY;
	}
}
//暂时不实现
//void dma_stop(void)
//{
//}

void DMA2IntHandle(void) {
	dma_status = DMA_FINISH;
	if (dma_end_func) {
		int ret = dma_end_func();
		//如果继续DMA那么不执行dma_exit
		if (ret == 1)
			return;
	}
	dma_exit();
	//putc('#');
}

void dma_start(void) {
	assert(dma_status == DMA_READY);
	if (dma_status == DMA_READY) {
		request_irq(INT_DMA2, DMA2IntHandle);
		dma_regs.dmasktrig  = (1 << 1);
		dma_status = DMA_RUN;
	}
}

int dma_can_run() {
	return dma_status != DMA_RUN;
}

