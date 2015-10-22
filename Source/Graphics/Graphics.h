//////////////////////////////////////////////////////////////////////////////////////
// Mystic OS Copyright 2012-2013 (c) Wojciech Figat
// The software is provided 'as is'. 
// The original author is not responsible for any harm which may result from use of this software or its source code
// See also my 3d game engine --> http://celelej.com
//////////////////////////////////////////////////////////////////////////////////////

#ifndef __OS_GRAPHICS_H__
#define __OS_GRAPHICS_H__

// Include
//#include "..\Kernel\Config.h"
//#include "..\Kernel\Types.h"

// Graphics Device Status
#define	OS_GPU_UNKNOW			0		// Graphics Device was not initializated
#define	OS_GPU_READY			1		// Graphics Device is ready to work
#define	OS_GPU_SUSPENDED		2		// Graphics Device is suspended
#define	OS_GPU_LOST				3		// Graphics Device has lost connection with Mystic
#define	OS_GPU_CRASHED			4		// Graphics Device was crashed or some data is invalid (invalid orientation, screen dimension or data leaks)
#define	OS_GPU_TERMINATED		5		// Graphics Device was terminated

// Screen Orientation
#define OS_GPU_LANDSCAPE		0		// Screen is wide and landscape, good for games
#define OS_GPU_PORTRAIT			1		// Screen is in portrait mode, good for mobile phones

// Graphics Handle Status
#define OS_GPU_HANDLE_NONE		0		// None
#define OS_GPU_HANDLE_FOCUSED	1		// GraphicsHandle is focused. Can render to screen
#define OS_GPU_HANDLE_RENDER	2		// GraphicsHandle needs to redraw

// Graphics Device Driver
#define  INVALID    			0		// Invalid Driver
#define  ILI9320    			1		// 0x9320
#define  ILI9325    			2		// 0x9325
#define  ILI9328    			3		// 0x9328
#define  ILI9331    			4		// 0x9331
#define  SSD1298    			5		// 0x8999
#define  SSD1289    			6		// 0x8989
#define  ST7781     			7		// 0x7783
#define  LGDP4531   			8		// 0x4531
#define  SPFD5408B  			9		// 0x5408
#define  R61505U    			10		// 0x1505 0x0505 
#define  HX8347D    			11		// 0x0047
#define  HX8347A    			12		// 0x0047
#define  LGDP4535   			13		// 0x4535
#define  SSD2119    			14		// 3.5 LCD 0x9919

// GraphicsDevice
typedef struct GraphicsDevice 
{
	
	Byte Status;										// Graphics Device status
	Byte Code;											// LCD Driver Code
	Byte Orientation;									// Current Screen Orientation
	Short Width;										// Screen Width
	Short Height;										// Screen Height
	
} GraphicsDevice;

// Point
typedef struct Point 
{
	
	SShort X;											// X coordinate
	SShort Y;											// Y coordniate
	
} Point;

// GraphicsHandle
typedef struct GraphicsHandle 
{
	
	Byte Status;										// Handle status
	Short X;											// Surface X corner
	Short Y;											// Surface Y corner
	Short Width;										// Surface Width
	Short Height;										// Surface Height
	
} GraphicsHandle;

#define RGB565CONVERT(red, green, blue)\
(uint16_t)( (( red  >> 3 ) << 11 ) | \
(( green >> 2 ) << 5  ) | \
( blue  >> 3 ))

// LCD color
#define White          		0xFFFF
#define Black          		0x0000
#define Grey           		0xF7DE
#define Blue           		0x001F
#define Blue2          		0x051F
#define Red            		0xF800
#define Magenta        		0xF81F
#define Green          		0x07E0
#define Cyan           		0x7FFF
#define Yellow         		0xFFE0
#define System_Front   		0x9F1F
#define System_Back    		0x7DFB
#define System_Grey    		0x738E	 // 112,112,112
#define System_LightGrey    0xE73C	 // 229,229,229
#define Orange RGB565CONVERT(255, 201, 14)

#if OS_GPU_ENABLE

	// Init Graphics Device
	void OS_GPU_Init(Byte Orientation, Short Width, Short Height);

	// Suspend Graphics Device
	void OS_GPU_Suspend(void);

	// Run from being suspended Graphics Device
	void OS_GPU_Run(void);

	// Terminate Graphics Device
	void OS_GPU_Terminate(void);

	// Check connection with graphics device and all data to be sure that everything goes fine (if not flags in status are set)
	void OS_GPU_Check(void);

	// Clear screen with color
	void OS_GPU_Clear(Color color);

	// Get color of the pixel on the screen
	Color OS_GPU_GetPoint(SShort X, SShort Y);

	// Set pixel on the screen to color
	void OS_GPU_SetPoint(SShort X, SShort Y, Color color);
	
	// Draw char on the screen
	void OS_GPU_Print_Char(SShort X, SShort Y, Char ASCII, Color color);

	// Print text on the screen
	void OS_GPU_Print_Text(SShort X, SShort Y, Char* str, Color color);

	// Print word on the screen
	void OS_GPU_Print_Word(SShort X, SShort Y, Word num, Color color);

	// Print sword on the screen
	void OS_GPU_Print_SWord(SShort X, SShort Y, SWord num, Color color);

	// Print float on the screen
	void OS_GPU_Print_Float(SShort X, SShort Y, Float num, Color color);

	// Draw line
	void OS_GPU_DrawLine(SShort x0, SShort y0, SShort x1, SShort y1, Color color);

	// Draw image on the screen
	void OS_GPU_DrawImage(SShort X, SShort Y, Short Width, Short Height, Byte* Image);
	
	// Draw and fill rectangle
	void OS_GPU_DrawRect(SShort X, SShort Y, Short Width, Short Height, Color color);
	void OS_GPU_FillRect(SShort X, SShort Y, Short Width, Short Height, Color color);

	// Draw and fill circle
	void OS_GPU_FillCircle(SShort X, SShort Y, Short Radius, Color color);
	void OS_GPU_DrawCircle(SShort X, SShort Y, Short Radius, Color color);

	// Draw and fill round rectangle
	void OS_GPU_DrawRoundRect(SShort X, SShort Y, Short Width, Short Height, Color color);
	void OS_GPU_FillRoundRect(SShort X, SShort Y, Short Width, Short Height, Color color);

	// Draw horizontal line nad vertical line
	void OS_GPU_DrawHorizontalLine(SShort X, SShort Y, Short Size, Color color);
	void OS_GPU_DrawVerticalLine(SShort X, SShort Y, Short Size, Color color);
	
#endif

#endif
