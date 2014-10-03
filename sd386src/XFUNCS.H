/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   xfuncs.h                                                             827*/
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   ESP function prototypes.                                                */
/*                                                                           */
/* History:                                                                  */
/*                                                                           */
/*   06/23/93 Created.                                                       */
/*... 03/29/94  917   Joe       Ctrl-Break handling for single process.      */
/*... 05/05/94  919   Joe       Add child process support.                   */
/*                                                                           */
/*****************************************************************************/

/*---------------------------------------------------------------------------*/
/* xsrvmain.c defined functions.                                             */
/*---------------------------------------------------------------------------*/
#ifdef INCL_MAIN
 int    main( int , char ** );
 void   ParseOptions        ( int, char **, ESP_PARMS *, CONNECTION * );
 void   ParseEspChildOptions( int, char **, ESP_PARMS *, CONNECTION * );
 int    EspParseComPort( char );
 ULONG _System Handler(PEXCEPTIONREPORTRECORD     ,                     /*917*/
                      PEXCEPTIONREGISTRATIONRECORD,                     /*917*/
                      PCONTEXTRECORD              ,                     /*917*/
                      PVOID );                                          /*917*/
 void   PrintCmdMessage( int );                                         /*919*/

 ESP_PARMS *GetEspParms( void );

 BOOL IsParent      ( void );
 BOOL SingleMultiple( void );
 BOOL IsVerbose     ( void );
 BOOL DosDebugTrace ( void );
 BOOL UseExecPgm    ( void );
 BOOL DebugChild    ( void );
#endif

/*---------------------------------------------------------------------------*/
/* rx.c  defined functions.                                                  */
/*---------------------------------------------------------------------------*/
#ifdef INCL_RX
 void RxFindExe(COMMAND);
 void RxStartUser(COMMAND);
 void RxGoInit(COMMAND);
 void RxGoStep(COMMAND);
 void RxGoEntry(COMMAND);
 void RxDefBrk(COMMAND);
 void RxUndBrk(COMMAND);
 void RxPutInBrk(COMMAND);
 void RxPullOutBrk(COMMAND);
 void RxInsertAllBrk( void );
 void RxRemoveAllBrk( void );
 void RxDosDebug(COMMAND );
 void RxGetThreadInfo(COMMAND );
 void RxFreezeThread(COMMAND);
 void RxThawThread(COMMAND);
 void RxGetExeOrDllEntryOrExitPt(COMMAND);
 void RxSetExecAddr( COMMAND );
 void RxGoFast(COMMAND cmd);
 void RxDefWps( COMMAND cmd );
 void RxPutInWps( COMMAND cmd );
 void RxPullOutWps( COMMAND cmd );
 void RxGetDataBytes( COMMAND cmd );
 void RxGetMemBlocks( COMMAND cmd );
 void RxSetExceptions( COMMAND cmd );
 void RxSetExecThread(COMMAND cmd);
 void RxGetCallStack(COMMAND);
 void RxWriteRegs( COMMAND cmd );
 void RxGetCoRegs( COMMAND cmd );
 void RxNormalQuit( COMMAND );
 void RxSetEspRunOpts( COMMAND cmd );
 void RxStartEspQue( COMMAND cmd );
#endif

/*---------------------------------------------------------------------------*/
/* xsrvmte.c  defined functions.                                             */
/*---------------------------------------------------------------------------*/
#ifdef INCL_XSRVMTE
 void   AddToModuleLoadTable( UINT**, ULONG, ULONG);
 void   AddFreeToModuleLoadTable( UINT **, ULONG );
 UINT  *GetObjectTable( FILE * , ULONG, ULONG );
 UINT  *PackModuleLoadTable(UINT * , int *);
 UINT  *AddModuleLoadTables(UINT *,int , UINT *,int ,int *);
 void   AddObjTableToList( UINT *, ULONG );
 void   DelObjTableFromList( ULONG );
 UCHAR _GetBitness( ULONG );

 ULONG  GetBaseAddr( ULONG );
 ULONG _GetLoadAddr( ULONG , UINT );

 OBJTABLEENTRY *GetPtrToObjectTable( ULONG );
 UINT *CoalesceTables( UINT *, int, UINT *, int, int *);
 void  FreeObjectList( void );
#endif

/*---------------------------------------------------------------------------*/
/* xsrvbrk.c defined functions.                                              */
/*---------------------------------------------------------------------------*/
#ifdef INCL_XSRVBRK
 BRK  *XSrvDefBrk( ULONG );
 void  XSrvUndBrk( ULONG );
 void  XSrvPutInBrk( ULONG );
 int   XSrvPullOutBrk( ULONG);
 void  XSrvInsertAllBrk(void);
 void  XSrvRemoveAllBrk(void);
 int   XSrvInsertOneBrk(ULONG);
 void  XSrvRemoveOneBrk(ULONG);
 BRK  *GetBrk( ULONG );
 void  ClearBrks( void );
 BRK  *_IfBrkOnAddr( UINT address );
#endif

/*---------------------------------------------------------------------------*/
/* xsrvgo.c   defined functions.                                             */
/*---------------------------------------------------------------------------*/
#ifdef INCL_XSRVGO
 APIRET XSrvGoInit( PtraceBuffer *,ULONG*,UINT ** , int* );
 APIRET XSrvRestartAppl( PtraceBuffer *,ULONG*,UINT **, int *, int);
 APIRET XSrvGoStep(PtraceBuffer*,ULONG*,UINT **,int *,ULONG,ULONG,ULONG,int,int);
 APIRET XSrvGoFast( PtraceBuffer *,ULONG *,UINT **,int *,ULONG,int );
 APIRET XSrvNormalQuit( int , UINT , ULONG);
 APIRET XSrvSetExecAddr( ULONG );
 APIRET XSrvWriteRegs(ULONG *,PtraceBuffer *);
 APIRET XSrvGetCoRegs(void * );

 APIRET StepOverCall( PtraceBuffer  *, ULONG, UINT **,int *,ULONG,ULONG*,int);
 int    IsCallInst(UINT);
 void   SpinThread( void *);                                            /*919*/
 void   SendNewProcessToQue( USHORT );                                  /*919*/
 void   StuffInModName( ULONG, char *, ULONG);                          /*919*/
 ULONG  GetEspProcessID( void );                                        /*919*/
 ULONG  GetEspSessionID( void );                                        /*919*/
 BOOL   IsChildTerminated( void );
 APIRET GetProcessMte( USHORT , USHORT * );
#endif

/*---------------------------------------------------------------------------*/
/* xsrvdis.c defined functions/ subset of disasm.c                           */
/*---------------------------------------------------------------------------*/
#ifdef INCL_XSRVDIS
 UCHAR _InstLength( ULONG );
 void  _GetInstrPacket( UINT, DTIF *);
#endif

/*---------------------------------------------------------------------------*/
/* xsrvfile.c defined functions/ subset of fileacc.c                         */
/*---------------------------------------------------------------------------*/
#ifdef INCL_XSRVFILE
 void  *Talloc(UINT);
#endif

/*---------------------------------------------------------------------------*/
/* xsrvdbif.c                                                                */
/*---------------------------------------------------------------------------*/
#ifdef INCL_XSRVDBIF
 ULONG    XSrvGetExeOrDllEntryOrExitPt( UINT );
 UCHAR   *XSrvGetMemoryBlock( ULONG , int , int * );
 void    *XSrvGetMemoryBlocks(void * ,int *);
 ULONG    GetExeStackTop(UINT , USHORT *,ULONG *);
 UINT     Putnbytes(ULONG,UINT,UCHAR *);
 UCHAR   *Getnbytes(ULONG,UINT,UINT *);
#endif

/*---------------------------------------------------------------------------*/
/* xsrvfexe.c                                                                */
/*---------------------------------------------------------------------------*/
#ifdef INCL_XSRVFEXE
 APIRET XSrvFindExe(char *,char *,UINT );
#endif

/*---------------------------------------------------------------------------*/
/* ESPSTART.C                                                                */
/*---------------------------------------------------------------------------*/
#ifdef INCL_ESPSTART
 APIRET EspStartUser(char * ,
                     char * ,
                     USHORT ,
                     ULONG *,
                     ULONG *,
                     ULONG *,
                     ULONG  ,
                     char *);

 void   EspSetEspRunOpts( ESP_RUN_OPTS *);
 BOOL   UseExecPgm( void );
 BOOL   DebugChild( void );
 BOOL   IsDebug( char *, USHORT, ULONG *, ULONG *, USHORT * );
 APIRET GetSessionID( USHORT, ULONG * );
 void   PostWait4ChildSema4( void );
 ULONG  GetEspParentProcessID( void );
 ULONG  GetEspParentSessionID( void );
 ULONG  GetTerminateSessionID( void );
 void   SetParentSessionID( ULONG );
 void   SetParentProcessID( ULONG );
#endif

/*---------------------------------------------------------------------------*/
/* XSRVTHDS.C                                                                */
/*---------------------------------------------------------------------------*/
#ifdef INCL_XSRVTHDS
 ULONG  XSrvGetThreadInfo( THREADINFO * );
 void   XSrvFreezeThread(ULONG);
 void   XSrvThawThread(ULONG);
#endif

/*---------------------------------------------------------------------------*/
/* XSRVUNWD.C                                                                */
/*---------------------------------------------------------------------------*/
#ifdef INCL_XSRVUNWD
 ULONG  XSrvGetCallStack(STACK_PARMS *, UCHAR **, UINT **, UINT ** );
 int    IsFarReturn(UINT);
 int    IsNearReturn(UINT,USHORT);
 UCHAR _InstLengthGlob( UCHAR * , UCHAR );
 UINT  _rollback(UINT ,int ,UINT );
 UINT   CsetThunk(UINT * , UINT * );
 BOOL   IsValid_EBP( ULONG );
#endif

/*---------------------------------------------------------------------------*/
/* File : xsrvacvt.c                                                         */
/*---------------------------------------------------------------------------*/
#ifdef INCL_XSRVACVT
 void  Sys_Flat2SelOff (ULONG,USHORT *,USHORT *);
 ULONG Sys_SelOff2Flat (USHORT ,USHORT );
#endif

/*---------------------------------------------------------------------------*/
/* File : xsrvwps.c                                                          */
/*---------------------------------------------------------------------------*/
#ifdef INCL_XSRVWPS
 void  XSrvPutInWps( ULONG*);
 void  XSrvDefWps( void *,int );
 void  XSrvPullOutWps( void );
 void  MarkWpNotSet( ULONG );
#endif

/*---------------------------------------------------------------------------*/
/* File : xsrvxcpt.c                                                         */
/*---------------------------------------------------------------------------*/
#ifdef INCL_XSRVXCPT
 void   XSrvSetExceptions( UCHAR *, int );
 APIRET XSrvSetExecThread(ULONG *,PtraceBuffer *, UINT );
 int    GetExceptionMap(  int );
 int    GetExceptionIndex( int );
 int    GetExceptionNumber( int );
 char  *GetExceptionType( int );
 ULONG  ResolveException( PtraceBuffer * );
#endif

/*---------------------------------------------------------------------------*/
/* File : LOG.C                                                              */
/*---------------------------------------------------------------------------*/
APIRET Dbg(PtraceBuffer *);
void   DumpPtraceBuffer(FILE *,PtraceBuffer *);
void   EspSetEspRunOpts( ESP_RUN_OPTS *);                               /*919*/
void   EspSetDbgTrace( int );                                           /*919*/
ULONG  DbgResolveException(PtraceBuffer *,ULONG *,char **);

