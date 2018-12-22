/****************************************************************
 NAME: usbout.c
 DESC: USB bulk-OUT operation related functions
 HISTORY:
 Mar.25.2002:purnnamu: ported for S3C2410X.
 Mar.27.2002:purnnamu: DMA is enabled.
 ****************************************************************/
#include <s3c24x0.h>
#include <interrupt.h>
#include "def.h"
#include "2440usb.h"
#include "usbmain.h"
#include "usb.h"
#include "usblib.h"
#include "usbsetup.h"
#include "usbout.h"
#include "usbinit.h"

extern volatile u32_t dwUSBBufReadPtr;
extern volatile u32_t dwUSBBufWritePtr;
extern volatile u32_t dwWillDMACnt;
extern volatile u32_t bDMAPending;
extern volatile u32_t dwUSBBufBase;
extern volatile u32_t dwUSBBufSize;

extern S3C24X0_INTERRUPT * intregs;
extern S3C24X0_USB_DEVICE * usbdevregs;
extern S3C24X0_DMAS * dmaregs;

static void PrintEpoPkt(u8_t *pt, int cnt);
static void RdPktEp3_CheckSum(u8_t *buf, int num);

// ===================================================================
// All following commands will operate in case
// - out_csr3 is valid.
// ===================================================================

#define CLR_EP3_OUT_PKT_READY() usbdevregs->OUT_CSR1_REG = ( out_csr3 & (~EPO_WR_BITS) &(~EPO_OUT_PKT_READY) )
#define SET_EP3_SEND_STALL()	usbdevregs->OUT_CSR1_REG = ( out_csr3 & (~EPO_WR_BITS) | EPO_SEND_STALL)
#define CLR_EP3_SENT_STALL()	usbdevregs->OUT_CSR1_REG = ( out_csr3 & (~EPO_WR_BITS) &(~EPO_SENT_STALL) )
#define FLUSH_EP3_FIFO()		usbdevregs->OUT_CSR1_REG = ( out_csr3 & (~EPO_WR_BITS) |EPO_FIFO_FLUSH) 

/* VERY IMPORTANT NOTE */
// Prepare for the packit size constraint!!!
// EP3 = OUT end point.

static u8_t ep3Buf[EP3_PKT_SIZE];

void Ep3Handler(void) {
	u8_t out_csr3;
	int fifoCnt;
	usbdevregs->INDEX_REG = 3;
	out_csr3 = usbdevregs->OUT_CSR1_REG;

	DbgPrintf("<3:%x]\n", out_csr3);

	if (out_csr3 & EPO_OUT_PKT_READY) {
		fifoCnt = usbdevregs->OUT_FIFO_CNT1_REG;
#if 0
		RdPktEp3(ep3Buf, fifoCnt);
		PrintEpoPkt(ep3Buf, fifoCnt);
#else

		if (downloadFileSize == 0) {
			RdPktEp3((u8_t *)downPt, 8);

			if (download_run == 0) {
				downloadAddress = tempDownloadAddress;
			} else {
				downloadAddress = ((u32_t *)downPt)[0];
				dwUSBBufReadPtr = downloadAddress;
				dwUSBBufWritePtr = downloadAddress;
			}
			downloadFileSize = ((u32_t *)downPt)[1];
			checkSum = 0;
			downPt = (u8_t *)downloadAddress;

			RdPktEp3_CheckSum((u8_t *)downPt, fifoCnt - 8); //The first 8-bytes are deleted.
			downPt += fifoCnt - 8;

#if USBDMA
			//CLR_EP3_OUT_PKT_READY() is not executed.
			//So, USBD may generate NAK until DMA2 is configured for USB_EP3;
			INTSUBMSK_clr(INT_USBD); //for debug
			return;
#endif
		} else {
#if USBDMA
			printf("<ERROR>\n");
#endif
			RdPktEp3_CheckSum((u8_t *)downPt, fifoCnt);
			downPt += fifoCnt; //fifoCnt=64
		}
#endif
		CLR_EP3_OUT_PKT_READY();
#if 0
		if (((rOUT_CSR1_REG & 0x1) == 1) && ((rEP_INT_REG & 0x8) == 0)) {
			fifoCnt = rOUT_FIFO_CNT1_REG;
			RdPktEp3_CheckSum((u8_t *)downPt, fifoCnt);
			downPt += fifoCnt; //fifoCnt=64
			CLR_EP3_OUT_PKT_READY();
		}
#endif
		return;
	}


	//I think that EPO_SENT_STALL will not be set to 1.
	if (out_csr3 & EPO_SENT_STALL) {
		DbgPrintf("[STALL]\n");
		CLR_EP3_SENT_STALL();
		return;
	}
}



void PrintEpoPkt(u8_t *pt, int cnt) {
	DbgPrintf("[BOUT:%d:", cnt);
	for (int i = 0; i < cnt; i++)
		DbgPrintf("%x,", pt[i]);
	DbgPrintf("]\n");
}


void RdPktEp3_CheckSum(u8_t *buf, int num) {
	for (int i = 0; i < num; i++) {
		buf[i] = (u8_t)usbdevregs->fifo[3].EP_FIFO_REG;
		checkSum += buf[i];
	}
}



void IsrDma2(void) {
	u8_t out_csr3;
	u32_t dwEmptyCnt;
	u8_t saveIndexReg = usbdevregs->INDEX_REG;
	usbdevregs->INDEX_REG = 3;
	out_csr3 = usbdevregs->OUT_CSR1_REG;

	//When the first DMA interrupt happened, it has received max (0x80000 + EP3_PKT_SIZE) bytes data from PC
	if (!totalDmaCount)
		totalDmaCount = dwWillDMACnt + EP3_PKT_SIZE;
	else
		totalDmaCount += dwWillDMACnt;
	dwUSBBufWritePtr = ((dwUSBBufWritePtr + dwWillDMACnt - dwUSBBufBase) % dwUSBBufSize) + dwUSBBufBase;

	if (totalDmaCount >= downloadFileSize) {//传输完毕
		totalDmaCount = downloadFileSize;
		ConfigEp3IntMode();
		if (out_csr3 & EPO_OUT_PKT_READY) {
			CLR_EP3_OUT_PKT_READY();
		}
		INTMSK_clr(INT_DMA2);
		INTMSK_set(INT_USBD);
	} else {//传输未完成
		if ((totalDmaCount + 0x80000) < downloadFileSize) {
			dwWillDMACnt = 0x80000;
		} else {
			dwWillDMACnt = downloadFileSize - totalDmaCount;
		}
		dwEmptyCnt = (dwUSBBufReadPtr - dwUSBBufWritePtr - 1 + dwUSBBufSize) % dwUSBBufSize;
		if (dwEmptyCnt >= dwWillDMACnt) {
			ConfigEp3DmaMode(dwUSBBufWritePtr, dwWillDMACnt);
		} else {
			bDMAPending = 1;
		}
	}
	usbdevregs->INDEX_REG = saveIndexReg;
}


void ClearEp3OutPktReady(void) {
	u8_t out_csr3;
	usbdevregs->INDEX_REG = 3;
	out_csr3 = usbdevregs->OUT_CSR1_REG;
	CLR_EP3_OUT_PKT_READY();
}
