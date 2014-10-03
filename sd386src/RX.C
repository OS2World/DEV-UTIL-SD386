/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   rx.c                                                                    */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*  Execute x-server functions.                                              */
/*                                                                           */
/* History:                                                                  */
/*                                                                           */
/*   06/24/93 Created.                                                       */
/*                                                                           */
/*...                                                                        */
/*... 06/24/92  827   Joe   Add support for remote debug.                    */
/*... 05/05/94  919   Joe   Add child process support.                       */
/*...                                                                        */
/*****************************************************************************/
#include "all.h"

/*****************************************************************************/
/* RxFindExe()                                                               */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   This is server find EXE function.                                       */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   cmd        received command structure containing the length of          */
/*              the parameter packet.                                        */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   none.                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   none.                                                                   */
/*                                                                           */
/*****************************************************************************/
void RxFindExe(COMMAND cmd)
{
 char         *progname;
 ULONG         base;
 TXRX_FINDEXE  ParmPacket;
 RET_FINDEXE   ReturnPacket;

 memset(ReturnPacket.buffer,0,sizeof(ReturnPacket.buffer));
 RmtRecv(DEFAULT_HANDLE,&ParmPacket,cmd.len);
 base = (ULONG)&ParmPacket;
 progname = (char*)(base + (ULONG)ParmPacket.progname);


 ReturnPacket.rc = XSrvFindExe( progname,ReturnPacket.buffer,
                                sizeof(ReturnPacket.buffer) );

 cmd.api = FINDEXE;
 cmd.len = sizeof(APIRET) + strlen(ReturnPacket.buffer) + 1;
 RmtSend(DEFAULT_HANDLE,&cmd,sizeof(cmd));
 RmtSend(DEFAULT_HANDLE,&ReturnPacket,cmd.len );
}

/*****************************************************************************/
/* RxStartUser()                                                             */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Start( DosStartSession ) the debuggee.                                  */
/*                                                                           */
/*   The parameter packet for DosStartSession consists of:                   */
/*    - the STARTDATA structure +                                            */
/*    - PgmTitle string +                                                    */
/*    - PgmName  string +                                                    */
/*    - PgmInputs string.                                                    */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   cmd        received command structure containing the length of          */
/*              the parameter packet.                                        */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   none.                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   pUserEXE != NULL                                                        */
/*                                                                           */
/*****************************************************************************/
void RxStartUser(COMMAND cmd)
{

 TXRX_STARTUSER  ParmPacket;
 RET_STARTUSER   ReturnPacket;
 char           *pParmPacket;
 char           *pUserEXE;
 char           *pUserParms;
 USHORT          SessionType;
 ULONG           ErrorBufLen;

 /****************************************************************************/
 /* - receive the parm packet.                                               */
 /* - define the ptr to the exe. The string comes over as a length           */
 /*   prefixed z-string so we have to add a +1 to get past the length        */
 /*   prefix.                                                                */
 /* - define the ptr to the program parameters. This is also a length        */
 /*   prefixed z-string.                                                     */
 /****************************************************************************/
 pParmPacket = (char*)&ParmPacket;
 RmtRecv(DEFAULT_HANDLE,pParmPacket,cmd.len);

 pUserEXE    = pParmPacket + ParmPacket.pUserEXE + 1;

 pUserParms = NULL;
 if( ParmPacket.pUserParms != 0 )
  pUserParms  = pParmPacket + ParmPacket.pUserParms + 1;
 SessionType = ParmPacket.SessionType;

 ErrorBufLen = sizeof(ReturnPacket.ErrorBuf);

 memset( ReturnPacket.ErrorBuf, 0, ErrorBufLen );

 ReturnPacket.rc = EspStartUser( pUserEXE,
                                 pUserParms,
                                 SessionType,
                                &ReturnPacket.SessionID,
                                &ReturnPacket.ProcessID,
                                &ReturnPacket.ProcessType,
                                 ErrorBufLen,
                                 ReturnPacket.ErrorBuf);

 cmd.api = STARTUSER;

 cmd.len = sizeof(ReturnPacket) - sizeof(ReturnPacket.ErrorBuf);

 if( ReturnPacket.rc != 0 )
  cmd.len += strlen(ReturnPacket.ErrorBuf) + 1;

 RmtSend( DEFAULT_HANDLE, &cmd, sizeof(cmd) );
 RmtSend( DEFAULT_HANDLE, &ReturnPacket, cmd.len );
}

/*****************************************************************************/
/* RxGoInit()                                                                */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Initialize the EXE file for debugging.                                  */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   cmd        received command structure containing the length of          */
/*              the parameter packet.                                        */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   none.                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   none.                                                                   */
/*                                                                           */
/*****************************************************************************/
void RxGoInit(COMMAND cmd)
{
 TXRX_GOINIT  ParmPacket;
 RET_GOINIT   ReturnPacket;
 UINT        *pModuleLoadTable = NULL;
 int          ModuleLoadTableLength = 0;

 RmtRecv(DEFAULT_HANDLE,&ParmPacket,cmd.len);

 memset(&ReturnPacket,0,sizeof(ReturnPacket));
 ReturnPacket.ptb.Pid = ParmPacket.Pid;
 ReturnPacket.ptb.Tid = ParmPacket.Sid;

 ReturnPacket.rc = XSrvGoInit(&ReturnPacket.ptb,
                              &ReturnPacket.ExecAddr,
                              &pModuleLoadTable,
                              &ModuleLoadTableLength);

 cmd.api = GOINIT;
 cmd.len = sizeof(APIRET) + sizeof(PtraceBuffer);

 RmtSend(DEFAULT_HANDLE,&cmd,sizeof(cmd));
 RmtSend(DEFAULT_HANDLE,&ReturnPacket,cmd.len );

 cmd.len = ModuleLoadTableLength;
 RmtSend(DEFAULT_HANDLE,&cmd,sizeof(cmd));
 RmtSend(DEFAULT_HANDLE,pModuleLoadTable,cmd.len );

 /****************************************************************************/
 /* Free the storage allocated for the module table.                         */
 /****************************************************************************/
 if(pModuleLoadTable)
  Tfree(pModuleLoadTable);
}

/*****************************************************************************/
/* RxGoStep()                                                                */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Step the deguggee.                                                      */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   cmd        received command structure containing the length of          */
/*              the parameter packet.                                        */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   none.                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   none.                                                                   */
/*                                                                           */
/*****************************************************************************/
void RxGoStep(COMMAND cmd)
{
 TXRX_GOSTEP ParmPacket;
 RET_GOSTEP  ReturnPacket;
 UINT       *pModuleLoadTable = NULL;
 int         ModuleLoadTableLength = 0;

 RmtRecv(DEFAULT_HANDLE,&ParmPacket,cmd.len);
 ReturnPacket.ptb = ParmPacket.ptb;

 if(SerialParallel() == SERIAL)
 {
  SerialConnect( DISCONNECT, GetEspProcessID(), _ESP, SendMsgToEspQue );
 }
 ReturnPacket.rc=XSrvGoStep(&ReturnPacket.ptb,
                            &ReturnPacket.ExecAddr,
                            &pModuleLoadTable,
                            &ModuleLoadTableLength,
                            ParmPacket.ExecAddr,
                            ParmPacket.ExecAddrlo,
                            ParmPacket.ExecAddrhi,
                            ParmPacket.how,
                            ParmPacket.ExecFlags );

 if(SerialParallel() == SERIAL)                                         /*919*/
  SerialConnect( CONNECT_WAIT, GetEspProcessID(), _ESP, SendMsgToEspQue );
 cmd.api = GOSTEP;
 cmd.len = sizeof(ReturnPacket);

 RmtSend(DEFAULT_HANDLE,&cmd,sizeof(cmd));
 RmtSend(DEFAULT_HANDLE,&ReturnPacket,cmd.len );

 cmd.len = ModuleLoadTableLength;
 RmtSend(DEFAULT_HANDLE,&cmd,sizeof(cmd));
 RmtSend(DEFAULT_HANDLE,pModuleLoadTable,cmd.len );

 /****************************************************************************/
 /* Free the storage allocated for the module table.                         */
 /****************************************************************************/
 if(pModuleLoadTable)
  Tfree(pModuleLoadTable);
}

/*****************************************************************************/
/* RxGoFast()                                                                */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Run  the deguggee.                                                      */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   cmd        received command structure containing the length of          */
/*              the parameter packet.                                        */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   none.                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   none.                                                                   */
/*                                                                           */
/*****************************************************************************/
void RxGoFast(COMMAND cmd)
{
 TXRX_GOFAST ParmPacket;
 RET_GOFAST  ReturnPacket;
 UINT       *pModuleLoadTable = NULL;
 int         ModuleLoadTableLength = 0;

 RmtRecv(DEFAULT_HANDLE,&ParmPacket,cmd.len);
 ReturnPacket.ptb = ParmPacket.ptb;

 if(SerialParallel() == SERIAL)                                         /*919*/
 {
  SerialConnect( DISCONNECT, GetEspProcessID(), _ESP, SendMsgToEspQue );
 }
 ReturnPacket.rc=XSrvGoFast(&ReturnPacket.ptb,
                            &ReturnPacket.ExecAddr,
                            &pModuleLoadTable,
                            &ModuleLoadTableLength,
                            ParmPacket.ExecAddr,
                            ParmPacket.ExecFlags);
 if(SerialParallel() == SERIAL)                                         /*919*/
  SerialConnect( CONNECT_WAIT, GetEspProcessID(), _ESP, SendMsgToEspQue );
 cmd.api = GOFAST;
 cmd.len = sizeof(ReturnPacket);

 RmtSend(DEFAULT_HANDLE,&cmd,sizeof(cmd));
 RmtSend(DEFAULT_HANDLE,&ReturnPacket,cmd.len );

 cmd.len = ModuleLoadTableLength;
 RmtSend(DEFAULT_HANDLE,&cmd,sizeof(cmd));
 RmtSend(DEFAULT_HANDLE,pModuleLoadTable,cmd.len );
 /****************************************************************************/
 /* Free the storage allocated for the module table.                         */
 /****************************************************************************/
 if(pModuleLoadTable)
 {
  Tfree(pModuleLoadTable);
 }
}

/*****************************************************************************/
/* RxGoEntry()                                                               */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Run to the initial entry point.                                         */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   cmd        received command structure containing the length of          */
/*              the parameter packet.                                        */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   none.                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   none.                                                                   */
/*                                                                           */
/*****************************************************************************/
void RxGoEntry(COMMAND cmd)
{
 TXRX_GOFAST ParmPacket;
 RET_GOFAST  ReturnPacket;
 UINT       *pModuleLoadTable = NULL;
 int         ModuleLoadTableLength = 0;

 RmtRecv(DEFAULT_HANDLE,&ParmPacket,cmd.len);
 ReturnPacket.ptb = ParmPacket.ptb;

 ReturnPacket.rc=XSrvGoFast(&ReturnPacket.ptb,
                            &ReturnPacket.ExecAddr,
                            &pModuleLoadTable,
                            &ModuleLoadTableLength,
                            ParmPacket.ExecAddr,
                            ParmPacket.ExecFlags);

 cmd.api = GOENTRY;
 cmd.len = sizeof(ReturnPacket);

 RmtSend(DEFAULT_HANDLE,&cmd,sizeof(cmd));
 RmtSend(DEFAULT_HANDLE,&ReturnPacket,cmd.len );

 cmd.len = ModuleLoadTableLength;
 RmtSend(DEFAULT_HANDLE,&cmd,sizeof(cmd));
 RmtSend(DEFAULT_HANDLE,pModuleLoadTable,cmd.len );
 /****************************************************************************/
 /* Free the storage allocated for the module table.                         */
 /****************************************************************************/
 if(pModuleLoadTable)
  Tfree(pModuleLoadTable);
}

/*****************************************************************************/
/* RxDefBrk()                                                                */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Define a breakpoint.                                                    */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   cmd        received command structure containing the length of          */
/*              the parameter packet.                                        */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   none.                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   none.                                                                   */
/*                                                                           */
/*****************************************************************************/
void   RxDefBrk(COMMAND cmd)
{
 TXRX_DEFBRK ParmPacket;

 RmtRecv(DEFAULT_HANDLE,&ParmPacket,cmd.len);
 XSrvDefBrk( ParmPacket.where );
}
/*****************************************************************************/
/* RxUndBrk()                                                                */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Undefine a breakpoint.                                                  */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   cmd        received command structure containing the length of          */
/*              the parameter packet.                                        */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   none.                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   none.                                                                   */
/*                                                                           */
/*****************************************************************************/
void   RxUndBrk(COMMAND cmd)
{
 TXRX_UNDBRK ParmPacket;

 RmtRecv(DEFAULT_HANDLE,&ParmPacket,cmd.len);
 XSrvUndBrk( ParmPacket.where );
}

/*****************************************************************************/
/* RxPutInBrk()                                                              */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Put in the CC.                                                          */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   cmd        received command structure containing the length of          */
/*              the parameter packet.                                        */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   none.                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   none.                                                                   */
/*                                                                           */
/*****************************************************************************/
void   RxPutInBrk(COMMAND cmd)
{
 TXRX_PUTINBRK ParmPacket;

 RmtRecv(DEFAULT_HANDLE,&ParmPacket,cmd.len);
 XSrvPutInBrk( ParmPacket.where );
}

/*****************************************************************************/
/* RxPullOutBrk()                                                            */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Pull out the CC.                                                        */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   cmd        received command structure containing the length of          */
/*              the parameter packet.                                        */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   none.                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   none.                                                                   */
/*                                                                           */
/*****************************************************************************/
void   RxPullOutBrk(COMMAND cmd)
{
 TXRX_PULLOUTBRK ParmPacket;

 RmtRecv(DEFAULT_HANDLE,&ParmPacket,cmd.len);
 XSrvPullOutBrk( ParmPacket.where );
}

/*****************************************************************************/
/* RxInsertAllBrk()                                                          */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Put in CCs for all defined breakpoints.                                 */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
void   RxInsertAllBrk( void )
{
 XSrvInsertAllBrk( );
}

/*****************************************************************************/
/* RxRemoveAllBrk()                                                          */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Remove all CCs for all defined breakpoints.                             */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
void   RxRemoveAllBrk( void )
{
 XSrvRemoveAllBrk( );
}

/*****************************************************************************/
/* RxDosDebug()                                                              */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   X-server DosDebug API handler.                                          */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   cmd        received command structure containing the length of          */
/*              the parameter packet.                                        */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   none.                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   none.                                                                   */
/*                                                                           */
/*****************************************************************************/
void RxDosDebug(COMMAND cmd)
{
 TXRX_DOSDEBUG ParmPacket;
 RET_DOSDEBUG  ReturnPacket;
 long          DebugCmd;

 RmtRecv(DEFAULT_HANDLE,&ParmPacket,cmd.len);

 ReturnPacket.ptb = ParmPacket.ptb;

 DebugCmd = cmd.cmd;
 switch( DebugCmd )
 {
  case DBG_C_ReadMemBuf:
  case DBG_C_ThrdStat:
   ReturnPacket.ptb.Buffer = (ULONG)&ReturnPacket.buffer;
   break;


  case DBG_C_WriteMemBuf:
   ReturnPacket.ptb.Buffer = (ULONG)&ParmPacket.buffer;
   break;

 }

 ReturnPacket.rc = DosDebug( &ReturnPacket.ptb );

 cmd.len = sizeof(ReturnPacket.rc) + sizeof(ReturnPacket.ptb);
 if( (ReturnPacket.rc == 0 ) &&
     ( (DebugCmd == DBG_C_ReadMemBuf) || (DebugCmd == DBG_C_ThrdStat) )
   )
 {
  cmd.len += ParmPacket.ptb.Len;
 }

 cmd.api = DOSDEBUG;
 RmtSend(DEFAULT_HANDLE,&cmd,sizeof(cmd));
 RmtSend(DEFAULT_HANDLE,&ReturnPacket,cmd.len);
}

/*****************************************************************************/
/* RxGetThreadInfo()                                                         */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*    Get the thread info for the debuggee.                                  */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   cmd        received command structure containing the length of          */
/*              the parameter packet.                                        */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   none                                                                    */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   none.                                                                   */
/*                                                                           */
/*****************************************************************************/
void RxGetThreadInfo(COMMAND cmd)
{
 ULONG   ntids;
 RET_GETTHREADINFO ReturnPacket;

 ntids = XSrvGetThreadInfo((THREADINFO *)&ReturnPacket.ti);
 ReturnPacket.ntids = ntids;

 cmd.api  = GETTHREADINFO;
 cmd.len  = sizeof(ReturnPacket.ntids) + ntids*sizeof(THREADINFO);

 RmtSend(DEFAULT_HANDLE, &cmd , sizeof(cmd) );
 RmtSend(DEFAULT_HANDLE,&ReturnPacket,cmd.len);
}

/*****************************************************************************/
/* RxFreezeThread()                                                          */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Freeze a thread.                                                        */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   cmd        received command structure containing the length of          */
/*              the parameter packet.                                        */
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
void  RxFreezeThread(COMMAND cmd)
{
 ULONG   tid;

 RmtRecv(DEFAULT_HANDLE,&tid,cmd.len);
 XSrvFreezeThread(tid);
 cmd.api  = FREEZETHREAD;
 RmtSend(DEFAULT_HANDLE, &cmd , sizeof(cmd) );
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
/*   cmd        received command structure containing the length of          */
/*              the parameter packet.                                        */
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
void  RxThawThread(COMMAND cmd)
{
 ULONG   tid;

 RmtRecv(DEFAULT_HANDLE,&tid,cmd.len);
 XSrvThawThread(tid);
 cmd.api  = THAWTHREAD;
 RmtSend(DEFAULT_HANDLE, &cmd , sizeof(cmd) );
}
/*****************************************************************************/
/* RxGetCallStack()                                                          */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Get the call stack tables.                                              */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   cmd        received command structure containing the length of          */
/*              the parameter packet.                                        */
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
void RxGetCallStack(COMMAND cmd )
{
 ULONG    NActFrames = 0;
 UCHAR   *pActCSAtrs = NULL;
 UINT    *pActFrames = NULL;
 UINT    *pActFaddrs = NULL;
 int      size;
 int      nbytes;
 char    *ptable;
 TXRX_GETCALLSTACK ParmPacket;
 RET_GETCALLSTACK  *pReturnPacket;

 RmtRecv(DEFAULT_HANDLE, &ParmPacket, cmd.len );

 NActFrames = XSrvGetCallStack( &ParmPacket.parms,
                                &pActCSAtrs,
                                &pActFrames,
                                &pActFaddrs );


 size = sizeof(RET_GETCALLSTACK) - 1 +
        NActFrames*(sizeof(UCHAR) + sizeof(UINT) + sizeof(UINT) );

 pReturnPacket = Talloc(size);

 pReturnPacket->NActFrames = NActFrames;

 ptable = pReturnPacket->buffer;
 nbytes = NActFrames*sizeof(UCHAR);
 memcpy(ptable,pActCSAtrs,nbytes);
 pReturnPacket->pActCSAtrs = (ULONG)(ptable - (char*)pReturnPacket);

 ptable = ptable + nbytes;
 nbytes = NActFrames*sizeof(UINT);
 memcpy(ptable,pActFrames,nbytes);
 pReturnPacket->pActFrames = (ULONG)(ptable - (char*)pReturnPacket);

 ptable = ptable + nbytes;
 nbytes = NActFrames*sizeof(UINT);
 memcpy(ptable,pActFaddrs,nbytes);
 pReturnPacket->pActFaddrs = (ULONG)(ptable - (char*)pReturnPacket);

 if(pActCSAtrs) Tfree(pActCSAtrs);
 if(pActFrames) Tfree(pActFrames);
 if(pActFaddrs) Tfree(pActFaddrs);

 cmd.len = size;
 RmtSend(DEFAULT_HANDLE, &cmd  , sizeof(cmd) );
 RmtSend(DEFAULT_HANDLE, pReturnPacket, cmd.len );

 Tfree(pReturnPacket);
}

/*****************************************************************************/
/* RxGetExeOrDllEntryOrExitPt()                                              */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Get the entry point for a dll or exe.                                   */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   cmd        received command structure containing the length of          */
/*              the parameter packet.                                        */
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
void RxGetExeOrDllEntryOrExitPt( COMMAND cmd )
{
 UINT  mte;
 ULONG EntryPtAddr = 0;

 RmtRecv(DEFAULT_HANDLE, &mte, cmd.len );

 EntryPtAddr = XSrvGetExeOrDllEntryOrExitPt( mte );

 cmd.api  = GETEXEORDLLENTRY;
 cmd.len  = sizeof(EntryPtAddr);

 RmtSend(DEFAULT_HANDLE, &cmd , sizeof(cmd) );
 RmtSend(DEFAULT_HANDLE, &EntryPtAddr, cmd.len );
}

/*****************************************************************************/
/* RxNormalQuit()                                                            */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Terminate the debuggee process.                                         */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   cmd        received command structure containing the length of          */
/*              the parameter packet.                                        */
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
void RxNormalQuit( COMMAND cmd )
{
 TXRX_GETEXEORDLLENTRY ParmPacket;

 RmtRecv(DEFAULT_HANDLE, &ParmPacket, cmd.len );

 XSrvNormalQuit( ParmPacket.AppTerminated,
                 ParmPacket.mte,
                 ParmPacket.EntryPt );
}

/*****************************************************************************/
/* RxSetExecAddr()                                                           */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Set the exec address for the current thread.                            */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   cmd        received command structure containing the length of          */
/*              the parameter packet.                                        */
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
void RxSetExecAddr( COMMAND cmd )
{
 ULONG ExecAddr;
 APIRET  rc;

 RmtRecv(DEFAULT_HANDLE, &ExecAddr, cmd.len );
 rc = XSrvSetExecAddr( ExecAddr );

 cmd.api  = SETEXECADDR;
 cmd.len  = sizeof(rc);

 RmtSend(DEFAULT_HANDLE, &cmd , sizeof(cmd) );
 RmtSend(DEFAULT_HANDLE, &rc , cmd.len );
}

/*****************************************************************************/
/* RxDefWps()                                                                */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Define the watch points in the x-server.                                */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   cmd        received command structure containing the length of          */
/*              the parameter packet.                                        */
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
void RxDefWps( COMMAND cmd )
{
 TXRX_DEFWPS ParmPacket;

 RmtRecv(DEFAULT_HANDLE, &ParmPacket, cmd.len );
 XSrvDefWps( &ParmPacket.regs, ParmPacket.size);

 cmd.api  = DEFWPS;
 cmd.len  = 0;

 RmtSend(DEFAULT_HANDLE, &cmd , sizeof(cmd) );
}

/*****************************************************************************/
/* RxPutInWps()                                                              */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Put in the watch points.                                                */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   cmd        received command structure containing the length of          */
/*              the parameter packet.                                        */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   none.                                                                   */
/*                                                                           */
/*****************************************************************************/
void RxPutInWps( COMMAND cmd )
{
 RET_PUTINWPS ReturnPacket;


 XSrvPutInWps( ReturnPacket.indexes );

 cmd.api  = PUTINWPS;
 cmd.len  = sizeof(ReturnPacket);

 RmtSend(DEFAULT_HANDLE, &cmd , sizeof(cmd) );
 RmtSend(DEFAULT_HANDLE, &ReturnPacket, cmd.len );
}

/*****************************************************************************/
/* RxPullOutWps()                                                            */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Pull out the watch points.                                              */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   cmd        received command structure containing the length of          */
/*              the parameter packet.                                        */
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
void RxPullOutWps( COMMAND cmd )
{
 XSrvPullOutWps( );

 cmd.api  = PUTINWPS;
 cmd.len  = 0;

 RmtSend(DEFAULT_HANDLE, &cmd , sizeof(cmd) );
}

/*****************************************************************************/
/* RxGetDataBytes()                                                          */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Get a block of data.                                                    */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   cmd        received command structure containing the length of          */
/*              the parameter packet.                                        */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   BytesWanted <= 16.                                                      */
/*                                                                           */
/*****************************************************************************/
void RxGetDataBytes( COMMAND cmd )
{
 TXRX_GETDATABYTES ParmPacket;
 RET_GETDATABYTES  ReturnPacket;
 int               BytesWanted;
 int               BytesObtained;
 UCHAR            *pbytes;
 ULONG             addr;

 RmtRecv(DEFAULT_HANDLE, &ParmPacket, cmd.len );

 addr        = ParmPacket.addr;
 BytesWanted = ParmPacket.BytesWanted;

 pbytes = XSrvGetMemoryBlock( addr,BytesWanted, &BytesObtained );

 cmd.api  = GETDATABYTES;
 cmd.len = sizeof(ReturnPacket.BytesObtained) + BytesObtained;

 ReturnPacket.BytesObtained = BytesObtained;
 if(pbytes && ( BytesObtained != 0 ) )
  memcpy(ReturnPacket.bytes,pbytes,BytesObtained);

 RmtSend(DEFAULT_HANDLE, &cmd , sizeof(cmd) );
 RmtSend(DEFAULT_HANDLE, &ReturnPacket, cmd.len );
}

/*****************************************************************************/
/* RxGetMemBlocks()                                                          */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Get some memory blocks.                                                 */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   cmd        received command structure containing the length of          */
/*              the parameter packet.                                        */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
void RxGetMemBlocks( COMMAND cmd )
{
 void  *pDefBlks = NULL;
 void  *pMemBlks = NULL;
 int    MemBlkSize;

 pDefBlks = Talloc( cmd.len );

 RmtRecv(DEFAULT_HANDLE, pDefBlks, cmd.len );
 pMemBlks = XSrvGetMemoryBlocks( pDefBlks, &MemBlkSize );

 cmd.api = GETMEMBLKS;
 cmd.len = MemBlkSize;

 RmtSend(DEFAULT_HANDLE, &cmd , sizeof(cmd) );
 RmtSend(DEFAULT_HANDLE, pMemBlks, cmd.len );
 Tfree( pMemBlks );
}

/*****************************************************************************/
/* RxSetExceptions()                                                         */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Set the exception notify/nonotify map.                                  */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   cmd        received command structure containing the length of          */
/*              the parameter packet.                                        */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
void RxSetExceptions( COMMAND cmd )
{
 UCHAR XcptMap[MAXEXCEPTIONS+1];
 int   length;

 length = cmd.len;

 RmtRecv(DEFAULT_HANDLE, XcptMap, length );
 XSrvSetExceptions( XcptMap, length );

 cmd.api = SETXCPTNOTIFY;
 cmd.len = 0;

 RmtSend(DEFAULT_HANDLE, &cmd , sizeof(cmd) );
}
/*****************************************************************************/
/* RxSetExecThread()                                                         */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Set execution context to a specified thread.                            */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   cmd        received command structure containing the length of          */
/*              the parameter packet.                                        */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   none.                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   none.                                                                   */
/*                                                                           */
/*****************************************************************************/
void RxSetExecThread(COMMAND cmd)
{
 UINT tid;
 RET_SETEXECTHREAD ReturnPacket;

 RmtRecv(DEFAULT_HANDLE,&tid,cmd.len);

 memset(&ReturnPacket,0,sizeof(ReturnPacket));

 ReturnPacket.rc = XSrvSetExecThread(&ReturnPacket.ExecAddr,
                                     &ReturnPacket.ptb,
                                     tid);

 cmd.api = SETEXECTHREAD;
 cmd.len = sizeof(ReturnPacket);

 RmtSend(DEFAULT_HANDLE,&cmd,sizeof(cmd));
 RmtSend(DEFAULT_HANDLE,&ReturnPacket,cmd.len );

}

/*****************************************************************************/
/* RxWriteRegs()                                                             */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Write the registers to the app.                                         */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   cmd        received command structure containing the length of          */
/*              the parameter packet.                                        */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   none.                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   none.                                                                   */
/*                                                                           */
/*****************************************************************************/
void RxWriteRegs(COMMAND cmd)
{
 PtraceBuffer ptb;
 RET_WRITEREGS ReturnPacket;

 RmtRecv(DEFAULT_HANDLE, &ptb,cmd.len);

 ReturnPacket.rc = XSrvWriteRegs(&ReturnPacket.ExecAddr,&ptb);

 cmd.api = WRITEREGS;
 cmd.len = sizeof(ReturnPacket);

 RmtSend(DEFAULT_HANDLE,&cmd,sizeof(cmd));
 RmtSend(DEFAULT_HANDLE,&ReturnPacket,cmd.len );

}

/*****************************************************************************/
/* RxGetCoRegs()                                                             */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Get the co-processor registers.                                         */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   cmd        received command structure containing the length of          */
/*              the parameter packet.                                        */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   none.                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   none.                                                                   */
/*                                                                           */
/*****************************************************************************/
void RxGetCoRegs(COMMAND cmd)
{
 RET_GETCOREGS ReturnPacket;

 ReturnPacket.rc = XSrvGetCoRegs(&ReturnPacket.coproc_regs);

 cmd.api = GETCOREGS;
 cmd.len = sizeof(ReturnPacket);

 RmtSend(DEFAULT_HANDLE,&cmd,sizeof(cmd));
 RmtSend(DEFAULT_HANDLE,&ReturnPacket,cmd.len );
}

/*****************************************************************************/
/* RxSetEspRunOpts()                                                      919*/
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Receive the esp run options.                                            */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   cmd        received command structure containing the length of          */
/*              the parameter packet.                                        */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
void RxSetEspRunOpts( COMMAND cmd )
{
 ESP_RUN_OPTS *pOpts = NULL;

 pOpts = (ESP_RUN_OPTS*)Talloc(cmd.len);
 RmtRecv(DEFAULT_HANDLE, pOpts, cmd.len );
 EspSetEspRunOpts( pOpts);
 if(pOpts) Tfree(pOpts);
}

/*****************************************************************************/
/* RxStartEspQue()                                                        919*/
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Start up a que to manage esp messages.                                  */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   cmd        received command structure containing the length of          */
/*              the parameter packet.                                        */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   rc                                                                      */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
void RxStartEspQue( COMMAND cmd )
{
 APIRET rc;

 rc = StartEspQue( );

 cmd.api  = START_ESP_QUE;
 cmd.len  = sizeof(rc);

 RmtSend(DEFAULT_HANDLE, &cmd , sizeof(cmd) );
 RmtSend(DEFAULT_HANDLE, &rc  , cmd.len );
}
