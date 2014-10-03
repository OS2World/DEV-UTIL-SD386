/*****************************************************************************/
/* File:                                                                     */
/*   debfile.c                                                               */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Read the debug file and setup the MODULE linked list.                   */
/*                                                                           */
/*                                                                           */
/* History:                                                                  */
/*                                                                           */
/*   04/06/95 Updated.                                                       */
/*                                                                           */
/*****************************************************************************/

#include "all.h"

#define __MIG_LIB__
#include <io.h>

/*****************************************************************************/
/* debfileinit()                                                             */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   pdf        pointer to debug file structure.                             */
/*                                                                           */
/* Return:                                                                   */
/*   rc         0 ==> success                                                */
/*              1 ==> failure                                                */
/* Assumptions:                                                              */
/*                                                                           */
/*  pdf -> EXE or DLL that contains a valid debug info block.                */
/*                                                                           */
/*****************************************************************************/
APIRET debfileinit( DEBFILE *pdf )
{
 ULONG   DebHdrOff;
 ULONG   NumEntries;
 ULONG   DirectorySize = 0;
 USHORT  usNumEntries;
 ULONG   ulNumEntries;
 UCHAR  *pDirectoryBuffer = NULL;

 pdf->MidAnchor = NULL;

 /*****************************************************************************/
 /* - Move the file pointer to the start of the debug information past        */
 /*   the first 4 bytes of the debug signature( NB00, etc. ).                 */
 /* - Read the next 4 bytes. This is the lfo from the start of the debug      */
 /*   info to the start of the directory header.                              */
 /*****************************************************************************/
 DebHdrOff = 0;
 seekf( pdf, pdf->DebugOff+4 );
 if( readf( (UCHAR *)&DebHdrOff, 4, pdf) )
  return(1);

 /*****************************************************************************/
 /* - The following code gets the number of directory entries.                */
 /* - In the HLL format, the number of directory entries is contained in      */
 /*   the 4 bytes following the first 4 bytes of useless information.         */
 /* - In the other formats, the number of header entries is contained         */
 /*   in the 2 bytes at the start of the directory.                           */
 /*****************************************************************************/
 NumEntries = usNumEntries = ulNumEntries = 0;
 if( pdf->ExeFlags.NBxxType != NB04 )
 {
  seekf( pdf, pdf->DebugOff + DebHdrOff );
  if( readf( (UCHAR*)&usNumEntries, 2, pdf ) )
   return(1);
  NumEntries = usNumEntries;
 }
 else
 {
  seekf(pdf, pdf->DebugOff + DebHdrOff + 4 );
  if( readf( (UCHAR*)&ulNumEntries, 4, pdf ) )
   return(1);
  NumEntries = ulNumEntries;
 }
 /*****************************************************************************/
 /* Check if the number of header entries is zero (!!). It happens in some    */
 /* system DLL with some bogus debug info in them. If so report an error.     */
 /*****************************************************************************/
 if( NumEntries == 0 )
  return(1);

 /******************************************************************************/
 /* - Calculate the directory block size from the number of entries.           */
 /* - Allocate memory for the directory block buffer.                          */
 /* - Read the directory block into the buffer.                                */
 /******************************************************************************/
 if( pdf->ExeFlags.NBxxType == NB04 )
  DirectorySize = NumEntries * sizeof( HDRENTRYHLL );
 else
  DirectorySize = NumEntries * sizeof( HDRENTRY );

 pDirectoryBuffer = (char *)Talloc( DirectorySize );

 if( readf( pDirectoryBuffer, DirectorySize, pdf ) )
 {
  if( pDirectoryBuffer ) Tfree( pDirectoryBuffer );
   return(1);
 }

 /*****************************************************************************/
 /* - Call InitModules to build the module linked list.                       */
 /*****************************************************************************/
 InitModules( pDirectoryBuffer, NumEntries, pdf);

  /***************************************************************************/
  /* - Call InitModuleAddrs to convert module object numbers to addresses.   */
  /* - Call SetDebugFormatFlags to set the various flags in the modules.     */
  /* - Free the directory and file name block buffers.                       */
  /***************************************************************************/
  if( InitModuleAddrs( pdf ) )
    return(1);

#if 0
/*---------------------------------------------------------------------------*/
/* - dump the modules.                                                       */
/*****************************************************************************/
{
 MODULE *pModule;

 for( pModule = pdf->MidAnchor; pModule; pModule = pModule->NextMod )
 {
  DumpModuleStructure( pModule );
 }
}
/*---------------------------------------------------------------------------*/
#endif

  SetDebugFormatFlags( pDirectoryBuffer, NumEntries, pdf );

  Tfree( pDirectoryBuffer );
  return( 0 );
}

/*****************************************************************************/
/*  FindDebugStart()                                                         */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Gets some information about the exe/dll and finds the start of the      */
/*   debug information.                                                      */
/*                                                                           */
/* Parameters:                                                               */
/*   FileHandle  file handle returned from DosOpen().                        */
/*   pDebugOff   -> receiver of lfo for the debug info.                      */
/*   pNBxx       -> receiver of debug info signature...NB00, NB02, or NB04.  */
/*   pExeType    -> receiver of executable type...LX or NE.                  */
/*                                                                           */
/* Return:                                                                   */
/*   rc      0 - OK for source level or assembler level.                     */
/*           1 - error.                                                      */
/*                                                                           */
/*****************************************************************************/
#define  BLOB  512
APIRET FindDebugStart( HFILE  FileHandle,
                       ULONG *pDebugOff,
                       int   *pNBxx,
                       int   *pExeType )
{
 ULONG   EndOfFile;
 ULONG   BackOff;
 ULONG   LfoOfExeHdr;
 UCHAR   buffer[BLOB];
 UCHAR  *bp;
 UCHAR  *cp;
 UCHAR  *sp;
 ULONG   blob;
 char    DebugSignature[5];
 int     ExeType;
 USHORT  NE_or_LX;
 int     NBxx;
 char    NByy[4];

 /****************************************************************************/
 /* - Get the type of the executable, NE or LX.                              */
 /* - Init the file pointer to the lfo that contains the lfo of the start    */
 /*   of the EXE header.                                                     */
 /* - Read the lfo of the EXE header.                                        */
 /* - Set the file pointer to the lfo of the EXE header.                     */
 /* - Read the first 2 bytes of the EXE header( should be NE or LX )         */
 /****************************************************************************/
 if(
    (seekdos(FileHandle, (ULONG)LFO_OF_EXE_HEADER_LFO, SEEK_SET)) ||
    (readdos(&LfoOfExeHdr, sizeof(LfoOfExeHdr), 1, FileHandle)  ) ||
    (seekdos(FileHandle, LfoOfExeHdr, SEEK_SET)                 ) ||
    (readdos(&NE_or_LX, 2, 1, FileHandle)                       )
   )
  return(1);

 ExeType = 0;
 if( !memcmp(&NE_or_LX, "LX", 2) )
  ExeType = LX;
 else if( !memcmp(&NE_or_LX, "NE", 2) )
  ExeType = NE;
 else
  return(1);

 *pExeType = ExeType;

/*****************************************************************************/
/* - We are now going to get the NBxx signature to determine whether or      */
/*   not this file contains debug information.                               */
/* - Set the file pointer to the end-of_file.                                */
/* - Get the lfo of the end-of-file.                                         */
/* - scan backwards BLOB bytes from the end of the file.                     */
/* - make adjustments if the file is smaller than BLOB bytes.                */
/* - allocate a blob and read the bytes.                                     */
/*                                                                           */
/*  Note:                                                                    */
/*   Due to possible padding at the end of a file, we cannot establish the   */
/*   absolute position of the debug signature.  So, we must scan backwards   */
/*   a reasonable amount looking for it. This is why we use the blob bytes.  */
/*                                                                           */
/*****************************************************************************/
 if( seekdos(FileHandle,0L,SEEK_END) )
  return(1);
 EndOfFile= tell(FileHandle);

 blob=BLOB;
 if(EndOfFile < blob)
   blob=(UINT)EndOfFile;

 memset(buffer, 0, sizeof(buffer) );
 if( (seekdos(FileHandle, EndOfFile-(ULONG)blob, SEEK_SET) ) ||
     (readdos(buffer, blob, 1, FileHandle)                 )
   )
  return(1);

/*****************************************************************************/
/* -  At this point we have the ending blob of the file in a buffer.         */
/* -  Now, start backing up looking for a valid NBxx signature. We can       */
/*    start 8 bytes from the end.                                            */
/* -  Also, the "true" end-of-file will be set.                              */
/*****************************************************************************/
 strcpy(DebugSignature,"NB00");
 NBxx = 0;
 bp   = buffer;
 for(cp=bp+blob-8;
     cp>=bp;
     cp--,EndOfFile--
    )
 {
  sp = cp;
  if( *sp++ == 'N' &&
      *sp++ == 'B' &&
      *sp++ == '0'
    )
  {
   if( *sp == '0' )
   {
    NBxx = NB00;
    break;
   }
   if( *sp == '2' )
   {
    NBxx = NB02;
    break;
   }
   if ( *sp == '4')
   {
    NBxx = NB04;
    break;
   }
  }
 }
 *pNBxx = NBxx;

 /****************************************************************************/
 /* - Now, we need to get the lfo of the start of the debug info.            */
 /* - If NBxx == 0, then this executable does not have debug info.           */
 /****************************************************************************/
 if( NBxx == 0 )
 {
  *pDebugOff = 0;
  return(0);
 }

 /****************************************************************************/
 /* - Define the debug signature string.                                     */
 /****************************************************************************/
 DebugSignature[0] = 'N';
 DebugSignature[1] = 'B';
 DebugSignature[2] = '0';
 if( NBxx == NB00 )
  DebugSignature[3] = '0';
 else if( NBxx == NB02 )
  DebugSignature[3] = '2';
 else /* NBxx == NB04 */
  DebugSignature[3] = '4';

 /****************************************************************************/
 /* - At this point, we have the lfo for the "true" end-of-file, at least    */
 /*   for our purposes.                                                      */
 /* - Set the file pointer to the end-of-file - 4 bytes.  These last 4       */
 /*   bytes contain the offset from the end-of-file back to the start of     */
 /*   the debug information.                                                 */
 /* - set the file pointer to the start of the debug information.            */
 /* - read the NBxx bytes at the start of the debug information and          */
 /*   verify that it matches the NBxx bytes that we read at the end of       */
 /*   the file.                                                              */
 /* - define the lfo of the debug offset.                                    */
 /****************************************************************************/
 if( (seekdos(FileHandle, EndOfFile-4L, SEEK_SET) ) ||
     (readdos(&BackOff, 4 , 1 , FileHandle)       )
   )
  return(1);

 if(  (seekdos(FileHandle,EndOfFile-BackOff,SEEK_SET) ) ||
      (readdos(NByy, 4, 1, FileHandle )               )
   )
  return(1);

 if( strncmp(NByy, DebugSignature, 4) )
   return(1);

 *pDebugOff = EndOfFile - BackOff;

 return( 0 );
}                                       /* end FindDebugStart()              */

/*****************************************************************************/
/* FindOS2hdr()                                                              */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   find the long offset into the debug file of the OS/2 header.            */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   pdf        -> to the debug file node.                                   */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   offset     long offset of header begin.                                 */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   pdf is valid and the file is open.                                      */
/*                                                                           */
/*****************************************************************************/
ULONG FindOS2hdr(DEBFILE *pdf)
{
 char     buffer[4];

 seekf(pdf, LFO_OF_EXE_HEADER_LFO);
 if(readf(buffer, 4, pdf) == 0 )
  return(*( ULONG *)buffer);
 return(0L);
}

/*****************************************************************************/
/* InitModules()                                                             */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*  Build the module list from the directory block of information.           */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*  pDirectoryBuffer -> to the directory block read from file.               */
/*  NumEntries          contains the number of entries in the directory      */
/*                      block.                                               */
/*  pdf              -> to debug file structure.                             */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*  - pdf != NULL.                                                           */
/*  - NumEntries > 0.                                                        */
/*                                                                           */
/*****************************************************************************/
void  InitModules( char *pDirectoryBuffer, int NumEntries, DEBFILE *pdf)
{
 int          i;
 MODULE      *pModule;
 MODULE      *pNewModule;
 HDRENTRYHLL *pDirectoryEntryHLL;
 HDRENTRY    *pDirectoryEntry;

 /****************************************************************************/
 /* - Allocate the first module and connect it to the list.                  */
 /****************************************************************************/
 pModule        = (MODULE *)Talloc( sizeof( MODULE ) );
 pdf->MidAnchor = pModule;
 pModule->pdf   = pdf;

 if( pdf->ExeFlags.NBxxType == NB04 )
 {
  /***************************************************************************/
  /* - Allocate/initialize the modules for HLL format.                       */
  /***************************************************************************/
  pDirectoryEntryHLL = (HDRENTRYHLL *)pDirectoryBuffer;
  pModule->mid       = pDirectoryEntryHLL->ModIndex;

  pDirectoryEntryHLL++;

  for( i = 2; i <= NumEntries; i++, pDirectoryEntryHLL++ )
  {
   if( (pDirectoryEntryHLL->ModIndex != pModule->mid) &&
       (pDirectoryEntryHLL->ModIndex != 0 )
     )
   {
    pNewModule       = (MODULE *)Talloc( sizeof( MODULE ) );
    pNewModule->mid  = pDirectoryEntryHLL->ModIndex;
    pModule->NextMod = pNewModule;
    pModule          = pNewModule;
   }
  }
 }
 else /* pdf->ExeFlags.NBxxType == NB00 or NB02 */
 {
  /***************************************************************************/
  /* - Allocate/init the modules for MS formats( 32 and 16 bit ).            */
  /***************************************************************************/
  pDirectoryEntry = (HDRENTRY *)pDirectoryBuffer;
  pModule->mid    = pDirectoryEntry->ModIndex;

  pDirectoryEntry++;

  for( i = 2; i <= NumEntries; i++, pDirectoryEntry++ )
  {
   if( (pDirectoryEntry->ModIndex != pModule->mid) &&
       (pDirectoryEntry->ModIndex != 0 )
     )
   {
    pNewModule       = (MODULE *)Talloc( sizeof( MODULE ) );
    pNewModule->mid  = pDirectoryEntry->ModIndex;
    pModule->NextMod = pNewModule;
    pModule          = pNewModule;
   }
  }
 }

 if( pdf->ExeFlags.NBxxType == NB04 )
 {
  /***************************************************************************/
  /* - Finish initializing the modules for HLL format.                       */
  /***************************************************************************/
  pDirectoryEntryHLL = (HDRENTRYHLL *)pDirectoryBuffer;
  for( i = 1; i <= NumEntries; i++, pDirectoryEntryHLL++ )
  {
   /***************************************************************************/
   /* - We don't build modules for modindexes of 0.                           */
   /***************************************************************************/
   if( pDirectoryEntryHLL->ModIndex == 0 )
    continue;

   /***************************************************************************/
   /* - scan to module allocated for this directory entry.                    */
   /***************************************************************************/
   for( pModule = pdf->MidAnchor;/* no test */; pModule = pModule->NextMod )
   {
    if( pModule->mid == pDirectoryEntryHLL->ModIndex)
     break;
   }

   /***************************************************************************/
   /* - now add additional information to the module based on the subsection  */
   /*   type.                                                                 */
   /***************************************************************************/
   switch( pDirectoryEntryHLL->SubSectType  )
   {
     case SSTMODULES:
      pModule->FileName    = pDirectoryEntryHLL->SubSectOff;
      pModule->FileNameLen = pDirectoryEntryHLL->SubSectLen;
      break;

     case SSTPUBLICS:
      pModule->Publics = pDirectoryEntryHLL->SubSectOff;
      pModule->PubLen  = pDirectoryEntryHLL->SubSectLen;
      break;

     case SSTTYPES:
      pModule->TypeDefs = pDirectoryEntryHLL->SubSectOff;
      pModule->TypeLen  = pDirectoryEntryHLL->SubSectLen;
      break;

     case SSTSYMBOLS:
      pModule->Symbols = pDirectoryEntryHLL->SubSectOff;
      pModule->SymLen  = pDirectoryEntryHLL->SubSectLen;
      break;

     case SSTSRCLINES:
      pModule->LineNums    = pDirectoryEntryHLL->SubSectOff;
      pModule->LineNumsLen = pDirectoryEntryHLL->SubSectLen;
      break;

     case SSTSRCLNSEG:
      pModule->LineNums    = pDirectoryEntryHLL->SubSectOff;
      pModule->LineNumsLen = pDirectoryEntryHLL->SubSectLen;
      break;

     case SSTIBMSRC:
      pModule->LineNums    = pDirectoryEntryHLL->SubSectOff;
      pModule->LineNumsLen = pDirectoryEntryHLL->SubSectLen;
      break;

     case SSTLIBRARIES:
      break;
   }
  }
 }

 if( (pdf->ExeFlags.NBxxType == NB00) || (pdf->ExeFlags.NBxxType == NB02) )
 {
  /***************************************************************************/
  /* - Finish initializing the modules for MS formats( 32 and 16 bit).       */
  /***************************************************************************/
  pDirectoryEntry = (HDRENTRY *)pDirectoryBuffer;
  for( i = 1; i <= NumEntries; i++, pDirectoryEntry++ )
  {
   /***************************************************************************/
   /* - We don't build modules for modindexes of 0.                           */
   /***************************************************************************/
   if( pDirectoryEntry->ModIndex == 0 )
    continue;

   /***************************************************************************/
   /* - scan to module allocated for this directory entry.                    */
   /***************************************************************************/
   for( pModule = pdf->MidAnchor;/* no test */; pModule = pModule->NextMod )
   {
    if( pModule->mid == pDirectoryEntry->ModIndex)
     break;
   }

   /***************************************************************************/
   /* - now add additional information to the module based on the subsection  */
   /*   type.                                                                 */
   /***************************************************************************/
   switch( pDirectoryEntry->SubSectType  )
   {
     case SSTMODULES:
      pModule->FileName    = pDirectoryEntry->SubSectOff;
      pModule->FileNameLen = pDirectoryEntry->SubSectLen;
      break;

     case SSTPUBLICS:
      pModule->Publics = pDirectoryEntry->SubSectOff;
      pModule->PubLen  = pDirectoryEntry->SubSectLen;
      break;

     case SSTTYPES:
      pModule->TypeDefs = pDirectoryEntry->SubSectOff;
      pModule->TypeLen  = pDirectoryEntry->SubSectLen;
      break;

     case SSTSYMBOLS:
      pModule->Symbols = pDirectoryEntry->SubSectOff;
      pModule->SymLen  = pDirectoryEntry->SubSectLen;
      break;

     case SSTSRCLINES:
      pModule->LineNums    = pDirectoryEntry->SubSectOff;
      pModule->LineNumsLen = pDirectoryEntry->SubSectLen;
      break;

     case SSTSRCLNSEG:
      pModule->LineNums    = pDirectoryEntry->SubSectOff;
      pModule->LineNumsLen = pDirectoryEntry->SubSectLen;
      break;

     case SSTIBMSRC:
      pModule->LineNums    = pDirectoryEntry->SubSectOff;
      pModule->LineNumsLen = pDirectoryEntry->SubSectLen;
      break;

     case SSTLIBRARIES:
      break;
   }
  }
 }
}

/*****************************************************************************/
/* InitModuleAddrs()                                                         */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*  Read the filename entry information for each module in the module list   */
/*  and convert the memory object numbers for the modules to flat addresses. */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*  pdf      -> to debug file structure.                                     */
/*                                                                           */
/* Return:                                                                   */
/*            0  -  Success.                                                 */
/*            1  -  Failure.                                                 */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*  - pdf is valid.                                                          */
/*                                                                           */
/*****************************************************************************/
APIRET InitModuleAddrs( DEBFILE *pdf )
{
 MODULE       *pModule;
 FREC_NE      *pFrec_NE;
 FREC_NB00_LX *pFrec_NB00_LX;
 FREC_NB00_LX  Frec_NB00_LX;
 FREC_NB04    *pFrec_NB04;
 ULONG         LoadAddr;
 ULONG         CsectLo;
 ULONG         CsectHi;

 /****************************************************************************/
 /* - For each module get the filename information from the buffer. Add      */
 /*   info about the seg#:offset and length of the module.                   */
 /* - Convert the object numbers to flat addressess.                         */
 /****************************************************************************/
 for( pModule = pdf->MidAnchor; pModule; pModule = pModule->NextMod )
 {
  seekf(pdf,pdf->DebugOff + pModule->FileName);

  /***************************************************************************/
  /* Process only the modules which have a ParentMid of zero.  This was      */
  /* added because when we encounter a module with more than one segment we  */
  /* add a new module at the end of the module linked list with all info     */
  /* same as Parent module except the midbase parameters.  This new "child"  */
  /* module will have Parentmid.                                             */
  /***************************************************************************/
  if( (pdf->ExeFlags.NBxxType == NB02) ||
      ( (pdf->ExeFlags.NBxxType == NB00) && (pdf->ExeFlags.ExeType == NE) )
    )
  {
   int reclen;

   /*************************************************************************/
   /* - Case of Code View 16bit format.                                     */
   /* - Multiple segments supported in this format.                         */
   /*************************************************************************/
   reclen   = pModule->FileNameLen;
   pFrec_NE = (FREC_NE*)Talloc(reclen);
   readf( (UCHAR*)pFrec_NE, reclen, pdf );

   pModule->pCsects          = (CSECT*)Talloc(sizeof(CSECT));
   pModule->pCsects->next    = NULL;
   pModule->pCsects->pModule = pModule;
   pModule->pCsects->SegNum  = pFrec_NE->SegObject.SegNum;

   LoadAddr = GetLoadAddr(pdf->mte, pModule->pCsects->SegNum);
   CsectLo  = CsectHi = 0;
   if( LoadAddr != 0 )
   {
    CsectLo  = LoadAddr + pFrec_NE->SegObject.ImOff;
    CsectHi  = CsectLo  + pFrec_NE->SegObject.ImLen - 1;
   }

   pModule->pCsects->SegFlatAddr = LoadAddr;
   pModule->pCsects->CsectLo     = CsectLo;
   pModule->pCsects->CsectHi     = CsectHi;
   pModule->pCsects->CsectSize   = pFrec_NE->SegObject.ImLen;

   if( pFrec_NE->NoSegs > 1 )
   {
    /************************************************************************/
    /* - if more than one segment for this compile unit, then add the       */
    /*   additional segments.                                               */
    /************************************************************************/
    FREC_NE_SEGOBJECT *pFrecCsect;

    CSECT *pCsect;
    int    NumCsects;
    UCHAR *pFrec;

    /************************************************************************/
    /* - get a count of the number of additional csects.                    */
    /* - build a ptr to the next csect record in the file.                  */
    /************************************************************************/
    NumCsects  = pFrec_NE->NoSegs - 1;
    pFrec      = (UCHAR *)pFrec_NE + sizeof(FREC_NE)+ pFrec_NE->fnlength;
    pFrecCsect = (FREC_NE_SEGOBJECT*)(pFrec);

    /************************************************************************/
    /* - now, build the additional csects.                                  */
    /************************************************************************/
    pCsect = pModule->pCsects;
    while( NumCsects > 0 )
    {
     pCsect->next         = (CSECT*)Talloc(sizeof(CSECT));
     pCsect->pModule      = pModule;
     pCsect->next->SegNum = pFrecCsect->SegNum;

     LoadAddr = GetLoadAddr(pdf->mte, pFrecCsect->SegNum);
     CsectLo  = CsectHi = 0;
     if( LoadAddr != 0 )
     {
      CsectLo  = LoadAddr + pFrecCsect->ImOff;
      CsectHi  = CsectLo  + pFrecCsect->ImLen - 1;
     }

     pCsect->next->SegFlatAddr = LoadAddr;
     pCsect->next->CsectLo     = CsectLo;
     pCsect->next->CsectHi     = CsectHi;
     pCsect->next->CsectSize   = pFrecCsect->ImLen;

     pFrecCsect++;
     pCsect = pCsect->next;
     NumCsects--;
    }
   }

   /*************************************************************************/
   /* - free the filename record.                                           */
   /*************************************************************************/
   Tfree(pFrec_NE);

  }
  else if( (pdf->ExeFlags.NBxxType == NB00) && (pdf->ExeFlags.ExeType == LX) )
  {
   /*************************************************************************/
   /* - Case of Code View 32bit format.                                     */
   /* - Note that only one memory object per compile unit is supported      */
   /*   for this format.                                                    */
   /*************************************************************************/
   pFrec_NB00_LX = &Frec_NB00_LX;
   readf( (UCHAR*)pFrec_NB00_LX, sizeof(FREC_NB00_LX), pdf );

   pModule->pCsects          = (CSECT*)Talloc(sizeof(CSECT));
   pModule->pCsects->next    = NULL;
   pModule->pCsects->pModule = pModule;
   pModule->pCsects->SegNum  = pFrec_NB00_LX->SegNum;

   LoadAddr = GetLoadAddr(pdf->mte, pModule->pCsects->SegNum);
   CsectLo  = CsectHi = 0;
   if( LoadAddr != 0 )
   {
    CsectLo  = LoadAddr + pFrec_NB00_LX->ImOff;
    CsectHi  = CsectLo  + pFrec_NB00_LX->ImLen - 1;
   }

   pModule->pCsects->SegFlatAddr = LoadAddr;
   pModule->pCsects->CsectLo     = CsectLo;
   pModule->pCsects->CsectHi     = CsectHi;
   pModule->pCsects->CsectSize   = pFrec_NB00_LX->ImLen;
  }
  else if( pdf->ExeFlags.NBxxType == NB04 )
  {
   int reclen;

   /*************************************************************************/
   /* - Case of IBM HLL 32bit format.                                       */
   /* - Multiple segments supported in this format.                         */
   /*************************************************************************/
   reclen     = pModule->FileNameLen;
   pFrec_NB04 = (FREC_NB04*)Talloc(reclen);
   readf( (UCHAR*)pFrec_NB04, reclen, pdf );

   /*************************************************************************/
   /* - put in the information for the first segment.                       */
   /*************************************************************************/
   pModule->pCsects          = (CSECT*)Talloc(sizeof(CSECT));
   pModule->pCsects->next    = NULL;
   pModule->pCsects->pModule = pModule;
   pModule->pCsects->SegNum  = pFrec_NB04->SegObject.SegNum;

   LoadAddr = GetLoadAddr(pdf->mte, pModule->pCsects->SegNum);
   CsectLo  = CsectHi = 0;
   if( LoadAddr != 0 )
   {
    CsectLo = LoadAddr + pFrec_NB04->SegObject.ImOff;
    CsectHi = CsectLo  + pFrec_NB04->SegObject.ImLen - 1;
   }

   pModule->pCsects->SegFlatAddr = LoadAddr;
   pModule->pCsects->CsectLo     = CsectLo;
   pModule->pCsects->CsectHi     = CsectHi;
   pModule->pCsects->CsectSize   = pFrec_NB04->SegObject.ImLen;

   if( pFrec_NB04->NoSegs > 1 )
   {
    /************************************************************************/
    /* - if more than one segment for this compile unit, then add the       */
    /*   additional segments.                                               */
    /************************************************************************/
    FREC_NB04_SEGOBJECT *pFrecCsect;

    CSECT *pCsect;
    int    NumCsects;
    UCHAR *pFrec;

    /************************************************************************/
    /* - get a count of the number of additional csects.                    */
    /* - build a ptr to the next csect record in the file.                  */
    /************************************************************************/
    NumCsects  = pFrec_NB04->NoSegs - 1;
    pFrec      = (UCHAR *)pFrec_NB04 + sizeof(FREC_NB04)+ pFrec_NB04->fnlength;
    pFrecCsect = (FREC_NB04_SEGOBJECT*)(pFrec);

    /************************************************************************/
    /* - now, build the additional csects.                                  */
    /************************************************************************/
    pCsect = pModule->pCsects;
    while( NumCsects > 0 )
    {
     pCsect->next         = (CSECT*)Talloc(sizeof(CSECT));
     pCsect->pModule      = pModule;
     pCsect->next->SegNum = pFrecCsect->SegNum;

     LoadAddr = GetLoadAddr(pdf->mte, pFrecCsect->SegNum);
     CsectLo  = CsectHi = 0;
     if( LoadAddr != 0 )
     {
      CsectLo  = LoadAddr + pFrecCsect->ImOff;
      CsectHi  = CsectLo  + pFrecCsect->ImLen - 1;
     }

     pCsect->next->SegFlatAddr = LoadAddr;
     pCsect->next->CsectLo     = CsectLo;
     pCsect->next->CsectHi     = CsectHi;
     pCsect->next->CsectSize   = pFrecCsect->ImLen;

     pFrecCsect++;
     pCsect = pCsect->next;
     NumCsects--;
    }
   }
   /*************************************************************************/
   /* - free the filename record.                                           */
   /*************************************************************************/
   Tfree(pFrec_NB04);
  }
 }
 return( 0 );
}

/*****************************************************************************/
/* SetDebugFormatFlags()                                                     */
/*                                                                           */
/* Description:                                                              */
/*  Scan through the Directory buffer and File Name block buffer and set the */
/*  various debug format flags for each module.                              */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*  DirectoryBuffer - contains the directory block of information.           */
/*  NumEntries      - contains the number of entries in the                  */
/*                    directory block.                                       */
/*  pdf             - pointer to debug file structure.                       */
/*                                                                           */
/* Return:                                                                   */
/*            None.                                                          */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*  pdf is valid.                                                            */
/*                                                                           */
/*****************************************************************************/
void  SetDebugFormatFlags( char    *pDirectoryBuffer,
                           int      NumEntries,
                           DEBFILE *pdf )
{
 int          i = 0;
 MODULE      *pModule     = NULL;
 FREC_NB04    *pFrec_NB04 = NULL;
 FREC_NB04     Frec_NB04;
 HDRENTRYHLL *pDirectoryEntryHLL = NULL;
 HDRENTRY    *pDirectoryEntry    = NULL;
 int          ModuleType = 0;

 memset( &Frec_NB04, 0, sizeof(Frec_NB04));
 /****************************************************************************/
 /* - Define the flags for the publics.                                      */
 /*                                                                          */
 /*      NB00,NB02             NB04                                          */
 /*      ---------             ----                                          */
 /*          |                  |                                            */
 /*          |                  |                                            */
 /*          |                  |                                            */
 /*          |                  |                                            */
 /*    ------------             |                                            */
 /*   |            |            |                                            */
 /*   |            |            |                                            */
 /*   |            |            |                                            */
 /*   |NE          |LX          |                                            */
 /*   |            |            |                                            */
 /*   |            |            |                                            */
 /*   |            |            |                                            */
 /*   |            |            |                                            */
 /*   TYPE_PUB_16  TYPE_PUB_32  TYPE_PUB_32                                  */
 /*                                                                          */
 /*==========================================================================*/
 /*                                                                          */
 /* - Define the flags for the line numbers.                                 */
 /*                                                                          */
 /*   NB00        NB00,NB02                            NB04                  */
 /*   ----        ---------                            ----                  */
 /*    |              |                                 |                    */
 /*    |              |                                 |                    */
 /*   105            109                               10B ( always LX )     */
 /*    |              |                                 |                    */
 /*    |        ------------                    -----------------            */
 /*    |       |            |                  |HL01 |HL02 |HL03 |HL04       */
 /*    |       |            |                  |     |     |     |           */
 /*    |       |            |                  |     |     |     TYPE10B_HL04*/
 /*    |       |NE          |LX                |     |     |                 */
 /*    |       |            |                  |     |     TYPE10B_HL03      */
 /*    |       |            |                  |     |                       */
 /*    |       |            |                  |     TYPE10B_HL02            */
 /*    |       |            |                  |                             */
 /* TYPE105    TYPE109_16   TYPE109_32         TYPE10B_HL01                  */
 /* (16 bit)                                                                 */
 /*                                                                          */
 /*  NOTES:                                                                  */
 /*                                                                          */
 /*    - IBM C/2 1.1 generates type 105 line numbers.                        */
 /*      LINK386 will map these to type 109.                                 */
 /*    - MSC 6.0 generates type 109 line numbers.                            */
 /*    - CL386 also generates type 109 line numbers.                         */
 /*    - IBM C-Set/2 etc generates type 10B, but you have to                 */
 /*      look in the file name records to find the true format, i.e.,        */
 /*      HL01,HL02,..etc.                                                    */
 /*==========================================================================*/
 /*                                                                          */
 /* - Define the symbols and types flags.                                    */
 /*                                                                          */
 /*            NB00,NB02                                                     */
 /*            ---------                                                     */
 /*                |                                                         */
 /*                |                                                         */
 /*               104,3                                                      */
 /*                |                                                         */
 /*          ---------------------                                           */
 /*         |                     |                                          */
 /*         |                     |                                          */
 /*         |                     |                                          */
 /*         |NE                   |LX                                        */
 /*         |                     |                                          */
 /*         |                     |                                          */
 /*         |                     |                                          */
 /*         |                     |                                          */
 /*   ------------                |                                          */
 /*  |            |               |                                          */
 /*  |NB00        |NB02           |                                          */
 /*  |            |               |                                          */
 /*  TYPE104_C211 TYPE104_C600    TYPE104_CL386                              */
 /*        3            3         |     3                                    */
 /*                               |                                          */
 /*                           Module 16,32                                   */
 /*                      -------------------                                 */
 /*                     |                   |                                */
 /*                     |16(see note)       |                                */
 /*                     |                   |                                */
 /*                     TYPE104_C600        TYPE104_CL386                    */
 /*                           3                   3                          */
 /*                                                                          */
 /*                                                                          */
 /*          NB04                                                            */
 /*          ----                                                            */
 /*           |                                                              */
 /*           |                                                              */
 /*          104,3 ( always LX )                                             */
 /*           |                                                              */
 /*   ----------------------------------------                               */
 /*  |HL01 |HL02 |HL03 |HL04                  |OTHER                         */
 /*  |     |     |     |                      |                              */
 /*  |     |     |     TYPE104_HL04           |                              */
 /*  |     |     |           3                |                              */
 /*  |     |     |                        Module 16,32                       */
 /*  |     |     TYPE104_HL03        -------------------                     */
 /*  |     |           3            |                   |                    */
 /*  |     |                        |16(see note)       |                    */
 /*  |     TYPE104_HL02             |                   |                    */
 /*  |           3                  TYPE104_C600        TYPE104_CL386        */
 /*  |                                    3                   3              */
 /*  TYPE104_HL01                                       ( could happen       */
 /*        3                                              when linking       */
 /*                                                       assembler )        */
 /*                                                                          */
 /*                                                                          */
 /*   - An LX 32 bit executable can contain 16 bit symbols and types         */
 /*     subsectiions. Before we can assign a flag, we have to look at the    */
 /*     bitness of the load address defined in the filename record for       */
 /*     this subsection. If the bitness is 16 bit, then we use a             */
 /*     TYPE104_C600(TYPE103_C600) flag.                                     */
 /*                                                                          */
 /****************************************************************************/
 if( (pdf->ExeFlags.NBxxType == NB00) || (pdf->ExeFlags.NBxxType == NB02) )
 {
  pDirectoryEntry = (HDRENTRY *)pDirectoryBuffer;
  for( i = 1; i <= NumEntries; i++, pDirectoryEntry++ )
  {
   /***************************************************************************/
   /* - We don't build modules for modindexes of 0.                           */
   /***************************************************************************/
   if( pDirectoryEntry->ModIndex == 0 )
    continue;

   /**************************************************************************/
   /* - scan to module allocated for this directory entry.                   */
   /**************************************************************************/
   for( pModule = pdf->MidAnchor;/* no test */; pModule = pModule->NextMod )
   {
    if( pModule->mid == pDirectoryEntry->ModIndex)
     break;
   }

   switch( pDirectoryEntry->SubSectType  )
   {
     case SSTMODULES:
     case SSTLIBRARIES:
      break;

     case SSTPUBLICS:
      if( pdf->ExeFlags.ExeType == NE )
       pModule->DbgFormatFlags.Pubs = TYPE_PUB_16;
      else /* if( pdf->ExeFlags.ExeType == LX ) */
       pModule->DbgFormatFlags.Pubs = TYPE_PUB_32;
      break;

     case SSTSYMBOLS:
      if( pdf->ExeFlags.ExeType == NE )
      {
       if( pdf->ExeFlags.NBxxType == NB00 )
        pModule->DbgFormatFlags.Typs = TYPE104_C211;
       else /* if( pdf->ExeFlags.NBxxType == NB00 ) */
        pModule->DbgFormatFlags.Typs = TYPE104_C600;
      }
      else /* if( pdf->ExeFlags.ExeType == LX ) */
      {
       pModule->DbgFormatFlags.Typs = TYPE104_CL386;
       /**********************************************************************/
       /* We could be having 16 bit modules linked with 32 bit exe, so check */
       /* for the bitness of the address and over ride the earlier settings. */
       /**********************************************************************/
       ModuleType = GetBitness( pModule->pCsects->CsectLo );
       if( ModuleType == BIT16 )
        pModule->DbgFormatFlags.Typs = TYPE104_C600;
      }
      break;

     case SSTTYPES:
      if( pdf->ExeFlags.ExeType == NE )
      {
       if( pdf->ExeFlags.NBxxType == NB00 )
        pModule->DbgFormatFlags.Syms = TYPE103_C211;
       else /* if( pdf->ExeFlags.NBxxType == NB00 ) */
        pModule->DbgFormatFlags.Syms = TYPE103_C600;
      }
      else /* if( pdf->ExeFlags.ExeType == LX ) */
      {
       pModule->DbgFormatFlags.Syms = TYPE103_CL386;
       /**********************************************************************/
       /* We could be having 16 bit modules linked with 32 bit exe, so check */
       /* for the bitness of the address and over ride the earlier settings. */
       /**********************************************************************/
       ModuleType = GetBitness( pModule->pCsects->CsectLo );
       if( ModuleType == BIT16 )
        pModule->DbgFormatFlags.Syms = TYPE103_C600;
      }
      break;

     case SSTSRCLINES:
      pModule->DbgFormatFlags.Lins = TYPE105;
      break;

     case SSTSRCLNSEG:
      if( pdf->ExeFlags.ExeType == NE )
       pModule->DbgFormatFlags.Lins = TYPE109_16;
      else /* if( pdf->ExeFlags.ExeType == LX ) */
       pModule->DbgFormatFlags.Lins = TYPE109_32;
      break;
   }
  }
 }
 else /*if( pdf->ExeFlags.NBxxType == NB04 )*/
 {
  pDirectoryEntryHLL = (HDRENTRYHLL *)pDirectoryBuffer;
  for( i = 1; i <= NumEntries; i++, pDirectoryEntryHLL++ )
  {
   /***************************************************************************/
   /* - We don't build modules for modindexes of 0.                           */
   /***************************************************************************/
   if( pDirectoryEntryHLL->ModIndex == 0 )
    continue;

   /***************************************************************************/
   /* - scan to module allocated for this directory entry.                    */
   /***************************************************************************/
   for( pModule = pdf->MidAnchor;/* no test */; pModule = pModule->NextMod )
   {
    if( pModule->mid == pDirectoryEntryHLL->ModIndex)
     break;
   }

   switch( pDirectoryEntryHLL->SubSectType  )
   {
    case SSTMODULES:
    case SSTLIBRARIES:
     break;

    case SSTPUBLICS:
     pModule->DbgFormatFlags.Pubs = TYPE_PUB_32;
     break;

    case SSTTYPES:
     /*********************************************************************/
     /* If the debug format is IBM, it can be either Level 1,2,3, or 4.   */
     /* Read the debug style info from the file block buffer and set the  */
     /* debug format flags accordingly.                                   */
     /*********************************************************************/
     seekf(pdf,pdf->DebugOff + pModule->FileName);
     pFrec_NB04 = &Frec_NB04;
     readf( (UCHAR*)pFrec_NB04,sizeof(FREC_NB04), pdf );

     if( pFrec_NB04->DebugStyle == HLL )
     {
      switch( pFrec_NB04->version[1] )
      {
       case 0x1:
        pModule->DbgFormatFlags.Typs = TYPE103_HL01;
        break;

       case 0x2:
        pModule->DbgFormatFlags.Typs = TYPE103_HL02;
        break;

       case 0x3:
        pModule->DbgFormatFlags.Typs = TYPE103_HL03;
        break;

       case 0x4:
        pModule->DbgFormatFlags.Typs = TYPE103_HL04;
        break;
      }
     }
     else /* If Debug style is not HLL */
     {
      pModule->DbgFormatFlags.Typs = TYPE103_CL386;
      ModuleType = GetBitness( pModule->pCsects->CsectLo );
      if( ModuleType == BIT16 )
       pModule->DbgFormatFlags.Typs = TYPE103_C600;
     }
     break;

    case SSTSYMBOLS:
     /*********************************************************************/
     /* If the debug format is IBM, it can be either Level 1,2,3, or 4.   */
     /* Read the debug style info from the file block buffer and set the  */
     /* debug format flags accordingly.                                   */
     /*********************************************************************/
     seekf(pdf,pdf->DebugOff + pModule->FileName);
     pFrec_NB04 = &Frec_NB04;
     readf( (UCHAR*)pFrec_NB04,sizeof(FREC_NB04), pdf );

     if( pFrec_NB04->DebugStyle == HLL )
     {
      switch( pFrec_NB04->version[1] )
      {
       case 0x1:
        pModule->DbgFormatFlags.Syms = TYPE104_HL01;
        break;

       case 0x2:
        pModule->DbgFormatFlags.Syms = TYPE104_HL02;
        break;

       case 0x3:
        pModule->DbgFormatFlags.Syms = TYPE104_HL03;
        break;

       case 0x4:
        pModule->DbgFormatFlags.Syms = TYPE104_HL04;
        break;
      }
     }
     else /* If Debug style is not HLL */
     {
      pModule->DbgFormatFlags.Syms = TYPE104_CL386;
      ModuleType = GetBitness( pModule->pCsects->CsectLo );
      if( ModuleType == BIT16 )
       pModule->DbgFormatFlags.Syms = TYPE104_C600;
     }
     break;

    case SSTSRCLINES:
     /* should never happen, but just in case. */
     pModule->DbgFormatFlags.Lins = TYPE105;
     break;

    case SSTSRCLNSEG:
     pModule->DbgFormatFlags.Lins = TYPE109_32;
     break;

    case SSTIBMSRC:
     /************************************************************************/
     /* - Now get some additional info from the the file name subsection     */
     /*   so we can decide HL01,HL02,HL03,HL04 styles.                       */
     /************************************************************************/
     seekf(pdf,pdf->DebugOff + pModule->FileName);
     pFrec_NB04 = &Frec_NB04;
     readf( (UCHAR*)pFrec_NB04,sizeof(FREC_NB04), pdf );
     switch( pFrec_NB04->version[1] )
     {
      case 0x1:
       pModule->DbgFormatFlags.Lins = TYPE10B_HL01;
       break;

      case 0x2:
       pModule->DbgFormatFlags.Lins = TYPE10B_HL02;
       break;

      case 0x3:
       pModule->DbgFormatFlags.Lins = TYPE10B_HL03;
       break;

      case 0x4:
       pModule->DbgFormatFlags.Lins = TYPE10B_HL04;
       break;
     }
     break;
   }
  }
 }
}

