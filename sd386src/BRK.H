/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   Brk.h                                                                   */
/*                                                                           */
/* Description:                                                              */
/*   definations for managing break points.                                  */
/*                                                                           */
/* History:                                                                  */
/*                                                                           */
/*   02/08/91 Creation of 32-bit SD86, from 16-bit version.                  */
/*                                                                           */
/*...Release 1.00 (Pre-release 108 12/05/91)                                 */
/*...                                                                        */
/*... 02/10/92  515   Srinivas  Multiple hits of a deferred break points     */
/*                              (same func names).                           */
/*...                                                                        */
/*...Release 1.00 (03/03/92)                                                 */
/*...                                                                        */
/*... 03/12/92  604   Srinivas  Conditional break points failing on signed   */
/*...                           variables.                                   */
/*****************************************************************************/

/*****************************************************************************/
/*                  CONDITION FOR BREAK POINT                                */
/*****************************************************************************/
typedef struct {
  UCHAR   relation;
  signed char    opsize;                /* added explicitly signed char   604*/
  UINT    opaddr;
  long    constant;
  char   *pCondition;                   /* conditional bp text.              */
} BRKCOND;

/*****************************************************************************/
/*          VALUES FOR THE RELATION FIELD IN THE BRKCOND STRUCTURE           */
/*****************************************************************************/
#define  COND_NE   1
#define  COND_EQ   2
#define  COND_GT   3
#define  COND_GE   4
#define  COND_LT   5
#define  COND_LE   6
#define  COND_GTU  7
#define  COND_GEU  8
#define  COND_LTU  9
#define  COND_LEU  10

#define  COND_MSH  100

typedef struct _brkflags
{
 int   DorI       :2;
 int   DefineType :3;
 int   ActionType :3;
 int   Reported   :1;
 int   File       :1;
 int   WasDeferred:1;
 int   Reserved   :21;
}BRK_FLAGS;

typedef struct BRKstruct {
  struct     BRKstruct *next;
  struct     BRKstruct *prev;
  ULONG      brkat;
  ULONG      mte;
  USHORT     objnum;
  UINT       mid;
  ULONG      lno;
  int        sfi;
  BRKCOND   *cond;
  UCHAR      byte;
  UCHAR      once;
  BRK_FLAGS  flag;
  char      *funcname;
  char      *dllname;
  char      *srcname;
} BRK;

/*****************************************************************************/
/*        VALUES PASSED TO SETLINEBRK AND SETADDRBRK                         */
/*****************************************************************************/
#define BP_DEFR      1
#define BP_IMMEDIATE 2

#define BP_ADDR      1
#define BP_FUNC_NAME 2
#define BP_SRC_LNO   3
#define BP_INTERNAL  4
#define BP_RESTART   5
#define BP_LOAD_ADDR 6
#define BP_DLL_LOAD  7

#define BRK_SIMP     1
#define BRK_COND     2
#define BRK_ONCE     3

#define BREAKPT8086OPCODE 0xCC
