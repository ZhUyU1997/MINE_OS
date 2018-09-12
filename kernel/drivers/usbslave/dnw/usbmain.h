/****************************************************************
 NAME: usbmain.h
 DESC:
 HISTORY:
 Mar.25.2002:purnnamu: reuse the source of S3C2400X u24xmon
 ****************************************************************/

#ifndef __USBMAIN_H__
#define __USBMAIN_H__

void UsbdMain(void);
void IsrUsbd(void);
void IsrDma2(void);
#if 1
void DbgPrintf(char *fmt,...);
#else
#define DbgPrintf(fmt, args...)
#endif

#endif /*__USBMAIN_H__*/
