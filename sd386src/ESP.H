/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   esp.h                                                                917*/
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Debuggee startup/queue management.                                      */
/*                                                                           */
/* History:                                                                  */
/*                                                                           */
/*   03/08/94 Created                                                        */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/* Esp Message definitions.                                                  */
/*****************************************************************************/
#define ESP_QMSG_END_SESSION   0        /* From the Session Manager.         */
#define ESP_QMSG_NEW_SESSION   1        /* From the Session Manager.         */

#define ESP_QMSG_CTRL_BREAK      200
#define ESP_QMSG_DISCONNECT      201
#define ESP_QMSG_CONNECT_REQUEST 202
#define ESP_QMSG_QUE_TERM        203
#define ESP_QMSG_ERROR           204
#define ESP_QMSG_EMPTY           205
#define ESP_QMSG_NEW_PROCESS     206
#define ESP_QMSG_OPEN_CONNECT    207
#define ESP_QMSG_SELECT_SESSION  208
#define ESP_QMSG_SELECT_ESP      209
#define ESP_QMSG_PARENT_TERM     210
#define ESP_QMSG_CHILD_TERM      211
#define ESP_QMSG_GIVE_ACCESS     212


/*****************************************************************************/
/* - Termination que messages.                                               */
/*****************************************************************************/
#define ESP_TERMINATE            300
#define ESP_PROBE_TERM           301

/*****************************************************************************/
/* Esp run time options.                                                     */
/*****************************************************************************/
typedef struct _espflags
{
 int   Verbose       : 1;               /* trace DosDebug() calls.           */
 int   IsParentEsp   : 1;               /* is this a parent probe.           */
 int   UseExecPgm    : 1;               /* Use DosExecPgm for parent.     919*/
 int   DebugChild    : 1;               /* Debug child process(es).       919*/
 int   SingleMultiple: 1;               /* Single child or multiple.      919*/
 int   DosDebugTrace : 1;               /* Trace DosDebug traffic.        919*/
 int   UseDebug      : 1;               /* spawn a debugger instead of esp.  */
 int   Reserved      :25;                                               /*919*/
}ESP_FLAGS;

/*****************************************************************************/
/* - Esp invocation parameters structure.                                    */
/*****************************************************************************/
typedef struct _espparms
{
 ESP_FLAGS EspFlags;
 ULONG     ProcessID;                   /* pid this session is debugging.    */
 ULONG     SessionID;                   /* sid for the pid being debugged.   */
 LHANDLE   handle;                      /* com port handle inherited from    */
}ESP_PARMS;

typedef struct _esprunopts
{
 ESP_FLAGS EspFlags;
 ULONG     NameBlockSize;
 char      NameBlock;
}ESP_RUN_OPTS;

/*****************************************************************************/
/* Structure of the termination/session queue data element. This             */
/* structure is allocated in shared memory.                                  */
/*****************************************************************************/
typedef struct
{
 USHORT ChildSid;                        /* child session id.                */
 USHORT ChildPid;                        /* child process id.                */
 USHORT ParentSid;                       /* parent session id.               */
 USHORT ParentPid;                       /* parent process id.               */
}ESP_QUE_ELEMENT;                        /*                                  */

/*****************************************************************************/
/* - error structure for esp error messages.                                 */
/*****************************************************************************/
#define MAX_QUE_ERROR_MSG_LEN 256
typedef struct
{
 ULONG  rc;
 char   ErrorMsg[MAX_QUE_ERROR_MSG_LEN];
}QUE_ERR_ELEMENT;

/*****************************************************************************/
/* - flag structure and defines for spawning a probe.                        */
/*****************************************************************************/
#define ESP_VISIBLE   0
#define ESP_INVISIBLE 1

#define ESP_USE_DOSEXECPGM       0
#define ESP_USE_DOSSTARTSESSION  1

typedef struct _espspawnflags
{
 int   Visible       : 1; /* make the probe visible or invisible when        */
                          /* started with DosStartSession().                 */
 int   SpawnMethod   : 1; /* use DosExecPgm() or DosStartSession() to spawn  */
 int   SpawnOrigin   : 1; /* use DosExecPgm() or DosStartSession() to spawn  */
 int   Reserved      :30; /* the probe.                                      */
}ESP_SPAWN_FLAGS;


/*****************************************************************************/
/* - defines for SpawnOrigin. The parent probe is spawned by the parent      */
/*   debugger when debugging single machine child/multiple process(es).      */
/*****************************************************************************/
#define ESP_SPAWN_FROM_DBG  0
#define ESP_SPAWN_FROM_ESP  1

/*****************************************************************************/
/* - structure defining a new process notification.                          */
/*****************************************************************************/
typedef struct _newpid
{
 struct _newpid *next;
 USHORT          pid;
}NEWPID;
