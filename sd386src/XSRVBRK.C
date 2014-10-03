/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   srvrbrk.c                                                            827*/
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*  Handle breakpoints.                                                      */
/*                                                                           */
/* History:                                                                  */
/*                                                                           */
/*   06/04/93 Created.                                                       */
/*                                                                           */
/*... 06/04/93  827   Joe       Add remote debug support.                    */
/*...                                                                        */
/*****************************************************************************/
#include "all.h"

static BRK *pAllBrks;
/*****************************************************************************/
/* XSrvDefBrk                                                                */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Add a breakpoint to the server breakpoint list.                         */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   where      address where the breakpoint is to be set.                   */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   p         -> to the BRK structure.                                      */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   The returned BRK* is only used within the x-server.                     */
/*                                                                           */
/*****************************************************************************/
BRK *XSrvDefBrk( ULONG where )
{
 BRK *p;

 p = (BRK *)Talloc(sizeof(BRK));
 p->brkat = where;
 p->byte = BREAKPT8086OPCODE;
 p->next = pAllBrks;
 pAllBrks = p ;
 return(p);
}

/*****************************************************************************/
/* XSrvUndBrk()                                                              */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*  Undefine a break point.                                                  */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*    brkat   address we're undefining.                                      */
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
void XSrvUndBrk( ULONG brkat )
{
 BRK *p, *pprev;

 /****************************************************************************/
 /* - scan the break point list and remove this break point.                 */
 /****************************************************************************/
 pprev = (BRK*)&pAllBrks;
 p = pprev->next;
 for( ; p ; )
 {
  if( p->brkat == brkat )
  {
   pprev->next = p->next;
   if( p->cond )
     Tfree( (void*)p->cond );
   Tfree((void*)p);
   return;
  }
  pprev = p;
  p = pprev->next;
 }
}
/*****************************************************************************/
/* XSrvPutInBrk()                                                            */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Stuff in a break.                                                       */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   brk       structure containing the breakpoint information.              */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
void XSrvPutInBrk( ULONG where )
{
 UCHAR opcode;
 UINT  bytesread;
 UCHAR *p;
 BRK   *pBrk;

#if 0
 /****************************************************************************/
 /* - deferred breakpoints do not have addresses yet, so we can't            */
 /*   "insert" them.                                                         */
 /****************************************************************************/
 if((brk->flag & BP_DEFR_ONCE) ||
   (brk->flag & BP_DEFR_ALL))
  return;
#endif

 /****************************************************************************/
 /* - get a pointer to the BRK structure for this address.                   */
 /* - extract the opcode from the address where the breakpoint will go.      */
 /* - if read fails then drop into error.                                    */
 /****************************************************************************/
 pBrk = GetBrk( where );
 if( pBrk == NULL )
  return;
 p=Getnbytes(pBrk->brkat,1,&bytesread);
 if( bytesread )
  opcode = *p;
 else
  goto error;

 /****************************************************************************/
 /* - if there's not a break point already there then put one in.            */
 /****************************************************************************/
 if( opcode != BREAKPT8086OPCODE)
 {
  pBrk->byte = opcode;
  opcode = BREAKPT8086OPCODE;
  if( Putnbytes(pBrk->brkat,1,&opcode) )
   goto error;
 }
 return;

error:
 /****************************************************************************/
 /* - toss the breakpoint on an error.                                       */
 /****************************************************************************/
 XSrvUndBrk( where );
}
/*****************************************************************************/
/* XSrvPullOutBrk()                                                          */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Replaces an INT3 with the saved away opcode.                            */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   brk        -> to structure containing address where we want to lift     */
/*                 the break.                                                */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*  rc          0=>breakpoint removed.                                       */
/*              1=>unable to remove the breakpoint.                          */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
int XSrvPullOutBrk( ULONG where )
{
 UCHAR  opcode;
 UCHAR *p;
 UINT   bytesread;
 BRK   *pBrk;

 /****************************************************************************/
 /* - get a pointer to the BRK structure for this address.                   */
 /* - extract the INT3 from the address.                                     */
 /* - if read fails then drop into error.                                    */
 /****************************************************************************/
 pBrk = GetBrk( where );
 if( pBrk == NULL )
  return(0);
 p=Getnbytes(pBrk->brkat,1,&bytesread);
 if( bytesread )
  opcode = *p;
 else
  goto error;

 /****************************************************************************/
 /* - put the original opcode back in.                                       */
 /****************************************************************************/
 if( opcode == BREAKPT8086OPCODE )
 {
  opcode = pBrk->byte;
  if( Putnbytes(pBrk->brkat,1,&opcode ) )
   goto error;
 }
 return(0);

error:
 /****************************************************************************/
 /* If we can't peek or poke the breakpoint, then return unsuccessful.       */
 /****************************************************************************/
 return(1);
}

/*****************************************************************************/
/* XSrvInsertAllBrk()                                                        */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Stuff in all breaks.                                                    */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
void XSrvInsertAllBrk( void )
{
 BRK *p;

 for( p = pAllBrks; p ; p = p->next )
  XSrvPutInBrk( p->brkat );
 return;
}

/*****************************************************************************/
/* XSrvRemoveAllBrk()                                                        */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Remove INT3s and put back opcodes.                                      */
/*                                                                           */
/* Parameters:                                                               */
/*   void                                                                    */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   void                                                                    */
/*                                                                           */
/*****************************************************************************/
void XSrvRemoveAllBrk( void )
{
 BRK *p;
 BRK *plast;

 p = pAllBrks;
 plast = (BRK*)&pAllBrks;
 for( ; p ; plast = p, p = p->next )
 {
  if( XSrvPullOutBrk( p->brkat ) == 1 )
  {
   XSrvUndBrk( p->brkat );
   p = plast;
  }
 }
 return;
}

/*****************************************************************************/
/* XSrvInsertOneBrk                                                              */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Put in a single breakpoint from the list. If not in the list then       */
/*   add it to the list.                                                     */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   where      address where the breakpoint goes.                           */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   TRUE       if the breakpoint had to be defined then inform the          */
/*              caller.                                                      */
/*   FALSE      if the breakpoint was already in the list.                   */
/*                                                                           */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*  There exists a BRK * defining the breakpoint.                            */
/*                                                                           */
/*****************************************************************************/
int XSrvInsertOneBrk( ULONG where )
{
 BRK   *p;

 for( p = pAllBrks; p ; p = p->next )
 {
  if( p->brkat == where )
  {
    XSrvPutInBrk( p->brkat );
    return(FALSE);
  }
 }
 p = XSrvDefBrk( where );
 XSrvPutInBrk( p->brkat );
 return(TRUE);
}

/*****************************************************************************/
/* XSrvRemoveOneBrk()                                                        */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Remove a breakpoint.                                                    */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   where      address of breakpoint to remove.                             */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
void XSrvRemoveOneBrk( ULONG where)
{
 BRK *p;

 for( p = pAllBrks; p ; p = p->next )
 {
  if( p->brkat == where )
  {
   XSrvPullOutBrk( p->brkat );
   return;
  }
 }
}

/*****************************************************************************/
/* GetBreak()                                                                    */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Get a pointer to the breakpoint structure for the parameter address.    */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   where      address we want.                                             */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*  pBrk        -> to BRK structure defining where.                          */
/*              NULL if not found.                                           */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
BRK *GetBrk( ULONG where )
{
 BRK *pBrk;

 for( pBrk = pAllBrks; pBrk ; pBrk = pBrk->next )
 {
  if( pBrk->brkat == where )
   break;
 }
 return(pBrk);
}

/*****************************************************************************/
/* ClearBrks()                                                               */
/*                                                                           */
/* Description:                                                              */
/*   free all break points in the xserver.                                   */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*                                                                           */
/*****************************************************************************/
void ClearBrks( )
{
  BRK   *bp;
  BRK   *bpnext;
  BRK   *bplast;

  bplast=(BRK*)&pAllBrks;
  bp=pAllBrks;
  while(  bp )
  {
   bpnext= bp->next;
   if(bp->funcname)
    Tfree((void*)bp->funcname);
   if(bp->dllname)
    Tfree((void*)bp->dllname);
   Tfree((void*)bp);
   bplast->next=bpnext;
   bp=bpnext;
  }
  pAllBrks = NULL;
}

/*****************************************************************************/
/* _IfBrkOnAddr()                                                            */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Is there a break defined at this address.                               */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   address    breakpoint address we're testing for.                        */
/*                                                                           */
/* Return Value:                                                             */
/*                                                                           */
/*   TRUE or FALSE                                                           */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
BRK *_IfBrkOnAddr( UINT address )
{
 BRK *p = NULL;

 for( p = pAllBrks; p ; p = p->next )
 {
  if( p->brkat == address ){
   return( p );
  }
 }
 return( NULL );
}
