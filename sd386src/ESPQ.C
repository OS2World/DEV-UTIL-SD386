/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*                                                                           */
/*   espq.c                                                                  */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Queue handling for Esp.                                                 */
/*                                                                           */
/* History:                                                                  */
/*                                                                           */
/*   04/14/94 Created.                                                       */
/*                                                                           */
/*****************************************************************************/

#include "all.h"

static PSZ     EspQueBaseName = "\\QUEUES\\EspQue";
static char    EspQueName[32];
static BOOL    ReadQueReady;
static BOOL    AllProbesAreDeadFlag;
static int     RestartInstance = 0;

void    SetEspQueName( char *pQueName ) { strcpy(EspQueName, pQueName); }
BOOL    AllProbesAreDead( void ) { return(AllProbesAreDeadFlag);}
void    ResetAllProbesAreDeadFlag( void ) { AllProbesAreDeadFlag=FALSE;}

/*****************************************************************************/
/* StartEspQue()                                                             */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*  Start up a que to handle esp messages.                                   */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*    rc                                                                     */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
APIRET StartEspQue( void )
{
 APIRET  rc;
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

 memset(EspQueName, 0, sizeof(EspQueName));
 strcat(EspQueName, EspQueBaseName);

 memset(PidString, 0, sizeof(PidString));
 RestartInstance++;
 sprintf( PidString, "%d-%d", ThisPid, RestartInstance );
 strcat(EspQueName, PidString );

 /****************************************************************************/
 /* - Set a flag to wait on indicating that the queue thread is ready to     */
 /*   read.                                                                  */
 /****************************************************************************/
 ReadQueReady = FALSE;

 rc = StartQue(EspQueName, ReadEspQue);
 if(rc) return(rc);

 /****************************************************************************/
 /* - Also wait for the read queue thread to be started. In some cases,      */
 /*   the DosStartSession() may cause the queue to be posted before(!)       */
 /*   the queue thread gets started. So, we have to guarantee that this      */
 /*   doesn't happen.                                                        */
 /****************************************************************************/
 while( ReadQueReady == FALSE ) DosSleep(100);
 return( 0 );
}

/*****************************************************************************/
/* - Get the pointer to the EspQueName.                                      */
/*****************************************************************************/
PSZ GetEspQue( void )
{
 return(EspQueName);
}

/*****************************************************************************/
/* ReadEspQue()                                                              */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Read and handle the Esp queue messages. This function runs              */
/*   in its own thread in a do forever loop.                                 */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   pQueHandle     -> to the queue handle. This is passed in by the         */
/*                     StartQue() function when the queue is created.        */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*  void                                                                     */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
void ReadEspQue( void *qHandle)
{
 REQUESTDATA  Request;
 ULONG        DataLen;
 BYTE         ElePri;
 ULONG        EleCode;
 USHORT       pid;
 ULONG        sid;
 ULONG        type;
 USHORT       mte;
 APIRET       rc;
 BOOL         EndThread = FALSE;
 BOOL         ParentStarted = FALSE;
 LHANDLE      LsnHandle = 0;
 ALLPIDS     *p;
 BOOL         TorF;
 LHANDLE      QueHandle;
 BOOL         ParentTerminating = FALSE;

 ESP_QUE_ELEMENT *pDataAddr=NULL;

 if( IsVerbose() ) {printf("\nEsp Que Started");fflush(0);}
 /****************************************************************************/
 /* - The queue has its own handle for communications between itself and     */
 /*   the dbg que.   When connected serially, then there is only one handle  */
 /*   and it must be shared by all dbg/esp connections.                      */
 /****************************************************************************/
 if( (SerialParallel() == SERIAL) )
  LsnHandle = GetComHandle();

 QueHandle = (LHANDLE)qHandle;
 EleCode   = 0;

 /****************************************************************************/
 /* - The starting of the queue is delayed until this flag is set.  The      */
 /*   reason is that in some cases, the DosStartSession() may cause the      */
 /*   queue to be posted before(!)  the queue thread gets started.  So,      */
 /*   we want to guarantee that que will be started and ready to read        */
 /*   before DosStartSession() is executed.                                  */
 /****************************************************************************/
 ReadQueReady = TRUE;

 for(; EndThread == FALSE ;)
 {
  /***************************************************************************/
  /* We're going to read an element from the esp queue.  The queue is read   */
  /* in FIFO order and we wait until there is a message to read.  We get an  */
  /* element priority but we don't care about it.  Request.ulData contains   */
  /* the message.                                                            */
  /*                                                                         */
  /***************************************************************************/
  rc = DosReadQueue(QueHandle,
                    &Request,
                    &DataLen,
                    (PPVOID)&pDataAddr,
                    EleCode,
                    DCWW_WAIT,
                    &ElePri,
                    0L);

  /***************************************************************************/
  /* - Handle the private verbose option.                                    */
  /***************************************************************************/
  if( IsVerbose() ) PrintQueMessage( &Request,pDataAddr);

  switch( Request.ulData )
  {
   case ESP_QMSG_SELECT_SESSION:
    pid = pDataAddr->ChildPid;
    p   = GetPid( pid );
    rc  = DosSelectSession( p->sid );
    if( IsVerbose() ) {printf("\nSelect Session id=%d rc=%d",p->sid, rc);fflush(0);}
    break;

   case ESP_QMSG_END_SESSION:
    break;

   case ESP_QMSG_OPEN_CONNECT:
    /*************************************************************************/
    /* - This message comes with parallel connections only.                  */
    /* - A separate connection is established between the dbg and esp ques.  */
    /*************************************************************************/
    OpenAnotherConnection( &LsnHandle );
    _beginthread(ListenForMsgFromDbg,NULL,0x8000,(void *)LsnHandle);
    break;

   case ESP_QMSG_NEW_SESSION:
   case ESP_QMSG_NEW_PROCESS:
    /*************************************************************************/
    /*                                                                       */
    /* - DosStartSession() will send the queue a new session notification    */
    /*   when the parent process is started. We ignore that notification     */
    /*   and send our own new process notification from EspStartUser().      */
    /* - The parent probe will debug the parent process or the first         */
    /*   child process that has been designated as debuggable.               */
    /* - After the parent probe locks in on the process that it will         */
    /*   be debugging, subsequent child processes will be assigned a         */
    /*   child probe.                                                        */
    /* - If a new process notification comes for a non-debug process,        */
    /*   then we simply start a generic spin thread to handle it.            */
    /*                                                                       */
    /*           Is this pid one that we're debugging?                       */
    /*             |                              |                          */
    /*             | TRUE                         | FALSE                    */
    /*             |                              |                          */
    /*     Has the Parent Session           Start a spin thread.             */
    /*     been started?                                                     */
    /*      |                  |                                             */
    /*      | TRUE             | FALSE                                       */
    /*      |                  |                                             */
    /*     Spawn a probe      Release the parent                             */
    /*     for this process.  session and note                               */
    /*                        parent started.                                */
    /*                                                                       */
    /*************************************************************************/
    pid = pDataAddr->ChildPid;

    if( IsProcessInList(pid) == TRUE)
     continue;

    AddProcessToList(pid);

    TorF = IsDebug( NULL, pid, &sid, &type, &mte );
    if( IsVerbose() )
    {
     CHAR ModNameBuf[CCHMAXPATH];

     DosQueryModuleName( mte, CCHMAXPATH, ModNameBuf );
     printf("\n%s", ModNameBuf);
     printf(" pid=%d sid=%d", pid, sid);
     fflush(0);
    }
    /*************************************************************************/
    /* - Add the pid to the list of pids and set a bit to indicate           */
    /*   whether or not this pid is being debugged.                          */
    /*************************************************************************/

    AddPid(pid, 0, 0, 0, NULL );
    p = GetPid( pid );
    p->PidFlags.IsDebug = TorF;

    /*************************************************************************/
    /* - if we're debugging a single child process and that process is       */
    /*   being exec'd multiple times, then we debug only the first instance. */
    /*************************************************************************/
    if( (TorF == TRUE) &&
        (SingleMultiple() == SINGLE) &&
        (ParentStarted == TRUE)
      )
    {
     _beginthread( SpinThread, NULL, 0x8000, (void*)pid);
     continue;
    }

    if( TorF == FALSE )
    {
     /************************************************************************/
     /* - Not debugging this process...start a thread to run it.             */
     /************************************************************************/
     _beginthread( SpinThread, NULL, 0x8000, (void*)pid);
    }
    else
    {
     if( ParentStarted == TRUE )
     {
      /***********************************************************************/
      /* - This process is one that we want to debug.                        */
      /* - Come here if the parent probe has already been started.           */
      /***********************************************************************/
      USHORT            EspPid;
      ULONG             EspSid;
      ULONG             ErrorMsgLen;
      QUE_ERR_ELEMENT   Que_ErrElement;

      /***********************************************************************/
      /* - Send a message to the dbg que about the new process.              */
      /***********************************************************************/
      if( (SerialParallel() == SERIAL) && (SingleMultiple() == MULTIPLE) )
      {
       SQE_NEW_PROCESS sqe;
       sqe.pid  = pid;
       sqe.type = type;
       sqe.mte  = mte;
       AddMessage( NEW_PROCESS, &sqe, sizeof(sqe) );
      }
      else
      {
       SendNewProcessToDbg( LsnHandle, pid, type, mte );
      }
      /***********************************************************************/
      /* - Start the probe and handle any errors.                            */
      /***********************************************************************/
      {
       ESP_SPAWN_FLAGS SpawnFlags;

       /**********************************************************************/
       /* - Define the flags telling SpawnProbe() how to spawn the child     */
       /*   child probe.                                                     */
       /* - When running child/multiple processes on a single machine,       */
       /*   the probes are made invisible. They will still show up in the    */
       /*   task list.                                                       */
       /**********************************************************************/
       SpawnFlags.Visible     = ESP_VISIBLE;
       SpawnFlags.SpawnMethod = ESP_USE_DOSSTARTSESSION;

       if( UseExecPgm() == TRUE )
        SpawnFlags.SpawnMethod = ESP_USE_DOSEXECPGM;
       else
        if( ConnectType()==LOCAL_PIPE )
         SpawnFlags.Visible = ESP_INVISIBLE;

       ErrorMsgLen = sizeof(Que_ErrElement.ErrorMsg);

       memset( Que_ErrElement.ErrorMsg, 0, ErrorMsgLen );

       SpawnFlags.SpawnOrigin = ESP_SPAWN_FROM_ESP;
       rc = SpawnProbe( &EspPid,
                        &EspSid,
                         EspQueName,
                         ErrorMsgLen-1, /* guarantee null termination. */
                         Que_ErrElement.ErrorMsg,
                         SpawnFlags );
      }
      if( rc )
      {
       Que_ErrElement.rc = rc;

       SendMsgToEspQue(ESP_QMSG_ERROR, &Que_ErrElement, sizeof(Que_ErrElement));
      }

      /***********************************************************************/
      /* - Add the session id and process id of the "probe" to the           */
      /*   structure for this pid.                                           */
      /* - When there is a flurry of new process notifications we cannot     */
      /*   guarantee that the probe generated for this pid will be connected */
      /*   to by the debugger for this pid.  So,we invalidate the pid making */
      /*   the probe we've just spawned a generic probe.  Later, a debugger  */
      /*   will connect to this probe and tell it what pid it will be        */
      /*   debugging.                                                        */
      /* - However, we need to make a note that there has been a probe       */
      /*   spawned for the new process pid so that we don't spawn multiple   */
      /*   probes for a pid.                                                 */
      /***********************************************************************/
      p                   = GetPid( pid );
      p->EspPid           = EspPid;
      p->EspSid           = EspSid;
      p->pid              = 0;

      /***********************************************************************/
      /* - Wait for the connect sema4 to be created and put into the shared  */
      /*   heap structure.                                                   */
      /***********************************************************************/
      if( (SerialParallel() == SERIAL) && (SingleMultiple() == MULTIPLE) )
      {
       while( p->ConnectSema4 == 0 ) { DosSleep(200); }
      }
     }
     else
     {
      /***********************************************************************/
      /* - Come here if the parent probe has not been started.               */
      /***********************************************************************/
      ParentStarted = TRUE;
      SetParentProcessID( pid);
      SetParentSessionID( sid);
      PostWait4ChildSema4();
     }
    }
    break;

   case ESP_QMSG_CTRL_BREAK:
    /*************************************************************************/
    /* - This message comes when the user enters Ctrl-Break and responds     */
    /*   that he wants to "stop the debuggee."                               */
    /*************************************************************************/
    {
     USHORT             YieldPid;
     USHORT             CtrlBreakPid;
     ALLPIDS           *pYield;
     ALLPIDS           *pCtrlBreak;
     SQE_MESSAGE       *pSqe;

     CtrlBreakPid = pDataAddr->ChildPid;
     pCtrlBreak   = GetPid( CtrlBreakPid );

     if( (SerialParallel() == SERIAL) && (SingleMultiple() == MULTIPLE) )
     {
      /***********************************************************************/
      /* - If the connection is serial, then we have to:                     */
      /*    - Disconnect a pid that may be currently connected.              */
      /*    - Connect the pid that's the target of the ctrl-break.           */
      /*    - Send the kill process signal unless there is a connect request */
      /*      queued for the target pid. In this case, simply let the        */
      /*      connect be processed.                                          */
      /***********************************************************************/
      pCtrlBreak->Connect = CONNECTED;

      YieldPid         = pDataAddr->ChildSid;
      pYield           = GetPid( YieldPid );
      if(pYield)
       pYield->Connect = DISCONNECTED;

      pSqe = IsConnectRequestQueued( pid );
      if( pSqe )
       RemoveMessage( pSqe );
      else
      {
       pCtrlBreak->PidFlags.CtrlBreak = TRUE;
       DosKillProcess( DKP_PROCESS, pCtrlBreak->EspPid );
      }
      TorF = TRUE;
      if( CtrlBreakPid == GetEspProcessID() )
       TorF = FALSE;

      PostConnectSema4( &pCtrlBreak->ConnectSema4, TorF );
     }
     else
     {
      /***********************************************************************/
      /* - If connected with a parallel connection, then send the kill       */
      /*   process signal.                                                   */
      /***********************************************************************/
      pCtrlBreak->PidFlags.CtrlBreak = TRUE;
      DosKillProcess( DKP_PROCESS, (PID)(pCtrlBreak->EspPid) );
     }
    }
    break;

/*---------------------------------------------------------------------------*/
/* All of the messages until the next dashed comment line are for serial     */
/* connections only.                                                         */
/*---------------------------------------------------------------------------*/

   case ESP_QMSG_CONNECT_REQUEST:
    /*************************************************************************/
    /* - Serial connect message only.                                        */
    /* - One of the esp services has a notification to report.               */
    /* - If we're debugging a single process, then simply post the sema4.    */
    /* - If we're debugging multiple processes, then add the message to the  */
    /*   message queue in the shared heap. It will be polled in by the       */
    /*   dbg.                                                                */
    /*************************************************************************/
    p = GetPid( pDataAddr->ChildPid );
    pid = p->pid;
    if( SingleMultiple() == MULTIPLE )
     AddMessage( CONNECT_DBG, &pid, sizeof(pid) );
    else
    {
     p->Connect = CONNECTED;

     SetPeekForCtrlBreakFlag( FALSE );
     PostConnectSema4( &p->ConnectSema4, FALSE );
    }
    break;

   case ESP_QMSG_DISCONNECT:
    /*************************************************************************/
    /* - Serial connect message only.                                        */
    /* - One of the esp execution services is being executed and is          */
    /*   giving up its connection.                                           */
    /* - If debugging multiple processes, then start a thread to respond     */
    /*   to the polling requests from the dbg.                               */
    /*************************************************************************/
    p          = GetPid( pDataAddr->ChildPid );
    p->Connect = DISCONNECTED;
    if( SingleMultiple() == MULTIPLE )
     _beginthread( WaitForConnection, NULL, 0x8000, NULL );
    else
     _beginthread( PeekForCtrlBreak, NULL, 0x8000, NULL );
    break;

/*---------------------------------------------------------------------------*/

   case ESP_QMSG_SELECT_ESP:
    /*************************************************************************/
    /* - Bring the probe for this pid to the foreground. Used to display     */
    /*   error messages.                                                     */
    /*************************************************************************/
    pid = pDataAddr->ChildPid;
    p   = GetEspPid( pid );
    DosSelectSession( p->EspSid );
    break;

   case ESP_QMSG_PARENT_TERM:

    /*************************************************************************/
    /* - Set a flag so that child termination messages will not start        */
    /*   connection threads when running serial.                             */
    /*************************************************************************/
    ParentTerminating = TRUE;
    /*************************************************************************/
    /* - The parent probe is terminating.                                    */
    /* - Note: At this point the AllProbesAreDeadFlag==FALSE.                */
    /*************************************************************************/
    for( p = GetAllpids(); p ; p = p->next )
    {
     if( (p->PidFlags.IsDebug) == TRUE && (p->pid != GetEspProcessID()) )
     {
      ESP_QUE_ELEMENT Qelement;

      Qelement.ChildPid = p->EspPid;

      SendMsgToEspTermQue(ESP_TERMINATE, &Qelement, sizeof(Qelement) );
     }
    }

    /*************************************************************************/
    /* - now, remove the pid structures from the list for the processes      */
    /*   that are NOT being debugged.                                        */
    /*************************************************************************/
    for( p = GetAllpids(); p ; p = p->next )
    {
     if( (p->PidFlags.IsDebug) == FALSE && (p->pid != GetEspProcessID()) )
     {
      RemovePid( p->pid );
     }
    }

    /*************************************************************************/
    /* - There may not be any child probes, so set this flag so the parent   */
    /*   can terminate.                                                      */
    /*************************************************************************/
    if(GetAllpids()->next == NULL)
    {
     AllProbesAreDeadFlag = TRUE;
    }
    break;

   case ESP_QMSG_CHILD_TERM:
    /*************************************************************************/
    /* - A child probe has terminated.                                       */
    /*************************************************************************/
    RemovePid( pDataAddr->ChildPid );

    /*************************************************************************/
    /* - After the last probe has terminated, then report all probes dead.   */
    /*************************************************************************/
    if(GetAllpids()->next == NULL)
    {
     AllProbesAreDeadFlag = TRUE;
    }
    if( (ParentTerminating == FALSE)    &&
        (SerialParallel()  == SERIAL)   &&
        (SingleMultiple()  == MULTIPLE)
      )
     _beginthread( WaitForConnection, NULL, 0x8000, NULL );
    break;

   case ESP_QMSG_QUE_TERM:
    /*************************************************************************/
    /* - Terminate and reset the queue for a possible restart.               */
    /* - Free memory allocated for the pid structures.                       */
    /*************************************************************************/
    DosCloseQueue(QueHandle);

    if( (SerialParallel() == PARALLEL) && LsnHandle )
     ConnectClose( LsnHandle );

    if( SingleMultiple() == MULTIPLE )
     FreeSharedHeap();

    ParentStarted = FALSE;
    EndThread     = TRUE;
    ReadQueReady  = 0;
    break;

   case ESP_QMSG_ERROR:
    {
     QUE_ERR_ELEMENT *pQueErrElement =  (QUE_ERR_ELEMENT *)pDataAddr;
     char            *pErrorMsg;
     char             RcMessage[32];

     rc        = pQueErrElement->rc;
     pErrorMsg = pQueErrElement->ErrorMsg;

     DosSelectSession(0);

     sprintf( RcMessage, "Esp Error rc=%d", rc );

     ErrorPrintf( ERR_ESP_QUE_ERROR, 2, RcMessage, pErrorMsg );
    }
    break;

   default:
    break;

  }

  /***************************************************************************/
  /* - Free the shared memory used for the message and initialize the        */
  /*   data element pointer for the next message.                            */
  /***************************************************************************/
  if(pDataAddr) DosFreeMem(pDataAddr);
  pDataAddr = NULL;
 }
 if( IsVerbose() ) {printf("\nEsp Que Ended");fflush(0);}
 _endthread();
}                                       /* end ReadQ().                      */

/*****************************************************************************/
/* SendMsgToEspQue()                                                         */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*  Send ESP_QMSG_xxxxxx message to ESP queue.                               */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   Msg            ESP_QMSG_xxxxxx message.                                 */
/*   PQueElement    -> to optional queue element.                            */
/*   QueElementLen  length of the queue element.                             */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
void SendMsgToEspQue( ULONG Msg , void *pQueElement, ULONG QueElementLen)
{
 SendMsgToQue( EspQueName, Msg, pQueElement, QueElementLen );
}

/*****************************************************************************/
/* ListenForMsgFromDbg()                                                     */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   This function runs in its own thread and listens messages coming        */
/*   to the queue from esp. This thread only exists when we're running       */
/*   with a parallel connection.                                             */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   handle   connection handle.                                             */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
void ListenForMsgFromDbg( void *handle )
{
 LHANDLE LsnHandle = (LHANDLE)handle;
 COMMAND cmd;
 BOOL    EndThread;

 if( IsVerbose() ) {printf("\nListen thread started");fflush(0);}
 for( EndThread = FALSE ; EndThread == FALSE ; )
 {
  RmtRecv( LsnHandle,&cmd,sizeof(cmd) );
  if( IsVerbose() ) PrintCmdMessage( cmd.api ) ;
  switch( cmd.api )
  {
   case CTRL_BREAK:
   {
    ESP_QUE_ELEMENT Qelement;
    USHORT          CtrlBreakPid;

    RmtRecv( LsnHandle, &CtrlBreakPid, cmd.len );

    Qelement.ChildPid = CtrlBreakPid;
    Qelement.ChildSid = 0;

    SendMsgToEspQue(ESP_QMSG_CTRL_BREAK, &Qelement, sizeof(Qelement));
   }
   break;

   case KILL_LISTEN_THREAD:
   /**************************************************************************/
   /* - This command will come from the dbg queue when it gets a command     */
   /*   to kill the listen thread during termination. The reason we do       */
   /*   it this way is that DosKillThread() was causing a session hang       */
   /*   when using MPTS on LS 4.0, so now, the debugger sends a command      */
   /*   to the probe telling it to kill its listen thread. The probe sends   */
   /*   a command back to the debugger telling it to also kill its listen    */
   /*   thread.                                                              */
   /**************************************************************************/
   {
    cmd.api  = KILL_LISTEN_THREAD;
    cmd.cmd  = 0;
    cmd.len  = 0;

    RmtSend( LsnHandle, &cmd , sizeof(cmd) );
    EndThread = TRUE;
   }
   break;
  }
 }
 if( IsVerbose() ) {printf("\nListen thread ended");fflush(0);}
 _endthread();
}

/*****************************************************************************/
/* WaitForConnection()                                                       */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   This thread spins when there is no probe to debugger connection         */
/*   which happens after the queue processes an ESP_QMSG_DISCONNECT.  The    */
/*   dbg will be polling for the next probe to connect and this thread       */
/*   will make the connection and will die after the connect is made.        */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
void WaitForConnection( void *dummy )
{
 LHANDLE LsnHandle;
 COMMAND cmd;
 BOOL    EndThread=FALSE;


 if( IsVerbose() ) {printf("\nWait for connection started");fflush(0);}
 LsnHandle = GetComHandle();
 for(; EndThread==FALSE ; )
 {
  RmtRecv( LsnHandle,&cmd,sizeof(cmd) );
  if( IsVerbose() ) PrintCmdMessage( cmd.api ) ;
  switch( cmd.api )
  {
   case SERIAL_POLL:
    ReportMessage();
    break;

   case CONNECT_ESP:
   {
    USHORT   GoToPid;
    ALLPIDS *pGoTo;
    BOOL     TorF;

    /*************************************************************************/
    /* - Receive the pid to be connected and mark it connected.              */
    /* - If this pid has not yet been assigned to a probe, then pGoTo        */
    /*   will be NULL.                                                       */
    /* - There MUST be a probe with a pid==0, so we release that probe.      */
    /*   ( The pid will be stuffed into the structure at goinit() time.)     */
    /* - Post the connect sema4 for the goto pid.                            */
    /*************************************************************************/
    RmtRecv( LsnHandle, &GoToPid, cmd.len );
    pGoTo          = GetPid( GoToPid );

    if( pGoTo == NULL )
     pGoTo = GetPid(0);

    pGoTo->Connect = CONNECTED;

    TorF = TRUE;
    if( GoToPid == GetEspProcessID() )
     TorF = FALSE;

    /*************************************************************************/
    /* - Send back verification that the connection has been made.           */
    /*************************************************************************/
    memset(&cmd,0,sizeof(cmd) );

    cmd.api = CONNECT_ESP;
    RmtSend( LsnHandle, &cmd, sizeof(cmd) );

    /*************************************************************************/
    /* - A little insurance to make sure the sema4 has been created.         */
    /*************************************************************************/
    while( pGoTo->ConnectSema4 == 0 ) { DosSleep(200); }
    PostConnectSema4( &pGoTo->ConnectSema4, TorF );
    EndThread = TRUE;
   }
   break;

   case CTRL_BREAK:
   {
    ESP_QUE_ELEMENT Qelement;
    USHORT          CtrlBreakPid;

    RmtRecv( LsnHandle, &CtrlBreakPid, cmd.len );

    Qelement.ChildPid = CtrlBreakPid;
    Qelement.ChildSid = 0;

    SendMsgToEspQue(ESP_QMSG_CTRL_BREAK, &Qelement, sizeof(Qelement));
    EndThread=TRUE;

   }
   break;

  }
  if( IsPidConnected() )
  {
    EndThread=TRUE;
  }
 }
 if( IsVerbose() ) {printf("\nWait for connection ended");fflush(0);}
 _endthread();
}


/*****************************************************************************/
/* SendNewProcessToDbg()                                                     */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*  Send a new process notification back to the dbg.                         */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   handle      handle of the connection.                                   */
/*   pid         process id of the new process.                              */
/*   type        type of the new process.                                    */
/*   mte         module handle of the new process.                           */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
void SendNewProcessToDbg( LHANDLE handle,
                          USHORT  pid,
                          ULONG   type,
                          USHORT  mte )
{
 COMMAND cmd;
 int     size;

 TXRX_NEW_PROCESS packet;

 cmd.api = NEW_PROCESS;
 cmd.cmd = NEW_PROCESS;
 cmd.len = sizeof(packet) - sizeof(packet.ProcessFileSpec);

 size = sizeof(packet.ProcessFileSpec);
 memset(packet.ProcessFileSpec, 0, size );
 DosQueryModuleName( mte, size, packet.ProcessFileSpec );

 packet.pid  = pid;
 packet.type = type;

 cmd.len += strlen(packet.ProcessFileSpec) + 1;

 RmtSend( handle, &cmd, sizeof(cmd) );
 RmtSend( handle, &packet, cmd.len );

}

/*****************************************************************************/
/* ConnectDbg()                                                              */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*  Send a connect command to the dbg que.                                   */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   handle      handle of the connection.                                   */
/*   pid         process id of the new process.                              */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   This function is only called for serial connections.                    */
/*                                                                           */
/*****************************************************************************/
void ConnectDbg( LHANDLE handle, USHORT pid )
{
 COMMAND cmd;

 /****************************************************************************/
 /* - The probe does not send messages to the dbg when debugging a single    */
 /*   parent or child process.                                               */
 /****************************************************************************/
 if( SingleMultiple() == SINGLE )
  return;

 cmd.api = CONNECT_DBG;
 cmd.cmd = CONNECT_DBG;
 cmd.len = sizeof(pid);

 RmtSend( handle, &cmd, sizeof(cmd) );
 RmtSend( handle, &pid, cmd.len );
 RmtRecv( handle, &cmd, sizeof(cmd) );
}

/*****************************************************************************/
/* PeekForCtrlBreak()                                                        */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*  When debugging a single process whether parent or child and we're        */
/*  using a serial connection, then we need to peek the connection to see    */
/*  if there has been a ctrl-break sent from the dbg.                        */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   The only function that can come across the serial connection is         */
/*   ctrl-break so when there is something to peek it is "assumed"           */
/*   that it's a ctrl-break.                                                 */
/*                                                                           */
/*****************************************************************************/
static BOOL PeekForCtrlBreakFlag = TRUE;
void   SetPeekForCtrlBreakFlag( BOOL TorF ) { PeekForCtrlBreakFlag = TorF;}
BOOL   GetPeekForCtrlBreakFlag( void ) { return( PeekForCtrlBreakFlag ); }

void PeekForCtrlBreak( void* dummy )
{

 if( IsVerbose() ) {printf("\nPeek ctrl-break started");fflush(0);}
 for(; PeekForCtrlBreakFlag==TRUE;)
 {
  if( AsyncPeekComPort() )
  {
   COMMAND cmd = {0,0};

   memset(&cmd,0,sizeof(cmd) );
   RmtRecv(DEFAULT_HANDLE, (char*)&cmd, sizeof(cmd));
   if( IsVerbose() ) PrintCmdMessage( cmd.api ) ;
   switch( cmd.api )
   {
    case CTRL_BREAK:
    {
     ESP_QUE_ELEMENT Qelement;
     USHORT          CtrlBreakPid;

     RmtRecv( DEFAULT_HANDLE, &CtrlBreakPid, cmd.len );

     Qelement.ChildPid = CtrlBreakPid;
     Qelement.ChildSid = 0;

     SendMsgToEspQue(ESP_QMSG_CTRL_BREAK, &Qelement, sizeof(Qelement));
     PeekForCtrlBreakFlag=FALSE;
    }
    break;

    default:
    break;
   }
  }
  DosSleep(300);
 }
 PeekForCtrlBreakFlag=TRUE;
 if( IsVerbose() ) {printf("\nPeek ctrl-break ended");fflush(0);}
 _endthread();
}

/*****************************************************************************/
/* StartEspTermQue()                                                         */
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

APIRET StartEspTermQue( void )
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
 rc = StartQue(TermQueName, ReadEspTermQue );
 if(rc) return(rc);

 /****************************************************************************/
 /* - Now, add the name of the queue to the shared memory structure for      */
 /*   this debugger.                                                         */
 /****************************************************************************/
 p = GetEspPid( ThisPid );

 DosSubAllocMem( GetShrMem(), (PPVOID)&(p->pTermQue), strlen(TermQueName) + 1 );
 strcpy(p->pTermQue, TermQueName );

 return(0);
}

/*****************************************************************************/
/* ReadEspTermQue()                                                          */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Read and handle termination message(s) coming to this probe             */
/*   from the parent probe.                                                  */
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
void ReadEspTermQue( void *qHandle)
{
 REQUESTDATA  Request;
 ULONG        DataLen;
 BYTE         ElePri;
 BOOL         EndThread = FALSE;
 LHANDLE      TermQueHandle;

 ESP_QUE_ELEMENT *pDataAddr=NULL;

 TermQueHandle    = (LHANDLE)qHandle;

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
   case ESP_TERMINATE:
   {
    ALLPIDS *p;
    ULONG    EntryPt = 0;
    USHORT   pid;
    USHORT   mte;

    ESP_QUE_ELEMENT Qelement;

    /*************************************************************************/
    /* - At this point, thread 1 could be blocked in DosDebug(). When        */
    /*   we make the normalquit() call, then thread 1 may return and         */
    /*   try return back to the debugger resulting in a communications       */
    /*   send error. So, to prevent this, we set a flag to block any         */
    /*   further communication attempts.                                     */
    /*************************************************************************/
    pid     = GetEspProcessID();
    p       = GetPid( pid );
    mte     = p->mte;
    EntryPt = XSrvGetExeOrDllEntryOrExitPt( mte );

    XSrvNormalQuit( IsChildTerminated(), mte, EntryPt );

    CloseConnectSema4();
    /*************************************************************************/
    /* - At this                                                             */
    /*************************************************************************/
    ConnectClose( DEFAULT_HANDLE );

    memset( &Qelement, 0, sizeof(Qelement) );
    Qelement.ChildPid = pid;
    SendMsgToEspQue( ESP_QMSG_CHILD_TERM, &Qelement, sizeof(Qelement) );

    EndThread = TRUE;
   }
   break;

   case ESP_PROBE_TERM:
    /*************************************************************************/
    /* - At this point, thread 1 could be blocked in DosDebug().  When we    */
    /*   terminate the parent, then thread 1 may return and try return back  */
    /*   to the debugger resulting in a communications send error.  So, to   */
    /*   prevent this, we set a flag to block any further communication      */
    /*   attempts.                                                           */
    /*************************************************************************/
    SetHaltCommunicationsFlag();
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
 /* - Come here to terminate the probe.                                      */
 /****************************************************************************/
 if( IsVerbose() ) {printf("\nProbe terminated");fflush(0);}
 exit(0);
}                                       /* end ReadQ().                      */

/*****************************************************************************/
/* SendMsgToEspTermQue()                                                     */
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
void SendMsgToEspTermQue(ULONG Msg , void *pQueElement, ULONG QueElementLen)
{
 USHORT   EspPid;
 ALLPIDS *p;
 char    *pQueName;

 /****************************************************************************/
 /* - At this time, we are not actually sending anything in a queue element. */
 /*   However, we're allowing for future expansion.                          */
 /****************************************************************************/

 /****************************************************************************/
 /* - Get a ptr to the shared memory structure of the queue owner.           */
 /****************************************************************************/
 EspPid = ((ESP_QUE_ELEMENT*)pQueElement)->ChildPid;
 p      = GetEspPid( EspPid );
 if( p )
 {
  pQueName = p->pTermQue;
  SendMsgToQue( pQueName, Msg, pQueElement, QueElementLen );
 }
}

/*****************************************************************************/
/* AddProcessToList()                                                        */
/* IsProcessInList()                                                         */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Maintain a list of new process notifications.                           */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   NewPid      the new process id to be added to the list.                 */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
static NEWPID *pNewPidList;
void AddProcessToList( USHORT NewPid )
{
 NEWPID *p;
 NEWPID *q;

 if( IsProcessInList(NewPid) == TRUE )
  return;

 q = Talloc(sizeof(NEWPID));

 memset(q, 0, sizeof(NEWPID) );

 q->pid = NewPid;

 for( p = (NEWPID*)&pNewPidList; p->next != NULL ; p = p->next ){;}

 p->next = q;

 return;
}

BOOL IsProcessInList( USHORT pid )
{
 NEWPID *p;

 for( p = pNewPidList ; p ; p = p->next )
  if( p->pid == pid )
   return(TRUE);

 return(FALSE);
}
