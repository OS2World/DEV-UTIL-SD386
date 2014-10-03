/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   pid.c                                                                   */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Functions for creating, accessing, and maintaining the shared memory    */
/*   heap on the dbg side of the connection when debugging multiple          */
/*   processes.  On the esp side, we use a c-runtime heap since it           */
/*   doesn't have to be shared.                                              */
/*                                                                           */
/* History:                                                                  */
/*                                                                           */
/*   10/14/94 Created.                                                       */
/*                                                                           */
/*****************************************************************************/

#include "all.h"

#define PID_SHARED_MEM_SIZE  5*4096

static ALLPIDS *pAllpids = NULL;
static ULONG   *pSharedMem;

ULONG   *GetShrMem ( void ) { return( pSharedMem ); }
ALLPIDS *GetAllpids( void ) { return( pAllpids   ); }

/*****************************************************************************/
/* AddPid()                                                                  */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Add a pid structure to the allpids list in the shared heap.             */
/*   DosAllocMem() is used because of sharing requirements.                  */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   pid         the process id of a process being debugged.                 */
/*   sid         the session id that the pid is running in.                  */
/*   EspSid      the session id of the probe( local/child/multiple process ) */
/*   type        the type of the process.                                    */
/*   pFileSpec ->the name of the process.                                    */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
void AddPid( USHORT  pid,
             ULONG   sid,
             ULONG   EspSid,
             ULONG   type,
             char   *pFileSpec )
{
 ALLPIDS *p, *q, *r;
 ULONG    flags;

 /****************************************************************************/
 /* - test to see if this pid is already accounted for.                      */
 /****************************************************************************/
 for( p = pAllpids ; p ; p = p->next )
  if( p->pid == pid )
   return;

 /****************************************************************************/
 /* - Don't use a shared heap if we're debugging a single process.           */
 /****************************************************************************/
 if( SingleMultiple() == SINGLE )
 {
  p = Talloc(sizeof(ALLPIDS));
  memset(p, 0, sizeof(ALLPIDS) );
  p->pid                     = pid;
  p->sid                     = sid;
  p->type                    = type;
  p->EspSid                  = EspSid;
  p->Connect                 = DISCONNECTED;

  if( pFileSpec != NULL )
  {
   p->pFileSpec = Talloc( strlen(pFileSpec) + 1 );
   strcpy(p->pFileSpec, pFileSpec);
  }

  for( q = (ALLPIDS*)&pAllpids, r = pAllpids ; r ; q = r, r = r->next ){;}

  q->next = p;

  return;
 }

 /****************************************************************************/
 /* - Come here only if we're using configurations that require a shared     */
 /*   heap.                                                                  */
 /* - allocate a node and add it to the head of the list.                    */
 /****************************************************************************/
 if( pAllpids == NULL )
 {
  if( IsVerbose() ) {printf("\nHeap Started");fflush(0);}
  flags = OBJ_GIVEABLE | PAG_READ | PAG_WRITE | OBJ_GETTABLE;
  DosAllocSharedMem( (PPVOID)&pSharedMem, NULL, PID_SHARED_MEM_SIZE, flags );
  flags = DOSSUB_INIT | DOSSUB_SPARSE_OBJ;
  DosSubSetMem( pSharedMem, flags, PID_SHARED_MEM_SIZE );
 }
 DosSubAllocMem( pSharedMem, (PPVOID)&p, sizeof(ALLPIDS) );

 memset(p, 0, sizeof(ALLPIDS) );
 p->pid                     = pid;
 p->sid                     = sid;
 p->EspSid                  = EspSid;
 p->type                    = type;
 p->Connect                 = DISCONNECTED;
 p->PidFlags.ConnectYielded = FALSE;
 p->PidFlags.IsDebug        = FALSE;
 p->PidFlags.Initializing   = TRUE;
 p->pSqeMsg                 = NULL;
 if( pFileSpec != NULL )
 {
  DosSubAllocMem( pSharedMem, (PPVOID)&(p->pFileSpec), strlen(pFileSpec) + 1 );
  strcpy(p->pFileSpec, pFileSpec);
 }

 for( q = (ALLPIDS*)&pAllpids, r = pAllpids ; r ; q = r, r = r->next ){;}

 q->next = p;
}

void FreeAllPids( void )
{
 ALLPIDS *ptr;
 ALLPIDS *ptrnext;

 for( ptr = pAllpids; ptr != NULL; )
 {
  ptrnext=ptr->next;
  if(ptr->pFileSpec) Tfree(ptr->pFileSpec);
  Tfree((void*)ptr);
  ptr=ptrnext;
 }
 pAllpids = NULL;
}

/*****************************************************************************/
/* RemovePid()                                                               */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Remove a pid structure from the shared heap.                            */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   pid         the process id of a process being debugged.                 */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
void RemovePid( USHORT pid )
{
 ALLPIDS *pprev;
 ALLPIDS *p;

 pprev = (ALLPIDS *)&pAllpids;
 for( p = pprev->next; p ; pprev = p, p = pprev->next )
 {
  if( p->pid == pid )
  {
   pprev->next = p->next;
   if( p->pFileSpec != NULL )
    DosSubFreeMem( pSharedMem, p->pFileSpec, strlen(p->pFileSpec) + 1 );
   if( p->pTermQue != NULL )
    DosSubFreeMem( pSharedMem, p->pTermQue, strlen(p->pTermQue) + 1 );
   DosSubFreeMem( pSharedMem, p, sizeof(ALLPIDS) );
   break;
  }
 }
}


/*****************************************************************************/
/* - Free the shared memory heap structures.                                 */
/*****************************************************************************/
void FreeSharedHeap( void )
{
 ALLPIDS *p;

 if( IsVerbose() ) { printf("\nShared Heap freed."); fflush(0); }
 for( p = pAllpids ; p ; p = pAllpids )
 {
  RemovePid( p->pid );
 }
 DosFreeMem(pSharedMem);
 pSharedMem = NULL;
}

/*****************************************************************************/
/* - Get read/write access to the shared memory heap for this debugger.      */
/*****************************************************************************/
void SetShrMem( ULONG *ShrMemBaseAddr )
{
 pSharedMem = ShrMemBaseAddr;
 DosGetSharedMem( pSharedMem, PAG_READ | PAG_WRITE );
}

/*****************************************************************************/
/* - set the pointer to the shared memory heap as inherited from the         */
/*   parent debugger at invocation time.                                     */
/*****************************************************************************/
void SetShrHeap( ULONG *HeapBigPtr )
{
 pAllpids = (ALLPIDS *)HeapBigPtr;
}

/*****************************************************************************/
/* - Get a pointer to a pid structure.                                       */
/*****************************************************************************/
ALLPIDS *GetPid( USHORT pid )
{
 ALLPIDS *p;

 for( p = pAllpids ; p ; p = p->next )
 {
  if( p->pid == pid )
   break;
 }
 return(p);
}


/*****************************************************************************/
/*                                                                           */
/*  - Test to see if a pid is currently connected.  This only applies to     */
/*    serial connections.                                                    */
/*                                                                           */
/*****************************************************************************/
BOOL IsPidConnected( void )
{
 ALLPIDS *p;

 for( p = pAllpids ; p ; p = p->next )
  if( p->Connect == CONNECTED )
   return( TRUE );
 return( FALSE );
}


/*****************************************************************************/
/* GetPidIndex()                                                             */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Get a pointer to a specific node.                                       */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   index   the index of the process node we're seeking.                    */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   p       -> to the structure for the parameter pid.                      */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   index > 0                                                               */
/*                                                                           */
/*****************************************************************************/
ALLPIDS *GetPidIndex( int index )
{
 ALLPIDS *p;

 for( p = pAllpids, --index; index > 0; p = p->next, index-- ){;}
 return(p);
}

/*****************************************************************************/
/*                                                                           */
/* - This only applies to serial connections only.                           */
/* - Get a pointer to the node that is currently connected.  Applies to      */
/*   serial connections only.                                                */
/*                                                                           */
/*****************************************************************************/
ALLPIDS *GetPidConnected( void )
{
 ALLPIDS *p;

 for( p = pAllpids ; p ; p = p->next )
  if( p->Connect == CONNECTED )
   return( p );
 return( NULL );
}

#ifdef __ESP__
/*---------------------------------------------------------------------------*/

/*****************************************************************************/
/*                                                                           */
/* - Get a pointer to the structure containing the pid of an esp attached    */
/*   to a process.                                                           */
/*                                                                           */
/*****************************************************************************/
ALLPIDS *GetEspPid( USHORT EspPid )
{
 ALLPIDS *p;

 for( p = pAllpids ; p ; p = p->next )
 {
  if( p->EspPid == EspPid )
   break;
 }
 return(p);
}

/*---------------------------------------------------------------------------*/
#endif


#ifdef __DBG__
/*---------------------------------------------------------------------------*/

/*****************************************************************************/
/* - add the session id of a debugger session to a pid structure.            */
/*****************************************************************************/
void AddDbgPidAndSid( USHORT pid, USHORT DbgPid, ULONG DbgSid )
{
 ALLPIDS *p;

 for( p = pAllpids ; p ; p = p->next )
  if( p->pid == pid )
  {
   p->DbgSid = DbgSid;
   p->DbgPid = DbgPid;
   break;
  }
}

/*****************************************************************************/
/*                                                                           */
/* - This only applies to serial connections.                                */
/* - This thread periodically checks for a manual disconnect by the user     */
/*   and displays a message if it happens.                                   */
/*                                                                           */
/*****************************************************************************/
static BOOL KillChk4Disconnect = FALSE;

void SetKillChk4Disconnect( BOOL TorF ) { KillChk4Disconnect = TorF; }

void CheckForDisConnect( void *dummy )
{
 ALLPIDS *p;

 /****************************************************************************/
 /* - This thread gets launched after the connectsema4 is posted.            */
 /*   It waits for a possible disconnection, posts a message, and dies.      */
 /****************************************************************************/
 p = GetPid( DbgGetProcessID() );

 if( IsVerbose() ) {printf("\nChk4Disconnect started");fflush(0);}
 for( ; KillChk4Disconnect == FALSE ; )
 {
  if( p->PidFlags.ConnectYielded == TRUE )
  {
   fmterr("Disconnected...double click on window to reconnect.");
   p->PidFlags.ConnectYielded = FALSE ;
  }
  DosSleep(1500);
 }
 if( IsVerbose() ) {printf("\nChk4Disconnect ended");fflush(0);}
 _endthread();
}

void Check4ConnectRequest( void *dummy )
{
 ALLPIDS *p;

 /****************************************************************************/
 /* - This thread gets launched after the connectsema4 is posted.            */
 /****************************************************************************/

 if( IsVerbose() ) {printf("\nChk4 connect request started");fflush(0);}
 for(;;)
 {
  p = GetPid( DbgGetProcessID() );
  if( p && (p->PidFlags.RequestConnect == TRUE) )
  {
   beep();
   SayStatusMsg("Connect Request... Ctrl-Break or select Misc->Process");
   p->PidFlags.RequestConnect = FALSE ;
  }
  DosSleep(2000);
 }
}

/*****************************************************************************/
/*                                                                           */
/* - Get a pointer to the structure containing the pid of a debugger.        */
/*                                                                           */
/*****************************************************************************/
ALLPIDS *GetDbgPid( USHORT DbgPid )
{
 ALLPIDS *p;

 for( p = pAllpids ; p ; p = p->next )
 {
  if( p->DbgPid == DbgPid )
   break;
 }
 return(p);
}

/*****************************************************************************/
/* - Get read/write access to the shared memory heap for this debugger.      */
/*****************************************************************************/
void SetExecutionFlag( BOOL TorF )
{
 ALLPIDS *p;

 p = GetPid( DbgGetProcessID() );
 p->PidFlags.Executing = TorF;
}

/*---------------------------------------------------------------------------*/
#endif

#ifdef __ESP__
/*---------------------------------------------------------------------------*/
/*****************************************************************************/
/* AddMessage()                                                              */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Add a message to the message queue for serial multiple process.         */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   msg    the id of the message that is being queued.                      */
/*   pBuf   -> to a buffer of parms for this notification.                   */
/*   BufLen size of the parm block for this message.                         */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
void AddMessage( UCHAR msg, void *pBuf, ULONG BufLen )
{
 SQE_MESSAGE  *pSqe, *q, *r;
 ULONG         sqe_size;

 sqe_size = sizeof(SQE_MESSAGE) + BufLen - sizeof(pSqe->parms);

 pSqe = NULL;
 DosSubAllocMem( pSharedMem, (PPVOID)&pSqe, sqe_size );

 memset( pSqe, 0, sqe_size );
 pSqe->next     = NULL;
 pSqe->Reported = FALSE;
 pSqe->msg      = msg;
 pSqe->size     = sqe_size;
 memcpy( pSqe->parms, pBuf, BufLen );


 q = (SQE_MESSAGE*)&(pAllpids->pSqeMsg);
 r = pAllpids->pSqeMsg;

 for( ; r ; q = r, r = r->next ){;}

 q->next = pSqe;

 q = (SQE_MESSAGE*)&(pAllpids->pSqeMsg);
 r = pAllpids->pSqeMsg;
}

/*****************************************************************************/
/* RemoveMessage()                                                           */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Remove a message from the message queue for serial multiple process.    */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   pSqe   -> to the message element to be removed.                         */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
void RemoveMessage( SQE_MESSAGE *pSqe )
{
 SQE_MESSAGE  *q, *r;

 q = (SQE_MESSAGE*)&(pAllpids->pSqeMsg);
 r = pAllpids->pSqeMsg;

 while( r != pSqe ) { r = r->next; }

 q->next = r->next;

 for( ; r ; q = r, r = r->next ){;}

 DosSubFreeMem( pSharedMem, pSqe, pSqe->size );

 q = (SQE_MESSAGE*)&(pAllpids->pSqeMsg);
 r = pAllpids->pSqeMsg;
}

/*****************************************************************************/
/* GetConnectNotification()                                                  */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   We want to poll in notifications for connect requests that cannot       */
/*   be serviced.                                                            */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   pSqe   -> to the connect request that has not been serviced or          */
/*             reported.                                                     */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
SQE_MESSAGE *GetConnectNotification( void )
{
 SQE_MESSAGE  *pSqe;



 for( pSqe = pAllpids->pSqeMsg; pSqe ; pSqe = pSqe->next )
 {
  if( (pSqe->msg == CONNECT_DBG) && (pSqe->Reported == FALSE) )
  {
   pSqe->Reported = TRUE;
   return(pSqe);
  }
 }
 return(NULL);
}

/*****************************************************************************/
/* IsConnectRequestQueued()                                                  */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Check to see if there is a connect queued for this pid.                 */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   pid    -> to the message element to be removed.                         */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   pSqe   -> to the connect request that has not been serviced or          */
/*             reported.                                                     */
/*          NULL==> not queued.                                              */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*  pAllpids != NULL                                                         */
/*                                                                           */
/*****************************************************************************/
SQE_MESSAGE *IsConnectRequestQueued( USHORT pid )
{
 SQE_MESSAGE  *pSqe;
 USHORT        QuePid;

 for( pSqe = pAllpids->pSqeMsg; pSqe ; pSqe = pSqe->next )
 {
  if( pSqe->msg == CONNECT_DBG )
  {
   QuePid = *(USHORT*)pSqe->parms;
   if( QuePid == pid )
    return(pSqe);
  }
 }
 return(NULL);
}

/*****************************************************************************/
/* ReportMessage()                                                           */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Report a message from the message queue for serial multiple process.    */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
void ReportMessage( void )
{
 SQE_MESSAGE *pSqe = NULL;
 COMMAND      cmd;
 LHANDLE      handle;


 /****************************************************************************/
 /* - Test for the existence of the shared heap.                             */
 /****************************************************************************/
 if( pAllpids == NULL )
  return;

 /****************************************************************************/
 /* - Get the handle to the serial channel.                                  */
 /****************************************************************************/
 handle = GetComHandle();

 /****************************************************************************/
 /* - Get a pointer to the first message in the que.                         */
 /****************************************************************************/
 pSqe = pAllpids->pSqeMsg;

 if( pSqe == NULL )
 {
  /***************************************************************************/
  /* - If no messages, then simply close the serial poll loop.               */
  /***************************************************************************/
  memset(&cmd,0,sizeof(cmd) );

  cmd.api = SERIAL_POLL;
  cmd.cmd = 0;

  RmtSend( handle, &cmd, sizeof(cmd) );
 }
 else
 {
  /***************************************************************************/
  /* - Else, grab the next mesage in the queue and report it.                */
  /***************************************************************************/
  switch( pSqe->msg )
  {
   case NEW_PROCESS:
   {
    SQE_NEW_PROCESS *pnp;
    USHORT pid;
    ULONG  type;
    ULONG  mte;

    pnp  = (SQE_NEW_PROCESS*)(pSqe->parms);
    pid  = pnp->pid;
    type = pnp->type;
    mte  = pnp->mte;

    SendNewProcessToDbg( handle, pid, type, mte );
    RemoveMessage( pSqe );
   }
   break;

   case CONNECT_DBG:
   {
    ALLPIDS *p;
    USHORT   pid;
    BOOL     TorF;

    pid = *(USHORT*)pSqe->parms;


    if( IsPidConnected() == FALSE )
    {

     p   = GetPid( pid );
     ConnectDbg( handle, pid );

     p->Connect = CONNECTED;

     TorF = TRUE;
     if( p->pid == GetEspProcessID() )
      TorF = FALSE;

     PostConnectSema4( &p->ConnectSema4, TorF );
     RemoveMessage( pSqe );
    }
    else
    {
     /************************************************************************/
     /* - At this point, we have a queued connect request.                   */
     /* - If this pid is already connected, then remove it.                  */
     /* - Else, leave it in the queue and complete the serial poll.          */
     /************************************************************************/
     if( GetPidConnected()->pid == pid )
      RemoveMessage( pSqe );

     memset(&cmd,0,sizeof(cmd) );
     cmd.api = SERIAL_POLL;
     cmd.cmd = 0;

     /************************************************************************/
     /* - Check to see if there are any connect requests that haven't        */
     /*   been reported.                                                     */
     /* - If there are, then send back the pid.                              */
     /************************************************************************/
     pSqe = GetConnectNotification();
     if( pSqe )
      cmd.cmd = CONNECT_NOTIFY;

     RmtSend( handle, &cmd, sizeof(cmd) );
     if( pSqe )
     {
      pid = *(USHORT*)pSqe->parms;
      RmtSend( handle, &pid, sizeof(pid) );
     }
    }
   }
   break;
  }
 }
}
/*---------------------------------------------------------------------------*/
#endif
