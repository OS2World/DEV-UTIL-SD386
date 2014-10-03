/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   xsrvwps.c                                                               */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Allow user to define watch points and put in the watch points.          */
/*                                                                           */
/* History:                                                                  */
/*                                                                           */
/*...Release 1.00 (03/03/92)                                                 */
/*...                                                                        */
/*... 03/10/92  602   Srinivas  Hooking up watch points.                     */
/*                                                                           */
/*****************************************************************************/

#include "all.h"

static WP_REGISTER Db_Regs[NODEBUGREGS];/* hardware debug registers          */

/*****************************************************************************/
/* XSrvDefWps()                                                              */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*    Defines the watch points in the x-server.                              */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   pRegs    -  ->array of watch point definitions.                         */
/*   size     -  size of the block of debug register data.                   */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   void                                                                    */
/*****************************************************************************/
void XSrvDefWps( void *pRegs , int size )
{
 /****************************************************************************/
 /* clear any watch points currently set as well as any pending wp           */
 /* notifications.                                                           */
 /****************************************************************************/
 XSrvPullOutWps( );
 memcpy(Db_Regs,pRegs,size );
}

/*****************************************************************************/
/* XSrvPutInWps()                                                            */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*    Scans the debug registers and sets the watch points.                   */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*  pIndexes   ->to an array of watch point indexes.                         */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*****************************************************************************/
void  XSrvPutInWps( ULONG *pIndexes )
{
  PtraceBuffer ptb;
  uint         i;
  int          rc;

  for ( i = 0 ; i < NODEBUGREGS ; i++)
  {
     if( (Db_Regs[i].Address != 0) &&
         (Db_Regs[i].Status == ENABLED) &&
         (Db_Regs[i].IsSet  == FALSE) )
     {
        memset(&ptb,0,sizeof(ptb));
        ptb.Pid   = GetEspProcessID();
        ptb.Addr  = Db_Regs[i].Address;
        ptb.Index = 0;
        ptb.Tid   = 1;

        /*********************************************************************/
        /* set the size of the watch point.                                  */
        /*********************************************************************/
        switch (Db_Regs[i].Size)
        {
           case 0:
             ptb.Len = 1;
             break;
           case 1:
             ptb.Len = 2;
             break;
           case 2:
             ptb.Len = 4;
             break;
        }

        /*********************************************************************/
        /* set the scope of the watch point.                                 */
        /*********************************************************************/
        switch (Db_Regs[i].Scope)
        {
           case WPS_LOCAL:
             ptb.Value = DBG_W_Local;
             break;
           case WPS_GLOBAL:
             ptb.Value = DBG_W_Global;
             break;
        }

        /*********************************************************************/
        /* set the type of the watch point.                                  */
        /*********************************************************************/
        switch (Db_Regs[i].Type)
        {
           case READWRITE:
             ptb.Value += DBG_W_ReadWrite;
             break;
           case WRITE:
             ptb.Value += DBG_W_Write;
             break;
           case EXECUTE:
             ptb.Value += DBG_W_Execute;
             break;
        }

        ptb.Cmd = DBG_C_SetWatch;
        rc = DosDebug(&ptb);
        if ( rc || (ptb.Cmd != DBG_N_Success) )
           return;

        Db_Regs[i].IsSet   = TRUE;
        Db_Regs[i].Wpindex = ptb.Index;
     }
  }
  /***************************************************************************/
  /* - give the assigned indexes back to the caller.                         */
  /***************************************************************************/
  for( i=0; i<NODEBUGREGS; i++ )
   *pIndexes++ = Db_Regs[i].Wpindex;

  return;
}

/*****************************************************************************/
/* XSrvPullOutWps()                                                          */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*    Scans the debug registers and clears watch points that are currently   */
/*    set. If a watch point was hit it has already been cleared.             */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*****************************************************************************/
void  XSrvPullOutWps( void )
{
  PtraceBuffer ptb;
  uint         i;

  /***************************************************************************/
  /* - Clear the watch points. If there are any pending notifications        */
  /*   resulting from multiple watch point hits occurring at the             */
  /*   same execution address, then these notifications will be lost         */
  /*   into the ether. This could happen if you set all four watch points    */
  /*   on the same variable.                                                 */
  /***************************************************************************/
  for ( i = 0 ; i < NODEBUGREGS ; i++)
  {
     if( Db_Regs[i].Status == ENABLED )
     {
        memset(&ptb,0,sizeof(ptb));
        ptb.Pid   = GetEspProcessID();
        ptb.Index = Db_Regs[i].Wpindex;
        ptb.Cmd   = DBG_C_ClearWatch;
        DosDebug(&ptb);
     }
  }
  return;
}

/*****************************************************************************/
/* MarkWpNotSet()                                                            */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*    After a watch point is hit, then we mark it as not set.                */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   wpindex   index of the watchpoint that got hit.                         */
/*                                                                           */
/* Return:                                                                   */
/*****************************************************************************/
void MarkWpNotSet( ULONG wpindex )
{
 int i;

 for(i=0;i < NODEBUGREGS; i++ )
  if( Db_Regs[i].Wpindex == wpindex )
   Db_Regs[i].IsSet = FALSE;
}
