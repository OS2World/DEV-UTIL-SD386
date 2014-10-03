/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   pipes.c                                                                 */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*  Handle client/server communications using pipes.                         */
/*                                                                           */
/* History:                                                                  */
/*                                                                           */
/*   03/05/94 Created.                                                       */
/*                                                                           */
/*****************************************************************************/

#include "all.h"

/*****************************************************************************/
/* - Pipe handle and name for local pipe connections.                        */
/*****************************************************************************/
static char    PipeName[15]="\\PIPE\\SD386PIP";

/*****************************************************************************/
/* PipeInit()                                                                */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Set up a local pipe for debugging.                                      */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   pConnection   -> to the structure defining the remote connection.       */
/*   pLsnHandle    -> to the receiver of the logical session handle.         */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
APIRET PipeInit( CONNECTION *pConnection, LHANDLE *pLsnHandle )
{
 APIRET rc;
 int    ProcessType;

 ProcessType = DBG_SERVER;
 if( pConnection->DbgOrEsp == _ESP )
  ProcessType = ESP_CLIENT;

 switch( ProcessType )
 {
  case DBG_SERVER:
   /**************************************************************************/
   /* Create a pipe for client/server communications. The Dbg end of the     */
   /* connection is considered to be the server. The server creates the      */
   /* pipe and then waits for a client to connect to it.                     */
   /**************************************************************************/
   rc = CreateNamedPipe(pLsnHandle);
   if( rc != 0 )
    break;


   /**************************************************************************/
   /* - At this point, the pipe is created and in the Disconnected state.    */
   /* - We will connect to the pipe putting it in the Listening state        */
   /*   and then wait for a client probe to make a connection.               */
   /* - After the client opens this instance of the pipe, the pipe           */
   /*   will be in the Connected state.                                      */
   /**************************************************************************/
   rc = DosConnectNPipe(*pLsnHandle);
   break;

  case ESP_CLIENT:
   /**************************************************************************/
   /* - Connect to a pipe on the client end.                                 */
   /**************************************************************************/
   rc = OpenNamedPipe(pLsnHandle);
   break;
 }
 return(rc);
}

/*****************************************************************************/
/* PipeClose()                                                               */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Close a local pipe.                                                     */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   pConnection   -> to the structure defining the remote connection.       */
/*   LsnHandle        handle of the pipe to close.                           */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
void PipeClose( CONNECTION *pConnection, LHANDLE LsnHandle )
{
 int    ProcessType;

 ProcessType = DBG_SERVER;
 if( pConnection->DbgOrEsp == _ESP )
  ProcessType = ESP_CLIENT;

 switch( ProcessType )
 {
  case DBG_SERVER:
   /**************************************************************************/
   /* - Disconnect the server end of the pipe.                               */
   /* - Close the pipe.                                                      */
   /**************************************************************************/
   DosDisConnectNPipe(LsnHandle);
   DosClose(LsnHandle);
   break;

  case ESP_CLIENT:
   /**************************************************************************/
   /* - Close the client end of the pipe.                                    */
   /**************************************************************************/
   DosClose(LsnHandle);
   break;
 }
}

/*****************************************************************************/
/* CreateNamedPipe()                                                         */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Create an instance of a pipe and connect to it. Wait for a client       */
/*   to connect to it. This happens in the debugger session.                 */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   pPipeHandle  ptr to where the caller wants the pipe handle stuffed.     */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*  rc                                                                       */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
APIRET CreateNamedPipe( HPIPE *pPipeHandle )
{
 APIRET rc;
 ULONG  OpenMode;
 ULONG  PipeMode;
 ULONG  OutBufSize;
 ULONG  InBufSize;
 ULONG  TimeOut;

 OpenMode = NP_ACCESS_DUPLEX | NP_NOINHERIT;
 PipeMode = NP_WAIT                |
            NP_TYPE_MESSAGE        |
            NP_READMODE_MESSAGE    |
            NP_UNLIMITED_INSTANCES;

 OutBufSize = 4096;
 InBufSize  = 2048;
 TimeOut    = -1;

 /****************************************************************************/
 /* Create a duplex message pipe for this debugger session.                  */
 /*                                                                          */
 /*  OpenMode bits.                                                          */
 /*                                                                          */
 /*   31-16 = reserved                                                       */
 /*      15 = reserved and must be zero                                      */
 /*      14 = 0 This is a write through bit and only has meaning for a       */
 /*             remote pipe connection.                                      */
 /*   13- 8 = reserved.                                                      */
 /*       7 = 1 inheritance flag. The pipe handle is private to this process */
 /*    6- 3 = reserved and must be zero.                                     */
 /*    2- 0 = 010 duplex pipe.                                               */
 /*                                                                          */
 /*  PipeMode bits.                                                          */
 /*                                                                          */
 /*   31-16 = reserved                                                       */
 /*      15 = 0 blocking mode.                                               */
 /*   14-12 = reserved.                                                      */
 /*   11-10 = 01 message pipe.                                               */
 /*    9- 8 = 01 the pipe will be read as a message stream.                  */
 /*    7- 0 = -1 allow an unlimited number of pipe instances.                */
 /*                                                                          */
 /*  Transmit buffer = 4K.                                                   */
 /*  Receive  buffer = 2K.                                                   */
 /*  Timeout         = -1 wait indefinitely for a pipe instance to be        */
 /*                       created.                                           */
 /*                                                                          */
 /****************************************************************************/
 rc = DosCreateNPipe(PipeName,
                     pPipeHandle,
                     OpenMode,
                     PipeMode,
                     OutBufSize,
                     InBufSize,
                     TimeOut);
 return(rc);

}

/*****************************************************************************/
/* OpenNamedPipe()                                                           */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Open a pipe on the client side of the client/server communications.     */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   pPipeHandle  ptr to where the caller wants the pipe handle stuffed.     */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   rc                                                                      */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
APIRET OpenNamedPipe( HPIPE *pPipeHandle )
{
 APIRET rc;
 ULONG  PipeHandleState;
 ULONG  Action;


 /****************************************************************************/
 /* The open will get an ERROR_PIPE_BUSY error if the server has not         */
 /* created and connected to the pipe.                                       */
 /****************************************************************************/
 rc = ERROR_PIPE_BUSY;
 while( (rc == ERROR_PIPE_BUSY) || ( rc == ERROR_PATH_NOT_FOUND ) )
 {
  rc = DosOpen(PipeName,
               pPipeHandle,
               &Action,
               0,
               FILE_NORMAL,
               FILE_OPEN,
               OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYNONE,
               NULL);
 DosSleep(100);
 }
 if( rc )
  return(rc);

 /****************************************************************************/
 /* Change the read mode of the pipe to a message pipe.                      */
 /****************************************************************************/
 PipeHandleState = NP_WAIT | NP_READMODE_MESSAGE;
 rc = DosSetNPHState( *pPipeHandle, PipeHandleState );
 return(rc);
}

/*****************************************************************************/
/* PipeSend()                                                                */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Write to a pipe.                                                        */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   pBuf       -> our transmit buffer.                                      */
/*   Len        number of bytes to transmit.                                 */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
void PipeSend( LHANDLE PipeHandle, PVOID pBuf, ULONG Len)
{
 APIRET rc;
 ULONG  BytesWritten = 0;

again:
 rc = DosWrite(PipeHandle, pBuf, Len,&BytesWritten);
 if( rc == ERROR_INTERRUPT )
  goto again;
 /****************************************************************************/
 /* - If we get an error, then go comatose and wait for a death blow.        */
 /****************************************************************************/
 if( rc != 0 )
 {
  if( IsVerbose() ){printf("\nPipe send error rc=%d",rc);fflush(0);}
  for(;;){ DosSleep(60000); }
 }
}

/*****************************************************************************/
/* PipeRecv()                                                                */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Read from a pipe.                                                       */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   pBuf       -> our transmit buffer.                                      */
/*   Len        number of bytes to receive.                                  */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   rc     1 ==>failure                                                     */
/*          0 ==>success                                                     */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   Pipe will be closed after an error return.                              */
/*                                                                           */
/*****************************************************************************/
void PipeRecv( LHANDLE PipeHandle, PVOID pBuf, ULONG Len)
{
 ULONG  BytesRead = 0;
 APIRET rc = 0;

again:

 rc = DosRead(PipeHandle, pBuf, Len,&BytesRead);
 if( rc == ERROR_INTERRUPT )
  goto again;

 /****************************************************************************/
 /* - We will get this return code when thread 1 of the probe is sitting     */
 /*   on a DosRead of the pipe and the pipe connection is closed by a        */
 /*   ConnectClose() from the TermQue thread.                                */
 /* - Just wait for the operating system to kill this thread.                */
 /****************************************************************************/
 if( rc == ERROR_PIPE_NOT_CONNECTED )
 {
  for(;;){ DosSleep(60000); }
 }
 /****************************************************************************/
 /* - If we get an error, then go comatose and wait for a death blow.        */
 /****************************************************************************/
 if( rc != 0 )
 {
  if( IsVerbose() ){printf("\nPipe receive error rc=%d",rc);fflush(0);}
  for(;;){ DosSleep(60000); }
 }
}

