/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*                                                                           */
/*  sema4.c                                                                  */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   These functions are for serial connections only.  Functions for         */
/*   creating, setting, querying, and waiting for the connect sema4          */
/*   resource. Posting is done in the queue threads.                         */
/*                                                                           */
/* History:                                                                  */
/*                                                                           */
/*   10/14/94 Created.                                                       */
/*                                                                           */
/*****************************************************************************/

#include "all.h"

static HEV      ConnectSema4;

/*****************************************************************************/
/* CreateConnectSema4()                                                      */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Create a connect sema4 for the pid parameter.  Register the sema4       */
/*   with the queue.                                                         */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   pid                 pid of the debugger or probe.                       */
/*                       process id of the probe that this sema4 is for.     */
/*   DbgOrEsp            whether the create occurs in the dbg or esp.        */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
#define INIT_STATE_RESET  FALSE
#define INIT_STATE_POSTED TRUE

void CreateConnectSema4( USHORT pid, int DbgOrEsp )
{
 BOOL     InitState;
 ALLPIDS *p;

 /****************************************************************************/
 /* - Note that the QueCaller is no longer used.                             */
 /****************************************************************************/

 InitState = INIT_STATE_RESET;
 if( (DbgOrEsp == _DBG) && IsParent() )
  InitState = INIT_STATE_POSTED;

 DosCreateEventSem(NULL, &ConnectSema4, DC_SEM_SHARED, InitState );

#ifdef __ESP__
/*---------------------------------------------------------------------------*/
  p = GetEspPid( pid );
/*---------------------------------------------------------------------------*/
#endif

#ifdef __DBG__
/*---------------------------------------------------------------------------*/
  p = GetDbgPid( pid );
/*---------------------------------------------------------------------------*/
#endif

 p->ConnectSema4 = ConnectSema4;

 if( IsVerbose() ) {printf("\nConnect Sema4 Created");fflush(0);}
}

/*****************************************************************************/
/* SerialConnect()                                                           */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   For serial connections, these functions the connect/disconnect of       */
/*   the debuggers and the probes.                                           */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   ConnectOrDisConnect Sema4 action to take.                               */
/*   pid                 process id that the action applies to.              */
/*   DbgOrEsp            whether the action occurs in the dbg or esp.        */
/*   QueCaller           either the esp or dbg que message sender.           */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
void SerialConnect( int   ConnectOrDisConnect,
                    ULONG pid,
                    int   DbgOrEsp,
                    void  (* QueCaller)( ULONG , void * , ULONG ) )
{
 struct
 {
  USHORT pid1;
  USHORT pid2;
 }Qelement;

 ULONG QueMsg;

 /****************************************************************************/
 /* - If a sema4 was not created for this debugger/probe then just return.   */
 /****************************************************************************/
 if( ConnectSema4 == 0 )
  return;

 /****************************************************************************/
 /* - When sending the message to dbg, the pid goes in element 1.            */
 /* - When sending the message to esp, the pid goes in element 2.            */
 /****************************************************************************/
 if( DbgOrEsp == _DBG )
  Qelement.pid1 = (USHORT)pid;
 else
  Qelement.pid2 = (USHORT)pid;

 switch( ConnectOrDisConnect )
 {
  case CONNECT_WAIT:

   /**************************************************************************/
   /* - send a connect message to the que and wait for a post.               */
   /*                                                                        */
   /* Note: At this point in time, the CONNECT_WAIT is only generated        */
   /*       in an esp.                                                       */
   /**************************************************************************/
   QueMsg = ESP_QMSG_CONNECT_REQUEST;

   (* QueCaller)( QueMsg, &Qelement, sizeof(Qelement) );

   WaitConnectSema4(ConnectSema4);
   break;

  case DISCONNECT:

   /**************************************************************************/
   /* - set the sema4 for this dbg/esp and inform the que of the disconnect. */
   /**************************************************************************/
   SetConnectSema4(&ConnectSema4, FALSE );

   QueMsg = DBG_QMSG_DISCONNECT;
   if( DbgOrEsp == _ESP )
    QueMsg = ESP_QMSG_DISCONNECT;

   (* QueCaller)( QueMsg, &Qelement, sizeof(Qelement) );

   break;

  case DISCONNECT_WAIT:

   /**************************************************************************/
   /* - set the sema4 for this dbg/esp and inform the que of the disconnect. */
   /* - wait for a post.                                                     */
   /**************************************************************************/
   SetConnectSema4(&ConnectSema4, FALSE );

   QueMsg = DBG_QMSG_DISCONNECT;
   if( DbgOrEsp == _ESP )
    QueMsg = ESP_QMSG_DISCONNECT;

   (* QueCaller)( QueMsg, &Qelement, sizeof(Qelement) );

   WaitConnectSema4(ConnectSema4);
   break;

  case JUST_WAIT:

   /**************************************************************************/
   /* - just wait for a post.                                                */
   /**************************************************************************/
   WaitConnectSema4(ConnectSema4);
   break;

  case SET_WAIT:

   /**************************************************************************/
   /* - set the sema4 for this dbg/esp. Do not inform the que.               */
   /* - wait for a post.                                                     */
   /**************************************************************************/
   SetConnectSema4(&ConnectSema4, FALSE );
   WaitConnectSema4(ConnectSema4);
   break;
 }
}

/*****************************************************************************/
/* SetConnectSema4()                                                         */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   For serial connections, set( actually reset) the connect sema4.         */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   pConnectSema4     ->connect sema4.                                      */
/*   TorF                TRUE==>open the sema4...it's owned by another       */
/*                                               process.                    */
/*                       FALSE==>owned by calling process...no need to open. */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
void SetConnectSema4( HEV *pConnectSema4, BOOL TorF )
{
 ULONG      Count = 0;

 /****************************************************************************/
 /* - open the event sema4 if it's not owned by the calling process.         */
 /****************************************************************************/
 if( TorF == TRUE )
  DosOpenEventSem( NULL, pConnectSema4 );

 /****************************************************************************/
 /* - reset the sema4.                                                       */
 /****************************************************************************/
 DosResetEventSem( *pConnectSema4, &Count );

 /****************************************************************************/
 /* - If the sema4 is not owned by the calling process AND                   */
 /*   the sema4 count is not 0, then close the sema4.                        */
 /* - Closing the sema4 wiht a count=0 obliterates the sema4.                */
 /****************************************************************************/
 if( TorF == TRUE )
 {
  DosQueryEventSem( *pConnectSema4, &Count);
  if( Count != 0 )
   DosCloseEventSem( *pConnectSema4 );
 }
}

/*****************************************************************************/
/* - wait for a sema4 to be posted.                                          */
/*****************************************************************************/
void WaitConnectSema4( HEV hSema4 )
{
 APIRET rc;

getbackinthere:
 rc = DosWaitEventSem( hSema4, SEM_INDEFINITE_WAIT);
 if( rc == ERROR_INTERRUPT ) goto getbackinthere;
}

/*****************************************************************************/
/* - close a sema4.                                                          */
/*****************************************************************************/
void CloseConnectSema4( void )
{
 /****************************************************************************/
 /* - If a sema4 was not created for this debugger/probe then just return.   */
 /****************************************************************************/
 if( ConnectSema4 == 0 )
  return;

 DosCloseEventSem(ConnectSema4);

 ConnectSema4 = 0;
 if( IsVerbose() ) {printf("\nConnect Sema4 Closed");fflush(0);}
}

/*****************************************************************************/
/* - test a sema4 to see if it's reset or posted.                            */
/*****************************************************************************/
int QueryConnectSema4( void )
{
 ULONG count = 0;

 /****************************************************************************/
 /* - If a sema4 was not created for this debugger/probe then just return.   */
 /****************************************************************************/
 if( ConnectSema4 == 0 )
  return( SEMA4_NOT );

 DosQueryEventSem( ConnectSema4, &count );

 if( count == 0 )
  return( SEMA4_RESET );
 return( SEMA4_POSTED );
}

/*****************************************************************************/
/* PostConnectSema4()                                                        */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   For serial connections only. Open, if need be, and post a connect       */
/*   sema4 to release a debugger. Close the sema4, if need be.               */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*  pConnectSema4  -> to the connect sema4 to be posted.                     */
/*  TorF              whether or not to open the sema4 before posting.       */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
void PostConnectSema4( HEV *pConnectSema4, BOOL TorF )
{
 ULONG  count;
 APIRET rc;

 if( TorF == TRUE )
  DosOpenEventSem( NULL, pConnectSema4 );

 rc = DosPostEventSem( *pConnectSema4 );
 if(rc)
  {printf("\npost sema4 failure rc=%d",rc);fflush(0);exit(0);}

 if( TorF == TRUE )
 {
  DosQueryEventSem( *pConnectSema4, &count);
  if( count != 0 )
   DosCloseEventSem( *pConnectSema4 );
 }
}

/*****************************************************************************/
/* CreateSerialMutexSema4()                                                  */
/* OpenSerialMutexSema4()                                                    */
/* RequestSerialMutexSema4()                                                 */
/* ReleaseSerialMutexSema4()                                                 */
/* CloseSerialMutexSema4()                                                   */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   These functions provide for the management of access to the serial      */
/*   channel when debugging multiple processes.                              */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   This function is only called by the dbg end of the connection.          */
/*                                                                           */
/*****************************************************************************/

static char SerialMutexSema4[]="\\SEM32\\SERSEMA4";
static HMTX hmtxSerialMutexSema4 = 0;

void CreateSerialMutex( void )
{
 /****************************************************************************/
 /* - Create a named mutex sema4 initially unowned.                          */
 /* - The DC_SEM_SHARED flag is ignored, but it's included to simply         */
 /*   document the fact that this sema4 is shared among the debuggers.       */
 /****************************************************************************/
 DosCreateMutexSem(  SerialMutexSema4,
                    &hmtxSerialMutexSema4,
                     DC_SEM_SHARED,
                     FALSE);
 if( IsVerbose() ) {printf("\nMutex Sema4 Created");fflush(0);}
}

void OpenSerialMutex( void )
{
 APIRET rc;
 /****************************************************************************/
 /* - Open the serial mutex sema4.                                           */
 /* - This is done in a child debugger to gain access to the sema4 created   */
 /*   by the parent debugger.                                                */
 /****************************************************************************/
 rc = DosOpenMutexSem( SerialMutexSema4, &hmtxSerialMutexSema4 );
 if(rc)
  {printf("\nmutex error rc=%d",rc);fflush(0);exit(0);}

}

void RequestSerialMutex( void )
{
 APIRET rc;

 if( !hmtxSerialMutexSema4 ) return;

 /****************************************************************************/
 /* - Request ownership of the serial mutex sema4 for this process.          */
 /****************************************************************************/
 rc = DosRequestMutexSem( hmtxSerialMutexSema4, SEM_INDEFINITE_WAIT );
 if(rc)
  {printf("\nmutex error rc=%d",rc);fflush(0);exit(0);}

}

void ReleaseSerialMutex( void )
{
 if( !hmtxSerialMutexSema4 ) return;
 /****************************************************************************/
 /* - Release ownership of the serial mutex sema4 for this process.          */
 /****************************************************************************/
 DosReleaseMutexSem( hmtxSerialMutexSema4 );
}

void CloseSerialMutex( void )
{
 if( !hmtxSerialMutexSema4 ) return;
 /****************************************************************************/
 /* - Close the serial mutex sema4 for this process.                         */
 /* - The final close by the parent debugger will remove the sema4 from      */
 /*   the system.                                                            */
 /****************************************************************************/
 DosCloseMutexSem( hmtxSerialMutexSema4 );
 if( IsVerbose() ) {printf("\nMutex Sema4 Closed");fflush(0);}
}
