//////////////////////////////////////////////////////////////////////////////////////
// Mystic OS Copyright 2012-2013 (c) Wojciech Figat
// The software is provided 'as is'. 
// The original author is not responsible for any harm which may result from use of this software or its source code
// See also my 3d game engine --> http://celelej.com
//////////////////////////////////////////////////////////////////////////////////////


// Includes
#include "Types.h"
#include "Process.h"
#include "Kernel.h"
#include "Utilities.h"

// Processes
extern List Processes;

// Returns first free PID, or 0 when thre is no more free PIDs
static Byte GetFirstFreePID(void)
{
	// Data
	Byte i, pid = 1, can;
	Process *wsk;
	
	// Check if there are any free handles
	if(Processes.Count == 0)
	{
		// First process
		return 1;
	}
	
	// Check all possible PIDs
	for( ; pid <= 255; pid++)
	{
		// Check if pid is free
		can = 1;
		for(i = 0; i < Processes.Count; i++)
		{
			// Get process at i
			wsk = (Process*)OS_List_GetAt(&Processes, i);
			
			// Check if PIS is used
			if(wsk->PID == pid)
			{
				can = 0;
				break;
			}
		}
		
		if(can)
		{
			return pid;
		}
	}
	
	// Error
	return Nothing;
}

// Create new process with name, file pointer, flags, priority and two parametrs( Param1 and Param2 will be saved in eax and ebx)
Process* OS_Proc_Create(Char *Name, File *File, Byte Flag, Byte Priotity, Word Param1, Word Param2)
{
	// Create data
	Process *Proc;
	
	// Allocate memory for new process
	Proc = (Process*)OS_Allocate(sizeof(Process));
	
	// Set up
	Proc->Status = OS_PROC_SUSPENDED;
	Proc->Flag = Flag;
	Proc->Priotity = Priotity;
	OS_UTILS_MemoryCopy(Proc->Name, Name, OS_PROC_NameLength);
	Proc->Name[OS_PROC_NameLength-1] = 0;
	Proc->Reg[0] = Param1;
	Proc->Reg[1] = Param2;
	Proc->File = File;
	Proc->Current = (Block*)Null;
	OS_List_Init(&Proc->Blocks);
	Proc->PID = GetFirstFreePID();
	Proc->Graphics = Null;
	Proc->StartTime = 0;
	
	// Check if PID is not zero
	if(Proc->PID == Nothing)
	{
		// Error
		return Null;
	}
	
	// Add to the list
	OS_List_Add(&Processes, Proc);
	
	#if OS_DEBUG == 1
		OS_Debug_Print("New process created. PID=%d, Name=%s",Proc->PID, Proc->Name);
	#endif
	
	// Return pointner to that process
	return Proc;
}

// Terminate Process
void OS_Proc_Terminate(Process *Proc)
{
	// Data
	Block *wsk = (Block*)OS_List_GetLast(&Proc->Blocks);
	
	#if OS_DEBUG == 1
		OS_Debug_Print("Killing process %s, PID=%d", Proc->Name, Proc->PID);
	#endif
	
	// Remove process from list
	OS_List_Remove(&Processes, Proc);
	
	// Clean blocks
	while(wsk != Null)
	{
		// Clean block data
		free(wsk->Data);
		
		// Remove block from list
		OS_List_RemoveLast(&Proc->Blocks);
		
		// Get last block
		wsk = (Block*)OS_List_GetLast(&Proc->Blocks);
		
	}
	
	// Clean memory
	Proc->Status = OS_PROC_TERMINATED;
	free(Proc);
}

// Suspend Process
void OS_Proc_Suspend(Process *Proc)
{
	Proc->Status = OS_PROC_SUSPENDED;
}

// Run Process if was sleeping or suspended
void OS_Proc_Run(Process *Proc)
{
	if(Proc->Status == OS_PROC_SLEEPING || Proc->Status == OS_PROC_SUSPENDED)
	{
		// Make it ready!
		Proc->Status = OS_PROC_READY;
	}
}

// Sleep Process
void OS_Proc_Sleep(Process *Proc, Word Ticks)
{
	///////////////////TODO Sleeping processes
}

// Returns process with given PID, if process does not exists returns Null
Process* OS_Proc_GetProcessByPID(Byte PID)
{
	// Data	
	Byte i = 0;
	Process* wsk;
	
	// Find
	for( ; i < Processes.Count; i++)
	{
		// Get process at i
		wsk = (Process*)OS_List_GetAt(&Processes, i);
		
		// Compare PID
		if(PID == wsk->PID)
		{
			// Found
			return wsk;
		}
		
	}
	
	// Nothing
	return Null;
}

// Returns process with given name, if process does not exists returns Null
Process* OS_Proc_GetProcessByName(Char Name[OS_PROC_NameLength])
{
	// Data	
	Byte i = 0;
	Process* wsk;
	
	// Find
	for( ; i < Processes.Count; i++)
	{
		// Get process at i
		wsk = (Process*)OS_List_GetAt(&Processes, i);
		
		// Compare Names
		if(OS_UTILS_MemoryCompare(Name, wsk->Name, OS_PROC_NameLength) == 0)
		{
			// Found
			return wsk;
		}
	}
	
	// Nothing
	return Null;
}

// Get current process private memory usage
Word OS_Proc_GetMemoryUsage(Process *Proc)
{
	// Data
	Word i, res = 0;
	Block *b;
	
	// Check if is good
	if(Proc != Null)
	{
		// Static usage
		res = sizeof(Process);
		
		// Count all blocks
		for(i = 0; i < Proc->Blocks.Count; i++)
		{
			// Cache block
			b = (Block*)OS_List_GetAt(&Proc->Blocks, i);
			
			// Data usage by block
			res += b->Size;
		}
	}
	
	// Return
	return res;
}
