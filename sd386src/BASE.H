/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   base.h                                                                  */
/*                                                                           */
/* Description:                                                              */
/*   base definitions                                                        */
/*                                                                           */
/* History:                                                                  */
/*                                                                           */
/*   02/08/91 Creation of 32-bit SD86, from 16-bit version.                  */
/*                                                                           */
/*...Release 1.00                                                            */
/*...                                                                        */
/*... 02/08/91  100   made changes for 32-bit compilation.                   */
/*...                                                                        */
/*...Release 1.00 (After pre-release 1.08)                                   */
/*...                                                                        */
/*... 02/12/92  521   Joe       Port to C-Set/2.                             */
/*...                           ( some macros moved to dfile.h )             */
/*...                                                                        */
/*...Release 1.01 (04/03/92)                                                 */
/*...                                                                        */
/*... 05/08/92  701   Srinivas  Cua Interface.                               */
/*... 09/21/92  706   Joe       Fix for 32-16 C-Set/2 call unwind.           */
/*...                                                                        */
/*...Release 1.02 (10/22/92)                                                 */
/*...                                                                        */
/*... 03/03/93  813   Joe       Revised types handling for HL03.             */
/*****************************************************************************/

/*****************************************************************************/
/*                             DATA TYPES                                    */
/*****************************************************************************/
#ifdef INCL_BASE                                                        /*813*/
typedef unsigned char   uchar;
typedef unsigned int    uint;
typedef unsigned long   ulong;
typedef unsigned short  ushort;
typedef unsigned short  twoc;           /* two characters                    */
typedef unsigned int    fourc;          /* four characters                   */

typedef uint (* _System UINTFUNC)(uint,...);  /*                          521*/

typedef uchar varstr[1];                /* Length byte, bytes of data        */
typedef uchar STRING[10];               /* string of 10 chars             701*/

typedef unsigned short OFFSET;                                          /*226*/
typedef unsigned int SELECTOR;

/*****************************************************************************/
/*                         MISCELLANEOUS CONSTANTS                           */
/*****************************************************************************/

#define   YES    1
#define   NO     0
#undef    NULL
#define   NULL   0
/*****************************************************************************/
/* SD386, in order to try to eliminate the confusion over the definition,    */
/* has commented all definitions in the c and os2 header files and provided  */
/* its own definition.                                                       */
/*****************************************************************************/

#define MAXUINT    ((uint)-1)
#define MAXINT     MAXUINT/2
#define MAXSEGSIZE 65536L
#define MAXPER   150         /* max # of screen rows which can be handled 518*/
#define PROMAX 255
#define MAXCOLS 255                     /* max # of chars used / line of text*/
#define MAXSYM  255                     /* max # of chars for a symbol    809*/
#define MAXAFRAMES 100                  /* max # of active stack frames   706*/
#define MAXTRDS 20                      /* max # of active threads           */
#define MAXEXECS 20                     /* max # of active exec'ed processes */
#define MAXEXEMID 500                   /* max module id for the EXE file    */

/*****************************************************************************/
/* Views                                                                     */
/*****************************************************************************/

#define  SOURCEVIEW            1                                        /*701*/
#define  ASSEMBLYVIEW          2                                        /*701*/
#define  DATAVIEW              3                                        /*701*/
#define  ACTBARVIEW            4        /* event occurred on action bar   701*/
#define  DATAVIEWASM           5                                        /*701*/
#define  DATAVIEWSRC           6                                        /*701*/
#define  EXPANDDATA            7       /* 701                              */
#define  BROWSEFILE            8       /* 701                              */
#define  BROWSEMSHFILE         9                                        /*701*/
/*****************************************************************************/
/* areas in the storage window.                                           701*/
/*****************************************************************************/
#define  EDITAREA              1        /* edit area in data window.      701*/
#define  STORAGEAREA           2        /* storage area in data window.   701*/
#define  ASCIIAREA             3        /* ascii area in data window.     701*/

/*****************************************************************************/
/* This is a return code used in mouse support to return back to the callr701*/
/* for an update of the field being edited in the storage or ascii area.  701*/
/*****************************************************************************/
#define RECIRCULATE 2                   /*                                701*/

#define  SHRINKUPARROWCOL     73
#define  EXPANDDNARROWCOL     75

/*****************************************************************************/
/* This is the maximum name length for symbols and types subsections.     813*/
/*  255 for the name + 2 for the length prefix + 1 for \0 terminator.     813*/
/*****************************************************************************/
#define  MAXNAMELEN  258

#endif                                                                  /*813*/
/*****************************************************************************/
/*                         POINTER MANIPULATION MACROS                       */
/*****************************************************************************/
#ifdef INCL_MACROS                                                      /*813*/

#define HiFlat(p)           (*( (USHORT *)(void*)&(p) + 1 ))
#define LoFlat(p)           (*( (USHORT *)&(p) ))

#define Sel2Flat(s)         (((ULONG)(((USHORT)s)>>3))<<16)
#define Flat2Sel(p)         ((SEL)((HiFlat(p)<<3) | 7))
#define OffOf(p)            (*( (UINT *)&(p) ))
#define SelOf(p)            (*( (UINT *)(void*)&(p) + 1 ))

/*****************************************************************************/
/*                           BIT MANIPULATION MACROS                         */
/*****************************************************************************/

#define SetBitOff(v,b) ((v) &= ~(b))
#define SetBit(X,N) ((X) |= (1UL<<(N)))
#define ResetBit(X,N) ((X) &= ~(1UL<<(N)))
#define TestBit(X,N) ((X) & (1UL<<(N)))
#endif                                                                  /*813*/
