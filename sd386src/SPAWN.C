/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   spawn.c                                                                 */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*  Spawn another copy of dbg/esp.                                           */
/*                                                                           */
/* History:                                                                  */
/*                                                                           */
/*   07/20/94 Created.                                                       */
/*                                                                           */
/*****************************************************************************/

#include "all.h"

/*****************************************************************************/
/* SpawnProbe()                                                              */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*  Start up another copy of esp in response to a new process                */
/*  notification.                                                            */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*  pEspPid    -> receiver of the process id of the probe.                   */
/*  pEspSid    -> receiver of the session id of the probe.                   */
/*  pQueName  -> the dbg que name.                                           */
/*  MsgLen     length of the buffer to receive offending mod if error.       */
/*  pMsgBuf    -> buffer to receive offending module name.                   */
/*  SpawnFlags some flags affecting how the probe is to be spawned.          */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*  rc        DosStartSession() return code.                                 */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
APIRET SpawnProbe( USHORT          *pEspPid,
                   ULONG           *pEspSid,
                   char            *pQueName,
                   ULONG            MsgLen,
                   char            *pMsgBuf,
                   ESP_SPAWN_FLAGS  SpawnFlags)

{
 APIRET      rc;
 STARTDATA   sd;
 ULONG       sid;
 ULONG       pid;
 TIB       *pTib;
 PIB       *pPib;
 char       parms[512];
 char       PgmName[CCHMAXPATH];
 int        PgmNameLen;
 char       child[20];
 char       shr[20];
 char       heap[20];
 char       handle[20];
 char       descriptor[20];
 char       qname[32];
 char      *PgmArgs;

 memset( parms, 0, sizeof(parms) );

 if( SpawnFlags.SpawnOrigin == ESP_SPAWN_FROM_DBG )
 {
  char       esp[]  = "esp.exe";
  char       opts[] = " /b ";

  /***************************************************************************/
  /* - If we're spawning the probe from the debugger when using local        */
  /*   pipes for child and multiple processes, then fall in here.            */
  /***************************************************************************/
  strcpy( PgmName, esp );
  strcpy( parms,   opts );

  if( UseDebug() == TRUE )
  {
   strcpy(parms,   esp  );
   strcat(parms,   opts );
   strcpy(PgmName, "sd386.exe");
  }

 }
 else
 {
  /***************************************************************************/
  /* - get the program name using the mte from DosGetInfoBlocks() and        */
  /*   passing it to DosQueryModuleName().                                   */
  /* - get the arguments from DosGetInfoBlocks().                            */
  /*                                                                         */
  /*    Call DosGetInfoBlocks() to get a pointer to command line invocation  */
  /*    We will get a pointer to a block formatted like so:                  */
  /*                                                                         */
  /*      string1\0string2\0\0.                                              */
  /*                                                                         */
  /*      where:                                                             */
  /*                                                                         */
  /*        string1=fully qualified invocation.                              */
  /*        string2=user parms.                                              */
  /*                                                                         */
  /***************************************************************************/


  /***************************************************************************/
  /* - Get the exact invocation of the parent probe( this one ) which        */
  /*   includes the fully qualified filespec. We want to use this so that    */
  /*   we invoke a child which is the same as the parent.                    */
  /* - Also get a copy of the parent's invocation options. The child will    */
  /*   inherit these from the parent and will use any that are appropriate.  */
  /***************************************************************************/
  DosGetInfoBlocks(&pTib,&pPib);
  PgmNameLen = sizeof(PgmName);
  memset(PgmName,' ', PgmNameLen );
  DosQueryModuleName(pPib->pib_hmte, PgmNameLen, PgmName );

  /***************************************************************************/
  /* - Get a copy of the parent's arguments.                                 */
  /***************************************************************************/
  PgmArgs = pPib->pib_pchcmd + strlen(pPib->pib_pchcmd) + 1;

  /***************************************************************************/
  /* - At this point, we have the program name and invocation parms.         */
  /* - Now, we will define some additional invocation parameters:            */
  /*                                                                         */
  /*   - /child=xxxxx  where child  = child debugger and                     */
  /*                         xxxxx  = child pid (for serial connections only.*/
  /*                                                                         */
  /*                   For parallel connections, the only thing we care      */
  /*                   about is the "child" part of the parameter.  For      */
  /*                   serial connections, the xxxxx value appended after    */
  /*                   the "=" is used.                                      */
  /*                                                                         */
  /*   - /handle=xxxxx where handle = switch for com handle( serial only )   */
  /*                         xxxxx  = parent's com handle - inherited by     */
  /*                                  the child.                             */
  /*                                                                         */
  /*                   For serial connections, the child has to inherit the  */
  /*                   handle from the parent.                               */
  /*                                                                         */
  /*   - /shr=xxxxx    where shr    = switch for shared heap memory handle.  */
  /*                         xxxxx  = shared heap memory handle.             */
  /*                                                                         */
  /*   - /heap=xxxxx   where heap   = switch for shared heap start address.  */
  /*                         xxxxx  = shared heap start address.             */
  /*                                                                         */
  /*   - /qname=xxxxx where  qname  = switch for dbg queue name.             */
  /*                         xxxxx  = dbg queue name.                        */
  /*                                                                         */
  /***************************************************************************/
  memset(&child, 0, sizeof(child));
  strcpy(child," /child=");

  memset(&shr, 0, sizeof(shr));
  strcpy(shr," /shr=");
  sprintf(shr + strlen(shr),"%-lu ", GetShrMem() );

  memset(&heap, 0, sizeof(heap));
  strcpy(heap," /heap=");
  sprintf(heap + strlen(heap),"%-lu ", GetAllpids() );

  if( (SerialParallel() == SERIAL) )
  {
   memset(&handle, 0, sizeof(handle));
   strcpy(handle," /handle=");
   sprintf(handle + strlen(handle),"%-lu ",GetComHandle() );
  }

  if( ConnectType() == SOCKET )
  {
   memset(&descriptor, 0, sizeof(descriptor));
   strcpy(descriptor," /descriptor=");
   sprintf(descriptor + strlen(descriptor),"%-lu ", SockGetDescriptor() );
  }

  memset(&qname, 0, sizeof(qname));
  strcpy(qname," /qname=");
  sprintf(qname + strlen(qname),"%s ", pQueName );

  if( UseDebug() == TRUE )
  {
   strcat(parms,PgmName);
   strcpy( PgmName, "sd386.exe");
  }

  /***************************************************************************/
  /* - Now, build the parameter string to pass to the child probe.           */
  /* - The child parameter MUST be first.                                    */
  /***************************************************************************/
  strcat(parms,child);
  if( SerialParallel() == SERIAL )
   strcat(parms,handle);
  if( ConnectType() == LOCAL_PIPE )
   strcat(parms, " /b ");
  if( ConnectType() == SOCKET )
   strcat(parms,descriptor);
  strcat(parms,shr);
  strcat(parms,heap);
  strcat(parms,qname);
  strcat(parms,PgmArgs);
 }

 if( SpawnFlags.SpawnMethod == ESP_USE_DOSEXECPGM )
 {
  /***************************************************************************/
  /* - If the user specified on the invocation line that the parent          */
  /*   session was to be started using DosExecPgm, then we assume that       */
  /*   all child probes are to be started using DosExecPgm() as well.        */
  /***************************************************************************/
  char         ExecPgmArgs[512];
  RESULTCODES  result;
  ULONG        ExecFlags;

  /***************************************************************************/
  /* - build the program name and program arguments and call DosExecPgm.     */
  /***************************************************************************/
  memset( ExecPgmArgs, 0, sizeof(ExecPgmArgs) );
  strcpy( ExecPgmArgs, PgmName);
  strcpy( ExecPgmArgs + strlen(PgmName) + 1, parms );

  ExecFlags = EXEC_ASYNC;
  rc = DosExecPgm( pMsgBuf,
                   MsgLen,
                   ExecFlags,
                   ExecPgmArgs,
                   NULL,
                   &result,
                   ExecPgmArgs);

  sid = 0;
  pid = result.codeTerminate;
 }
 else /* use DosStartSession() */
 {
  ULONG PgmControl;

  /***************************************************************************/
  /* - Start the child probe. The error message buffer was cleared by caller.*/
  /***************************************************************************/
  memset( &sd, 0, sizeof(sd) );

  PgmControl = SSF_CONTROL_VISIBLE;
  if( SpawnFlags.Visible == ESP_INVISIBLE )
   PgmControl = SSF_CONTROL_INVISIBLE;

  PgmControl |= SSF_CONTROL_MINIMIZE;

  sd.Length        = sizeof(sd);
  sd.Related       = SSF_RELATED_CHILD;
  sd.FgBg          = SSF_FGBG_BACK;
  sd.TraceOpt      = SSF_TRACEOPT_NONE;
  sd.InheritOpt    = SSF_INHERTOPT_PARENT;
  sd.PgmName       = PgmName;
  sd.PgmInputs     = parms;
  sd.SessionType   = SSF_TYPE_WINDOWABLEVIO;
  sd.PgmTitle      = "Esp";
  sd.PgmControl    = PgmControl;
  sd.ObjectBuffer  = pMsgBuf;
  sd.ObjectBuffLen = MsgLen;

  rc = DosStartSession( ( PSTARTDATA )&sd, &sid, &pid );
 }

 if( rc != 0 )
 {;
  /***************************************************************************/
  /* - If there's an error, then the message buffer will have been filled    */
  /*   in for the caller.                                                    */
  /* - Give the probe access to the shared heap.                             */
  /***************************************************************************/
 }
 else
 {
  *pEspPid = (USHORT)pid;
  *pEspSid = sid;
  DosGiveSharedMem( GetShrMem(), pid, PAG_READ | PAG_WRITE );
 }
 return(rc);
}

#ifdef __DBG__
/*****************************************************************************/
/* SpawnDbg()                                                                */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*  Start up another copy of the debugger in response to a new process       */
/*  notification.                                                            */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*  ChildPid  the process id of the child we're going to debug.              */
/*  pDbgPid   -> receiver of the process id of the child debugger.           */
/*  pDbgSid   -> receiver of the session id of the child debugger.           */
/*  pQueName  -> the dbg que name.                                           */
/*  MsgLen    length of the buffer to receive offending mod if error.        */
/*  pMsgBuf   -> buffer to receive offending module name.                    */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*  rc        DosStartSession() return code.                                 */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
APIRET SpawnDbg( USHORT  ChildPid,
                 USHORT *pDbgPid,
                 ULONG  *pDbgSid,
                 char   *pQueName,
                 ULONG   MsgLen,
                 char   *pMsgBuf )
{
 APIRET     rc;
 TIB       *pTib;
 PIB       *pPib;
 STARTDATA  sd;
 ULONG      pid;
 ULONG      sid;
 char       parms[512];
 char       child[20];
 char       shr[20];
 char       heap[20];
 char       PgmName[CCHMAXPATH];
 int        PgmNameLen;
 char      *PgmArgs;
 char       handle[20];
 char       qname[32];
 char       PgmTitle[12] = "Dbg-";
 ULONG      SessionType;

 /****************************************************************************/
 /* - Get the exact invocation of the parent debugger( this one ) which      */
 /*   includes the fully qualified filespec. We want to use this so that     */
 /*   we invoke a child which is the same as the parent.                     */
 /* - Also get a copy of the parent's invocation options. The child will     */
 /*   inherit these from the parent and will use any that are appropriate.   */
 /****************************************************************************/
 DosGetInfoBlocks(&pTib,&pPib);
 PgmNameLen = sizeof(PgmName);
 memset(PgmName,' ', PgmNameLen );
 rc = DosQueryModuleName(pPib->pib_hmte, PgmNameLen, PgmName );
 if( rc )
  return( rc );

 /****************************************************************************/
 /* - Get the type of the parent debugger. All child debuggers will inherit  */
 /*   this.                                                                  */
 /****************************************************************************/
 SessionType = SSF_TYPE_WINDOWABLEVIO;
 {
  ULONG    MyType;

  if( (GetProcessType(&MyType) == 0) &&
      (MyType != SSF_TYPE_WINDOWABLEVIO)
    )
   SessionType = SSF_TYPE_FULLSCREEN;
 }

 /****************************************************************************/
 /* - Get a copy of the parent's arguments.                                  */
 /****************************************************************************/
 PgmArgs = pPib->pib_pchcmd + strlen(pPib->pib_pchcmd) + 1;

 /****************************************************************************/
 /* - Now, we will define some additional invocation parameters:             */
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
 memset(&child,0,sizeof(child));
 strcpy(child," /child=");
 sprintf(child + strlen(child),"%-hu ",ChildPid);

 memset(&shr, 0, sizeof(shr));
 strcpy(shr," /shr=");
 sprintf(shr + strlen(shr),"%-lu ", GetShrMem() );

 memset(&heap, 0, sizeof(heap));
 strcpy(heap," /heap=");
 sprintf(heap + strlen(heap),"%-lu ", GetAllpids() );

 if( (SerialParallel() == SERIAL) )
 {
  memset(&handle, 0, sizeof(handle));
  strcpy(handle," /handle=");
  sprintf(handle + strlen(handle),"%-lu ",GetComHandle() );
 }

 memset(&qname, 0, sizeof(qname));
 strcpy(qname," /qname=");
 sprintf(qname + strlen(qname),"%s ", pQueName );

 /****************************************************************************/
 /* - Build the invocation string. It is REQUIRED that "/child=xxxxx" be     */
 /*   the first parameter. The child debugger will key off of it.            */
 /****************************************************************************/
 memset( parms, 0, sizeof(parms) );
 if( UseDebug() == TRUE )
 {
  strcat(parms,PgmName);
  strcpy( PgmName, "d:\\sd386305\\sd.exe");
 }

 strcat(parms,child);
 if( (SerialParallel() == SERIAL) )
  strcat(parms,handle);
 strcat(parms,shr);
 strcat(parms,heap);
 strcat(parms,qname);
 strcat(parms,PgmArgs);

 /****************************************************************************/
 /* - Build a program title base on the pid of the child to be debugged.     */
 /****************************************************************************/
 sprintf(PgmTitle + strlen(PgmTitle),"%-hu ", ChildPid );

 /****************************************************************************/
 /* - spawn the child debugger.                                              */
 /****************************************************************************/
 memset( &sd, 0, sizeof(sd) );
 sd.Length        = sizeof(sd);
 sd.Related       = SSF_RELATED_CHILD;
 sd.FgBg          = SSF_FGBG_FORE;
 sd.TraceOpt      = SSF_TRACEOPT_NONE;
 sd.InheritOpt    = SSF_INHERTOPT_PARENT;
 sd.PgmName       = PgmName;
 sd.PgmInputs     = parms;
 sd.SessionType   = SessionType;
 sd.PgmTitle      = PgmTitle;
 sd.PgmControl    = SSF_CONTROL_VISIBLE;
 sd.ObjectBuffer  = pMsgBuf;
 sd.ObjectBuffLen = MsgLen;

 rc = DosStartSession( ( PSTARTDATA )&sd, &sid, &pid );
 if( (rc == 0) || (rc == ERROR_SMG_START_IN_BACKGROUND) )
 {
  /***************************************************************************/
  /* - Give the session id of the child debugger back to the caller.         */
  /* - Give the child debugger access to the shared heap.                    */
  /***************************************************************************/
  *pDbgSid = sid;
  *pDbgPid = pid;
  DosGiveSharedMem( GetShrMem(), pid, PAG_READ | PAG_WRITE );
  rc = 0;
 }
 else
 {;
  /***************************************************************************/
  /* - If there's an error, then the message buffer will have been filled    */
  /*   in for the caller.                                                    */
  /***************************************************************************/
 }
 return(rc);
}
#endif
