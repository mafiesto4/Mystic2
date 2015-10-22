//////////////////////////////////////////////////////////////////////////////////////
// Mystic OS Copyright 2012-2013 (c) Wojciech Figat
// The software is provided 'as is'. 
// The original author is not responsible for any harm which may result from use of this software or its source code
// See also my 3d game engine --> http://celelej.com
//////////////////////////////////////////////////////////////////////////////////////


#ifndef __OS_KERNEL_H__
#define __OS_KERNEL_H__


// Load configuration and types
#include "Types.h"
#include "Config.h"
#ifndef __OS_CONFIG_H__
	#error "OS configuration has not been loaded!"
#endif

#ifdef __GNUC__
	//With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf set to 'Yes') calls __io_putchar()
	#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
	asdadasd
#else
	#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif  /*__GNUC__ */

// General Initialization
void OS_Init(void);

// Start system and keep it running
void OS_Run(void);

// Close OS and release all resources. Turns off CPU.
void OS_Exit(void);

// Close OS, release all resources and restart OS
void OS_Restart(void);

// Allocate memory block that has given size
void* OS_Allocate(Word size);

// Executes given process
void OS_Execute(Process *exe);

// Executes system interrupt for given Process
void OS_Interrupt(Process *exe, Byte id);

// Get amount of free ram memory in bytes
Word OS_FreeRam(void);

// Show critical error
void OS_Error(Byte id, Word val);

// Start up new application
Byte OS_App_Open(Byte flags, const Char* path, Word argc, char argv[]);

// Kill applicaiotn
void OS_App_Kill(Application *app);

// Returns first free app handle (ID). It cannot be 0
Byte OS_App_GetFirstFreeID(void);

// Get amount of memnory in bytes that is used by given application
Word OS_App_MemoryUsage(Application *app);

// Returns application with given ID, if application does not exists returns Nothing
Application* OS_App_GetAppByID(Byte id);

// Check if process parent is application or driver and kills it, with process
void OS_KillProcessParent(Process *proc);

// Linked List functions
void OS_List_Init(List *list);
void OS_List_Add(List *list, void *data);
void OS_List_RemoveLast(List *list);
void OS_List_Clear(List *list);
void* OS_List_GetLast(List *list);
void* OS_List_GetAt(List *list, Word index);
Byte OS_List_Contains(List *list, void* data);
SWord OS_List_IndexOf(List *list, void* data);
//void OS_List_Insert(List *list, Word index, void* data);
//void OS_List_RemoveAt(List *list, Word index);
void OS_List_Remove(List *list, void* data);

// Debug Mode ON
#if OS_DEBUG == 1
	
	// Print line to output
	void OS_Debug_Print(const char * format, ...);
	
	// Process User Input
	void OS_Debug_Input(void);
	
	// Print fat informations
	void OS_Debug_FatInfo(void);
	
	// Print all Process registers
	void OS_Debug_PrintReg( Process *exe );
	
	// Print whole Process memory
	void OS_Debug_PrintMemory( Process *exe );

#endif

// UART Functions
#if OS_UART_ENABLE
	
	// Init
	void OS_UART_Init(void);
	
	// Get byte
	uint8_t OS_UART_GetByte(void);
	
	// Get Data Block
	uint32_t OS_UART_GetBlock( uint8_t* buf , uint8_t size);
	
#endif


#endif /* __OS_KERNEL_H__ */
