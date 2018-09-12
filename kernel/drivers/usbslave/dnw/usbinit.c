/****************************************************************
 NAME: u2440mon.c
 DESC: u2440mon entry point,menu,download
 HISTORY:
 Mar.25.2002:purnnamu: S3C2400X profile.c is ported for S3C2410X.
 Mar.27.2002:purnnamu: DMA is enabled.
 Apr.01.2002:purnnamu: isDownloadReady flag is added.
 Apr.10.2002:purnnamu: - Selecting menu is available in the waiting loop.
                         So, isDownloadReady flag gets not needed
                       - UART ch.1 can be selected for the console.
 Aug.20.2002:purnnamu: revision number change 0.2 -> R1.1
 Sep.03.2002:purnnamu: To remove the power noise in the USB signal, the unused CLKOUT0,1 is disabled.
 ****************************************************************/
#include <s3c24x0.h>
#include <interrupt.h>
#include <timer.h>
#include "usbmain.h"
#include "usbout.h"
#include "usblib.h"
#include "2440usb.h"

extern S3C24X0_INTERRUPT * intregs;
S3C24X0_USB_DEVICE * usbdevregs;
S3C24X0_DMAS * dmaregs;
S3C24X0_CLOCK_POWER * clk_powerregs;
S3C24X0_GPIO * gpioregs;

void USB_ISR_Init(void);
U32 usb_receive(char *buf, size_t len, U32 wait);
/*
extern void Timer_InitEx(void);
extern void Timer_StartEx(void);
extern unsigned int Timer_StopEx(void);
*/
//TODO:计时功能待实现
#define Timer_InitEx()
#define Timer_StartEx()
#define Timer_StopEx() (60)

/*
 * Reads and returns a character from the serial port
 *   - Times out after delay iterations checking for presence of character
 *   - Sets *error_p to UART error bits or - on timeout
 *   - On timeout, sets *error_p to -1 and returns 0
 */

// char awaitkey(unsigned long delay, int* error_p) {
// int i;
// char c;

// if (delay == -1) {
// while (1) {
// if (tstc()) /* we got a key press	*/
// return getc();
// }
// } else {
// for (i = 0; i < delay; i++) {
// if (tstc()) /* we got a key press	*/
// return getc();
// delay_u(10*1000);
// }
// }

// if (error_p)
// *error_p = -1;
// return 0;
// }

char awaitkey(unsigned long delay, int* error_p) {
	//TODO:可能会造成死循环
	return getc();
}
#define CTRL(x)   ((x) & 0x1f)
#define INTR      CTRL('C')

void Clk0_Enable(int clock_sel);
void Clk1_Enable(int clock_sel);
void Clk0_Disable(void);
void Clk1_Disable(void);

volatile U32 downloadAddress;

void (*restart)(void) = 0;
void (*run)(void);


volatile unsigned char *downPt;
volatile U32 downloadFileSize;
volatile U16 checkSum;
volatile unsigned int err = 0;
volatile U32 totalDmaCount;

volatile int isUsbdSetConfiguration;

int download_run = 0;
volatile U32 tempDownloadAddress;
volatile U32 dwUSBBufReadPtr;
volatile U32 dwUSBBufWritePtr;
volatile U32 dwWillDMACnt;
volatile U32 bDMAPending;
volatile U32 dwUSBBufBase;
volatile U32 dwUSBBufSize;


//***************************[ PORTS ]****************************************************
void Port_Init(void) {
	gpioregs = S3C24X0_GetBase_GPIO();
	//CAUTION:Follow the configuration order for setting the ports.
	// 1) setting value(GPnDAT)
	// 2) setting control register  (GPnCON)
	// 3) configure pull-up resistor(GPnUP)

	//32bit data bus configuration
	//*** PORT A GROUP
	//Ports  :	GPA22	GPA21	GPA20	GPA19	GPA18	GPA17	GPA16	GPA15	GPA14	GPA13	GPA12
	//Signal :	nFCE	nRSTOUT	nFRE	nFWE	ALE		CLE		nGCS5	nGCS4	nGCS3	nGCS2	nGCS1
	//Binary :  1		1		1		1		1		1		1		1		1		1		1
	//Ports  :	GPA11	GPA10	GPA9	GPA8	GPA7	GPA6	GPA5	GPA4	GPA3	GPA2	GPA1	GPA0
	//Signal :	ADDR26	ADDR25	ADDR24	ADDR23	ADDR22	ADDR21	ADDR20	ADDR19	ADDR18	ADDR17	ADDR16	ADDR0
	//Binary :	1		1		1		1		1		1		1		1		1		1		1		1
	gpioregs->GPACON = 0x7fffff;

	//**** PORT B GROUP
	//Ports  :	GPB10	GPB9	GPB8	GPB7	GPB6	GPB5		GPB4	GPB3	GPB2	GPB1		GPB0
	//Signal :	nXDREQ0	nXDACK0	nXDREQ1	nXDACK1	nSS_KBD	nDIS_OFF	L3CLOCK	L3DATA	L3MODE	nIrDATXDEN	Keyboard
	//Setting:	INPUT	Input	INPUT	OUTPUT	INPUT	OUTPUT		OUTPUT	OUTPUT	OUTPUT	OUTPUT		OUTPUT
	//Binary :	00 		00		00		01		00		01			01		01		01		01			01
	gpioregs->GPBCON = 0x004555;
	gpioregs->GPBUP  = 0x7ff;     // The pull up function is disabled GPB[10:0]
	gpioregs->GPBDAT &= ~(1 << 0);	/* 禁止蜂鸣器 */

	//*** PORT C GROUP
	//Ports  :	GPC15	GPC14	GPC13	GPC12	GPC11	GPC10	GPC9	GPC8	GPC7	GPC6	GPC5	GPC4	GPC3	GPC2	GPC1	GPC0
	//Signal :	VD7		VD6		VD5		VD4		VD3		VD2		VD1		VD0		LCDVF2	LCDVF1	OUTPUT	VM		VFRAME	VLINE	VCLK	LEND
	//Binary :	10		10		10		10		10		10		10		10		10		10		01		10		10		10		10		10
	gpioregs->GPCCON = 0xaaaaa6aa;
	gpioregs->GPCUP  = 0xffff;     // The pull up function is disabled GPC[15:0]
	gpioregs->GPCDAT &= ~(1 << 5);	/* disable USB Device, enable later*/

	//*** PORT D GROUP
	//Ports  : GPD15	GPD14	GPD13	GPD12	GPD11	GPD10	GPD9	GPD8	GPD7	GPD6	GPD5	GPD4	GPD3	GPD2	GPD1	GPD0
	//Signal : VD23		VD22	VD21	VD20	VD19	VD18	VD17	VD16	VD15	VD14	VD13	VD12	VD11	VD10	VD9		VD8
	//Binary : 10		10		10		10		10		10		10		10		10		10		10		10		10		10		10		10
	gpioregs->GPDCON = 0xaaaaaaaa;
	gpioregs->GPDUP  = 0xffff;     // The pull up function is disabled GPD[15:0]

	//*** PORT E GROUP
	//Ports  :	GPE15	GPE14	GPE13	GPE12	GPE11	GPE10	GPE9	GPE8	GPE7	GPE6	GPE5	GPE4
	//Signal :	IICSDA	IICSCL	SPICLK	SPIMOSI	SPIMISO	SDDATA3	SDDATA2	SDDATA1	SDDATA0	SDCMD	SDCLK	IN
	//Binary :	10		10		10		10		10		10		10		10		10		10		10		00
	//-------------------------------------------------------------------------------------------------------
	//Ports  :	GPE3	GPE2	GPE1	GPE0
	//Signal :	IN		IN		IN		IN
	//Binary :	00		00		00		00
	//rGPECON = 0xaaaaaaaa;
	//rGPEUP  = 0xffff;     // The pull up function is disabled GPE[15:0]
	gpioregs->GPECON = 0xaaaaa800; // For added AC97 setting
	gpioregs->GPEUP  = 0xffff;

	//*** PORT F GROUP
	//Ports  :	GPF7	GPF6	GPF5	GPF4	GPF3		GPF2	GPF1	GPF0
	//Signal :	nLED_8	nLED_4	nLED_2	nLED_1	nIRQ_PCMCIA	EINT2	KBDINT	EINT0
	//Setting:	Output	Output	Output	Output	EINT3		EINT2	EINT1	EINT0
	//Binary :	01		01		01		01		10			10		10		10
	gpioregs->GPFCON = 0x55aa;
	gpioregs->GPFUP  = 0xff;     // The pull up function is disabled GPF[7:0]

	//*** PORT G GROUP
	//Ports  :	GPG15	GPG14	GPG13	GPG12	GPG11	GPG10		GPG9		GPG8		GPG7		GPG6
	//Signal :	nYPON	YMON	nXPON	XMON	EINT19	DMAMODE1	DMAMODE0	DMASTART	KBDSPICLK	KBDSPIMOSI
	//Setting:	nYPON	YMON	nXPON	Input	EINT19	Output		Input		Output		SPICLK1		SPIMOSI1
	//Binary :	11		11		11		00		10		01			00			01			11			11
	//-----------------------------------------------------------------------------------------
	//Ports  :	GPG5		GPG4		GPG3	GPG2	GPG1	GPG0
	//Signal :	KBDSPIMISO	LCD_PWREN	EINT11	nSS_SPI	IRQ_LAN IRQ_PCMCIA
	//Setting:	SPIMISO1	LCD_PWRDN	EINT11	nSS0	EINT9	EINT8
	//Binary :	11			11			10		11		10		10
	//rGPGCON = 0xff95ffba;
	gpioregs->GPGCON = 0xfc91ffba;
	gpioregs->GPGUP  = 0xffff;    // The pull up function is disabled GPG[15:0]
	//*** PORT H GROUP
	//Ports  :	GPH10	GPH9	GPH8	GPH7	GPH6	GPH5	GPH4	GPH3	GPH2	GPH1	GPH0
	//Signal :	CLKOUT1 CLKOUT0	UCLK	nCTS1	nRTS1	RXD1	TXD1	RXD0	TXD0	nRTS0	nCTS0
	//Binary :	10		10     10		11		11		10		10		10		10		10 	   10
	gpioregs->GPHCON = 0x2afaaa;
	gpioregs->GPHUP  = 0x7ff;    // The pull up function is disabled GPH[10:0]

	// Added for S3C2440X, DonGo
	//*** PORT J GROUP
	//Ports  :	GPJ12		GPJ11		GPJ10	GPJ9		GPJ8		GPJ7		GPJ6		GPJ5		GPJ4		GPJ3		GPJ2		GPJ1		GPJ0
	//Signal :	CAMRESET	CAMPCLKOUT	CAMHREF	CAMVSYNC	CAMPCLKIN	CAMDAT[7]	CAMDAT[6]	CAMDAT[5]	CAMDAT[4]	CAMDAT[3]	CAMDAT[2]	CAMDAT[1]	CAMDAT[0]
	//Binary :	10			10			10		10			10			10			10			10			10			10			10			10			10
	gpioregs->GPJCON = 0x02aaaaaa;
	gpioregs->GPJUP  = 0x1fff;    // The pull up function is disabled GPH[10:0]

	//External interrupt will be falling edge triggered.
	gpioregs->EXTINT0 = 0x22222222;    // EINT[7:0]
	gpioregs->EXTINT1 = 0x22222222;    // EINT[15:8]
	gpioregs->EXTINT2 = 0x22222222;    // EINT[23:16]
}


void usb_init_slave(void) {
	char *mode;
	clk_powerregs = S3C24X0_GetBase_CLOCK_POWER();
	usbdevregs = S3C24X0_GetBase_USB_DEVICE();
	dmaregs = S3C24X0_GetBase_DMAS();
	USB_ISR_Init();
	gpioregs->MISCCR = gpioregs->MISCCR&~(1 << 3); // USBD is selected instead of USBH1
	gpioregs->MISCCR = gpioregs->MISCCR&~(1 << 13); // USB port 1 is enabled.
	//USBD should be initialized first of all.
	isUsbdSetConfiguration = 0;

	UsbdMain();
	udelay(100000);
	/* enable USB Device */
	gpioregs->GPCDAT |= (1 << 5);

#if USBDMA
	mode = "DMA";
#else
	mode = "Int";
#endif

#if 0
	U32 mpll_val = clk_powerregs->MPLLCON;
	U32 upll_val = clk_powerregs->UPLLCON;
	printf("UPLLVal [M:%xh,P:%xh,S:%xh]\n", (upll_val & (0xff << 12)) >> 12, (upll_val & (0x3f << 4)) >> 4, (upll_val & 0x3));
	printf("MPLLVal [M:%xh,P:%xh,S:%xh]\n", (mpll_val & (0xff << 12)) >> 12, (mpll_val & (0x3f << 4)) >> 4, (mpll_val & 0x3));
	printf("CLKDIVN:%xh\n", clk_powerregs->CLKDIVN);

	printf("INTMOD = %x\n", intregs->INTMOD);
	printf("INTMSK = %x\n", intregs->INTMSK);
	printf("INTSUBMSK = %x\n", intregs->INTSUBMSK);

	printf("GPBCON = %x\n", gpioregs->GPBCON);
	printf("GPBDAT = %x\n", gpioregs->GPBDAT);
	printf("GPCCON = %x\n", gpioregs->GPCCON);
	printf("GPCUP  = %x\n", gpioregs->GPCUP);
	printf("GPCDAT = %x\n", gpioregs->GPCDAT);
	printf("GPGCON = %x\n", gpioregs->GPGCON);
	printf("GPGDAT = %x\n", gpioregs->GPGDAT);
	printf("\n\n");
#endif
	download_run = 0; //The default menu is the Download & Run mode.
	printf("USB slave is enable!\n");
}

U32 usb_receive(char *buf, size_t len, U32 wait) {
	int first = 1;
	U8 tempMem[16];
	U32 j;
	unsigned int dwRecvTimeSec = 0;

	dwUSBBufReadPtr = dwUSBBufBase;		// USB_BUF_BASE
	dwUSBBufWritePtr = dwUSBBufBase;	// USB_BUF_BASE
	bDMAPending = 0;

	tempDownloadAddress = dwUSBBufBase; // USB_BUF_BASE // RAM_BASE, changed by thisway.diy for wince, 2006.06.18

	downloadAddress = (U32)tempMem; //_RAM_STARTADDRESS;
	downPt = (unsigned char *)downloadAddress;
	//This address is used for receiving first 8 byte.
	downloadFileSize = 0;


	/* File download */
	if (isUsbdSetConfiguration == 0) {
		printf("USB host is not connected yet.\n");
	}

	while (downloadFileSize == 0) { /* wait until send a file */
		if (first == 1 && isUsbdSetConfiguration != 0) {
			printf("USB host is connected. Waiting a download.\n");
			first = 0;
		}
		//TODO:由于awaitkey没有完善，展示去除取消下载的功能
		/*
		char c = awaitkey(1, 0);
		if ((c & 0x7f) == INTR) {
			printf("Cancelled by user\n");
			return 0;
		}
		*/
	}

	if (downloadFileSize - 10 > len) {
		printf("Length of file is too big : %d > %d\n", downloadFileSize - 10, len);
		return 0;
	}

	Timer_InitEx();
	Timer_StartEx();

#if USBDMA

	INTMSK_set(INT_DMA2);

	ClearEp3OutPktReady();
	// indicate the first packit is processed.
	// has been delayed for DMA2 cofiguration.

	if (downloadFileSize > EP3_PKT_SIZE) {
		if (downloadFileSize - EP3_PKT_SIZE <= (0x80000)) {
			/* set the source and length */
			dwUSBBufWritePtr = downloadAddress + EP3_PKT_SIZE - 8;
			dwWillDMACnt = downloadFileSize - EP3_PKT_SIZE;
		} else {
			dwUSBBufWritePtr = downloadAddress + EP3_PKT_SIZE - 8;
			// dwWillDMACnt = 0x80000 - EP3_PKT_SIZE;

			/*
			 * We want When the first DMA interrupt happened,
			 * it has received (0x80000 + 8) bytes data from PC
			 * The format of data PC send out is: <ADDR(DATA):4>+<SIZE(n+10):4>+<DATA:n>+<CS:2>
			 * So, the first 8 bytes isn't the real data we want
			 * We want the dwUSBBufWritePtr is always 0x80000 aligin
			 */
			dwWillDMACnt = 0x80000 + 8 - EP3_PKT_SIZE;
		}
		totalDmaCount = 0;
		ConfigEp3DmaMode(dwUSBBufWritePtr, dwWillDMACnt);
	} else {
		dwUSBBufWritePtr = downloadAddress + downloadFileSize - 8;
		totalDmaCount = downloadFileSize;
	}
#endif

	printf("\nNow, Downloading [ADDRESS:%xh,TOTAL:%d]\n",
		   downloadAddress, downloadFileSize);

	if (wait) {
		printf("RECEIVED FILE SIZE:%8d", 0);

		j = totalDmaCount + 0x10000;
		while (totalDmaCount != downloadFileSize) {
			if (totalDmaCount > j) {
				printf("\b\b\b\b\b\b\b\b%8d", j);
				j = totalDmaCount + 0x10000;
			}
		}
		printf("\b\b\b\b\b\b\b\b%8d ", totalDmaCount);
		dwRecvTimeSec = Timer_StopEx();
		if (dwRecvTimeSec == 0) {
			dwRecvTimeSec = 1;
		}
		printf("(%dKB/S, %dS)\n", (downloadFileSize / dwRecvTimeSec / 1024), dwRecvTimeSec);
	}

	return downloadFileSize - 10;

}
