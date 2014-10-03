/*****************************************************************************/
/* File:                                                                     */
/*   maplines.c                                                              */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Mapping funtions for line numbers from other formats to the internal    */
/*   format.                                                                 */
/*                                                                           */
/* History:                                                                  */
/*                                                                           */
/*   04/06/95 Updated.                                                       */
/*                                                                           */
/*****************************************************************************/

#include "all.h"

/*****************************************************************************/
/* MapHL04LineNumbers()                                                      */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Map HL04 line numbers to an internal table.                             */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   pLineNumberTable          -> to the table we're mapping from.           */
/*   CountOfLineNumberEntries  number of entries in the table.               */
/*   pTrueLineNumberEntryCount -> receiver of the actual number of entries   */
/*                               mapped.                                     */
/*   BaseOffset               base added to line number table value.         */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   pTrueLineNumberTable -> to the table we're mapping into.                */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
LINNUM_ENTRY *MapHL04LineNumbers(LINE_NUMBER_TABLE_ENTRY_HL04 *pLineNumberTable,
                                 USHORT  CountOfLineNumberEntries,
                                 USHORT *pTrueLineNumberEntryCount,
                                 ULONG   BaseOffset )
{
 int i;

 USHORT             TrueLineNumberEntryCount;
 USHORT             PreviousLineNumber;
 ULONG              TrueLineNumberTableSize;
 LINNUM_ENTRY     *pTrueLineNumberTable;

 LINE_NUMBER_TABLE_ENTRY_HL04 *pLineNumberEntry;
 LINNUM_ENTRY                 *pTrueLineNumberEntry;

 /****************************************************************************/
 /* - We will extract the line number entries from the first table for       */
 /*   the top level source file( source file index = 1).                     */
 /*                                                                          */
 /*   NOTE:                                                                  */
 /*                                                                          */
 /*   There may be multiple tables if the user has used the alloc_text       */
 /*   pragma.  There may also be multiple tables if the user has pulled in   */
 /*   code from an include file.  At this point, neither multiple code       */
 /*   segments or code in include files is supported.                        */
 /*                                                                          */
 /****************************************************************************/

 /****************************************************************************/
 /* - Scan the table and zero out the line numbers that are not for source   */
 /*   file index == 1.                                                       */
 /****************************************************************************/
 pLineNumberEntry   = pLineNumberTable;
 PreviousLineNumber = pLineNumberEntry->LineNumber;

 for( i=1; i <= CountOfLineNumberEntries; i++ )
 {
  if( pLineNumberEntry->SourceFileIndex != 1 )
   pLineNumberEntry->LineNumber = 0;
  pLineNumberEntry++;
 }

 /****************************************************************************/
 /* - sort the table precipitating zero entries.                             */
 /****************************************************************************/
 qsort( pLineNumberTable,               /* ->to line number table.           */
        CountOfLineNumberEntries,       /* number of table entries.          */
        sizeof(*pLineNumberEntry),      /* size of a single entry.           */
        compare                         /* qsort required compare function.  */
      );

 /****************************************************************************/
 /* Now, eliminate duplicate entries.                                        */
 /*                                                                          */
 /*  - scan past the 0 entries at the bottom of the table.                   */
 /*  - set duplicate entries to 0 while retaining the duplicate with the     */
 /*    lowest offset.                                                        */
 /****************************************************************************/
 pLineNumberEntry = pLineNumberTable;
 for( i=1; i <= CountOfLineNumberEntries; i++ )
 {
  if(pLineNumberEntry->LineNumber != 0)
   break;
  pLineNumberEntry++;
 }

 PreviousLineNumber = pLineNumberEntry->LineNumber;
 pLineNumberEntry++;

 for( ; i <= CountOfLineNumberEntries; i++ )
 {
  if( pLineNumberEntry->LineNumber == PreviousLineNumber )
  {
   /**************************************************************************/
   /* - count the number of duplicate entries                                */
   /**************************************************************************/
   LINE_NUMBER_TABLE_ENTRY_HL04 *pEntry;

   int NumberOfDuplicateEntries;
   int j;

   NumberOfDuplicateEntries = 1;
   pEntry                   = pLineNumberEntry;
   j                        = i;

   while( (pEntry->LineNumber == PreviousLineNumber) &&
          (j <= CountOfLineNumberEntries)
        )
   {
    NumberOfDuplicateEntries++; j++; pEntry++;
   }

   /**************************************************************************/
   /* - sort the piece of the table to preserve the offset order. this may   */
   /*   have been screwed up by the first sort.                              */
   /**************************************************************************/
   pEntry = pLineNumberEntry;
   pEntry--;
   qsort( pEntry,                         /* ->to line number table.         */
          NumberOfDuplicateEntries,       /* number of entries duplicated.   */
          sizeof(*pLineNumberEntry),      /* size of a single entry.         */
          ucompare                        /* qsort required compare function.*/
        );

   /**************************************************************************/
   /* - set duplicate entries to zero.                                       */
   /**************************************************************************/
   pEntry++;
   for( j=2; j <= NumberOfDuplicateEntries; j++, pEntry++ )
   {
    pEntry->LineNumber = 0;
   }

   /**************************************************************************/
   /* - update the loop variables.                                           */
   /**************************************************************************/
   i                += NumberOfDuplicateEntries;
   pLineNumberEntry =  pEntry;
  }
  PreviousLineNumber = pLineNumberEntry->LineNumber;
  pLineNumberEntry++;
 }

 /****************************************************************************/
 /* - now, count the number of entries that we're going to map.              */
 /****************************************************************************/
 pLineNumberEntry         = pLineNumberTable;
 PreviousLineNumber       = pLineNumberEntry->LineNumber;
 TrueLineNumberEntryCount = 0;

 for( i=1; i <= CountOfLineNumberEntries; i++ )
 {
  if( pLineNumberEntry->LineNumber != 0 )
   TrueLineNumberEntryCount++;
  pLineNumberEntry++;
 }

 /****************************************************************************/
 /* - give the caller the number of entries that will be in the table.       */
 /****************************************************************************/
 *pTrueLineNumberEntryCount = TrueLineNumberEntryCount;

 /****************************************************************************/
 /* - now, do the mapping.                                                   */
 /****************************************************************************/
 TrueLineNumberTableSize = TrueLineNumberEntryCount* sizeof(LINNUM_ENTRY);
 pTrueLineNumberTable    = Talloc( TrueLineNumberTableSize );
 pLineNumberEntry        = pLineNumberTable;
 pTrueLineNumberEntry    = pTrueLineNumberTable;

 for( i=1; i <= CountOfLineNumberEntries; i++ )
 {
  if( pLineNumberEntry->LineNumber != 0 )
  {
   pTrueLineNumberEntry->lno = pLineNumberEntry->LineNumber;
   pTrueLineNumberEntry->off = pLineNumberEntry->Offset + BaseOffset;

   pLineNumberEntry++;
   pTrueLineNumberEntry++;
  }
  else
   pLineNumberEntry++;
 }

#if 0
/*****************************************************************************/
/* - print the line number table                                             */
/*****************************************************************************/
{
 LINNUM_ENTRY *pEntry;

 pEntry = pTrueLineNumberTable;
 printf("\n\nBaseoffset=%x", BaseOffset);
 for( i = 1; i<=TrueLineNumberEntryCount; i++ )
 {
  printf("\nlno=%d, offset=%x", pEntry->lno, pEntry->off );fflush(0);
  pEntry++;
 }
}
#endif

 return(pTrueLineNumberTable);
}

/*****************************************************************************/
/* MapCV16LineNumbers()                                                      */
/*                                                                           */
/* Description:                                                              */
/*   Map a 16 bit MSC raw line number table to the internal line number      */
/*   table.                                                                  */
/*                                                                           */
/* Parameters:                                                               */
/*   RawTable       (input)  - Pointer to the raw line number table.         */
/*   InternalTable  (output) - Pointer to the internal line number table.    */
/*   TableSize      (input)  - Size of the raw line number table.            */
/*   ModSegIndex    (input)  - Segment number of the module.                 */
/*                                                                           */
/* Return:                                                                   */
/*   None.                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/* - Raw line number table has already been loaded into the memory.          */
/* - Memory for the internal line number table has already been allocated.   */
/*                                                                           */
/*****************************************************************************/
void  MapCV16LineNumbers( uint *RawTable, uint *InternalTable,
                            uint TableSize, ushort ModSegIndex )
{
  uchar  *TablePtr;
  uchar  *EndTable;
  uint    Skip;
  uint    NumLnoEntries;
  uint    Size;
  ushort  SegIndex;
  uchar   *IntTablePtr = (uchar *)InternalTable;

  /***************************************************************************/
  /* The raw line number table for MSC 16 bit module has                     */
  /* - 2 byte line number                                                    */
  /* - 2 byte offset                                                         */
  /* for each line number entry. We have to copy the line number and offset  */
  /* to our internal table which has 4 byte offset for each entry.           */
  /***************************************************************************/
  TablePtr = (uchar *)RawTable;
  EndTable = TablePtr + TableSize;

  /***************************************************************************/
  /* - Scan through the raw table and map those entries which are in the     */
  /*   module's segment to the internal line number table.                   */
  /***************************************************************************/
  while( TablePtr < EndTable )
  {
    Skip = *TablePtr + 1;
    TablePtr += Skip;

    SegIndex = *(ushort *)TablePtr;
    TablePtr += 2;

    NumLnoEntries = *(ushort *)TablePtr;
    TablePtr += 2;

    Size = NumLnoEntries * (2 * sizeof( ushort ));

    if( SegIndex == ModSegIndex )
    {
      LNOTAB16  *pLnoTable16;
      LNOTAB    *pLnoTable32;
      int        i;

      pLnoTable16 = (LNOTAB16 *)TablePtr;
      pLnoTable32 = (LNOTAB *)IntTablePtr;

      for( i = 0; i < NumLnoEntries; i++ )
      {
        pLnoTable32->lno = pLnoTable16->lno;
        pLnoTable32->off = pLnoTable16->off;
        pLnoTable32++;
        pLnoTable16++;
      }
      IntTablePtr = (uchar *)pLnoTable32;
    }
    TablePtr += Size;
  }
}

/*****************************************************************************/
/* MapT105LineNumbers()                                                      */
/*                                                                           */
/* Description:                                                              */
/*   Map a IBM C2 1.1 raw line number table to the internal line number      */
/*   table.                                                                  */
/*                                                                           */
/* Parameters:                                                               */
/*   RawTable       (input)  - Pointer to the raw line number table.         */
/*   InternalTable  (output) - Pointer to the internal line number table.    */
/*   TableSize      (input)  - Size of the raw line number table.            */
/*                                                                           */
/* Return:                                                                   */
/*   None.                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/* - Raw line number table has already been loaded into the memory.          */
/* - Memory for the internal line number table has already been allocated.   */
/*                                                                           */
/*****************************************************************************/
void  MapT105LineNumbers( uint *RawTable, uint *InternalTable,
                          uint TableSize )
{
  uchar    *TablePtr;
  uchar    *EndTable;
  uint     Skip;
  uint     NumLnoEntries;
  uint     Size;
  LNOTAB16 *pLnoTable16;
  LNOTAB   *pLnoTable32;
  int      i;
  uchar    *IntTablePtr = (uchar *)InternalTable;

  TablePtr = (uchar *)RawTable;
  EndTable = TablePtr + TableSize;

  /***************************************************************************/
  /* The raw line number table for IBM C2 1.1 module has                     */
  /* - 2 byte line number                                                    */
  /* - 2 byte offset                                                         */
  /* for each line number entry. We have to copy the line number and offset  */
  /* to our internal table which has 4 byte offset for each entry.           */
  /* - Scan through the raw table and map the entries to the internal line   */
  /*   number table.                                                         */
  /***************************************************************************/
  while( TablePtr < EndTable )
  {
    Skip = *TablePtr + 1;
    TablePtr += Skip;

    NumLnoEntries = *(ushort *)TablePtr;
    TablePtr += 2;

    Size = NumLnoEntries * (2 * sizeof( ushort ));

    pLnoTable16 = (LNOTAB16 *)TablePtr;
    pLnoTable32 = (LNOTAB *)IntTablePtr;

    for( i = 0; i < NumLnoEntries; i++ )
    {
      pLnoTable32->lno = pLnoTable16->lno;
      pLnoTable32->off = pLnoTable16->off;
      pLnoTable32++;
      pLnoTable16++;
    }
    IntTablePtr = (uchar *)pLnoTable32;
    TablePtr += Size;
  }
}

/*****************************************************************************/
/* MapCV32LineNumbers()                                                      */
/*                                                                           */
/* Description:                                                              */
/*   Map a 32 bit MSC raw line number table to the internal line number      */
/*   table.                                                                  */
/*                                                                           */
/* Parameters:                                                               */
/*   RawTable       (input)  - Pointer to the raw line number table.         */
/*   InternalTable  (output) - Pointer to the internal line number table.    */
/*   TableSize      (input)  - Size of the raw line number table.            */
/*   ModSegIndex    (input)  - Segment number of the module.                 */
/*                                                                           */
/* Return:                                                                   */
/*   None.                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/* - Raw line number table has already been loaded into the memory.          */
/* - Memory for the internal line number table has already been allocated.   */
/*                                                                           */
/*****************************************************************************/
void  MapCV32LineNumbers( uint *RawTable, uint *InternalTable,
                          uint TableSize, ushort ModSegIndex )
{
  uchar  *TablePtr;
  uchar  *EndTable;
  uint    Skip;
  uint    NumLnoEntries;
  uint    Size;
  ushort  SegIndex;

  /***************************************************************************/
  /* The raw line number table for MSC 32 bit module has                     */
  /* - 2 byte line number                                                    */
  /* - 4 byte offset                                                         */
  /* for each line number entry. We can do a memcpy for the entries since it */
  /* has the same format as our internal line number table.                  */
  /***************************************************************************/
  TablePtr = (uchar *)RawTable;
  EndTable = TablePtr + TableSize;

  /***************************************************************************/
  /* Scan through the raw line number table, memcpy the info for the entries */
  /* in the module's segment.                                                */
  /***************************************************************************/
  while( TablePtr < EndTable )
  {
    Skip = *TablePtr + 1;
    TablePtr += Skip;

    SegIndex = *(ushort *)TablePtr;
    TablePtr += 2;

    NumLnoEntries = *(ushort *)TablePtr;
    TablePtr += 2;

    Size = NumLnoEntries * (sizeof( ushort ) + sizeof( ulong ));

    if( SegIndex == ModSegIndex )
    {
      memcpy( InternalTable, TablePtr, Size );
      InternalTable += Size;
    }
    TablePtr += Size;
  }
}

/*****************************************************************************/
/* compare function for qsort                                                */
/*****************************************************************************/
int compare( const void *key, const void *element)
{
 if( *(USHORT*)key <  *(USHORT*)element) return(-1) ;
 if( *(USHORT*)key == *(USHORT*)element) return( 0) ;
 if( *(USHORT*)key >  *(USHORT*)element) return(+1) ;
}

int ucompare( const void *key, const void *element)
{
 LINE_NUMBER_TABLE_ENTRY_HL04 *pkey;
 LINE_NUMBER_TABLE_ENTRY_HL04 *pelement;

 pkey     = (LINE_NUMBER_TABLE_ENTRY_HL04 *)key;
 pelement = (LINE_NUMBER_TABLE_ENTRY_HL04 *)element;

 if( pkey->Offset <  pelement->Offset ) return(-1) ;
 if( pkey->Offset == pelement->Offset ) return( 0) ;
 if( pkey->Offset >  pelement->Offset ) return(+1) ;
}

