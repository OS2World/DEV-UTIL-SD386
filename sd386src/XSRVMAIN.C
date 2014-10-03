/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   xsrvmain.c                                                              */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*  Esp main function.                                                       */
/*                                                                           */
/* History:                                                                  */
/*                                                                           */
/*   12/08/92 Created.                                                       */
/*                                                                           */
/*****************************************************************************/
#include "all.h"

static ESP_PARMS EspParms;

ESP_PARMS *GetEspParms   ( void ) { return(&EspParms); }

BOOL IsParent      ( void ) { return(EspParms.EspFlags.IsParentEsp)   ;}
BOOL SingleMultiple( void ) { return(EspParms.EspFlags.SingleMultiple);}
BOOL IsVerbose     ( void ) { return(EspParms.EspFlags.Verbose)       ;}
BOOL DosDebugTrace ( void ) { return(EspParms.EspFlags.DosDebugTrace );}
BOOL UseExecPgm    ( void ) { return(EspParms.EspFlags.UseExecPgm )   ;}
BOOL DebugChild    ( void ) { return(EspParms.EspFlags.DebugChild )   ;}
BOOL UseDebug      ( void ) { return(EspParms.EspFlags.UseDebug)      ;}

/*****************************************************************************/
/* main()                                                                    */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Execution services probe main routine.                                  */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   argc       Number of input parameters.                                  */
/*   argv       -> -> to argument strings.                                   */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
int main( int argc, char **argv )
{
 APIRET      rc;
 COMMAND     cmd = {0,0};
 int         n;
 char       *cp;
 char        rc_string[12];
 CONNECTION  Connection;

 EXCEPTIONREGISTRATIONRECORD  reg_rec;

 ESP_QUE_ELEMENT Qelement;


 if( argc == 1 )
  SayMsg(HELP_INVOCATION_ESP);

 /****************************************************************************/
 /*                                                                          */
 /* Parse the invocation options.                                            */
 /*                                                                          */
 /* - If this is a child debugger, then these additional parameters will     */
 /*   precede the invocation parameters inherited from the parent.           */
 /*                                                                          */
 /*   - /child=xxxxx  where child  = child debugger and                      */
 /*                         xxxxx  = child pid (for serial connections only.)*/
 /*                                                                          */
 /*   - /handle=xxxxx where handle = switch for com handle( serial only )    */
 /*                         xxxxx  = parent's com handle - inherited by      */
 /*                                  the child.                              */
 /*                                                                          */
 /****************************************************************************/
 memset( &EspParms,   0, sizeof( ESP_PARMS ) );
 memset( &Connection, 0, sizeof(Connection) );
 if( strstr( argv[1], "/child" ) )
 {
  ParseEspChildOptions( argc, argv, &EspParms, &Connection );
 }
 else
 {
  ParseOptions( argc, argv, &EspParms, &Connection );
 }

 printf("\nESP Version 5.00 \n");fflush(0);

 /****************************************************************************/
 /* - Send connection info to the router.                                    */
 /****************************************************************************/
 SendConnectionToRouter( &Connection );

 /****************************************************************************/
 /* - Make the connection.                                                   */
 /****************************************************************************/
 {
  int RcMoreInfo = 0;

  rc = ConnectInit( &RcMoreInfo );

  if( rc != 0 )
  {
   char  BadConnectMsg[32] = "";
   int   MsgId;

   if( (Connection.ConnectType == _NETBIOS) && (RcMoreInfo != 0) )
   {
    /*************************************************************************/
    /* - handle netbios specific connect errors.                             */
    /*************************************************************************/
    n     = 1;
    MsgId = ERR_NB_INADEQUATE_RESOURCES;

    switch( RcMoreInfo )
    {
     case CANT_LOAD_NETB_DLL:
      n     = 0;
      MsgId = ERR_NB_CANT_LOAD_DLL;
      break;

     case INADEQUATE_SESSIONS:
      strcpy( BadConnectMsg,"sessions");
      break;

     case INADEQUATE_COMMANDS:
      strcpy( BadConnectMsg,"commands");
      break;

     case INADEQUATE_NAMES:
      strcpy( BadConnectMsg,"names");
      break;

     default:
      n = 1;
      MsgId = ERR_BAD_CONNECT;
      sprintf( BadConnectMsg, "NetBios error rc=%d", rc );
      break;
    }
   }
   else if( (Connection.ConnectType == SOCKET) && (RcMoreInfo != 0) )
   {
    /*************************************************************************/
    /* - handle tcpip specific connect errors.                               */
    /*************************************************************************/
    switch( RcMoreInfo )
    {
     case CANT_LOAD_TCPIP_DLL:
      n     = 1;
      MsgId = ERR_TCPIP_CANT_LOAD_DLL;

      sprintf( BadConnectMsg, "%d", rc );
      break;

     case TCPIP_NOT_RUNNING:
      n     = 0;
      MsgId = ERR_TCPIP_NOT_RUNNING;
      break;

     case TCPIP_ERROR:
      n     = 0;
      MsgId = ERR_TCPIP_ERROR;
      break;

     case TCPIP_NO_SERVICES_PORT:
      n     = 0;
      MsgId = ERR_TCPIP_NO_SERVICES_PORT;
      break;

     default:
      n = 1;
      MsgId = ERR_BAD_CONNECT;
      sprintf( BadConnectMsg, "tcpip error rc=%d", rc );
      break;
    }
   }
   else
   {
    /*************************************************************************/
    /* - handle generic connect errors.                                      */
    /*************************************************************************/
    n = 1;
    MsgId = ERR_BAD_CONNECT;
    sprintf( BadConnectMsg, "rc=%d", rc );
   }
   ErrorPrintf( MsgId, n, BadConnectMsg );
  }
 }

 /****************************************************************************/
 /* - register an exception handler for the probe.                           */
 /****************************************************************************/
 reg_rec.ExceptionHandler = Handler;
 DosSetExceptionHandler(&reg_rec);

 /****************************************************************************/
 /* - Add a connect sema4 for serial connections and wait to be posted.      */
 /****************************************************************************/
 if( (SerialParallel() == SERIAL) && ( IsParent() == FALSE ) )
 {
  USHORT   EspPid;
  TIB     *pTib;
  PIB     *pPib;
  ALLPIDS *p;

  SetComHandle( EspParms.handle );

  DosGetInfoBlocks(&pTib,&pPib);
  EspPid = (USHORT)pPib->pib_ulpid;
  CreateConnectSema4( EspPid, _ESP );
  SerialConnect( JUST_WAIT, 0, _ESP, SendMsgToEspQue );
  p = GetEspPid( EspPid );
  p->Connect = CONNECTED;
 }

 /****************************************************************************/
 /* - Each child debugger will have a termination que so we can kill the     */
 /*   child debuggers on quit/restart.                                       */
 /****************************************************************************/
 if( IsParent() == FALSE )
 {
  rc = StartEspTermQue( );
  if( rc != 0 )
  {
   sprintf(rc_string, "%d",rc);
   ErrorPrintf( ERR_CANT_START_QUE, TRUE, 1, rc_string );
  }
 }
 /****************************************************************************/
 /* - Now, start the command processing loop.                                */
 /****************************************************************************/
 for(;;)
 {
  memset(&cmd,0,sizeof(cmd) );
  RmtRecv(DEFAULT_HANDLE, (char*)&cmd, sizeof(cmd));
  if( IsVerbose() ) PrintCmdMessage( cmd.api ) ;
  switch( cmd.api )
  {
   case FINDEXE:
    RxFindExe(cmd);
    break;

   case STARTUSER:
    RxStartUser( cmd );
    break;

   case GOINIT:
    RxGoInit(cmd);
    break;

   case GOENTRY:
    RxGoEntry(cmd);
    break;

   case DEFBRK:
    RxDefBrk(cmd);
    break;

   case UNDBRK:
    RxUndBrk(cmd);
    break;

   case PUTINBRK:
    RxPutInBrk(cmd);
    break;

   case PULLOUTBRK:
    RxPullOutBrk(cmd);
    break;

   case INSERTALLBRK:
    RxInsertAllBrk();
    break;

   case REMOVEALLBRK:
    RxRemoveAllBrk();
    break;

   case SELECT_SESSION:
    /*************************************************************************/
    /* - Only the parent probe can select one of the debuggee sessions, so   */
    /*   we send a message and tell him to do it.                            */
    /*************************************************************************/
    Qelement.ChildPid = GetEspProcessID();
    SendMsgToEspQue( ESP_QMSG_SELECT_SESSION, &Qelement, sizeof(Qelement) );

    memset(&cmd,0,sizeof(cmd) );

    cmd.api = SELECT_SESSION;
    RmtSend( DEFAULT_HANDLE, &cmd, sizeof(cmd) );
    break;

   case GOSTEP:
    RxGoStep(cmd);
    break;

   case GOFAST:
    RxGoFast(cmd);
    break;

   case DOSDEBUG:
    RxDosDebug( cmd );
    break;

   case GETTHREADINFO:
    RxGetThreadInfo( cmd );
    break;

   case FREEZETHREAD:
    RxFreezeThread( cmd );
    break;

   case THAWTHREAD:
    RxThawThread( cmd );
    break;

   case GETCALLSTACK:
    RxGetCallStack(cmd);
    break;

   case GETEXEORDLLENTRY:
    RxGetExeOrDllEntryOrExitPt(cmd);
    break;

   case NORMALQUIT:
    RxNormalQuit(cmd);

    /*************************************************************************/
    /* - The que has to be up until after the normal quit because the        */
    /*   system will need to post an end session message to the queue.       */
    /*************************************************************************/
    if( IsParent() )
    {
     if( SingleMultiple() == MULTIPLE )
     {
      ALLPIDS *p;
      /***********************************************************************/
      /* - Send a message to all of the child probes telling them that       */
      /*   they are going to be killed.                                      */
      /***********************************************************************/
      for( p = GetAllpids(); p ; p = p->next )
      {
       if( (p->PidFlags.IsDebug) == TRUE && (p->pid != GetEspProcessID()) )
       {
        ESP_QUE_ELEMENT Qelement;

        Qelement.ChildPid = p->EspPid;

        SendMsgToEspTermQue(ESP_PROBE_TERM, &Qelement, sizeof(Qelement) );
       }
      }
      /***********************************************************************/
      /* - send a message to the que to kill all the child probes and        */
      /*   then wait until they are all dead.                                */
      /***********************************************************************/
      ResetAllProbesAreDeadFlag( );
      SendMsgToEspQue(ESP_QMSG_PARENT_TERM,NULL,0);
      while( AllProbesAreDead() == FALSE ){ DosSleep(100) ;}
     }
     SendMsgToEspQue(ESP_QMSG_QUE_TERM,NULL,0);
    }

    CloseConnectSema4();

    cmd.api  = NORMALQUIT;
    cmd.len  = sizeof(rc);

    /*************************************************************************/
    /* - Now, tell dbg that we're finished normal quitting.                  */
    /*************************************************************************/
    RmtSend(DEFAULT_HANDLE, &cmd , sizeof(cmd) );
    RmtSend(DEFAULT_HANDLE, &rc, cmd.len );
    break;

   case SETEXECADDR:
    RxSetExecAddr(cmd);
    break;

   case DEFWPS:
    RxDefWps(cmd);
    break;

   case PUTINWPS:
    RxPutInWps(cmd);
    break;

   case PULLOUTWPS:
    RxPullOutWps(cmd);
    break;

   case GETDATABYTES:
    RxGetDataBytes(cmd);
    break;

   case GETMEMBLKS:
    RxGetMemBlocks(cmd);
    break;

   case SETXCPTNOTIFY:
    RxSetExceptions(cmd);
    break;

   case SETEXECTHREAD:
    RxSetExecThread(cmd);
    break;

   case WRITEREGS:
    RxWriteRegs(cmd);
    break;

   case GETCOREGS:
    RxGetCoRegs(cmd);
    break;

   case SETESPRUNOPTS:
    RxSetEspRunOpts(cmd);
    break;

   case TERMINATEESP:
    if( IsParent() == FALSE )
    {
     memset( &Qelement, 0, sizeof(Qelement) );
     Qelement.ChildPid = GetEspProcessID();

     SendMsgToEspQue( ESP_QMSG_CHILD_TERM, &Qelement, sizeof(Qelement) );
    }
    DosUnsetExceptionHandler(&reg_rec);
    RmtSend(DEFAULT_HANDLE, &cmd , sizeof(cmd) );
    ConnectClose( DEFAULT_HANDLE );
    exit(0);
    break;

   case START_QUE_LISTEN:
    SendMsgToEspQue(ESP_QMSG_OPEN_CONNECT,NULL,0);
    break;

   case START_ESP_QUE:
    RxStartEspQue(cmd);
    break;

   case CONNECT_ESP:
    /*************************************************************************/
    /* - Serial connection only.                                             */
    /*************************************************************************/
    {
     USHORT   GoToPid;
     USHORT   YieldPid;
     ALLPIDS *pYield;
     ALLPIDS *pGoTo;
     BOOL     TorF;

     /************************************************************************/
     /* - Receive the pid to be connected and mark it connected.             */
     /* - If this pid has not yet been assigned to a probe, then pGoTo       */
     /*   will be NULL.                                                      */
     /* - There MUST be a probe with a pid==0, so we release that probe.     */
     /*   ( The pid will be stuffed into the structure at goinit() time.)    */
     /* - Post the connect sema4 for the goto pid.                           */
     /************************************************************************/
     RmtRecv( DEFAULT_HANDLE, &GoToPid, cmd.len );
     pGoTo          = GetPid( GoToPid );

     if( pGoTo == NULL )
      pGoTo = GetPid(0);

     pGoTo->Connect = CONNECTED;


     TorF = TRUE;
     if( GoToPid == GetEspProcessID() )
      TorF = FALSE;

     /************************************************************************/
     /* - Send back verification that the connection has been made.          */
     /************************************************************************/
     memset(&cmd,0,sizeof(cmd) );

     cmd.api = SERIAL_POLL;
     RmtSend( DEFAULT_HANDLE, &cmd, sizeof(cmd) );

     PostConnectSema4( &pGoTo->ConnectSema4, TorF );

     /************************************************************************/
     /* - Disconnect/block this probe.                                       */
     /************************************************************************/
     YieldPid        = (USHORT)GetEspProcessID();
     pYield          = GetPid( YieldPid );
     pYield->Connect = DISCONNECTED;

     SerialConnect( SET_WAIT, YieldPid, _ESP, SendMsgToEspQue );
    }
    break;

   case CTRL_BREAK:
    {
     USHORT          ThisPid;
     USHORT          CtrlBreakPid;

     RmtRecv( DEFAULT_HANDLE, &CtrlBreakPid, cmd.len );

     ThisPid = (USHORT)GetEspProcessID();

     Qelement.ChildPid = CtrlBreakPid;
     Qelement.ChildSid = ThisPid;

     SendMsgToEspQue(ESP_QMSG_CTRL_BREAK, &Qelement, sizeof(Qelement));

     SerialConnect( SET_WAIT, ThisPid,  _ESP, SendMsgToEspQue );
    }
    break;

   case SERIAL_POLL:
    ReportMessage();
    break;

   default:
    cp = (char*)&cmd;
    for( n=1; n<=sizeof(cmd); n++,cp++)
     printf("%c",*cp);
    AsyncFlushModem();
    break;
  }
 }
}

/*****************************************************************************/
/* ParseOptions()                                                            */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   - Parse the invocation options.                                         */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   argc        Number of input parameters.                                 */
/*   argv        -> -> to argument strings.                                  */
/*   pEspParms   -> to the structure that will define the invocation parms.  */
/*   pConnection -> to the structure that will define the connection parms.  */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   void                                                                    */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
void ParseOptions( int          argc,
                   char       **argv,
                   ESP_PARMS    *pEspParms,
                   CONNECTION  *pConnection)
{
 char          ParmString[512];
 char          FileSpec[CCHMAXPATH];
 int           i;
 char         *pOption;
 char         *cp;
 char         *cpo;
 int           len;

 /****************************************************************************/
 /* - If user entered no arguments, go to case help.                         */
 /****************************************************************************/
 if( argc == 1 )
   goto casehelp;

 /****************************************************************************/
 /* - set any default options.                                               */
 /****************************************************************************/
 pConnection->DbgOrEsp = _ESP;

 pEspParms->EspFlags.IsParentEsp    = TRUE;

 /****************************************************************************/
 /* - Build a string of tokens delimited by a single blank.                  */
 /****************************************************************************/
 ParmString[0] = '\0';
 for( i=1 ; i<argc; i++)
 {
  strcat(ParmString,argv[i]);
  strcat(ParmString," ");
 }
 strcat(ParmString,"\0");

 /****************************************************************************/
 /* - Scan the parm block and pull off the options and program name.         */
 /****************************************************************************/
 pOption = strtok(ParmString," ");
 do
 {
  switch( *pOption )
  {
   case '/':
    switch( tolower(*(pOption + 1)) )
    {
 casehelp:
     /************************************************************************/
     /* - handle the help options.                                           */
     /************************************************************************/
     case 'h':
     case '?':
      SayMsg(HELP_INVOCATION_ESP);
      break;

     case 'a':
      /***********************************************************************/
      /* - Parse the com port.                                               */
      /***********************************************************************/
      pConnection->ComPort = EspParseComPort( *(pOption+2) );
      break;

     case 'r':
     pConnection->ConnectType = ASYNC;
     {
     /************************************************************************/
     /* - parse off the speed option.                                        */
     /************************************************************************/
      switch( *(pOption + 2) )
      {
       case '0':
        pConnection->BitRate = 300;
        break;

       case '1':
        pConnection->BitRate = 1200;
        break;

       case '2':
        pConnection->BitRate = 2400;
        break;

       case '3':
        pConnection->BitRate = 4800;
        break;

       case '4':
        pConnection->BitRate = 9600;
        break;

       case '5':
        pConnection->BitRate = 19200;
        break;

       case '6':
        pConnection->BitRate = 38400;
        break;

       default:
        /*********************************************************************/
        /* - assume online and already connected.                            */
        /*********************************************************************/
        pConnection->BitRate = 0;
        break;
      }
     }
     break;

caseo:
     case 'o':
      pConnection->modem = TRUE;
      /***********************************************************************/
      /* - Pull off the program name.                                        */
      /***********************************************************************/
      cpo = pOption + 2;
      if( strlen(cpo) != 0 )
      {
       for( cp=FileSpec; (*cpo != ' ') && (*cpo != 0); )
        *cp++ = toupper(*cpo++);
       *cp = '\0';
       pConnection->pModemFile = Talloc(strlen(FileSpec)+1);
       strcpy(pConnection->pModemFile,FileSpec);
      }
      break;

     case '+': /* Joe's private option */
      pEspParms->EspFlags.Verbose = TRUE;
      break;

     case '!':
      pEspParms->EspFlags.UseDebug  = TRUE;
      break;

     case 'n':
      /***********************************************************************/
      /* - connect using netbios.                                            */
      /* - get the logical adapter name.                                     */
      /***********************************************************************/
      pConnection->ConnectType = _NETBIOS;
      len = strlen(pOption+2);
      if( len != 0 )
      {
       if( len > ( MAX_LSN_NAME - LSN_RES_NAME ) )
        len = MAX_LSN_NAME - LSN_RES_NAME;
       cp = Talloc(len + 1);
       strncpy( cp, pOption+2, len );
       pConnection->pLsnName = cp;
      }
      break;

     case 't':
     {
      /***********************************************************************/
      /* - connect using sockets.                                            */
      /***********************************************************************/
      pConnection->ConnectType = SOCKET;
      len = strlen(pOption+2);
      cp  = Talloc(len + 1);

      strncpy( cp, pOption+2, len );

      pConnection->pLsnName = cp;
     }
     break;

     case 'b':
      /***********************************************************************/
      /* - This option tells the probe that child processes are going to     */
      /*   be debugged. It should only show up in the call to parse          */
      /*   invocation options when the "parent" probe is spawned by the      */
      /*   debugger which only happens when debugging child(multiple)        */
      /*   processes on a single machine using local pipes.                  */
      /***********************************************************************/
      pConnection->ConnectType = LOCAL_PIPE;
      break;


     /************************************************************************/
     /* - handle invalid "/" option.                                         */
     /************************************************************************/
     default:
      printf("\n Invalid Option");
       goto casehelp;


    }
    break;

   default:
    printf("\n Invalid Option");
    goto casehelp;
  }
  pOption = strtok(NULL," " );
 }
 while( pOption );
}

/*****************************************************************************/
/* EspParseComPort                                                           */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*  Get the com port to use for remote debugging.                            */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*  c            user specified com port 1-4.                                */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*  ComPort      user specified Com Port 1-4.                                */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
int EspParseComPort( char c )
{
 int ComPort;

 switch( c )
 {
  case '1':
   ComPort = 1;
   break;

  case '2':
   ComPort = 2;
   break;

  case '3':
   ComPort = 3;
   break;

  case '4':
   ComPort = 4;
   break;

  default:
   SayMsg(ERR_COM_PORT_INVALID);
   break;
 }
 return(ComPort);
}

/*****************************************************************************/
/* Handler()                                                                 */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*  Handler for SD386 exceptions. Currently, we're only interested in        */
/*  C-Break.                                                                 */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   x        See the OS/2 doc.                                              */
/*   y                                                                       */
/*   z                                                                       */
/*   a                                                                       */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
ULONG _System Handler(PEXCEPTIONREPORTRECORD x,
                      PEXCEPTIONREGISTRATIONRECORD y,
                      PCONTEXTRECORD z,
                      PVOID a)
{
 ULONG        rc;

 switch (x->ExceptionNum)
 {
  case XCPT_GUARD_PAGE_VIOLATION :
  case XCPT_UNABLE_TO_GROW_STACK :
   rc = XCPT_CONTINUE_SEARCH;
   break;

  case XCPT_ACCESS_VIOLATION :
  case XCPT_INTEGER_DIVIDE_BY_ZERO :
  case XCPT_FLOAT_DIVIDE_BY_ZERO :
  case XCPT_FLOAT_INVALID_OPERATION :
  case XCPT_ILLEGAL_INSTRUCTION :
  case XCPT_PRIVILEGED_INSTRUCTION :
  case XCPT_INTEGER_OVERFLOW :
  case XCPT_FLOAT_OVERFLOW :
  case XCPT_FLOAT_UNDERFLOW :
  case XCPT_FLOAT_DENORMAL_OPERAND :
  case XCPT_FLOAT_INEXACT_RESULT :
  case XCPT_FLOAT_STACK_CHECK :
  case XCPT_DATATYPE_MISALIGNMENT :
  case XCPT_ASYNC_PROCESS_TERMINATE:
   DosUnsetExceptionHandler(y);
   rc = XCPT_CONTINUE_SEARCH;
   break;

  default :
   rc = XCPT_CONTINUE_SEARCH;
   break;

  case XCPT_SIGNAL:
   switch (x->ExceptionInfo[0])
   {
     case XCPT_SIGNAL_KILLPROC :
      {
       ALLPIDS *p = NULL;

       p = GetPid( GetEspProcessID() );
       /**********************************************************************/
       /* - if p==NULL, then the probe is being killed before starting       */
       /*   to debug any processes.                                          */
       /**********************************************************************/
       if( p == NULL)
        goto casexcpt_signal_intr;

       if( p->PidFlags.CtrlBreak == TRUE )
       {
        /*********************************************************************/
        /* - If we fall into this code, then the user is trying to do        */
        /*   a Ctrl-Break:                                                   */
        /*                                                                   */
        /*    - Thread 1 is waiting on a DosDebug command.                   */
        /*    - The signal was sent by the esp queue in response to          */
        /*      a Ctrl-Break message.                                        */
        /*********************************************************************/
        p->PidFlags.CtrlBreak = FALSE;
        rc = XCPT_CONTINUE_EXECUTION;
       }
       else
       {
        /*********************************************************************/
        /* - If we get here, then we're being killed by an external process  */
        /*   such as PSPM2. We need to be a good citizen and die.            */
        /*********************************************************************/
        rc = XCPT_CONTINUE_SEARCH;
       }

      }
      break;

casexcpt_signal_intr:
     case XCPT_SIGNAL_INTR :
     case XCPT_SIGNAL_BREAK :
     {
      /***********************************************************************/
      /* - Come here when the user enters a Ctrl-Break in the probe          */
      /*   session.                                                          */
      /***********************************************************************/
      CloseConnectSema4();
      ConnectClose( DEFAULT_HANDLE );
      DosUnsetExceptionHandler(y);
      rc = XCPT_CONTINUE_SEARCH;
     }
     break;
   }
   break;
 }
 return(rc);
}


/*****************************************************************************/
/* ParseEspChildOptions()                                                    */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   - Parse the invocation options coming to a child probe.                 */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   argc        Number of input parameters.                                 */
/*   argv        -> -> to argument strings.                                  */
/*   pEspParms   -> to the structure that will define the invocation parms.  */
/*   pConnection -> to the structure that will define the connection parms.  */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   void                                                                    */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
void ParseEspChildOptions( int          argc,
                           char       **argv,
                           ESP_PARMS    *pEspParms,
                           CONNECTION  *pConnection)
{
 char          ParmString[512];
 int           i;
 char         *pOption;
 char         *cp;
 int           len;


 /****************************************************************************/
 /* - set any default options.                                               */
 /****************************************************************************/
 pConnection->DbgOrEsp = _ESP;

 pEspParms->EspFlags.IsParentEsp    = FALSE;

 /****************************************************************************/
 /* - Build a string of tokens delimited by a single blank.                  */
 /****************************************************************************/
 ParmString[0] = '\0';
 for( i=1 ; i<argc; i++)
 {
  strcat(ParmString,argv[i]);
  strcat(ParmString," ");
 }
 strcat(ParmString,"\0");

 /****************************************************************************/
 /* - Scan the parm block and pull off the options and program name.         */
 /****************************************************************************/
 pOption = strtok(ParmString," ");
 do
 {
  printf("\n%s", pOption);fflush(0);
  switch( *pOption )
  {
   case '/':
    switch( tolower(*(pOption + 1)) )
    {
     case 'a':
      /***********************************************************************/
      /* - Parse the com port.                                               */
      /***********************************************************************/
      pConnection->ComPort = EspParseComPort( *(pOption+2) );
      break;

     case 'r':
      pConnection->ConnectType    = ASYNC;
      break;

     case '+': /* Joe's private option */
      pEspParms->EspFlags.Verbose = TRUE;
      break;

     case 'n':
      /***********************************************************************/
      /* - connect using netbios.                                            */
      /* - get the logical adapter name.                                     */
      /***********************************************************************/
      pConnection->ConnectType    = _NETBIOS;
      len = strlen(pOption+2);
      if( len != 0 )
      {
       if( len > ( MAX_LSN_NAME - LSN_RES_NAME ) )
        len = MAX_LSN_NAME - LSN_RES_NAME;
       cp = Talloc(len + 1);
       strncpy( cp, pOption+2, len );
       pConnection->pLsnName = cp;
      }
      break;                                                            /*919*/

     case 't':
     {
      /***********************************************************************/
      /* - connect using sockets.                                            */
      /***********************************************************************/
      pConnection->ConnectType = SOCKET;
      len = strlen(pOption+2);
      cp  = Talloc(len + 1);

      strncpy( cp, pOption+2, len );

      pConnection->pLsnName = cp;
     }
     break;

     case 'c':
      /***********************************************************************/
      /* - handle the /child= option which MUST be argv[1];                  */
      /***********************************************************************/
      break;


     case 'b':
      /***********************************************************************/
      /* - This option tells the probe that child processes are going to     */
      /*   be debugged. It should only show up when esp is spawned           */
      /*   internally when specifying the /b option on the sd386             */
      /*   invocation line. Note that esp does not have an explicit          */
      /*   /b invocation option.                                             */
      /***********************************************************************/
      pConnection->ConnectType = LOCAL_PIPE;
      break;

     case 's':
      /***********************************************************************/
      /* - Set the shared heap memory handle.                                */
      /***********************************************************************/
      if( strstr( pOption+1, "shr" ) )
      {
       ULONG *pshrmem;

       pshrmem = (ULONG*)atol( strchr( pOption+1, '=' ) + 1 );
       SetShrMem( pshrmem );
      }
      break;

     case 'h':
      /***********************************************************************/
      /* - Set the start address of the shared heap.                         */
      /***********************************************************************/
      if( strstr( pOption+1, "heap" ) )
      {
       ULONG *heap;

       heap = (ULONG*)atol( strchr( pOption+1, '=' ) + 1 );
       SetShrHeap( heap );
      }
      else if( strstr( pOption+1, "handle" ) )
      {
       pEspParms->handle = (LHANDLE)atol( strchr( pOption+1, '=' ) + 1 );
      }
      break;

     case 'd':
     {
      int descriptor;
      /***********************************************************************/
      /* - Get the base socket descriptor if using sockets.                  */
      /***********************************************************************/
      descriptor = (int)atol( strchr( pOption+1, '=' ) + 1 );
      SockSetDescriptor(descriptor);
     }
     break;

     case 'q':
      /***********************************************************************/
      /* - Set the queue name for sending messages.                          */
      /***********************************************************************/
      if( strstr( pOption+1, "qname" ) )
      {
       char *pQueName;

       pQueName = strchr( pOption+1, '=' ) + 1 ;
       SetEspQueName( pQueName );
      }
      break;

     /************************************************************************/
     /* - handle invalid "/" option.                                         */
     /************************************************************************/
     default:
      break;

    }
    break;

   default:
    break;
  }
  pOption = strtok(NULL," " );
 }
 while( pOption );
}

/*****************************************************************************/
/* - Display a fatal error message in a child probe and then exit.           */
/*****************************************************************************/
void MyExit( void )
{
 ESP_QUE_ELEMENT  Qelement;
 USHORT           PidOfThisEsp;
 TIB             *pTib;
 PIB             *pPib;

 /****************************************************************************/
 /* - If we get an error in a child probe, then we need to bring that        */
 /*   probe session to the foreground and display the message.               */
 /****************************************************************************/
 if( IsParent() == FALSE )
 {
  DosGetInfoBlocks(&pTib,&pPib);

  PidOfThisEsp = (USHORT)pPib->pib_ulpid;

  Qelement.ChildPid = PidOfThisEsp;
  if( UseExecPgm() == FALSE )
   SendMsgToEspQue( ESP_QMSG_SELECT_ESP, &Qelement, sizeof(Qelement) );
  DosSleep(15000);
 }
 exit(0);
}
