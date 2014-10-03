/*****************************************************************************/
/* File:                                                                     */
/*                                                                           */
/*   linnum.c                                                                */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Line number table interface functions.                                  */
/*                                                                           */
/* History:                                                                  */
/*                                                                           */
/*   04/10/95 Updated.                                                       */
/*                                                                           */
/*****************************************************************************/

#include "all.h"

/*****************************************************************************/
/* DBGetLnoTab                                                               */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Get pointer to a module's lineno table.                                 */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   mid        module(compile unit) we want the table for.                  */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
void DBGetLnoTab( ULONG mid )
{
 UCHAR    LnoFlag;
 MODULE  *pModule;
 DEBFILE *pdf;

 pdf     = DBFindPdf( mid );
 pModule = GetPtrToModule( mid, pdf );
 LnoFlag = pModule->DbgFormatFlags.Lins;
 switch( LnoFlag )
 {
  case TYPE10B_HL01:
   BuildHL01Tables( pModule, pdf );
   break;

  case TYPE10B_HL02: /* not supported any more/never released*/
  case TYPE10B_HL03:
   BuildHL03Tables( pModule, pdf );
   break;

  case TYPE10B_HL04:
   BuildHL04Tables( pModule, pdf );
   break;

  case TYPE105:
   Build105Tables( pModule, pdf );
   break;

  case TYPE109_32:
   Build109_32Tables( pModule, pdf );
   break;

  case TYPE109_16:
   Build109_16Tables( pModule, pdf );
   break;
 }
}

/*****************************************************************************/
/* DBMapInstAddr                                                             */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   get mid and line number table entry for the instruction address.        */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   address       instruction address.                                      */
/*   ppLnoTabEntry receiver of a ptr to the line number table entry          */
/*                 containing this address.                                  */
/*   pdf                                                                     */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   mid        module id containing the address.                            */
/*              0 ==> not found.                                             */
/*                                                                           */
/*****************************************************************************/
ULONG DBMapInstAddr( ULONG address, LNOTAB **ppLnoTabEntry ,DEBFILE *pdf )
{
 MODULE *pModule;
 CSECT  *pCsect;
 ULONG   mid;

 pModule = GetModuleWithAddr( address, pdf );

 if( (pModule != NULL) &&
     (pModule->LineNums != 0) &&
     (pModule->modflags.LnoTabLoaded == FALSE) )
 {
  DBGetLnoTab( pModule->mid );
  pModule->modflags.LnoTabLoaded = TRUE;
 }

 pCsect = GetCsectWithAddr( pModule,  address );

 *ppLnoTabEntry = NULL;
 if( pCsect )
  *ppLnoTabEntry = GetLnoWithAddr( pCsect, address );

 mid = 0;
 if( pModule )
  mid = pModule->mid;
 return(mid);
}

/*****************************************************************************/
/* DBMapLno                                                                  */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   get the base and span of a line or module.                              */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   mid        - module id.                                                 */
/*   lno        - line number  for line number info.                         */
/*                0 for module info.                                         */
/*   sfi        - source file(index) containing the line number.             */
/*   span       - where to put the span of line or module.                   */
/*   pdf                                                                     */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   BaseAddr   if lno  = 0, returns Flat base address of csect and          */
/*                           size of the csect.                              */
/*              if lno != 0, returns Flat base address of line in module     */
/*                           and the span of the line.                       */
/*              no find lno, return NULL and span = 0;                       */
/*                                                                           */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
ULONG DBMapLno( ULONG mid, int lno, int sfi, ULONG *span , DEBFILE *pdf )
{
 MODULE *pModule;
 LNOTAB *pLnoTabEntry;
 ULONG   BaseAddr;

 /****************************************************************************/
 /* - load the line number table if it hasn't been loaded.                   */
 /****************************************************************************/
 pModule = GetPtrToModule( mid, pdf );
 if( (pModule == NULL)           ||
     (pModule->LineNums == 0)    ||
     (pModule->LineNumsLen == 0)
   )
 {
  *span    = 0;
  BaseAddr = 0;
 }

 /****************************************************************************/
 /* - load the line number table if not yet loaded.                          */
 /****************************************************************************/
 if( pModule->modflags.LnoTabLoaded == FALSE )
 {
  DBGetLnoTab( pModule->mid );
  pModule->modflags.LnoTabLoaded = TRUE;
 }

 if( lno != 0 )
 {
  pLnoTabEntry = GetLnoTabEntry( pModule, lno, sfi );
  if( pLnoTabEntry != NULL )
  {
   BaseAddr = pLnoTabEntry->off;
   *span    = GetLineSpan( pModule, pLnoTabEntry);
  }
  else
  {
   BaseAddr = 0;
   *span    = 0;
  }
 }
 else if( lno == 0 )
 {
  BaseAddr     = 0;
  pLnoTabEntry = GetLnoTabEntry( pModule, 0, sfi );
  if( pLnoTabEntry )
   BaseAddr = pLnoTabEntry->off;
 }
 return(BaseAddr);
}

/*****************************************************************************/
/*  DBMapNonExLine()                                                         */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Map a non-executable line number to an executable line number.          */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   mid                                                                     */
/*   lno        non-executable line number.                                  */
/*   sfi        source file containing the line number.                      */
/*   pdf                                                                     */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   BaseAddr   address of the mapped executable line.                       */
/*                                                                           */
/* Assumptions                                                               */
/*                                                                           */
/*****************************************************************************/
ULONG DBMapNonExLine( ULONG mid, int lno, int sfi, DEBFILE *pdf )
{
 MODULE *pModule;
 LNOTAB *pLnoTabEntry;
 ULONG   BaseAddr;

 pModule = GetPtrToModule( mid, pdf );

 if( pModule == NULL )
  return(0);

 pLnoTabEntry = GetLnoTabNextEntry( pModule, lno, sfi );

 BaseAddr = 0;
 if( pLnoTabEntry != NULL )
  BaseAddr = pLnoTabEntry->off;

 return(BaseAddr);
}

/*****************************************************************************/
/* GetSourceFileIndex()                                                      */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Get the sfi for a mid and filename. The filename does not include       */
/*   path info.                                                              */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   mid        module defining file table to search.                        */
/*   pFileName  -> the filename we're looking for.                           */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   sfi        index of the source file if found.                           */
/*              0 if not found.                                              */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   pFileName is a length-prefixed z-string.                                */
/*                                                                           */
/*****************************************************************************/
int GetSourceFileIndex( ULONG mid, UCHAR *pFileName )
{
 MODULE   *pModule;
 UCHAR     LnoFlag;
 int       sfi;
 FILENAME *pFile;

 pModule = GetPtrToModule( mid, NULL );
 if(pModule == NULL)
  return(0);

 /****************************************************************************/
 /* - scan the loaded file name tables first.                                */
 /****************************************************************************/
 sfi = 0;
 for( pFile = pModule->pFiles; pFile != NULL; pFile=pFile->next )
 {
  int   len;
  char *cp;
  char *cpp;

  cp  = pFile->FileName;
  cpp = strrchr( cp+1, '\\' );

  if( cpp == NULL )
  {
   /**************************************************************************/
   /* - come here if no path info.                                           */
   /**************************************************************************/
   len = *cp;
   if( (len == *pFileName) && (strnicmp(pFileName, cp, len+1) == 0) )
   {
    sfi = pFile->sfi;
    break;
   }
  }
  else
  {
   /**************************************************************************/
   /* - come here if there is path info.                                     */
   /**************************************************************************/
   cpp =  strrchr( cp+1, '\\' );
   cpp = cpp + 1;
   len = *cp -(cpp-cp) + 1;
   if( (len == *pFileName) && (strnicmp(pFileName+1, cpp, len) == 0) )
   {
    sfi = pFile->sfi;
    break;
   }
  }
 }
 if( sfi != 0 )
  return(sfi);

 /****************************************************************************/
 /* - if we haven't found it at this point, then go look in the files        */
 /*   defined in the line number tables.                                     */
 /****************************************************************************/
 LnoFlag = pModule->DbgFormatFlags.Lins;
 switch( LnoFlag )
 {
  case TYPE10B_HL01:
   sfi =  GetHL01Sfi( mid, pFileName );
   break;

  case TYPE10B_HL02:                  /* not supported/never released      */
  case TYPE10B_HL03:
   sfi =  GetHL03Sfi( mid, pFileName );
   break;

  case TYPE10B_HL04:
   sfi = GetHL04Sfi( mid, pFileName );
   break;

  case TYPE105:
   sfi = Get105Sfi( mid, pFileName );
   break;

  case TYPE109_32:
   sfi = Get109_32Sfi( mid, pFileName );
   break;

  case TYPE109_16:
   sfi = Get109_16Sfi( mid, pFileName );
   break;
 }
 return(sfi);
}

/*****************************************************************************/
/* MapSourceFileToMidSfi()                                                   */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Find the mid for a given source file name.                              */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   pFileName  -> to the source file name.                                  */
/*   psfi       -> to the receiver of the sfi.                               */
/*   pdf                                                                     */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   mid                                                                     */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
ULONG MapSourceFileToMidSfi( char *pSourceFileName, int *psfi, DEBFILE *pdf )
{
 MODULE *pModule;
 ULONG   mid;
 int     sfi;

 sfi = mid = 0;
 for( pModule = pdf->MidAnchor; pModule != NULL ; pModule=pModule->NextMod)
 {
  sfi = GetSourceFileIndex( pModule->mid, pSourceFileName );
  if( sfi )
  {
   mid = pModule->mid;
   break;
  }
 }
 *psfi = sfi;
 return(mid);
}

/*****************************************************************************/
/* BuildHL04Tables()                                                         */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Build the HL04 tables.                                                  */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   pModule                                                                 */
/*   pdf                                                                     */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*****************************************************************************/
void BuildHL04Tables(  MODULE *pModule, DEBFILE *pdf )
{
 HEADER_HL04       *pHeader;
 ULONG              lfo;
 ULONG              lfoend;
 ULONG              LineNumberTableSize;
 FIRST_ENTRY_HL04  *pFirstEntry;
 CSECTMAP          *pCsectMap = NULL;
 CSECTMAP          *pmap;
 CSECTMAP          *pnextmap;
 UCHAR             *pTablePtr;
 USHORT             NumEntries;
 CSECT             *pCsect;
 UCHAR             *pLineNumberTable;
 UCHAR             *pTable;
 UCHAR             *pTableEnd;
 UCHAR             *pFileNameTable;
 ULONG              BaseOffset;
 ULONG              MteLoadAddr;
 ULONG              CsectLo;
 ULONG              CsectHi;

 MteLoadAddr = GetLoadAddr( pdf->mte, 1 );

 /****************************************************************************/
 /* - define long file offsets for the start and end of the table.           */
 /****************************************************************************/
 lfo    = pdf->DebugOff + pModule->LineNums;
 lfoend = lfo + pModule->LineNumsLen - 1;

 /****************************************************************************/
 /* - now read the rest of the file into memory.                             */
 /****************************************************************************/
 LineNumberTableSize = lfoend - lfo + 1;
 pLineNumberTable    = Talloc( LineNumberTableSize );

 seekf(pdf, lfo);
 readf( pLineNumberTable, LineNumberTableSize, pdf);

 /****************************************************************************/
 /* - Scan the line number table and build  a map of the the table(s) to     */
 /*   csects.                                                                */
 /****************************************************************************/
 pHeader     = (HEADER_HL04 *)pLineNumberTable;
 pTable      = pLineNumberTable + sizeof(HEADER_HL04) +
               pHeader->FileNameTableSize;

 pTableEnd   = pLineNumberTable + LineNumberTableSize;
 pFirstEntry = (FIRST_ENTRY_HL04 *)pTable;
 pmap        = (CSECTMAP *)&pCsectMap;

 for( ; pTable < pTableEnd; )
 {
  NumEntries = pFirstEntry->NumEntries;
  BaseOffset = pFirstEntry->BaseOffset;

  /***************************************************************************/
  /* - find the Csect that this table section maps to.                       */
  /***************************************************************************/
  for(pCsect = pModule->pCsects; pCsect != NULL; pCsect=pCsect->next )
  {
   CsectLo = pCsect->CsectLo - MteLoadAddr + 0x10000;
   CsectHi = pCsect->CsectHi - MteLoadAddr + 0x10000;

   if( (BaseOffset >= CsectLo) &&
       (BaseOffset <= CsectHi)
     )
    break;
  }

  if( pCsect )
  {
   pmap->next              = (CSECTMAP*)Talloc(sizeof(CSECTMAP));
   pmap->next->pFirstEntry = pFirstEntry;
   pmap->next->pCsect      = pCsect;
   pmap->next->NumEntries  = NumEntries;
   pmap                    = pmap->next;
  }

  pTable = pTable + sizeof(FIRST_ENTRY_HL04) +
                     NumEntries*sizeof(LINE_NUMBER_TABLE_ENTRY_HL04);

  pFirstEntry =  (FIRST_ENTRY_HL04 *)pTable;
 }

 /****************************************************************************/
 /* - At this point, we have built a map of table sections to csects.        */
 /* - For each Csect, scan the map and count the number of entries           */
 /*   that will be in the line number table for that Csect.                  */
 /****************************************************************************/
 for(pCsect = pModule->pCsects; pCsect != NULL; pCsect=pCsect->next )
 {
  for( pmap = pCsectMap; pmap != NULL; pmap=pmap->next )
  {
   if(pmap->pCsect == pCsect)
    pCsect->NumEntries += pmap->NumEntries; /* call func to get real count */
  }
 }

/*---------------------------------------------------------------------------*/
/* - dump the map of line number tables to csects.                           */
/*****************************************************************************/
// DumpCsectMap( pModule, pCsectMap, TYPE10B_HL04 );
/*---------------------------------------------------------------------------*/

 /****************************************************************************/
 /* - Now, scan the Csects, allocate the table, and map the entries to       */
 /*   the table.                                                             */
 /****************************************************************************/
 for(pCsect = pModule->pCsects; pCsect != NULL; pCsect=pCsect->next )
 {
  pCsect->pLnoTab = Talloc(pCsect->NumEntries*sizeof(LNOTAB));
  pTablePtr = (UCHAR*)pCsect->pLnoTab;
  for( pmap = pCsectMap; pmap != NULL; pmap=pmap->next )
  {
   if(pmap->pCsect == pCsect)
   {
    pTablePtr  = AppendHL04Table( (LNOTAB *)pTablePtr, pmap->pFirstEntry,
                                  MteLoadAddr );
   }
  }
 }

 /****************************************************************************/
 /* - Now, scan the Csects and build the file names list for the source      */
 /*   file indexes that appear in the table.                                 */
 /****************************************************************************/
 pFileNameTable = pLineNumberTable + sizeof(HEADER_HL04) +
                  sizeof(FILE_NAME_TABLE_ENTRY_HL04) - 1;

 AddFileNames( pModule, pFileNameTable );

/*---------------------------------------------------------------------------*/
/* - dump the map of line number tables to csects.                           */
/*****************************************************************************/
// DumpLineNums( pModule );
/*---------------------------------------------------------------------------*/

 /****************************************************************************/
 /* - At this point, the line number table has been mapped to the Csect      */
 /*   tables.                                                                */
 /* - So, we can free the map.                                               */
 /* - And, we can free the bulk table.                                       */
 /****************************************************************************/
 for( pmap = pCsectMap ; pmap ; pmap = pnextmap )
  {pnextmap = pmap->next; Tfree(pmap);}

 if(pLineNumberTable) Tfree(pLineNumberTable);
}

/*****************************************************************************/
/* BuildHL03Tables()                                                         */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Build the HL03 tables.                                                  */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   pModule                                                                 */
/*   pdf                                                                     */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*****************************************************************************/
void BuildHL03Tables(  MODULE *pModule, DEBFILE *pdf )
{
 ULONG             lfo;
 ULONG             lfoend;
 ULONG             LineNumberTableSize;
 FIRST_ENTRY_HL03 *pFirstEntry;
 CSECTMAP         *pCsectMap = NULL;
 CSECTMAP         *pmap;
 CSECTMAP         *pnextmap;
 UCHAR            *pTablePtr;
 USHORT            NumEntries;
 CSECT            *pCsect;
 UCHAR            *pLineNumberTable;
 UCHAR            *pTable;
 UCHAR            *pTableEnd;
 UCHAR            *pFileNameTable;

 /****************************************************************************/
 /* - define long file offsets for the start and end of the table.           */
 /****************************************************************************/
 lfo    = pdf->DebugOff + pModule->LineNums;
 lfoend = lfo + pModule->LineNumsLen - 1;

 /****************************************************************************/
 /* - now read the rest of the file into memory.                             */
 /****************************************************************************/
 LineNumberTableSize = lfoend - lfo + 1;
 pLineNumberTable    = Talloc( LineNumberTableSize );

 seekf(pdf, lfo);
 readf( pLineNumberTable, LineNumberTableSize, pdf);

 /****************************************************************************/
 /* - Scan the line number table and build  a map of the the table(s) to     */
 /*   csects.                                                                */
 /****************************************************************************/
 pFirstEntry = (FIRST_ENTRY_HL03 *)pLineNumberTable;
 pTable      = pLineNumberTable;
 pTableEnd   = pLineNumberTable + LineNumberTableSize;
 pmap        = (CSECTMAP *)&pCsectMap;

 for( ; pTable < pTableEnd; )
 {
  NumEntries = pFirstEntry->NumEntries;

  /***************************************************************************/
  /* - find the Csect that this table section maps to.                       */
  /***************************************************************************/
  for(pCsect = pModule->pCsects; pCsect != NULL; pCsect=pCsect->next )
  {
   if( pCsect->SegNum == pFirstEntry->SegNum )
    break;
  }

  if( pCsect )
  {
   pmap->next              = (CSECTMAP*)Talloc(sizeof(CSECTMAP));
   pmap->next->pFirstEntry = pFirstEntry;
   pmap->next->pCsect      = pCsect;
   pmap->next->NumEntries  = NumEntries;
   pmap                    = pmap->next;
  }

  pTable = pTable + sizeof(FIRST_ENTRY_HL03) +
                    NumEntries*sizeof(LINE_NUMBER_TABLE_ENTRY_HL03) +
                    pFirstEntry->FileNameTableSize;

  pFirstEntry =  (FIRST_ENTRY_HL03 *)pTable;
 }

 /****************************************************************************/
 /* - At this point, we have built a map of table sections to csects.        */
 /* - For each Csect, scan the map and count the number of entries           */
 /*   that will be in the line number table for that Csect.                  */
 /****************************************************************************/
 for(pCsect = pModule->pCsects; pCsect != NULL; pCsect=pCsect->next )
 {
  for( pmap = pCsectMap; pmap != NULL; pmap=pmap->next )
  {
   if(pmap->pCsect == pCsect)
    pCsect->NumEntries += pmap->NumEntries; /* call func to get real count */
  }
 }

/*---------------------------------------------------------------------------*/
/* - dump the map of line number tables to csects.                           */
/*****************************************************************************/
// DumpCsectMap( pModule, pCsectMap, TYPE10B_HL03 );
/*---------------------------------------------------------------------------*/

 /****************************************************************************/
 /* - Now, scan the Csects, allocate the table, and map the entries to       */
 /*   the table.                                                             */
 /****************************************************************************/
 for(pCsect = pModule->pCsects; pCsect != NULL; pCsect=pCsect->next )
 {
  pCsect->pLnoTab = Talloc(pCsect->NumEntries*sizeof(LNOTAB));
  pTablePtr = (UCHAR*)pCsect->pLnoTab;
  for( pmap = pCsectMap; pmap != NULL; pmap=pmap->next )
  {
   if(pmap->pCsect == pCsect)
   {
    pTablePtr = AppendHL03Table((LNOTAB *)pTablePtr,
                                pmap->pFirstEntry, pCsect->CsectLo);
   }
  }
 }

 /****************************************************************************/
 /* - Now, scan the Csects and build the file names list for the source      */
 /*   file indexes that appear in the table.                                 */
 /****************************************************************************/
 pFirstEntry    = (FIRST_ENTRY_HL03 *)pLineNumberTable;
 NumEntries     = pFirstEntry->NumEntries;
 pFileNameTable = pLineNumberTable + sizeof(FIRST_ENTRY_HL03) +
                  NumEntries*sizeof(LINE_NUMBER_TABLE_ENTRY_HL03) +
                  sizeof(FILE_NAME_TABLE_ENTRY_HL03) - 1;

 AddFileNames( pModule, pFileNameTable );

/*---------------------------------------------------------------------------*/
/* - dump the map of line number tables to csects.                           */
/*****************************************************************************/
// DumpLineNums( pModule );
/*---------------------------------------------------------------------------*/

 /****************************************************************************/
 /* - At this point, the line number table has been mapped to the Csect      */
 /*   tables.                                                                */
 /* - So, we can free the map.                                               */
 /* - And, we can free the bulk table.                                       */
 /****************************************************************************/
 for( pmap = pCsectMap ; pmap ; pmap = pnextmap )
  {pnextmap = pmap->next; Tfree(pmap);}

 if(pLineNumberTable) Tfree(pLineNumberTable);
}

/*****************************************************************************/
/* BuildHL01Tables()                                                         */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Build the HL01 tables.                                                  */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   pModule                                                                 */
/*   pdf                                                                     */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*****************************************************************************/
void BuildHL01Tables(  MODULE *pModule, DEBFILE *pdf )
{
 ULONG             lfo;
 ULONG             lfoend;
 ULONG             LineNumberTableSize;
 FIRST_ENTRY_HL01 *pFirstEntry;
 UCHAR            *pTablePtr;
 USHORT            NumEntries;
 CSECT            *pCsect;

 UCHAR            *pLineNumberTable;
 UCHAR            *pFileNameTable;

 /****************************************************************************/
 /* - define long file offsets for the start and end of the table.           */
 /* - move the file pointer to the start of the table.                       */
 /* - read the first entry in the table.                                     */
 /* - define the number of entries in the table.                             */
 /* - compute the size of the table.                                         */
 /* - then, read it.                                                         */
 /****************************************************************************/
 lfo    = pdf->DebugOff + pModule->LineNums;
 lfoend = lfo + pModule->LineNumsLen - 1;

 LineNumberTableSize = lfoend - lfo + 1;
 pLineNumberTable    = Talloc( LineNumberTableSize );

 seekf(pdf, lfo);
 readf( pLineNumberTable, LineNumberTableSize, pdf);

 /***************************************************************************/
 /* - There is only one HL01 table and it is "assumed" to map to            */
 /*   the first Csect listed for the module.                                */
 /***************************************************************************/
 pFirstEntry = (FIRST_ENTRY_HL01 *)pLineNumberTable;

 pCsect      = pModule->pCsects;

 if( pCsect )
 {
  pCsect->NumEntries = pFirstEntry->NumEntries;
 }

 /****************************************************************************/
 /* - Now, allocate the table and map the entries to the table.              */
 /****************************************************************************/
 pCsect = pModule->pCsects;
 if( pCsect )
 {
  pCsect->pLnoTab = Talloc(pCsect->NumEntries*sizeof(LNOTAB));
  pTablePtr       = (UCHAR*)pCsect->pLnoTab;

  AppendHL01Table( (LNOTAB *)pTablePtr, pFirstEntry, pCsect->CsectLo );
 }

 /****************************************************************************/
 /* - Now, scan the Csects and build the file names list for the source      */
 /*   file indexes that appear in the table.                                 */
 /****************************************************************************/
 NumEntries      =  pFirstEntry->NumEntries;
 pFileNameTable  =  (char*)pLineNumberTable + sizeof(FIRST_ENTRY_HL01);
 pFileNameTable += NumEntries*sizeof(LINE_NUMBER_TABLE_ENTRY_HL01);
 pFileNameTable += sizeof(FILE_NAME_TABLE_ENTRY_HL01) - 1;

 AddFileNames( pModule, pFileNameTable );

/*---------------------------------------------------------------------------*/
/* - dump the map of line number tables to csects.                           */
/*****************************************************************************/
// DumpLineNums( pModule );
/*---------------------------------------------------------------------------*/

 /****************************************************************************/
 /* - free the bulk table.                                                   */
 /****************************************************************************/
 if(pLineNumberTable) Tfree(pLineNumberTable);
}

/*****************************************************************************/
/* Build109_16Tables()                                                       */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Build the 109_16.                                                       */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   pModule                                                                 */
/*   pdf                                                                     */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*****************************************************************************/
void Build109_16Tables(  MODULE *pModule, DEBFILE *pdf )
{
 ULONG               lfo;
 ULONG               lfoend;
 ULONG               LineNumberTableSize;
 FIRST_ENTRY_109_16 *pFirstEntry;
 CSECTMAP           *pCsectMap = NULL;
 CSECTMAP           *pmap;
 CSECTMAP           *pnextmap;
 UCHAR              *pTablePtr;
 USHORT              NumEntries;
 CSECT              *pCsect;
 UCHAR              *pLineNumberTable;
 UCHAR              *pTable;
 UCHAR              *pTableEnd;
 int                 FileNameLen;
 int                 sfi;
 UCHAR              *pFileName;

 /****************************************************************************/
 /* - define long file offsets for the start and end of the table.           */
 /* - read the line number table into memory.                                */
 /****************************************************************************/
 lfo    = pdf->DebugOff + pModule->LineNums;
 lfoend = lfo + pModule->LineNumsLen - 1;

 LineNumberTableSize = lfoend - lfo + 1;
 pLineNumberTable    = Talloc( LineNumberTableSize );

 seekf(pdf, lfo);
 readf( pLineNumberTable, LineNumberTableSize, pdf);

 /****************************************************************************/
 /* - Scan the line number table and build  a map of the the table(s) to     */
 /*   csects.                                                                */
 /****************************************************************************/
 pTable       = pLineNumberTable;
 pTableEnd    = pLineNumberTable + LineNumberTableSize;
 pmap         = (CSECTMAP *)&pCsectMap;

 /****************************************************************************/
 /* - Move past the first file name                                          */
 /****************************************************************************/

 for( ; pTable < pTableEnd; )
 {
  FileNameLen = *pTable;
  pFirstEntry = (FIRST_ENTRY_109_16*)(pTable + FileNameLen + 1);
  NumEntries  = pFirstEntry->NumEntries;

  /***************************************************************************/
  /* - find the Csect that this table section maps to.                       */
  /***************************************************************************/
  for(pCsect = pModule->pCsects; pCsect != NULL; pCsect=pCsect->next )
  {
   if( pCsect->SegNum == pFirstEntry->SegNum )
    break;
  }

  if( pCsect )
  {
   pmap->next              = (CSECTMAP*)Talloc(sizeof(CSECTMAP));
   pmap->next->pFirstEntry = pTable;
   pmap->next->pCsect      = pCsect;
   pmap->next->NumEntries  = NumEntries;
   pmap                    = pmap->next;
  }

  pTable = pTable + FileNameLen + 1 + sizeof(FIRST_ENTRY_109_16) +
                     (NumEntries)*sizeof(LINE_NUMBER_TABLE_ENTRY_109_16);
 }

 /****************************************************************************/
 /* - At this point, we have built a map of table sections to csects.        */
 /* - For each Csect, scan the map and count the number of entries           */
 /*   that will be in the line number table for that Csect.                  */
 /****************************************************************************/
 for(pCsect = pModule->pCsects; pCsect != NULL; pCsect=pCsect->next )
 {
  for( pmap = pCsectMap; pmap != NULL; pmap=pmap->next )
  {
   if(pmap->pCsect == pCsect)
    pCsect->NumEntries += pmap->NumEntries; /* call func to get real count */
  }
 }

 /****************************************************************************/
 /* - Add some "contrived" source file indexes for MS style tables.          */
 /****************************************************************************/
 if( pModule->pCsects )
  MakeSfis( pCsectMap );

/*---------------------------------------------------------------------------*/
/* - dump the map of line number tables to csects.                           */
/*****************************************************************************/
// DumpCsectMap( pModule, pCsectMap, TYPE109_16 );
/*---------------------------------------------------------------------------*/

 /****************************************************************************/
 /* - Now, scan the Csects, allocate the table, and map the entries to       */
 /*   the table.                                                             */
 /****************************************************************************/
 for( pCsect=pModule->pCsects; pCsect!=NULL; pCsect=pCsect->next )
 {
  pCsect->pLnoTab = Talloc(pCsect->NumEntries*sizeof(LNOTAB));
  pTablePtr = (UCHAR*)pCsect->pLnoTab;
  for( pmap = pCsectMap; pmap != NULL; pmap=pmap->next )
  {
   sfi = pmap->mssfi;
   if(pmap->pCsect == pCsect)
   {
    FIRST_ENTRY_109_16 *p109_16Table;

    pFileName    = pmap->pFirstEntry;
    p109_16Table = (FIRST_ENTRY_109_16 *)(pFileName + *pFileName + 1);

    pTablePtr = Append109_16Table((LNOTAB *)pTablePtr, p109_16Table,
                                  pCsect->SegFlatAddr, sfi);
   }
  }
 }

 /****************************************************************************/
 /* - now, build/add a file names list to the module.                        */
 /****************************************************************************/
 if( pModule->pCsects )
  AddFilesToModule( pModule, pCsectMap );

/*---------------------------------------------------------------------------*/
/* - dump the map of line number tables to csects.                           */
/*****************************************************************************/
// DumpLineNums( pModule );
/*---------------------------------------------------------------------------*/

 /****************************************************************************/
 /* - At this point, the line number table has been mapped to the Csect      */
 /*   tables.                                                                */
 /* - So, we can free the map.                                               */
 /* - And, we can free the bulk table.                                       */
 /****************************************************************************/
 for( pmap = pCsectMap ; pmap ; pmap = pnextmap )
  {pnextmap = pmap->next; Tfree(pmap);}

 if(pLineNumberTable) Tfree(pLineNumberTable);
}

/*****************************************************************************/
/* Build109_32Tables()                                                       */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Build the 109_32 tables.                                                */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   pModule                                                                 */
/*   pdf                                                                     */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*****************************************************************************/
void Build109_32Tables(  MODULE *pModule, DEBFILE *pdf )
{
 ULONG               lfo;
 ULONG               lfoend;
 ULONG               LineNumberTableSize;
 FIRST_ENTRY_109_32 *pFirstEntry;
 CSECTMAP           *pCsectMap = NULL;
 CSECTMAP           *pmap;
 CSECTMAP           *pnextmap;
 UCHAR              *pTablePtr;
 USHORT              NumEntries;
 CSECT              *pCsect;
 UCHAR              *pLineNumberTable;
 UCHAR              *pTable;
 UCHAR              *pTableEnd;
 int                 FileNameLen;
 int                 sfi;
 UCHAR              *pFileName;

 /****************************************************************************/
 /* - define long file offsets for the start and end of the table.           */
 /* - read the line number table into memory.                                */
 /****************************************************************************/
 lfo    = pdf->DebugOff + pModule->LineNums;
 lfoend = lfo + pModule->LineNumsLen - 1;

 LineNumberTableSize = lfoend - lfo + 1;
 pLineNumberTable    = Talloc( LineNumberTableSize );

 seekf(pdf, lfo);
 readf( pLineNumberTable, LineNumberTableSize, pdf);

 /****************************************************************************/
 /* - Scan the line number table and build  a map of the the table(s) to     */
 /*   csects.                                                                */
 /****************************************************************************/
 pTable       = pLineNumberTable;
 pTableEnd    = pLineNumberTable + LineNumberTableSize;
 pmap         = (CSECTMAP *)&pCsectMap;

 for( ; pTable < pTableEnd; )
 {
  FileNameLen = *pTable;
  pFirstEntry = (FIRST_ENTRY_109_32*)(pTable + FileNameLen + 1);
  NumEntries  = pFirstEntry->NumEntries;

  /***************************************************************************/
  /* - find the Csect that this table section maps to.                       */
  /***************************************************************************/
  for(pCsect = pModule->pCsects; pCsect != NULL; pCsect=pCsect->next )
  {
   if( pCsect->SegNum == pFirstEntry->SegNum )
    break;
  }

  if( pCsect )
  {
   pmap->next              = (CSECTMAP*)Talloc(sizeof(CSECTMAP));
   pmap->next->pFirstEntry = pTable;
   pmap->next->pCsect      = pCsect;
   pmap->next->NumEntries  = NumEntries;
   pmap                    = pmap->next;
  }

  pTable = pTable + FileNameLen + 1 + sizeof(FIRST_ENTRY_109_32) +
                     (NumEntries)*sizeof(LINE_NUMBER_TABLE_ENTRY_109_32);
 }

 /****************************************************************************/
 /* - At this point, we have built a map of table sections to csects.        */
 /* - For each Csect, scan the map and count the number of entries           */
 /*   that will be in the line number table for that Csect.                  */
 /****************************************************************************/
 for(pCsect = pModule->pCsects; pCsect != NULL; pCsect=pCsect->next )
 {
  for( pmap = pCsectMap; pmap != NULL; pmap=pmap->next )
  {
   if(pmap->pCsect == pCsect)
    pCsect->NumEntries += pmap->NumEntries; /* call func to get real count */
  }
 }

 /****************************************************************************/
 /* - Add some "contrived" source file indexes for MS style tables.          */
 /****************************************************************************/
 if( pModule->pCsects )
  MakeSfis( pCsectMap );

/*---------------------------------------------------------------------------*/
/* - dump the map of line number tables to csects.                           */
/*****************************************************************************/
// DumpCsectMap( pModule, pCsectMap, TYPE109_32 );
/*---------------------------------------------------------------------------*/

 /****************************************************************************/
 /* - Now, scan the Csects, allocate the table, and map the entries to       */
 /*   the table.                                                             */
 /****************************************************************************/
 for( pCsect=pModule->pCsects; pCsect!=NULL; pCsect=pCsect->next )
 {
  pCsect->pLnoTab = Talloc(pCsect->NumEntries*sizeof(LNOTAB));
  pTablePtr = (UCHAR*)pCsect->pLnoTab;
  for( pmap = pCsectMap; pmap != NULL; pmap=pmap->next )
  {
   sfi = pmap->mssfi;
   if(pmap->pCsect == pCsect)
   {
    FIRST_ENTRY_109_32 *p109_32Table;

    pFileName    = pmap->pFirstEntry;
    p109_32Table = (FIRST_ENTRY_109_32 *)(pFileName + *pFileName + 1);

    pTablePtr = Append109_32Table((LNOTAB *)pTablePtr, p109_32Table,
                                  pCsect->SegFlatAddr, sfi);
   }
  }
 }

 /****************************************************************************/
 /* - now, build/add a file names list to the module.                        */
 /****************************************************************************/
 if( pModule->pCsects )
  AddFilesToModule( pModule, pCsectMap );

/*---------------------------------------------------------------------------*/
/* - dump the map of line number tables to csects.                           */
/*****************************************************************************/
// DumpLineNums( pModule );
/*---------------------------------------------------------------------------*/

 /****************************************************************************/
 /* - At this point, the line number table has been mapped to the Csect      */
 /*   tables.                                                                */
 /* - So, we can free the map.                                               */
 /* - And, we can free the bulk table.                                       */
 /****************************************************************************/
 for( pmap = pCsectMap ; pmap ; pmap = pnextmap )
  {pnextmap = pmap->next; Tfree(pmap);}

 if(pLineNumberTable) Tfree(pLineNumberTable);
}

/*****************************************************************************/
/* Build105Tables()                                                          */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Build the 105 tables.                                                   */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   pModule                                                                 */
/*   pdf                                                                     */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*****************************************************************************/
void Build105Tables(  MODULE *pModule, DEBFILE *pdf )
{
 ULONG            lfo;
 ULONG            lfoend;
 ULONG            LineNumberTableSize;
 FIRST_ENTRY_105 *pFirstEntry;
 CSECTMAP        *pCsectMap = NULL;
 CSECTMAP        *pmap;
 CSECTMAP        *pnextmap;
 UCHAR           *pTablePtr;
 USHORT           NumEntries;
 CSECT           *pCsect;
 UCHAR           *pLineNumberTable;
 UCHAR           *pTable;
 UCHAR           *pTableEnd;
 int              FileNameLen;
 int              sfi;
 UCHAR           *pFileName;

 /****************************************************************************/
 /* - define long file offsets for the start and end of the table.           */
 /* - read the line number table into memory.                                */
 /****************************************************************************/
 lfo    = pdf->DebugOff + pModule->LineNums;
 lfoend = lfo + pModule->LineNumsLen - 1;

 LineNumberTableSize = lfoend - lfo + 1;
 pLineNumberTable    = Talloc( LineNumberTableSize );

 seekf(pdf, lfo);
 readf( pLineNumberTable, LineNumberTableSize, pdf);

 /****************************************************************************/
 /* - Scan the line number table and build  a map of the the table(s) to     */
 /*   csects.                                                                */
 /****************************************************************************/
 pTable       = pLineNumberTable;
 pTableEnd    = pLineNumberTable + LineNumberTableSize;
 pmap         = (CSECTMAP *)&pCsectMap;

 /****************************************************************************/
 /* - Move past the first file name                                          */
 /****************************************************************************/

 for( ; pTable < pTableEnd; )
 {
  FileNameLen = *pTable;
  pFirstEntry = (FIRST_ENTRY_105*)(pTable + FileNameLen + 1);
  NumEntries  = pFirstEntry->NumEntries;

  /***************************************************************************/
  /* - find the Csect that this table section maps to.                       */
  /* - 105 types only have one csect.                                        */
  /***************************************************************************/
  pCsect = pModule->pCsects;

  if( pCsect )
  {
   pmap->next              = (CSECTMAP*)Talloc(sizeof(CSECTMAP));
   pmap->next->pFirstEntry = pTable;
   pmap->next->pCsect      = pCsect;
   pmap->next->NumEntries  = NumEntries;
   pmap                    = pmap->next;
  }

  pTable = pTable + FileNameLen + 1 + sizeof(FIRST_ENTRY_105) +
                     (NumEntries)*sizeof(LINE_NUMBER_TABLE_ENTRY_105);
 }

 /****************************************************************************/
 /* - At this point, we have built a map of table sections to csects.        */
 /* - For each Csect, scan the map and count the number of entries           */
 /*   that will be in the line number table for that Csect.                  */
 /****************************************************************************/
 pCsect = pModule->pCsects;

 if( pCsect )
 {
  for( pmap = pCsectMap; pmap != NULL; pmap=pmap->next )
  {
   if(pmap->pCsect == pCsect)
    pCsect->NumEntries += pmap->NumEntries; /* call func to get real count */
  }
 }

 /****************************************************************************/
 /* - Add some "contrived" source file indexes for MS style tables.          */
 /****************************************************************************/
 if( pModule->pCsects )
  MakeSfis( pCsectMap );

/*---------------------------------------------------------------------------*/
/* - dump the map of line number tables to csects.                           */
/*****************************************************************************/
// DumpCsectMap( pModule, pCsectMap, TYPE105 );
/*---------------------------------------------------------------------------*/

 /****************************************************************************/
 /* - Now, scan the Csects, allocate the table, and map the entries to       */
 /*   the table.                                                             */
 /****************************************************************************/
 for( pCsect=pModule->pCsects; pCsect!=NULL; pCsect=pCsect->next )
 {
  pCsect->pLnoTab = Talloc(pCsect->NumEntries*sizeof(LNOTAB));
  pTablePtr = (UCHAR*)pCsect->pLnoTab;
  for( pmap = pCsectMap; pmap != NULL; pmap=pmap->next )
  {
   sfi = pmap->mssfi;
   if(pmap->pCsect == pCsect)
   {
    FIRST_ENTRY_105 *p105Table;

    pFileName = pmap->pFirstEntry;
    p105Table = (FIRST_ENTRY_105 *)(pFileName + *pFileName + 1);

    pTablePtr = Append105Table( (LNOTAB *)pTablePtr, p105Table,
                                pCsect->SegFlatAddr, sfi);
   }
  }
 }

 /****************************************************************************/
 /* - now, build/add a file names list to the module.                        */
 /****************************************************************************/
 if( pModule->pCsects )
  AddFilesToModule( pModule, pCsectMap );

/*---------------------------------------------------------------------------*/
/* - dump the map of line number tables to csects.                           */
/*****************************************************************************/
// DumpLineNums( pModule );
/*---------------------------------------------------------------------------*/

 /****************************************************************************/
 /* - At this point, the line number table has been mapped to the Csect      */
 /*   tables.                                                                */
 /* - So, we can free the map.                                               */
 /* - And, we can free the bulk table.                                       */
 /****************************************************************************/
 for( pmap = pCsectMap ; pmap ; pmap = pnextmap )
  {pnextmap = pmap->next; Tfree(pmap);}

 if(pLineNumberTable) Tfree(pLineNumberTable);
}

/*****************************************************************************/
/* GetModuleWithAddr()                                                       */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*  Find the module that contains the address.                               */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*  addr                                                                     */
/*  pdf                                                                      */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*  pModule                                                                  */
/*  NULL                                                                     */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*    pdf != NULL                                                            */
/*                                                                           */
/*****************************************************************************/
MODULE *GetModuleWithAddr( ULONG addr, DEBFILE *pdf )
{
 MODULE *pModule;

 for( pModule = pdf->MidAnchor; pModule; pModule = pModule->NextMod )
 {
  if( GetCsectWithAddr( pModule, addr) )
   break;
 }
 return( pModule );
}

/*****************************************************************************/
/* GetLnoWithAddr()                                                          */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*  Find the line number that contains this address.                         */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*  addr                                                                     */
/*  pCsect     -> to the csect containing the address.                       */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*  p          -> to the table entry with the line number containing         */
/*                this address.                                              */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*  - the line number table is already loaded.                               */
/*                                                                           */
/*****************************************************************************/
LNOTAB *GetLnoWithAddr( CSECT* pCsect, ULONG addr )
{
 LNOTAB *plnolo;
 LNOTAB *plnohi;
 int     i;
 LNOTAB *pLnoTab;
 int     NumEntries;

 pLnoTab    = pCsect->pLnoTab;
 NumEntries = pCsect->NumEntries;
 plnolo     = pLnoTab;
 plnohi     = plnolo + 1;

 for( i=1; i < NumEntries; i++ )
 {
  if( (addr >= plnolo->off) && ( addr < plnohi->off ) )
   { break;}
  plnolo++;
  if(plnolo->sfi == 0)
   plnolo++;
  plnohi++;
  if(plnohi->sfi == 0)
   plnohi++;
 }

 return( plnolo );
}

/*****************************************************************************/
/* GetFileName()                                                             */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*  Get the file name for a given module and sfi.                            */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*  mid                                                                      */
/*  sfi        source file index.                                            */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*  cp         -> to the file name.                                          */
/*             NULL==> did not find.                                         */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
char *GetFileName( ULONG mid, int sfi )
{
 MODULE   *pModule;
 FILENAME *pFile;
 char     *cp = NULL;

 pModule = GetPtrToModule( mid, NULL);
 if( pModule == NULL )
  return(NULL);

 for( pFile = pModule->pFiles; pFile != NULL; pFile=pFile->next )
 {
  if( sfi == pFile->sfi )
  {
   cp = pFile->FileName;
   break;
  }
 }
 return( cp );
}

/*****************************************************************************/
/* AddFileNames()                                                            */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*  Add the file name linked list to the module structure.                   */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*  pModule                                                                  */
/*  pFileNameTable                                                           */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*  void                                                                     */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
void AddFileNames( MODULE *pModule, UCHAR *pFileNameTable )
{
 CSECT    *pCsect;
 int       NumEntries;
 UCHAR    *pFileName;
 LNOTAB   *pLnoTabEntry;
 FILENAME *pLast;
 FILENAME *pFile;

 for(pCsect = pModule->pCsects; pCsect != NULL; pCsect=pCsect->next )
 {
  NumEntries   = pCsect->NumEntries;
  pLnoTabEntry = pCsect->pLnoTab;

  for( ; NumEntries--; pLnoTabEntry++ )
  {
   int sfi;

   sfi = pLnoTabEntry->sfi;
   if( sfi == 0 )
    continue;
   /**************************************************************************/
   /* - scan the list of files to see if this sfi is accounted for.          */
   /**************************************************************************/
   pLast = (FILENAME *)&(pModule->pFiles);
   for( pFile = pModule->pFiles; pFile != NULL; pLast=pFile, pFile=pFile->next )
   {
    if( sfi == pFile->sfi )
     break;
   }

   if( pFile == NULL )
   {
    int i;
    int size;
    int len;
    /*************************************************************************/
    /* - fall in here if this sfi is not yet accounted for.                  */
    /*************************************************************************/
    pFileName = pFileNameTable;
    for( i = 1; i < sfi; i++ )
    {
     pFileName += *pFileName + 1;
    }

    /*************************************************************************/
    /* - add the file name to the list.                                      */
    /*************************************************************************/
    len         = *pFileName;
    size        = sizeof(FILENAME) + len + 1;
    pFile       = Talloc(size);
    pFile->sfi  = sfi;
    strncpy( pFile->FileName, pFileName, len + 1 );
    pLast->next = pFile;
   }
  }
 }
}

/*****************************************************************************/
/* MakeSfis()                                                                */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*  Make source file indexes for MS style tables.                            */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*  pCsectMap                                                                */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*  void                                                                     */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
void MakeSfis( CSECTMAP *pCsectMap )
{
 int       sfi;
 UCHAR    *pFileName;
 UCHAR    *pFileNamex;
 CSECTMAP *pmap;
 CSECTMAP *pmapx;

 sfi = 1;
 for( pmap = pCsectMap; pmap != NULL; pmap=pmap->next, sfi++ )
 {
  pmap->IsFileUnique = TRUE;

  pFileName = pmap->pFirstEntry;
  for( pmapx = pCsectMap; pmapx != pmap; pmapx=pmapx->next )
  {
   pFileNamex = pmapx->pFirstEntry;
   if( strncmp( pFileName, pFileNamex, *pFileName+1 ) == 0 )
   {
    pmap->mssfi        = pmapx->mssfi;
    pmap->IsFileUnique = FALSE;
    break;
   }
  }
  if( pmap->IsFileUnique == TRUE )
   pmap->mssfi = sfi;
 }
}

/*****************************************************************************/
/* AddFilesToModule()                                                        */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*  Make a file name table for MS style tables.                              */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*  pCsectMap                                                                */
/*  pModule                                                                  */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*  void                                                                     */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
void AddFilesToModule( MODULE *pModule, CSECTMAP *pCsectMap )
{
 UCHAR    *pFileName;
 UCHAR    *pFileNamex;
 UCHAR    *pFileNameTable;
 CSECTMAP *pmap;
 int       FileTableLen;
 int       FileNameLen;

 FileTableLen = 0;
 for( pmap = pCsectMap; pmap != NULL; pmap=pmap->next )
 {
  pFileName     = pmap->pFirstEntry;
  FileTableLen += *pFileName + 1;
 }

 pFileNameTable = pFileNamex = Talloc(FileTableLen);

 for( pmap = pCsectMap; pmap != NULL; pmap=pmap->next )
 {
  pFileName   = pmap->pFirstEntry;
  FileNameLen = *pFileName;
  strncpy(pFileNamex, pFileName, FileNameLen + 1 );
  pFileNamex  += FileNameLen + 1;
 }
 AddFileNames( pModule, pFileNameTable );
 if(pFileNameTable) Tfree(pFileNameTable);
}


/*****************************************************************************/
/* DBLsegInfo                                                                */
/*                                                                           */
/* Description:                                                              */
/*   get logical segment containing address.                                 */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   addr       contained address.                                           */
/*   lenptr     where to put the segment size for the caller.                */
/*   pdf        pointer to debug file with the data.                         */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   base       base address of the csect containing the addr.               */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*  The addr is contained within a valid pdf-it has not been freed           */
/*  by DosFreeModule().                                                      */
/*                                                                           */
/*                                                                           */
/*****************************************************************************/
ULONG DBLsegInfo( ULONG addr, uint *lenptr , DEBFILE *pdf)
{
 MODULE     *mptr;
 CSECT  *pCsect;

 for(
     mptr = pdf->MidAnchor;
     mptr != NULL;
     mptr = mptr->NextMod
    )
 {
  if( (pCsect = GetCsectWithAddr( mptr, addr )) != NULL)
   break;
 }

 if ( pCsect )
 {
  *lenptr = pCsect->CsectSize;
  return(  pCsect->CsectLo );
 }

 /****************************************************************************/
 /* If we get here, then we were not able to find a module within the        */
 /* pdf that contained the addr. So, we will back up one level of            */
 /* resolution and try to find the code object that contains this            */
 /* address. We will use the code object and size.                           */
 /****************************************************************************/
 {
  int           i;
  uint          NumCodeObjs;
  uint          *pi;
  OBJTABLEENTRY *te;
  int            foundit = FALSE;


  NumCodeObjs = *(pi=pdf->CodeObjs);
  te = (OBJTABLEENTRY *)++pi;
  for(i=1; (foundit==FALSE) && (i <= NumCodeObjs); i++,te++ )
  {
   if( (te->ObjType == CODE)     &&
       (addr >= te->ObjLoadAddr) &&
       (addr <  te->ObjLoadAddr + te->ObjLoadSize) )
     { foundit = TRUE; break; }
  }

  /* te is pointing to the code object we want. */

  if( foundit == TRUE )
  {
   *lenptr = te->ObjLoadSize;
   return( te->ObjLoadAddr );
  }
 }
 return( NULL );
}

/*****************************************************************************/
/* DBModName                                                                 */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   get module name for a specifed module id.                               */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   mid           module id for the name we're looking for.                 */
/*   pdf        -> to EXE/DLL structure.                                     */
/*   ftype         0==>exe/dll name. 1==>source file name.                   */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   p          -> to the module name.                                       */
/*                                                                           */
/*****************************************************************************/
UCHAR *DBModName( ULONG mid , int sfi, DEBFILE *pdf , int *ftype)
{
 MODULE  *pModule;
 char    *p;
 UCHAR    ModuleNameLength = 0;
 char    *pSourceFileName;

 *ftype  = 0;
 /****************************************************************************/
 /* If linked without /CO, then return the EXE/DLL name.                     */
 /****************************************************************************/
 if ( pdf->SrcOrAsm == ASM_LEVEL )
 {
  ModuleNameLength = strlen(pdf->DebFilePtr->fn);
  p = Talloc(ModuleNameLength + 1);
  p[0] = ModuleNameLength;
  strncpy(p+1, pdf->DebFilePtr->fn, ModuleNameLength);
  return(p);
 }

 /****************************************************************************/
 /* - get a ptr to the module structure.                                     */
 /****************************************************************************/
 pModule = GetPtrToModule( mid, pdf );
 if(pModule == NULL)
  return(NULL);

 /****************************************************************************/
 /* - find the source file name.                                             */
 /****************************************************************************/
 pSourceFileName = GetFileName(mid, sfi);
 if( pSourceFileName != NULL )
 {
  *ftype = 1;
 }
 return(pSourceFileName);
}

/*****************************************************************************/
/* AppendHL01Tables()                                                        */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Append a line number table.                                             */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   pLnoTab    -> to the table we're mapping to.                            */
/*   pHL01Table -> to the table we're mapping from.                          */
/*   BaseOffset    table offsets are relative to this offset.                */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   pToEntry   -> to next available table LNOTAB slot.                      */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   pHL01Table points to a "FirstEntry" record.                             */
/*                                                                           */
/*****************************************************************************/
UCHAR *AppendHL01Table( LNOTAB *pLnoTab,
                        FIRST_ENTRY_HL01 *pHL01Table,
                        ULONG BaseOffset)
{
 LINE_NUMBER_TABLE_ENTRY_HL01 *pFromEntry;

 int     i;
 LNOTAB *pToEntry;
 USHORT  NumEntries;
 UCHAR  *pTable;

 NumEntries = pHL01Table->NumEntries;
 pTable     = (char*)pHL01Table + sizeof(FIRST_ENTRY_HL01);

 pFromEntry   = (LINE_NUMBER_TABLE_ENTRY_HL01 *)pTable;
 pToEntry     = pLnoTab;

 for( i=1; i <= NumEntries; i++ )
 {
  pToEntry->lno = pFromEntry->LineNumber;
  pToEntry->off = pFromEntry->Offset + BaseOffset;
  pToEntry->sfi = pFromEntry->SourceFileIndex;
  pToEntry++;
  pFromEntry++;
 }
 return((UCHAR*)pToEntry);
}

/*****************************************************************************/
/* AppendHL04Tables()                                                        */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Append a line number table.                                             */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   pLnoTab    -> to the table we're mapping to.                            */
/*   pHL04Table -> to the table we're mapping from.                          */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   pToEntry   -> to next available table LNOTAB slot.                      */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   pHL04Table points to a "FirstEntry" record.                             */
/*                                                                           */
/*****************************************************************************/
UCHAR *AppendHL04Table( LNOTAB           *pLnoTab,
                        FIRST_ENTRY_HL04 *pHL04Table,
                        ULONG             MteLoadAddr )
{
 LINE_NUMBER_TABLE_ENTRY_HL04 *pFromEntry;

 int     i;
 LNOTAB *pToEntry;
 char   *cptr;
 USHORT  NumEntries;
 ULONG   BaseOffset;

 /****************************************************************************/
 /* - define the number of entries from the first entry record.              */
 /* - define the base offset address from the first entry record.            */
 /* - define a ptr to the first (lno,off,sfi) entry.                         */
 /****************************************************************************/
 NumEntries = pHL04Table->NumEntries;
 BaseOffset = MteLoadAddr + pHL04Table->BaseOffset - 0x10000;
 cptr       = (char*)pHL04Table + sizeof(FIRST_ENTRY_HL04);
 pFromEntry = (LINE_NUMBER_TABLE_ENTRY_HL04 *)cptr;
 pToEntry   = pLnoTab;

 for( i=1; i <= NumEntries; i++ )
 {
  pToEntry->lno = pFromEntry->LineNumber;
  pToEntry->off = pFromEntry->Offset + BaseOffset;
  pToEntry->sfi = pFromEntry->SourceFileIndex;
  pToEntry++;
  pFromEntry++;
 }
 return((UCHAR*)pToEntry);
}

/*****************************************************************************/
/* AppendHL03Tables()                                                        */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Append a line number table to a csect table.                            */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   pLnoTab    -> to the table we're mapping to.                            */
/*   pHL04Table -> to the table we're mapping from.                          */
/*   BaseOffset    table offsets are relative to this base.                  */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   pToEntry   -> to next available table LNOTAB slot.                      */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   pHL03Table points to a "FirstEntry" record.                             */
/*                                                                           */
/*****************************************************************************/
UCHAR *AppendHL03Table( LNOTAB *pLnoTab,
                        FIRST_ENTRY_HL03 *pHL03Table,
                        ULONG BaseOffset)
{
 LINE_NUMBER_TABLE_ENTRY_HL03 *pFromEntry;

 int     i;
 LNOTAB *pToEntry;
 char   *cptr;
 USHORT  NumEntries;

 /****************************************************************************/
 /* - define the number of entries from the first entry record.              */
 /* - define the base offset address from the first entry record.            */
 /* - define a ptr to the first (lno,off,sfi) entry.                         */
 /****************************************************************************/
 NumEntries   = pHL03Table->NumEntries;

 cptr         = (char*)pHL03Table + sizeof(FIRST_ENTRY_HL03);
 pFromEntry   = (LINE_NUMBER_TABLE_ENTRY_HL03 *)cptr;
 pToEntry     = pLnoTab;

 for( i=1; i <= NumEntries; i++ )
 {
  pToEntry->lno = pFromEntry->LineNumber;
  pToEntry->off = pFromEntry->Offset + BaseOffset;
  pToEntry->sfi = pFromEntry->SourceFileIndex;
  pToEntry++;
  pFromEntry++;
 }
 return((UCHAR*)pToEntry);
}

/*****************************************************************************/
/* Append109_16Tables()                                                      */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Append a line number table to a csect table.                            */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   pLnoTab    -> to the table we're mapping to.                            */
/*   p109_16Table  -> to the table we're mapping from.                       */
/*   BaseOffset    table offsets are relative to this base.                  */
/*   sfi           source file index                                         */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   pToEntry   -> to next available table LNOTAB slot.                      */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   p109_16Table points to a "FirstEntry" record.                           */
/*                                                                           */
/*****************************************************************************/
UCHAR *Append109_16Table( LNOTAB *pLnoTab,
                       FIRST_ENTRY_109_16 *p109_16Table,
                        ULONG BaseOffset,
                        int sfi)
{
 LINE_NUMBER_TABLE_ENTRY_109_16 *pFromEntry;

 int     i;
 LNOTAB *pToEntry;
 char   *cptr;
 USHORT  NumEntries;

 /****************************************************************************/
 /* - define the number of entries from the first entry record.              */
 /* - define the base offset address from the first entry record.            */
 /* - define a ptr to the first (lno,off,sfi) entry.                         */
 /****************************************************************************/
 NumEntries   = p109_16Table->NumEntries;
 cptr         = (char*)p109_16Table + sizeof(FIRST_ENTRY_109_16);
 pFromEntry   = (LINE_NUMBER_TABLE_ENTRY_109_16 *)cptr;
 pToEntry     = pLnoTab;

 for( i=1; i <= NumEntries; i++ )
 {
  pToEntry->lno = pFromEntry->LineNumber;
  pToEntry->off = pFromEntry->Offset + BaseOffset;
  pToEntry->sfi = sfi;
  pToEntry++;
  pFromEntry++;
 }
 return((UCHAR*)pToEntry);
}

/*****************************************************************************/
/* Append109_32Tables()                                                      */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Append a line number table.                                             */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   pLnoTab    -> to the table we're mapping to.                            */
/*   p109_32Table  -> to the table we're mapping from.                       */
/*   BaseOffset    table values are relative to this base.                   */
/*                 relative.                                                 */
/*   sfi           source file index                                         */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   pToEntry   -> to next available table LNOTAB slot.                      */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   p109_32Table points to a "FirstEntry" record.                           */
/*                                                                           */
/*****************************************************************************/
UCHAR *Append109_32Table( LNOTAB *pLnoTab,
                          FIRST_ENTRY_109_32 *p109_32Table,
                          ULONG BaseOffset,
                          int    sfi)
{
 LINE_NUMBER_TABLE_ENTRY_109_32 *pFromEntry;

 int     i;
 LNOTAB *pToEntry;
 char   *cptr;
 USHORT  NumEntries;

 /****************************************************************************/
 /* - define the number of entries from the first entry record.              */
 /* - define the base offset address from the first entry record.            */
 /* - define a ptr to the first (lno,off,sfi) entry.                         */
 /****************************************************************************/
 NumEntries   = p109_32Table->NumEntries;
 cptr         = (char*)p109_32Table + sizeof(FIRST_ENTRY_109_32);
 pFromEntry   = (LINE_NUMBER_TABLE_ENTRY_109_32 *)cptr;
 pToEntry     = pLnoTab;

 for( i=1; i <= NumEntries; i++ )
 {
  pToEntry->lno = pFromEntry->LineNumber;
  pToEntry->off = pFromEntry->Offset + BaseOffset;
  pToEntry->sfi = sfi;
  pToEntry++;
  pFromEntry++;
 }
 return((UCHAR*)pToEntry);
}

/*****************************************************************************/
/* Append105Tables()                                                         */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Append a line number table.                                             */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   pLnoTab    -> to the table we're mapping to.                            */
/*   p105Table  -> to the table we're mapping from.                          */
/*   BaseOffset    table offsets are relative to this base.                  */
/*   sfi           source file index                                         */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   pToEntry   -> to next available table LNOTAB slot.                      */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   p105Table points to a "FirstEntry" record.                              */
/*                                                                           */
/*****************************************************************************/
UCHAR *Append105Table( LNOTAB *pLnoTab,
                       FIRST_ENTRY_105 *p105Table,
                       ULONG BaseOffset,
                       int    sfi)
{
 LINE_NUMBER_TABLE_ENTRY_105 *pFromEntry;

 int     i;
 LNOTAB *pToEntry;
 char   *cptr;
 USHORT  NumEntries;

 /****************************************************************************/
 /* - define the number of entries from the first entry record.              */
 /* - define the base offset address from the first entry record.            */
 /* - define a ptr to the first (lno,off,sfi) entry.                         */
 /****************************************************************************/
 NumEntries   = p105Table->NumEntries;
 cptr         = (char*)p105Table + sizeof(FIRST_ENTRY_105);
 pFromEntry   = (LINE_NUMBER_TABLE_ENTRY_105 *)cptr;
 pToEntry     = pLnoTab;

 for( i=1; i <= NumEntries; i++ )
 {
  pToEntry->lno = pFromEntry->LineNumber;
  pToEntry->off = pFromEntry->Offset + BaseOffset;
  pToEntry->sfi = sfi;
  pToEntry++;
  pFromEntry++;
 }
 return((UCHAR*)pToEntry);
}

/*****************************************************************************/
/* GetLnoTabEntry()                                                          */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Get the line number table entry for a given (lno,sfi).                  */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   pModule                                                                 */
/*   lno                                                                     */
/*   sfi           source file index                                         */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   pLnoTab       line number table entry.                                  */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   sfi != 0                                                                */
/*                                                                           */
/*****************************************************************************/
LNOTAB *GetLnoTabEntry( MODULE *pModule, int lno, int sfi )
{
 CSECT  *pCsect;
 LNOTAB *pLnoTabEntry;
 LNOTAB *pLnoTab;
 int     NumEntries;
 ULONG   lnolo;

 pLnoTab = NULL;
 pCsect  = pModule->pCsects;

 if( lno == 0 )
 {
  /***************************************************************************/
  /* - find the lowest line number in the tables for this sfi.               */
  /***************************************************************************/
  lnolo = 0xffffffff;
  for( ; pCsect != NULL; pCsect=pCsect->next)
  {
   NumEntries   = pCsect->NumEntries;
   pLnoTabEntry = pCsect->pLnoTab;

   if( (pLnoTabEntry != NULL) && ( NumEntries > 0 ) )
   {
    for( ; NumEntries; pLnoTabEntry++, NumEntries-- )
    {
     if( (pLnoTabEntry->sfi == sfi) && (pLnoTabEntry->lno < lnolo ) )
     {
      lnolo   = pLnoTabEntry->lno;
      pLnoTab = pLnoTabEntry;
      break;
     }
    }
   }
  }
 }
 else /* if(lno != 0) */
 {
  for( ; (pLnoTab == NULL) && (pCsect != NULL); pCsect=pCsect->next)
  {
   NumEntries   = pCsect->NumEntries;
   pLnoTabEntry = pCsect->pLnoTab;

   if( (pLnoTabEntry != NULL) && ( NumEntries > 0 ) )
   {
    for( ; NumEntries; pLnoTabEntry++, NumEntries-- )
    {
     if( (pLnoTabEntry->sfi == sfi) && (pLnoTabEntry->lno == lno ) )
     {
      pLnoTab = pLnoTabEntry;
      break;
     }
    }
   }
  }
 }
 return(pLnoTab);
}

/*****************************************************************************/
/* GetLnoTabNextEntry()                                                      */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Get the next executable line following the (lno,sfi).                   */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   pModule                                                                 */
/*   lno                                                                     */
/*   sfi           source file index                                         */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   pLnoTab       line number table entry.                                  */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   sfi != 0                                                                */
/*   pModule != NULL                                                         */
/*                                                                           */
/*****************************************************************************/
LNOTAB *GetLnoTabNextEntry( MODULE *pModule, int lno, int sfi )
{
 CSECT  *pCsect;
 LNOTAB *pLnoTabEntry;
 LNOTAB *pLnoNear;
 int     NumEntries;
 ULONG   LnoNear;

 /****************************************************************************/
 /* - scan the tables and get the max lno for this sfi.                      */
 /****************************************************************************/
 LnoNear  = 0xffffffff;
 pLnoNear = NULL;
 pCsect   = pModule->pCsects;

 for( ; pCsect != NULL; pCsect=pCsect->next)
 {
  NumEntries   = pCsect->NumEntries;
  pLnoTabEntry = pCsect->pLnoTab;

  if( (pLnoTabEntry != NULL) && ( NumEntries > 0 ) )
  {
   for( ; NumEntries; pLnoTabEntry++, NumEntries-- )
   {
    if( (pLnoTabEntry->sfi == sfi) &&
        (pLnoTabEntry->lno > lno ) &&
        (pLnoTabEntry->lno < LnoNear)
      )
    {
     pLnoNear = pLnoTabEntry;
     LnoNear  = pLnoNear->lno;
    }
   }
  }
 }

 return(pLnoNear);
}

/*****************************************************************************/
/* GetLineSpan()                                                             */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Get the span of a line.                                                 */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   pModule                                                                 */
/*   pLnoTabBase   -> to line number table entry containing the base line    */
/*                    that we want the span for.                             */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   span                                                                    */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   pModule != NULL                                                         */
/*   pLnoTabBase != NULL                                                     */
/*                                                                           */
/*****************************************************************************/
ULONG GetLineSpan( MODULE *pModule, LNOTAB *pLnoTabBase  )
{
 CSECT  *pCsect;
 LNOTAB *pLnoTab;
 int     NumEntries;
 ULONG   addrlo;
 ULONG   addrhi;
 ULONG   LeastDelta;
 ULONG   delta;
 ULONG   span;
 int     n;

 span   = 0;
 pCsect = GetCsectWithAddr( pModule, pLnoTabBase->off);

 if( pCsect )
 {
  LeastDelta = 0xFFFFFFFF;
  addrlo     = pLnoTabBase->off;
  NumEntries = pCsect->NumEntries;
  pLnoTab    = pCsect->pLnoTab;

  for ( n=1 ; n <= NumEntries ; n++, pLnoTab++ )
  {
   addrhi = pLnoTab->off;
   if( (addrhi > addrlo) &&
       ( (delta = (addrhi - addrlo)) < LeastDelta )
     )
    LeastDelta = delta;
/* printf("\n%10x %10x %10x", addrlo, addrhi, LeastDelta);fflush(0); */
  }

  if( LeastDelta == 0xFFFFFFFF )
   span = pCsect->CsectHi - pLnoTabBase->off + 1;
  else
   span = LeastDelta;
 }

 return(span);
}

/*****************************************************************************/
/* GetHL04Sfi()                                                              */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Get the sfi for the file.                                               */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   pFileName                                                               */
/*   mid                                                                     */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*****************************************************************************/
int GetHL04Sfi( ULONG mid, UCHAR *pFileName )
{
 HEADER_HL04  Header;
 MODULE      *pModule;
 ULONG        FileNameTableLen;
 UCHAR       *pFileNameTable = NULL;
 ULONG        lfo;
 DEBFILE     *pdf;
 int          sfi;

 pdf     = DBFindPdf( mid );
 pModule = GetPtrToModule( mid, pdf );
 if( !(pdf && pModule) )
  return(NULL);

 /**************************************************************************/
 /* - move the file pointer to the start of the line number subsection.    */
 /* - read the header and get the filename table size.                     */
 /**************************************************************************/
 lfo = pdf->DebugOff + pModule->LineNums;

 seekf(pdf, lfo);
 readf((UCHAR *)&Header, sizeof(Header), pdf);

 FileNameTableLen = Header.FileNameTableSize -
                     sizeof(FILE_NAME_TABLE_ENTRY_HL04) + 2;

 /**************************************************************************/
 /* - allocate the table.                                                  */
 /* - move past the header.                                                */
 /**************************************************************************/
 pFileNameTable = Talloc(FileNameTableLen);

 lfo += sizeof(HEADER_HL04) + sizeof(FILE_NAME_TABLE_ENTRY_HL04) - 1;
 seekf(pdf, lfo);
 readf( (UCHAR *)pFileNameTable, FileNameTableLen, pdf );

 sfi = GetSfi( pFileName, pFileNameTable, FileNameTableLen );

 Tfree(pFileNameTable);
 return(sfi);
}

/*****************************************************************************/
/* GetHL03Sfi()                                                              */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Get the sfi for the file.                                               */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   pFileName                                                               */
/*   mid                                                                     */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*****************************************************************************/
int GetHL03Sfi( ULONG mid, UCHAR *pFileName )
{
 FIRST_ENTRY_HL03  FirstEntry;
 MODULE           *pModule;
 ULONG             FileNameTableLen;
 UCHAR            *pFileNameTable = NULL;
 ULONG             lfo;
 DEBFILE          *pdf;
 int               sfi;
 int               NumEntries;

 pdf     = DBFindPdf( mid );
 pModule = GetPtrToModule( mid, pdf );
 if( !(pdf && pModule) )
  return(NULL);

 /************************************************************************/
 /* - move the file pointer to the start of the line number subsection.  */
 /* - read the first entry.                                              */
 /* - calculate the table size - the table header.                       */
 /* - allocate the table.                                                */
 /* - move to the start of the table and read it.                        */
 /************************************************************************/
 lfo = pdf->DebugOff + pModule->LineNums;
 seekf(pdf, lfo);
 readf((UCHAR *)&FirstEntry, sizeof(FirstEntry), pdf);

 FileNameTableLen = FirstEntry.FileNameTableSize -
                     sizeof(FILE_NAME_TABLE_ENTRY_HL03) + 2;

 pFileNameTable = Talloc(FileNameTableLen);

 NumEntries = FirstEntry.NumEntries;

 lfo  = lfo + sizeof(FIRST_ENTRY_HL03) +
        NumEntries*sizeof(LINE_NUMBER_TABLE_ENTRY_HL03) +
        sizeof(FILE_NAME_TABLE_ENTRY_HL03) - 1;

 seekf(pdf, lfo);

 readf( (UCHAR *)pFileNameTable, FileNameTableLen, pdf );

 sfi = GetSfi( pFileName, pFileNameTable, FileNameTableLen );
 Tfree(pFileNameTable);
 return(sfi);
}

/*****************************************************************************/
/* GetHL01Sfi()                                                              */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Get the sfi for the file.                                               */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   pFileName                                                               */
/*   mid                                                                     */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*****************************************************************************/
int GetHL01Sfi( ULONG mid, UCHAR *pFileName )
{
 FIRST_ENTRY_HL01  FirstEntry;
 MODULE           *pModule;
 ULONG             FileNameTableLen;
 UCHAR            *pFileNameTable = NULL;
 ULONG             lfo;
 DEBFILE          *pdf;
 int               sfi;
 int               NumEntries;
 ULONG             lfostart;
 UCHAR             FileNameLen;
 USHORT            PathTableEntries;

 FILE_NAME_TABLE_ENTRY_HL01    FirstFileNameTableEntry;

 pdf     = DBFindPdf( mid );
 pModule = GetPtrToModule( mid, pdf );
 if( !(pdf && pModule) )
  return(NULL);

 /************************************************************************/
 /* - move the file pointer to the start of the line number subsection.  */
 /* - read the first entry.                                              */
 /************************************************************************/
 lfo = pdf->DebugOff + pModule->LineNums;
 seekf(pdf, lfo);
 readf((UCHAR *)&FirstEntry, sizeof(FirstEntry), pdf);

 /****************************************************************************/
 /* - move to the start of the file name table and read the first entry.     */
 /****************************************************************************/
 NumEntries       = FirstEntry.NumEntries;
 PathTableEntries = FirstEntry.PathTableEntries;
 lfo = lfo + sizeof(FirstEntry) +
       NumEntries*sizeof(LINE_NUMBER_TABLE_ENTRY_HL01) +
       PathTableEntries*sizeof(PATH_TABLE_ENTRY_HL01);

 seekf(pdf, lfo);
 readf( (UCHAR *)&FirstFileNameTableEntry,
                    sizeof(FirstFileNameTableEntry), pdf);

 lfostart         = lfo + sizeof(FirstFileNameTableEntry) - 1;
 NumEntries       = FirstFileNameTableEntry.NumberOfSourceFiles;
 FileNameTableLen = 0;
 for( lfo = lfostart; NumEntries--; )
 {
  seekf(pdf, lfo);
  readf( (UCHAR *)&FileNameLen, 1, pdf );
  FileNameTableLen += FileNameLen + 1;
  lfo += FileNameLen + 1;
 }
 FileNameTableLen += 1; // to grow on

 pFileNameTable = Talloc(FileNameTableLen);

 seekf( pdf, lfostart );
 readf( (UCHAR *)pFileNameTable, FileNameTableLen, pdf );

 sfi = GetSfi( pFileName, pFileNameTable, FileNameTableLen );
 Tfree(pFileNameTable);
 return(sfi);
}

int GetSfi( UCHAR *pFileName, UCHAR *pFileNameTable, ULONG FileNameTableLen )
{
 int               sfi;
 int               index;
 UCHAR            *pTable;
 UCHAR            *pTableEnd;

 pTable    = pFileNameTable;
 pTableEnd = pFileNameTable + FileNameTableLen - 1;
 sfi       = 0;
 for( index=1 ; pTable < pTableEnd; index++ )
 {
  char   *cp  = NULL;
  char   *cpp = NULL;
  int     len;
  UCHAR   SaveByte;

  cp  = pTable;

  SaveByte    = pTable[*cp+1];
  pTable[*cp+1] = 0;
  cpp         = strrchr( cp+1, '\\' );

  if( cpp == NULL )
  {
   /**************************************************************************/
   /* - come here if no path info.                                           */
   /**************************************************************************/
   len = *cp;
   if( (len == *pFileName) && (strnicmp(pFileName, cp, len+1) == 0) )
   {
    sfi = index;
    break;
   }
  }
  else
  {
   /**************************************************************************/
   /* - come here if there is path info.                                     */
   /**************************************************************************/
   cpp =  strrchr( cp+1, '\\' );
   cpp = cpp + 1;
   len = *cp -(cpp-cp) + 1;
   if( (len == *pFileName) && (strnicmp(pFileName+1, cpp, len) == 0) )
   {
    sfi = index;
    break;
   }
  }
  pTable[*cp+1] = SaveByte;
  pTable += *pTable + 1;
 }
 return(sfi);
}

/*****************************************************************************/
/* Get109_16Sfi()                                                            */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Get the source file index in a 109_16 table.                            */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   pFileName                                                               */
/*   mid                                                                     */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*****************************************************************************/
int Get109_16Sfi( ULONG mid, UCHAR *pFileName )
{
 ULONG                lfo;
 ULONG                lfoend;
 ULONG                LineNumberTableSize;
 FIRST_ENTRY_109_16  *pFirstEntry;
 FILE_NAME_TABLE_MAP *pTableMap = NULL;
 FILE_NAME_TABLE_MAP *pmap;
 FILE_NAME_TABLE_MAP *pnextmap;
 USHORT               NumEntries;
 UCHAR               *pLineNumberTable;
 UCHAR               *pTable;
 UCHAR               *pTableEnd;
 int                  FileNameLen;
 int                  sfi;
 UCHAR               *pFileNameTable;
 ULONG                FileNameTableLen;
 DEBFILE             *pdf;
 MODULE              *pModule;
 int                  len;

 pdf     = DBFindPdf( mid );
 pModule = GetPtrToModule( mid, pdf );
 if( !(pdf && pModule) )
  return(NULL);

 /****************************************************************************/
 /* - define long file offsets for the start and end of the table.           */
 /* - read the line number table into memory.                                */
 /****************************************************************************/
 lfo    = pdf->DebugOff + pModule->LineNums;
 lfoend = lfo + pModule->LineNumsLen - 1;

 LineNumberTableSize = lfoend - lfo + 1;
 pLineNumberTable    = Talloc( LineNumberTableSize );

 seekf(pdf, lfo);
 readf( pLineNumberTable, LineNumberTableSize, pdf);

 /****************************************************************************/
 /* - Scan the line number table and build  a map of pointers to tables.     */
 /****************************************************************************/
 pTable       = pLineNumberTable;
 pTableEnd    = pLineNumberTable + LineNumberTableSize;
 pmap         = (FILE_NAME_TABLE_MAP *)&pTableMap;

 for( ; pTable < pTableEnd; )
 {
  FileNameLen = *pTable;
  pFirstEntry = (FIRST_ENTRY_109_16*)(pTable + FileNameLen + 1);
  NumEntries  = pFirstEntry->NumEntries;

  len = sizeof(FILE_NAME_TABLE_MAP);

  pmap->next              = (FILE_NAME_TABLE_MAP*)Talloc(len);
  pmap->next->pFirstEntry = pTable;
  pmap                    = pmap->next;

  pTable = pTable + FileNameLen + 1 + sizeof(FIRST_ENTRY_109_16) +
                     (NumEntries)*sizeof(LINE_NUMBER_TABLE_ENTRY_109_16);
 }

 pFileNameTable = BuildFileNameTable( pTableMap, &FileNameTableLen );
 sfi            = GetSfi( pFileName, pFileNameTable, FileNameTableLen );

 /****************************************************************************/
 /* - free the table allocated by BuildFileNameTable().                      */
 /* - free the map.                                                          */
 /* - free the line number table.                                            */
 /****************************************************************************/
 if( pFileNameTable ) Tfree(pFileNameTable);

 for( pmap = pTableMap ; pmap ; pmap = pnextmap )
  {pnextmap = pmap->next; Tfree(pmap);}

 if(pLineNumberTable) Tfree(pLineNumberTable);
 return(sfi);
}

/*****************************************************************************/
/* Get105Sfi()                                                               */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Get the source file index in a 109_16 table.                            */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   pFileName                                                               */
/*   mid                                                                     */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*****************************************************************************/
int Get105Sfi( ULONG mid, UCHAR *pFileName )
{
 ULONG                lfo;
 ULONG                lfoend;
 ULONG                LineNumberTableSize;
 FIRST_ENTRY_105     *pFirstEntry;
 FILE_NAME_TABLE_MAP *pTableMap = NULL;
 FILE_NAME_TABLE_MAP *pmap;
 FILE_NAME_TABLE_MAP *pnextmap;
 USHORT               NumEntries;
 UCHAR               *pLineNumberTable;
 UCHAR               *pTable;
 UCHAR               *pTableEnd;
 int                  FileNameLen;
 int                  sfi;
 UCHAR               *pFileNameTable;
 ULONG                FileNameTableLen;
 DEBFILE             *pdf;
 MODULE              *pModule;
 int                  len;

 pdf     = DBFindPdf( mid );
 pModule = GetPtrToModule( mid, pdf );
 if( !(pdf && pModule) )
  return(NULL);

 /****************************************************************************/
 /* - define long file offsets for the start and end of the table.           */
 /* - read the line number table into memory.                                */
 /****************************************************************************/
 lfo    = pdf->DebugOff + pModule->LineNums;
 lfoend = lfo + pModule->LineNumsLen - 1;

 LineNumberTableSize = lfoend - lfo + 1;
 pLineNumberTable    = Talloc( LineNumberTableSize );

 seekf(pdf, lfo);
 readf( pLineNumberTable, LineNumberTableSize, pdf);

 /****************************************************************************/
 /* - Scan the line number table and build  a map of pointers to tables.     */
 /****************************************************************************/
 pTable       = pLineNumberTable;
 pTableEnd    = pLineNumberTable + LineNumberTableSize;
 pmap         = (FILE_NAME_TABLE_MAP *)&pTableMap;

 for( ; pTable < pTableEnd; )
 {
  FileNameLen = *pTable;
  pFirstEntry = (FIRST_ENTRY_105*)(pTable + FileNameLen + 1);
  NumEntries  = pFirstEntry->NumEntries;

  len = sizeof(FILE_NAME_TABLE_MAP);

  pmap->next              = (FILE_NAME_TABLE_MAP*)Talloc(len);
  pmap->next->pFirstEntry = pTable;
  pmap                    = pmap->next;

  pTable = pTable + FileNameLen + 1 + sizeof(FIRST_ENTRY_105) +
                     (NumEntries)*sizeof(LINE_NUMBER_TABLE_ENTRY_105);
 }

 pFileNameTable = BuildFileNameTable( pTableMap, &FileNameTableLen );
 sfi            = GetSfi( pFileName, pFileNameTable, FileNameTableLen );

 /****************************************************************************/
 /* - free the table allocated by BuildFileNameTable().                      */
 /* - free the map.                                                          */
 /* - free the line number table.                                            */
 /****************************************************************************/
 if( pFileNameTable ) Tfree(pFileNameTable);

 for( pmap = pTableMap ; pmap ; pmap = pnextmap )
  {pnextmap = pmap->next; Tfree(pmap);}

 if(pLineNumberTable) Tfree(pLineNumberTable);
 return(sfi);
}

/*****************************************************************************/
/* Get109_32Sfi()                                                            */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Get the source file index in a 109_16 table.                            */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   pFileName                                                               */
/*   mid                                                                     */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*****************************************************************************/
int Get109_32Sfi( ULONG mid, UCHAR *pFileName )
{
 ULONG                lfo;
 ULONG                lfoend;
 ULONG                LineNumberTableSize;
 FIRST_ENTRY_109_32  *pFirstEntry;
 FILE_NAME_TABLE_MAP *pTableMap = NULL;
 FILE_NAME_TABLE_MAP *pmap;
 FILE_NAME_TABLE_MAP *pnextmap;
 USHORT               NumEntries;
 UCHAR               *pLineNumberTable;
 UCHAR               *pTable;
 UCHAR               *pTableEnd;
 int                  FileNameLen;
 int                  sfi;
 UCHAR               *pFileNameTable;
 ULONG                FileNameTableLen;
 DEBFILE             *pdf;
 MODULE              *pModule;
 int                  len;

 pdf     = DBFindPdf( mid );
 pModule = GetPtrToModule( mid, pdf );
 if( !(pdf && pModule) )
  return(NULL);

 /****************************************************************************/
 /* - define long file offsets for the start and end of the table.           */
 /* - read the line number table into memory.                                */
 /****************************************************************************/
 lfo    = pdf->DebugOff + pModule->LineNums;
 lfoend = lfo + pModule->LineNumsLen - 1;

 LineNumberTableSize = lfoend - lfo + 1;
 pLineNumberTable    = Talloc( LineNumberTableSize );

 seekf(pdf, lfo);
 readf( pLineNumberTable, LineNumberTableSize, pdf);

 /****************************************************************************/
 /* - Scan the line number table and build  a map of pointers to tables.     */
 /****************************************************************************/
 pTable       = pLineNumberTable;
 pTableEnd    = pLineNumberTable + LineNumberTableSize;
 pmap         = (FILE_NAME_TABLE_MAP *)&pTableMap;

 for( ; pTable < pTableEnd; )
 {
  FileNameLen = *pTable;
  pFirstEntry = (FIRST_ENTRY_109_32*)(pTable + FileNameLen + 1);
  NumEntries  = pFirstEntry->NumEntries;

  len = sizeof(FILE_NAME_TABLE_MAP);

  pmap->next              = (FILE_NAME_TABLE_MAP*)Talloc(len);
  pmap->next->pFirstEntry = pTable;
  pmap                    = pmap->next;

  pTable = pTable + FileNameLen + 1 + sizeof(FIRST_ENTRY_109_32) +
                     (NumEntries)*sizeof(LINE_NUMBER_TABLE_ENTRY_109_32);
 }

 pFileNameTable = BuildFileNameTable( pTableMap, &FileNameTableLen );
 sfi            = GetSfi( pFileName, pFileNameTable, FileNameTableLen );

 /****************************************************************************/
 /* - free the table allocated by BuildFileNameTable().                      */
 /* - free the map.                                                          */
 /* - free the line number table.                                            */
 /****************************************************************************/
 if( pFileNameTable ) Tfree(pFileNameTable);

 for( pmap = pTableMap ; pmap ; pmap = pnextmap )
  {pnextmap = pmap->next; Tfree(pmap);}

 if(pLineNumberTable) Tfree(pLineNumberTable);
 return(sfi);
}

/*****************************************************************************/
/* BuildFileNameTable()                                                      */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*  Build a file name table for MS style tables.                             */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*  pTableMap                                                                */
/*  pTableLen                                                                */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*  pFileNameTable                                                           */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
UCHAR *BuildFileNameTable( FILE_NAME_TABLE_MAP *pTableMap, ULONG *pTableLen )
{
 UCHAR    *pFileName;
 UCHAR    *pFileNamex;
 UCHAR    *pFileNameTable;
 int       FileTableLen;
 int       FileNameLen;

 FILE_NAME_TABLE_MAP *pmap;

 FileTableLen = 0;
 for( pmap = pTableMap; pmap != NULL; pmap=pmap->next )
 {
  pFileName     = pmap->pFirstEntry;
  FileTableLen += *pFileName + 1;
 }

 FileTableLen   += 1;
 pFileNameTable  = pFileNamex = Talloc(FileTableLen);

 for( pmap = pTableMap; pmap != NULL; pmap=pmap->next )
 {
  pFileName   = pmap->pFirstEntry;
  FileNameLen = *pFileName;
  strncpy(pFileNamex, pFileName, FileNameLen + 1 );
  pFileNamex  += FileNameLen + 1;
 }

 *pTableLen = FileTableLen;
 return(pFileNameTable);
}
