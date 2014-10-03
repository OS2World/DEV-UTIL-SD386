/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   cuamenu.h                                                               */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Cua Action bar related data.                                            */
/*                                                                           */
/*****************************************************************************/
#define NUM_PULLDOWNS         9         /* on the menu bar                   */

#define PULLDOWN_FILE         0
#define PULLDOWN_RUN          1
#define PULLDOWN_BREAKPOINT   2
#define PULLDOWN_SEARCH       3
#define PULLDOWN_DATA         4
#define PULLDOWN_VIEW         5
#define PULLDOWN_SETTINGS     6
#define PULLDOWN_MISC         7
#define PULLDOWN_HELP         8
#define PULLDOWN_NULL                   /* Don't ask me!!                    */
#define PULLDOWN_RUN_CASCADE  10
#define PULLDOWN_OBJ_SOURCE   11
#define PULLDOWN_OBJ_FORMAT   12
#define PULLDOWN_OBJ_DATA     13

/*****************************************************************************/
/* File pulldown                                                             */
/*  the following line goes back in after Browse File                        */
/*     Browse MSH Log...\0\                                                  */
/*                                                                           */
/* note: you must redefine the bit mask of separators in the pulldown        */
/*       structure if you change the number of entries in the FILE           */
/*       pulldown.                                                           */
/*                                                                           */
/*****************************************************************************/
#define FILE_PULLDOWN_ENTRIES  13       /* including separators              */

#define PULLDOWN_FILE_GETFUNCTION   0
#define PULLDOWN_FILE_GETADDRESS    1
#define PULLDOWN_FILE_GETFILE       2
#define PULLDOWN_FILE_FINDFUNCTION  3
#define PULLDOWN_FILE_FINDEXECLINE  4
#define PULLDOWN_FILE_NULL          -
#define PULLDOWN_FILE_NEXTFILE      6
#define PULLDOWN_FILE_DROPFILE      7
#define PULLDOWN_FILE_NULL          -
#define PULLDOWN_FILE_BROWSE        9
#define PULLDOWN_FILE_NULL          -
#define PULLDOWN_FILE_RESTART      11
#define PULLDOWN_FILE_QUIT         12

static UCHAR Pull1_Names[] =
{
"Get Function...\0\
Get Address...\0\
Get File...\0\
Find Function\0\
Find Exec Line\0\
-\0\
Next File\0\
Drop File\0\
-\0\
Browse File...\0\
-\0\
Restart\0\
Quit\0"
};

/*static uchar Pull1_HotKeys[]   = "GETFX ND BM RQ"; */
static UCHAR Pull1_HotKeys[FILE_PULLDOWN_ENTRIES+1] = "GETFX ND B RQ";

static ULONG Pull1_Help[FILE_PULLDOWN_ENTRIES] =
{
  HELP_PULL_GETFUNC,
  HELP_PULL_GETADDR,
  HELP_PULL_GETFILE,
  HELP_PULL_FINDFUNC,
  HELP_PULL_FINDXLINE,
  NULL,
  HELP_PULL_NEXTFILE,
  HELP_PULL_DROPFILE,
  NULL,
  HELP_PULL_BROWSE,
/*HELP_PULL_BROWSEMSH, */
  NULL,
  HELP_PULL_RESTART,
  HELP_PULL_QUIT
};

/*static UCHAR Pull1_SelPos[]    = { 0,0x1,0x2,0,0x6,0,0,0,0,0,0x7,0,0,0};*/

static UCHAR Pull1_SelPos[FILE_PULLDOWN_ENTRIES] =
{
  0,     /*GETFUNCTION */
  0x1,   /*GETADDRESS  */
  0x2,   /*GETFILE     */
  0,     /*FINDFUNCTION*/
  0x6,   /*FINDEXECLINE*/
  0,     /*NULL        */
  0,     /*NEXTFILE    */
  0,     /*DROPFILE    */
  0,     /*NULL        */
  0,     /*BROWSE      */
  0,     /*NULL        */
  0,     /*RESTART     */
  0      /*QUIT        */
};

static UCHAR Pull1_FuncCodes[FILE_PULLDOWN_ENTRIES] =
{
  GETFUNCTION,
  GETADDRESS,
  GETFILE,
  FINDFUNCTION,
  FINDEXECLINE,
  NULL,
  NEXTFILE,
  DROPFILE,
  NULL,
  BROWSE,
/*BROWSEMSH, */
  NULL,
  RESTART,
  QUIT
};

/*static STRING Pull1_Accelkeys[14];*/
static STRING Pull1_Accelkeys[FILE_PULLDOWN_ENTRIES];

PULLDOWN Pull1 =
{ 1,                                    /* row to start pulldown             */
  0,                                    /* column to start pulldown          */
  23,                                   /* width of pulldown box             */
  FILE_PULLDOWN_ENTRIES,
  0,                                    /* bit mask of entries not available */
  Pull1_Names,                          /* list of labels concat together    */
  Pull1_HotKeys,                        /* list of single char selections    */
  Pull1_SelPos,                         /* Single char postions in selection */
  Pull1_Help,                           /* list of help screens              */
  Pull1_FuncCodes,                      /* function codes for each selection */
  &Pull1_Accelkeys[0],                  /* Names of Accel Keys.              */
  0x0520                                /* bit mask of separators in pulldown*/
};                                      /* Pointer to cascaded pulldown.     */
                                        /* -> screen save area.              */

/*****************************************************************************/
/* Run pulldown                                                              */
/*****************************************************************************/
#define RUN_PULLDOWN_ENTRIES  11       /* including separators              */

#define PULLDOWN_RUN_RUN                   0
#define PULLDOWN_RUN_SSTEP                 1
#define PULLDOWN_RUN_SSTEPINTOFUNC         2
#define PULLDOWN_RUN_RUNTOCURSOR           3
#define PULLDOWN_RUN_NULL                  -
#define PULLDOWN_RUN_RUNNOSWAP             5
#define PULLDOWN_RUN_SSTEPNOSWAP           6
#define PULLDOWN_RUN_SSTEPINTOFUNCNOSWAP   7
#define PULLDOWN_RUN_RUNTOCURSORNOSWAP     8
#define PULLDOWN_RUN_NULL                  -
#define PULLDOWN_RUN_SETEXECLINE          10

/*****************************************************************************/
/* Run pulldown cascade pulldown structures.                                 */
/*---------------------------------------------------------------------------*/
static UCHAR Pull21_Names[] =
{
"Swap\0\
No Swap\0"
};

static UCHAR Pull21_HotKeys[] = "SN";

static ULONG Pull21_Help[] =
{
 HELP_PULL_SWAP,
 HELP_PULL_NOSWAP
};

static UCHAR Pull21_SelPos[] = {0,0};

static STRING Pull21_Accelkeys[2];

PULLDOWN Pull21 =
{
 4,
 26,
 20,
 2,
 0,
 Pull21_Names,
 Pull21_HotKeys,
 Pull21_SelPos,
 Pull21_Help,
 NULL,
 &Pull21_Accelkeys[0]
};

CAS_PULLDOWN RunOptions[RUN_PULLDOWN_ENTRIES] =
{
 {PULLDOWN_RUN_CASCADE, 0x0001},
 {PULLDOWN_RUN_CASCADE, 0x0001},
 {PULLDOWN_RUN_CASCADE, 0x0001},
 {PULLDOWN_RUN_CASCADE, 0x0001},
 {0,0},
 {0,0},
 {0,0},
 {0,0},
 {0,0},
 {0,0},
 {0,0}
};
/*---------------------------------------------------------------------------*/

/*****************************************************************************/
/* Now, back to the Run pulldown itself.                                     */
/*****************************************************************************/
static UCHAR Pull2_Names[] =
{
"Run\0\
Single Step\0\
Step Into\0\
Run To Cursor\0\
-\0\
Run              NoSwap\0\
Single Step      NoSwap\0\
Step Into        NoSwap\0\
Run To Cursor    NoSwap\0\
-\0\
Set Exec Line\0"
};

static UCHAR Pull2_HotKeys[RUN_PULLDOWN_ENTRIES+1] = "RSIC UNEO X";

static ULONG Pull2_Help[RUN_PULLDOWN_ENTRIES] =
{
 HELP_PULL_RUN,
 HELP_PULL_SSTEP,
 HELP_PULL_STEPINTO,
 HELP_PULL_RUNCURSOR,
 NULL,
 HELP_PULL_RUNNOSWP,
 HELP_PULL_SSTEPNOSWP,
 HELP_PULL_STEPINTONOSWP,
 HELP_PULL_RUNCURSORNOSWP,
 NULL,
 HELP_PULL_SETXLINE
};

static UCHAR Pull2_SelPos[RUN_PULLDOWN_ENTRIES] =
{
 0,          /*RUN                  */
 0,          /*SSTEP                */
 0x5,        /*SSTEPINTOFUNC        */
 0x7,        /*RUNTOCURSOR          */
 0,          /*NULL                 */
 0x1,        /*RUNNOSWAP            */
 0x2,        /*SSTEPNOSWAP          */
 0x2,        /*SSTEPINTOFUNCNOSWAP  */
 0x5,        /*RUNTOCURSORNOSWAP    */
 0,          /*NULL                 */
 0x5         /*SETEXECLINE          */
};

static UCHAR Pull2_FuncCodes[RUN_PULLDOWN_ENTRIES] =
{
 RUN,
 SSTEP,
 SSTEPINTOFUNC,
 RUNTOCURSOR,
 NULL,
 RUNNOSWAP,
 SSTEPNOSWAP,
 SSTEPINTOFUNCNOSWAP,
 RUNTOCURSORNOSWAP,
 NULL,
 SETEXECLINE
};

static STRING Pull2_Accelkeys[RUN_PULLDOWN_ENTRIES];

PULLDOWN Pull2 =
{
 1,
 6,
 34,
 RUN_PULLDOWN_ENTRIES,
 0,
 Pull2_Names,
 Pull2_HotKeys,
 Pull2_SelPos,
 Pull2_Help,
 Pull2_FuncCodes,
 &Pull2_Accelkeys[0],
 0x0210,
 &RunOptions[0]
};

/*****************************************************************************/
/* Breakpoint pulldown.                                                      */
/*****************************************************************************/
#define BREAKPOINT_PULLDOWN_ENTRIES 14  /* including separators              */

#define PULLDOWN_BREAKPOINT_SETCLRBKP       0
#define PULLDOWN_BREAKPOINT_SETCONDBKP      1
#define PULLDOWN_BREAKPOINT_FUNCENTRY       2
#define PULLDOWN_BREAKPOINT_ADDRESS         3
#define PULLDOWN_BREAKPOINT_DEFERRED        4
#define PULLDOWN_BREAKPOINT_ADDRLOAD        5
#define PULLDOWN_BREAKPOINT_DLLLOAD         6
#define PULLDOWN_BREAKPOINT_CLRBKPS         7
#define PULLDOWN_BREAKPOINT_NULL            -
#define PULLDOWN_BREAKPOINT_SAVEBKPS        9
#define PULLDOWN_BREAKPOINT_RESTOREBKPS    10
#define PULLDOWN_BREAKPOINT_EDITBKPS       11
#define PULLDOWN_BREAKPOINT_NULL            -
#define PULLDOWN_BREAKPOINT_HARDWAREREGS   13

static UCHAR Pull3_Names[] =
{
"Set/Clear on Cursor\0\
Set Conditional on Cursor...\0\
Set Function Name Break...\0\
Set Address Break...\0\
Set Deferred Break...\0\
Set Address Load Break...\0\
Set DLL Load Break...\0\
Clear All Breaks\0\
-\0\
Save Breakpoint File\0\
Restore Breakpoint File\0\
Edit Breakpoint File\0\
-\0\
Hardware Debug Regs...\0"
};

static UCHAR Pull3_HotKeys[BREAKPOINT_PULLDOWN_ENTRIES+1] = "SEFATLOC BRD H";

static ULONG Pull3_Help[BREAKPOINT_PULLDOWN_ENTRIES] =
{
 HELP_PULL_SETCLRBKP,
 HELP_PULL_SETCONDBKP,
 HELP_PULL_FUNCENTRY,
 HELP_PULL_ADDRESS,
 HELP_PULL_DEFERRED,
 HELP_PULL_ADDRLOAD,
 HELP_PULL_DLLLOAD,
 HELP_PULL_CLRBKPS,
 NULL,
 HELP_PULL_SAVEBKPS,
 HELP_PULL_RESTOREBKPS,
 HELP_PULL_EDITBKPS,
 NULL,
 HELP_PULL_HARDWAREREGS
};

static UCHAR Pull3_SelPos[BREAKPOINT_PULLDOWN_ENTRIES] =
{
 0,     /*SETCLEARBKPT         */
 1,     /*SETCLEARCONDBKPT     */
 4,     /*SETFUNCTIONBKPT      */
 4,     /*SETADDRESSBKPT       */
 2,     /*SETDEFERREDBKPT      */
 12,    /*SETADDRLOADBKPT      */
 9,     /*SETDLLLOADBKPT       */
 0,     /*CLEARALLBKPTS        */
 0,     /*NULL                 */
 5,     /*SAVEBKPTS            */
 0,     /*RESTOREBKPTS         */
 1,     /*EDITBKPTS            */
 0,     /*NULL                 */
 0      /*SETWATCHPOINTS       */
};

static UCHAR Pull3_FuncCodes[BREAKPOINT_PULLDOWN_ENTRIES] =
{
 SETCLEARBKPT,
 SETCLEARCONDBKPT,
 SETFUNCTIONBKPT,
 SETADDRESSBKPT,
 SETDEFERREDBKPT,
 SETADDRLOADBKPT,
 SETDLLLOADBKPT,
 CLEARALLBKPTS,
 NULL,
 SAVEBKPTS,
 RESTOREBKPTS,
 EDITBKPTS,
 NULL,
 SETWATCHPOINTS
};

static STRING Pull3_Accelkeys[BREAKPOINT_PULLDOWN_ENTRIES];

PULLDOWN Pull3 =
{
 1,                                     /* row to start pulldown             */
 11,                                    /* column to start pulldown          */
 37,                                    /* width of pulldown box             */
 BREAKPOINT_PULLDOWN_ENTRIES,           /* # of entries in pulldown          */
 0,                                     /* bit mask of entries not available */
 Pull3_Names,                           /* list of labels concat together    */
 Pull3_HotKeys,                         /* list of single char selections    */
 Pull3_SelPos,                          /* Single char postions in selection */
 Pull3_Help,                            /* list of help screens              */
 Pull3_FuncCodes,                       /* function codes for each selection */
 &Pull3_Accelkeys[0],                   /* Names of Accel Keys.              */
 0x1100                                 /* bit mask of separators in pulldown*/
};                                      /* Pointer to cascaded pulldown.     */
                                        /* -> screen save area.              */

/*****************************************************************************/
/* Search pulldown                                                           */
/*****************************************************************************/
#define SEARCH_PULLDOWN_ENTRIES 2       /* including separators              */

#define PULLDOWN_SEARCH_FIND            0
#define PULLDOWN_SEARCH_REPEATFIND      1

static UCHAR Pull4_Names[] =
{
"Find...\0\
RepeatFind\0"
};

static UCHAR Pull4_HotKeys[SEARCH_PULLDOWN_ENTRIES+1] = "FR";

static ULONG Pull4_Help[SEARCH_PULLDOWN_ENTRIES] =
{
 HELP_PULL_FIND,
 HELP_PULL_REPEATFIND
};

static UCHAR Pull4_SelPos[SEARCH_PULLDOWN_ENTRIES] = {0,0};

static UCHAR Pull4_FuncCodes[SEARCH_PULLDOWN_ENTRIES] =
{
 FIND,
 REPEATFIND
};

static STRING Pull4_Accelkeys[SEARCH_PULLDOWN_ENTRIES];

PULLDOWN Pull4 =
{
 1,                                     /* row to start pulldown             */
 24,                                    /* column to start pulldown          */
 20,                                    /* width of pulldown box             */
 SEARCH_PULLDOWN_ENTRIES,               /* # of entries in pulldown          */
 0,                                     /* bit mask of entries not available */
 Pull4_Names,                           /* list of labels concat together    */
 Pull4_HotKeys,                         /* list of single char selections    */
 Pull4_SelPos,                          /* Single char postions in selection */
 Pull4_Help,                            /* list of help screens              */
 Pull4_FuncCodes,                       /* function codes for each selection */
 &Pull4_Accelkeys[0]                    /* Names of Accel Keys.              */
};                                      /* bit mask of separators in pulldown*/
                                        /* Pointer to cascaded pulldown.     */
                                        /* -> screen save area.              */

/*****************************************************************************/
/* Data pulldown                                                             */
/*MshGet Var\0\    these go back in after expand var                         */
/*MshPut Var\0\                                                              */
/*****************************************************************************/
#define DATA_PULLDOWN_ENTRIES 11

#define PULLDOWN_DATA_SHOWVAR         0
#define PULLDOWN_DATA_SHOWVARPTSTO    1
#define PULLDOWN_DATA_WATCHVAR        2
#define PULLDOWN_DATA_WATCHVARPTSTO   3
#define PULLDOWN_DATA_EXPANDVAR       4
#define PULLDOWN_DATA_NULL            -
#define PULLDOWN_DATA_EDITEXPRESSION  6
#define PULLDOWN_DATA_EDITSTORAGE     7
#define PULLDOWN_DATA_FORMATVAR       8
#define PULLDOWN_DATA_REMOVEVAR       9
#define PULLDOWN_DATA_INSERT         10

static UCHAR Pull5_Names[] =
{
"Show Var\0\
Show Var->\0\
Watch Var\0\
Watch Var->\0\
Expand Var\0\
-\0\
Edit Expression\0\
Edit Storage\0\
Format Var\0\
Remove Var\0\
Insert\0"
};

/* static UCHAR Pull5_HotKeys[]   = "SVWAEGP XDORI"; */
static UCHAR Pull5_HotKeys[DATA_PULLDOWN_ENTRIES+1]   = "SVWAE XDORI";

static ULONG Pull5_Help[DATA_PULLDOWN_ENTRIES] =
{
  HELP_PULL_SHOWVAR,
  HELP_PULL_SHOWVARPTR,
  HELP_PULL_WATCHVAR,
  HELP_PULL_WATCHVARPTR,
  HELP_PULL_EXPANDVAR,
/*HELP_PULL_MSHGET,  */
/*HELP_PULL_MSHPUT,  */
  NULL,
  HELP_PULL_EDITEXP,
  HELP_PULL_EDITSTORAGE,
  HELP_PULL_FORMATVAR,
  HELP_PULL_REMOVEVAR,
  HELP_PULL_INSERT
};

static UCHAR Pull5_SelPos[DATA_PULLDOWN_ENTRIES] =
{
  0,      /*SHOWVAR,              */
  0x5,    /*SHOWVARPTSTO,         */
  0,      /*PUTVARINSTG,          */
  0x1,    /*PUTVARPTSTOINSTG,     */
  0,      /*EXPANDVAR,            */
/*0x3,*/  /*MSHGET,               */
/*0x3,*/  /*MSHPUT,               */
  0,      /*NULL,                 */
  0x6,    /*EDITFORMULA,          */
  0x1,    /*TABRIGHT,             */
  0x1,    /*FORMATVAR,            */
  0,      /*DELETELINE,           */
  0       /*INSERTLINE            */
};

static UCHAR Pull5_FuncCodes[DATA_PULLDOWN_ENTRIES] =
{
  SHOWVAR,
  SHOWVARPTSTO,
  PUTVARINSTG,
  PUTVARPTSTOINSTG,
  EXPANDVAR,
/*MSHGET,   */
/*MSHPUT,   */
  NULL,
  EDITFORMULA,
  TABRIGHT,
  FORMATVAR,
  DELETELINE,
  INSERTLINE
};

static STRING Pull5_Accelkeys[DATA_PULLDOWN_ENTRIES];

PULLDOWN Pull5 = { 1,                   /* row to start pulldown             */
                   32,                  /* column to start pulldown          */
                   27,                  /* width of pulldown box             */
                   DATA_PULLDOWN_ENTRIES, /* # of entries in pulldown        */
                   0,                   /* bit mask of entries not available */
                   Pull5_Names,         /* list of labels concat together    */
                   Pull5_HotKeys,       /* list of single char selections    */
                   Pull5_SelPos,        /* Single char postions in selection */
                   Pull5_Help,          /* list of help screens              */
                   Pull5_FuncCodes,     /* function codes for each selection */
                   &Pull5_Accelkeys[0], /* Names of Accel Keys.              */
                   0x0020 };            /* bit mask of separators in pulldown*/
                                        /* Pointer to cascaded pulldown.     */
                                        /* -> screen save area.              */

/*****************************************************************************/
/* View pulldown.                                                            */
/*****************************************************************************/
#define VIEW_PULLDOWN_ENTRIES 5

#define PULLDOWN_VIEW_SOURCE          0
#define PULLDOWN_VIEW_ASSEMBLER       1
#define PULLDOWN_VIEW_DATA            2
#define PULLDOWN_VIEW_DATASHOWHIDE    3
#define PULLDOWN_VIEW_TIMEDSHOWAPP    4

static UCHAR Pull6_Names[] =
{
"Source\0\
Assembler\0\
Data\0\
Data Show/Hide\0\
Timed Show App\0"
};

static UCHAR Pull6_HotKeys[VIEW_PULLDOWN_ENTRIES+1] = "SADHT";

static ULONG Pull6_Help[VIEW_PULLDOWN_ENTRIES] =
{
 HELP_PULL_SOURCE,
 HELP_PULL_ASSEMBLER,
 HELP_PULL_DATA,
 HELP_PULL_DATASHOW,
 HELP_PULL_SHOWAPP
};

static UCHAR Pull6_SelPos[VIEW_PULLDOWN_ENTRIES] =
{
 0,   /*SOURCE      */
 0,   /*ASSEMBLER   */
 0,   /*DATA        */
 0xA, /*DATASHOWHIDE*/
 0    /*SHOWAPP     */
};

static UCHAR Pull6_FuncCodes[VIEW_PULLDOWN_ENTRIES] =
{
 TOGGLEASM,
 TOGGLEASM,
 TOGGLESTORAGE,
 SHOWHIDESTORAGE,
 TIMEDSHOWAPPWINDOW
};

static STRING Pull6_Accelkeys[VIEW_PULLDOWN_ENTRIES];

PULLDOWN Pull6 =
{
 1,
 38,
 23,
 VIEW_PULLDOWN_ENTRIES,
 0,
 Pull6_Names,
 Pull6_HotKeys,
 Pull6_SelPos,
 Pull6_Help,
 Pull6_FuncCodes,
 &Pull6_Accelkeys[0]
};

/*****************************************************************************/
/* Settings pulldown.                                                        */
/*****************************************************************************/
#define SETTINGS_PULLDOWN_ENTRIES 5     /* including separators              */

#define PULLDOWN_SETTINGS_COLORS          0
#define PULLDOWN_SETTINGS_EXCEPTIONS      1
#define PULLDOWN_SETTINGS_ASSEMBLYSOURCE  2
#define PULLDOWN_SETTINGS_MNEMONICS       3
#define PULLDOWN_SETTINGS_TOGGLEMEMFNCS   4

static UCHAR Pull7_Names[] =
{
"Colors\0\
Exceptions\0\
Assembler ñ Source\0\
Mnemonics\0\
ShowMemberFuncs\0"
};

static UCHAR Pull7_HotKeys[SETTINGS_PULLDOWN_ENTRIES+1] = "CEAMS";

static ULONG Pull7_Help[SETTINGS_PULLDOWN_ENTRIES] =
{
 HELP_PULL_COLORS,
 HELP_PULL_EXCEPTIONS,
 HELP_PULL_ASSEMBLYSOURCE,
 HELP_PULL_MNEMONICS,
 HELP_PULL_TOGGLEMEMFNCS
};

static UCHAR Pull7_SelPos[SETTINGS_PULLDOWN_ENTRIES] =
{
 0,   /*COLORS         */
 0,   /*EXCEPTIONS     */
 0,   /*ASSEMBLYSOURCE */
 0,   /*MNEMONICS      */
 0    /*MNEMONICS      */
};

static UCHAR Pull7_FuncCodes[SETTINGS_PULLDOWN_ENTRIES] =
{
 SETCOLORS,
 SETEXCEPTIONS,
 TOGGLEASMVIEW,
 TOGGLEDIS,
 TOGGLEMEMFNCS
};

static STRING Pull7_Accelkeys[SETTINGS_PULLDOWN_ENTRIES];

PULLDOWN Pull7 =
{
  1,
  44,
  30,
  SETTINGS_PULLDOWN_ENTRIES,
  0,
  Pull7_Names,
  Pull7_HotKeys,
  Pull7_SelPos,
  Pull7_Help,
  Pull7_FuncCodes,
  &Pull7_Accelkeys[0]
};

/*****************************************************************************/
/* Misc pulldown                                                             */
/*****************************************************************************/
#define MISC_PULLDOWN_ENTRIES 7         /* including separators              */

#define PULLDOWN_MISC_CALLSTACK       0
#define PULLDOWN_MISC_PROCESSES       1
#define PULLDOWN_MISC_THREADS         2
#define PULLDOWN_MISC_DLLS            3
#define PULLDOWN_MISC_REGS            4
#define PULLDOWN_MISC_EDITREGS        5
#define PULLDOWN_MISC_COPROCREGS      6

static UCHAR Pull8_Names[] =
{
"CallStack\0\
Processes\0\
Threads\0\
Dlls\0\
Registers\0\
Edit Registers\0\
Coprocessor Regs\0"
};

static UCHAR Pull8_HotKeys[MISC_PULLDOWN_ENTRIES+1] = "CPTDREO";

static ULONG Pull8_Help[MISC_PULLDOWN_ENTRIES] =
{
 HELP_PULL_CALLSTACK,
 HELP_PULL_PROCESSES,
 HELP_PULL_THREADS,
 HELP_PULL_DLLS,
 HELP_PULL_REGS,
 HELP_PULL_EDITREGS,
 HELP_PULL_COPROCREGS
};

static UCHAR Pull8_SelPos[MISC_PULLDOWN_ENTRIES] =
{
 0,    /*CALLSTACK  */
 0,    /*PROCESSES  */
 0,    /*THREADS    */
 0,    /*DLLS       */
 0,    /*REGS       */
 0,    /*EDITREGS   */
 1     /*COPROCREGS */
};

static UCHAR Pull8_FuncCodes[MISC_PULLDOWN_ENTRIES] =
{
 SHOWCALLSTACK,
 SHOWPROCESSES,
 SHOWTHREADS,
 SHOWDLLS,
 REGISTERDISPLAY,
 EDITREGISTERS,
 COREGISTERDISPLAY
};

static STRING Pull8_Accelkeys[MISC_PULLDOWN_ENTRIES];

PULLDOWN Pull8 =
{
 1,
 53,
 26,
 MISC_PULLDOWN_ENTRIES,
 0,
 Pull8_Names,
 Pull8_HotKeys,
 Pull8_SelPos,
 Pull8_Help,
 Pull8_FuncCodes,
 &Pull8_Accelkeys[0]
};

/*****************************************************************************/
/* Help pulldown                                                             */
/*****************************************************************************/
#define HELP_PULLDOWN_ENTRIES 2         /* including separators              */

#define PULLDOWN_HELP_GENHELP  0
#define PULLDOWN_HELP_KEYS     1

static UCHAR Pull9_Names[] =
{
"General Help\0\
Keys Help\0"
};

static UCHAR Pull9_HotKeys[HELP_PULLDOWN_ENTRIES+1] = "GKM";

static ULONG Pull9_Help[HELP_PULLDOWN_ENTRIES] =
{
 HELP_PULL_GENHELP,
 HELP_PULL_KEYS
};

static UCHAR Pull9_SelPos[HELP_PULLDOWN_ENTRIES] =
{
 0,  /* GENHELP */
 0   /* KEYS    */
};

static UCHAR Pull9_FuncCodes[HELP_PULLDOWN_ENTRIES] =
{
 GENHELP,
 FUNCKEYSHELP
};

static STRING Pull9_Accelkeys[HELP_PULLDOWN_ENTRIES];

PULLDOWN Pull9 =
{
 1,
 57,
 21,
 HELP_PULLDOWN_ENTRIES,
 0,
 Pull9_Names,
 Pull9_HotKeys,
 Pull9_SelPos,
 Pull9_Help,
 Pull9_FuncCodes,
 &Pull9_Accelkeys[0]
};

/*****************************************************************************/
/* Data for Action Bar.                                                      */
/*****************************************************************************/
static UCHAR MenuNames[] =
{
"File\0\
Run\0\
Breakpoints\0\
Search\0\
Data\0\
View\0\
Settings\0\
Misc\0\
Help\0"
};

static UCHAR  Menukeys[] = "frbsdvemh";

static uint   MenuCols[] = {1,1,1,1,1,1,2,1,1};

static UCHAR  Menuhids[] = {
                             HELP_ABAR_FILE,
                             HELP_ABAR_RUN,
                             HELP_ABAR_BREAKPOINTS,
                             HELP_ABAR_SEARCH,
                             HELP_ABAR_DATA,
                             HELP_ABAR_VIEW,
                             HELP_ABAR_SETTINGS,
                             HELP_ABAR_MISC,
                             HELP_ABAR_HELP
                           };

/*****************************************************************************/
/* Object pulldown in Source/Data views.                                     */
/*****************************************************************************/
#define PULLDOWN_OBJ_SOURCE_ENTRIES 11  /* including separators              */

#define PULLDOWN_OBJ_SOURCE_SETCLRBKP    0
#define PULLDOWN_OBJ_SOURCE_RUN          1
#define PULLDOWN_OBJ_SOURCE_SSTEP        2
#define PULLDOWN_OBJ_SOURCE_STEPINTO     3
#define PULLDOWN_OBJ_SOURCE_RUNCURSOR    4
#define PULLDOWN_OBJ_SOURCE_NULL         -
#define PULLDOWN_OBJ_SOURCE_SHOWVAR      6
#define PULLDOWN_OBJ_SOURCE_SHOWVARPTR   7
#define PULLDOWN_OBJ_SOURCE_WATCHVAR     8
#define PULLDOWN_OBJ_SOURCE_WATCHVARPTR  9
#define PULLDOWN_OBJ_SOURCE_EXPANDVAR   10

static UCHAR PullObj1_Names[] =
{
"Set/Clear Break\0\
Run\0\
Single Step\0\
Step Into\0\
Run to Cursor\0\
-\0\
Show Var\0\
Show Var->\0\
Watch Var\0\
Watch Var->\0\
Expand Var\0"
};

static UCHAR PullObj1_HotKeys[PULLDOWN_OBJ_SOURCE_ENTRIES+1] = "BRSIC VAWTE";

static ULONG PullObj1_Help[PULLDOWN_OBJ_SOURCE_ENTRIES] =
{
 HELP_PULL_SETCLRBKP,
 HELP_PULL_RUN,
 HELP_PULL_SSTEP,
 HELP_PULL_STEPINTO,
 HELP_PULL_RUNCURSOR,
 NULL,
 HELP_PULL_SHOWVAR,
 HELP_PULL_SHOWVARPTR,
 HELP_PULL_WATCHVAR,
 HELP_PULL_WATCHVARPTR,
 HELP_PULL_EXPANDVAR
};

static UCHAR PullObj1_SelPos[PULLDOWN_OBJ_SOURCE_ENTRIES] =
{
 0xA,    /*SETCLEARBKPT          */
 0,      /*RUN                   */
 0,      /*SSTEP                 */
 0x5,    /*SSTEPINTOFUNC         */
 0x7,    /*RUNTOCURSOR           */
 0,      /*NULL                  */
 0x5,    /*SHOWVAR               */
 0x6,    /*SHOWVARPTSTO          */
 0,      /*PUTVARINSTG           */
 0x2,    /*PUTVARPTSTOINSTG      */
 0       /*EXPANDVAR             */
};

static UCHAR PullObj1_FuncCodes[PULLDOWN_OBJ_SOURCE_ENTRIES] =
{
 SETCLEARBKPT,
 RUN,
 SSTEP,
 SSTEPINTOFUNC,
 RUNTOCURSOR,
 NULL,
 SHOWVAR,
 SHOWVARPTSTO,
 PUTVARINSTG,
 PUTVARPTSTOINSTG,
 EXPANDVAR
};

PULLDOWN PullObj1 =
{
 1,
 0,
 23,
 PULLDOWN_OBJ_SOURCE_ENTRIES,
 0,
 PullObj1_Names,
 PullObj1_HotKeys,
 PullObj1_SelPos,
 PullObj1_Help,
 PullObj1_FuncCodes,
 NULL,
 0x0020
};

/*****************************************************************************/
/* Variable format pulldown used in Data view.                               */
/*****************************************************************************/
#define PULLDOWN_OBJ_FORMAT_ENTRIES 12  /* including separators              */

#define PULLDOWN_OBJ_FORMAT_ANY          0
#define PULLDOWN_OBJ_FORMAT_HEX          1
#define PULLDOWN_OBJ_FORMAT_CHAR         2
#define PULLDOWN_OBJ_FORMAT_INT          3
#define PULLDOWN_OBJ_FORMAT_LONG         4
#define PULLDOWN_OBJ_FORMAT_UCHAR        5
#define PULLDOWN_OBJ_FORMAT_UINT         6
#define PULLDOWN_OBJ_FORMAT_ULONG        7
#define PULLDOWN_OBJ_FORMAT_FLOAT        8
#define PULLDOWN_OBJ_FORMAT_DOUBLE       9
#define PULLDOWN_OBJ_FORMAT_LDOUBLE     10
#define PULLDOWN_OBJ_FORMAT_MENU        11

static UCHAR PullObj2_Names[] =
{
"?\0\
Hex\0\
Char\0\
Short\0\
Long\0\
Uchar\0\
Ushort\0\
Ulong\0\
Float\0\
Double\0\
Ldouble\0\
\0________________________________\0\\0"
};

static UCHAR PullObj2_HotKeys[PULLDOWN_OBJ_FORMAT_ENTRIES+1] = "?HCSLUONFDB.";

static ULONG PullObj2_Help[PULLDOWN_OBJ_FORMAT_ENTRIES] =
{
 HELP_PULL_ANY,
 HELP_PULL_HEX,
 HELP_PULL_CHAR,
 HELP_PULL_INT,
 HELP_PULL_LONG,
 HELP_PULL_UCHAR,
 HELP_PULL_UINT,
 HELP_PULL_ULONG,
 HELP_PULL_FLOAT,
 HELP_PULL_DOUBLE,
 HELP_PULL_LDOUBLE,
 HELP_PULL_MENU
};

static UCHAR PullObj2_SelPos[PULLDOWN_OBJ_FORMAT_ENTRIES] =
{
 0,     /*ANY         */
 0,     /*HEX         */
 0,     /*CHAR        */
 0,     /*INT         */
 0,     /*LONG        */
 0,     /*UCHAR       */
 3,     /*UINT        */
 3,     /*ULONG       */
 0,     /*FLOAT       */
 0,     /*DOUBLE      */
 4      /*LDOUBLE     */
};      /*MENU        */

static UCHAR PullObj2_FuncCodes[PULLDOWN_OBJ_FORMAT_ENTRIES] =
{
 1,
 2,
 3,
 4,
 5,
 6,
 7,
 8,
 9,
 10,
 11,
 12
};

PULLDOWN PullObj2 =
{
 11,
 35,
 11,
 PULLDOWN_OBJ_FORMAT_ENTRIES,
 0,
 PullObj2_Names,
 PullObj2_HotKeys,
 PullObj2_SelPos,
 PullObj2_Help,
 PullObj2_FuncCodes,
 NULL,
 0x0000
};

/*****************************************************************************/
/* Date view object pulldown                                                 */
/*****************************************************************************/
#define PULLDOWN_OBJ_DATA_ENTRIES 6     /* including separators              */

#define PULLDOWN_OBJ_DATA_EXPANDVAR    0
#define PULLDOWN_OBJ_DATA_EDITEXP      1
#define PULLDOWN_OBJ_DATA_EDITSTORAGE  2
#define PULLDOWN_OBJ_DATA_FORMATVAR    3
#define PULLDOWN_OBJ_DATA_REMOVEVAR    4
#define PULLDOWN_OBJ_DATA_INSERT       5

static UCHAR PullObj3_Names[] =
{
"Expand Var\0\
Edit Expression\0\
Edit Storage\0\
Format Var\0\
Remove Var\0\
Insert\0"
};

static UCHAR PullObj3_HotKeys[PULLDOWN_OBJ_DATA_ENTRIES+1] = "XESFRI";

static ULONG PullObj3_Help[PULLDOWN_OBJ_DATA_ENTRIES] =
{
 HELP_PULL_EXPANDVAR,
 HELP_PULL_EDITEXP,
 HELP_PULL_EDITSTORAGE,
 HELP_PULL_FORMATVAR,
 HELP_PULL_REMOVEVAR,
 HELP_PULL_INSERT
};

static UCHAR PullObj3_SelPos[PULLDOWN_OBJ_DATA_ENTRIES] =
{
 0x1,   /*EXPANDVAR    */
 0,     /*EDITEXP      */
 0x5,   /*EDITSTORAGE  */
 0,     /*FORMATVAR    */
 0,     /*REMOVEVAR    */
 0      /*INSERT       */
};

static UCHAR PullObj3_FuncCodes[PULLDOWN_OBJ_DATA_ENTRIES] =
{
 EXPANDVAR,
 EDITFORMULA,
 TABRIGHT,
 FORMATVAR,
 DELETELINE,
 INSERTLINE
};

PULLDOWN PullObj3 =
{
 1,
 0,
 23,
 PULLDOWN_OBJ_DATA_ENTRIES,
 0,
 PullObj3_Names,
 PullObj3_HotKeys,
 PullObj3_SelPos,
 PullObj3_Help,
 PullObj3_FuncCodes,
 NULL,
 0
};

/*****************************************************************************/
/* Array of all pulldowns.                                                   */
/*****************************************************************************/
static PULLDOWN  *pullarray[] = {
                                  &Pull1,      /* PULLDOWN_FILE       */
                                  &Pull2,      /* PULLDOWN_RUN        */
                                  &Pull3,      /* PULLDOWN_BREAKPOINT */
                                  &Pull4,      /* PULLDOWN_SEARCH     */
                                  &Pull5,      /* PULLDOWN_DATA       */
                                  &Pull6,      /* PULLDOWN_VIEW       */
                                  &Pull7,      /* PULLDOWN_SETTINGS   */
                                  &Pull8,      /* PULLDOWN_MISC       */
                                  &Pull9,      /* PULLDOWN_HELP       */
                                  NULL,        /* ??                  */
                                  &Pull21,     /* PULLDOWN_RUN_CASCADE*/
                                  &PullObj1,   /* PULLDOWN_OBJ_SOURCE */
                                  &PullObj2,   /* PULLDOWN_OBJ_FORMAT */
                                  &PullObj3    /* PULLDOWN_OBJ_DATA   */
                                };

