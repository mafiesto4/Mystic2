//////////////////////////////////////////////////////////////////////////////////////
// Mystic OS Copyright 2012-2013 (c) Wojciech Figat
// The software is provided 'as is'. 
// The original author is not responsible for any harm which may result from use of this software or its source code
// See also my 3d game engine --> http://celelej.com
//////////////////////////////////////////////////////////////////////////////////////

// Include
#include "..\Kernel\Config.h"
#include "..\Kernel\Types.h"
#include "Graphics.h"

// Current Graphics Device (it is not exactly GPU but service that has moreover the same functions)
extern GraphicsDevice GPU;

// Current Graphics Handle
volatile GraphicsHandle *CGH;

#if OS_GPU_ENABLE == 1

#if OS_DEST_PLATFORM == OS_LPC17xx

	// Library
	#include "..\Platform\LPC17xx\LPC17xx.h"

	// Pins
	#define PIN_EN		(1 << 19)
	#define PIN_LE		(1 << 20)
	#define PIN_DIR		(1 << 21)
	#define PIN_CS      (1 << 22)
	#define PIN_RS		(1 << 23)
	#define PIN_WR		(1 << 24)
	#define PIN_RD		(1 << 25)   

	// Functions
	#define LCD_EN(x)   ((x) ? (LPC_GPIO0->FIOSET = PIN_EN) : (LPC_GPIO0->FIOCLR = PIN_EN));
	#define LCD_LE(x)   ((x) ? (LPC_GPIO0->FIOSET = PIN_LE) : (LPC_GPIO0->FIOCLR = PIN_LE));
	#define LCD_DIR(x)  ((x) ? (LPC_GPIO0->FIOSET = PIN_DIR) : (LPC_GPIO0->FIOCLR = PIN_DIR));
	#define LCD_CS(x)   ((x) ? (LPC_GPIO0->FIOSET = PIN_CS) : (LPC_GPIO0->FIOCLR = PIN_CS));
	#define LCD_RS(x)   ((x) ? (LPC_GPIO0->FIOSET = PIN_RS) : (LPC_GPIO0->FIOCLR = PIN_RS));
	#define LCD_WR(x)   ((x) ? (LPC_GPIO0->FIOSET = PIN_WR) : (LPC_GPIO0->FIOCLR = PIN_WR));
	#define LCD_RD(x)   ((x) ? (LPC_GPIO0->FIOSET = PIN_RD) : (LPC_GPIO0->FIOCLR = PIN_RD));

	static Inline void LCD_Send(uint16_t byte) 
	{
		LPC_GPIO2->FIODIR |= 0xFF;          // P2.0...P2.7 Output
		LCD_DIR(1)		   				  	// Interface A->B
		LCD_EN(0)	                       	// Enable 2A->2B
		LPC_GPIO2->FIOPIN =  byte;         	// Write D0..D7
		LCD_LE(1)                         	
		LCD_LE(0)							// Latch D0..D7
		LPC_GPIO2->FIOPIN =  byte >> 8;     // Write D8..D15
	}

	void wait_delay(int count)
	{
		while(count--);
	}

	static Inline uint16_t LCD_Read (void) 
	{
		uint16_t value;

		LPC_GPIO2->FIODIR &= ~(0xFF);              	// P2.0...P2.7 Input
		LCD_DIR(0);		   				           	// Interface B->A
		LCD_EN(0);	                               	// Enable 2B->2A
		wait_delay(80);							   	// Delay some times
		value = LPC_GPIO2->FIOPIN0;                 // Read D8..D15
		LCD_EN(1);	                               	// Enable 1B->1A
		wait_delay(80);							   	// Delay some times
		value = (value << 8) | LPC_GPIO2->FIOPIN0;	// Read D0..D7
		LCD_DIR(1);
		
		return  value;
	}

	static Inline void LCD_WriteIndex(uint16_t index)
	{
		LCD_CS(0);
		LCD_RS(0);
		LCD_RD(1);
		LCD_Send( index ); 
		wait_delay(70);	
		LCD_WR(0);  
		wait_delay(1);
		LCD_WR(1);
		LCD_CS(1);
	}

	static Inline void LCD_WriteData(uint16_t data)
	{				
		LCD_CS(0);
		LCD_RS(1);   
		LCD_Send( data );
		LCD_WR(0);     
		wait_delay(1);
		LCD_WR(1);
		LCD_CS(1);
	}

	static Inline uint16_t LCD_ReadData(void)
	{ 
		uint16_t value;

		LCD_CS(0);
		LCD_RS(1);
		LCD_WR(1);
		LCD_RD(0);
		value = LCD_Read();

		LCD_RD(1);
		LCD_CS(1);

		return value;
	}

	static Inline void LCD_WriteReg(uint16_t LCD_Reg,uint16_t LCD_RegValue)
	{ 
		// Write 16-bit Index
		LCD_WriteIndex(LCD_Reg);     
			
		// Write 16-bit Reg
		LCD_WriteData(LCD_RegValue);  
	}


	static __inline uint16_t LCD_ReadReg(uint16_t LCD_Reg)
	{
		// Write 16-bit Index
		LCD_WriteIndex(LCD_Reg);
		
		// Read 16-bit Reg
		return LCD_ReadData();
	}

	static void LCD_SetCursor(uint16_t X, uint16_t Y)
	{
		if( GPU.Orientation == OS_GPU_LANDSCAPE )
		{
			uint16_t temp = X;
			X = Y;
			Y = ( GPU.Width - 1 ) - temp;  
		}
		
		switch( GPU.Code )
		{ 
			// 0x8999 0x8989
			 case SSD1298: 	 
			 case SSD1289:
				 
				  LCD_WriteReg(0x004e, X);      
				  LCD_WriteReg(0x004f, Y);
			 
				  break;  
			 
			// 0x0047 0x0047
			 case HX8347A:
			 case HX8347D:
				 
				  LCD_WriteReg(0x02, X>>8);                                                  
				  LCD_WriteReg(0x03, X);  
				  LCD_WriteReg(0x06, Y>>8);                         
				  LCD_WriteReg(0x07, Y);
			 
				  break;    
			 
			 // 3.5 LCD 0x9919 
			 case SSD2119:
				  break; 
			 
			 // 0x9320 0x9325 0x9328 0x9331 0x5408 0x1505 0x0505 0x7783 0x4531 0x4535
			 default:
				 
				  LCD_WriteReg(0x0020, X);     
				  LCD_WriteReg(0x0021, Y);
			 
				  break;
		  }
	}


	static void delay_ms(uint16_t ms)    
	{ 
		uint16_t i,j; 
		for( i = 0; i < ms; i++ )
		{ 
			for( j = 0; j < 1141; j++ );
		}
	}
	
#else

		#error "This platform does not support Graphics"
		
#endif

//RRRRRGGGGGGBBBBB to BBBBBGGGGGGRRRRR
static uint16_t LCD_BGR2RGB(uint16_t color)
{
	uint16_t  r, g, b, rgb;
	
	b = ( color>>0 )  & 0x1f;
	g = ( color>>5 )  & 0x3f;
	r = ( color>>11 ) & 0x1f;
	
	rgb =  (b<<11) + (g<<5) + (r<<0);
	
	return( rgb );
}

// Run from being suspended Graphics Device
void OS_GPU_Run(void)
{
	#if OS_DEST_PLATFORM == OS_LPC17xx
	
	#else

		#error "This platform does not support Graphics"
		
	#endif
}

// Check connection with graphics device and all data to be sure that everything goes fine (if not flags in status are set)
void OS_GPU_Check(void)
{
	#if OS_DEST_PLATFORM == OS_LPC17xx
	
	#else

		#error "This platform does not support Graphics"
		
	#endif
}

// Suspend Graphics Device
void OS_GPU_Suspend(void)
{
	#if OS_DEST_PLATFORM == OS_LPC17xx
	
	#else

		#error "This platform does not support Graphics"
		
	#endif
}

// Terminate Graphics Device
void OS_GPU_Terminate(void)
{
	#if OS_DEST_PLATFORM == OS_LPC17xx
	
	#else

		#error "This platform does not support Graphics"
		
	#endif
}

// Clear screen with color
void OS_GPU_Clear(Color color)
{
	// Check if GraphicsHandle is valid and GPU is ready
	if(CGH != Nothing && GPU.Status == OS_GPU_READY)
	{
		#if OS_DEST_PLATFORM == OS_LPC17xx
		
			// Data
			Word index;
			
			if(GPU.Code == HX8347D || GPU.Code == HX8347A)
			{
				LCD_WriteReg(0x02, 0x00);                                                  
				LCD_WriteReg(0x03, 0x00);          
				LCD_WriteReg(0x04, 0x00);                           
				LCD_WriteReg(0x05, 0xEF);              
				LCD_WriteReg(0x06, 0x00);                  
				LCD_WriteReg(0x07, 0x00);              
				LCD_WriteReg(0x08, 0x01);              
				LCD_WriteReg(0x09, 0x3F);
			}
			else
			{
				LCD_SetCursor(0, 0); 
			}
			
			LCD_CS(0);
			LCD_WriteIndex(0x0022);
			
			for(index = 0; index < GPU.Width * GPU.Height; index++)
			{
				LCD_WriteData(color);
			}
			
			LCD_CS(1);
		
		#else

			#error "This platform does not support Graphics"
			
		#endif
	}
}

// Get color of the pixel on the screen
Color OS_GPU_GetPoint(SShort X, SShort Y)
{
	#if OS_DEST_PLATFORM == OS_LPC17xx
	
		Color dummy;
		
		LCD_SetCursor(X,Y);
		LCD_CS(0);
		LCD_WriteIndex(0x0022);  
		
		switch( GPU.Code )
		{
			case ST7781:
			case LGDP4531:
			case LGDP4535:
			case SSD1289:
			case SSD1298:
				 dummy = LCD_ReadData();
				 dummy = LCD_ReadData(); 
				 LCD_CS(1);	
				 return  dummy;	 
			
			case HX8347A:
			case HX8347D:
				 {
					uint8_t red,green,blue;
					red = LCD_ReadData()>>3; 
					green = LCD_ReadData()>>3; 
					blue = LCD_ReadData()>>2; 
					dummy = ( green << 11 ) | (blue << 5 ) | red;
				 }
				 LCD_CS(1);	
				 return  dummy;

			default:	// 0x9320 0x9325 0x9328 0x9331 0x5408 0x1505 0x0505 0x9919
				 dummy = LCD_ReadData();
				 dummy = LCD_ReadData(); 
				 LCD_CS(1);	
				 return  LCD_BGR2RGB( dummy );
		}
	
	#else

		#error "This platform does not support Graphics"
		
	#endif
}

// Set point on the screen
void OS_GPU_SetPoint(SShort X, SShort Y, Color color)
{
	// Check if GraphicsHandle is valid and GPU is ready and point is in the Graphics Handle Surface
	if(CGH != Nothing && GPU.Status == OS_GPU_READY && X < CGH->Width && Y < CGH->Height)
	{
		// Translate XY to CGH Surface
		X += CGH->X;
		Y += CGH->Y;
		
		// Check if point is valid
		if(X >= 0 && Y >= 0)
		{
			#if OS_DEST_PLATFORM == OS_LPC17xx
			
				LCD_SetCursor(X, Y);
				LCD_WriteReg(0x0022, color);
				
			#else

				#error "This platform does not support Graphics"
				
			#endif
		}
	}
}

// Draw image on the screen
void OS_GPU_DrawImage(SShort X, SShort Y, Short Width, Short Height, Byte* Image)
{
	// Check if GraphicsHandle is valid and GPU is ready and point is in the Current Graphics Handle Surface
	if(CGH != Nothing && GPU.Status == OS_GPU_READY && X < CGH->Width && Y < CGH->Height)
	{
		// Temporary data
		SShort offset, x, y, a, b;
		
		// Translate XY to CGH Surface
		X += CGH->X;
		Y += CGH->Y;
		
		// Render image
		for(y = 0; y < Height; y++)
		{
			// Cache pixel offset
			offset = y * Width;
			
			// Draw horizontal strip
			for(x = 0; x < Width; x++)
			{
				// Calculate point location
				a = X + x;
				b = Y + y;
				
				// Check if point is valid
				if(a >= 0 && b >= 0)
				{
					#if OS_DEST_PLATFORM == OS_LPC17xx
						
						LCD_SetCursor(a, b);
						LCD_WriteReg(0x0022, Image[offset + x]);
						
					#else
						
						#error "This platform does not support Graphics"
						
					#endif
				}
			}
		}
	}
}

// Init Graphics Device
void OS_GPU_Init(Byte Orientation, Short Width, Short Height)
{
	// Data
	uint16_t DeviceCode;

	// Set up basic GPU data
	GPU.Orientation = Orientation;
	GPU.Width = Width;
	GPU.Height = Height;
	GPU.Code = INVALID;
	GPU.Status = OS_GPU_UNKNOW;
	
	#if OS_DEST_PLATFORM == OS_LPC17xx
	
		// Configure the LCD Control pins
		// EN = P0.19 , LE = P0.20 , DIR = P0.21 , CS = P0.22 , RS = P0.23 , RS = P0.23
		// RS = P0.23 , WR = P0.24 , RD = P0.25 , DB[0.7] = P2.0...P2.7 , DB[8.15]= P2.0...P2.7
		LPC_GPIO0->FIODIR   |= 0x03f80000;
		LPC_GPIO0->FIOSET    = 0x03f80000;

		// Get device code
		DeviceCode = LCD_ReadReg(0x0000);
		
		if(DeviceCode == 0x9325 || DeviceCode == 0x9328)	
		{
			GPU.Code = ILI9325;
			
			LCD_WriteReg(0x00e7, 0x0010);      
			LCD_WriteReg(0x0000, 0x0001);
			LCD_WriteReg(0x0001, 0x0100);     
			LCD_WriteReg(0x0002, 0x0700);
			LCD_WriteReg(0x0003, (1<<12)|(1<<5)|(1<<4)|(0<<3) );
			LCD_WriteReg(0x0004, 0x0000);                                   
			LCD_WriteReg(0x0008, 0x0207);	           
			LCD_WriteReg(0x0009, 0x0000);         
			LCD_WriteReg(0x000a, 0x0000);      
			LCD_WriteReg(0x000c, 0x0001);        
			LCD_WriteReg(0x000d, 0x0000); 			        
			LCD_WriteReg(0x000f, 0x0000);
			LCD_WriteReg(0x0010, 0x0000);   
			LCD_WriteReg(0x0011, 0x0007);
			LCD_WriteReg(0x0012, 0x0000);                                                                 
			LCD_WriteReg(0x0013, 0x0000);
			delay_ms(50);
			LCD_WriteReg(0x0010, 0x1590);   
			LCD_WriteReg(0x0011, 0x0227);
			delay_ms(50);
			LCD_WriteReg(0x0012, 0x009c);		
			delay_ms(50);
			LCD_WriteReg(0x0013, 0x1900);   
			LCD_WriteReg(0x0029, 0x0023);
			LCD_WriteReg(0x002b, 0x000e);
			delay_ms(50);		
			LCD_WriteReg(0x0020, 0x0000);                                                            
			LCD_WriteReg(0x0021, 0x0000);           
			delay_ms(50);		
			LCD_WriteReg(0x0030, 0x0007); 
			LCD_WriteReg(0x0031, 0x0707);   
			LCD_WriteReg(0x0032, 0x0006);
			LCD_WriteReg(0x0035, 0x0704);
			LCD_WriteReg(0x0036, 0x1f04); 
			LCD_WriteReg(0x0037, 0x0004);
			LCD_WriteReg(0x0038, 0x0000);        
			LCD_WriteReg(0x0039, 0x0706);     
			LCD_WriteReg(0x003c, 0x0701);
			LCD_WriteReg(0x003d, 0x000f);
			delay_ms(50);		
			LCD_WriteReg(0x0050, 0x0000);        
			LCD_WriteReg(0x0051, 0x00ef);   
			LCD_WriteReg(0x0052, 0x0000);     
			LCD_WriteReg(0x0053, 0x013f);
			LCD_WriteReg(0x0060, 0xa700);        
			LCD_WriteReg(0x0061, 0x0001); 
			LCD_WriteReg(0x006a, 0x0000);
			LCD_WriteReg(0x0080, 0x0000);
			LCD_WriteReg(0x0081, 0x0000);
			LCD_WriteReg(0x0082, 0x0000);
			LCD_WriteReg(0x0083, 0x0000);
			LCD_WriteReg(0x0084, 0x0000);
			LCD_WriteReg(0x0085, 0x0000);
			LCD_WriteReg(0x0090, 0x0010);     
			LCD_WriteReg(0x0092, 0x0000);  
			LCD_WriteReg(0x0093, 0x0003);
			LCD_WriteReg(0x0095, 0x0110);
			LCD_WriteReg(0x0097, 0x0000);        
			LCD_WriteReg(0x0098, 0x0000);   
			LCD_WriteReg(0x0007, 0x0133);
			LCD_WriteReg(0x0020, 0x0000);                                                          
			LCD_WriteReg(0x0021, 0x0000);     
		}
		else if( DeviceCode == 0x9320 || DeviceCode == 0x9300 )
		{
			GPU.Code = ILI9320;
			
			LCD_WriteReg(0x00, 0x0000);
			LCD_WriteReg(0x01, 0x0100);
			LCD_WriteReg(0x02, 0x0700);
			LCD_WriteReg(0x03, 0x1018);
			LCD_WriteReg(0x04, 0x0000);
			LCD_WriteReg(0x08, 0x0202);
			LCD_WriteReg(0x09, 0x0000);
			LCD_WriteReg(0x0a, 0x0000);
			LCD_WriteReg(0x0c, (1<<0));
			LCD_WriteReg(0x0d, 0x0000);
			LCD_WriteReg(0x0f, 0x0000);
			delay_ms(100);	
			LCD_WriteReg(0x07, 0x0101);
			delay_ms(100);		
			LCD_WriteReg(0x10, (1<<12)|(0<<8)|(1<<7)|(1<<6)|(0<<4));
			LCD_WriteReg(0x11, 0x0007);
			LCD_WriteReg(0x12, (1<<8)|(1<<4)|(0<<0));
			LCD_WriteReg(0x13, 0x0b00);
			LCD_WriteReg(0x29, 0x0000);
			LCD_WriteReg(0x2b, (1<<14)|(1<<4));
			LCD_WriteReg(0x50, 0);
			LCD_WriteReg(0x51, 239);
			LCD_WriteReg(0x52, 0);
			LCD_WriteReg(0x53, 319);
			LCD_WriteReg(0x60, 0x2700);
			LCD_WriteReg(0x61, 0x0001);
			LCD_WriteReg(0x6a, 0x0000);
			LCD_WriteReg(0x80, 0x0000);
			LCD_WriteReg(0x81, 0x0000);
			LCD_WriteReg(0x82, 0x0000);
			LCD_WriteReg(0x83, 0x0000);
			LCD_WriteReg(0x84, 0x0000);
			LCD_WriteReg(0x85, 0x0000);
			LCD_WriteReg(0x90, (0<<7)|(16<<0));
			LCD_WriteReg(0x92, 0x0000);
			LCD_WriteReg(0x93, 0x0001);
			LCD_WriteReg(0x95, 0x0110);
			LCD_WriteReg(0x97, (0<<8));	
			LCD_WriteReg(0x98, 0x0000);
			LCD_WriteReg(0x07, 0x0173);
		}
		else if( DeviceCode == 0x9331 )
		{
			GPU.Code = ILI9331;
			
			LCD_WriteReg(0x00E7, 0x1014);
			LCD_WriteReg(0x0001, 0x0100);
			LCD_WriteReg(0x0002, 0x0200);
			LCD_WriteReg(0x0003, 0x1030);
			LCD_WriteReg(0x0008, 0x0202);
			LCD_WriteReg(0x0009, 0x0000);
			LCD_WriteReg(0x000A, 0x0000);
			LCD_WriteReg(0x000C, 0x0000);
			LCD_WriteReg(0x000D, 0x0000);
			LCD_WriteReg(0x000F, 0x0000);
			LCD_WriteReg(0x0010, 0x0000);
			LCD_WriteReg(0x0011, 0x0007);
			LCD_WriteReg(0x0012, 0x0000);
			LCD_WriteReg(0x0013, 0x0000);
			delay_ms(200);		
			LCD_WriteReg(0x0010, 0x1690);
			LCD_WriteReg(0x0011, 0x0227);
			delay_ms(50);		
			LCD_WriteReg(0x0012, 0x000C);
			delay_ms(50);		
			LCD_WriteReg(0x0013, 0x0800);
			LCD_WriteReg(0x0029, 0x0011);
			LCD_WriteReg(0x002B, 0x000B);
			delay_ms(50);		
			LCD_WriteReg(0x0020, 0x0000);
			LCD_WriteReg(0x0021, 0x0000);
			LCD_WriteReg(0x0030, 0x0000);
			LCD_WriteReg(0x0031, 0x0106);
			LCD_WriteReg(0x0032, 0x0000);
			LCD_WriteReg(0x0035, 0x0204);
			LCD_WriteReg(0x0036, 0x160A);
			LCD_WriteReg(0x0037, 0x0707);
			LCD_WriteReg(0x0038, 0x0106);
			LCD_WriteReg(0x0039, 0x0707);
			LCD_WriteReg(0x003C, 0x0402);
			LCD_WriteReg(0x003D, 0x0C0F);
			LCD_WriteReg(0x0050, 0x0000);
			LCD_WriteReg(0x0051, 0x00EF);
			LCD_WriteReg(0x0052, 0x0000);
			LCD_WriteReg(0x0053, 0x013F);
			LCD_WriteReg(0x0060, 0x2700);
			LCD_WriteReg(0x0061, 0x0001);
			LCD_WriteReg(0x006A, 0x0000);
			LCD_WriteReg(0x0080, 0x0000);
			LCD_WriteReg(0x0081, 0x0000);
			LCD_WriteReg(0x0082, 0x0000);
			LCD_WriteReg(0x0083, 0x0000);
			LCD_WriteReg(0x0084, 0x0000);
			LCD_WriteReg(0x0085, 0x0000);
			LCD_WriteReg(0x0090, 0x0010);
			LCD_WriteReg(0x0092, 0x0600);
			LCD_WriteReg(0x0007, 0x0021);		
			delay_ms(50);		
			LCD_WriteReg(0x0007, 0x0061);
			delay_ms(50);		
			LCD_WriteReg(0x0007, 0x0133);
		}
		else if( DeviceCode == 0x9919 )
		{
			GPU.Code = SSD2119;
			
			LCD_WriteReg(0x28, 0x0006);
			LCD_WriteReg(0x00, 0x0001);		
			LCD_WriteReg(0x10, 0x0000);		
			LCD_WriteReg(0x01, 0x72ef);
			LCD_WriteReg(0x02, 0x0600);
			LCD_WriteReg(0x03, 0x6a38);	
			LCD_WriteReg(0x11, 0x6874);
			LCD_WriteReg(0x0f, 0x0000);
			LCD_WriteReg(0x0b, 0x5308);
			LCD_WriteReg(0x0c, 0x0003);
			LCD_WriteReg(0x0d, 0x000a);
			LCD_WriteReg(0x0e, 0x2e00);  
			LCD_WriteReg(0x1e, 0x00be);
			LCD_WriteReg(0x25, 0x8000);
			LCD_WriteReg(0x26, 0x7800);
			LCD_WriteReg(0x27, 0x0078);
			LCD_WriteReg(0x4e, 0x0000);
			LCD_WriteReg(0x4f, 0x0000);
			LCD_WriteReg(0x12, 0x08d9);
			LCD_WriteReg(0x30, 0x0000);
			LCD_WriteReg(0x31, 0x0104);	 
			LCD_WriteReg(0x32, 0x0100);	
			LCD_WriteReg(0x33, 0x0305);	
			LCD_WriteReg(0x34, 0x0505);	 
			LCD_WriteReg(0x35, 0x0305);	
			LCD_WriteReg(0x36, 0x0707);	
			LCD_WriteReg(0x37, 0x0300);	
			LCD_WriteReg(0x3a, 0x1200);	
			LCD_WriteReg(0x3b, 0x0800);		 
			LCD_WriteReg(0x07, 0x0033);
		}
		else if( DeviceCode == 0x1505 || DeviceCode == 0x0505 )
		{
			GPU.Code = R61505U;
			
			LCD_WriteReg(0x0007, 0x0000);
			delay_ms(50);		
			LCD_WriteReg(0x0012, 0x011C);
			LCD_WriteReg(0x00A4, 0x0001);
			LCD_WriteReg(0x0008, 0x000F);
			LCD_WriteReg(0x000A, 0x0008);
			LCD_WriteReg(0x000D, 0x0008);    
			LCD_WriteReg(0x0030, 0x0707);
			LCD_WriteReg(0x0031, 0x0007); 
			LCD_WriteReg(0x0032, 0x0603); 
			LCD_WriteReg(0x0033, 0x0700); 
			LCD_WriteReg(0x0034, 0x0202); 
			LCD_WriteReg(0x0035, 0x0002); 
			LCD_WriteReg(0x0036, 0x1F0F);
			LCD_WriteReg(0x0037, 0x0707); 
			LCD_WriteReg(0x0038, 0x0000); 
			LCD_WriteReg(0x0039, 0x0000); 
			LCD_WriteReg(0x003A, 0x0707); 
			LCD_WriteReg(0x003B, 0x0000); 
			LCD_WriteReg(0x003C, 0x0007); 
			LCD_WriteReg(0x003D, 0x0000); 
			delay_ms(50);		
			LCD_WriteReg(0x0007, 0x0001);
			LCD_WriteReg(0x0017, 0x0001);
			delay_ms(50);		
			LCD_WriteReg(0x0010, 0x17A0); 
			LCD_WriteReg(0x0011, 0x0217);
			LCD_WriteReg(0x0012, 0x011E);
			LCD_WriteReg(0x0013, 0x0F00);
			LCD_WriteReg(0x002A, 0x0000);  
			LCD_WriteReg(0x0029, 0x000A);
			LCD_WriteReg(0x0012, 0x013E);
			LCD_WriteReg(0x0050, 0x0000);
			LCD_WriteReg(0x0051, 0x00EF); 
			LCD_WriteReg(0x0052, 0x0000); 
			LCD_WriteReg(0x0053, 0x013F); 
			LCD_WriteReg(0x0060, 0x2700); 
			LCD_WriteReg(0x0061, 0x0001); 
			LCD_WriteReg(0x006A, 0x0000); 
			LCD_WriteReg(0x0080, 0x0000); 
			LCD_WriteReg(0x0081, 0x0000); 
			LCD_WriteReg(0x0082, 0x0000); 
			LCD_WriteReg(0x0083, 0x0000); 
			LCD_WriteReg(0x0084, 0x0000); 
			LCD_WriteReg(0x0085, 0x0000); 
			LCD_WriteReg(0x0090, 0x0013);	
			LCD_WriteReg(0x0092, 0x0300); 
			LCD_WriteReg(0x0093, 0x0005); 
			LCD_WriteReg(0x0095, 0x0000); 
			LCD_WriteReg(0x0097, 0x0000); 
			LCD_WriteReg(0x0098, 0x0000); 
			LCD_WriteReg(0x0001, 0x0100); 
			LCD_WriteReg(0x0002, 0x0700); 
			LCD_WriteReg(0x0003, 0x1030); 
			LCD_WriteReg(0x0004, 0x0000); 
			LCD_WriteReg(0x000C, 0x0000); 
			LCD_WriteReg(0x000F, 0x0000); 
			LCD_WriteReg(0x0020, 0x0000); 
			LCD_WriteReg(0x0021, 0x0000); 
			LCD_WriteReg(0x0007, 0x0021); 
			delay_ms(200);		
			LCD_WriteReg(0x0007, 0x0061); 
			delay_ms(200);	
			LCD_WriteReg(0x0007, 0x0173); 
		}							 
		else if( DeviceCode == 0x8989 )
		{
			GPU.Code = SSD1289;
			
			LCD_WriteReg(0x0000, 0x0001);    delay_ms(50);
			LCD_WriteReg(0x0003, 0xA8A4);    delay_ms(50);   
			LCD_WriteReg(0x000C, 0x0000);    delay_ms(50);   
			LCD_WriteReg(0x000D, 0x080C);    delay_ms(50);   
			LCD_WriteReg(0x000E, 0x2B00);    delay_ms(50);   
			LCD_WriteReg(0x001E, 0x00B0);    delay_ms(50);   
			LCD_WriteReg(0x0001, 0x2B3F);    delay_ms(50); 
			LCD_WriteReg(0x0002, 0x0600);    delay_ms(50);
			LCD_WriteReg(0x0010, 0x0000);    delay_ms(50);
			LCD_WriteReg(0x0011, 0x6070);    delay_ms(50);
			LCD_WriteReg(0x0005, 0x0000);    delay_ms(50);
			LCD_WriteReg(0x0006, 0x0000);    delay_ms(50);
			LCD_WriteReg(0x0016, 0xEF1C);    delay_ms(50);
			LCD_WriteReg(0x0017, 0x0003);    delay_ms(50);
			LCD_WriteReg(0x0007, 0x0133);    delay_ms(50);         
			LCD_WriteReg(0x000B, 0x0000);    delay_ms(50);
			LCD_WriteReg(0x000F, 0x0000);    delay_ms(50);
			LCD_WriteReg(0x0041, 0x0000);    delay_ms(50);
			LCD_WriteReg(0x0042, 0x0000);    delay_ms(50);
			LCD_WriteReg(0x0048, 0x0000);    delay_ms(50);
			LCD_WriteReg(0x0049, 0x013F);    delay_ms(50);
			LCD_WriteReg(0x004A, 0x0000);    delay_ms(50);
			LCD_WriteReg(0x004B, 0x0000);    delay_ms(50);
			LCD_WriteReg(0x0044, 0xEF00);    delay_ms(50);
			LCD_WriteReg(0x0045, 0x0000);    delay_ms(50);
			LCD_WriteReg(0x0046, 0x013F);    delay_ms(50);
			LCD_WriteReg(0x0030, 0x0707);    delay_ms(50);
			LCD_WriteReg(0x0031, 0x0204);    delay_ms(50);
			LCD_WriteReg(0x0032, 0x0204);    delay_ms(50);
			LCD_WriteReg(0x0033, 0x0502);    delay_ms(50);
			LCD_WriteReg(0x0034, 0x0507);    delay_ms(50);
			LCD_WriteReg(0x0035, 0x0204);    delay_ms(50);
			LCD_WriteReg(0x0036, 0x0204);    delay_ms(50);
			LCD_WriteReg(0x0037, 0x0502);    delay_ms(50);
			LCD_WriteReg(0x003A, 0x0302);    delay_ms(50);
			LCD_WriteReg(0x003B, 0x0302);    delay_ms(50);
			LCD_WriteReg(0x0023, 0x0000);    delay_ms(50);
			LCD_WriteReg(0x0024, 0x0000);    delay_ms(50);
			LCD_WriteReg(0x0025, 0x8000);    delay_ms(50);
			LCD_WriteReg(0x004f,0);
			LCD_WriteReg(0x004e,0);
		}
		else if( DeviceCode == 0x8999 )
		{
			GPU.Code = SSD1298;		
			
			LCD_WriteReg(0x0028, 0x0006);
			LCD_WriteReg(0x0000, 0x0001);
			LCD_WriteReg(0x0003, 0xaea4);
			LCD_WriteReg(0x000c, 0x0004);
			LCD_WriteReg(0x000d, 0x000c);
			LCD_WriteReg(0x000e, 0x2800);
			LCD_WriteReg(0x001e, 0x00b5);
			LCD_WriteReg(0x0001, 0x3b3f);     
			LCD_WriteReg(0x0002, 0x0600);
			LCD_WriteReg(0x0010, 0x0000);
			LCD_WriteReg(0x0011, 0x6830);
			LCD_WriteReg(0x0005, 0x0000);
			LCD_WriteReg(0x0006, 0x0000);
			LCD_WriteReg(0x0016, 0xef1c);  
			LCD_WriteReg(0x0007, 0x0033);   
			LCD_WriteReg(0x000b, 0x0000);
			LCD_WriteReg(0x000f, 0x0000);
			LCD_WriteReg(0x0041, 0x0000);
			LCD_WriteReg(0x0042, 0x0000);
			LCD_WriteReg(0x0048, 0x0000);
			LCD_WriteReg(0x0049, 0x013f);
			LCD_WriteReg(0x004a, 0x0000);
			LCD_WriteReg(0x004b, 0x0000); 
			LCD_WriteReg(0x0044, 0xef00);
			LCD_WriteReg(0x0045, 0x0000);
			LCD_WriteReg(0x0046, 0x013f);
			LCD_WriteReg(0x004e, 0x0000);
			LCD_WriteReg(0x004f, 0x0000);  
			LCD_WriteReg(0x0030, 0x0707);
			LCD_WriteReg(0x0031, 0x0202);
			LCD_WriteReg(0x0032, 0x0204);
			LCD_WriteReg(0x0033, 0x0502);
			LCD_WriteReg(0x0034, 0x0507);
			LCD_WriteReg(0x0035, 0x0204);
			LCD_WriteReg(0x0036, 0x0204);
			LCD_WriteReg(0x0037, 0x0502);
			LCD_WriteReg(0x003a, 0x0302);
			LCD_WriteReg(0x003b, 0x0302); 
			LCD_WriteReg(0x0023, 0x0000);
			LCD_WriteReg(0x0024, 0x0000);
			LCD_WriteReg(0x0025, 0x8000);
			LCD_WriteReg(0x0026, 0x7000);
			LCD_WriteReg(0x0020, 0xb0eb);
			LCD_WriteReg(0x0027, 0x007c);
		}
		else if( DeviceCode == 0x5408 )
		{
			GPU.Code = SPFD5408B;
			
			LCD_WriteReg(0x0001, 0x0100); 
			LCD_WriteReg(0x0002, 0x0700);
			LCD_WriteReg(0x0003, 0x1030);
			LCD_WriteReg(0x0004, 0x0000);
			LCD_WriteReg(0x0008, 0x0207);
			LCD_WriteReg(0x0009, 0x0000);
			LCD_WriteReg(0x000A, 0x0000);
			LCD_WriteReg(0x000C, 0x0000);
			LCD_WriteReg(0x000D, 0x0000);
			LCD_WriteReg(0x000F, 0x0000);
			delay_ms(50);
			LCD_WriteReg(0x0007, 0x0101);
			delay_ms(50);
			LCD_WriteReg(0x0010, 0x16B0);
			LCD_WriteReg(0x0011, 0x0001); 
			LCD_WriteReg(0x0017, 0x0001);
			LCD_WriteReg(0x0012, 0x0138); 
			LCD_WriteReg(0x0013, 0x0800);
			LCD_WriteReg(0x0029, 0x0009);
			LCD_WriteReg(0x002a, 0x0009);
			LCD_WriteReg(0x00a4, 0x0000);  
			LCD_WriteReg(0x0050, 0x0000);
			LCD_WriteReg(0x0051, 0x00EF);
			LCD_WriteReg(0x0052, 0x0000);
			LCD_WriteReg(0x0053, 0x013F);
			LCD_WriteReg(0x0060, 0x2700);
			LCD_WriteReg(0x0061, 0x0003);
			LCD_WriteReg(0x006A, 0x0000);
			LCD_WriteReg(0x0080, 0x0000);
			LCD_WriteReg(0x0081, 0x0000);
			LCD_WriteReg(0x0082, 0x0000);
			LCD_WriteReg(0x0083, 0x0000);
			LCD_WriteReg(0x0084, 0x0000);
			LCD_WriteReg(0x0085, 0x0000);
			LCD_WriteReg(0x0090, 0x0013);
			LCD_WriteReg(0x0092, 0x0000); 
			LCD_WriteReg(0x0093, 0x0003);
			LCD_WriteReg(0x0095, 0x0110);
			LCD_WriteReg(0x0007, 0x0173);
		}
		else if( DeviceCode == 0x4531 )
		{	
			GPU.Code = LGDP4531;
			
			LCD_WriteReg(0x00, 0x0001);
			LCD_WriteReg(0x10, 0x0628);
			LCD_WriteReg(0x12, 0x0006);
			LCD_WriteReg(0x13, 0x0A32);
			LCD_WriteReg(0x11, 0x0040);
			LCD_WriteReg(0x15, 0x0050);
			LCD_WriteReg(0x12, 0x0016);
			delay_ms(50);
			LCD_WriteReg(0x10, 0x5660);
			delay_ms(50);
			LCD_WriteReg(0x13, 0x2A4E);
			LCD_WriteReg(0x01, 0x0100);
			LCD_WriteReg(0x02, 0x0300);	
			LCD_WriteReg(0x03, 0x1030);		
			LCD_WriteReg(0x08, 0x0202);
			LCD_WriteReg(0x0A, 0x0000);
			LCD_WriteReg(0x30, 0x0000);
			LCD_WriteReg(0x31, 0x0402);
			LCD_WriteReg(0x32, 0x0106);
			LCD_WriteReg(0x33, 0x0700);
			LCD_WriteReg(0x34, 0x0104);
			LCD_WriteReg(0x35, 0x0301);
			LCD_WriteReg(0x36, 0x0707);
			LCD_WriteReg(0x37, 0x0305);
			LCD_WriteReg(0x38, 0x0208);
			LCD_WriteReg(0x39, 0x0F0B);
			delay_ms(50);
			LCD_WriteReg(0x41, 0x0002);
			LCD_WriteReg(0x60, 0x2700);
			LCD_WriteReg(0x61, 0x0001);
			LCD_WriteReg(0x90, 0x0119);
			LCD_WriteReg(0x92, 0x010A);
			LCD_WriteReg(0x93, 0x0004);
			LCD_WriteReg(0xA0, 0x0100);
			delay_ms(50);
			LCD_WriteReg(0x07, 0x0133);
			delay_ms(50);
			LCD_WriteReg(0xA0, 0x0000);
		}
		else if( DeviceCode == 0x4535 )
		{	
			GPU.Code = LGDP4535;	
			
			LCD_WriteReg(0x15, 0x0030);                                             
			LCD_WriteReg(0x9A, 0x0010); 
			LCD_WriteReg(0x11, 0x0020);
			LCD_WriteReg(0x10, 0x3428);
			LCD_WriteReg(0x12, 0x0002);
			LCD_WriteReg(0x13, 0x1038);
			delay_ms(40); 
			LCD_WriteReg(0x12, 0x0012);
			delay_ms(40); 
			LCD_WriteReg(0x10, 0x3420);
			LCD_WriteReg(0x13, 0x3045);
			delay_ms(70); 
			LCD_WriteReg(0x30, 0x0000);
			LCD_WriteReg(0x31, 0x0402);
			LCD_WriteReg(0x32, 0x0307); 
			LCD_WriteReg(0x33, 0x0304);
			LCD_WriteReg(0x34, 0x0004);
			LCD_WriteReg(0x35, 0x0401);
			LCD_WriteReg(0x36, 0x0707);
			LCD_WriteReg(0x37, 0x0305);
			LCD_WriteReg(0x38, 0x0610);
			LCD_WriteReg(0x39, 0x0610);
			LCD_WriteReg(0x01, 0x0100); 
			LCD_WriteReg(0x02, 0x0300);
			LCD_WriteReg(0x03, 0x1030);
			LCD_WriteReg(0x08, 0x0808);
			LCD_WriteReg(0x0A, 0x0008);		
			LCD_WriteReg(0x60, 0x2700);	
			LCD_WriteReg(0x61, 0x0001);
			LCD_WriteReg(0x90, 0x013E);
			LCD_WriteReg(0x92, 0x0100);
			LCD_WriteReg(0x93, 0x0100);
			LCD_WriteReg(0xA0, 0x3000);
			LCD_WriteReg(0xA3, 0x0010);
			LCD_WriteReg(0x07, 0x0001);
			LCD_WriteReg(0x07, 0x0021);
			LCD_WriteReg(0x07, 0x0023);
			LCD_WriteReg(0x07, 0x0033);
			LCD_WriteReg(0x07, 0x0133);
		} 		 		
		else if( DeviceCode == 0x0047 )
		{
			GPU.Code = HX8347D;
			
			LCD_WriteReg(0xEA, 0x00);                          
			LCD_WriteReg(0xEB, 0x20);                                                     
			LCD_WriteReg(0xEC, 0x0C);                                                   
			LCD_WriteReg(0xED, 0xC4);                                                    
			LCD_WriteReg(0xE8, 0x40);                                                     
			LCD_WriteReg(0xE9, 0x38);                                                    
			LCD_WriteReg(0xF1, 0x01);                                                    
			LCD_WriteReg(0xF2, 0x10);                                                    
			LCD_WriteReg(0x27, 0xA3);                                                    
			LCD_WriteReg(0x40, 0x01);                           
			LCD_WriteReg(0x41, 0x00);                                                   
			LCD_WriteReg(0x42, 0x00);                                                   
			LCD_WriteReg(0x43, 0x10);                                                    
			LCD_WriteReg(0x44, 0x0E);                                                   
			LCD_WriteReg(0x45, 0x24);                                                  
			LCD_WriteReg(0x46, 0x04);                                                  
			LCD_WriteReg(0x47, 0x50);                                                   
			LCD_WriteReg(0x48, 0x02);                                                    
			LCD_WriteReg(0x49, 0x13);                                                  
			LCD_WriteReg(0x4A, 0x19);                                                  
			LCD_WriteReg(0x4B, 0x19);                                                 
			LCD_WriteReg(0x4C, 0x16);                                                 
			LCD_WriteReg(0x50, 0x1B);                                                   
			LCD_WriteReg(0x51, 0x31);                                                    
			LCD_WriteReg(0x52, 0x2F);                                                     
			LCD_WriteReg(0x53, 0x3F);                                                    
			LCD_WriteReg(0x54, 0x3F);                                                     
			LCD_WriteReg(0x55, 0x3E);                                                     
			LCD_WriteReg(0x56, 0x2F);                                                   
			LCD_WriteReg(0x57, 0x7B);                                                     
			LCD_WriteReg(0x58, 0x09);                                                  
			LCD_WriteReg(0x59, 0x06);                                                 
			LCD_WriteReg(0x5A, 0x06);                                                   
			LCD_WriteReg(0x5B, 0x0C);                                                   
			LCD_WriteReg(0x5C, 0x1D);                                                   
			LCD_WriteReg(0x5D, 0xCC);                                                   
			LCD_WriteReg(0x1B, 0x18);                                                    
			LCD_WriteReg(0x1A, 0x01);                                                    
			LCD_WriteReg(0x24, 0x15);                                                    
			LCD_WriteReg(0x25, 0x50);                                                    
			LCD_WriteReg(0x23, 0x8B);                                                   
			LCD_WriteReg(0x18, 0x36);                           
			LCD_WriteReg(0x19, 0x01);                                                    
			LCD_WriteReg(0x01, 0x00);                                                   
			LCD_WriteReg(0x1F, 0x88);                                                    
			delay_ms(50);
			LCD_WriteReg(0x1F, 0x80);                                                  
			delay_ms(50);
			LCD_WriteReg(0x1F, 0x90);                                                   
			delay_ms(50);
			LCD_WriteReg(0x1F, 0xD0);                                                   
			delay_ms(50);
			LCD_WriteReg(0x17, 0x05);                                                    
			LCD_WriteReg(0x36, 0x00);                                                    
			LCD_WriteReg(0x28, 0x38);                                                 
			delay_ms(50);
			LCD_WriteReg(0x28, 0x3C);                                                
		}
		else if( DeviceCode == 0x7783 )
		{
			GPU.Code = ST7781;
			
			LCD_WriteReg(0x00FF, 0x0001);
			LCD_WriteReg(0x00F3, 0x0008);
			LCD_WriteReg(0x0001, 0x0100);
			LCD_WriteReg(0x0002, 0x0700);
			LCD_WriteReg(0x0003, 0x1030);  
			LCD_WriteReg(0x0008, 0x0302);
			LCD_WriteReg(0x0008, 0x0207);
			LCD_WriteReg(0x0009, 0x0000);
			LCD_WriteReg(0x000A, 0x0000);
			LCD_WriteReg(0x0010, 0x0000);  
			LCD_WriteReg(0x0011, 0x0005);
			LCD_WriteReg(0x0012, 0x0000);
			LCD_WriteReg(0x0013, 0x0000);
			delay_ms(50);
			LCD_WriteReg(0x0010, 0x12B0);
			delay_ms(50);
			LCD_WriteReg(0x0011, 0x0007);
			delay_ms(50);
			LCD_WriteReg(0x0012, 0x008B);
			delay_ms(50);	
			LCD_WriteReg(0x0013, 0x1700);
			delay_ms(50);	
			LCD_WriteReg(0x0029, 0x0022);		
			LCD_WriteReg(0x0030, 0x0000);
			LCD_WriteReg(0x0031, 0x0707);
			LCD_WriteReg(0x0032, 0x0505);
			LCD_WriteReg(0x0035, 0x0107);
			LCD_WriteReg(0x0036, 0x0008);
			LCD_WriteReg(0x0037, 0x0000);
			LCD_WriteReg(0x0038, 0x0202);
			LCD_WriteReg(0x0039, 0x0106);
			LCD_WriteReg(0x003C, 0x0202);
			LCD_WriteReg(0x003D, 0x0408);
			delay_ms(50);				
			LCD_WriteReg(0x0050, 0x0000);		
			LCD_WriteReg(0x0051, 0x00EF);		
			LCD_WriteReg(0x0052, 0x0000);		
			LCD_WriteReg(0x0053, 0x013F);		
			LCD_WriteReg(0x0060, 0xA700);		
			LCD_WriteReg(0x0061, 0x0001);
			LCD_WriteReg(0x0090, 0x0033);				
			LCD_WriteReg(0x002B, 0x000B);		
			LCD_WriteReg(0x0007, 0x0133);
		}
		else
		{
			DeviceCode = LCD_ReadReg(0x67);
			
			if( DeviceCode == 0x0047 )
			{
				GPU.Code = HX8347A;
				
				LCD_WriteReg(0x0042, 0x0008);   
				LCD_WriteReg(0x0046, 0x00B4); 
				LCD_WriteReg(0x0047, 0x0043); 
				LCD_WriteReg(0x0048, 0x0013); 
				LCD_WriteReg(0x0049, 0x0047); 
				LCD_WriteReg(0x004A, 0x0014); 
				LCD_WriteReg(0x004B, 0x0036); 
				LCD_WriteReg(0x004C, 0x0003); 
				LCD_WriteReg(0x004D, 0x0046); 
				LCD_WriteReg(0x004E, 0x0005);  
				LCD_WriteReg(0x004F, 0x0010);  
				LCD_WriteReg(0x0050, 0x0008);  
				LCD_WriteReg(0x0051, 0x000a);  
				LCD_WriteReg(0x0002, 0x0000); 
				LCD_WriteReg(0x0003, 0x0000); 
				LCD_WriteReg(0x0004, 0x0000); 
				LCD_WriteReg(0x0005, 0x00EF); 
				LCD_WriteReg(0x0006, 0x0000); 
				LCD_WriteReg(0x0007, 0x0000); 
				LCD_WriteReg(0x0008, 0x0001); 
				LCD_WriteReg(0x0009, 0x003F); 
				delay_ms(10); 
				LCD_WriteReg(0x0001, 0x0006); 
				LCD_WriteReg(0x0016, 0x00C8);   
				LCD_WriteReg(0x0023, 0x0095); 
				LCD_WriteReg(0x0024, 0x0095); 
				LCD_WriteReg(0x0025, 0x00FF); 
				LCD_WriteReg(0x0027, 0x0002); 
				LCD_WriteReg(0x0028, 0x0002); 
				LCD_WriteReg(0x0029, 0x0002); 
				LCD_WriteReg(0x002A, 0x0002); 
				LCD_WriteReg(0x002C, 0x0002); 
				LCD_WriteReg(0x002D, 0x0002); 
				LCD_WriteReg(0x003A, 0x0001);  
				LCD_WriteReg(0x003B, 0x0001);  
				LCD_WriteReg(0x003C, 0x00F0);    
				LCD_WriteReg(0x003D, 0x0000); 
				delay_ms(20); 
				LCD_WriteReg(0x0035, 0x0038); 
				LCD_WriteReg(0x0036, 0x0078); 
				LCD_WriteReg(0x003E, 0x0038); 
				LCD_WriteReg(0x0040, 0x000F); 
				LCD_WriteReg(0x0041, 0x00F0);  
				LCD_WriteReg(0x0038, 0x0000); 
				LCD_WriteReg(0x0019, 0x0049);  
				LCD_WriteReg(0x0093, 0x000A); 
				delay_ms(10); 
				LCD_WriteReg(0x0020, 0x0020);   
				LCD_WriteReg(0x001D, 0x0003);   
				LCD_WriteReg(0x001E, 0x0000);  
				LCD_WriteReg(0x001F, 0x0009);   
				LCD_WriteReg(0x0044, 0x0053);  
				LCD_WriteReg(0x0045, 0x0010);   
				delay_ms(10);  
				LCD_WriteReg(0x001C, 0x0004);  
				delay_ms(20); 
				LCD_WriteReg(0x0043, 0x0080);    
				delay_ms(5); 
				LCD_WriteReg(0x001B, 0x000a);    
				delay_ms(40);  
				LCD_WriteReg(0x001B, 0x0012);    
				delay_ms(40);   
				LCD_WriteReg(0x0090, 0x007F); 
				LCD_WriteReg(0x0026, 0x0004); 
				delay_ms(40);  
				LCD_WriteReg(0x0026, 0x0024); 
				LCD_WriteReg(0x0026, 0x002C); 
				delay_ms(40);   
				LCD_WriteReg(0x0070, 0x0008); 
				LCD_WriteReg(0x0026, 0x003C);  
				LCD_WriteReg(0x0057, 0x0002); 
				LCD_WriteReg(0x0055, 0x0000); 
				LCD_WriteReg(0x0057, 0x0000);
			}
			else
			{
				// INVALID Driver
				GPU.Status = OS_GPU_CRASHED;
				GPU.Code = INVALID;
				
				return;
			}
		}
		delay_ms(50);
	
	#else

		#error "This platform does not support Graphics"
		
	#endif
	
	// Ready
	GPU.Status = OS_GPU_READY;	
}

#endif

