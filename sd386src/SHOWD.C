/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   showd.c                                                                 */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*  data display routines.                                                   */
/*                                                                           */
/*                                                                           */
/* History:                                                                  */
/*                                                                           */
/*   02/08/91 Creation of 32-bit SD86, from 16-bit version.                  */
/*                                                                           */
/*...16->32 port.                                                            */
/*...                                                                        */
/*... 02/08/91  101   Joe       port to 32 bit.                              */
/*... 02/08/91  107   Dave      port to 32 bit.                              */
/*... 02/08/91  112   Joe       port to 32 bit.                              */
/*                                                                           */
/*...Release 1.00 (Pre-release 1)                                            */
/*...                                                                        */
/*... 07/09/91  203   srinivas  ptr->ptr hangs in the data window            */
/*                              (POINTERTEW testcase)                        */
/*... 07/09/91  205   srinivas  Hooking up of register variables.            */
/*... 07/15/91  212   srinivas  Bit Fields not being displayed.              */
/*... 07/26/91  219   srinivas  handling near pointers.                      */
/*... 08/05/91  221   srinivas  Hooking up registers and constants in data   */
/*                              window.                                      */
/*... 08/14/91  215   Christina add support for HLL format                   */
/*... 08/21/91  233   Christina fix TYPEDEFs in HLL format                   */
/*... 09/16/91  239   Srinivas  Void ptr handling for cl386 & Toroc.         */
/*... 09/25/91  242   Srinivas  PLX bit fields not being shown properly.     */
/*... 09/25/91  243   Srinivas  ISADDREXPR global flag problems.             */
/*... 09/26/91  244   Srinivas  optimise calls to parse expr.                */
/*...                                                                        */
/*...Release 1.00 (Pre-release 105 10/10/91)                                 */
/*...                                                                        */
/*... 10/23/91  307   Srinivas  Trap when a pointer points to a long string  */
/*...                                                                        */
/*...Release 1.00 (Pre-release 1.08 10/10/91)                                */
/*...                                                                        */
/*... 12/27/91  502   Srinivas  Simple PL/X based vars display the name but  */
/*...                           have no value associated with them.          */
/*... 02/07/92  512   Srinivas  Handle Toronto "C" userdefs.                 */
/*... 02/12/92  521   Srinivas  Port to C-Set/2.                             */
/*... 02/13/92  522   Srinivas  Size of bit fields in cl386 is not correct.  */
/*... 02/13/92  523   Srinivas  Handle Toronto "C" Bitfields.                */
/*...                                                                        */
/*...Release 1.01 (04/03/92)                                                 */
/*...                                                                        */
/*... 05/08/92  701   Srinivas  Cua Interface.                               */
/*...                                                                        */
/*...Release 1.02 (10/22/92)                                                 */
/*...                                                                        */
/*... 01/26/93  809   Selwyn    HLL Level 2 support.                         */
/*... 03/03/93  813   Joe       Revised types handling for HL03.             */
/*...                           (Moved FormatdataItem to it's own file.)     */
/*... 03/06/93  814   Joe       Signed 8 bit char not displaying correct     */
/*...                           negative.                                    */
/**Includes*******************************************************************/

#include "all.h"                        /* SD86 include files                */

/**Defines *******************************************************************/

#define SPCTAB 12                       /* space between dec & hex display   */

/**External declararions******************************************************/

extern uint        DataFileTop;         /* 1st rec shown of data file (0..N) */
extern uint        DataFileBot;         /* last rec shown of data file (0..N)*/
extern DFILE*      DataFileQ;           /*                                   */
extern uint        ExprAddr;            /* Set by ParseExpr-Addr val of ex101*/
extern SCOPE       ExprScope;           /* Set by ParseExpr-scope for expr   */
extern uint        VioStartOffSet;      /* flag to tell were to start screen */
                                        /* display.                       701*/

/**Static definitions ********************************************************/

static  struct {                        /*                                   */
  uchar a;                              /*                                   */
  uchar t[26];                          /*                                   */
} CantEvalMsg =                         /* message issued when dfile expr    */
            {Attrib(vaStgVal),          /* cannot be evaluated.              */
             "Can't evaluate expression"/*                                   */
            };                          /*                                   */
                                        /*                                   */
static  uchar BadAddrMsg[] =            /*                                   */
            "Invalid Address";          /*                                   */

       uchar  Re_Parse_Data = FALSE;    /* flag to indicate wether all    244*/
                                        /* variables in datawindow are to 244*/
                                        /* be reparsed.                   244*/

/*****************************************************************************/
/*  ShowData()                                                               */
/*                                                                           */
/* Description:                                                              */
/*   display a variable.                                                     */
/*                                                                           */
/* Parameters:                                                               */
/*   Nrows     the number  of rows available in the data window.             */
/*                                                                           */
/* Return:                                                                   */
/*   void                                                                    */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/* Note: This code assumes "next" is the 1st field in the DFILE struct       */
/*                                                                           */
/*****************************************************************************/
 void                                   /*                                   */
ShowData( uint Nrows )                  /*                                   */
{                                       /*                                   */
 uint    n;                             /* just a integer.                   */
 uint    rec;                           /* current data file record (0..N)   */
 uint    row;                           /* current screen row (0..N)         */
 uint    col;                           /* data window column.               */
 int    incr = 0;                       /* # screen rows to used by the data */
                                        /* object which is the same as the521*/
                                        /* record consumed in the data file. */
 DFILE *dfp;                            /* -> dfile node to display.         */
 DFILE *dfpx;                           /* -> to lookahead dfile node.       */
 DFILE *dfpfirst;                       /* -> to first dfile node to display.*/
 uchar *cp;                             /* vestigial expr from ParseExpr.    */
 uchar *msg;                            /* -> to a message buffer.           */
 uchar  msgbuf[ MAXSYM+1 + 25 ];        /* the message buffer.               */
 uchar  buffer[ MAXSYM+1 ];             /* another buffer for procedure name.*/
 uint   maxdatarec;                     /* last rec of data file display(0-N)*/
 uint   numnodes;                       /* number of nodes to display.       */
 uint   nodecntr;                       /* display node index.               */
 int    rc;                             /* return code.                      */
 uchar *procname;                       /* procedure name.                   */
 int    skip;                           /* # of items of data object to skip */
                                        /* before the display.               */
/*****************************************************************************/
/* Let's handle some special cases first.                                    */
/*  1. No dfile nodes and Nrows is not zero so init a data window and a file.*/
/*     The data window will have Nrows display lines and the data file will  */
/*     have Nrows data records.                                              */
/*                                                                           */
/*  2. some dfile nodes but Nrows is zero.                                   */
/*****************************************************************************/
 dfpfirst = (DFILE *)&DataFileQ;        /* ->to dfile ring pointer.          */
 dfpx = dfpfirst->next;                 /* ->to lookahead node.              */
 if( !dfpx )                            /* if empty dfile ring and           */
 {                                      /*                                   */
  if( Nrows - VioStartOffSet )          /* if a data window has not been  701*/
  {                                     /* initialized, then initialize one. */
   ClrScr(VioStartOffSet,Nrows-1,vaStgExp);                             /*701*/
                                        /* window size = Nrows.              */
   if(!DataFileTop)                     /*                                   */
    DataFileBot = Nrows - 1;            /*                                   */
  }                                     /*                                   */
  Re_Parse_Data = FALSE;                /* Reset the parse var flag       244*/
  return;                               /*                                   */
 }                                      /*                                   */
 else if( !Nrows )                      /* if dfile ring is not empty but    */
 {                                      /*                                244*/
  Re_Parse_Data = FALSE;                /* Reset the parse var flag       244*/
  return;                               /* there is no data window, just ret.*/
 }                                      /*                                244*/
/*****************************************************************************/
/* First we want to find the first node that falls in the data window.       */
/* We know that there is at least one node in the dfile ring at this point   */
/* and dfpx currently points to it.                                          */
/*****************************************************************************/
 for(;                                  /* scan ring beginning with 1st node.*/
     dfpx;                              /* quit at the ground node.          */
     dfpx = dfpx->next                  /* next node.                        */
    )                                   /*                                   */
 {                                      /*                                   */
  if(dfpx->lrn <= DataFileTop)          /* if this node is before the data   */
  {                                     /* window and there aren't any more  */
   if(!dfpx->next)                      /* nodes then we're done.            */
    break;                              /*                                   */
   if(dfpx->next->lrn > DataFileTop)    /* if the next node is past the      */
     break;                             /* start of the data window then     */
  }                                     /* we're also done.                  */
  else if(dfpx->lrn <= DataFileTop +    /* if the node falls inside the data */
                      Nrows - 1 )       /* window then we're done.           */
   break;                               /*                                   */
 }                                      /* Otherwise, continue for loop.     */
/*****************************************************************************/
/* After we exit from this loop, we may have a null dfpfirst meaning that    */
/* there are no nodes to display in the window.                              */
/*****************************************************************************/
 dfpfirst = dfpx;
 if(!dfpfirst)                          /* if empty window then              */
 {                                      /*                                   */
  ClrScr(VioStartOffSet,Nrows-1,vaStgExp);/* clear the current window and 701*/
  Re_Parse_Data = FALSE;                /* Reset the parse var flag       244*/
  return;                               /* return.                           */
 }                                      /*                                   */
/*****************************************************************************/
/* Now let's find the number of nodes to display info for. At this point,    */
/* we know that there is a node in the dfile ring, but we don't know how     */
/* many to display.                                                          */
/*****************************************************************************/
 numnodes = 0;                          /* one node for sure at this point.  */
 maxdatarec = DataFileTop + Nrows - 1;  /* bottom of window in the data file.*/
 for ( dfpx = dfpfirst;                 /* start at first node.   .          */
       dfpx &&                          /*                                   */
       dfpx->lrn <= maxdatarec;         /* done when lookahead out of window.*/
       dfpx = dfpx->next,               /* update lookahead node.            */
       numnodes++
     ){;}
/*****************************************************************************/
/* Now let's display the data. Again, at this point, we know there is at     */
/* least one node to display.                                                */
/*                                                                           */
/* NOTE:                                                                     */
/* Each data object takes up incr lines in the data window and rec records in*/
/* the data file.  This is a function of the difference between lrns of      */
/* successive nodes. We have to remember that there may be gaps between nodes*/
/* caused by ghost nodes of erase data objects.                              */
/*                                                                           */
/* NOTE:                                                                     */
/* Normally the display row will be initialized to 0. However, this is not   */
/* always the case as the user may enter an object anywhere in the window.   */
/* Also, if the first node is erased then its ghost is left in the data file.*/
/*                                                                           */
/*****************************************************************************/
 row = dfpfirst->lrn - DataFileTop;     /* compute first display row.        */
 if((int)row < (int)VioStartOffSet)     /* can't be less than 0 though.   701*/
  row = VioStartOffSet;                 /*                                701*/
 if(row-VioStartOffSet)                 /* if it's not the first row,then 701*/
  ClrScr(VioStartOffSet,row-1,vaStgExp);/* erase area prior to disp start 701*/
 rec=DataFileTop + row;                 /* start at first data file record.  */
 nodecntr = 1;                          /*                                   */
 dfp = dfpfirst ;                       /* start at the first node.          */
 for( ;                                 /*                                   */
      nodecntr <= numnodes;             /* quit after all nodes processed.   */
      nodecntr++,                       /* bump the node count.              */
      row += incr,                      /* bump display row.                 */
      rec += incr,                      /* bump data file row.               */
      dfp = dfpx )                      /* -> to next node                   */
 {                                      /*                                   */
/*****************************************************************************/
/* At this point we're going to display the node info, but we have to compute*/
/* the increment of display for this node.                                   */
/*****************************************************************************/
  dfpx= dfp->next;                      /* -> to lookahead node.             */
  incr = (dfpx)?dfpx->lrn - rec:MAXINT; /* compute the increment.            */
  if( incr > (int)(Nrows - row) )       /* truncate increment if too big.    */
   incr = Nrows - row;                  /*                                   */

 if (Re_Parse_Data)                     /*if flag is set reparse all vars 244*/
 {                                      /*                                244*/
/*****************************************************************************/
/* Now we want to parse the expression and update the base address.          */
/*****************************************************************************/
  cp = ParseExpr(dfp->expr,             /* the node expression.              */
                 0x10,                  /* use optional source line context. */
                 dfp->mid,              /* module id.                        */
                 dfp->lno,              /* source line number.               */
                 dfp->sfi               /* source file index.                */
                );                      /*                                   */
  if( !cp || *cp )                      /* if can't evaluate the expression, */
  {                                     /*                                   */
   msg = (uchar*) &CantEvalMsg;         /* then display a bad expression msg.*/
   ClrScr(row,row+incr-1,vaStgExp);     /*                                   */
   putrc( row, 0, dfp->expr );          /*                                   */
   n = strlen(dfp->expr);               /*                                   */
   col=(n > (STGCOL-1) ) ? n+2:STGCOL-1;/*                                   */
   putrc( row, col , msg );             /*                                   */
   continue;                            /*                                   */
  }                                     /*                                   */
  dfp->baseaddr = ExprAddr;             /* update -> to the data block.      */
  dfp->scope    = ExprScope;            /* update -> to the data block.      */
 }                                      /*                                244*/

/*****************************************************************************/
/* Now we want to establish the stack frame of the expression if there is    */
/* one.                                                                      */
/*****************************************************************************/
  dfp->sfx=StackFrameIndex(dfp->scope); /* get the stack frame index.        */
  if( dfp->scope && !dfp->sfx )         /* if this is a stack variable but   */
  {                                     /* it's not active, then display     */
   msg = msgbuf;                        /* a " not active" message.          */
   memset(buffer, 0, sizeof(buffer));
   procname=CopyProcName(dfp->scope,    /*                                   */
                         buffer,        /*                                   */
                         sizeof(buffer) /*                                   */
                        );              /*                                   */
   sprintf( msg,                        /*                                521*/
           "%c\"%s\" not active",       /*                                   */
           Attrib(vaStgVal),            /*                                   */
           procname                     /*                                   */
         );                             /*                                   */
   ClrScr(row,row+incr-1,vaStgExp);     /*                                   */
   putrc( row, 0, dfp->expr );          /*                                   */
   n = strlen(dfp->expr);               /*                                   */
   col=(n > (STGCOL-1) ) ? n+2:STGCOL-1;/*                                   */
   putrc( row, col , msg );             /*                                   */
   continue;                            /*                                   */
  }                                     /*                                   */
/*****************************************************************************/
/* Now display the node info.                                                */
/*****************************************************************************/
  skip = rec - dfp->lrn;
  rc =(* dfp->shower) (dfp,             /* -> to dfile node.                 */
                       row,             /* display row to start display.     */
                       incr,            /* amount of rows to display.        */
                       skip
                      );                /*                                   */
                                        /*                                   */
  if( rc == FALSE )                     /* if can't show, then clear area.   */
   ClrScr(row,row+incr-1,vaStgExp);     /*                                   */
 }                                      /* end for loop.                     */
 Re_Parse_Data = FALSE;                 /* Reset the parse var flag       244*/
}                                       /* end ShowData().                   */

/*****************************************************************************/
/*  DFILE *dfp;                                                              */
/*  uint row;      screen row                                                */
/*  uint rows;     # of screen rows                                          */
/*  uint skip;     # of logical data file records to skip                    */
/*                                                                           */
/*****************************************************************************/
 uint
ShowScalar( DFILE *dfp, uint row, uint rows, uint skip )
{
  uchar buffer[DATALINESPAN];

  uint   baseaddr;                                                      /*101*/
  baseaddr = dfp->baseaddr;


    if( skip )
        return( FALSE );
    InitDataBuffer( buffer, sizeof(buffer), dfp );
    FormatDataItem( buffer + STGCOL+1, baseaddr,
        dfp->mid, dfp->showtype, dfp->sfx );
    putrc( row, 0, buffer );
    if( rows > 1 )
        ClrScr( row + 1, row + rows - 1, vaStgExp );
    return( TRUE );
}
/*****************************************************************************/
/*  DFILE *dfp;                                                              */
/*  uint row;      screen row                                                */
/*  uint rows;     # of screen rows                                          */
/*  uint skip;     # of logical data file records to skip                    */
/*                                                                           */
/*****************************************************************************/
 uint
ShowConstant( DFILE *dfp, uint row, uint rows, uint skip )
{
    uint n;                             /* was register.                  112*/
    uchar buffer[DATALINESPAN];
    uchar *cp = buffer + STGCOL+1;
    static uchar ConstValueMsg[] = "  (Constant)";
    uint addr;                          /*                                112*/
    uint sfx;                           /* stack frame index.             112*/

    if( skip )
        return( FALSE );

    InitDataBuffer( buffer, sizeof(buffer), dfp );
    addr = dfp->baseaddr;
    if(dfp->datatype == ADDRESS_CONSTANT)                               /*243*/
    {                                                                   /*243*/
     if( TestBit( addr , STACKADDRBIT) )                                /*112*/
     {                                                                  /*112*/
       sfx =  StackFrameIndex( dfp->scope );                            /*112*/
       addr = StackBPRelToAddr( addr , sfx );                           /*112*/
     }                                                                  /*112*/
    }                                                                   /*243*/

    n = utoa( addr, cp );               /*                                221*/
    *(cp+n) = ' ';                      /* remove the null terminator.    221*/
    n = sprintf((cp+SPCTAB), "%#lx", addr) ;  /* print the val in hex     221*/
    *(cp+SPCTAB+n) = ' ';               /* remove the null terminator.    221*/
    n = 2 * SPCTAB ;                    /* calculate the new pointer.     221*/

    memcpy(cp+n, ConstValueMsg,  sizeof(ConstValueMsg)-1 );             /*101*/

    putrc( row, 0, buffer );
    if( rows > 1 )
        ClrScr( row + 1, row + rows - 1, vaStgExp );
    return( TRUE );
}
/*****************************************************************************/
/*  DFILE *dfp;                                                              */
/*  uint row;      screen row                                                */
/*  uint rows;     # of screen rows                                          */
/*  uint skip;     # of logical data file records to skip                    */
/*                                                                           */
/*****************************************************************************/
 uint
ShowHexBytes( DFILE *dfp, uint row, uint rows, uint skip )
{
    uint    n;
    uint    rr;
    uchar  *cp;
    uchar  *data;
    uint    nbytes;
    uint    limit=0;
    uint    used;
    uint    sfx = dfp->sfx;              /*                               101*/
    uint    dp;                          /*                               101*/
    uchar   buffer[DATALINESPAN];

 uint   baseaddr;                       /*  replaces ExprAddr             101*/

 baseaddr = dfp->baseaddr;              /*  replaces ExprAddr                */

    dp = baseaddr + 16*skip;            /*                                101*/

    if( !skip && ((GetAppData(baseaddr, 1, &nbytes, sfx) == NULL) ||
                  nbytes == 0) )
    {
        InitDataBuffer( buffer, sizeof(buffer), dfp );
        memcpy( buffer + STGCOL+1,BadAddrMsg , sizeof(BadAddrMsg)-1 );  /*101*/
        putrc( row, 0, buffer );
        rr = 1;
    }else{
        if( dfp->showtype )
            limit = QtypeSize(dfp->mid, dfp->showtype);
        if( skip )
            dfp = NULL;
        for( rr=0 ; rr < rows ; ){
            n = 16;
            if( limit && ((used = 16*(skip+rr+1)) > limit) )
                if( ((n = limit - (used-16)) == 0) || (n > limit) )
                    break;
            if( ( (data = GetAppData(dp, n, &nbytes, sfx)) == NULL ) ||
                  nbytes == 0 )
                break;
            InitDataBuffer( buffer, sizeof(buffer), dfp );  dfp = NULL;
            for( n=0, cp = buffer + STGCOL+1; n < nbytes; ++n, cp += 3 ){
                utox2(data[n], cp);  if( (n & 3) == 3 )  cp += 1;  }

            for( n=0, cp = buffer + ASCIICOL+1; n < nbytes; ++n ){
                *cp++ = graphic(data[n]);  }
            putrc( row + rr, 0, buffer );
            rr += 1;
            if( OffOf(dp) > MAXUINT-16 )
                break;
            dp += 16;
    }   }
    if( rr < rows )
        ClrScr( row + rr, row + rows - 1, vaStgExp );
    return( TRUE );
}
    uchar
graphic( uchar c )
{
 if ( c == 0 )
  return( ACHAR_NULL );

 if ( c < 0x20 || c > 0x7F )
  return( ACHAR_NONA );

 return( c );
}

/*****************************************************************************/
/* InitDataBuffer()                                                          */
/*                                                                           */
/* Description:                                                              */
/*   Initialize a line data buffer for the data window.                      */
/*                                                                           */
/* Parameters:                                                               */
/*   cp        input - pointer to data buffer.                               */
/*   sizecp    input - size of the data buffer.                              */
/*   dfp       input - pointer to dfile node.                                */
/*                                                                           */
/* Return:                                                                   */
/*   TRUE                                                                    */
/*   FALSE                                                                   */
/*                                                                           */
/*****************************************************************************/
 void
InitDataBuffer(uchar *cp,uint sizecp,DFILE *dfp)
                                        /* -> to the buffer to initialize.   */
                                        /* size of the buffer.               */
                                        /* -> to the dfile node.             */
{                                       /*                                   */
 uint n;                                /* just a number.                    */
                                        /*                                   */
 memset( cp, ' ' ,sizecp );             /* fill the buffe with blanks.    101*/
 if( dfp )                              /*                                   */
 {                                      /*                                   */
  n = strlen(dfp->expr);                /* compute length of expression.     */
  if( n > SHORTX )                      /* truncate to allocated field width.*/
  {                                     /*                                   */
   n=SHORTX;                            /*                                   */
   cp[n+1] = SXCHAR;                    /*                                   */
  }                                     /*                                   */
  memcpy( cp+1,dfp->expr, n );          /* copy expr/truncated expr to buffer*/
 }                                      /*                                   */
 cp[0] = Attrib(vaStgExp);              /* set expression attribute.         */
 cp[STGCOL] = Attrib(vaStgVal);         /* set data display attribute.       */
 cp[sizecp-1] = 0;                      /* put \0 at end of buffer           */
}                                       /* end initdatabuffer().             */

/*****************************************************************************/
/* GetAppData()                                                           112*/
/*                                                                        112*/
/* Description:                                                           112*/
/*   Gets data from the user's address space.                             112*/
/*                                                                        112*/
/* Parameters:                                                            112*/
/*   addr        input - -> into user space where we want the data from.  112*/
/*   nbytes      input - -> number of bytes to get.                       112*/
/*   pused       output     number of bytes actually read.                112*/
/*   sfx         input - stack frame index for a stack variable.          112*/
/*                                                                        112*/
/* Return:                                                                112*/
/*   PtrToData   -> to buffer of data maintained by DBGet().              112*/
/*               DBGet allocates and frees the buffer on each get.        112*/
/* Assumptions:                                                           112*/
/*                                                                        112*/
/*   addr is flat.                                                        112*/
/*                                                                        112*/
/* Notes:                                                                 112*/
/*   Since the buffer allocated by DBGet() is in dynamic memory we can    112*/
/*   return a NULL  pointer to indicate failure.                          112*/
/*                                                                        112*/
/*****************************************************************************/
 uchar *                                                                /*112*/
GetAppData( uint addr, uint nbytes, uint *pused, uint sfx )             /*112*/
{                                                                       /*112*/
 uint bad_reg_add = 0;                 /* flag to indicate invalid reg var205*/
                                                                        /*112*/
                                                                        /*112*/
 if( TestBit(addr,STACKADDRBIT) )                                       /*112*/
 {                                                                      /*112*/
  addr = StackBPRelToAddr( addr , sfx );                                /*112*/
  if( addr == NULL )                                                    /*112*/
   goto BadAddr;                                                        /*112*/
                                                                        /*112*/
 }else if( (addr >> REGADDCHECKPOS) == REGISTERTYPEADDR ){              /*205*/
     if( sfx != 1 )   /* If not in the executing frame */               /*112*/
     {
         bad_reg_add = 1;               /* set up the flag to indicate    205*/
         goto BadAddr;                  /* unable to remember reg vars    205*/
     }
 }                                                                      /*112*/
                                                                        /*112*/
 return( DBGet(addr, nbytes, pused) );                                  /*112*/
                                                                        /*112*/
 BadAddr:                                                               /*112*/
     if( pused )                                                        /*112*/
       if (bad_reg_add )                /* if unable to remember reg vars 205*/
          *pused = 0xFFFFFFFF;          /* put a NULLF in bytes read,this 205*/
       else                             /* value is checked while putting 205*/
          *pused = 0;                   /* out error message in calling   205*/
     return( NULL );                    /* routine.                       205*/
}                                       /* end GetAppData().              112*/
/*****************************************************************************/
/* StackBPRelToAddr()                                                     112*/
/*                                                                        112*/
/* Description:                                                           112*/
/*   Get the "real" address for a stack bp relative address.              112*/
/*                                                                        112*/
/* Parameters:                                                            112*/
/*   StackBPRelAddr input -> The BP relative address we want the addr for.112*/
/*   sfx            input - stack frame index for a stack variable.       112*/
/*                                                                        112*/
/* Return:                                                                112*/
/*   addr           the "real" address.                                   112*/
/*                                                                        112*/
/* Assumptions:                                                           112*/
/*                                                                        112*/
/*                                                                        112*/
/* Notes:                                                                 112*/
/*                                                                        112*/
/*****************************************************************************/
 uint                                                                   /*112*/
StackBPRelToAddr( uint StackBPRelAddr, uint sfx )                       /*112*/
{                                                                       /*112*/
 uint FrameOffset;                                                      /*112*/
 uint FrameAddr;                                                        /*112*/
 uint addr;                                                             /*112*/
                                                                        /*112*/
 if( TestBit(StackBPRelAddr,STACKADDRSIGN) == FALSE )                   /*112*/
  ResetBit( StackBPRelAddr, STACKADDRBIT );                             /*112*/
 FrameOffset = StackBPRelAddr;                                          /*112*/
 FrameAddr   = StackFrameAddress(sfx);                                  /*112*/
                                                                        /*112*/
 if( FrameAddr == NULL )                                                /*112*/
  return( NULL );                                                       /*112*/
                                                                        /*112*/
 addr = FrameAddr + FrameOffset;                                        /*112*/
 return( addr );                                                        /*112*/
                                                                        /*112*/
}                                                                       /*112*/

/*****************************************************************************/
/* BytesToValue()                                                         523*/
/*                                                                        523*/
/* Description:                                                           523*/
/*   Finds out the value of a bitfield which spans across bytes           523*/
/*                                                                        523*/
/* Parameters:                                                            523*/
/*   Addr        input - -> into user space where we want the data from.  523*/
/*   OffSet      input - -> Starting offset into the fisrt byte.          523*/
/*   Length      input - -> length of the bitfield in terms of bits.      523*/
/*   sfx         input - -> stack frame index for a stack variable.       523*/
/*   Value       output  -> pointer to calculated bitfield value.         523*/
/*                                                                        523*/
/* Return:                                                                523*/
/*   True or False                                                        523*/
/*                                                                        523*/
/* Assumptions:                                                           523*/
/*****************************************************************************/
uint   BytesToValue(uint Addr,uint OffSet,uint Length,uint sfx,uint *Value)
{
   uint  ReturnValue;                   /* Holds bit field value in hex   523*/
   uchar ByteValue;                     /* Byte read from users space     523*/
   uchar ByteMask;                      /* mask to check if a bit is on   523*/
   uint  i;                             /*                                523*/
   uint  WorkOffset;                    /* -> offset into each byte       523*/
   uint  read;                          /* no of bytes read               523*/
   uint  Mask;                          /* mask to cal return value.      523*/
   uchar *dp;                           /* -> returned by get app data.   523*/


   /**************************************************************************/
   /*  Example:                                                           523*/
   /*                                                                     523*/
   /*    A Bit field value can span across bytes. We need to extract      523*/
   /*    bits from these bytes to get the value of bit field in hex.      523*/
   /*                                                                     523*/
   /*    As example if we have a bit field which is 12 bits and spanning  523*/
   /*    across 3 bytes. The memory lay out is given below. We need to    523*/
   /*    get the hex value of this bit field.                             523*/
   /*                                                                     523*/
   /*   ÚÄÂÄÂÄÂÄÂÄÂÄÂÄÂÄ¿    ÚÄÂÄÂÄÂÄÂÄÂÄÂÄÂÄ¿     ÚÄÂÄÂÄÂÄÂÄÂÄÂÄÂÄ¿      523*/
   /*   ³ ³ ³ ³ ³ ³ ³ ³ ³    ³ ³ ³ ³ ³ ³ ³ ³ ³     ³ ³ ³ ³ ³ ³ ³ ³ ³      523*/
   /*   ³2³1³ ³ ³ ³ ³ ³ ³    ³1³9³8³7³6³5³4³3³     ³ ³ ³ ³ ³ ³ ³1³1³      523*/
   /*   ³ ³ ³ ³ ³ ³ ³ ³ ³    ³0³ ³ ³ ³ ³ ³ ³ ³     ³ ³ ³ ³ ³ ³ ³2³1³      523*/
   /*   ÀÄÁÄÁÄÁÄÁÄÁÄÁÄÁÄÙ    ÀÄÁÄÁÄÁÄÁÄÁÄÁÄÁÄÙ     ÀÄÁÄÁÄÁÄÁÄÁÄÁÄÁÄÙ      523*/
   /*    7 6 5 4 3 2 1 0      7 6 5 4 3 2 1 0       7 6 5 4 3 2 1 0       523*/
   /*                                                                     523*/
   /*                  |                    |                     |       523*/
   /*     Address X ---      Address X+1 ---       Address X+2 ---        523*/
   /*                                                                     523*/
   /*    The numbers in the boxes indicates the bit nos of the bitfield.  523*/
   /*                                                                     523*/
   /*    In the above example the input paramters to this func will be:   523*/
   /*       Addr    :  X                                                  523*/
   /*       OffSet  :  6                                                  523*/
   /*       Length  :  12                                                 523*/
   /*                                                                     523*/
   /**************************************************************************/
   ReturnValue = 0;                                                     /*523*/
   ByteMask = 1;                                                        /*523*/
   Mask = 1;                                                            /*523*/
   /**************************************************************************/
   /* Read the byte value at the address given if not able to read return 523*/
   /* back with a failure.                                                523*/
   /**************************************************************************/
   dp = GetAppData(Addr,1,&read,sfx);                                   /*523*/
   if (!dp)                                                             /*523*/
     return(FALSE);                                                     /*523*/
   ByteValue = *dp;                                                     /*523*/
   WorkOffset = OffSet;                                                 /*523*/
   /**************************************************************************/
   /* - Loop for the number of bits in the bit field.                     523*/
   /* - If the bitfield bit is set then increment the return value.       523*/
   /* - If we complete processing a byte read then read next byte.        523*/
   /**************************************************************************/
   for (i = 0 ; i < Length ; i++)                                       /*523*/
   {                                                                    /*523*/
      if ( ByteValue & (uchar)(ByteMask << WorkOffset) )                /*523*/
         ReturnValue |= (Mask << i);                                    /*523*/
      WorkOffset++;                                                     /*523*/
      if (WorkOffset == 8)                                              /*523*/
      {                                                                 /*523*/
        WorkOffset = 0;                                                 /*523*/
        Addr++;                                                         /*523*/
        dp = GetAppData(Addr,1,&read,sfx);                              /*523*/
        if (!dp)                                                        /*523*/
          return(FALSE);                                                /*523*/
        ByteValue = *dp;                                                /*523*/
      }                                                                 /*523*/
   }                                                                    /*523*/
   *Value = ReturnValue;                                                /*523*/
   return(TRUE);                                                        /*523*/
}                                                                       /*523*/
