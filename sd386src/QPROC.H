/*****************************************************************************/
/* File:                                                                     */
/*   qproc.h                                                                 */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Header file for the undocumented(currently) DosQProcStatus() call.      */
/*                                                                           */
/* History:                                                                  */
/*                                                                           */
/*   03/08/94 Created                                                        */
/*                                                                           */
/*...                                                                        */
/*... 05/05/94  919   Joe   Add child process support.                       */
/*...                                                                        */
/*****************************************************************************/

/*****************************************************************************/
/* DosQProcStatus() prototype.                                               */
/*                                                                           */
/*       buffer - is a pointer to a buffer that will be filled with system   */
/*                resource summary information records                       */
/*                                                                           */
/*                                                                           */
/*  buffer_size - is the size of the buffer in bytes (the maximum value of   */
/*                64KB is typically the safest sized buffer to use since     */
/*                it is impossible to predict how much information a         */
/*                DosQProcStatus call will return)                           */
/*                                                                           */
/*****************************************************************************/
APIRET16 APIENTRY16 DosQProcStatus(ULONG *buffer , USHORT buffer_size );

/*---------------------------------------------------------------------------*/
/* Pointer Record Section                                                    */
/*                                                                           */
/* The following pointer record structure is the first section in the        */
/* buffer.  It contains pointers to the heads of the system resource         */
/* information sections.                                                     */
/*****************************************************************************/
typedef struct qsPtrRec_s
{
 struct qsGrec_t        *pGlobalRec;    /* ptr to the global data section    */
 struct qsPrec_t        *pProcRec;      /* ptr to process record section     */
 struct qsS16Headrec_t  *p16SemRec;     /* ptr to 16 bit sem section         */
 ULONG  reserved;
 struct qsMrec_t        *pShrMemRec;    /* ptr to shared mem section         */
 struct qsLrec_t        *pLibRec;       /* ptr to exe module recrd section   */
} qsPtrRec_t;

/*---------------------------------------------------------------------------*/
/* Global Data Section                                                       */
/*                                                                           */
/* The following C language structure defines the information contained      */
/* within the global data section that describes global system information.  */
/*****************************************************************************/
typedef struct qsGrec_s
{
 ULONG         cThrds;                     /* number of threads in use       */
 ULONG         Reserved1;
 ULONG         Reserved2;
}qsGrec_t;

/*---------------------------------------------------------------------------*/
/* Process Data Section                                                      */
/*                                                                           */
/* The following C language declarations define the information contained    */
/* within the system resource summary information record that describes an   */
/* OS/2 process.  Each process record is immediately followed by one or      */
/* more thread records for the given process.  The pointer qsTrec_t points   */
/* to the first thread record structure for the process.                     */
/*                                                                           */
/*                                                                           */
/* Process Record structure                                                  */
/*      Holds all per process information.                                   */
/*      ________________________________                                     */
/*      |       RecType  (=0x00000001)  |                                    */
/*      |-------------------------------|                                    */
/*      |       pThrdRec                |----|                               */
/*      |-------------------------------|    |                               */
/*      |       pid                     |    |                               */
/*      |-------------------------------|    |                               */
/*      |       ppid                    |    |                               */
/*      |-------------------------------|    |                               */
/*      |       type                    |    |                               */
/*      |-------------------------------|    |                               */
/*      |       stat                    |    |                               */
/*      |-------------------------------|    |                               */
/*      |       sgid                    |    |                               */
/*      |-------------------------------|    |                               */
/*      |       hMte                    |    |                               */
/*      |-------------------------------|    |                               */
/*      |       cTCB                    |    |                               */
/*      |-------------------------------|    |                               */
/*      |       c16Sem                  |    |                               */
/*      |-------------------------------|    |                               */
/*      |       cLib                    |    |                               */
/*      |-------------------------------|    |                               */
/*      |       cShrMem                 |    |                               */
/*      |-------------------------------|    |                               */
/*      |       p16SemRec               |----|--------|                      */
/*      |-------------------------------|    |        |                      */
/*      |       pLibRec                 |----|--------|------|               */
/*      |-------------------------------|    |        |      |               */
/*      |       pShrMemRec              |----|--------|------|----|          */
/*      |-------------------------------|    |        |      |    |          */
/*      |       16SemIndxç0Ÿ            |<---|--------|      |    |          */
/*      |          .                    |    |               |    |          */
/*      |          .                    |    |               |    |          */
/*      |          .                    |    |               |    |          */
/*      |       16SemIndxçc16Sem-1Ÿ     |    |               |    |          */
/*      |-------------------------------|    |               |    |          */
/*      |       hmteç0Ÿ (or "name str") |<---|---------------|    |          */
/*      |          .                    |    |                    |          */
/*      |          .                    |    |                    |          */
/*      |          .                    |    |                    |          */
/*      |       hmteçcLib-1Ÿ            |    |                    |          */
/*      |-------------------------------|    |                    |          */
/*      |       hshmemç0Ÿ               |<---|--------------------|          */
/*      |          .                    |    |                               */
/*      |          .                    |    |                               */
/*      |          .                    |    |                               */
/*      |       hshmemçcShrMem-1Ÿ       |    |                               */
/*      |-------------------------------|    |                               */
/*       * refer to thread rec structure<-----                               */
/*                                                                           */
/*                                                                           */
/* The pLibRec value points to an array of mte handles that                  */
/* indicate the DLL modules that are directly referred to by the             */
/* process (.EXE) program file module.  The cLib value indicates             */
/* how many dynamic link library module handles are in the array.            */
/*                                                                           */
/* The pShrMemRec value points to an array of shared memory                  */
/* handles that indicate the shared memory segments that are being           */
/* used by the process.  The cShrMem value indicates how many                */
/* shared memory segment handles are in the array.                           */
/*                                                                           */
/* The p16SemRec value points to an array of 16 bit system                   */
/* semaphore indices that indicate the 16 bit system semaphores              */
/* that are being used by the process.  The c16Sem value indicates           */
/* how many 16 bit system semaphore indices are in the array.                */
/*                                                                           */
/* The status flag gives an indication of the current status of a            */
/* process.  The following list defines the possible values for              */
/* the status flag:                                                          */
/*                                                                           */
/*    0x01 - Doing ExitList Processing                                       */
/*    0x02 - Exiting thread 1                                                */
/*    0x04 - The whole process is exiting                                    */
/*    0x10 - Parent cares about termination                                  */
/*    0x20 - Parent did an exec-and-wait                                     */
/*    0x40 - Process is dying                                                */
/*    0x80 - Process in embryonic state                                      */
/*                                                                           */
/*****************************************************************************/
typedef struct qsPrec_s
{
 ULONG                RecType;          /* type of record being processed    */
                                        /* process rectype = 1               */
 struct qsTrec_t FAR *pThrdRec;         /* ptr to 1st thread rec for this prc*/
 USHORT               pid;              /* process ID                        */
 USHORT               ppid;             /* parent process ID                 */
 ULONG                type;             /* process type                      */
 ULONG                stat;             /* process status                    */
 ULONG                sgid;             /* process screen group              */
 USHORT               hMte;             /* program module handle for process */
 USHORT               cTCB;             /* # of TCBs in use in process       */
 ULONG                Reserved1;
 ULONG                Reserved2;
 USHORT               c16Sem;           /*# of 16 bit system sems in use     */
 USHORT               cLib;             /* number of runtime linked libs     */
 USHORT               cShrMem;          /* number of shared memory handles   */
 USHORT               Reserved3;
 USHORT   FAR        *p16SemRec;        /*ptr to head of 16 bit sem inf      */
 USHORT   FAR        *pLibRec;          /*ptr to list of runtime libs        */
 USHORT   FAR        *pShrMemRec;       /*ptr to list of shared mem handles  */
 USHORT   FAR         Reserved4;
}qsPrec_t;

/*---------------------------------------------------------------------------*/
/* Thread Data Section                                                       */
/*                                                                           */
/* The following C language declarations define the information contained    */
/* within the system resource summary information record that describes an   */
/* OS/2 thread:                                                              */
/*                                                                           */
/*     Thread Record Structure                                               */
/*     ________________________________                                      */
/*     |       RecType  (=0x00000100)  |----|                                */
/*     |-------------------------------|    |                                */
/*     |       tid                     |    |                                */
/*     |-------------------------------|    |                                */
/*     |       slot                    |    |                                */
/*     |-------------------------------|    |                                */
/*     |       sleepid                 |    |                                */
/*     |-------------------------------|    |                                */
/*     |       priority                |    |                                */
/*     |-------------------------------|    |                                */
/*     |       systime                 |    |                                */
/*     |-------------------------------|    |                                */
/*     |       usertime                |    |                                */
/*     |-------------------------------|    |                                */
/*     |       state                   |    |                                */
/*     |-------------------------------|    |                                */
/*                                 <--------|                                */
/*                                                                           */
/*                                                                           */
/* The thread slot number is an internal OS/2 thread identifier.             */
/* OS/2 must uniquely identify all threads within the system.                */
/*                                                                           */
/* The state value gives an indication of the current state of a             */
/* thread.                                                                   */
/*                                                                           */
/* The following list defines some typical thread state values:              */
/*                                                                           */
/*    1 - Ready to run                                                       */
/*    2 - Blocked                                                            */
/*    5 - Running                                                            */
/*                                                                           */
/*****************************************************************************/
typedef struct qsTrec_t
{
 ULONG    RecType;                      /* Record Type                       */
                                        /* Thread rectype = 100              */
 USHORT   tid;                          /* thread ID                         */
 USHORT   slot;                         /* "unique" thread slot number       */
 ULONG    sleepid;                      /* sleep id thread is sleeping on    */
 ULONG    priority;                     /* thread priority                   */
 ULONG    systime;                      /* thread system time                */
 ULONG    usertime;                     /* thread user time                  */
 UCHAR    state;                        /* thread state                      */
 CHAR     pad1;
 SHORT    pad2;
}qsTrec_t;

/*---------------------------------------------------------------------------*/
/* 16 Bit Semaphore Section                                                  */
/*                                                                           */
/* The following C language declarations define the information contained    */
/* within the system resource summary information record that is the first   */
/* record in the 16 bit system semaphore record section of the DosQProcStatus*/
/* buffer.                                                                   */
/*                                                                           */
/*    16 bit System Semaphore Header Record Structure                        */
/*      ________________________________                                     */
/*      |       qsS16Headrec            |                                    */
/*      |-------------------------------|                                    */
/*      |       qsS16rec_s              |____                                */
/*      |_______________________________|    |                               */
/*      |                               |<---|                               */
/*      |       qsS16rec_s              |----                                */
/*      |______________________________ |   |                                */
/*      |                               |<--|                                */
/*      |       qsS16rec_s              |____                                */
/*      |_______________________________|   |                                */
/*      |            .                  |<--|                                */
/*      |-------------------------------|                                    */
/*                                                                           */
/*                                                                           */
/*                                                                           */
/*                                                                           */
/* The following C language declarations define the 16 bit system            */
/* semaphore records:                                                        */
/*                                                                           */
/*    16 bit System Semaphore Record Structure                               */
/*      ________________________________                                     */
/*      |       pNextRec                |----|                               */
/*      |-------------------------------|    |                               */
/*      |                               |    |                               */
/*      |       SysSemOwner             |    |                               */
/*      |       SysSemFlag              |    |                               */
/*      |       SysSemRecCnt            |    |                               */
/*      |       SysSemProcCnt           |    |                               */
/*      |                               |    |                               */
/*      |-------------------------------|    |                               */
/*      |SemName:                       |    |                               */
/*      |       "pathname"              |    |                               */
/*      |-------------------------------|    |                               */
/*                                      <-----                               */
/*                                                                           */
/*                                                                           */
/* The s_SysSemOwner is the slot ID of the semaphore's owner                 */
/*                                                                           */
/* The s_SysSemFlag is the system semaphore flag bit field which             */
/* describes the type or action the semaphore is performing.                 */
/*                                                                           */
/* The s_SysSemRecCnt is the number of references to a specific              */
/* semaphore.                                                                */
/*                                                                           */
/* The SemName value is actually the first byte of an ASCIIZ                 */
/* string that is the complete pathname of the 16 bit system                 */
/* semaphore.                                                                */
/*                                                                           */
/*****************************************************************************/
typedef struct qsS16Headrec_s
{
 ULONG SRecType;                        /* semaphore rectype = 3             */
 ULONG Reserved1;                       /* overlays NextRec of 1st qsS16rec_t*/
 ULONG Reserved2;
 ULONG S16TblOff;                       /* index of first semaphore,         */
                                        /* SEE PSTAT OUTPUT                  */
                                        /* System Semaphore Info Section     */
}qsS16Headrec_t;

typedef struct qsS16rec_s
{
 ULONG  NextRec;                        /* offset to next record in buffer   */
 USHORT s_SysSemOwner;                  /* thread owning this semaphore      */
 UCHAR  s_SysSemFlag;                   /* system semaphore flag bit field   */
 UCHAR  s_SysSemRefCnt;                 /* number of references to this      */
                                        /*   system semaphore                */
 UCHAR  s_SysSemProcCnt;                /*number of requests by sem owner    */
 UCHAR  Reserved1;
 UCHAR  Reserved2;
 UCHAR  Reserved3;
 union
 {
  USHORT SemIndex;                      /* start of semaphore index.         */
  struct
  {
   UCHAR  DontCare;
   UCHAR  SemName;                      /* start of semaphore name string.   */
  }SemName;
 }s_SysSemNameIndex;
} qsS16rec_t;

/*---------------------------------------------------------------------------*/
/* Executable Module Section                                                 */
/*                                                                           */
/* The following C language declarations define the information contained    */
/* within the system resource summary information record that describes an   */
/* OS/2 DLL module:                                                          */
/*                                                                           */
/*      System wide MTE information                                          */
/*      ________________________________                                     */
/*      |       pNextRec                |----|                               */
/*      |-------------------------------|    |                               */
/*      |       hmte                    |    |                               */
/*      |-------------------------------|    |                               */
/*      |       ctImpMod                |    |                               */
/*      |-------------------------------|    |                               */
/*      |       Reserved                |    |                               */
/*      |-------------------------------|    |                               */
/*      |       Reserved                |    |                               */
/*      |-------------------------------|    |                               */
/*      |       pName                   |----|----|                          */
/*      |-------------------------------|    |    |                          */
/*      |       imported module handles |    |    |                          */
/*      |          .                    |    |    |                          */
/*      |          .                    |    |    |                          */
/*      |          .                    |    |    |                          */
/*      |-------------------------------| <--|----|                          */
/*      |       "pathname"              |    |                               */
/*      |_______________________________|    |                               */
/*                                      <-----                               */
/*                                                                           */
/*                                                                           */
/*                                                                           */
/*                                                                           */
/* The hmte value is a unique identifier that identifies this DLL            */
/* module.                                                                   */
/*                                                                           */
/* The ctImpMod value indicates how many other DLL's this module             */
/* statically references.  The ctImpMod value does not take into             */
/* account any DLL's that were dynamically linked at run time.               */
/*                                                                           */
/* The pName value is a 16 bit pointer that points to an ASCIIZ              */
/* string that follows the DLL module record.  The string contains           */
/* the pathname of the DLL module.                                           */
/*                                                                           */
/*****************************************************************************/
typedef struct qsLrec_s
{
 void   FAR   *pNextRec;                /* pointer to next record in buffer  */
 USHORT        hmte;                    /* handle for this mte               */
 USHORT        Reserved;                /* Reserved                          */
 ULONG         ctImpMod;                /* # of imported modules in table    */
 ULONG         Reserved1;               /* Reserved                          */
 ULONG         Reserved2;               /* Reserved                          */
 UCHAR  FAR   *pName;                   /* ptr to name string following stru */
 USHORT        ImpModTable;             /* first imported module handle.     */
} qsLrec_t;

/*---------------------------------------------------------------------------*/
/* Executable Module Section                                                 */
/*                                                                           */
/* The following C language declarations define the information contained    */
/* within the system resource summary information record that describes an   */
/* OS/2 shared memory segment:                                               */
/*                                                                           */
/*     Shared Mem Record Structure                                           */
/*     ________________________________                                      */
/*     |       pNextRec                |----                                 */
/*     |-------------------------------|    |                                */
/*     |       hmem                    |    |                                */
/*     |-------------------------------|    |                                */
/*     |       sel                     |    |                                */
/*     |-------------------------------|    |                                */
/*     |       refcnt                  |    |                                */
/*     |-------------------------------|    |                                */
/*     |       Memname                 |    |                                */
/*     |_______________________________|    |                                */
/*                                     <----                                 */
/*                                                                           */
/*                                                                           */
/* Like the 16 bit system semaphore record, the shared memory segment record */
/* contains its full ASCIIZ string pathname at the end of the record.        */
/*****************************************************************************/
typedef struct qsMrec_s
{
 ULONG  pNextRec;                       /* offset to next record in buffer   */
 USHORT hmem;                           /* handle for shared memory          */
 USHORT sel;                            /* shared memory selector            */
 USHORT refcnt;                         /* reference count                   */
 CHAR   Memname;                        /* start of shared memory name string*/
} qsMrec_t;

/*****************************************************************************/
/* Process status message structure.                                         */
/*****************************************************************************/
typedef struct
{
 ULONG  stat;
 char  *statmsg;
}STATUS;

