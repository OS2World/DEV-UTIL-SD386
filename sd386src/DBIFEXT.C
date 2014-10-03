/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   dbifext.c                                                               */
/*                                                                           */
/* Description:                                                              */
/*   More interface functions.                                               */
/*                                                                           */
/*                                                                           */
/* History:                                                                  */
/*                                                                           */
/*   02/08/91 Creation of 32-bit SD86, from 16-bit version.                  */
/*                                                                           */
/*...16->32 port.                                                            */
/*...                                                                        */
/*... 02/08/91  101   Joe       made changes for 32-bit compilation.         */
/*... 05/20/91  106   Srinivas  hooking up the assembler window.             */
/*... 05/29/91  105   Christina error in processing MODULE chain             */
/*... 05/30/91  110   Srinivas  changes to make restart work.                */
/*... 06/02/91  111   Christina fix warnings                                 */
/*...                                                                        */
/*...Release 1.00 (Pre-release 1)                                            */
/*...                                                                        */
/*... 08/22/91  234   Joe       PL/X gives "varname" is incorrect message    */
/*...                           when entering a parameter name in the data   */
/*...                           window.  This happens when the cursor is on  */
/*...                           an internal procedure definition statement   */
/*...                           and you use F2 to get into the data window   */
/*...                           and then type the name.                      */
/*... 08/30/91  235   Joe       Cleanup/rewrite ascroll() to fix several bugs*/
/*...                                                                        */
/*...Release 1.00 (After pre-release 1.08)                                   */
/*...                                                                        */
/*... 02/12/92  521   Joe       Port to C-Set/2.                             */
/*... 02/14/92  530   Srinivas  Jumping across ID in case of PLX disassembly */
/*... 02/17/92  533   Srinivas  Trap when we type in a variable in data      */
/*...                           window, when we don't have debug info.       */
/*...                                                                        */
/*...Release 1.00 (03/03/92)                                                 */
/*...                                                                        */
/*... 03/18/92  606   Srinivas  Handle multiple segment numbers in lno table */
/*...                           due to alloc_text pragma.                    */
/*...Release 1.01 (03/31/92)                                                 */
/*...                                                                        */
/*... 05/04/92  700   Joe       Fix code/data object bitness.                */
/*... 06/04/93  827   Joe       Add remote debug support.                    */
/*... 02/03/93  905   Joe       Fix Data window update bug.                  */
/*...                                                                        */
/**Includes*******************************************************************/

#include "all.h"                        /* SD86 include files                */

/**Externs********************************************************************/

extern AFILE        *allfps;
extern PROCESS_NODE *pnode;                                             /*827*/


/*****************************************************************************/
/*  DBFindPdf()                                                              */
/*                                                                           */
/* Description:                                                              */
/*   Find the pointer to the debug file containing the mid parmater.         */
/*                                                                           */
/* Parameters:                                                               */
/*   mid        module id in the debug file.                                 */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   pdf        pointer to debug file structure.                             */
/*                                                                           */
/*****************************************************************************/
  DEBFILE *                             /*                                   */
DBFindPdf( uint mid  )                  /*                                   */
{                                       /* begin  DBPub                      */
 DEBFILE      *pdf;                     /* executing debug file              */
 MODULE       *mptr;                    /* -> to a module node               */
                                        /*                                   */
 for( pdf = pnode->ExeStruct;           /* scan all EXE/DLL nodes.           */
      pdf;                              /*                                   */
      pdf=pdf->next                     /*                                   */
    )                                   /*                                   */
 {                                      /*                                   */
  for(                                  /* scan modules                      */
      mptr = pdf->MidAnchor;            /*                                   */
      mptr != NULL;                     /*                                   */
      mptr = mptr->NextMod              /*                                   */
     )                                  /*                                   */
  {                                     /*                                   */
   if(mptr->mid == mid)                 /* if found, break from inner loop   */
    break;                              /*                                   */
  }                                     /*                                   */
  if (mptr != NULL)                     /* 105 */
    if (mptr->mid == mid)               /* if found, break from outer loop   */
      break;                            /*                                   */
 }                                      /*                                   */
 return(pdf);                           /*                                   */
}                                       /* end DBFindPdf()                   */
/*************************************************************************101*/
/* FindExeOrDllWithAddr()                                                 101*/
/*                                                                        101*/
/* Description:                                                           101*/
/*   Find the pointer to the debug file containing this address.          101*/
/*                                                                        101*/
/* Parameters:                                                            101*/
/*   addr       address.                                                  101*/
/*                                                                        101*/
/* Return:                                                                101*/
/*   pdf        pointer to debug file structure.                          101*/
/*   NULL       did not locate the address.                               101*/
/*                                                                        101*/
/*************************************************************************101*/
                                        /*                                101*/
    DEBFILE *                           /*                                101*/
FindExeOrDllWithAddr( ULONG addr )      /*                                101*/
{                                       /*                                101*/
 DEBFILE       *pdf;                    /* executing debug file           101*/
 OBJTABLEENTRY *te;                     /* -> to a object table entry.    101*/
 uint           NumCodeObjs;            /* number of table entries.       521*/
 int            i;                      /*                                101*/
 uint          *p;                      /*                                521*/

 if( addr == 0 )
  return(NULL);

/*************************************************************************101*/
/* The purpose of the following code is to scan the modules in the debug  101*/
/* files and find the debug file that contains the address. This function 101*/
/* used for a process with focus. There is a FindPdfAddr that is used     101*/
/* when the process does not have focus.                                  101*/
/*                                                                        101*/
/* - get a pointer to the process node.                                   101*/
/* - scan pdf object table for the object containing this addr.           101*/
/* - return the pdf.                                                      101*/
/*                                                                        101*/
/*************************************************************************101*/
 for( pdf = pnode->ExeStruct;                                           /*101*/
      pdf;                                                              /*101*/
      pdf=pdf->next                                                     /*101*/
    )                                                                   /*101*/
 {                                                                      /*101*/
/*DumpObjectTable( pdf );*/
  p=pdf->CodeObjs;                                                      /*521*/
  NumCodeObjs = *p;                                                     /*521*/
  te = (OBJTABLEENTRY *)++p;                                            /*101*/
  for(i=1; i <= NumCodeObjs; i++,te++ )                                 /*101*/
  {                                                                     /*101*/
   if((addr >= te->ObjLoadAddr) &&                                      /*101*/
      ( addr <  (te->ObjLoadAddr + te->ObjLoadSize)))                   /*101*/
    return(pdf);                                                        /*101*/
  }                                                                     /*101*/
 }                                                                      /*101*/
 return(NULL);                          /*                              /*101*/
}                                       /* end FindExeOrDllWithAddr().  /*101*/

/*****************************************************************************/
/* FindExeOrDllWithSelOff()                                                  */
/*                                                                           */
/* Description:                                                              */
/*   Find the pointer to the debug file containing this sel:off.             */
/*                                                                           */
/* Parameters:                                                               */
/*   sel        16 bit selector.                                             */
/*   off        16 bit offset.                                               */
/*                                                                           */
/* Return:                                                                   */
/*   pdf        pointer to debug file structure.                             */
/*   NULL       did not locate the sel:off.                                  */
/*                                                                           */
/*****************************************************************************/
DEBFILE *FindExeOrDllWithSelOff( USHORT sel, USHORT off )
{
 DEBFILE       *pdf;
 OBJTABLEENTRY *te;
 uint           NumObjs;
 int            i;
 uint          *p;

 for( pdf = pnode->ExeStruct;
      pdf;
      pdf=pdf->next
    )
 {
  p=pdf->CodeObjs;
  NumObjs = *p;
  te = (OBJTABLEENTRY *)++p;
  for(i=1; i <= NumObjs; i++,te++ )
  {
   if( (sel == te->ObjLoadSel ) )
   {
    USHORT LoOffset;
    USHORT HiOffset;

    LoOffset = te->ObjLoadOff;
    HiOffset = LoOffset + te->ObjLoadSize - 1;
    if( (LoOffset <= off) && (off <= HiOffset) )
     return(pdf);
   }
  }
 }
 return(NULL);
}

/*****************************************************************************/
/* dfilefree()                                                                 */
/*                                                                           */
/* Description:                                                              */
/*   free the DFILE structures for a restart.                                */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*  Will be called only on a Restart().                                      */
/*                                                                           */
/*****************************************************************************/
extern DFILE*      DataFileQ;

void dfilefree()
{
  DFILE *dfp;
  DFILE *dfpnext;
  DFILE *dfplast;

  /***************************************************************************/
  /* - reset the pointers into the symbols areas.                            */
  /***************************************************************************/
  dfplast=(DFILE*)&DataFileQ;
  dfp=DataFileQ;
  for( dfp=DataFileQ ; dfp; dfp=dfp->next )
  {
   dfp->scope = NULL;
  }

  dfplast=(DFILE*)&DataFileQ;
  dfp=DataFileQ;
  while( dfp )
  {
   dfpnext=dfp->next;
   if( dfp->mid > MAXEXEMID )
   {
    Tfree((void*)dfp);                                                   /*521*/
    dfplast->next=dfpnext;
    dfp=dfpnext;
   }
   else
   {
    dfplast=dfp;
    dfp=dfpnext;
   }
  }
}

/*****************************************************************************/
/* afilefree()                                                                 */
/*                                                                           */
/* Description:                                                              */
/*   free the afile structures for a restart.                                */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*  Will be called only on a Restart().                                      */
/*                                                                           */
/*****************************************************************************/
void afilefree()
{
 AFILE        *fp;
 AFILE        *fpnext;

 fp=allfps;

 while( fp )                            /* scan afile linked list and        */
 {
  fpnext=fp->next;
  freefp(fp);                           /* free each structure               */
  fp=fpnext;
 }
 allfps=NULL;
}

/*****************************************************************************/
/* freepdf()                                                                 */
/*                                                                           */
/* Description:                                                              */
/*   free up the memory allocated for the debug info associated with a DLL.  */
/*                                                                           */
/* Parameters:                                                               */
/*   pdf        debug file.                                                  */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*  The pdf is valid.                                                        */
/*                                                                           */
/*****************************************************************************/
void freepdf(DEBFILE *pdf)
{
 MODULE       *pModule;
 MODULE       *pModuleNext;
 AFILE        *fp;
 AFILE        *fpnext;
 FILENAME     *pFile;
 FILENAME     *pFileNext;
 CSECT        *pCsect;
 CSECT        *pCsectNext;

 /****************************************************************************/
 /* - scan the modules and free the line number tables.                      */
 /****************************************************************************/
 for( pModule = pdf->MidAnchor; pModule != NULL ; pModule=pModule->NextMod)
 {
  for(pCsect = pModule->pCsects; pCsect != NULL; pCsect=pCsect->next )
  {
   if(pCsect->pLnoTab)
    Tfree(pCsect->pLnoTab);
  }
 }

 /****************************************************************************/
 /* - now, scan the modules and free the csects.                             */
 /****************************************************************************/
 for( pModule = pdf->MidAnchor; pModule != NULL ; pModule=pModule->NextMod)
 {
  for(pCsect = pModule->pCsects; pCsect != NULL; )
  {
   pCsectNext = pCsect->next;
   Tfree(pCsect);
   pCsect=pCsectNext;
  }
 }

 /****************************************************************************/
 /* - now, scan the modules and free the files.                              */
 /****************************************************************************/
 for( pModule = pdf->MidAnchor; pModule != NULL ; pModule=pModule->NextMod)
 {
  for(pFile = pModule->pFiles; pFile != NULL; )
  {
   pFileNext = pFile->next;
   Tfree(pFile);
   pFile=pFileNext;
  }
 }

 /****************************************************************************/
 /* - now, free the modules.                                                 */
 /****************************************************************************/
 for( pModule = pdf->MidAnchor; pModule != NULL ; )
 {
  pModuleNext = pModule->NextMod;
  Tfree(pModule);
  pModule=pModuleNext;
 }

/*****************************************************************************/
/* Free symbols,types,publics, and line numbers.                             */
/*****************************************************************************/
  FreeSyms(pdf);
  FreeTyps(pdf);
  FreePubs(pdf);

/*****************************************************************************/
/* Close the source file and free the MFILE structure and file name          */
/* storage.                                                                  */
/*****************************************************************************/
 if( pdf->DebFilePtr->fh != 0 )
   closedos(pdf->DebFilePtr->fh);
 Tfree((void*)pdf->DebFilePtr->fn);      /* free file name space           521*/
 Tfree((void*)pdf->DebFilePtr);          /* free the MFILE structure       521*/

/*************************************************************************813*/
/* Free the object tables.                                                813*/
/*************************************************************************813*/
 if(pdf->CodeObjs)
  Tfree(pdf->CodeObjs);

 /****************************************************************************/
 /* Now, free any views that may have been built for this pdf.               */
 /****************************************************************************/
 for( fp = allfps; fp; )
 {
  fpnext = fp->next;
  if( fp->pdf == pdf )
  {
   freefp(fp);
  }
  fp = fpnext;
 }
}

/*****************************************************************************/
/* Free symbols.                                                             */
/*****************************************************************************/
void FreeSyms( DEBFILE *pdf )
{
 SYMNODE      *sptr;
 SYMNODE      *sptrnext;
  sptr=pdf->psyms;
  while( sptr )
  {
   sptrnext=sptr->next;
   if(sptr->symptr)
    Tfree((void*)sptr->symptr);                                          /*521*/
   Tfree((void*)sptr);                                                   /*521*/
   sptr=sptrnext;
  }
}
/*****************************************************************************/
/* Free types.                                                               */
/*****************************************************************************/
void FreeTyps( DEBFILE *pdf )
{
 TYPENODE     *tptr;
 TYPENODE     *tptrnext;
  tptr=pdf->ptyps;
  while( tptr )
  {
   tptrnext=tptr->next;
   if(tptr->typptr)
    Tfree((void*)tptr->typptr);                                          /*521*/
   Tfree((void*)tptr);                                                   /*521*/
   tptr=tptrnext;
  }
}

/*****************************************************************************/
/* Free public info.                                                         */
/*****************************************************************************/
void FreePubs( DEBFILE *pdf )
{
 PUBNODE      *pptr;
 PUBNODE      *pptrnext;
  pptr=pdf->ppubs;
  while( pptr )
  {
   pptrnext=pptr->next;
   if(pptr->pubptr)
    Tfree((void*)pptr->pubptr);                                          /*521*/
   Tfree((void*)pptr);                                                   /*521*/
   pptr=pptrnext;
  }
}
/*****************************************************************************/
/* DBGetAsmLines()                                                           */
/*                                                                           */
/* Description:                                                              */
/*   get the number of assembler lines for an executable source line.        */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   fp        input  - afile for this asm window.                           */
/*   lno       input  - the executable line number.                          */
/*   pAddr     output - -> to location receiving starting address of      235*/
/*                         the executable line.                              */
/*   SkipBytes input  - no of bytes to be skipped while calculating span  530*/
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   count     the number of assembler lines required for this module.       */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   lno is executable.                                                      */
/*                                                                           */
/*****************************************************************************/
extern INSTR   *icache;                 /* the instruction cache.         235*/
 int                                    /*                                   */
DBGetAsmLines( AFILE *fp , uint lno, uint *pAddr,uint SkipBytes )       /*530*/
{                                       /*                                   */
 uint  addr;                            /*                                235*/
 ULONG span = 0;                        /*                                521*/
 uchar *InstrBytesBuffer;               /*                                235*/
 uint   BytesRead = 0;                  /*                                521*/
 int    InstrCount;                     /*                                235*/
 int    InstrLength;                    /*                                235*/
 uchar *PtrToEndOfBuffer;               /*                                235*/
 uchar *InstrPtr;                       /*                                235*/
 UCHAR  type;
 UCHAR  bitness;

 *pAddr = addr = DBMapLno(fp->mid, lno, fp->sfi, &span, fp->pdf);

 bitness = GetBitness( addr);
 type = (bitness==BIT16)?USE16:USE32;
 InstrBytesBuffer = GetCodeBytes( addr, span, &BytesRead );
 if( BytesRead != span )
  return(0);

 InstrCount = 0;
 PtrToEndOfBuffer = InstrBytesBuffer + span;
 InstrPtr = InstrBytesBuffer;
 /****************************************************************************/
 /* Modify the starting instr ptr depending on th Skip bytes value, this  530*/
 /* Skip bytes is always 0 except when we try to handle names ID's in     530*/
 /* PLX dis assembly.                                                     530*/
 /****************************************************************************/
 InstrPtr = InstrPtr + SkipBytes;                                       /*530*/
 while( InstrPtr < PtrToEndOfBuffer )                                   /*235*/
 {                                                                      /*235*/
  InstrCount++;                                                         /*235*/
  InstrLength = InstLengthGlob( InstrPtr , type );
  InstrPtr += InstrLength;                                              /*235*/
 }                                                                      /*235*/
 return( InstrCount );                                                  /*235*/
}                                                                       /*235*/

/*************************************************************************101*/
/* GetBitness()                                                           101*/
/*                                                                        101*/
/* Description:                                                           101*/
/*   Get the 16 or 32 bitness of the address parameter.                   101*/
/*                                                                        101*/
/* Parameters:                                                            101*/
/*   addr       address.                                                  101*/
/*                                                                        101*/
/* Return:                                                                101*/
/*   BIT32      address is 32 bit.                                        101*/
/*   BIT16      address is 16 bit.                                        101*/
/*   BITUNKNOWN can't determine.                                          101*/
/*                                                                        101*/
/*************************************************************************101*/
 uchar                                  /*                                101*/
GetBitness( uint addr )                 /*                                101*/
{                                       /*                                101*/
 DEBFILE       *pdf;                    /* executing debug file           101*/
 OBJTABLEENTRY *te;                     /* -> to a object table entry.    101*/
 uint           NumCodeObjs;            /* number of table entries.       521*/
 int            i;                      /*                                101*/
 uint          *p;                      /*                                521*/
                                        /*                                101*/
/*************************************************************************101*/
/* - get a pointer to the process node.                                   101*/
/* - scan pdf object table for the object containing this addr.           101*/
/* - return the type of the object.                                       101*/
/*************************************************************************101*/
 pdf = pnode->ExeStruct;                                                /*101*/
 for( ; pdf; pdf=pdf->next )                                            /*101*/
 {                                                                      /*101*/
  p=pdf->CodeObjs;                                                      /*101*/
  if (p == NULL)                                                        /*107*/
    continue;                                                           /*107*/
  NumCodeObjs = *p;                                                     /*101*/
  if ( NumCodeObjs == 0 )                                               /*101*/
   continue;                                                            /*101*/
  te = (OBJTABLEENTRY *)++p;                                            /*101*/
  for(i=1; i <= NumCodeObjs; i++,te++ )                                 /*101*/
  {                                                                     /*101*/
   if( addr >= te->ObjLoadAddr &&                                       /*101*/
       addr <  te->ObjLoadAddr + te->ObjLoadSize )                      /*101*/
    return(te->ObjBitness);                                             /*606*/
  }                                                                     /*101*/
 }                                                                      /*101*/
 return(BITUNKNOWN);                                                    /*101*/
}                                       /* end GetBitness().            /*101*/

/*************************************************************************115*/
/* MapAddrtoObjnum()                                                         */
/*                                                                           */
/* Description:                                                              */
/*   Map a address to a object number                                        */
/*                                                                           */
/* Parameters:                                                               */
/*   pdf        pointer to debug file structure.                             */
/*   addr       address.                                                     */
/*                                                                           */
/* Return:                                                                   */
/*   i          object number.                                               */
/*                                                                           */
/*************************************************************************115*/
ushort
MapAddrtoObjnum(DEBFILE *pdf , uint addr , uint *LoadAddr)
{
 OBJTABLEENTRY *te;                     /* -> to a object table entry.    115*/
 uint           NumObjs;                /* number of table entries.       521*/
 int            i;                      /*                                115*/
 uint          *p;                      /*                                521*/
                                        /*                                115*/
  p=pdf->CodeObjs;
  if (p == NULL)
    return(0);
  NumObjs = *p;
  if ( NumObjs == 0 )
    return(0);
  te = (OBJTABLEENTRY *)++p;
  for(i=1; i <= NumObjs; i++,te++ )
  {
     /************************************************************************/
     /* search only the code objects                                      606*/
     /************************************************************************/
     if ( te->ObjType == CODE)                                          /*606*/
     {
        if( addr >= te->ObjLoadAddr &&
           addr <  te->ObjLoadAddr + te->ObjLoadSize )
        {
           *LoadAddr = te->ObjLoadAddr;
            return((ushort)i);
        }
     }
  }
  return(0);
}                                       /* end MapAddrtoObjnum().        115*/

/*************************************************************************115*/
/* MapFlatAddrToBase()                                                       */
/*                                                                           */
/* Description:                                                              */
/*   Map a flat address to its 16:16 base.                                   */
/*                                                                           */
/* Parameters:                                                               */
/*   pdf        pointer to debug file structure.                             */
/*   addr       flat address in a 16 bit object.                             */
/*   pLoadAddr  -> to receiver of flat load address.                         */
/*   pSel       -> to receiver of 16 bit load selector.                      */
/*   pOff       -> to receiver of 16 bit load offset.                        */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   0=>success                                                              */
/*   1=>failure                                                              */
/*                                                                           */
/*************************************************************************115*/
int  MapFlatAddrToBase(DEBFILE *pdf,
                       ULONG    addr,
                       ULONG   *pLoadAddr,
                       USHORT  *pSel,
                       USHORT  *pOff )
{
 OBJTABLEENTRY *te;
 uint           NumObjs;
 int            i;
 uint          *p;

 p=pdf->CodeObjs;
 if( (p == NULL) || ((NumObjs = *p) == 0) )
  return(1);

 NumObjs = *p;
 *pLoadAddr = 0;
 *pSel      = 0;
 *pOff      = 0;
 te = (OBJTABLEENTRY *)++p;
 for(i=1; i <= NumObjs; i++,te++ )
 {
  if((addr >= te->ObjLoadAddr) && (addr <  te->ObjLoadAddr + te->ObjLoadSize))
  {
   *pLoadAddr = te->ObjLoadAddr;
   *pSel      = te->ObjLoadSel;
   *pOff      = te->ObjLoadOff;
   return(0);
  }
 }
 return(1);
}

/*****************************************************************************/
/* MapSelOffToBase()                                                         */
/*                                                                           */
/* Description:                                                              */
/*   Map a flat address to its 16:16 base.                                   */
/*                                                                           */
/* Parameters:                                                               */
/*   pdf        pointer to debug file structure.                             */
/*   Selector   16 bit selector.                                             */
/*   Offset     16 offset.                                                   */
/*   pLoadAddr  -> to receiver of flat load address.                         */
/*   pLoadOff   -> to receiver of 16 bit load offset.                        */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   0=>success                                                              */
/*   1=>failure                                                              */
/*                                                                           */
/*************************************************************************115*/
int   MapSelOffToBase(DEBFILE *pdf,
                      USHORT  Selector,
                      USHORT  Offset,
                      ULONG  *pLoadAddr,
                      USHORT *pLoadOff)
{
 OBJTABLEENTRY *te;
 uint           NumObjs;
 int            i;
 uint          *p;

 p=pdf->CodeObjs;
 if( (p == NULL) || ((NumObjs = *p) == 0) )
  return(1);

 NumObjs = *p;
 *pLoadAddr = 0;
 *pLoadOff  = 0;
 te = (OBJTABLEENTRY *)++p;
 for(i=1; i <= NumObjs; i++,te++ )
 {
  if( (Selector == te->ObjLoadSel ) )
  {
   USHORT LoOffset;
   USHORT HiOffset;

   LoOffset = te->ObjLoadOff;
   HiOffset = LoOffset + te->ObjLoadSize - 1;
   if( (LoOffset <= Offset) && (Offset <= HiOffset) )
   {
    *pLoadAddr = te->ObjLoadAddr;
    *pLoadOff  = te->ObjLoadOff;
    return(0);
   }
  }
 }
 return(1);
}

/*****************************************************************************/
/* hexstr2higit()                                                            */
/*                                                                           */
/* Description:                                                              */
/*   convert a string to higits.                                             */
/*                                                                           */
/* Parameters:                                                               */
/*   cp         -> to string.                                                */
/*   phigit     -> to higit buffer.                                          */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*  cp -> to a string of hex characters.                                     */
/*                                                                           */
/*****************************************************************************/
void hexstr2higit( char *cp, char *phigit )
{
 char  c;
 int   n = 1;

 while( (c=*cp++) != 0 )
 {
  if( '0'<=c && c<='9' )
   c -= 0x30;
  else
   c -= 0x37;

  *phigit |= (c<<(n*4));
  n = (n==1)?0:1;
  phigit += n;
 }
}

/*****************************************************************************/
/* GetCodebytes;                                                             */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   gets n bytes of code from the app address space.                        */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   address    pointer to address space in user's application.              */
/*   nbytes     number of bytes to copy from the user's application.         */
/*   totlptr    pointer to number of bytes that were read in.                */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   p          pointer to buffer that holds the bytes read in.              */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   address  is a flat address.                                             */
/*                                                                           */
/*****************************************************************************/
static UCHAR *pCurrentBuffer = NULL;
static ULONG HiAddr;
static ULONG LoAddr;
#define PAGESIZE 0x400

UCHAR* GetCodeBytes( ULONG address, UINT nbytes, UINT *totlptr  )
{
 APIRET         rc;
 PtraceBuffer   ptb;
 ULONG          LoObjectAddr;
 ULONG          HiObjectAddr;
 ULONG          ObjectSize;
 ULONG          EndAddress;
 ULONG          ReadAddr;
 ULONG          ReadLen;
 int            i;
 UINT           NumCodeObjs;
 UINT          *pObj;
 OBJTABLEENTRY *te;
 int            foundit = FALSE;
 DEBFILE       *pdf;
 int            twotries;

 for( twotries = 1; twotries <= 2; twotries++ )
 {
  /****************************************************************************/
  /* - check to see if we already have the bytes cached.                      */
  /****************************************************************************/
  if( (address >= LoAddr) &&
      ( (address + nbytes - 1) <= HiAddr )
    )
  {
   *totlptr = nbytes;
   return( pCurrentBuffer + (address - LoAddr) );
  }
  else
  {
   /****************************************************************************/
   /* - at this point, we have to recache.                                     */
   /* - get a ptr to the code object table entry that contains this address.   */
   /****************************************************************************/
   pdf = FindExeOrDllWithAddr( address );
   if( pdf )
   {
    NumCodeObjs = *(pObj=pdf->CodeObjs);
    te = (OBJTABLEENTRY *)++pObj;
    for(i=1; (foundit == FALSE) && (i <= NumCodeObjs); i++,te++ )
    {
     if( (te->ObjType == CODE)     &&
         (address >= te->ObjLoadAddr) &&
         (address <  te->ObjLoadAddr + te->ObjLoadSize) )
       {foundit=TRUE;break;}
    }
   }

   /****************************************************************************/
   /*   if the read request spans the object then                              */
   /*    cache the requested block.                                            */
   /*   else if the object size is < a page size                               */
   /*    cache the entire object.                                              */
   /*   else if the requested block is near the bottom of the object           */
   /*    cache a page at the botton of the object                              */
   /*    ( we assume that the requested block is small enough to still be      */
   /*      contained within a page after the start address for the read        */
   /*      is adjusted to the object start address.)                           */
   /*   else if the requested block is near the top of the page                */
   /*    cache a page at the top of the object                                 */
   /*   else if the requested block is in the "middle" of the object           */
   /*    cache a page "around" the requested block.                            */
   /*                                                                          */
   /* At the time of this code, the largest read request block size was 300    */
   /* bytes.                                                                   */
   /****************************************************************************/
   LoObjectAddr = te->ObjLoadAddr;
   HiObjectAddr = te->ObjLoadAddr + te->ObjLoadSize - 1;
   EndAddress   = address + nbytes -1;
   ObjectSize   = te->ObjLoadSize;
   ReadLen  = PAGESIZE;

   if( (foundit == FALSE) || (EndAddress > HiObjectAddr) )
   {
    ReadAddr = address;
    ReadLen  = nbytes;
   }
   else if( ObjectSize <= PAGESIZE )
   {
    ReadAddr = te->ObjLoadAddr;
    ReadLen  = te->ObjLoadSize;
   }
   else if( (address - LoObjectAddr + 1) <= PAGESIZE/2 )
   {
    ReadAddr = LoObjectAddr;
   }
   else if( (HiObjectAddr - address + 1) <= PAGESIZE/2 )
   {
    ReadAddr = HiObjectAddr - PAGESIZE + 1;
   }
   else
   {
    ReadAddr = address - PAGESIZE/2 + 1;
   }

  /*****************************************************************************/
  /* - Free the storage from the prevoius call.                                */
  /* - Allocate the new storage block.                                         */
  /* - Read in all the bytes if we can.                                        */
  /* - If we can't, then try reading one at a time and return the number       */
  /*   read.                                                                   */
  /*                                                                           */
  /*****************************************************************************/

   if ( pCurrentBuffer != NULL )
     Tfree( (void*)pCurrentBuffer );
   pCurrentBuffer = (UCHAR*)Talloc( ReadLen);

   *totlptr = 0;

   memset(&ptb,0,sizeof(ptb));
   ptb.Pid    = DbgGetProcessID();
   ptb.Cmd    = DBG_C_ReadMemBuf ;
   ptb.Addr   = ReadAddr;
   ptb.Buffer = (ULONG)pCurrentBuffer;
   ptb.Len    = ReadLen;
   rc = xDosDebug( &ptb );
   *totlptr=ptb.Len;

   /***************************************************************************/
   /* If not able to read the number of bytes asked for , try reading byte    */
   /* by byte as many bytes as possible. This might be slow...                */
   /***************************************************************************/
   if(rc || ptb.Cmd != DBG_N_Success)
   {
    UINT ui;
    *totlptr = 0;
    for (ui = 0 ; ui < nbytes ; ui++)
    {
     memset(&ptb,0,sizeof(ptb));
     ptb.Pid    = DbgGetProcessID();
     ptb.Cmd    = DBG_C_ReadMemBuf ;
     ptb.Addr   = address+ui;
     ptb.Buffer = (ULONG)(pCurrentBuffer + ui);
     ptb.Len    = 1;
     rc = xDosDebug( &ptb );
     if(rc || ptb.Cmd!=DBG_N_Success)
         break;
     (*totlptr)++;
    }
   }
   LoAddr = ReadAddr;
   HiAddr = LoAddr + *totlptr - 1;
  }
 }
 /****************************************************************************/
 /* - we were not able to read any bytes...address must be bogus.            */
 /****************************************************************************/
 return(NULL);
}

/*****************************************************************************/
/* GetDataBytes;                                                             */
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
/*   -> bytes requested or NULL.                                             */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   address  is a flat address.                                             */
/*                                                                           */
/*****************************************************************************/
typedef struct _dblock
{
 struct _dblock *next;
 ULONG           baseaddr;
 int             size;
 char           *pbytes;
}DBLOCK;

static DBLOCK *pAllBlks = NULL;

extern int         DataRecalc;

UCHAR* GetDataBytes( ULONG address, UINT nbytes, UINT *totlptr  )
{
 DBLOCK       *pBlk;
 DBLOCK       *pb;
 ULONG         loaddr;
 ULONG         hiaddr;
 UCHAR        *pbytes;
 int           BytesObtained;


 /****************************************************************************/
 /* - First filter out address = 0.                                          */
 /****************************************************************************/
 if( address == 0 )
 {
  *totlptr = 0;
  return(NULL);
 }
 /****************************************************************************/
 /* - refresh the memory blocks if necessary.                                */
 /****************************************************************************/
 if( DataRecalc == TRUE )
 {
  GetMemoryBlocks();
  DataRecalc = FALSE;
 }

rescan:
 /****************************************************************************/
 /* - scan the list of memory blocks and test for the address.               */
 /****************************************************************************/
 for( pBlk = pAllBlks; pBlk; pBlk=pBlk->next )
 {
  loaddr = pBlk->baseaddr;
  hiaddr = loaddr + pBlk->size - 1;     /* added the "-1"                    */
  if( (address >= loaddr) &&
      ( (address + nbytes - 1) >= loaddr ) && /* check for a wrap            */
      ( (address + nbytes - 1) <= hiaddr )
    )
  {
   *totlptr = nbytes;
   return( pBlk->pbytes + (address - loaddr) );
  }
 }

 /****************************************************************************/
 /* - Get the memory block.                                                  */
 /****************************************************************************/
 BytesObtained = 0;
 pbytes = xGetMemoryBlock( address, nbytes, &BytesObtained );
 if( pbytes && ( BytesObtained != 0 ) )
 {
  /***************************************************************************/
  /* - Scan the memory blocks and see if this block can be merged with       */
  /*   another block. This will help prevent fragmentation of the memory.    */
  /***************************************************************************/
  for( pBlk = pAllBlks; pBlk; pBlk=pBlk->next )
  {
   if( pBlk->baseaddr + pBlk->size == address )
   {
    UCHAR *pTemp;
    int    size;

    size = pBlk->size + BytesObtained;

    pTemp = Talloc( size );
    memcpy(pTemp,pBlk->pbytes,pBlk->size);
    memcpy(pTemp+pBlk->size,pbytes,BytesObtained);
    Tfree(pBlk->pbytes);
    pBlk->pbytes = pTemp;
    pBlk->size  = size;
    goto rescan;
   }
  }
  /***************************************************************************/
  /* - If we get here, then the block could not be merged and we have        */
  /*   to add to the chain. pBlk = NULL at this point.                       */
  /***************************************************************************/
  if( pBlk == NULL )
  {
   pBlk           = Talloc(sizeof(DBLOCK));
   pBlk->next     = NULL;
   pBlk->baseaddr = address;
   pBlk->size     = BytesObtained;
   pBlk->pbytes   = Talloc(BytesObtained);
   memcpy(pBlk->pbytes, pbytes , BytesObtained );
   for(pb = (DBLOCK*)&pAllBlks; pb->next; pb=pb->next){;}
   pb->next = pBlk;

   *totlptr = BytesObtained;
   return(pBlk->pbytes);
  }
 }
 /****************************************************************************/
 /* - return sadness.                                                       */
 /****************************************************************************/
 *totlptr = 0;
 return(NULL);
}

/*****************************************************************************/
/* GetMemoryBlocks()                                                         */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Get a refresh of the memory blocks currently being viewed.              */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
void GetMemoryBlocks( void )
{
 typedef struct
 {
  ULONG addr;
  int   size;
 }MEM_BLK_DEF;

 MEM_BLK_DEF  *pDefBlks;
 MEM_BLK_DEF  *pd;
 DBLOCK       *pBlk;
 DBLOCK       *pnext;
 int           n;
 int           i;
 int           LengthOfDefBlk;

 typedef struct
 {
  ULONG addr;
  int   size;
  char  bytes[1];
 }MEM_BLKS;

 MEM_BLKS  *pMemBlks;
 MEM_BLKS  *pmblk;
 int        LengthOfMemBlks;
 DBLOCK    *pb;

 /****************************************************************************/
 /* - scan and get a count of the current number of blocks.                  */
 /****************************************************************************/
 n = 0;
 for(pBlk = pAllBlks; pBlk; pBlk=pBlk->next,n++){;}
 if( n == 0 )
  return;

 /****************************************************************************/
 /* - allocate the storage to contain the definitions of the memory blocks   */
 /*   and the total size of the memory blocks( including overhead ).         */
 /****************************************************************************/
 LengthOfDefBlk = (n+1)*sizeof(MEM_BLK_DEF);
 pd = pDefBlks = (MEM_BLK_DEF *)Talloc( sizeof(LengthOfMemBlks) +
                                               LengthOfDefBlk);
 /****************************************************************************/
 /* - bump past the field reserved for LengthOfMemBlks.                      */
 /****************************************************************************/
 pd = (MEM_BLK_DEF*)((char*)pd + sizeof(int));

 /****************************************************************************/
 /* - build an array of memory block definitions.                            */
 /* - terminate the blocks with a (NULL,0) entry.                            */
 /* - calculate the "expected" size of the return block.                     */
 /****************************************************************************/
 LengthOfMemBlks = 0;
 for(pBlk = pAllBlks; pBlk; pBlk=pBlk->next,pd++)
 {
  pd->addr = pBlk->baseaddr;
  pd->size = pBlk->size;
  LengthOfMemBlks += sizeof(MEM_BLK_DEF) + pd->size;
 }
 pd->addr = NULL;
 pd->size = 0;

 /****************************************************************************/
 /* - put in the total length of the memory blocks.                          */
 /****************************************************************************/
 *(int *)pDefBlks = LengthOfMemBlks;

 /****************************************************************************/
 /* - free the old chain.                                                    */
 /****************************************************************************/
 pnext = NULL;
 for(pBlk = pAllBlks; pBlk; pBlk=pnext,pd++)
 {
  if( pBlk->pbytes )
   Tfree(pBlk->pbytes);
  pnext = pBlk->next;
  Tfree( pBlk );
 }
 pAllBlks = NULL;

 /****************************************************************************/
 /* - Get the memory blocks.                                                 */
 /* - LengthOfMemBlks will contain the "expected" size of the new block      */
 /*   on the call and will contain the "real" size on return.                */
 /****************************************************************************/
 pmblk = pMemBlks = (MEM_BLKS *)xGetMemoryBlocks(pDefBlks, LengthOfDefBlk);

 /****************************************************************************/
 /* - free up the block definitions.                                         */
 /****************************************************************************/
 if( pDefBlks)
  Tfree(pDefBlks);

 /****************************************************************************/
 /* - Now build a new chain.                                                 */
 /****************************************************************************/
 pb = (DBLOCK*)&pAllBlks;
 for( i=0;i < n ; i++ )
 {
  pBlk           = Talloc(sizeof(DBLOCK));
  pBlk->next     = NULL;
  pBlk->baseaddr = pmblk->addr;
  pBlk->size     = pmblk->size;
  pBlk->pbytes   = Talloc(pBlk->size);
  memcpy(pBlk->pbytes,pmblk->bytes,pmblk->size);

  pb->next = pBlk;
  pb = pb->next;
  pmblk = (MEM_BLKS *)((UCHAR*)pmblk + sizeof(MEM_BLKS) - 1 + pmblk->size);
 }
 /****************************************************************************/
 /* - free the memory blocks.                                                */
 /****************************************************************************/
 if( pMemBlks )
  Tfree(pMemBlks);
}

void FreeMemBlks( void )
{
 DBLOCK       *pBlk;
 DBLOCK       *pBlkNext;

 for(pBlk = pAllBlks; pBlk != NULL; )
 {
  pBlkNext = pBlk->next;
  if(pBlk->pbytes) Tfree(pBlk->pbytes);
  Tfree( pBlk );
  pBlk = pBlkNext;
 }
 pAllBlks = NULL;
}
