//////////////////////////////////////////////////////////////////////////////////////
// Mystic OS Copyright 2012-2013 (c) Wojciech Figat
// The software is provided 'as is'. 
// The original author is not responsible for any harm which may result from use of this software or its source code
// See also my 3d game engine --> http://celelej.com
//////////////////////////////////////////////////////////////////////////////////////


#include "Types.h"
#include "Time.h"
 
 
 // Get full, easy to operate time version from packed 32 bits
Time OS_Time_PackedToFull( PackedTime time )
{
	Time res;
	
	// Hour
	res.Hour  = 1         & time;
	res.Hour += 1 << 0x01 & time;
	res.Hour += 1 << 0x02 & time;
	res.Hour += 1 << 0x03 & time;
	res.Hour += 1 << 0x04 & time;
	
	// Minute
	time = time >> 0x05;
	res.Minute  = 1         & time;
	res.Minute += 1 << 0x01 & time;
	res.Minute += 1 << 0x02 & time;
	res.Minute += 1 << 0x03 & time;
	res.Minute += 1 << 0x04 & time;
	res.Minute += 1 << 0x05 & time;
	
	// Second
	time = time >> 0x06;
	res.Second  = 1         & time;
	res.Second += 1 << 0x01 & time;
	res.Second += 1 << 0x02 & time;
	res.Second += 1 << 0x03 & time;
	res.Second += 1 << 0x04 & time;
	res.Second *= 2;
	
	// Year
	time = time >> 0x05;
	res.Year  = 1         & time;
	res.Year += 1 << 0x01 & time;
	res.Year += 1 << 0x02 & time;
	res.Year += 1 << 0x03 & time;
	res.Year += 1 << 0x04 & time;
	res.Year += 1 << 0x05 & time;
	res.Year += 1 << 0x06 & time;
	res.Year += 1980;
	
	// Month
	time = time >> 0x07;
	res.Month  = 1         & time;
	res.Month += 1 << 0x01 & time;
	res.Month += 1 << 0x02 & time;
	res.Month += 1 << 0x03 & time;

	// Day
	res.Day = 0;
	time = time >> 0x04;
	res.Day  = 1         & time;
	res.Day += 1 << 0x01 & time;
	res.Day += 1 << 0x02 & time;
	res.Day += 1 << 0x03 & time;
	res.Day += 1 << 0x04 & time;
	
	return res;
}


// Get packed time verison on 32 bits from full version
PackedTime OS_Time_FullToPacked( Time time )
{
	PackedTime res = 0;
	
	// Hour
	BITMASK_SET(res, time.Hour);
	
	// Minute
	BITMASK_SET(res, time.Minute << 0x05);
	
	// Second
	BITMASK_SET(res, ( time.Second / 2 ) << 0x0B);
	
	// Year
	BITMASK_SET(res, (time.Year - 1980) << 0x10);
	
	// Month
	BITMASK_SET(res, time.Month << 0x17);	
	
	// Day
	BITMASK_SET(res, time.Day << 0x1B);
	
	return res;
}


// Check if given time tis valid
Byte Os_Time_IsVaildTime( Time time )
{
	return OS_Math_IsSmaller(time.Hour, 23)
		&& OS_Math_IsSmaller(time.Minute, 59)
		&& OS_Math_IsSmaller(time.Second, 59)
		&& OS_Math_Contains(time.Day, 1, 31)
		&& OS_Math_Contains(time.Month, 1, 12)
		&& OS_Math_Contains(time.Year, 1980, 2100);
}


