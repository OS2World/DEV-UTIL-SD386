/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   brk.c                                                                   */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Breakpoint handling.                                                    */
/*                                                                           */
/*                                                                           */
/* History:                                                                  */
/*                                                                           */
/*...Release 1.00                                                            */
/*...                                                                        */
/*... 02/08/91  100   made changes for 32-bit compilation.                   */
/*... 02/08/91  101   made changes for 32-bit compilation. ( by Joe C. )     */
/*    04/04/91  107   Change calls to PeekData/PokeData to DBGet/DBPut.   107*/
/*... 06/02/91  111   fix warnings                                           */
/*                                                                           */
/*...Release 1.00 (Pre-release 1)                                            */
/*...                                                                        */
/*... 08/22/91  234   Joe       PL/X gives "varname" is incorrect message    */
/*...                           when entering a parameter name in the data   */
/*...                           window.  This happens when the cursor is on  */
/*...                           an internal procedure definition statement   */
/*...                           and you use F2 to get into the data window   */
/*...                           and then type the name.                      */
/*...                                                                        */
/*...Release 1.00 (Pre-release 108 12/05/91)                                 */
/*...                                                                        */
/*... 02/10/92  515   Srinivas  Multiple hits of a deferred break points     */
/*                              (same func names).                           */
/*... 02/12/92  521   Joe       Port to C-Set/2.                             */
/*...                                                                        */
/*...Release 1.01 (04/03/92)                                                 */
/*...                                                                        */
/*... 05/08/92  701   Srinivas  Cua Interface.                               */
/*... 05/08/92  701   Joe       Cua Interface.                               */
/*... 06/04/93  827   Joe       Add remote debug support.                    */
/**Includes*******************************************************************/
#include "all.h"

extern PROCESS_NODE *pnode;
extern AFILE        *allfps;
extern CmdParms      cmd;

/*****************************************************************************/
/* DefBrk                                                                    */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Add a breakpoint to the list of breakpoints                             */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   where      address where the breakpoint is to be set.                   */
/*   TorF       optional xbox call.                                          */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   BRK *      -> breakpoint structure built for this address.              */
/*                                                                           */
/*****************************************************************************/
BRK *DefBrk( ULONG where, int TorF)
{
 BRK *p;

 p = (BRK *)Talloc(sizeof(BRK));
 p->brkat = where;
 p->byte = BREAKPT8086OPCODE;
 p->next = pnode->allbrks;
 pnode->allbrks = p ;
 if( TorF == TRUE )
  xDefBrk( where );
 return( p );
}

/*****************************************************************************/
/* UndBrk()                                                                  */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*  Undefine a break point.                                                  */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*    where   address to be zapped.                                          */
/*    TorF    optional xbox call.                                            */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   brk != NULL &&                                                          */
/*   brk is a pointer to a brk* structure in the list of breakpoints.        */
/*   This guarantees that we will always be able to find it.                 */
/*                                                                           */
/*****************************************************************************/
void UndBrk( ULONG where, int TorF )
{
 BRK *p, *pprev;

 /****************************************************************************/
 /* - scan the break point list and remove this break point.                 */
 /****************************************************************************/
 pprev = (BRK*)&pnode->allbrks;
 p = pprev->next;
 for( ; p ; )
 {
  if( p->brkat == where )
  {
   pprev->next = p->next;

   if( p->cond )
   {
    if( p->cond->pCondition )
     Tfree(p->cond->pCondition);
    Tfree( p->cond );
   }
   Tfree((void*)p);

   goto fini;
  }
  pprev = p;
  p = pprev->next;
 }
fini:
 if( TorF == TRUE )
  xUndBrk(where);
}

/*****************************************************************************/
/* FreeAllBrks()                                                             */
/*                                                                           */
/* Description:                                                              */
/*   free all break points and reset the ring to null.                       */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*                                                                           */
/*****************************************************************************/
 void
FreeAllBrks( )
{
  BRK   *bp;
  BRK   *bpnext;
  BRK   *bplast;
  AFILE *fp;


/*****************************************************************************/
/* - let's not reset the breakpoint file time in this case.                  */
/*****************************************************************************/
/*ResetBreakpointFileTime();*/

  for ( fp = allfps;
        fp;
        fp=fp->next
      )
   UnMarkLineBRKs(fp);

  bplast=(BRK*)&(pnode->allbrks);
  bp=pnode->allbrks;

  while(  bp )
  {
   xUndBrk(bp->brkat);                                                  /*827*/
   bpnext= bp->next;
   if(bp->funcname)
    Tfree((void*)bp->funcname);                                           /*521*/
   if(bp->dllname)
    Tfree((void*)bp->dllname);                                           /*521*/
   if(bp->srcname)
    Tfree((void*)bp->srcname);                                           /*521*/
   Tfree((void*)bp);                                                     /*521*/
   bplast->next=bpnext;
   bp=bpnext;
  }
}

/*****************************************************************************/
/* UnMarkLineBRKs()                                                          */
/*                                                                           */
/* Description:                                                              */
/*   Turn of all the breakpoints in this afile.                              */
/*                                                                           */
/* Parameters:                                                               */
/*   fp        input - the afile for this source.                            */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*                                                                           */
/*****************************************************************************/
 void
UnMarkLineBRKs( AFILE *fp)
{
    BRK *brk;
    UINT mid = fp->mid;
    int rellno;

    /* Note: "next" must be 1st field in the BPT block for this code.*/
    for( brk = (BRK *) &pnode->allbrks ; brk != NULL ; brk = brk->next  )
    {
        if( (brk->mid == mid)
         && ((rellno = (int)(brk->lno - fp->Nbias)) > 0)
         && (rellno <= (int)fp->Nlines)
          ) *( fp->source + fp->offtab[ rellno   ] - 1 ) &= ~LINE_BP;   /*234*/
    }
}

 void
DropBrk( AFILE *fp,
         BRK *brk)
{
    UINT lno = brk->lno, mid = brk->mid;

    UndBrk( brk->brkat , TRUE);                                         /*827*/

    /* check to see if any more breakpoints are set for this line */
    /* Note: "next" must be 1st field in the BPT block for this code */
    for( brk = (BRK*)&pnode->allbrks; brk !=NULL ; brk = brk->next )
        if( (brk->lno == lno) && (brk->mid == mid) )
            return;

    /* clear the active breakpoint flag for the source line */
    if( fp && fp->source && ((lno -= fp->Nbias    ) < fp->Nlines) )     /*234*/
        SetBitOff( *(fp->source + fp->offtab[lno] - 1), LINE_BP );
}

 void
DropOnceBrks( UINT address)
{
    BRK *p, *pprev;

    /* Note:  This code assumes that "next" is the 1st field in BRK */

    pprev = (BRK*) &pnode->allbrks ;

    for( p = pprev->next ; p != NULL ; p = pprev->next ){
        if( p->once && (p->brkat == address) ){
            DropBrk( mid2fp(p->mid), p );
        }else{
            pprev = p;
    }   }
}

/*****************************************************************************/
/* SetLineBRK()                                                              */
/*                                                                           */
/* Description:                                                              */
/*   set/reset breakpoints on source lines.                                  */
/*                                                                           */
/* Parameters:                                                               */
/*   fp         pointer to the afile for this source line.                   */
/*   lno        line number.                                                 */
/*   lp         pointer to source line in buffer.                            */
/*   special    pointer to debug file structure.                             */
/*               BRK_COND 1 => conditional breakpoint                        */
/*               BRK_ONCE 2 => onetime breakpoint                            */
/*   expr       expression for a conditional breakpoint.                     */
/*                                                                           */
/* Return:                                                                   */
/*   msg        an error message from ParseCbrk().                           */
/*              NULL    - success.                                           */
/*              BADNESS - not successful when trying to handle breakpoints   */
/*                        that do not call ParseCbrk().                      */
/*                                                                           */
/*****************************************************************************/
#define BADNESS (void *)1

 char *
SetLineBRK( AFILE *fp, UINT   lno, UCHAR *lp, UINT special , char *expr )
{
 BRK     *p;                            /* pointer to breakpoint node.       */
 BRK     *pp;                           /* pointer to breakpoint node.       */
 UINT     addr;                         /*                                101*/
 ULONG    span;                         /*                                   */

 /****************************************************************************/
 /*                                                                       701*/
 /* if there's a bp already on the line                                   701*/
 /* {                                                                     701*/
 /*  scan the list of breakpoints                                         701*/
 /*  {                                                                    701*/
 /*   if this is the node with the breakpoint info for this line          701*/
 /*   {                                                                   701*/
 /*    if this is a conditional breakpoint                                701*/
 /*    {                                                                  701*/
 /*     allocate a conditional node if it hasn't been done already.       701*/
 /*     copy the expression into the conditional structure.               701*/
 /*     parse the expression.                                             701*/
 /*     if it doesn't parse                                               701*/
 /*     {                                                                 701*/
 /*      remove all traces of the allocation.                             701*/
 /*     }                                                                 701*/
 /*     return the error message or null.                                 701*/
 /*      ( effectively this converts a complex breakpoint to a simple one)701*/
 /*    }                                                                  701*/
 /*    else                                                               701*/
 /*     remove the breakpoint. ( this just toggles the break off )        701*/
 /*   {                                                                   701*/
 /*   else                                                                701*/
 /*    bump to the next breakpoint node.                                  701*/
 /*  }                                                                    701*/
 /*  set the breakpoint flag in the source line buffer.                   701*/
 /* }                                                                     701*/
 /****************************************************************************/
 if( *(lp-1) & LINE_BP )                                                /*701*/
 {                                                                      /*701*/
  for( pp = (BRK *) &(pnode->allbrks),p = pp->next;                     /*701*/
       p != NULL;                                                       /*701*/
       p = pp->next                                                     /*701*/
     )                                                                  /*701*/
  {                                                                     /*701*/
   if((p->lno==lno)&&(p->mid==fp->mid))                                 /*701*/
   {                                                                    /*701*/
    if( special == BRK_COND )                                           /*701*/
    {                                                                   /*701*/
     {                                                                  /*701*/
      UCHAR *msg;                                                       /*701*/
                                                                        /*701*/
      if( !p->cond )                                                    /*701*/
      {
       p->cond             = (BRKCOND*) Talloc(sizeof(BRKCOND));
       p->cond->pCondition = Talloc(strlen(expr)+1);
      }

      strcpy(p->cond->pCondition, expr);
      msg = ParseCbrk(p->cond, fp->mid, lno, fp->sfi);
      if( msg )                                                         /*701*/
      {                                                                 /*701*/
       Tfree((void*)p->cond);                                            /*701*/
       Tfree((void*)p->cond->pCondition);
       p->cond = NULL;                                                  /*701*/
      }                                                                 /*701*/
      return(msg);                                                      /*701*/
     }                                                                  /*701*/
    }                                                                   /*701*/
    else                                                                /*701*/
     UndBrk( p->brkat , TRUE);                                          /*701*/
   }                                                                    /*701*/
   else                                                                 /*701*/
       pp = p;                                                          /*701*/
  }                                                                     /*701*/
  *(lp-1) &= ~LINE_BP;                                                  /*701*/
  return(NULL);                                                         /*701*/
 }                                                                      /*701*/
 else                                                                   /*701*/
 {                                                                      /*701*/
 /****************************************************************************/
 /*                                                                       701*/
 /* else there's not a bp already on the line                             701*/
 /* {                                                                     701*/
 /*  map the source line number to an address.                            701*/
 /*  if it maps ok                                                        701*/
 /*  {                                                                    701*/
 /*    allocate a break point structure.                                  701*/
 /*    add module id,lno, and brk type to the structure.                  701*/
 /*    if this is a conditional breakpoint                                701*/
 /*    {                                                                  701*/
 /*     allocate a conditional node.                                      701*/
 /*     copy the expression into the conditional structure.               701*/
 /*     parse the expression.                                             701*/
 /*     if it doesn't parse                                               701*/
 /*     {                                                                 701*/
 /*      remove all traces of the allocation.                             701*/
 /*      remove the break point node.                                     701*/
 /*      return an error message.                                         701*/
 /*     }                                                                 701*/
 /*    }                                                                  701*/
 /*    set the breakpoint flag in the source line buffer.                 701*/
 /*    return ok.                                                         701*/
 /*  }                                                                    701*/
 /*  else                                                                 701*/
 /*   return an error since line doesn't map to an address.               701*/
 /* }                                                                     701*/
 /****************************************************************************/
  addr = DBMapLno(fp->mid, lno, fp->sfi, &span, fp->pdf);
  if( addr )                                                            /*701*/
  {                                                                     /*701*/
   p = DefBrk(addr,TRUE);                                               /*827*/
   p->mid = fp->mid;                                                    /*701*/
   p->lno = lno;                                                        /*701*/
   p->sfi = fp->sfi;
   p->once = (UCHAR)(special==BRK_ONCE);                                /*701*/
   if( special == BRK_COND )                                            /*701*/
   {                                                                    /*701*/
    UCHAR *msg;                                                         /*701*/
    p->cond             = (BRKCOND*) Talloc(sizeof(BRKCOND));                       /*701*/
    p->cond->pCondition = Talloc(strlen(expr) + 1);
    strcpy(p->cond->pCondition, expr);
    msg = ParseCbrk(p->cond,fp->mid,lno,fp->sfi);
    if( msg )                                                           /*701*/
    {                                                                   /*701*/
     Tfree((void*)p->cond->pCondition);
     Tfree((void*)p->cond);                                              /*701*/
     p->cond = NULL;                                                    /*701*/
     UndBrk( p->brkat , TRUE );                                         /*827*/
     return(msg);                                                       /*701*/
    }                                                                   /*701*/
   }                                                                    /*701*/
   *(lp-1) |= LINE_BP;                                                  /*701*/
   return(NULL);                                                        /*701*/
  }                                                                     /*701*/
  else                                                                  /*701*/
   return( BADNESS);                                                    /*701*/
 }                                                                      /*701*/
}                                       /* end SetLineBrk()             /*701*/

/*****************************************************************************/
/* SetAddrBrk()                                                              */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Set/Reset a simple or a one time breakpoint.                            */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   fp         -> to view where break will be set.                          */
/*   address    address where the breakpoint is to be set.                   */
/*   special    one time or simple breakpoint.                               */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*    void                                                                   */
/*                                                                           */
/*****************************************************************************/
void SetAddrBRK(AFILE *fp, ULONG address, int special)
{
 BRK     *p;
 DEBFILE *pdf;
 UINT     lineno;
 ULONG    lno;

 pdf = fp->pdf;

 /****************************************************************************/
 /* - don't do anything if we're trying to set a one time break and          */
 /*   there's already a breakpoint on that address.                          */
 /****************************************************************************/
 if( (special == BRK_ONCE)  && ( IfBrkOnAddr(address) ) )
  return;

 /****************************************************************************/
 /* - if it's a simple breakpoint and there is already a breakpoint          */
 /*   defined for that address, then drop the breakpoint.                    */
 /****************************************************************************/
 if( special == BRK_SIMP )
 {
  p = IfBrkOnAddr(address);
  if( p != NULL )
  {
   DropBrk(fp, p);
   return;
  }
 }

 /****************************************************************************/
 /* - define the breakpoint.                                                 */
 /****************************************************************************/
 lno = 0;
 if( fp->source )
 {
  LNOTAB *pLnoTabEntry;
  ULONG   mid;


  mid = DBMapInstAddr(address, &pLnoTabEntry, pdf);
  lno = 0;
  if( pLnoTabEntry )
   lno = pLnoTabEntry->lno;
  if( ( mid != 0 )                        &&
      ((lineno = lno) > fp->Nbias)        &&
        (lineno -= fp->Nbias) <= fp->Nlines  )
   *(fp->source + fp->offtab[lineno] - 1) |= LINE_BP;
 }

 p = DefBrk(address, TRUE);

 p->mid  = fp->mid;
 p->lno  = lno;
 p->sfi  = fp->sfi;
 p->once = (special == BRK_ONCE)?TRUE:FALSE;
 return;
}

/* Mark all line that have an active breakpoint */
 void
MarkLineBRKs( AFILE *fp )
{
    BRK *brk;
    UINT mid = fp->mid;
    int  sfi = fp->sfi;
    int rellno;

    /* Note: "next" must be 1st field in the BPT block for this code.  */
    for( brk = (BRK *) &(pnode->allbrks) ; brk != NULL ; brk = brk->next ){
        if( (brk->mid == mid)
         && (brk->sfi == sfi)
         && ((rellno = (int)(brk->lno - fp->Nbias)) > 0)
         && (rellno <= (int)fp->Nlines)
          ) *( fp->source + fp->offtab[ rellno   ] - 1 ) |= LINE_BP;
    }
}

/*****************************************************************************/
/* IsBrk()                                                                   */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Is there a break point on this line.                                    */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   fp         -> to view structure.                                        */
/*   lno        executable line number.                                      */
/*                                                                           */
/* Return Value:                                                             */
/*                                                                           */
/*   p or NULL  return a ptr to the BRK node if there is a bp for this line. */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   lno is executable.                                                      */
/*                                                                           */
/*****************************************************************************/
BRK *IsBrk( AFILE *fp, UINT lno )
{
 BRK          *p;

 for(p=pnode->allbrks; p ; p=p->next)
  if( (p->lno == lno) && (p->mid == fp->mid) )
   break;

 return( (p)?p:NULL );
}

/*****************************************************************************/
/* IfBrkOnAddr()                                                             */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Is there a break point on this address.                                 */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   address    breakpoint address we're testing for.                        */
/*                                                                           */
/* Return Value:                                                             */
/*                                                                           */
/*   p or NULL  return a ptr to the BRK node if there is a bp for this line. */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
BRK *IfBrkOnAddr( UINT address )
{
 BRK *p = NULL;

 for( p = pnode->allbrks; p ; p = p->next )
 {
  if( p->brkat == address ){
   return( p );
  }
 }
 return( p);
}

/*****************************************************************************/
/* IsDeferredBrk()                                                           */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Is there a deferred break point defined.                                */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   TRUE or FALSE                                                           */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
int  IsDeferredBrk( void )
{
 BRK *p = NULL;

 for( p = pnode->allbrks; p ; p = p->next )
 {
  if( p->flag.DorI == BP_DEFR )
   return( TRUE );
 }
 return( FALSE );
}
