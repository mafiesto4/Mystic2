//////////////////////////////////////////////////////////////////////////////////////
// Mystic OS Copyright 2012-2013 (c) Wojciech Figat
// The software is provided 'as is'. 
// The original author is not responsible for any harm which may result from use of this software or its source code
// See also my 3d game engine --> http://celelej.com
//////////////////////////////////////////////////////////////////////////////////////

// Include
#include "Kernel.h"
#include "Config.h"
		
#if OS_UART_ENABLE

	#if OS_DEST_PLATFORM == OS_LPC17xx
			
		// Include
		#include "..\Platform\LPC17xx\lpc17xx_uart.h"
			
	#else

		#error "This platform does not support UART"
		
	#endif
	
	void OS_UART_Init(void)
	{
		// UART for Cortex M3
		#if OS_DEST_PLATFORM == OS_LPC17xx
			
			PINSEL_CFG_Type PinCfg;
			uint32_t  Fdiv;
			
			PinCfg.Funcnum = 1;
			PinCfg.OpenDrain = 0;
			PinCfg.Pinmode = 0;
			PinCfg.Portnum = 0;
			
			if ( OS_UART_PORT == LPC_UART0 )
				PinCfg.Pinnum = 2;
			if( OS_UART_PORT == LPC_UART2)
				PinCfg.Pinnum = 10;
			
			PINSEL_ConfigPin(&PinCfg);
			
			if (OS_UART_PORT == LPC_UART0)
				PinCfg.Pinnum = 3;
			if (OS_UART_PORT == LPC_UART2)
				PinCfg.Pinnum = 11;
			
			PINSEL_ConfigPin(&PinCfg);
			
			LPC_SC->PCONP = LPC_SC->PCONP|(1<<24);	  
			
			// 8 bits, no Parity, 1 Stop bit
			OS_UART_PORT->LCR = 0x83;//0x83 or 131 or 1000_0011	              
			
			#define FOSC 12000000           
			
			#define FCCLK (FOSC  * 8)              

			#define FPCLK (FCCLK / 4)               
			
			//Baud Rate
			Fdiv = ( FPCLK / 16 ) / OS_UART_BAUDRATE ;
			
			OS_UART_PORT->DLM = Fdiv / 256;		
			
			OS_UART_PORT->DLL = Fdiv % 256;
			
			//DLAB = 0
			OS_UART_PORT->LCR = 0x03;
			
			//Enable and reset TX and RX FIFO.
			OS_UART_PORT->FCR = 0x07;
			
		#else
		
			#error "This platform does not support UART"
			
		#endif
	}
	
	uint8_t OS_UART_GetByte(void)
	{
		#if OS_DEST_PLATFORM == OS_LPC17xx
		
			uint8_t tmp = 0;
			//UART_Receive(OS_UART_PORT, &tmp, 1, BLOCKING);
			tmp = UART_ReceiveByte(OS_UART_PORT);
			return tmp;
		
		#else
		
			#error "This platform does not support UART"
		
		#endif
	}
	
	uint32_t OS_UART_GetBlock(uint8_t* buf, uint8_t size)
	{
		#if OS_DEST_PLATFORM == OS_LPC17xx
		
			return UART_Receive(OS_UART_PORT, buf, size, BLOCKING);
		
		#else
		
			#error "This platform does not support UART"
		
		#endif
	}
	
#endif
	
PUTCHAR_PROTOTYPE
{
	#if (OS_DEST_PLATFORM == OS_LPC17xx && OS_UART_ENABLE == 1)
			
		// Wait for current transmission complete - THR must be empty
		while (UART_CheckBusy(OS_UART_PORT) == SET);
				
		UART_SendByte(OS_UART_PORT, ch );
			
	#endif
			
	return ch;
}
