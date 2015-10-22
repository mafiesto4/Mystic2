//////////////////////////////////////////////////////////////////////////////////////
// Mystic OS Copyright 2012-2013 (c) Wojciech Figat
// The software is provided 'as is'. 
// The original author is not responsible for any harm which may result from use of this software or its source code
// See also my 3d game engine --> http://celelej.com
//////////////////////////////////////////////////////////////////////////////////////

volatile unsigned int _stack;

//#include "timer.h"

//extern uint32_t timer0_counter;

#include "Kernel\Kernel.h"

/*
Some code history:
Program Size: Code=24014 RO-data=2214 RW-data=56 ZI-data=5256  
Program Size: Code=23926 RO-data=2214 RW-data=48 ZI-data=5272  
Program Size: Code=23930 RO-data=2214 RW-data=48 ZI-data=5272  
Program Size: Code=23938 RO-data=2214 RW-data=48 ZI-data=5272  
Program Size: Code=23918 RO-data=2214 RW-data=48 ZI-data=5272  
Program Size: Code=23926 RO-data=2214 RW-data=44 ZI-data=5268  
Program Size: Code=23942 RO-data=2214 RW-data=44 ZI-data=5268  
Program Size: Code=23954 RO-data=2214 RW-data=44 ZI-data=5268 
Program Size: Code=24194 RO-data=2214 RW-data=44 ZI-data=5268  
Program Size: Code=24194 RO-data=2214 RW-data=44 ZI-data=5268  
Program Size: Code=24166 RO-data=2214 RW-data=44 ZI-data=5268  
Program Size: Code=23978 RO-data=2214 RW-data=44 ZI-data=5268  
Program Size: Code=23982 RO-data=2214 RW-data=44 ZI-data=5268   
Program Size: Code=24026 RO-data=2258 RW-data=44 ZI-data=5268  
Program Size: Code=24570 RO-data=2294 RW-data=44 ZI-data=5268  
Program Size: Code=24690 RO-data=2206 RW-data=44 ZI-data=5268  
Program Size: Code=24004 RO-data=2068 RW-data=52 ZI-data=5268  


*/

/*
FIL fsrc;        
FRESULT res;
UINT br;

char path[512]="0:";
uint8_t textFileBuffer[] = "Mystic OS 2.0 testing file ^_^ \r\n";   

int SD_TotalSize(void);
FRESULT scan_files (char* path);
*/

int main(void)
{
	/*
	uint32_t counter = 0;
   
  LPC_GPIO2->FIODIR = 0x000000FF;		// P2.0...P2.7 defined as Outputs
  LPC_GPIO2->FIOCLR = 0x000000FF;		// turn off all the LEDs
    
  init_timer( 0, TIME_INTERVAL ); // 10ms	
  enable_timer( 0 );

  while (1) 
  {	
	if ( timer0_counter >= (50 * counter) )	   // 0.5s
	{
	  LPC_GPIO2->FIOSET = 1 << counter;
	  counter++;
	  if ( counter > 8 )
	  {
		counter = 0;	
		timer0_counter = 0;
		LPC_GPIO2->FIOCLR = 0x000000FF;
	  }
	}
  }
  */
  
	// Init Mystic
	OS_Init();

	// Run Mystic
	OS_Run();
	
	/*
	res = f_open( &fsrc , _T("0:/Demo.TXT") , FA_CREATE_NEW | FA_WRITE);		

    if ( res == FR_OK )
    { 
      //Write buffer to file
      res = f_write(&fsrc, textFileBuffer, sizeof(textFileBuffer), &br);     
		
	  printf("Demo.TXT successfully created        \r\n");
    	GUI_Text( 5 , 50 , "DEMO.txt created" , White );

      //close file
      f_close(&fsrc);      
    }
    else if ( res == FR_EXIST )
    {
	  printf("Demo.TXT created in the disk      \r\n");
	  GUI_Text( 5 , 50 , "DEMO.txt already created" , White );
    }
	*/
	// Print application date and time
		/*
		res += OS_FS_Seek (&New->File, New->Exe.Reg[8]-4);
		{
			Time t2;
			PackedTime t1;
			Byte D[4];
			
			OS_FS_Read( &New->File , D , 4 );
	
			*((Byte*)(&t1) + 3) = D[3];
			*((Byte*)(&t1) + 2) = D[2];
			*((Byte*)(&t1) + 1) = D[1];
			*((Byte*)(&t1) + 0) = D[0];
			
			t2 = OS_Time_PackedToFull( t1 );
			
			OS_Debug_Print( "Second %d Minute %d Hour %d Day %d Month %d Year %d",
			t2.Second,									
			t2.Minute,									
			t2.Hour,								
			t2.Day,										
			t2.Month,					
			t2.Year);
		}
		*/
		
	//SystemInit();
	
	//scan_files(path);
	
	//SD_TotalSize();
}

/*
FRESULT scan_files (char* path)
{
    FILE_INFO fno;
    DIR dir;
    int i;
    char *fn;
#if _USE_LFN
    static char lfn[_MAX_LFN * (_DF1S ? 2 : 1) + 1];
    fno.LName = (TCHAR*)lfn;
    fno.LSize = sizeof(lfn);
#endif

    res = f_opendir(&dir, (TCHAR*)path);
    if (res == FR_OK) {
        i = strlen(path);
        for (;;) {
            res = f_readdir(&dir, &fno);
            if (res != FR_OK || fno.Name[0] == 0) break;
            if (fno.Name[0] == '.') continue;
#if _USE_LFN
            fn = (char*)(*fno.LName ? fno.LName : fno.Name);
#else
            fn = fno.Name;
#endif
            if (fno.Attribute & AM_DIR) {
                sprintf(&path[i], "/%s", fn);
                res = scan_files(path);
                if (res != FR_OK) break;
                path[i] = 0;
            } else {
                printf("%s/%s \r\n", path, fn);
            }
        }
    }
    return res;
}

int SD_TotalSize(void)
{
    FAT *fs;
    DWORD fre_clust;        
	
    res = f_getfree(_T("0:"), &fre_clust, &fs);
    if ( res==FR_OK ) 
    {
	  //Print free space in unit of MB (assuming 512 bytes/sector)
      printf("\r\n%d MB total drive space.\r\n"
           "%d MB available.\r\n",
           ( (fs->n_fatent - 2) * fs->csize ) / 2 /1024 , (fre_clust * fs->csize) / 2 /1024 );
		
	  return ENABLE;
	}
	else 
	  return DISABLE;
}	 
*/




