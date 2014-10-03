/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   q.c                                                                     */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*  Queue function for multiple process support.                             */
/*                                                                           */
/* History:                                                                  */
/*                                                                           */
/*   03/05/94 Created.                                                       */
/*                                                                           */
/*****************************************************************************/

#include "all.h"

#define  QUE_STACK_SIZE 0x4000          /* read queue thread stack size.     */

/*****************************************************************************/
/* StartQue()                                                                */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Create a queue.                                                         */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   pQueName     -> queue name.                                             */
/*   pQueReader   -> function that will read and process queue messages.     */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   rc                                                                      */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*  All queues are FIFO.                                                     */
/*                                                                           */
/*****************************************************************************/
APIRET StartQue( char *pQueName, void (* _Optlink pQueReader)(void *) )
{
 APIRET   rc;
 LHANDLE  QueHandle = 0;

 /****************************************************************************/
 /* Create a termination/session notification queue.                         */
 /****************************************************************************/
 rc=DosCreateQueue((PHQUEUE)&QueHandle, QUE_FIFO, pQueName);
 if( rc != 0)
  return(rc);

 /****************************************************************************/
 /* Start a thread to read the queue.                                        */
 /****************************************************************************/
 _beginthread(pQueReader , NULL, QUE_STACK_SIZE, (void*)QueHandle);
 return(0);
}

/*****************************************************************************/
/* SendMsgToQue()                                                            */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Send a message to a queue.                                              */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   pQueName  ->name of the queue that we're sending the message to.        */
/*   Msg         message going to the queue.                                 */
/*   pBuf      ->queue element buffer.                                       */
/*   Len         queue element buffer length.                                */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   rc      0==>success                                                     */
/*           1==>failure                                                     */
/*                                                                           */
/* Assumption:                                                               */
/*                                                                           */
/*****************************************************************************/
void SendMsgToQue( char *pQueName, ULONG Msg, void *pBuf, ULONG Len)
{
 ULONG     Pid;
 HQUEUE    QueHandle;
 void     *pQueElement = NULL;
 ULONG     QueElementLen = Len;
 ULONG     flags;
 TIB      *pTib;
 PIB      *pPib;

 /****************************************************************************/
 /* - Open the a queue. The returned pid is the process id of the queue      */
 /*   owner. This call also returns a handle to the queue.                   */
 /****************************************************************************/
 DosOpenQueue(&Pid,&QueHandle,pQueName);

 /****************************************************************************/
 /* - Allocate a shared memory for the queue element if there is one.        */
 /* - Copy the message into the shared segment.                              */
 /* - Give the owner of the queue access to the queue element.               */
 /****************************************************************************/
 if( pBuf != NULL )
 {
  flags = PAG_COMMIT | OBJ_GIVEABLE | PAG_READ | PAG_WRITE;
  DosAllocSharedMem( &pQueElement, NULL, QueElementLen, flags );

  memcpy(pQueElement,pBuf,QueElementLen);

  flags = PAG_READ;
  DosGiveSharedMem( pQueElement, Pid, flags );
 }
 /****************************************************************************/
 /* - Send the message to the queue.                                         */
 /****************************************************************************/
 DosWriteQueue( QueHandle, Msg, QueElementLen, pQueElement,0);

 /****************************************************************************/
 /* - Get the pid of "this" process sending the message.                     */
 /* - If the owner of the pid and pid sending the message are the same,      */
 /*   then don't DosFreeMem() the queue element. It will be freed by the     */
 /*   owner.                                                                 */
 /****************************************************************************/
 if(pQueElement != NULL )
 {
  DosGetInfoBlocks(&pTib,&pPib);
  if( pPib->pib_ulpid != Pid )
  {
   DosFreeMem(pQueElement);
  }
 }
}


#if 0
/*****************************************************************************/
/* - these functions are not currently used but the code is retained just    */
/*   in case.                                                                */
/*****************************************************************************/
void *PeekQueMsg( LHANDLE QueHandle, ULONG QueMsg )
{
 REQUESTDATA  Request;
 ULONG        DataLen;
 BYTE         ElePri;
 ULONG        EleCode;
 ULONG        count;

 DBG_QUE_ELEMENT *pDataAddr=NULL;

 count = 0;
 DosQueryQueue( QueHandle, &count);
 for( EleCode=0 ;count>0;count-- )
 {
  printf("\nQue count=%d",count);fflush(0);
  DosPeekQueue(QueHandle,
               &Request,
               &DataLen,
               (PPVOID)&pDataAddr,
               &EleCode,
               DCWW_WAIT,
               &ElePri,
               0L);

  if( Request.ulData == QueMsg )
  {
   return( ( void*)pDataAddr );
  }
 }
 return(NULL);
}

void FlushMsgFromQue( LHANDLE QueHandle, ULONG QueMsg )
{
 REQUESTDATA  Request;
 ULONG        DataLen;
 BYTE         ElePri;
 ULONG        EleCode;
 ULONG        count;

 DBG_QUE_ELEMENT *pDataAddr=NULL;
 APIRET rc;

 count = 0;
 DosQueryQueue( QueHandle, &count);

 for( EleCode=0 ;count>0;count-- )
 {
  rc = DosPeekQueue(QueHandle,
                    &Request,
                    &DataLen,
                    (PPVOID)&pDataAddr,
                    &EleCode,
                    DCWW_WAIT,
                    &ElePri,
                    0L);

  if( Request.ulData == QueMsg )
  {
   rc = DosReadQueue(QueHandle,
                     &Request,
                     &DataLen,
                     (PPVOID)&pDataAddr,
                     EleCode,
                     DCWW_WAIT,
                     &ElePri,
                     0L);
   EleCode = 0;
  }
 }
}
#endif
