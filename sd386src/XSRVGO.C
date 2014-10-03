/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   srvrgo.c                                                             827*/
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*  Server execution functions.                                              */
/*                                                                           */
/* History:                                                                  */
/*                                                                           */
/*   05/04/93 Created.                                                       */
/*                                                                           */
/*...Release 1.04 (04/30/93)                                                 */
/*...                                                                        */
/*... 05/04/93  822   Joe       Add mte table handling.                      */
/*... 09/16/93  901   Joe       Add code to handle resource interlock error. */
/*...                                                                        */
/*****************************************************************************/
#include "all.h"

/*****************************************************************************/
/* limbo variables. do something with them.                                  */
/*****************************************************************************/

UCHAR  AppRegsZapped = 0;

/*---------------------------------------------------------------------------*/
static int xcpt_continue_flag;
static int Error_Interrupt_Flag = FALSE;

static ULONG EspProcessID;
static ULONG EspSessionID;
static BOOL  ChildTerminatedFlag;

ULONG GetEspProcessID( void ) {return(EspProcessID);}
ULONG GetEspSessionID( void ) {return(EspSessionID);}
BOOL  IsChildTerminated( void ) {return(ChildTerminatedFlag);}
/*****************************************************************************/
/* XSrvGoInit()                                                              */
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
APIRET XSrvGoInit( PtraceBuffer  *pptb,
                   ULONG         *pExecAddr,
                   UINT         **ppModuleLoadTable,
                   int           *pModuleTableLength)
{
 APIRET  rc;
 UINT   *pModuleList = NULL;

 /****************************************************************************/
 /* - This flag is used for terminating a child probe                        */
 /****************************************************************************/
 ChildTerminatedFlag = FALSE;

 /****************************************************************************/
 /* - Set the process id and the session id of the debuggee.                 */
 /* - Make a call to get the session id when the probe is remote.            */
 /* - pptb->Tid was used as a carrier for the sessionid...set it back to 0.  */
 /****************************************************************************/
 EspProcessID = pptb->Pid;
 EspSessionID = pptb->Tid;

 pptb->Tid    = 0;

#ifdef __ESP__
/*---------------------------------------------------------------------------*/
 {
  ALLPIDS *p;
  TIB     *pTib;
  PIB     *pPib;
  USHORT   EspPid;
  ULONG    sid;
  USHORT   mte;

  /***************************************************************************/
  /* - Update shared memory structure with pid and sid that this probe       */
  /*   will be debugging.                                                    */
  /***************************************************************************/
  DosGetInfoBlocks(&pTib,&pPib);
  EspPid = (USHORT)pPib->pib_ulpid;
  p = GetEspPid( EspPid );
  p->pid = EspProcessID;
  if( GetSessionID(EspProcessID, &sid) != 0 )
   return(1);
  EspSessionID = p->sid = sid;

  /***************************************************************************/
  /* - Get the mte for this process.                                         */
  /***************************************************************************/
  GetProcessMte( EspProcessID, &mte );
  p->mte = mte;
 }
/*---------------------------------------------------------------------------*/
#endif

 /****************************************************************************/
 /* Connect to the debuggee. The Pid is contained in the ptrace buffer.      */
 /****************************************************************************/
 pptb->Cmd = DBG_C_Connect;
 pptb->Value = DBG_L_386;
 pptb->Addr  = 1;                                                       /*901*/
 rc = DosDebug(pptb);
 if(rc)
  return(rc);

 if( pptb->Cmd == DBG_N_Error )
  return(1);

 /****************************************************************************/
 /* This code will build a module table for dll loads and frees. ll other    */
 /* notifications are simply returned back to the caller.                    */
 /****************************************************************************/
 for(;;)
 {
  pptb->Cmd = DBG_C_Stop;
  rc = DosDebug(pptb);
  if(rc || (pptb->Cmd == DBG_N_Error) )
   goto fini;
  switch( pptb->Cmd )
  {
   case DBG_N_Success:
    goto fini;

   case DBG_N_ModuleLoad:
   AddToModuleLoadTable(&pModuleList, pptb->Pid, pptb->Value);
   break;

   case DBG_N_ModuleFree:
    AddFreeToModuleLoadTable(&pModuleList, pptb->Value);
    break;

   case DBG_N_ThreadCreate:
    break;

   default:
    goto fini;
  }
 }
fini:

 if( pptb->CSAtr )
  *pExecAddr = pptb->EIP;
 else
  *pExecAddr =  Sys_SelOff2Flat(pptb->CS,LoFlat(pptb->EIP));

 /****************************************************************************/
 /* Pack the module table and get out.                                       */
 /****************************************************************************/
 if( pModuleList )
  *ppModuleLoadTable = PackModuleLoadTable(pModuleList , pModuleTableLength);
 return(rc);
}

/*****************************************************************************/
/* XSrvRestartAppl()                                                         */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Application execution.                                                  */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   pptb               -> to the ptrace buffer.                             */
/*   pExecAddr          where to put the current exec address.               */
/*   ppModuleLoadTable  where to put a ptr to the ModuleLoadTable.           */
/*   pModuleTableLength where to put the length of the ModuleLoadtable.      */
/*   ExecFlags          decision control flags.                              */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   rc         Trap code.                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   pptb->Pid is the correct process id.                                    */
/*   pptb->Cmd = DBG_C_Go,DBG_C_Step, or DBG_C_Continue.                     */
/*                                                                           */
/*****************************************************************************/
APIRET XSrvRestartAppl( PtraceBuffer  *pptb,
                        ULONG         *pExecAddr,
                        UINT         **ppModuleLoadTable,
                        int           *pModuleTableLength,
                        int           ExecFlags   )
{
 APIRET rc;
 ULONG  exception;
 int    n;
 UINT   *pModuleList = NULL;
 ULONG  DbgCmd = pptb->Cmd;
 ULONG  Pid    = pptb->Pid;

 /****************************************************************************/
 /* - reset this flag which is used at quit/restart time to determine        */
 /*   how to terminate the app.                                              */
 /****************************************************************************/
 Error_Interrupt_Flag = FALSE;

RestartApplication:
 /****************************************************************************/
 /* - Continue execution after an exception, or                              */
 /* - Call DosDebug with one of the following commands:                      */
 /*     - DBG_C_Go.                                                          */
 /*     - DBG_C_Step.                                                        */
 /****************************************************************************/
 if (xcpt_continue_flag == 1)
 {
  pptb->Cmd=DBG_C_Continue;
  pptb->Value=XCPT_CONTINUE_SEARCH;
  xcpt_continue_flag = 0;
 }
 else
  pptb->Cmd = DbgCmd;

 pptb->Pid = Pid;
 rc = DosDebug( pptb );
 /****************************************************************************/
 /* We will get this error when processing Signals.                          */
 /****************************************************************************/
 if( (rc == ERROR_INTERRUPT) || ( (rc==0) && (pptb->Cmd > 0) ) )
 {
  pptb->Pid = Pid;
  pptb->Cmd = DBG_C_ReadReg;
  DosDebug( pptb);
  rc = 0;
  pptb->Cmd = DBG_N_AsyncStop;
  Error_Interrupt_Flag = TRUE;
 }

 if ( rc != 0 )
  return( TRAP_ABEND );

 switch(pptb->Cmd)
 {
  case DBG_N_ProcTerm:                  /* -6                                */
  case DBG_N_Success:                   /*  0                                */
   ChildTerminatedFlag = TRUE;
   rc = TRAP_EXIT;
   break;

  case DBG_N_Error:
   if(pptb->Value == ERROR_EXCL_SEM_ALREADY_OWNED)                      /*901*/
    goto RestartApplication;                                            /*901*/
   /**************************************************************************/
   /* - test for a resource interlock kickout by testing for a thread        */
   /*   status on thread 1. If we get an error, then we've been kicked       */
   /*   out of debugging. This happens on OS/2 2.1 GA and was fixed by       */
   /*   APAR PJ09240.                                                        */
   /**************************************************************************/
   {                                                                    /*901*/
    PtraceBuffer ptb;                                                   /*901*/
    ULONG        buffer;                                                /*901*/
                                                                        /*901*/
    ptb.Pid = pptb->Pid;                                                /*901*/
    ptb.Cmd = DBG_C_ThrdStat;                                           /*901*/
    ptb.Tid = 1;                                                        /*901*/
    ptb.Buffer = (ULONG)&buffer;                                        /*901*/
    ptb.Len = sizeof( buffer);                                          /*901*/
    rc = DosDebug( &ptb);                                               /*901*/
    if( ptb.Cmd == DBG_N_Error )                                        /*901*/
    {                                                                   /*901*/
     *pExecAddr = pptb->EIP;                                            /*901*/
     rc = TRAP_INTERLOCK;                                               /*901*/
     return(rc);                                                        /*901*/
    }                                                                   /*901*/
    else
     rc = TRAP_ABEND;
   }
   break;

  case DBG_N_AsyncStop:
   rc = TRAP_ASYNC;
   break;

  case DBG_N_AliasFree:
  default:
   rc = TRAP_ABEND;
   break;

#ifdef __ESP__                                                          /*919*/
/*---------------------------------------------------------------------------*/
  case DBG_N_NewProc:                                                   /*919*/
   SendNewProcessToQue( pptb->Value );                                  /*919*/
   goto RestartApplication;                                             /*919*/
/*---------------------------------------------------------------------------*/
#endif                                                                  /*919*/

  case DBG_N_ThreadCreate:
  case DBG_N_ThreadTerm:
  case DBG_N_RangeStep:
   goto RestartApplication;

  case DBG_N_CoError:
   rc = TRAP_FP;
   break;

  case DBG_N_Exception:
   exception = pptb->Buffer;
   switch( exception )
   {
    case XCPT_BREAKPOINT:
    case XCPT_SINGLE_STEP:
     pptb->Cmd=DBG_C_Continue;
     pptb->Value=XCPT_CONTINUE_STOP;
     rc = DosDebug(pptb);
     if( rc != 0 || pptb->Cmd != 0 )
     { rc = TRAP_ABEND; break; }
     rc = TRAP_BPT;
     if( exception == XCPT_SINGLE_STEP )
      rc=TRAP_SS;
     break;

    default:
     /************************************************************************/
     /* - handle exceptions.                                                 */
     /************************************************************************/
     if(pptb->Value == DBG_X_STACK_INVALID)
     {;}
     else
     {
      /***********************************************************************/
      /* - Resolve the exception to the XCPT_??? exception number.           */
      /***********************************************************************/
      exception = ResolveException(pptb);
     }

     xcpt_continue_flag = 1;

     n = GetExceptionIndex(exception);

     if( GetExceptionMap(n) == NO_NOTIFY)
      goto RestartApplication;

     rc = GetExceptionNumber(n);
   }
   break;

  case DBG_N_ModuleLoad:                /* -8                                */
   if( ExecFlags & LOAD_MODULES )
   {
    AddToModuleLoadTable(&pModuleList, pptb->Pid, pptb->Value);
    if( ExecFlags & RUNNING_DEFERRED )
    {
     rc = (APIRET)DBG_N_ModuleLoad;
     break;
    }
   }
   goto RestartApplication;

  case DBG_N_ModuleFree:                /* -16                               */
   if( ExecFlags & LOAD_MODULES )
    AddFreeToModuleLoadTable(&pModuleList, pptb->Value);
   goto RestartApplication;

  case DBG_N_Watchpoint:                /* -14                               */
   MarkWpNotSet(pptb->Index);
   rc = TRAP_WATCH;
   break;
 }

 if( pptb->CSAtr )
  *pExecAddr = pptb->EIP;
 else
 {
  *pExecAddr = Sys_SelOff2Flat(pptb->CS,LoFlat(pptb->EIP));
 }

 /****************************************************************************/
 /* Pack the module table and get out.                                       */
 /****************************************************************************/
 if( pModuleList )
  *ppModuleLoadTable = PackModuleLoadTable(pModuleList , pModuleTableLength);
 return(rc);
}

/*****************************************************************************/
/* XSrvGoStep()                                                              */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Step a source line stepping over any call instructions.                 */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   target   address of instruction following the call.                     */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   rc       TRAP_SS if we land on the bp after the call.                   */
/*            Whatever rc we get from restartappl() otherwise.               */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
APIRET XSrvGoStep( PtraceBuffer  *pptb,
                   ULONG         *pExecAddr,
                   UINT         **ppModuleLoadTable,
                   int           *pModuleTableLength,
                   ULONG         ExecAddr,
                   ULONG         ExecAddrlo,
                   ULONG         ExecAddrhi,
                   int           how,
                   int           ExecFlags )
{
 APIRET rc;
 ULONG  InstructionLen;
 ULONG  CallInstructionAddr=0;
 ULONG  eip = 0;
 UINT   *pModuleLoadTable = NULL;
 int     ModuleTableLength = 0;
 UINT   *pModuleLoadTableAccum = NULL;
 int     ModuleTableLengthAccum = 0;

 /****************************************************************************/
 /* - If the user tries to single step when an exception is pending then     */
 /*   it gets mapped to a GoFast.                                            */
 /****************************************************************************/
 if (xcpt_continue_flag == 1)
 {
  rc=XSrvGoFast(pptb,
                pExecAddr,
                ppModuleLoadTable,
                pModuleTableLength,
                ExecAddr,
                ExecFlags);
  return(rc);
 }
 /***********************************************************************101*/
 /* - execute instructions within a source line until beyond the source  101*/
 /*   line.                                                              101*/
 /* - step over call instructions if skipcalls parameter is set.         101*/
 /*                                                                      101*/
 /***********************************************************************101*/
 rc = TRAP_SS_ERROR;                                                   /*101*/
 eip = ExecAddr;
 while( (eip >= ExecAddrlo) && (eip <= ExecAddrhi) )
 {                                                                     /*101*/
  if( (how==OVERCALL) && IsCallInst(eip) )
  {                                                                    /*101*/
   CallInstructionAddr = eip;
   InstructionLen = _InstLength(eip);                                  /*101*/
   rc = StepOverCall( pptb,
                      ExecAddr,
                      &pModuleLoadTable,
                      &ModuleTableLength,
                      eip + InstructionLen,
                      &eip,
                      ExecFlags );
  }                                                                    /*101*/
  else                                                                 /*101*/
  {
   pptb->Cmd = DBG_C_SStep;
   rc = XSrvRestartAppl(pptb,
                        &eip,
                        &pModuleLoadTable,
                        &ModuleTableLength,
                        ExecFlags);
  }

  if(pModuleLoadTable)
  {
   pModuleLoadTableAccum = CoalesceTables( pModuleLoadTableAccum,
                                           ModuleTableLengthAccum,
                                           pModuleLoadTable,
                                           ModuleTableLength,
                                           &ModuleTableLengthAccum );
   pModuleLoadTable = NULL;
   ModuleTableLength = 0;
  }

  if( rc != TRAP_SS )
   break;
 }

 if( rc == DBG_N_ModuleLoad )
  *pExecAddr = CallInstructionAddr;
 else if( pptb->CSAtr )
  *pExecAddr = pptb->EIP;
 else
  *pExecAddr =  Sys_SelOff2Flat(pptb->CS,LoFlat(pptb->EIP));

 *ppModuleLoadTable = pModuleLoadTableAccum;
 *pModuleTableLength = ModuleTableLengthAccum;
 return( rc );                                                         /*101*/
}

/*****************************************************************************/
/* XSrvGoFast()                                                              */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Run until we get a notification from DosDebug.                          */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   pptb               -> to the ptrace buffer.                             */
/*   pExecAddr          -> receiver of next execution address.(after GoFast) */
/*   ppModuleLoadTable  where to put a ptr to the ModuleLoadTable.           */
/*   pModuleTableLength where to put the length of the ModuleLoadtable.      */
/*   ExecAddr           address of exec instruction.(before GoFast )         */
/*   ExecFlags          execution flags.                                     */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
APIRET XSrvGoFast( PtraceBuffer  *pptb,
                   ULONG         *pExecAddr,
                   UINT         **ppModuleLoadTable,
                   int           *pModuleTableLength,
                   ULONG         ExecAddr,
                   int           ExecFlags)
{
 APIRET  rc;
 UINT   *pModuleLoadTable = NULL;
 int     ModuleTableLength = 0;
 UINT   *pModuleLoadTableAccum = NULL;
 int     ModuleTableLengthAccum = 0;

 /****************************************************************************/
 /* - If there's a break point defined for the current CS:EIP, then          */
 /*   single step before inserting the breakpoints.                          */
 /* - Insert all the breakpoints.                                            */
 /* - Run the application.                                                   */
 /* - Handle any dll loads/frees.                                            */
 /****************************************************************************/
 if( _IfBrkOnAddr(ExecAddr) != NULL )
 {
  /***************************************************************************/
  /* - step if a break point is defined for ExecAddr.                        */
  /***************************************************************************/
  pptb->Cmd = DBG_C_SStep;
  rc=XSrvRestartAppl(pptb,
                     pExecAddr,
                     &pModuleLoadTable,
                     &ModuleTableLength,
                     ExecFlags);
  if(pModuleLoadTable)
  {
   pModuleLoadTableAccum = CoalesceTables( pModuleLoadTableAccum,
                                           ModuleTableLengthAccum,
                                           pModuleLoadTable,
                                           ModuleTableLength,
                                           &ModuleTableLengthAccum );
   pModuleLoadTable = NULL;
   ModuleTableLength = 0;
  }
  if( rc != TRAP_SS )
   goto fini;
 }

 /****************************************************************************/
 /* - let 'er rip.                                                           */
 /****************************************************************************/
 pptb->Cmd = DBG_C_Go;
 XSrvInsertAllBrk();
 rc=XSrvRestartAppl(pptb,
                    pExecAddr,
                    &pModuleLoadTable,
                    &ModuleTableLength,
                    ExecFlags);
 if(pModuleLoadTable)
 {
  pModuleLoadTableAccum = CoalesceTables( pModuleLoadTableAccum,
                                          ModuleTableLengthAccum,
                                          pModuleLoadTable,
                                          ModuleTableLength,
                                          &ModuleTableLengthAccum );
  pModuleLoadTable = NULL;
  ModuleTableLength = 0;
 }
 XSrvRemoveAllBrk();

fini:
 *ppModuleLoadTable = pModuleLoadTableAccum;
 *pModuleTableLength = ModuleTableLengthAccum;

 return(rc);
}

/*****************************************************************************/
/* StepOverCall()                                                            */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Steps over a call instruction by setting a breakpoint at the next       */
/*   instruction and executing to that point.                                */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   pptb               -> to the ptrace buffer.                             */
/*   ExecAddr           current execution address.                           */
/*   ppModuleLoadTable  where to put a ptr to the ModuleLoadTable.           */
/*   pModuleTableLength where to put the length of the ModuleLoadtable.      */
/*   target             address of instruction following the call.           */
/*   pEip               where to put the exec address after the call.        */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   rc       TRAP_SS if we land on the bp after the call.                   */
/*            Whatever rc we get from restartappl() otherwise.               */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
APIRET StepOverCall( PtraceBuffer  *pptb,
                     ULONG          ExecAddr,
                     UINT         **ppModuleLoadTable,
                     int           *pModuleTableLength,
                     ULONG          target,
                     ULONG         *pEip,
                     int            ExecFlags )
{
 APIRET  rc;
 ULONG   eip;
 UINT    OldEip;
 UINT   *pModuleLoadTable = NULL;
 int     ModuleTableLength = 0;
 UINT   *pModuleLoadTableAccum = NULL;
 int     ModuleTableLengthAccum = 0;

 XSrvDefBrk(target);
 XSrvInsertAllBrk();
 if( _IfBrkOnAddr( ExecAddr ) )
 {
  OldEip = ExecAddr;
  XSrvRemoveOneBrk( ExecAddr );
  pptb->Cmd = DBG_C_SStep;
  rc = XSrvRestartAppl(pptb,&eip,&pModuleLoadTable,&ModuleTableLength,ExecFlags);
  if(pModuleLoadTable)
  {
   pModuleLoadTableAccum = CoalesceTables( pModuleLoadTableAccum,
                                           ModuleTableLengthAccum,
                                           pModuleLoadTable,
                                           ModuleTableLength,
                                           &ModuleTableLengthAccum );
   pModuleLoadTable = NULL;
   ModuleTableLength = 0;
  }
  if( rc != TRAP_SS)
   goto fini;
  XSrvInsertOneBrk( OldEip);
 }

 pptb->Cmd = DBG_C_Go;
 rc=XSrvRestartAppl(pptb,&eip,&pModuleLoadTable,&ModuleTableLength,ExecFlags);
 if(pModuleLoadTable)
 {
  pModuleLoadTableAccum = CoalesceTables( pModuleLoadTableAccum,
                                          ModuleTableLengthAccum,
                                          pModuleLoadTable,
                                          ModuleTableLength,
                                          &ModuleTableLengthAccum );
  pModuleLoadTable = NULL;
  ModuleTableLength = 0;
 }

fini:

 XSrvRemoveAllBrk();
 XSrvUndBrk(target);

 *ppModuleLoadTable = pModuleLoadTableAccum;
 *pModuleTableLength = ModuleTableLengthAccum;

 *pEip = eip;
 if( (rc == TRAP_BPT) && (eip == target) )
  return(TRAP_SS);
 return(rc);
}

  int
IsCallInst(UINT address)
{
  UCHAR dataword[2];                    /* word returned from Getnbytes      */
  short   *wordptr = NULL;              /* pointer to word in user addr sp102*/
  UINT    bytesread = 0;                /* bytes read from user addr space521*/

    wordptr = (short *)Getnbytes(address,2,&bytesread);
    while( wordptr )
    {
     *(short *)dataword = *wordptr;
     switch( dataword[0] )
     {
       case 0x9A:  /* far call */
       case 0xE8:  /* near call */
         return( TRUE );
       case 0xFF:  /* indirect */
         return(
           ((dataword[1] & 0x38) == 0x18) ||
           ((dataword[1] & 0x38) == 0x10)
         );
       case 0x26:  /* seg es: */
       case 0x2E:  /* seg ds: */
       case 0x36:  /* seg ss: */
       case 0x3E:  /* seg cs: */
         address += 1;
         break;
       default:
         return( FALSE );
     }
     wordptr = (short *)Getnbytes(address,2,&bytesread);
    }
    return( FALSE );
}

/*****************************************************************************/
/* XSrvNormalQuit()                                                              */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Quit the debugger.                                                      */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   none                                                                    */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*    0 or 1                                                                 */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   The application is either in the "DBG_N_ProcTerm" since the             */
/*   app has executed a DosExit, or the state is unknown.                    */
/*                                                                           */
/*****************************************************************************/
#define DOSEXITORDINAL32BIT 234

#pragma map(DosExitEntryPt16,"_DosExitEntryPt16")
extern char * _Seg16 DosExitEntryPt16;
extern ushort _Far16 Get16BitDosExitEntry( ulong );

APIRET XSrvNormalQuit( int AppTerminated, UINT MteOfExe, ULONG EntryPt )
{
 PtraceBuffer  ptb;
 uchar         CallDosExit16[5];
 ulong         mte;
 char          buf[256];
 ulong         ActionCode;
 ulong         ResultCode;
 ulong         DosExitEntryPt=0;
 uchar        *p;
 int           i;
 int           rc;
 ulong         flatsssp;
 ulong         StackSize;

 /****************************************************************************/
 /* clear any watch points currently set as well as any pending wp           */
 /* notifications.                                                           */
 /****************************************************************************/
 XSrvPullOutWps( );

 if( (AppTerminated == FALSE) && (Error_Interrupt_Flag == FALSE) )
 {
  memset(&ptb,0,sizeof(ptb));
  ActionCode = 1;
  ResultCode = 0;
  if( _GetBitness(EntryPt) == BIT32 )
  {
   ptb.Pid = GetEspProcessID();
   ptb.Tid = 1;

   /**************************************************************************/
   /* For 32 bit exes, we find the DosExit entry pt and set up a stack       */
   /* frame for making the call on behalf of the debuggee. This is a bit     */
   /* different from the 16 bit scenario where we write a "Call DosExit"     */
   /* instruction in the user's address space and then execute it.           */
   /*                                                                        */
   /* - Set esp to point 1k off the top of the stack.                        */
   /* - Align the address on a dword.                                        */
   /* - Set up a stack frame as follows:                                     */
   /*                                                                        */
   /*    esp--->+0| xxxxx  | <----ret addr ( don't care since no return )    */
   /*           +4|   1    | <----Action code                                */
   /*           +8|   0    | <----Result code                                */
   /*    ebp--->+C|        |                                                 */
   /*                                                                        */
   /**************************************************************************/
   flatsssp = GetExeStackTop(MteOfExe,&ptb.SS,&StackSize);
   flatsssp -= 1096;
   flatsssp += 3;
   flatsssp &= 0xfffffffc;

   ptb.EBP = flatsssp;
   ptb.ESP = ptb.EBP - 0xC;

   /**************************************************************************/
   /* Set up the flat address registers.                                     */
   /**************************************************************************/
   ptb.DS  = 0x53;
   ptb.ES  = 0x53;
   ptb.SS  = 0x53;
   ptb.CS  = 0x5B;
   ptb.FS  = 0x150B;
   ptb.GS  = 0x00;

   /**************************************************************************/
   /* Set up the stack for a DosExit call and find the DosExit entry pt.     */
   /**************************************************************************/
   if( Putnbytes(ptb.ESP+4,4,(uchar*)&ActionCode) ||
       Putnbytes(ptb.ESP+8,4,(uchar*)&ResultCode) ||
       DosLoadModule( buf, sizeof(buf), "DOSCALL1", &mte ) ||
       DosQueryProcAddr(mte,DOSEXITORDINAL32BIT,NULL,(PFN*)&DosExitEntryPt)||
       DosFreeModule(mte)
     )
    return( 1 );

   ptb.EIP = DosExitEntryPt;
  }
  else
  {
   /**************************************************************************/
   /* handle 16 bit EXE.                                                     */
   /**************************************************************************/
   ptb.Pid = GetEspProcessID();
   ptb.Tid = 1;

   /**************************************************************************/
   /* - Set esp to point into the middle of the stack or 1k off the          */
   /*   top of the stack.                                                    */
   /* - Get the load SS at the same time.                                    */
   /* - Align the address on a dword.                                        */
   /**************************************************************************/
   flatsssp = GetExeStackTop(MteOfExe,&ptb.SS,&StackSize);
   if( StackSize == 0 )
    flatsssp -= 1096;
   else
    flatsssp -= (StackSize>>1);

   flatsssp += 3;
   flatsssp &= 0xfffffffc;

   ptb.ESP = *((ushort*)&flatsssp);

   /**************************************************************************/
   /* Set up the stack for a DosExit call and find the DosExit entry pt.     */
   /**************************************************************************/
   if( !flatsssp  ||
        Putnbytes(flatsssp  ,2,(uchar*)&ResultCode) ||
        Putnbytes(flatsssp+2,2,(uchar*)&ActionCode) ||
        DosLoadModule( buf, sizeof(buf), "DOSCALLS", &mte ) ||
        Get16BitDosExitEntry( mte )
     )
    return( 1 );

   /**************************************************************************/
   /* Build an instruction for the DosExit call.                             */
   /**************************************************************************/
   p = (uchar *)&DosExitEntryPt16;
   CallDosExit16[0] = 0x9A;
   for(i=1;i<=4;i++)
    CallDosExit16[i] = *p++;

   /**************************************************************************/
   /* - Write the instruction a the user's entry point.                      */
   /* - Set up the CS:IP for the call.                                       */
   /**************************************************************************/
   if( Putnbytes(EntryPt,sizeof(CallDosExit16),CallDosExit16) )
    return( 1 );
   Sys_Flat2SelOff(EntryPt,&ptb.CS,(ushort*)&ptb.EIP);
  }

  ptb.Cmd = DBG_C_WriteReg;
  if( DosDebug(&ptb) || (ptb.Cmd != 0) )
   return(1);
 }                                      /* end of if !AppTerminated          */

 rc = 0;
 /****************************************************************************/
 /*  - If we're quitting/restarting after a c-break, then we proceed         */
 /*    immediately to the DBG_C_TERM and forget the GoTerm()                 */
 /*  - Else, if the app has terminated, then it should be in                 */
 /*    the DBG_N_ProcTerm state. GoTerm() will finish the job.               */
 /****************************************************************************/
 if( Error_Interrupt_Flag || GoTerm() )
 {
  /***************************************************************************/
  /* If our normal termination did not work, we'll try DosDebug which        */
  /* should be all that we need anyway, but...                               */
  /***************************************************************************/
  memset( &ptb,0,sizeof(ptb) );
  ptb.Cmd = DBG_C_Term;
  ptb.Pid = GetEspProcessID();
  rc=( DosDebug(&ptb) || ptb.Cmd )?1:0;
 }

 /****************************************************************************/
 /* - clean up the breakpoint structures.                                    */
 /****************************************************************************/
 ClearBrks();
 FreeObjectList();

 /****************************************************************************/
 /* - end the parent session.                                                */
 /****************************************************************************/
 DosStopSession( STOP_SESSION_ALL, GetTerminateSessionID() );           /*919*/
 return(rc);
}                                       /* end NormalQuit()                  */


/*****************************************************************************/
/* GoTerm()                                                                  */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Terminate the debuggee process.                                         */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   none                                                                    */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   0 or 1                                                                  */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   The application is either in the "DBG_N_ProcTerm" since the             */
/*   app has executed a DosExit, or the state is unknown.                    */
/*                                                                           */
/*****************************************************************************/
int GoTerm(  )
{
 long         DosDebugCmd;
 BOOL         success;
 PtraceBuffer ptb;
 APIRET       rc;
 ULONG        exception;

 memset(&ptb,0,sizeof(ptb));
 success = FALSE;


 /****************************************************************************/
 /* - read the current registers for thread 1.                               */
 /****************************************************************************/
 ptb.Pid = GetEspProcessID();
 ptb.Cmd = DBG_C_ReadReg;
 ptb.Tid = 1;
 rc = DosDebug( &ptb);
 if ( rc != 0 || ptb.Cmd != DBG_N_Success )
  return(1);

 /****************************************************************************/
 /*  - Execute a final go command to force the debuggee to run a DosExit.    */
 /*  - ...unless we're sitting on an exception, we'll give the exception     */
 /*    back to the operating system and take it from there.                  */
 /*                                                                          */
 /****************************************************************************/
 DosDebugCmd = DBG_C_Go;
 if (xcpt_continue_flag == 1)
 {
  DosDebugCmd=DBG_C_Continue;
  ptb.Value=XCPT_CONTINUE_SEARCH;
  xcpt_continue_flag = 0;
 }

 while( success == FALSE )
 {
  ptb.Cmd = DosDebugCmd;
  ptb.Pid = GetEspProcessID();

  if( DosDebug( &ptb ) ||
      ptb.Cmd == DBG_N_Error )
   return(  1  );

  /***************************************************************************/
  /* Process notifications from DosDebug.                                    */
  /*                                                                         */
  /* Termination should proceed as follows:                                  */
  /*  - DBG_N_Exception with XCPT_PROCESS_TERMINATE.                         */
  /*  - DBG_N_ThreadTerm for thread 1.                                       */
  /*  - DBG_N_ProcTerm.                                                      */
  /*  - DBG_N_Success.                                                       */
  /*                                                                         */
  /* If the DBG_N_ProcTerm notification has already been received as a       */
  /* result of the debuggee executing a DosExit, then the first 2            */
  /* notifications will have already gone by in Go().                        */
  /*                                                                         */
  /***************************************************************************/
  switch( ptb.Cmd )
  {
   case DBG_N_ThreadTerm:
    DosDebugCmd = DBG_C_Go;
    continue;

   case DBG_N_ProcTerm:
    DosDebugCmd = DBG_C_Go;
    continue;

   case DBG_N_Success:
    success = TRUE;
    break;

   case DBG_N_Exception:
    exception = ResolveException( &ptb);
    if( ( exception == XCPT_PROCESS_TERMINATE ) ||
        ( exception == XCPT_ASYNC_PROCESS_TERMINATE ) ||
        ( exception == XCPT_SIGNAL )
      )
    {
     DosDebugCmd  = DBG_C_Continue;
     ptb.Value = XCPT_CONTINUE_SEARCH;
     continue;
    }
    else
     return(  1  );

   default:
    DosDebugCmd = DBG_C_Go;
    continue;
  }
 }
 return( 0 );
}                                       /* end GoTerm()                      */

/*****************************************************************************/
/* XSrvSetExecAddr()                                                         */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Change the execution address of the current thread.                     */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   ExecAddr   the new execution address.                                   */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   rc                                                                      */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
APIRET XSrvSetExecAddr( ULONG ExecAddr )
{
 PtraceBuffer ptb;
 APIRET       rc;

 /****************************************************************************/
 /* - read the current registers for the current thread.                     */
 /****************************************************************************/
 memset(&ptb,0,sizeof(ptb));
 ptb.Pid = GetEspProcessID();
 ptb.Cmd = DBG_C_ReadReg;
 ptb.Tid = 0;
 rc = DosDebug( &ptb);
 if ( rc != 0 || ptb.Cmd != DBG_N_Success )
  return(1);

 /****************************************************************************/
 /* - Write the new execution address.                                       */
 /****************************************************************************/
 if(ptb.CSAtr)
  ptb.EIP = ExecAddr;
 else
  Sys_Flat2SelOff(ExecAddr,&ptb.CS,(USHORT*)&ptb.EIP);

 ptb.Cmd = DBG_C_WriteReg;
 rc = DosDebug( &ptb);
 if ( rc != 0 || ptb.Cmd != DBG_N_Success )
  return(1);
 return(0);
}

/*****************************************************************************/
/* XSrvWriteRegs()                                                           */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Write the registers and return exec addr.                               */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   pExecAddr -> to address that will receive the flat execution address.   */
/*   pptb      -> to the ptrace buffer with the new registers.               */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   rc        0=>success                                                    */
/*             1=>failure                                                    */
/*                                                                           */
/* Assumption:                                                               */
/*                                                                           */
/*  pptb->tid is the thread we're writing the registers for.                 */
/*                                                                           */
/*****************************************************************************/
APIRET XSrvWriteRegs(ULONG *pExecAddr,PtraceBuffer *pptb)
{
 APIRET rc;

 pptb->Pid = GetEspProcessID();
 pptb->Cmd = DBG_C_WriteReg;
 pptb->Tid = pptb->Tid; /* just to emphasize the point! */
 rc = DosDebug( pptb);
 if( (rc != 0) || (pptb->Cmd != 0) )
  return(1);

 if( pptb->CSAtr )
  *pExecAddr = pptb->EIP;
 else
  *pExecAddr =  Sys_SelOff2Flat(pptb->CS,LoFlat(pptb->EIP));
 return(0);
}

/*****************************************************************************/
/* XSrvGetCoregs()                                                           */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Get the co-processor registers.                                         */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   pCoproc_regs -> to the buffer receiving the regs.                       */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   rc        0=>success                                                    */
/*             1=>failure                                                    */
/*                                                                           */
/* Assumption:                                                               */
/*                                                                           */
/*  pptb->tid is the thread we're writing the registers for.                 */
/*                                                                           */
/*****************************************************************************/
APIRET XSrvGetCoRegs(void *pCoproc_regs)
{
 APIRET           rc;
 PtraceBuffer     ptb;

 memset(&ptb,0,sizeof(ptb));
 ptb.Cmd    = DBG_C_ReadCoRegs;
 ptb.Value  = DBG_CO_387;
 ptb.Pid    = GetEspProcessID();
 ptb.Tid    = 0;
 ptb.Buffer = (ulong)pCoproc_regs;
 ptb.Len    = 108;
 ptb.Index  = 0; /* reserved must be 0.*/

 rc = DosDebug(&ptb);
 if( rc != 0 || ptb.Cmd != 0 )
  return(1);
 return(0);
}

/*---------------------------------------------------------------------------*/
#ifdef __ESP__
/*****************************************************************************/
/* SpinThread()                                                           919*/
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   This thread executes a non-debug process under control of DosDebug.     */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   ppid               -> to the pid being debu. is for.                    */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   This function is invoked via _beginthread(). It is not called.          */
/*                                                                           */
/*****************************************************************************/
void SpinThread( void *ulpid)
{
 APIRET       rc;
 PtraceBuffer ptb;
 USHORT       pid;
 ULONG        DbgCmd;
 BOOL         ErrorCondition = FALSE;
 BOOL         Success = FALSE;
 ULONG        Exception;

 pid = (USHORT)ulpid;

 /****************************************************************************/
 /* - connect to this pid.                                                   */
 /****************************************************************************/
 memset(&ptb,0,sizeof(ptb));

 ptb.Cmd   = DBG_C_Connect;
 ptb.Pid   = pid;
 ptb.Value = DBG_L_386;
 ptb.Addr  = 1;

 rc = DosDebug(&ptb);
 if( rc || (ptb.Cmd == DBG_N_Error) )
  ErrorCondition = TRUE;

 /****************************************************************************/
 /* - Read the registers and extract an mte. We might need to know this      */
 /*   so that we can get the module name if there is an error.               */
 /****************************************************************************/
 ptb.Cmd   = DBG_C_ReadReg;
 ptb.Pid   = pid;
 ptb.Tid   = 1;
 rc        = DosDebug(&ptb);
 if( rc || (ptb.Cmd == DBG_N_Error) )
  ErrorCondition = TRUE;

 /****************************************************************************/
 /* - run this pid.                                                          */
 /****************************************************************************/
 DbgCmd = DBG_C_Go;
 for( ; (ErrorCondition == FALSE) && (Success == FALSE) ; )
 {
  ptb.Cmd = DbgCmd;
  rc = DosDebug(&ptb);
  if( rc || (ptb.Cmd == DBG_N_Error) )
   { ErrorCondition = TRUE; break; }

  switch( ptb.Cmd )
  {
   case DBG_N_Success:
    Success = TRUE;
    break;

   case DBG_N_NewProc:
    SendNewProcessToQue( ptb.Value );
    DbgCmd = DBG_C_Go;
    break;

   case DBG_N_Exception:
    Exception = ResolveException(&ptb);
    switch( Exception )
    {
     case XCPT_SINGLE_STEP:
     case XCPT_BREAKPOINT:
      break;

     case XCPT_ACCESS_VIOLATION:
      break;

     default:
      break;
    }
    DbgCmd    = DBG_C_Continue;
    ptb.Value = XCPT_CONTINUE_SEARCH;
    break;

   case DBG_N_AsyncStop:
    DbgCmd = DBG_C_Term;
    break;

   default:
    DbgCmd = DBG_C_Go;
    break;
  }
 }

 _endthread();
}

/*****************************************************************************/
/* SendNewProcessToQue()                                                  919*/
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Send a new process notification to the esp que.                         */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   pid       process id of the new process.                                */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
void SendNewProcessToQue( USHORT pid)
{
 ESP_QUE_ELEMENT     Qelement;

 Qelement.ChildSid  = 0;
 Qelement.ChildPid  = pid;
 Qelement.ParentSid = 0;
 Qelement.ParentPid = 0;

 SendMsgToEspQue( ESP_QMSG_NEW_PROCESS,&Qelement,sizeof(Qelement) );
}
#endif
/*---------------------------------------------------------------------------*/

/*****************************************************************************/
/* GetProcessMte()                                                           */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Get the mte associated with a given pid.                                */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   pMte     -> to the receiver of the mte.                                 */
/*   pid      process id of the EXE that we want the mte for.                */
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
APIRET GetProcessMte( USHORT pid, USHORT *pMte )
{
 void           *pProcStatBuf;
 ULONG           flags;
 qsPrec_t       *pProcRec;              /* ptr to process record section    */
 qsS16Headrec_t *p16SemRec;             /* ptr to 16 bit sem section        */

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
 /* - give the mte back to the caller.                                       */
 /****************************************************************************/
 *pMte  = pProcRec->hMte;
 return(0);
}
