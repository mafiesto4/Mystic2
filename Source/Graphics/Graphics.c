//////////////////////////////////////////////////////////////////////////////////////
// Mystic OS Copyright 2012-2013 (c) Wojciech Figat
// The software is provided 'as is'. 
// The original author is not responsible for any harm which may result from use of this software or its source code
// See also my 3d game engine --> http://celelej.com
//////////////////////////////////////////////////////////////////////////////////////

// Include
#include "..\Kernel\Types.h"
#include "Graphics.h" 
#include "AsciiLib.h"
#include <stdio.h>

#if OS_GPU_ENABLE

// Current Graphics Device (it is not exactly GPU but service that has moreover the same functions)
extern GraphicsDevice GPU;

// Print char on the screen
void OS_GPU_Print_Char(SShort X, SShort Y, Char ASCII, Color color)
{
	// Data
	SShort i, j;
    Char buffer[16], tmp_char;
	
	// Get ascii code based on char from font
    GetASCIICode(buffer, ASCII);
	
	// Draw 16 rows
    for(i = 0; i < 16; i++)
    {
		// Cache char
        tmp_char = buffer[i];
		
		// Draw row
        for(j = 0; j < 8; j++)
        {
			// CHeck if draw point
            if((tmp_char >> 7 - j) & 0x01 == 0x01)
            {
				// Draw point
                OS_GPU_SetPoint(X + j, Y + i, color);
            }
        }
    }
}

// Print text on the screen
void OS_GPU_Print_Text(SShort X, SShort Y, Char *str, Color color)
{
	// Data
    Char TempChar;
	
	// Draw chars until end 0 char
    while (*str != Nothing)
    {
		// Cache
        TempChar = *str++;
        
		// Print char
		OS_GPU_Print_Char(X, Y, TempChar, color); 
		
		// Move
        X += 8;
    }
}

// Print word on the screen
void OS_GPU_Print_Word(SShort X, SShort Y, Word num, Color color)
{
	Char st[10];
	sprintf(st, "%d", num);
	
	OS_GPU_Print_Text(X, Y, st, color);
}

// Print sword on the screen
void OS_GPU_Print_SWord(SShort X, SShort Y, SWord num, Color color)
{
	Char st[11];
	sprintf(st, "%d", num);
	
	OS_GPU_Print_Text(X, Y, st, color);
}

// Print float on the screen
void OS_GPU_Print_Float(SShort X, SShort Y, Float num, Color color)
{
	Char st[11];
	sprintf(st, "%f", num);
	
	OS_GPU_Print_Text(X, Y, st, color);
}

// Draw line
void OS_GPU_DrawLine(SShort x0, SShort y0, SShort x1, SShort y1, Color color)
{
	// Data
    SShort dx,dy, temp;
	
    if(x0 > x1)
    {
	    temp = x1;
		x1 = x0;
		x0 = temp;   
    }
    if(y0 > y1)
    {
		temp = y1;
		y1 = y0;
		y0 = temp;   
    }
  
	dx = x1-x0;
	dy = y1-y0;

    if(dx == 0)
    {
        do
        { 
            OS_GPU_SetPoint(x0, y0, color);
            y0++;
        }
        while(y1 >= y0); 
		return; 
    }
	
    if(dy == 0) 
    {
        do
        {
            OS_GPU_SetPoint(x0, y0, color);
            x0++;
        }
        while(x1 >= x0); 
		return;
    }
	
    if(dx > dy)
    {
		temp = 2 * dy - dx;    

		while(x0 != x1)
		{
			OS_GPU_SetPoint(x0, y0, color);
			
			x0++;
			
			if(temp > 0)
			{
				y0++;
				temp += 2 * dy - 2 * dx; 
			}
			else         
			{
				temp += 2 * dy; 
			}       
		}

		OS_GPU_SetPoint(x0, y0, color);
    }  
    else
    {
	    temp = 2 * dx - dy;  
		
        while(y0 != y1)
        {
	 	    OS_GPU_SetPoint(x0, y0, color);
			
            y0++;
			
            if(temp > 0)  
            {
                x0++;               
                temp+= 2 * dy - 2 * dx; 
            }
            else
			{
                temp += 2 * dy;
			}
        }
		
        OS_GPU_SetPoint(x0,y0,color);
	}
}

// Draw rectangle
void OS_GPU_DrawRect(SShort X, SShort Y, Short Width, Short Height, Color color)
{
	// Draw 2 horizontal lines
	OS_GPU_DrawHorizontalLine(X, Y, Width, color);
	OS_GPU_DrawHorizontalLine(X, Y + Height, Width, color);
	
	// Draw 2 vertical lines
	OS_GPU_DrawVerticalLine(X, Y, Height, color);
	OS_GPU_DrawVerticalLine(X + Width, Y, Height, color);
}

// Fill rectangle
void OS_GPU_FillRect(SShort X, SShort Y, Short Width, Short Height, Color color)
{
	// Draw horizontal lines until end
	while(Height > 0)
	{
		// Draw horizontal line
		OS_GPU_DrawHorizontalLine(X, Y + Height, Width, color);
		
		// Move
		Height--;
	}
}

// Draw circle
void OS_GPU_DrawCircle(SShort X, SShort Y, Short Radius, Color color)
{
	// Data
	SShort f = 1 - Radius;
  	SShort ddF_x = 1;
 	SShort ddF_y = -2 * Radius;
  	SShort x = 0;
  	SShort y = Radius;
	
	// Draw 4 easy to calculate points
  	OS_GPU_SetPoint(X, Y + Radius, color);
  	OS_GPU_SetPoint(X, Y - Radius, color);
  	OS_GPU_SetPoint(X + Radius, Y, color);
  	OS_GPU_SetPoint(X - Radius, Y, color);

	// Draw rest of points
	while(x < y)
	{
		if(f >= 0) 
		{
			y--;
			ddF_y += 2;
			f += ddF_y;
		}
		
		x++;
		ddF_x += 2;
		f += ddF_x;
		
		OS_GPU_SetPoint(X + x, Y + y, color);
		OS_GPU_SetPoint(X - x, Y + y, color);
		OS_GPU_SetPoint(X + x, Y - y, color);
		OS_GPU_SetPoint(X - x, Y - y, color);
		OS_GPU_SetPoint(X + y, Y + x, color);
		OS_GPU_SetPoint(X - y, Y + x, color);
		OS_GPU_SetPoint(X + y, Y - x, color);
		OS_GPU_SetPoint(X - y, Y - x, color);
	}
}

// Fill circle
void OS_GPU_FillCircle(SShort X, SShort Y, Short Radius, Color color)
{
	// Data
	SShort tmp1 = -Radius;
	SShort tmp2;
	
	// Fill rectangle, and check if points are inside circle
	for( ; tmp1 <= Radius; tmp1++) 
	{
		for( tmp2 = -Radius; tmp2 <= Radius; tmp2++) 
		{
			// Check if point is inside circle
			if(tmp2 * tmp2 + tmp1 * tmp1 <= Radius * Radius) 
			{
				// Set point
				OS_GPU_SetPoint(tmp1 + X, tmp2 + Y, color);
			}
		}
	}
}

// Draw round rectangle
void OS_GPU_DrawRoundRect(SShort X, SShort Y, Short Width, Short Height, Color color)
{
	// Draw corners
	OS_GPU_SetPoint(X + 1, Y + 1, color);
	OS_GPU_SetPoint(X + Width - 1, Y + 1, color);
	OS_GPU_SetPoint(X + 1, Y + Height - 1, color);
	OS_GPU_SetPoint(X + Width - 1, Y + Height - 1, color);
	
	// Draw horizontal lines
	OS_GPU_DrawHorizontalLine(X + 1, Y, Width - 2, color);
	OS_GPU_DrawHorizontalLine(X + 1, Y + Height, Width - 2, color);
	
	// Draw vertical lines
	OS_GPU_DrawVerticalLine(X, Y + 1, Height - 2, color);
	OS_GPU_DrawVerticalLine(X + Width, Y + 1, Height - 2, color);
}

// Fill reound rectangle
void OS_GPU_FillRoundRect(SShort X, SShort Y, Short Width, Short Height, Color color)
{
	// Data
	SShort i =0;
	
	// Loop pixels and drow horizontal lines
	for ( ; i < (Height / 2) + 1; i++)
	{
		switch(i)
		{
			case 0:
				OS_GPU_DrawHorizontalLine(X + 2, Y + i, Width - 4, color);
				OS_GPU_DrawHorizontalLine(X + 2, Y + Height - i, Width - 4, color);
			break;
			
			case 1:
				OS_GPU_DrawHorizontalLine(X + 1, Y + i, Width - 2, color);
				OS_GPU_DrawHorizontalLine(X + 1, Y + Height - i, Width - 2, color);		
			break;
			
			default:
				OS_GPU_DrawHorizontalLine(X, Y + i, Width, color);
				OS_GPU_DrawHorizontalLine(X, Y + Height - i, Width, color);
		}
	}
}

// Draw horizontal line
void OS_GPU_DrawHorizontalLine(SShort X, SShort Y, Short Size, Color color)
{
	while(Size > 0)
	{
		OS_GPU_SetPoint(X + Size, Y, color);
		Size--;
	}
}

// Draw vertical line
void OS_GPU_DrawVerticalLine(SShort X, SShort Y, Short Size, Color color)
{
	while(Size > 0)
	{
		OS_GPU_SetPoint(X, Y + Size, color);
		Size--;
	}
}

#endif
