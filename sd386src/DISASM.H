/*****************************************************************************/
/*                                                                           */
/*  File:                                                IBM Confidential    */
/*      DISASM.H                                                             */
/*                                                                           */
/*  OS/2 Protect Mode EXE File Debug, Interface to Debug Toolbox             */
/*  Copyright 1988 by IBM Corp.  All rights reserved.                        */
/*                                                                           */
/*                                                                           */
/*  Description:                                                             */
/*      Typedef and constant for disassembler routine, also define the       */
/*      interface needed                                                     */
/*  History:                                                                 */
/*      11/01/88   Jim Christensen (JimC at YKTVMH)     - created/written    */
/*      04/19/91   Add more rettype constants.                            107*/
/*      05/30/91   hooking up the assembler window.                       106*/
/*                                                                           */
/*...Release 1.00 (03/03/92)                                                 */
/*...                                                                        */
/*... 03/18/92  606   Srinivas  Handle multiple segment numbers in lno table */
/*...                           due to alloc_text pragma.                    */
/*...Release 1.01 (03/31/92)                                                 */
/*...                                                                        */
/*... 05/13/92  702   Joe       Fix for display of 16 bit opndsin use32 segs.*/
/*****************************************************************************/

#define LINT_ARGS 1


/*****************************************************************************/
/* -do not change these defines.                                             */
/*****************************************************************************/
#define  USE16 0
#define  USE32 1

typedef struct
{
  uchar  *InstPtr;                      /* instruction stream                */
  uchar  *HexBuffer;                    /* disassembled hex output           */
  uchar  *MneBuffer;                    /* disassembled mnemonic output      */
  uchar  *TextBuffer;                   /* disassembled instruction output   */
  ulong  InstEIP;                       /* EIP value for this instruction    */
  struct
  {
    uint   Use32bit : 1;                /* 1 : use32, 0 : use16           101*/
    uint   N387bit  : 1;                /* 1 : 80387                      101*/
    uint   MASMbit  : 1;                /* 1 : masm, 0 : al/86            101*/
    uint   Unused1  : 29;               /* reserved                       521*/
  } Flags;                              /* flagbits for options              */
  uchar  retInstLen;                    /* instruction length                */
  uchar  retType;                       /* type of operand info              */
  uchar  retReg;                        /* register field                    */
  ulong  retOffset;                     /* instrn's offset/displacement      */
  ushort retSeg;                        /* instrn's selector field           */
  uchar  retBase;                       /* base register field               */
  uchar  retIndex;                      /* index register field              */
  uchar  retScale;                      /* scaling factor field              */
  struct
  {                                     /* retOpSize and retAddr32 swapped702*/
     uint   retOpSize : 1;              /* 1 : opsize32, 0 : opsize16     702*/
     uint   retAddr32 : 1;              /* 1 : addr32, 0 : addr16         702*/
     uint   Unused2   : 30;             /* reserved                       521*/
  }retbits;
  ushort retescap;                      /* coprocessor code                  */
  ulong  retImmed;                      /* immediate value                   */
}DTIF;                                  /* Dave Toll's interface             */


/* rettype definitions */

#define NOOPND              0            /* no operand info returned          */
#define MEM8BIT             1            /* inst accesses 8 bit mem loc       */
#define MEM16OR32BIT        2            /* inst accessss 16 bit mem loc      */
                                         /* retOpSize = 0 => 32 bit           */
                                         /* retOpSize = 1 => 16 bit           */
#define MEM1616OR3216BIT    3            /* inst accessss 16:16 operand       */
                                         /* retOpSize = 0 => 16:16            */
                                         /* retOpSize = 1 => 32:16            */
#define JMPIPDISP           4            /* inst jumps to IP/EIP+disp         */
#define JMPNEARIND          5            /* inst jumps to addr in mem loc     */
#define JMPREGNEAR          6            /* inst jumps to addr in reg         */
#define JMPFARIND           7            /* inst jumps to addr in mem loc     */
#define JMPFARDIR           8            /* inst jumps to addr of memory      */
#define INTNINST            9            /* inst is INT n                     */
#define XLATINST           10            /* inst is XLAT                      */
#define RETNEAR            11            /* inst is a near return             */
#define RETFAR             12            /* inst is a far return              */
#define INTRET             13            /* inst is an IRET                   */
#define ILLEGINST          14            /* an unknown instruction            */
#define LEAINST            15            /* inst is load effective address    */
#define ESCMEM             16            /* inst is escape that accesses mem  */
#define ESCINST            17            /* inst is escape (no mem access)    */
#define BOUNDINST          18            /* inst is BOUND                     */
#define LGDTETC            19            /* LGDT/LIDT/SGDT/SIDT instruction   */
#define BADSEGOV           20            /* a spurious segment override       */
#define IMMEDINST          21            /* inst involving reg and immed      */
#define CALLIPDISP         22            /* a near call to IP/EIP+disp        */
#define CALLNEARIND        23            /* inst calls addr in mem loc        */
#define CALLREGNEAR        24            /* inst calls addr in register       */
#define CALLFARIND         25            /* inst calls addr in mem loc        */
#define CALLFARDIR         26            /* inst calls addr of memory         */
#define REPETC             27            /* REP/REPZ/REPNZ/REPE/REPNE prefix  */
#define BYTESTRINST        28            /* CMPS/LODS/MOVS/SCAS/STOS byte     */
#define WORDSTRINST        29            /*  "                  " word/dword  */

/* retreg definitions (for rettypes 1, 2, 3, 5, 7, 10, 20, 23, or 25 ) */

#define RETREGES    1                   /* ES register                       */
#define RETREGCS    2                   /* CS register                       */
#define RETREGSS    3                   /* SS register                       */
#define RETREGDS    4                   /* DS register                       */
#define RETREGFS    5                   /* FS register                       */
#define RETREGGS    6                   /* GS register                       */

/* retreg definitions (for rettype 6 or 24) */

#define RETREGAX      0                 /* AX register                       */
#define RETREGCX      1                 /* CX register                       */
#define RETREGDX      2                 /* DX register                       */
#define RETREGBX      3                 /* BX register                       */
#define RETREGSP      4                 /* SP register                       */
#define RETREGBP      5                 /* BP register                       */
#define RETREGSI      6                 /* SI register                       */
#define RETREGDI      7                 /* DI register                       */
#define RETREGEAX     8                 /* EAX register                      */
#define RETREGECX     9                 /* ECX register                      */
#define RETREGEDX    10                 /* EDX register                      */
#define RETREGEBX    11                 /* EBX register                      */
#define RETREGESP    12                 /* ESP register                      */
#define RETREGEBP    13                 /* EBP register                      */
#define RETREGESI    14                 /* ESI register                      */
#define RETREGEDI    15                 /* EDI register                      */
#define RETREGNONE  255                 /* no register used                  */

/* repeat type return code             */
#define REPTYPE     27


#define TextCol 19                      /* instr output column number        */
#define ModNameCol TextCol+15           /* allow min of 15 space for inst    */
#define Use387    1                     /* if 1 use 387 instruction, 0 no    */
/*  #define  AL86_MASM   1                 MYVIEW                            */


void printmnem (USHORT);
void prt8reg16 (UCHAR);
void print387m (USHORT);

#define HEXBUFFSIZE  33
#define MNEMBUFFSIZE 10 /* use this size for strings like"BOUND????"      905*/
#define TEXTBUFFSIZE 49

typedef struct
{
 uint    instaddr;                                                      /*108*/
 char    mne[MNEMBUFFSIZE];                                             /*108*/
 uchar   hex[HEXBUFFSIZE];                                              /*108*/
 uchar   text[TEXTBUFFSIZE];                                            /*108*/
 uchar   len;
 uchar   mod_type;                      /* => 16- or 32-bit module        106*/
                                        /*  0 - 16 bit                    106*/
                                        /*  1 - 32 bit                    106*/
 uchar   type;                          /* type of operand info              */
 ushort  OpSize;                        /* 1 =>opsize32, 0  opsize16      108*/
 uchar   reg;                           /* register field                    */
 ulong   offset;                        /* instrn's offset/displacement      */
 ushort  seg;                           /* instrn's selector field           */
 uchar   base;                          /* base register field               */
 uchar   index;                         /* index register field              */
 uchar   scale;                         /* scaling factor field           108*/
}INSTR;

#define ENDOFCACHE NULL                 /*                                235*/
#define ADDRNOTFOUND -1                 /* addr not in cache flag.        606*/
