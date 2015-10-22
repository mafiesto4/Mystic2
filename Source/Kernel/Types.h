//////////////////////////////////////////////////////////////////////////////////////
// Mystic OS Copyright 2012-2013 (c) Wojciech Figat
// The software is provided 'as is'. 
// The original author is not responsible for any harm which may result from use of this software or its source code
// See also my 3d game engine --> http://celelej.com
//////////////////////////////////////////////////////////////////////////////////////

#ifndef OS_TYPES
#define OS_TYPES

//////////////////////////////////////////////////////////////////////////////////////
// Preprocessor
/////////////////////////////////////////////////////////////////////////////////////

// Get Config
#include "Config.h"

// Libraries to include
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>

#include "SPI_MSD_Driver.h"

//////////////////////////////////////////////////////////////////////////////////////
// Data Types
/////////////////////////////////////////////////////////////////////////////////////

typedef unsigned char Byte;
typedef unsigned short Short;
typedef unsigned long Word;

typedef char SByte;
typedef short SShort;
typedef long SWord;

typedef float Float;

typedef char Char;

typedef unsigned short Color;

/////////////////////////////////////////////////////////////////////////////////////
// Compiler dependent types
/////////////////////////////////////////////////////////////////////////////////////

// Clear any old defines
#if defined Asm
	#undef Asm
#endif

#if defined Inline
	#undef Inline
#endif

#if defined Static
	#undef Static
#endif

#if defined Volatile
	#undef Volatile
#endif

#if defined Const
	#undef Const
#endif

// ARM Compiler
#if defined ( __CC_ARM   )

  #define Asm __asm
  #define Inline __inline
  
 // IAR Compiler, Only avaiable in High optimization mode!
#elif defined ( __ICCARM__ )

  #define Asm __asm
  #define Inline inline
  
// GNU Compiler
#elif defined (  __GNUC__  )

  #define Asm __asm
  #define Inline inline
  
// TASKING Compiler
#elif defined   (  __TASKING__  )

  #define Asm __asm
  #define Inline inline
  
// Unknown Compiler
#else
	
	#error "You are using unknown compiler! Please contact with author for help: mafiesto4@wp.pl"

#endif

/////////////////////////////////////////////////////////////////////////////////////
// Other defines
/////////////////////////////////////////////////////////////////////////////////////

// True
#ifndef True
	#define True 1
#endif

// False
#ifndef False
	#define False 0
#endif

// Good
#ifndef Good
	#define Good 1
#endif

// Error
#ifndef Error
	#define Error 0
#endif

// Nothing
#ifndef Nothing
	#define Nothing 0
#endif

// Null pointer
#ifndef Null
	#define Null (void*)0
#endif

#define OS_Math_Contains(x, min, max) ((x >= min) ? ((x <= max) ? 1 : 0) : 0)
#define OS_Math_IsSmaller(x, max) ((x <= max) ? 1 : 0)
#define OS_Math_IsBigger(x, max) ((x > max) ? 1 : 0)

/* a=target variable, b=bit number to act upon 0-n */
#define BIT_SET(a,b) ((a) |= (1<<(b)))
#define BIT_CLEAR(a,b) ((a) &= ~(1<<(b)))
#define BIT_FLIP(a,b) ((a) ^= (1<<(b)))
#define BIT_CHECK(a,b) ((a) & (1<<(b)))

/* x=target variable, y=mask */
#define BITMASK_SET(x,y) ((x) |= (y))
#define BITMASK_CLEAR(x,y) ((x) &= (~(y)))
#define BITMASK_FLIP(x,y) ((x) ^= (y))
#define BITMASK_CHECK(x,y) ((x) & (y))

// Functions return codes
#define RES_GOOD					0			// Succeeded
#define RES_FAILED					20			// Failed
#define RES_INVALID_DATA			21			// Invalid Data
#define RES_OUT_OF_MEMORY			22			// Out of memory
#define RES_OUT_OF_FREE_APP_IDS		23			// Out of free application ids
#define RES_CANNOT_CREATE_PROCESS	24			// Cannot create new process

// Dynamic List
typedef struct List 
{
	
	Word Count;							// List items count
	Word Capacity;						// List capacity
	void **Data;						// Data pointer
	
} List;

/////////////////////////////////////////////////////////////////////////////////////
// File System
/////////////////////////////////////////////////////////////////////////////////////

// Maximum sector size to be handled. Possible values: 512, 1024, 2048 and 4096.
#define	_MAX_SS 512

// File system object structure
typedef struct FileSystem
{

	Byte	fs_type;			// FAT sub-type (0:Not mounted)
	Byte	csize;				// Sectors per cluster (1,2,4...128)
	Byte	n_fats;				// Number of FAT copies (1,2)
	Byte	wflag;				// win[] dirty flag (1:must be written back)
	Byte	fsi_flag;			// fsinfo dirty flag (1:must be written back)
	Short	id;					// File system mount ID
	Short	n_rootdir;			// Number of root directory entries (FAT12/16)
	#if MAX_SS != 512
		Short	ssize;			// Bytes per sector (512, 1024, 2048, 4096)
	#endif
	#if _FS_REENTRANT
		_SYNC_t	sobj;			// Identifier of sync object
	#endif
	#if !_FS_READONLY
		Word	last_clust;		// Last allocated cluster
		Word	free_clust;		// Number of free clusters
		Word	fsi_sector;		// fsinfo sector (FAT32)
	#endif
	#if _FS_RPATH
		Word	cdir;			// Current directory start cluster (0:root)
	#endif
	Word	n_fatent;			// Number of FAT entries (= number of clusters + 2)
	Word	fsize;				// Sectors per FAT
	Word	volbase;			// Volume start sector
	Word	fatbase;			// FAT start sector
	Word	dirbase;			// Root directory start sector (FAT32:Cluster#)
	Word	database;			// Data start sector
	Word	winsect;			// Current sector appearing in the win[]
	Byte	win[_MAX_SS];		// Disk access window for Directory, FAT (and Data on tiny cfg)
	
} FileSystem;


// File object structure
typedef struct File
{
	
	FileSystem*	fs;					// Pointer to the owner file system object
	Short	id;						// Owner file system mount ID
	Byte	flag;					// File status flags
	Byte	pad1;
	Word	fptr;					// File read/write pointer (0 on file open)
	Word	fsize;					// File size
	Word	sclust;					// File start cluster (0:no data cluster, always 0 when fsize is 0)
	Word	clust;					// Current clusterr of fpter
	Word	dsect;					// Current data sector of fpter
	Word	dir_sect;				// Sector containing the directory entry
	Byte*	dir_ptr;				// Ponter to the directory entry in the window
	#if !_FS_TINY
		Byte	buf[_MAX_SS];		// File data read/write buffer
	#endif
	
} File;


// Directory object structure
typedef struct Directory
{
	
	FileSystem*	fs;			// Pointer to the owner file system object
	Short	id;				// Owner file system mount ID
	Short	index;			// Current read/write index number
	Word	sclust;			// Table start cluster (0:Root dir)
	Word	clust;			// Current cluster
	Word	sect;			// Current sector
	Byte*	dir;			// Pointer to the current SFN entry in the win[]
	Byte*	fn;				// Pointer to the SFN (in/out) {file[8],ext[3],status[1]}
	#if _USE_LFN
		WCHAR*	lfn;		// Pointer to the LFN working buffer
		Short	lfn_idx;	// Last matched LFN index number (0xFFFF:No LFN)
	#endif
	
} Directory;


// File Info structure
typedef struct FileInfo
{
	
	Word	Size;			// File size
	Word	Date;			// Last modified date
	Word	Time;			// Last modified time
	Byte	Attribute;		// Attribute
	Char	Name[13];		// Short file name (8.3 format)
	Char	*LName;			// Pointer to the LFN buffer
	Short 	LSize;			// Size of LFN buffer in Char
	
} FileInfo;


// Fat Info structure
typedef struct FatInfo
{
	
	Byte FatType;          // Fat type: FS_FAT12 = 12, FS_FAT16 = 16, FS_FAT32 = 32
	Word TotalSpace;       // Total Fat size 
	Word FreeSpace;        // Total free space
	
} FatInfo;

// File System operations result
#define FS_RESULT_OK                    0  // No errors
#define FS_RESULT_DISK_ERR              1  // A hard error occured in the low level disk I/O layer
#define FS_RESULT_INT_ERR               2  // Assertion failed
#define FS_RESULT_NOT_READY             3  // The physical drive cannot work
#define FS_RESULT_NO_FILE               4  // Could not find the file
#define FS_RESULT_NO_PATH               5  // Could not find the path
#define FS_RESULT_INVALID_NAME          6  // The path name format is invalid
#define FS_RESULT_DENIED                7  // Acces denied due to prohibited access or directory full
#define FS_RESULT_EXIST                 8  // Acces denied due to prohibited access
#define FS_RESULT_INVALID_OBJECT        9  // The file/directory object is invalid
#define FS_RESULT_WRITE_PROTECTED       10 // The physical drive is write protected
#define FS_RESULT_INVALID_DRIVE	        11 // The logical drive number is invalid
#define FS_RESULT_NOT_ENABLED           12 // The volume has no work area
#define FS_RESULT_NO_FILESYSTEM         13 // There is no valid FAT volume on the physical drive
#define FS_RESULT_MKFS_ABORTED          14 // The f_mkfs() aborted due to any parameter error
#define FS_RESULT_TIMEOUT               15 // Could not get a grant to access the volume within defined period
#define FS_RESULT_LOCKED                16 // The operation is rejected according to the file shareing policy
#define FS_RESULT_NOT_ENOUGH_CORE       17 // LFN working buffer could not be allocated
#define FS_RESULT_TOO_MANY_OPEN_FILES   18 // Number of open files > _FS_SHARE

// FAT type
#define FS_FAT12	12
#define FS_FAT16	26
#define FS_FAT32	32

// File access control and file status flags
#define FILE_ACCESS_OPEN_EXISTING  0x00   					// Opens the file. The function fails if the file is not existing.
#define FILE_ACCESS_READ           0x01   					// Specifies read access to the object. Data can be read from the file.
#define FILE_ACCESS_WRITE          0x02   					// Specifies write access to the object. Data can be written to the file.
#define FILE_ACCESS_CREATE_NEW     0x04   					// Creates a new file. The function fails if the file is already existing.
#define FILE_ACCESS_CREATE_ALWAYS  0x08   					// Creates a new file. If the file is existing, it is truncated and overwritten.
#define FILE_ACCESS_OPEN_ALWAYS    0x10   					// Opens the file if it is existing. If not, a new file is created. To append data to the file, use f_lseek function after file open in this method.
#define FILE_ACCESS_APPEND         0x100  					// FA_OPEN_ALWAYS Opens the file if it is existing. If not, a new file is created. To append data to the file, use f_lseek function after file open in this method.
#define FILE_ACCESS_READ_WRITE     FILE_READ | FILE_WRITE   // Specifies read-write access.

// File attributes
#define	FILE_ATTRIBUTE_READ_ONLY  	0x01   // Read only
#define	FILE_ATTRIBUTE_HIDDEN	    0x02   // Hidden
#define	FILE_ATTRIBUTE_SYSTEM	    0x04   // System
#define FILE_ATTRIBUTE_ARCHIVE    	0x20   // Archive

/////////////////////////////////////////////////////////////////////////////////////
// Applications
/////////////////////////////////////////////////////////////////////////////////////

// Process
#include "Process.h"

// Application
typedef struct Application
{
	
	Byte ID;                                            // Uniqe ID of the application
	Char* Name;                                         // Application name, ends with 0
	Byte NameSize;                                      // Application name size in bytes
	GraphicsHandle Graphics;                            // Graphics Handle
	Process *Exe;                                       // Appliaction main process
	File *File;                                         // File to read program
	
} Application;

/////////////////////////////////////////////////////////////////////////////////////
// Informations
/////////////////////////////////////////////////////////////////////////////////////

// Current Informations about the OS
#define OS_VERSION_MIN 20
#define OS_VERSION_MAX 20
#define OS_VERSION_STRING "2.0"
#define OS_NAME "Mystic"
#define OS_DATE "29-06-2013"
#define OS_WORD_SIZE "32 bit"
#define OS_FULLNAME "Mystic OS v2.0"

#endif /* OS_TYPES */
