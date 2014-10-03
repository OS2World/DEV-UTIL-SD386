/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   funcs.h                                                                 */
/*                                                                           */
/* Description:                                                              */
/*   SD86 function prototypes                                                */
/*                                                                           */
/* History:                                                                  */
/*                                                                           */
/*   02/08/91 Creation of 32-bit SD86, from 16-bit version.                  */
/*                                                                           */
/*...16->32 port.                                                            */
/*...                                                                        */
/*... 02/08/91  100   Philip    port to 32 bit.                              */
/*... 02/08/91  101   Joe       port to 32 bit.                              */
/*... 02/08/91  102   Pratima   port to 32 bit.                              */
/*... 02/08/91  103   Dave      port to 32 bit.                              */
/*... 02/08/91  104                                                          */
/*... 02/08/91  105   Christina port to 32 bit.                              */
/*... 02/08/91  106   Srinivas  port to 32 bit.                              */
/*... 02/08/91  107   Dave      port to 32 bit.                              */
/*... 02/08/91  108   Dave      port to 32 bit.                              */
/*... 02/08/91  109                                                          */
/*... 02/08/91  110   Srinivas  port to 32 bit.                              */
/*... 02/08/91  111   Christina port to 32 bit.                              */
/*... 02/08/91  112   Joe       port to 32 bit.                              */
/*... 02/08/91  113                                                          */
/*... 02/08/91  114                                                          */
/*... 02/08/91  115   Srinivas  port to 32 bit.                              */
/*... 02/08/91  116   Joe       port to 32 bit.                              */
/*                                                                           */
/*...Release 1.00 (Pre-release 1)                                            */
/*...                                                                        */
/*... 07/09/91  213   srinivas  one byte memory operand problem.             */
/*... 07/18/91  216   srinivas  Better processing for FAKE MID problems.     */
/*... 07/26/91  219   srinivas  handling near pointers.                      */
/*... 08/13/91  215   Christina Add support for HLL format                   */
/*... 08/15/91  226   srinivas  modifying pointers in data window.           */
/*... 08/19/91  229   srinivas  ESC key should take back to action bar from  */
/*                              showthds and editregs.                       */
/*                                                                           */
/*...Release 1.00 (Pre-release 107 11/13/91)                                 */
/*...                                                                        */
/*... 11/13/91  400   Srinivas  Vertical Register Display.                   */
/*... 11/18/91  401   Srinivas  Floating point Register Display.             */
/*...                                                                        */
/*...Release 1.00 (Pre-release 108 12/05/91)                                 */
/*...                                                                        */
/*... 01/24/92  505   Srinivas  Take into account path info in GetFile       */
/*... 01/28/92  508   Srinivas  Added Set Execution Line function.           */
/*... 02/07/92  512   Srinivas  Handle Toronto "C" userdefs.                 */
/*... 02/10/92  513   Srinivas  Allow array indexs in datawindow to be       */
/*...                           variables.                                   */
/*... 02/11/92  519   Srinivas  Zoomrec Sizing Problems.                     */
/*... 02/12/92  521   Joe       Port to C-Set/2.                             */
/*... 02/13/92  523   Srinivas  Handle Toronto "C" Bitfields.                */
/*... 02/14/92  530   Srinivas  Jumping across ID in case of PLX disassembly */
/*...                                                                        */
/*...Release 1.00 (03/03/92)                                                 */
/*...                                                                        */
/*... 03/10/92  602   Srinivas  Hooking up watch points.                     */
/*... 03/18/92  606   Srinivas  Handle multiple segment numbers in lno table */
/*...                           due to alloc_text pragma.                    */
/*... 03/20/92  607   Srinivas  CRMA fixes.                                  */
/*... 04/01/92  610   Srinivas  Add "@" prefix for FormatVar "?" prompt.     */
/*...                                                                        */
/*...Release 1.01 (04/03/92)                                                 */
/*...                                                                        */
/*... 05/08/92  701   Srinivas  Cua Interface.                               */
/*... 01/26/93  809   Selwyn    HLL Level 2 support.                         */
/*...                                                                        */
/*****************************************************************************/

/*---------------------------------------------------------------------------*/
/* File : BYTES.ASM                                                          */
/*---------------------------------------------------------------------------*/
extern USHORT _System bindex(void *,UINT,UINT);                         /*521*/
extern USHORT _System windex(void *,UINT,USHORT);                       /*521*/
extern USHORT _System lindex(void *,UINT,UINT);                         /*521*/
#if 0
/*---------------------------------------------------------------------------*/
/* File : PUTRC.ASM                                                          */
/*---------------------------------------------------------------------------*/
extern void _System putrcx(UINT, UINT, char *, UINT);

/*---------------------------------------------------------------------------*/
/* File : PUTUP.ASM                                                          */
/*---------------------------------------------------------------------------*/
extern void _System putup(UCHAR *, USHORT *, UINT, UINT, UINT, UCHAR *, UINT);
extern void _System putxb(int,int);

/*---------------------------------------------------------------------------*/
/* File : VBOX.ASM                                                           */
/*---------------------------------------------------------------------------*/
extern void _System Vputbox(void  *,UINT,UINT,UINT,UINT);               /*521*/
extern void _System Vgetbox(void  *,UINT,UINT,UINT,UINT);               /*521*/
extern void _System Vfmtbox(void  *,UINT,UINT,UINT,UINT);               /*521*/
#endif

/*---------------------------------------------------------------------------*/
/* File : PUTRC.ASM                                                          */
/*---------------------------------------------------------------------------*/
extern void _System putrcx(UINT, UINT, char *);                         /*521*/

/*---------------------------------------------------------------------------*/
/* File : PUTUP.ASM                                                          */
/*---------------------------------------------------------------------------*/
extern void _System putup(UCHAR *, USHORT *, UINT, UINT, UINT, UCHAR *);/*521*/
extern void _System putxb(int,int);                                     /*521*/

/*---------------------------------------------------------------------------*/
/* File : VBOX.ASM                                                           */
/*---------------------------------------------------------------------------*/
extern void _System Vputbox(void  *,UINT,UINT,UINT,UINT);               /*521*/
extern void _System Vgetbox(void  *,UINT,UINT,UINT,UINT);               /*521*/
extern void _System Vfmtbox(void  *,UINT,UINT,UINT,UINT);               /*521*/

/*---------------------------------------------------------------------------*/
/* File : LOWERCAS.ASM                                                       */
/*---------------------------------------------------------------------------*/
extern void _System LowerCase(UCHAR *,UCHAR *);                         /*521*/

/*---------------------------------------------------------------------------*/
/* File : ENCODE.ASM                                                         */
/*---------------------------------------------------------------------------*/
extern UINT _System Encode(char *,char  *,UINT);                        /*521*/
extern UINT _System Decode(UCHAR *,UCHAR *);                            /*521*/

/*---------------------------------------------------------------------------*/
/* File : FINDVSTR.ASM                                                       */
/*---------------------------------------------------------------------------*/
extern UINT _System FindVstr(UCHAR *,UCHAR *,UINT);                     /*521*/

/*---------------------------------------------------------------------------*/
/* File : MISC.ASM                                                           */
/*---------------------------------------------------------------------------*/
extern UINT _System utoa(UINT,UCHAR *);                                 /*521*/

#ifdef INCL_FUNCS                                                       /*813*/
/*---------------------------------------------------------------------------*/
/* File : DAVETOLL.C                                          (DisAssembler) */
/*---------------------------------------------------------------------------*/
void DisAsm (DTIF *);

/*---------------------------------------------------------------------------*/
/* File : SD386.C                                                            */
/*---------------------------------------------------------------------------*/
int    main(int, char**);
void   GetRunTimeEntryPt( ULONG * , DEBFILE * );
void   GetMainEntryPt( ULONG * , DEBFILE * );
void   RestartInit( void );
ULONG _System Handler(PEXCEPTIONREPORTRECORD         ,
                      PEXCEPTIONREGISTRATIONRECORD   ,
                      PCONTEXTRECORD                 ,
                      PVOID );
void   UnRegisterExceptionHandler( void );
int    BuildChildProcessNamesBlock( char *, char**, ULONG*, int* );

ESP_RUN_OPTS *BuildEspParms( CmdParms *);
int         checkline(char * );
BOOL    IsDbgDebugChild( void );
ULONG   DbgGetProcessID( void );
ULONG   DbgGetSessionID( void );
void    MyExit( void );
BOOL    IsParentDbg( void );
BOOL    UseDebug( void );
BOOL    IsHotKey( void );
APIRET  GetProcessType( ULONG * );

/*---------------------------------------------------------------------------*/
/* invoke.c                                                                  */
/*---------------------------------------------------------------------------*/
void  ParseInvocationOptions     ( int, char**, CmdParms *, CONNECTION * );
void  ParseChildInvocationOptions( int, char**, CmdParms *, CONNECTION * );
char *ParseUserParameters( void );
int   ParseBitRate( char );
int   ParseComPort( char );

/*---------------------------------------------------------------------------*/
/* File : VIO.C                                                              */
/*---------------------------------------------------------------------------*/
void ClrScr(uint,uint,uint);
void VioInit(void);
void ClrPhyScr(uint,uint,uint);
void PutPhyScr(uint,uint);
void _System Vio32ShowBuf(ushort,ushort,ushort);                        /*521*/
void putrc(uint, uint, char *);                                         /*400*/

/*---------------------------------------------------------------------------*/
/* File : KBD.C                                                              */
/*---------------------------------------------------------------------------*/
void   HideCursor(void);
void   ShowCursor(void);
void   PutCsr(CSR *);
uint   GetKey(void);
void   SetCsrRowCol(uint,uint);
void   beep(void);
void   ResetFlushBufferFlag(void);
void   SetFlushBufferFlag( int );
int    NoFlushBuffer(void);
void   SetHotKey( void );
int    Convert(uint keycode);

/*---------------------------------------------------------------------------*/
/* File : PANIC.C                                                            */
/*---------------------------------------------------------------------------*/
void panic ( uchar );

/*---------------------------------------------------------------------------*/
/* File : HELP.C                                                             */
/*---------------------------------------------------------------------------*/
void   Help(ULONG);
uint   ShowHelpBox(uchar *,UINTFUNC,uint,void *);
uint   Prompt(uchar *,uchar *,uint);
void   fmterr(uchar *);
void   fmt2err(uchar *,uchar *);
void   putmsg(uchar *);
void   SayMsgBox1(uchar *);
uint   DialogError(char *);
void   SayMsgBox2(uchar *,ulong);
void   SayMsgBox3(uchar *);
uchar *SayMsgBox4( uchar * );
void   RemoveMsgBox( uchar *, uchar * );
void   CuaShowHelpBox( uchar * );
void   DisplayHelpBoxText( DIALOGSHELL *, DIALOGCHOICE * );
uint   HelpBoxDialogFunction( DIALOGSHELL *, DIALOGCHOICE *, EVENT *, void * );
void  _System Error(ULONG, int, int ,...);
void  _System Message(ULONG, int ,int,...);
void   SayStatusMsg(UCHAR *);
UINT   SayMsgBox(uchar *);
UINT   MsgYorN(ULONG);
void  _System SayMsgBox5(ULONG, int,...);
/*---------------------------------------------------------------------------*/
/* File : GETMSG.C                                                           */
/*---------------------------------------------------------------------------*/
uchar *GetHelpMsg(ULONG, UCHAR **, ULONG);
void   SayMsg(ULONG);
void  _System ErrorPrintf(ULONG, int, ...);

/*---------------------------------------------------------------------------*/
/* File : KEYSTR.C                                                           */
/*---------------------------------------------------------------------------*/
void keybeep( void );
uint GetString( uint, uint, uint, uint, uint *, uchar *, uint,POPUPSHELL*);
void ShiftRight( uint );
void ShiftLeft( uint );
void DisplayField( uint );
int  GetMouseState( PEVENT );

/*---------------------------------------------------------------------------*/
/* File : FILEACC.C                                                          */
/*---------------------------------------------------------------------------*/
int   opendos(char *,char *,HFILE *);                                   /*105*/
int   closedos(HFILE);                                                  /*105*/
int   readdos(void *,int,int,HFILE);                                    /*105*/
int   seekdos(HFILE,long int,int);                                      /*105*/
void  seekf(DEBFILE *,ulong);
uint  readf(uchar *,uint,DEBFILE *);

/*---------------------------------------------------------------------------*/
/* File : DBIF.C                                                             */
/*---------------------------------------------------------------------------*/
uint       DBPub(uchar *,DEBFILE *);
uint       DBPut(UINT, UINT, UCHAR *);
UINT       DBNextMod(UINT,DEBFILE *);
uchar     *DBGet(uint,uint,uint *);
UCHAR     *DBFindProcName(ULONG, DEBFILE *);
UINT       GetMid(ULONG addr,DEBFILE *);
MODULE    *GetPtrToModule(uint,DEBFILE *);
uchar      IsNearPtr16Or32(uint,uint,uint);

/*---------------------------------------------------------------------------*/
/* File : linnum.c                                                           */
/*---------------------------------------------------------------------------*/
void    DBGetLnoTab( ULONG mid );
ULONG   DBMapInstAddr( ULONG address, LNOTAB **pLnoTEntry ,DEBFILE *pdf );
ULONG   DBLsegInfo(ULONG, UINT *, DEBFILE *);
UCHAR  *DBModName( ULONG mid , int sfi, DEBFILE *pdf , int *ftype);
ULONG   DBMapLno( ULONG mid, int lno, int sfi, ULONG *span , DEBFILE *pdf );
ULONG   DBMapNonExLine( ULONG mid, int LineNumberToMap, int sfi, DEBFILE *pdf );
UINT    DBNextMod(UINT,DEBFILE *);
UCHAR  *DBFindProcName(ULONG, DEBFILE *);
char   *GetSourceFileName( ULONG mid, int sfi );
ULONG   MapSourceFileToMidSfi( char *pSourceFileName, int *psfi, DEBFILE *pdf );
CSECT  *GetCsectWithAddr( MODULE *pModule, ULONG address);
void    BuildHL04Tables( MODULE *pModule, DEBFILE *pdf );
void    BuildHL03Tables( MODULE *pModule, DEBFILE *pdf );
void    BuildHL01Tables( MODULE *pModule, DEBFILE *pdf );
void    Build109_16Tables( MODULE *pModule, DEBFILE *pdf );
void    Build109_32Tables( MODULE *pModule, DEBFILE *pdf );
void    Build105Tables( MODULE *pModule, DEBFILE *pdf );
MODULE *GetModuleWithAddr( ULONG addr, DEBFILE *pdf );
LNOTAB *GetLnoWithAddr( CSECT* pCsect, ULONG addr );
char   *GetFileName( ULONG mid, int sfi );
void    AddFileNames( MODULE *pModule, UCHAR *pFileNameTable );
void    MakeSfis( CSECTMAP *pCsectMap );
void    AddFilesToModule( MODULE *pModule, CSECTMAP *pCsectMap );

UCHAR  *AppendHL04Table( LNOTAB *pLnoTab, FIRST_ENTRY_HL04 *pHL04Table,
                         ULONG   MteLoadAddr);

UCHAR  *AppendHL01Table( LNOTAB *pLnoTab, FIRST_ENTRY_HL01 *pHL01Table,
                         ULONG BaseOffset );

UCHAR  *AppendHL03Table( LNOTAB *pLnoTab, FIRST_ENTRY_HL03 *pHL03Table,
                         ULONG BaseOffset );

UCHAR  *Append105Table(  LNOTAB *pLnoTab, FIRST_ENTRY_105 *p105Table,
                         ULONG BaseOffset, int sfi );

UCHAR  *Append109_16Table( LNOTAB *pLnoTab, FIRST_ENTRY_109_16 *p109_16Table,
                           ULONG BaseOffset, int sfi );

UCHAR  *Append109_32Table( LNOTAB *pLnoTab, FIRST_ENTRY_109_32 *p109_32Table,
                           ULONG BaseOffset, int sfi );

LNOTAB *GetLnoTabEntry( MODULE *pModule, int lno, int sfi );
LNOTAB *GetLnoTabNextEntry( MODULE *pModule, int lno, int sfi );
ULONG   GetLineSpan( MODULE *pModule, LNOTAB *pLnoTabBase  );
UCHAR  *GetHL04FileName( ULONG mid, int sfi );
void    GetHL03FileNameTable( ULONG mid );
void    GetHL01FileNameTable( ULONG mid );
int     GetHL04Sfi( ULONG mid, UCHAR *pFileName );
int     GetHL03Sfi( ULONG mid, UCHAR *pFileName );
int     GetHL01Sfi( ULONG mid, UCHAR *pFileName );
int     Get109_16Sfi( ULONG mid, UCHAR *pFileName );
int     Get105Sfi( ULONG mid, UCHAR *pFileName );
int     Get109_32Sfi( ULONG mid, UCHAR *pFileName );
int     GetSfi( UCHAR *pFileName, UCHAR *pFileNameTable, ULONG FileNameTableLen );
UCHAR  *BuildFileNameTable( FILE_NAME_TABLE_MAP *pTableMap, ULONG *pTableLen);
/*---------------------------------------------------------------------------*/
/* File : DBIFEXT.C                                                          */
/*---------------------------------------------------------------------------*/
DEBFILE *DBFindPdf(uint);
DEBFILE *FindExeOrDllWithAddr( ULONG );                                 /*101*/
DEBFILE *FindExeOrDllWithSelOff( USHORT , USHORT );                     /*827*/
void     dfilefree(void);
void     afilefree(void);
void     freepdf(DEBFILE *);
void     FreeSyms( DEBFILE * );
void     FreeTyps( DEBFILE * );
void     FreePubs( DEBFILE * );
int      DBGetAsmLines(AFILE *,uint,uint *,uint);                       /*530*/
uchar    GetBitness(uint);                                              /*101*/
ushort   MapAddrtoObjnum(DEBFILE *,uint,uint *);                        /*115*/
int      MapFlatAddrToBase(DEBFILE *,ULONG,ULONG *,USHORT *,USHORT *);  /*827*/
int      MapSelOffToBase(DEBFILE *,USHORT,USHORT,ULONG *,USHORT *);     /*827*/
void     hexstr2higit( char *cp, char *phigit );                        /*827*/
UCHAR   *GetCodeBytes( ULONG , UINT, UINT *  );                         /*827*/
UCHAR   *GetDataBytes( ULONG , UINT, UINT *  );                         /*827*/
void     GetMemoryBlocks( void );
void     FreeMemBlks( void );

/*---------------------------------------------------------------------------*/
/* File : RUN.C                                                              */
/*---------------------------------------------------------------------------*/
AFILE *Getfp_focus(void);
void   Run( ULONG ,ULONG);
void   TransLateRcToMessage(uint , char *);
void   SetExecuteFlag( BOOL );
BOOL   IsExecuteFlag( void );

/*---------------------------------------------------------------------------*/
/* File : IDBRK.C                                                            */
/*---------------------------------------------------------------------------*/
uint     ParseNameAddr(char*, ULONG *, uchar *);
APIRET   SetIDBrk( UCHAR *, int, int, char **);
APIRET   ConvertDefBrks( PROCESS_NODE *,DEBFILE *,int);
void     SaveBrks(PROCESS_NODE *);
void     DisplayBkPntChoice( DIALOGSHELL *, DIALOGCHOICE * );
BOOL     DoYouWantToDefer( void );

/*---------------------------------------------------------------------------*/
/* File : FINDFP.C                                                           */
/*---------------------------------------------------------------------------*/
AFILE *findfp( ULONG instaddr );
AFILE *mid2fp(uint);
void  DropOrMakeFakefp(uint,uint,DEBFILE *);                            /*216*/
AFILE *FindFuncOrAddr(uchar *,int);

/*---------------------------------------------------------------------------*/
/* File : THREADS.C                                                          */
/*---------------------------------------------------------------------------*/
void    InitThreadList(void);
void    FreeThreadList(void);
void    SetThdDbgState(uint,uchar);
uchar   GetThdDbgState(uint);
uchar   GetThreadState(uint);
void    BuildThreadList(void);
int     IsRunnableThread(void);
uint    RunThread(uint);
TSTATE *GetThreadTSTATE( uint );

AFILE *showthds(uint *);                                                /*229*/


/*---------------------------------------------------------------------------*/
/* File : PAGEFP.C                                                           */
/*---------------------------------------------------------------------------*/
void pagefp(AFILE*,uint);
void MarkLineBrksOK( DEBFILE *pdf,
                     ULONG    mid,
                     int      sfi,
                     UCHAR   *srcbuf,
                     USHORT  *offtab,
                     UINT     FirstLineInSrcBuf,
                     UINT     LastLineInSrcBuf);

/*---------------------------------------------------------------------------*/
/* File : MAKEFP.C                                                           */
/*---------------------------------------------------------------------------*/
AFILE *makefp(DEBFILE *pdf, ULONG mid, ULONG instaddr, UCHAR *fname);
AFILE *fakefp(UINT, UINT, DEBFILE *);

/*---------------------------------------------------------------------------*/
/* File : LDSOURCE.C                                                         */
/*---------------------------------------------------------------------------*/
void  LoadSource(uchar *,uchar *,ushort *,uint,ushort *,uint *,uint *); /*101*/
uint  FRopen(uchar*,uchar*,ushort);                                     /*100*/
uint  FRclose(uchar*);                                                  /*101*/
uchar *FRread(uchar*);                                                  /*101*/

/*---------------------------------------------------------------------------*/
/* File : LDMSH.C                                                            */
/*---------------------------------------------------------------------------*/
void  LoadMSH   (        uchar *,ushort *,uint,ushort *,uint *,uint *); /*101*/
/*---------------------------------------------------------------------------*/
/* File : FREEFP.C                                                           */
/*---------------------------------------------------------------------------*/
void freefp(AFILE *);

/*---------------------------------------------------------------------------*/
/* File : SHOWC.C                                                            */
/*---------------------------------------------------------------------------*/
uint    showC(AFILE *,uchar *);         /* chg to return uint.            521*/
uint    dofwdtab(AFILE *,uchar *,uint);
uint    dobwdtab(AFILE *,uchar *,uint);
void    dohcsr(AFILE *,int);
void    dohscr(AFILE *,int);
void    dovcsr(AFILE *,int);
void    dovscr(AFILE *,int);
uint    IsOKchar(uchar);
uint    IsOnCRT(AFILE *);
int     ResizeWindow(AFILE *,int);      /* removed static                 701*/
int     ShrinkStorage(AFILE *,uint);
int     ExpandStorage(AFILE *,uint);
void    SetCursorPos( void );                                           /*701*/
int     GetEventView( void );                                           /*701*/
void    SetSrcViewCsr( AFILE * );                                       /*701*/
void    WaitForInterrupt( void * );
void    ConnectThisPid( int );

/*---------------------------------------------------------------------------*/
/* File : SHOWCF.C                                                           */
/*---------------------------------------------------------------------------*/
void   fmtscr(AFILE *);
void   fmtfname(AFILE *);
void   fmtpos(AFILE *);
void   fmttxt(AFILE *);

AFILE *locatefp(uchar *,uint);
uint   token(uchar *,uint, char *);
DFILE *dumpvar(AFILE *, UCHAR *, UINT, UINT, UINT, AFILE **);
void   RefreshSrcPart(AFILE *,int,int,int);

/*---------------------------------------------------------------------------*/
/* File : TYPES.C                                                            */
/*---------------------------------------------------------------------------*/
UCHAR  *QtypeName(UINT ,USHORT );                                       /*813*/
USHORT  QtypeNumber(UINT, UCHAR *, UINT *);

uint    QtypeSize(uint,USHORT);                                          /*813*/
uint    QtypeGroup(uint,uint);
Trec   *QtypeRec(uint,USHORT);                                           /*813*/
Trec   *QbasetypeRec(uint,USHORT);                                       /*813*/
int     QstructField(uint,uchar*,uint *,uint *);                         /*826*/
int     QarrayItem(uint,uint,uint *,uint *);                             /*245*/
int     QderefType(uint,USHORT *);                                       /*813*/
ULONG   QNameList( Trec *, int , int );                                  /*813*/
uint    HandleUserDefs(uint,uint);                                       /*512*/
char   *QTagName( Trec * );
USHORT  QtypeNumberAndMid( UCHAR *, UINT * );
ULONG   QClassItemList( UINT mid, TD_TYPELIST *tp, int Request , int ItemNo );
USHORT  Qbasetype(UINT mid, USHORT typeno);

/*---------------------------------------------------------------------------*/
/* File : UNWIND.C                                                           */
/*---------------------------------------------------------------------------*/
uint   ActiveProcsMenu(AFILE **);
uint   Cua_ActiveProcsMenu(AFILE **);
void   SetActFrames(void);
uint   StackFrameIndex(SCOPE);
uint   StackFrameAddress(uint);                                         /*101*/
void   FormatProcName(uchar *,uint,SCOPE,DEBFILE *,uchar,int);          /*824*/
uint   DBDLLLoadInfo(uint,DEBFILE *);                                   /*107*/
uchar  StackFrameMemModel(uint);                                        /*112*/
char*  bcopy(char*,char*,uint);                                      /*215107*/
uint   CsetThunk(uint * , uint *);                                      /*706*/
void   SetShowAll( int );                                               /*701*/
void   SetShowNamed( int );                                             /*701*/

/*---------------------------------------------------------------------------*/
/* File : BOXMENU.C                                                          */
/*---------------------------------------------------------------------------*/
uint _System BoxMenu(uint,uint,uint,uint,BoxMenuData *);                /*521*/
uint item2row(int,ulong);

/*---------------------------------------------------------------------------*/
/* File : FINDEXEC.C                                                         */
/*---------------------------------------------------------------------------*/
AFILE *FindExecLine(AFILE *,uint);
AFILE *FindExecAddr(AFILE *,uint);                                      /*107*/
uint   IndexOfMidLnoForFrames(uint,uint);                               /*107*/

/*---------------------------------------------------------------------------*/
/* File : SHOWD.C                                                            */
/*---------------------------------------------------------------------------*/
void   ShowData(uint);
uint   ShowScalar(DFILE *,uint,uint,uint);
uint   ShowConstant(DFILE *,uint,uint,uint);
uint   ShowHexBytes(DFILE *,uint,uint,uint);
uchar  graphic(uchar);
void   InitDataBuffer(uchar *,uint,DFILE *);
uchar *GetAppData(uint,uint,uint*,uint);                                /*101*/
uint   StackBPRelToAddr(uint,uint);                                     /*112*/
uint   BytesToValue(uint,uint,uint,uint,uint *);                        /*523*/

/*---------------------------------------------------------------------------*/
/* File : SHOWDK.C                                                           */
/*---------------------------------------------------------------------------*/
typedef int (* SIZEFUNC)(AFILE *,int);                                  /*521*/
void   InitDataWinCsr(void);                                            /*701*/
int    typoS(SIZEFUNC, AFILE *, AFILE **);
DFILE *AppendDataFile(DFILE *);
void   InsDataFile(uint,DFILE * );
void   SetShowType(DFILE *,uint);                                       /*101*/
void   QueueDrec(DFILE *);
void   DropDrec(DFILE *);
void   DelDataFile(uint);
int    SetShowLines(DFILE *);
uchar *ZapExpr(uint,uchar *,uint *,AFILE *);
DFILE *FindDrec(uint);
DFILE *FindNearDrec(uint);
void   SetType(DFILE *);                                                /*813*/
void  SetDataViewCsr( void );                                           /*701*/
/*---------------------------------------------------------------------------*/
/* File : SHOWDX.C                                                           */
/*---------------------------------------------------------------------------*/
uint ZapHexBytes(DFILE *,uint,uint);
void FmtAdrBox(uchar *,uint);
int  x2tou(uchar *,uchar *);
int  GetIthHexField( PEVENT );                                           /*701*/
int  GetStorageArea( PEVENT );                                           /*701*/

/*---------------------------------------------------------------------------*/
/* File : SHOWDZ.C                                                           */
/*---------------------------------------------------------------------------*/
uint ZapPointer(DFILE *,uint,uint);                                     /*521*/
int  _System KeyPointer(uint,uint,uint,uint,uint,uint);                 /*521*/
uint x4tou(uchar *,ushort *);
uint x8tou(uchar *,uint *);                                             /*108*/
uint ZapScalar(DFILE *,uint,uint);                                      /*521*/
int  _System KeyScalar(uint,uint,uint,uint,uint,uint);                  /*521*/
int  ParseScalar(uchar *,uint,ulong *);
int  PutAppData(uint,uint,uchar *,uint);
uint ZapHexWord(uint,uint,uchar *);                                     /*226*/
uint ZapHexDWord(uint,uint,uchar *);                                    /*226*/

ulong cvtnum(uchar *,uint );            /* moved here.                    521*/

/*---------------------------------------------------------------------------*/
/* File : SHOWD2.C                                                           */
/*---------------------------------------------------------------------------*/
uint    ShowStruct(DFILE *,uint,uint,uint);                             /*521*/
uint    ZapStruct(DFILE *,uint,uint);                                   /*521*/
uint    ShowArray(DFILE *,uint,uint,uint);                              /*521*/
uint    ZapArray(DFILE *,uint,uint);                                    /*521*/
UINTFUNC PickKeyer(uint,uint);                                          /*112*/
uint     FalseKeyer(void);

/*---------------------------------------------------------------------------*/
/* File : SHOWDF.C                                                           */
/*---------------------------------------------------------------------------*/
void   fmtdwinstat(uint,int);

/*---------------------------------------------------------------------------*/
/* File : EXPR.C                                                             */
/*---------------------------------------------------------------------------*/
UCHAR *ParseExpr(UCHAR *, uint, uint, uint, int sfi);
uint   GetAbsAddr(uint,uint,uint *);
uint   ParseArrayIndex(uchar *,uint,uint, int sfi, uint *);

/*---------------------------------------------------------------------------*/
/* File : SYMBOLS.C                                                          */
/*---------------------------------------------------------------------------*/
uchar    *CopyProcName(void *,uchar *,uint);
void     *FindScope(uint,uint *);
void     *LocateScope(uint,uint,DEBFILE *);
uint     findlvar(uint,uchar *,uint, int sfi, BOOL *this);
uint     findsvar(uint,uchar *);
SSRec    *Qsymbol( uint mid, uint lno, int sfi, uchar *vsym );
uchar    *findsym(uchar *,uchar **,uchar*);

/*---------------------------------------------------------------------------*/
/* File : DBSEGS.C                                                           */
/*---------------------------------------------------------------------------*/
SSRec   *DBSymSeg(uint, ULONG *, DEBFILE *);
uchar   *DBTypSeg(uint,uint *,DEBFILE *);
uchar   *DBPubSeg(uint,uint *,DEBFILE *);
uchar    IsSymSeg32(uchar *SymSegPtr);

/*---------------------------------------------------------------------------*/
/* File : SD86PRO.C                                                          */
/*---------------------------------------------------------------------------*/
void Profile( KEY2FUNC[]);
void HelpScreen(void);
uint HelpDialogFunction( DIALOGSHELL *, DIALOGCHOICE *, EVENT *, void * );
void DisplayHelpChoice( DIALOGSHELL *, DIALOGCHOICE * );

/*---------------------------------------------------------------------------*/
/* File : SHOWA.C                                                            */
/*---------------------------------------------------------------------------*/
uint showA(AFILE *,uchar *);
int  Recalibrate(AFILE *,int);
void SetAsmViewCsr( AFILE *, uint * );                                  /*701*/

/*---------------------------------------------------------------------------*/
/* File : SCROLLA.C                                                          */
/*---------------------------------------------------------------------------*/
uint ascroll(AFILE *,int);                                              /*235*/
uint IsOnAsmCRT(uint);                                                  /*235*/
void RefreshAsmPart(int,int);                                           /*519*/
void FreeDisasmViewBuffers(void);                                       /*813*/

/*---------------------------------------------------------------------------*/
/* File : SHOWAF.C                                                           */
/*---------------------------------------------------------------------------*/
void   fmtasm(AFILE *);
void   utox2(uchar,uchar *);                                            /*213*/
void   utox4(uint,uchar *);
void   utox8(uint,uchar *);                                             /*112*/
uint   atou(uchar *);
void   fmtthread(void);

/*---------------------------------------------------------------------------*/
/* File : MENU.C                                                             */
/*---------------------------------------------------------------------------*/
uint MenuBar(uchar *,uchar *, ULONG *,uchar *,uint *,uint,uint);

/*---------------------------------------------------------------------------*/
/* File : ACTBAR.C                                                           */
/*---------------------------------------------------------------------------*/
int   scan(AFILE *,uchar *);                                            /*701*/

/*---------------------------------------------------------------------------*/
/* File : DISASM.C                                                           */
/*---------------------------------------------------------------------------*/
int    DBDisa(uint,uint,uint,int);
void  *instdelta(void*,int,uint,uint);
uint   rollback(uint,int,uint);         /* 1st parm uint last parm dropped107*/
int    cache(uint,uint);                /* 1st parm changed to uint.      235*/
void   GetInstrPacket(uint,DTIF *);     /* 1st parm changed to uint.      101*/
uchar  InstLength(uint);                /* replaced InstLength & InstLem  101*/
uchar  InstLengthGlob(uchar*,uchar);    /* to expedite consecutive calls  107*/
int    FindAddrInCache( uint addr );    /*                                235*/

/*---------------------------------------------------------------------------*/
/* File : CBRK.C                                                             */
/*---------------------------------------------------------------------------*/
int    EvalCbrk(BRKCOND *);
uchar *ParseCbrk(BRKCOND *,uint,uint, int sfi);
uchar *ParseDbrk(uchar *);
int    SetHardBreak(void);
int    ResetHardBreak(void);

/*---------------------------------------------------------------------------*/
/* File : DLL.C                                                              */
/*---------------------------------------------------------------------------*/
APIRET dllinit(HMODULE,char *,OBJTABLEENTRY *);

/*---------------------------------------------------------------------------*/
/* File : EXE.C                                                              */
/*---------------------------------------------------------------------------*/
ULONG exeinit(HMODULE,char *,OBJTABLEENTRY *);                          /*822*/

/*---------------------------------------------------------------------------*/
/* File : ASMINIT.C                                                          */
/*---------------------------------------------------------------------------*/
void  asminit(DEBFILE * );

/*---------------------------------------------------------------------------*/
/* File : DEBFILE.C                                                          */
/*---------------------------------------------------------------------------*/
APIRET  debfileinit(DEBFILE *) ;
void    InitModules(char *, int, DEBFILE *);
APIRET  InitModuleAddrs( DEBFILE *);
void    SetDebugFormatFlags(char *, int, DEBFILE *);
MODULE *GetModulePtr(ULONG, DEBFILE *);
APIRET  FindDebugStart( HFILE, ULONG *, int *, int *);
ULONG   FindOS2hdr(DEBFILE *);

/*---------------------------------------------------------------------------*/
/* File : FINDFILE.C                                                         */
/*---------------------------------------------------------------------------*/
void  findsrc( char *, char *, uint);
UINT  FindExe( char *, char *, UINT);

/*---------------------------------------------------------------------------*/
/* File : ZOOMDATA.C                                                         */
/*---------------------------------------------------------------------------*/
uint  zoomrec(AFILE *, DFILE *, int, AFILE **);

/*---------------------------------------------------------------------------*/
/* File : BROWSE.C                                                           */
/*---------------------------------------------------------------------------*/
int   browse(char *, int);
void  scrollwindow(AFILE *,int);
void  scrollfile(AFILE *,uint);


/*---------------------------------------------------------------------------*/
/* File : FINDPRO.C                                                          */
/*---------------------------------------------------------------------------*/
void  findpro(char *,char *,uint);

/*---------------------------------------------------------------------------*/
/* File : VIEWASM.C                                                          */
/*---------------------------------------------------------------------------*/
void   buildasmline(uchar *,INSTR *);                    /* return void   108*/
void   buildopnd(uchar *,INSTR *);
void   buildsyms(uchar,int);
uint   selreg(uint);
uint   offreg(uint);
ULONG  BaseVal(INSTR *);
uchar* grabopnd(INSTR *,uint);
void   buildsidesrc(uchar *,AFILE *,uint);
void   buildasmsrc(uchar *,AFILE *,uint);

/*---------------------------------------------------------------------------*/
/* File : TERM.C                                                             */
/*---------------------------------------------------------------------------*/
void Restart(void);
int  NormalQuit(int);                                                   /*710*/
int  GoTerm( void );                                                    /*710*/
void PostWait4DbgQueTermSema4( void );                                  /*919*/

/*---------------------------------------------------------------------------*/
/* File : SHOWDLLS.C                                                         */
/*---------------------------------------------------------------------------*/
void ShowDlls(void);
void DisplayDllChoice (DIALOGSHELL *,DIALOGCHOICE *);                   /*701*/
uint DllDialogFunction( DIALOGSHELL *, DIALOGCHOICE *, EVENT *, void * );

/*---------------------------------------------------------------------------*/
/* File : GETFILE.C                                                          */
/*---------------------------------------------------------------------------*/
AFILE* GetFile( uchar * );

/*---------------------------------------------------------------------------*/
/* File : SETCOLOR.C                                                         */
/*---------------------------------------------------------------------------*/
void SetColors(void);
void RefreshColors(uint,uint,uint,uint);
uint SaveColors(uchar *);
void DisplayColorChoice(DIALOGSHELL *,DIALOGCHOICE *);                  /*701*/
void ReDisplaySingleColor(uint,uint,uint,uchar *);                      /*701*/
void SetDefaultColorMap(void);                                          /*701*/
uint GetEventPositionInDialog( DIALOGSHELL *, DIALOGCHOICE *, uint *, uchar );
                                                                        /*701*/
uint ColorDialogFunction( DIALOGSHELL *, DIALOGCHOICE *, EVENT *, void * );

/*---------------------------------------------------------------------------*/
/* File : SETEXCEP.C                                                         */
/*---------------------------------------------------------------------------*/
void SetExceptions(void);                                               /*308*/
void RefreshExceptions(uint,uint,uint,uint);                            /*308*/
uint SaveExceptions(uchar *);                                           /*308*/
void DisplayExcepChoice (DIALOGSHELL *,DIALOGCHOICE *);                 /*701*/
void SetDefaultExcepMap(void);                                          /*701*/
uint ExcepDialogFunction( DIALOGSHELL *, DIALOGCHOICE *, EVENT *, void * );

/*---------------------------------------------------------------------------*/
/* File : MAPLINES.C                                                      813*/
/*---------------------------------------------------------------------------*/
LINNUM_ENTRY *MapHL04LineNumbers( LINE_NUMBER_TABLE_ENTRY_HL04 *,
                                  USHORT,
                                  USHORT*,
                                  ULONG);

void    MapCV16LineNumbers( uint *, uint *, uint, ushort );
void    MapCV32LineNumbers( uint *, uint *, uint, ushort );
void    MapT105LineNumbers( uint *, uint *, uint );
USHORT  MapHLLLineNumbers( uint *, uint *, uint, MODULE * );
int     compare( const void *key, const void *element);
int     ucompare( const void *key, const void *element);

/*---------------------------------------------------------------------------*/
/* File : MAPTYPES.C                                                      813*/
/*---------------------------------------------------------------------------*/
uchar *MapHLLTypes( uchar *RawTable, MODULE *mptr );
uchar *MapMSTypes( uchar *RawTable, MODULE *pModule );
uchar *GetField( uchar *, void *, uint * );
ushort GetInternal0_16PtrIndex( ushort TypeIndex );
USHORT GetEncLength( ENCLEN *pEncLen, int *pNumBytes );
BOOL   IsVirtual( HL_MEMFNC *pHLLMemFnc );
BOOL   IsStatic( HL_CLSMEM *pHLLClsMem );
BOOL   IsVtable( HL_CLSMEM *pHLLClsMem );

/*---------------------------------------------------------------------------*/
/* File : MAPSYMS.C                                                       813*/
/*---------------------------------------------------------------------------*/
void  MapHLLSyms( uchar *, SSRec *, MODULE * );                         /*809*/
void  MapMS16Syms( uchar *RawTable, SSRec *IntSymTable, uint symlen );  /*809*/
void  MapMS32Syms( uchar *RawTable, SSRec *IntSymTable, uint symlen );  /*809*/
ushort  GetRecordLength( uchar *, MODULE * );                           /*809*/
uchar   GetRecordType( uchar *, MODULE * );                             /*809*/

/*---------------------------------------------------------------------------*/
/* File : POINTER.C                                                          */
/*---------------------------------------------------------------------------*/
UINT  GetPtrType(uint,USHORT );                                         /*813*/
uint  DerefPointer(uint,uint);                                   /*240*//*112*/
uint  ResolveAddr(uint,DFILE *,uint);                                   /*219*/

/*---------------------------------------------------------------------------*/
/* File : SHOWVREG.C                                                      400*/
/*---------------------------------------------------------------------------*/
void   ShowvRegs(void);                                                 /*400*/
AFILE *KeyvRegs(uint *);                                                /*400*/
uint   _System ZapvRegs(void);                                      /*521 400*/
void   GetScrAccess(void);                                              /*400*/
void   SetScrAccess(void);                                              /*400*/
void   SetRegChgMask(void);                                             /*400*/
uint   SetExecLine(uint );                                              /*508*/

/*---------------------------------------------------------------------------*/
/* File : SHOWCORG.C                                                      401*/
/*---------------------------------------------------------------------------*/
void   ShowCoRegs(void);                                                /*401*/
void   Converttox(uchar *,uchar *);                                     /*401*/
void   FillIn(uchar *,uchar );                                          /*401*/

/*---------------------------------------------------------------------------*/
/* Proto types for 16 bit DosAllocSeg, DosReallocSeg and DosFreeseg       521*/
/*---------------------------------------------------------------------------*/
#define DosAllocSeg     DOS16ALLOCSEG
#define DosFreeSeg      DOS16FREESEG
#define DosReallocSeg   DOS16REALLOCSEG

APIRET16  APIENTRY16 DosAllocSeg(USHORT cbSize, PSEL pSel, USHORT fsAlloc);
APIRET16  APIENTRY16 DosFreeSeg(SEL sel);
APIRET16  APIENTRY16 DosReallocSeg(USHORT cbNewSize, SEL sel);

/*---------------------------------------------------------------------------*/
/* File : SETWPS.C                                                        602*/
/*---------------------------------------------------------------------------*/
void  SetWps(AFILE *);                                                  /*602*/
uchar EncodeSize(uint);                                                 /*602*/
uchar AlignAddress(uint *,uchar);                                       /*602*/
void  PutInWps(void);                                                   /*602*/
uint  PullOutWps(DEBUG_REGISTER *);                                     /*602*/
void  ResetWp(uint);                                                    /*602*/
int   IsWatchPoint( void );

/*---------------------------------------------------------------------------*/
/* File : ALLOCTXT.C                                                      606*/
/*---------------------------------------------------------------------------*/
void AddModuleToPdf(DEBFILE *,MODULE *,ushort,uint,uint);               /*606*/

/*---------------------------------------------------------------------------*/
/* File : acvt.c                                                          607*/
/*---------------------------------------------------------------------------*/
void  Data_Flat2SelOff(ULONG addr,USHORT *pSelector,USHORT *pOffset);
ULONG Data_SelOff2Flat (USHORT Selector, USHORT Offset);
void  Code_Flat2SelOff(ULONG addr,USHORT *pSelector,USHORT *pOffset);
ULONG Code_SelOff2Flat (ushort Selector,ushort Offset);

/*---------------------------------------------------------------------------*/
/* File : Dialog.c                                                           */
/*---------------------------------------------------------------------------*/
void  windowsv(int,int,int,int,char *);                                 /*701*/
void  windowrst(int,int,int,int,char *);                                /*701*/
void  DisplayDialog(DIALOGSHELL *, int );                               /*701*/
void  RemoveDialog(DIALOGSHELL *);                                      /*701*/
uchar VerifyMouseinDialog (DIALOGSHELL *,DIALOGCHOICE *,uint *,int *);  /*701*/
uchar VerifyMouseinDialogButtons (DIALOGSHELL *,uint *);                /*701*/
UINT  ProcessDialog( DIALOGSHELL *, DIALOGCHOICE *, uchar, void * );     /*701*/
UINT  ProcessYesNoBox( DIALOGSHELL *, DIALOGCHOICE * );

/*---------------------------------------------------------------------------*/
/* File : popup.c                                                            */
/*---------------------------------------------------------------------------*/
void    DisplayPop(POPUPSHELL * );                                      /*701*/
uint    GetPopStr(POPUPSHELL *,uint,uint,uint,uchar *);                 /*701*/
BUTTON *GetButtonPtr (POPUPSHELL *, PEVENT);                            /*910*/
void    stripblk (uchar *);                                             /*701*/
void    GetPopArea( char * );                                           /*701*/
void    PutPopArea( char * );                                           /*701*/

/*---------------------------------------------------------------------------*/
/* File : Mouse.c                                                            */
/*---------------------------------------------------------------------------*/
void   AccessMouse( void );
PEVENT GetEvent( uint );
PEVENT GetCurrentEvent( void );
void   HideMouse(void);
void   ShowMouse(void);
void   CloseMouse(void);
void   MouseThread(void *);
void   KbdThread(void *);
void   StartThreads( void );
void   CloseEventSemaphores(void);

/*---------------------------------------------------------------------------*/
/* File : Cuamenu.c                                                          */
/*---------------------------------------------------------------------------*/
int   CuaMenuBar( int , void *);
void  DisplayMenu( void );
int   GetEventPositionInMenu( int , int * );
int   MouseEventInActionBar( int  * );
int   MouseEventInPulldown( int , int  * );
int   MouseEventInArrow( PULLDOWN *, int );
void  RemoveMenuCursor( int  );
void  DisplayMenuCursor( int  );
void  DisplayPullCursor( int , int  );
void  RemovePullCursor( int , int  );
void  DisplayPulldown( int  );
void  RemovePulldown( int  );
void  AdjustPulldownMask( AFILE * );
void  UpdatePullDownsWithAccelKeys( void );
int   HandleMouseEvent( void );
int   HandleKbdEvent( void ) ;
uint  KeyInActionBarExpressKeys( int * );
void  DisplayCasPulldown( CAS_PULLDOWN );
int   GetCasPulldownChoice( CAS_PULLDOWN *, int *, int * );
int   GetPulldownChoice( int, int *, int *, int );
int   GetActionbarChoice( int *, int *, int *, int * );
int   GetObjectPullChoice( int, int * );
int   IsSwapFlag( int );
void  SetSelectMask( int , int, int );
void  SetMenuMask( int );
PULLDOWN  *GetPullPointer( int );

void  DisableDropFile( void );
void  EnableDropFile ( void );
void  ReSetSelectBit ( int PullIndex, int PullChoiceIndex );
void  SetSelectBit   ( int PullIndex, int PullChoiceIndex );

/*---------------------------------------------------------------------------*/
/* File : Cuashowa.c                                                         */
/*---------------------------------------------------------------------------*/
uint Cua_showA(AFILE *,uchar *);                                        /*701*/

/*---------------------------------------------------------------------------*/
/* File : CuaMap.c                                                           */
/*---------------------------------------------------------------------------*/
int  GetFuncsFromEvents( int , void * );                                /*701*/

/*---------------------------------------------------------------------------*/
/* File : Cuasetwp.c                                                         */
/*---------------------------------------------------------------------------*/
void  Cua_SetWps(AFILE *);                                              /*701*/
void  DisplayWhPntChoice (DIALOGSHELL *,DIALOGCHOICE *);                /*701*/
uint  GetDialogWhPntChoice (DIALOGSHELL *,DIALOGCHOICE *,AFILE *);      /*701*/

/*---------------------------------------------------------------------------*/
/* File : Cuathds.c                                                          */
/*---------------------------------------------------------------------------*/
AFILE *Cua_showthds(void);                                              /*701*/
void   DisplayThdsChoice (DIALOGSHELL *,DIALOGCHOICE *);                /*701*/
uint   ThreadsDialogFunction( DIALOGSHELL *, DIALOGCHOICE *, EVENT *, void * );

/*---------------------------------------------------------------------------*/
/* File : cuaproc.c                                                          */
/*---------------------------------------------------------------------------*/
void   Cua_showproc( void );
void   DisplayProcChoice( DIALOGSHELL *, DIALOGCHOICE * );
uint   ProcessDialogFunction( DIALOGSHELL *, DIALOGCHOICE *, EVENT *, void * );

/*---------------------------------------------------------------------------*/
/* File : Cuaclstk.c                                                         */
/*---------------------------------------------------------------------------*/
void DisplayClstkChoice (DIALOGSHELL *,DIALOGCHOICE *);                 /*701*/
uint ClstkDialogFunction( DIALOGSHELL *, DIALOGCHOICE *, EVENT *, void * );

/*---------------------------------------------------------------------------*/
/* File : Action.c                                                        701*/
/*---------------------------------------------------------------------------*/
AFILE       *GetFunction( char* , uint );
AFILE       *GetF( void );
void         FindStr( AFILE *);
int          ScanStr( AFILE *);
void         BrowseFile( void);
void         SetCondBrk( AFILE *, uint , uchar * );
uint         PopPrompt( char *, char *);
void         SetNameOrAddrBkpt( AFILE *, uint );
int          GetLineNumber( char );
UINT         GetFormatType( DFILE * );

/*---------------------------------------------------------------------------*/
/* File : Import.c                                                           */
/*---------------------------------------------------------------------------*/
uint     ResolveImport( char *, DEBFILE * );
uint     ResolveImport32( char *, DEBFILE *, uchar * );
DEBFILE *findpdf( char * );
uint     findexport( DEBFILE *, char * );
uint     findexport32( DEBFILE *, char * );
int      PassbyImportByOrdFix( int, int, int );
int      PassbyImportByNameFix( int, int, int );
int      PassbyInternalEntFix( int, int, int );
int      PassbyInternalFix( int, int, int );
int      FindImportName( char *, DEBFILE *, ulong, int );

#endif /* end of INCL_FUNCS */                                          /*813*/
/*---------------------------------------------------------------------------*/
/* File : okeystr.c                                                          */
/*---------------------------------------------------------------------------*/
#if 0
uint         keystr(uint,uint,uint,uint *,uchar *, uint);
static uint  FldUsed(void);
static uchar shiftRight(void);
static void  shiftLeft(void);
static int   Left(uint);
static int   Right(uint);
static int   Home(uint);
static int   End(uint);
static int   Insert(uint);
static int   Delete(uint);
static int   Backspace(uint);
static int   Default(uint);
static void  keybeep(void);

#endif

/*---------------------------------------------------------------------------*/
/* fmtdata.c defined functions.                                              */
/*---------------------------------------------------------------------------*/
#ifdef INCL_FMTDATA
 void   FormatDataItem(UCHAR *,UINT,UINT,USHORT,UINT);
 void   FmtChar( char *, ULONG , UINT, USHORT);
 void   FmtShort( char *, ULONG , UINT, USHORT);
 void   FmtLong( char *, ULONG , UINT, USHORT);
 void   FmtFloat( UINT, char *, ULONG , UINT, USHORT);
 USHORT FmtPtr( char **, ULONG*, UINT, USHORT, UINT);
 void   FmtBytes( char *, ULONG , UINT , UINT );
 void   FmtString( char *, char *,ULONG, UINT );
 USHORT FmtEnums( Trec *,char **, ULONG , UINT , USHORT, UINT );
 void   FmtBitField(Trec *, char *, ULONG,  UINT);
 void   FmtMemFncAttrs( Trec *pTrec, UCHAR *cp );
 void   FmtClsMemAttrs( Trec *pTrec, UCHAR *cp );
 void   FmtBseClsAttrs( Trec *pTrec, UCHAR *cp );
 void   FmtClsDefAttrs( Trec *pTrec, UCHAR *cp );
 void   FmtRef( char *cp, ULONG UserAddr, UINT sfx );
 void   FmtFriendAttrs( Trec *pTrec, UCHAR *cp );
#endif

/*---------------------------------------------------------------------------*/
/* mte.c defined functions.                                                  */
/*---------------------------------------------------------------------------*/
#ifdef INCL_MTE
 void   *UpdateMteTable( UINT*);
 void    MarkTheFreedMtes( UINT );
 void    AddObjectsToMteTable( UINT , OBJTABLEENTRY * );
 APIRET  ExeDllInit( UINT * );
 ULONG   GetLoadAddr( ULONG , UINT );
 void    FreeMteTable( void );
 void    CopySharedMteTab( ULONG *);
 void    FreeMte( HMODULE );
#endif


/*---------------------------------------------------------------------------*/
/* File : BRK.C                                                              */
/*---------------------------------------------------------------------------*/
#ifdef INCL_BRK
BRK    *DefBrk(ULONG,int);
void    UndBrk(ULONG,int);
void    FreeAllBrks(void);
void    UnMarkLineBRKs(AFILE *);
void    DropBrk(AFILE *,BRK *);
void    DropOnceBrks(UINT);
void    SetAddrBRK(AFILE *, ULONG address, int );
char   *SetLineBRK(AFILE *, UINT, UCHAR *, UINT, char *);
void    MarkLineBRKs(AFILE *);
BRK    *IsBrk(AFILE *,UINT);
BRK    *IfBrkOnAddr(UINT);
int     IsDeferredBrk( void );
#endif


/*---------------------------------------------------------------------------*/
/* File : GO.C                                                               */
/*---------------------------------------------------------------------------*/
#ifdef INCL_GO                                                          /*827*/
APIRET  GoInit( ULONG, ULONG );                                         /*827*/
APIRET  GoEntry( void );
int     Go(UINT);                                                       /*827*/
APIRET  GoFast(int);                                                    /*827*/
APIRET  GoStep(int,int);                                                /*827*/
TSTATE *GetExecThread(void);                                            /*919*/
AFILE  *GetExecfp(void);                                                /*919*/
ULONG   GetExecTid(void);                                               /*919*/
ULONG   GetExecAddr(void);                                              /*919*/
LNOTAB *GetExecLnoTabEntry(void);
ULONG   GetExecLno(void);                                               /*919*/
ULONG   GetExecMid(void);                                               /*919*/
void    SetExecAddr( ULONG );                                           /*919*/
void    SetExecValues( ULONG , int );                                   /*919*/
AFILE  *SetExecfp( void );                                              /*919*/
#endif
/*---------------------------------------------------------------------------*/
/* debug.c    defined functions.                                             */
/*---------------------------------------------------------------------------*/
#ifdef INCL_DEBUG
 void  DumpModuleLoadTable( UINT * );
 void  DumpMteTable( void * );
 void  DumpPubRec16( void * );
 void  DumpObjectTable( DEBFILE *);
 void  DumpUnwindTables( void );
 void  DumpLineNums( MODULE *pModule );
 void  DumpCache( INSTR *);
 void  DumpSourceLineCache( UINT  *);
 void  DumpModuleStructure( void * );
 void  DumpAfileStructure( void * );
 void  DumpString( void *);
 void  DumpINSTRStructure( void *);
 void  DumpTSTATEStructure( void *);
 void  DumpFILE( void * );
 void  DumpEntryList(void*,int);
 void  DumpQue( void *,void *);
 void  DumpCsectMap( MODULE*pm, CSECTMAP *pc, int type);
 void  DumpClassRecord( TD_CLASS *pClassRecord );
 void  DumpMemFncRecord( TD_MEMFNC *pMemFncRecord );
 void  DumpBseClsRecord( TD_BSECLS *pIntBseCls );
#endif

/*---------------------------------------------------------------------------*/
/* xbox.c defined functions.                                              827*/
/*---------------------------------------------------------------------------*/
#ifdef INCL_XBOX
 APIRET xFindExe(char *,char *,UINT );
 APIRET xStartUser(char * ,
                   char * ,
                   USHORT ,
                   ULONG *,
                   ULONG *,
                   ULONG *,
                   ULONG  ,
                   char * );

 APIRET xGoInit( PtraceBuffer *,ULONG *,UINT ** , int* );
 APIRET xGoEntry( PtraceBuffer  *,ULONG *,UINT **,int *,ULONG,int );
 void   xDefBrk( ULONG );
 void   xUndBrk( ULONG );
 void   xPutInBrk( ULONG );
 void   xPullOutBrk( ULONG);
 void   xInsertAllBrk(void);
 void   xRemoveAllBrk(void);
 APIRET APIENTRY xDosDebug(PtraceBuffer *);
 ULONG  xGetThreadInfo(THREADINFO *);
 void   xFreezeThread(ULONG);
 void   xThawThread(ULONG);
 ULONG  xGetCallStack(STACK_PARMS *,UCHAR **,UINT **,UINT ** );
 ULONG  xGetExeOrDllEntryOrExitPt( UINT );
 APIRET xNormalQuit( int, UINT, ULONG);
 APIRET xSetExecAddr(ULONG );
 void   xDefWps( void *, int);
 void   xPutInWps( ULONG *);
 void   xPullOutWps( void );
 UCHAR *xGetMemoryBlock( ULONG , int , int * );
 void  *xGetMemoryBlocks(void *,int );
 void   xSetExceptions( UCHAR *,int );
 APIRET xSetExecThread(ULONG *,PtraceBuffer *, UINT );
 APIRET xWriteRegs(ULONG *,PtraceBuffer *);
 APIRET xGetCoRegs(void * );

 APIRET xGoStep(PtraceBuffer*,ULONG*,UINT **,int *,ULONG,ULONG,ULONG,int,int);
 APIRET xGoFast( PtraceBuffer  *,ULONG *,UINT **,int *,ULONG,int );
 void   xTerminateESP( void );
 void   xSetEspRunOpts( ESP_RUN_OPTS *);                                /*919*/
 void   xSendCtrlBreak( LHANDLE, USHORT );                              /*919*/
 APIRET xStartEspQue( void );                                           /*919*/
 void   xStartQueListenThread( void );                                  /*919*/
 void   xConnectEsp( LHANDLE, USHORT );                                 /*919*/
 void   xSendKillListen( LHANDLE );
 void   xSelectSession( void );
#endif

/*---------------------------------------------------------------------------*/
/* com.c defined functions.                                                  */
/*---------------------------------------------------------------------------*/
#ifdef INCL_COM
 void    AsyncInit( CONNECTION * );
 void    AsyncSetupComPort(int,int);
 APIRET  AsyncOpen(int);
 void    AsyncClose( CONNECTION * );
 APIRET  AsyncSetDCB( USHORT, USHORT );
 APIRET  AsyncSetLineCtrl(void);
 APIRET  AsyncSetBitRate(int);
 APIRET  AsyncSetClearDTR(int);
 void    AsyncSend(PVOID,ULONG);
 void    AsyncRecv(PVOID,ULONG);
 ULONG   AsyncCheckComError(void);
 void    AsyncFlushBuffers(void );
 APIRET  AsyncGetComMsg( char* );
 APIRET  AsyncPeekRecvBuf(USHORT *);
 void    AsyncFlushModem( void );
 int     comdump( ULONG );
 void    Connect( FILE *, CONNECTION * );
 int     AsyncPeekComPort( void );
 LHANDLE GetComHandle( void );
 void    SetComHandle( LHANDLE );
 void    SendAT_CommandToModem( char*, BOOL );
 void    ReadAT_CommandResponse( void );
#endif

/*---------------------------------------------------------------------------*/
/* tx.c defined functions.                                                827*/
/*---------------------------------------------------------------------------*/
#ifdef INCL_TX
 APIRET TxFindExe(char *,char *,UINT );
 APIRET TxStartUser(char * ,
                    char * ,
                    USHORT ,
                    ULONG *,
                    ULONG *,
                    ULONG *,
                    char * );

 APIRET TxGoInit( PtraceBuffer *,ULONG *,UINT ** , int* );
 APIRET TxGoEntry( PtraceBuffer  *,ULONG *,UINT **,int *,ULONG,int );
 APIRET TxGoStep(PtraceBuffer *,ULONG*,UINT **,int *,ULONG,ULONG,ULONG,int,int);
 void   TxDefBrk( ULONG );
 void   TxUndBrk( ULONG );
 void   TxPutInBrk( ULONG );
 void   TxPullOutBrk( ULONG);
 void   TxInsertAllBrk(void);
 void   TxRemoveAllBrk(void);
 APIRET APIENTRY TxDosDebug(PtraceBuffer *);
 ULONG  TxGetThreadInfo(THREADINFO *);
 void   TxFreezeThread(ULONG tid );
 void   TxThawThread(ULONG tid );
 ULONG  TxGetCallStack(STACK_PARMS *,UCHAR **,UINT **,UINT ** );
 ULONG  TxGetExeOrDllEntryOrExitPt( UINT );
 APIRET TxNormalQuit( int, UINT, ULONG );
 APIRET TxSetExecAddr( ULONG );
 APIRET TxGoFast( PtraceBuffer  *,ULONG *,UINT **,int *,ULONG,int );
 void   TxDefWps( void *,int );
 void   TxPutInWps( ULONG * );
 void   TxPullOutWps( void );
 UCHAR *TxGetMemoryBlock( ULONG ,int , int * );
 void  *TxGetMemoryBlocks(void *,int );
 void   TxSetExceptions( UCHAR *, int );
 APIRET TxSetExecThread(ULONG *,PtraceBuffer *, UINT );
 APIRET TxWriteRegs(ULONG *,PtraceBuffer *);
 APIRET TxGetCoRegs(void * );
 void   TxTerminateESP( void );
 void   TxSetEspRunOpts( ESP_RUN_OPTS *);                               /*919*/
 void   TxSendCtrlBreak( LHANDLE, USHORT );                             /*917*/
 void   TxStartQueListenThread( void );                                 /*919*/
 APIRET TxStartEspQue( void );                                          /*919*/
 void   TxConnectEsp( LHANDLE, USHORT );                                /*919*/
 void   TxSendKillListen( LHANDLE );
 void   TxSelectSession( void );
#endif

/*---------------------------------------------------------------------------*/
/* q.c defined functions.                                                 917*/
/*---------------------------------------------------------------------------*/
 APIRET  StartQue( char *, void (* _Optlink)(void*) );                  /*917*/
 void    SendMsgToQue( char *, ULONG,void *, ULONG);                    /*917*/
 void   *PeekQueMsg( LHANDLE, ULONG );

/*---------------------------------------------------------------------------*/
/* File : ESPQ.C                                                          917*/
/*---------------------------------------------------------------------------*/
APIRET  StartEspQue( void );                                            /*917*/
PSZ     GetEspQue( void );                                              /*919*/
void    ReadEspQue( void *);                                            /*917*/
void    SendMsgToEspQue( ULONG , void * , ULONG );                      /*919*/
void    cbrk( void *);                                                  /*917*/
ULONG  _System EspQueThreadHandler(PEXCEPTIONREPORTRECORD ,             /*917*/
                                   PEXCEPTIONREGISTRATIONRECORD ,       /*917*/
                                   PCONTEXTRECORD ,                     /*917*/
                                   PVOID );                             /*917*/
ULONG  _System ListenThreadHandler(PEXCEPTIONREPORTRECORD ,             /*917*/
                                   PEXCEPTIONREGISTRATIONRECORD ,       /*917*/
                                   PCONTEXTRECORD ,                     /*917*/
                                   PVOID );                             /*917*/
ULONG  _System cbrkhandler(PEXCEPTIONREPORTRECORD ,                     /*917*/
                           PEXCEPTIONREGISTRATIONRECORD ,               /*917*/
                           PCONTEXTRECORD ,                             /*917*/
                           PVOID );                                     /*917*/
void    OpenNamedEspPipe( void *);                                      /*919*/
void    CloseNamedEspPipe( void );                                      /*919*/
void    ListenForMsgFromDbg( void * );                                  /*919*/
void    SendNewProcessToDbg( LHANDLE, USHORT, ULONG, USHORT );
void    ConnectDbg( LHANDLE, USHORT );
void    WaitForConnection( void * );
void    PeekForCtrlBreak( void * );
void    SetPeekForCtrlBreakFlag( BOOL );
BOOL    GetPeekForCtrlBreakFlag( void );
void    SetEspQueName( char * );
APIRET  StartEspTermQue( void );
void    ReadEspTermQue( void *);                                        /*917*/
void    SendMsgToEspTermQue(ULONG , void *, ULONG );                    /*919*/
BOOL    AllProbesAreDead( void );
void    ResetAllProbesAreDeadFlag( void );
void    AddProcessToList( USHORT );
BOOL    IsProcessInList( USHORT );

/*---------------------------------------------------------------------------*/
/* File : DBGQ.C                                                          919*/
/*---------------------------------------------------------------------------*/
APIRET  StartDbgQue( void );                                            /*917*/
void    ReadDbgQue( void *);                                            /*917*/
void    SendMsgToDbgQue(ULONG , void *, ULONG );                        /*919*/
APIRET  OpenNamedDbgPipe( void );                                       /*919*/
void    OpenDbgPipeListenThread( void );                                /*919*/
void    CloseNamedDbgPipe( void );                                      /*919*/
void    Create_IO_Sema4( void );                                        /*919*/
void    Request_IO_Sema4( void );                                       /*919*/
void    Release_IO_Sema4( void );                                       /*919*/
void    ListenForMsgFromEsp( void * );                                  /*919*/
void    PostConnectSema4( HEV *, BOOL );                                /*919*/
void    SetPollingThreadFlag( int );
int     GetPollingThreadFlag( void );
void    PollForMsgFromEsp( void * );
void    SetDbgQueName( char * );
APIRET  StartDbgTermQue( void );
void    ReadDbgTermQue( void *);                                        /*917*/
void    SendMsgToDbgTermQue(ULONG , void *, ULONG );                    /*919*/
BOOL    AllDbgsAreDead( void );
void    ResetAllDbgsAreDeadFlag( void );

/*---------------------------------------------------------------------------*/
/* pipe.c defined functions.                                                 */
/*---------------------------------------------------------------------------*/
APIRET PipeInit( CONNECTION *, LHANDLE *);
APIRET CreateNamedPipe( LHANDLE *);
APIRET OpenNamedPipe( LHANDLE *);
void   PipeSend( LHANDLE, PVOID, ULONG );
void   PipeRecv( LHANDLE, PVOID, ULONG );
void   PipeClose( CONNECTION *, LHANDLE );

/*---------------------------------------------------------------------------*/
/* router.c defined functions.                                            919*/
/*---------------------------------------------------------------------------*/
BOOL   IsEspRemote( void );
void   SendConnectionToRouter( CONNECTION * );
APIRET ConnectInit( int * );
void   ConnectClose( LHANDLE );
void   OpenAnotherConnection( LHANDLE * );
int    ConnectType( void );
int    SerialParallel( void );
void   RmtSend( LHANDLE, PVOID, ULONG );
void   RmtRecv( LHANDLE, PVOID, ULONG );
void   SetHaltCommunicationsFlag( void );

/*---------------------------------------------------------------------------*/
/* netbios.c defined functions.                                           919*/
/*---------------------------------------------------------------------------*/
APIRET NetBiosInit( CONNECTION *, LHANDLE *, int * );
void   BuildLsnName( char *, char *, CONNECTION * );
APIRET NetBios_Reset( BYTE, BYTE, BYTE, BYTE, int * );
APIRET NetBios_AddName( char * );
APIRET NetBios_Call_Answer( int, char *, char *, LHANDLE * );
void   NetBios_Send( LHANDLE, PVOID, ULONG );
void   NetBios_Recv( LHANDLE, PVOID, ULONG );
APIRET NetBiosOriginateSession( CONNECTION *, LHANDLE * );
void   NetBiosClose( LHANDLE );

/*---------------------------------------------------------------------------*/
/* qproc.c defined functions.                                                */
/*---------------------------------------------------------------------------*/
void QueryProcesses( void );

/*---------------------------------------------------------------------------*/
/* spawn.c defined functions.                                             919*/
/*---------------------------------------------------------------------------*/
APIRET SpawnProbe( USHORT *,
                   ULONG *,
                   char *,
                   ULONG,
                   char *,
                   ESP_SPAWN_FLAGS);

APIRET SpawnDbg( USHORT,
                 USHORT *,
                 ULONG *,
                 char *,
                 ULONG   ,
                 char * );

/*---------------------------------------------------------------------------*/
/* pid.c defined functions.                                                  */
/*---------------------------------------------------------------------------*/
void         AddPid( USHORT, ULONG, ULONG, ULONG, char *);
void         FreeAllPids( void );
ALLPIDS     *GetPid( USHORT );
BOOL         IsPidConnected( void );
ALLPIDS     *GetAllpids     ( void );
ALLPIDS     *GetPidIndex    ( int  );
ALLPIDS     *GetPidConnected( void );
ULONG       *GetShrMem( void );
void         SetShrMem( ULONG * );
void         SetShrHeap( ULONG * );
void         AddDbgPidAndSid( USHORT, USHORT, ULONG );
int          QueryConnectSema4( void );
void         SetKillChk4Disconnect( BOOL );
void         CheckForDisConnect( void * );
ALLPIDS     *GetEspPid( USHORT );
void         FreeEspPidHeap( void );
void         RemovePid( USHORT );
void         FreeSharedHeap( void );
void         AddMessage( UCHAR, void *, ULONG );
void         RemoveMessage( SQE_MESSAGE * );
void         ReportMessage( void );
SQE_MESSAGE *GetConnectNotification( void );
void         Check4ConnectRequest( void * );
SQE_MESSAGE *IsConnectRequestQueued( USHORT );
ALLPIDS     *GetDbgPid( USHORT );
void         SetExecutionFlag( BOOL );

/*---------------------------------------------------------------------------*/
/* vb.c  defined functions.                                               919*/
/*---------------------------------------------------------------------------*/
void PrintQueMessage( void *,void *);                                   /*919*/
void PrintDbgQueMessage( void *);

/*---------------------------------------------------------------------------*/
/* sema4.c defined functions.                                                */
/*---------------------------------------------------------------------------*/
void CreateConnectSema4( USHORT, int );

void SerialConnect( int,
                    ULONG,
                    int,
                    void  (* QueCaller)( ULONG , void * , ULONG )  );

void SetConnectSema4   ( HEV *, BOOL );
void WaitConnectSema4  ( HEV );
void CloseConnectSema4 ( void );
void PostConnectSema4  ( HEV *, BOOL );
void CreateSerialMutex( void );
void OpenSerialMutex( void );
void RequestSerialMutex( void );
void ReleaseSerialMutex( void );
void CloseSerialMutex( void );

/*****************************************************************************/
/* talloc.c                                                                  */
/*****************************************************************************/
void *Talloc(UINT size);
void *Tfree(void *ptr);

/*---------------------------------------------------------------------------*/
/* File : Breakpnt.c                                                         */
/*---------------------------------------------------------------------------*/
void   SaveBreakpoints( void);
void   InitBreakpoints( void );
void   ResetBreakpointFileTime(void);
void   RestoreBreakpoints( void );
void   EditBreakpoints( void );
void   Strip(char *, char, char, char);
APIRET StripComments( char *, char * );
char  *FindEndOfComment( char *, char * );
void   BlankOut( char *, char *);
void   BlankOutCRLFs( char *, char * );
int   *BuildBkptLinNumInfo( char *, char *, int * );
void   SetUsingBreakpointFileFlag(void);
void   ResetUsingBreakpointFileFlag(void);
void   KillEditorSession( void );
char  *GetSD386Brk( void );
void   FreeSD386Brk( void );
char  *GetSD386Editor( void );
void   FreeSD386Editor( void );
void   MergeBreakpoints(void);

/*---------------------------------------------------------------------------*/
/* File : showclas.c                                                         */
/*---------------------------------------------------------------------------*/
uint  ShowClass(DFILE *dfp, uint row, uint rows, uint skip);
UINT  ZapClass (DFILE *dfp, uint row, uint skip );
void  PutInTabStops( char *cp );
void  PullOutTabStops( char *cp );
char *NextTab( char *cp );
void  InitClassItem(UCHAR *cp, int bufsize);
BOOL  unmangle( UCHAR *pDeMangledName, UCHAR *pMangledName );
void  SetViewMemFncs( void );
BOOL  GetViewMemFncs( void );
UINT  GetLinesNoMemFncs( UINT mid, Trec *tp );

/*---------------------------------------------------------------------------*/
/* File : tcpip.c                                                            */
/*---------------------------------------------------------------------------*/
APIRET SockInit( CONNECTION *pConnection,
                 LHANDLE    *pLsnHandle,
                 int *p_MoreRcInfo );
int    SockSocket( void );
int    SockConnect( int SocketDescriptor, char *ip_name );
int    SockBind( int SocketDescriptor );
int    SockListen( int SocketDescriptor );
int    SockAccept( int SocketDescriptor );
void   SockSend( int SocketDescriptor, PVOID pBuf, ULONG Len );
void   SockRecv( int SocketDescriptor, PVOID pBuf, ULONG Len );
void   SockClose( int SocketDescriptor );
int    SockGetAnotherSocket( CONNECTION *pConnection );
int    SockGetDescriptor(void);
void   SockSetDescriptor(int);

/*===========================================================================*/
/*                                                                           */
/* Everything below is added for MSH support                                 */
/*                                                                           */
/*===========================================================================*/
#ifdef MSH

/*---------------------------------------------------------------------------*/
/* File : BROWSEM.C                                                          */
/*---------------------------------------------------------------------------*/
int   browsem(int, AFILE *, WINDOWEVENT *);

/*---------------------------------------------------------------------------*/
/* File : FINDD.C                                                            */
/*---------------------------------------------------------------------------*/
void   FindData(UINT);
uint   FindScalar(DFILE *,char *,MSHOBJECT *);
uint   FindConstant(DFILE *,char *,MSHOBJECT *);
uint   FindHexBytes(DFILE *,char *,MSHOBJECT *);

/*---------------------------------------------------------------------------*/
/* File : FINDD2.C                                                           */
/*---------------------------------------------------------------------------*/
uint    FindStruct(DFILE *,char *,MSHOBJECT *);
uint    FindArray(DFILE *,char *,MSHOBJECT *);

/*---------------------------------------------------------------------------*/
/* File : MSH.C                                                              */
/*---------------------------------------------------------------------------*/
UCHAR *MshAppData(UINT, UINT*, UINT, UINT*, UINT, MSHOBJECT *);                                /*101*/
int    MshSD386CommandProcessor(UINT, void *);
int    MshParseExpr(UCHAR *, UCHAR *, UINT, UINT, UINT, MSHOBJECT *);
void   MshMonitor(int);
int    SD386Log(int icode, char *string);
int   _System mshgetenv(char *);
void   mshsessioninit(void);
int   _Optlink writemshsd386(char *);

/* int   _System MshPutObject(MSHOBJECT *mshObject, int, int, ...); */
void   mshInit(void);
void   APIENTRY MSHExit(void);

char  *GetMSHHome( void );
void   FreeMSHHome( void );

/*---------------------------------------------------------------------------*/
/* File : cpydata.c                                                          */
/*---------------------------------------------------------------------------*/
int  mshsd386command(char *);

/*****************************************************************************/
/* MSHCALC entry points.                                                     */
/*****************************************************************************/
typedef int  (* _System MSHPUTOBJECT )(MSHOBJECT *mshObject, int, int, ...);
typedef int  (* _System MSHOBJECTSIZE)(MSHOBJECT *mshObject);
typedef int  (* _System MSHOBJECTDATASIZE)(MSHOBJECT *);
typedef int  (* _System MSHPUTSIMPLEOBJECT)(MSHOBJECT *, int, int, ...);
typedef void (* _System MSHINITOBJECT)(MSHOBJECT *) ;
typedef int  (* _System MSHGET_DIRECT)(char *, int *, int *, int *, char *);
typedef int  (* _System MSHPUT_DIRECT)(char *, int, int, int, char *);
typedef int  (* _System MSHXLDBCOMMAND)(char *, int, int );
typedef int  (* _System MSHXLDBINIT)(int,
                                     char **,
                                     char **,
                                     int (*importfns[])(void));
typedef int  (* _System MSHOS2INIT)(void);
typedef int  (* _System KILL)(int, int);
typedef Shmcom* SHMCOM;

/*****************************************************************************/
/* REXXMSH entry point.                                                      */
/*****************************************************************************/
typedef int (* _Optlink WRITE_MSH_REXX)(char *);
typedef int (* _System  MSH_LOAD_FUNCS)(CHAR *name, ULONG numargs, RXSTRING args[],
                           CHAR *queuename, RXSTRING *retstr);

/*****************************************************************************/
/* REXX entry point.                                                         */
/*****************************************************************************/
typedef LONG (* _System REXX_START)(LONG ,
                                    PRXSTRING,
                                    PSZ,
                                    PRXSTRING,
                                    PSZ,
                                    LONG ,
                                    PRXSYSEXIT,
                                    PSHORT,
                                    PRXSTRING );

/*****************************************************************************/
/* talloc.c                                                                  */
/*****************************************************************************/
void *T2alloc(UINT size, int chain);
char *MyMalloc(int, int);
char *MyFree(char *);
char *FreeChain(int);

/*---------------------------------------------------------------------------*/
/* File : Action.c  this function is added to support MSH                    */
/*---------------------------------------------------------------------------*/
WINDOWEVENT *BrowseMshFile(AFILE *fp);

/*---------------------------------------------------------------------------*/
/* cpydata.c defined functions.                                              */
/*---------------------------------------------------------------------------*/
 void   CopyDataItem(CHAR *,COPYDATA **,UINT,UINT,USHORT,UINT,UINT,MSHOBJECT*);
 void   CpyChar( COPYDATA **, ULONG , UINT, USHORT, UINT,MSHOBJECT*);
 void   CpyShort( COPYDATA **, ULONG , UINT, USHORT, UINT,MSHOBJECT*);
 void   CpyLong( COPYDATA **, ULONG , UINT, USHORT, UINT,MSHOBJECT*);
 void   CpyFloat( UINT, COPYDATA **, ULONG , UINT, USHORT, UINT,MSHOBJECT*);
 USHORT CpyPtr( COPYDATA **, ULONG*, UINT, USHORT, UINT,MSHOBJECT*);
 void   CpyBytes( COPYDATA **, ULONG , UINT , UINT ,MSHOBJECT*);
 void   CpyString( COPYDATA **, char *,ULONG, UINT ,MSHOBJECT*);
 USHORT CpyEnums( Trec *,COPYDATA **, ULONG , UINT , USHORT, UINT ,MSHOBJECT*);
 void   CpyBitField(Trec *, COPYDATA **, ULONG , UINT , USHORT ,UINT ,MSHOBJECT*);

/*---------------------------------------------------------------------------*/
/* File : HELP.C                                                             */
/*---------------------------------------------------------------------------*/
void   FmtErr(UCHAR *msg);

#endif
