/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   com.c                                                                   */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*  Handle async communications between debugger and debuggee.               */
/*                                                                           */
/* History:                                                                  */
/*                                                                           */
/*   11/30/92 Created.                                                       */
/*                                                                           */
/*...Release 1.02 (10/22/92)                                                 */
/*...                                                                        */
/*... 03/18/92  803   Joe       Add support for remote debug.                */
/*...                                                                        */
/*****************************************************************************/

#include "all.h"

static LHANDLE ComHandle;

LHANDLE GetComHandle( void ) { return(ComHandle); }
void    SetComHandle( LHANDLE handle ) { ComHandle = handle; }

/*****************************************************************************/
/* AsyncInit()                                                               */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Set up the com port for remote debug.                                   */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   pConnection   -> to the structure defining the remote connection.       */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
void AsyncInit( CONNECTION *pConnection )
{
 FILE   *fpm;
 APIRET  rc;

 /****************************************************************************/
 /* - initialize the selected comport at the user defined bitrate.           */
 /****************************************************************************/
 AsyncSetupComPort(pConnection->BitRate,pConnection->ComPort);

 if( pConnection->DbgOrEsp == _DBG )
 {
  /***************************************************************************/
  /* - if we're using a modem, then read a file of modem commands            */
  /*   specifically defined for the Dbg end of the connection and then       */
  /*   make the connection to esp through the modem.                         */
  /***************************************************************************/
  if( pConnection->modem == TRUE )
  {
   fpm = NULL;
   if( pConnection->pModemFile != NULL )
   {
    fpm = fopen( pConnection->pModemFile, "r" );
    if( fpm == NULL )
     SayMsg(ERR_CANT_OPEN_MODEM);
   }
   Connect( fpm, pConnection );

   rc = AsyncGetComMsg("Esp Ready");
   if( rc == FALSE )
    SayMsg(ERR_ESP_NOT_READY);
  }
 }
 else /* pConnection->DbgOrEsp == _ESP */
 {
  /***************************************************************************/
  /* - if we're using a modem there are two possibilities:                   */
  /*    - the modem is in auto-answer with hardware handshaking turned on, or*/
  /*    - the user has specified a response file of modem commands. These    */
  /*      commands will be sent to the modem one line at a time.             */
  /***************************************************************************/
  if( pConnection->modem == TRUE )
  {
   FILE  *fp         = NULL;
   char   EspReady[] = "Esp Ready";

   if( pConnection->pModemFile != NULL )
   {
    fp = fopen( pConnection->pModemFile, "r" );
    if( fp == NULL )
     SayMsg(ERR_CANT_OPEN_MODEM);
   }

   /**************************************************************************/
   /* - Make the connection.                                                 */
   /* - Flush the modem.                                                     */
   /* - Flush the receive and transmit buffers.                              */
   /* - Send a probe ready message.                                          */
   /**************************************************************************/
   Connect(fp, pConnection );
   AsyncFlushModem();
   AsyncFlushBuffers();
   AsyncSend( EspReady,strlen(EspReady) );
  }
 }
}

/*****************************************************************************/
/* AsyncSetupComPort()                                                       */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Set up the com port for remote debug.                                   */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   pcmd        -> to user invocation options.                              */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
void AsyncSetupComPort(int BitRate, int ComPort)
{
 APIRET rc;
 USHORT ReadTimeOut  = 0xFFFF;
 USHORT WriteTimeOut = 30000;

 /****************************************************************************/
 /* - Try to open the com port.                                              */
 /****************************************************************************/
 rc = AsyncOpen(ComPort);
 if(rc != 0)
  SayMsg(ERR_COM_PORT_OPEN);

 /****************************************************************************/
 /* - If a BitRate has been specified                                        */
 /*    - Set up the device control block for the com port                    */
 /*    - Set the parity, stop bits, and data bits                            */
 /*    - Set the bit rate.                                                   */
 /*    - Raise the DTR line.                                                 */
 /*                                                                          */
 /****************************************************************************/
 if( ( BitRate != 0)  &&
     ( AsyncSetDCB(ReadTimeOut, WriteTimeOut)     ||
       AsyncSetLineCtrl()                         ||
       AsyncSetBitRate(BitRate)                   ||
       AsyncSetClearDTR(TRUE)
     )
   )
  SayMsg(ERR_COM_PORT_PARMS);
}

/*****************************************************************************/
/* AsyncOpen()                                                               */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Open the COM port.                                                      */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   none                                                                    */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   rc             DosOpen return code.                                     */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   ComPort is 0,1,2,3,4.                                                   */
/*   A value of 0 indicates that the user wants to take the default com      */
/*   port.                                                                   */
/*                                                                           */
/*****************************************************************************/

static char *port[4] =  {
                         "COM1",
                         "COM2",
                         "COM3",
                         "COM4"
                        };

APIRET AsyncOpen(int ComPort)
{
 ULONG      Action;
 APIRET     rc;

 /****************************************************************************/
 /* - Set the default com port if the user didn't specify one.               */
 /****************************************************************************/
 if( ComPort == 0 )
  ComPort = 1;

 rc = DosOpen(port[ComPort-1], &ComHandle, &Action, 0, FILE_NORMAL, FILE_OPEN,
               OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYNONE, NULL);
 return(rc);
}

/*****************************************************************************/
/* AsyncClose()                                                              */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Close the COM1 port.                                                    */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   pConnection   -> to the structure defining the remote connection.       */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*                  DosOpen return code.                                     */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   none.                                                                   */
/*                                                                           */
/*****************************************************************************/
void AsyncClose( CONNECTION *pConnection )
{
 AsyncSetClearDTR(FALSE);

 if( pConnection->modem == TRUE )
  AsyncFlushModem();
 DosClose(ComHandle);
}

/*****************************************************************************/
/* AsyncSetDCB()                                                             */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Setup the com port device control block.                                */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   ReadTimeOut   how long to wait for the DosRead to return.               */
/*   WriteTimeOut  how long to wait for the DosWrite to return.              */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   rc             DosDevIOCtl return code.                                 */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   none.                                                                   */
/*                                                                           */
/*****************************************************************************/
APIRET AsyncSetDCB( USHORT ReadTimeOut, USHORT WriteTimeOut)
{
 APIRET rc=0;
 DCB    Dcb;
 ULONG  DcbLenInOut = sizeof(Dcb);

 /****************************************************************************/
 /* Query DCB.                                                               */
 /****************************************************************************/
 {
  rc = DosDevIOCtl( ComHandle, IOCTL_ASYNC, ASYNC_GETDCBINFO, NULL, 0, NULL,
                    &Dcb, DcbLenInOut, &DcbLenInOut );

  if( rc ) return( rc );

  /***************************************************************************/
  /* Set read/write timeout values. ( see flags3).                           */
  /***************************************************************************/
  Dcb.WriteTimeOut = WriteTimeOut;      /* write timeout.                    */
  Dcb.ReadTimeOut  = ReadTimeOut;       /* max USHORTx.01 = 10.92 minutes.   */

  /***************************************************************************/
  /* Set flags1.                                                             */
  /*                                                                         */
  /* - disable DTR control mode.                                             */
  /* - enable  CTS output handshaking.                                       */
  /* - disable DSR output handshaking.                                       */
  /* - disable DCD output handshaking.                                       */
  /* - disable DSR input sensitivity.                                        */
  /***************************************************************************/
  Dcb.flags1.DTR_Ctrl=0;
  Dcb.flags1.CTS_Enable = 1;
  Dcb.flags1.DSR_Enable = 0;
  Dcb.flags1.DCD_Enable = 0;
  Dcb.flags1.DSR_Sensi  = 0;

  /***************************************************************************/
  /* Set flags2.                                                             */
  /*                                                                         */
  /* - disable automatic transmit flow control.                              */
  /* - disable automatic receive flow control.                               */
  /* - disable error replacement character.                                  */
  /* - disable null stripping.                                               */
  /* - disable break replacement character.                                  */
  /* - set automatic receive flow control to normal.( not full duplex )      */
  /* - enable RTS input handshaking.                                         */
  /***************************************************************************/

  Dcb.flags2.Tx_Auto_Flow_Enable = 0;
  Dcb.flags2.Rx_Auto_Flow_Enable = 0;
  Dcb.flags2.Error_Replacement   = 0;
  Dcb.flags2.Null_Stripping      = 0;
  Dcb.flags2.Break_Replacement   = 0;
  Dcb.flags2.Rx_Auto_Flow        = 0;
  Dcb.flags2.RTS_Ctrl            = 2;

  /***************************************************************************/
  /* Set flags3.                                                             */
  /*                                                                         */
  /* - enable write timeout processing.                                      */
  /* - set Normal read timeout processing.                                   */
  /* - set Extended hardware bufferring.                                     */
  /***************************************************************************/

  Dcb.flags3.WriteInfiniteEnable = 0;
  Dcb.flags3.ReadTimeOut = 1;
  Dcb.flags3.ExtHdweBuffering = 0;

  DcbLenInOut = sizeof(Dcb);
  rc = DosDevIOCtl(ComHandle, IOCTL_ASYNC, ASYNC_SETDCBINFO, &Dcb, DcbLenInOut,
                   &DcbLenInOut, NULL, 0, NULL );
  return( rc );
 }
}

/*****************************************************************************/
/* AsyncSetLineCtrl()                                                        */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Set the data bits,parity,and stop bits.                                 */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   none                                                                    */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   rc             DosDevIOCtl return code.                                 */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   none.                                                                   */
/*                                                                           */
/*****************************************************************************/
APIRET AsyncSetLineCtrl()
{
 APIRET   rc=0;
 LINECTRL LineCtrl;
 ULONG    LineCtrlInOut;

 /****************************************************************************/
 /* Set 8 data bits, no parity, and 1 stop bit.                              */
 /****************************************************************************/
 LineCtrl.DataBits = 8;
 LineCtrl.Parity   = 0;
 LineCtrl.StopBits = 0;

 LineCtrlInOut = sizeof(LineCtrl);
 rc = DosDevIOCtl(ComHandle, IOCTL_ASYNC, ASYNC_SETLINECTRL,
                  &LineCtrl, LineCtrlInOut, &LineCtrlInOut, NULL, 0, NULL );
 return(rc);
}

/*****************************************************************************/
/* AsyncSetBitRate()                                                         */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Set the bit(baud) rate.                                                 */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   BitRate        set the bit rate of the com port.                        */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   rc             DosDevIOCtl return code.                                 */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   none.                                                                   */
/*                                                                           */
/*****************************************************************************/
APIRET AsyncSetBitRate( int BitRate )
{
 APIRET rc=0;
 ULONG  BitRateLenInOut = sizeof(BitRate);

 rc = DosDevIOCtl( ComHandle, IOCTL_ASYNC, ASYNC_SETBAUDRATE,
                   &BitRate, BitRateLenInOut, &BitRateLenInOut, NULL,0,NULL);
 return(rc);
}

/*****************************************************************************/
/* AsyncSetClearDTR()                                                        */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Set/Clear the DTR line.                                                 */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   TRUE or FALSE  TRUE==>Set FALSE=>Clear.                                 */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   rc             DosDevIOCtl return code.                                 */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   none.                                                                   */
/*                                                                           */
/*****************************************************************************/
APIRET AsyncSetClearDTR( int TorF )
{
 APIRET rc=0;
 MODEMCTRL ModemCtrl;
 ULONG     ModemCtrlLenInOut;
 COMERROR  ComError;
 ULONG     ComErrorInOut;

 if( TorF == TRUE )
 {
  ModemCtrl.OnMask = 1;
  ModemCtrl.OffMask = 0xFF;
 }
 else
 {
  ModemCtrl.OnMask = 0;
  ModemCtrl.OffMask = 0xFE;
 }

 ModemCtrlLenInOut = sizeof(ModemCtrl);

 memset(&ComError,0,sizeof(ComError));
 ComErrorInOut = sizeof(ComError);
 rc = DosDevIOCtl( ComHandle, IOCTL_ASYNC, ASYNC_SETMODEMCTRL,
                   &ModemCtrl, ModemCtrlLenInOut, &ModemCtrlLenInOut,
                   &ComError , ComErrorInOut,     &ComErrorInOut);
 return(rc);
}

/*****************************************************************************/
/* AsyncSend()                                                               */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Send a buffer to the com port.                                          */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   pBuf       -> our transmit buffer.                                      */
/*   Len        number of bytes to transmit.                                 */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   none.      If an error occurs, we're dead,so just exit.                 */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   none.                                                                   */
/*                                                                           */
/*****************************************************************************/
void AsyncSend( PVOID pBuf, ULONG Len)
{
 ULONG  BytesWritten;

 if( DosWrite(ComHandle, pBuf, Len,&BytesWritten) ||
     (BytesWritten != Len) ||
     AsyncCheckComError() )
  {
   printf("\nCommunications send error or write timeout.");
   printf("\nLen=%d",Len);
   {
    UCHAR *cp = (UCHAR*)pBuf;
    int i;

    printf("\n");
    for( i=1; i<=Len ; i++, cp++)
    {
     printf("%2x",*cp);
    }
   }
   fflush(0);
   exit(0);
  }
}

/*****************************************************************************/
/* AsyncRecv()                                                               */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Get a buffer from the com port.                                         */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   pBuf       -> our transmit buffer.                                      */
/*   Len        number of bytes to transmit.                                 */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   none.      If an error occurs, we're dead,so just exit.                 */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   Len >= 0.                                                               */
/*                                                                           */
/*****************************************************************************/
void AsyncRecv( PVOID pBuf, ULONG Len)
{
 ULONG  BytesRead = 0;
 APIRET rc = 0;

 /****************************************************************************/
 /* - currently we equate (rc==0 && BytesRead) as a receive timeout.         */
 /****************************************************************************/
again:
 rc = DosRead(ComHandle, pBuf, Len,&BytesRead);
 if( rc==0 && BytesRead!=Len )
  goto again;

 if( rc == ERROR_INTERRUPT )
  goto again;

 if( rc ||
     (BytesRead != Len) ||
     AsyncCheckComError() )
  {printf("\nCommunications receive error.");
   exit(0);
  }
}

/*****************************************************************************/
/* AsyncCheckComError()                                                      */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Check the com port to see if an error occurred.                         */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   none                                                                    */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   0              no error.                                                */
/*   1              error.                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   none.                                                                   */
/*                                                                           */
/*****************************************************************************/
ULONG AsyncCheckComError()
{
 COMERROR ComError;
 ULONG    ComErrorInOut;

 memset(&ComError,0,sizeof(ComError));
 ComErrorInOut = sizeof(ComError);
 if(  DosDevIOCtl ( ComHandle, IOCTL_ASYNC, ASYNC_GETCOMMERROR,
                    NULL, 0, NULL, &ComError, ComErrorInOut, &ComErrorInOut) ||
     ComError.RxQueueOverrun   ||
     ComError.RxHdweOverrun    ||
     ComError.HdweParityError  ||
     ComError.HdweFrameError )
 {
  return(1);
 }
 return(0);
}

/*****************************************************************************/
/* AsyncFlushBuffers()                                                       */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Flush the input and output buffers.                                     */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
void AsyncFlushBuffers()
{
 UCHAR  InByte;
 UCHAR  OutByte;
 ULONG  InByteLenInOut;
 ULONG  OutByteLenInOut;

 InByte = 0;
 InByteLenInOut = sizeof(UCHAR);
 OutByte = 0;
 OutByteLenInOut = sizeof(UCHAR);
 DosDevIOCtl( ComHandle, IOCTL_GENERAL, DEV_FLUSHINPUT,
               &InByte, InByteLenInOut,  &InByteLenInOut,
               &OutByte,OutByteLenInOut, &OutByteLenInOut );
 InByte = 0;
 InByteLenInOut = sizeof(UCHAR);
 OutByte = 0;
 OutByteLenInOut = sizeof(UCHAR);
 DosDevIOCtl( ComHandle, IOCTL_GENERAL, DEV_FLUSHOUTPUT,
               &InByte, InByteLenInOut,  &InByteLenInOut,
               &OutByte,OutByteLenInOut, &OutByteLenInOut );
 return;
}

/*****************************************************************************/
/* AsyncPeekRecvBuf()                                                     917*/
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Peek the receive buffer to see if a message has come in.                */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   pNumChars     ->to receiver of the number of characters.                */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   rc             DosDevIOCtl return code.                                 */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   none.                                                                   */
/*                                                                           */
/*****************************************************************************/
APIRET AsyncPeekRecvBuf(USHORT *pNumChars)
{
 APIRET   rc=0;
 PEEKBUF  PeekBuf;
 ULONG    PeekBufInOut;

 PeekBufInOut = sizeof(PeekBuf);
 rc =  DosDevIOCtl ( ComHandle, IOCTL_ASYNC, ASYNC_GETINQUECOUNT,
                     NULL, 0, NULL, &PeekBuf, PeekBufInOut, &PeekBufInOut);

 *pNumChars = PeekBuf.NumCharsInBuf;
 return(rc);
}

/*****************************************************************************/
/* Connect()                                                                 */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   - Establish a connection to ESP.                                        */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   fp        file handle of the modem command file.                        */
/*   pConnection   -> to the structure defining the remote connection.       */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
void Connect( FILE *fp, CONNECTION *pConnection )
{
 APIRET rc;
 char   ResponseString[128];
 char   line[256];
 char  *cp;
 ULONG  BytesRead;
 char   ExpectedResponseString[128];
 int    i;

 /****************************************************************************/
 /* -Set up a 5  second wait for a read/write to the modem.                  */
 /****************************************************************************/
 AsyncSetDCB(500, 500);

 /****************************************************************************/
 /* - Send the AT command to the modem.                                      */
 /****************************************************************************/
 {
  ULONG BytesWritten;
  char  ATstring[4];
  ULONG Len;

  strcpy(ATstring,"AT\r");
  Len = strlen(ATstring);
  if( DosWrite(ComHandle, ATstring, Len, &BytesWritten) ||
      (BytesWritten != Len) ||
      AsyncCheckComError()
    )
   SayMsg(ERR_CANT_GET_MODEM_ATTENTION);
 }

 /****************************************************************************/
 /* - We should get two response strings.                                    */
 /*    - AT is an echo of the command.                                       */
 /*    - OK is the result code.                                              */
 /****************************************************************************/
 strcpy(ExpectedResponseString,"AT");
 for(i=1;i<=2;i++)
 {
  memset(ResponseString,0,sizeof(ResponseString) );
  cp = ResponseString;
  for(;;cp++)
  {
   rc = DosRead(ComHandle, cp, 1,&BytesRead);
   if( rc==0 && BytesRead!=1 )
    SayMsg(ERR_CANT_GET_MODEM_ATTENTION);

   /**************************************************************************/
   /* - test for end of response string.                                     */
   /**************************************************************************/
   if(*cp == '\n')
    break;
  }

  *strrchr(ResponseString,'\r') = '\0';
  printf("\n%s",ResponseString);fflush(0);

  if( strstr(ResponseString,ExpectedResponseString) == NULL )
    SayMsg(ERR_CANT_GET_MODEM_ATTENTION);
  strcpy(ExpectedResponseString,"OK");
 }

 /****************************************************************************/
 /* - At this point, the modem is in command mode.                           */
 /****************************************************************************/
 if( pConnection->DbgOrEsp == _ESP )
 {
  /***************************************************************************/
  /* - Esp will be called from the debugger.                                 */
  /* - Turn on auto-answer and answer after one ring.                        */
  /***************************************************************************/
  SendAT_CommandToModem( "ATS0=1", TRUE );
  ReadAT_CommandResponse();
 }
 else /* debugger end of the connection. */
 {
  char  PhoneNumber[32];

  /***************************************************************************/
  /* - Tell the modem to ignore the escape code sequence.                    */
  /* - Tell the modem to wait 60 seconds for a connection.                   */
  /***************************************************************************/
  SendAT_CommandToModem( "ATS2=255", TRUE );
  ReadAT_CommandResponse();
  SendAT_CommandToModem( "ATS7=60", TRUE );
  ReadAT_CommandResponse();

  /***************************************************************************/
  /* - Now get the phone number.                                             */
  /* - Since dialing takes a long time we need to tell the com port          */
  /*   not to get excited and return on a read timeout before the modem      */
  /*   has had time to process its own timeout value specified in the        */
  /*   ATS7=... above.                                                       */
  /***************************************************************************/
  AsyncSetDCB(12000, 30000);

tryagain:
  memset( PhoneNumber, 0, sizeof(PhoneNumber) );
  strcpy( PhoneNumber, "ATDT");
  printf( "\nEnter the phone number:");fflush(0);
  DosBeep( 1800, 1000 );
  if( gets( PhoneNumber + strlen(PhoneNumber) ) == NULL )
  {
   DosBeep( 1800, 1000 );
   printf("\nPhone number error, try again");fflush(0);
   goto tryagain;
  }
  SendAT_CommandToModem( PhoneNumber, FALSE );

  /***************************************************************************/
  /* - Reset the read/write timeout values.                                  */
  /***************************************************************************/
  AsyncSetDCB(500, 500);
 }

 /****************************************************************************/
 /* - At this point, the modem is in command mode.                           */
 /* - The standard originate/answer commands have been sent.                 */
 /* - The  the file and process command strings.                             */
 /* - Clock the commands in at 1/2 second intervals.                         */
 /****************************************************************************/
 if( fp != NULL )
 {
  for(;;)
  {
   int   len;
   char *cpx;

   fgets(line,256,fp);
   if(feof(fp))
    break;
   else
   {
    *strrchr(line,'\n') = '\r';
    cpx = line;len = 0;
    while(*cpx++ != '\r') len++;
    len++;
    AsyncSend(line,len );
    DosSleep(500);
   }
  }
 }

 /****************************************************************************/
 /* - At this point, if we're at the probe end of the connection then        */
 /*   the modem has been placed in auto-answer mode. If we're at the         */
 /*   debugger end of the connection then a phone number has been sent       */
 /*   to the modem and it should be in the process of dialing the probe.     */
 /* - Both the probe and the debugger will get a "CONNECT" message back      */
 /*   from the modem.                                                        */
 /* - Poll the modem until the "CONNECT" message is received.                */
 /****************************************************************************/
 {
  BOOL Connected;
  int  Retries;

  AsyncSetDCB(500, 30000);
  printf("\nStarting the connection\n");fflush(0);
  for( Connected = FALSE; Connected == FALSE ; )
  {
   /**************************************************************************/
   /* - Read a response string from the modem.                               */
   /**************************************************************************/
   memset(ResponseString,0,sizeof(ResponseString) );
   cp = ResponseString;
   for( Retries = 80 ; Retries > 0 ; )
   {
    rc = DosRead(ComHandle, cp, 1,&BytesRead);
    if( rc==0 && BytesRead!=1 )
    {
     printf(".");fflush(0);
     Retries--;
     continue;
    }

    /**************************************************************************/
    /* - test for end of response string.                                     */
    /**************************************************************************/
    if(*cp == '\n')
     break;

    /*************************************************************************/
    /* - Set cp for the next character to be put into the response string.   */
    /*************************************************************************/
    cp++;
   }

   /**************************************************************************/
   /* - Test for timeout while trying to read a response string.             */
   /**************************************************************************/
   if( Retries == 0 )
    SayMsg(ERR_CONNECT_TIMEOUT);

   /**************************************************************************/
   /* - If we get here, then we have a valid response string.                */
   /* - Show the response string to the user.                                */
   /**************************************************************************/
   *strrchr(ResponseString,'\r') = '\0';
   printf("\n%s",ResponseString);fflush(0);

   /***************************************************************************/
   /* - test for a connection.                                                */
   /* - sleep for a second so the user can see the connect message.           */
   /***************************************************************************/
   if( strstr(ResponseString,"CONNECT") != NULL )
   {
    DosSleep(1000);
    Connected = TRUE;
   }
   /***************************************************************************/
   /* - test for no carrier.                                                  */
   /* - exit.                                                                 */
   /***************************************************************************/
   if( (strstr(ResponseString,"NO CARRIER")  != NULL ) ||
       (strstr(ResponseString,"NO DIALTONE") != NULL )
     )
    exit(0);
  }
 }

 /****************************************************************************/
 /* - set read timeout back to max.                                          */
 /****************************************************************************/
 AsyncSetDCB(0xFFFF, 30000);
}

/*****************************************************************************/
/* AsyncGetComMsg()                                                          */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   - Get a message fromt the com port.                                     */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   pMsgToGet   -> to the message we want to get.                           */
/*                                                                           */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   TRUE or FALSE                                                           */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
APIRET AsyncGetComMsg( char* pMsgToGet )
{
 APIRET rc;
 char   c;
 char  *cp = pMsgToGet;
 int    n;
 ULONG  BytesRead=0;


 AsyncSetDCB(6000, 30000);
 printf("\n");
 for(;;)
 {
  /***************************************************************************/
  /* - read a character or timeout.                                          */
  /***************************************************************************/
  rc = DosRead(ComHandle, &c, 1,&BytesRead);
  if( rc==0 && BytesRead!=1 )
   return(FALSE);
  printf("%c",c);
  fflush(0);

  /***************************************************************************/
  /* - test for character match to first message character.                  */
  /* - read characters until a string match occurs or a timeout.             */
  /***************************************************************************/
  if( *cp == c )
   for(n = strlen(cp)-1,cp++; n > 0; n--)
   {
    rc = DosRead(ComHandle, &c, 1,&BytesRead);
    if( rc==0 && BytesRead!=1 )
     return(FALSE);
    printf("%c",c);
    fflush(0);
    if(*cp++ != c )
     {cp = pMsgToGet;break;}
   }
   /**************************************************************************/
   /* - n will be 0 if we got the message.                                   */
   /**************************************************************************/
   if( n == 0 )
    break;
 }
 /****************************************************************************/
 /* - set read timeout back to max.                                          */
 /****************************************************************************/
 AsyncSetDCB(0xFFFF, 30000);
 return(TRUE);
}

/*****************************************************************************/
/* FlushModem()                                                              */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   - Read bytes from the modem until a timeout.                            */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
void AsyncFlushModem( void )
{
 APIRET rc;
 char   c;
 ULONG  BytesRead=0;

 AsyncSetDCB(100, 30000);
 for(;;)
 {
  rc = DosRead(ComHandle, &c, 1,&BytesRead);
  if( rc==0 && BytesRead!=1 )
   break;
  printf("%c",c);
  fflush(0);
 }
 /****************************************************************************/
 /* - set read timeout back to max.                                          */
 /****************************************************************************/
 AsyncSetDCB(0xFFFF, 30000);
}

/*****************************************************************************/
/* This function checks the com port to see if there are any messages        */
/* waiting to be read.                                                       */
/*****************************************************************************/
int AsyncPeekComPort( void )
{
 APIRET rc;
 USHORT NumChars;

 rc = FALSE;
 if( ( AsyncPeekRecvBuf(&NumChars)==0 ) &&
     ( NumChars != 0 )
   )
  rc = TRUE;
 return(rc);
}

/*****************************************************************************/
/* - Send an AT command string to the modem and read the echo of the         */
/*   the command.                                                            */
/*****************************************************************************/
void SendAT_CommandToModem( char* pszCommand, BOOL WaitForEcho )
{
 char   Command[256];
 char   CommandLength;

 /****************************************************************************/
 /* - Send the command.                                                      */
 /****************************************************************************/
 CommandLength = strlen(pszCommand);

 memcpy(Command, pszCommand, CommandLength);
 Command[CommandLength] =  '\r';
 CommandLength += 1;
 AsyncSend(Command, CommandLength);

 /****************************************************************************/
 /* - Read the echo.                                                         */
 /* - Wait at most 5 seconds for any character of the command.               */
 /****************************************************************************/
 if( WaitForEcho == TRUE )
 {
  char    EchoString[256];
  char   *cp;
  APIRET  rc;
  ULONG   BytesRead = 0;

  memset(EchoString, 0, sizeof(EchoString) );

  for( cp = EchoString; ;cp++ )
  {
   rc = DosRead(ComHandle, cp, 1,&BytesRead);
   if( rc==0 && BytesRead!=1 )
    SayMsg(ERR_AT_COMMAND);

   /**************************************************************************/
   /* - test for end of echo string.                                         */
   /**************************************************************************/
   if(*cp == '\n')
    break;
  }

  *strrchr(EchoString,'\r') = '\0';
  printf("\n%s", EchoString);fflush(0);
 }
}


/*****************************************************************************/
/* - Read the modem response to an AT command.                               */
/*****************************************************************************/
void ReadAT_CommandResponse( void )
{
 char    AT_ResponseString[256];
 char   *cp;
 APIRET  rc;
 ULONG   BytesRead = 0;
 BOOL    Done;

 for( Done = FALSE; Done == FALSE; )
 {

  memset(AT_ResponseString, 0, sizeof(AT_ResponseString) );

  for( cp = AT_ResponseString; ;cp++ )
  {
   rc = DosRead(ComHandle, cp, 1,&BytesRead);
   if( rc==0 && BytesRead!=1 )
    SayMsg(ERR_AT_COMMAND);

   /**************************************************************************/
   /* - test for end of echo string.                                         */
   /**************************************************************************/
   if(*cp == '\n')
    break;
  }

  /***************************************************************************/
  /* - Print the echo string.                                                */
  /***************************************************************************/
  *strrchr(AT_ResponseString,'\r') = '\0';
  printf("\n%s", AT_ResponseString);fflush(0);

  /***************************************************************************/
  /* - If it's the OK string, then we're done.                               */
  /***************************************************************************/
  if( strcmp(AT_ResponseString, "OK") == 0 )
    Done = TRUE;
 }
}
