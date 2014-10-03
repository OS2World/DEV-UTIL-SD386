/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   go.c                                                                 822*/
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*  Execution functions.                                                     */
/*                                                                           */
/* History:                                                                  */
/*                                                                           */
/*   06/11/93 Created.                                                       */
/*                                                                           */
/*...                                                                        */
/*... 05/04/93  822   Joe       Add mte table handling.                      */
/*... 06/04/93  827   Joe       Add remote debug support.                    */
/*... 12/06/93  906   Joe       Fix for not hitting conditional breaks.      */
/*... 05/19/94  920   Joe       Fix hang.                                    */
/*...                                                                        */
/*****************************************************************************/
#include "all.h"

int    WatchpointFlag = TRUE;

extern PROCESS_NODE   *pnode;
extern uchar           Re_Parse_Data;   /* flag to indicate wether all       */
                                        /* variables in datawindow are to    */
                                        /* be reparsed.                      */
extern CmdParms       cmd;

int          DataRecalc;
PtraceBuffer AppPTB;

/*****************************************************************************/
/* Functions to get/set Exec variables.                                      */
/*****************************************************************************/
static TSTATE *ExecThread;
static AFILE  *Execfp;
static ULONG   ExecTid;
static ULONG   ExecAddr;
static LNOTAB *pExecLnoTabEntry;
static ULONG   ExecAddrlo;
static ULONG   ExecAddrhi;
static ULONG   ExecMid;

TSTATE *GetExecThread(void)      { return(ExecThread); }
AFILE  *GetExecfp(void)          { return(Execfp);     }
ULONG   GetExecTid(void)         { return(ExecTid);    }
ULONG   GetExecAddr(void)        { return(ExecAddr);   }
LNOTAB *GetExecLnoTabEntry(void) { return(pExecLnoTabEntry); }
ULONG   GetExecMid(void)         { return(ExecMid);    }

void    SetExecAddr( ULONG addr) { ExecAddr = addr;    }

ULONG   GetExecLno(void)         { return(NULL);    }/*!!!!! remove */
/*****************************************************************************/
/* GoInit()                                                               827*/
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*  Initialize the exe for debugging.                                        */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*  pid     this is the pid of the process that we're going to connect to    */
/*          and debug.                                                       */
/*  sid     this is the session id containing the pid.                       */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*  rc                                                                       */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*  none                                                                     */
/*                                                                           */
/*****************************************************************************/
APIRET GoInit( ULONG pid, ULONG sid )
{
 UINT    *pModuleLoadTable = NULL;
 int      ModuleLoadTableLength;
 APIRET   rc = 0;

 memset( &AppPTB,0,sizeof(AppPTB) );
 AppPTB.Pid = pid;
 AppPTB.Tid = sid;

 rc = xGoInit( &AppPTB,
               (ULONG*)&ExecAddr,
               &pModuleLoadTable,
               &ModuleLoadTableLength );

 if( rc == 1 )
  return(1);

 if( rc )
  return( rc );

 if( pModuleLoadTable )
 {
//DumpModuleLoadTable( pModuleLoadTable );

  UpdateMteTable( pModuleLoadTable );
  ExeDllInit( pModuleLoadTable );
  Tfree(pModuleLoadTable);
 }
 return(0);
}

/*****************************************************************************/
/* GoEntry()                                                              827*/
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*  Run to an initial entry point.                                           */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*  rc                                                                       */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*  none                                                                     */
/*                                                                           */
/*****************************************************************************/
APIRET GoEntry( void )
{
 UINT    *pModuleLoadTable = NULL;
 int      ModuleLoadTableLength;
 APIRET   rc = 0;
 int      ExecFlags;

 /***************************************************************************/
 /* - When dealing with multiple processes you have to know that the        */
 /*   session that issued the DosStartSession() to start the parent         */
 /*   debuggee is the only session that can select the debuggee sessions.   */
 /*                                                                         */
 /*   (The child debuggee sessions are essentially treated like child       */
 /*   sessions of the parent probe even though they were not directly       */
 /*   started by the debugger using DosStartSession().  )                   */
 /*                                                                         */
 /* - ...But, the session that started the parent debuggee( or one of its   */
 /*   children) must also be in the foreground before it can issue          */
 /*   the DosSelectSession() successfully.                                  */
 /*                                                                         */
 /* - So, if we're running child/multiple processes on a single             */
 /*   machine, we have to tell the parent debugger to bring the             */
 /*   parent probe to the foreground so that when we send the               */
 /*   xSelectSession() the parent probe will be in the foreground and       */
 /*   capable of issueing a successful DosSelectSession().                  */
 /***************************************************************************/
 if( ConnectType()==LOCAL_PIPE )
 {
  SendMsgToDbgQue( DBG_QMSG_SELECT_PARENT_ESP, NULL, 0 );
 }
 xSelectSession();

 ExecFlags = 0;
 ExecFlags |= LOAD_MODULES;

 rc = xGoEntry( &AppPTB,
                (ULONG*)&ExecAddr,
                &pModuleLoadTable,
                &ModuleLoadTableLength,
                ExecAddr,
                ExecFlags);

 if( pModuleLoadTable )
 {
  UpdateMteTable( pModuleLoadTable );
  ExeDllInit( pModuleLoadTable );
  Tfree(pModuleLoadTable);
 }

 /****************************************************************************/
 /* - DosStartSession() has a restriction that only a parent can select      */
 /*   itself or a child session.  So, we have to send a message to the       */
 /*   parent debugger telling it to bring this debugger session to the       */
 /*   foreground.                                                            */
 /****************************************************************************/
 if( ConnectType()==LOCAL_PIPE )
 {
  DBG_QUE_ELEMENT Qelement;

  Qelement.pid = DbgGetProcessID();
  SendMsgToDbgQue( DBG_QMSG_SELECT_SESSION, &Qelement, sizeof(Qelement) );
 }

 SetExecValues( AppPTB.Tid, FALSE );
 SetActFrames();                       /* query active stack frames         */
 return(rc);
}

/*****************************************************************************/
/* Go()                                                                      */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Application execution function.                                         */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   func       the execution function.                                      */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   rc         return code.                                                 */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
int Go( UINT func)
{                                       /*                                   */
 APIRET   rc;                           /* local return code                 */
 int      UserSessionSelected;
 int      ExecFlags;

 Re_Parse_Data = TRUE;                  /* Set the parse var flag         244*/

 if( IsWatchPoint() && WatchpointFlag )                                                   /*701*/
  PutInWps();

 /****************************************************************************/
 /* - Breakpoint file may have been updated...check it.                      */
 /****************************************************************************/
 MergeBreakpoints();

 /****************************************************************************/
 /* - Bring the app to foreground if need be.                                */
 /* - The swap flags for the source and asm views are tied together.         */
 /*   That's why we have the checks on ASMSTEP and ASMSTEPINTO.              */
 /****************************************************************************/
 UserSessionSelected = FALSE;
 {
  UINT  funcx;

  funcx = func;
  if( func == ASMSSTEP)
   funcx = SSTEP;
  else if( func == ASMSSTEPINTOFUNC )
   funcx = SSTEPINTOFUNC;

  if( IsSwapFlag(funcx) )
  {
   /**************************************************************************/
   /* - If we're running with the probe bound to the debugger, then we       */
   /*   simply select the child debugger session like always.                */
   /**************************************************************************/
   if( ConnectType()==BOUND )
    DosSelectSession(DbgGetSessionID());
   else
   {
    /*************************************************************************/
    /* - When dealing with multiple processes you have to know that the      */
    /*   session that issued the DosStartSession() to start the parent       */
    /*   debuggee is the only session that can select the debuggee sessions. */
    /*                                                                       */
    /*   (The child debuggee sessions are essentially treated like child     */
    /*   sessions of the parent probe even though they were not directly     */
    /*   started by the debugger using DosStartSession().  )                 */
    /*                                                                       */
    /* - ...But, the session that started the parent debuggee( or one of its */
    /*   children) must also be in the foreground before it can issue        */
    /*   the DosSelectSession() successfully.                                */
    /*                                                                       */
    /* - So, if we're running child/multiple processes on a single           */
    /*   machine, we have to tell the parent debugger to bring the           */
    /*   parent probe to the foreground so that when we send the             */
    /*   xSelectSession() the parent probe will be in the foreground and     */
    /*   capable of issueing a successful DosSelectSession().                */
    /*************************************************************************/
    if( ConnectType()==LOCAL_PIPE )
    {
     SendMsgToDbgQue( DBG_QMSG_SELECT_PARENT_ESP, NULL, 0 );
    }
    xSelectSession();
   }

   UserSessionSelected = TRUE;
  }
 }

carryon:

 ExecFlags = 0;
 ExecFlags |= LOAD_MODULES;
 if( IsDeferredBrk() )
  ExecFlags |= RUNNING_DEFERRED;

 switch( func )                                                         /*701*/
 {                                                                      /*701*/

  case RUNNOLOAD:
   /**************************************************************************/
   /* - turn off the load modules bit.                                       */
   /**************************************************************************/
   ExecFlags &= ~LOAD_MODULES;
   rc = GoFast( ExecFlags );
   break;

  case RUN:                                                             /*701*/
  case RUNTOCURSOR:                                                     /*701*/
  case RUNNOSWAP:                                                       /*701*/
  case RUNTOCURSORNOSWAP:                                               /*701*/
   rc = GoFast( ExecFlags );                                            /*827*/
   break;                                                               /*701*/
                                                                        /*701*/
  case SSTEP:                           /* Source line step. Step over. /*701*/
  case SSTEPNOSWAP:                                                     /*701*/
   rc = GoStep(OVERCALL,ExecFlags);                                     /*827*/
   break;                                                               /*701*/
                                                                        /*701*/
  case ASMSSTEP:                                                        /*701*/
  case ASMSSTEPNOSWAP:                                                  /*701*/
   if( func == ASMSSTEP)                                                /*701*/
    func = SSTEP;                                                       /*701*/
   ExecAddrlo = ExecAddrhi = ExecAddr;                                  /*827*/
   rc = GoStep(OVERCALL,ExecFlags);                                     /*827*/
   break;                                                               /*701*/
                                                                        /*701*/
  case ASMSSTEPINTOFUNC:                                                /*701*/
  case ASMSSTEPINTOFUNCNOSWAP:                                          /*701*/
   if( func == ASMSSTEPINTOFUNC)                                        /*701*/
    func = SSTEPINTOFUNC;                                               /*701*/
   ExecAddrlo = ExecAddrhi = ExecAddr;                                  /*827*/
   rc = GoStep(INTOCALL,ExecFlags);                                     /*827*/
   break;                                                               /*701*/
                                                                        /*701*/
  case SSTEPINTOFUNC:                                                   /*701*/
  case SSTEPINTOFUNCNOSWAP:                                             /*701*/
   rc = GoStep(INTOCALL,ExecFlags);                                     /*827*/
   break;                                                               /*701*/
                                                                        /*701*/
 }                                                                      /*701*/

 /****************************************************************************/
 /* - We may come here when running with deferred break points, so           */
 /*   just back up and continue with the original function.                  */
 /****************************************************************************/
 if( rc == (APIRET)DBG_N_ModuleLoad )
  goto carryon;

 /****************************************************************************/
 /* - set the DataRecalc flag.                                               */
 /****************************************************************************/
 DataRecalc = TRUE;

 /****************************************************************************/
 /* - Now, bring the debugger back to the foreground if necessary.           */
 /****************************************************************************/
 if(UserSessionSelected == TRUE && rc != TRAP_INTERLOCK )               /*901*/
 {
  if( ConnectType()==BOUND )
   DosSelectSession(0);
  else
  {
   /**************************************************************************/
   /* - DosStartSession() has a restriction that only a parent can select    */
   /*   itself or a child session.  So, we have to send a message to the     */
   /*   parent debugger telling it to bring this debugger session to the     */
   /*   foreground.                                                          */
   /**************************************************************************/
   if( ConnectType()==LOCAL_PIPE )
   {
    DBG_QUE_ELEMENT Qelement;

    Qelement.pid = DbgGetProcessID();
    SendMsgToDbgQue( DBG_QMSG_SELECT_SESSION, &Qelement, sizeof(Qelement) );
   }
  }
 }


 if( rc != TRAP_INTERLOCK )                                             /*920*/
 {                                                                      /*920*/
  SetExecValues( AppPTB.Tid, FALSE );
  SetActFrames();                       /* query active stack frames         */
                                        /*                                   */
  SetRegChgMask();                      /* set the registers change mask  400*/
 }                                                                      /*920*/
 return( rc );                          /* return to caller with ret code.   */
}

/*****************************************************************************/
/* GoFast()                                                                  */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Run until we get a notification from DosDebug.                          */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
APIRET GoFast( int ExecFlags)
{
 BRK    *atbrk;
 APIRET  rc;
 APIRET  rcx;
 UINT   *pModuleLoadTable = NULL;
 int     ModuleLoadTableLength = 0;

 /****************************************************************************/
 /* - If there's a break point defined for the current CS:EIP, then          */
 /*   single step before inserting the breakpoints.                          */
 /* - Insert all the breakpoints.                                            */
 /* - Run the application.                                                   */
 /* - Update the mte table if needed.                                        */
 /* - Handle any dll loads/frees.                                            */
 /* - If we land on a breakpoint and it's conditional and the condition is   */
 /*   not met, then stay in the loop.                                        */
 /****************************************************************************/
 for(;;)
 {
  rc = xGoFast( &AppPTB,
                (ULONG*)&ExecAddr,
                &pModuleLoadTable,
                &ModuleLoadTableLength,
                ExecAddr,
                ExecFlags);
  if( pModuleLoadTable )
  {
   UpdateMteTable( pModuleLoadTable );
   rcx = ExeDllInit( pModuleLoadTable );
   Tfree(pModuleLoadTable);
  }

  /***************************************************************************/
  /* - test for dll load and address load breakpoints.                       */
  /***************************************************************************/
  if( (rcx == TRAP_ADDR_LOAD) ||
      (rcx == TRAP_DLL_LOAD)
    )
   return(rcx);

  /***************************************************************************/
  /* - At this point, xGoFast() will have defined a new ExecAddr.            */
  /* - Get out of here if not a breakpoint notification.                     */
  /***************************************************************************/
  if( rc != TRAP_BPT )
   break;

  /***************************************************************************/
  /* - At this point, we're at a breakpoint.                                 */
  /* - Take the break if it's not one we know about or it's a simple break.  */
  /***************************************************************************/
  atbrk = IfBrkOnAddr( ExecAddr );
  if( (atbrk == NULL) ||
      (atbrk->cond == NULL )
    )
   break;

  /***************************************************************************/
  /* - At this point, we have a conditional breakpoint.                      */
  /* - If the condition is met then take the break.                          */
  /***************************************************************************/
  DataRecalc = TRUE;                                                    /*906*/
  if( EvalCbrk(atbrk->cond) == TRUE )
   break;
 }
 return(rc);
}

/*****************************************************************************/
/* GoStep()                                                                  */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Step a source line stepping over any call instructions.                 */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   void                                                                    */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   void                                                                    */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
APIRET GoStep( int how, int ExecFlags )
{
 APIRET  rc;
 APIRET  rcx;
 UINT   *pModuleLoadTable = NULL;
 int     ModuleLoadTableLength = 0;

 rc = xGoStep( &AppPTB,
               (ULONG*)&ExecAddr,
               &pModuleLoadTable,
               &ModuleLoadTableLength,
               ExecAddr,
               ExecAddrlo,
               ExecAddrhi,
               how,
               ExecFlags );
 if( pModuleLoadTable )
 {
  UpdateMteTable( pModuleLoadTable );
  rcx = ExeDllInit( pModuleLoadTable );
  Tfree(pModuleLoadTable);
  /***************************************************************************/
  /* - test for dll load and address load breakpoints.                       */
  /***************************************************************************/
  if( (rcx == TRAP_ADDR_LOAD) ||
      (rcx == TRAP_DLL_LOAD)
    )
   return(rcx);
 }
 return(rc);
}

/*****************************************************************************/
/* SetExecValues()                                                           */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Setting pertinent execution context values after returning from         */
/*   an execution function including:                                        */
/*                                                                           */
/*    ExecTid = The thread id that execution stopped in.                     */
/*                                                                           */
/*                                                                           */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   void                                                                    */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   void                                                                    */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
extern TSTATE *thead;
void SetExecValues( ULONG exectid, int GetRegsFlag)
{
 TSTATE       *np;                      /* thread state node pointer         */
 DEBFILE      *pdf;                     /* -> to debug file structure        */
 APIRET        rc;
 LNOTAB       *pLnoTabEntry;
 ULONG         span;

 /****************************************************************************/
 /* - if we're switching context to another thread then we need to read      */
 /*   the registers.                                                         */
 /****************************************************************************/
 if( GetRegsFlag == TRUE )
 {
  PtraceBuffer ptb;

  /***************************************************************************/
  /* - tell esp to set it's execution context to a new thread.               */
  /* - if there's an error, don't change anything and return.                */
  /***************************************************************************/
  ptb  = AppPTB;
  rc = xSetExecThread( &ExecAddr, &AppPTB, exectid);
  if( rc != 0 )
  {
   AppPTB = ptb;
   return;
  }
 }

 /****************************************************************************/
 /* - build the thread list.                                                 */
 /****************************************************************************/
 BuildThreadList();

 /****************************************************************************/
 /* - scan the list and set the pointer to the executing thread and          */
 /*   set the executing thread id.                                           */
 /****************************************************************************/
 for ( np = thead; np != NULL ; np = np->next )
 {
  if( np->tid == exectid)
   break;
 }
 ExecThread = np;
 ExecTid    = 0;
 if( np )
  ExecTid = np->tid;

 /****************************************************************************/
 /* - find the pdf we're executing in.                                       */
 /****************************************************************************/
 pdf = FindExeOrDllWithAddr( ExecAddr );

 /****************************************************************************/
 /* If we can't find a pdf for this address, then we have landed on          */
 /* an address in hyperspace.  This may happen when we pop a bad eip         */
 /* on a ret instruction. So, as far as building a view, we're dead meat     */
 /* at this point. Just return and report the execption relative to the      */
 /* old exec values.                                                         */
 /****************************************************************************/
 if( pdf == NULL )
  return;

 /****************************************************************************/
 /* - set the executing line. 0 ==> no source line for this ExecAddr.        */
 /* - set the executing module id. 0 ==> no mid for this address.            */
 /****************************************************************************/
 ExecMid          = DBMapInstAddr( ExecAddr, &pLnoTabEntry, pdf);
 pExecLnoTabEntry = pLnoTabEntry;

 /****************************************************************************/
 /* - Set the start and end addresses if we landed on a source line. Else,   */
 /*   the start and end addresses will be the same for an assembler line.    */
 /****************************************************************************/
 ExecAddrlo = ExecAddrhi = ExecAddr;
 if( (pExecLnoTabEntry != NULL) &&  (pExecLnoTabEntry->lno != 0) )
 {
  int sfi;
  int lno;

  sfi = pExecLnoTabEntry->sfi;
  lno = pExecLnoTabEntry->lno;

  ExecAddrlo = DBMapLno(ExecMid, lno, sfi, &span , pdf );
  ExecAddrhi = ExecAddrlo + span - 1;

  if( ExecAddr > ExecAddrhi )
  {
   MODULE *pModule;
   LNOTAB *pLnoTabEntry;
   CSECT  *pCsect;

   ExecAddrlo = ExecAddrhi = ExecAddr;

   if( (pModule      = GetPtrToModule( ExecMid, pdf)         ) &&
       (pCsect       = GetCsectWithAddr( pModule,  ExecAddr) ) &&
       (pLnoTabEntry = GetLnoWithAddr( pCsect, ExecAddr)     ) &&
       (span         = GetLineSpan( pModule, pLnoTabEntry)   )
     )
   {
    ExecAddrlo = ExecAddr;
    ExecAddrhi = ExecAddrlo + span - 1;
   }
  }
 }

}

/*****************************************************************************/
/* SetExecfp()                                                               */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Build a view for the context of the execution address.                  */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   void                                                                    */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   void                                                                    */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
AFILE *SetExecfp( void )
{
 AFILE       *fp;

 fp = Execfp;
 Execfp = findfp( ExecAddr);
 if( Execfp == NULL )
  Execfp = fp;
 return(Execfp);
}
