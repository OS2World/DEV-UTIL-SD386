/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   espstart.c                                                              */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Start the debuggee.                                                     */
/*                                                                           */
/* History:                                                                  */
/*                                                                           */
/*   06/25/93 Created.                                                       */
/*                                                                           */
/*****************************************************************************/
#include "all.h"

#ifdef __ESP__

 static HEV Wait4ChildSema4;

#endif

/*****************************************************************************/
/* - Process id and session id get/set for the parent debuggee.              */
/*****************************************************************************/
static ULONG ParentProcessID;
static ULONG ParentSessionID;

static ULONG TerminateSessionID;

void  SetParentSessionID( ULONG sid ) { ParentSessionID = sid; }
void  SetParentProcessID( ULONG pid ) { ParentProcessID = pid; }

ULONG GetEspParentProcessID() { return(ParentProcessID);   }
ULONG GetEspParentSessionID() { return(ParentSessionID);   }

ULONG GetTerminateSessionID() { return(TerminateSessionID);   }

/*****************************************************************************/
/* EspStartUser()                                                            */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Start the user's application.                                           */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   pUserEXE       -> fully qualified debuggee specification.               */
/*   pUserParms     -> debuggee parms.                                       */
/*   SessionType       user specified session type.                          */
/*   pSessionID        where to put the SessionID.                           */
/*   pProcessID        where to put the ProcessID.                           */
/*   pProcessType      where to put the type( PM, VIO, etc. ) of the process.*/
/*   MsgLen            length of the buffer to receive offending mod if error*/
/*   pMsgBuf        -> buffer to receive offending module name.              */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   rc         return code                                                  */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
APIRET EspStartUser(char   *pUserEXE,
                    char   *pUserParms,
                    USHORT  SessionType,
                    ULONG  *pSessionID,
                    ULONG  *pProcessID,
                    ULONG  *pProcessType,
                    ULONG   MsgLen,
                    char   *pMsgBuf )
{
 STARTDATA    sd;
 APIRET       rc;
 char        *cp;
 char         TitleName[35];

 /****************************************************************************/
 /* - now start up the parent.                                               */
 /****************************************************************************/
 if( UseExecPgm() )
 {
  char         ExecPgmArgs[512];
  RESULTCODES  result;
  ULONG        ExecFlags;

  /***************************************************************************/
  /* - build the program name and program arguments and call DosExecPgm.     */
  /***************************************************************************/
  memset( ExecPgmArgs, 0, sizeof(ExecPgmArgs) );
  strcpy( ExecPgmArgs, pUserEXE);
  if( pUserParms != NULL )
   strcpy( ExecPgmArgs + strlen(pUserEXE) + 1, pUserParms );

  ExecFlags = EXEC_TRACE;

#ifdef __ESP__
/*---------------------------------------------------------------------------*/
  if( DebugChild() == TRUE )
  {
   ExecFlags = EXEC_ASYNCRESULTDB;
   DosCreateEventSem(NULL,&Wait4ChildSema4,0L,FALSE);
  }
/*---------------------------------------------------------------------------*/
#endif

  rc = DosExecPgm( pMsgBuf,
                   MsgLen,
                   ExecFlags,
                   ExecPgmArgs,
                   NULL,
                   &result,
                   ExecPgmArgs);
  if( rc != 0 )
   return(rc);

  ParentProcessID = result.codeTerminate;
 }
 else /* Use DosStartSession */
 {
  /***************************************************************************/
  /* - Build the data structure for DosStartSession.                         */
  /* - Note: these are the applicable session types.                         */
  /*     0 = not specified                                                   */
  /*     1 = NOTWINDOWCOMPAT(full scrn)                                      */
  /*     2 = WINDOWCOMPAT                                                    */
  /*     3 = WINDOWAPI ( PM )                                                */
  /***************************************************************************/
   memset( &sd, 0, sizeof(sd) );
   sd.Length        = sizeof(sd);
   sd.Related       = SSF_RELATED_CHILD;
   sd.FgBg          = SSF_FGBG_BACK;
   sd.TraceOpt      = SSF_TRACEOPT_TRACE;
   sd.InheritOpt    = SSF_INHERTOPT_PARENT;
   sd.PgmName       = pUserEXE;
   sd.PgmInputs     = pUserParms;
   sd.SessionType   = SessionType;
   sd.ObjectBuffLen = MsgLen;
   sd.ObjectBuffer  = pMsgBuf;

#ifdef __ESP__
/*---------------------------------------------------------------------------*/
   /**************************************************************************/
   /* - if we're debugging child processes then attach the termination       */
   /*   queue and tell DosStartSession() that we want to debug               */
   /*   all descendants.                                                     */
   /* - create a sema4 with an initial state of "set" that will be posted    */
   /*   when the first(or only) child is started.                            */
   /**************************************************************************/
   if( DebugChild() )
   {
    sd.TraceOpt = SSF_TRACEOPT_TRACEALL;
    sd.TermQ    = GetEspQue();
    DosCreateEventSem(NULL,&Wait4ChildSema4,0L,FALSE);
   }
/*---------------------------------------------------------------------------*/
#endif

   cp = strrchr( pUserEXE,0x5C );
   sprintf( TitleName , "SD386 App %s" , ++cp );

   sd.PgmTitle = TitleName;

   rc = DosStartSession( (PSTARTDATA)&sd,
                          &ParentSessionID,
                          &ParentProcessID);

   if(rc != 0)
    return(rc);
 }

 *pSessionID = TerminateSessionID = ParentSessionID;
 *pProcessID = ParentProcessID;

 {
  ULONG type;

  /***************************************************************************/
  /* - Get the type of the application.                                      */
  /* - DosQueryAppType puts the app type in bits 0,1,2.                      */
  /***************************************************************************/
  DosQueryAppType( pUserEXE, &type );
  *pProcessType = type & 0x7;
 }

#ifdef __ESP__
/*---------------------------------------------------------------------------*/
 {
  ALLPIDS *p;
  TIB     *pTib;
  PIB     *pPib;

  /***************************************************************************/
  /* - Add the pid structure for the DosStartSession'd process.              */
  /***************************************************************************/
  AddPid( ParentProcessID, ParentSessionID, 0, 0, NULL );

  if( DebugChild() == TRUE )
  {
    /*************************************************************************/
    /* - We need to bring the user's session to the foreground in this       */
    /*   case, else he may not be able to do whatever he needs to do         */
    /*   to give control to the debugger.                                    */
    /*************************************************************************/
    if( ConnectType() == LOCAL_PIPE )
     DosSelectSession(ParentSessionID);

   /***************************************************************************/
   /* - Send a message to the queue for the parent process.                   */
   /* - Wait for the child to be started.                                     */
   /* - The queue will update the ParentProcessID and the ParentSessionID.    */
   /***************************************************************************/
   SendNewProcessToQue( ParentProcessID );
   DosWaitEventSem(Wait4ChildSema4,SEM_INDEFINITE_WAIT);
   DosCloseEventSem(Wait4ChildSema4);

   *pSessionID = ParentSessionID;
   *pProcessID = ParentProcessID;
  }

  /***************************************************************************/
  /* - Put the process id and the session id into the pid structure.         */
  /* - Put the process id of the probe into the pid structure.               */
  /***************************************************************************/
  p      = GetPid( ParentProcessID );
  p->sid = ParentSessionID;

  DosGetInfoBlocks(&pTib,&pPib);

  p->EspPid = (USHORT)pPib->pib_ulpid;

  /***************************************************************************/
  /* - Create the connect sema4 for the parent probe.                        */
  /***************************************************************************/
  if( SerialParallel() == SERIAL )
   CreateConnectSema4( p->EspPid, _ESP );
 }
/*---------------------------------------------------------------------------*/
#endif

 return(rc);
}

#ifdef __ESP__
/*---------------------------------------------------------------------------*/
/*****************************************************************************/
/* EspSetRunOpts()                                                           */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Read a block of esp flags and child/multiple process debugging          */
/*   names. This is only called when the probe is running remote.            */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   pOpts          ->to esp opts.                                           */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
static ESP_RUN_OPTS *pEspRunOpts = NULL;
void EspSetEspRunOpts( ESP_RUN_OPTS *pOpts)
{
 int len;
 ESP_PARMS *pEspParms = GetEspParms();


 len          = sizeof(ESP_RUN_OPTS) + pOpts->NameBlockSize - 1;
 pEspRunOpts  = (ESP_RUN_OPTS*)Talloc(len);

 memcpy(pEspRunOpts,pOpts,len);

 /***************************************************************************/
 /* - Add any flags that need to go to the probe.                           */
 /***************************************************************************/
 pEspParms->EspFlags.UseExecPgm     = pEspRunOpts->EspFlags.UseExecPgm;
 pEspParms->EspFlags.DebugChild     = pEspRunOpts->EspFlags.DebugChild;
 pEspParms->EspFlags.DosDebugTrace  = pEspRunOpts->EspFlags.DosDebugTrace;
 pEspParms->EspFlags.SingleMultiple = pEspRunOpts->EspFlags.SingleMultiple;

}

/*****************************************************************************/
/* - This function posts for the connect sema4.                              */
/*****************************************************************************/
void PostWait4ChildSema4( void )
{
 DosPostEventSem(Wait4ChildSema4);
}

/*****************************************************************************/
/* IsDebug()                                                                 */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Test a pid to see if this is the name of a process that we want         */
/*   to debug. If it is, then also return the screen group.                  */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   pid         new process pid.                                            */
/*   psid        -> receiver of the screen group.                            */
/*   ptype       -> receiver of the process type.                            */
/*   pmte        -> receiver of the module handle.                           */
/*   pModuleName ->to the exe name for the new process pid.                  */
/*               NULL ==> use DosQprocStatus() to get name from pid.         */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   TRUE or FALSE                                                           */
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
BOOL IsDebug( char *pModuleName,
              USHORT pid,
              ULONG *psid,
              ULONG *ptype,
              USHORT *pmte)
{
 char      ModuleName[CCHMAXPATH];
 char      ExeName[CCHMAXPATH];
 char     *pexe;
 char     *cp;
 void     *pProcStatBuf;
 ULONG     flags;

 *psid = 0;
 if( pModuleName == NULL )
 {
  qsPrec_t        *pProcRec  = NULL;     /* ptr to process record section    */
  qsS16Headrec_t  *p16SemRec = NULL;     /* ptr to 16 bit sem section        */

  /***************************************************************************/
  /* - Allocate a 64k buffer. This is the recommended size since a large     */
  /*   system may generate this much. It's allocated on a 64k boundary       */
  /*   because DosQprocStatus() is a 16 bit call and we don't want the       */
  /*   buffer to overlap a 64k boundary.                                     */
  /***************************************************************************/
  flags = PAG_COMMIT|PAG_READ|PAG_WRITE|OBJ_TILE;
  if( DosAllocMem( &pProcStatBuf,BUFFER_SIZE,flags) ||
      DosQProcStatus( (ULONG*)pProcStatBuf , BUFFER_SIZE )
    )
   return(FALSE);

  /***************************************************************************/
  /* Define a pointer to the process subsection of information.              */
  /***************************************************************************/
  pProcRec   = (qsPrec_t       *)((qsPtrRec_t*)pProcStatBuf)->pProcRec;
  p16SemRec  = (qsS16Headrec_t *)((qsPtrRec_t*)pProcStatBuf)->p16SemRec;

  /***************************************************************************/
  /* - scan to the proc record for the pid.                                  */
  /***************************************************************************/
  for( ;pProcRec->pid != pid; )
  {
   /**************************************************************************/
   /* Get a pointer to the next process block and test for past end of block. /
   /**************************************************************************/
   pProcRec = (qsPrec_t *)( (char*)(pProcRec->pThrdRec) +
                                   (pProcRec->cTCB)*sizeof(qsTrec_t));

   if((void*)pProcRec >= (void*)p16SemRec )
   {
    pProcRec = NULL;
    break;
   }
  }

  if(pProcRec == NULL )
   return(FALSE);

  /***************************************************************************/
  /* - give the session id for this process back to the caller.              */
  /* - get the module name associated with this pid.                         */
  /* - scan the block of names checking to see if this is one of             */
  /*   the child processes that we want to debug.                            */
  /* - the names may be in one of the following formats:                     */
  /*                                                                         */
  /*      1. case1                                                           */
  /*      2. case1.exe                                                       */
  /*      3. d:\path1\path2\case1                                            */
  /*      4. d:\path1\path2\case1.exe                                        */
  /*                                                                         */
  /* - copy a name from the name block into a local buffer.                  */
  /* - append a .exe if needed.                                              */
  /* - if the exe name contains a path then perform an explicit compare.     */
  /* - if the exe name does not contain a path, then only compare it to      */
  /*   name part of the module name.                                         */
  /*                                                                         */
  /***************************************************************************/
  *psid  = pProcRec->sgid;
  *ptype = pProcRec->type & 0x7;
  *pmte  = pProcRec->hMte;
  memset(ModuleName,' ',sizeof(ModuleName) );
  if( DosQueryModuleName(pProcRec->hMte,sizeof(ModuleName),ModuleName) )
   return(FALSE);
 }
 else
 {
  strcpy(ModuleName,pModuleName);
 }

 /****************************************************************************/
 /* - compare the module name with the response file names.                  */
 /****************************************************************************/
 for( pexe = &pEspRunOpts->NameBlock; *pexe != '\0'; pexe += strlen(pexe)+1 )
 {
  strcpy(ExeName,pexe);
  if( strstr( ExeName , ".EXE") == NULL)
   strcat( ExeName,".EXE");
  if( strrchr(ExeName,'\\') || strrchr(ExeName,'/') )
   cp = ModuleName;
  else
  {
   cp  = strrchr(ModuleName,'\\');
   cp += 1;
  }
  if( stricmp(cp,ExeName) == 0 )
  {
   return(TRUE);
  }
 }
 return(FALSE);
}

/*****************************************************************************/
/* GetSessionID()                                                            */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Wrapper around IsDebug() function to get a session id.                  */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   pid         process pid.                                                */
/*   psid        receiver of the screen group.                               */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   rc = 0==> success                                                       */
/*        1==> failure                                                       */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
APIRET GetSessionID( USHORT pid, ULONG *psid )
{
 ULONG          flags;
 qsPrec_t       *pProcRec  = NULL;       /* ptr to process record section    */
 qsS16Headrec_t *p16SemRec = NULL;       /* ptr to 16 bit sem section        */
 void           *pProcStatBuf;

 /***************************************************************************/
 /* - Allocate a 64k buffer. This is the recommended size since a large     */
 /*   system may generate this much. It's allocated on a 64k boundary       */
 /*   because DosQprocStatus() is a 16 bit call and we don't want the       */
 /*   buffer to overlap a 64k boundary.                                     */
 /***************************************************************************/
 flags = PAG_COMMIT|PAG_READ|PAG_WRITE|OBJ_TILE;
 if( DosAllocMem( &pProcStatBuf,BUFFER_SIZE,flags) ||
     DosQProcStatus( (ULONG*)pProcStatBuf , BUFFER_SIZE )
   )
  return(1);

 /***************************************************************************/
 /* Define a pointer to the process subsection of information.              */
 /***************************************************************************/
 pProcRec   = (qsPrec_t       *)((qsPtrRec_t*)pProcStatBuf)->pProcRec;
 p16SemRec  = (qsS16Headrec_t *)((qsPtrRec_t*)pProcStatBuf)->p16SemRec;

 /***************************************************************************/
 /* - scan to the proc record for the pid.                                  */
 /***************************************************************************/
 for( ;pProcRec->pid != pid; )
 {
  /**************************************************************************/
  /* Get a pointer to the next process block and test for past end of block. /
  /**************************************************************************/
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

 *psid = pProcRec->sgid;
 return(0);
}
/*---------------------------------------------------------------------------*/
#endif

