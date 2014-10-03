/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   Regwin.h                                                                */
/*                                                                           */
/* Description:                                                              */
/*   Register window stuff.                                                  */
/*                                                                           */
/* History:                                                                  */
/*                                                                           */
/*...Release 1.00 (Pre-release 107 11/13/91)                                 */
/*...                                                                        */
/*... 11/18/91  401   Srinivas  Floating point Register Display.             */
/*...                                                                        */
/*****************************************************************************/

typedef struct
{
 UCHAR Mantisa[8];
 UCHAR Exp[2];
} REG80BIT;

typedef struct _COPROCESSORREGS
{
 USHORT   ControlWord;
 USHORT   Reserved1;
 USHORT   StatusWord;
 USHORT   Reserved2;
 USHORT   TagWord;
 USHORT   Reserved3;
 ULONG    IpOffset;
 USHORT   CsSelector;
 USHORT   Reserved4;
 ULONG    OpOffset;
 USHORT   OpSelector;
 USHORT   Reserved5;
 REG80BIT Stack[8];
} COPROCESSORREGS;

#define FCincr 4                        /* Space in columns between 2 flags  */
#define FC0 0                           /* 1st flag column position          */
#define FC1 (FC0+FCincr)                /* 2nd flag column position          */
#define FC2 (FC1+FCincr)                /* 3rd flag column position          */
#define FC3 (FC2+FCincr)                /* 4th flag column position          */
#define FC4 (FC3+FCincr)                /* 5th flag column position          */
#define FC5 (FC4+FCincr)                /* 6th flag column position          */
#define HIGHLIGHT 0x8                   /* Mask for toggling high light bit  */

#define NO32REGS 9                      /* # of 32-bit regs in window        */
#define NO16REGS 6                      /* # of 16-bit regs in window        */
#define NOFREGS 7                       /* # of flags in window              */
#define REGSWINWIDTH  16                /* Register Window width             */
#define REGSWINLENGTH 20                /* Register Window Length            */
#define REG16MODE 1                     /* editing a 16-bit reg              */
#define REG32MODE 2                     /* editing a 32-bit reg              */
#define FLAGSMODE 3                     /* editing a flag bit                */

#define COREGSWINWIDTH  33              /* Register Window width             */
#define COREGSWINLENGTH 24              /* Register Window Length            */
#define StatusRow 20
#define ControlRow 22
#define SPACE    0x20
#define ZERO     0x30
#define EMPTY    0x2E
#define TAG_VALID  0
#define TAG_ZERO   1
#define TAG_INFIN  2
#define TAG_EMPTY  3
#define PRINTLEN   27

#define REPAINT    0
#define REGS386BIT 1
#define REGS387BIT 2
