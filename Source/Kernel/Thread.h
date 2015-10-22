//////////////////////////////////////////////////////////////////////////////////////
// Mystic OS Copyright 2012-2013 (c) Wojciech Figat
// The software is provided 'as is'. 
// The original author is not responsible for any harm which may result from use of this software or its source code
// See also my 3d game engine --> http://celelej.com
//////////////////////////////////////////////////////////////////////////////////////

#ifndef __OS_THREAD_H__
#define __OS_THREAD_H__

// Include
#include "Time.h"
#include "..\Graphics\Graphics.h"

// Config
#define OS_PROC_NameLength 		16 		// Default process name length. Last char must be zero. (changing it on higher value may occur errors)
#define OS_PROC_SpeedFactor		2		// Bigger it is, more 'ticks' will do every process during system tick

// Process Status
#define	OS_PROC_READY			5		// Process is ready to run
#define	OS_PROC_RUNNING			4		// Process is running
#define	OS_PROC_SLEEPING		3		// Process is sleeping
#define	OS_PROC_SUSPENDED		2		// Process is suspended
#define	OS_PROC_TERMINATING		1		// Process is terminating or is about to terminate
#define	OS_PROC_TERMINATED		0		// Process was terminated

// Proces Priority
#define OS_PROC_High			8		// Very fast and very good for games
#define OS_PROC_AboveNormal		5		// A little bit faster than normal
#define OS_PROC_Normal			4		// Normal spped and priority, best for most apps
#define OS_PROC_BelowNormal		3		// A little bit slower than normal
#define OS_PROC_Low				2		// Process is running in background with slow speed
#define OS_PROC_Lowest			1		// The lowest

// Process flag
#define	OS_PROC_NONE            0       // None
#define	OS_PROC_NEEDPERM        1       // Needs permission to do special operations like modify system options
#define	OS_PROC_SHELL           2       // System Shell Process
#define	OS_PROC_APP             4       // Process parent is application
#define	OS_PROC_DRIVER          8       // Process parent is driver

// Normal application permissions
#define OS_PROC_NORMALPERM ( OS_PROC_NEEDPERM | OS_PROC_APP )

// System application permissions
#define OS_PROC_SYSTEMPERM ( OS_PROC_SHELL )

// Default process priorty
#define OS_PROC_DefaultProcessPriroity OS_PROC_Normal

// Block
typedef struct Block 
{
	
	void *Data;							// Holds whole block data (all variables, both bss and data sections)
	Word Size;							// Informs system how much memory uses block
	Word FP;							// File Pointer, Last position in file (may be not valid if this block is currenty executed, used if process is going back to the prevoius block)

} Block;

// Process
typedef struct sProcess
{

	Byte Status;										// Process status
	Byte Flag;											// Process flag
	Byte PID;											// Process ID
	Byte Priotity;										// Process Priotity
	Char Name[OS_PROC_NameLength];						// Name of the Process
	Word Reg[12];					 					// All 12 registers, each 32 bit
	List Blocks;										// List of all Process blocks
	Block *Current;										// Current block
	PackedTime StartTime;								// Time when process started (Packed version)
	File *File;											// Pointer to file that program will read from
	GraphicsHandle *Graphics;							// Graphics Handle, Null if Process does not have own
	
} Process;

// Create new process with name, file pointer, flags, priority and two parametrs( Param1 and Param2 will be saved in eax and ebx)
Process* OS_Proc_Create(Char *Name, File *File, Byte Flag, Byte Priotity, Word Param1, Word Param2);

// Terminate Process
void OS_Proc_Terminate(Process *Proc);

// Suspend Process
void OS_Proc_Suspend(Process *Proc);

// Run Process if was sleeping or suspended
void OS_Proc_Run(Process *Proc);

// Sleep Process
void OS_Proc_Sleep(Process *Proc, Word Ticks);

// Returns process with given PID, if process does not exists returns Null
Process* OS_Proc_GetProcessByPID(Byte PID);

// Returns process with given name, if process does not exists returns Null
Process* OS_Proc_GetProcessByName(Char Name[OS_PROC_NameLength]);

// Count processes
Byte OS_Proc_Count(void);

// Get current process private memory usage
Word OS_Proc_GetMemoryUsage(Process *Proc);

#endif
