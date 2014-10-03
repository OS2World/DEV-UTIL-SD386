/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   scrolla.c                                                               */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*  Handle scrolling in the disassembly view.                                */
/*                                                                           */
/* History:                                                                  */
/*                                                                           */
/*   08/30/91 Creation of 32-bit SD86, from 16-bit version.                  */
/*            Extracted from showa.c and rewritten.                          */
/*                                                                           */
/*...Release 1.00 (Pre-release 1)                                            */
/*...                                                                        */
/*... 08/30/91  235   Joe       Cleanup/rewrite ascroll() to fix several bugs*/
/*...                                                                        */
/*...Release 1.00 (Pre-release 107 11/13/91)                                 */
/*...                                                                        */
/*... 11/13/91  400   Srinivas  Vertical Register Display.                   */
/*...                                                                        */
/*... 11/18/91  401   Srinivas  Floating point Register Display.             */
/*...                                                                        */
/*... 11/21/91  402   Joe       Fix source/disassembly synchronization.      */
/*...                                                                        */
/*... 11/21/91  404   Joe       S_f3 in MIXED disasm view with all src lines */
/*...                           hangs.                                       */
/*...                                                                        */
/*...Release 1.00 (Pre-release 108 12/05/91)                                 */
/*...                                                                        */
/*... 02/11/92  518   Srinivas  Remove limitation on max no of screen rows.  */
/*... 02/11/92  519   Srinivas  Zoomrec Sizing Problems.                     */
/*... 02/12/92  521   Srinivas  Port to C-Set/2.                             */
/*... 02/14/92  530   Srinivas  Jumping across ID in case of PLX disassembly */
/*...                                                                        */
/*...Release 1.00 (03/03/92)                                                 */
/*...                                                                        */
/*... 03/18/92  606   Srinivas  Handle multiple segment numbers in lno table */
/*...                           due to alloc_text pragma.                    */
/*... 12/14/93  911   Joe       Hang showing PL/X disassembly of select stmt.*/
/**Includes*******************************************************************/
                                        /*                                   */
#include "all.h"                        /* SD86 include files                */
static int iview=0;                     /*                                   */
/**Macros ********************************************************************/
                                        /*                                   */
#define N1KEYS 29                       /*                                   */
#define ENDOFTABLE 0xFFFFFFFF           /* marker for end of display table521*/

#define ATTR_BKPT_OK_LINE      Attrib(vaBptOk)  /* line ok for brkpt.        */
#define ATTR_BKPT_ON_LINE      Attrib(vaBptOn)  /* bkpt on this line.        */
#define ATTR_EXEC_LINE         Attrib(vaXline)  /* executing line.           */
#define ATTR_BKPT_ON_EXEC_LINE Attrib(vaXlineOn)/* brkpt on executing line.  */
#define ATTR_SOURCE_LINE       Attrib(vaMxdSrc) /* source line color.        */

/**External declararions******************************************************/
                                        /*                                   */
extern uint   VideoCols;                /*                                   */
extern uint   VideoRows;                /*                                   */
extern uint   NActFrames;               /*                                   */
extern uchar *ActFaddrs[];              /*                                   */
extern uint   RowsUsed;                 /*                                   */
extern int    AsmTop;                   /*                                   */
extern uchar  Reg_Display;              /*                                400*/
extern PROCESS_NODE *pnode;                                             /*827*/
                                        /*                                   */
/**External definitions*******************************************************/
                                        /*                                   */
uint   *AddrCache;                      /*                                518*/
uint   *SourceLineCache;                /*                                518*/
static uint   *TempAddrCache;           /*                                518*/
static uint   *TempSourceLineCache;     /*                                518*/
static uint   AsmTableSize;             /* disassembly table size         518*/
static uchar  *textbuf;                 /* -> to disasembly text          518*/
static uint   TextBufSize;              /* length of disassembly text buf 518*/
int     AsmRows;                        /*                                   */
INSTR  *icache;                         /*                                   */
int     CacheAnchorIndex;               /*                                   */
int     InstrCacheSize;                 /*                                   */

extern  int iview;
/**Begin Code*****************************************************************/
                                        /*                                   */
/*****************************************************************************/
/* ascroll()                                                                 */
/*                                                                           */
/* Description:                                                              */
/*   display the assembler window.                                           */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   fp        input - afile for this asm window.                            */
/*   delta     input - how much to adjust this window.                       */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   seekline  ?????????????????????????????????                             */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   none.                                                                   */
/*                                                                           */
/*****************************************************************************/
                                        /*                                   */
  uint                                  /*                                   */
ascroll(AFILE *fp, int  delta )         /*                                   */
{                                       /*                                   */
 uint      n;                           /* assembler text buffer index.      */
 uint      lno;                         /* a source line number.             */
 uchar    *cp;                          /* -> into disassembled text buffer. */
 uchar     digits[6];                   /* buffer for lno ascii digits.      */
 uint      i;                           /* just an i.                        */
 uint      ndigits;                     /* # of digits converted by buffmt.  */
 uint      brkxline;                    /* line # of brkpt & exec line.      */
 uint      addr; /* !!! */              /* an address.                       */
 uchar    *ptr;                         /* a ptr.                            */
  int     row;                          /* was uint              !!!     235 */
  uint    span;                         /*                                   */
  uint    count;                        /*                                   */
  uint    read;                         /*                                   */
   int index;

   int   TopMargin;
   ULONG  StartLineNumber;              /* changed to uint                521*/
   uint   junk;                         /* changed to uint                521*/
   uint  StartAddr;
   int   TableDone;
   int   LastIndex;
   int   idx;
   uint FirstAddrForLine;


 if( icache == NULL )
 {
  int size;
  InstrCacheSize = 4*(VideoRows - 2);   /* removed regrows                400*/
  CacheAnchorIndex = InstrCacheSize/2;
  size = (InstrCacheSize + 1)*sizeof(INSTR);
  icache = ( INSTR *)Talloc( size );                                    /*521*/

  /***************************************************************************/
  /* Allocate memory for the arrays depending on the number of videorows. 518*/
  /* Removed array declarations and added dynamic allocation.             518*/
  /*                                                                      518*/
  /* Note :                                                               518*/
  /*   - AsmTableSize is equal to VideoRows minus two status lines at     518*/
  /*     the bottom of the screen plus one entry for end of table.        518*/
  /***************************************************************************/
  AsmTableSize         =  (VideoRows - 1) * sizeof(int);                /*518*/                /*518*/
  TempAddrCache        =  (uint *)Talloc( AsmTableSize );           /*521 518*/
  TempSourceLineCache  =  (uint *)Talloc( AsmTableSize );           /*521 518*/
  AddrCache            =  (uint *)Talloc( AsmTableSize );           /*521 518*/
  SourceLineCache      =  (uint *)Talloc( AsmTableSize );           /*521 518*/
  SourceLineCache[0]   =  ENDOFTABLE;                                   /*518*/


  /***************************************************************************/
  /* Allocate memory for the dis assembly text.                           518*/
  /***************************************************************************/
  TextBufSize = (VideoRows - 2) * (VideoCols+2);                                   /*518*/
  textbuf = (uchar *) Talloc( TextBufSize );                        /*521 518*/

 }

 if( fp->source == NULL || fp->sview == NOSRC )
 {
  /** First, we'll handle views without source *******************************/
  /*                                                                         */
  /* We come into ascroll() with a delta value that tells us how far to      */
  /* move up or down in the disassembly view.  What we will do is define a   */
  /* StartAddr that we will use to build a set of display tables.  The       */
  /* tables will be kept in static storage because we'll need them for       */
  /* some history.                                                           */
  /*                                                                         */
  /* - delta < 0 ==> moving up...pgup,cursor up, etc.                        */
  /* - delta > 0 ==> moving dn...pgdn,cursor dn, etc.                        */
  /* - delta = 0 ==> coming if from mixed source/asm view or refreshing.     */
  /*                                                                         */
  /* delta = 0                                                               */
  /*                                                                         */
  /*   ToggleAsmVIew, i.e.  switching to asm view from mixed view.           */
  /*   The AF_SYNC flag will be set.  We take the current topoff and         */
  /*   expand from there. If there is no topoff, say for a mixed view        */
  /*   showing all comment lines, then don't do anything.                    */
  /*                                                                         */
  /* - Single Step, Go, etc., in which case AF_ZOOM will be set.             */
  /*   If the hotaddr is already on the display, then don't rebuild          */
  /*   the tables, else move the hotaddr to the top of the display.          */
  /*                                                                         */
  /*                                                                         */
  /* delta < 0                                                               */
  /*                                                                         */
  /*  In this case, we make a special call to DBDisa that lets us back up    */
  /*  in the disassembly view.                                               */
  /*                                                                         */
  /* delta > 0                                                               */
  /*                                                                         */
  /*  This is the easy one.  All we have to do is get the StartAddr and      */
  /*  from the last entries in the current display buffers and then          */
  /*  expand down from there.                                                */
  /*                                                                         */
  /***************************************************************************/
  if( delta <= 0 )
  {
   if( delta == 0 )
   {
    if( fp->flags & AF_ZOOM )
    {
     if( IsOnAsmCRT(fp->hotaddr ) )
      goto BUILDTEXT;

     StartAddr = fp->topoff;
    }

    else if( fp->flags & ASM_VIEW_NEW )
      StartAddr = fp->topoff;

    else if( fp->flags & ASM_VIEW_TOGGLE )
    {
     StartAddr = fp->topoff;
     if( StartAddr == NULL )                                            /*404*/
      return( 0);                                                       /*404*/
    }
    else if( fp->flags & ASM_VIEW_CHANGE )
      StartAddr = fp->topoff;

    else if( fp->flags & ASM_VIEW_NEXT )
    {
     StartAddr = fp->AsmView.StartAddr;
     goto REBUILDTABLE;
    }
    else if( fp->flags & ASM_VIEW_MNE  )
    {
      DBDisa(fp->mid, 0, 0, 0 );
      goto BUILDTEXT;
    }
    else if( fp->flags & ASM_VIEW_REFRESH )
    {
     goto BUILDTEXT;
    }
   }

   else if( delta < 0 )
   {
    /*************************************************************************/
    /* At this point, we have to back up in the disassembly view.            */
    /*************************************************************************/
    index = DBDisa(fp->mid, AddrCache[0], AsmRows, delta );
    StartAddr = icache[index].instaddr;
   }
  }

  else if( delta > 0 )
  {
   StartAddr = AddrCache[delta];
  }

REBUILDTABLE:
  /***************************************************************************/
  /* Now, we have a StartAddr to begin generating dispaly tables.            */
  /* The display tables will look like so:                                   */
  /*                                                                         */
  /*   SourceLineNumber        AddrCache                                     */
  /*        0                     addr1                                      */
  /*        0                     .                                          */
  /*        0                     .                                          */
  /*        0                     .                                          */
  /*        0                     addri                                      */
  /*        0                     addrj                                      */
  /*        0                     addrk                                      */
  /*        0                     .                                          */
  /*        0                     .                                          */
  /*        0                     .                                          */
  /*        -1 ( ENDOFTABLE )     .                                          */
  /*                                                                         */
  /***************************************************************************/
  TableDone = FALSE;
  addr  = StartAddr;
  memset( TempSourceLineCache, 0 , AsmTableSize );                      /*518*/
  memset( TempAddrCache , 0 , AsmTableSize );                           /*518*/
  index  = DBDisa(fp->mid, StartAddr , AsmRows, 0 );
  LastIndex = AsmRows - 1;
  for ( row = 0 ; row <= LastIndex && TableDone == FALSE; index++,row++ )
  {
   addr = icache[index].instaddr;
   if( addr == ENDOFCACHE )
   {
    TableDone = TRUE;
    break;
   }
   TempAddrCache[row] = addr;
  }

  /***************************************************************************/
  /* On a pgdn or cursor down we want to anchor the bottom line to the       */
  /* bottom of the display. So, we check to see if the display table is      */
  /* full, and if not, we make an adjustment and go back and fill it up.     */
  /*                                                                         */
  /* - so, mark the end of the table.                                        */
  /* - check to see if we have a full display.                               */
  /* - if we don't then adjust delta and redo the table.                     */
  /* - if all is ok, then copy the temp tables to the static tables.         */
  /***************************************************************************/
  TempSourceLineCache[row] = ENDOFTABLE;
  for( row=0; TempSourceLineCache[row] != ENDOFTABLE; row++ ){;}
  if( delta > 0 && row < AsmRows )
  {
   delta -= AsmRows - row;
   if( delta < 0 )
    delta = 0;
   StartAddr       = AddrCache[delta];
   goto REBUILDTABLE;
  }
  memcpy( SourceLineCache, TempSourceLineCache, AsmTableSize );         /*518*/
  memcpy( AddrCache,TempAddrCache , AsmTableSize );                     /*518*/
  goto BUILDTEXT;
 }
/*===========================================================================*/

/** Come here for Mixed Source/Disassembly view ******************************/
/*                                                                           */
/* We come into ascroll() with a delta value that tells us how far to        */
/* move up or down in the disassembly view. What we will do is define        */
/* a StartAddr and a StartLineNumber that we will use to build a             */
/* set of display tables. The tables will be kept in static storage because  */
/* we'll need them for some history.                                         */
/*                                                                           */
/* - delta < 0 ==> moving up...pgup,cursor up, etc.                          */
/* - delta > 0 ==> moving dn...pgdn,cursor dn, etc.                          */
/* - delta = 0 ==> coming if from source level or refreshing.                */
/*                                                                           */
/*                                                                           */
/* delta = 0                                                                 */
/*                                                                           */
/* - ToggleAsm, i.e.  switching to asm view from source view.  In this case, */
/*   the AF_SYNC flag will be set.  We are going to take the cursor line in  */
/*   the source view and essentailly move it to the same relative position   */
/*   in the disassembly view.  We will then expand up to fill in the margin  */
/*   above and expand down to fill in the margin below.                      */
/*                                                                           */
/* - Single Step, Go, etc., in which case AF_ZOOM will be set.               */
/*   If the hotaddr is already on the display, then don't rebuild            */
/*   the tables, else move the hotaddr to the top of the display.            */
/*                                                                           */
/*                                                                           */
/* delta < 0                                                                 */
/*                                                                           */
/*  In this case, we are going to get the StartAddr and StartLineNumber      */
/*  from the first entries in the display buffers and build a margin of      */
/*  delta lines above them and then expand down below the margin.            */
/*                                                                           */
/* delta > 0                                                                 */
/*                                                                           */
/*  This is the easy one. All we have to do is get the StartAddr and         */
/*  StartLineNumber from the last entries in the current display buffers     */
/*  and then expand down from there.                                         */
/*                                                                           */
/*                                                                           */
/* The primary problem with all of the above cases is when a disassembled    */
/* line overlays the top or bottom of the display.                           */
/*                                                                           */
/*****************************************************************************/
 if( delta <= 0 )
 {
  if( delta == 0 )
  {
   if( fp->flags & AF_ZOOM )
   {
    if( IsOnAsmCRT( fp->hotaddr) )
     goto BUILDTEXT;

    TopMargin = 0;
    StartAddr = fp->topoff;
    {
     LNOTAB *pLnoTabEntry;

     DBMapInstAddr( StartAddr, &pLnoTabEntry, fp->pdf);
     StartLineNumber = 0;
     if( pLnoTabEntry )
      StartLineNumber = pLnoTabEntry->lno;
    }
   }

   else if( fp->flags & ASM_VIEW_TOGGLE )
   {
    TopMargin = fp->csrline - fp->topline;
    StartLineNumber = fp->csrline + fp->Nbias;
    StartAddr = NULL;
   }

   else if( fp->flags & ASM_VIEW_REFRESH )
   {
    goto BUILDTEXT;
   }

   else if( fp->flags & ASM_VIEW_CHANGE )
   {
    TopMargin = fp->csrline - fp->topline;
    StartLineNumber = fp->csrline + fp->Nbias;
    StartAddr = NULL;
   }
   else if( fp->flags & ASM_VIEW_NEXT )
   {
    StartAddr = fp->AsmView.StartAddr;
    StartLineNumber = fp->AsmView.StartLine;
    goto BUILDTABLE;
   }
   else if( fp->flags & ASM_VIEW_MNE  )
   {
    DBDisa(fp->mid, 0, 0, 0 );
    goto BUILDTEXT;
   }
  }

  else if( delta < 0 )
  {
   TopMargin = -delta;
   StartLineNumber = SourceLineCache[0];
   StartAddr       = AddrCache[0];
   /**************************************************************************/
   /* Check to see if we're bumping against the top of the display.          */
   /* If so, then we proceed directly to build the table. We have to         */
   /* rebuild the table even though it may have the same starting values     */
   /* because it may be expanding/contracting due to the data window.        */
   /**************************************************************************/
   if( StartLineNumber == 1 && StartAddr == NULL )
    goto BUILDTABLE;
   if ( StartAddr != NULL )
   {
    /**************************************************************************/
    /* If we come here then an executable line disassembly is overlapping     */
    /* the top of the display.  We are going to quench the TopMargin, but     */
    /* we only want to count the part of this disassembled line that will     */
    /* be above the display.                                                  */
    /*                                                                        */
    /*  - get the span ( number of disassembly lines ) for this line.         */
    /*  - get the cache index of the first address of this line.              */
    /*  - find the index of StartAddr relative to the first addr.             */
    /*  - adjust TopMargin for the partially expanded line + 1 for the        */
    /*    source line.                                                        */
    /*  - if TopMargin goes negative, then the number of lines "above"        */
    /*    the display are more than needed to quench the delta, so            */
    /*    we have to blow by the lines we don't want to finally get to        */
    /*    the StartAddr that we "really" want.                                */
    /*                                                                        */
    /**************************************************************************/
    span = DBGetAsmLines(fp,StartLineNumber,&addr,0);                    /*530*/
ReCache1:                                                                /*530*/
    idx  = DBDisa(fp->mid, addr, span, 0 );
    for(index = idx ; icache[index].instaddr != StartAddr
                    && icache[index].instaddr != ENDOFCACHE; index++ ){;}
   /**************************************************************************/
   /* We will not find the address in cache when we are looking at PLX    530*/
   /* dis assembly which has name ID in it.                               530*/
   /* To solve this Problem follow :                                      530*/
   /*   - Read the 1st instruction stream                                 530*/
   /*   - Check if it is a Jmp Instruction, If yes then get the offset.   530*/
   /*   - Call DbgetAsmlines with Offset value and get the span           530*/
   /*   - Set the addr to StartAddr we are looking for.                   530*/
   /*   - Go back to Dbdisa to recache the cache.                         530*/
   /**************************************************************************/
   if (icache[index].instaddr == ENDOFCACHE)                            /*530*/
   {                                                                    /*530*/
      uint BytesRead, Skip = 0;                                         /*530*/

      ptr = GetCodeBytes( icache[0].instaddr,(uint)2, &BytesRead );     /*530*/
      if ( *ptr++ == 0xEB )                                             /*530*/
         Skip = (uint)*(ptr) + 2;                                       /*530*/
      span = DBGetAsmLines(fp,StartLineNumber,&addr,Skip);              /*530*/
      addr = StartAddr;                                                 /*530*/
      goto ReCache1;                                                    /*530*/
   }                                                                    /*530*/
    span = index - idx + 1;
    TopMargin -= span;
    StartAddr = NULL;
    if ( TopMargin < 0 )
    {
     TopMargin++;
     for ( ; TopMargin < 0; TopMargin++,idx++ ){;}
     StartAddr = icache[idx].instaddr;
    }
   }
  }

  /****************************************************************************/
  /* At this point, we are going to determine what line number we have to     */
  /* back up to in order to quench the TopMargin.                             */
  /****************************************************************************/
  while( TopMargin > 0 && StartLineNumber > 1 )
  {
   StartLineNumber--;
   if( (fp->flags & AF_HUGE) && (StartLineNumber <= fp->Nbias) )
    pagefp( fp, StartLineNumber );
   ptr=fp->source + fp->offtab[StartLineNumber-fp->Nbias] -1 ;
   if (*ptr & LINE_OK)
    TopMargin -= DBGetAsmLines(fp,StartLineNumber,&junk,0);             /*530*/
   TopMargin--;
  }

  /***************************************************************************/
  /* If the TopMargin hits precisely at zero, then we have stopped on        */
  /* a non-executable line or a disassembled line has expanded to exactly    */
  /* fill the available space. In either case, the StartAddr = NULL.         */
  /*                                                                         */
  /* If the TopMargin < 0, then a disassembled line overlaps the top of      */
  /* the display and we have to move back down within the line to the        */
  /* correct StartAddr.                                                      */
  /*                                                                         */
  /* If TopMargin > 0, then we don't have enough lines to expand to          */
  /* be able to fill it in so we go with the StartAddr and StartLineNumber   */
  /* that we currently have. This will happen when we do something like      */
  /* pgup from line 1.                                                       */
  /*                                                                         */
  /***************************************************************************/
  if ( TopMargin == 0 && ( fp->flags & AF_SYNC ) )
   StartAddr = NULL;

  else if( TopMargin < 0 )
  {
   span = DBGetAsmLines(fp,StartLineNumber,&StartAddr,0);               /*530*/
   idx  = DBDisa(fp->mid, StartAddr , span, 0 );
   TopMargin++;
   for ( ; TopMargin < 0; TopMargin++,idx++ ){;}
   StartAddr = icache[idx].instaddr;
  }

 }

 else if( delta > 0 )
 {
  /***************************************************************************/
  /* - test for start line past end of the source line table.                */
  /***************************************************************************/
  n = 0;
  while(SourceLineCache[n] != ENDOFTABLE ) n++;
  n = (delta >= n)?0:delta;
  StartLineNumber = SourceLineCache[n];
  StartAddr       = AddrCache[n];
 }

BUILDTABLE:
/*****************************************************************************/
/* Now, we have a StartAddr and StartLineNumber to begin generating          */
/* display tables.  The display tables will look like so:                    */
/*                                                                           */
/*   SourceLineNumber        AddrCache                                       */
/*        .                     .                                            */
/*        .                     .                                            */
/*        .                     .                                            */
/*        6                     0         <--- executable line 1st entry     */
/*        6                     addr1     <--- executable line disasm entry. */
/*        6                     addr2     <--- executable line disasm entry. */
/*        6                     addr3     <--- executable line disasm entry. */
/*        7                     0         <--- non-executable line.          */
/*        8                     0         <--- non-executable line.          */
/*        9                     .                                            */
/*        .                     .                                            */
/*        .                     .                                            */
/*        -1 ( ENDOFTABLE )     .                                            */
/*                                                                           */
/*                                                                           */
/* Normally, we will enter an (lno,0) for the first entry of an executable   */
/* line and then expand the disassembly addresses below it.  But, in the case*/
/* where an executable overlaps the top of the display, we only want part of */
/* the line + disassembly.                                                   */
/*                                                                           */
/*****************************************************************************/

 TableDone = FALSE;
 addr  = StartAddr;
 lno   = StartLineNumber;
 memset( TempSourceLineCache, 0 , AsmTableSize );                       /*518*/
 memset( TempAddrCache , 0 , AsmTableSize );                            /*518*/
 for ( row = 0 ; row < AsmRows && TableDone == FALSE ; row++ )
 {
  if( (fp->flags & AF_HUGE) &&
      (lno > fp->Nbias + fp->Nlines) &&
      (lno < fp->Tlines)
    )
   pagefp( fp, lno );

  if( lno > fp->Tlines )
   break;
  ptr=fp->source + fp->offtab[lno - fp->Nbias] -1 ;
  if(*ptr & LINE_OK)
  {
   if( addr == NULL )
   {
    /*************************************************************************/
    /* If we get here, then we are precisely at the start of an executable   */
    /* line.                                                                 */
    /*                                                                       */
    /*  - make the lno,0 entry.                                              */
    /*  - map the lno and get ready to add the disassembly lines.            */
    /*                                                                       */
    /*************************************************************************/
    TempSourceLineCache[row] = lno;
    TempAddrCache[row++] = NULL;
    addr = DBMapLno(fp->mid, lno, fp->sfi, &read , fp->pdf );
   }

   /**************************************************************************/
   /* - get the index in the cache of the executable line.                   */
   /* - reduce the span of the line for expansion of a partial line.         */
   /*   ( when an executable line overlays the top of the display. )         */
   /* - expand the line with the disassembly addresses.                      */
   /* - upon entry to this loop, row is the index of the next entry          */
   /*   to be written.                                                       */
   /* - upon exit from this loop, row is one more than the last              */
   /*   entry written to the display table so we decrement it.               */
   /**************************************************************************/
   span   = DBGetAsmLines(fp, lno, &FirstAddrForLine, 0);
ReCache:
   index  = DBDisa(fp->mid, FirstAddrForLine , span, 0 );

   for(; icache[index].instaddr != addr
         && icache[index].instaddr != ENDOFCACHE; span--,index++){;}    /*530*/
   /**************************************************************************/
   /* We will not find the address in cache when we are looking at PLX    530*/
   /* dis assembly which has name ID in it.                               530*/
   /* To solve this Problem follow :                                      530*/
   /*   - Read the 1st instruction stream                                 530*/
   /*   - Check if it is a Jmp Instruction, If yes then get the offset.   530*/
   /*   - Call DbgetAsmlines with Offset value and get the span           530*/
   /*   - Set the FirstAddrForline to Current address we are looking for. 530*/
   /*   - Go back to Dbdisa to recache the cache.                         530*/
   /**************************************************************************/
   if (icache[index].instaddr == ENDOFCACHE)                            /*530*/
   {                                                                    /*530*/
      uint BytesRead, Skip = 0;                                         /*530*/

      ptr = GetCodeBytes( icache[0].instaddr,(uint)2, &BytesRead );     /*530*/
      if ( *ptr++ == 0xEB )                                             /*530*/
         Skip = (uint)*(ptr) + 2;                                       /*530*/
      span = DBGetAsmLines(fp,lno,&FirstAddrForLine,Skip);              /*530*/
      FirstAddrForLine = addr;                                          /*530*/
      goto ReCache;                                                     /*530*/
   }                                                                    /*530*/
   LastIndex = AsmRows - 1;                                             /*911*/
   count  = 0;                                                          /*911*/
   for ( ; row <= LastIndex && count < span ; index++,row++,count++ )   /*911*/
   {                                                                    /*911*/
    ULONG _address;                                                     /*911*/
    ULONG lastaddr=0;                                                   /*911*/
                                                                        /*911*/
    _address = icache[index].instaddr;                                  /*911*/
    if( _address == ENDOFCACHE )                                        /*911*/
    {                                                                   /*911*/
     ULONG LastAddrInLine;                                              /*911*/
     ULONG LineSpan;                                                    /*911*/
                                                                        /*911*/
     LastAddrInLine = DBMapLno(fp->mid,                                 /*911*/
                               lno,                                     /*911*/
                               fp->sfi,                                 /*911*/
                               &LineSpan,                               /*911*/
                               fp->pdf);
     LastAddrInLine += LineSpan;                                        /*911*/
     if( lastaddr >= LastAddrInLine )                                   /*911*/
      break;                                                            /*911*/
                                                                        /*911*/
     index = DBDisa(fp->mid, lastaddr , span, 0 );                      /*911*/
     index++;                                                           /*911*/
     _address = icache[index].instaddr;                                 /*911*/
    }                                                                   /*911*/

    TempSourceLineCache[row] = lno;
    TempAddrCache[row] = _address;
    lastaddr = _address;
   }
   /**************************************************************************/
   /* - decrement row to adjust for the overshoot.                           */
   /* - bump to the next source line.                                        */
   /* - init the addr for the next line.                                     */
   /**************************************************************************/
   row--;
   lno++;
   addr = NULL;
  }
  else
  {
   /**************************************************************************/
   /* If the line is non-executable,then come here.                          */
   /**************************************************************************/
   TempSourceLineCache[row] = lno;
   TempAddrCache[row] = NULL;
   lno++;
  }
 }
 /****************************************************************************/
 /* On a pgdn or cursor down we want to anchor the bottom line to the        */
 /* bottom of the display. So, we check to see if the display table is       */
 /* full, and if not, we make an adjustment and go back and fill it up.      */
 /*                                                                          */
 /* - so, mark the end of the table.                                         */
 /* - check to see if we have a full display.                                */
 /* - if we don't then adjust delta and redo the table.                      */
 /* - if all is ok, then copy the temp tables to the static tables.          */
 /****************************************************************************/
 TempSourceLineCache[row] = ENDOFTABLE;
 for( row=0; TempSourceLineCache[row] != ENDOFTABLE; row++ ){;}
 if( delta > 0 && row < AsmRows )
 {
  delta -= AsmRows - row;
  if( delta < 0 )
   delta = 0;
  StartLineNumber = SourceLineCache[delta];
  StartAddr       = AddrCache[delta];
  goto BUILDTABLE;
 }
 memcpy( SourceLineCache, TempSourceLineCache, AsmTableSize );          /*518*/
 memcpy( AddrCache,TempAddrCache , AsmTableSize );                      /*518*/

#if 0
/*****************************************************************************/
/* - turn this block on to dump the source line and address tables.          */
/*****************************************************************************/
{
 int n=0;

 while( SourceLineCache[n] != ENDOFTABLE )
 {
  printf("\nn=%6d lno=%d addr=%x",n,SourceLineCache[n],AddrCache[n]);fflush(0);
  n++;
 }
}
#endif

BUILDTEXT:
 /****************************************************************************/
 /*                                                                          */
 /* Set some values in the fp structure.                                     */
 /*                                                                          */
 /* - define RowsUsed - the number of display rows used in this view.        */
 /* - set the topoff in the view or 0 if no disassembly lines.               */
 /* - set the topline for the mixed view.                                    */
 /* - save the starting values for this view for possible later restore.     */
 /*                                                                          */
 /****************************************************************************/
 for( RowsUsed=0; SourceLineCache[RowsUsed] != ENDOFTABLE
      && RowsUsed<AsmRows ; RowsUsed++ ){;} /*DLJ: with windows, display
                                             window is smaller than compute window.*/
 for(row=0;
      SourceLineCache[row] != ENDOFTABLE && row<AsmRows
     && !(fp->topoff=AddrCache[row]);row++){;}

 /************************************************************************402*/
 /* we removed the setting of fp->topline.                                402*/
 /************************************************************************402*/

 fp->AsmView.StartAddr = AddrCache[0];
 fp->AsmView.StartLine = SourceLineCache[0];

 /****************************************************************************/
 /* Now build the text buffer.                                               */
 /*                                                                          */
 /* - init the text buffer to all blanks.                                    */
 /* - write in the background attributes effectively making AsmRows of       */
 /*   blank lines.                                                           */
 /* - get the cache index of the top addr in AddrCache.                      */
 /* - build source lines and asm lines adding attributes to asm lines.       */
 /* - define brkxline if for a line that has both a bkpt and is the exec     */
 /*   line.                                                                  */
 /* - terminate each line of display buffer with a \0. (To stop the display).*/
 /* - display the text buffer.                                               */
 /* - add underscores to the bkpt/exec line.                                 */
 /*                                                                          */
 /****************************************************************************/
 memset( textbuf, ' ',TextBufSize);
 cp = textbuf;

 for(row=0; row<(uint)AsmRows;++row,cp+=VideoCols+2)
   *cp = ATTR_BKPT_OK_LINE;

 brkxline = 0;
 index = -1;
 if( fp->topoff != NULL )
  index = DBDisa(fp->mid, fp->topoff, 1, 0 );

 StartLineNumber = AsmTop;                                              /*400*/
 for ( cp = textbuf, row=0; row < (int)RowsUsed; ++row, cp += VideoCols+2 )
 {
  addr  = AddrCache[row];
  if ( addr == NULL )
  {
   lno = SourceLineCache[row];
   buildasmsrc( cp+1, fp, lno );
   *cp = ATTR_SOURCE_LINE;
  }
  else
  {
   index = DBDisa(fp->mid, addr, 1, 0 );
   buildasmline( cp+1, &icache[index]);

   /**************************************************************************/
   /* - write the line display attribute in the first byte of the line.      */
   /* - make a note of the line if we have a breakpoint on an execline.      */
   /**************************************************************************/
   *cp = ATTR_BKPT_OK_LINE;
   if ( IfBrkOnAddr(AddrCache[row]) )
    *cp = ATTR_BKPT_ON_LINE;
   if( AddrCache[row] == GetExecAddr() )
   {
    if( *cp == ATTR_BKPT_OK_LINE )
     *cp = ATTR_EXEC_LINE;
    else
     *cp = ATTR_BKPT_ON_EXEC_LINE;
   }

   if( *cp == Attrib(vaXlineOn) )
      brkxline = row+1;
  }
  *(cp + VideoCols + 1) = '\0';                                         /*400*/
  putrc( StartLineNumber++, 0, cp );                                    /*400*/
 }


 /****************************************************************************/
 /* Put blank rows on the screen when the assembly lines are less than    400*/
 /* the number of AsmRows                                                 400*/
 /****************************************************************************/
 for ( row = RowsUsed ; row < AsmRows; ++row,cp += VideoCols+2)         /*400*/
 {                                                                      /*400*/
    *(cp + VideoCols + 1) = '\0';                                       /*400*/
    putrc( StartLineNumber++, 0, cp );                                  /*400*/
 }                                                                      /*400*/


 if( brkxline )
  putxb( AsmTop + brkxline - 1, '_' );

 /****************************************************************************/
 /* Scan the table of stack return addresses looking for addresses that      */
 /* may appear in the display table. If we find one, then add shading        */
 /* to that line and add the level of the function in the call stack.        */
 /****************************************************************************/
 for( n=0; n < NActFrames; ++n )
 {
  i = lindex(AddrCache, RowsUsed, (ulong)ActFaddrs[n] );
  if( i < RowsUsed )
  {
   putxb( AsmTop + i, ACTCALLSHADE );
   ndigits = sprintf(digits, "%c (-%u)", Attrib(0), n+1);
   ndigits -= 1;
   putrc(AsmTop+i, VideoCols-ndigits, digits );
  }
 }
 if(!iview) {
 if(TestBit(Reg_Display,REGS386BIT))    /* if the register display flag   400*/
    ShowvRegs();                        /* is set display the registers.  400*/
 if(TestBit(Reg_Display,REGS387BIT))    /* if the coproregister display   401*/
    ShowCoRegs();                       /* flag is set display the regs   401*/
 }
 return( 0);                            /*                                   */
}                                       /* end ascroll()                     */

/*****************************************************************************/
/* IsOnAsmCRT()                                                              */
/*                                                                           */
/* Description:                                                              */
/*   Test for an address being currently in the display table.               */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   fp        input - afile structure.                                      */
/*   addr      input - address that we're testing.                           */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   TRUE or FALSE                                                           */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   none.                                                                   */
/*                                                                           */
/*****************************************************************************/
 uint
IsOnAsmCRT( uint addr )
{
 int    row;
 if (SourceLineCache == NULL)                                           /*518*/
   return(FALSE);                                                       /*518*/
 for( row = 0;
      SourceLineCache[row] != ENDOFTABLE && row<AsmRows;
      row++ )
  if( addr == AddrCache[row])
   return( TRUE );
 return(FALSE);
}                                       /* end IsOnAsmCRT().                 */

/*****************************************************************************/
/* RefreshAsmPart()                                                       519*/
/*                                                                           */
/* Description:                                                              */
/*   Refreshs the screen with the dis assembly view from a given line        */
/*   to the bottom of the screen                                             */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   StartLine     input - Startline number on the screen.                   */
/*   RowsSkip      input - Rows to be skipped.                               */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   none.                                                                   */
/*                                                                           */
/*****************************************************************************/
void RefreshAsmPart(int StartLine, int RowsSkip)                        /*519*/
{                                                                       /*519*/
  char *cp;                                                             /*519*/
  int n,row;                                                            /*519*/
  /***************************************************************************/
  /* The disassembly view buffer has been already built, so simply scan   519*/
  /* the buffer and skip the number of rows given.                        519*/
  /* Then call putrc to start refreshing from the given start line.       519*/
  /***************************************************************************/
  cp = textbuf;                                                         /*519*/
  for(n = 0; n < RowsSkip ; ++n , cp += VideoCols + 2){;}               /*519*/
  for ( row = RowsSkip ; row < AsmRows; ++row,cp += VideoCols+2)        /*519*/
    putrc( StartLine++, 0, cp );                                        /*519*/
  if(TestBit(Reg_Display,REGS386BIT))   /* if the register display flag   519*/
     ShowvRegs();                       /* is set display the registers.  519*/
  if(TestBit(Reg_Display,REGS387BIT))   /* if the coproregister display   519*/
     ShowCoRegs();                      /* flag is set display the regs   519*/
}                                                                       /*519*/
/*****************************************************************************/
/* FreeDisasmViewBuffers()                                                813*/
/*                                                                           */
/* Description:                                                              */
/*   Frees up all the buffers allocated for disassembly.                     */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
void FreeDisasmViewBuffers()
{
 if(icache){Tfree(icache);icache=NULL;}
 if(textbuf){Tfree(textbuf);textbuf=NULL;}
 if(TempAddrCache){ Tfree(TempAddrCache);TempAddrCache=NULL;}
 if(TempSourceLineCache){ Tfree(TempSourceLineCache);TempSourceLineCache=NULL;}
 if(AddrCache){ Tfree(AddrCache);AddrCache=NULL;}
 if(SourceLineCache){ Tfree(SourceLineCache);SourceLineCache=NULL;}
}
