//////////////////////////////////////////////////////////////////////////////////////
// Mystic OS Copyright 2012-2013 (c) Wojciech Figat
// The software is provided 'as is'. 
// The original author is not responsible for any harm which may result from use of this software or its source code
// See also my 3d game engine --> http://celelej.com
//////////////////////////////////////////////////////////////////////////////////////

#ifndef __OS_CONFIG_H__
#define __OS_CONFIG_H__

//////////////////////////////////////////////////////////////////////////////////////
// Destination Platform
/////////////////////////////////////////////////////////////////////////////////////


// Possible destination platforms
// If You want to add Your own, please contact with me for more help: mafiesto4@wp.pl
#define OS_x86 			0       	// 0 - x86 :-)
#define OS_LPC17xx 		1 		    // 1 - LPC17xx
//Working on it...#define OS_ArduinoUno	2			// 2 - Arduino Uno
//Working on it...#define OS_ArduinoMega 3     // 3 - Arduino Mega 2560)


// Supported Processors
#define OS_CoreI7		0			// 0 - Just joke :-)
#define OS_CortexM3		1			// 1 - Cortex M3
//Working on it...#define OS_CortexM4		2			// 2 - Cortex M4
//Working on it...#define OS_ATmega328	3			// 3 - ATmega328
//Working on it...#define OS_ATmega2560		4			// 4 - ATmega2560


// Destination Platform
#define OS_DEST_PLATFORM	OS_LPC17xx
#define OS_DEST_CPU			OS_CortexM3


//////////////////////////////////////////////////////////////////////////////////////
// Debug Mode
/////////////////////////////////////////////////////////////////////////////////////
// 0 - No Bebug Mode
// 1 - Enable Debug Mode
#define OS_DEBUG 1


// Process execution debugging level:
// 0 - No debuging during process execution (but sending errors to kernel)
// 1 - Debug only important commands and errors
// 2 - Debug most important commands and interrupts
// 3 - Debug all command and interrupts (Be carefull!! May send a millions of data per second so Your debug terminal may crash)
// Note: Remember that final code size may be much more bigger if You use debugging
#define OS_DEBUG_PROCESS 0


// Debug SD card driver
#define OS_DEBUG_SD 1


//////////////////////////////////////////////////////////////////////////////////////
// UART - Universal Asynchronous Receiver and Transmitter
// 8 bits, no Parity, 1 Stop bit
// Custom baudrate
/////////////////////////////////////////////////////////////////////////////////////
// UART is supported for:
// Cortex M3 - UART0 or UART1


/////////////////////////////////////////////////////////////////////////////////////
// 0 - Turn OFF UART Comunication (NO DEBUG)
// 1 - Enable UART Comunication
#define OS_UART_ENABLE 1

// Baud Rate (bits per second)
#define OS_UART_BAUDRATE 115200

#if OS_DEST_PLATFORM == OS_LPC17xx
	// LPC_UART0 - Use port UART0 for comunication
	// LPC_UART2 - Use port UART2 for comunication
	#define OS_UART_PORT LPC_UART2
#endif


//////////////////////////////////////////////////////////////////////////////////////
// Graphics Device
/////////////////////////////////////////////////////////////////////////////////////

// Enable Graphics Device or disable it
#define OS_GPU_ENABLE 1



// LCD Control pins for:
// Cortex M3: EN = P0.19 , LE = P0.20 , DIR = P0.21 , CS = P0.22 , RS = P0.23 , WR = P0.24 , RD = P0.25 , DB[0.7] = P2.0...P2.7 , DB[8.15]= P2.0...P2.7


//////////////////////////////////////////////////////////////////////////////////////
// SPI
/////////////////////////////////////////////////////////////////////////////////////
// SPI pins for:
// Cortex M3: P3.25 - SD_CD - used as GPIO, P1.20 - SCK, P1.21 - SD_CS - used as GPIO, P1.23 - MISO, P1.24 - MOSI, P3.26 - SD_PWR - used as GPIO
/////////////////////////////////////////////////////////////////////////////////////




//////////////////////////////////////////////////////////////////////////////////////
// DO NOT EDIT THIS PART OF THE FILE!
/////////////////////////////////////////////////////////////////////////////////////


#if (OS_DEST_PLATFORM == 0 || OS_DEST_PLATFORM > 2)
	//Check Destination Platform
	#error "Invalid destination platform"
#endif
#if (OS_DEBUG_SD == 1 && OS_DEBUG == 0)
	#error "Cannot debug SD card without Debugging Mode"
#endif
#if (OS_DEBUG == 1 && OS_UART_ENABLE == 0)
	//Cannot debug and turn off uart
	#error "Cannot use Debug Mode without UART"
#endif
#if (OS_UART_ENABLE == 1 && OS_DEST_PLATFORM != OS_LPC17xx)
	//UART not supported
	#error "This platform does not support UART"
#endif


#endif /* __OS_CONFIG_H__ */
