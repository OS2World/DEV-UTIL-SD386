/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   xsrvdbif.c                                                           827*/
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*  some x-server data access functions.( subset of dbif.c)                  */
/*                                                                           */
/* History:                                                                  */
/*                                                                           */
/*   06/23/93 Created.                                                       */
/*                                                                           */
/*...                                                                        */
/*... 06/04/93  827   Joe       Add remote debug support.                    */
/*...                                                                        */
/*****************************************************************************/
#include "all.h"

/*****************************************************************************/
/* Getnbytes;                                                                */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   gets n bytes of data from the app address space.                        */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   address    pointer to address space in user's application.              */
/*   nbytes     number of bytes to copy from the user's application.         */
/*   totlptr    pointer to number of bytes that were read in.                */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   p          pointer to Getnbytes() buffer that holds the bytes read in.  */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   address  is a flat address.                                             */
/*                                                                           */
/*****************************************************************************/
static UCHAR *p = NULL;
  UCHAR*
Getnbytes( ULONG address, UINT nbytes, UINT *totlptr  )
{
 APIRET       rc;
 PtraceBuffer ptb;

/*****************************************************************************/
/* - Free the storage from the prevoius call.                                */
/* - Allocate the new storage block.                                         */
/* - Read in all the bytes if we can.                                        */
/* - If we can't, then try reading one at a time and return the number       */
/*   read.                                                                   */
/*                                                                           */
/*****************************************************************************/

 if ( p != NULL )
   Tfree( (void*)p );
 p = (UCHAR*)Talloc( nbytes );

 *totlptr = 0;

 memset(&ptb,0,sizeof(ptb));
 ptb.Pid    = GetEspProcessID();
 ptb.Cmd    = DBG_C_ReadMemBuf ;
 ptb.Addr   = address;
 ptb.Buffer = (ULONG)p;
 ptb.Len    = nbytes ;
 rc = DosDebug( &ptb );
 *totlptr=ptb.Len;

 /***************************************************************************/
 /* If not able to read the number of bytes asked for , try reading byte    */
 /* by byte as many bytes as possible.                                      */
 /***************************************************************************/
 if(rc || ptb.Cmd != DBG_N_Success)
 {
  UINT i;
  *totlptr = 0;
  for (i = 0 ; i < nbytes ; i++)
  {
   memset(&ptb,0,sizeof(ptb));
   ptb.Pid    = GetEspProcessID();
   ptb.Cmd    = DBG_C_ReadMemBuf ;
   ptb.Addr   = address+i;
   ptb.Buffer = (ULONG)(p+i);
   ptb.Len    = 1;
   rc = DosDebug( &ptb );
   if(rc || ptb.Cmd!=DBG_N_Success)
       break;
   (*totlptr)++;
  }
 }
 return(p);
}

/*****************************************************************************/
/* Putnbytes                                                                 */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   puts data into the user's application address space.                    */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   address    pointer to address space in user's application.              */
/*   nbytes     number of bytes to copy to the user's application.           */
/*   source     pointer to buffer of data that is targeted for the app.      */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   rc         whether it worked (0) or not (1).                            */
/*                                                                           */
/*****************************************************************************/
UINT Putnbytes( ULONG address, UINT nbytes, UCHAR *source)
{
 APIRET       rc;
 PtraceBuffer ptb;

 memset(&ptb,0,sizeof(ptb));
 ptb.Pid    = GetEspProcessID();
 ptb.Cmd    = DBG_C_WriteMemBuf;
 ptb.Addr   =  address;
 ptb.Buffer = (ULONG)source;
 ptb.Len    = nbytes;
 rc = DosDebug( &ptb );
 if(rc || (ptb.Cmd != DBG_N_Success ))
   return ( 1 );

 return ( 0 );
}

/*****************************************************************************/
/* GetExeOrDllEntryOrExitPt()                                             116*/
/*                                                                        116*/
/* Description:                                                           116*/
/*  Get the location of the dll initialization or exit code or the exe    116*/
/*  entry pt.                                                             116*/
/*                                                                        116*/
/* Parameters:                                                            116*/
/*                                                                        116*/
/*  mte             - the EXE/DLL handle.                                 116*/
/*                                                                        116*/
/* Return:                                                                116*/
/*                                                                        116*/
/*            entry point converted to flat addr.                         116*/
/*                                                                        116*/
/* Assumptions:                                                           116*/
/*                                                                        116*/
/*                                                                        116*/
/*************************************************************************116*/
#define OFFSETOFOS2HDR   0x3CL          /* loc contains offset of start of116*/
                                        /* OS/2 exe header.               116*/
                                        /*                                116*/
/* The following offsets are relative to the start of the EXE header      116*/
/* for 16 bit EXE/DLLs.                                                   116*/
                                        /*                                116*/
#define OFF_OF_ENTRY_PT_OBJECT_16 0x16L /* loc with entry pt object #.    116*/
#define OFF_OF_ENTRY_PT_OFFSET_16 0x14L /* loc with entry pt offset.      116*/
#define SIZE_OF_ENTRY_PT_OBJECT_16 2    /* bytes to read for object.      116*/
#define SIZE_OF_ENTRY_PT_OFFSET_16 2    /* bytes to read for offset.      116*/
                                                                        /*116*/
/* The following offsets are relative to the start of the EXE header      116*/
/* for 32 bit EXE/DLLs.                                                   116*/
                                                                        /*116*/
#define OFF_OF_ENTRY_PT_OBJECT_32 0x18L /* loc with entry pt object #.    116*/
#define OFF_OF_ENTRY_PT_OFFSET_32 0x1CL /* loc with entry pt offset.      116*/
#define SIZE_OF_ENTRY_PT_OBJECT_32 4    /* bytes to read for object.      116*/
#define SIZE_OF_ENTRY_PT_OFFSET_32 4    /* bytes to read for offset.      116*/
                                                                        /*116*/
#define MAXBUFFERSIZE     4             /* max read buffer size.          116*/
                                        /*                                116*/
ULONG XSrvGetExeOrDllEntryOrExitPt( UINT mte )
{
 char   ModuleName[CCHMAXPATH];
 FILE  *fpModule;
 char   buffer[MAXBUFFERSIZE];
 ULONG  StartOS2Hdr;                    /* loc of the OS/2 EXE/DLL hdr.      */
 UCHAR  ExeType;                        /* 16 bit/32 bit EXE/DLL.         116*/
 int    NumBytesToRead;                 /* bytes to read from the file.   116*/
 UINT   OffOfEntryPtObject;             /* file loc of object #.          116*/
 UINT   OffOfEntryPtOffset;             /* file loc of offset.            116*/
 UINT   EntryPtObject;                  /* object#. converts to load addr.116*/
 UINT   EntryPtLoadAddr;
 UINT   EntryPtOffset;

 /****************************************************************************/
 /* - Get the module name.                                                   */
 /* - Open the module and if it fails then proceed as if the address         */
 /*   is not available.                                                      */
 /****************************************************************************/
 DosQueryModuleName( mte,CCHMAXPATH,ModuleName);
 fpModule = fopen( ModuleName,"rb");
 if( fpModule == NULL )
  return(0);

/*************************************************************************116*/
/* - Get the location of the OS/2 EXE/DLL header.                         116*/
/* - Establish 16 or 32 bitness.                                          116*/
/* - Read the location containing the entry point object and offset.      116*/
/* - Convert object number to load address.                               116*/
/* - Compute the entry point and put in exe/dll struct.                   116*/
/*                                                                        116*/
/*************************************************************************116*/
 fseek(fpModule,OFFSETOFOS2HDR,SEEK_SET);                               /*116*/
 fread( buffer,1,4,fpModule);                                           /*116*/
 StartOS2Hdr = *( ULONG *)buffer;                                       /*116*/

 /************************************************************************116*/
 /* Establish 16 or 32 bitness.                                           116*/
 /************************************************************************116*/
 fseek(fpModule, StartOS2Hdr,SEEK_SET);                                 /*116*/
 fread( buffer , 1,1 , fpModule);                                       /*116*/
 ExeType = BIT16;                                                       /*116*/
 if( buffer[0] == 'L' )                                                 /*116*/
  ExeType = BIT32;                                                      /*116*/

 /************************************************************************116*/
 /* Read the entry point object number.                                   116*/
 /************************************************************************116*/
 OffOfEntryPtObject = OFF_OF_ENTRY_PT_OBJECT_16;                        /*116*/
 NumBytesToRead     = SIZE_OF_ENTRY_PT_OBJECT_16;                       /*116*/
 if ( ExeType == BIT32 )                                                /*116*/
 {                                                                      /*116*/
  OffOfEntryPtObject = OFF_OF_ENTRY_PT_OBJECT_32;                       /*116*/
  NumBytesToRead     = SIZE_OF_ENTRY_PT_OBJECT_32;                      /*116*/
 }                                                                      /*116*/
 memset(buffer,0,sizeof(buffer) );                                      /*116*/
 fseek(fpModule, StartOS2Hdr + OffOfEntryPtObject,SEEK_SET);            /*116*/
 fread( buffer , 1,NumBytesToRead , fpModule);                          /*116*/
 EntryPtObject  = *( int *)buffer;                                      /*116*/

 /****************************************************************************/
 /* Convert the entry point object to a flat address.                     822*/
 /****************************************************************************/
 EntryPtLoadAddr = _GetLoadAddr(mte,EntryPtObject);                     /*827*/

 /************************************************************************116*/
 /* Read the entry point offset.                                          116*/
 /************************************************************************116*/
 OffOfEntryPtOffset = OFF_OF_ENTRY_PT_OFFSET_16;                        /*116*/
 NumBytesToRead     = SIZE_OF_ENTRY_PT_OFFSET_16;                       /*116*/
 if ( ExeType == BIT32 )                                                /*116*/
 {                                                                      /*116*/
  OffOfEntryPtOffset = OFF_OF_ENTRY_PT_OFFSET_32;                       /*116*/
  NumBytesToRead     = SIZE_OF_ENTRY_PT_OFFSET_32;                      /*116*/
 }                                                                      /*116*/
 memset(buffer,0,sizeof(buffer) );                                      /*116*/
 fseek(fpModule, StartOS2Hdr + OffOfEntryPtOffset,SEEK_SET);            /*116*/
 fread( buffer , 1, NumBytesToRead , fpModule);                         /*116*/
 EntryPtOffset  = *( int *)buffer;                                      /*116*/
                                                                        /*116*/
 fclose(fpModule);
 return( EntryPtLoadAddr + EntryPtOffset );

/*****************************************************************************/
/* error handling for GetExeOrDllEntryOrExitPt.                              */
/*****************************************************************************/
error:                                  /*                                   */
 if( fpModule )
  fclose(fpModule);
 return(0);                             /* dump errors into this bucket.  116*/
}                                       /* end GetExeOrDllEntryOrExitPt.  116*/

/*****************************************************************************/
/* GetExeStackTop                                                         800*/
/*                                                                           */
/* Description:                                                              */
/*  Get a pointer to the top of the stack.                                   */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*  pdf       input  - EXE file structure.                                   */
/*  pss       output - where to stuff the SS selector for the caller.        */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*  flattop   flat address top of stack.                                     */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
#define OFFSETOFOS2HDR   0x3CL          /* loc contains offset of start of   */
                                        /* OS/2 exe header.                  */
                                        /*                                   */
/* The following offsets are relative to the start of the EXE header         */
/* for 16 bit EXE/DLLs.                                                      */
                                        /*                                   */
#define OFF_OF_SS               0x1A    /* 16 bit object# location.          */
#define SIZE_OF_SS              2       /* bytes to read.                    */

#define OFF_OF_SP               0x18    /* 16 bit SP location.               */
#define SIZE_OF_SP              2       /* bytes to read.                    */

#define OFF_OF_STACK_SIZE       0x12    /* 16 bit stack size location.       */
#define SIZE_OF_STACK_SIZE      2       /* bytes to read.                    */

#define OFF_OF_STACK_OBJNUM_32  0x20    /* 32 bit object# location.          */
#define SIZE_OF_STACK_OBJNUM_32 4       /* bytes to read.                    */

#define OFF_OF_ESP_32           0x24    /* loc with entry stack size.        */
#define SIZE_OF_ESP_32          4       /* bytes to read for stack size.     */

#if 0

#define OFF_OF_ENTRY_PT_OFFSET_16 0x14L /* loc with entry pt offset.         */

/* The following offsets are relative to the start of the EXE header         */
/* for 32 bit EXE/DLLs.                                                      */

#define OFF_OF_ENTRY_PT_OBJECT_32 0x18L /* loc with entry pt object #.       */
#define OFF_OF_ENTRY_PT_OFFSET_32 0x1CL /* loc with entry pt offset.         */
#define SIZE_OF_ENTRY_PT_OBJECT_32 4    /* bytes to read for object.         */
#define SIZE_OF_ENTRY_PT_OFFSET_32 4    /* bytes to read for offset.         */
#endif

#define MAXBUFFERSIZE     4             /* max read buffer size.             */
                                        /*                                   */
ULONG GetExeStackTop(UINT mte , USHORT *pStackSel , ULONG *pStackSize)
{
 char         ModuleName[CCHMAXPATH];
 FILE        *fpModule;
 char         buffer[MAXBUFFERSIZE];    /* read buffer.                      */
 ULONG        StartOS2Hdr;              /* loc of the OS/2 EXE/DLL hdr.      */
 PtraceBuffer ptb;                      /* DosDebug buffer.                  */
 UCHAR        ExeType;                  /* 16 bit/32 bit EXE/DLL.            */
 int          NumBytesToRead;           /* bytes to read from the file.      */
 UINT         rc;                       /*                                   */

 UINT           OffOfStackObject;
 UINT           StackObject;
 UINT           StackObjectAddress;
 UINT           OffOfStackSize;
 UINT           StackSize;
 UINT           OffOfESP;
 UINT           ESP;
 UINT           DgroupSize;
 OBJTABLEENTRY *te;

 /****************************************************************************/
 /* - Get the module name.                                                   */
 /* - Open the module and if it fails then proceed as if the address         */
 /*   is not available.                                                      */
 /****************************************************************************/
 DosQueryModuleName( mte,CCHMAXPATH,ModuleName);
 fpModule = fopen( ModuleName,"rb");
 if( fpModule == NULL )
  return(0);

/*****************************************************************************/
/* - Get the location of the OS/2 EXE/DLL header.                            */
/* - Establish 16 or 32 bitness.                                             */
/* - Read the location containing the entry point object and offset.         */
/* - Convert object number to load address.                                  */
/* - Compute the entry point and put in exe/dll struct.                      */
/*                                                                           */
/*****************************************************************************/
 fseek(fpModule,OFFSETOFOS2HDR,SEEK_SET);
 fread( buffer,1,4,fpModule);
 StartOS2Hdr = *( ULONG *)buffer;

 /****************************************************************************/
 /* Establish 16 or 32 bitness.                                              */
 /****************************************************************************/
 fseek(fpModule, StartOS2Hdr,SEEK_SET);
 fread( buffer , 1,1 , fpModule);
 ExeType = BIT16;
 if( buffer[0] == 'L' )
  ExeType = BIT32;

 /****************************************************************************/
 /* Get the object number of SS.                                             */
 /****************************************************************************/
 OffOfStackObject = OFF_OF_SS;
 NumBytesToRead   = SIZE_OF_SS;
 if ( ExeType == BIT32 )
 {
  OffOfStackObject = OFF_OF_STACK_OBJNUM_32;
  NumBytesToRead   = SIZE_OF_STACK_OBJNUM_32;
 }
 memset(buffer,0,sizeof(buffer) );
 fseek(fpModule, StartOS2Hdr + OffOfStackObject,SEEK_SET);
 fread( buffer ,1, NumBytesToRead , fpModule);
 StackObject  = *( int *)buffer;

 /****************************************************************************/
 /* Convert SS to its flat address location and give it back to caller.      */
 /****************************************************************************/
 StackObjectAddress = _GetLoadAddr(mte,StackObject);

 /****************************************************************************/
 /* Read the (E)SP.                                                          */
 /****************************************************************************/
 OffOfESP       = OFF_OF_SP;
 NumBytesToRead = SIZE_OF_SP;
 if ( ExeType == BIT32 )
 {
  OffOfESP = OFF_OF_ESP_32;
  NumBytesToRead   = SIZE_OF_ESP_32;
 }
 memset(buffer,0,sizeof(buffer) );
 fseek(fpModule, StartOS2Hdr + OffOfESP,SEEK_SET);
 fread( buffer ,1, NumBytesToRead , fpModule);
 ESP  = *( int *)buffer;

 /****************************************************************************/
 /* The linkers will put an ss:e(sp) in the exe header. There are two        */
 /* cases to consider:                                                       */
 /*                                                                          */
 /* 1. e(sp) == 0. This happens with linker version 5.10.005 which is        */
 /*                part of MSC 6.00. In this case, there will be a           */
 /*                valid stacksize, so we compute the top of the stack       */
 /*                as follows:                                               */
 /*                                                                          */
 /*                 tos = StackObjectAddress + DgroupSize + StackSize.       */
 /*                                                                          */
 /*                                                                          */
 /*                The DgroupSize in the exehdr is the size of DGROUP -      */
 /*                the stack size.                                           */
 /*                                                                          */
 /*                                                                          */
 /* 2. e(sp) != 0. This happens with link386 and linker version 1.1          */
 /*                which is part of C/2 1.1. In this case, the top           */
 /*                of the stack will be as follows:                          */
 /*                                                                          */
 /*                 tos = StackObjectAddress + ESP.                          */
 /*                                                                          */
 /*                 The Stack size given in the EXE header will be zero.     */
 /*                 The Dgroup size will include the stack size. We have     */
 /*                 no way of knowing how much of the Dgroupsize is for      */
 /*                 stack and how much is for data.                          */
 /*                                                                          */
 /*                                                                          */
 /*                                                                          */
 /*                | e(sp) = 0             | e(sp) != 0                      */
 /*   -------------------------------------|----------------------------     */
 /*   Top of stack | StackObjectAddress +  | StackObjectAddress +            */
 /*                | DgroupSize +          | ESP                             */
 /*                | StackSize             |                                 */
 /*                |                       | ( stacksize = 0 )               */
 /*                |                       | ( dgroupsize includes stacksize)*/
 /*   -------------------------------------------------------------------    */
 /*   StackSize    | StackSize             |  0                              */
 /*                                                                          */
 /*                                                                          */
 /****************************************************************************/
 if( ESP )
 {
   /**************************************************************************/
   /* Handle e(sp) != 0.                                                     */
   /**************************************************************************/
   *pStackSize = 0;

   memset(&ptb,0,sizeof(ptb));
   ptb.Pid   = GetEspProcessID();
   ptb.Cmd   = DBG_C_LinToSel;
   ptb.Addr  = StackObjectAddress;
   rc = DosDebug( &ptb );
   if(rc || ptb.Cmd != 0)
    goto error;
   *pStackSel = (USHORT)ptb.Value;

   return( StackObjectAddress + ESP );
 }

 /****************************************************************************/
 /* Handle e(sp) == 0.                                                       */
 /****************************************************************************/
 OffOfStackSize   = OFF_OF_STACK_SIZE;
 NumBytesToRead   = SIZE_OF_STACK_SIZE;
 memset(buffer,0,sizeof(buffer) );
 fseek(fpModule, StartOS2Hdr + OffOfStackSize,SEEK_SET);
 fread( buffer ,1, NumBytesToRead , fpModule);
 *pStackSize = StackSize  = *( int *)buffer;

 /****************************************************************************/
 /* Get the size of DGROUP.                                                  */
 /****************************************************************************/
 te = GetPtrToObjectTable(mte);
 DgroupSize = te[StackObject-1].ObjLoadSize;

 /****************************************************************************/
 /* Let's give the caller the selector for the stack object.                 */
 /****************************************************************************/
 memset(&ptb,0,sizeof(ptb));
 ptb.Pid   = GetEspProcessID();
 ptb.Cmd   = DBG_C_LinToSel;
 ptb.Addr  = StackObjectAddress;
 rc = DosDebug( &ptb );
 if(rc || ptb.Cmd != 0)
  goto error;

 *pStackSel = (USHORT)ptb.Value;

 /****************************************************************************/
 /* The value returned will be slightly different from the one you'll see    */
 /* at load time. We use the actual DGROUP size instead of the size in       */
 /* memory. There is a slight difference in these two numbers.               */
 /****************************************************************************/
 fclose(fpModule);
 return( StackObjectAddress + DgroupSize + StackSize );

error:                                  /*                                   */
 if( fpModule )
  fclose(fpModule);
 return(0);                             /* dump errors into this bucket.     */

}

/*****************************************************************************/
/* XSrvGetMemoryBlock()                                                      */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*  Get a block of data.                                                     */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*  addr            location from where we want the data.                    */
/*  BytesWanted     the number of bytes we want.                             */
/*  pBytesObtained  the number of bytes we got.                              */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*  pbytes        -> to the memory block  requested.                         */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
UCHAR *XSrvGetMemoryBlock( ULONG addr, int BytesWanted, int *pBytesObtained )
{
 UCHAR *pbytes = NULL;

 pbytes = Getnbytes( addr, BytesWanted , (UINT*)pBytesObtained );
 return(pbytes);
}


/*****************************************************************************/
/* XSrvGetMemoryBlocks()                                                     */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*  Get some memory blocks.                                                  */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*  pDefBlks         -> to an array of addresses and sizes of mem blocks.    */
/*  pLengthOfMemBlks -> total size of all blocks + addr + size.              */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*  pMemBlks      -> to the blocks of memory requested.                      */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*  The pDefBlks array is terminated with a (NULL,0) entry.                  */
/*  The caller will free the MemBlks.                                        */
/*                                                                           */
/*****************************************************************************/
void *XSrvGetMemoryBlocks(void *pDefBlks, int *pMemBlkSize )
{
 typedef struct
 {
  ULONG addr;
  int   size;
 }MEM_BLK_DEF;
 MEM_BLK_DEF  *pd = (MEM_BLK_DEF*)pDefBlks;

 typedef struct
 {
  ULONG addr;
  int   size;
  char  bytes[1];
 }MEM_BLKS;
 MEM_BLKS  *pMemBlks;
 MEM_BLKS  *pmblk;
 int        n;
 UINT       BytesObtained;
 UCHAR     *pbytes;
 int        increment;
 int        LengthOfMemBlks;
 int        MemBlkSize;


 /****************************************************************************/
 /* - get the size of the memory block area and bump pointer.                */
 /****************************************************************************/
 LengthOfMemBlks = *(int *)pd;
 pd = (MEM_BLK_DEF*)((char*)pd + sizeof(int));

 /****************************************************************************/
 /* - allocate memory for the memory blocks.                                 */
 /****************************************************************************/
 pmblk = pMemBlks = Talloc( LengthOfMemBlks );

 MemBlkSize = 0;
 for( n = 0; pd[n].addr != NULL; n++ )
 {
  pbytes = Getnbytes( pd[n].addr, pd[n].size , &BytesObtained );
  pmblk->addr = pd[n].addr;
  pmblk->size = BytesObtained;
  memcpy( pmblk->bytes, pbytes, BytesObtained );
  increment = sizeof(MEM_BLKS) - 1 + BytesObtained;
  MemBlkSize += increment;
  pmblk = (MEM_BLKS*)( (UCHAR*)pmblk + increment);
 }

 *pMemBlkSize = MemBlkSize;
 return( (void *)pMemBlks );
}

