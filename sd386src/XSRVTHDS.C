/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   xsrvthds.c                                                              */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Start the debuggee.                                                     */
/*                                                                           */
/* History:                                                                  */
/*                                                                           */
/*   06/30/93 Created.                                                       */
/*                                                                           */
/*...                                                                        */
/*... 06/04/93  827   Joe       Add remote debug support.                    */
/*... 09/16/93  901   Joe       Add code to handle resource interlock error. */
/*... 12/06/93  907   Joe       Fix for not updating thread list.            */
/*...                                                                        */
/*****************************************************************************/
#include "all.h"

/*****************************************************************************/
/* XSrvGetThreadInfo()                                                       */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Get thread info.                                                        */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   pBuffer        -> to receiver of thread info.                           */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   n          number of threads.                                           */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   We will not hang while traversing the circular list of thread           */
/*   info from DosDebug.                                                     */
/*                                                                           */
/*****************************************************************************/
ULONG XSrvGetThreadInfo( THREADINFO *pBuffer )
{
 ULONG         tid;
 ULONG         nexttid;
 ULONG         n;
 PtraceBuffer  ptb;
 APIRET        rc;
 ULONG         eip;

 memset(&ptb,0,sizeof(ptb));
 /****************************************************************************/
 /* - Threads are kept in a circular list, so break on a repeat of           */
 /*   thread 1.                                                              */
 /****************************************************************************/
 nexttid = 0;
 for( n = 0 , tid = 1 ; nexttid != 1; tid = nexttid )
 {
  /***************************************************************************/
  /* - get the thread status.                                                */
  /***************************************************************************/
  ptb.Pid = GetEspProcessID();
  ptb.Cmd = DBG_C_ThrdStat;
  ptb.Tid = tid;
  ptb.Buffer = (ULONG)pBuffer;
  ptb.Len = sizeof( ULONG );   /* thread stat buffer is 4 bytes */
  rc = DosDebug( &ptb);

  /***************************************************************************/
  /* - In order to keep from hanging in this loop on a resource interlock,901*/
  /*   if we get an error on thread 1, then the debuggee has been severed.901*/
  /*   In this case, simply return.                                       901*/
  /***************************************************************************/
  if( ptb.Cmd == DBG_N_Error && tid == 1 )                              /*901*/
   return(0);                                                           /*901*/

  /***************************************************************************/
  /* This test blows by gaps in the list of threads.                         */
  /***************************************************************************/
  if ( rc != 0 || ptb.Cmd != DBG_N_Success )
   continue;

  pBuffer->tid = tid;                                                   /*907*/
  nexttid = ptb.Value;
  /***************************************************************************/
  /* - get the thread eip.                                                   */
  /***************************************************************************/
  memset(&ptb,0,sizeof(ptb));
  ptb.Pid = GetEspProcessID();
  ptb.Cmd = DBG_C_ReadReg;
  ptb.Tid = tid;
  rc = DosDebug( &ptb);

  if ( rc != 0 || ptb.Cmd != DBG_N_Success )
   continue;

  if( ptb.CSAtr )
   eip = ptb.EIP;
  else
   eip = Sys_SelOff2Flat(ptb.CS, LoFlat(ptb.EIP));

  pBuffer->eip = eip;

  /***************************************************************************/
  /* - bump counter/pointer.                                                 */
  /***************************************************************************/
  n++;
  pBuffer++;
 }
 return(n);
}

/*****************************************************************************/
/* XSrvFreezeThread()                                                        */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Freeze single or all threads.                                           */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   tid      thread id. If tid = 0 then all threads will be frozen.         */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   void                                                                    */
/*                                                                           */
/* Note:                                                                     */
/*                                                                           */
/*   Thread list must have been updated using BuildThreadList().             */
/*                                                                           */
/*****************************************************************************/
void XSrvFreezeThread ( ULONG tid )
{
 PtraceBuffer ptb;

 memset(&ptb,0,sizeof(ptb));
 ptb.Cmd = DBG_C_Freeze;
 ptb.Pid = GetEspProcessID();
 ptb.Tid = tid;
 DosDebug(&ptb);
}

/*****************************************************************************/
/* XSrvThawThread()                                                          */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Thaw a thread or thaw all threads.                                      */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   tid      thread to thaw or 0.                                           */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   void                                                                    */
/*                                                                           */
/* Assumption:                                                               */
/*                                                                           */
/*   Thread list must have been updated using BuildThreadList().             */
/*                                                                           */
/*****************************************************************************/
void XSrvThawThread( ULONG tid )
{
 PtraceBuffer ptb;

 memset(&ptb,0,sizeof(ptb));
 ptb.Cmd = DBG_C_Resume  ;
 ptb.Pid = GetEspProcessID();
 ptb.Tid = tid;
 DosDebug(&ptb);
}

/*****************************************************************************/
/* XSrvSetExecThread()                                                       */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Set the execution context to another thread.                            */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   pExecAddr -> to address that will receive the flat execution address.   */
/*   pptb      -> to the ptrace buffer that we will fill in for the caller.  */
/*   tid          the thread we're establishing context in.                  */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   rc        0=>success                                                    */
/*             1=>failure                                                    */
/*                                                                           */
/* Assumption:                                                               */
/*                                                                           */
/*****************************************************************************/
APIRET XSrvSetExecThread(ULONG *pExecAddr,PtraceBuffer *pptb, UINT tid)
{
 APIRET rc;

 memset(pptb,0,sizeof(PtraceBuffer));

 pptb->Pid = GetEspProcessID();
 pptb->Cmd = DBG_C_ReadReg;
 pptb->Tid = tid;
 rc = DosDebug( pptb);
 if( (rc != 0) || (pptb->Cmd != 0) )
  return(1);

 if( pptb->CSAtr )
  *pExecAddr = pptb->EIP;
 else
  *pExecAddr = Sys_SelOff2Flat(pptb->CS,LoFlat(pptb->EIP));
 return(0);
}
