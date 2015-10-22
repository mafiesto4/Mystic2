
// Include
#include "diskio.h"
#include "..\Kernel\Config.h"

// Select Driver library based on destination platform
#if OS_DEST_PLATFORM == OS_LPC17xx

	#include "SPI_MSD_Driver.h"
	
	// Card Info
	extern MSD_CARDINFO CardInfo;

#else

	#error "Invalid destination platform!"

#endif

// Initialize a Drive
Byte disk_initialize(void)
{
	#if OS_DEST_PLATFORM == OS_LPC17xx
	
		return MSD_Init();
	
	#endif
}

// Get Disk Status
Byte disk_status(void)
{
	return 0;
	//return STA_NOINIT;
}

// Read Sector(s)
DRESULT disk_read (
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Sector address (LBA) */
	BYTE count		/* Number of sectors to read (1..128) */
)
{
	int Status;
  if( !count )
  {    
    return RES_PARERR;
  }

    if(count==1)  
    {       
	  Status =  MSD_ReadSingleBlock( sector ,buff );
    }                                                
    else  
    {   
      Status = MSD_ReadMultiBlock( sector , buff ,count);
    }                                                
    if(Status == 0)
    {
      return RES_OK;
    }
    else
    {
      return RES_ERROR;
    }
}

// Write Sector(s)
#if _USE_WRITE
DRESULT disk_write (
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Sector address (LBA) */
	BYTE count			/* Number of sectors to write (1..128) */
)
{
	int Status;
  if( !count )
  {  
    return RES_PARERR;
  }
  
    if(count==1)
    {   
       Status = MSD_WriteSingleBlock( sector , (uint8_t *)(&buff[0]) ); 
    }                                                
    else   
    {   
       Status = MSD_WriteMultiBlock( sector , (uint8_t *)(&buff[0]) , count );	  
    }                                                
    if(Status == 0)
    {
       return RES_OK;
    }
	
       return RES_ERROR;
}
#endif

// Miscellaneous Functions

#if _USE_IOCTL
DRESULT disk_ioctl (
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	MSD_GetCardInfo(&CardInfo);

	switch (cmd) 
	{
	  case CTRL_SYNC :
	      
		return RES_OK;

	  case GET_SECTOR_COUNT : 
	    *(DWORD*)buff = CardInfo.Capacity/CardInfo.BlockSize;
	    return RES_OK;

	  case GET_BLOCK_SIZE :
	    *(WORD*)buff = CardInfo.BlockSize;
	    return RES_OK;	

	  case CTRL_POWER :
		break;

	  case CTRL_LOCK :
		break;

	  case CTRL_EJECT :
		break;

      /* MMC/SDC command */
	  case MMC_GET_TYPE :
		break;

	  case MMC_GET_CSD :
		break;

	  case MMC_GET_CID :
		break;

	  case MMC_GET_OCR :
		break;

	  case MMC_GET_SDSTAT :
		break;	
	}
	return RES_PARERR;   
}

/* 得到文件Calendar格式的建立日期,是DWORD get_fattime (void) 逆变换 */							
/*-----------------------------------------------------------------------*/
/* User defined function to give a current time to fatfs module          */
/* 31-25: Year(0-127 org.1980), 24-21: Month(1-12), 20-16: Day(1-31) */                                                                                                                                                                                                                                          
/* 15-11: Hour(0-23), 10-5: Minute(0-59), 4-0: Second(0-29 *2) */                                                                                                                                                                                                                                                
DWORD get_fattime (void)
{
	
    return 0;
}

#endif
