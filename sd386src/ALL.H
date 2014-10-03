/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   all.h                                                                   */
/*                                                                           */
/* Description:                                                              */
/*   all include files for SD386                                             */
/*                                                                           */
/* History:                                                                  */
/*                                                                           */
/*   02/08/91 Creation of 32-bit SD86, from 16-bit version.                  */
/*                                                                           */
/*****************************************************************************/

#ifdef __DEBUG_ALLOC__
 #pragma strings(readonly)
#endif

#include <stdio.h>                      /* i/o support                       */
#include <stdlib.h>                     /* commonly used library functions   */
#include <setjmp.h>                     /* machine state restore routines    */
#include <string.h>                     /* non KBD, VIO, MOU subsystem calls */
#include <ctype.h>                      /*                                   */
#include <float.h>
#include <sys\stat.h>

/*****************************************************************************/
/* - includes for netbios support.                                           */
/*****************************************************************************/
#define E32TO16
#include <lan_7_c.h>
#include <netb_1_c.h>
#include <netb_2_c.h>
#include <netb_4_c.h>

#define INCL_SUB                        /* kbd, vio, mouse routines          */
#define INCL_DOSMODULEMGR                                               /*100*/
#define INCL_DOSMISC                                                    /*100*/
#define INCL_DOSSESMGR                                                  /*100*/
#define INCL_DOSPROCESS                                                 /*100*/
#define INCL_DOSERRORS                                                  /*101*/
#define INCL_DOSFILEMGR   /* File Manager values */
#define INCL_DOSEXCEPTIONS                                              /*213*/
#define INCL_DOSSEMAPHORES                                              /*701*/
#define INCL_DOSMEMMGR                                                  /*701*/
#define INCL_DOSDEVICES                                                 /*701*/
#define INCL_DOSQUEUES                                                  /*917*/
#define INCL_DOSNMPIPES                                                 /*919*/
#define INCL_REXXSAA                                                    /*919*/
#include <os2.h>
#include <bsedev.h>
#include <rexxsaa.h>

#define INCL_BASE                                                       /*813*/
#define INCL_MACROS                                                     /*813*/
#include "base.h"
#include "xcpt.h"                       /* exception codes                   */
#include "kbd.h"                        /* keyboard scan codes.              */
#include "sd386pro.h"                   /* startup profile stuff             */
#include "helpid.h"
#include "errorid.h"                                                    /*827*/
#include "panic.h"
#include "fmt.h"                        /* video related stuff               */
#include "msgdef.h"                     /* SD86 message definitions          */
#include "router.h"                                                     /*919*/
#include "esp.h"                        /*                                917*/
#include "sd386.h"
#include "$$symbol.h"                                                   /*809*/
#include "exe.h"                        /* EXE/DLL structures and formats.   */
#include "mte.h"                        /*                                822*/
#include "afile.h"
#include "brk.h"                        /* breakpoint definitions            */
#include "dfile.h"
#include "ptrace.h"
#include "boxmenu.h"
#include "$$types.h"
#include "iapx286.h"                    /* 80286 architecture definitions    */
#include "go.h"                         /* variables for GO.C interfacing    */
#include "disasm.h"                     /* Dave Toll's disassembler.         */
#include "threads.h"                    /* multithread support.              */
#include "setcolor.h"                   /* color support.                 101*/
#include "regwin.h"                     /* Regsiter windows stuff         401*/
#include "wps.h"                        /* watch point stuff              602*/
#include "mouse.h"                      /* mouse releated stuff           701*/
#include "dialog.h"                     /* Dialog #defines.               701*/
#include "pulldown.h"                   /* pull down definitions          701*/
#include "stack.h"                      /* pull down definitions          827*/
#include "txrx.h"
#include "com.h"                        /* async comm structures.            */
#include "keystr.h"                     /* definitions for GetString().   910*/
#include "qproc.h"                      /* DosQProcStatus() header.       919*/
#include "pipe.h"                                                       /*919*/
#include "dbgq.h"                                                       /*919*/
#include "netbios.h"                                                    /*919*/
#include "tcpip.h"
#include "pid.h"
#include "maptypes.h"

#ifdef MSH
 #include "msh.h"
 #include "ci.h"
 #include "window.h"
#endif

#define INCL_FUNCS
#define INCL_FMTDATA
#define INCL_FINDEXE
#define INCL_MTE
#define INCL_XBOX
#define INCL_BRK
#define INCL_GO
#define INCL_DEBUG
#define INCL_COM
#define INCL_TX
#include "funcs.h"

#define INCL_MAIN
#define INCL_XSRVMTE
#define INCL_XSRVBRK
#define INCL_XSRVGO
#define INCL_XSRVDIS
#define INCL_XSRVDBIF
#define INCL_XSRVFEXE
#define INCL_XSRVACVT
#define INCL_ESPSTART
#define INCL_XSRVXCPT
#define INCL_XSRVWPS
#define INCL_XSRVTHDS
#define INCL_XSRVUNWD
#define INCL_RX
#include "xfuncs.h"

/*****************************************************************************/
/* - a redefinition for DosDebug()                                           */
/*****************************************************************************/
#define DosDebug(x) Dbg(x)
/*#define Talloc(x)   calloc(1,x)*/


/*****************************************************************************/
/* - these defines are for compatibility with some prior betas.              */
/*****************************************************************************/
extern USHORT ColStart, RowStart;
#define VIEW_DONT_SHOW   2
#define VIEW_NORMAL      0
#define VIEW_SHOW        1
