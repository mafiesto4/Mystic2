//////////////////////////////////////////////////////////////////////////////////////
// Mystic OS Copyright 2012-2013 (c) Wojciech Figat
// The software is provided 'as is'. 
// The original author is not responsible for any harm which may result from use of this software or its source code
// See also my 3d game engine --> http://celelej.com
//////////////////////////////////////////////////////////////////////////////////////


#ifndef __OS_UTILITIES_H__
#define __OS_UTILITIES_H__

// Include
#include "Config.h"
#include "Types.h"


// Copy memory to memory
static void OS_UTILS_MemoryCopy(void* dst, const void* src, Word size)
{
	Byte *d = (Byte*)dst;
	const Byte *s = (const Byte*)src;
	
	while (size--)
		*d++ = *s++;
}

// Fill memory
static void OS_UTILS_MemorySet( void* dst, Byte val, Word size) 
{
	Byte *d = (Byte*)dst;

	while (size--)
		*d++ = val;
}

// Compare memory
static Word OS_UTILS_MemoryCompare(const void* dst, const void* src, Word size) 
{
	const Byte *d = (const Byte*)dst, *s = (const Byte*)src;
	Word r = 0;
	
	while (size-- && (r = *d++ - *s++) == 0);
	
	return r;
}





#endif /* __OS_UTILITIES_H__ */
