/*****************************************************************************/
/* File:                                                                     */
/*   dbgq.h                                                               919*/
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Queue structures and message definitions for Dbg.                       */
/*                                                                           */
/* History:                                                                  */
/*                                                                           */
/*   06/10/94 Created.                                                       */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/* Structure of the Dbg session queue element.                               */
/*****************************************************************************/
typedef struct _dbgmsgflags
{
 int   InformEsp :1;                    /* TRUE==>inform FALSE==>noinform    */
 int   Reserved  :31;
}DBG_MSG_FLAGS;

typedef struct
{
 USHORT        pid;
 ULONG         sid;
 ULONG         type;
 DBG_MSG_FLAGS DbgMsgFlags;
 char          FileSpec[512];
}DBG_QUE_ELEMENT;

#define DBG_QMSG_CTRL_BREAK        100
#define DBG_QMSG_OPEN_CONNECT      101
#define DBG_QMSG_QUE_TERM          102
#define DBG_QMSG_NEW_PROCESS       103
#define DBG_QMSG_DISCONNECT        104    /* Serial connections only.        */
#define DBG_QMSG_CONNECT_ESP       105    /* Serial connections only.        */
#define DBG_QMSG_SELECT_SESSION    106
#define DBG_QMSG_ERROR             107
#define DBG_QMSG_KILL_LISTEN       108
#define DBG_QMSG_PARENT_TERM       109
#define DBG_QMSG_CHILD_TERM        110
#define DBG_QMSG_SELECT_PARENT_ESP 111
#define DBG_QMSG_REQUEST_ACCESS    112  /* for my debugging only.            */

/*****************************************************************************/
/* - Termination que messages.                                               */
/*****************************************************************************/
#define DBG_TERMINATE            400
#define DBG_PROBE_TERM           401

/*****************************************************************************/
/*   PollingThreadFlag states:                                               */
/*                                                                           */
/*    RUNNING        Thread is running.                                      */
/*    NOT_RUNNING    Thread is not running.                                  */
/*    THREAD_TERM    Transitional state from RUNNING to NOT_RUNNING.         */
/*                                                                           */
/*****************************************************************************/
#define RUNNING      0
#define NOT_RUNNING  1
#define THREAD_TERM  2
