/*****************************************************************************/
/* - Remote message/api service structure.                                   */
/*****************************************************************************/
typedef struct _command
{
 signed char api;
 UCHAR       cmd;
 ULONG  len;
}COMMAND;

/*****************************************************************************/
/* X-Server functions.                                                       */
/*****************************************************************************/
#define FINDEXE                     -1
#define STARTUSER                   -2
#define GOINIT                      -3
#define DEFBRK                      -4
#define UNDBRK                      -5
#define PUTINBRK                    -6
#define PULLOUTBRK                  -7
#define INSERTALLBRK                -8
#define REMOVEALLBRK                -9
#define DOSDEBUG                    -10
#define GETTHREADINFO               -11
#define FREEZETHREAD                -12
#define THAWTHREAD                  -13
#define GETCALLSTACK                -14
#define GOSTEP                      -15
#define GETEXEORDLLENTRY            -16
#define NORMALQUIT                  -17
#define SETEXECADDR                 -18
#define GOFAST                      -19
#define DEFWPS                      -20
#define PUTINWPS                    -21
#define PULLOUTWPS                  -22
#define GETDATABYTES                -23
#define GETMEMBLKS                  -24
#define SETXCPTNOTIFY               -25
#define SETEXECTHREAD               -26
#define WRITEREGS                   -27
#define GETCOREGS                   -28
#define TERMINATEESP                -29
#define SETESPRUNOPTS               -30
#define CTRL_BREAK                  -31
#define NEW_PROCESS                 -32
#define START_ESP_QUE               -33
#define START_QUE_LISTEN            -34
#define CONNECT_DBG                 -35
#define CONNECT_ESP                 -36
#define SERIAL_POLL                 -37
#define CONNECT_NOTIFY              -38
#define GOENTRY                     -39
#define KILL_LISTEN_THREAD          -40
#define SELECT_SESSION              -41

/*****************************************************************************/
/* - FINDEXE                                                                 */
/*****************************************************************************/
typedef struct
{
  ULONG  progname;
  char  *pn;
  UINT   pnlen;
  char  buffer[512];
}TXRX_FINDEXE;

typedef struct
{
 APIRET rc;
 char   buffer[256];
}RET_FINDEXE;

/*****************************************************************************/
/* - STARTUSER                                                               */
/*****************************************************************************/
typedef struct
{
 ULONG  pUserEXE;
 ULONG  pUserParms;
 USHORT SessionType;
 char   buffer[512];
}TXRX_STARTUSER;

typedef struct
{
 APIRET     rc;
 ULONG      SessionID;
 ULONG      ProcessID;
 ULONG      ProcessType;
 char       ErrorBuf[CCHMAXPATH];
}RET_STARTUSER;

/*****************************************************************************/
/* - GOINIT                                                                  */
/*****************************************************************************/
typedef struct
{
 ULONG  Pid;
 ULONG  Sid;
}TXRX_GOINIT;

typedef struct
{
 APIRET rc;
 ULONG  ExecAddr;
 PtraceBuffer ptb;
}RET_GOINIT;

/*****************************************************************************/
/* GOSTEP                                                                    */
/*****************************************************************************/
typedef struct
{
 PtraceBuffer ptb;
 ULONG        ExecAddr;
 ULONG        ExecAddrlo;
 ULONG        ExecAddrhi;
 int          how;
 int          ExecFlags;
}TXRX_GOSTEP;

typedef struct
{
 APIRET rc;
 ULONG  ExecAddr;
 PtraceBuffer ptb;
}RET_GOSTEP;

/*****************************************************************************/
/* GOFAST                                                                    */
/*****************************************************************************/
typedef struct
{
 PtraceBuffer ptb;
 ULONG        ExecAddr;
 int          ExecFlags;
}TXRX_GOFAST;

typedef struct
{
 APIRET rc;
 ULONG  ExecAddr;
 PtraceBuffer ptb;
}RET_GOFAST;

/*****************************************************************************/
/* - DEFBRK                                                                  */
/*****************************************************************************/
typedef struct
{
 ULONG where;
}TXRX_DEFBRK;

/*****************************************************************************/
/* - UNDBRK                                                                  */
/*****************************************************************************/
typedef struct
{
 ULONG where;
}TXRX_UNDBRK;

/*****************************************************************************/
/* - PUTINBRK                                                                */
/*****************************************************************************/
typedef struct
{
 ULONG where;
}TXRX_PUTINBRK;

/*****************************************************************************/
/* - PULLOUTBRK                                                              */
/*****************************************************************************/
typedef struct
{
 ULONG where;
}TXRX_PULLOUTBRK;

/*****************************************************************************/
/* DOSDEBUG                                                                  */
/*****************************************************************************/
typedef struct
{
 PtraceBuffer ptb;
 char         buffer[2048];
}TXRX_DOSDEBUG;

typedef struct
{
 APIRET       rc;
 PtraceBuffer ptb;
 char         buffer[2048];
}RET_DOSDEBUG;

/*****************************************************************************/
/* GETTHREADINFO                                                             */
/*****************************************************************************/
typedef struct
{
 ULONG      ntids;
 THREADINFO ti[100];
}RET_GETTHREADINFO;

/*****************************************************************************/
/* GETCALLSTACK                                                              */
/*****************************************************************************/
typedef struct
{
 STACK_PARMS parms;
}TXRX_GETCALLSTACK;

typedef struct
{
 ULONG NActFrames;
 ULONG pActCSAtrs;
 ULONG pActFrames;
 ULONG pActFaddrs;
 char  buffer[1];
}RET_GETCALLSTACK;

/*****************************************************************************/
/* GETEXEORDLLENTRY                                                          */
/*****************************************************************************/
typedef struct
{
 int   AppTerminated;
 UINT  mte;
 ULONG EntryPt;
}TXRX_GETEXEORDLLENTRY;

/*****************************************************************************/
/* DEFWPS                                                                    */
/*****************************************************************************/
typedef struct
{
 int         size;
 WP_REGISTER regs[NODEBUGREGS];
}TXRX_DEFWPS;

/*****************************************************************************/
/* PUTINWPS                                                                  */
/*****************************************************************************/
typedef struct
{
 ULONG  indexes[NODEBUGREGS];
}RET_PUTINWPS;

/*****************************************************************************/
/* GETDATABYTES                                                              */
/*****************************************************************************/
typedef struct
{
 ULONG  addr;
 int    BytesWanted;
}TXRX_GETDATABYTES;

typedef struct
{
 int    BytesObtained;
 char   bytes[256]; /* currently the max bytes on any get.*/
}RET_GETDATABYTES;

/*****************************************************************************/
/* SETEXECTHREAD                                                             */
/*****************************************************************************/
typedef struct
{
 APIRET rc;
 ULONG  ExecAddr;
 PtraceBuffer ptb;
}RET_SETEXECTHREAD;

/*****************************************************************************/
/* WRITEREGS                                                                 */
/*****************************************************************************/

typedef struct
{
 APIRET rc;
 ULONG  ExecAddr;
}RET_WRITEREGS;

/*****************************************************************************/
/* GETCOREGS                                                                 */
/*****************************************************************************/
typedef struct
{
 APIRET rc;
 COPROCESSORREGS  coproc_regs;
}RET_GETCOREGS;

/*****************************************************************************/
/* Esp to Dbg que message.                                                   */
/*****************************************************************************/
typedef struct
{
 ULONG  pid;
 ULONG  type;
 char   ProcessFileSpec[CCHMAXPATH];
}TXRX_NEW_PROCESS;

