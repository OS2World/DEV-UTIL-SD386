/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   sd386.c                                                                 */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Startup program for SD386.                                              */
/*                                                                           */
/* History:                                                                  */
/*                                                                           */
/*   11/12/93 Rewritten.                                                     */
/*                                                                           */
/*****************************************************************************/

#include "all.h"

extern KEY2FUNC      defk2f[];
extern int           AppTerminated;
extern PtraceBuffer  AppPTB;
extern int           BlowBy;
extern UCHAR        *BoundPtr;
extern UINT          TopLine;
extern UINT          VideoCols;
extern UINT          CurrentMid;
extern UCHAR         Re_Parse_Data;

PROCESS_NODE        *pnode;
jmp_buf              RestartContext;

static EXCEPTIONREGISTRATIONRECORD  *preg_rec;
static BRK *allbrks = NULL;

CmdParms             cmd;

ULONG DbgGetProcessID( void ) { return(cmd.ProcessID);   }
ULONG DbgGetSessionID( void ) { return(cmd.SessionID);   }

BOOL IsParent       ( void ) { return(cmd.DbgFlags.IsParentDbg)   ;}
BOOL SingleMultiple ( void ) { return(cmd.DbgFlags.SingleMultiple);}
BOOL IsVerbose      ( void ) { return(cmd.DbgFlags.Verbose)       ;}
BOOL DosDebugTrace  ( void ) { return(cmd.DbgFlags.DosDebugTrace );}
BOOL UseExecPgm     ( void ) { return(cmd.DbgFlags.UseExecPgm )   ;}
BOOL IsDbgDebugChild( void ) { return(cmd.DbgFlags.DebugChild)    ;}
BOOL UseDebug       ( void ) { return(cmd.DbgFlags.UseDebug)      ;}
BOOL IsHotKey       ( void ) { return(cmd.DbgFlags.HotKey)        ;}

int main( int argc, char **argv )
{
 APIRET        rc;
 DEBFILE      *pdf;
 ULONG         ExeEntryPt;
 ULONG         RunTimeEntryPt;
 ULONG         MainEntryPt;
 char          buffer[CCHMAXPATH];
 char         *ExeFileSpec;
 CONNECTION    Connection;
 ESP_RUN_OPTS *pEspRunOpts = NULL;
 char          BadStartBuffer[CCHMAXPATH];
 ALLPIDS      *p;
 char          rc_string[12];
 USHORT        DbgPid;

 EXCEPTIONREGISTRATIONRECORD  reg_rec = {0,&Handler};

 if( argc == 1 )
  SayMsg(HELP_INVOCATION_SD386);

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
 /*   - /shr=xxxxx    where shr    = switch for shared heap memory handle.   */
 /*                         xxxxx  = shared heap memory handle.              */
 /*                                                                          */
 /*   - /heap=xxxxx   where heap   = switch for shared heap start address.   */
 /*                         xxxxx  = shared heap start address.              */
 /*                                                                          */
 /*   - /handle=xxxxx where handle = switch for com handle( serial only )    */
 /*                         xxxxx  = parent's com handle - inherited by      */
 /*                                  the child.                              */
 /*                                                                          */
 /*   - /qname=xxxxx where  qname  = switch for dbg queue name.              */
 /*                         xxxxx  = dbg queue name.                         */
 /*                                                                          */
 /****************************************************************************/
 memset(&cmd, 0, sizeof(cmd));
 memset(&Connection, 0, sizeof(Connection) );
 if( strstr( argv[1], "/child" ) )
 {
  /***************************************************************************/
  /* - This code is added when were trying to debug a child debugger so      */
  /*   that we can gain access to shared memory.                             */
  /***************************************************************************/
#if 0
  {
   TIB             *pTib;
   PIB             *pPib;
   USHORT           ThisPid;
   USHORT           PidToBeDebugged;
   DBG_QUE_ELEMENT  Qelement;
   char            *pQueName;

   PidToBeDebugged = atoi( strchr( argv[1], '=' ) + 1 );


   DosGetInfoBlocks(&pTib, &pPib);
   ThisPid = (USHORT)pPib->pib_ulpid;

   pQueName = strchr( argv[4]+1, '=' ) + 1 ;

   printf("\n%s",pQueName);fflush(0);
   SetDbgQueName( pQueName );

   Qelement.pid = PidToBeDebugged;
   Qelement.sid = ThisPid;

   SendMsgToDbgQue(DBG_QMSG_REQUEST_ACCESS, &Qelement, sizeof(Qelement));
   printf("\nmessage sent to que ");fflush(0);

   printf("\nwaiting for shared memory access...");fflush(0);
   DosSleep(10000);
  }
#endif

  ParseChildInvocationOptions( argc, argv, &cmd, &Connection );
 }
 else
  ParseInvocationOptions( argc, argv, &cmd, &Connection );

 /****************************************************************************/
 /* - handle a null exe for the parent session.                              */
 /****************************************************************************/
 if( (IsParent() == TRUE ) && (cmd.pUserExe == NULL) )
  SayMsg(ERR_NO_EXE);


 /****************************************************************************/
 /* - If this is a child debugging session, then we set the process id       */
 /*   and session id of the debuggee.                                        */
 /* - The child process id and the child session id were sent over by        */
 /*   the probe along with the new process notification and were added       */
 /*   to the node for this pid prior to spawing the child debugger.          */
 /****************************************************************************/
 if( IsParent() == FALSE )
 {
  p = GetPid( cmd.ProcessID);
  cmd.SessionID   = 0;
  cmd.ProcessType = p->type;
 }

 /****************************************************************************/
 /* - If we're debugging child processes and the above parsing did not       */
 /*   specify a connection, then we default to a single machine.  In this    */
 /*   case, the probe is still considered to be remote since we'll be        */
 /*   debugging over a local pipe.                                           */
 /*                                                                          */
 /* - Send the connection details to the router.                             */
 /****************************************************************************/
 if( IsDbgDebugChild() && (Connection.ConnectType == BOUND) )
  Connection.ConnectType = LOCAL_PIPE;
 SendConnectionToRouter( &Connection );
 /****************************************************************************/
 /* - Before making the connection, there needs to a probe that we can       */
 /*   connect to.  If the user has specified the /b invocation option to     */
 /*   debug child processes and we're running in a single machine            */
 /*   configuration, then we need to spawn a parent esp to connect to.       */
 /****************************************************************************/
 if( (ConnectType() == LOCAL_PIPE) && (IsParent() == TRUE) )
 {
  ULONG           ErrorBufLen;
  ESP_SPAWN_FLAGS SpawnFlags;

  ErrorBufLen = sizeof(BadStartBuffer);
  memset( BadStartBuffer, 0, ErrorBufLen );

  /***************************************************************************/
  /*                                                                         */
  /* - We want to spawn an invisible probe.  Why?  Because the               */
  /*   Session Manager has a restriction that only a parent can              */
  /*   bring a child to the foreground, and since the probe is the           */
  /*   parent of the debuggee, it must be brought to the foreground          */
  /*   before the child's session can be selected.  However, seeing          */
  /*   the probe session pop up in front of your face is annoying so         */
  /*   we make it invisible.                                                 */
  /*                                                                         */
  /* - We always want to start the top level probe using                     */
  /*   DosStartSession() when debugging child process(es) on a               */
  /*   single machine.                                                       */
  /*                                                                         */
  /***************************************************************************/
  SpawnFlags.Visible     = ESP_INVISIBLE;
  SpawnFlags.SpawnMethod = ESP_USE_DOSSTARTSESSION;
  SpawnFlags.SpawnOrigin = ESP_SPAWN_FROM_DBG;

  rc = SpawnProbe( &cmd.EspPid,
                   &cmd.EspSid,
                    NULL,
                   ErrorBufLen,
                   BadStartBuffer,
                   SpawnFlags);

  if( rc != 0 )
  {
   char rcstring[6];

   sprintf(rcstring,"%-d",rc);
   ErrorPrintf( ERR_DOSSTARTSESSION, 2, rcstring, BadStartBuffer );
  }
  /***************************************************************************/
  /* - See the comments above about an invisible probe.                      */
  /***************************************************************************/
  DosSelectSession( cmd.EspSid );
 }

 /****************************************************************************/
 /* - if debugging remote( including local pipes then make the               */
 /*   connection.                                                            */
 /****************************************************************************/
 if( IsEspRemote() )
 {
  int   RcMoreInfo = 0;

  rc = ConnectInit( &RcMoreInfo );

  if( rc != 0 )
  {
   char  BadConnectMsg[32] = "";
   int   MsgId;
   int   n;

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

     case TCPIP_NO_HOST_NAME:
      n     = 0;
      MsgId = ERR_TCPIP_NO_HOST_NAME;
      break;

     case TCPIP_ESP_NOT_STARTED:
      n     = 0;
      MsgId = ERR_TCPIP_ESP_NOT_STARTED;
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
 /* - Initialize the screen and display the logo.                            */
 /* - From here on error messages should show up in error boxes.             */
 /****************************************************************************/
 VioInit();

 /****************************************************************************/
 /* - Read the user profile if invoked. This must come after VioInit()       */
 /*   because VioInit() defines some arrays that we need.                    */
 /****************************************************************************/
 if( cmd.Profile == TRUE )
  Profile( defk2f );

 UpdatePullDownsWithAccelKeys();
 SetDefaultExcepMap();
 SetDefaultColorMap();


 /****************************************************************************/
 /*                                                                          */
 /* - If this is a child debugger and we're debugging multiple processes     */
 /*   over a serial connection, then                                         */
 /*                                                                          */
 /*    - We need a connect sema4 that will be used for connecting and        */
 /*      disconnecting the the debugger session. It is created in the        */
 /*      set state.                                                          */
 /*                                                                          */
 /*    - We need to tell the router the com handle since it was inherited    */
 /*      from the parent and passed to this debugger through the invocation  */
 /*      paramenters.                                                        */
 /*                                                                          */
 /*    - We need to wait until this debugger gets connected.                 */
 /*                                                                          */
 /****************************************************************************/
 if( (IsParent() == FALSE) && (SerialParallel() == SERIAL) )
 {
  p      = GetPid( cmd.ProcessID );
  DbgPid = p->DbgPid;
  CreateConnectSema4( DbgPid, _DBG );
  OpenSerialMutex();
  SetComHandle( cmd.handle );
  SerialConnect( JUST_WAIT, cmd.ProcessID, _DBG, SendMsgToDbgQue );
 }

 /****************************************************************************/
 /* - Each child debugger will have a termination que so we can kill the     */
 /*   child debuggers on quit/restart.                                       */
 /****************************************************************************/
 if( IsParent() == FALSE )
 {
  rc = StartDbgTermQue();
  if( rc != 0 )
  {
   sprintf(rc_string, "%d",rc);
   Error( ERR_CANT_START_QUE, TRUE, 1, rc_string );
  }
 }

 /*--------------------------------------------------------------------------*/
 /* - At this point, we are connected to the probe and can start             */
 /*   sending/receiving messages.                                            */
 /*--------------------------------------------------------------------------*/

 /****************************************************************************/
 /* - Build runtime options for the probe. The options will be held in       */
 /*   dynamic memory and will be freed after the connection is made and      */
 /*   the options are sent.                                                  */
 /* - The single/multiple flag in DbgFlags is also set at this time by       */
 /*   the call to BuildEspParms().                                           */
 /****************************************************************************/
 if( IsEspRemote() )
 {
  pEspRunOpts = BuildEspParms(&cmd);
  if(pEspRunOpts == NULL )
    Error(ERR_RESPONSE_FILE, TRUE, 1, cmd.pChildProcesses);

  /***************************************************************************/
  /* - Add any flags that need to go to the probe.                           */
  /***************************************************************************/
  pEspRunOpts->EspFlags.UseExecPgm     = cmd.DbgFlags.UseExecPgm;
  pEspRunOpts->EspFlags.DebugChild     = cmd.DbgFlags.DebugChild;
  pEspRunOpts->EspFlags.DosDebugTrace  = cmd.DbgFlags.DosDebugTrace;
  pEspRunOpts->EspFlags.SingleMultiple = cmd.DbgFlags.SingleMultiple;
  pEspRunOpts->EspFlags.UseDebug       = cmd.DbgFlags.UseDebug;

  if( (SerialParallel() == SERIAL) && (SingleMultiple() == MULTIPLE) )
   RequestSerialMutex();
  xSetEspRunOpts(pEspRunOpts);
  Tfree(pEspRunOpts);
 }

 /****************************************************************************/
 /* - Get/Set the fully qualified filespec for the debuggee.                 */
 /****************************************************************************/
 if( IsParent() == TRUE )
 {
  rc = xFindExe(cmd.pUserExe,buffer,sizeof(buffer));
  if( rc != 0 )
   Error(ERR_FILE_CANT_FIND_EXE,TRUE, 1, cmd.pUserExe);
  ExeFileSpec = Talloc(strlen(buffer)+1);
  strcpy(ExeFileSpec,buffer);
  Tfree(cmd.pUserExe);
  cmd.pUserExe = ExeFileSpec;

  /***************************************************************************/
  /* - Come here on Restart().                                               */
  /***************************************************************************/
  while( setjmp(RestartContext) )
   RestartInit();
 }

 /****************************************************************************/
 /* - register an exception handler for thread 1. We need it for             */
 /*   Ctrl-Break support.                                                    */
 /****************************************************************************/
 DosSetExceptionHandler(&reg_rec);
 preg_rec = &reg_rec;

 /****************************************************************************/
 /* - Now, handle the initial startup.                                       */
 /****************************************************************************/
 if( IsParent() == TRUE )
 {
  if( IsEspRemote() )
  {
   /**************************************************************************/
   /* - start up an esp queue.                                               */
   /* - start up a  dbg queue.                                               */
   /**************************************************************************/
   rc = xStartEspQue();
   if( rc != 0 )
   {
    sprintf(rc_string, "%d",rc);
    Error( ERR_CANT_START_QUE, TRUE, 1, rc_string );
   }

   rc = StartDbgQue();
   if( rc != 0 )
   {
    sprintf(rc_string, "%d",rc);
    Error( ERR_CANT_START_QUE, TRUE, 1, rc_string );
   }

   /**************************************************************************/
   /*                                                                        */
   /* - For parallel connections, the queues are connected with a session    */
   /*   of their own. This session is distinct from the connection that      */
   /*   will be used for api services.                                       */
   /* - The handle for this connection is owned solely by the queue threads. */
   /*                                                                        */
   /**************************************************************************/
   if( SerialParallel() == PARALLEL)
   {
    xStartQueListenThread();
    SendMsgToDbgQue(DBG_QMSG_OPEN_CONNECT,NULL,0);
   }
  }

  /***************************************************************************/
  /* - start the debuggee and handle any errors.                             */
  /***************************************************************************/
  memset( BadStartBuffer, 0, sizeof(BadStartBuffer) );
  rc = xStartUser( cmd.pUserExe,
                   cmd.pUserParms,
                   cmd.SessionType,
                  &cmd.SessionID,
                  &cmd.ProcessID,
                  &cmd.ProcessType,
                   sizeof(BadStartBuffer),
                   BadStartBuffer);

  if( rc != 0 )
  {
   char rcstring[6];

   sprintf(rcstring,"%-d",rc);
   Error( ERR_DOSSTARTSESSION, TRUE, 2, rcstring, BadStartBuffer );
  }

  /***************************************************************************/
  /* - Build the pid structure.                                              */
  /***************************************************************************/
  AddPid( cmd.ProcessID,
          cmd.SessionID,
          cmd.EspSid,
          cmd.ProcessType,
          cmd.pUserExe );

 }

 /*--------------------------------------------------------------------------*/
 /* !!!These comments are out of date and have been left here as a basis     */
 /*    for updating in the future.                                           */
 /*                                                                          */
 /* - At this point, all of the resources have been allocated according      */
 /*   to the following chart.                                                */
 /*                                                                          */
 /*  1. Local -Parent Process - Bound                                        */
 /*  2. Remote-Parent Process - Async                                        */
 /*  3. Remote-Parent Process - Netbios                                      */
 /*                                                                          */
 /*  4. Remote-Single Child Process - Local pipe                             */
 /*  5. Remote-Single Child Process - Async                                  */
 /*  6. Remote-Single Child Process - Netbios                                */
 /*                                                                          */
 /*  7. Remote-Multiple Child Processes - Local pipe                         */
 /*  8. Remote-Multiple Child Processes - Async                              */
 /*  9. Remote-Multiple Child Processes - Netbios                            */
 /*                                                                          */
 /*                                               ------- Parent Dbg only    */
 /*                                              .    .                      */
 /*                                              .    .                      */
 /*                                              .    .         -------Parent*/
 /*                                              .    .        .    .  Esp   */
 /*                                              .    .        .    .  only  */
 /*                                              .    .        .    .        */
 /*                                              .    .        .    .        */
 /*                        | c t | c s | c d t | d | d t | s | e | e t |     */
 /*                        | o y | o e | h i h | b | b h | h | s | s h |     */
 /*                        | n p | n m | e s r | g | g r | r | p | p r |     */
 /*                        | n e | n a | c c e |   |   e | d |   |   e |     */
 /*                        | e   | e 4 | k o a | q | l a |   | q | l a |     */
 /*                        | c   | c   |   n d | u | i d | m | u | i d |     */
 /*                        | t   | t   | f n   | e | s   | e | e | s   |     */
 /*                        |     |     | o e   | u | t   | m | u | t   |     */
 /*  Local/|Connect|Parent/|     |     | r c   | e | e   |   | e | e   |     */
 /*  Remote|Class  |Child  |     |     |   t   |   | n   |   |   | n   |     */
 /*  ------|---------------|-----|---- |-------|---|-----|---|---|-----|     */
 /* 1. L   | BOUND |  P    | B   |  N  |  N    | N |  N  | N | N |  N  |     */
 /* 2. R   | SERIAL|  P    | A   |  N  |  N    | Y |  N  | N | Y |  N  |     */
 /* 3. R   | PRLLEL|  P    | N   |  N  |  N    | Y |  Y  | N | Y |  Y  |     */
 /*        |       |       |     |     |       |   |     |   |   |     |     */
 /* 4. R   | PRLLEL|  C    | LP  |  N  |  N    | Y |  Y  | N | Y |  Y  |     */
 /* 5. R   | SERIAL|  C    | A   |  N  |  N    | Y |  N  | N | Y |  N  |     */
 /* 6. R   | PRLLEL|  C    | N   |  N  |  N    | Y |  Y  | N | Y |  Y  |     */
 /*        |       |       |     |     |       |   |     |   |   |     |     */
 /* 7. R   | PRLLEL|  M    | LP  |  Y  |  N    | Y |  Y  | Y | Y |  Y  |     */
 /* 8. R   | SERIAL|  M    | A   |  Y  |  Y    | Y |  N  | Y | Y |  N  |     */
 /* 9. R   | PRLLEL|  M    | N   |  Y  |  N    | Y |  Y  | Y | Y |  Y  |     */
 /*                                                                          */
 /*                                                                          */
 /*--------------------------------------------------------------------------*/
 if( (Connection.ConnectType==BOUND) || (Connection.ConnectType==LOCAL_PIPE) )
 {
  p = GetPid( cmd.ProcessID );

  /***************************************************************************/
  /* - If the user's session is a PM session and SD386 is being started in   */
  /*   a VIO window, then we have a problem and need to give the user a      */
  /*   warning.                                                              */
  /* - If the call to GetProcessType() fails, then proceed. The net result   */
  /*   will be that he will miss the warning message and may proceed to      */
  /*   hang his machine. This is basically what happened in releases prior   */
  /*   to this.                                                              */
  /***************************************************************************/
  if( p->type == SSF_TYPE_PM )
  {
   ULONG    MyType;

   if( GetProcessType( &MyType ) == 0 )
   {
    if( MyType == SSF_TYPE_WINDOWABLEVIO )
     Error(ERR_PM_APP_TYPE, FALSE, 0 );
   }
  }
  else
  /***************************************************************************/
  /* - If the user's application is not a PM app and he has turned off       */
  /*   Ctrl-Esc and Alt-Esc access to the desktop, then turn access back on  */
  /* - It it only needs to be turned off when the application is PM.         */
  /***************************************************************************/
  {
   if( IsHotKey() )
   {
    cmd.DbgFlags.HotKey = FALSE;
    SetHotKey();
   }
  }
 }

 /****************************************************************************/
 /* -Set the maximum number of low level I/O files open for SD386.           */
 /****************************************************************************/
 DosSetMaxFH(250);

 /****************************************************************************/
 /* - Allocate the process node.                                             */
 /****************************************************************************/
 pnode          = (PROCESS_NODE*)Talloc(sizeof(PROCESS_NODE));
 pnode->allbrks = allbrks;              /* need for restart.                 */
 pnode->pid     = cmd.ProcessID;
 pnode->sid     = cmd.SessionID;

 /****************************************************************************/
 /* - Do the DosDebug initialization.                                        */
 /****************************************************************************/
 rc  = GoInit( cmd.ProcessID, cmd.SessionID );
 if( rc != 0 )
 {
  char buf[10];

  if( rc == 1 )
   Error(ERR_DBG_INIT, TRUE, 1, cmd.pUserExe );
  else
  {
   sprintf(buf,"%d",rc);
   Error(ERR_DOSDEBUG_INIT, TRUE, 1, buf);
  }
 }

 /****************************************************************************/
 /* - Get the EXE, the c-runtime, and the main entry points.                 */
 /*   (pdf is a global that will have been set in GoInit()/exeint().         */
 /****************************************************************************/
 pdf = pnode->ExeStruct;
 if( pdf != NULL &&
     pdf->EntryExitPt != 0
   )
  ExeEntryPt = pdf->EntryExitPt;
 else
  Error(ERR_EXE_ENTRY,TRUE,1,cmd.pUserExe);

 MainEntryPt = 0;
 RunTimeEntryPt = 0;
 GetRunTimeEntryPt( &RunTimeEntryPt, pdf);
 GetMainEntryPt   ( &MainEntryPt,    pdf);

 /****************************************************************************/
 /* - Display the action bar.                                                */
 /****************************************************************************/
 DisplayMenu();

 /****************************************************************************/
 /* - Initialize for breakpoint files.                                       */
 /****************************************************************************/
 InitBreakpoints();

 /****************************************************************************/
 /* - Add MSH Initialization                                                 */
 /****************************************************************************/
#ifdef MSH
 if( cmd.DbgFlags.UseMsh == TRUE )
  mshInit();
#endif

/* commandLine.nparms=0; */

 /****************************************************************************/
 /* - Finally...pant...pant...pant, let's go do it.                          */
 /****************************************************************************/
 Run(MainEntryPt,ExeEntryPt);
/* SD386Log(-1,NULL); */ /*Close the logfile.*/
 return(0);
}

/*****************************************************************************/
/* GetRunTimeEntryPt()                                                       */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*  Get the location of exe c-runtime entry point.                           */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*  fp              FILE * structure of exe/dll file.                        */
/*  pRunTimeEntryPt Where to put the address.                                */
/*  pid             Process id.                                              */
/*  mte             Module table handle of exe/dll.                          */
/*  pdf             debug file structure for exe.                            */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*  void                                                                     */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/

#define NUM_OF_RUNTIME_NAMES  2
char *C_RunTimePubName[NUM_OF_RUNTIME_NAMES] =
{
 "__RunExitList",
 "__astart"
};

void GetRunTimeEntryPt( ULONG *pRunTimeEntryPt, DEBFILE *pdf )

{
 int          i;

 *pRunTimeEntryPt = 0;
 for( i = 0; i < NUM_OF_RUNTIME_NAMES ; i++ )
 {
  if( (*pRunTimeEntryPt = DBPub(C_RunTimePubName[i],pdf) ) != NULL )
   return;
 }
 return;
}

/*****************************************************************************/
/* GetMainEntryPt()                                                          */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*  Get the location of exe c-runtime entry point.                           */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*  fp              FILE * structure of exe/dll file.                        */
/*  pRunTimeEntryPt Where to put the address.                                */
/*  pid             Process id.                                              */
/*  mte             Module table handle of exe/dll.                          */
/*  pdf             debug file structure for exe.                            */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*  void                                                                     */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/

#define NUM_OF_MAIN_NAMES  3
char *MainPubName[NUM_OF_MAIN_NAMES] =
{
 "main"    ,
 "_main"   ,
 "winmain"
};

void GetMainEntryPt( ULONG *pMainEntryPt, DEBFILE *pdf )
{
 int          i;

 *pMainEntryPt = 0;
 for( i = 0; i < NUM_OF_MAIN_NAMES ; i++ )
 {
  if( (*pMainEntryPt = DBPub(MainPubName[i],pdf) ) != NULL )
   return;
 }
 return;
}

/*****************************************************************************/
/* RestartInit()                                                             */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Initialize for restarting.                                              */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
void RestartInit( void )
{
 DEBFILE    *pdf;
 DEBFILE    *pdfnext;

 FreeThreadList();

 for( pdf=pnode->ExeStruct; pdf != NULL; pdf=pdf->next )
  freepdf(pdf);

 for( pdf=pnode->ExeStruct; pdf != NULL; )
 {
  pdfnext=pdf->next;
  Tfree((void*)pdf);
  pdf=pdfnext;
 }


 allbrks = pnode->allbrks;

 Tfree(pnode);
 pnode = NULL;
 FreeAllPids();

 afilefree();
 dfilefree();Re_Parse_Data = TRUE;
 FreeDisasmViewBuffers();

 AppTerminated = FALSE;
 CurrentMid = 0;

 /***************************************************************************/
 /* - The BlowBy flag is used to skip the GetFuncsFromEvents call in        */
 /*   showc() and showa so that we can execute functions from the           */
 /*   data window and come back into the data window without stopping       */
 /*   in showc() or showa().                                                */
 /* - To avoid an annoying blink when running functions from the data       */
 /*   window we set the BoundPtr[TopLine] = 0 so that this line could       */
 /*   not be written to. What we have to do here is reset the BoundPtr      */
 /*   back to what it should be.                                            */
 /*                                                                         */
 /***************************************************************************/
 {
  extern UCHAR  Reg_Display;
  BlowBy = NO;
  BoundPtr[TopLine] = VideoCols;
  if(TestBit(Reg_Display,REGS386BIT))
   BoundPtr[TopLine] = VideoCols-REGSWINWIDTH;
  if(TestBit(Reg_Display,REGS387BIT))
   BoundPtr[TopLine] = VideoCols-COREGSWINWIDTH;
 }

 /***************************************************************************/
 /* - Free the mte table from a previous run.                               */
 /***************************************************************************/
  FreeMteTable();

  FreeMemBlks();

 /***************************************************************************/
 /* - Free the SD386 breakpoint environment variable.                       */
 /***************************************************************************/
  FreeSD386Brk();

/*_dump_allocated(0);*/
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
 ULONG rc;
 UINT  key;

 switch (x->ExceptionNum)
 {
  /***************************************************************************/
  /* - give these non-fatal exceptions back to the os.                       */
  /***************************************************************************/
  case XCPT_GUARD_PAGE_VIOLATION :
  case XCPT_UNABLE_TO_GROW_STACK :
   rc = XCPT_CONTINUE_SEARCH;
   break;

  /***************************************************************************/
  /* - these are fatal exceptions that should not happen;however,            */
  /*   in case they do, we get rid of our exception handler and give it      */
  /*   back to the os.                                                       */
  /***************************************************************************/
  default :
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
   rc = XCPT_CONTINUE_SEARCH;
   DosUnsetExceptionHandler(y);
   break;

  /***************************************************************************/
  /* - At this point, we ask the user if he wants to "stop the app" or    917*/
  /*   "kill sd386." If he wants to kill sd386, then we handle it as      917*/
  /*   any other fatal exception like we did above. If he wants to        917*/
  /*   stop the app, then we tell the os that we handled the signal.      917*/
  /*   This will cause DosDebug() to break out of the kernel and this is  917*/
  /*   the effect that we want.                                           917*/
  /***************************************************************************/
  case XCPT_SIGNAL:                                                     /*917*/
   switch (x->ExceptionInfo[0])
   {
caseXCPT_SIGNAL_BREAK:
    case XCPT_SIGNAL_BREAK :
    case XCPT_SIGNAL_INTR :
     /************************************************************************/
     /* - Come here to handle the Ctrl-Break dialog.                         */
     /************************************************************************/
     key = MsgYorN(HELP_CTRL_BREAK);
     if( key == key_1 )
     {
      if( IsEspRemote() && IsExecuteFlag() )
      {
       DBG_QUE_ELEMENT Qelement;

       Qelement.pid = DbgGetProcessID();

       SendMsgToDbgQue(DBG_QMSG_CTRL_BREAK, &Qelement, sizeof(Qelement));
      }
      rc = XCPT_CONTINUE_EXECUTION;
     }
     else if( key == key_2 )
     {
      rc = XCPT_CONTINUE_SEARCH;
      DosUnsetExceptionHandler(y);
      CloseConnectSema4();
      ConnectClose( DEFAULT_HANDLE );
      CloseSerialMutex();

      /***********************************************************************/
      /* - Clean up the screen before we get out of here.                    */
      /***********************************************************************/
      {
       extern UINT VideoRows;
              CSR  DosCsr;

       /**********************************************************************/
       /* - Toggle alt-esc and ctrl-esc back on.                             */
       /**********************************************************************/
       if( IsHotKey() ) SetHotKey();

       ClrPhyScr( 0, VideoRows-1, vaClear );
       DosCsr.col = 0;
       DosCsr.row = ( uchar )(VideoRows-1);
       PutCsr( &DosCsr );
      }

     }
     else
      goto caseXCPT_SIGNAL_BREAK;
     break;

    case XCPT_SIGNAL_KILLPROC :
     {
      /***********************************************************************/
      /* - If we fall into this code, then the we've been sent a kill        */
      /*   process signal from an external process.                          */
      /*                                                                     */
      /* - Close the connection sema4 if there is one.                       */
      /* - Take this exception handler out of the chain.                     */
      /* - Close the connection.                                             */
      /* - Tell the que that the child is proceeing to termination.          */
      /* - Tell the operating system that we've handled the signal.          */
      /* - The operating system will interrupt the receive command           */
      /*   at which time, we will terminate the thread and the probe         */
      /*   process...unless we're using an async connection, then            */
      /*   all this bullshit doesn't work so just let the operating          */
      /*   system handle it.                                                 */
      /***********************************************************************/
      DosUnsetExceptionHandler(y);
      CloseConnectSema4();
      ConnectClose( DEFAULT_HANDLE );
      if( IsParent() == FALSE )
      {
       DBG_QUE_ELEMENT Qelement;

       memset( &Qelement, 0, sizeof(Qelement) );
       Qelement.pid = DbgGetProcessID();

       SendMsgToDbgQue( DBG_QMSG_CHILD_TERM, &Qelement, sizeof(Qelement) );
      }
      rc = XCPT_CONTINUE_SEARCH;
      /***********************************************************************/
      /* - Clean up the screen before we get out of here.                    */
      /***********************************************************************/
      {
       extern UINT VideoRows;
              CSR  DosCsr;

       /**********************************************************************/
       /* - Toggle alt-esc and ctrl-esc back on.                             */
       /**********************************************************************/
       if( IsHotKey() ) SetHotKey();

       ClrPhyScr( 0, VideoRows-1, vaClear );
       DosCsr.col = 0;
       DosCsr.row = ( uchar )(VideoRows-1);
       PutCsr( &DosCsr );
      }
     }
     break;
   }
   break;
 }
 return(rc);
}

/*****************************************************************************/
/* UnRegisterExceptionHandler()                                              */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*  Remove the exception handler.                                            */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
void UnRegisterExceptionHandler( void )
{
 DosUnsetExceptionHandler(preg_rec);
}

/*****************************************************************************/
/* BuildEspParms()                                                        919*/
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*  Build a block of flags and child process names information to send       */
/*  to esp.                                                                  */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*  pcmd         ->  to the command/invocation parms structure.              */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*  pEspRunOpts  ->  to the dynamic memory block built by this function.     */
/*  NULL         ==> error.                                                  */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*  pcmd->pChildProcesses -> to a filespec.                                  */
/*                                                                           */
/*  where the filespec is something like:                                    */
/*                                                                           */
/*   "@children.lst" specifies a file with a list of child process names.    */
/*   "child.exe"     specifies only a single child.                          */
/*                                                                           */
/*  If pcmd->pChildProcesses==NULL then debugging of all child processes     */
/*  is assumed.                                                              */
/*                                                                           */
/*****************************************************************************/
ESP_RUN_OPTS *BuildEspParms( CmdParms *pcmd )
{
 int    len;
 char  *cp;
 char  *pNameBlock;
 ULONG  NameBlockSize;
 int    NumOfProcs = 0;

 ESP_RUN_OPTS *pEspRunOpts = NULL;

 /****************************************************************************/
 /* At invocation time, we might have had the following type of invocation   */
 /* to specify child process debugging:                                      */
 /*                                                                          */
 /*   /b[@][filespec]                                                        */
 /*                                                                          */
 /* If we did then we have to build a name block so we can tell esp the      */
 /* names of the child processes to debug.                                   */
 /*                                                                          */
 /* We're going to build the following block of flags and names to send      */
 /* to esp.                                                                  */
 /*                                                                          */
 /*      ---  -----------------------                                        */
 /*       |  | 32 bits of flags      |                                       */
 /*       |   -----------------------                                        */
 /*       |  | 32 bit name block size|                                       */
 /*       |   -----------------------  ---                                   */
 /*       |  | child1.exe\0          |  |                                    */
 /*   len |  | child2.exe\0          |  |                                    */
 /*       |  |     .                 |  |                                    */
 /*       |  |     .                 |  | NameBlockSize                      */
 /*       |  |     .                 |  |                                    */
 /*       |  | childn.exe\0          |  |                                    */
 /*       |  | \0                    |  |                                    */
 /*      ---  -----------------------  ---                                   */
 /*                                                                          */
 /*   NameBlockSize = 0 ==> debug all child processes.                       */
 /*                                                                          */
 /****************************************************************************/

 cp            = pcmd->pChildProcesses;
 NameBlockSize = 0;
 pNameBlock    = NULL;

 if(cp != NULL)
 {
  /***************************************************************************/
  /* bump len by the length of the name block.                               */
  /***************************************************************************/
  if( *cp != '@' )
  {
   NameBlockSize = strlen(cp) + 2;
   pNameBlock    = Talloc(NameBlockSize);
   strcpy(pNameBlock,cp);
  }
  else
  {
   if( BuildChildProcessNamesBlock( cp+1,
                                    &pNameBlock,
                                    &NameBlockSize,
                                    &NumOfProcs  )
     )
    return(NULL);
  }
 }

 /****************************************************************************/
 /* - set a flag to indicate single/multiple child process debugging.        */
 /****************************************************************************/
 if( NumOfProcs > 1 )
  pcmd->DbgFlags.SingleMultiple = MULTIPLE;

 /****************************************************************************/
 /* now build the block and return a pointer to the caller.                  */
 /****************************************************************************/
 len = sizeof(ESP_RUN_OPTS) - 1 + NameBlockSize;

 pEspRunOpts                 = (ESP_RUN_OPTS*)Talloc(len);
 pEspRunOpts->NameBlockSize  = NameBlockSize;

 if( pNameBlock )
 {
  memcpy(&(pEspRunOpts->NameBlock),pNameBlock,NameBlockSize);
  Tfree(pNameBlock);
 }
 return(pEspRunOpts);
}

/*****************************************************************************/
/* BuildChildProcessNamesBlock()                                          919*/
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*  Read a user specified response file to get a list of names of            */
/*  child processes that we want to debug. The names block is                */
/*  as a string of strings.                                                  */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*  pResponseFileName  -> to file name(s) we're going to read.               */
/*  pNameBlock         -> to receiver of a pointer to the name block.        */
/*  pNameBlockSize     -> to receiver of a the name block size.              */
/*  pNumOfProcs        -> to receiver of a the number of processes.          */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*  rc = 0 ==>success.                                                       */
/*       1 ==>failure.                                                       */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/* - The file is small.                                                      */
/* - The file is in the current directory or the user has provided an        */
/*   explicit filespec.                                                      */
/* - The file is a text file.                                                */
/* - There is only one exe name per line. EXE extension is not required.     */
/*                                                                           */
/*****************************************************************************/
int BuildChildProcessNamesBlock( char  *pResponseFileName ,
                                 char **pNameBlock,
                                 ULONG *pNameBlockSize,
                                 int   *pNumOfProcs)
{
 FILE  *fp;
 int    len;
 char   line[CCHMAXPATH];
 char  *cp;
 char  *cpp;
 int    n;

 /****************************************************************************/
 /* open the response file and return error if it won't open.                */
 /****************************************************************************/
 if( !( fp = fopen(pResponseFileName, "rb" ) ) )
  return(1);

 /****************************************************************************/
 /* - scan the file to get the size of the block needed to hold the names.   */
 /****************************************************************************/
 len = 0;
 for(;;)
 {
  char *lp;

  /***************************************************************************/
  /* - fgets reads a line from a file.                                       */
  /***************************************************************************/
  lp = fgets(line, sizeof(line), fp);
  if( (lp == NULL) )
  {
   /**************************************************************************/
   /* - A null return indicates an error or an end-of-file condition.  A     */
   /*   null will be returned for a file that does not have an end-of-file   */
   /*   character.                                                           */
   /**************************************************************************/
   if( feof(fp) != 0 )
    break;
   else
    return(1);
  }
  else
  {
   /**************************************************************************/
   /* - lp may be pointing to the end-of-file character. Check for end-of-   */
   /*   file for a file with an end-of_file mark.                            */
   /**************************************************************************/
   if( feof(fp) != 0 )
    break;
  }

  /***************************************************************************/
  /* - check for a binary file and treat as an error.                        */
  /***************************************************************************/
  if( checkline(lp) )
   return(1);

  /***************************************************************************/
  /* - toss blank lines                                                      */
  /***************************************************************************/
  cpp=strrchr(line,'\n');if(cpp) *cpp = '\0';
  cpp=strrchr(line,'\r');if(cpp) *cpp = '\0';
  n = strlen(line); if( n==0 ) continue;
  len += n + 1;
 }
 len += 1;                              /* add one for the ending '\0'       */

 /****************************************************************************/
 /* give the block length and pointer to the caller.                         */
 /****************************************************************************/
 *pNameBlockSize = len;
 *pNameBlock     = cp = Talloc(len);

 /****************************************************************************/
 /* now build the following block of names.                                  */
 /*                                                                          */
 /*                 -----------------------  ---                             */
 /*  pNameBlock--->| child1.exe\0          |  |                              */
 /*                | child2.exe\0          |  |                              */
 /*                |     .                 |  |                              */
 /*                |     .                 |  | NameBlockSize                */
 /*                |     .                 |  |                              */
 /*                | childn.exe\0          |  |                              */
 /*                | \0                    |  |                              */
 /*                 -----------------------  ---                             */
 /*                                                                          */
 /* The ending '\0' comes from initializing the allocated block to 0s.       */
 /* Blank lines will be tossed.                                              */
 /****************************************************************************/
 fseek(fp,0L,SEEK_SET);
 *pNumOfProcs = 0;
 for(;;)
 {
  fgets(line,sizeof(line),fp);
  if(feof(fp))
   break;
  cpp=strrchr(line,'\n');if(cpp) *cpp = '\0';
  cpp=strrchr(line,'\r');if(cpp) *cpp = '\0';
  if( strlen(line) == 0 ) continue;
  strcpy( cp,strupr(line) );
  cp  += strlen(cp) + 1;
  *pNumOfProcs += 1;
 }
 fclose(fp);
 return(0);
}

/*****************************************************************************/
/* CheckLine()                                                            919*/
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*  Verify a text line.                                                      */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*  pLine         -> to the line to be verified.                             */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*  rc = 0 ==>ok.                                                            */
/*       1 ==>must be binary.                                                */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/* - Text lines consist of characters >= 0x32 and < 0x80 and may contain     */
/*   crs and lfs.                                                            */
/* - The line is null terminated.                                            */
/*                                                                           */
/*****************************************************************************/
int checkline(char *pLine )
{
 char *cp = pLine;

 while(*cp != '\0')
 {
  if( ( (0x20 <= *cp)&&(*cp < 0x80) ) ||
        (*cp == 0x0D)                 ||
        (*cp == 0x0A)                 ||
        (*cp == 0x1A)
    )
   cp++;
  else
   return(1);
 }
 return(0);
}



void MyExit( void )
{
 DBG_QUE_ELEMENT  Qelement;

 if( IsParent() == FALSE )
 {
  Qelement.pid = (ULONG)DbgGetProcessID();
  SendMsgToDbgQue( DBG_QMSG_SELECT_SESSION, &Qelement, sizeof(Qelement) );
  DosSleep(15000);
 }
 exit(0);
}

/*****************************************************************************/
/* GetProcessType()                                                          */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Get the process type of this debugger.                                  */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   ptype    -> to the receiver of the type.                                */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   rc                                                                      */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   This function uses the undocumented DosQProcStatus() call so be         */
/*   wary of different behavior for different versions of the operating      */
/*   system. The documentation that we have at this time is unofficial       */
/*   but should be pretty close to what eventually gets documented.          */
/*   (At this time, there are plans to document the call.)                   */
/*                                                                           */
/*   We use a stack buffer of 64K as suggested by the DosQProcStatus()       */
/*   doc.                                                                    */
/*                                                                           */
/*****************************************************************************/
#define BUFFER_SIZE 64*1024-1
APIRET GetProcessType( ULONG *ptype )
{
 void           *pProcStatBuf;
 ULONG           flags;
 TIB            *pTib;
 PIB            *pPib;
 USHORT          pid;
 qsPrec_t       *pProcRec;              /* ptr to process record section    */
 qsS16Headrec_t *p16SemRec;             /* ptr to 16 bit sem section        */

 /****************************************************************************/
 /* - First, we have to know the pid.                                        */
 /****************************************************************************/
 if( DosGetInfoBlocks(&pTib,&pPib) )
  return(1);

 pid = (USHORT)pPib->pib_ulpid;

 /****************************************************************************/
 /* - Now get the type.                                                      */
 /****************************************************************************/
 pProcRec  = NULL;
 p16SemRec = NULL;

 /****************************************************************************/
 /* - Allocate a 64k buffer. This is the recommended size since a large      */
 /*   system may generate this much. It's allocated on a 64k boundary        */
 /*   because DosQprocStatus() is a 16 bit call and we don't want the        */
 /*   buffer to overlap a 64k boundary.                                      */
 /****************************************************************************/
 flags = PAG_COMMIT|PAG_READ|PAG_WRITE|OBJ_TILE;
 if( DosAllocMem( &pProcStatBuf,BUFFER_SIZE,flags) ||
     DosQProcStatus( (ULONG*)pProcStatBuf , BUFFER_SIZE )
   )
  return(1);

 /****************************************************************************/
 /* Define a pointer to the process subsection of information.               */
 /****************************************************************************/
 pProcRec   = (qsPrec_t       *)((qsPtrRec_t*)pProcStatBuf)->pProcRec;
 p16SemRec  = (qsS16Headrec_t *)((qsPtrRec_t*)pProcStatBuf)->p16SemRec;

 /****************************************************************************/
 /* - scan to the proc record for the pid.                                   */
 /****************************************************************************/
 for( ;pProcRec->pid != pid; )
 {
  /***************************************************************************/
  /* Get a pointer to the next process block and test for past end of block. */
  /***************************************************************************/
  pProcRec = (qsPrec_t *)( (char*)(pProcRec->pThrdRec) +
                                  (pProcRec->cTCB)*sizeof(qsTrec_t));

  if((void*)pProcRec >= (void*)p16SemRec )
  {
   pProcRec = NULL;
   break;
  }
 }

 if(pProcRec == NULL )
  return(1);

 /****************************************************************************/
 /* - give the type back to the caller.                                      */
 /****************************************************************************/
 *ptype = pProcRec->type & 0x7;
 return(0);
}
