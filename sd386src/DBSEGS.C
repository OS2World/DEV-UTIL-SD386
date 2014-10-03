/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   dbsegs.c                                                                */
/*                                                                           */
/* Description:                                                              */
/*   Interface functions for segments.                                       */
/*                                                                           */
/*                                                                           */
/* History:                                                                  */
/*                                                                           */
/*...16->32 port.                                                            */
/*...                                                                        */
/*... 02/08/91  101   Joe       port to 32 bit.                              */
/*                                                                           */
/*...Release 1.00 (Pre-release 1)                                            */
/*...                                                                        */
/*... 07/31/91  215   Christina Add support for HLL format                   */
/*... 09/11/91  237   Srinivas  Proper initialization of debug format flags. */
/*                                                                           */
/*...Release 1.00 (Pre-release 108 12/05/91)                                 */
/*...                                                                        */
/*... 02/12/92  521   Srinivas  Port to C-Set/2.                             */
/*...                                                                        */
/*... 01/26/93  809   Selwyn    HLL Level 2 support.                         */
/*...                                                                        */
/*... 03/22/93  815   Selwyn - Workaround for compiler bug. 0:32 should be   */
/*                             16:16.                                        */
/*...                                                                        */
/*... 04/13/93  803   Selwyn    Resolving imports.                           */
/**Includes*******************************************************************/

#include "all.h"                        /* SD86 include files                */
#include "mapsyms.h"

extern CmdParms cmd;                                                    /*803*/
void PrtHLSym(UCHAR *, UCHAR *);
void PrtHLType( UCHAR *, UCHAR *);

/*****************************************************************************/
/* DBSymSeg                                                                  */
/*                                                                           */
/* Description:                                                              */
/*   Get pointer to a module's symbol area and give this area's length.      */
/*                                                                           */
/* Parameters:                                                               */
/*   mid        module id.                                                   */
/*   lenptr     points to module's symbol area length.                       */
/*   pdf        pointer to debug file structure.                             */
/*                                                                           */
/* Return:                                                                   */
/*   symptr     points to module's symbol area.                              */
/*   NULL       no pointer to symbol area.                                   */
/*                                                                           */
/*****************************************************************************/
SSRec *DBSymSeg( uint mid, ULONG *lenptr, DEBFILE *pdf )
                                        /* module id in question             */
                                        /* ->mid's symbol area length        */
                                        /* -> to debug file structure        */
{                                       /* begin DBSymSeg                    */

  SYMNODE      *sptr;                   /* to cruise through ring of pointers*/
  uchar        *RawTable;               /* -> Raw symbol table.           809*/
  uchar        *symend;                 /* ->end of symbol area unused.      */
  MODULE       *mptr;                   /* pointer to the EXE module info    */
  ulong         seekpos;                /* position in file to seek          */
  SSVar        *sp;                     /* -> static symbol record.          */
  uchar         SymbolType, SymbolFlag;                                 /*809*/
  int           IntTableSize;           /* Internal symbol table size.    809*/
  SSRec        *IntSymTable;            /* -> internal symbol table.      809*/
  uchar        *symptr, *EndPtr;                                        /*809*/
  ushort        RecordLength = 0;                                       /*809*/
  ushort        Adjust = 0;                                             /*809*/
  uchar         RecordType;                                             /*809*/

  if( (mid == 0) || (pdf == NULL) )
   return( NULL );

  /***************************************************************************/
  /* Scan through the symbol table linked list to see if the symbol table of */
  /* the module has already been loaded. If so return the pointer to the     */
  /* symbol table.                                                           */
  /***************************************************************************/
  for( sptr = pdf->psyms; sptr != NULL; sptr = sptr->next )
  {
    if( sptr->mid == mid )
    {
      *lenptr = sptr->symlen;
      return( sptr->symptr );
    }
  }

  /***************************************************************************/
  /* If the symbol table has not been loaded, check to see whether a module  */
  /* entry is present and the module has a symbol table. If any one of the   */
  /* above condition fails, return a NULL symbol pointer and a zero length.  */
  /***************************************************************************/
  symptr = NULL;                                                        /*809*/
  *lenptr = 0;                                                          /*809*/

  mptr = pdf->MidAnchor;                                                /*809*/
  for( ; mptr && mptr->mid != mid; mptr = mptr->NextMod ){;}            /*809*/

  if( !mptr || !mptr->Symbols )                                         /*809*/
    {return( NULL );}                                                   /*809*/

  SymbolFlag = mptr->DbgFormatFlags.Syms;                               /*809*/

  /***************************************************************************/
  /* - Allocate memory for the raw symbol table.                             */
  /* - Read the raw symbol table for the module into the buffer.             */
  /***************************************************************************/
  RawTable = Talloc( mptr->SymLen );                                    /*809*/
  seekpos = pdf->DebugOff + mptr->Symbols;                              /*809*/
  seekf( pdf, seekpos );                                                /*809*/
  readf( RawTable, mptr->SymLen, pdf );                                 /*809*/

  /***************************************************************************/
  /* We have to calculate the size of the internal symbol table. For this we */
  /* scan through the raw symbol table, add allowance for each type of symbol*/
  /* record  to calculate the total size of the internal symbol table.       */
  /***************************************************************************/
  IntTableSize = 0;                                                     /*809*/
  symptr = RawTable;                                                    /*809*/
  EndPtr = symptr + mptr->SymLen;                                       /*809*/
  /***************************************************************************/
  /* Scan through the raw table.                                             */
  /***************************************************************************/
  for( ; symptr < EndPtr; symptr += (RecordLength + Adjust) )           /*809*/
  {                                                                     /*809*/
    /*************************************************************************/
    /* - Calculate the record length (Encoded incase of HLL Level 3).        */
    /* - Get the type of the symbol record.                                  */
    /* - Set Adjust so that is skips the length field. ( 2 incase of Level 3 */
    /*   encoded length or else it is 1 ).                                   */
    /*************************************************************************/
    RecordLength = GetRecordLength( symptr, mptr );                     /*809*/
    RecordType   = GetRecordType( symptr, mptr );                       /*809*/

    Adjust = 1;
    if( TestBit( *symptr, 7 ) && ((SymbolFlag == TYPE104_HL03) ||
                                  (SymbolFlag == TYPE104_HL04)
                                 )
      )
      Adjust = 2;                                                       /*809*/

    switch( RecordType )                                                /*809*/
    {                                                                   /*809*/
      /***********************************************************************/
      /* No name field is involved for begin records, so the size of the     */
      /* begin records are constant.                                         */
      /***********************************************************************/
      case SSBEGIN:                                                     /*809*/
      case SSBEGIN32:                                                   /*809*/
        IntTableSize += sizeof( SSBegin );                              /*809*/
        break;                                                          /*809*/

      case SSPROC:                                                      /*809*/
      case SSENTRY:                                                     /*809*/
      case SSPROC32:                                                    /*809*/
      case SSMEMFUNC:                                                   /*809*/
      case SSPROCCPP:                                                   /*809*/
      {                                                                 /*809*/
        switch( SymbolFlag )                                            /*809*/
        {                                                               /*809*/
          case TYPE104_C211:                                            /*809*/
          case TYPE104_C600:                                            /*809*/
            /*****************************************************************/
            /* Difference in 16 bit MSC 6.0/C211 proc record  and internal   */
            /* symbol record lengths                                         */
            /*   - Record length        2                                    */
            /*   - Flags                1                                    */
            /*   - Proc offset          2                                    */
            /*   - Proc length          2                                    */
            /*   - Debug end            2                                    */
            /*   - Name length          1                                    */
            /*        Total  ====>     10                                    */
            /*****************************************************************/
            IntTableSize += (RecordLength + 10);                        /*809*/
            break;                                                      /*809*/

          case TYPE104_CL386:                                           /*809*/
            /*****************************************************************/
            /* Difference in 32 bit CL386 proc record  and internal symbol   */
            /* record lengths                                                */
            /*   - Record length        2                                    */
            /*   - Flags                1                                    */
            /*   - Proc length          2                                    */
            /*   - Debug end            2                                    */
            /*   - Name length          1                                    */
            /*        Total  ====>      8                                    */
            /*****************************************************************/
            IntTableSize += (RecordLength + 8);                         /*809*/
            break;                                                      /*809*/

          case TYPE104_HL01:                                            /*809*/
          case TYPE104_HL02:                                            /*809*/
          case TYPE104_HL03:                                            /*809*/
          case TYPE104_HL04:                                            /*809*/
            /*****************************************************************/
            /* Difference in HLL proc record and internal symbol record      */
            /* lengths                                                       */
            /*   - Record length        2                                    */
            /*   - Flags                1                                    */
            /*   - Name length          1                                    */
            /*        Total  ====>      4                                    */
            /*****************************************************************/
            IntTableSize += (RecordLength + 4);                         /*809*/
            break;                                                      /*809*/
        }                                                               /*809*/
        break;                                                          /*809*/
      }                                                                 /*809*/

      case SSDEF:                                                       /*809*/
      case SSDEF32:                                                     /*809*/
      {                                                                 /*809*/
        switch( SymbolFlag )                                            /*809*/
        {                                                               /*809*/
          case TYPE104_C211:                                            /*809*/
          case TYPE104_C600:                                            /*809*/
            /*****************************************************************/
            /* Difference in 16 bit MSC 6.0/C211 def  record  and internal   */
            /* symbol record lengths                                         */
            /*   - Record length        2                                    */
            /*   - Frame offset         2                                    */
            /*   - Name length          1                                    */
            /*        Total  ====>      5                                    */
            /*****************************************************************/
            IntTableSize += (RecordLength + 5);                         /*809*/
            break;                                                      /*809*/

          case TYPE104_CL386:                                           /*809*/
          case TYPE104_HL01:                                            /*809*/
          case TYPE104_HL02:                                            /*809*/
          case TYPE104_HL03:                                            /*809*/
          case TYPE104_HL04:                                            /*809*/
            /*****************************************************************/
            /* Difference in 32 bit CL386/HLL def record and internal symbol */
            /* record lengths                                                */
            /*   - Record length        2                                    */
            /*   - Name length          1                                    */
            /*        Total  ====>      3                                    */
            /*****************************************************************/
            IntTableSize += (RecordLength + 3);                         /*809*/
            break;                                                      /*809*/
        }                                                               /*809*/
        break;                                                          /*809*/
      }                                                                 /*809*/

      case SSVAR:                                                       /*809*/
      case SSVAR32:                                                     /*809*/
      case SSVARCPP:                                                    /*809*/
      {                                                                 /*809*/
        switch( SymbolFlag )                                            /*809*/
        {                                                               /*809*/
          case TYPE104_C211:                                            /*809*/
          case TYPE104_C600:                                            /*809*/
            /*****************************************************************/
            /* Difference in 16 bit MSC 6.0/C211 var  record  and internal   */
            /* symbol record lengths                                         */
            /*   - Record length        2                                    */
            /*   - Flags                1                                    */
            /*   - Offset               2                                    */
            /*   - Name length          1                                    */
            /*        Total  ====>      6                                    */
            /*****************************************************************/
            IntTableSize += (RecordLength + 6);                         /*809*/
            break;                                                      /*809*/

          case TYPE104_CL386:                                           /*809*/
          case TYPE104_HL01:                                            /*809*/
          case TYPE104_HL02:                                            /*809*/
          case TYPE104_HL03:                                            /*809*/
          case TYPE104_HL04:                                            /*809*/
            /*****************************************************************/
            /* Difference in 32 bit CL386/HLL var record and internal symbol */
            /* record lengths                                                */
            /*   - Record length        2                                    */
            /*   - Flags                1                                    */
            /*   - Name length          1                                    */
            /*        Total  ====>      4                                    */
            /*****************************************************************/
            IntTableSize += (RecordLength + 4);                         /*809*/
            break;                                                      /*809*/
        }                                                               /*809*/
        break;                                                          /*809*/
      }                                                                 /*809*/

      case SSEND:                                                       /*809*/
      case SSEND32:                                                     /*809*/
      /***********************************************************************/
      /* No name field is involved for end records, so the size of the end   */
      /* records are constant.                                               */
      /***********************************************************************/
        IntTableSize += sizeof( SSEnd );                                /*809*/
        break;                                                          /*809*/

      case SSREG:                                                       /*809*/
      case SSREG32:                                                     /*809*/
      /***********************************************************************/
      /* The size of the reg records are the same for all formats, so the    */
      /* difference in lengths is                                            */
      /*   - Record length        2                                          */
      /*   - Name length          1                                          */
      /*        Total  ====>      3                                          */
      /***********************************************************************/
        IntTableSize += (RecordLength + 3);                             /*809*/
        break;                                                          /*809*/

      case SSCHGDEF:                                                    /*809*/
      /***********************************************************************/
      /* ChgDefSeg records are supported in HL01 & HL02 formats, and since   */
      /* no name field is involved the length is constant.                   */
      /***********************************************************************/
        IntTableSize += sizeof( SSChgDef );                             /*809*/
        break;                                                          /*809*/

      case SSUSERDEF:                                                   /*809*/
      /***********************************************************************/
      /* The size of the User def records are the same for all formats, so   */
      /* the difference in lengths is                                        */
      /*   - Record length        2                                          */
      /*   - Name length          1                                          */
      /*        Total  ====>      3                                          */
      /***********************************************************************/
        IntTableSize += (RecordLength + 3);                             /*809*/
        break;                                                          /*809*/

      default:                                                          /*809*/
      /***********************************************************************/
      /* For the record types which are supported by SD386, no entries are   */
      /* present in the internal table, so blow past those records.          */
      /***********************************************************************/
        break;                                                          /*809*/
    }                                                                   /*809*/
  }                                                                     /*809*/

  /***************************************************************************/
  /* If there was no symbol info, then return a NULL pointer.                */
  /***************************************************************************/
  if( !IntTableSize )                                                   /*809*/
    return( NULL );                                                     /*809*/

  /***************************************************************************/
  /* - Allocate memory for the internal symbol table.                        */
  /* - Depending on the the symbol type call the respective mapping function */
  /*   to map the raw symbol table to our internal symbol table.             */
  /* - Free the memory allocated for the raw table.                          */
  /***************************************************************************/
  IntSymTable = Talloc( IntTableSize );                                 /*809*/
  switch( SymbolFlag )                                                  /*809*/
  {                                                                     /*809*/
    case TYPE104_C211:                                                  /*809*/
    case TYPE104_C600:                                                  /*809*/
      MapMS16Syms( RawTable, IntSymTable, mptr->SymLen );               /*809*/
      break;                                                            /*809*/

    case TYPE104_CL386:                                                 /*809*/
      MapMS32Syms( RawTable, IntSymTable, mptr->SymLen );               /*809*/
      break;                                                            /*809*/

    case TYPE104_HL01:                                                  /*809*/
    case TYPE104_HL02:                                                  /*809*/
    case TYPE104_HL03:                                                  /*809*/
    case TYPE104_HL04:
      MapHLLSyms( RawTable, IntSymTable, mptr );                        /*809*/
      break;                                                            /*809*/
  }                                                                     /*809*/
  Tfree( RawTable );                                                     /*809*/

#if 0
  PrtHLSym( (UCHAR *)IntSymTable, ((UCHAR *)IntSymTable + IntTableSize) );
#endif

  mptr->SymLen = IntTableSize;                                          /*809*/
  if( SymbolFlag == TYPE104_C211 || SymbolFlag == TYPE104_C600 )        /*809*/
    SymbolType = BIT16;                                                 /*809*/
  else                                                                  /*809*/
    SymbolType = BIT32;                                                 /*809*/

  /***************************************************************************/
  /* Add the symbol table to the linked list of symbol tables.               */
  /***************************************************************************/
  sptr = Talloc( sizeof( SYMNODE ) );   /* grab heap space for SYMNODE    521*/
  sptr->next = pdf->psyms;              /* link in newest structure          */
  sptr->mid = mid;                      /* put in mid stamp                  */
  sptr->symptr = IntSymTable;           /* set up pointer to symbols area    */
  sptr->symlen = mptr->SymLen;          /* put in symbol area length         */
  *lenptr = mptr->SymLen;               /* set symbol area length for caller */
  pdf->psyms=sptr;                      /* reset root ptr to top of chain    */

  /***************************************************************************/
  /* At this point, the new node has been built. sptr points to the new node.*/
  /* We want to convert segment numbers to selectors for all SSVar records.  */
  /* If the SSVar record has a 0 segment number, then the variable is an     */
  /* import.                                                                 */
  /***************************************************************************/
  for( symend = (uchar *)IntSymTable + sptr->symlen;
       (uchar *)IntSymTable < symend;
       IntSymTable = NextSSrec(IntSymTable) )
  {
   switch( IntSymTable->RecType )       /* we are only interested in the     */
   {                                    /* SSvar records.                    */
    case SSVAR:                         /*                                   */
                                        /*                                   */
     sp = (SSVar * )IntSymTable;        /* define a static symbol rec ptr.   */
                                        /*                                   */
     if( sp->ObjectNum )                /* if this is an import,then         */
     {                                  /* if statically resolved at         */
       sp->Offset += GetLoadAddr(pdf->mte,sp->ObjectNum);               /*822*/
     }                                  /* the symbol record.                */
     else
     /************************************************************************/
     /* If the Object number is zero, it could be an imported variable. So   */
     /* resolve the import only if the user had invoked with /m option.      */
     /************************************************************************/
     if( cmd.ResolveImports == TRUE )                                   /*803*/
     {
       uchar *SymbolName = Talloc( strlen( sp->Name ) + 2 );            /*809*/

       sprintf( SymbolName, "%c%s", sp->NameLen, sp->Name );            /*809*/
       if( SymbolType == BIT32 )
       {                                                                /*815*/
         uchar  ExpModuleType;                                          /*815*/

         sp->Offset = ResolveImport32( SymbolName, pdf,                 /*815*/
                                       &ExpModuleType );                /*815*/
         if( ExpModuleType == BIT16 )                                   /*815*/
         {                                                              /*815*/
           if( (sp->TypeIndex & 0xF0) == 0xA0 || sp->TypeIndex == 0xB7 )/*815*/
           {                                                            /*815*/
             sp->TypeIndex &= 0x9F;                                     /*815*/
             sp->TypeIndex |= 0x40;                                     /*815*/
           }                                                            /*815*/
         }                                                              /*815*/
       }                                                                /*815*/
       else
         sp->Offset = ResolveImport( SymbolName, pdf );
       Tfree( SymbolName );                                              /*809*/
     }                                  /*                                   */
     break;                             /*                                   */
/*****************************************************************************/
/*                                                                           */
/*****************************************************************************/
    default:                            /* let all other records pass thro   */
     break;                             /*                                   */
   }                                    /* end of switch                     */
  }                                     /* end of SSvar processing           */
  return ( sptr->symptr );              /* return pointer to symbol area     */
}                                       /* end DBSymSeg                      */
/*****************************************************************************/
/* DBTypSeg                                                                  */
/*                                                                           */
/* Description:                                                              */
/*   get pointer and length for module's type area. Build type area linked   */
/*   list if not already built.                                              */
/*                                                                           */
/* Parameters:                                                               */
/*   mid        module id.                                                   */
/*   lenptr     points to module's type area length.                         */
/*   pdf        pointer to debug file structure.                             */
/*                                                                           */
/* Return:                                                                   */
/*   typptr     points to module's type area.                                */
/*                                                                           */
/*****************************************************************************/


 uchar *
DBTypSeg(uint mid,uint *lenptr,DEBFILE *pdf)
{                                       /* begin DBTypSeg                    */
  TYPENODE *tptr;                       /* type info node pointer            */


  for(
      tptr = pdf->ptyps;                /* point to start of typ area ring   */
      tptr != NULL;                     /* while ring members exist...       */
      tptr = tptr->next                 /*   ...traverse typ area ring       */
     )
   {                                    /* begin scan of type info ring      */
     if ( tptr->mid == mid )            /* find mid in existing ring ?       */
     {                                  /* then we are done searching        */
       *lenptr = tptr->typlen;          /* set # of bytes in type area       */
       return ( tptr->typptr );         /*   and return ptr to type area     */
     }                                  /* end we are done searching         */
   }                                    /* end scan of module list           */

/*****************************************************************************/
/* If we get here, then the type info does not exist for the mid specified.  */
/* We need to cruise thru the module ring to find the mid's type area        */
/* pointer in the debug file. Then we can allocate space for the types and   */
/* save for future use.                                                      */
/*****************************************************************************/
  {
   MODULE   *mptr;                      /* pointer to the module info        */
   uchar    *areaptr = NULL;            /* pointer to raw type data          */

   *lenptr = 0;                         /*   mid does not have type area     */

   for(
        mptr = pdf->MidAnchor;          /* point to start of Mid list        */
        mptr != NULL;                   /* scan to the end of the list       */
        mptr = mptr->NextMod            /* next module                       */
      )
    {                                   /* begin scan of module list         */
      if ( mptr->mid == mid )           /* found the mid specified ?         */
      {                                 /* yes, we found the module id       */
        if ( mptr->TypeDefs )           /* does module have a type info ?    */
        {                               /* module does have a type info      */
          areaptr=Talloc(mptr->TypeLen);/* grab heap space.               521*/

          seekf( pdf,                   /* seek the type info                */
                 pdf->DebugOff +        /*                                   */
                 mptr->TypeDefs);       /*                                   */

          readf( areaptr,               /* read the type info                */
                 mptr->TypeLen,         /*                                   */
                 pdf);
                   /* if HLL format, map type records to MS 32 bit format 215*/
          switch( mptr->DbgFormatFlags.Typs )
          {
            case TYPE103_HL01:
            case TYPE103_HL02:
            case TYPE103_HL03:
            case TYPE103_HL04:
              areaptr = MapHLLTypes(areaptr,mptr);
              break;

            case TYPE103_C600:
            case TYPE103_C211:
            case TYPE103_CL386:
              areaptr = MapMSTypes(areaptr,mptr);
#if 0
              PrtHLType( areaptr, areaptr + mptr->TypeLen);
#endif
              break;
          }
        }
        break;                          /* we are done searching for the mid */
      }                                 /* end found the mid specified       */
    }                                   /* end scan of module list           */

   if ( areaptr )                       /* mid and type info found ?         */
    {                                   /* set up a new type node in list    */

      tptr = Talloc(sizeof(TYPENODE));  /*grab heap space for the node    521*/
      tptr->next = pdf->ptyps;          /* link it into the ring             */
      tptr->mid = mid;                  /* put in mid stamp                  */
      tptr->typptr = ( areaptr );       /* set up pointer to raw type data   */
      tptr->typlen = mptr->TypeLen;     /* put in length of raw type data    */
      *lenptr = mptr->TypeLen;          /* set type   area length for caller */
      pdf->ptyps = tptr;                /* reset root pointer to top of chain*/
      return ( tptr->typptr );          /* ->type   area in heap for caller  */
    }                                   /* end set up a new type node        */
   return ( NULL );                     /* mid or type info not found        */
  }                                     /* end of build type ring            */
}                                       /* end DBTypSeg                      */

/*****************************************************************************/
/* DBPubSeg                                                                  */
/*                                                                           */
/* Description:                                                              */
/*   get pointer and length for module's Public area. Build public area      */
/*   linked list if not already built.                                       */
/*                                                                           */
/* Parameters:                                                               */
/*   mid        module id.                                                   */
/*   lenptr     points to module's public area length.                       */
/*   pdf        pointer to debug file structure.                             */
/*                                                                           */
/* Return:                                                                   */
/*   pubptr     points to module's public area.                              */
/*                                                                           */
/* Assumptions;                                                              */
/*                                                                           */
/*  none.                                                                    */
/*****************************************************************************/
 uchar *
DBPubSeg( uint mid, uint *lenptr , DEBFILE *pdf)
{
 PUBNODE      *pptr;
 MODULE       *mptr;
 uchar        *areaptr = NULL;
 PUBREC32     *precptr;
 PUBREC16     *precptr16;
 uchar        *pubend;
/*****************************************************************************/
/*                                                                           */
/* If we get here, then the pub  info does not exist for the mid specified.  */
/* We need to cruise thru the module ring to find the mid's pub  area        */
/* pointer in the .EXE file.  Then we can allocate space for the pub   and   */
/* put the .EXE information in heap storage for future calls. If we are      */
/* debugging without debug info , no /CO linker option, there will not be    */
/* any public info.                                                          */
/*                                                                           */
/* 1. First, test for a valid mid.                                           */
/* 2. Get a ptr to the public segment in the ring of currently loaded        */
/*    public segments and return it.                                         */
/* 3. If the publics are not loaded yet, then load them.                     */
/* 4. Add them to the ring of public segments.                               */
/* 5. Convert public record objnums:offsets to flat addresses.               */
/* 6. Return the pointer to the public segment.                              */
/*                                                                           */
/*****************************************************************************/

 if ( !mid )
  return(NULL);
 *lenptr = 0;
 for( pptr = pdf->ppubs; pptr != NULL; pptr=pptr->next )
 {
  if ( pptr->mid == mid )
  {
   *lenptr = pptr->publen;
   return ( pptr->pubptr );
  }
 }

 /****************************************************************************/
 /* publics not loaded so load them.                                         */
 /****************************************************************************/
 for( mptr = pdf->MidAnchor; mptr != NULL; mptr = mptr->NextMod )
 {
  if ( mptr->mid == mid )
  {
   if ( mptr->Publics )
   {
    areaptr=Talloc(mptr->PubLen);                                       /*521*/
    seekf( pdf, pdf->DebugOff + mptr->Publics);
    readf( areaptr, mptr->PubLen, pdf);
    break;
   }
   return( NULL );
  }
 }

 /****************************************************************************/
 /* add the public segment read to the ring of public segments.              */
 /****************************************************************************/
 if ( areaptr )
 {
  pptr = Talloc(sizeof(PUBNODE));                                       /*521*/
  pptr->next = pdf->ppubs;
  pptr->mid = mid;
  pptr->pubptr = ( areaptr );
  pptr->publen = mptr->PubLen;
  *lenptr = mptr->PubLen;
  pdf->ppubs = pptr;
 }

 /****************************************************************************/
 /* scan public records and convert obj:offsets to flat addresses.           */
 /****************************************************************************/
 precptr = ( PUBREC32 *)areaptr;
 precptr16 = ( PUBREC16 *)areaptr;

 switch( mptr->DbgFormatFlags.Pubs )
 {
  case TYPE_PUB_32:
   if ( precptr != NULL )
   {
    pubend = (uchar *)precptr + pptr->publen;
    for( ; ( uchar *)precptr < pubend; )
    {
     precptr->Offset += GetLoadAddr(pdf->mte,precptr->ObjNum);         /*822*/
     precptr = (PUBREC32 *)NextPubRec32(precptr);
    }
   }
   break;

  case TYPE_PUB_16:                                                     /*237*/
   if ( precptr16 != NULL)
   {
    pubend = (uchar *)precptr16 + pptr->publen;
    for( ; ( uchar *)precptr16 < pubend; )
    {

     int loadaddr;                                                     /*822*/
                                                                       /*822*/
     loadaddr=GetLoadAddr(pdf->mte,                                    /*822*/
                          precptr16->Pub16Addr.RawAddr.ObjNum);        /*822*/
                                                                       /*822*/
     precptr16->Pub16Addr.FlatAddr = loadaddr +                        /*822*/
                                   precptr16->Pub16Addr.RawAddr.Offset;/*822*/

     precptr16 = (PUBREC16 *)NextPubRec16(precptr16);
    }
   }
   break;
 }
 return ( pptr->pubptr );
}                                       /* end DBPubSeg                      */

