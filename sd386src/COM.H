/*****************************************************************************/
/* DCB parameter structure.                                                  */
/*****************************************************************************/
typedef struct _dcb
{
 USHORT WriteTimeOut;
 USHORT ReadTimeOut;

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
 }flags1;                     /*                                             */
                              /*                                             */
 struct _flags2               /*                                             */
 {                            /*                                             */
  UINT  Tx_Auto_Flow_Enable:1;/* Enable Auto xmit Flow Control(XON/XOFF).    */
  UINT  Rx_Auto_Flow_Enable:1;/* Enable Auto rcv  Flow Control(XON/XOFF).    */
  UINT  Error_Replacement  :1;/* Enable error replacement character.         */
  UINT  Null_Stripping     :1;/* Enable null stripping( remove null bytes).  */
  UINT  Break_Replacement  :1;/* Enable break replacement character.         */
  UINT  Rx_Auto_Flow       :1;/* 0 = normal 1 = full duplex.                 */
  UINT  RTS_Ctrl           :2;/* b7 b6                                       */
                              /*  0 0 Disable.                               */
                              /*  0 1 Enable.                                */
                              /*  1 0 Input Handshaking.                     */
                              /*  1 1 Toggling on transmit.                  */
 }flags2;                     /*                                             */
                              /*                                             */
 struct _flags3               /*                                             */
 {                            /*                                             */
  UINT  WriteInfiniteEnable:1;/* Enable Write Infinite Timeout processing.   */
  UINT  ReadTimeOut        :2;/* b2 b3                                       */
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
 }flags3;                     /*                                             */
                              /*                                             */
 UCHAR ErrorReplacement;      /* Returned value.                             */
 UCHAR BreakReplacement;      /* Returned value.                             */
 UCHAR XON_Character;         /* Returned value.                             */
 UCHAR XOFF_Character;        /* Returned value.                             */
}DCB;                         /*                                             */

/*****************************************************************************/
/* Line Control "Set" structure. Note that the structure is different        */
/* for the 62H query call.                                                   */
/*****************************************************************************/
typedef struct _linectrl
{
 UCHAR DataBits;
 UCHAR Parity;
 UCHAR StopBits;
}LINECTRL;

/*****************************************************************************/
/* Com error structure.                                                      */
/*****************************************************************************/
typedef struct _comerror
{
 UINT RxQueueOverrun :1;    /* Receive Queue overrun.                        */
 UINT RxHdweOverrun  :1;    /* Receive hardware overrun.                     */
 UINT HdweParityError:1;    /* The hardware detected a parity error.         */
 UINT HdweFrameError :1;    /* The hardware detected a framing error.        */
 UINT Undef          :12;   /* Undefined.                                    */
}COMERROR;

/*****************************************************************************/
/* Peek receive buffer structure.                                         917*/
/*****************************************************************************/
typedef struct _peekbuf
{
 USHORT NumCharsInBuf;      /* Number of characters in the receive buffer.917*/
 USHORT SizeOfRcvBuf;       /* Size of the receive buffer.                917*/
}PEEKBUF;

/*****************************************************************************/
/* Modem Control structure.                                                  */
/*****************************************************************************/
typedef struct _modemctrl
{
 UCHAR OnMask;
 UCHAR OffMask;
}MODEMCTRL;
