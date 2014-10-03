/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*                                                                           */
/*   dbgq.c                                                                  */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Queue handling for the debugger.                                        */
/*                                                                           */
/* History:                                                                  */
/*                                                                           */
/*   06/08/94 Created.                                                       */
/*                                                                           */
/*****************************************************************************/

#include "all.h"

static PSZ     DbgQueBaseName = "\\QUEUES\\DbgQue";
static char    DbgQueName[32];
static BOOL    ReadQueReady;
static BOOL    AllDbgsAreDeadFlag;
static int     RestartInstance;

void    SetDbgQueName( char *pQueName ) { strcpy(DbgQueName, pQueName); }
BOOL    AllDbgsAreDead( void ) { return(AllDbgsAreDeadFlag);}
void    ResetAllDbgsAreDeadFlag( void ) { AllDbgsAreDeadFlag=FALSE;}

/*****************************************************************************/
/* StartDbgQue()                                                             */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*  Start up a que to handle SD386 messages.                                 */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*  rc    System API return code.                                            */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
APIRET StartDbgQue( void )
{
 APIRET rc;
 char    PidString[12];
 TIB    *pTib;
 PIB    *pPib;
 USHORT  ThisPid;

 /****************************************************************************/
 /* - Get the pid of this probe and use it to define a unique name.          */
 /* - Stuff in the base que name.                                            */
 /* - Concatenate the pid to make the name unique for the system.            */
 /****************************************************************************/
 DosGetInfoBlocks(&pTib, &pPib);
 ThisPid = (USHORT)pPib->pib_ulpid;

 memset(DbgQueName, 0, sizeof(DbgQueName));
 strcat(DbgQueName, DbgQueBaseName);

 memset(PidString, 0, sizeof(PidString));
 RestartInstance++;
 sprintf( PidString, "%d-%d", ThisPid, RestartInstance );
 strcat(DbgQueName, PidString );

 /****************************************************************************/
 /* - Set a flag to wait on indicating that the queue thread is ready to     */
 /*   read.                                                                  */
 /****************************************************************************/
 ReadQueReady = FALSE;

 rc = StartQue(DbgQueName, ReadDbgQue);
 if(rc) return(rc);

 /****************************************************************************/
 /* - Wait until the queue is ready to read.                                 */
 /****************************************************************************/
 while( ReadQueReady == FALSE ) DosSleep(100);
 return( 0 );
}

/*****************************************************************************/
/* ReadDbgQue()                                                              */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Read and handle SD386 queue messages. This function runs                */
/*   in its own thread in a do forever loop.                                 */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   qHandle        -> to the queue handle. This is passed in by the         */
/*                     StartQue() function when the queue is created.        */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*  void                                                                     */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
void ReadDbgQue( void *qHandle)
{
 APIRET       rc;
 REQUESTDATA  Request;
 ULONG        DataLen;
 BYTE         ElePri;
 BOOL         EndThread = FALSE;
 LHANDLE      LsnHandle = 0;
 ALLPIDS     *p;
 BOOL         TorF;
 USHORT       pid;
 int          ListenThreadID = 0;
 LHANDLE      QueHandle;

 DBG_QUE_ELEMENT *pDataAddr=NULL;


 if( IsVerbose() ) {printf("\nDbg Que Started");fflush(0);}
 /****************************************************************************/
 /* - The queue has its own handle for communications between itself and     */
 /*   the probe que. When connected serially, then there is only one handle  */
 /*   and it must be shared by all dbg/esp connections.                      */
 /****************************************************************************/
 if( (SerialParallel() == SERIAL) )
  LsnHandle = GetComHandle();

 QueHandle    = (LHANDLE)qHandle;

 /****************************************************************************/
 /* - The queue thread needs to be started and ready to read before          */
 /*   the StartQue() call completes. The StartQue() waits for the            */
 /*   following flag to be set before returning its caller.                  */
 /****************************************************************************/
 ReadQueReady = TRUE;

 /****************************************************************************/
 /* - Start the que reading loop.                                            */
 /****************************************************************************/
 for(; EndThread == FALSE ;)
 {
  /***************************************************************************/
  /* We're going to read an element from the dbg queue.  The queue is read   */
  /* in FIFO order and we wait until there is a message to read.  We get an  */
  /* element priority but we don't care about it.  Request.ulData contains   */
  /* the message.                                                            */
  /***************************************************************************/
  rc = DosReadQueue(QueHandle,
                    &Request,
                    &DataLen,
                    (PPVOID)&pDataAddr,
                    QUE_FIFO,
                    DCWW_WAIT,
                    &ElePri,
                    0L);

  /***************************************************************************/
  /* - Handle the private verbose option.                                    */
  /***************************************************************************/
  if( IsVerbose() ) PrintDbgQueMessage( &Request );

  switch( Request.ulData )
  {
   /**************************************************************************/
   /*  This message is added when we're trying to debug a child debugger.    */
   /**************************************************************************/
#if 0
   {
    case DBG_QMSG_REQUEST_ACCESS:
     rc = DosGiveSharedMem( GetShrMem(), pDataAddr->sid, PAG_READ | PAG_WRITE );
     printf("\naccess given to pid=%d rc=%d",pDataAddr->pid,rc);fflush(0);
     AddDbgPidAndSid( pDataAddr->pid, pDataAddr->sid, 0 );
     break;
   }
#endif

   case DBG_QMSG_SELECT_SESSION:
    /*************************************************************************/
    /* - Bring a debugger session to the foreground. This has to be done     */
    /*   by the parent debugger so it's accomplished by sending this         */
    /*   message to the que.                                                 */
    /*************************************************************************/
    p = GetPid( pDataAddr->pid );
    DosSelectSession( p->DbgSid );
    break;

   case DBG_QMSG_SELECT_PARENT_ESP:
    /*************************************************************************/
    /* - Bring the er session to the foreground. This has to be done in the  */
    /*   by the parent debugger and so it's accomplished by sending this     */
    /*   message to the que.                                                 */
    /*************************************************************************/
    p = GetPidIndex( 1 );
    DosSelectSession( p->EspSid );
    break;

   case DBG_QMSG_CTRL_BREAK:
    if( (SerialParallel() == SERIAL) && (SingleMultiple() == MULTIPLE) )
    {
     /************************************************************************/
     /* - Kill the polling thread if running serial.  This will guarantee    */
     /*   that no other messages will be serviced during ctrl-break          */
     /*   processing.                                                        */
     /* - Wait for the polling thread to die.                                */
     /* - Grab the serial mutex.                                             */
     /************************************************************************/
     SetPollingThreadFlag( THREAD_TERM );
     while( GetPollingThreadFlag() != NOT_RUNNING ){ DosSleep(100) ;}
     RequestSerialMutex();

     /************************************************************************/
     /* - The assumption at this point is that the debugger sending the      */
     /*   ctrl-break is not connected.                                       */
     /* - Give the connection to the debugger that's sending the ctrl-break. */
     /*   This will guarantee that the debugger thread will get the next     */
     /*   access to the serial mutex.                                        */
     /************************************************************************/
     {
      USHORT             GoToPid;
      ALLPIDS           *pYield;
      ALLPIDS           *pGoTo;

      pYield = GetPidConnected();

      if( pYield )
      {
       SetConnectSema4( &pYield->ConnectSema4, TRUE );
       pYield->Connect                 = DISCONNECTED;
       pYield->PidFlags.ConnectYielded = TRUE;
      }

      GoToPid         = pDataAddr->pid;
      pGoTo           = GetPid( GoToPid );
      pGoTo->Connect  = CONNECTED;

      pGoTo->PidFlags.ConnectYielded = FALSE;

      /***********************************************************************/
      /* - Set a flag to indicate whether or not the sema4 has to be opened  */
      /*   before it is posted.                                              */
      /* - Post the connect sema4 for this debugger.                         */
      /***********************************************************************/
      TorF = TRUE;
      if( GoToPid == DbgGetProcessID() )
       TorF = FALSE;

      PostConnectSema4( &pGoTo->ConnectSema4, TorF ); /* do not move!!!          */
     }
    }

    /*************************************************************************/
    /* - Send the ctrl-break.                                                */
    /*************************************************************************/
    xSendCtrlBreak(LsnHandle, pDataAddr->pid );

    if( (SerialParallel() == SERIAL) && (SingleMultiple() == MULTIPLE) )
    {
     /************************************************************************/
     /* - Release the serial mutex.                                          */
     /* - Start the polling thread back up.                                  */
     /************************************************************************/
     ReleaseSerialMutex();
     _beginthread( PollForMsgFromEsp, NULL, 0x8000, NULL );
    }
    break;

   case DBG_QMSG_OPEN_CONNECT:
    /*************************************************************************/
    /* - This message comes with parallel connections only.                  */
    /* - A separate connection is established between the dbg and esp ques.  */
    /*************************************************************************/
    OpenAnotherConnection( &LsnHandle );
    ListenThreadID = _beginthread(ListenForMsgFromEsp, NULL, 0x8000, (void *)LsnHandle);
    break;

   case DBG_QMSG_NEW_PROCESS:
    /*************************************************************************/
    /* - All the probes send their new process notifications here.           */
    /*************************************************************************/
    {
     USHORT          DbgPid;
     ULONG           DbgSid;
     ULONG           ErrorMsgLen;
     QUE_ERR_ELEMENT Que_ErrElement;

     /************************************************************************/
     /* - If debugging serial/multiple, then at this point the polling       */
     /*   thread flag will be ended.                                         */
     /************************************************************************/

     /************************************************************************/
     /* - Add a node to the shared heap for this process.                    */
     /************************************************************************/
     AddPid( pDataAddr->pid,
             pDataAddr->sid,
             0,
             pDataAddr->type,
             pDataAddr->FileSpec );

     /************************************************************************/
     /* - Spawn a debugger for the new process.                              */
     /************************************************************************/
     ErrorMsgLen = sizeof( Que_ErrElement.ErrorMsg );

     memset( Que_ErrElement.ErrorMsg, 0, ErrorMsgLen );

     rc = SpawnDbg(  pDataAddr->pid,
                    &DbgPid,
                    &DbgSid,
                     DbgQueName,
                     ErrorMsgLen,
                     Que_ErrElement.ErrorMsg );

      if( rc && (rc != ERROR_SMG_START_IN_BACKGROUND) )
      {
       Que_ErrElement.rc = rc;

       SendMsgToDbgQue(DBG_QMSG_ERROR, &Que_ErrElement, sizeof(Que_ErrElement));
      }

     /************************************************************************/
     /* - Add the process id and the session id of the spawned debugger      */
     /*   to the pid structure.                                              */
     /************************************************************************/
     AddDbgPidAndSid( pDataAddr->pid, DbgPid, DbgSid );

     if( (SerialParallel() == SERIAL) && (SingleMultiple()==MULTIPLE) )
     {
      /***********************************************************************/
      /* - For serial multiple process debugging:                            */
      /***********************************************************************/
      p = GetPidConnected();

      if( p )
      {
       /**********************************************************************/
       /* - Preempt the current connection.                                  */
       /**********************************************************************/

       TorF = TRUE;
       if( p->pid == DbgGetProcessID() )
        TorF = FALSE;

       SetConnectSema4( &p->ConnectSema4, TorF );

       p->PidFlags.ConnectYielded = TRUE;
       p->Connect                 = DISCONNECTED;
      }

      /***********************************************************************/
      /* - Connect the new process.                                          */
      /***********************************************************************/
      pid        = pDataAddr->pid;
      p          = GetPid( pid );
      p->Connect = CONNECTED;

      /**********************************************************************/
      /* - Wait for the connect sema4 to be created and put into the shared */
      /*   heap structure.                                                  */
      /**********************************************************************/
      if( (SerialParallel() == SERIAL) && (SingleMultiple() == MULTIPLE) )
      {
       while( p->ConnectSema4 == 0 ) { DosSleep(200); }
      }

      /**********************************************************************/
      /* - Connect the probe of the new process.                            */
      /* - Post the connect sema4 and let the new process start.            */
      /**********************************************************************/
      xConnectEsp( LsnHandle, pid );
      PostConnectSema4( &p->ConnectSema4, TRUE );

      /**********************************************************************/
      /* - Wait for the debugger to finish initializing.                    */
      /**********************************************************************/
      while( p->PidFlags.Initializing == TRUE) { DosSleep(200); }

      /***********************************************************************/
      /* - Start the polling thread back up.                                 */
      /***********************************************************************/
      _beginthread( PollForMsgFromEsp, NULL, 0x8000, NULL );
     }
    }
    break;

/*---------------------------------------------------------------------------*/
/* All of the messages until the next dashed comment line are for serial     */
/* connections only.                                                         */
/*---------------------------------------------------------------------------*/

   case DBG_QMSG_CONNECT_ESP:
    /************************************************************************/
    /* - Serial message only.                                               */
    /* - Kill the polling thread if running serial.  This will guarantee    */
    /*   that no other messages will be serviced during ctrl-break          */
    /*   processing.                                                        */
    /* - Wait for the polling thread to die.                                */
    /* - Grab the serial mutex.                                             */
    /************************************************************************/
    if( GetPollingThreadFlag() == RUNNING )
    {
     SetPollingThreadFlag( THREAD_TERM );
     while( GetPollingThreadFlag() != NOT_RUNNING ){ DosSleep(100) ;}
    }
    RequestSerialMutex();

    /*************************************************************************/
    /* - Serial channel message only.                                        */
    /* - Disconnect the probe that is currently connected and connect the    */
    /*   probe for the pid specified in the message.                         */
    /*************************************************************************/
    {
     USHORT             YieldPid;
     USHORT             GoToPid;
     ALLPIDS           *pYield;
     ALLPIDS           *pGoTo;

     YieldPid = (USHORT)pDataAddr->sid;
     GoToPid  = pDataAddr->pid;

     /************************************************************************/
     /* - Disconnect the current pid.                                        */
     /************************************************************************/
     if( YieldPid != 0 )
     {
      pYield          = GetPid( YieldPid );
      pYield->Connect = DISCONNECTED;

      pYield->PidFlags.ConnectYielded = TRUE;
     }

     /************************************************************************/
     /* - Connect the new pid.                                               */
     /************************************************************************/
     pGoTo           = GetPid( GoToPid );
     pGoTo->Connect  = CONNECTED;
     pGoTo->PidFlags.ConnectYielded = FALSE;

     /************************************************************************/
     /* - Connect the probe for the goto pid.                                */
     /************************************************************************/
     if( pDataAddr->DbgMsgFlags.InformEsp == TRUE )
      xConnectEsp( LsnHandle, GoToPid );

     /************************************************************************/
     /* - Set a flag to indicate whether or not the sema4 has to be opened   */
     /*   before it is posted.                                               */
     /* - Post the connect sema4 for this debugger.                          */
     /************************************************************************/
     TorF = TRUE;
     if( GoToPid == DbgGetProcessID() )
      TorF = FALSE;

     PostConnectSema4( &pGoTo->ConnectSema4, TorF ); /* do not move!!!           */

     ReleaseSerialMutex();
     _beginthread( PollForMsgFromEsp, NULL, 0x8000, NULL );
    }
    break;

   case DBG_QMSG_DISCONNECT:
    /*************************************************************************/
    /* - One of the esp execution services is being executed and is          */
    /*   giving up its connection.                                           */
    /*************************************************************************/
    p          = GetPid( pDataAddr->pid );
    p->Connect = DISCONNECTED;
    break;

/*---------------------------------------------------------------------------*/

   case DBG_QMSG_ERROR:
   {
    QUE_ERR_ELEMENT *pQueErrElement =  (QUE_ERR_ELEMENT *)pDataAddr;
    char            *pErrorMsg;
    char             RcMessage[32];

    rc        = pQueErrElement->rc;
    pErrorMsg = pQueErrElement->ErrorMsg;

    DosSelectSession(0);

    sprintf( RcMessage, "Esp Error rc=%d", rc );

    ErrorPrintf( ERR_DBG_QUE_ERROR, 2, RcMessage, pErrorMsg );
   }
   break;

   case DBG_QMSG_KILL_LISTEN:
    /*************************************************************************/
    /* - Kill the listen thread or the polling threads.                      */
    /*************************************************************************/
    if( ListenThreadID )
     xSendKillListen( LsnHandle );
    break;

   case DBG_QMSG_PARENT_TERM:
    /*************************************************************************/
    /* - The parent probe is terminating.                                    */
    /*************************************************************************/
    for( p = GetAllpids(); p ; p = p->next )
    {
     if( p->pid != DbgGetProcessID() )
     {
      DBG_QUE_ELEMENT Qelement;

      Qelement.pid = p->DbgPid;

      SendMsgToDbgTermQue(DBG_TERMINATE, &Qelement, sizeof(Qelement) );
     }
    }
    /*************************************************************************/
    /* - There may not be any child dbgs, so set this flag so the parent     */
    /*   can terminate.                                                      */
    /*************************************************************************/
    if(GetAllpids()->next == NULL)
    {
     AllDbgsAreDeadFlag = TRUE;
    }
    break;

   case DBG_QMSG_CHILD_TERM:
    /*************************************************************************/
    /* - A child probe has terminated.                                       */
    /*************************************************************************/
    RemovePid( pDataAddr->pid );

    /*************************************************************************/
    /* - After the last probe has terminated, then report all probes dead.   */
    /*************************************************************************/
    if(GetAllpids()->next == NULL)
    {
     AllDbgsAreDeadFlag = TRUE;
    }

    if( SerialParallel() == SERIAL )
    {
     DBG_QUE_ELEMENT Qelement;

     /************************************************************************/
     /* - If the child is the currently connected debugger, then we          */
     /*   we need to give the focus to another debugger. So,                 */
     /*   give the focus to the parent debugger.                             */
     /************************************************************************/
     memset( &Qelement, 0, sizeof(Qelement) );
     Qelement.pid = DbgGetProcessID();

     SendMsgToDbgQue( DBG_QMSG_SELECT_SESSION, &Qelement, sizeof(Qelement) );
    }
    break;

   case DBG_QMSG_QUE_TERM:
    /*************************************************************************/
    /* - Terminate and reset the queue for a possible restart.               */
    /* - Close the queue to queue message channel.                           */
    /* - Free the shared memory allocated for the pid structures.            */
    /*************************************************************************/
    DosCloseQueue(QueHandle);

    if( (SerialParallel() == PARALLEL) && LsnHandle )
     ConnectClose( LsnHandle );

    if( SingleMultiple() == MULTIPLE )
     FreeSharedHeap();

    QueHandle     = 0;
    EndThread     = TRUE;
    ReadQueReady  = 0;

    break;
  }

  /***************************************************************************/
  /* - Free the shared memory used for the message and initialize the        */
  /*   data element pointer for the next message.                            */
  /***************************************************************************/
  if( pDataAddr )
   DosFreeMem(pDataAddr);

  pDataAddr = NULL;
 }

 /****************************************************************************/
 /* - Come here after the DBG_QMSG_QUE_TERM message and end the que          */
 /*   thread. We start all of this stuff back up on a restart.               */
 /****************************************************************************/
 if( IsVerbose() ) {printf("\nDbg Que Ended");fflush(0);}
 _endthread();
}                                       /* end ReadQ().                      */

/*****************************************************************************/
/* SendMsgToDbgQue()                                                         */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*  Send DBG_QMSG_xxxxxx message to DBG queue. We pass through here just     */
/*  to pick up the queue name.                                               */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   Msg            DBG_QMSG_xxxxxx message.                                 */
/*   PQueElement    -> to optional queue element( buffer ).                  */
/*   QueElementLen  length of the queue element.                             */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
void SendMsgToDbgQue(ULONG Msg , void *pQueElement, ULONG QueElementLen)
{
 SendMsgToQue( DbgQueName, Msg, pQueElement, QueElementLen );
}

/*****************************************************************************/
/* ListenForMsgFromEsp()                                                     */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   This function runs in its own thread and listens messages coming        */
/*   to the queue from esp. This thread only exists when we're running       */
/*   with a parallel connection.                                             */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   handle   this is the handle to the connection.                          */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
void ListenForMsgFromEsp( void *handle )
{
 LHANDLE          LsnHandle = (LHANDLE)handle;
 COMMAND          cmd;
 DBG_QUE_ELEMENT  Qelement;
 BOOL             EndThread;

 TXRX_NEW_PROCESS packet;

 if( IsVerbose() ) {printf("\nListen thread started");fflush(0);}
 for( EndThread = FALSE ; EndThread == FALSE ; )
 {
  /***************************************************************************/
  /* - Wait for a message coming from the probe.                             */
  /***************************************************************************/
  memset(&cmd,0,sizeof(cmd) );
  RmtRecv(LsnHandle, (char*)&cmd, sizeof(cmd));

  if( IsVerbose() ) PrintCmdMessage( cmd.api ) ;
  switch( cmd.api )
  {
   case NEW_PROCESS:

    /*************************************************************************/
    /* - Handle new process notifications coming back from the remote probe. */
    /*************************************************************************/
    RmtRecv( LsnHandle, &packet, cmd.len );

    Qelement.pid  = packet.pid;
    Qelement.type = packet.type;
    strcpy(Qelement.FileSpec, packet.ProcessFileSpec);

    /*************************************************************************/
    /* - Pass the message on to the que.                                     */
    /*************************************************************************/
    SendMsgToDbgQue( DBG_QMSG_NEW_PROCESS, &Qelement, sizeof(Qelement) );
    break;

   case KILL_LISTEN_THREAD:
   /**************************************************************************/
   /* - This command will come from the esp queue when it gets a command     */
   /*   to kill the listen thread during termination. The reason we do       */
   /*   it this way is that DosKillThread() was causing a session hang       */
   /*   when using MPTS on LS 4.0. So now, the debugger sends a command      */
   /*   to the probe telling it to kill its listen thread. The probe sends   */
   /*   a command back to the debugger telling it to also kill its listen    */
   /*   thread and we end up raght 'chere.                                   */
   /**************************************************************************/
   EndThread = TRUE;
   break;
  }
 }
 if( IsVerbose() ) {printf("\nListen thread ended");fflush(0);}
 _endthread();
}

/*****************************************************************************/
/* PollForMsgFromEsp()                                                       */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   This function runs in its own thread and polls for messages from        */
/*   esp. It's only applicable to serial multiple process configurations.    */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   PollingThreadFlag states:                                               */
/*                                                                           */
/*    RUNNING        Thread is running.                                      */
/*    NOT_RUNNING    Thread is not running.                                  */
/*    THREAD_TERM    Transitional state from RUNNING to NOT_RUNNING.         */
/*                                                                           */
/*****************************************************************************/
static int  PollingThreadFlag = NOT_RUNNING;
void   SetPollingThreadFlag( int state ) { PollingThreadFlag = state;}
int    GetPollingThreadFlag( void ) { return( PollingThreadFlag ); }
void PollForMsgFromEsp( void *dummy )
{
 LHANDLE LsnHandle = GetComHandle();
 COMMAND cmd;

 PollingThreadFlag = RUNNING;

 if( IsVerbose() ) {printf("\nPoll Thread Started");fflush(0);}
 for( ; PollingThreadFlag == RUNNING ; )
 {
  DosSleep(200);

  RequestSerialMutex();

  /***************************************************************************/
  /* - If this thread acquires the mutex after the termination flag has      */
  /*   been set, then go ahead and terminate.                                */
  /***************************************************************************/
  memset(&cmd,0,sizeof(cmd) );

  cmd.api = SERIAL_POLL;
  cmd.cmd = 0;
  if( PollingThreadFlag == THREAD_TERM )
  {;
  }
  else
  {
   if( IsVerbose() ) { printf("%c",'_'); fflush(0); }

   RmtSend( LsnHandle, &cmd, sizeof(cmd) );
   RmtRecv( LsnHandle, &cmd, sizeof(cmd) );
  }

  if( cmd.cmd != 0 )
  {

   if( IsVerbose() ) PrintCmdMessage( (signed char)cmd.cmd ) ;

   switch( (signed char)cmd.cmd ) /* <---- Note this is the ".cmd" member  */
   {
    case NEW_PROCESS:
    {
     DBG_QUE_ELEMENT  Qelement;
     TXRX_NEW_PROCESS packet;

     /************************************************************************/
     /* - Handle new process notifications coming back from the remote probe.*/
     /************************************************************************/
     RmtRecv( LsnHandle, &packet, cmd.len );

     Qelement.pid = packet.pid;
     strcpy(Qelement.FileSpec, packet.ProcessFileSpec);

     /*************************************************************************/
     /* - Pass the message on to the que.                                     */
     /*************************************************************************/
     SendMsgToDbgQue( DBG_QMSG_NEW_PROCESS, &Qelement, sizeof(Qelement) );
     PollingThreadFlag = THREAD_TERM;
    }
    break;

    case CONNECT_DBG:
    {
     USHORT   pid;
     ALLPIDS *p;
     BOOL     TorF;

     /************************************************************************/
     /* - Handle connect requests coming from the remote probe.              */
     /************************************************************************/
     {
      DBG_QUE_ELEMENT Qelement;

      /***********************************************************************/
      /* - Receive the pid to be connected and connect it.                   */
      /***********************************************************************/
      RmtRecv( LsnHandle, &pid, cmd.len );

      p = GetPid( pid );

      p->Connect = CONNECTED;

      TorF = TRUE;
      if( p->pid == DbgGetProcessID() )
       TorF = FALSE;

      PostConnectSema4( &p->ConnectSema4, TorF );

      /***********************************************************************/
      /* - Select the session of the debugger for this pid.                  */
      /***********************************************************************/
      Qelement.pid = pid;
      SendMsgToDbgQue( DBG_QMSG_SELECT_SESSION, &Qelement, sizeof(Qelement) );

      /***********************************************************************/
      /* - Tell the probe that the connection was made. A handshaking        */
      /*   maneuver.                                                         */
      /***********************************************************************/
      memset(&cmd,0,sizeof(cmd) );
      cmd.api = CONNECT_DBG;
      RmtSend( LsnHandle, &cmd , sizeof(cmd) );
     }
    }
    break;

    case CONNECT_NOTIFY:
    {
     ALLPIDS *p;
     USHORT   pid;

     /************************************************************************/
     /* - Receive the pid requesting the connection.                         */
     /* - Turn on a flag to tell the debugger for this pid to pick up        */
     /*   his notification.                                                  */
     /************************************************************************/
     RmtRecv( LsnHandle, &pid, sizeof(pid) );

     p = GetPid( pid );
     p->PidFlags.RequestConnect = TRUE;
    }
    break;
   }
  }
  ReleaseSerialMutex();
 }
 PollingThreadFlag = NOT_RUNNING;
 if( IsVerbose() ) {printf("\nPoll Thread Ended");fflush(0);}
 _endthread();
}

/*****************************************************************************/
/* StartDbgTermQue()                                                         */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*  Start up a que for terminating the debugger.                             */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*  rc    System API return code.                                            */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
static PSZ TermQueBaseName = "\\QUEUES\\TermQue";

APIRET StartDbgTermQue( void )
{
 APIRET   rc;
 TIB     *pTib;
 PIB     *pPib;
 USHORT   ThisPid;
 char     TermQueName[32];
 char     PidString[12];
 ALLPIDS *p;

 /****************************************************************************/
 /* - Get the pid of this debugger and use it to define a unique name        */
 /*   for this debugger's termination que.                                   */
 /* - Stuff in the base que name.                                            */
 /* - Concatenate the pid to make the name unique for the system.            */
 /****************************************************************************/
 DosGetInfoBlocks(&pTib, &pPib);
 ThisPid = (USHORT)pPib->pib_ulpid;

 memset(TermQueName, 0, sizeof(TermQueName));
 strcat(TermQueName, TermQueBaseName);

 memset(PidString, 0, sizeof(PidString));
 sprintf( PidString, "%d", ThisPid );
 strcat(TermQueName, PidString );

 /****************************************************************************/
 /* - Now, start the que with a thread for this debugger.                    */
 /****************************************************************************/
 rc = StartQue(TermQueName, ReadDbgTermQue );
 if(rc) return(rc);

 /****************************************************************************/
 /* - Now, add the name of the queue to the shared memory structure for      */
 /*   this debugger.                                                         */
 /****************************************************************************/
 p = GetDbgPid( ThisPid );

 DosSubAllocMem( GetShrMem(), (PPVOID)&(p->pTermQue), strlen(TermQueName) + 1 );
 strcpy(p->pTermQue, TermQueName );

 return(0);
}

/*****************************************************************************/
/* ReadDbgTermQue()                                                          */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Read and handle termination message(s) coming to this debugger          */
/*   from the parent debugger.                                               */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   qHandle        -> to the queue handle. This is passed in by the         */
/*                     StartQue() function when the queue is created.        */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*  void                                                                     */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*  This function runs in its own thread.                                    */
/*                                                                           */
/*****************************************************************************/
void ReadDbgTermQue( void *qHandle)
{
 REQUESTDATA  Request;
 ULONG        DataLen;
 BYTE         ElePri;
 BOOL         EndThread = FALSE;
 LHANDLE      TermQueHandle;

 DBG_QUE_ELEMENT *pDataAddr=NULL;

 TermQueHandle = (LHANDLE)qHandle;

 /****************************************************************************/
 /* - Start the que reading loop.                                            */
 /****************************************************************************/
 for(; EndThread == FALSE ;)
 {
  /***************************************************************************/
  /* We're going to read an element from the term queue.  The queue is read  */
  /* in FIFO order and we wait until there is a message to read.  We get an  */
  /* element priority but we don't care about it.  Request.ulData contains   */
  /* the message.                                                            */
  /***************************************************************************/
  DosReadQueue(TermQueHandle,
               &Request,
               &DataLen,
               (PPVOID)&pDataAddr,
               QUE_FIFO,
               DCWW_WAIT,
               &ElePri,
               0L);

  /***************************************************************************/
  /* - Handle the termination que message(s).                                */
  /***************************************************************************/
  switch( Request.ulData )
  {
   case DBG_TERMINATE:
   {
    USHORT          pid;
    DBG_QUE_ELEMENT Qelement;

    pid = DbgGetProcessID();

    CloseConnectSema4();
    ConnectClose( DEFAULT_HANDLE );

    memset( &Qelement, 0, sizeof(Qelement) );
    Qelement.pid = pid;
    SendMsgToDbgQue( DBG_QMSG_CHILD_TERM, &Qelement, sizeof(Qelement) );

    EndThread = TRUE;
   }
   break;

   case DBG_PROBE_TERM:
   {
    SetHaltCommunicationsFlag();
   }
   break;

   default:
    break;
  }

  /***************************************************************************/
  /* - Free the shared memory used for the message and initialize the        */
  /*   data element pointer for the next message.                            */
  /***************************************************************************/
  if( pDataAddr )
   DosFreeMem(pDataAddr);

  pDataAddr = NULL;
 }

 /****************************************************************************/
 /* - Come here to end the que reading thread.                               */
 /****************************************************************************/
 if( IsVerbose() ) {printf("\nDebugger terminated");fflush(0);}
 exit(0);
}                                       /* end ReadQ().                      */

/*****************************************************************************/
/* SendMsgToDbgTermQue()                                                     */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*  Send TRM_QMSG_xxxxxx message to a child debugger.                        */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   Msg            TRM_QMSG_xxxxxx message.                                 */
/*   PQueElement    -> to optional queue element( buffer ).                  */
/*   QueElementLen  length of the queue element.                             */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   The process id receiving the queue message is passed in the queue       */
/*   element.                                                                */
/*                                                                           */
/*****************************************************************************/
void SendMsgToDbgTermQue(ULONG Msg , void *pQueElement, ULONG QueElementLen)
{
 USHORT   DbgPid;
 ALLPIDS *p;
 char    *pQueName;

 /****************************************************************************/
 /* - At this time, we are not actually sending anything in a queue element. */
 /*   However, we're allowing for future expansion.                          */
 /****************************************************************************/

 /****************************************************************************/
 /* - Get a ptr to the shared memory structure of the queue owner.           */
 /****************************************************************************/
 DbgPid = ((DBG_QUE_ELEMENT*)pQueElement)->pid;
 p      = GetDbgPid( DbgPid );
 if( p )
 {
  pQueName = p->pTermQue;
  SendMsgToQue( pQueName, Msg, pQueElement, QueElementLen );
 }
}
