/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   term.c                                                                  */
/*                                                                           */
/* Description:                                                              */
/*   Functions for terminating and restarting.                               */
/*                                                                           */
/*                                                                           */
/* History:                                                                  */
/*                                                                           */
/*   02/08/91 Creation of 32-bit SD86, from 16-bit version.                  */
/*                                                                           */
/*...16->32 port.                                                            */
/*...                                                                        */
/*... 02/08/91  110   Srinivas  port to 32 bit.                              */
/*                                                                           */
/*...Release 1.00 (Pre-release 1)                                            */
/*...                                                                        */
/*... 08/29/91  215   srinivas    Add support for HLL format                 */
/*... 08/29/91  236   srinivas    blinking restart & other restart problems. */
/*...                                                                        */
/*...Release 1.00 (Pre-release 107 11/13/91)                                 */
/*...                                                                        */
/*... 11/18/91  401   Srinivas  Co processor Register Display.               */
/*...                                                                        */
/*...Release 1.00 (Pre-release 108 12/05/91)                                 */
/*...                                                                        */
/*... 02/12/92  521   Srinivas  Port to C-Set/2.                             */
/*...                                                                        */
/*...Release 1.00 (03/03/92)                                                 */
/*...                                                                        */
/*... 03/10/92  602   Srinivas  Hooking up watch points.                     */
/*... 03/20/92  607   Srinivas  CRMA fixes.                                  */
/*... 03/21/92  608   Srinivas  Restart Not Cleaning up the Session properly.*/
/*...                                                                        */
/*...Release 1.01 (04/03/92)                                                 */
/*...                                                                        */
/*... 10/15/92  710   Joe       Rewrite term.c to fix term/restart probs.    */
/*...                                                                        */
/*...Release 1.02 (10/22/92)                                                 */
/*... 10/28/92  800   Joe       Add code to get exe stack size.              */
/*...                           Use in quit/restart.                         */
/**Includes*******************************************************************/

#include "all.h"                        /* SD86 include files                */

/**Defines *******************************************************************/
extern PROCESS_NODE *pnode;             /* -> to process list.               */
extern int          AppTerminated;      /* application terminated flag.      */
extern uint         VideoRows;          /* # of rows per screen              */
extern jmp_buf      RestartContext;     /* stack/registers for restart.      */
extern CmdParms     cmd;

 int
NormalQuit( int ExitDebug )
{
 APIRET   rc;
 DEBFILE *pdf;
 CSR      DosCsr;
 ALLPIDS *p;

 pdf = pnode->ExeStruct;

 /****************************************************************************/
 /* - Kill the listen/polling threads.                                       */
 /****************************************************************************/
 if( IsParent() && (ConnectType() != BOUND ) )
 {
  if( SerialParallel() == PARALLEL)
   SendMsgToDbgQue(DBG_QMSG_KILL_LISTEN,NULL,0);
  if( (SerialParallel() == SERIAL) && (SingleMultiple() == MULTIPLE) )
  {
   SetKillChk4Disconnect( TRUE );
   SetPollingThreadFlag( THREAD_TERM );
   while( GetPollingThreadFlag() != NOT_RUNNING ){ DosSleep(100) ;}
  }
  /**************************************************************************/
  /* - Send a message to all of the child debuggers telling them that       */
  /*   we're terminating their probes.                                      */
  /**************************************************************************/
  for( p = GetAllpids(); p ; p = p->next )
  {
   if( p->pid != DbgGetProcessID() )
   {
    DBG_QUE_ELEMENT Qelement;

    Qelement.pid = p->DbgPid;

    SendMsgToDbgTermQue(DBG_PROBE_TERM, &Qelement, sizeof(Qelement) );
   }
  }
 }
 /****************************************************************************/
 /* - Thaw any threads that may be frozen.                                   */
 /* - Call DosExit() in the apps address space.                              */
 /****************************************************************************/
 xThawThread( 0 );                                                      /*827*/
 rc = xNormalQuit(AppTerminated,pdf->mte,pdf->EntryExitPt);

 /****************************************************************************/
 /* - Clean up the resources used by the debugger:                           */
 /*    - the queue...wait until the queue is terminated.                     */
 /*    - the connect sema4 for serial connections.                           */
 /****************************************************************************/
 if( IsEspRemote() && IsParent() )
 {
  if( SingleMultiple() == MULTIPLE )
  {
   /**************************************************************************/
   /* - send a message to the que to kill all the child debuggers            */
   /* - then wait until they are all dead.                                   */
   /**************************************************************************/
   ResetAllDbgsAreDeadFlag();
   SendMsgToDbgQue(DBG_QMSG_PARENT_TERM,NULL,0);
   while( AllDbgsAreDead() == FALSE ){ DosSleep(100) ;}
  }
  SendMsgToDbgQue(DBG_QMSG_QUE_TERM,NULL,0);
 }
 CloseConnectSema4();

 /****************************************************************************/
 /* - Send a return code back to restart.                                    */
 /****************************************************************************/
 if( ExitDebug == FALSE )
  return(rc);

 /****************************************************************************/
 /* - Ask the user if he wants to terminate the probe.                       */
 /****************************************************************************/
 {
#if 0
  uint key = 0;
  if( IsEspRemote() )
   key = MsgYorN(HELP_QUIT_ESP);
  if( key == key_y || key == key_Y )
#endif
   xTerminateESP();
 }

 /****************************************************************************/
 /* - Now, close the connection and the serial mutex sema4 if there is one.  */
 /****************************************************************************/
 ConnectClose( DEFAULT_HANDLE );
 CloseSerialMutex();

 if( IsParent() == FALSE )
 {
  DBG_QUE_ELEMENT Qelement;
  memset( &Qelement, 0, sizeof(Qelement) );
  Qelement.pid = DbgGetProcessID();

  SendMsgToDbgQue( DBG_QMSG_CHILD_TERM, &Qelement, sizeof(Qelement) );
 }

 /****************************************************************************/
 /* - disconnect the signal handler.                                         */
 /****************************************************************************/
 UnRegisterExceptionHandler( );

 /****************************************************************************/
 /* - Toggle alt-esc and ctrl-esc back on.                                   */
 /****************************************************************************/
 if( IsHotKey() ) SetHotKey();

 ClrPhyScr( 0, VideoRows-1, vaClear );
 DosCsr.col = 0;
 DosCsr.row = ( uchar )(VideoRows-1);
 PutCsr( &DosCsr );
 /****************************************************************************/
 /* - MSH may be waiting on the event semaphore. Post, then close.           */
 /****************************************************************************/
#if 0
 CloseEventSemaphores();
#endif

 KillEditorSession();

 exit(0);
 return(0);                             /* keep the compiler happy.          */
}                                       /* end NormalQuit()                  */

/*****************************************************************************/
/* Restart()                                                                 */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Restart the debuggee.                                                   */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
void Restart()                          /* restart the debuggee.             */
{                                       /*                                   */
 int ExitDebug;

 char BadSessTerm[100] = "\r Your session may not have terminated properly\r\
 Quit SD386 and start again.";

 /***************************************************************************/
 /* Since BadSessTerm is a local variable and the string is copied afresh   */
 /* everytime, we dont have to worry about showhelpbox modifying the message*/
 /***************************************************************************/

 SayMsgBox3("Restarting...");           /* could be a delay.                 */
 /****************************************************************************/
 /*  prepare breakpoints for restart.                                        */
 /****************************************************************************/
 SaveBrks(pnode);

 ExitDebug = FALSE;
 if( IsParent() == FALSE )
  ExitDebug = TRUE;

 if( NormalQuit( ExitDebug ) )
  CuaShowHelpBox( BadSessTerm );
 longjmp( RestartContext, 1);
}
