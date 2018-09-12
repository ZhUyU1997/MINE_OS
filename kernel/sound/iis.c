#include "s3c24xx.h"
#include "common.h"

#define PCLK 50000000
#define ABS(a, b) ((a>b)?(a-b):(b-a))

void init_IIS_bus(int fs) {
	IISCON = 0;
	IISMOD = 0;
	IISFCON = 0;

	IISCON |= IISCON_PRESCALER_ENABLE//IIS 预分频器使能
			  | IISCON_TX_DMA
			  | IISCON_RX_IDLE; //接收通道空闲命令:初始化为空闲

	//fs:采样频率
	/* bit[9] : Master clock select, 0-PCLK
	 * bit[8] : 0 = Master mode
	 * bit[7:6] : 10 = Transmit mode
	 * bit[4] : 1 = MSB (Left)-justified format
	 * bit[2] : 384fs, 确定了MASTER CLOCK之后, fs = MASTER CLOCK/384
	 * bit[1:0] : Serial bit clock frequency select, 32fs
	 */

	IISMOD |= IISMOD_SCLK_32FS | IISMOD_MCLK_384FS | IISMOD_SERIAL_BIT_PER_CH_16 | IISMOD_TXMOD | IISMOD_MSB_FORMAT;


	/*
	 * bit15 : Transmit FIFO access mode select, 1-DMA
	 * bit13 : Transmit FIFO, 1-enable
	 */
	IISFCON =  IISFCON_TX_ENABLE | IISFCON_TX_FIFO_DMA;
	int min = 0xffff;
	int pre	= 0;
	for (int i = 0; i < 32; i++) {
		int tmp_fs = PCLK / 384 / (i + 1);
		if (ABS(tmp_fs, fs) < min) {
			min = ABS(tmp_fs, fs);
			pre = i;
		}
	}
	IISPSR = (pre << 5) | pre;
}

void init_gpio_L3_port() {
	GPBCON |= GPB2_OUT | GPB3_OUT | GPB4_OUT;
	//GPBUP != (GPB2_PULL_UP&GPB4_PULL_UP);
}

void init_IIS_port() {
	/*[0-1] [2-3] [4-5] [6-7] [8-9]*/
	//*** PORT E GROUP
	//Ports  : GPE15  GPE14 GPE13   GPE12   GPE11   GPE10   GPE9    GPE8     GPE7  GPE6  GPE5   GPE4
	//Signal : IICSDA IICSCL SPICLK SPIMOSI SPIMISO SDDATA3 SDDATA2 SDDATA1 SDDATA0 SDCMD SDCLK I2SDO
	//Binary :  10     10  ,  10      10  ,  10      10   ,  10      10   ,   10    10  , 10     10  ,
	//-------------------------------------------------------------------------------------------------------
	//Ports  :  GPE3   GPE2  GPE1    GPE0
	//Signal :  I2SDI  CDCLK I2SSCKL I2SLRCK
	//Binary :  10     10  , 10      10
	//rGPECON |= 0x2aa;
	//rGPEUP  |= 0x1f;     // The pull up function is disabled GPE[15:0]
	GPECON |= GPE0_I2SLRCK | GPE1_I2SSCKL | GPE2_CDCLK | GPE3_I2SDI | GPE4_I2SDO;
	GPEUP |= GPE0_NOT_PULL_UP | GPE1_NOT_PULL_UP | GPE2_NOT_PULL_UP | GPE3_NOT_PULL_UP | GPE4_NOT_PULL_UP;
}
