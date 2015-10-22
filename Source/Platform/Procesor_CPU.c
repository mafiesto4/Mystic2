//////////////////////////////////////////////////////////////////////////////////////
// Mystic OS Copyright 2012-2013 (c) Wojciech Figat
// The software is provided 'as is'. 
// The original author is not responsible for any harm which may result from use of this software or its source code
// See also my 3d game engine --> http://celelej.com
//////////////////////////////////////////////////////////////////////////////////////


#include "..\Kernel\Config.h"
#include "..\Kernel\Types.h"
#include "Procesor_CPU.h"


#if OS_DEST_PLATFORM == OS_LPC17xx

	// LPC17xx
	#define OS_DEST_PLATFORM_NAME "LPC17xx"

	#include "..\Platform\LPC17xx\LPC17xx.h"
	#include "..\Platform\LPC17xx\lpc17xx_clkpwr.h"

	extern volatile unsigned int _stack;
	extern uint32_t SystemCoreClock;

	#if OS_DEST_CPU	== OS_CortexM3

		// Delay cpu on number of miliseconds
		void OS_CPU_Delay(Word mili)
		{
			Word m=50000000;
			for(; mili != 0; mili--)
				for(; m != 0; m--);
		}

		// Enable External Interrupt
		Inline void OS_CPU_Enable_IRQ(void)
		{

		}

		// Disable External Interrupt
		Inline void OS_CPU_Disable_IRQ(void)
		{

		}

		// Restart cpu
		void OS_CPU_Restart(void)
		{
			NVIC_SystemReset();
		}

		// Turn off cpu
		void OS_CPU_TurnOff(void)
		{
			//CLKPWR_Sleep();
			//CLKPWR_DeepSleep();
			//CLKPWR_PowerDown();
			CLKPWR_DeepPowerDown();
		}


		/*
		#define DEBUG_OUTPUT_BUFFER_SIZE 82
		#define DEBUG_INPUT_BUFFER_SIZE 22

		extern unsigned char *__heaps;
		extern const unsigned long _vStackTop;
		extern const unsigned char _pvHeapStart;
		const unsigned long stackinitconst = 0xDEADBEEF;

		#define StackHeapSize() (((unsigned long)&_vStackTop) - ((unsigned long)&_pvHeapStart))



		void StackHeapInit(void)
		{
			register unsigned long *StackPointer Asm("sp");
			unsigned long *StackLimit = (unsigned long *)&_pvHeapStart;

			while(StackLimit < StackPointer)
				*(StackLimit++) = stackinitconst;
		}

		unsigned long StackHeapFree(void)
		{
			register unsigned long *StackPointer Asm("sp");
			unsigned long *StackLimit;

		#if defined (__REDLIB__)
			if(!__heaps)
		#endif
				StackLimit = (unsigned long *)&_pvHeapStart;
		#if defined (__REDLIB__)
			else
				StackLimit = (unsigned long *)__heaps;

			if((unsigned long)StackLimit&0x3)
			{
				StackLimit += 4 - (((unsigned long)StackLimit)&0x3);
			}
		#endif

			return (StackPointer - StackLimit) * 4;
		}

		unsigned long StackHeapMinFree(void)
		{
			unsigned long UnusedCount = 0, MaxUnusedCount = 0;
			register unsigned long *StackPointer Asm("sp");
			unsigned long *StackLimit;

		#if defined (__REDLIB__)
			if(!__heaps)
		#endif
				StackLimit = (unsigned long *)&_pvHeapStart;
		#if defined (__REDLIB__)
			else
				StackLimit = (unsigned long *)__heaps;

			if((unsigned long)StackLimit&0x3)
			{
				StackLimit += 4 - (((unsigned long)StackLimit)&0x3);
			}
		#endif

			while(StackLimit < StackPointer)
			{
				if((*StackLimit) == stackinitconst)
				{
					UnusedCount++;
				} else
				{
					if(UnusedCount > MaxUnusedCount)
						MaxUnusedCount = UnusedCount;
					UnusedCount = 0;
				}
				StackLimit++;
			}
			if(UnusedCount > MaxUnusedCount)
				MaxUnusedCount = UnusedCount;
			return MaxUnusedCount*4;
		}
		*/
		// Get informations about current cpu
		void OS_CPU_GetInfo(CPU_Info *info)
		{
			info->Platform = OS_DEST_PLATFORM;
			info->Cpu = OS_DEST_CPU;
			info->Clock = SystemCoreClock;
			info->SystemMemory = 0;
			info->FreeMemory = 0;
			info->SystemStackSize = _stack;	
		}

	#else

		#error "Invalid destination platform cpu!"

	#endif /* OS_DEST_CPU */

#else

	#error "Invalid destination platform!"

#endif /* OS_DEST_PLATFORM */

