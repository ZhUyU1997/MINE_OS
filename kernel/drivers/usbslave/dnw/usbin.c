/****************************************************************
 NAME: usbin.c
 DESC: usb bulk-IN operation
 HISTORY:
 Mar.25.2002:purnnamu: ported for S3C2410X.
 ****************************************************************/
#include <s3c24x0.h>
#include "def.h"
#include "2440usb.h"
#include "usbmain.h"
#include "usb.h"
#include "usblib.h"
#include "usbsetup.h"
#include "usbin.h"

extern S3C24X0_USB_DEVICE * usbdevregs;
extern S3C24X0_DMAS * dmaregs;

static void PrintEpiPkt(u8_t *pt, int cnt);


/*
 * All following commands will operate in case
 * - in_csr1 is valid.
 */

#define SET_EP1_IN_PKT_READY()  usbdevregs->EP0_CSR_IN_CSR1_REG = ( in_csr1 & (~EPI_WR_BITS) | EPI_IN_PKT_READY )
#define SET_EP1_SEND_STALL()	usbdevregs->EP0_CSR_IN_CSR1_REG = ( in_csr1 & (~EPI_WR_BITS) | EPI_SEND_STALL   )
#define CLR_EP1_SENT_STALL()	usbdevregs->EP0_CSR_IN_CSR1_REG = ( in_csr1 & (~EPI_WR_BITS) & (~EPI_SENT_STALL))
#define FLUSH_EP1_FIFO()		usbdevregs->EP0_CSR_IN_CSR1_REG = ( in_csr1 & (~EPI_WR_BITS) | EPI_FIFO_FLUSH   )


/* VERY IMPORTANT NOTE */
// Prepare the code for the packit size constraint!!!
// EP1 = IN end point.

u8_t ep1Buf[EP1_PKT_SIZE];
int transferIndex = 0;

void PrepareEp1Fifo(void) {
	usbdevregs->INDEX_REG = 1;
	u8_t in_csr1 = usbdevregs->EP0_CSR_IN_CSR1_REG;
	for (int i = 0; i < EP1_PKT_SIZE; i++)
		ep1Buf[i] = (u8_t)(transferIndex + i);
	WrPktEp1(ep1Buf, EP1_PKT_SIZE);
	SET_EP1_IN_PKT_READY();
}


void Ep1Handler(void) {
	u8_t in_csr1;
	int i;
	usbdevregs->INDEX_REG = 1;
	in_csr1 = usbdevregs->EP0_CSR_IN_CSR1_REG;
	DbgPrintf("<1:%x]\n", in_csr1);
	//I think that EPI_SENT_STALL will not be set to 1.
	if (in_csr1 & EPI_SENT_STALL) {
		DbgPrintf("[STALL]\n");
		CLR_EP1_SENT_STALL();
		return;
	}
	//IN_PKT_READY is cleared
	//The data transfered was ep1Buf[] which was already configured
	PrintEpiPkt(ep1Buf, EP1_PKT_SIZE);
	transferIndex++;
	PrepareEp1Fifo();
	//IN_PKT_READY is set
	//This packit will be used for next IN packit.
	return;
}

void PrintEpiPkt(u8_t *pt, int cnt) {
	DbgPrintf("[B_IN:%d:", cnt);
	for (int i = 0; i < cnt; i++)
		DbgPrintf("%x,", pt[i]);
	DbgPrintf("]\n");
}

