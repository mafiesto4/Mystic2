//////////////////////////////////////////////////////////////////////////////////////
// Mystic OS Copyright 2012-2013 (c) Wojciech Figat
// The software is provided 'as is'. 
// The original author is not responsible for any harm which may result from use of this software or its source code
// See also my 3d game engine --> http://celelej.com
//////////////////////////////////////////////////////////////////////////////////////

/*
TODO
- Computer Name
- System Directories
- Get System Version
- Last error number in process
- System errors handling

*/

// Includes
#include "Kernel.h"
#include "Config.h"
#include "Process.h"
#include "FileSystem.h"
#include "..\Graphics\Graphics.h"
#include "..\Platform\Procesor_CPU.h"


#include "RTC.h"

// Stardard C
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

// Application list
List Applications;

// Processes list
List Processes;

// Graphics Device
GraphicsDevice GPU;

// File System
FileSystem Fat;

// Mystic Graphics Handle, allows to draw over whole screen. Used by dialog boxes and other stuff
GraphicsHandle MysticGH;

// Current Graphics Handle
extern volatile GraphicsHandle *CGH;

// Debug Mode
#if OS_DEBUG

	// Debugging Input Buffer
	static Byte data[2];
	
#endif


Process *ppp;
/*
extern volatile uint32_t alarm_on;
//RTC_TIME_Type alarm_time, current_time;
RTCTime local_time, alarm_time, current_time;
*/

// Allocate memory block that has given size
void* OS_Allocate(Word size)
{
	// Allocate data
	void* p = malloc(size);
	
	// Check if good
	if(p == Null)
	{
		// Show critical error
		OS_Error(RES_OUT_OF_MEMORY, size);
	}
	
	// Return pointner
	return p;
}

// General Initialization
void OS_Init(void)
{
	// 1 second delay to prevent dual start etc.
	OS_CPU_Delay(1000);
	
	// UART
	#if OS_UART_ENABLE == 1
		OS_UART_Init();
	#endif
	
	// Welcome Message
	#if OS_DEBUG == 1
		OS_Debug_Print( "======================================================" );
		OS_Debug_Print( "Mystic OS v2.0 32 bit" );
	#endif
	
	// Init Graphics Device	
	GPU.Status = OS_GPU_UNKNOW;
	OS_GPU_Init(OS_GPU_LANDSCAPE, 320, 240);
	
	// Set up default graphics handle
	MysticGH.Status = OS_GPU_HANDLE_FOCUSED;
	MysticGH.X = 0;
	MysticGH.Y = 0;
	MysticGH.Width = GPU.Width;
	MysticGH.Height = GPU.Height;
	CGH = &MysticGH;
	
	// Check GPU ready
	if(GPU.Status != OS_GPU_READY)
	{
		#if OS_DEBUG == 1
			OS_Debug_Print("Cannot init GPU");
		#endif
	}

	#if OS_DEBUG == 1
		OS_Debug_Print("GPU Status %d, Driver Code %d" , GPU.Status, GPU.Code);
	#endif
	
	// Initialization entry point
	RETRY:
	
	// Loading - Start
	OS_GPU_Clear(White);
	OS_GPU_Print_Text(130, 70, "Mystic", Black);
	OS_GPU_Print_Text(120, 85, "Loading..", System_Grey);
	#if OS_DEBUG == 1
		OS_GPU_Print_Text( 230 , 220 , "Debug Mode" , System_Grey );
	#endif
	
	// Set up
	OS_List_Init(&Applications);
	OS_List_Init(&Processes);
	
	// Init SD card
	if(OS_FS_Available() == False)
    {
		#if OS_DEBUG == 1
			OS_Debug_Print("Please connect a SD card");
		#endif
		
		OS_GPU_Print_Text(100, 110, "Insert SD card", Orange);
		
		// Wait fo SD card
		while(OS_FS_Available() == False);
    }
	
	#if OS_DEBUG == 1
		OS_Debug_Print("SD card detected");
	#endif
	
	// Init FileSystem
	if(OS_FS_Init() != FS_RESULT_OK)
	{
		#if OS_DEBUG == 1
			OS_Debug_Print("Cannot init FileSystem");
		#endif
		///////// TODO wypisywanie result code
		OS_GPU_Print_Text(80, 130, "Cannot init FileSystem", Red);
		
		// 3 second delay
		OS_CPU_Delay(3000);
		
		goto RETRY;
	}
	
	#if OS_DEBUG == 1
	{
		CPU_Info info;
		OS_CPU_GetInfo(&info);
		
		OS_Debug_Print("Platform %d, Cpu %d, Clock Speed %dMHz,\nSystemMemory %x, FreeMemory %x, SystemStackSize %x",
		info.Platform,
		info.Cpu,
		info.Clock / 1000 / 1000,
		info.SystemMemory,
		info.FreeMemory,
		info.SystemStackSize
		);
	}
	#endif
	
	// Loading - End
	OS_GPU_Clear(White);
	CGH = Null;
}

// Private. Close all.
void P_OS_Close(void)
{
	// Close FileSystem
	OS_FS_Exit();
	
	// Clear screen
	OS_GPU_Clear(Black);
}

// Close OS and release all resources
void OS_Exit(void)
{
	#if OS_DEBUG == 1
		OS_Debug_Print("Closing...");
	#endif
	
	// Close all
	P_OS_Close();
	
	// Turn off
	OS_CPU_TurnOff();
}

// Close OS, release all resources and restart OS
void OS_Restart(void)
{
	// Debug
	#if OS_DEBUG == 1
		OS_Debug_Print("Restarting...");
	#endif
	
	// Close all
	P_OS_Close();
	
	// Reset CPU
	OS_CPU_Restart();
}
/*
// buffer must have length >= sizeof(int) + 1
// Write to the buffer backwards so that the binary representation
// is in the correct order i.e. the LSB is on the far right
// instead of the far left of the printed string
char *int2bin(int a, char *buffer, int buf_size) 
{
	int i = 31;
    buffer += (buf_size - 1);

    for (; i >= 0; i--) 
	{
        *buffer-- = (a & 1) + '0';

        a >>= 1;
    }

    return buffer;
}

#define BUF_SIZE 33
*/
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#define countof(x) (sizeof(x)/sizeof(x[0]))
 
int min(int value1, int value2)
{
	return ( (value1 < value2) ? value1 : value2);
}
 
int max(int value1, int value2)
{
	 return ( (value1 > value2) ? value1 : value2);
}
 
typedef int bool;
typedef unsigned char byte;
//typedef unsigned short word;
typedef unsigned char word;
typedef unsigned long dword;
 
#define WIDTH 120
#define HEIGHT 81
#define BPP 8

typedef struct
{
	float x;
	float y;
	float z;
} Vector3;
 
typedef struct
{
	int x;
	int y;
} Vector2;
 
Vector3 object[] = {
 {  40,  40,  40 },
 {  40,  40, -40 },
 {  40, -40,  40 },
 {  40, -40, -40 },
 { -40,  40,  40 },
 { -40,  40, -40 },
 { -40, -40,  40 },
 { -40, -40, -40 },
};

int indicesQuad[] = {
	0, 2, 6, 4,
	5, 7, 3, 1,
	4, 5, 1, 0,
	2, 3, 7, 6,
	0, 1, 3, 2,
	6, 7, 5, 4
};
 
Vector3 transPoly[countof(object)];
Vector2 screenPoly[countof(object)];

word* pBackBuffer;

// Render pixel span to the memory
void RenderSpan(int x, int y, int length, word color)
{
	// Check location
    if (x < 0 || y < 0)
	{
        return;
	}
	else
	{
		// Counter
		int i =0;
	
		// Calculate first pixel offset in span in memory
		int offset = (y * WIDTH) + x;
		
		// Render to memory
		for (i = 0; i < length; i++)
		{
			pBackBuffer[offset++] = color;
		}
	}
}
 
void renderPolygonSub(Vector2* pVerts[], int count, word color)
{
	// Vertices must be pre-rotated with lowest Y at index 0.
	// Order must be counter-clockwise,
	// Left edges from beginning, right edges from end.
	int x = pVerts[0]->x;
	int y = pVerts[0]->y;
	int edgeLeft = 0;
	int edgeRight = count;
	int newx, newy, prestep, length, ystop;
	
	// Endpoints for current edges
	int xLeft  = x;
	int yLeft  = y;
	int xRight = x;
	int yRight = y;
 
	int dxLeft, dyLeft, dxRight, dyRight;
	long dxdyLeft=0, dxdyRight=0;
	long curxLeft=0, curxRight=0;
	
	y >>= 4;
 
	while (edgeLeft < edgeRight)
	{
		// Update edges
		if ((yLeft >> 4) <= y)
		{
			edgeLeft++;
 
			newx = pVerts[edgeLeft]->x;
			newy = pVerts[edgeLeft]->y;
 
			dyLeft = newy - yLeft;
 
			if (dyLeft > 0)
			{
				dxLeft = newx - xLeft;
 
				dxdyLeft = ((long)dxLeft << (16))/dyLeft;
				curxLeft = (long)xLeft << (16-4);
 
				prestep = 15 - (yLeft & 0xF);
				curxLeft += (prestep*dxdyLeft >> 4);
 
				yLeft = newy;
			}
 
			xLeft = newx;
		}
 
		if ((yRight >> 4) <= y)
		{
			edgeRight--;
 
			newx = pVerts[edgeRight]->x;
			newy = pVerts[edgeRight]->y;
 
			dyRight = newy - yRight;
 
			if (dyRight > 0)
			{
				dxRight = newx - xRight;
 
				dxdyRight = ((long)dxRight << (16))/dyRight;
				curxRight = (long)xRight << (16-4);
 
				prestep = 15 - (yRight & 0xF);
				curxRight += (prestep*dxdyRight >> 4);
 
				yRight = newy;
			}
 
			xRight = newx;
		}
 
		ystop = min(yLeft, yRight) >> 4;
 
		while (y < ystop)
		{
			// Emit scanline
			length = (curxRight >> (16)) - (curxLeft >> (16));
			if (length > 0)
				RenderSpan(curxLeft >> 16, y, length, color);
			y++;
 
			// Step left edge
			curxLeft  += dxdyLeft;
 
			// Step right edge
			curxRight += dxdyRight;
		}
	}
}
 
void Transform( Vector3* pObject, int length, float angle )
{
	#define SCALE 16
	
	int i;
	float iz;
	Vector3 temp, vec;
	Vector3* pVertex;
	float sina = sin(angle);
	float cosa = cos(angle);
 
	for (i = 0; i < length; i++)
	{
		pVertex = &pObject[i];
		
		vec.x = (pVertex->x*cosa - pVertex->y*sina);
		vec.y = (pVertex->x*sina + pVertex->y*cosa);
		vec.z = pVertex->z;
 
		temp = vec;
		pVertex = &temp;
 
		vec.x = pVertex->x;
		vec.y = (pVertex->y*cosa - pVertex->z*sina);
		vec.z = (pVertex->y*sina + pVertex->z*cosa);
 
		vec.z += 400.0f;
 
		transPoly[i].x = vec.x;
		transPoly[i].y = vec.y;
		transPoly[i].z = vec.z;
 
		iz = (200.0f*SCALE)/vec.z;
 
		screenPoly[i].x = (vec.x*iz) + ((WIDTH/2)*SCALE);
		screenPoly[i].y = (vec.y*iz) + ((HEIGHT/2)*SCALE);
	}
}
 
void GetNormal( Vector3* pNormal, Vector3* pV0, Vector3* pV1, Vector3* pV2 )
{
	Vector3 ba, ca;
 
	ba.x = pV1->x - pV0->x;
	ba.y = pV1->y - pV0->y;
	ba.z = pV1->z - pV0->z;
 
	ca.x = pV2->x - pV0->x;
	ca.y = pV2->y - pV0->y;
	ca.z = pV2->z - pV0->z;
 
	// Crossproduct for normal vector
	pNormal->x = (ba.y * ca.z) - (ba.z * ca.y);
	pNormal->y = (ba.z * ca.x) - (ba.x * ca.z);
	pNormal->z = (ba.x * ca.y) - (ba.y * ca.x);
}

bool IsFrontFace( Vector3* pNormal, float d )
{
	return (pNormal->z > d);
}

bool RotatePoly( Vector2* pPoly[], Vector2* pVerts, int* pIndices, int count)
{
	// Polygon vertices must always be orderd clockwise or counter-clockwise
	// So if we find the top vertex, we can walk the edges left and right
	int topIndex = 0;
	int topy = pVerts[pIndices[0]].y;
	int i, edgeLeft, edgeRight;
 
	// Determine whether it is clockwise or counter-clockwise
	int x0 = pVerts[pIndices[0]].x;
	int y0 = pVerts[pIndices[0]].y;
	int x1 = pVerts[pIndices[1]].x;
	int y1 = pVerts[pIndices[1]].y;
	int x2 = pVerts[pIndices[2]].x;
	int y2 = pVerts[pIndices[2]].y;
 
	int dx0 = x1 - x0;
	int dx1 = x2 - x0;
	int dy0 = y1 - y0;
	int dy1 = y2 - y0;
 
	bool cw = dx0*(long)dy1 < dx1*(long)dy0;
 
	for (i = 1; i < count; i++)
		if (topy > pVerts[pIndices[i]].y)
		{
			topy = pVerts[pIndices[i]].y;
			topIndex = i;
		}
 
	edgeLeft = topIndex - 1;
	if (edgeLeft < 0)
		edgeLeft = count - 1;
	edgeRight = topIndex + 1;
	if (edgeRight >= count)
		edgeRight = 0;
 
	// Swap vertices if required, so that they are always
	// ordered counter-clockwise
	// (left side at beginning, right side at end)
	if (cw)
	{
		// Now copy polygon in rotated form
		i = 0;
		for (; topIndex < count; i++, topIndex++)
			pPoly[i] = &pVerts[pIndices[topIndex]];
 
		topIndex = 0;
 
		for (; i < count; i++, topIndex++)
			pPoly[i] = &pVerts[pIndices[topIndex]];
	}
	else
	{
		// Now copy polygon in rotated form, inverting order
		i = 0;
		for (; topIndex >= 0; i++, topIndex--)
			pPoly[i] = &pVerts[pIndices[topIndex]];
 
		topIndex = count - 1;
 
		for (; i < count; i++, topIndex--)
			pPoly[i] = &pVerts[pIndices[topIndex]];
	}
 
	return !cw;
}

// Start system and keep it running
void OS_Run(void)
{
	// Data
	Byte i, p;
	Process *proc;

	/*
	
// 	LPC_RTC_TypeDef t;

// 	PackedTime t1;
// 	Time t2;
// 	char buffer[BUF_SIZE];
	
	#if OS_DEBUG == 1
	OS_Debug_Print( "Opening app 1: Result: %d." , OS_App_Open( OS_PROC_NORMALPERM , "0:\\Mystic\\YourName.exe" , 0 , 0 ) );
	#else
	OS_App_Open( OS_PROC_NORMALPERM , "0:\\Mystic\\YourName.exe" , 0 , 0 );
		#endif
	*/

	
	int a = 0;
	int j;
	float angle, ilen, lenSq;
	word color;
	
	CGH = &MysticGH;
	pBackBuffer = (word*)OS_Allocate(WIDTH * HEIGHT * (BPP / 8));
	
	while (1)
	{
		// Clear screen
		memset(pBackBuffer, 0, WIDTH * HEIGHT * (BPP / 8));
		
		angle = 0.05 * a;
		
		a++;
		
		Transform(object, countof(object), angle);
		
		for (j = 0; j < countof(indicesQuad); j += 4)
		{
			int i0 = indicesQuad[j+0];
			int i1 = indicesQuad[j+1];
			int i2 = indicesQuad[j+2];
			//int i3 = indicesQuad[j+3];
			
			Vector2* pVerts[4];
			
			if (RotatePoly(pVerts, screenPoly, &indicesQuad[j], 4))
			{
				Vector3 normal;
				GetNormal(
					&normal,
					&transPoly[i0],
					&transPoly[i1],
					&transPoly[i2]);
				
				// Determine light
				lenSq = normal.x * normal.x + normal.y * normal.y + normal.z * normal.z;
				
				ilen = sqrt(lenSq);
				if (fabs(ilen) > 0.0001f)
					ilen = 1.0f / ilen;
				else
					ilen = 0;
				
				normal.z *= ilen;
				
				// Map to 5-bit blue colour
				color = normal.z * 31.5f;
				
				renderPolygonSub(pVerts, 4, color);
			}
		}
		
		// Render image
		OS_GPU_DrawImage(60, 40, WIDTH, HEIGHT, pBackBuffer);
	}
	
	
	/*
	t2.Second = 59;									
	t2.Minute = 14;									
	t2.Hour = 19;								
	t2.Day = 12;										
	t2.Month = 12;					
	t2.Year = 2012;	
	
	OS_Debug_Print( "Valid: %d",Os_Time_IsVaildTime( t2 ));
	
	t1 = OS_Time_FullToPacked( t2 );
	t2 = OS_Time_PackedToFull( t1 );
	
    buffer[BUF_SIZE - 1] = '\0';
    int2bin(t1, buffer, BUF_SIZE - 1);
    OS_Debug_Print("a = %s", buffer);

	OS_Debug_Print( "Second %d Minute %d Hour %d Day %d Month %d Year %d",
	t2.Second,									
	t2.Minute,									
	t2.Hour,								
	t2.Day,										
	t2.Month,					
	t2.Year);
	*/
	
/*
	NVIC_EnableIRQ (RTC_IRQn);	// Set the RTC Interrupts
 
	LPC_GPIO1->FIODIR |= (1<<28);		// GPIO1.28 as a output pin with a LED
	LPC_GPIO2->FIODIR |= (1<<5)|(1<<6);	// GPIO1.28 as a output pin with a LED
	LPC_GPIO2->FIOSET |= (1<<5);
	LPC_GPIO2->FIOCLR |= (1<<6);
 
	LPC_SC->PCONP |= (1<<9);	// Set the power and the clock signal to the Real Time Clock
 
	LPC_RTC->CCR  = 0;			// Reset the Clock Control Register
	LPC_RTC->AMR  = 0;			// Reset the Alarm Mask Register
	LPC_RTC->CIIR = 0;			// Reset the Counter Increment Interrupt Register
	LPC_RTC->CIIR |= (1<<0);	// An increment of the Second value generates an interrupt
 
	LPC_RTC->HOUR  =  9;		// Set the hour
	LPC_RTC->MIN   = 49;		// Set the minutes
	LPC_RTC->SEC   =  0;		// Set the seconds
	LPC_RTC->DOM   = 28;		// Set the day
	LPC_RTC->MONTH =  6;		// Set the month
	LPC_RTC->YEAR  = 12;		// Set the year
 
	LPC_RTC->CCR  |= (1<<0);	// Enable the RTC
	*/
	/*
	RTCInit();

  local_time.RTC_Sec = 0;
  local_time.RTC_Min = 0;
  local_time.RTC_Hour = 0;
  local_time.RTC_Mday = 8;
  local_time.RTC_Wday = 3;
  local_time.RTC_Yday = 12;		// current date 05/12/2010
  local_time.RTC_Mon = 5;
  local_time.RTC_Year = 2010;
  RTCSetTime( local_time );		// Set local time

  alarm_time.RTC_Sec = 0;
  alarm_time.RTC_Min = 0;
  alarm_time.RTC_Hour = 0;
  alarm_time.RTC_Mday = 1;
  alarm_time.RTC_Wday = 0;
  alarm_time.RTC_Yday = 1;		// alarm date 01/01/2011
  alarm_time.RTC_Mon = 1;
  alarm_time.RTC_Year = 2011;
  RTCSetAlarm( alarm_time );		// set alarm time

  NVIC_EnableIRQ(RTC_IRQn);

  // mask off alarm mask, turn on IMYEAR in the counter increment interrupt register 
  RTCSetAlarmMask(AMRSEC|AMRMIN|AMRHOUR|AMRDOM|AMRDOW|AMRDOY|AMRMON|AMRYEAR);
  LPC_RTC->CIIR = IMMIN | IMYEAR | IMSEC;
  
  // 2007/01/01/00:00:00 is the alarm on
  
  RTCStart();
 */
  
  
  /*
  RTC_Init(&t);
  
  current_time.SEC = 0;
  current_time.MIN = 0;
  current_time.HOUR = 0;
  current_time.DOM = 8;
  current_time.DOW = 3;
  current_time.DOY = 12;		// current date 05/12/2010
  current_time.MONTH = 5;
  current_time.YEAR = 2010;
  
  RTC_SetFullTime ( &t, &current_time);
  
  RTC_CntIncrIntConfig ( &t, RTC_TIMETYPE_SECOND, ENABLE );
  
   RTC_CalibCounterCmd( &t , ENABLE );
   
  RTC_ResetClockTickCounter(&t);
  
  RTC_Cmd ( &t, ENABLE );
  */
  
  
  /*
  
	// MAIN SYSTEM LOOP
	while(1)
	{
		// Process User Input if Debug Mode is enable
		#if OS_DEBUG == 1
			OS_Debug_Input();
		#endif
		
		
			//current_time = RTCGetTime();
		
// 			if ( alarm_on != 0 )
// 			{
// 			  alarm_on = 0;
// 			  current_time = RTCGetTime();	
// 			}
	

		//RTC_GetFullTime (&t, &current_time);
		
		// TODO save execution, when process terminates it needts to be save event durin while(loop)
		
		// Execute all processes
		for(i = 0; i < Processes.Count; i++)
		{
			// Cache process
			proc = (Process*)OS_List_GetAt(&Processes, i);
			
			// Check if is ready
			if(proc->Status == OS_PROC_READY)
			{
				// Set status
				proc->Status = OS_PROC_RUNNING;
				
				// Compute priority
				p = proc->Priotity * OS_PROC_SpeedFactor;
				
				// Set graphis handle
				CGH = proc->Graphics;
				
				// Execute
				while(p > 0)
				{
					OS_Execute(proc);
					p--;
				}
				
				// Finish
				proc->Status = OS_PROC_READY;
			}
		}
	}
	*/
}

// Show critical error
void OS_Error(Byte id, Word val)
{
	// Data
	Char buffer[32];
	
	#if OS_DEBUG == 1
		OS_Debug_Print("Critical Error! Id: %d, Value: %d", id, val);
	#endif
	
	// Blue Screen
	CGH = &MysticGH;
	sprintf(buffer, "Critical Error: %d, %d", id, val);
	OS_GPU_Clear(Blue);
	OS_GPU_Print_Text(70, 70, buffer, White);
	
	// End
	while(True)
	{ }
}

// Start up new application
Byte OS_App_Open(Byte flags, const Char* path, Word argc, char argv[])
{
	// Data
	Byte flag;
	Byte id;
	Byte res;
	Byte Buf[6];
	Application *New;
	Word start;
	
	// Get first free id
	id = OS_App_GetFirstFreeID();
	
	// No free ids
	if(id == 0)
	{
		// Error
		return RES_OUT_OF_FREE_APP_IDS;
	}
	
	// Start
	#if OS_DEBUG == 1
		OS_Debug_Print("Openning application with ID: %d", id);
	#endif
	
	// Allocate data
	New = (Application*)OS_Allocate(sizeof(Application));
	New->File = (File*)OS_Allocate(sizeof(File));
	
	// Set id
	New->ID = id;
	
	// Open file
	res = OS_FS_Open(New->File, path, FILE_ACCESS_READ);
	
	// Check if file is opened
	if (res != FS_RESULT_OK)
	{
		// Error
		free(New);
		return res;
	}
	
	// Read static-length header
	res = OS_FS_Read(New->File, &Buf, 6);
	
	// Check errors
	if (res != FS_RESULT_OK)
	{
		// Error
		goto ERROR_OPEN_APP;
	}
	
	// Check ME header
	if(Buf[0] != 'M' || Buf[1] != 'E' || Buf[3] < OS_VERSION_MIN || OS_VERSION_MAX < Buf[3])
	{
		// Error
		res = RES_INVALID_DATA;
		goto ERROR_OPEN_APP;
	}
	
	// Get code start adress
	start = Buf[2];
	
	// Get Flags
	flag = Buf[4];
	
	// Name size
	New->NameSize = Buf[5];
	New->Name = (Char*)OS_Allocate(Buf[5]);
	
	// Read name
	res = OS_FS_Read(New->File, New->Name, Buf[5]);
	
	// Errors - cannot read
	if(res != FS_RESULT_OK)
	{
		// Error
		goto ERROR_OPEN_APP;
	}
	
	// Move to start code adress
	res = OS_FS_Seek(New->File, start);
	
	// Errors - cannot seek
	if(res != FS_RESULT_OK)
	{
		// Error
		goto ERROR_OPEN_APP;
	}
	
	// Create new process
	New->Exe = OS_Proc_Create(New->Name, New->File, flag, OS_PROC_DefaultProcessPriroity, 0, 0);
	
	// If cannot create new process
	if(New->Exe == Null)
	{
		#if OS_DEBUG == 1
			OS_Debug_Print( "Cannot create new process!");
		#endif
		
		// Error
		res = RES_CANNOT_CREATE_PROCESS;
		goto ERROR_OPEN_APP;
	}
	
	// Assign Graphics Handle to the process
	New->Exe->Graphics = &New->Graphics;
	New->Graphics.Status = OS_GPU_HANDLE_FOCUSED;
	New->Graphics.X = 0;
	New->Graphics.Y = 0;
	New->Graphics.Width = GPU.Width;
	New->Graphics.Height = GPU.Height - 32;
	
	ppp = New->Exe;
	
	// Add application to the list
	OS_List_Add(&Applications, New);
	
	#if OS_DEBUG == 1
		OS_Debug_Print( "Done! Memory usage: %d bytes", OS_App_MemoryUsage(New));
	#endif
	
	// Run process
	OS_Proc_Run(New->Exe);
	
	// Good
	return RES_GOOD;
	
	// Error
	ERROR_OPEN_APP:
	OS_FS_Close(New->File);
	free(New);
	return res;
}

// Kill applicaiotn
void OS_App_Kill(Application *app)
{
	// Check input
	if(app == Null)
	{
		// Error
		return;
	}
	
	#if OS_DEBUG == 1
		OS_Debug_Print("Killing application '%s', ID=%d", app->Exe->Name, app->ID);
	#endif
	
	// Remove from list
	OS_List_Remove(&Applications, app);
	
	// Close process
	OS_Proc_Terminate(app->Exe);
	
	// Close file
	OS_FS_Close(app->File);
	
	// Clean memory
	free(app->File);
	free(app->Name);
	free(app);
	
	#if OS_DEBUG == 1
		OS_Debug_Print("Killed!");
	#endif
}

// Returns application with given ID, if application does not exists returns Nothing
Application* OS_App_GetAppByID(Byte id)
{
	// Data
	Application *wsk;
	Byte i = 0;
	
	// Check all applications
	for( ; i < Applications.Count; i++)
	{
		// Get application at i
		wsk = (Application*)OS_List_GetAt(&Applications, i);
			
		// Check if id is used
		if(wsk->ID == id)
		{
			// Return application
			return wsk;
		}
	}
	
	// Nothing
	return Nothing;
}

// Returns first free app handle (ID)
Byte OS_App_GetFirstFreeID(void)
{
	// Data
	Byte i, id = 1, can;
	Application *wsk;
	
	// Check if there are any free handles
	if(Applications.Count == 0)
	{
		// First app
		return 1;
	}
	
	// Check all possible ids
	for( ; id <= 255; id++)
	{
		// Check if id is free
		can = 1;
		for(i = 0; i < Applications.Count; i++)
		{
			// Get application at i
			wsk = (Application*)OS_List_GetAt(&Applications, i);
			
			// Check if id is used
			if(wsk->ID == id)
			{
				can = 0;
				break;
			}
		}
		
		if(can)
		{
			return id;
		}
	}
	
	// Error
	return Nothing;
}

// Check if process parent is application or driver and kills it, with process
void OS_KillProcessParent(Process *proc)
{	
	// Check if process parent is application
	if(proc->Flag & OS_PROC_APP > 0)
	{
		// Data
		Byte i;
		Application *app;
		
		// Check all applications
		for(i = 0; i < Applications.Count; i++)
		{
			// Get application at i
			app = (Application*)OS_List_GetAt(&Applications, i);
			
			// Check if its process is the same as given
			if(app->Exe == proc)
			{
				// Kill application
				OS_App_Kill(app);
				
				// End
				return;
			}
		}
	}
}

// Get amount of memnory in bytes that is used by given application
Word OS_App_MemoryUsage(Application *app)
{
	// Data
	Word result;
	
	// Static size
	result = sizeof(Application) + sizeof(File) + app->NameSize;
	
	// Process memory usage
	result += OS_Proc_GetMemoryUsage(app->Exe);
	
	// Return
	return result;
}

// Get amount of free ram memory in bytes
Word OS_FreeRam(void)
{
	Word byteCounter = 0;
	Byte *byteArray;
	/*
	while ( (byteArray = (Byte*) malloc (byteCounter * sizeof(Byte))) != NULL )
	{
		byteCounter++;
		free(byteArray);
	}
	
	// Secondary layer to get a better approximation of free RAM
	if (byteCounter >= 512)
	{
		byteArray = (Byte*) malloc ((byteCounter - 1) * sizeof(Byte));
		byteCounter += OS_FreeRam();
	}

	free(byteArray);*/
	return byteCounter;
}

// Debug Mode
#if OS_DEBUG == 1
	
	
	// Print line to output
	void OS_Debug_Print(const char * format, ... )
	{
		va_list args;
		putchar('\n');
		va_start( args, format );
		vfprintf( stderr, format, args );
		va_end( args );
	}
	
	// Process User Input
	void OS_Debug_Input(void)
	{
		uint8_t tmp = 0;
		Application *wsk;
		
		if(data[0] == 0)
		{
			data[0] = OS_UART_GetByte();
		}
		else
		{
			while(data[1] == 0)
			{
				data[1] =  OS_UART_GetByte();
				tmp++;
				
				if(tmp > 250)
				{
					data[0] = 0;
					data[1] = 0;
					return;
				}
			}
			
			// Some changes
			data[1]--;
			
			// Commands
			switch(data[0])
			{
				// test
				case 1:  
				
				OS_Debug_Print("Recived data: %d", data[1] + 1); 
				
				CGH = ppp->Graphics;
				OS_Execute(ppp);
				
				break;
				
				// exit
				case 2: OS_Exit(); break;
				
				// restart
				case 3: OS_Restart(); break;
				
				// info
				case 4: OS_Debug_Print("Applications count: %d\nProcesses count: %d\nFree memory: %d bytes", Applications.Count, Processes.Count, OS_FreeRam()); OS_Debug_FatInfo(); break;
				
				// apps
				case 5: 
					OS_Debug_Print("Active applications:");
					for(tmp = 0; tmp < Applications.Count; tmp++)
					{
						// Cache
						wsk = (Application*)OS_List_GetAt(&Applications, tmp);
						
						// Print
						OS_Debug_Print("- %d, %s, Memory: %d bytes", wsk->ID , wsk->Exe->Name, OS_App_MemoryUsage(wsk));
					}					
				break;
				
				// kill
				case 6:
					wsk = OS_App_GetAppByID(data[1]);
					if(wsk != Null)
					{
						OS_App_Kill(wsk); 
					}
					else
					{
						OS_Debug_Print("Invalid application id");
					}
					break;
				
				// reg
				case 8:
					wsk = OS_App_GetAppByID(data[1]);
					if(wsk != Null)
					{
						OS_Debug_PrintReg(wsk->Exe); 
					}
					else
					{
						OS_Debug_Print("Invalid application id");
					}
				break;
				
				// mem
				case 9: 
					wsk = OS_App_GetAppByID(data[1]);
					if(wsk != Null)
					{
						OS_Debug_PrintMemory(wsk->Exe); 
					}
					else
					{
						OS_Debug_Print("Invalid application id");
					}
				break;
				
				// Error
				default: OS_Debug_Print("Invalid command"); break;
			}
			
			// Clean buffer
			data[0] = 0;
			data[1] = 0;
		}
	}
	
	// Print fat informations
	void OS_Debug_FatInfo(void)
	{
		// Data
		Byte res;
		
		// Allocate memory
		FatInfo *info = (FatInfo*)OS_Allocate(sizeof(FatInfo));
		
		// Get Fat info
		res = OS_FS_GetFatInfo(info);
		
		// Check if good
		if(res == FS_RESULT_OK)
		{
			// Print
			OS_Debug_Print( "FileSystem: Fat%d\nTotal space: %d MB\nFree space: %d MB", info->FatType, info->TotalSpace / 2048, info->FreeSpace / 2048);
		}
		else
		{
			// Error
			OS_Debug_Print("Cannot get FatInfo. Result: &d", res);
		}
		
		// Clean
		free(info);
	}
	
	// Print all Process registers
	void OS_Debug_PrintReg(Process *exe)
	{
		// Check if is good
		if(exe != Null)
		{
			OS_Debug_Print("Registers of \'%s\', PID=%d", exe->Name, exe->PID);
			OS_Debug_Print("eax: %d", exe->Reg[0]);
			OS_Debug_Print("ebx: %d", exe->Reg[1]);
			OS_Debug_Print("ecx: %d", exe->Reg[2]);
			OS_Debug_Print("edx: %d", exe->Reg[3]);
			OS_Debug_Print("eex: %d", exe->Reg[4]);
			OS_Debug_Print("efx: %d", exe->Reg[5]);
			OS_Debug_Print("egx: %d", exe->Reg[6]);
			OS_Debug_Print("ehx: %d", exe->Reg[7]);		
		}
	}
	
	// Print whole Process memory
	void OS_Debug_PrintMemory(Process *Proc)
	{
		// Data
		Word i, a;
		Byte *x;
		Block *b;
		
		// Check if is good
		if(Proc != Null)
		{
			// Main info
			OS_Debug_Print("Memory of \'%s\', PID=%d", Proc->Name, Proc->PID);
			
			// Show all blocks
			for(a = 0; a < Proc->Blocks.Count; a++)
			{
				// Cache block
				b = (Block*)OS_List_GetAt(&Proc->Blocks, a);
				
				// Show all fields
				for(i = 0; i < b->Size; i++)
				{
					// Get byte
					x=((Byte*)b->Data + i);
					
					// Print
					OS_Debug_Print("Address %#x, value %d (ASCII: %c)", x, *x, (*x < 32) ? '\0' : *x);
				}
			}
		}
	}

#endif
