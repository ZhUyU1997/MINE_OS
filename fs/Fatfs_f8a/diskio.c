/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2010        */
/*-----------------------------------------------------------------------*/
/*                                                                       */
/* This is a stub disk I/O module that acts as front end of the existing */
/* disk I/O modules and attach it to FatFs module with common interface. */
/*-----------------------------------------------------------------------*/

#include "diskio.h"
#include "sdi.h"


/*-----------------------------------------------------------------------*/
/* Correspondence between physical drive number and physical drive.      */
/* Note that Tiny-FatFs supports only single drive and always            */
/* accesses drive number 0.                                              */

int assign_drives(int x,int y)
{
   return 0;
}


/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*
Parameter     :Drive Specifies the physical drive number to initialize. 
Return Values :This function returns a disk status as the result.For details 
               of the disk status, refer to the disk_status function.

Description   :The disk_initialize function initializes a physical drive and 
               put it ready to read/write. When the function succeeded, 
               STA_NOINIT flag in the return value is cleard.Application program 
               should not call this function, or FAT structure on the volume 
               can be collapted. To re-initialize the file system, use f_mount 
               function.This function is called on volume mount process in 
               the FatFs module to manage the media change
*/
DSTATUS disk_initialize (
	BYTE drv				/* Physical drive nmuber (0..) */
)
{
	
	
	return 0;
}



/*-----------------------------------------------------------------------*/
/* Return Disk Status                                                    */
/*
Parameter     :Drive Specifies the physical drive number to be confirmed. 
Return Values :The disk status is returned in combination of following flags. 
               FatFs refers only STA_NOINIT and STA_PROTECTED.

               STA_NOINIT 
               Indicates that the disk drive has not been initialized. 
               This flag is set on: system reset, disk removal and disk_initialize 
               function failed, and cleared on: disk_initialize function succeeded. 
               
               STA_NODISK 
               Indicates that no medium in the drive. This is always cleared on fixed
               disk drive. 
              
               STA_PROTECTED 
               Indicates that the medium is write protected. This is always cleared 
               on the drive that does not support write protect notch. Not valid 
               when STA_NODISK is set.
*/
DSTATUS disk_status (
	BYTE drv		/* Physical drive nmuber (0..) */
)
{	
	return 0;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*
Parameters    :Drive 
               Specifies the physical drive number. 
               
               Buffer 
               Pointer to the byte array to store the read data. The buffer 
               size of number of bytes to be read, sector size * sector count, 
               is required. Note that the specified memory address is not that 
               always aligned to word boundary. If the hardware does not 
               support misaligned data transfer, it must be solved in this function. 
              
               SectorNumber 
               Specifies the start sector number in logical block address (LBA). 
               
               SectorCount 
               Specifies number of sectors to read. The value can be 1 to 128. 
               Generally, a multiple sector transfer request must not be split 
               into single sector transactions to the device, or you may not get 
               good read performance. 

Return Value  :RES_OK (0) 
               The function succeeded. 
               RES_ERROR 
               Any hard error occured during the read operation and could not recover it. 
               RES_PARERR 
               Invalid parameter. 
               RES_NOTRDY 
               The disk drive has not been initialized. 
*/


DRESULT disk_read (
	BYTE drv,		/* Physical drive nmuber (0..) */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Sector address (LBA) */
	BYTE count		/* Number of sectors to read (1..255) */
)
{
    drv=drv;
    
    if( Read_Block( (U32)(sector+LBA_OFFSET),count,(BYTE *)buff) )
    {
	   return 0;
	}
	else
	{
	   return RES_ERROR;
	}
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */

#if _READONLY == 0
DRESULT disk_write (
	BYTE drv,			/* Physical drive nmuber (0..) */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Sector address (LBA) */
	BYTE count			/* Number of sectors to write (1..255) */
)
{
	
	drv=drv;
	
	if( Write_Block( (U32)(sector+LBA_OFFSET),count,(BYTE *)buff) )
	{   
	   return 0;
	}
	else
	{
	   return RES_ERROR;
	}

}
#endif /* _READONLY */



/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */

DRESULT disk_ioctl (
	BYTE drv,		/* Physical drive nmuber (0..) */
	BYTE ctrl,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{

	return 0;
}



DWORD get_fattime(void){
	return 0;
}

