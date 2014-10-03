/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   funccode.h                                                              */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Stuff for the user profile.                                             */
/*                                                                           */
/* History:                                                                  */
/*                                                                           */
/*   02/08/91 Creation of 32-bit SD386, from 16-bit version SD86             */
/*                                                                           */
/*****************************************************************************/

#define  UPCURSOR              1
#define  DOWNCURSOR            2
#define  RIGHTCURSOR           3
#define  LEFTCURSOR            4
#define  TABRIGHT              5
#define  TABLEFT               6
#define  PREVWINDOW            7
#define  NEXTWINDOW            8
#define  FIRSTWINDOW           9
#define  LASTWINDOW            10
#define  TOPOFWINDOW           11
#define  BOTOFWINDOW           12
#define  SHRINKSTORAGE         13
#define  EXPANDSTORAGE         14
#define  SSTEPNOSWAP           15
#define  SSTEPINTOFUNC         16
#define  RUN                   17
#define  RUNTOCURSOR           18
#define  HELP                  19
#define  FINDEXECLINE          20
#define  SHOWTHREADS           21
#define  TOGGLESTORAGE         22
#define  TOGGLEASM             23
#define  SHOWCALLSTACK         24
#define  FINDFUNCTION          25
#define  NEXTFILE              26
#define  SETCLEARCONDBKPT      27
#define  SETCLEARBKPT          28
#define  ACTIONBAR             29
#define  SHOWVAR               30
#define  SHOWVARPTSTO          31
#define  PUTVARINSTG           32
#define  PUTVARPTSTOINSTG      33
#define  DONOTHING             34
#define  TOGGLEDIS             35
#define  FORMATVAR             36
#define  EDITDATA              37
#define  INSERTLINE            38
#define  DELETELINE            39
#define  SHOWSEGS              40
#define  EXPANDVAR             41
#define  FIND                  42
#define  REPEATFIND            43
#define  BROWSE                44
#define  CLEARALLBKPTS         45
#define  EDITFORMULA           46
#define  TOGGLEASMLINE         47
#define  TOGGLEASMVIEW         48
#define  TOGGLEHIDESTORAGE     49
#define  SHOWHIDESTORAGE       50
#define  TIMEDSHOWAPPWINDOW    51
#define  SSTEP                 52
#define  SHOWDLLS              53
#define  SETDATABREAK          54
#define  SETCOLORS             55
#define  SETEXCEPTIONS         56
#define  REGISTERDISPLAY       57
#define  COREGISTERDISPLAY     58
#define  SETEXECLINE           59
#define  RUNNOSWAP             60
#define  SETWATCHPOINTS        61
#define  GETFUNCTION           62
#define  GETFILE               63
#define  DROPFILE              64
#define  RESTART               65
#define  QUIT                  66
#define  SETFUNCTIONBKPT       67
#define  SETADDRESSBKPT        68
#define  DISASMOPTIONS         69
#define  AL86MNEMONICS         70
#define  MASMMNEMONICS         71
#define  EDITREGISTERS         72
#define  SETCURSORPOS          73
#define  ESCAPE                74
#define  GETADDRESS            75
#define  SSTEPINTOFUNCNOSWAP   76
#define  RUNTOCURSORNOSWAP     77
#define  GENHELP               78
#define  FUNCKEYSHELP          79
#define  EXPRESSBAR            80
#define  RUNNOLOAD             81
#define  SHOWPROCESSES         82
#define  TOGGLEMEMFNCS         83

#define  MSHGET                90
#define  MSHPUT                91
#define  MSHCOMMAND            92
#define  MSHHELP               93
#define  SAVEBKPTS             94
#define  RESTOREBKPTS          95
#define  EDITBKPTS             96
#define  SETDEFERREDBKPT       97
#define  SETADDRLOADBKPT       98
#define  SETDLLLOADBKPT        99

#define  LEFTMOUSECLICK          200
#define  RIGHTMOUSECLICK         201
#define  ASMSSTEP                202
#define  ASMSSTEPNOSWAP          203
#define  ASMSSTEPINTOFUNC        204
#define  ASMSSTEPINTOFUNCNOSWAP  205

/*****************************************************************************/
/* Arrays defined for handling the user profile, SD86.PRO.                   */
/*  defk2f[]    - key code to function code map.                             */
/*  func2code[] - function text to function code map.                        */
/*  key2code[]  - key text to key code map.                                  */
/*****************************************************************************/
#define  KEYNUMSOC  92
#define  USERFUNCS  74
#define  USERKEYS   68

typedef struct {                        /* structure for scancode-to-        */
  uint scode;                           /* funccode table                    */
  int  fcode;
} KEY2FUNC;

typedef struct {                        /* structure for char-to-funccode    */
  char func[20];                        /* table                             */
  int  fcode;
} FUNCCODE;

typedef struct {                        /* structure for char-to-scancode    */
  char key[20];                         /* table                             */
  uint scode;
} KEYSCODE;

#define KEYCODE    0x4000000 /*Mask used in storing keys in the  logfile.*/
#define STRINGCODE 0x2000000 /*Mask used in storing strings in the  logfile.*/

#ifdef INCL_PROFILE_ARRAYS

        KEY2FUNC defk2f[KEYNUMSOC] = {
           UP,          UPCURSOR,
           C_UP,        SHRINKSTORAGE,
           DOWN,        DOWNCURSOR,
           C_DOWN,      EXPANDSTORAGE,
           LEFT,        LEFTCURSOR,
           C_LEFT,      DONOTHING,
           BACKSPACE,   LEFTCURSOR,
           RIGHT,       RIGHTCURSOR,
           C_RIGHT,     DONOTHING,
           TAB,         TABRIGHT,
           S_TAB,       TABLEFT,
           PGUP,        PREVWINDOW,
           C_PGUP,      TOPOFWINDOW,
           PGDN,        NEXTWINDOW,
           C_PGDN,      BOTOFWINDOW,
           HOME,        DONOTHING,
           C_HOME,      FIRSTWINDOW,
           END,         DONOTHING,
           C_END,       LASTWINDOW,
           SPACEBAR,    SSTEPNOSWAP,
           C_SPACEBAR,  DONOTHING,
           A_SPACEBAR,  DONOTHING,
           S_SPACEBAR,  DONOTHING,
           ENTER,       EXPANDVAR,
           C_ENTER,     FORMATVAR,
           A_ENTER,     EDITFORMULA,
           S_ENTER,     DONOTHING,
           INS,         SHOWVAR,
           C_INS,       PUTVARINSTG,
           S_INS,       EXPANDVAR,
           A_INS,       DONOTHING,
           DEL,         SHOWVARPTSTO,
           C_DEL,       PUTVARPTSTOINSTG,
           S_DEL,       DONOTHING,
           A_DEL,       DONOTHING,
           ESC,         EXPRESSBAR,
           PADPLUS,     INSERTLINE,
           PADMINUS,    DELETELINE,
           PADENTER,    EDITDATA,
           F1,          GENHELP,
           F2,          TOGGLESTORAGE,
           F3,          TOGGLEASM,
           F4,          TIMEDSHOWAPPWINDOW,
           F5,          RUN,
           F6,          FINDEXECLINE,
           F7,          RUNTOCURSOR,
           F8,          SSTEPNOSWAP,
           F9,          SETCLEARBKPT,
           F10,         ACTIONBAR,
           F11,         BROWSE,
           F12,         SETCOLORS,
           C_F1,        FUNCKEYSHELP,
           C_F2,        TOGGLEHIDESTORAGE,
           C_F3,        TOGGLEMEMFNCS,
           C_F4,        DONOTHING,
           C_F5,        FIND,
           C_F6,        REPEATFIND,
           C_F7,        DONOTHING,
           C_F8,        DONOTHING,
           C_F9,        CLEARALLBKPTS,
           C_F10 ,      COREGISTERDISPLAY,
           C_F11,       DONOTHING,
           C_F12,       SETEXCEPTIONS,
           A_F1,        SHOWCALLSTACK,
           A_F2,        SHOWTHREADS,
           A_F3,        TOGGLEDIS,
           A_F4,        NEXTFILE,
           A_F5,        DONOTHING,
           A_F6,        FINDFUNCTION,
           A_F7,        SETEXECLINE,
           A_F8,        SSTEPINTOFUNCNOSWAP,
           A_F9,        SETCLEARCONDBKPT,
           A_F10,       DONOTHING,
           A_F11 ,      DONOTHING,
           A_F12 ,      DONOTHING,
           S_F1 ,       SHOWDLLS,
           S_F2 ,       SHOWHIDESTORAGE,
           S_F3 ,       TOGGLEASMVIEW,
           S_F4 ,       TIMEDSHOWAPPWINDOW,
           S_F5 ,       RUNNOSWAP,
           S_F6 ,       DONOTHING,
           S_F7 ,       SETWATCHPOINTS,
           S_F8 ,       SSTEP,
           S_F9 ,       DONOTHING,
           S_F10 ,      REGISTERDISPLAY,
           S_F11 ,      DONOTHING,
           S_F12 ,      DONOTHING,
           SLASH ,      FIND,
           key_F,       FIND,
           key_R,       REPEATFIND,
           key_f,       FIND,
           key_r,       REPEATFIND
        };

        FUNCCODE func2code[USERFUNCS] = {                            /* 0*/
           "GetFunction",         GETFUNCTION,                       /* 1*/
           "GetAddress",          GETADDRESS,                        /* 2*/
           "GetFile",             GETFILE,                           /* 3*/
           "FindFunction",        FINDFUNCTION,                      /* 4*/
           "FindExecLine",        FINDEXECLINE,                      /* 5*/
           "NextFile",            NEXTFILE,                          /* 6*/
           "DropFile",            DROPFILE,                          /* 7*/
           "Browse",              BROWSE,                            /* 8*/
           "Restart",             RESTART,                           /* 9*/
           "Run",                 RUN,                               /*10*/
           "SStep",               SSTEP,                             /*11*/
           "SStepIntoFunc",       SSTEPINTOFUNC,                     /*12*/
           "RunToCursor",         RUNTOCURSOR,                       /*13*/
           "RunNoScreenSwap",     RUNNOSWAP,                         /*14*/
           "BgSStep",             SSTEPNOSWAP,                       /*15*/
           "BgSStepIntoFunc",     SSTEPINTOFUNCNOSWAP,               /*16*/
           "BgRunToCursor",       RUNTOCURSORNOSWAP,                 /*17*/
           "SetExecutionLine",    SETEXECLINE,                       /*18*/
           "SetClearBkPt",        SETCLEARBKPT,                      /*19*/
           "SetClearCondBkPt",    SETCLEARCONDBKPT,                  /*20*/
           "SetFuncBkPt",         SETFUNCTIONBKPT,                   /*21*/
           "SetAddrBkPt",         SETADDRESSBKPT,                    /*22*/
           "ClearAllBkpts",       CLEARALLBKPTS,                     /*23*/
           "SetWatchPoints",      SETWATCHPOINTS,                    /*24*/
           "Find",                FIND,                              /*25*/
           "RepeatFind",          REPEATFIND,                        /*26*/
           "ShowVar",             SHOWVAR,                           /*27*/
           "ShowVarPtsTo",        SHOWVARPTSTO,                      /*28*/
           "PutVarInStg",         PUTVARINSTG,                       /*29*/
           "PutVarPtsToInStg",    PUTVARPTSTOINSTG,                  /*30*/
           "ExpandVar",           EXPANDVAR,                         /*31*/
           "EditFormula",         EDITFORMULA,                       /*32*/
           "EditStorage",         TABRIGHT,                          /*33*/
           "FormatVar",           FORMATVAR,                         /*34*/
           "DeleteLine",          DELETELINE,                        /*35*/
           "InsertLine",          INSERTLINE,                        /*36*/
           "ToggleAsm",           TOGGLEASM,                         /*37*/
           "ToggleStorage",       TOGGLESTORAGE,                     /*38*/
           "ShowHideStorage",     SHOWHIDESTORAGE,                   /*39*/
           "TimedShowAppWindow",  TIMEDSHOWAPPWINDOW,                /*40*/
           "SetColors",           SETCOLORS,                         /*41*/
           "SetExceptions",       SETEXCEPTIONS,                     /*42*/
           "ToggleAsmView",       TOGGLEASMVIEW,                     /*43*/
           "ToggleDis",           TOGGLEDIS,                         /*44*/
           "ShowCallStack",       SHOWCALLSTACK,                     /*45*/
           "ShowThreads",         SHOWTHREADS,                       /*46*/
           "ShowDlls",            SHOWDLLS,                          /*47*/
           "RegisterDisplay",     REGISTERDISPLAY,                   /*48*/
           "EditRegisters",       EDITREGISTERS,                     /*49*/
           "CoProRegDisplay",     COREGISTERDISPLAY,                 /*50*/
           "ShrinkStorage",       SHRINKSTORAGE,                     /*51*/
           "ExpandStorage",       EXPANDSTORAGE,                     /*52*/
           "ActionBar",           ACTIONBAR,                         /*53*/
           "DoNothing",           DONOTHING,                         /*54*/
           "EditData",            EDITDATA,                          /*55*/
           "ShowSegs",            DONOTHING,                         /*56*/
           "ToggleAsmLine",       DONOTHING,                         /*57*/
           "ToggleHideStorage",   TOGGLEHIDESTORAGE,                 /*58*/
           "GenHelp",             GENHELP,                           /*59*/
           "KeysHelp",            FUNCKEYSHELP,                      /*60*/
           "ExpressBar",          EXPRESSBAR,                        /*61*/
           "ShowProcesses",       SHOWPROCESSES,                     /*62*/
           "MshGet",              MSHGET,                            /*63*/
           "MshPut",              MSHPUT,                            /*64*/
           "MshCommand",          MSHCOMMAND,                        /*65*/
           "MshHelp",             MSHHELP,                           /*66*/
           "SaveBkPts",           SAVEBKPTS,                         /*67*/
           "RestoreBkPts",        RESTOREBKPTS,                      /*68*/
           "EditBkPts",           EDITBKPTS,                         /*69*/
           "Quit",                QUIT,                              /*70*/
           "SetDeferredBkpt",     SETDEFERREDBKPT,                   /*71*/
           "SetAddressLoadBkpt",  SETADDRLOADBKPT,                   /*72*/
           "SetDllLoadBkpt",      SETDLLLOADBKPT                     /*73*/






        };

        KEYSCODE key2code[USERKEYS] = {
           "F1",                  F1,
           "F2",                  F2,
           "F3",                  F3,
           "F4",                  F4,
           "F5",                  F5,
           "F6",                  F6,
           "F7",                  F7,
           "F8",                  F8,
           "F9",                  F9,
           "F10",                 F10,
           "F11",                 F11,
           "F12",                 F12,
           "C_F1",                C_F1,
           "C_F2",                C_F2,
           "C_F3",                C_F3,
           "C_F4",                C_F4,
           "C_F5",                C_F5,
           "C_F6",                C_F6,
           "C_F7",                C_F7,
           "C_F8",                C_F8,
           "C_F9",                C_F9,
           "C_F10",               C_F10,
           "C_F11",               C_F11,
           "C_F12",               C_F12,
           "A_F1",                A_F1,
           "A_F2",                A_F2,
           "A_F3",                A_F3,
           "A_F4",                A_F4,
           "A_F5",                A_F5,
           "A_F6",                A_F6,
           "A_F7",                A_F7,
           "A_F8",                A_F8,
           "A_F9",                A_F9,
           "A_F10",               A_F10,
           "A_F11",               A_F11,
           "A_F12",               A_F12,
           "S_F1",                S_F1,
           "S_F2",                S_F2,
           "S_F3",                S_F3,
           "S_F4",                S_F4,
           "S_F5",                S_F5,
           "S_F6",                S_F6,
           "S_F7",                S_F7,
           "S_F8",                S_F8,
           "S_F9",                S_F9,
           "S_F10",               S_F10,
           "S_F11",               S_F11,
           "S_F12",               S_F12,
           "TAB",                 TAB,
           "SBAR",                SPACEBAR,
           "C_SBAR",              C_SPACEBAR,
           "A_SBAR",              A_SPACEBAR,
           "S_SBAR",              S_SPACEBAR,
           "ENTER",               ENTER,
           "C_ENT",               C_ENTER,
           "A_ENT",               A_ENTER,
           "S_ENT",               S_ENTER,
           "INS",                 INS,
           "C_INS",               C_INS,
           "S_INS",               S_INS,
           "A_INS",               A_INS,
           "DEL",                 DEL,
           "C_DEL",               C_DEL,
           "S_DEL",               S_DEL,
           "A_DEL",               A_DEL,
           "ESC",                 ESC,
           "PAD+",                PADPLUS,
           "PAD-",                PADMINUS,
        };

#endif
