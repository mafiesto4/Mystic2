/*---------------------------------------------------------------------------/
/  FatFs - FAT file system module include file  R0.09b    (C)ChaN, 2013
/----------------------------------------------------------------------------/
/ FatFs module is a generic FAT file system module for small embedded systems.
/ This is a free software that opened for education, research and commercial
/ developments under license policy of following terms.
/
/  Copyright (C) 2013, ChaN, all right reserved.
/
/ * The FatFs module is a free software and there is NO WARRANTY.
/ * No restriction on use. You can use, modify and redistribute it for
/   personal, non-profit or commercial product UNDER YOUR RESPONSIBILITY.
/ * Redistributions of source code must retain the above copyright notice.
/
/----------------------------------------------------------------------------*/

#ifndef _FATFS
#define _FATFS	82786	// Revision ID

#ifdef __cplusplus
extern "C" {
#endif

// Include
#include "integer.h"
#include "ffconf.h"
#include "..\Kernel\Types.h"

#if _FATFS != _FFCONF
	#error "Wrong configuration file (ffconf.h)."
#endif


/* Definitions of volume management */

#if _MULTI_PARTITION		
	
	/* Multiple partition configuration */
	typedef struct {
		BYTE pd;	/* Physical drive number */
		BYTE pt;	/* Partition: 0:Auto detect, 1-4:Forced partition) */
	} PARTITION;
	
	extern PARTITION VolToPart[];	/* Volume - Partition resolution table */
	
	#define LD2PD(vol) (VolToPart[vol].pd)	/* Get physical drive number */
	#define LD2PT(vol) (VolToPart[vol].pt)	/* Get partition index */

#else	
	
	/* Single partition configuration */
	#define LD2PD(vol) (BYTE)(vol)	/* Each logical drive is bound to the same physical drive number */
	#define LD2PT(vol) 0			/* Always mounts the 1st partition or in SFD */
	
#endif



/* Type of path name strings on FatFs API */

#if _LFN_UNICODE			/* Unicode string */
#if !_USE_LFN
#error _LFN_UNICODE must be 0 in non-LFN cfg.
#endif
#ifndef _INC_TCHAR
typedef WCHAR TCHAR;
#define _T(x) L ## x
#define _TEXT(x) L ## x
#endif

#else						/* ANSI/OEM string */
#ifndef _INC_TCHAR
typedef char TCHAR;
#define _T(x) x
#define _TEXT(x) x
#endif

#endif

// File function return code (FRESULT)
typedef enum {
	FR_OK = 0,				/* (0) Succeeded */
	FR_DISK_ERR,			/* (1) A hard error occurred in the low level disk I/O layer */
	FR_INT_ERR,				/* (2) Assertion failed */
	FR_NOT_READY,			/* (3) The physical drive cannot work */
	FR_NO_FILE,				/* (4) Could not find the file */
	FR_NO_PATH,				/* (5) Could not find the path */
	FR_INVALID_NAME,		/* (6) The path name format is invalid */
	FR_DENIED,				/* (7) Access denied due to prohibited access or directory full */
	FR_EXIST,				/* (8) Access denied due to prohibited access */
	FR_INVALID_OBJECT,		/* (9) The file/directory object is invalid */
	FR_WRITE_PROTECTED,		/* (10) The physical drive is write protected */
	FR_INVALID_DRIVE,		/* (11) The logical drive number is invalid */
	FR_NOT_ENABLED,			/* (12) The volume has no work area */
	FR_NO_FILESYSTEM,		/* (13) There is no valid FAT volume */
	FR_MKFS_ABORTED,		/* (14) The f_mkfs() aborted due to any parameter error */
	FR_TIMEOUT,				/* (15) Could not get a grant to access the volume within defined period */
	FR_LOCKED,				/* (16) The operation is rejected according to the file sharing policy */
	FR_NOT_ENOUGH_CORE,		/* (17) LFN working buffer could not be allocated */
	FR_TOO_MANY_OPEN_FILES,	/* (18) Number of open files > _FS_SHARE */
	FR_INVALID_PARAMETER	/* (19) Given parameter is invalid */
} FRESULT;

// FatFs module application interface

FRESULT f_mount (BYTE vol, FileSystem* fs);								/* Mount/Unmount a logical drive */
FRESULT f_open (File* fp, const TCHAR* path, BYTE mode);				/* Open or create a file */
FRESULT f_read (File* fp, void* buff, UINT btr, UINT* br);			/* Read data from a file */
FRESULT f_lseek (File* fp, DWORD ofs);								/* Move file pointer of a file object */
FRESULT f_close (File* fp);											/* Close an open file object */
FRESULT f_opendir (Directory* dj, const TCHAR* path);						/* Open an existing directory */
FRESULT f_readdir (Directory* dj, FileInfo* fno);							/* Read a directory item */
FRESULT f_stat (const TCHAR* path, FileInfo* fno);					/* Get file status */
FRESULT f_write (File* fp, const void* buff, UINT btw, UINT* bw);	/* Write data to a file */
FRESULT f_getfree (const TCHAR* path, DWORD* nclst, FileSystem** fatfs);	/* Get number of free clusters on the drive */
FRESULT f_truncate (File* fp);										/* Truncate file */
FRESULT f_sync (File* fp);											/* Flush cached data of a writing file */
FRESULT f_unlink (const TCHAR* path);								/* Delete an existing file or directory */
FRESULT	f_mkdir (const TCHAR* path);								/* Create a new directory */
FRESULT f_chmod (const TCHAR* path, BYTE value, BYTE mask);			/* Change attribute of the file/dir */
FRESULT f_utime (const TCHAR* path, const FileInfo* fno);			/* Change times-tamp of the file/dir */
FRESULT f_rename (const TCHAR* path_old, const TCHAR* path_new);	/* Rename/Move a file or directory */
FRESULT f_chdrive (BYTE drv);										/* Change current drive */
FRESULT f_chdir (const TCHAR* path);								/* Change current directory */
FRESULT f_getcwd (TCHAR* buff, UINT len);							/* Get current directory */
FRESULT	f_getlabel (const TCHAR* path, TCHAR* label, DWORD* sn);	/* Get volume label */
FRESULT	f_setlabel (const TCHAR* label);							/* Set volume label */
FRESULT f_forward (File* fp, UINT(*func)(const BYTE*,UINT), UINT btf, UINT* bf);	/* Forward data to the stream */
FRESULT f_mkfs (BYTE vol, BYTE sfd, UINT au);						/* Create a file system on the drive */
FRESULT	f_fdisk (BYTE pdrv, const DWORD szt[], void* work);			/* Divide a physical drive into some partitions */
int f_putc (TCHAR c, File* fp);										/* Put a character to the file */
int f_puts (const TCHAR* str, File* cp);								/* Put a string to the file */
int f_printf (File* fp, const TCHAR* str, ...);						/* Put a formatted string to the file */
TCHAR* f_gets (TCHAR* buff, int len, File* fp);						/* Get a string from the file */

#define f_eof(fp) (((fp)->fptr == (fp)->fsize) ? 1 : 0)
#define f_error(fp) (((fp)->flag & FA__ERROR) ? 1 : 0)
#define f_tell(fp) ((fp)->fptr)
#define f_size(fp) ((fp)->fsize)

#ifndef EOF
#define EOF (-1)
#endif




/*--------------------------------------------------------------*/
/* Additional user defined functions                            */

/* RTC function */
#if !_FS_READONLY
DWORD get_fattime (void);
#endif

/* Unicode support functions */
#if _USE_LFN							/* Unicode - OEM code conversion */
WCHAR ff_convert (WCHAR chr, UINT dir);	/* OEM-Unicode bidirectional conversion */
WCHAR ff_wtoupper (WCHAR chr);			/* Unicode upper-case conversion */
#if _USE_LFN == 3						/* Memory functions */
void* ff_memalloc (UINT msize);			/* Allocate memory block */
void ff_memfree (void* mblock);			/* Free memory block */
#endif
#endif

/* Sync functions */
#if _FS_REENTRANT
int ff_cre_syncobj (BYTE vol, _SYNC_t* sobj);	/* Create a sync object */
int ff_req_grant (_SYNC_t sobj);				/* Lock sync object */
void ff_rel_grant (_SYNC_t sobj);				/* Unlock sync object */
int ff_del_syncobj (_SYNC_t sobj);				/* Delete a sync object */
#endif




/*--------------------------------------------------------------*/
/* Flags and offset address                                     */


// File access control and file status flags (File.flag)

#define	FA_READ				0x01
#define	FA_OPEN_EXISTING	0x00
#define FA__ERROR			0x80

#if !_FS_READONLY
#define	FA_WRITE			0x02
#define	FA_CREATE_NEW		0x04
#define	FA_CREATE_ALWAYS	0x08
#define	FA_OPEN_ALWAYS		0x10
#define FA__WRITTEN			0x20
#define FA__DIRTY			0x40
#endif

// File attribute bits for directory entry
#define	AM_RDO	0x01	// Read only
#define	AM_HID	0x02	// Hidden
#define	AM_SYS	0x04	// System
#define	AM_VOL	0x08	// Volume label
#define AM_LFN	0x0F	// LFN entry
#define AM_DIR	0x10	// Directory
#define AM_ARC	0x20	// Archive
#define AM_MASK	0x3F	// Mask of defined bits


/* Fast seek feature */
#define CREATE_LINKMAP	0xFFFFFFFF


/*--------------------------------*/
/* Multi-byte word access macros  */

#if _WORD_ACCESS == 1	/* Enable word access to the FAT structure */
#define	LD_WORD(ptr)		(WORD)(*(WORD*)(BYTE*)(ptr))
#define	LD_DWORD(ptr)		(DWORD)(*(DWORD*)(BYTE*)(ptr))
#define	ST_WORD(ptr,val)	*(WORD*)(BYTE*)(ptr)=(WORD)(val)
#define	ST_DWORD(ptr,val)	*(DWORD*)(BYTE*)(ptr)=(DWORD)(val)
#else					/* Use byte-by-byte access to the FAT structure */
#define	LD_WORD(ptr)		(WORD)(((WORD)*((BYTE*)(ptr)+1)<<8)|(WORD)*(BYTE*)(ptr))
#define	LD_DWORD(ptr)		(DWORD)(((DWORD)*((BYTE*)(ptr)+3)<<24)|((DWORD)*((BYTE*)(ptr)+2)<<16)|((WORD)*((BYTE*)(ptr)+1)<<8)|*(BYTE*)(ptr))
#define	ST_WORD(ptr,val)	*(BYTE*)(ptr)=(BYTE)(val); *((BYTE*)(ptr)+1)=(BYTE)((WORD)(val)>>8)
#define	ST_DWORD(ptr,val)	*(BYTE*)(ptr)=(BYTE)(val); *((BYTE*)(ptr)+1)=(BYTE)((WORD)(val)>>8); *((BYTE*)(ptr)+2)=(BYTE)((DWORD)(val)>>16); *((BYTE*)(ptr)+3)=(BYTE)((DWORD)(val)>>24)
#endif

#ifdef __cplusplus
}
#endif

#endif /* _FATFS */
