//////////////////////////////////////////////////////////////////////////////////////
// Mystic OS Copyright 2012-2013 (c) Wojciech Figat
// The software is provided 'as is'. 
// The original author is not responsible for any harm which may result from use of this software or its source code
// See also my 3d game engine --> http://celelej.com
//////////////////////////////////////////////////////////////////////////////////////


#ifndef OS_TIME


// Time
typedef struct Time {

	Byte Second;										// Seconds (0–59)
	Byte Minute;										// Minutes (0–59)
	Byte Hour;											// Hours (0-23)
	Byte Day;											// Day (1–31)
	Byte Month;											// Month (1-12)
	Short Year;											// Year (1980-2100)
	
} Time;
/*
    uint32_t RTC_Sec;     // Second value - [0,59]
    uint32_t RTC_Min;     // Minute value - [0,59]
    uint32_t RTC_Hour;    // Hour value - [0,23]
    uint32_t RTC_Mday;    // Day of the month value - [1,31]
    uint32_t RTC_Mon;     // Month value - [1,12]
    uint32_t RTC_Year;    // Year value - [0,4095]
    uint32_t RTC_Wday;    // Day of week value - [0,6]
    uint32_t RTC_Yday;    // Day of year value - [1,365]
*/
// Packed Time
// Based on: http://msdn.microsoft.com/en-us/library/9kkf9tah(v=vs.71).aspx
//
// Time (first 16 bits)
// Bit position:	0   1   2   3   4	5   6   7   8   9   A	B   C   D   E   F
// Length:			5	6	5
// Contents:		hours	minutes	2-second increments
// Value Range:		0–23	0–59	0–29 in 2-second intervals
// 
// Date (last 16 bits)
// Bit position:	0   1   2   3   4   5   6	7   8   9   A	B   C   D   E   F
// Length:			7	4	5
// Contents:		year	month	day
// Value Range:		0–119	1–12	1–31
// 				(relative to 1980)	 	 
typedef Word PackedTime;
	  

// Get full, easy to operate time version from packed 32 bits
Time OS_Time_PackedToFull( PackedTime time );

// Get packed time verison on 32 bits from full version
PackedTime OS_Time_FullToPacked( Time time );

// Check if given time tis valid
Bool Os_Time_IsVaildTime( Time time ); 


// Time was set up!
#define OS_TIME

#endif /* OS_TIME */
