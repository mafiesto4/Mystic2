//////////////////////////////////////////////////////////////////////////////////////
// Mystic OS Copyright 2012-2013 (c) Wojciech Figat
// The software is provided 'as is'. 
// The original author is not responsible for any harm which may result from use of this software or its source code
// See also my 3d game engine --> http://celelej.com
//////////////////////////////////////////////////////////////////////////////////////

#ifndef __OS_PROCESSOR_CPU_H__
#define __OS_PROCESSOR_CPU_H__

#include "..\Kernel\Config.h"
#include "..\Kernel\Types.h"

#if OS_DEST_PLATFORM == OS_LPC17xx

	// LPC17xx
	#define OS_DEST_PLATFORM_NAME "LPC17xx"

	#include "..\Platform\LPC17xx\LPC17xx.h"

#if OS_DEST_CPU	== OS_CortexM3

	// Cortex M3
	#define OS_DEST_CPU_NAME "Cortex M3"

	// GPIO (General Purpose Input/Output)
	#define OS_CPU_PIN_SET(x)			(LPC_GPIO0->FIOSET = x);
	#define OS_CPU_PIN_CLR(x)			(LPC_GPIO0->FIOCLR = x);
	#define OS_CPU_PIN_SWITCH(x,y)		((x) ? (LPC_GPIO0->FIOSET = y) : (LPC_GPIO0->FIOCLR = y));

#else

	#error "Invalid destination platform cpu!"

#endif /* OS_DEST_CPU */

#else

	#error "Invalid destination platform!"

#endif /* OS_DEST_PLATFORM */

// CPU_Info
typedef struct CPU_Info
{

	Byte Platform;										// Platform Code
	Byte Cpu;											// Cpu Code
	Word Clock;											// Cpu clock speed in Hz
	Word SystemMemory;									// Whole system memory size in bytes
	Word FreeMemory;									// Free system memory size in bytes
	Word SystemStackSize;								// System stack size in bytes

} CPU_Info;


// Delay cpu on number of miliseconds
void OS_CPU_Delay(Word mili);

// Enable External Interrupt
Inline void OS_CPU_Enable_IRQ(void);

// Disable External Interrupt
Inline void OS_CPU_Disable_IRQ(void);

// Restart cpu
void OS_CPU_Restart(void);

// Turn off cpu
void OS_CPU_TurnOff(void);

// Get informations about current cpu
void OS_CPU_GetInfo(CPU_Info *info);

#endif /* __OS_PROCESSOR_CPU_H__ */
