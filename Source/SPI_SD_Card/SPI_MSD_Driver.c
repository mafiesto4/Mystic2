
// Include
#include "SPI_MSD_Driver.h"
#include <stdio.h>
#include "..\Kernel\Config.h"
#include "..\Kernel\Kernel.h"

// Card info
MSD_CARDINFO CardInfo;

// Write data to SPI
__inline int _spi_read_write(uint8_t data)
{
	// Wait for current SSP activity complete
	while (SSP_GetStatus(LPC_SSP0, SSP_STAT_BUSY) ==  SET);

	SSP_SendData(LPC_SSP0, (uint16_t) data);

	while (SSP_GetStatus(LPC_SSP0, SSP_STAT_RXFIFO_NOTEMPTY) == RESET);
	
	return (SSP_ReceiveData(LPC_SSP0));
}

// SD Card SPI Configuration
void MSD_SPI_Configuration(void)
{
	PINSEL_CFG_Type PinCfg;
    SSP_CFG_Type SSP_ConfigStruct;
	/*
	 * Initialize SPI pin connect
	 * P3.25 - SD_CD - used as GPIO
	 * P1.20 - SCK
	 * P1.21 - SD_CS - used as GPIO
	 * P1.23 - MISO
	 * P1.24 - MOSI
	 * P3.26 - SD_PWR - used as GPIO
	 */
	PinCfg.Funcnum = 3;
	PinCfg.OpenDrain = 0;
	PinCfg.Pinmode = 0;
	PinCfg.Portnum = 1;
	PinCfg.Pinnum = 20;
	PINSEL_ConfigPin(&PinCfg);
	PinCfg.Pinnum = 23;
	PINSEL_ConfigPin(&PinCfg);
	PinCfg.Pinnum = 24;
	PINSEL_ConfigPin(&PinCfg);
	PinCfg.Pinnum = 21;
	PinCfg.Funcnum = 0;
	PINSEL_ConfigPin(&PinCfg);
	PinCfg.Portnum = 3;
	PinCfg.Pinnum = 25;
	PinCfg.Funcnum = 0;
	PINSEL_ConfigPin(&PinCfg);
	PinCfg.Pinnum = 26;
	PINSEL_ConfigPin(&PinCfg);

    /* P3.25 SD_CD is Input */
    GPIO_SetDir(SD_CD_PORT_NUM, (1<<SD_CD_PIN_NUM), 0);
	GPIO_SetValue(SD_CD_PORT_NUM, (1<<SD_CD_PIN_NUM));  
    /* P1.21 SD_CS is output */
    GPIO_SetDir(SD_CS_PORT_NUM, (1<<SD_CS_PIN_NUM), 1);
	GPIO_SetValue(SD_CS_PORT_NUM, (1<<SD_CS_PIN_NUM));  
    /* P3.26 SD_PWR is output */
    GPIO_SetDir(SD_PWR_PORT_NUM, (1<<SD_PWR_PIN_NUM), 1);
	GPIO_SetValue(SD_PWR_PORT_NUM, (1<<SD_PWR_PIN_NUM));  

	// Initialize SSP configuration structure to default
	SSP_ConfigStructInit(&SSP_ConfigStruct);
	
	// Initialize SSP peripheral with parameter given in structure above
	SSP_Init(LPC_SSP0, &SSP_ConfigStruct);
	
	_card_disable(); 
	
	MSD_SPIHighSpeed(0);	

	SSP_Cmd(LPC_SSP0, ENABLE);	
}

/*******************************************************************************
* Function Name  : MSD_SPIHighSpeed
* Description    : SD Card Speed Set
* Input          : - b_high: 1 = 18MHz, 0 = 281.25Hz
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
void MSD_SPIHighSpeed(uint8_t b_high)
{
    SSP_CFG_Type SSP_ConfigStruct;

	// Initialize SSP configuration structure to default
	SSP_ConfigStructInit(&SSP_ConfigStruct);

	// Speed select
	if(b_high == 0)
	{
		SSP_ConfigStruct.ClockRate = 1000000;
	}
	else
	{
	    SSP_ConfigStruct.ClockRate = 10000000;
	}
	
	SSP_Init(LPC_SSP0, &SSP_ConfigStruct);
	
	SSP_Cmd(LPC_SSP0, ENABLE);	
}

// SD Card initializtion
int MSD_Init(void)
{
	uint8_t r1;	
	uint8_t buff[6] = {0};
	uint16_t retry; 

	MSD_SPI_Configuration();

	// Check, if no card insert
	if( _card_insert() )
	{ 
		#if OS_DEBUG_SD == 1 
			OS_Debug_Print("There is no card detected!");
		#endif
		
		// FATFS error flag
		return -1;
	}

	#if OS_DEBUG_SD == 1 
		OS_Debug_Print("SD Card initializtion...");
	#endif
	
	// Power on and delay some times
	for(retry = 0; retry < 0x100; retry++)
	{
		_card_power_on();
	}	

	// Start send 74 clocks at least
	for(retry = 0; retry < 10; retry++)
	{
		_spi_read_write(DUMMY_BYTE);
	}	
	
	// Start send CMD0 till return 0x01 means in IDLE state
	for(retry = 0; retry < 0xFFF; retry++)
	{
		r1 = _send_command(CMD0, 0, 0x95);
		if(r1 == 0x01)
		{
			retry = 0;
			break;
		}
	}
	
	// Timeout return
	if(retry == 0xFFF)
	{
		#if OS_DEBUG_SD == 1 
			OS_Debug_Print("Reset card into IDLE state failed!");
		#endif
		
		return 1;
	}
	
	// Get the card type, version
	r1 = _send_command_hold(CMD8, 0x1AA, 0x87);
	
	// r1=0x05 -> V1.0
	if(r1 == 0x05)
	{
		CardInfo.CardType = CARDTYPE_SDV1;

		// End of CMD8, chip disable and dummy byte
		_card_disable();
		_spi_read_write(DUMMY_BYTE);

		/* SD1.0/MMC start initialize */
		/* Send CMD55+ACMD41, No-response is a MMC card, otherwise is a SD1.0 card */
		for(retry=0; retry<0xFFF; retry++)
		{
			// Should be return 0x01
			r1 = _send_command(CMD55, 0, 0);
			
			if(r1 != 0x01)
			{
				
				#if OS_DEBUG_SD == 1 
					OS_Debug_Print("Send CMD55 should return 0x01, response=0x%02x", r1);
				#endif
					
				return r1;
			}
		
			// Should be return 0x00 
			r1 = _send_command(ACMD41, 0, 0);
			if(r1 == 0x00)
			{
				retry = 0;
				break;
			}
		}

			/* MMC card initialize start */
			if(retry == 0xFFF)
			{
			for(retry=0; retry<0xFFF; retry++)
			{
			r1 = _send_command(CMD1, 0, 0);		/* should be return 0x00 */
			if(r1 == 0x00)
			{
			retry = 0;
			break;
			}
		}

		/* Timeout return */
		if(retry == 0xFFF)
		{
		#if OS_DEBUG_SD == 1 
		OS_Debug_Print("Send CMD1 should return 0x00, response=0x%02x", r1);
		#endif
		return 2;
		}	

		CardInfo.CardType = CARDTYPE_MMC;		
		#if OS_DEBUG_SD == 1 
		OS_Debug_Print("Card Type: MMC");
		#endif 				
	  }		
		// SD1.0 card detected, print information
		#if OS_DEBUG_SD == 1 
			  else
			  {
				  OS_Debug_Print("Card Type: SD V1");
			  }
		#endif 
		
	  // Set spi speed high
	  MSD_SPIHighSpeed(1);		
		
	  /* CRC disable */
	  r1 = _send_command(CMD59, 0, 0x01);
	  if(r1 != 0x00)
	  {
			#if OS_DEBUG_SD == 1 
				OS_Debug_Print("Send CMD59 should return 0x00, response=0x%02x", r1);
			#endif
		  return r1;		/* response error, return r1 */
	  }
		  
	  /* Set the block size */
	  r1 = _send_command(CMD16, MSD_BLOCKSIZE, 0xFF);
	  if(r1 != 0x00)
	  {
			#if OS_DEBUG_SD == 1 
				OS_Debug_Print("Send CMD16 should return 0x00, response=0x%02x", r1);
			#endif
		  return r1;		// response error, return r1
	  }
   }	
	
   // r1=0x01 -> V2.x, read OCR register, check version
   else if(r1 == 0x01)
   {
	 /* 4Bytes returned after CMD8 sent	*/
	 buff[0] = _spi_read_write(DUMMY_BYTE);				/* should be 0x00 */
	 buff[1] = _spi_read_write(DUMMY_BYTE);				/* should be 0x00 */
	 buff[2] = _spi_read_write(DUMMY_BYTE);				/* should be 0x01 */
	 buff[3] = _spi_read_write(DUMMY_BYTE);				/* should be 0xAA */
		
	 /* End of CMD8, chip disable and dummy byte */ 
	 _card_disable();
	 _spi_read_write(DUMMY_BYTE);
		
	 /* Check voltage range be 2.7-3.6V	*/
	 if(buff[2]==0x01 && buff[3]==0xAA)
	 {
		for(retry=0; retry<0xFFF; retry++)
		{
			r1 = _send_command(CMD55, 0, 0);			/* should be return 0x01 */
			if(r1!=0x01)
			{
				#if OS_DEBUG_SD == 1 
						OS_Debug_Print("Send CMD55 should return 0x01, response=0x%02x", r1);
				#endif
				return r1;
			}				

			r1 = _send_command(ACMD41, 0x40000000, 0);	/* should be return 0x00 */
			if(r1 == 0x00)
			{
				retry = 0;
				break;
			}
		}
 		 	
		/* Timeout return */
		if(retry == 0xFFF)
		{
			#if OS_DEBUG_SD == 1 
					OS_Debug_Print("Send ACMD41 should return 0x00, response=0x%02x", r1);
			#endif
			return 3;
		}

		/* Read OCR by CMD58 */
	    r1 = _send_command_hold(CMD58, 0, 0);
	    if(r1!=0x00)
	    {
			#if OS_DEBUG_SD == 1 
				OS_Debug_Print("Send CMD58 should return 0x00, response=0x%02x", r1);
			#endif
            return r1;		/* response error, return r1 */
	    }

	    buff[0] = _spi_read_write(DUMMY_BYTE);					
		buff[1] = _spi_read_write(DUMMY_BYTE);					
		buff[2] = _spi_read_write(DUMMY_BYTE);					
		buff[3] = _spi_read_write(DUMMY_BYTE);					

		/* End of CMD58, chip disable and dummy byte */
		_card_disable();
		_spi_read_write(DUMMY_BYTE);
	
	    /* OCR -> CCS(bit30)  1: SDV2HC	 0: SDV2 */
	    if(buff[0] & 0x40)
	    {
           CardInfo.CardType = CARDTYPE_SDV2HC;
			#if OS_DEBUG_SD == 1 
				OS_Debug_Print("Card Type: SD V2HC");
			#endif 	
	    }
	    else
	    {
           CardInfo.CardType = CARDTYPE_SDV2;
			#if OS_DEBUG_SD == 1 
				OS_Debug_Print("Card Type: SD V2");
			#endif 	
	    }

		// Set spi speed high
		MSD_SPIHighSpeed(1);
		}	
   }
   
   #if OS_DEBUG_SD == 1 
		OS_Debug_Print("SD card ready!");
	#endif
   
	// Good
	return 0;
}

// Get SD Card Information
int MSD_GetCardInfo(PMSD_CARDINFO cardinfo)
{
  uint8_t r1;
  uint8_t CSD_Tab[16];
  uint8_t CID_Tab[16];

  /* Send CMD9, Read CSD */
  r1 = _send_command(CMD9, 0, 0xFF);
  if(r1 != 0x00)
  {
    return r1;
  }

  if(_read_buffer(CSD_Tab, 16, RELEASE))
  {
	return 1;
  }

  /* Send CMD10, Read CID */
  r1 = _send_command(CMD10, 0, 0xFF);
  if(r1 != 0x00)
  {
    return r1;
  }

  if(_read_buffer(CID_Tab, 16, RELEASE))
  {
	return 2;
  }  

  /* Byte 0 */
  cardinfo->CSD.CSDStruct = (CSD_Tab[0] & 0xC0) >> 6;
  cardinfo->CSD.SysSpecVersion = (CSD_Tab[0] & 0x3C) >> 2;
  cardinfo->CSD.Reserved1 = CSD_Tab[0] & 0x03;
  /* Byte 1 */
  cardinfo->CSD.TAAC = CSD_Tab[1] ;
  /* Byte 2 */
  cardinfo->CSD.NSAC = CSD_Tab[2];
  /* Byte 3 */
  cardinfo->CSD.MaxBusClkFrec = CSD_Tab[3];
  /* Byte 4 */
  cardinfo->CSD.CardComdClasses = CSD_Tab[4] << 4;
  /* Byte 5 */
  cardinfo->CSD.CardComdClasses |= (CSD_Tab[5] & 0xF0) >> 4;
  cardinfo->CSD.RdBlockLen = CSD_Tab[5] & 0x0F;
  /* Byte 6 */
  cardinfo->CSD.PartBlockRead = (CSD_Tab[6] & 0x80) >> 7;
  cardinfo->CSD.WrBlockMisalign = (CSD_Tab[6] & 0x40) >> 6;
  cardinfo->CSD.RdBlockMisalign = (CSD_Tab[6] & 0x20) >> 5;
  cardinfo->CSD.DSRImpl = (CSD_Tab[6] & 0x10) >> 4;
  cardinfo->CSD.Reserved2 = 0; /* Reserved */
  cardinfo->CSD.DeviceSize = (CSD_Tab[6] & 0x03) << 10;
  /* Byte 7 */
  cardinfo->CSD.DeviceSize |= (CSD_Tab[7]) << 2;
  /* Byte 8 */
  cardinfo->CSD.DeviceSize |= (CSD_Tab[8] & 0xC0) >> 6;
  cardinfo->CSD.MaxRdCurrentVDDMin = (CSD_Tab[8] & 0x38) >> 3;
  cardinfo->CSD.MaxRdCurrentVDDMax = (CSD_Tab[8] & 0x07);
  /* Byte 9 */
  cardinfo->CSD.MaxWrCurrentVDDMin = (CSD_Tab[9] & 0xE0) >> 5;
  cardinfo->CSD.MaxWrCurrentVDDMax = (CSD_Tab[9] & 0x1C) >> 2;
  cardinfo->CSD.DeviceSizeMul = (CSD_Tab[9] & 0x03) << 1;
  /* Byte 10 */
  cardinfo->CSD.DeviceSizeMul |= (CSD_Tab[10] & 0x80) >> 7;
  cardinfo->CSD.EraseGrSize = (CSD_Tab[10] & 0x7C) >> 2;
  cardinfo->CSD.EraseGrMul = (CSD_Tab[10] & 0x03) << 3;
  /* Byte 11 */
  cardinfo->CSD.EraseGrMul |= (CSD_Tab[11] & 0xE0) >> 5;
  cardinfo->CSD.WrProtectGrSize = (CSD_Tab[11] & 0x1F);
  /* Byte 12 */
  cardinfo->CSD.WrProtectGrEnable = (CSD_Tab[12] & 0x80) >> 7;
  cardinfo->CSD.ManDeflECC = (CSD_Tab[12] & 0x60) >> 5;
  cardinfo->CSD.WrSpeedFact = (CSD_Tab[12] & 0x1C) >> 2;
  cardinfo->CSD.MaxWrBlockLen = (CSD_Tab[12] & 0x03) << 2;
  /* Byte 13 */
  cardinfo->CSD.MaxWrBlockLen |= (CSD_Tab[13] & 0xc0) >> 6;
  cardinfo->CSD.WriteBlockPaPartial = (CSD_Tab[13] & 0x20) >> 5;
  cardinfo->CSD.Reserved3 = 0;
  cardinfo->CSD.ContentProtectAppli = (CSD_Tab[13] & 0x01);
  /* Byte 14 */
  cardinfo->CSD.FileFormatGrouop = (CSD_Tab[14] & 0x80) >> 7;
  cardinfo->CSD.CopyFlag = (CSD_Tab[14] & 0x40) >> 6;
  cardinfo->CSD.PermWrProtect = (CSD_Tab[14] & 0x20) >> 5;
  cardinfo->CSD.TempWrProtect = (CSD_Tab[14] & 0x10) >> 4;
  cardinfo->CSD.FileFormat = (CSD_Tab[14] & 0x0C) >> 2;
  cardinfo->CSD.ECC = (CSD_Tab[14] & 0x03);
  /* Byte 15 */
  cardinfo->CSD.CSD_CRC = (CSD_Tab[15] & 0xFE) >> 1;
  cardinfo->CSD.Reserved4 = 1;

  if(cardinfo->CardType == CARDTYPE_SDV2HC)
  {
	 /* Byte 7 */
	 cardinfo->CSD.DeviceSize = (uint16_t)(CSD_Tab[8]) *256;
	 /* Byte 8 */
	 cardinfo->CSD.DeviceSize += CSD_Tab[9] ;
  }

  cardinfo->Capacity = cardinfo->CSD.DeviceSize * MSD_BLOCKSIZE * 1024;
  cardinfo->BlockSize = MSD_BLOCKSIZE;

  /* Byte 0 */
  cardinfo->CID.ManufacturerID = CID_Tab[0];
  /* Byte 1 */
  cardinfo->CID.OEM_AppliID = CID_Tab[1] << 8;
  /* Byte 2 */
  cardinfo->CID.OEM_AppliID |= CID_Tab[2];
  /* Byte 3 */
  cardinfo->CID.ProdName1 = CID_Tab[3] << 24;
  /* Byte 4 */
  cardinfo->CID.ProdName1 |= CID_Tab[4] << 16;
  /* Byte 5 */
  cardinfo->CID.ProdName1 |= CID_Tab[5] << 8;
  /* Byte 6 */
  cardinfo->CID.ProdName1 |= CID_Tab[6];
  /* Byte 7 */
  cardinfo->CID.ProdName2 = CID_Tab[7];
  /* Byte 8 */
  cardinfo->CID.ProdRev = CID_Tab[8];
  /* Byte 9 */
  cardinfo->CID.ProdSN = CID_Tab[9] << 24;
  /* Byte 10 */
  cardinfo->CID.ProdSN |= CID_Tab[10] << 16;
  /* Byte 11 */
  cardinfo->CID.ProdSN |= CID_Tab[11] << 8;
  /* Byte 12 */
  cardinfo->CID.ProdSN |= CID_Tab[12];
  /* Byte 13 */
  cardinfo->CID.Reserved1 |= (CID_Tab[13] & 0xF0) >> 4;
  /* Byte 14 */
  cardinfo->CID.ManufactDate = (CID_Tab[13] & 0x0F) << 8;
  /* Byte 15 */
  cardinfo->CID.ManufactDate |= CID_Tab[14];
  /* Byte 16 */
  cardinfo->CID.CID_CRC = (CID_Tab[15] & 0xFE) >> 1;
  cardinfo->CID.Reserved2 = 1;

  return 0;  
}

/*******************************************************************************
* Function Name  : _read_buffer
* Description    : None
* Input          : - *buff:
*				   - len:
*				   - release:
* Output         : None
* Return         : 0��NO_ERR; TRUE: Error
* Attention		 : None
*******************************************************************************/
int _read_buffer(uint8_t *buff, uint16_t len, uint8_t release)
{
  uint8_t r1;
  uint16_t retry;

  /* Card enable, Prepare to read	*/
  _card_enable();

  /* Wait start-token 0xFE */
  for(retry=0; retry<2000; retry++)
  {
	 r1 = _spi_read_write(DUMMY_BYTE);
	 if(r1 == 0xFE)
	 {
		 retry = 0;
		 break;
	 }
  }

  /* Timeout return	*/
  if(retry == 2000)
  {
	 _card_disable();
	 return 1;
  }

  /* Start reading */
  for(retry=0; retry<len; retry++)
  {
     *(buff+retry) = _spi_read_write(DUMMY_BYTE);
  }

  /* 2bytes dummy CRC */
  _spi_read_write(DUMMY_BYTE);
  _spi_read_write(DUMMY_BYTE);

  /* chip disable and dummy byte */ 
  if(release)
  {
	 _card_disable();
	 _spi_read_write(DUMMY_BYTE);
  }

  return 0;
}

/*******************************************************************************
* Function Name  : MSD_ReadSingleBlock
* Description    : None
* Input          : - sector:
*				   - buffer:
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
int MSD_ReadSingleBlock(uint32_t sector, uint8_t *buffer)
{
  uint8_t r1;

  /* if ver = SD2.0 HC, sector need <<9 */
  if(CardInfo.CardType != CARDTYPE_SDV2HC)
  {
	 sector = sector<<9;
  }
	
  /* Send CMD17 : Read single block command */
  r1 = _send_command(CMD17, sector, 0);
	
  if(r1 != 0x00)
  {
	 return 1;
  }
	
  /* Start read and return the result */
  r1 = _read_buffer(buffer, MSD_BLOCKSIZE, RELEASE);

  /* Send stop data transmit command - CMD12 */
  _send_command(CMD12, 0, 0);

  return r1;
}

/*******************************************************************************
* Function Name  : MSD_ReadMultiBlock
* Description    : None
* Input          : - sector:
*				   - buffer:
*                  - NbrOfSector:
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
int MSD_ReadMultiBlock(uint32_t sector, uint8_t *buffer, uint32_t NbrOfSector)
{
  uint8_t r1;
  uint32_t i;

  /* if ver = SD2.0 HC, sector need <<9 */
  if(CardInfo.CardType != CARDTYPE_SDV2HC)
  {
	 sector = sector<<9;
  }

  /* Send CMD18 : Read multi block command */
  r1 = _send_command(CMD18, sector, 0);
  if(r1 != 0x00)
  {
     return 1;
  }

  /* Start read	*/
  for(i=0; i<NbrOfSector; i++)
  {
     if(_read_buffer(buffer+i*MSD_BLOCKSIZE, MSD_BLOCKSIZE, HOLD))
     {
		 /* Send stop data transmit command - CMD12	*/
		 _send_command(CMD12, 0, 0);
		 /* chip disable and dummy byte */
		 _card_disable();
		 return 2;
     }
  }
	
  /* Send stop data transmit command - CMD12 */
  _send_command(CMD12, 0, 0);

  /* chip disable and dummy byte */
  _card_disable();
  _spi_read_write(DUMMY_BYTE);
	
  return 0;
}

/*******************************************************************************
* Function Name  : MSD_WriteSingleBlock
* Description    : None
* Input          : - sector:
*				   - buffer:
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
int MSD_WriteSingleBlock(uint32_t sector, const uint8_t *buffer)
{
  uint8_t r1;
  uint16_t i;
  uint32_t retry;

  /* if ver = SD2.0 HC, sector need <<9 */
  if(CardInfo.CardType != CARDTYPE_SDV2HC)
  {
	 sector = sector<<9;
  }
	
  /* Send CMD24 : Write single block command */
  r1 = _send_command(CMD24, sector, 0);
	
  if(r1 != 0x00)
  {
	 return 1;
  }

  /* Card enable, Prepare to write */
  _card_enable();
  _spi_read_write(DUMMY_BYTE);
  _spi_read_write(DUMMY_BYTE);
  _spi_read_write(DUMMY_BYTE);
  /* Start data write token: 0xFE */
  _spi_read_write(0xFE);
	
  /* Start single block write the data buffer */
  for(i=0; i<MSD_BLOCKSIZE; i++)
  {
    _spi_read_write(*buffer++);
  }

  /* 2Bytes dummy CRC */
  _spi_read_write(DUMMY_BYTE);
  _spi_read_write(DUMMY_BYTE);
	
  /* MSD card accept the data */
  r1 = _spi_read_write(DUMMY_BYTE);
  if((r1&0x1F) != 0x05)
  {
    _card_disable();
    return 2;
  }
	
  /* Wait all the data programm finished */
  retry = 0;
  while(_spi_read_write(DUMMY_BYTE) == 0x00)
  {	
	 /* Timeout return */
	 if(retry++ == 0x40000)
	 {
	    _card_disable();
	    return 3;
	 }
  }

  /* chip disable and dummy byte */ 
  _card_disable();
  _spi_read_write(DUMMY_BYTE);
	
  return 0;
}

/*******************************************************************************
* Function Name  : MSD_WriteMultiBlock
* Description    : None
* Input          : - sector:
*				   - buffer:
*                  - NbrOfSector:
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
int MSD_WriteMultiBlock(uint32_t sector, const uint8_t *buffer, uint32_t NbrOfSector)
{
  uint8_t r1;
  uint16_t i;
  uint32_t n;
  uint32_t retry;

  /* if ver = SD2.0 HC, sector need <<9 */
  if(CardInfo.CardType != CARDTYPE_SDV2HC)
  {
	  sector = sector<<9;
  }

  /* Send command ACMD23 berfore multi write if is not a MMC card */
  if(CardInfo.CardType != CARDTYPE_MMC)
  {
	  _send_command(ACMD23, NbrOfSector, 0x00);
  }
	
  /* Send CMD25 : Write nulti block command	*/
  r1 = _send_command(CMD25, sector, 0);
	
  if(r1 != 0x00)
  {
	  return 1;
  }

  /* Card enable, Prepare to write */
  _card_enable();
  _spi_read_write(DUMMY_BYTE);
  _spi_read_write(DUMMY_BYTE);
  _spi_read_write(DUMMY_BYTE);

  for(n=0; n<NbrOfSector; n++)
  {	
	 /* Start multi block write token: 0xFC */
	 _spi_read_write(0xFC);

	 for(i=0; i<MSD_BLOCKSIZE; i++)
	 {
		_spi_read_write(*buffer++);
	 }	

	 /* 2Bytes dummy CRC */
	 _spi_read_write(DUMMY_BYTE);
	 _spi_read_write(DUMMY_BYTE);

	 /* MSD card accept the data */
	 r1 = _spi_read_write(DUMMY_BYTE);
	 if((r1&0x1F) != 0x05)
	 {
	    _card_disable();
	    return 2;
	 }

	 /* Wait all the data programm finished	*/
	 retry = 0;
	 while(_spi_read_write(DUMMY_BYTE) != 0xFF)
	 {	
		/* Timeout return */
		if(retry++ == 0x40000)
		{
		   _card_disable();
		   return 3;
		}
	 }
  }

  /* Send end of transmit token: 0xFD */
  r1 = _spi_read_write(0xFD);
  if(r1 == 0x00)
  {
	 return 4;
  }

  /* Wait all the data programm finished */
  retry = 0;
  while(_spi_read_write(DUMMY_BYTE) != 0xFF)
  {	
	 /* Timeout return */
	 if(retry++ == 0x40000)
	 {
	     _card_disable();
	     return 5;
	 }
  }

  /* chip disable and dummy byte */
  _card_disable();
  _spi_read_write(DUMMY_BYTE);

  return 0;
}


/*******************************************************************************
* Function Name  : _send_command
* Description    : None
* Input          : - cmd:
*				   - arg:
*                  - crc:
* Output         : None
* Return         : R1 value, response from card
* Attention		 : None
*******************************************************************************/
int _send_command(uint8_t cmd, uint32_t arg, uint8_t crc)
{
  uint8_t r1;
  uint8_t retry;

  /* Dummy byte and chip enable */
  _spi_read_write(DUMMY_BYTE);
  _card_enable();

  /* Command, argument and crc */
  _spi_read_write(cmd | 0x40);
  _spi_read_write(arg >> 24);
  _spi_read_write(arg >> 16);
  _spi_read_write(arg >> 8);
  _spi_read_write(arg);
  _spi_read_write(crc);
  
  /* Wait response, quit till timeout */
  for(retry=0; retry<200; retry++)
  {
	 r1 = _spi_read_write(DUMMY_BYTE);
	 if(r1 != 0xFF)
	 {
		 break;
	 }
  }

  /* Chip disable and dummy byte */ 
  _card_disable();
  _spi_read_write(DUMMY_BYTE);

  return r1;
}	

/*******************************************************************************
* Function Name  : _send_command_hold
* Description    : None
* Input          : - cmd:
*				   - arg:
*                  - crc:
* Output         : None
* Return         : R1 value, response from card
* Attention		 : None
*******************************************************************************/
int _send_command_hold(uint8_t cmd, uint32_t arg, uint8_t crc)
{
  uint8_t r1;
  uint8_t retry;

  /* Dummy byte and chip enable */
  _spi_read_write(DUMMY_BYTE);
  _card_enable();

  /* Command, argument and crc */
  _spi_read_write(cmd | 0x40);
  _spi_read_write(arg >> 24);
  _spi_read_write(arg >> 16);
  _spi_read_write(arg >> 8);
  _spi_read_write(arg);
  _spi_read_write(crc);
  
  /* Wait response, quit till timeout */
  for(retry=0; retry<200; retry++)
  {
	 r1 = _spi_read_write(DUMMY_BYTE);
	 if(r1 != 0xFF)
	 {
		 break;
	 }
  }

  return r1;
}

