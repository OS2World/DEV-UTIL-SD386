/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   router.c                                                             919*/
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*  These functions initialize the connection to a remote esp and            */
/*  routes the remote send/receive calls through that connection.            */
/*                                                                           */
/* History:                                                                  */
/*                                                                           */
/*   07/08/94 Created.                                                       */
/*                                                                           */
/*****************************************************************************/
#include "all.h"

static CONNECTION Connection = {BOUND};
static LHANDLE    LsnHandle;
static BOOL       HaltCommunicationsFlag;

void  SetHaltCommunicationsFlag( void ) { HaltCommunicationsFlag=TRUE;}


/*****************************************************************************/
/* - define the connection to the router.                                    */
/*****************************************************************************/
void SendConnectionToRouter( CONNECTION *pConnection )
{
 Connection = *pConnection;
}

/*****************************************************************************/
/* - return the local or remote esp connection.                              */
/*****************************************************************************/
BOOL IsEspRemote( void )
{
 BOOL rc = FALSE;

 if( Connection.ConnectType != BOUND )
  rc = TRUE;
 return(rc);
}

/*****************************************************************************/
/* - return the type of the connection.                                      */
/*****************************************************************************/
int ConnectType( void )
{
 return( Connection.ConnectType );
}

/*****************************************************************************/
/* - return serial/parallel.                                                 */
/*****************************************************************************/
int SerialParallel( void )
{
 int type;

 type = SERIAL;
 if( Connection.ConnectType != ASYNC )
  type = PARALLEL;

 return( type );
}


/*****************************************************************************/
/* - Initialize a Dbg/Esp connection.                                        */
/*****************************************************************************/
APIRET ConnectInit( int *pRcMoreInfo )
{
 APIRET rc = 0;

 HaltCommunicationsFlag = FALSE;

 switch( Connection.ConnectType )
 {
  case BOUND:
  default:
   break;

  case ASYNC:
   if( IsParent() == TRUE )
    AsyncInit( &Connection );
   LsnHandle = GetComHandle();
   break;

  case LOCAL_PIPE:
   rc = PipeInit( &Connection, &LsnHandle );
   break;

  case _NETBIOS:
  {
   /**************************************************************************/
   /* - for some strange reason, istink( alias ilink )                       */
   /*   trashes the first character of WaitMsg...so we have to put it back.  */
   /**************************************************************************/
   char WaitMsg[29] = "\nWaiting Netbios Connect...";

   WaitMsg[0] = '\n';
   printf( "%s", WaitMsg );fflush(0);
   rc = NetBiosInit( &Connection, &LsnHandle, pRcMoreInfo);
  }
  break;

  case SOCKET:
  {
   /**************************************************************************/
   /* - for some strange reason, ilink                                       */
   /*   trashes the first character of WaitMsg...so we have to put it back.  */
   /**************************************************************************/
   char WaitMsg[29] = "\nWaiting Sockets Connect...";

   WaitMsg[0] = '\n';
   printf( "%s", WaitMsg );fflush(0);
   rc = SockInit( &Connection, &LsnHandle, pRcMoreInfo);
  }
  break;

 }
 return(rc);
}

/*****************************************************************************/
/* - Initialize a Dbg/Esp connection.                                        */
/*****************************************************************************/
void ConnectClose( LHANDLE handle )
{
 LHANDLE Lsn_Handle;


 if( handle == DEFAULT_HANDLE )
  Lsn_Handle = LsnHandle;
 else
  Lsn_Handle = handle;

 switch( Connection.ConnectType )
 {
  case BOUND:
  default:
   break;

  case ASYNC:
   if( IsParent() == TRUE )
    AsyncClose( &Connection );
   break;

  case LOCAL_PIPE:
   PipeClose( &Connection, Lsn_Handle );
   break;

  case _NETBIOS:
   NetBiosClose( Lsn_Handle );
   break;

  case SOCKET:
   SockClose( Lsn_Handle );
   if( IsParent() && (handle == DEFAULT_HANDLE) )
    SockClose( 0 );
   break;
 }
}

/*****************************************************************************/
/* - grab another Dbg/Esp connection.                                        */
/* - resources have already been allocated.                                  */
/*****************************************************************************/
void OpenAnotherConnection( LHANDLE *pLsnHandle )
{
 switch( Connection.ConnectType )
 {
  case BOUND:
  default:
   break;

  case ASYNC:
   *pLsnHandle = GetComHandle();
   break;

  case LOCAL_PIPE:
   PipeInit( &Connection, pLsnHandle );
   break;

  case _NETBIOS:
   NetBiosOriginateSession( &Connection , pLsnHandle );
   break;

  case SOCKET:
  {
   int SocketDescriptor;

   SocketDescriptor = SockGetAnotherSocket( &Connection );
   *pLsnHandle      = SocketDescriptor;
  }
  break;
 }
}

/*****************************************************************************/
/* RmtSend()                                                                 */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Sends a buffer of data over a network connection.                       */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   handle     the handle of the connection to send the buffer to if        */
/*              not the default.                                             */
/*   pBuf       -> our transmit buffer.                                      */
/*   Len        number of bytes to transmit.                                 */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   handle == DEFAULT_HANDLE ==> use the default handle.                    */
/*                                                                           */
/*****************************************************************************/
void RmtSend( LHANDLE handle ,PVOID pBuf, ULONG Len )
{
 LHANDLE Lsn_Handle;

 /****************************************************************************/
 /* - When we are terminating the world, we want to halt any further         */
 /*   communications. So, what we do is just sit here and wait for the       */
 /*   operating system to kill this thread.                                  */
 /****************************************************************************/
 if( HaltCommunicationsFlag == TRUE )
 {
   if( IsVerbose() ){ printf("\ncommunications halted");fflush(0);}
   for(;;){ DosSleep(60000); }
 }

 Lsn_Handle = LsnHandle;
 if( handle != DEFAULT_HANDLE )
  Lsn_Handle = handle;

 switch( Connection.ConnectType )
 {
  case BOUND:
  default:
   break;

  case ASYNC:
   AsyncSend( pBuf, Len );
   break;

  case LOCAL_PIPE:
   PipeSend( Lsn_Handle, pBuf, Len );
   break;

  case _NETBIOS:
   NetBios_Send( Lsn_Handle, pBuf, Len );
   break;

  case SOCKET:
   SockSend( Lsn_Handle, pBuf, Len );
   break;
 }
}

/*****************************************************************************/
/* RmtRecv()                                                                 */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Receive a buffer of data over a network connection.                     */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   handle     the handle of the connection to receive the buffer to if     */
/*              not the default.                                             */
/*   pBuf       -> our transmit buffer.                                      */
/*   Len        number of bytes to transmit.                                 */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   handle == DEFAULT_HANDLE ==> use the default handle.                    */
/*                                                                           */
/*****************************************************************************/
void RmtRecv( LHANDLE handle, PVOID pBuf, ULONG Len )
{
 LHANDLE Lsn_Handle;

 Lsn_Handle = LsnHandle;
 if( handle != DEFAULT_HANDLE )
  Lsn_Handle = handle;

 switch( Connection.ConnectType )
 {
  case BOUND:
  default:
   break;

  case ASYNC:
   AsyncRecv( pBuf, Len );
   break;

  case LOCAL_PIPE:
   PipeRecv( Lsn_Handle, pBuf, Len );
   break;

  case _NETBIOS:
   NetBios_Recv( Lsn_Handle, pBuf, Len );
   break;

  case SOCKET:
   SockRecv( Lsn_Handle, pBuf, Len );
   break;
 }
 /****************************************************************************/
 /* - When we are terminating the world, we want to halt any further         */
 /*   communications. So, what we do is just sit here and wait for the       */
 /*   operating system to kill this thread.                                  */
 /****************************************************************************/
 if( HaltCommunicationsFlag == TRUE )
 {
   if( IsVerbose() ){ printf("\ncommunications halted");fflush(0);}
   for(;;){ DosSleep(60000); }
 }
}

