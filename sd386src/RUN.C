/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   run.c                                                                   */
/*                                                                           */
/* Description:                                                              */
/*   execution driver function(s).                                           */
/*                                                                           */
/* History:                                                                  */
/*                                                                           */
/*...                                                                        */
/*...Rewritten  11/21/93                                                     */
/*... 03/29/94  917   Joe       Ctrl-Break handling for single process.      */
/*...                                                                        */
/**Includes*******************************************************************/

#include "all.h"

uchar msgbuf[80];
int   AppTerminated;

/**Externs********************************************************************/

extern PtraceBuffer  AppPTB;
extern CmdParms      cmd;
extern PROCESS_NODE *pnode;
extern int           WatchpointFlag;

AFILE  *fp_focus;
AFILE  *Getfp_focus(void) { return(fp_focus); }

USHORT  DgroupDS;
/*****************************************************************************/
/* Run()                                                                     */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*  Run the application.                                                     */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*    MainEntryPt  address of "main"                                         */
/*    ExeEntryPt   where the loader starts the EXE.                          */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
void Run( ULONG MainEntryPt , ULONG ExeEntryPt )
{
 uint     stopcode;
 int      how;
 BRK     *RestartBrks;
 DEBFILE *pdf;
 BRK     *pBrk;
 BRK     *plast;

 /****************************************************************************/
 /* - Save the current breakpoint ring.                                      */
 /* - Init a ring for startup.                                               */
 /****************************************************************************/
 RestartBrks     = pnode->allbrks;
 pnode->allbrks  = NULL;

 /****************************************************************************/
 /* - Blow by main if that's the user's choice.                              */
 /* - If the /i option was specified, then set breaks on the entry           */
 /*   points of all currently attached executables.                          */
 /* - Else, set a breakpoint on main.                                        */
 /* - And if there is no "main", then stop at initial cs:eip.                */
 /****************************************************************************/
 if( cmd.NoStopAtMain == TRUE )
 {;}
 else if( cmd.ShowInit == TRUE )
 {
  for( pdf=pnode->ExeStruct; pdf!=NULL; pdf=pdf->next )
  {
   if( pdf->EntryExitPt != NULL )
   {
    pBrk                  = DefBrk( pdf->EntryExitPt, TRUE);
    pBrk->flag.DorI       = BP_IMMEDIATE;
    pBrk->flag.DefineType = BP_INTERNAL;
   }
  }
 }
 else if( MainEntryPt != NULL )
 {
  pBrk                    = DefBrk(MainEntryPt,TRUE);
  pBrk->flag.DorI       = BP_IMMEDIATE;
  pBrk->flag.DefineType = BP_INTERNAL;
 }
 else
 {
  pBrk                  = DefBrk(ExeEntryPt,TRUE);
  pBrk->flag.DorI       = BP_IMMEDIATE;
  pBrk->flag.DefineType = BP_INTERNAL;
 }

 /****************************************************************************/
 /* - Merge breaks from file.                                                */
 /* - Turn off any watch points.                                             */
 /* - Run to the initial control point.                                      */
 /* - Free all the internal breakpoints.                                     */
 /* - Turn watch points back on.                                             */
 /****************************************************************************/
 MergeBreakpoints();
 WatchpointFlag = FALSE;
 stopcode = GoEntry();

 for( pBrk = pnode->allbrks; pBrk ; )
 {
  if( pBrk->flag.DefineType != BP_INTERNAL )
   pBrk  = pBrk->next;
  else
  {
   ULONG where;

   where = pBrk->brkat;
   pBrk  = pBrk->next;
   UndBrk( where, TRUE );
  }
 }

 /****************************************************************************/
 /* - Now attach the restart breakpoints to the front of the chain.          */
 /****************************************************************************/
 if( RestartBrks )
 {
  for( pBrk = RestartBrks; pBrk ; plast = pBrk, pBrk = pBrk->next ){;}

  plast->next    = pnode->allbrks;
  pnode->allbrks = RestartBrks;
 }

 WatchpointFlag = TRUE;

 /****************************************************************************/
 /* - We need to get a DS to use in conjunction with 16 bit near pointers.   */
 /****************************************************************************/
 DgroupDS = AppPTB.DS;

 /****************************************************************************/
 /* - Build the initial control point message.                               */
 /****************************************************************************/
 TransLateRcToMessage(stopcode,msgbuf);


 /****************************************************************************/
 /* - We need to make sure that we have managed to build an initial view.    */
 /****************************************************************************/
 fp_focus = SetExecfp();
 if( (fp_focus == NULL) )
  Error(ERR_VIEW_CANT_BUILD, TRUE,0, NULL);

 /****************************************************************************/
 /* - Create/Start sema4s and polling threads for serial/multiple.           */
 /****************************************************************************/
 if( (SerialParallel()==SERIAL) && (SingleMultiple()==MULTIPLE) )
 {
  ALLPIDS  *p;

  /***************************************************************************/
  /* - Set a flag to indicate that this debugger is finished initializing.   */
  /***************************************************************************/
  p = GetPid( cmd.ProcessID );

  p->PidFlags.Initializing = FALSE;
  ReleaseSerialMutex();

  /***************************************************************************/
  /* - Start a thread to check for notifications when the debugger is not    */
  /*   connected.                                                            */
  /***************************************************************************/
  _beginthread( Check4ConnectRequest, NULL, 0x8000, NULL );

  /***************************************************************************/
  /* - Start a thread to detect disconnects.                                 */
  /***************************************************************************/
   SetKillChk4Disconnect( FALSE );
   _beginthread(CheckForDisConnect, NULL, 0x8000, NULL );

  if( IsParent() )
  {
   /**************************************************************************/
   /* - The parent process creates the connect sema4, the mutex sema4,       */
   /*   and starts the polling thread.                                       */
   /**************************************************************************/
   USHORT    DbgPid;

   p->Connect = CONNECTED;
   DbgPid     = p->DbgPid;

   CreateConnectSema4( DbgPid, _DBG );
   CreateSerialMutex();
   _beginthread( PollForMsgFromEsp, NULL, 0x8000, NULL );
  }
 }

 /****************************************************************************/
 /* - We may have brought the user's session to the foreground so that       */
 /*   he can do whatever he needs to do to hit the main entry point          */
 /*   of the child. Now bring the debug session back to the foreground.      */
 /****************************************************************************/
 DosSelectSession(0);

 for( ;; )
 {
  /***************************************************************************/
  /* - Build a view for the current ExecAddr.                                */
  /* - Set the flag for showing the currently executing line.                */
  /***************************************************************************/
  fp_focus = SetExecfp();
  fp_focus->flags |= AF_ZOOM;

  /***************************************************************************/
  /* - Toss any "one time" break points such as those hit by the             */
  /*   RunToCursor() function.                                               */
  /***************************************************************************/
  DropOnceBrks( GetExecAddr() );

  how = -1;
  while( how == -1 )
   how = (int)(* fp_focus->shower)(fp_focus, msgbuf);

  /***************************************************************************/
  /* - If we're just changing thread focus, then merely go build             */
  /*   the thread view.                                                      */
  /***************************************************************************/
  if ( how == GOTID )
   continue;

  /***************************************************************************/
  /* - Now run the application.                                              */
  /* - Then, build a message for the reason we stopped.                      */
  /***************************************************************************/
  SayStatusMsg("Running...Ctrl-Break to interrupt.");
  SetKillChk4Disconnect( TRUE );
  SetExecuteFlag( TRUE );
  stopcode = Go(how);
  SetExecuteFlag( FALSE );
  TransLateRcToMessage(stopcode,msgbuf);

  /***************************************************************************/
  /* - If we're debugging multiple processes over a serial connection, then  */
  /*   we need a thread that will check to see if this debugger gets         */
  /*   disconnected when the user clicks on another window.                  */
  /***************************************************************************/
  if( (SerialParallel() == SERIAL) && (SingleMultiple() == MULTIPLE) )
  {
   SetKillChk4Disconnect( FALSE );
   _beginthread(CheckForDisConnect, NULL, 0x8000, NULL );
  }
 }
}

/*****************************************************************************/
/* TransLateRcToMessage()                                                 509*/
/*                                                                           */
/* Description:                                                              */
/*      Formats a Return code from Dosdebug into message with address.       */
/* Parameters:                                                               */
/*      RetCode   -  Return Code from DosDebug.                              */
/*      msg       -  -> to buffer where the message needs to be put.         */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*      none                                                                 */
/*****************************************************************************/
extern DEBUG_REGISTER  Debug_Regs[];

static uchar IstepMsg[] = "Instruction Step";
static uchar BrkMsg[] = "Stopped at Breakpoint";

void TransLateRcToMessage(uint RetCode, char *msg)
{
 int      i;
 USHORT   Sel;                                                          /*607*/
 USHORT   Off;                                                          /*607*/

 switch (RetCode)
 {
   case TRAP_ADDR_LOAD:
   {
    BRK *pbrk;

    /*************************************************************************/
    /* - find the unreported breakpoint, report it, then mark it.            */
    /*************************************************************************/
    pbrk = pnode->allbrks;
    for(;pbrk; pbrk=pbrk->next )
    {
     if( pbrk->flag.Reported == 1 )
     {
      sprintf(msg, "Address %08X Loaded.", pbrk->brkat);
     }
    }
   }
   break;

   case TRAP_DLL_LOAD:
   {
    BRK *pbrk;

    /*************************************************************************/
    /* - find the unreported breakpoint, report it, then mark it.            */
    /*************************************************************************/
    pbrk = pnode->allbrks;
    for(;pbrk; pbrk=pbrk->next )
    {
     if( pbrk->flag.Reported == 1 )
     {
      pbrk->flag.Reported = 0;
      sprintf(msg, "%s Loaded.", pbrk->dllname);
     }
    }
   }
   break;

   case TRAP_SS:
     memcpy( msg, IstepMsg, sizeof(IstepMsg) );
     break;

   case TRAP_BPT:
     memcpy( msg, BrkMsg, sizeof(BrkMsg) );
     break;

   case TRAP_ASYNC:
     sprintf( msg, "Asynchronous stop" );
     break;

   case TRAP_EXIT:
   {
    UCHAR bitness;

     bitness = GetBitness(GetExecAddr());
     if(bitness == BIT32)
       sprintf(msg, "Exit at %08X", GetExecAddr());
     else
     {
       Code_Flat2SelOff(GetExecAddr(),&Sel,&Off);
       sprintf(msg, "Exit at %04X:%04X",Sel,Off);
     }
   }
   AppTerminated = TRUE;
   break;

   case TRAP_DATA:
     sprintf( msg, "Data breakpoint" );                                 /*521*/
     break;

   case TRAP_ABEND:
     sprintf( msg, "DosDebug Anomaly");                                 /*521*/
     break;

   case TRAP_INTERLOCK:
     beep();                                                            /*920*/
     Error(ERR_TRAP_INTERLOCK,TRUE,0);                                  /*920*/
     break;

   case TRAP_SS_ERROR:
     beep();                                                            /*901*/
     sprintf( msg, "Single Step Error");                                /*901*/
     break;

   case XCPT_GUARD_PAGE_VIOLATION:
   case XCPT_UNABLE_TO_GROW_STACK:
   case XCPT_DATATYPE_MISALIGNMENT:
   case XCPT_ACCESS_VIOLATION:
   case XCPT_ILLEGAL_INSTRUCTION:
   case XCPT_FLOAT_DENORMAL_OPERAND:
   case XCPT_FLOAT_DIVIDE_BY_ZERO:
   case XCPT_FLOAT_INEXACT_RESULT:
   case XCPT_FLOAT_INVALID_OPERATION:
   case XCPT_FLOAT_OVERFLOW:
   case XCPT_FLOAT_STACK_CHECK:
   case XCPT_FLOAT_UNDERFLOW:
   case XCPT_INTEGER_DIVIDE_BY_ZERO:
   case XCPT_INTEGER_OVERFLOW:
   case XCPT_PRIVILEGED_INSTRUCTION:
   case XCPT_IN_PAGE_ERROR:
   case XCPT_PROCESS_TERMINATE:
   case XCPT_ASYNC_PROCESS_TERMINATE:
   case XCPT_NONCONTINUABLE_EXCEPTION:
   case XCPT_INVALID_DISPOSITION:
   case XCPT_INVALID_LOCK_SEQUENCE:
   case XCPT_ARRAY_BOUNDS_EXCEEDED:
   case XCPT_UNWIND:
   case XCPT_BAD_STACK:
   case XCPT_INVALID_UNWIND_TARGET:
   case XCPT_SIGNAL:
   case XCPT_PROGRAM:
   {
     int n;
     beep();

     n = GetExceptionIndex( RetCode);

     if( AppPTB.CSAtr )
      sprintf( msg, "Exception-%s At %08X", GetExceptionType(n), AppPTB.EIP );
     else
     {
      Sel = AppPTB.CS;
      Off = LoFlat(AppPTB.EIP);
      sprintf( msg, "Exception-%s At %04X:%04X", GetExceptionType(n), Sel, Off );
     }
     break;
   }

   case TRAP_PROC:
     sprintf(msg,"Unable to debug new process");                        /*521*/
     break;

   case TRAP_NMI:
   case TRAP_FP:
     break;

   case TRAP_WATCH:
     /************************************************************************/
     /* Scan the hardware debug registers for the watch point index hit,  602*/
     /* and display the register number and address.                      602*/
     /************************************************************************/
     for (i = 0 ; i < NODEBUGREGS ; i++ )                               /*602*/
     {                                                                  /*602*/
         if( Debug_Regs[i].Wpindex == AppPTB.Index )                    /*602*/
         {                                                              /*602*/
            Debug_Regs[i].Status = ENABLED;                             /*602*/
            Debug_Regs[i].Wpindex = 0;                                  /*602*/
            sprintf(msg,"Watch Point %1d hit for Address %08X",         /*602*/
                        i+1,Debug_Regs[i].Address);                     /*602*/
            break;                                                      /*602*/
         }                                                              /*602*/
     }                                                                  /*602*/
     break;                                                             /*602*/

   default:
     printf( "\nSD386 System Error %u\n", RetCode);                     /*521*/
     panic(OOstop);
 }
}

static BOOL ExecuteFlag;
void SetExecuteFlag( BOOL TorF ) { ExecuteFlag = TorF; }
BOOL IsExecuteFlag( void ) { return(ExecuteFlag); }
