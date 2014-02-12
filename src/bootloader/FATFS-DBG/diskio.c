/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2007        */
/*-----------------------------------------------------------------------*/
/* This is a stub disk I/O module that acts as front end of the existing */
/* disk I/O modules and attach it to FatFs module with common interface. */
/*-----------------------------------------------------------------------*/

#include "diskio.h"

#include <string.h> // memcpy

#include "stm32f4xx.h"
#include "stm32f4_discovery_sdio_sd.h"

#define BLOCK_SIZE            512 /* Block Size in Bytes */

/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */

DSTATUS disk_initialize (
        BYTE drv                                /* Physical drive nmuber (0..) */
)
{
	DSTATUS stat = 0;

#ifdef DBGIO
	printf("disk_initialize %d\n", drv);
#endif
	
 	/* Supports only single drive */
  if (drv)
  {
    stat |= STA_NOINIT;
  }
	
	/*-------------------------- SD Init ----------------------------- */
  if (SD_Init() !=SD_OK)
  {
#ifdef DBGIO
		puts("Initialization Fail");
#endif
    stat |= STA_NOINIT;
  }

	return(stat);
}



/*-----------------------------------------------------------------------*/
/* Return Disk Status                                                    */

DSTATUS disk_status (
        BYTE drv                /* Physical drive nmuber (0..) */
)
{
	DSTATUS stat = 0;
	
	if (SD_Detect() != SD_PRESENT)
		stat |= STA_NODISK;

	// STA_NOTINIT - Subsystem not initailized
	// STA_PROTECTED - Write protected, MMC/SD switch if available
	
	return(stat);
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */

DRESULT disk_read (
        BYTE drv,               /* Physical drive nmuber (0..) */
        BYTE *buff,             /* Data buffer to store read data */
        DWORD sector,   				/* Sector address (LBA) */
        BYTE count              /* Number of sectors to read (1..255) */
)
{
	SD_Error Status;

#ifdef DBGIO
	printf("disk_read %d %p %10d %d\n",drv,buff,sector,count);
#endif
	
	if (SD_Detect() != SD_PRESENT)
		return(RES_NOTRDY);

	if ((DWORD)buff & 3) // DMA Alignment failure, do single up to aligned buffer
	{
		DRESULT res = RES_OK;
		DWORD scratch[BLOCK_SIZE / 4]; // Alignment assured, you'll need a sufficiently big stack

		while(count--)
		{
			res = disk_read(drv, (void *)scratch, sector++, 1);

			if (res != RES_OK)
				break;

			memcpy(buff, scratch, BLOCK_SIZE);

			buff += BLOCK_SIZE;
		}

		return(res);
	}

  Status = SD_ReadMultiBlocksFIXED(buff, sector, BLOCK_SIZE, count); // 4GB Compliant

	if (Status == SD_OK)
	{
		SDTransferState State;

		Status = SD_WaitReadOperation(); // Check if the Transfer is finished

		while((State = SD_GetStatus()) == SD_TRANSFER_BUSY); // BUSY, OK (DONE), ERROR (FAIL)

		if ((State == SD_TRANSFER_ERROR) || (Status != SD_OK))
			return(RES_ERROR);
		else
			return(RES_OK);
	}
	else
		return(RES_ERROR);
}

/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */

#if _READONLY == 0
DRESULT disk_write (
        BYTE drv,                       /* Physical drive nmuber (0..) */
        const BYTE *buff,       /* Data to be written */
        DWORD sector,           /* Sector address (LBA) */
        BYTE count                      /* Number of sectors to write (1..255) */
)
{
	SD_Error Status;

#ifdef DBGIO
	printf("disk_write %d %p %10d %d\n",drv,buff,sector,count);
#endif
	
	if (SD_Detect() != SD_PRESENT)
		return(RES_NOTRDY);

	if ((DWORD)buff & 3) // DMA Alignment failure, do single up to aligned buffer
	{
		DRESULT res = RES_OK;
		DWORD scratch[BLOCK_SIZE / 4]; // Alignment assured, you'll need a sufficiently big stack

		while(count--)
		{
			memcpy(scratch, buff, BLOCK_SIZE);

			res = disk_write(drv, (void *)scratch, sector++, 1);

			if (res != RES_OK)
				break;

			buff += BLOCK_SIZE;
		}

		return(res);
	}

  Status = SD_WriteMultiBlocksFIXED((uint8_t *)buff, sector, BLOCK_SIZE, count); // 4GB Compliant

	if (Status == SD_OK)
	{
		SDTransferState State;

		Status = SD_WaitWriteOperation(); // Check if the Transfer is finished

		while((State = SD_GetStatus()) == SD_TRANSFER_BUSY); // BUSY, OK (DONE), ERROR (FAIL)

		if ((State == SD_TRANSFER_ERROR) || (Status != SD_OK))
			return(RES_ERROR);
		else
			return(RES_OK);
	}
	else
		return(RES_ERROR);
}
#endif /* _READONLY */




/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */

DRESULT disk_ioctl (
        BYTE drv,               /* Physical drive nmuber (0..) */
        BYTE ctrl,              /* Control code */
        void *buff              /* Buffer to send/receive control data */
)
{
        return RES_OK;
}

/*-----------------------------------------------------------------------*/
/* Get current time                                                      */
/*-----------------------------------------------------------------------*/
DWORD get_fattime(void){
        return 0;
}
