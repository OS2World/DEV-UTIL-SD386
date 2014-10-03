/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   xbox.c                                                               827*/
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   The x-box interface.                                                    */
/*                                                                           */
/* History:                                                                  */
/*                                                                           */
/*   06/09/93 Created.                                                       */
/*                                                                           */
/*... 06/04/93  827   Joe       Add remote debug support.                    */
/*... 03/29/94  917   Joe       Ctrl-Break handling for single process.      */
/*...                                                                        */
/*****************************************************************************/
#include "all.h"

extern CmdParms cmd;

/*****************************************************************************/
/* xFindExe()                                                                */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Find the EXE file from the server. Search in the following order:       */
/*                                                                           */
/*     1. Explicit program name including path reference.                    */
/*     2. Current directory.                                                 */
/*     3. SD386SRC environment variable.                                     */
/*     4. PATH environment variable.                                         */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   progname The EXE name plus optional path reference.                     */
/*   pn       Pointer to the caller's buffer to receive fully qualified      */
/*            filespec.                                                      */
/*   pnlen    Length of the caller's buffer receiving filespec.              */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   rc       The return code from DosSearchPath().                          */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*  The EXE extension has been added to the filespec.                        */
/*                                                                           */
/*****************************************************************************/
APIRET xFindExe(char *progname,char *pn,UINT pnlen)
{
 APIRET rc;

 if(IsEspRemote())
 {

  RequestSerialMutex();
  rc = TxFindExe(progname,pn,pnlen);
  ReleaseSerialMutex();
 }
 else
 {
  rc = XSrvFindExe(progname,pn,pnlen);
 }
 return(rc);
}

/*****************************************************************************/
/* xStartUser()                                                              */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Start the debuggee in the probe.                                        */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   pUserEXE       -> fully qualified debuggee specification.               */
/*   pUserParms     -> debuggee parms.                                       */
/*   SessionType       user specified session type.                          */
/*   pSessionID        receiver of the SessionID.                            */
/*   pProcessID        receiver of the ProcessID..                           */
/*   pProcessType      receiver of the Process type.                         */
/*   MsgLen            length of the buffer to receive offending mod if error*/
/*   pMsgBuf        -> buffer to receive offending module name.              */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   rc            return code from DosStartSession on the remote debuggee.  */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   pUserExe   != NULL                                                      */
/*   pUserParms may be = NULL                                                */
/*                                                                           */
/*****************************************************************************/
APIRET xStartUser(char   *pUserEXE,
                  char   *pUserParms,
                  USHORT  SessionType,
                  ULONG  *pSessionID,
                  ULONG  *pProcessID,
                  ULONG  *pProcessType,
                  ULONG   MsgLen,
                  char   *pMsgBuf )
{
 APIRET rc;

 if(IsEspRemote())
 {
  RequestSerialMutex();
  rc = TxStartUser( pUserEXE,
                    pUserParms,
                    SessionType,
                    pSessionID,
                    pProcessID,
                    pProcessType,
                    pMsgBuf );
  ReleaseSerialMutex();
 }
 else
 {
  rc = EspStartUser( pUserEXE,
                     pUserParms,
                     SessionType,
                     pSessionID,
                     pProcessID,
                     pProcessType,
                     MsgLen,
                     pMsgBuf );
 }
 return(rc);
}

/*****************************************************************************/
/* xGoInit()                                                                 */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Initialize a process for debugging.                                     */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   pptb               -> to the ptrace buffer.                             */
/*   ppModuleLoadTable  where to put a ptr to the ModuleLoadTable.           */
/*   pModuleTableLength where to put the length of the ModuleLoadtable.      */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   rc           DosDebug return code.                                      */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   The Pid is contained in the ptrace buffer.                              */
/*                                                                           */
/*****************************************************************************/
APIRET xGoInit( PtraceBuffer  *pptb,
                ULONG         *pExecAddr,
                UINT         **ppModuleLoadTable,
                int           *pModuleTableLength)
{
 APIRET rc;

 if(IsEspRemote())
 {
  RequestSerialMutex();
  rc = TxGoInit(pptb,pExecAddr,ppModuleLoadTable,pModuleTableLength);
  ReleaseSerialMutex();
 }
 else
 {
  rc = XSrvGoInit(pptb,pExecAddr,ppModuleLoadTable,pModuleTableLength);
 }
 return(rc);
}

/*****************************************************************************/
/* xGoEntry()                                                                */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Run to the initial entry point.                                         */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   pptb               -> to the ptrace buffer.                             */
/*   pExecAddr          where to put the exec address.                       */
/*   ppModuleLoadTable  where to put a ptr to the ModuleLoadTable.           */
/*   pModuleTableLength where to put the length of the ModuleLoadtable.      */
/*   ExecAddr           current EIP.                                         */
/*   ExecFlags          execution control flags such as RUNNING_DEFERRED or  */
/*                      LOAD_MODULES.                                        */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   rc           trap code.                                                 */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   The Pid is contained in the ptrace buffer.                              */
/*                                                                           */
/*****************************************************************************/
APIRET xGoEntry( PtraceBuffer  *pptb,
                 ULONG         *pExecAddr,
                 UINT         **ppModuleLoadTable,
                 int           *pModuleTableLength,
                 ULONG         ExecAddr,
                 int           ExecFlags)
{
 APIRET rc;

 if(IsEspRemote())
 {
  RequestSerialMutex();
  rc=TxGoEntry(pptb,
               pExecAddr,
               ppModuleLoadTable,
               pModuleTableLength,
               ExecAddr,
               ExecFlags );
  ReleaseSerialMutex();
 }
 else
 {
  rc=XSrvGoFast(pptb,
                pExecAddr,
                ppModuleLoadTable,
                pModuleTableLength,
                ExecAddr,
                ExecFlags);
 }
 return(rc);
}

/*****************************************************************************/
/* xGoStep()                                                                 */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Step debuggee at source or assembler level.                             */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   pptb               -> to the ptrace buffer.                             */
/*   pExecAddr          where to put the exec address.                       */
/*   ppModuleLoadTable  where to put a ptr to the ModuleLoadTable.           */
/*   pModuleTableLength where to put the length of the ModuleLoadtable.      */
/*   ExecAddr           current EIP.                                         */
/*   ExecAddrlo         start addr of the instructions for a source line.    */
/*   ExecAddrhi         first addr past the instructions for a source line.  */
/*   how                step qualifier such as OVERCALL or INTOCALL.         */
/*   ExecFlags          execution control flags such as RUNNING_DEFERRED or  */
/*                      LOAD_MODULES.                                        */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   rc           trap code.                                                 */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   The Pid is contained in the ptrace buffer.                              */
/*                                                                           */
/*****************************************************************************/
APIRET xGoStep( PtraceBuffer  *pptb,
                ULONG         *pExecAddr,
                UINT         **ppModuleLoadTable,
                int           *pModuleTableLength,
                ULONG         ExecAddr,
                ULONG         ExecAddrlo,
                ULONG         ExecAddrhi,
                int           how,
                int           ExecFlags)
{
 APIRET rc;

 if(IsEspRemote())
 {
  RequestSerialMutex();
  rc=TxGoStep(pptb,
              pExecAddr,
              ppModuleLoadTable,
              pModuleTableLength,
              ExecAddr,
              ExecAddrlo,
              ExecAddrhi,
              how,
              ExecFlags );
 ReleaseSerialMutex();
 }
 else
 {
  rc=XSrvGoStep(pptb,
                pExecAddr,
                ppModuleLoadTable,
                pModuleTableLength,
                ExecAddr,
                ExecAddrlo,
                ExecAddrhi,
                how,
                ExecFlags );
 }
 return(rc);
}

/*****************************************************************************/
/* xGoFast()                                                                 */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Run the debuggee.                                                       */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   pptb               -> to the ptrace buffer.                             */
/*   pExecAddr          where to put the exec address.                       */
/*   ppModuleLoadTable  where to put a ptr to the ModuleLoadTable.           */
/*   pModuleTableLength where to put the length of the ModuleLoadtable.      */
/*   ExecAddr           current EIP.                                         */
/*   ExecFlags          execution control flags such as RUNNING_DEFERRED or  */
/*                      LOAD_MODULES.                                        */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   rc           trap code.                                                 */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   The Pid is contained in the ptrace buffer.                              */
/*                                                                           */
/*****************************************************************************/
APIRET xGoFast( PtraceBuffer  *pptb,
                ULONG         *pExecAddr,
                UINT         **ppModuleLoadTable,
                int           *pModuleTableLength,
                ULONG         ExecAddr,
                int           ExecFlags)
{
 APIRET rc;

 if(IsEspRemote())
 {
  RequestSerialMutex();
  rc=TxGoFast(pptb,
              pExecAddr,
              ppModuleLoadTable,
              pModuleTableLength,
              ExecAddr,
              ExecFlags );
  ReleaseSerialMutex();
 }
 else
 {
  rc=XSrvGoFast(pptb,
                pExecAddr,
                ppModuleLoadTable,
                pModuleTableLength,
                ExecAddr,
                ExecFlags);
 }
 return(rc);
}


/*****************************************************************************/
/* xDefBrk()                                                                 */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Define a break point in the probe.                                      */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   where      addr where to set the break point.                           */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
void xDefBrk(ULONG where )
{
 if(IsEspRemote())
 {
  RequestSerialMutex();
  TxDefBrk( where );
  ReleaseSerialMutex();
 }
 else
 {
  XSrvDefBrk( where );
 }
}

/*****************************************************************************/
/* xUndBrk()                                                                 */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   UnDefine a break point in the probe.                                    */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   where    addr of break point to undefine.                               */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
void xUndBrk( ULONG where )
{
 if(IsEspRemote())
 {
  RequestSerialMutex();
  TxUndBrk( where );
  ReleaseSerialMutex();
 }
 else
 {
  XSrvUndBrk( where );
 }
}

/*****************************************************************************/
/* xPutInBrk()                                                               */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Put in a breakpoint.                                                    */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   where      addr where to put in the break point.                        */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
void xPutInBrk(ULONG where )
{
 if(IsEspRemote())
 {
  RequestSerialMutex();
  TxPutInBrk( where );
  ReleaseSerialMutex();
 }
 else
 {
  XSrvPutInBrk( where );
 }
}

/*****************************************************************************/
/* xPullOutBrk()                                                             */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Pull out a breakpoint.                                                  */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   where      addr where to extact the break point.                        */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
void xPullOutBrk( ULONG where )
{
 if(IsEspRemote())
 {
  RequestSerialMutex();
  TxPullOutBrk( where );
  ReleaseSerialMutex();
 }
 else
 {
  XSrvPullOutBrk( where );
 }
}

/*****************************************************************************/
/* xInsertAllBrk()                                                           */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Insert all the breakpoints.                                             */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
void xInsertAllBrk( void )
{

 if(IsEspRemote())
 {
  RequestSerialMutex();
  TxInsertAllBrk();
  ReleaseSerialMutex();
 }
 else
 {
  XSrvInsertAllBrk();
 }
 return;
}

/*****************************************************************************/
/* xRemoveAllBrk()                                                           */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Remove all the breakpoints.                                             */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
void xRemoveAllBrk( void )
{

 if(IsEspRemote())
 {
  RequestSerialMutex();
  TxRemoveAllBrk();
  ReleaseSerialMutex();
 }
 else
 {
  XSrvRemoveAllBrk();
 }
 return;
}

/*****************************************************************************/
/* xDosDebug()                                                               */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   DosDebug API.                                                           */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   pptb       -> to the DosDebug buffer.                                   */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   rc            rc from DosDebug.                                         */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   none.                                                                   */
/*                                                                           */
/*****************************************************************************/
APIRET APIENTRY xDosDebug(PtraceBuffer *pptb)
{
 APIRET rc;

 if(IsEspRemote())
 {
  RequestSerialMutex();
  rc = TxDosDebug(pptb);
  ReleaseSerialMutex();
 }
 else
 {
  rc = DosDebug(pptb);
 }
 return(rc);
}

/*****************************************************************************/
/* xGetThreadInfo()                                                          */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Get the thread info for the debuggee.                                   */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   pBuffer    -> to the buffer to receive the info.                        */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   ntids      the number of threads.                                       */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*                                                                           */
/*****************************************************************************/
ULONG xGetThreadInfo(THREADINFO *pBuffer)
{
 ULONG ntids;

 if(IsEspRemote())
 {
  RequestSerialMutex();
  ntids = TxGetThreadInfo(pBuffer);
  ReleaseSerialMutex();
 }
 else
 {
  ntids = XSrvGetThreadInfo(pBuffer);
 }
 return(ntids);
}

/*****************************************************************************/
/* xFreezeThread()                                                           */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Freeze a thread.                                                        */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   tid      thread id to freeze.                                           */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
void xFreezeThread( ULONG tid )
{
 if(IsEspRemote())
 {
  RequestSerialMutex();
  TxFreezeThread( tid );
  ReleaseSerialMutex();
 }
 else
 {
  XSrvFreezeThread( tid );
 }
}

/*****************************************************************************/
/* xThawThread()                                                             */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Thaw a thread.                                                          */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   tid      thread id to thaw.                                             */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
void xThawThread( ULONG tid )
{
 if(IsEspRemote())
 {
  RequestSerialMutex();
  TxThawThread( tid );
  ReleaseSerialMutex();
 }
 else
 {
  XSrvThawThread( tid );
 }
}

/*****************************************************************************/
/* xGetCallStack()                                                           */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Establishes the following arrays for the current state of the user's    */
/*   stack:                                                                  */
/*                                                                           */
/*    ActFaddrs[]   the CS:IP or EIP values for the stack frames.            */
/*    ActCSAtrs[]   0 => 16-bit stack frame  1 => 32-bit stack frame.        */
/*    ActFrames[]   offsets in the stack of the stack frames ( EBP values )  */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   pparms        Stuff needed to boot the stack unwind.                    */
/*   ppActCSAtrs   -> ActCSAtrs[].                                           */
/*   ppActFrames   -> ActFrames[].                                           */
/*   ppActFaddrs   -> ActFaddrs[].                                           */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   NActFrames     Number of active frames unwound.                         */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
ULONG xGetCallStack(STACK_PARMS *pparms,
                    UCHAR       **ppActCSAtrs,
                    UINT        **ppActFrames,
                    UINT        **ppActFaddrs )
{
 ULONG NActFrames;

 if(IsEspRemote())
 {
  RequestSerialMutex();
  NActFrames = TxGetCallStack(pparms,ppActCSAtrs,ppActFrames,ppActFaddrs );
  ReleaseSerialMutex();
 }
 else
 {
  NActFrames = XSrvGetCallStack(pparms,ppActCSAtrs,ppActFrames,ppActFaddrs );
 }
 return(NActFrames);
}

/*****************************************************************************/
/* TxGetExeOrDllEntryOrExitPt()                                              */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Get the entry point for a dll or exe.                                   */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   mte        module table handle of exe/dll.                              */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   EntryExitAddr  the entry/exit address from the exe/dll header.          */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
ULONG xGetExeOrDllEntryOrExitPt( UINT mte )
{
 ULONG EntryPtAddr = 0;

 if(IsEspRemote())
 {
  RequestSerialMutex();
  EntryPtAddr = TxGetExeOrDllEntryOrExitPt( mte );
  ReleaseSerialMutex();
 }
 else
 {
  EntryPtAddr = XSrvGetExeOrDllEntryOrExitPt( mte );
 }
 return(EntryPtAddr);
}

/*****************************************************************************/
/* xNormalQuit()                                                             */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Terminate the debuggee process.                                         */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   AppTerminated  Flag indicating whether or not the app is terminated.    */
/*   mte            module table handle of exe.                              */
/*   EntryPt        exe entry point.                                         */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   rc             0 or 1                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   The application is either in the "DBG_N_ProcTerm" since the             */
/*   app has executed a DosExit, or the state is unknown.                    */
/*                                                                           */
/*****************************************************************************/
APIRET xNormalQuit( int AppTerminated, UINT mte, ULONG EntryPt )
{
 APIRET  rc;

 if(IsEspRemote())
 {
  RequestSerialMutex();
  rc = TxNormalQuit( AppTerminated, mte, EntryPt );
  ReleaseSerialMutex();
 }
 else
 {
  rc = XSrvNormalQuit( AppTerminated, mte, EntryPt );
 }
 return(rc);
}

/*****************************************************************************/
/* xSetExecAddr()                                                            */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Set the exec address for the current thread.                            */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   ExecAddr   Eip where we want to execute.                                */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   rc             0 or 1                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
APIRET xSetExecAddr(ULONG ExecAddr )
{
 APIRET rc = 0;

 if(IsEspRemote())
 {
  RequestSerialMutex();
  rc = TxSetExecAddr( ExecAddr );
  ReleaseSerialMutex();
 }
 else
 {
  rc = XSrvSetExecAddr( ExecAddr );
 }
 return(rc);
}

/*****************************************************************************/
/* xDefWps()                                                                 */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Define the watch points to the probe.                                   */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   pRegs    -  ->array of watch point definitions.                         */
/*   size     -  size of the block of debug register data.                   */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
void xDefWps( void *pRegs, int size )
{
 if(IsEspRemote())
 {
  RequestSerialMutex();
  TxDefWps( pRegs,size );
  ReleaseSerialMutex();
 }
 else
 {
  XSrvDefWps( pRegs,size);
 }
}

/*****************************************************************************/
/* xPutInWps()                                                               */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Put in watch points.                                                    */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   pIndexes -  ->array that will receive the watch point indexes.          */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
void xPutInWps( ULONG *pIndexes )
{
 if(IsEspRemote())
 {
  RequestSerialMutex();
  TxPutInWps( pIndexes );
  ReleaseSerialMutex();
 }
 else
 {
  XSrvPutInWps(pIndexes);
 }
}

/*****************************************************************************/
/* xPullOutWps()                                                             */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Pull out watch points.                                                  */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
void xPullOutWps( void )
{
 if(IsEspRemote())
 {
  RequestSerialMutex();
  TxPullOutWps( );
  ReleaseSerialMutex();
 }
 else
 {
  XSrvPullOutWps();
 }
}

/*****************************************************************************/
/* xGetMemoryBlock()                                                         */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Get a block of data.                                                    */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   addr            pointer to address space in user's application.         */
/*   BytesWanted     number of bytes to copy from the user's application.    */
/*   pBytesObtained  pointer to number of bytes that were read in.           */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   pbytes          -> to block of bytes obtained.                          */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
UCHAR *xGetMemoryBlock( ULONG addr , int BytesWanted, int *pBytesObtained )
{
 UCHAR *pbytes;

 if(IsEspRemote())
 {
  RequestSerialMutex();
  pbytes = TxGetMemoryBlock( addr,BytesWanted, pBytesObtained );
  ReleaseSerialMutex();
  return( pbytes );
 }
 else
 {
  return( XSrvGetMemoryBlock( addr,BytesWanted, pBytesObtained ) );
 }
}

/*****************************************************************************/
/* xGetMemoryBlocks()                                                        */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Get some blocks of memory.                                              */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*  pDefBlks         -> to an array of addresses and sizes of mem blocks.    */
/*  LengthOfDefBlk      length of the block defining the addrs & sizes.      */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*  pMemBlks            -> to memory blocks.                                 */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*  caller frees the pMemBlks allocation.                                    */
/*                                                                           */
/*****************************************************************************/
void *xGetMemoryBlocks(void *pDefBlks,int LengthOfDefBlk)
{
 int   junk;
 void *pMemBlks;

 if(IsEspRemote())
 {
  RequestSerialMutex();
  pMemBlks = TxGetMemoryBlocks( pDefBlks, LengthOfDefBlk );
  ReleaseSerialMutex();
  return( pMemBlks );
 }
 else
 {
  return( XSrvGetMemoryBlocks( pDefBlks,&junk ) );
 }

}

/*****************************************************************************/
/* xSetExceptions()                                                          */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Get some blocks of memory.                                              */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*  pExceptionMap    -> to an array of exception notify/nonotify specs.      */
/*  length              length of the map in bytes.                          */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
void xSetExceptions( UCHAR *pExceptionMap, int length)
{

 if(IsEspRemote())
 {
  RequestSerialMutex();
  TxSetExceptions( pExceptionMap , length);
  ReleaseSerialMutex();
 }
 else
 {
  XSrvSetExceptions( pExceptionMap , length);
 }
}

/*****************************************************************************/
/* xSetExecThread()                                                          */
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
APIRET xSetExecThread(ULONG *pExecAddr,PtraceBuffer *pptb, UINT tid)
{
 APIRET rc;

 if(IsEspRemote())
 {
  RequestSerialMutex();
  rc = TxSetExecThread(pExecAddr,pptb, tid);
  ReleaseSerialMutex();
 }
 else
 {
  rc = XSrvSetExecThread(pExecAddr,pptb, tid);
 }
 return(rc);
}

/*****************************************************************************/
/* xWriteRegs()                                                              */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Write the registers to the application.                                 */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   pExecAddr -> to address that will receive the flat execution address.   */
/*   pptb      -> to the ptrace buffer that we will fill in for the caller.  */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   rc        0=>success                                                    */
/*             1=>failure                                                    */
/*                                                                           */
/* Assumption:                                                               */
/*                                                                           */
/*****************************************************************************/
APIRET xWriteRegs(ULONG *pExecAddr,PtraceBuffer *pptb)
{
 APIRET rc;

 if(IsEspRemote())
 {
  RequestSerialMutex();
  rc = TxWriteRegs(pExecAddr,pptb);
  ReleaseSerialMutex();
 }
 else
 {
  rc = XSrvWriteRegs(pExecAddr,pptb);
 }
 return(rc);
}

/*****************************************************************************/
/* xGetCoRegs()                                                              */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Get the co-processor registers.                                         */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   pExecAddr -> to address that will receive the flat execution address.   */
/*   pptb      -> to the ptrace buffer that we will fill in for the caller.  */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   rc        0=>success                                                    */
/*             1=>failure                                                    */
/*                                                                           */
/* Assumption:                                                               */
/*                                                                           */
/*****************************************************************************/
APIRET xGetCoRegs(void *pCoproc_regs)
{
 APIRET rc;

 if(IsEspRemote())
 {
  RequestSerialMutex();
  rc = TxGetCoRegs(pCoproc_regs);
  ReleaseSerialMutex();
 }
 else
 {
  rc = XSrvGetCoRegs(pCoproc_regs);
 }
 return(rc);
}

/*****************************************************************************/
/* xTerminateESP()                                                           */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Terminate the probe.                                                    */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*  The decision to terminate ESP has already been made.                     */
/*                                                                           */
/*****************************************************************************/
void xTerminateESP( void )
{

 RequestSerialMutex();
 TxTerminateESP();
 ReleaseSerialMutex();
 return;
}

/*****************************************************************************/
/* xSetEspRunOpts()                                                       919*/
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Send run options to esp. Only called if probe is remote.                */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*  pEspRunOpts    ->to block of data containing flags and names             */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*                                                                           */
/*****************************************************************************/
void xSetEspRunOpts( ESP_RUN_OPTS *pEspRunOpts )
{
 RequestSerialMutex();
 TxSetEspRunOpts(pEspRunOpts);
 ReleaseSerialMutex();
}

/*****************************************************************************/
/* xSendCtrlBreak()                                                       917*/
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Send a Ctrl-Break message to esp.                                       */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   handle      the connection handle we're sending the ctrl_break to.      */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*  The queue will request/release the mutex during the ctrl-break           */
/*  handling.                                                                */
/*                                                                           */
/*****************************************************************************/
void xSendCtrlBreak( LHANDLE handle, USHORT pid )
{
 TxSendCtrlBreak( handle, pid );
 return;
}

/*****************************************************************************/
/* xStartQueListenThread()                                                919*/
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Start a que listen thread in esp.                                       */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*                                                                           */
/*****************************************************************************/
void xStartQueListenThread( void )
{
 RequestSerialMutex();
 TxStartQueListenThread( );
 ReleaseSerialMutex();
 return;
}

/*****************************************************************************/
/* xStartEspQue()                                                         919*/
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Start an esp que.                                                       */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   rc                                                                      */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
APIRET xStartEspQue( void )
{
 APIRET rc;

 RequestSerialMutex();
 rc = TxStartEspQue( );
 ReleaseSerialMutex();
 return(rc);
}

/*****************************************************************************/
/* xConnectEsp()                                                             */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*  Connect to the probe for the parameter pid and wait until we             */
/*  get verification that the message has been received and sent to          */
/*  the queue.                                                               */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   handle   handle of the connection to the probe.                         */
/*   pid      process id of the probe.                                       */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
void xConnectEsp( LHANDLE handle, USHORT pid )
{
 RequestSerialMutex();
 TxConnectEsp( handle, pid );
 ReleaseSerialMutex();
}

/*****************************************************************************/
/* xSendKillListen()                                                         */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Send a command to kill the listen thread when connected via a           */
/*   parallel connection.                                                    */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   handle      the connection handle.                                      */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
void xSendKillListen( LHANDLE handle )
{
 TxSendKillListen( handle );
 return;
}

/*****************************************************************************/
/* xSelectSession()                                                          */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Select the debuggee session.                                            */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumption:                                                               */
/*                                                                           */
/*****************************************************************************/
void xSelectSession( void )
{
 RequestSerialMutex();
 TxSelectSession();
 ReleaseSerialMutex();
}
