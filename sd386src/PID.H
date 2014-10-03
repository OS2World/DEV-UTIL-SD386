/*****************************************************************************/
/* - serial sema4 requests for connect/disconnect.                           */
/*****************************************************************************/
#define CONNECT_WAIT         0
#define DISCONNECT           1
#define DISCONNECT_WAIT      2
#define JUST_WAIT            3
#define SET_WAIT             4

/*****************************************************************************/
/* - Dbg Connected/Disconnected state definitions.                           */
/*****************************************************************************/
#define CONNECTED            10
#define DISCONNECTED         11

/*****************************************************************************/
/* - Some defines for informing esp of a connection at dbg.                  */
/*****************************************************************************/
#define INFORM_ESP    1
#define NO_INFORM_ESP 2

/*****************************************************************************/
/* - Some defines for sema4 states.                                          */
/*****************************************************************************/
#define SEMA4_NOT    0
#define SEMA4_RESET  1
#define SEMA4_POSTED 2

/*****************************************************************************/
/* Structure of the Dbg session queue element.                               */
/*****************************************************************************/
typedef struct _pidflags
{
 int   ConnectYielded :1;  /* flag indicating that this pid has yielded      */
                           /* it's connection. ( Applies to serial           */
                           /* connections only.)                             */
 int   IsDebug        :1;  /* Is this pid being debugged flag.               */
 int   RequestConnect :1;  /* The probe is requesting a connection.          */
 int   Initializing   :1;  /* Debugger running to entry point.               */
 int   Executing      :1;  /* Flag to indicate GoFast()/GoStep() processing. */
 int   CtrlBreak      :1;  /* Flag for handling Ctrl-Break.                  */
 int   Reserved       :26;
}PID_FLAGS;

/*****************************************************************************/
/* - structure for a queued notification element added to shared memory      */
/*   when debugging multiple processes over a serial channel. The element    */
/*   will be removed after it is reported.                                   */
/*****************************************************************************/
typedef struct _sqeMessage
{
 struct  _sqeMessage *next;
 ULONG                size;             /* size of this block.               */
 signed char          msg;              /* the message being queued.         */
 int                  Reported :1;      /* TRUE==>reported                   */
 int                  Reserved :31;     /*                                   */
 char                 parms[1];         /* parameters for the message.       */
}SQE_MESSAGE;

/*****************************************************************************/
/* - list node of all pids.                                                  */
/*****************************************************************************/
typedef struct _allpids
{
 struct _allpids *next;
 USHORT           pid;
 USHORT           mte;
 ULONG            sid;
 ULONG            type;
 char            *pFileSpec;
 HEV              ConnectSema4;
 int              Connect;
 USHORT           DbgPid;
 ULONG            DbgSid;
 USHORT           EspPid;
 ULONG            EspSid;
 SQE_MESSAGE     *pSqeMsg;              /* NULL for all except parent probe  */
                                        /* and is used for serial only.      */
 PID_FLAGS        PidFlags;
 char            *pTermQue;             /* NULL for parent dbg/esp.          */
}ALLPIDS;

typedef struct
{
 USHORT pid;
 ULONG  type;
 ULONG  mte;
}SQE_NEW_PROCESS;

