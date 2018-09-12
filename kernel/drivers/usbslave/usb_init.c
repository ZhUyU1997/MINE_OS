#include <s3c24x0.h>
#include <interrupt.h>
#include <timer.h>
#include <usb/2440usb.h>

S3C24X0_USB_DEVICE  *usbdevregs = S3C24X0_GetBase_USB_DEVICE();
S3C24X0_GPIO *gpioregs = S3C24X0_GetBase_GPIO();
S3C24X0_CLOCK_POWER *clk_powerregs = S3C24X0_GetBase_CLOCK_POWER();
//***************************[ PORTS ]****************************************************
void Port_Init(void) {
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
	gpioregs->GPBDAT &= ~(1 << 0);	/* ��ֹ������ */

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


//����UPLLCON�Ĵ���
void ChangeUPllValue(int mdiv, int pdiv, int sdiv) {
	clk_powerregs->UPLLCON = (mdiv << 12) | (pdiv << 4) | sdiv;
}
void usb_init_slave(void) {
	ChangeUPllValue(0x38, 2, 2);	// UCLK=48Mhz
	usbdev.reset();
	request_irq(IRQ_USBD, IsrUsbd);
	udelay(100000);
	/* enable USB Device */
	usbd_state = USBD_STATE_POWERED;
	gpioregs->GPCDAT |= (1 << 5);
	printf("USB slave is enable!\n");
}

