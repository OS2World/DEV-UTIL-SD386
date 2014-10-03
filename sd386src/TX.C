/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   tx.c                                                                 827*/
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   x-server remote functions calls.                                        */
/*                                                                           */
/* History:                                                                  */
/*                                                                           */
/*   06/24/93 Created.                                                       */
/*                                                                           */
/*... 06/24/93  827   Joe       Add remote debug support.                    */
/*... 03/29/94  917   Joe       Ctrl-Break handling for single process.      */
/*...                                                                        */
/*****************************************************************************/
#include "all.h"

/*****************************************************************************/
/* TxFindExe()                                                               */
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
/*   progname The filespec of the EXE we're looking for.                     */
/*   pn       Pointer to the caller's buffer to receive filespec.            */
/*   pnlen    Length of the caller's buffer.                                 */
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
APIRET TxFindExe(char *progname,char *pn,UINT pnlen)
{
 COMMAND      cmd;
 ULONG        LengthofProgName;
 TXRX_FINDEXE ParmPacket;
 RET_FINDEXE  ReturnPacket;

 memset(&ParmPacket,0,sizeof(ParmPacket) );
 LengthofProgName = strlen(progname) + 1;
 memcpy(ParmPacket.buffer,progname,LengthofProgName);
 ParmPacket.progname = (ULONG)((ULONG)ParmPacket.buffer-(ULONG)&ParmPacket);
 ParmPacket.pnlen = pnlen;
 ParmPacket.pn    = NULL;

 cmd.api = FINDEXE;
 cmd.cmd = 0;
 cmd.len = sizeof(ULONG)+sizeof(char*)+sizeof(UINT)+LengthofProgName;

 RmtSend(DEFAULT_HANDLE, &cmd , sizeof(cmd) );
 RmtSend(DEFAULT_HANDLE, &ParmPacket, cmd.len );

 memset(&ReturnPacket,0,sizeof(ReturnPacket) );
 RmtRecv(DEFAULT_HANDLE, &cmd, sizeof(cmd) );
 RmtRecv(DEFAULT_HANDLE, &ReturnPacket, cmd.len );
 memcpy(pn,ReturnPacket.buffer,cmd.len);
 return( ReturnPacket.rc );
}

/*****************************************************************************/
/* TxStartUser()                                                             */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Start the debuggee in the xserver.                                      */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   pUserEXE       -> fully qualified debuggee specification.               */
/*   pUserParms     -> debuggee parms.                                       */
/*   SessionType       user specified session type.                          */
/*   pSessionID        where to put the SessionID.                           */
/*   pProcessID        where to put the ProcessID.                           */
/*   pProcessType      receiver of the Process Type.                         */
/*   pMsgBuf        -> buffer to receive offending module name.              */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   rc            rc from DosStartSession on the remote debuggee.           */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   pUserExe   != NULL                                                      */
/*   pUserParms may be = NULL                                                */
/*                                                                           */
/*****************************************************************************/
APIRET TxStartUser(char   *pUserEXE,
                   char   *pUserParms,
                   USHORT  SessionType,
                   ULONG  *pSessionID,
                   ULONG  *pProcessID,
                   ULONG  *pProcessType,
                   char   *pMsgBuf )
{
 TXRX_STARTUSER  ParmPacket;
 RET_STARTUSER   ReturnPacket;
 COMMAND cmd;
 UCHAR  *p;
 UCHAR  *dcp;
 UCHAR  *scp;
 ULONG   ParmPacketLen;
 UCHAR  *pParmPacket;
 ULONG   ReturnPacketLen;

 /****************************************************************************/
 /* - Compute the length of the parm packet.                                 */
 /* - Copy the exe name to the parm packet.                                  */
 /* - Copy the session type to the parm packet.                              */
 /* - If parms, then copy those to the parm packet.                          */
 /****************************************************************************/
 ParmPacketLen = sizeof(pUserEXE) + sizeof(pUserParms) + sizeof(SessionType);
 ParmPacketLen += 1 + strlen(pUserEXE) + 1;
 if(pUserParms)
  ParmPacketLen += 1 + strlen(pUserParms) + 1;

 p   = pParmPacket = (UCHAR*)&ParmPacket;
 dcp = ParmPacket.buffer;
 scp = pUserEXE;

 memset(&ParmPacket,0,sizeof(ParmPacket));

 ParmPacket.SessionType = SessionType;
 ParmPacket.pUserEXE = (ULONG)(dcp - p);

 *dcp++ = strlen(pUserEXE);
 strcpy(dcp,scp);

 if(pUserParms)
 {
  dcp += strlen(dcp) + 1;
  ParmPacket.pUserParms = (ULONG)(dcp - p);
  scp = pUserParms;
  *dcp++ = strlen(pUserParms);
  strcpy(dcp,scp);
 }
 cmd.api = STARTUSER;
 cmd.len = ParmPacketLen;

 RmtSend(DEFAULT_HANDLE, &cmd , sizeof(cmd) );
 RmtSend(DEFAULT_HANDLE, pParmPacket, ParmPacketLen );

 RmtRecv(DEFAULT_HANDLE, &cmd, sizeof(cmd) );
 ReturnPacketLen = cmd.len;
 RmtRecv(DEFAULT_HANDLE, &ReturnPacket, ReturnPacketLen );

 *pSessionID   = ReturnPacket.SessionID;
 *pProcessID   = ReturnPacket.ProcessID;
 *pProcessType = ReturnPacket.ProcessType;
 if( ReturnPacket.rc != 0 )
  strcpy( pMsgBuf,ReturnPacket.ErrorBuf );

 return( ReturnPacket.rc );
}

/*****************************************************************************/
/* TxGoInit()                                                                */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Initialize the EXE file for debug.                                      */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   pptb               -> to the ptrace buffer.                             */
/*   pExecAddr          -> where to put the current exec address.            */
/*   ppModuleLoadTable  where to put a ptr to the ModuleLoadTable.           */
/*   pModuleTableLength where to put the length of the ModuleLoadtable.      */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   rc           trap code.                                                 */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
APIRET TxGoInit( PtraceBuffer  *pptb,
                 ULONG         *pExecAddr,
                 UINT         **ppModuleLoadTable,
                 int           *pModuleTableLength)
{
 COMMAND     cmd;
 TXRX_GOINIT ParmPacket;
 RET_GOINIT  ReturnPacket;

 /****************************************************************************/
 /* - We will receive two separate transmissions.                            */
 /* - One for the return packet and one for the module table.                */
 /*                                                                          */
 /****************************************************************************/

 ParmPacket.Pid = pptb->Pid;
 ParmPacket.Sid = pptb->Tid;

 cmd.api = GOINIT;
 cmd.len = sizeof(ParmPacket);

 RmtSend(DEFAULT_HANDLE, &cmd , sizeof(cmd) );
 RmtSend(DEFAULT_HANDLE, &ParmPacket, cmd.len );

 RmtRecv(DEFAULT_HANDLE, &cmd, sizeof(cmd) );
 RmtRecv(DEFAULT_HANDLE, &ReturnPacket, cmd.len );
 memcpy(pptb, &ReturnPacket.ptb, sizeof(PtraceBuffer) );
 *pExecAddr = ReturnPacket.ExecAddr;

 RmtRecv(DEFAULT_HANDLE, &cmd, sizeof(cmd) );
 *ppModuleLoadTable = Talloc(cmd.len);
 RmtRecv(DEFAULT_HANDLE, *ppModuleLoadTable, cmd.len );
 *pModuleTableLength = cmd.len;
 return( ReturnPacket.rc );
}

/*****************************************************************************/
/* TxGoEntry()                                                               */
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
/*   ExecFlags          exectuion control flags.                             */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   rc           trap code.                                                 */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   pptb->Pid == ProcessID                                                  */
/*                                                                           */
/*****************************************************************************/
APIRET TxGoEntry( PtraceBuffer  *pptb,
                  ULONG         *pExecAddr,
                  UINT         **ppModuleLoadTable,
                  int           *pModuleTableLength,
                  ULONG         ExecAddr,
                  int           ExecFlags)
{
 COMMAND     cmd;
 TXRX_GOFAST ParmPacket;
 RET_GOFAST  ReturnPacket;

 /****************************************************************************/
 /* - We will receive two separate transmissions.                            */
 /* - One for the return packet and one for the module table.                */
 /*                                                                          */
 /****************************************************************************/

 ParmPacket.ptb         = *pptb;
 ParmPacket.ExecAddr    = ExecAddr;
 ParmPacket.ExecFlags   = ExecFlags;

 cmd.api = GOENTRY;
 cmd.len = sizeof(ParmPacket);

 RmtSend(DEFAULT_HANDLE, &cmd , sizeof(cmd) );
 RmtSend(DEFAULT_HANDLE, &ParmPacket, cmd.len );

 RmtRecv(DEFAULT_HANDLE, &cmd, sizeof(cmd) );
 RmtRecv(DEFAULT_HANDLE, &ReturnPacket, cmd.len );
 memcpy(pptb, &ReturnPacket.ptb, sizeof(PtraceBuffer) );
 *pExecAddr = ReturnPacket.ExecAddr;

 RmtRecv(DEFAULT_HANDLE, &cmd, sizeof(cmd) );
 *ppModuleLoadTable = Talloc(cmd.len);
 RmtRecv(DEFAULT_HANDLE, *ppModuleLoadTable, cmd.len );
 *pModuleTableLength = cmd.len;
 return( ReturnPacket.rc );

}
/*****************************************************************************/
/* TxDefBrk()                                                                */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Define a break point in the x-server.                                   */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   where      addr where to set the break point.                           */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*  It will work!                                                            */
/*                                                                           */
/*****************************************************************************/
void TxDefBrk( ULONG where )
{
 COMMAND      cmd;
 TXRX_DEFBRK  ParmPacket;

 ParmPacket.where = where;
 cmd.api = DEFBRK;
 cmd.len = sizeof(ParmPacket);

 RmtSend(DEFAULT_HANDLE, &cmd , sizeof(cmd) );
 RmtSend(DEFAULT_HANDLE, &ParmPacket, cmd.len );
}

/*****************************************************************************/
/* TxUndBrk()                                                                */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   UnDefine a break point in the x-server.                                 */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   where    addr of break point to undefine.                               */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*  It will work!                                                            */
/*                                                                           */
/*****************************************************************************/
void TxUndBrk( ULONG where )
{
 COMMAND      cmd;
 TXRX_UNDBRK  ParmPacket;

 ParmPacket.where = where;
 cmd.api = UNDBRK;
 cmd.len = sizeof(ParmPacket);

 RmtSend(DEFAULT_HANDLE, &cmd , sizeof(cmd) );
 RmtSend(DEFAULT_HANDLE, &ParmPacket, cmd.len );
}

/*****************************************************************************/
/* TxPutInBrk()                                                              */
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
void TxPutInBrk( ULONG where )
{
 COMMAND       cmd;
 TXRX_PUTINBRK ParmPacket;

 ParmPacket.where = where;

 cmd.api = PUTINBRK;
 cmd.len = sizeof(ParmPacket);

 RmtSend(DEFAULT_HANDLE, &cmd , sizeof(cmd) );
 RmtSend(DEFAULT_HANDLE, &ParmPacket, cmd.len );
}

/*****************************************************************************/
/* TxPullOutBrk()                                                            */
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
void TxPullOutBrk( ULONG where )
{
 COMMAND         cmd;
 TXRX_PULLOUTBRK ParmPacket;

 ParmPacket.where = where;
 cmd.api = PULLOUTBRK;
 cmd.len = sizeof(ParmPacket);

 RmtSend(DEFAULT_HANDLE, &cmd , sizeof(cmd) );
 RmtSend(DEFAULT_HANDLE, &ParmPacket, cmd.len );
}

/*****************************************************************************/
/* TxInsertAllBrk()                                                          */
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
void TxInsertAllBrk()
{
 COMMAND      cmd;

 cmd.api = INSERTALLBRK;
 cmd.len = 0;

 RmtSend(DEFAULT_HANDLE, &cmd , sizeof(cmd) );
}

/*****************************************************************************/
/* TxRemoveAllBrk()                                                          */
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
void TxRemoveAllBrk()
{
 COMMAND      cmd;

 cmd.api = REMOVEALLBRK;
 cmd.len = 0;

 RmtSend(DEFAULT_HANDLE, &cmd , sizeof(cmd) );
}

/*****************************************************************************/
/* TxGoStep()                                                                */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Step debuggee instruction(s).                                           */
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
/*   ExecFlags          execution control flags.                             */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   rc           trap code.                                                 */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   pptb->Pid == ProcessID                                                  */
/*                                                                           */
/*****************************************************************************/
APIRET TxGoStep( PtraceBuffer  *pptb,
                 ULONG         *pExecAddr,
                 UINT         **ppModuleLoadTable,
                 int           *pModuleTableLength,
                 ULONG         ExecAddr,
                 ULONG         ExecAddrlo,
                 ULONG         ExecAddrhi,
                 int           how,
                 int           ExecFlags )
{
 COMMAND     cmd;
 TXRX_GOSTEP ParmPacket;
 RET_GOSTEP  ReturnPacket;

 /****************************************************************************/
 /* - We will receive two separate transmissions.                            */
 /* - One for the return packet and one for the module table.                */
 /*                                                                          */
 /****************************************************************************/

 ParmPacket.ptb         = *pptb;
 ParmPacket.ExecAddr    = ExecAddr;
 ParmPacket.ExecAddrlo  = ExecAddrlo;
 ParmPacket.ExecAddrhi  = ExecAddrhi;
 ParmPacket.how         = how;
 ParmPacket.ExecFlags   = ExecFlags;

 cmd.api = GOSTEP;
 cmd.len = sizeof(ParmPacket);

 RmtSend(DEFAULT_HANDLE, &cmd , sizeof(cmd) );
 RmtSend(DEFAULT_HANDLE, &ParmPacket, cmd.len );

 if(SerialParallel() == SERIAL)                                         /*919*/
 {
  SetExecutionFlag( TRUE );
  ReleaseSerialMutex();
  SerialConnect( DISCONNECT_WAIT, DbgGetProcessID(), _DBG, SendMsgToDbgQue );
  RequestSerialMutex();
  SetExecutionFlag( FALSE );
 }
 RmtRecv(DEFAULT_HANDLE, &cmd, sizeof(cmd) );
 RmtRecv(DEFAULT_HANDLE, &ReturnPacket, cmd.len );
 memcpy(pptb, &ReturnPacket.ptb, sizeof(PtraceBuffer) );
 *pExecAddr = ReturnPacket.ExecAddr;

 RmtRecv(DEFAULT_HANDLE, &cmd, sizeof(cmd) );
 *ppModuleLoadTable = Talloc(cmd.len);
 RmtRecv(DEFAULT_HANDLE, *ppModuleLoadTable, cmd.len );
 *pModuleTableLength = cmd.len;
 return( ReturnPacket.rc );

}

/*****************************************************************************/
/* TxGoFast()                                                                */
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
/*   ExecFlags          exectuion control flags.                             */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   rc           trap code.                                                 */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   pptb->Pid == ProcessID                                                  */
/*                                                                           */
/*****************************************************************************/
APIRET TxGoFast( PtraceBuffer  *pptb,
                 ULONG         *pExecAddr,
                 UINT         **ppModuleLoadTable,
                 int           *pModuleTableLength,
                 ULONG         ExecAddr,
                 int           ExecFlags)
{
 COMMAND     cmd;
 TXRX_GOFAST ParmPacket;
 RET_GOFAST  ReturnPacket;

 /****************************************************************************/
 /* - We will receive two separate transmissions.                            */
 /* - One for the return packet and one for the module table.                */
 /*                                                                          */
 /****************************************************************************/

 ParmPacket.ptb         = *pptb;
 ParmPacket.ExecAddr    = ExecAddr;
 ParmPacket.ExecFlags   = ExecFlags;

 cmd.api = GOFAST;
 cmd.len = sizeof(ParmPacket);

 RmtSend(DEFAULT_HANDLE, &cmd , sizeof(cmd) );
 RmtSend(DEFAULT_HANDLE, &ParmPacket, cmd.len );

 if(SerialParallel() == SERIAL)                                         /*919*/
 {
  SetExecutionFlag( TRUE );
  ReleaseSerialMutex();
  SerialConnect( DISCONNECT_WAIT, DbgGetProcessID(), _DBG, SendMsgToDbgQue );
  RequestSerialMutex();
  SetExecutionFlag( FALSE );
 }
 RmtRecv(DEFAULT_HANDLE, &cmd, sizeof(cmd) );
 RmtRecv(DEFAULT_HANDLE, &ReturnPacket, cmd.len );
 memcpy(pptb, &ReturnPacket.ptb, sizeof(PtraceBuffer) );
 *pExecAddr = ReturnPacket.ExecAddr;

 RmtRecv(DEFAULT_HANDLE, &cmd, sizeof(cmd) );
 *ppModuleLoadTable = Talloc(cmd.len);
 RmtRecv(DEFAULT_HANDLE, *ppModuleLoadTable, cmd.len );
 *pModuleTableLength = cmd.len;
 return( ReturnPacket.rc );

}
/*****************************************************************************/
/* TxDosDebug()                                                              */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Remote DosDebug API.                                                    */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   pptb       -> to the DosDebug buffer.                                   */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   rc            rc from DosDebug on the remote debuggee.                  */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   none.                                                                   */
/*                                                                           */
/*****************************************************************************/
APIRET APIENTRY TxDosDebug(PtraceBuffer *pptb)
{
 COMMAND       cmd;
 TXRX_DOSDEBUG ParmPacket;
 RET_DOSDEBUG  ReturnPacket;
 long          DebugCmd;

 ParmPacket.ptb = *pptb;

 cmd.api  = DOSDEBUG;
 cmd.cmd  = (UCHAR)pptb->Cmd;
 cmd.len  = sizeof(ParmPacket.ptb);
 DebugCmd = pptb->Cmd;

 if( DebugCmd == DBG_C_WriteMemBuf )
 {
  memcpy(ParmPacket.buffer,(char*)pptb->Buffer,pptb->Len);
  cmd.len += pptb->Len;
 }

 RmtSend(DEFAULT_HANDLE, &cmd , sizeof(cmd) );
 RmtSend(DEFAULT_HANDLE, &ParmPacket, cmd.len);

 RmtRecv(DEFAULT_HANDLE, &cmd, sizeof(cmd) );
 RmtRecv(DEFAULT_HANDLE, &ReturnPacket, cmd.len );

 if( (ReturnPacket.rc == 0 ) &&
     ( (DebugCmd == DBG_C_ReadMemBuf) || (DebugCmd == DBG_C_ThrdStat) )
   )
   memcpy((char*)pptb->Buffer,ReturnPacket.buffer,pptb->Len);

 memcpy(pptb,&ReturnPacket.ptb,sizeof(ReturnPacket.ptb));

 return( ReturnPacket.rc );
}

/*****************************************************************************/
/* TxGetThreadInfo()                                                         */
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
/*   none.                                                                   */
/*                                                                           */
/*****************************************************************************/
ULONG TxGetThreadInfo(THREADINFO *pBuffer)
{
 COMMAND cmd;
 ULONG   ntids;
 RET_GETTHREADINFO ReturnPacket;

 cmd.api  = GETTHREADINFO;
 cmd.len  = 0;
 RmtSend(DEFAULT_HANDLE, &cmd , sizeof(cmd) );
 RmtRecv(DEFAULT_HANDLE, &cmd, sizeof(cmd) );
 RmtRecv(DEFAULT_HANDLE, &ReturnPacket, cmd.len );

 ntids = ReturnPacket.ntids;
 memcpy(pBuffer, ReturnPacket.ti, ntids*sizeof(THREADINFO) );
 return(ntids);
}

/*****************************************************************************/
/* TxFreezeThread()                                                          */
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
/*   void                                                                    */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   none.                                                                   */
/*                                                                           */
/*****************************************************************************/
void  TxFreezeThread(ULONG tid )
{
 COMMAND cmd;

 cmd.api  = FREEZETHREAD;
 cmd.len  = sizeof(tid);

 RmtSend(DEFAULT_HANDLE, &cmd , sizeof(cmd) );
 RmtSend(DEFAULT_HANDLE, &tid , cmd.len );
 RmtRecv(DEFAULT_HANDLE, &cmd, sizeof(cmd) );
}
/*****************************************************************************/
/* TxThawThread()                                                            */
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
/*   void                                                                    */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   none.                                                                   */
/*                                                                           */
/*****************************************************************************/
void  TxThawThread(ULONG tid )
{
 COMMAND cmd;

 cmd.api  = THAWTHREAD;
 cmd.len  = sizeof(tid);

 RmtSend(DEFAULT_HANDLE, &cmd , sizeof(cmd) );
 RmtSend(DEFAULT_HANDLE, &tid , cmd.len );
 RmtRecv(DEFAULT_HANDLE, &cmd, sizeof(cmd) );
}

/*****************************************************************************/
/* TxGetCallStack()                                                          */
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
/*   void                                                                    */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   none.                                                                   */
/*                                                                           */
/*****************************************************************************/
ULONG TxGetCallStack(STACK_PARMS *pparms,
                     UCHAR       **ppActCSAtrs,
                     UINT        **ppActFrames,
                     UINT        **ppActFaddrs )
{
 COMMAND  cmd;
 char     buffer[  (sizeof(RET_GETCALLSTACK)-1 )   +
                   MAXAFRAMES*sizeof(UCHAR) +
                   MAXAFRAMES*sizeof(UINT) +
                   MAXAFRAMES*sizeof(UINT)
                ];

 ULONG    NActFrames;
 UCHAR   *pActCSAtrs = NULL;
 UINT    *pActFrames = NULL;
 UINT    *pActFaddrs = NULL;
 char    *p;
 int      nbytes;
 RET_GETCALLSTACK *pReturnPacket = (RET_GETCALLSTACK*)buffer;


 cmd.api  = GETCALLSTACK;
 cmd.len  = sizeof(STACK_PARMS);

 RmtSend(DEFAULT_HANDLE, &cmd  , sizeof(cmd) );
 RmtSend(DEFAULT_HANDLE, pparms, cmd.len );
 RmtRecv(DEFAULT_HANDLE, &cmd, sizeof(cmd) );
 RmtRecv(DEFAULT_HANDLE, buffer, cmd.len );


 NActFrames = pReturnPacket->NActFrames;
 if( NActFrames )
 {
  pActCSAtrs = Talloc(NActFrames*sizeof(UCHAR));
  pActFrames = Talloc(NActFrames*sizeof(UINT));
  pActFaddrs = Talloc(NActFrames*sizeof(UINT));

  p = buffer + pReturnPacket->pActCSAtrs;
  nbytes = NActFrames*sizeof(UCHAR);
  memcpy(pActCSAtrs,p,nbytes);

  p = buffer + pReturnPacket->pActFrames;
  nbytes = NActFrames*sizeof(UINT);
  memcpy(pActFrames,p,nbytes);

  p = buffer + pReturnPacket->pActFaddrs;
  nbytes = NActFrames*sizeof(UINT);
  memcpy(pActFaddrs,p,nbytes);

  *ppActCSAtrs = pActCSAtrs;
  *ppActFrames = pActFrames;
  *ppActFaddrs = pActFaddrs;
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
/*   none.                                                                   */
/*                                                                           */
/*****************************************************************************/
ULONG TxGetExeOrDllEntryOrExitPt( UINT mte )
{
 COMMAND cmd;
 ULONG   LoadAddr;

 cmd.api  = GETEXEORDLLENTRY;
 cmd.len  = sizeof(mte);

 RmtSend(DEFAULT_HANDLE, &cmd , sizeof(cmd) );
 RmtSend(DEFAULT_HANDLE, &mte , cmd.len );
 RmtRecv(DEFAULT_HANDLE, &cmd, sizeof(cmd) );
 RmtRecv(DEFAULT_HANDLE, &LoadAddr, cmd.len );

 return(LoadAddr);
}

/*****************************************************************************/
/* TxNormalQuit()                                                            */
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
APIRET TxNormalQuit( int AppTerminated, UINT mte, ULONG EntryPt )
{
 COMMAND cmd;
 TXRX_GETEXEORDLLENTRY ParmPacket;
 APIRET  rc;

 ParmPacket.AppTerminated = AppTerminated;
 ParmPacket.mte           = mte;
 ParmPacket.EntryPt       = EntryPt;

 cmd.api  = NORMALQUIT;
 cmd.len  = sizeof(ParmPacket);

 RmtSend(DEFAULT_HANDLE, &cmd , sizeof(cmd) );
 RmtSend(DEFAULT_HANDLE, &ParmPacket , cmd.len );
 RmtRecv(DEFAULT_HANDLE, &cmd, sizeof(cmd) );
 if(cmd.api != NORMALQUIT)
 {
  int   i;
  char *cp = (char*)&cmd;

  for( i=1; i<=sizeof(cmd); i++,cp++)
   printf("%c",*cp);

  AsyncFlushModem();
  exit(0);
 }
 RmtRecv(DEFAULT_HANDLE, &rc, cmd.len );

 return(rc);

}

/*****************************************************************************/
/* TxSetExecAddr()                                                           */
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
/*   rc                                                                      */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   none.                                                                   */
/*                                                                           */
/*****************************************************************************/
APIRET TxSetExecAddr( ULONG ExecAddr)
{
 COMMAND cmd;
 APIRET  rc;

 cmd.api  = SETEXECADDR;
 cmd.len  = sizeof(ExecAddr);

 RmtSend(DEFAULT_HANDLE, &cmd , sizeof(cmd) );
 RmtSend(DEFAULT_HANDLE, &ExecAddr , cmd.len );
 RmtRecv(DEFAULT_HANDLE, &cmd, sizeof(cmd) );
 RmtRecv(DEFAULT_HANDLE, &rc, cmd.len );

 return(rc);
}

/*****************************************************************************/
/* TxDefWps()                                                                */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Define the watch points in the server.                                  */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   pRegs    -  ->array of watch point definitions.                         */
/*   size     -  size of the block of debug register data.                   */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   rc                                                                      */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   none.                                                                   */
/*                                                                           */
/*****************************************************************************/
void TxDefWps( void *pRegs,int size )
{
 COMMAND cmd;
 TXRX_DEFWPS ParmPacket;

 cmd.api  = DEFWPS;
 cmd.len  = sizeof(ParmPacket);

 memcpy(&ParmPacket.regs,pRegs,size);
 ParmPacket.size = size;

 RmtSend(DEFAULT_HANDLE, &cmd , sizeof(cmd) );
 RmtSend(DEFAULT_HANDLE, &ParmPacket , cmd.len );
 RmtRecv(DEFAULT_HANDLE, &cmd, sizeof(cmd) );

}

/*****************************************************************************/
/* TxPutInWps()                                                              */
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
/*   none.                                                                   */
/*                                                                           */
/*****************************************************************************/
void TxPutInWps( ULONG *pIndexes )
{
 COMMAND cmd;
 RET_PUTINWPS ReturnPacket;

 cmd.api  = PUTINWPS;
 cmd.len  = 0;

 RmtSend(DEFAULT_HANDLE, &cmd , sizeof(cmd) );

 RmtRecv(DEFAULT_HANDLE, &cmd, sizeof(cmd) );
 RmtRecv(DEFAULT_HANDLE, &ReturnPacket,cmd.len);
 memcpy(pIndexes,ReturnPacket.indexes,cmd.len);
}

/*****************************************************************************/
/* TxPullOutWps()                                                            */
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
/*   none.                                                                   */
/*                                                                           */
/*****************************************************************************/
void TxPullOutWps( void )
{
 COMMAND cmd;

 cmd.api  = PULLOUTWPS;
 cmd.len  = 0;

 RmtSend(DEFAULT_HANDLE, &cmd , sizeof(cmd) );
 RmtRecv(DEFAULT_HANDLE, &cmd, sizeof(cmd) );
}

/*****************************************************************************/
/* TxGetMemoryBlock()                                                        */
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

static UCHAR *pbytes = NULL;
UCHAR *TxGetMemoryBlock( ULONG addr,int BytesWanted, int *pBytesObtained )
{
 COMMAND cmd;
 TXRX_GETDATABYTES  ParmPacket;
 RET_GETDATABYTES   ReturnPacket;

 cmd.api  = GETDATABYTES;
 cmd.len  = sizeof(ParmPacket);

 ParmPacket.addr        = addr;
 ParmPacket.BytesWanted = BytesWanted;

 RmtSend(DEFAULT_HANDLE, &cmd , sizeof(cmd) );
 RmtSend(DEFAULT_HANDLE, &ParmPacket , cmd.len );
 RmtRecv(DEFAULT_HANDLE, &cmd, sizeof(cmd) );
 RmtRecv(DEFAULT_HANDLE, &ReturnPacket,cmd.len );

 *pBytesObtained = ReturnPacket.BytesObtained;

 if( pbytes )
 {
  Tfree(pbytes);
  pbytes = NULL;
 }

 if( ReturnPacket.BytesObtained != 0 )
 {
  pbytes = Talloc( ReturnPacket.BytesObtained );
  memcpy(pbytes,ReturnPacket.bytes,ReturnPacket.BytesObtained);
 }
 return(pbytes);
}

/*****************************************************************************/
/* TxGetMemoryBlocks()                                                       */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Get a some blocks of memory data.                                       */
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
void *TxGetMemoryBlocks(void *pDefBlks, int   LengthOfDefBlk)
{
 COMMAND         cmd;
 int             LengthOfMemBlks;
 int            *pd;
 void           *pMemBlks;

 pd = (int *)pDefBlks;
 LengthOfMemBlks = *pd;

 cmd.api  = GETMEMBLKS;
 cmd.len  = LengthOfDefBlk;

 RmtSend(DEFAULT_HANDLE, &cmd , sizeof(cmd) );
 RmtSend(DEFAULT_HANDLE, pDefBlks , cmd.len );

 /****************************************************************************/
 /* - Allocate a block of data to receive the result block.                  */
 /****************************************************************************/
 pMemBlks = Talloc( LengthOfMemBlks );

 RmtRecv(DEFAULT_HANDLE, &cmd, sizeof(cmd) );
 RmtRecv(DEFAULT_HANDLE, pMemBlks,cmd.len );

 return(pMemBlks);
}

/*****************************************************************************/
/* TxGetMemoryBlocks()                                                       */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Get a some blocks of memory data.                                       */
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
void TxSetExceptions( UCHAR *pExceptionMap, int length )
{
 COMMAND         cmd;

 cmd.api  = SETXCPTNOTIFY;
 cmd.len  = length;

 RmtSend(DEFAULT_HANDLE, &cmd , sizeof(cmd) );
 RmtSend(DEFAULT_HANDLE, pExceptionMap , cmd.len );

 RmtRecv(DEFAULT_HANDLE, &cmd, sizeof(cmd) );
}

/*****************************************************************************/
/* TxSetExecThread()                                                         */
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
APIRET TxSetExecThread(ULONG *pExecAddr,PtraceBuffer *pptb, UINT tid)
{
 COMMAND cmd;
 RET_SETEXECTHREAD ReturnPacket;

 cmd.api  = SETEXECTHREAD;
 cmd.len  = sizeof(tid);

 RmtSend(DEFAULT_HANDLE, &cmd , sizeof(cmd) );
 RmtSend(DEFAULT_HANDLE, &tid , cmd.len );

 RmtRecv(DEFAULT_HANDLE, &cmd, sizeof(cmd) );
 RmtRecv(DEFAULT_HANDLE, &ReturnPacket, cmd.len );

 memcpy(pptb, &ReturnPacket.ptb, sizeof(PtraceBuffer) );
 *pExecAddr = ReturnPacket.ExecAddr;
 return( ReturnPacket.rc );
}

/*****************************************************************************/
/* TxWriteRegs()                                                             */
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
APIRET TxWriteRegs(ULONG *pExecAddr,PtraceBuffer *pptb)
{
 COMMAND cmd;
 RET_WRITEREGS ReturnPacket;

 cmd.api  = WRITEREGS;
 cmd.len  = sizeof(PtraceBuffer);

 RmtSend(DEFAULT_HANDLE, &cmd , sizeof(cmd) );
 RmtSend(DEFAULT_HANDLE, pptb , cmd.len );

 RmtRecv(DEFAULT_HANDLE, &cmd, sizeof(cmd) );
 RmtRecv(DEFAULT_HANDLE, &ReturnPacket, cmd.len );

 *pExecAddr = ReturnPacket.ExecAddr;
 return( ReturnPacket.rc );
}

/*****************************************************************************/
/* TxGetCoRegs()                                                             */
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
APIRET TxGetCoRegs(void *pCoproc_regs)
{
 COMMAND cmd;
 RET_GETCOREGS ReturnPacket;

 cmd.api  = GETCOREGS;
 cmd.len  = 0;

 RmtSend(DEFAULT_HANDLE, &cmd , sizeof(cmd) );

 RmtRecv(DEFAULT_HANDLE, &cmd, sizeof(cmd) );
 RmtRecv(DEFAULT_HANDLE, &ReturnPacket, cmd.len );

 memcpy( pCoproc_regs, &ReturnPacket.coproc_regs,sizeof(COPROCESSORREGS) );
 return( ReturnPacket.rc );
}

/*****************************************************************************/
/* TxTerminateESP()                                                          */
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
/*   none.                                                                   */
/*                                                                           */
/*****************************************************************************/
void TxTerminateESP( void )
{
 COMMAND cmd;

 cmd.api  = TERMINATEESP;
 cmd.len  = 0;
 RmtSend(DEFAULT_HANDLE, &cmd , sizeof(cmd) );
 RmtRecv(DEFAULT_HANDLE, &cmd, sizeof(cmd) );
}

/*****************************************************************************/
/* TxSetEspRunOpts()                                                      919*/
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Send run options to esp.                                                */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*  pEspRunOpts    ->to block of data containing flags and names             */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
void TxSetEspRunOpts( ESP_RUN_OPTS *pEspRunOpts)
{
 COMMAND cmd;

 cmd.api = SETESPRUNOPTS;
 cmd.len = sizeof(ESP_RUN_OPTS) + pEspRunOpts->NameBlockSize - 1;

 RmtSend(DEFAULT_HANDLE, &cmd , sizeof(cmd) );
 RmtSend(DEFAULT_HANDLE, pEspRunOpts , cmd.len );
}

/*****************************************************************************/
/* TxSendCtrlBreak()                                                      917*/
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Send a Ctrl-Break message to the probe.                                 */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   handle      the connection handle we're sending the ctrl_break to.      */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
void TxSendCtrlBreak( LHANDLE handle, USHORT pid )
{
 COMMAND cmd;

 cmd.api  = CTRL_BREAK;
 cmd.len  = sizeof(pid);

 RmtSend( handle, &cmd , sizeof(cmd) );
 RmtSend( handle, &pid , cmd.len     );
}

/*****************************************************************************/
/* TxStartQueListenThread()                                               919*/
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Tell the probe to start up a thread to listen for messages              */
/*   coming from dbg. This thread is only applicable to parallel             */
/*   connections.                                                            */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
void TxStartQueListenThread( void )
{
 COMMAND cmd;

 cmd.api  = START_QUE_LISTEN;
 cmd.len  = 0;

 RmtSend(DEFAULT_HANDLE, &cmd, sizeof(cmd) );
}

/*****************************************************************************/
/* TxStartEspQue()                                                        919*/
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Tell the probe to start up a que to handle esp messages.                */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
APIRET TxStartEspQue( void )
{
 COMMAND cmd;
 APIRET  rc = 0;

 cmd.api  = START_ESP_QUE;
 cmd.len  = 0;

 RmtSend(DEFAULT_HANDLE, &cmd, sizeof(cmd) );
 RmtRecv(DEFAULT_HANDLE, &cmd, sizeof(cmd) );
 RmtRecv(DEFAULT_HANDLE, &rc,  sizeof(rc) );
 return(rc);
}

/*****************************************************************************/
/* TxConnectEsp()                                                            */
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
void TxConnectEsp( LHANDLE handle, USHORT pid )
{
 COMMAND cmd;

 cmd.api = CONNECT_ESP;
 cmd.cmd = 0;
 cmd.len = sizeof(pid);

 RmtSend( handle, &cmd, sizeof(cmd) );
 RmtSend( handle, &pid, cmd.len );
 RmtRecv( handle, &cmd, sizeof(cmd) );
}

/*****************************************************************************/
/* TxSendKillListen()                                                        */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Send a command to kill the listen thread when connected via a           */
/*   parallel connection.                                                    */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   handle      the connection handle we're sending the command to.         */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
void TxSendKillListen( LHANDLE handle )
{
 COMMAND cmd;

 cmd.api  = KILL_LISTEN_THREAD;
 cmd.cmd  = 0;
 cmd.len  = 0;

 RmtSend( handle, &cmd , sizeof(cmd) );
}

/*****************************************************************************/
/* TxSelectSession()                                                         */
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
void TxSelectSession( void )
{
 COMMAND cmd;

 cmd.api  = SELECT_SESSION;
 cmd.cmd  = 0;
 cmd.len  = 0;

 RmtSend( DEFAULT_HANDLE, &cmd, sizeof(cmd) );
 RmtRecv( DEFAULT_HANDLE, &cmd, sizeof(cmd) );
}
