//////////////////////////////////////////////////////////////////////////////////////
// Mystic OS Copyright 2012-2013 (c) Wojciech Figat
// The software is provided 'as is'. 
// The original author is not responsible for any harm which may result from use of this software or its source code
// See also my 3d game engine --> http://celelej.com
//////////////////////////////////////////////////////////////////////////////////////

// Includes
#include "Kernel.h"
#include "Config.h"
#include "Process.h"
#include "FileSystem.h"
#include "..\Graphics\Graphics.h"
#include "..\Platform\Procesor_CPU.h"
#include "Utilities.h"

// Stardard C
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

// Current Graphics Handle
extern volatile GraphicsHandle *CGH;

static Inline Word Make_Word(Byte D3, Byte D2, Byte D1, Byte D0)
{
	Word Res = 0;
	
	*((Byte*)(&Res) + 3) = D0;
    *((Byte*)(&Res) + 2) = D1;
    *((Byte*)(&Res) + 1) = D2;
    *((Byte*)(&Res) + 0) = D3;
	
	return Res;
}

static Inline Word Make_Short(Byte D1, Byte D0)
{
	Short Res = 0;
	
    *((Byte*)(&Res) + 1) = D0;
    *((Byte*)(&Res) + 0) = D1;
	
	return Res;
}

static Inline Word Read_Short(File *file)
{
	Byte D[2];
	
	OS_FS_Read(file, D, 2);
	
	return Make_Short(D[0], D[1]);
}

static Inline Word Read_Word(File *file)
{
	Byte D[4];
	
	OS_FS_Read(file, D,  4);
	
	return Make_Word(D[0], D[1], D[2], D[3]);
}

// Executes given Process
void OS_Execute(Process *exe)
{
	// TODO
	// - optimalization
	// - safe mode
	// - error checking
	// - faster
	// - 16 bit offset adress in block memory
	// - global memory
	
	// Read command number
	exe->Reg[9] = OS_FS_Read(exe->File, &exe->Reg[8], 1);
	
	// Important note: Only at this time reading from file operation result is checked.
	if(exe->Reg[9] != FS_RESULT_OK)
	{
		//ERROR
		#if OS_DEBUG
			OS_Debug_Print("Cannot read from process file!");
		#endif
		
		// Terminate program
		OS_KillProcessParent(exe);
		return;
	}
	
	// DEBUG
	#if OS_DEBUG_PROCESS >= 3
		OS_Debug_Print("Command = %d" , exe->Reg[8] );
	#endif
	
	// Main switch
	switch(exe->Reg[8])
	{
		/////////////////////////////////////////////////////////////////////////////////////	
		// [Main section]
		/////////////////////////////////////////////////////////////////////////////////////
		
		// Error
		case 0: 
			
			//ERROR
			#if OS_DEBUG
				OS_Debug_Print("No more commands or data leak!");
			#endif
			
			// Terminate program
			OS_KillProcessParent(exe);
		
		break;
		
		// 5 bytes - start const32
		case 1:
			
			// Read size
			exe->Reg[8] = Read_Word(exe->File);
			
			// Allocate memory
			exe->Reg[9] = (Word)OS_Allocate(sizeof(Block));
			
			// Save memory usage
			((Block*)exe->Reg[9])->Size = exe->Reg[8];
			
			// Save block pointer
			((Block*)exe->Reg[9])->FP = exe->File->fptr;
			
			// When block allocates memory...
			if(exe->Reg[8] != 0)
			{
				// Allocate block private memory
				((Block*)exe->Reg[9])->Data = OS_Allocate(exe->Reg[8]);
			}
			else
			{
				// No memory usage
				((Block*)exe->Reg[9])->Data = Null;
			}
			
			// Add block to Process blocks list
			OS_List_Add(&exe->Blocks, (void*)exe->Reg[9]);
			
			// Set new block as current block
			exe->Current = ((Block*)exe->Reg[9]);
			
			//DEBUG
			#if OS_DEBUG_PROCESS >= 2
				OS_Debug_Print("Starting new block, Memory usage: %d bytes", exe->Reg[8]);
			#endif
			
		break;
			
		// 1 byte - ret
		case 2:
			
			// Free block private data
			free(exe->Current->Data);
			
			// Free block
			free(exe->Current);
			
			// Remove last block from collection
			OS_List_RemoveLast(&exe->Blocks);
			
			// Get last block from collection
			exe->Current = (Block*)OS_List_GetLast(&exe->Blocks);
			
			// If no blocks more kill it
			if(exe->Current == Null)
			{
				//DEBUG
				#if OS_DEBUG_PROCESS >= 1
					OS_Debug_Print("No more blocks, killing process parent");
				#endif
				
				// Kill process parent
				OS_KillProcessParent(exe);
			}
			else
			{
				// If there is more bloks just jump in file to last position of last block
				OS_FS_Seek(exe->File, exe->Current->FP);
				
				//DEBUG
				#if OS_DEBUG_PROCESS >= 3
					OS_Debug_Print("Returning to the previus block");
				#endif
			}
		
		break;
		
		// 2 bytes - int reg
		case 3:
			
			// Read reg
			OS_FS_Read(exe->File, &exe->Reg[8], 1);
		
			// Execute interrupt
			OS_Interrupt(exe, exe->Reg[exe->Reg[8]]);
		
		break;
		
		// 2 bytes - int const8
		case 4:
			
			// Read const8
			OS_FS_Read( exe->File, &exe->Reg[8], 1);
			
			// Execute interrupt
			OS_Interrupt(exe, exe->Reg[8]);
		
		break;
		
		// 9+n bytes- fillmem cont32 cont32 cont8[n]
		case 5:
			
			// Read offset
			exe->Reg[8] = Read_Word(exe->File);
			
			// Read size
			exe->Reg[9] = Read_Word(exe->File);
			
			// Fill memory
			OS_FS_Read(exe->File, (Byte*)exe->Current->Data + exe->Reg[8], exe->Reg[9]);
		
			//DEBUG
			#if OS_DEBUG_PROCESS >= 2
				OS_Debug_Print("Memory from %d, of size %d was set", exe->Reg[8], exe->Reg[9]);
			#endif
			
		break;
		
		// 5+n bytes- fillmem const32 cont8[n]
		case 6:
			
			// Read size
			exe->Reg[8] = Read_Word(exe->File);
		
			// Fill memory
			OS_FS_Read(exe->File, (Byte*)exe->Current->Data, exe->Reg[8]);
			
			//DEBUG
			#if OS_DEBUG_PROCESS >= 2
				OS_Debug_Print("Block memory at %d, of size %d was set", exe->Current->Data, exe->Reg[8]);
			#endif
			
		break;
		
		/////////////////////////////////////////////////////////////////////////////////////	
		// [Move section]
		/////////////////////////////////////////////////////////////////////////////////////
		
		// 3 bytes - mov reg reg
		case 20:
			
			// Read 1st register id
			OS_FS_Read(exe->File, &exe->Reg[8], 1);
			
			// Read 2nd register id
			OS_FS_Read(exe->File, &exe->Reg[9], 1);
			
			// Copy data
			exe->Reg[exe->Reg[8]] = exe->Reg[exe->Reg[9]];
			
			//DEBUG
			#if OS_DEBUG_PROCESS >= 3
				OS_Debug_Print("To register %d was moved %d from register %d", exe->Reg[8], exe->Reg[exe->Reg[8]], exe->Reg[9]);
			#endif
			
		break;
		/*
		// 3 bytes - mov !byte! reg [reg]
		case 21:
			
			// Read reg
			OS_FS_Read(exe->File, &exe->Reg[8], 1);
			
			// Read reg
			OS_FS_Read(exe->File, &exe->Reg[9], 1);
			
			// Copy data
			exe->Reg[exe->Reg[8]] = *(exe->Reg[exe->Reg[9]]);
			
			//DEBUG
			#if OS_DEBUG_PROCESS >= 3
				OS_Debug_Print("To register %d was moved %d from memory %d pointed by register %d", exe->Reg[8], exe->Reg[exe->Reg[8]], exe->Reg[exe->Reg[9]], exe->Reg[9]);
			#endif
			
		break;
		// 		21  - 3 - mov !byte! reg [reg] - przenosi 1 bajt danych z komorki o adresie zawartym w rejestrze 2 do rejestru 1
// 		22  - 3 - mov short reg [reg] - przenosi 2 bajty danych z komorki o adresie zawartym w rejestrze 2 do rejestru 1
// 		23  - 3 - mov word reg [reg] - przenosi 4 bajty danych z komorki o adresie zawartym w rejestrze 2 do rejestru 1

// 		24  - 3 - mov !byte! [reg] reg - przenosi 1 bajt danych z rejestru do komorki o adresie zawartym w rejestrze
// 		25  - 3 - mov short [reg] reg - przenosi 2 bajty danych z rejestru do komorki o adresie zawartym w rejestrze
// 		26  - 3 - mov word [reg] reg - przenosi 4 bajty danych z rejestru do komorki o adresie zawartym w rejestrze
		*/
		
		//  6 bytes - mov !byte! reg mem
		case 27:
			
			// Read register id
			OS_FS_Read( exe->File , &exe->Reg[8] , 1 );
			
			// Read mem
			exe->Reg[9] = Read_Word( exe->File );
			
			// Write data to reg from adress
			exe->Reg[exe->Reg[8]] = *((Byte*)exe->Current->Data + exe->Reg[9]);
			
			//DEBUG
			#if OS_DEBUG_PROCESS >= 3
				OS_Debug_Print("Register %d set %d from memory %d that has block offset %d", exe->Reg[8], exe->Reg[exe->Reg[8]], (Byte*)exe->Current->Data + exe->Reg[9], exe->Reg[9]);
			#endif
		
		break;
	
		//  6 bytes - mov short reg mem
		case 28:
			
			// Read register id
			OS_FS_Read( exe->File , &exe->Reg[8] , 1 );
			
			// Read mem
			exe->Reg[9] = Read_Word( exe->File );
			
			// Write data to reg from adress
			exe->Reg[exe->Reg[8]] =  *((Short*)((Byte*)exe->Current->Data + exe->Reg[9]));
			
			//DEBUG
			#if OS_DEBUG_PROCESS >= 3
				OS_Debug_Print("Register %d set %d from memory %d that has block offset %d", exe->Reg[8], exe->Reg[exe->Reg[8]], (Short*)exe->Current->Data + exe->Reg[9], exe->Reg[9]);
			#endif
		
		break;
		
		//  6 bytes - mov word reg mem 
		case 29:
			
			// Read register id
			OS_FS_Read( exe->File , &exe->Reg[8] , 1 );
			
			// Read mem
			exe->Reg[9] = Read_Word( exe->File );
		
			// Write data to reg from adress
			exe->Reg[exe->Reg[8]] = *((Word*)((Byte*)exe->Current->Data + exe->Reg[9]));
			
			//DEBUG
			#if OS_DEBUG_PROCESS >= 3
				OS_Debug_Print("Register %d set %d from memory %d that has block offset %d", exe->Reg[8], exe->Reg[exe->Reg[8]], (Word*)exe->Current->Data + exe->Reg[9], exe->Reg[9]);
			#endif
		
		break;
		
		//  6 bytes - mov reg [mem]
		case 30:
			
			// Read register id
			OS_FS_Read( exe->File , &exe->Reg[8] , 1 );
			
			// Read [mem]
			exe->Reg[9] = Read_Word( exe->File );
			
			// Write adress to reg
			exe->Reg[exe->Reg[8]] = ((Word)exe->Current->Data + exe->Reg[9]);
			
			//DEBUG
			#if OS_DEBUG_PROCESS >= 3
				OS_Debug_Print("To register %d was written adress %d", exe->Reg[8], exe->Reg[exe->Reg[8]]);
			#endif
		
		break;
		
		// 6 bytes - mov !byte! mem reg
		case 31:
			
			// Read mem
			exe->Reg[8] = Read_Word(exe->File);
			
			// Read reg
			OS_FS_Read(exe->File, &exe->Reg[9], 1);
			
			// Write data
			*((Byte*)((Byte*)exe->Current->Data + exe->Reg[8])) = exe->Reg[exe->Reg[9]];
			
			//DEBUG
			#if OS_DEBUG_PROCESS >= 3
				OS_Debug_Print("To memory %#x was written %d from register %d", (Byte*)((Byte*)exe->Current->Data + exe->Reg[8]), exe->Reg[exe->Reg[9]], exe->Reg[9]);
			#endif

		break;
		
		// 6 bytes - mov short mem reg
		case 32:
			
			// Read mem
			exe->Reg[8] = Read_Word(exe->File);
			
			// Read reg
			OS_FS_Read(exe->File, &exe->Reg[9], 1);
			
			// Write data
			*((Short*)((Byte*)exe->Current->Data + exe->Reg[8])) = exe->Reg[exe->Reg[9]];
			
			//DEBUG
			#if OS_DEBUG_PROCESS >= 3
				OS_Debug_Print("To memory %#x was written %d from register %d", (Short*)((Byte*)exe->Current->Data + exe->Reg[8]), exe->Reg[exe->Reg[9]], exe->Reg[9]);
			#endif
			
		break;
				
		// 6 bytes - mov word mem reg
		case 33:
			
			// Read mem
			exe->Reg[8] = Read_Word(exe->File);
			
			// Read reg
			OS_FS_Read(exe->File, &exe->Reg[9], 1);
			
			// Write data
			*((Word*)((Byte*)exe->Current->Data + exe->Reg[8])) = exe->Reg[exe->Reg[9]];
			
			//DEBUG
			#if OS_DEBUG_PROCESS >= 3
				OS_Debug_Print("To memory %#x was written %d from register %d", (Word*)((Byte*)exe->Current->Data + exe->Reg[8]), exe->Reg[exe->Reg[9]], exe->Reg[9]);
			#endif
			
		break;
		
		// 6 bytes - mov mem const8
		case 34:
			
			// Read mem
			exe->Reg[8] = Read_Word( exe->File );
			
			// Read const8
			OS_FS_Read( exe->File , &exe->Reg[9] , 1 );
			
			// Write data
			*(((Byte*)exe->Current->Data + exe->Reg[8])) = exe->Reg[9];
			
			//DEBUG
			#if OS_DEBUG_PROCESS >= 3
				OS_Debug_Print("To memory %#x was written %d", (Byte*)exe->Current->Data + exe->Reg[8], exe->Reg[9]);
			#endif
			
		break;
		
		// 7 bytes - mov mem const16
		case 35:
			
			// Read mem
			exe->Reg[8] = Read_Word( exe->File );
			
			// Read const16
			OS_FS_Read( exe->File , &exe->Reg[9] , 2 );
			
			// Write data
			*((Short*)((Byte*)exe->Current->Data + exe->Reg[8])) = Make_Short(exe->Reg[9],exe->Reg[10]);
			
			//DEBUG
			#if OS_DEBUG_PROCESS >= 3
				OS_Debug_Print("To memory %#x was wrtten %d", (Short*)exe->Current->Data + exe->Reg[8], Make_Short(exe->Reg[9],exe->Reg[10]));
			#endif
			
		break;
		
		// 9 bytes - mov mem const32
		case 36:
			
			// Read mem
			exe->Reg[8] = Read_Word( exe->File );
			
			// Read const32
			exe->Reg[9] = Read_Word( exe->File );
			
			// Write data
			*((Word*)((Byte*)exe->Current->Data + exe->Reg[8])) = exe->Reg[9];
			
			//DEBUG
			#if OS_DEBUG_PROCESS >= 3
				OS_Debug_Print("To memory %d#x was written %d", (Word*)exe->Current->Data + exe->Reg[8], exe->Reg[9]);
			#endif
			
		break;
		
		// 3 bytes - mov reg const8
		case 37:
			
			// Read register id
			OS_FS_Read( exe->File , &exe->Reg[8] , 1 );
			
			// Read const8
			OS_FS_Read( exe->File , &exe->Reg[9] , 1 );
			
			// Write data
			exe->Reg[exe->Reg[8]] = exe->Reg[9];
			
			//DEBUG
			#if OS_DEBUG_PROCESS >= 3
				OS_Debug_Print("To register %d was written %d", exe->Reg[8], exe->Reg[9]);
			#endif
			
		break;
		
		// 4 bytes - mov reg const16
		case 38:
			
			// Read register id
			OS_FS_Read( exe->File , &exe->Reg[8] , 1 );
			
			// Create word and write it
			exe->Reg[exe->Reg[8]] = Read_Short(exe->File);
			
			//DEBUG
			#if OS_DEBUG_PROCESS >= 3
				OS_Debug_Print("To register %d was written %d", exe->Reg[8], exe->Reg[exe->Reg[8]]);
			#endif
			
		break;
		
		// 6 bytes - mov reg const32
		case 39:
			
			// Read register id
			OS_FS_Read( exe->File , &exe->Reg[8] , 1 );
			
			// Read const32
			exe->Reg[exe->Reg[8]] = Read_Word(exe->File);
			
			//DEBUG
			#if OS_DEBUG_PROCESS >= 3
				OS_Debug_Print("To register %d was written %d", exe->Reg[8], exe->Reg[exe->Reg[8]]);
			#endif
			
		break;
		
		/////////////////////////////////////////////////////////////////////////////////////	
		// [Arithmetic section]
		/////////////////////////////////////////////////////////////////////////////////////
		
		// 3 bytes - add reg reg
		case 50:
			
			// Read 1st register
			OS_FS_Read( exe->File , &exe->Reg[8] , 1 );
			
			// Read 2nd register
			OS_FS_Read( exe->File , &exe->Reg[9] , 1 );
			
			// Perform addition
			exe->Reg[exe->Reg[8]] = exe->Reg[exe->Reg[8]] + exe->Reg[exe->Reg[9]];
			
			//DEBUG
			#if OS_DEBUG_PROCESS >= 3
				OS_Debug_Print("Register %d + %d, equals %d", exe->Reg[8], exe->Reg[9], exe->Reg[exe->Reg[8]]);
			#endif
			
		break;
		
		// 3 bytes - sub reg reg
		case 51:
			
			// Read 1st register
			OS_FS_Read( exe->File , &exe->Reg[8] , 1 );
			
			// Read 2nd register
			OS_FS_Read( exe->File , &exe->Reg[9] , 1 );
			
			// Perform substraction
			exe->Reg[exe->Reg[8]] = exe->Reg[exe->Reg[8]] - exe->Reg[exe->Reg[9]];
			
			//DEBUG
			#if OS_DEBUG_PROCESS >= 3
				OS_Debug_Print("Register %d - %d, equals %d", exe->Reg[8], exe->Reg[9], exe->Reg[exe->Reg[8]]);
			#endif
			
		break;
		
		// 3 bytes - mul reg reg
		case 52:
			
			// Read 1st register
			OS_FS_Read( exe->File , &exe->Reg[8] , 1 );
			
			// Read 2nd register
			OS_FS_Read( exe->File , &exe->Reg[9] , 1 );
			
			// Perform multiplication
			exe->Reg[exe->Reg[8]] = exe->Reg[exe->Reg[8]] * exe->Reg[exe->Reg[9]];
			
			//DEBUG
			#if OS_DEBUG_PROCESS >= 3
				OS_Debug_Print("Register %d * %d, equals %d", exe->Reg[8], exe->Reg[9], exe->Reg[exe->Reg[8]]);
			#endif
			
		break;
		
		/////////////////////////////////////////////////////////////////////////////////////	
		// [Flow control section]
		/////////////////////////////////////////////////////////////////////////////////////
		
		// 5 bytes - jump const32
		case 90:
			
			// Read Adress
			exe->Reg[8] = Read_Word( exe->File );
			
			// Save block pointer
			exe->Current->FP = exe->File->fptr;
		
			// Move file pointer of a file
			OS_FS_Seek (exe->File, exe->Reg[8]);
			
			//DEBUG
			#if OS_DEBUG_PROCESS >= 3
				OS_Debug_Print("Jump to %#x", exe->Reg[8]);
			#endif
		
		break;
		
		// 7 bytes - je reg reg const32
		case 91:
			
			// Read 1st register
			OS_FS_Read(exe->File, &exe->Reg[8], 1);
			
			// Read 2nd register
			OS_FS_Read(exe->File, &exe->Reg[9], 1);
			
			// Read jump adress
			exe->Reg[10] = Read_Word( exe->File );
			
			// Compare
			if(exe->Reg[exe->Reg[8]] == exe->Reg[exe->Reg[9]])
			{
				// Move file pointer of a file
				OS_FS_Seek ( exe->File , exe->Reg[10] );
				
				// Save block pointer
				exe->Current->FP = exe->File->fptr;
				
				//DEBUG
				#if OS_DEBUG_PROCESS >= 3
					OS_Debug_Print("Compare register %d with %d, and jump to %#x", exe->Reg[8], exe->Reg[9], exe->Reg[10]);
				#endif
			}
			else
			{
				//DEBUG
				#if OS_DEBUG_PROCESS >= 3
					OS_Debug_Print("Compare register %d with %d, without jump", exe->Reg[8], exe->Reg[9]);
				#endif
			}
		
		break;
		
		// 7 bytes - jne reg reg const32
		case 92:
			
			// Read 1st register
			OS_FS_Read(exe->File , &exe->Reg[8], 1);
			
			// Read 2nd register
			OS_FS_Read(exe->File , &exe->Reg[9], 1);
			
			// Read jump adress
			exe->Reg[10] = Read_Word( exe->File );
			
			// Compare
			if(exe->Reg[exe->Reg[8]] != exe->Reg[exe->Reg[9]])
			{
				// Move file pointer of a file
				OS_FS_Seek ( exe->File , exe->Reg[10] );
				
				// Save block pointer
				exe->Current->FP = exe->File->fptr;
				
				//DEBUG
				#if OS_DEBUG_PROCESS >= 3
					OS_Debug_Print("Compare register %d with %d, and jump to %#x", exe->Reg[8], exe->Reg[9], exe->Reg[10]);
				#endif
			}
			else
			{
				//DEBUG
				#if OS_DEBUG_PROCESS >= 3
					OS_Debug_Print("Compare register %d with %d, without jump", exe->Reg[8], exe->Reg[9]);
				#endif
			}
		
		break;
			
		// 7 bytes - jg reg reg const32
		case 93:
			
			// Read 1st register
			OS_FS_Read( exe->File , &exe->Reg[8] , 1 );
			
			// Read 2nd register
			OS_FS_Read( exe->File , &exe->Reg[9] , 1 );
			
			// Read jump adress
			exe->Reg[10] = Read_Word( exe->File );
			
			// Compare
			if(exe->Reg[exe->Reg[8]] > exe->Reg[exe->Reg[9]])
			{
				// Move file pointer of a file
				OS_FS_Seek ( exe->File , exe->Reg[10] );
				
				// Save block pointer
				exe->Current->FP = exe->File->fptr;
				
				//DEBUG
				#if OS_DEBUG_PROCESS >= 3
					OS_Debug_Print("Compare register %d with %d, and jump to %#x", exe->Reg[8], exe->Reg[9], exe->Reg[10]);
				#endif
			}
			else
			{
				//DEBUG
				#if OS_DEBUG_PROCESS >= 3
					OS_Debug_Print("Compare register %d with %d, without jump", exe->Reg[8], exe->Reg[9]);
				#endif
			}
		
		break;
			
		// 7 bytes - jge reg reg const32
		case 94:
			
			// Read 1st register
			OS_FS_Read( exe->File , &exe->Reg[8] , 1 );
			
			// Read 2nd register
			OS_FS_Read( exe->File , &exe->Reg[9] , 1 );
			
			// Read jump adress
			exe->Reg[10] = Read_Word( exe->File );
			
			// Compare
			if(exe->Reg[exe->Reg[8]] >= exe->Reg[exe->Reg[9]])
			{
				// Move file pointer of a file
				OS_FS_Seek ( exe->File , exe->Reg[10] );
				
				// Save block pointer
				exe->Current->FP = exe->File->fptr;
				
				//DEBUG
				#if OS_DEBUG_PROCESS >= 3
					OS_Debug_Print("Compare register %d with %d, and jump to %#x", exe->Reg[8], exe->Reg[9], exe->Reg[10]);
				#endif
			}
			else
			{
				//DEBUG
				#if OS_DEBUG_PROCESS >= 3
					OS_Debug_Print("Compare register %d with %d, without jump", exe->Reg[8], exe->Reg[9]);
				#endif
			}
		
		break;
			
		// 7 bytes - jl reg reg const32
		case 95:
			
			// Read 1st register
			OS_FS_Read( exe->File , &exe->Reg[8] , 1 );
			
			// Read 2nd register
			OS_FS_Read( exe->File , &exe->Reg[9] , 1 );
			
			// Read jump adress
			exe->Reg[10] = Read_Word( exe->File );
			
			// Compare
			if(exe->Reg[exe->Reg[8]] < exe->Reg[exe->Reg[9]])
			{
				// Move file pointer of a file
				OS_FS_Seek ( exe->File , exe->Reg[10] );
				
				// Save block pointer
				exe->Current->FP = exe->File->fptr;
				
				//DEBUG
				#if OS_DEBUG_PROCESS >= 3
					OS_Debug_Print("Compare register %d with %d, and jump to %#x", exe->Reg[8], exe->Reg[9], exe->Reg[10]);
				#endif
			}
			else
			{
				//DEBUG
				#if OS_DEBUG_PROCESS >= 3
					OS_Debug_Print("Compare register %d with %d, without jump", exe->Reg[8], exe->Reg[9]);
				#endif
			}
		
		break;
			
		// 7 bytes - jle reg reg const32
		case 96:
			
			// Read 1st register
			OS_FS_Read( exe->File , &exe->Reg[8] , 1 );
			
			// Read 2nd register
			OS_FS_Read( exe->File , &exe->Reg[9] , 1 );
			
			// Read jump adress
			exe->Reg[10] = Read_Word( exe->File );
			
			// Compare
			if(exe->Reg[exe->Reg[8]] <= exe->Reg[exe->Reg[9]])
			{
				// Move file pointer of a file
				OS_FS_Seek ( exe->File , exe->Reg[10] );
				
				// Save block pointer
				exe->Current->FP = exe->File->fptr;
				
				//DEBUG
				#if OS_DEBUG_PROCESS >= 3
					OS_Debug_Print("Compare register %d with %d, and jump to %#x", exe->Reg[8], exe->Reg[9], exe->Reg[10]);
				#endif
			}
			else
			{
				//DEBUG
				#if OS_DEBUG_PROCESS >= 3
					OS_Debug_Print("Compare register %d with %d, without jump", exe->Reg[8], exe->Reg[9]);
				#endif
			}
		
		break;

		//ERROR
		default:
			
			//ERROR
			#if OS_DEBUG_PROCESS >= 1
				OS_Debug_Print("Unknown command!");
			#endif
		
		break;
	}
	
	
	
	
}

/*
#include "RTC.h"
#include "..\Platform\LPC17xx\lpc17xx_rtc.h"
#include "..\Platform\LPC17xx\lpc17xx_clkpwr.h"
//extern RTC_TIME_Type current_time;
extern RTCTime current_time;
*/

// Executes system interrupt for given Process
void OS_Interrupt(Process *exe , Byte id)
{			
	//DEBUG
	#if OS_DEBUG_PROCESS >= 2
		OS_Debug_Print("Process \'%s\' PID=%d performs interrupt id=%d", exe->Name, exe->PID, id);
	#endif
	
	// Switch interrupt id
	switch(id)
	{		
		// Terminate program
		case 1:
			OS_KillProcessParent(exe);
		break;
		
		// Generate random number from given range <Min; Max>
		case 2:
			exe->Reg[0] =  exe->Reg[2] ? (rand() % exe->Reg[2] + exe->Reg[1]) : exe->Reg[1];
		break;
		
		// Get Application with given ID, ID 0 means to give current app data to structure. App struct is adress
		case 3:
			{
				// Cache Application structure
				Application *appI = Null;
				
				// Check if zero
				if(exe->Reg[1] == 0)
				{
					//..........................................................................
					
				}
				else
				{
					// Get application with id
					appI = OS_App_GetAppByID(exe->Reg[1]);
				}
				
				// Check if valid
				if(appI == Null)
				{
					// No application found
					exe->Reg[0] = 0;
				}
				else	
				{
					// Copy data
					OS_UTILS_MemoryCopy((void*)exe->Reg[0], appI, sizeof(Application));
				}
			}
		break;

		// Draw string on the screen at the point
		case 50: OS_GPU_Print_Text(((Point*)&exe->Reg[0])->X, ((Point*)&exe->Reg[0])->Y, ((Char*)exe->Reg[1]), exe->Reg[2]);
		
			 //OS_GPU_Print_Text(exe->Reg[0], exe->Reg[3], ((Char*)exe->Reg[1]), exe->Reg[2]);
// 		OS_Debug_Print("Time: %d/%d/%d/%d:%d:%d",
// 	current_time.RTC_Year,
// 	current_time.RTC_Mon,
// 	current_time.RTC_Yday,
// 	current_time.RTC_Hour,
// 	current_time.RTC_Min,
// 	current_time.RTC_Sec);
		
		break;
		
		// Draw number on the screen at the point
		case 51:
			OS_GPU_Print_Word(((Point*)&exe->Reg[0])->X, ((Point*)&exe->Reg[0])->Y, exe->Reg[1], exe->Reg[2]);
		break;
		
		// Draw singed number on the screen at the point
		case 52:
			OS_GPU_Print_SWord(((Point*)&exe->Reg[0])->X, ((Point*)&exe->Reg[0])->Y, exe->Reg[1], exe->Reg[2]);
		break;
		
		// Draw floating point on the screen at the point
		case 53:
			OS_GPU_Print_Float(((Point*)&exe->Reg[0])->X, ((Point*)&exe->Reg[0])->Y, exe->Reg[1], exe->Reg[2]);
		break;
		
		// Draw point on the screen with color
		case 54:
			OS_GPU_SetPoint(((Point*)&exe->Reg[0])->X, ((Point*)&exe->Reg[0])->Y, exe->Reg[1]);
		break;
		
		default:
			
			//DEBUG
			#if OS_DEBUG_PROCESS >= 1
				OS_Debug_Print("Invalid interrupt!");
			#endif
		
		break;
	}
}
