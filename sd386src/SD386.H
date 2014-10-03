/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   sd386.h                                                                 */
/*                                                                           */
/* Description:                                                              */
/*   SD386 include stuff                                                     */
/*                                                                           */
/* History:                                                                  */
/*                                                                           */
/*   02/08/91 Creation of 32-bit SD86, from 16-bit version.                  */
/*                                                                           */
/*...Release 1.00 (After pre-release 1.08)                                   */
/*...                                                                        */
/*... 02/12/92  521   Joe       Port to C-Set/2.                             */
/*...                                                                        */
/*...Release 1.01 (04/03/92)                                                 */
/*...                                                                        */
/*... 05/08/92  701   Srinivas  Cua Interface.                               */
/*...                                                                        */
/*...Release 1.02 (10/22/92)                                                 */
/*...                                                                        */
/*... 04/13/93  803   Selwyn    Resolving imports.                           */
/*... 04/14/93  819   Selwyn    Add /k option for keyboard only use.         */
/*... 04/14/93  820   Selwyn    Add /u option to not flush k/b buffer.       */
/*...                                                                        */
/*...Release 1.04 (04/30/93)                                                 */
/*...                                                                        */
/*... 05/04/93  822   Joe   Add mte table handling.                          */
/*... 05/05/94  919   Joe   Add child process support.                       */
/*...                                                                        */
/*****************************************************************************/

/*****************************************************************************/
/* Dbg run time options.                                                  919*/
/*****************************************************************************/
typedef struct _dbgflags                                                /*919*/
{                                                                       /*919*/
 int   Verbose       : 1;               /* print verbose printf messages. 919*/
 int   IsParentDbg   : 1;               /* Parent or Child Debugger.      919*/
 int   UseExecPgm    : 1;               /* Use DosExecPgm for parent.     919*/
 int   DebugChild    : 1;               /* Debug child process(es).       919*/
 int   SingleMultiple: 1;               /* Single child or multiple.      919*/
 int   DosDebugTrace : 1;               /* Trace DosDebug traffic.        919*/
 int   UseDebug      : 1;               /* spawn a debugger instead of dbg.  */
 int   MyAppType     : 3;               /* process type of parent debugger.  */
 int   HotKey        : 1;               /* spawn a debugger instead of dbg.  */
 int   UseMsh        : 1;               /* User MathShell/Rexx support.      */
 int   Reserved      :20;               /* alt-esc/ctrl-esc on/off           */
}DBG_FLAGS;                                                             /*919*/

typedef struct
{
 char           *pUserExe;              /* exe program name.                 */
 char           *pUserParms;            /* exe program parameters.           */
 int             CaseSens;              /* TRUE means be case sensitive      */
 int             ShowInit;              /* TRUE means do not skip init code  */
                                        /* at startup time.                  */
 USHORT          SessionType;           /* user specified session type       */
 UCHAR           ResolveImports;        /* TRUE - Resolve imports.           */
 UCHAR           KeyboardOnly;          /* TRUE - Only keyboard input.       */
 UCHAR           KBDBufferFlush;        /* TRUE - Do not flush KBD buffer.   */
 UCHAR           Profile;               /* 1 => - Use SD386.PRO.             */
 UCHAR           NoStopAtMain;          /* TRUE => don't stop at main.       */
 DBG_FLAGS       DbgFlags;              /*                                919*/
 char           *pChildProcesses;       /* response filespec for /b option919*/
 ULONG           ProcessID;             /* pid this session is debugging.    */
 ULONG           SessionID;             /* sid for the pid being debugged.   */
 USHORT          EspPid;                /* pid of a local probe.             */
 ULONG           EspSid;                /* sid of a local probe.             */
 ULONG           ProcessType;           /* type of process being debugged.   */
 LHANDLE         handle;                /* com port handle inherited from    */
                                        /* the parent debugger.              */
}CmdParms;

#define NOFLUSHNOW      1                                               /*820*/
#define NOFLUSHALLTIMES 2                                               /*820*/


/*****************************************************************************/
/* - Some defines for setting single child process versus multiple           */
/*   child process debugging.                                                */
/*****************************************************************************/
#define SINGLE   0
#define MULTIPLE 1
