//////////////////////////////////////////////////////////////////////////////////////
// Mystic OS Copyright 2012-2013 (c) Wojciech Figat
// The software is provided 'as is'. 
// The original author is not responsible for any harm which may result from use of this software or its source code
// See also my 3d game engine --> http://celelej.com
//////////////////////////////////////////////////////////////////////////////////////

#ifndef  __OS_FILESYSTEM_H__ 
#define  __OS_FILESYSTEM_H__ 

// Include
#include "Config.h"
#include "Types.h"

// Check if a drive is available. Returns True or False.
Byte OS_FS_Available(void);

// FileSystem General Initialization
Byte OS_FS_Init(void);

// Close FileSystem and release all resources
Byte OS_FS_Exit(void);

// Open or create file
Byte OS_FS_Open(File* file, const Char* path, Byte flags);

// Delete existing file or directory
Byte OS_FS_Delete(const Char* path);

// Create new directory
Byte OS_FS_CreateDir(const Char* path);

// Read data from file
Byte OS_FS_Read(File* file, void* buffer, Word size);

// Move file pointer of a file
Byte OS_FS_Seek(File* file, Word pos);

// Close file
Byte OS_FS_Close(File* file);

// Open Direcory
Byte OS_FS_OpenDir(Directory* dir, const Char* path);

// Read direcory item
Byte OS_FS_ReadDir(Directory *dir, FileInfo* info);

// Get file info
Byte OS_FS_FileInfo(const Char* path, FileInfo* info);

// Write data to file
Byte OS_FS_Write(File* file, const void* buffer, Word size);

// Get Fat informations
Byte OS_FS_GetFatInfo(FatInfo* info);

// Truncate file
Byte OS_FS_Truncate(File* file);		

// Flush cached data to the file
Byte OS_FS_Flush(File* file);

// Set attribute of the file or directory
Byte OS_FS_SetAttribute(const Char* path, Byte attribute);

// Set timestamp of the file or directory
Byte OS_FS_SetTime(const Char* path, const FileInfo* timestamp);

// Rename or move file
Byte OS_FS_Rename(const Char* Fold, const Char* Fnew);

// illegal chars
// "\"*:<>\?|\x7F"

#endif /* __OS_FILESYSTEM_H__ */

