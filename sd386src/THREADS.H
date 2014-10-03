/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   threads.h                                                               */
/*                                                                           */
/* Description:                                                              */
/*   include file for multithread handling.                                  */
/*                                                                           */
/*                                                                           */
/* History:                                                                  */
/*                                                                           */
/*   02/08/91 Creation of 32-bit SD86, from 16-bit version.                  */
/*                                                                           */
/*...Release 1.00                                                            */
/*...                                                                        */
/*... 02/08/91  100   made changes for 32-bit compilation.                   */
/*...                                                                        */
/*****************************************************************************/

#define TS_THAWED  0x00                 /* Values for the DebugState field   */
#define TS_FROZEN  0x01                 /* thread is frozen                  */
#define TS_DELAYED 0x02                 /* thread is ???                     */

#define TRC_C_RUNNABLE  0               /* Values for the ThreadState field */
#define TRC_C_SUSPENDED 1
#define TRC_C_BLOCKED   2
#define TRC_C_CRITSEC   3
/*DLJ*/
#define TRC_C_ENDED     4               /*Return value for GetThreadState */
                                        /*thead is null.*/

/*BUG: Two single steps on ended process crash it.*/
/*DLJ*/

typedef struct _threadstat{
  UCHAR  DebugState;
  UCHAR  ThreadState;
  USHORT Priority;                      /*                                100*/
}THREADSTAT;                            /* Struct from TRC_C_ThrdStat (1.1)  */

typedef struct _threadinfo                                              /*827*/
{                                                                       /*827*/
 THREADSTAT ts;                                                         /*827*/
 ULONG      eip;                                                        /*827*/
 ULONG      tid;                                                        /*907*/
}THREADINFO;

typedef struct tstate
{
 struct tstate *next;                   /* pointer to next tstate node       */
 struct tstate *prev;                   /* pointer to previous node          */
 UINT           pid;                    /* process id                        */
 ULONG          tid;                    /* thread  id                        */
 THREADSTAT     ts;                     /* thread  status                    */
 UINT           cs;                     /* executing cs                      */
 UINT           ip;                     /* executing ip                      */
 UINT           mid;                    /* executing mid for this thread     */
 UINT           lno;                    /* executing lno for this thread     */
 int            sfi;                    /* source file index.                */
 DEBFILE       *pdf;                    /* debug file this thread points to  */
}TSTATE;

/*****************************************************************************/
/* THREADWINDOWDATA STRUCTURE IS PASSED FROM ShowThds() TO ShowHelpBox() TO  */
/*                         MenuThreads()                                     */
/*****************************************************************************/
typedef struct {
  UINT   execitem;                      /* currently highlighted thread      */
  UCHAR *pAllInfo;                      /* thread information block          */
} ThreadsWindowData;
