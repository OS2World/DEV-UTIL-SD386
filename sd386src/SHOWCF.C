/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   showcf.c                                                                */
/*                                                                           */
/* Description:                                                              */
/*   Source File Display Formatting Routines                                 */
/*                                                                           */
/*                                                                           */
/* History:                                                                  */
/*                                                                           */
/*****************************************************************************/

#include "all.h"                        /* SD86 include files                */

static int iview=0;

extern uint   TopLine;
extern uint   LinesPer;
extern uint   NoPaint;
extern uint   VideoCols;
extern uchar  VideoAtr;
extern uint   NActFrames;
extern void*  ActLines[];
extern uint   TidTab[];
extern uint   ExprAddr;
extern SCOPE  ExprScope;
extern uint   ExprMid;
extern uint   ExprLno;
extern uint   ExprTid;
extern uchar* ParseError;
extern uchar  Reg_Display;
extern UINT   FnameRow;
extern UINT   MsgRow;
#define MAXPOSLEN 19                    /* Size of "line xxxx of xxxx" field.*/

UINT DstatRow;                          /* data window status row.           */

 void
fmtscr(AFILE *fp)
{
  if( !NoPaint )
  {
      ShowData( TopLine );
      fmtfname( fp );
      fmterr( NULL );
  }
  fmttxt( fp );
}
/*****************************************************************************/
/*  fmtfname()                                                               */
/*                                                                           */
/* Description:                                                              */
/*   Format the file name row of the display.                                */
/*                                                                           */
/* Parameters:                                                               */
/*   fp        pointer to an afile structure.                                */
/*                                                                           */
/* Return:                                                                   */
/*   void                                                                    */
/*                                                                           */
/* Assumptions:                                                              */
/*  mid is valid else afile would not have been built at this point.         */
/*                                                                           */
/*                                                                           */
/*****************************************************************************/
#define FNAMELEN  60                    /*                                   */
 void                                   /*                                   */
fmtfname(AFILE *fp)                     /*                                   */
{                                       /*                                   */
 char     buffer[129];                  /* source filespec buffer            */
 char    *bp;                           /* -> to fname row buffer            */
 MFILE   *dfp;                          /* -> to debuf file info             */
 int      i;
 uchar   Clear[5] = { Attrib(00), RepCnt(00), ' ', 0 };
/*****************************************************************************/
/* We are going to build FNAMELEN characters of filename information in the  */
/* following format:                                                         */
/*                                                                           */
/* debug filespec  <source filespec>                                         */
/*                                                                           */
/* If the combined fname row info exceeds FNAMELEN characters, then it is    */
/* truncated to FNAMELEN. This may be inadequate for long pathnames.         */
/*                                                                           */
/* If the afile is for a fake module, then the filename in the afile will    */
/* be the debug filespec and the source filespec will be null.               */
/*                                                                           */
/*****************************************************************************/
 Clear[0] = Attrib( vaHelp );
 Clear[1] = VideoCols - MAXPOSLEN;
 VideoAtr = (uchar)vaInfo;
 putrc( FnameRow, 0, Clear );
#if 0
 ClrScr( FnameRow, FnameRow, vaHelp );  /* clear fname row part of display   */
#endif
 memset(buffer,0,sizeof(buffer) );      /* init buffer to null  for later    */
 if(fp->mid!=FAKEMID)                   /* concatenation.                    */
 {                                      /*                                   */
  dfp = fp->pdf->DebFilePtr;            /* establish pointer to file info    */
  strcpy(buffer,dfp->fn);               /* put debug filespec  buffer        */
  strcat(buffer," <");                  /* put in "<"                        */
 }                                      /*                                   */
 strncat(buffer,                        /* append the source filespec        */
         fp->filename+1,                /*                                   */
         fp->filename[0]                /*                                   */
        );                              /*                                   */

 if(fp->mid==FAKEMID)
  strcat(buffer," <>");                 /* put in ">"                        */
 else
  strcat(buffer,">");
                                        /*                                   */
 bp = buffer;                           /* point to buffer begin             */
 if(strlen(buffer) > FNAMELEN )         /* if too much info then             */
 {                                      /*                                   */
  bp = buffer+strlen(buffer)-FNAMELEN;  /* truncate it.                      */
  bp[0]='~';                            /* add truncation indicator on left  */
  bp[FNAMELEN] = ' ';                                                   /*701*/
  bp[FNAMELEN+1] = '\0';                                                /*701*/
 }                                      /* margin.                           */
 else                                                                   /*701*/
 if( strlen( buffer ) < FNAMELEN )                                      /*701*/
 {                                                                      /*701*/
   for( i = strlen( buffer ); i <= FNAMELEN; i++ )                      /*701*/
     buffer[i] = ' ';                                                   /*701*/
   buffer[i] = '\0';                                                    /*701*/
 }                                                                      /*701*/
 putrc( FnameRow, 0, bp );              /* display it.                       */
}                                       /* end fmtfname()                    */

/*****************************************************************************/
/*  fmtpos()                                                                 */
/*                                                                           */
/* Description:                                                              */
/*   format the "line xxxx of xxxx" field of the fname row.                  */
/*                                                                           */
/* Parameters:                                                               */
/*   fp        pointer to an afile structure.                                */
/*                                                                           */
/* Return:                                                                   */
/*   void                                                                    */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*                                                                           */
/*****************************************************************************/
  /* Note:  Column number of cursor fp->skipcols + fp->csr.col + 1 */

                                        /*                                   */
 void                                   /*                                   */
fmtpos(AFILE *fp)                       /*                                   */
{                                       /*                                   */
 uint  n;                               /* # of chars formatted(field size). */
 uchar buffer[ MAXPOSLEN+4 ];           /*                                   */
                                        /*                                   */
 if( !NoPaint )                         /*                                   */
 {                                      /* Format the buffer.                */
  n = sprintf( buffer,                  /*                                521*/
              "%cline %u of %u",        /* Use this format                   */
              Attrib(vaInfo),           /* and this attribute.               */
              fp->csrline + fp->Nbias,  /* removed a +1.                  234*/
              fp->Tlines                /* The number of lines in the file.  */
            );                          /*                                   */
  memset(buffer+n,' ',MAXPOSLEN+1-n );  /* Pad end of buffer with blanks. 100*/
  buffer[ MAXPOSLEN+1 ] = 0;            /* terminate the string.             */
  putrc( FnameRow,                      /* display the string.               */
         FNAMELEN + 1,                  /*                                   */
         buffer                         /*                                   */
       );                               /*                                   */
 }                                      /*                                   */
}                                       /* end fmtpos()                      */


 void
fmttxt( AFILE *fp )
{
 uint n;
 uint lno;
 uint i;
 uint ndigits;
 uchar flag;
 uchar *afline;
 uchar attrs[MAXPER];
 uchar digits[16];
 ushort *offtab;                        /* changed to ushort.             101*/
 uint limit;
 uint xline;
 uchar *base;
 int special;
 int xon;
 LNOTAB *pExecLnoTabEntry;

 offtab  = fp->offtab;                  /* table of 2-byte values        100*/
 limit   = fp->Nlines;

 pExecLnoTabEntry = GetExecLnoTabEntry();
 xline = 0;
 if( pExecLnoTabEntry )
  xline = pExecLnoTabEntry->lno - fp->Nbias;
 base    = fp->source;
 special = 0;
 xon     = -1;

    lno=fp->topline;
    /*************************************************************************/
    /* change lno < limit to lno <= limit.                                234*/
    /*************************************************************************/
    for( n=0 ; (n < LinesPer) && (lno <= limit) ; ++n, ++lno )          /*234*/
    {
        if( (flag = *(base + offtab[lno] - 1)) & LINE_BP )
            attrs[n] = vaBptOn;
        else if( flag & LINE_OK )
            attrs[n] = vaBptOk;
        else
            attrs[n] = vaProgram;
    }
    fp->Nshown = n;

    if( NoPaint )
        return;

    if( (fp == GetExecfp()) && ((xon = xline - fp->topline) >= 0) && (xon < (int)n) ){
        if( *(base + offtab[xline] - 1) & LINE_BP ){
            attrs[xon] = vaXlineOn;     /* breakpoint on exec line */
            special = xon + 1;
        }else
            attrs[xon] = vaXline;
    }


    if(iview!=VIEW_DONT_SHOW)
       putup( base,  offtab + fp->topline ,
        TopLine, n, fp->skipcols, attrs );
    if( n < LinesPer )
        ClrScr( TopLine + n, TopLine + LinesPer - 1, vaProgram );
    if( special )
        if(iview!=VIEW_DONT_SHOW) putxb( TopLine + special - 1, '_' );

    HiFlat(afline) = ( ushort )fp->mid; /* higher 2-bytes                 100*/
    limit = n + (lno = fp->topline + fp->Nbias);                        /*234*/
    for( n=0; lno <= limit; ++n, ++lno ){                               /*234*/
        LoFlat(afline) = ( ushort )lno; /* lower 2-bytes                  100*/
        if( (i=IndexOfMidLnoForFrames( fp->mid, lno )) < NActFrames ){  /*107*/
            if(iview!=VIEW_DONT_SHOW)
            putxb( TopLine + n, ACTCALLSHADE );
            ndigits = sprintf(digits, "%c (-%u)", Attrib(0), i+1) - 1;  /*521*/
#if 0
        DoAnnotation:
#endif
            putrc( TopLine + n, VideoCols - ndigits, digits );
            continue; }
#if 0
        if( (Nthreads > 1) && (n != xon)
         && ((i = lindex(ActLines, Nthreads, (ulong)afline)) < Nthreads)  ){
           if(iview!=VIEW_DONT_SHOW)
            putxb( TopLine + n, ACTIVESHADE );
            ndigits = buffmt(digits, "%c [%u]", Attrib(0), TidTab[i]) - 1;
            goto DoAnnotation;
        }
#endif
    }
    if(iview!=VIEW_DONT_SHOW) {
    if(TestBit(Reg_Display,REGS386BIT)) /* if the register display flag   400*/
       ShowvRegs();                     /* is set display the registers.  400*/
    if(TestBit(Reg_Display,REGS387BIT)) /* if the coproregister display   401*/
       ShowCoRegs();                    /* flag is set display the regs   401*/
    }
}


/*****************************************************************************/
/*  RefreshSrcPart()                                                      519*/
/*                                                                           */
/* Description:                                                              */
/*   Refreshs the screen with the source view from a given row, for a        */
/*   given number of rows.                                                   */
/*                                                                           */
/* Parameters:                                                               */
/*   StartRow      - Screen Row where to start repainting.                   */
/*   RowsToPaint   - Number of rows to be repainted.                         */
/*   RowsToSkip    - Number of rows to be skipped.                           */
/*                                                                           */
/* Return:                                                                   */
/*   void                                                                    */
/*                                                                           */
/*  Note: Logic is same as fmttxt except that it uses all passed variables   */
/*        rather than global variables.                                      */
/*****************************************************************************/
 void                                                                   /*519*/
RefreshSrcPart(AFILE *fp,int StartRow,int RowsToPaint,int RowsToSkip)   /*519*/
{                                                                       /*519*/
 uint n;                                                                /*519*/
 uint lno;                                                              /*519*/
 uchar flag;                                                            /*519*/
 uchar attrs[MAXPER];                                                   /*519*/
 ushort *offtab;                                                        /*519*/
 uint limit;                                                            /*519*/
 uint xline;                                                            /*519*/
 uchar *base;                                                           /*519*/
 int special;                                                           /*519*/
 int xon;                                                               /*519*/
 LNOTAB *pExecLnoTabEntry;
                                                                        /*519*/
 offtab  = fp->offtab;                                                  /*519*/
 limit   = fp->Nlines;                                                  /*519*/

 pExecLnoTabEntry = GetExecLnoTabEntry();
 xline = pExecLnoTabEntry->lno - fp->Nbias;

 base    = fp->source;                                                  /*519*/
 special = 0;                                                           /*519*/
 xon     = -1;                                                          /*519*/
                                                                        /*519*/
    lno = fp->topline + RowsToSkip ;                                    /*519*/
    for( n=0 ; (n < RowsToPaint) && (lno <= limit) ; ++n, ++lno )       /*519*/
    {                                                                   /*519*/
        if ( (flag = *(base + offtab[lno] - 1)) & LINE_BP )             /*519*/
           attrs[n] = vaBptOn;                                          /*519*/
        else                                                            /*519*/
           if ( flag & LINE_OK )                                        /*519*/
              attrs[n] = vaBptOk;                                       /*519*/
           else                                                         /*519*/
              attrs[n] = vaProgram;                                     /*519*/
    }                                                                   /*519*/
                                                                        /*519*/
    xon = xline - fp->topline - RowsToSkip;                             /*519*/
    if ( (fp == GetExecfp()) && ( (xon >= 0) && (xon < (int)n) ) )      /*519*/
    {                                                                   /*519*/
       if ( *(base + offtab[xline] - 1) & LINE_BP )                     /*519*/
       {                                                                /*519*/
          attrs[xon] = vaXlineOn;                                       /*519*/
          special = xon + 1;                                            /*519*/
       }                                                                /*519*/
       else                                                             /*519*/
          attrs[xon] = vaXline;                                         /*519*/
    }                                                                   /*519*/
    if(iview!=VIEW_DONT_SHOW)                                           /*519*/
    putup( base, offtab + fp->topline + RowsToSkip,                     /*519*/
        StartRow, n, fp->skipcols, attrs);                              /*519*/
                                                                        /*519*/
    if( n < RowsToPaint)                                                /*519*/
        ClrScr( StartRow + n, StartRow + RowsToPaint - 1, vaProgram );  /*519*/
                                                                        /*519*/
    if( special )                                                       /*519*/
        if(iview!=VIEW_DONT_SHOW)
        putxb( StartRow + special - 1, '_' );                           /*519*/
                                                                        /*519*/
    if(iview!=VIEW_DONT_SHOW) {
    if(TestBit(Reg_Display,REGS386BIT)) /* if the register display flag   519*/
       ShowvRegs();                     /* is set display the registers.  519*/
    if(TestBit(Reg_Display,REGS387BIT)) /* if the coproregister display   519*/
       ShowCoRegs();                    /* flag is set display the regs   519*/
    }
}                                                                       /*519*/

    AFILE *
locatefp(uchar *lp,uint col)
{
    AFILE *fp = NULL;
    uchar symbol[ MAXSYM+1 ];


    if( !token(lp, col, symbol) )
        fmterr( "Cursor must be on a function name");
    else if( !(fp = FindFuncOrAddr(symbol,FALSE) ) )
        fmt2err( "Can't find", symbol );
    return( fp );
}

/*****************************************************************************/
/*  token()                                                                  */
/*                                                                           */
/* Description:                                                              */
/*   display a variable.                                                     */
/*                                                                           */
/* Parameters:                                                               */
/*   lp        input - pointer to block of text with the text line.          */
/*   col       input - column were the token begins.                         */
/*   tp        output- buffer where this routine stuffs the token.           */
/*                                                                           */
/* Return:                                                                   */
/*             number of characters in the token                             */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*                                                                           */
/*****************************************************************************/
 uint
token(uchar *lp,uint col,uchar *tp)     /* start of text line                */
                                        /* line offset to 1st char           */
                                        /* buffer for token                  */
{
 uchar *cp;
 uchar *op;
 uint   n;                              /* an index                          */
 uchar  line[MAXCOLS+1];                /* extracted line                    */


/*****************************************************************************/
/* The first thing we do here is extract the real text line from the raw     */
/* piece of source buffer passed.                                            */
/*                                                                           */
/*****************************************************************************/
 n=Decode(lp,line);                     /* extract the text line from the    */
                                        /* source buffer. n=# chars in line. */
 line[n]=0;                             /* make this line a Z string.        */

/*****************************************************************************/
/* The line is scanned backwards from the start of the token and the token   */
/* pointer is backed up to the first non valid character. This will be n=0   */
/* if the token starts at the beginning of a line, else it will be one.      */
/*                                                                           */
/* I don't know why this code is necessary!!!!                               */
/*****************************************************************************/
 for( n=0, cp = line + col;             /* scan the line backwards           */
      ;                                 /*                                   */
      --cp                              /*                                   */
    )                                   /*                                   */
 {                                      /*                                   */
  if( !IsOKchar(*cp) )                  /* if this is an invalid character,  */
      break;                            /* then quit.                        */
  if( cp == line )                      /*                                   */
  {                                     /* if at line begin, then            */
      n = 0;                            /*                                   */
      break;                            /* we won't back up any chars and    */
  }                                     /* quit.                             */
  n = 1;                                /* backed up one char                */
 }                                      /*                                   */
                                        /*                                   */
/*****************************************************************************/
/* Here we copy the token from the line buffer to the callers buffer and tell*/
/* him how many chars we copied.                                             */
/*                                                                           */
/*                                                                           */
/*****************************************************************************/
 for( op = tp, cp += n;                 /* initialize line and caller buffers*/
      IsOKchar(*cp) &&                  /* stay in loop til end of token or  */
      (tp - op) < MAXSYM;               /* we exceed max token length.       */
    )                                   /*                                   */
 {                                      /*                                   */
  if( *cp == 0x15 )                     /* map  to @ for the italians.      */
   *cp = '@';                           /*                                   */
  *tp++ = *cp++;                        /* copy ok char to caller buf & bump */
 }                                      /*                                   */
 *tp = 0;                               /* make Z string out of token        */
                                        /*                                   */
 return( tp - op );                     /* # of chars in token (0..N)        */
}                                       /* end token()                       */
/*****************************************************************************/
/*  dumpvar()                                                                */
/*                                                                           */
/* Description:                                                              */
/*   display a variable.                                                     */
/*                                                                           */
/* Parameters:                                                               */
/*   fp        the afile for this variable.                                  */
/*   lp        the start of the text line.                                   */
/*   line      source file line number.                                      */
/*   col       source file column.                                           */
/*   func      show variable or show what the variable points to selector.   */
/*               SHOWVAR          = show the contents of a variable          */
/*               SHOWVARPTSTO     = show contents var points to              */
/*               PUTVARINSTG      = put a variable in the storage win        */
/*               PUTVARPTSTOINSTG = put what variable points to in stg       */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   dfp                                                                     */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*                                                                           */
/*****************************************************************************/
DFILE * dumpvar(AFILE *fp,
                UCHAR *lp,
                UINT   line,
                UINT   col,
                UINT   func,
                AFILE **fpp)
{
 DFILE  d;                              /* a DFILE to put the variable in    */
 DFILE *dfp;                            /* appended dfile node.              */
 uint   deref;                          /* 0=variable value                  */
                                        /* 1=show what variable points to    */
 uint   sfx;                            /* stack frame index for stack vars  */
 uchar *cp;                             /*                                   */
 uchar  symbol[MAXSYM+4];               /*                                   */
 uchar  buffer[MAXSYM+25];              /*                                   */
 uint   nchars;                         /* number of chars in token          */
 uchar *procname;                       /* procedure name                    */
 int    rcshower;                       /* return code from the shower.      */
 int    n;                              /* just an integer.                  */
 uint   rc;                             /* a return code.                    */
 AFILE  *newfp;

/*****************************************************************************/
/* The first thing to do is determine whether the user wants to show the     */
/* variable or what the variable points to.                                  */
/*                                                                           */
/*****************************************************************************/
 deref=0;                               /* assume values to be displayed     */
 symbol[0]=' ';                         /*                                   */
 if ( func == SHOWVARPTSTO ||           /* if user asked for "*" then        */
      func == PUTVARPTSTOINSTG          /*                                   */
    )                                   /*                                   */
 {                                      /*                                   */
  deref=1;                              /* set a flag and prefix the symbol  */
  symbol[0] = '*';                      /* with "contents of" operator       */
 }                                      /*                                   */
/*****************************************************************************/
/* Now we have to tokenize what the cursor points to and verify that it is   */
/* a valid identifier. Then we verify that the identifier is a variable.     */
/*                                                                           */
/*****************************************************************************/
 nchars=token(lp, col, symbol+deref);   /* Tokenize.                         */
 if(nchars == 0 )
 {
  symbol[0] = 't';
  symbol[1] = 'h';
  symbol[2] = 'i';
  symbol[3] = 's';
  symbol[4] = '\0';
 }
#if 0
 if(!nchars)                            /* If not a valid token, then        */
 {                                      /*                                   */
  fmterr("Cursor must be on a variable name");  /*                        701*/
  return(NULL);                         /* return with bad name.             */
 }                                      /*                                   */
#endif
 cp=ParseExpr(symbol, 0x10, fp->mid, line, fp->sfi);

 if( !cp || *cp )                       /* If it's not a variable we know    */
 {                                      /*                                   */
  fmterr(ParseError);                   /* return error in parse expressio   */
  return(NULL);                         /*                                   */
 }                                      /*                                   */
                                        /*                                   */
 fmterr( "");                           /* proceed with good variable name   */
/*****************************************************************************/
/* At this point ExprScope has been established by ParseExpr(). ExprScope    */
/* points to the SSproc record in the symbols area for the function that     */
/* contains the variable. sfx is the index of the stack frame that contains  */
/* the variable. If the variable is not in an active stack frame, then       */
/* sfx=0. ExprScope not NULL implies a stack variable.                       */
/*                                                                           */
/*****************************************************************************/
 sfx=StackFrameIndex(ExprScope);        /* Get sfx for this variable.        */
 if( ExprScope && !sfx )                /* If stack variable but not active, */
 {                                      /*                                   */
  procname=CopyProcName(ExprScope,      /* then copy function name over the  */
                        symbol,         /* symbol name buffer and            */
                        sizeof(symbol)  /*                                   */
                       );               /*                                   */
                                        /*                                   */
  sprintf( buffer,                      /* build a  "function not active" 521*/
          "\"%s\" not active",          /* message.                          */
          procname);                    /*                                   */
                                        /*                                   */
  fmterr( buffer );                     /* format the error for later display*/
  return(NULL);                         /* ret null if func is not active 300*/
 }                                      /*                                   */
 else                                   /*                                   */
 {                                      /*                                   */
/*****************************************************************************/
/* At this point, the symbol is a one that we know about, and if it's a      */
/* stack variable, then it's in an active stack frame. So, now we build a    */
/* DFILE structure for it.                                                   */
/*                                                                           */
/*****************************************************************************/
  memset( &d, 0, sizeof(d) );           /* clear the DFILE structure.     100*/
                                        /* now add structure members:        */
  memcpy( d.expr,symbol,sizeof(d.expr) );/*    symbol name in the expr fld100*/
  d.mid = ExprMid;                      /*     mid containing the symbol     */
  d.lno = ExprLno;                      /*     line number symbol is on      */
  d.sfi = fp->sfi;                      /*     line number symbol is on      */
  d.sfx = sfx;                          /*     stack frame index             */
  d.scope = ExprScope;                  /*     scope if stack variable       */
  d.datatype = ExprTid;                                              /*813512*/

  {
   Trec        *tp;
   TD_TYPELIST *pClassItemList;
   USHORT       FirstItemInList;
   USHORT       ItemListIndex;

   tp = (Trec *)QtypeRec(ExprMid, ExprTid );
   if( tp && (tp->RecType == T_CLASS) )
   {
    ItemListIndex    = ((TD_CLASS*)tp)->ItemListIndex;
    pClassItemList   = (TD_TYPELIST*)QtypeRec(ExprMid, ItemListIndex);
    FirstItemInList  = pClassItemList->TypeIndex[0];
    if( FirstItemInList == 0 )
     d.DfpFlags.ClassType = DFP_BASE_CLASS;
    else
     d.DfpFlags.ClassType = DFP_DERIVED_CLASS;
   }
  }
                                        /* Get primitive typeno in case   512*/
                                        /* of primitive user defs.        512*/
  d.baseaddr = ExprAddr;                /*     base address for data block.  */
  SetShowType( &d, d.datatype );        /*     "show" function pointer.      */
  n = SetShowLines( &d );               /* set num of node display lines.    */

  if (n == 0)                           /* if length of data = 0          223*/
   {                                    /* search for symbol in globals   223*/
    cp=ParseExpr(symbol, 0x20, fp->mid, line, fp->sfi);
    d.datatype = ExprTid;                                            /*813512*/
                                        /* Get primitive typeno in case   512*/
                                        /* of primitive user defs.        512*/
    d.mid = ExprMid;                    /* and mid containing the global  223*/
    SetShowType( &d, d.datatype );      /* sym."show" function pointer.   223*/
    n = SetShowLines( &d );             /* set num of node display lines. 223*/
   }

  if( func == PUTVARINSTG ||            /* if the variable is destined for   */
      func == PUTVARPTSTOINSTG          /* the storage window, then          */
    )                                   /*                                   */
  {                                     /*                                   */
   dfp=AppendDataFile( &d );            /* append to the data file.          */
   return( dfp );                       /* returndone.                       */
  }                                     /*                                   */
/*****************************************************************************/
/* if func = SHOWVAR || SHOWVARPTSTO then we will display the data as we     */
/* always did on the message line.                                           */
/*                                                                           */
/*****************************************************************************/
  if( func == SHOWVAR ||                /* if the variable is destined for   */
      func == SHOWVARPTSTO              /* message line, then                */
    )                                   /*                                   */
  {                                     /*                                   */
   rcshower=(* d.shower)(&d,            /*  show the data and                */
                         MsgRow,        /*  overlay the message row.         */
                         1,             /*  show one data record.            */
                         0              /*  don't skip any records.          */
                        );              /*                                   */
                                        /*                                   */
   if( rcshower == FALSE )              /*  if can't show then beep          */
    beep();                             /*                                   */
  }
#if 0
  else if( func == MSHGET )             /* if the variable is destined for   */
  {                                     /*                                   */
   rcshower=(* d.finder)(&d,NULL,       /*  show the data and                */
                         NULL);         /*  NULL => mshput_direct.           */
                                        /*                                   */
   if( rcshower == FALSE )              /*  if can't show then beep          */
    beep();                             /*                                   */
  }
  else if( func == MSHPUT )             /* if the variable is destined for   */
  {                                     /*                                   */
   extern uchar       Re_Parse_Data;    /* flag to indicate wether all    244*/
   rcshower=(* d.finder)(&d,NULL,       /*  show the data and                */
             (MSHOBJECT *)-1);          /*  -1   => mshget_direct.           */
   {                                                               /*903*/
    extern int DataRecalc;                                         /*903*/
    extern uchar       Re_Parse_Data;
                                                                   /*903*/
    DataRecalc = TRUE;                                             /*903*/
    Re_Parse_Data=TRUE;                 /*                                   */
   }                                                               /*903*/
   Re_Parse_Data=TRUE;                  /*                                   */
   if( rcshower == FALSE )              /*  if can't show then beep          */
    beep();                             /*                                   */
  }
#endif
/********************************************c********************************/
/* if func = EXPANDVAR then we will display it in the expnsion window        */
/* without putting in the data file ring.  In this scenario the parent node  */
/* (dfp) is the node that we have built on the stack.                        */
/*                                                                           */
/*****************************************************************************/
  dfp = &d;                             /* define -> to temporary parent node*/
  if( func == EXPANDVAR                 /* if the variable is going to the   */
    )                                   /* expansion window then ...         */
  {                                     /*                                   */

#ifdef MSH
   VideoBuffer *videoBuffer=SaveVideoBuffer();
#endif

   DstatRow = TopLine;                  /* establish status line (0..N).     */
   newfp = NULL;
   rc=zoomrec( fp, dfp, 0, &newfp );
   if( newfp )
   {
    *fpp = newfp;
    return(0);
   }
   if(rc && rc!=ESC)                    /* back all the way out on ESC.      */
   {                                    /*                                   */
    beep();                             /*                                   */
    putmsg("can't expand this data");   /*                                   */
   }                                    /*                                   */
   dovscr( fp, 0 );                     /* refresh source part of the screen.*/
#ifdef MSH
#if 0
   dovscr( fp, 0 );                     /* refresh source part of the screen.*/
#else
   RestoreVideoBuffer(videoBuffer);
#endif
#endif
  }                                     /*                                   */
 }                                      /*                                   */
 return(NULL);                          /* keep the compiler happy.          */
}                                       /* end dumpvar()                     */
