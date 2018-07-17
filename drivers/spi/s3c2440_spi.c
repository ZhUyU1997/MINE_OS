#include "s3c24xx.h"
#include "timer.h"
#if 0
/* 用GPIO模拟SPI */

static void SPI_GPIO_Init(void) {
	/* GPF1 OLED_CSn output */
	GPFCON &= ~(3 << (1 * 2));
	GPFCON |= (1 << (1 * 2));
	GPFDAT |= (1 << 1);

	/* GPG2 FLASH_CSn output
	 * GPG4 OLED_DC   output
	 * GPG5 SPIMISO   input
	 * GPG6 SPIMOSI   output
	 * GPG7 SPICLK    output
	 */
	GPGCON &= ~((3 << (2 * 2)) | (3 << (4 * 2)) | (3 << (5 * 2)) | (3 << (6 * 2)) | (3 << (7 * 2)));
	GPGCON |= ((1 << (2 * 2)) | (1 << (4 * 2)) | (1 << (6 * 2)) | (1 << (7 * 2)));
	GPGDAT |= (1 << 2);
}

static void SPI_Set_CLK(char val) {
	if (val)
		GPGDAT |= (1 << 7);
	else
		GPGDAT &= ~(1 << 7);
}

static void SPI_Set_DO(char val) {
	if (val)
		GPGDAT |= (1 << 6);
	else
		GPGDAT &= ~(1 << 6);
}

static char SPI_Get_DI(void) {
	if (GPGDAT & (1 << 5))
		return 1;
	else
		return 0;
}

void SPISendByte(unsigned char val) {
	int i;
	for (i = 0; i < 8; i++) {
		SPI_Set_CLK(0);
		SPI_Set_DO(val & 0x80);
		SPI_Set_CLK(1);
		val <<= 1;
	}

}

unsigned char SPIRecvByte(void) {
	int i;
	unsigned char val = 0;
	for (i = 0; i < 8; i++) {
		val <<= 1;
		SPI_Set_CLK(0);
		if (SPI_Get_DI())
			val |= 1;
		SPI_Set_CLK(1);
	}
	return val;
}


void SPIInit(void) {
	/* 初始化引脚 */
	SPI_GPIO_Init();
}
#else

/* SPI controller */

static void SPI_GPIO_Init(void) {
	/* GPF1 OLED_CSn output */
	GPFCON &= ~(3 << (1 * 2));
	GPFCON |= (1 << (1 * 2));
	GPFDAT |= (1 << 1);

	/* GPG2 FLASH_CSn output
	* GPG4 OLED_DC   output
	* GPG5 SPIMISO
	* GPG6 SPIMOSI
	* GPG7 SPICLK
	*/
	GPGCON &= ~((3 << (2 * 2)) | (3 << (4 * 2)) | (3 << (5 * 2)) | (3 << (6 * 2)) | (3 << (7 * 2)));
	GPGCON |= ((1 << (2 * 2)) | (1 << (4 * 2)) | (3 << (5 * 2)) | (3 << (6 * 2)) | (3 << (7 * 2)));
	GPGDAT |= (1 << 2);
}


void SPISendByte(unsigned char val) {
	//while (!(SPSTA1 & 1));
	SPTDAT1 = val;
	while (!(SPSTA1 & 1));
}

unsigned char SPIRecvByte(void) {
	SPTDAT1 = 0xff;
	while (!(SPSTA1 & 1));
	return SPRDAT1;
}


static void SPIControllerInit(void) {
	/* OLED  : 100ns, 10MHz
	* FLASH : 104MHz
	* 取10MHz
	* 10 = 50 / 2 / (Prescaler value + 1)
	* Prescaler value = 1.5 = 2
	* Baud rate = 50/2/3=8.3MHz
	*/
	SPPRE0 = 2;
	SPPRE1 = 2;

	/* [6:5] : 00, polling mode
	* [4]   : 1 = enable
	* [3]   : 1 = master
	* [2]   : 0
	* [1]   : 0 = format A
	* [0]   : 0 = normal mode
	*/
	//SPCON0 = (1<<4) | (1<<3);
	SPCON1 = (1 << 4) | (1 << 3);

}

void SPIInit(void) {
	/* 初始化引脚 */
	SPI_GPIO_Init();

	SPIControllerInit();
}

#endif
