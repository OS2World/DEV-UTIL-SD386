/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   threads.c                                                               */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*  Thread handling routines.                                                */
/*                                                                           */
/* History:                                                                  */
/*                                                                           */
/*...16->32 port.                                                            */
/*...                                                                        */
/*... 02/08/91  100   made changes for 32-bit compilation.                   */
/*... 02/08/91  101   made changes for 32-bit compilation. ( by Joe C. )     */
/*    04/04/91  107   Change calls to PeekData/PokeData to DBGet/DBPut.   107*/
/*                                                                           */
/*...Release 1.00 (Pre-release 1)                                            */
/*...                                                                        */
/*... 07/18/91  216   srinivas  Better processing for FAKE MID problems.     */
/*...                                                                        */
/*...Release 1.08 (Pre-release 108 )       )                                 */
/*...                                                                        */
/*... 01/17/92  504   Joe       Can't step into 16 bit dll on 6.177H.        */
/*... 02/12/92  521   Joe       Port to C-Set/2.                             */
/*...                                                                        */
/*...Release 1.00 (03/03/92)                                                 */
/*...                                                                        */
/*... 03/20/92  607   Srinivas  CRMA fixes.                                  */
/*...                                                                        */
/*...Release 1.01 (04/03/92)                                                 */
/*...                                                                        */
/*... 10/06/92  709   Joe       Access violation at unknown addr causes trap.*/
/*... 09/16/93  901   Joe       Add code to handle resource interlock error. */
/*... 12/06/93  907   Joe       Fix for not updating thread list.            */
/**Includes*******************************************************************/

#include "all.h"                        /* SD86 include files                */

/**External declararions******************************************************/

extern uint         ProcessID;
extern PtraceBuffer AppPTB;
extern AFILE    *fp_focus;              /* a holder for an afile node.       */
extern PROCESS_NODE *pnode;                                             /*827*/

TSTATE        *thead = {NULL};          /* thread list head pointer          */
TSTATE        *tail = {(TSTATE *)(void*)&thead};       /* last node in list  */
/*****************************************************************************/
/* InitThreads()                                                             */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Allocate memory for thread 1 and initialize the linked list of threads. */
/*   Establish a pointer to the executing thread node which is thread one.   */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   void                                                                    */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   void                                                                    */
/*                                                                           */
/*****************************************************************************/
 void
InitThreadList()                        /*                                   */
{                                       /*                                   */
 TSTATE  *np;                           /* pointer to thread structure node  */
                                        /*                                   */
 thead = NULL;                          /* thread list head pointer          */
 tail = (TSTATE *)&thead;               /* last node in list                 */
 np=(TSTATE *)Talloc(sizeof(TSTATE));   /* allocate one node              521*/
 np->prev   = NULL;                     /* connect back pointer              */
 tail->next = np;                       /* add to end of the list            */
 np->next   = NULL;                     /* ground this node                  */
 tail = np;                             /* establish tail pointer            */
 np->tid = 1;                           /* this is thread 1                  */
}                                       /* end InitThreadList()              */
/*****************************************************************************/
/* FreeThreadList()                                                          */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Deallocate the thread list memory for reinitialization.                 */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   void                                                                    */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   void                                                                    */
/*                                                                           */
/*****************************************************************************/
 void
FreeThreadList()                        /*                                   */
{                                       /*                                   */
 TSTATE  *np;                           /* pointer to thread structure node  */
 TSTATE  *npnext;                       /* -> to thread structure node    521*/
 np = thead ;                           /* scan the list of threads       521*/
 while (np)                             /* while not grounded node        521*/
 {                                      /*                                521*/
   npnext = np->next;                   /* -> to next thread node         521*/
   Tfree( (void*)np );                   /* free the memory                521*/
   np = npnext;                         /* set the pointer to cp          521*/
 }                                      /*                                521*/
 thead = NULL;                          /* thread list head pointer       822*/
 tail = (TSTATE *)&thead;               /* last node in list              822*/
}                                       /* end FreeThreadList()              */

/*****************************************************************************/
/* GetThdDbgState()                                                          */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Get the Debug State of a thread.                                        */
/*    0 = Freeze                                                             */
/*    1 = Thaw                                                               */
/*    2 = Terminating ( A result of DBG_N_ThreadTerm from DosDebug )         */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   tid      thread id.                                                     */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   state    debug state of the thread.                                     */
/*                                                                           */
/*                                                                           */
/*****************************************************************************/
 uchar
GetThdDbgState( uint tid )              /*                                   */
{                                       /*                                   */
 TSTATE      *np;                       /* thread state node pointer         */
                                        /*                                   */
 for ( np = thead;                      /* scan the list of threads          */
       np->tid != tid;                  /*                                   */
       np = np->next                    /*                                   */
     ){;}                               /*                                   */
 return(np->ts.DebugState);             /* return the debug state            */
}                                       /* end of GetThdDbgState()           */

/*****************************************************************************/
/* GetThreadState()                                                          */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Get the Thread State of a thread.                                       */
/*    0 = Runnable                                                           */
/*    1 = Suspended                                                          */
/*    2 = Blocked                                                            */
/*    3 = Critical                                                           */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   tid      thread id.                                                     */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   state    thread state of the thread.                                    */
/*                                                                           */
/*                                                                           */
/*****************************************************************************/
 uchar
GetThreadState( uint tid )              /*                                   */
{                                       /*                                   */
 TSTATE      *np;                       /* thread state node pointer         */
                                        /*                                   */
 if(thead) {
 for ( np = thead;                      /* scan the list of threads          */
       np->tid != tid;                  /*                                   */
       np = np->next                    /*                                   */
     ){;}                               /*                                   */
 if(np) return(np->ts.ThreadState);            /* return the debug state            */
 }
 return(TRC_C_ENDED); /*Error condition. No threads or thread not found.*/
}                                       /* end of GetThreadState()           */

/*****************************************************************************/
/* GetThreadTSTATE()                                                         */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Get the TSTATE structure for the given tid.                             */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   tid      thread id.                                                     */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   np       -> to TSTATE structure.                                        */
/*                                                                           */
/*                                                                           */
/*****************************************************************************/
TSTATE * GetThreadTSTATE( uint tid )
{
 TSTATE      *np;

 for ( np = thead;
       np->tid != tid;
       np = np->next
     ){;}
 return(np);
}

/*****************************************************************************/
/* BuildThreadList()                                                         */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Build a double linked list of  thread info.                             */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   void                                                                    */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   void                                                                    */
/*                                                                           */
/* Notes:                                                                    */
/*                                                                           */
/*   We assume that once a thread id is established it is valid for the      */
/*   duration of the process. It may reach a terminating condition but we    */
/*   let the operating system and DosDebug manage that.                      */
/*                                                                           */
/*****************************************************************************/
 void
BuildThreadList()
{
 uint         tid;                      /* thread id                         */
 uint         mid;                      /* module id                         */
 LNOTAB      *pLnoTabEntry;
 uint         eip;                      /* flat address in thread.           */
 TSTATE      *np;                       /* thread state node pointer         */
 DEBFILE     *pdf;                      /* -> to debug file structure        */
 THREADINFO   buffer[100];                                              /*827*/
 ULONG        ntids;                                                    /*827*/

 /****************************************************************************/
 /* - Get the thread information and free up the old list.                   */
 /****************************************************************************/
 ntids = xGetThreadInfo( buffer );
 FreeThreadList();                                                      /*907*/
 for( tid = 1; tid<=ntids; tid++ )
 {
  /***************************************************************************/
  /* - allocate.                                                             */
  /* - fill in the info.                                                     */
  /***************************************************************************/
  np=(TSTATE *)Talloc(sizeof(TSTATE));
  np->prev   = tail;
  tail->next = np;
  np->next   = NULL;
  tail = np;

  np->ts  = buffer[tid-1].ts;
  np->ip  = buffer[tid-1].eip;
  np->tid = buffer[tid-1].tid;                                          /*906*/
 }
/*****************************************************************************/
/*                                                                           */
/* Now we want to scan the list we just built and add additional structure   */
/* info.                                                                     */
/*                                                                           */
/*****************************************************************************/
 pLnoTabEntry = NULL;
 for ( np = thead;                      /* scan the list we just built       */
       np != NULL;
       np = np->next
     )
 {
  eip = np->ip;
  mid = 0;

  /***************************************************************************/
  /* - scan the exe/dlls to find the mid, lno, and sfi for this eip.         */
  /***************************************************************************/
  for (pdf=pnode->ExeStruct; (pdf != NULL) && (mid == 0); pdf = pdf->next )                                 /*                                   */
  {
   mid=DBMapInstAddr(eip, &pLnoTabEntry, pdf);
   np->pdf = pdf;
  }

  np->mid = mid;
  if( pLnoTabEntry != NULL )
  {
   np->lno = pLnoTabEntry->lno;
   np->sfi = pLnoTabEntry->sfi;
  }

  if (mid==0)                           /* if we can't locate ourselves      */
  {                                     /* then fall into the fake mid       */
   np->mid = FAKEMID;                   /* this is the fake mid              */
   np->pdf = pnode->ExeStruct;          /* then assume the EXE file          */
  }
 }                                      /*                                   */
}                                       /* end of BuildThreadList()          */

/*****************************************************************************/
/* RunThread()                                                               */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   set user selection to the executing thread                              */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   threadID    input - thread id selected to be executed                   */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   0           not used.                                                   */
/*                                                                           */
/*****************************************************************************/
 uint
RunThread( uint threadID )
{
 uchar StatusMsg[20];

 SetExecValues( threadID, TRUE);
 SetActFrames();
 fp_focus = SetExecfp() ;

 sprintf(StatusMsg, "Selecting Tid %-3d", threadID);
 SayMsgBox2( StatusMsg, 450UL );

 if( fp_focus )
  fp_focus->flags |= AF_ZOOM;

 return( 0 );
}
