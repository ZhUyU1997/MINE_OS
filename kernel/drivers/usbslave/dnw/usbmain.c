/****************************************************************
 NAME: usbmain.c
 DESC: endpoint interrupt handler
       USB init jobs
 HISTORY:
 Mar.25.2002:purnnamu: ported for S3C2410X.
 Mar.27.2002:purnnamu: DMA is enabled.
 ****************************************************************/
#include <stdio.h>
#include <interrupt.h>
#include <s3c24x0.h>

#include "2440usb.h"
#include "usbmain.h"
#include "usblib.h"
#include "usbsetup.h"
#include "usbout.h"
#include "usbin.h"

extern S3C24X0_USB_DEVICE * usbdevregs;
extern S3C24X0_DMAS * dmaregs;
extern S3C24X0_CLOCK_POWER * clk_powerregs;
/**************************[UPLL]*******************************/
void ChangeUPllValue(int mdiv, int pdiv, int sdiv) {
	clk_powerregs->UPLLCON = (mdiv << 12) | (pdiv << 4) | sdiv;
}
/**************************
    Some PrepareEp1Fifo() should be deleted
 **************************/

void UsbdMain(void) {
	//ChangeUPllValue(0x38,2,1);	// UCLK=96Mhz
	ChangeUPllValue(0x38, 2, 2);	// UCLK=48Mhz
	InitDescriptorTable();
	ConfigUsbd();
	PrepareEp1Fifo();
}
void IsrUsbd(unsigned long nr, unsigned long parameter) {
	u8_t saveIndexReg = usbdevregs->INDEX_REG;
	u8_t usbdIntpnd	= usbdevregs->USB_INT_REG;
	u8_t epIntpnd		= usbdevregs->EP_INT_REG;
	DbgPrintf("{\n", epIntpnd, usbdIntpnd);
	DbgPrintf("[INT:EP_I=%x,USBI=%x]", epIntpnd, usbdIntpnd);

	if (usbdIntpnd & SUSPEND_INT) {
		usbdevregs->USB_INT_REG = SUSPEND_INT;
		DbgPrintf("<SUS]\n");
	}
	if (usbdIntpnd & RESUME_INT) {
		usbdevregs->USB_INT_REG = RESUME_INT;
		DbgPrintf("<RSM]\n");
	}
	if (usbdIntpnd & RESET_INT) {
		DbgPrintf("<RST] ReconfigUsbd\n");
		ReconfigUsbd();
		usbdevregs->USB_INT_REG = RESET_INT; //RESET_INT should be cleared after ResetUsbd().
		PrepareEp1Fifo();
	}

	if (epIntpnd & EP0_INT) {
		usbdevregs->EP_INT_REG = EP0_INT;
		Ep0Handler();
	}
	if (epIntpnd & EP1_INT) {
		usbdevregs->EP_INT_REG = EP1_INT;
		Ep1Handler();
	}
	if (epIntpnd & EP2_INT) {
		usbdevregs->EP_INT_REG = EP2_INT;
		DbgPrintf("<2:TBD]\n");   //not implemented yet
		//Ep2Handler();
	}
	if (epIntpnd & EP3_INT) {
		usbdevregs->EP_INT_REG = EP3_INT;
		Ep3Handler();
	}
	if (epIntpnd & EP4_INT) {
		usbdevregs->EP_INT_REG = EP4_INT;
		DbgPrintf("<4:TBD]\n");   //not implemented yet
		//Ep4Handler();
	}

	ClearPending(INT_USBD);
	usbdevregs->INDEX_REG = saveIndexReg;
	DbgPrintf("}\n");
}

void DbgPrintf(char *fmt,...) {
	static char str[1024*24];
	static char *p = str;
	if(strcmp(fmt, "show") == 0){
		for(int i = 0; i < strlen(str); i++)
			putc(str[i]);
		return;
	}
	va_list ap;
	va_start(ap,fmt);
	p += vsprintf(p,fmt,ap);
	va_end(ap);
}
