#define INCL_DOSSESMGR
#define INCL_DOSFILEMGR
#define INCL_DOSDEVICES
#include <os2.h>
#include <bsedev.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

 FILE      *comdmp = NULL;

int comdump( ULONG );
int comdump( ULONG ComHandle )
{
 APIRET     rc;

 if( comdmp == NULL )
  comdmp = fopen("com.dmp","w");
 else
  comdmp = fopen("com.dmp","a");
 fprintf(comdmp,"\n=======================");
 fprintf(comdmp,"\n===next dump ==========");
 fprintf(comdmp,"\n=======================");
 /****************************************************************************/
 /* Query the current bit rate.                                              */
 /****************************************************************************/
 {
  USHORT BitRate;
  ULONG  BitRateLenInOut = sizeof(BitRate);

  rc = (USHORT)DosDevIOCtl ( ComHandle,
                             IOCTL_ASYNC,
                             ASYNC_GETBAUDRATE,
                             NULL,
                             0,
                             NULL,
                             &BitRate,
                             BitRateLenInOut,
                             &BitRateLenInOut );
  if(rc){fprintf(comdmp,"ioctl error");fflush(0);exit(0);}
  fprintf(comdmp,"\nBit Rate  = %d",BitRate);
 }
/*****************************************************************************/
/* Query the line characteristics.                                           */
/*****************************************************************************/
{
 {
  struct
  {
   UCHAR DataBits;
   UCHAR Parity;
   UCHAR StopBits;
   UCHAR XmitBreak;
  }LineCtrl;

  ULONG  LineCtrlInOut = sizeof(LineCtrl);
  memset(&LineCtrl,0,LineCtrlInOut);

  rc = (USHORT)DosDevIOCtl ( ComHandle,
                             IOCTL_ASYNC,
                             ASYNC_GETLINECTRL,
                             NULL,
                             0,
                             NULL,
                             &LineCtrl,
                             LineCtrlInOut,
                             &LineCtrlInOut );

  if(rc){fprintf(comdmp,"ioctl error");fflush(0);exit(0);}
  fprintf(comdmp,"\n\n=====Line Characteristics=====\n");
  fprintf(comdmp,"\nData Bits = %d",LineCtrl.DataBits);
  {
   char *Parity[6]={ "None",
                     "Odd",
                     "Even",
                     "Mark",
                     "Space",
                     "Res" };
   if( LineCtrl.Parity > 4 ) LineCtrl.Parity = 5;
   fprintf(comdmp,"\nParity    = %s",Parity[LineCtrl.Parity]);
  }
  {
   char *StopBits[4]={ "1",
                       "1.5",
                       "2" };
   if( LineCtrl.StopBits > 3 ) LineCtrl.StopBits = 3;

   fprintf(comdmp,"\nStopBits  = %s",StopBits[LineCtrl.StopBits]);
  }
  {
   char *TxBrk[2] = { "Not Transmitting a break",
                      "Transmitting a break" };

   fprintf(comdmp,"\nXmitBreak = %s",TxBrk[LineCtrl.XmitBreak]);
  }
 }
}

/*****************************************************************************/
/* Query the extended bit rate.                                              */
/*****************************************************************************/
{
 {
  struct
  {
   ULONG BitRate;
   UCHAR BitRateFraction;
   ULONG MinBitRate;
   UCHAR MinBitRateFraction;
   ULONG MaxBitRate;
   UCHAR MaxBitRateFraction;
  }ExtendedBitRate;

  ULONG  ExtendedBitRateLenInOut = sizeof(ExtendedBitRate);

  memset(&ExtendedBitRate,0,ExtendedBitRateLenInOut);

  rc = (USHORT)DosDevIOCtl ( ComHandle,
                             IOCTL_ASYNC,
                             0x63,
                             NULL,
                             0,
                             NULL,
                             &ExtendedBitRate,
                             ExtendedBitRateLenInOut,
                             &ExtendedBitRateLenInOut);

  if(rc){fprintf(comdmp,"ioctl error");fflush(0);exit(0);}
  fprintf(comdmp,"\n\n=====Extended Bit Rate =======\n");
  fprintf(comdmp,"\nExtended BitRate              =%d",ExtendedBitRate.BitRate);
  fprintf(comdmp,"\nExtended BitRate Fraction     =%d",ExtendedBitRate.BitRateFraction);
  fprintf(comdmp,"\nExtended Min BitRate          =%d",ExtendedBitRate.MinBitRate);
  fprintf(comdmp,"\nExtended Min BitRate Fraction =%d",ExtendedBitRate.MinBitRateFraction);
  fprintf(comdmp,"\nExtended Max BitRate          =%d",ExtendedBitRate.MaxBitRate);
  fprintf(comdmp,"\nExtended Max BitRate Fraction =%d",ExtendedBitRate.MaxBitRateFraction);
 }
}
 /****************************************************************************/
 /* Query the COM status.                                                    */
 /****************************************************************************/
 {
  struct
  {
   UINT TxWaitCTS_On   :1;  /* Tx waiting for CTS to be turned on.           */
   UINT TxWaitDSR_On   :1;  /* Tx waiting for DSR to be turned on.           */
   UINT TxWaitDCD_On   :1;  /* Tx waiting for DCD to be turned on.           */
   UINT TxWaitXOFF_Rcv :1;  /* Tx waiting because XOFF received.             */
   UINT TxWaitXOFF_Xmit:1;  /* Tx waiting because XOFF transmitted.          */
   UINT TxWaitBRK_Xmit :1;  /* Tx waiting because break is being transmitted.*/
   UINT TxWaitXmitImmed:1;  /* Character waiting to transmit immediately.    */
   UINT RxWaitDSR_On   :1;  /* Rx waiting for DSR to be turned on.           */
  }ComStatus;

  char *ComInfo[8] = { "\nTx waiting for CTS to be turned on"            ,
                       "\nTx waiting for DSR to be turned on"            ,
                       "\nTx waiting for DCD to be turned on"            ,
                       "\nTx waiting because XOFF received"              ,
                       "\nTx waiting because XOFF transmitted"           ,
                       "\nTx waiting because break is being transmitted" ,
                       "\nCharacter waiting to transmit immediately"     ,
                       "\nRx waiting for DSR to be turned on"
                     };

  ULONG  ComStatusInOut = sizeof(ComStatus);

  memset(&ComStatus,0,sizeof(ComStatus));

  rc = (USHORT)DosDevIOCtl ( ComHandle,
                             IOCTL_ASYNC,
                             ASYNC_GETCOMMSTATUS,
                             NULL,
                             0,
                             NULL,
                             &ComStatus,
                             ComStatusInOut,
                             &ComStatusInOut);

  if(rc){fprintf(comdmp,"ioctl error");fflush(0);exit(0);}
  fprintf(comdmp,"\n\n=====Com Status ==============\n");
  if(ComStatus.TxWaitCTS_On)    fprintf(comdmp,ComInfo[0]);
  if(ComStatus.TxWaitDSR_On)    fprintf(comdmp,ComInfo[1]);
  if(ComStatus.TxWaitDCD_On)    fprintf(comdmp,ComInfo[2]);
  if(ComStatus.TxWaitXOFF_Rcv)  fprintf(comdmp,ComInfo[3]);
  if(ComStatus.TxWaitXOFF_Xmit) fprintf(comdmp,ComInfo[4]);
  if(ComStatus.TxWaitBRK_Xmit)  fprintf(comdmp,ComInfo[5]);
  if(ComStatus.TxWaitXmitImmed) fprintf(comdmp,ComInfo[6]);
  if(ComStatus.RxWaitDSR_On)    fprintf(comdmp,ComInfo[7]);
 }
 /****************************************************************************/
 /* Query the transmit data status.                                          */
 /****************************************************************************/
 {
  struct
  {
   UINT TxWrite        :1;  /* Write request packets in progress/queued.     */
   UINT TxData         :1;  /* Data in physical device driver xmit queue.    */
   UINT TxTransmitting :1;  /* Currently transmitting data.                  */
   UINT TxWaitXmitImmed:1;  /* Character waiting to transmit immediately.    */
   UINT TxWaitXON      :1;  /* Waiting to transmit an XON.                   */
   UINT TxWaitXOFF     :1;  /* Waiting to transmit an XOFF.                  */
   UINT undef1         :1;  /*                                               */
   UINT undef2         :1;  /*                                               */
  }TxStatus;

  char *Tx[6] = { "\nWrite request packets in progress/queued"    ,
                  "\nData in physical device driver xmit queue"   ,
                  "\nCurrently transmitting data"                 ,
                  "\nCharacter waiting to transmit immediately"   ,
                  "\nWaiting to transmit an XON"                  ,
                  "\nWaiting to transmit an XOFF"
                };

  ULONG  TxStatusInOut = sizeof(TxStatus);

  memset(&TxStatus,0,sizeof(TxStatus));

  rc = (USHORT)DosDevIOCtl ( ComHandle,
                             IOCTL_ASYNC,
                             ASYNC_GETLINESTATUS,
                             NULL,
                             0,
                             NULL,
                             &TxStatus,
                             TxStatusInOut,
                             &TxStatusInOut);

  if(rc){fprintf(comdmp,"ioctl error");fflush(0);exit(0);}
  fprintf(comdmp,"\n\n=====Transmit Status==========\n");
  if(TxStatus.TxWrite)         fprintf(comdmp,Tx[0]);
  if(TxStatus.TxData )         fprintf(comdmp,Tx[1]);
  if(TxStatus.TxTransmitting)  fprintf(comdmp,Tx[2]);
  if(TxStatus.TxWaitXmitImmed) fprintf(comdmp,Tx[3]);
  if(TxStatus.TxWaitXON)       fprintf(comdmp,Tx[4]);
  if(TxStatus.TxWaitXOFF)      fprintf(comdmp,Tx[5]);
 }
 /****************************************************************************/
 /* Query the modem control output signals.                                  */
 /****************************************************************************/
 {
  struct
  {
   UINT DTR            :1;  /* Data Terminal Ready.                          */
   UINT RTS            :1;  /* Request To Send.                              */
   UINT Res            :6;  /* Reserved.                                     */
  }ModemCtrl;

  ULONG  ModemCtrlLenInOut = sizeof(ModemCtrl);
  memset(&ModemCtrl,0,sizeof(ModemCtrl));

  rc = (USHORT)DosDevIOCtl ( ComHandle,
                             IOCTL_ASYNC,
                             ASYNC_GETMODEMOUTPUT,
                             NULL,
                             0,
                             NULL,
                             &ModemCtrl,
                             ModemCtrlLenInOut,
                             &ModemCtrlLenInOut);

  if(rc){fprintf(comdmp,"ioctl error");fflush(0);exit(0);}
  fprintf(comdmp,"\n\n=====Modem Control Output ====\n");
  fprintf(comdmp,"\nDTR %s",(ModemCtrl.DTR)?"ON":"OFF");
  fprintf(comdmp,"\nRTS %s",(ModemCtrl.RTS)?"ON":"OFF");
 }
 /****************************************************************************/
 /* Query the modem control output signals.                                  */
 /****************************************************************************/
 {
  struct
  {
   UINT Res            :4;  /* Reserved.                                     */
   UINT CTS            :1;  /* Clear to Send.                                */
   UINT DSR            :1;  /* Data Set Ready.                               */
   UINT RI             :1;  /* Ring Indicator.                               */
   UINT DCD            :1;  /* Data Carrier Detect.                          */
  }ModemCtrlInput;

  ULONG  ModemCtrlInputLenInOut = sizeof(ModemCtrlInput);
  memset(&ModemCtrlInput,0,sizeof(ModemCtrlInput));

  rc = (USHORT)DosDevIOCtl ( ComHandle,
                             IOCTL_ASYNC,
                             ASYNC_GETMODEMINPUT,
                             NULL,
                             0,
                             NULL,
                             &ModemCtrlInput,
                             ModemCtrlInputLenInOut,
                             &ModemCtrlInputLenInOut);

  if(rc){fprintf(comdmp,"ioctl error");fflush(0);exit(0);}
  fprintf(comdmp,"\n\n=====Modem Control Input =====\n");
  fprintf(comdmp,"\nCTS %s",(ModemCtrlInput.CTS)?"ON":"OFF");
  fprintf(comdmp,"\nDSR %s",(ModemCtrlInput.DSR)?"ON":"OFF");
  fprintf(comdmp,"\nRI  %s",(ModemCtrlInput.RI )?"ON":"OFF");
  fprintf(comdmp,"\nDCD %s",(ModemCtrlInput.DCD)?"ON":"OFF");
 }
 /****************************************************************************/
 /* Query size of Rx queue.                                                  */
 /****************************************************************************/
 {
  struct
  {
   USHORT  CharsQueued;
   USHORT  RxBufSize;
  }RxQueue;

  ULONG  RxQueueLenInOut = sizeof(RxQueue);

  rc = (USHORT)DosDevIOCtl ( ComHandle,
                             IOCTL_ASYNC,
                             ASYNC_GETINQUECOUNT,
                             NULL,
                             0,
                             NULL,
                             &RxQueue,
                             RxQueueLenInOut,
                             &RxQueueLenInOut );
  if(rc){fprintf(comdmp,"ioctl error");fflush(0);exit(0);}
  fprintf(comdmp,"\n\n=====Receive Queue ===========\n");
  fprintf(comdmp,"\nChars in Receive Queue %hd",RxQueue.CharsQueued);
  fprintf(comdmp,"\nReceive Queue Size     %hd",RxQueue.RxBufSize);
 }
 /****************************************************************************/
 /* Query size of Tx queue.                                                  */
 /****************************************************************************/
 {
  struct
  {
   USHORT  CharsQueued;
   USHORT  TxBufSize;
  }TxQueue;

  ULONG  TxQueueLenInOut = sizeof(TxQueue);

  rc = (USHORT)DosDevIOCtl ( ComHandle,
                             IOCTL_ASYNC,
                             ASYNC_GETOUTQUECOUNT,
                             NULL,
                             0,
                             NULL,
                             &TxQueue,
                             TxQueueLenInOut,
                             &TxQueueLenInOut );
  if(rc){fprintf(comdmp,"ioctl error");fflush(0);exit(0);}
  fprintf(comdmp,"\n\n=====Transmit Queue ===========\n");
  fprintf(comdmp,"\nChars in Transmit Queue %hd",TxQueue.CharsQueued);
  fprintf(comdmp,"\nTransmit Queue Size     %hd",TxQueue.TxBufSize);
 }
 /****************************************************************************/
 /* Query com error.                                                         */
 /****************************************************************************/
 {
  struct
  {
   UINT RxQueueOverrun :1;  /* Receive Queue overrun.                        */
   UINT RxHdweOverrun  :1;  /* Receive hardware overrun.                     */
   UINT HdweParityError:1;  /* The hardware detected a parity error.         */
   UINT HdweFrameError :1;  /* The hardware detected a framing error.        */
   UINT Undef          :12; /* Undefined.                                    */
  }ComError;

  char *ComErrorMsg[5] = { "\nReceive Queue Overrun."             ,
                           "\nReceive Hardware Overrun."          ,
                           "\nHardware detected a parity error."  ,
                           "\nHardware detected a framing error." ,
                           "\nUndefined error."
                          };

  ULONG  ComErrorInOut = sizeof(ComError);

  memset(&ComError,0,sizeof(ComError));

  rc = (USHORT)DosDevIOCtl ( ComHandle,
                             IOCTL_ASYNC,
                             ASYNC_GETCOMMERROR,
                             NULL,
                             0,
                             NULL,
                             &ComError,
                             ComErrorInOut,
                             &ComErrorInOut);

  if(rc){fprintf(comdmp,"ioctl error");fflush(0);exit(0);}
  fprintf(comdmp,"\n\n=====Com error ===============\n");
  if(ComError.RxQueueOverrun)  fprintf(comdmp,ComErrorMsg[0]);
  if(ComError.RxHdweOverrun)   fprintf(comdmp,ComErrorMsg[1]);
  if(ComError.HdweParityError) fprintf(comdmp,ComErrorMsg[2]);
  if(ComError.HdweFrameError)  fprintf(comdmp,ComErrorMsg[3]);
  if(ComError.Undef)           fprintf(comdmp,ComErrorMsg[4]);
 }
 /****************************************************************************/
 /* Query com event.                                                         */
 /****************************************************************************/
 {
  struct
  {
   UINT RxQueueToHdwe  :1;  /* Character transfered from hdwe to Rx queue.   */
   UINT RxTimeout      :1;  /* Receive timeout.                              */
   UINT TxLastCharSent :1;  /* Last Tx queue char sent to Tx hdwe.           */
   UINT DeltaCTS       :1;  /* Delta CTS.                                    */
   UINT DeltaDSR       :1;  /* Delta DSR.                                    */
   UINT DeltaDCD       :1;  /* Delta DCD.                                    */
   UINT BreakDetect    :1;  /* Break detected.                               */
   UINT PtyFrmRxHdwe   :1;  /* Parity,framing,or Rx hdwe overrun error.      */
   UINT DeltaRI        :1;  /* Delta RI low.                                 */
   UINT Undef          :7;  /* Undefined.                                    */
  }ComEvent;

  char *ComEventMsg[10] = {
                           "\nCharacter transfered from hdwe to Rx queue." ,
                           "\nReceive timeout."                            ,
                           "\nLast Tx queue char sent to Tx hdwe."         ,
                           "\nDelta CTS"                                   ,
                           "\nDelta DSR"                                   ,
                           "\nDelta DCD"                                   ,
                           "\nBreak detected."                             ,
                           "\nParity,framing,or Rx hdwe overrun error."    ,
                           "\nDelta RI low."                               ,
                           "\nUndefined."
                          };

  ULONG  ComEventInOut = sizeof(ComEvent);

  memset(&ComEvent,0,sizeof(ComEvent));

  rc = (USHORT)DosDevIOCtl ( ComHandle,
                             IOCTL_ASYNC,
                             ASYNC_GETCOMMEVENT,
                             NULL,
                             0,
                             NULL,
                             &ComEvent,
                             ComEventInOut,
                             &ComEventInOut);

  if(rc){fprintf(comdmp,"ioctl error");fflush(0);exit(0);}
  fprintf(comdmp,"\n\n=====Com event ===============\n");
  if(ComEvent.RxQueueToHdwe)   fprintf(comdmp,ComEventMsg[0]);
  if(ComEvent.RxTimeout)       fprintf(comdmp,ComEventMsg[1]);
  if(ComEvent.TxLastCharSent ) fprintf(comdmp,ComEventMsg[2]);
  if(ComEvent.DeltaCTS)        fprintf(comdmp,ComEventMsg[3]);
  if(ComEvent.DeltaDSR)        fprintf(comdmp,ComEventMsg[4]);
  if(ComEvent.DeltaDCD)        fprintf(comdmp,ComEventMsg[5]);
  if(ComEvent.BreakDetect)     fprintf(comdmp,ComEventMsg[6]);
  if(ComEvent.PtyFrmRxHdwe)    fprintf(comdmp,ComEventMsg[7]);
  if(ComEvent.DeltaRI)         fprintf(comdmp,ComEventMsg[8]);
  if(ComEvent.Undef)           fprintf(comdmp,ComEventMsg[9]);
 }
/*****************************************************************************/
/* Query the device control block.                                           */
/*****************************************************************************/
{
 struct
 {
  USHORT WriteTimeOut;        /* Write Timeout(msec)                         */
  USHORT ReadTimeOut;         /* Read  Timeout(msec)                         */

  struct _flags1
  {
   UINT  DTR_Ctrl  :2;         /* bit 0   bit 1                               */
                               /*  0        0   Disable.                      */
                               /*  0        1   Enable.                       */
                               /*  1        0   Input handshaking.            */
                               /*  1        1   Invalid.                      */
                               /*                                             */
   UINT  Reserved1 :1;         /* Reserved(returned as 0).                    */
   UINT  CTS_Enable:1;         /* Enable output handshaking using CTS.        */
   UINT  DSR_Enable:1;         /* Enable output handshaking using DSR.        */
   UINT  DCD_Enable:1;         /* Enable output handshaking using DCD.        */
   UINT  DSR_Sensi :1;         /* Enable input sensitivity using DSR.         */
   UINT  Reserved2 :1;         /* Reserved( returned as 0 ).                  */
  }HandShake;                  /*                                             */
                               /*                                             */
  struct _flags2               /*                                             */
  {                            /*                                             */
   UINT  Tx_Auto_Flow_Enable:1;/* Enable Auto xmit Flow Control(XON/XOFF).    */
   UINT  Rx_Auto_Flow_Enable:1;/* Enable Auto rcv  Flow Control(XON/XOFF).    */
   UINT  Error_Enable       :1;/* Enable error replacement character.         */
   UINT  Null_Enable        :1;/* Enable null stripping( remove null bytes).  */
   UINT  Break_Enable       :1;/* Enable break replacement character.         */
   UINT  Rx_Auto_Flow       :1;/* 0 = normal 1 = full duplex.                 */
   UINT  RTS_Ctrl           :2;/* b7 b6                                       */
                               /*  0 0 Disable.                               */
                               /*  0 1 Enable.                                */
                               /*  1 0 Input Handshaking.                     */
                               /*  1 1 Toggling on transmit.                  */
  }FlowReplace;                /*                                             */
                               /*                                             */
  struct _flags3               /*                                             */
  {                            /*                                             */
   UINT  WriteInfiniteEnable:1;/* Enable Write Infinite Timeout processing.   */
   UINT  ReadTimeout        :2;/* b2 b3                                       */
                               /*  0 1 Normal Read Timeout processing.        */
                               /*  1 0 Wait-for-something readout timeout.    */
                               /*  1 1 No-wait read timeout processing.       */
                               /*                                             */
   UINT  ExtHdweBuffering   :2;/* b4 b3                                       */
                               /*  0 0 Not supported.                         */
                               /*  0 1 Extended hardware buffering disabled.  */
                               /*  1 0 Extended hardware buffering enabled.   */
                               /*  1 1 Automatic Protocol override.           */
                               /*                                             */
   UINT  RxTriggerLevel     :2;/* b6 b5                                       */
                               /*  0 0 1 character.                           */
                               /*  0 1 4 characters.                          */
                               /*  1 0 8 characters.                          */
                               /*  1 1 14 characters.                         */
                               /*                                             */
   UINT  TxBufferLoadCount  :1;/* b7                                          */
                               /*  0   1  character.                          */
                               /*  1   16 characters.                         */
  }TimeOut;                    /*                                             */
                               /*                                             */
  UCHAR ErrorReplacement;      /* Returned value.                             */
  UCHAR BreakReplacement;      /* Returned value.                             */
  UCHAR XON_Character;         /* Returned value.                             */
  UCHAR XOFF_Character;        /* Returned value.                             */
 }Dcb;                         /*                                             */

  char *DcbHandShakeMsg[5] =
  {
   "\n...Reserved( error if you see this message )",
   "\n...CTS Output handshaking enabled)",
   "\n...DSR Output handshaking enabled)",
   "\n...DCD Output handshaking enabled)",
   "\n...DSR Input Sensitivity  enabled)"
  };

  char *DcbFlowReplaceMsg[5] =
  {
   "\n...Automatic Transmit Flow(XON/XOFF) Enabled )",
   "\n...Automatic Receive  Flow(XON/XOFF) Enabled )",
   "\n...Error Replacement character enabled)",
   "\n...Null stripping enabled.)",
   "\n...Break replacement character enabled)"
  };

  ULONG  DcbLenInOut = sizeof(Dcb);

  memset(&Dcb,0,DcbLenInOut);

  rc = (USHORT)DosDevIOCtl ( ComHandle,
                             IOCTL_ASYNC,
                             ASYNC_GETDCBINFO,
                             NULL,
                             0,
                             NULL,
                             &Dcb,
                             DcbLenInOut,
                             &DcbLenInOut);

  if(rc){fprintf(comdmp,"ioctl error");fflush(0);exit(0);}
  fprintf(comdmp,"\n\n=====Device Control Blk=======\n");
  fprintf(comdmp,"\nWrite TimeOut(x.01 sec) %d",Dcb.WriteTimeOut);
  fprintf(comdmp,"\nRead  TimeOut(x.01 sec) %d",Dcb.ReadTimeOut);

  /***************************************************************************/
  /* Handshake flags.                                                        */
  /***************************************************************************/
  fprintf(comdmp,"\n");
  fprintf(comdmp,"\nHandshake flags     %x",Dcb.HandShake);
  switch(Dcb.HandShake.DTR_Ctrl)
  {
   case 0:
    fprintf(comdmp,"\n...DTR Control Disabled");
    break;
   case 1:
    fprintf(comdmp,"\n...DTR Control Enabled");
    break;

   case 2:
    fprintf(comdmp,"\n...DTR Input Handshaking");
    break;

   case 3:
    fprintf(comdmp,"\n...DTR Invalid");
    break;
  }
  if(Dcb.HandShake.Reserved1  )   fprintf(comdmp,DcbHandShakeMsg[0] );
  if(Dcb.HandShake.CTS_Enable )   fprintf(comdmp,DcbHandShakeMsg[1] );
  if(Dcb.HandShake.DSR_Enable )   fprintf(comdmp,DcbHandShakeMsg[2] );
  if(Dcb.HandShake.DCD_Enable )   fprintf(comdmp,DcbHandShakeMsg[3] );
  if(Dcb.HandShake.DSR_Sensi  )   fprintf(comdmp,DcbHandShakeMsg[4] );
  if(Dcb.HandShake.Reserved2  )   fprintf(comdmp,DcbHandShakeMsg[0] );

  /***************************************************************************/
  /* Flow/Replace flags.                                                     */
  /***************************************************************************/
  fprintf(comdmp,"\n");
  fprintf(comdmp,"\nFlow/Replace flags  %x",Dcb.FlowReplace);
  if(Dcb.FlowReplace.Tx_Auto_Flow_Enable) fprintf(comdmp,DcbFlowReplaceMsg[0]);
  if(Dcb.FlowReplace.Rx_Auto_Flow_Enable) fprintf(comdmp,DcbFlowReplaceMsg[1]);
  if(Dcb.FlowReplace.Error_Enable       ) fprintf(comdmp,DcbFlowReplaceMsg[2]);
  if(Dcb.FlowReplace.Null_Enable        ) fprintf(comdmp,DcbFlowReplaceMsg[3]);
  if(Dcb.FlowReplace.Break_Enable       ) fprintf(comdmp,DcbFlowReplaceMsg[4]);
  switch(Dcb.FlowReplace.Rx_Auto_Flow)
  {
   case 0:
    fprintf(comdmp,"\n...Auto Receive Flow Control Normal");
    break;

   case 1:
    fprintf(comdmp,"\n...Auto Receive Flow Control Full Duplex");
    break;
  }
  switch(Dcb.FlowReplace.RTS_Ctrl)
  {
   case 0:
    fprintf(comdmp,"\n...RTS Control Disabled");
    break;

   case 1:
    fprintf(comdmp,"\n...RTS Control Enabled");
    break;

   case 2:
    fprintf(comdmp,"\n...RTS Input Handshaking");
    break;

   case 3:
    fprintf(comdmp,"\n...RTS Toggling on Transmit");
    break;
  }

  /***************************************************************************/
  /* Timeout flags.                                                          */
  /***************************************************************************/
  fprintf(comdmp,"\n");
  fprintf(comdmp,"\nTimeout flags  %x",Dcb.TimeOut);
  if(Dcb.TimeOut.WriteInfiniteEnable)
   fprintf(comdmp,"\n...Write Infinite Timeout Processing Enabled");
  switch(Dcb.TimeOut.ReadTimeout )
  {
   case 0:
    fprintf(comdmp,"\n...undefined");
    break;

   case 1:
    fprintf(comdmp,"\n...Normal read timeout ");
    break;

   case 2:
    fprintf(comdmp,"\n...Wait for something ");
    break;

   case 3:
    fprintf(comdmp,"\n...No wait ");
    break;
  }

  switch(Dcb.TimeOut.ExtHdweBuffering)
  {
   case 0:
    fprintf(comdmp,"\n...Not supported");
    break;

   case 1:
    fprintf(comdmp,"\n...Extended hardware buffering disabled");
    break;

   case 2:
    fprintf(comdmp,"\n...Extended hardware buffering enabled");
    break;

   case 3:
    fprintf(comdmp,"\n...Automatic protocol override enabled");
    break;
  }
  switch(Dcb.TimeOut.RxTriggerLevel)
  {
   case 0:
    fprintf(comdmp,"\n...Receive Trigger Level is 1 character");
    break;

   case 1:
    fprintf(comdmp,"\n...Receive Trigger Level is 4 characters");
    break;

   case 2:
    fprintf(comdmp,"\n...Receive Trigger Level is 8 characters");
    break;

   case 3:
    fprintf(comdmp,"\n...Receive Trigger Level is 14 characters");
    break;
  }
  switch(Dcb.TimeOut.TxBufferLoadCount)
  {
   case 0:
    fprintf(comdmp,"\n...Transmit buffer load count is 1 character");
    break;

   case 1:
    fprintf(comdmp,"\n...Transmit buffer load count is 16 characters");
    break;
  }
  fprintf(comdmp,"\n");
  fprintf(comdmp,"\nError Replacement character %c 0x%x",Dcb.ErrorReplacement,
                                                       Dcb.ErrorReplacement);
  fprintf(comdmp,"\nBreak Replacement character %c 0x%x",Dcb.BreakReplacement,
                                                       Dcb.BreakReplacement);
  fprintf(comdmp,"\nXON  Character              %c 0x%x",Dcb.XON_Character,
                                                       Dcb.XON_Character);
  fprintf(comdmp,"\nXOFF Character              %c 0x%x",Dcb.XOFF_Character,
                                                       Dcb.XOFF_Character);
}
 /****************************************************************************/
 /* Query Enhanced mode parameters.                                          */
 /****************************************************************************/
 {
  struct
  {
   UINT EnhSupport     :1;  /* Enhanced Mode supported by hardware.          */
   UINT EnhEnabled     :1;  /* Enhanced Mode enabled.                        */
   UINT RxDMA          :2;  /* b0 b1                                         */
                            /*  0 0 DMA receive capability disabled.         */
                            /*  0 1 DMA receive capability enabled.          */
                            /*  1 0 DMA channel dedicated to receive.        */
                            /*  1 1 Reserved.                                */
   UINT TxDMA          :2;  /* b0 b1                                         */
                            /*  0 0 DMA transmit capability disabled.        */
                            /*  0 1 DMA transmit capability enabled.         */
                            /*  1 0 DMA channel dedicated to transmit.       */
                            /*  1 1 Reserved.                                */
   UINT RxInDMA_Mode   :1;  /* Receive operation in DMA mode.                */
   UINT TxInDMA_Mode   :1;  /* Transmit operation in DMA mode.               */
   ULONG Reserved;
  }EnhMode;

  ULONG  EnhModeLenInOut = sizeof(EnhMode);
  memset(&EnhMode,0,sizeof(EnhMode));

  rc = (USHORT)DosDevIOCtl ( ComHandle,
                             IOCTL_ASYNC,
                             0x74,
                             NULL,
                             0,
                             NULL,
                             &EnhMode,
                             EnhModeLenInOut,
                             &EnhModeLenInOut);

  if(rc){fprintf(comdmp,"ioctl error");fflush(0);exit(0);}
  fprintf(comdmp,"\n\n=====Enhanced Mode Parameters=\n");
  fprintf(comdmp,"\n");
  fprintf(comdmp,"\nEnhanced mode %s supported by hardware.",
       (EnhMode.EnhSupport)?"is":"is not");
  fprintf(comdmp,"\nEnhanced mode %s.",
       (EnhMode.EnhEnabled)?"enabled":"disabled");

  switch(EnhMode.RxDMA)
  {
   case 0:
    fprintf(comdmp,"\nDMA receive capability disabled.");
    break;

   case 1:
    fprintf(comdmp,"\nDMA receive capability enabled.");
    break;

   case 2:
    fprintf(comdmp,"\nDMA channel dedicated to receive operation");
    break;

   case 3:
    fprintf(comdmp,"\nreserved.Check this out...bad news.  ");
    break;
  }
  switch(EnhMode.TxDMA)
  {
   case 0:
    fprintf(comdmp,"\nDMA transmit capability disabled.");
    break;

   case 1:
    fprintf(comdmp,"\nDMA transmit capability enabled.");
    break;

   case 2:
    fprintf(comdmp,"\nDMA channel dedicated to transmit operation");
    break;

   case 3:
    fprintf(comdmp,"\nreserved.Check this out...bad news.  ");
    break;
  }
  if(EnhMode.RxInDMA_Mode)
   fprintf(comdmp,"\nRx operation in DMA mode.");
  if(EnhMode.TxInDMA_Mode)
   fprintf(comdmp,"\nTx operation in DMA mode.");
 }
fclose(comdmp);
return(0);
}
