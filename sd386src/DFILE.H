/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   Dfile.h                                                                 */
/*                                                                           */
/* Description:                                                              */
/*   Data display structures and constants.                                  */
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
/*...                           ( some macros moved to here )                */
/*... 02/13/92  528   Srinivas  Increase the size of expression which can be */
/*...                           entered in data window.                      */
/*...                                                                        */
/*...Release 1.01 (04/03/92)                                                 */
/*...                                                                        */
/*... 08/03/92  701   Joe       Cua Interface.                               */
/*...                                                                        */
/*...Release 1.02 (10/22/92)                                                 */
/*...                                                                        */
/*... 03/03/93  813   Joe       Revised types handling for HL03.             */
/*...                                                                        */
/*****************************************************************************/

/*****************************************************************************/
/* The data displayed by the debugger is logically represented as a file.    */
/* The window that displays the data can be moved over this file in a simi-  */
/* lar way to the code window moving over the file of source or asm code.    */
/*****************************************************************************/

#define EXPRSIZE     255                /*                                528*/
#define DATALINESPAN 135     /* buffer size of data display lin(w/3 attrs)813*/
                             /* this value used to be 83.                 813*/
typedef void* SCOPE;                     /* variable scrope (null if global) */


#define DFP_BASE_CLASS     0x01
#define DFP_DERIVED_CLASS  0x02

typedef struct _dfpflags
{
 int   ClassType  :2;                   /* Base Class/Derived Class          */
 int   reserved   :30;
}DFP_FLAGS;

typedef struct DfileStruct {
  struct DfileStruct *next;
  DFP_FLAGS DfpFlags;
  UINT   lrn;       /* logical record number within the data file (0..N)     */
  UINT   lines;     /* number of lines needed to display this data object.   */
  UINT   mid;       /* module ID defining context for data                   */
  UINT   lno;       /* source line defining context for data                 */
  int    sfi;       /* source file index.                                    */
  UINT   datatype;  /* data type ID of variable                              */
  UINT   showtype;  /* data type ID for display                              */
  UINT   sfx;       /* stack frame index (1..N)                              */
  SCOPE  scope;     /* scope of variable (null if global)                    */

                    /* function for displaying expression                    */
  UINT   (* shower)( struct DfileStruct * , UINT , UINT , UINT );

                    /* function for changing storage of type showtype        */
  UINT    (* zapper)( struct DfileStruct * , UINT , UINT );             /*521*/
  UCHAR  expr[ EXPRSIZE ];  /* ASCII text for expression */
  UINT   baseaddr;          /* base address location of this data.        101*/
} DFILE;

typedef UINT (* SHOWER) (struct DfileStruct *, UINT, UINT, UINT);
typedef UINT (* ZAPPER) (struct DfileStruct *, UINT, UINT);

#define SHORTX   11                     /* # of chars for short expressions  */
#define STGCOL   13                     /* screen col for storage data (1..N)*/
#define ASCIICOL 65                     /* screen col for ASCII data (1..N)  */
#define SXCHAR   0xF7                   /* short expression continuation char*/

