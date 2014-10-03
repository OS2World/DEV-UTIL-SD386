/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   afile.h                                                                 */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/* In-Storage File Structure                                                 */
/*                                                                           */
/* There is one of these structures for each module in the ring of active    */
/* modules.  They are created by the makefp and fakefp procedures.           */
/*                                                                           */
/* For each line in the file, there is an element in the offset vector that  */
/* locates a (flags, text) sequence for that line.  The "source" field points*/
/* to the first line (sequence) of the file.                                 */
/*                                                                           */
/* History:                                                                  */
/*                                                                           */
/*...Release 1.00 (Pre-release 1)                                            */
/*...                                                                        */
/*... 08/30/91  235   Joe       Cleanup/rewrite ascroll() to fix several bugs*/
/*...                                                                        */
/*...Release 1.00 (After pre-release 1.08)                                   */
/*...                                                                        */
/*... 02/12/92  521   Joe       Port to C-Set/2.                             */
/*...                                                                        */
/*                                                                           */
/*****************************************************************************/


typedef struct afile {
    struct afile *next;
    UINT   (* shower)(struct afile*,UCHAR*); /*  return type chg to uint. 521*/
    DEBFILE *pdf;                 /* debug file for this afile               */
    UINT     mid;                 /* module ID from the linker               */
    UINT     mseg;                /* module selector for disassembly      827*/
    UCHAR   *source;              /* start of source buffer for file         */
    USHORT  *offtab;              /* pointer to table of line offsets     100*/
    UINT     Tlines;              /* # of lines total in source file         */
    UINT     Nlines;              /* # of lines in source buffer (1..N)      */
    UINT     Nbias;               /* # of lines skipped in source file       */
    UINT     topline;             /* Line number of top line (0..N) - Nbias  */
    UINT     csrline;             /* Line number of cursor (0..N) - Nbias    */
    UINT     hotline;             /* Line number of last find (0..N) - Nbias */
    UINT     hotaddr;             /* Address of last find                    */
    UINT     skipcols;            /* # of cols skiped on left (0..N)         */
    UINT     Nshown;              /* # of lines shown on screen (1..N)       */
    UINT     topoff;              /* top offset for ASM display              */
    CSR      csr;                 /* current cursor data                  827*/
    USHORT   flags;               /* state data                           235*/
    UCHAR    sview;               /* view in mixed source/disasm mode        */
                                  /* MIXEDN    1 multiple source lines.      */
                                  /* NOSRC     2 no source at all.           */
    struct
    {
      UINT   StartAddr;
      int    StartLine;
    } AsmView;

    int   sfi;                    /* source file index for this file.        */
    UCHAR filename[1];            /* full path name of file                  */
} AFILE;


/*****************************************************************************/
/* Values for the flags byte preceeding each line of text in file. The high  */
/* order 4 bits MUST be zero in these flay bytes because the Decode routine  */
/* uses this convention to detect the end of a line.                         */
/*****************************************************************************/
#define LINE_OK 0x01              /* segment offset known for this line      */
#define LINE_BP 0x02              /* active breakpoint on this line          */

/*****************************************************************************/
/* This macro defines the size of a new AFILE. The filename member           */
/* is a length prefixed z-string. This is why you see an n+1 in              */
/* the macro expansion.                                                      */
/*****************************************************************************/
#define SizeOfAFILE(n) (n+1+sizeof(AFILE))

/*****************************************************************************/
/*         Predicate for determining if C soucre code is available.          */
/*****************************************************************************/

#define ASMSRCVIEWS   2                 /*                                106*/
#define MIXEDN        1                 /* multiple source lines.         106*/
#define NOSRC         2                 /* no source at all.              106*/


/*****************************************************************************/
/* View flags.                                                            235*/
/*****************************************************************************/

#define AF_ZOOM          0x0001     /* show the currently executing line  235*/
#define AF_SYNC          0x0002     /* synchronize ASM and SOURCE views   235*/
#define AF_HUGE          0x0004     /* compressed source exceeds 64K      235*/
#define AF_FAKE          0x0008     /* fake name for module               235*/
#define ASM_VIEW_NEW     0x0020     /* new assembler view.                235*/
#define ASM_VIEW_REFRESH 0x0040     /* refresh of current view.           235*/
#define ASM_VIEW_TOGGLE  0x0080     /* asm/source & asm/asm toggle.       235*/
#define ASM_VIEW_CHANGE  0x0100     /* change views.                      235*/
#define ASM_VIEW_NEXT    0x0200     /* next asm view.                     235*/
#define ASM_VIEW_MNE     0x0400     /* change mnemonics.                  235*/

/*****************************************************************************/
/* On a pagefp( fp,lno) we will have LINESBEFORE lines in the source buffer  */
/* before the lno.                                                           */
/*****************************************************************************/

#define LINESBEFORE 500
