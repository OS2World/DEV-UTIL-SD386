/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   Fmt.h                                                                   */
/*                                                                           */
/* Description:                                                              */
/*   Screen formatting related macros and constants.                         */
/*                                                                           */
/* History:                                                                  */
/*                                                                           */
/*   02/08/91 Creation of 32-bit SD86, from 16-bit version.                  */
/*                                                                           */
/*...Release 1.01 (04/03/92)                                                 */
/*...                                                                        */
/*... 05/08/92  701   Srinivas  Cua Interface.                               */
/*****************************************************************************/
#define MAXSRCCOLS   23          /* max # of cols of source code on ASM view */
#define ACTIVESHADE  0xB1        /* shading char for active threads          */
#define ACTCALLSHADE 0xB0        /* shading char for the active call chain   */
#define ACHAR_NULL   0xF9        /* ASCII code point for null chars          */
#define ACHAR_NONA   0xFA        /* ASCII code point for non-ASCII chars     */

/*****************************************************************************/
/*                 LOGICAL DISPLAY SCREEN ATTRIBUTES                         */
/*****************************************************************************/
#define vaProgram   (UCHAR)1            /* program line attribute            */
#define vaBptOk     (UCHAR)2            /* breakpoint allowed on line        */
#define vaBptOn     (UCHAR)3            /* breakpoint active on line         */
#define vaXline     (UCHAR)4            /* executing line, no brkpoint active*/
#define vaXlineOn   (UCHAR)5            /* executing line, breakpoint active */
#define vaMenuBar   (UCHAR)6            /* menu bar                          */
#define vaMenuCsr   (UCHAR)7            /* menu bar cursor                   */
#define vaStgExp    (UCHAR)8            /* storage expression                */
#define vaStgVal    (UCHAR)9            /* storage values                    */
#define vaRegWind  (UCHAR)10            /* register window                   */
#define vaPrompt   (UCHAR)11            /* menu prompts                      */
#define vaError    (UCHAR)12            /* error messages                    */
#define vaInfo     (UCHAR)13            /* file and position                 */
#define vaHelp     (UCHAR)14            /* help pannels                      */
#define vaStgPro   (UCHAR)15            /* storage expression prompts        */
#define vaClear    (UCHAR)16            /* clear screen on exit              */
#define vaStgStat  (UCHAR)17            /* data window status line           */
#define vaMxdSrc   (UCHAR)18            /* asm window source lines           */
#define vaMenuSel  (UCHAR)19            /* Menubar item select char highlight*/
#define vaCallStk  (UCHAR)20            /* Call stack.                       */
#define vaCallSel  (UCHAR)21            /* Call stack select.                */
#define vaRegTogg  (UCHAR)22            /* Toggle registers, highlite value  */
#define vaTemp1    (UCHAR)23            /*                                   */
#define vaBadAct   (UCHAR)24            /* Non selectable item.           701*/
#define vaBadSel   (UCHAR)25            /* Non selectable item highlight. 701*/
#define vaShadow   (UCHAR)26            /* shadow stuff around pulldowns. 701*/

/*****************************************************************************/
/*                    HARDWARE COLOR ATTRIBUTE VALUES                        */
/*****************************************************************************/
#define FG_black   0x00
#define FG_blue    0x01
#define FG_green   0x02
#define FG_cyan    0x03
#define FG_red     0x04
#define FG_magenta 0x05
#define FG_brown   0x06
#define FG_white   0x07
#define FG_light   0x08
#define BG_black   0x00
#define BG_blue    0x10
#define BG_green   0x20
#define BG_cyan    0x30
#define BG_red     0x40
#define BG_magenta 0x50
#define BG_brown   0x60
#define BG_white   0x70
#define FG_yellow  ( FG_light | FG_brown )

/*****************************************************************************/
/*               HARDWARE MONOCHROME COLOR ATTRIBUTE VALUES                  */
/*****************************************************************************/
#define FG_uline     0x01
#define FG_attribute 0x0F

/*****************************************************************************/
/*               PUTRC FORMATING MACROS                                      */
/*****************************************************************************/
#define RepCnt(n) 0xFF,n                /* repeat count                      */
#define Attrib(c) ( 0x80 | (c) )
