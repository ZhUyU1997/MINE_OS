/****************************************************************
 NAME: u2440mon.h
 DESC:
 HISTORY:
 Mar.29.2002:purnnamu: created first
 ****************************************************************/

#ifndef __U241MON_H__
#define __U241MON_H__

extern volatile unsigned char *downPt;
extern volatile u32_t totalDmaCount;
extern volatile u32_t downloadFileSize;
extern volatile u32_t downloadAddress;
extern volatile u16_t checkSum;

extern int download_run;
extern u32_t tempDownloadAddress;

#endif /*__U241MON_H__*/
