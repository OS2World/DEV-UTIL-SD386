/******************************************************************************\
***                                                                          ***
***        COPYRIGHT   I B M   CORPORATION  1983, 1984, 1985                 ***
***                                   1986, 1987, 1988, 1989                 ***
***                                                                          ***
***        LICENSED MATERIAL  -  PROGRAM PROPERTY OF I B M                   ***
***                                                                          ***
***        REFER TO COPYRIGHT INSTRUCTIONS: FORM G120-2083                   ***
***                                                                          ***
********************************************************************************
*                                                                              *
*                                                                              *
*   MODULE-NAME      = DISASM.C                                                *
*                                                                              *
*   DESCRIPTIVE-NAME = Disassembler for 8086 family processors                 *
*                                                                              *
*   STATUS           = Version 1 Release 1                                     *
*                                                                              *
*   FUNCTION         = to disassemble one instruction into MASM or             *
*                      ASM/86 mnemonics and instruction formats.               *
*                                                                              *
*   Compiler         = IBM C/2 ver 1.1                                         *
*                                                                              *
*   HISTORY          = written in PLS/88 March 1983 by D. C. Toll              *
*                      for use with IWSDEB and CP/88.                          *
*                                                                              *
*                      CP/386 version (for 386 32-bit order code)              *
*                      started July 1987                                       *
*                                                                              *
*                      translated to MetaWare's High C and 387 support added   *
*                      March 1989 by                                           *
*                      D. C. Toll, IBM Research, Yorktown Heights              *
*                                                                              *
*                      IBM C/2 version converted by                            *
*                      Stephen Luk, CASS, Lexington, Ky  (Aug, 1989)           *
*                                                                              *
* ... 02/12/92  521   Srinivas  Port to C-Set/2.                               *
*                                                                              *
*                                                                              *
\******************************************************************************/

#if 0
char far * far pascal bcopy(void far *,void far * ,unsigned);/* Joe C.     504*/
#endif

                                        /* typedefs for PM convention         */
typedef char           CHAR;
typedef short          SHORT;
typedef long           LONG;
typedef unsigned       BIT;
typedef unsigned char  UCHAR;
typedef unsigned short USHORT;
typedef unsigned long  ULONG;
typedef unsigned int   UINT;            /* this is not a PM convention but to */
                                        /* eliminate compiler warning, use    */
                                        /* USHORT when port to 32 bits        */



typedef struct parlist {                /* the parameter area                 */
  UCHAR  *iptr;                         /* machine code ->                    */
  UCHAR  *hbuffer;                      /* hex output buffer ->               */
  UCHAR  *mbuffer;                      /* mnemonic output buffer ->          */
  UCHAR  *ibuffer;                      /* operand output buffer ->           */
  ULONG  instr_EIP;                     /* EIP value @ this instruction       */
  UINT   flagbits;                      /* flag bits :
                                            bit 2 (1) => MASM format decode
                                                  (0) => ASM/86 format decode

                                           NOTE: if the ASM/86 mnemonic table is
                                                 omitted, this bit is ignored.

                                            bit 1 (1) => ESC orders are decoded
                                                         287/387 orders
                                                  (0) => decoded as "ESC"

                                            bit 0 (1) => do 386 32-bit decode
                                                  (0) => do 16-bit decode     */
  UCHAR  retleng;                       /* length of dis-assembled instr      */
  UCHAR  rettype;                       /* type of returned operand info      */
  UCHAR  retreg;                        /* returned register field            */
  ULONG  retoffset;                     /* returned displacement/offset       */
  USHORT  retseg;                       /* returned segment field             */
  UCHAR  retbase;                       /* returned base register field       */
  UCHAR  retindex;                      /* returned index register field      */
  UCHAR  retscale;                      /* returned scale factor field        */
  UINT   retbits;                       /* returned bit flags:
                                           bit 0 (1) => operand size is 32 bits
                                                 (0) => otherwise 16 bits
                                           bit 1 (1) => address size is 32 bits
                                                 (0) => otherwiseis 16 bits   */
  USHORT  retescape;                    /* ESC instructions opcode            */
  ULONG retimmed;                       /* immediate value if any             */
} PARLIST;
                                        /* bit masks for "flagbits"           */
static UCHAR *disbyte(UCHAR,UCHAR *);                                   /*521*/
static UCHAR *hexbyte(UCHAR,UCHAR *);                                   /*521*/
static UCHAR *hexdword(ULONG,UCHAR *);                                  /*521*/
static UCHAR *hexword(ULONG,UCHAR *);                                   /*521*/
static UCHAR *printitem(ULONG,char *,UCHAR *);                          /*521*/
static void printmnem(USHORT);                                          /*521*/
static void print387m(USHORT);                                          /*521*/
static void prt8reg16(UCHAR);                                           /*521*/
static void prtbyte(void);                                              /*521*/
static void prtdword(void);                                             /*521*/
static void prtovseg(void);                                             /*521*/
static void prtword(void);                                              /*521*/
static void prtimmed(void);                                             /*521*/
static void putpar(void);                                               /*521*/
static void putstr(char *);                                             /*521*/
static void r1(void);                                                   /*521*/
static void setdw(void);                                                /*521*/
static void setrm(void);                                                /*521*/
void DisAsm (PARLIST *);                                                /*521*/

#define use32mask   1
#define N387mask    2
#define MASMmask    4

static PARLIST *parm = 0;               /* pointer to the parameter block    */
                                        /* values returned in rettype        */
#define  membtype       1
#define  memwtype       2
#define  memwwtype      3
#define  jreltype       4
#define  jnearmemtype   5
#define  jnearregtype   6
#define  jfartype       7
#define  jfarimmtype    8
#define  intntype       9
#define  xlattype       10
#define  retneartype    11
#define  retfartype     12
#define  intrettype     13
#define  illegtype      14
#define  LEAtype        15
#define  escmemtype     16
#define  escapetype     17
#define  BOUNDtype      18
#define  LGDTtype       19
#define  segovtype      20
#define  regimmedtype   21
#define  creltype       22
#define  cnearmemtype   23
#define  cnearregtype   24
#define  cfartype       25
#define  cfarimmtype    26
#define  reptype        27              /*                               1.01 */
#define  strbtype       28              /*                               1.01 */
#define  strwtype       29              /*                               1.01 */

#include "masmmnem.c"
#include "a86mnem.c"
#include <string.h>                     /*                                101*/
                                        /*segment register names             */
char segreg[] = {
       2,2,'E','S',
       2,2,'C','S',
       2,2,'S','S',
       2,2,'D','S',
       2,2,'F','S',
       2,2,'G','S',
       0            };
                                        /* 8-bit register names               */
char reg8[] = {
       2,2,'A','L',
       2,2,'C','L',
       2,2,'D','L',
       2,2,'B','L',
       2,2,'A','H',
       2,2,'C','H',
       2,2,'D','H',
       2,2,'B','H',
       0            };
                                        /* 16-bit register names              */
char reg16[] = {
       2,2,'A','X',
       2,2,'C','X',
       2,2,'D','X',
       2,2,'B','X',
       2,2,'S','P',
       2,2,'B','P',
       2,2,'S','I',
       2,2,'D','I',
       0            };
                                        /* 32-bit register names              */
char reg32[] = {
       3,3,'E','A','X',
       3,3,'E','C','X',
       3,3,'E','D','X',
       3,3,'E','B','X',
       3,3,'E','S','P',
       3,3,'E','B','P',
       3,3,'E','S','I',
       3,3,'E','D','I',
       0            };
                                        /* CRn, DRn and TRn register names    */
char specialreg[] = {
       3,3,'C','R','0',
       3,3,'C','R','1',
       3,3,'C','R','2',
       3,3,'C','R','3',
       3,3,'D','R','0',
       3,3,'D','R','1',
       3,3,'D','R','2',
       3,3,'D','R','3',
       3,3,'D','R','4',
       3,3,'D','R','5',
       3,3,'D','R','6',
       3,3,'D','R','7',
       3,3,'T','R','6',
       3,3,'T','R','7',
       0            };
                                        /* instruction buffer format          */
char bytevec[] = "BYTE_";
char wordvec[] = "WORD_";
char dwordvec[] = "DWORD_";
                                        /* vectors to convert 16-bit format
                                           mod-r/m bytes to base and
                                           index register forms               */
UCHAR basereg16[8] = { 4,4,6,6,7,8,6,4 };
UCHAR indexreg16[8] = { 7,8,7,8,0,0,0,0 };

                                        /* local variables                    */

static UCHAR   *startiptr;              /* instruction stream pointer         */
static UCHAR   *hbuff = 0;              /* hex output buffer pointer          */
static UCHAR   *mbuff = 0;              /* mnemonic output buffer pointer     */
static UCHAR   *ibuff = 0;              /* operand output buffer pointer      */
static UCHAR instr = 0;                 /* holds the current instruction      */
static UCHAR ESCinstr = 0;              /* holds the current 387 instruction  */
static UCHAR ovseg = 0;                 /* non-zero if there is a current
                                           segment override instr pending     */
static UCHAR defseg = 0;                /* default segment for operands       */
static UCHAR wbit = 0;                  /* holds "w" bit from instruction     */
static UCHAR dbit = 0;                  /* holds "d" bit from instruction.
                                           This may take the following values:
                                           0   => register is second operand
                                           2   => register is first operand
                                           244 => BT/BTC/BTR/BTS reg/mem,reg
                                           245 => MOVSX/MOVZX reg,word
                                           246 => MOVSX/MOVZX reg,byte
                                           247 => SHLD/SHRD mem/reg,reg,immed
                                           248 => SHLD/SHRD mem/reg,reg,CL
                                           249 => there is no register or
                                                  second operand
                                           (used for some '0F'X 286 orders)
                                           250 => for a shift n instruction
                                                  - the second
                                                  operand is an 8-bit immediate,
                                                  regardless of the type of the
                                                  first operand.
                                                  This is also used for 0F BA
                                                  instructions, which also
                                                  always have an 8-bit immediate
                                                  operand.
                                           251 => first operand is register
                                                  field, printed as a number
                                                  (used for escape instructions)
                                           252 => second operand is "1" used
                                                  for shift orders
                                           253 => second operand is "CL" used
                                                  for shift orders
                                           254 => only 1 (memory) operand
                                           255 => first operand is mem/reg,
                                                   second is immediate
                                                                              */
static UCHAR  regf = 0;                 /* holds "reg" field from instruction */
static UCHAR  mod = 0;                  /* holds "mod" field from instruction */
static UCHAR  rm = 0;                   /* holds "r/m" field from instruction */
static UCHAR  eee = 0;                  /* holds "eee" field from instruction */
static UCHAR  basereg = 0;              /* index into register names of the
                                           base register, 0 if none           */
static UCHAR  indexreg = 0;             /* index into register names of the
                                           index register, 0 if none          */
static UCHAR  scalefactor = 0;          /* scale factor, possible values are:
                                            0 => none
                                            1 => *2
                                            2 => *4
                                            3 => *8                           */
static ULONG opdisp = 0;                /* operand displacement from instr    */
static long  signeddisp = 0;            /* holds a signed displacement:
                                           if signedflag is set,then this holds
                                           meaningfull contents, and opdisp will
                                           be set to zero when the instruction
                                           displacement is examined           */
static ULONG f1 = 0, f2 = 0;
static UCHAR  ic, *p = 0;
static union {
  ULONG flagbytes;
  struct {
    BIT IODXbit       : 1;              /* if 1, I/O instruction uses DX,
                                           if 0, it has an immediate operand  */
    BIT signextbit    : 1;              /* if 1, we have a sign extended
                                           byte-> word immediate operand (for
                                           opcode @83)                        */
    BIT parflagbit    : 1;              /* if non-zero, we have printed a left
                                           parenthesis                        */
    BIT segrflagbit   : 1;              /* if non-zero, "reg" is actually a
                                           segment register                   */
    BIT segsflagbit   : 1;              /* if set, "prt8reg16" will print a
                                           segment register name, rather than an
                                           ordinary register                  */
    BIT quoteflagbit  : 1;              /* set non-zero if a closing quote
                                           is required after a hex value      */
    BIT disppresbit   : 1;              /* set non-zero if a displacement is
                                           present in this instruction        */
    BIT signedflagbit : 1;              /* if 1, we have a signed displacement*/
    BIT negflagbit    : 1;              /* if 1, the signed displacement is - */
    BIT use32bit      : 1;              /* if set, we are performing 32-bit
                                           decode                             */
    BIT addroverbit   : 1;              /* if set, an address override
                                           prefix has been found              */
    BIT opsizeoverbit : 1;              /* if set, an operand size override
                                           prefix has been found              */
    BIT opsize32bit   : 1;              /* set if operand size is 32-bits
                                           (this is exclusive-OR of use32 and
                                           opsizeover)                        */
    BIT addr32bit     : 1;              /* set if address size is 32-bits
                                           (this is exclusive-OR of use32 and
                                           addrover)                          */
    BIT nocommabit    : 1;              /* if set, prtimmed does not
                                           output a leading comma             */
    BIT sibbase5bit   : 1;              /* set if mod=0 and we have a SIB
                                           byte and the SIB base = 5 (which
                                           implies no base, but there is a
                                           disp32)                            */
  } flgbits;
} flags = { 0 };
                                        /* short hand for the stru above      */

#define  IODX        flags.flgbits.IODXbit
#define  signext     flags.flgbits.signextbit
#define  parflag     flags.flgbits.parflagbit
#define  segrflag    flags.flgbits.segrflagbit
#define  segsflag    flags.flgbits.segsflagbit
#define  quoteflag   flags.flgbits.quoteflagbit
#define  disppres    flags.flgbits.disppresbit
#define  signedflag  flags.flgbits.signedflagbit
#define  negflag     flags.flgbits.negflagbit
#define  use32       flags.flgbits.use32bit
#define  addrover    flags.flgbits.addroverbit
#define  opsizeover  flags.flgbits.opsizeoverbit
#define  opsize32    flags.flgbits.opsize32bit
#define  addr32      flags.flgbits.addr32bit
#define  nocomma     flags.flgbits.nocommabit
#define  sibbase5    flags.flgbits.sibbase5bit

static struct {
  char *mnemstrptr;                     /* mnemonic table pointer             */
  char *mnem387ptr;                     /* 387 mnemonic table pointer         */
  USHORT *mnemnumptr;                   /* mnemonic number table pointers     */
  USHORT *mnem8081ptr;
  USHORT *mnemAAptr;
  USHORT *mnemC6C7ptr;
  USHORT *mnemF6F7ptr;
  USHORT *mnemFFptr;
  USHORT *shiftmnemptr;
  USHORT *repmnemptr;
  USHORT *mnem0F00ptr;
  USHORT *mnem0F01ptr;
  USHORT *m387memptr;
  USHORT *m387regptr;
  USHORT *mnemD94ptr;
  USHORT *mnemDB4ptr;
  USHORT *mnem0Fptr;
  USHORT *mnem0FBAptr;
  USHORT escmnemval;                    /* index of mnemonic ESCAPE/ESC       */
  USHORT popmnemval;                    /* index of mnemonic POP              */
  USHORT popmnem_16val;                 /* index of mnemonic POP word         */
  USHORT popmnem_32val;                 /* index of mnemonic POP dword        */
  USHORT segovmnemval;                  /* index of mnemonic SEGOV/SEG        */
  USHORT lmnemval;                      /* index of mnemonic L/MOV            */
  USHORT stmnemval;                     /* index of mnemonic ST/MOV           */
  USHORT illegmnemval;                  /* index of mnemonic ???? (illegal)   */
  USHORT timnemval;                     /* index of mnemonic TI/TEST          */
  USHORT tibmnemval;                    /* index of mnemonic TIB/TEST         */
  USHORT tiwmnemval;                    /* index of mnemonic TIW/TEST         */
  USHORT pushmnemval;                   /* index of mnemonic PUSH             */
  USHORT bswapmnemval;                  /* index of mnemonic BSWAP            */
  USHORT fabsmnemval;                   /* index of 387 mnemonic FABS         */
  USHORT fnopmnemval;                   /* index of 387 mnemonic FNOP         */
  USHORT fcomppmnemval;                 /* index of 387 mnemonic FCOMPP       */
  USHORT fucomppmnemval;                /* index of 387 mnemonic FUCOMPP      */
  USHORT fstswmnemval;                  /* index of 387 mnemonic FSTSW        */
  USHORT fillegmnemval;                 /* index of 387 mnemonic ????
                                           (illegal)                          */
  char masmval;                         /* set if we are dis-assembling to MASM,
                                           unset for ASM/286                  */
} z = { 0 };

                                        /* short hand for z structure         */
#define  mnemstr     z.mnemstrptr
#define  mnem387     z.mnem387ptr
#define  mnemnum     z.mnemnumptr
#define  mnem8081    z.mnem8081ptr
#define  mnemAA      z.mnemAAptr
#define  mnemC6C7    z.mnemC6C7ptr
#define  mnemF6F7    z.mnemF6F7ptr
#define  mnemFF      z.mnemFFptr
#define  shiftmnem   z.shiftmnemptr
#define  repmnem     z.repmnemptr
#define  mnem0F00    z.mnem0F00ptr
#define  mnem0F01    z.mnem0F01ptr
#define  m387mem     z.m387memptr
#define  m387reg     z.m387regptr
#define  mnemD94     z.mnemD94ptr
#define  mnemDB4     z.mnemDB4ptr
#define  mnem0F      z.mnem0Fptr
#define  mnem0FBA    z.mnem0FBAptr
#define  escmnem     z.escmnemval
#define  popmnem     z.popmnemval
#define  popmnem_16  z.popmnem_16val
#define  popmnem_32  z.popmnem_32val
#define  segovmnem   z.segovmnemval
#define  lmnem       z.lmnemval
#define  movmnem     z.lmnemval
#define  stmnem      z.stmnemval
#define  illegmnem   z.illegmnemval
#define  timnem      z.timnemval
#define  testmnem    z.timnemval
#define  tibmnem     z.tibmnemval
#define  tiwmnem     z.tiwmnemval
#define  pushmnem    z.pushmnemval
#define  bswapmnem   z.bswapmnemval
#define  fabsmnem    z.fabsmnemval
#define  fnopmnem    z.fnopmnemval
#define  fcomppmnem  z.fcomppmnemval
#define  fucomppmnem z.fucomppmnemval
#define  fstswmnem   z.fstswmnemval
#define  fillegmnem  z.fillegmnemval
#define  masm        z.masmval

/******************************************************************************/
/****************************** disbyte ***************************************/
/******************************************************************************/
/*                                                                            */
/*  DESCRIPTION:                                                              */
/*              print the specified byte in hex, in the supplied buffer.      */
/*              If the first digit is 0, it is suppressed .                   */
/*                                                                            */
/*  PARAMETERS:                                                               */
/*              byte            Input:  current byte at instr stream          */
/*              *buff           Input:  buffer -> to hold the output          */
/*                                                                            */
/*  RETURNS:                                                                  */
/*              buffer ->       next position at the buffer                   */
/*                                                                            */
/*  GLOBAL DATA:                                                              */
/*              none                                                          */
/*                                                                            */
/******************************************************************************/
static UCHAR *disbyte(UCHAR byte, UCHAR *buff)
{
  *buff = (UCHAR)(((byte >> 4) & 0x0F) + '0');
  if (*buff != 0x30)
    {
      if (*buff > 0x39) *buff += 7;
      buff++;
    }
  *buff = (UCHAR)((byte & 0x0F) + '0');
  if (*buff > 0x39)
    *buff += 7;
  buff++;
  return(buff);
}
/******************************************************************************/
/****************************** hexbyte ***************************************/
/******************************************************************************/
/*                                                                            */
/*  DESCRIPTION:                                                              */
/*              print the specified byte in hex, in the supplied buffer.      */
/*                                                                            */
/*  PARAMETERS:                                                               */
/*              byte            Input:  current byte at instr stream          */
/*              *buff           Input:  buffer -> to hold the output          */
/*                                                                            */
/*  RETURNS:                                                                  */
/*              buffer ->       next position at the buffer                   */
/*                                                                            */
/*  GLOBAL DATA:                                                              */
/*              none                                                          */
/*                                                                            */
/******************************************************************************/
static UCHAR *hexbyte(UCHAR byte, UCHAR *buff)
{
  UCHAR c;
  c = (UCHAR)(((byte >> 4) & 0x0F) + '0');
  if (c > 0x39)
    {
      c += 7;
      #if defined(_ASM86_)
      if (masm)
        {
      #endif
                                        /*  for the MASM case, add a leading 0
                                        if the number starts with a letter    */
      *buff++ = '0';
      #if defined(_ASM86_)
        }
      #endif
    }
  *buff++ = c;
  *buff = (UCHAR)((byte & 0x0F) + '0');
  if (*buff > 0x39)
    *buff += 7;
  buff++;
  return(buff);
}
/******************************************************************************/
/****************************** hexdword **************************************/
/******************************************************************************/
/*                                                                            */
/*  DESCRIPTION:                                                              */
/*              print the specified dword in hex, in the supplied buffer.     */
/*                                                                            */
/*  PARAMETERS:                                                               */
/*              val             Input:  current dword at instr stream         */
/*              *buff           Input:  buffer -> to hold the output          */
/*                                                                            */
/*  RETURNS:                                                                  */
/*              buffer ->       next position at the buffer                   */
/*                                                                            */
/*  GLOBAL DATA:                                                              */
/*              none                                                          */
/*                                                                            */
/******************************************************************************/
static UCHAR *hexdword(ULONG val, UCHAR *buff)
{
  ULONG hi;
  UCHAR c, d;
  hi = 4;
  while (hi > 0)
    {
      hi--;
      d = (UCHAR)((val >> (hi * 8)) & 0xFF);
      c = (UCHAR)(((d >> 4) & 0x0F) + '0');
      if (c > 0x39)
         {
           c += 7;
           #if defined(_ASM86_)
           if (masm)
              {
           #endif
                                        /* for the MASM case, add a leading 0
                                        if the number starts with a letter    */
           if (hi == 3)
             *buff++ = '0';
           #if defined(_ASM86_)
              }
           #endif
         }
      *buff++ = c;
      *buff = (UCHAR)((d & 0x0F) + '0');
      if (*buff > 0x39)
        *buff += 7;
      buff++;
    }
  return(buff);
}
/******************************************************************************/
/****************************** hexword ***************************************/
/******************************************************************************/
/*                                                                            */
/*  DESCRIPTION:                                                              */
/*              print the specified word in hex, in the supplied buffer.      */
/*                                                                            */
/*  PARAMETERS:                                                               */
/*              val             Input:  current dword at instr stream         */
/*              *buff           Input:  buffer -> to hold the output          */
/*                                                                            */
/*  RETURNS:                                                                  */
/*              buffer ->       next position at the buffer                   */
/*                                                                            */
/*  GLOBAL DATA:                                                              */
/*              none                                                          */
/*                                                                            */
/******************************************************************************/
static UCHAR *hexword(ULONG val, UCHAR *buff)
{
  ULONG hi;
  UCHAR c, d;
  hi = 2;
  while (hi > 0)
    {
      hi--;
      d = (UCHAR)((val >> (hi * 8)) & 0xFF);
      c = (UCHAR)(((d >> 4) & 0x0F) + '0');
      if (c > 0x39)
        {
          c += 7;
          #if defined(_ASM86_)
          if (masm)
            {
          #endif
                                        /* for the MASM case, add a leading 0
                                        if the number starts with a letter    */
          if (hi == 1)
            *buff++ = '0';
          #if defined(_ASM86_)
            }
          #endif
        }
      *buff++ = c;
      *buff = (UCHAR)((d & 0x0F) + '0');
      if (*buff > 0x39)
        *buff += 7;
      buff++;
    }
  return(buff);
}
/******************************************************************************/
/****************************** printitem *************************************/
/******************************************************************************/
/*                                                                            */
/*  DESCRIPTION:                                                              */
/*              print the specified item from the table pointed by ptr        */
/*              item starts with index 0                                      */
/*                                                                            */
/*  PARAMETERS:                                                               */
/*              item            Input:  the item_th object in the table       */
/*              *ptr            Input:  the table pointer                     */
/*              *buff           Input:  output buffer pointer                 */
/*                                                                            */
/*  RETURNS:                                                                  */
/*              buffer ->       next position at the buffer                   */
/*                                                                            */
/*  GLOBAL DATA:                                                              */
/*              none                                                          */
/*                                                                            */
/******************************************************************************/
static UCHAR *printitem(ULONG item, char *ptr, UCHAR *buff)
{
  ULONG leng;
  while (item != 0)
    {
      ptr += (*ptr + 2);
      item--;
    }
  leng = *ptr++;
  ptr++;
  while (leng-- != 0)
    *buff++ = *ptr++;
  return(buff);
}
/******************************************************************************/
/****************************** printmnem *************************************/
/******************************************************************************/
/*                                                                            */
/*  DESCRIPTION:                                                              */
/*              prints a mnemonic - due to relic from the PLS version of this */
/*              code, the mnemonic numbers in the tables count from 1, not 0. */
/*              This routine corrects for this, rather than changing all the  */
/*              tables (which would be no small task)                         */
/*                                                                            */
/*  PARAMETERS:                                                               */
/*              mnem_num        Input:  the desired object in the table       */
/*                                                                            */
/*  RETURNS:                                                                  */
/*              none                                                          */
/*                                                                            */
/*  GLOBAL DATA:                                                              */
/*              none                                                          */
/*                                                                            */
/******************************************************************************/
static void printmnem(USHORT mnem_num)
{
  mbuff = printitem((ULONG)mnem_num-1, mnemstr, mbuff);
  return;
}
/******************************************************************************/
/****************************** print387m *************************************/
/******************************************************************************/
/*                                                                            */
/*  DESCRIPTION:                                                              */
/*              prints a 387 mnem - due to relic from the PLS version of this */
/*              code, the mnemonic numbers in the tables count from 1, not 0. */
/*              This routine corrects for this, rather than changing all the  */
/*              tables (which would be no small task)                         */
/*                                                                            */
/*  PARAMETERS:                                                               */
/*              mnem_num        Input:  the desired object in the table       */
/*                                                                            */
/*  RETURNS:                                                                  */
/*              none                                                          */
/*                                                                            */
/*  GLOBAL DATA:                                                              */
/*              none                                                          */
/*                                                                            */
/******************************************************************************/
static void print387m(USHORT mnem_num)
{
  mbuff = printitem((ULONG)mnem_num-1, mnem387, mbuff);
  return;
}
/******************************************************************************/
/****************************** prt8reg16 *************************************/
/******************************************************************************/
/*                                                                            */
/*  DESCRIPTION:                                                              */
/*              prints an 8, 16 or 32 bit register name, according to wbit    */
/*              and the current mode                                          */
/*                                                                            */
/*  PARAMETERS:                                                               */
/*              regnum          Input:  register number                       */
/*                                                                            */
/*  RETURNS:                                                                  */
/*              none                                                          */
/*                                                                            */
/*  GLOBAL DATA:                                                              */
/*              none                                                          */
/*                                                                            */
/******************************************************************************/
static void prt8reg16(UCHAR regnum)
{
  if (segsflag)
    {
                                        /* register is really a segment
                                           register                           */
      ibuff = printitem((ULONG)regnum,segreg,ibuff);
      segsflag = 0;
    }
  else
    {
                                        /* it is an ordinary register         */
      if (!wbit)
        {
          ibuff = printitem((ULONG)regnum,reg8,ibuff);
        }
      else
        {
          if (opsize32)
            ibuff = printitem((ULONG)regnum,reg32,ibuff);
          else
            ibuff = printitem((ULONG)regnum,reg16,ibuff);
        }
    }
  return;
}
/******************************************************************************/
/****************************** prtbyte ***************************************/
/******************************************************************************/
/*                                                                            */
/*  DESCRIPTION:                                                              */
/*              set ic to next byte of the instruction, and print it in hex   */
/*                                                                            */
/*  PARAMETERS:                                                               */
/*              none                                                          */
/*                                                                            */
/*  RETURNS:                                                                  */
/*              none                                                          */
/*                                                                            */
/*  GLOBAL DATA:                                                              */
/*              ic, hbuff                                                     */
/*                                                                            */
/******************************************************************************/
static void prtbyte()
{
  ic = *(parm->iptr)++;
  if (hbuff)
    {
      *hbuff = (UCHAR)(((ic >> 4) & 0x0F) + '0');
      if (*hbuff > 0x39)
        *hbuff += 7;
      hbuff++;
      *hbuff = (UCHAR)((ic & 0x0F) + '0');
      if (*hbuff > 0x39)
        *hbuff += 7;
      hbuff++;
    }
  return;
}
/******************************************************************************/
/****************************** prtdword **************************************/
/******************************************************************************/
/*                                                                            */
/*  DESCRIPTION:                                                              */
/*              set f1 to next dwoed of the instruction, and print it in hex  */
/*                                                                            */
/*  PARAMETERS:                                                               */
/*              none                                                          */
/*                                                                            */
/*  RETURNS:                                                                  */
/*              none                                                          */
/*                                                                            */
/*  GLOBAL DATA:                                                              */
/*              hbuff                                                         */
/*                                                                            */
/******************************************************************************/
static void prtdword()
{
  ULONG hi, val;
  UCHAR c;
  f1 = *(parm->iptr)++;
  f1 |= (*(parm->iptr)++ << 8);         /* note - the bytes are reversed!     */
  f1 |= (*(parm->iptr)++ << 16);        /* note - the bytes are reversed!     */
  f1 |= (*(parm->iptr)++ << 24);        /* note - the bytes are reversed!     */
  if (hbuff)
    {
                                        /* Print the instruction word in hex,
                                           byte reversed                      */
      hi = 0;
      val = f1;
      while (hi < 4)
        {
          c = (UCHAR)((val >> (hi * 8)) & 0xFF);
          *hbuff = (UCHAR)(((c >> 4) & 0x0F) + '0');
          if (*hbuff > 0x39)
            *hbuff += 7;
          hbuff++;
          *hbuff = (UCHAR)((c & 0x0F) + '0');
          if (*hbuff > 0x39)
            *hbuff += 7;
          hbuff++;
          hi++;
        }
    }
  return;
}
/******************************************************************************/
/****************************** prtovseg **************************************/
/******************************************************************************/
/*                                                                            */
/*  DESCRIPTION:                                                              */
/*              prints the override segment name, if it is set up             */
/*                                                                            */
/*  PARAMETERS:                                                               */
/*              none                                                          */
/*                                                                            */
/*  RETURNS:                                                                  */
/*              none                                                          */
/*                                                                            */
/*  GLOBAL DATA:                                                              */
/*              ibuff                                                         */
/*                                                                            */
/******************************************************************************/
static void prtovseg()
{
  if (ovseg)
    {
      ibuff = printitem((ULONG)ovseg-1, segreg, ibuff);
    }
  return;
}
/******************************************************************************/
/****************************** prtword ***************************************/
/******************************************************************************/
/*                                                                            */
/*  DESCRIPTION:                                                              */
/*              set f1 to next word of the instr, and print it in hex         */
/*                                                                            */
/*  PARAMETERS:                                                               */
/*              none                                                          */
/*                                                                            */
/*  RETURNS:                                                                  */
/*              none                                                          */
/*                                                                            */
/*  GLOBAL DATA:                                                              */
/*              hbuff                                                         */
/*                                                                            */
/******************************************************************************/
static void prtword()
{
  ULONG hi, val;
  UCHAR c;
  f1 = *(parm->iptr)++;
  f1 |= (*(parm->iptr)++ << 8);         /* note - the bytes are reversed!     */
  if (hbuff)
    {
      hi = 0;
      val = f1;
      while (hi < 2)
        {
          c = (UCHAR)((val >> (hi * 8)) & 0xFF);
          *hbuff = (UCHAR)(((c >> 4) & 0x0F) + '0');
          if (*hbuff > 0x39)
            *hbuff += 7;
          hbuff++;
          *hbuff = (UCHAR)((c & 0x0F) + '0');
          if (*hbuff > 0x39)
            *hbuff += 7;
          hbuff++;
          hi++;
        }
    }
  return;
}
/******************************************************************************/
/****************************** prtimmed **************************************/
/******************************************************************************/
/*                                                                            */
/*  DESCRIPTION:                                                              */
/*              to print the immediate operand to ibuff                       */
/*                                                                            */
/*  PARAMETERS:                                                               */
/*              none                                                          */
/*                                                                            */
/*  RETURNS:                                                                  */
/*              none                                                          */
/*                                                                            */
/*  GLOBAL DATA:                                                              */
/*              ibuff                                                         */
/*                                                                            */
/******************************************************************************/
static void prtimmed()
{
  if (!nocomma)
    *ibuff++ = ',';
  #if defined(_ASM86_)
  if (!masm)
    {
      *ibuff++ = 'X';
      *ibuff++ = '\'';
    }
  #endif
  if (wbit == 0)
    {
                                        /* a byte operand                     */
      prtbyte();
      ibuff = hexbyte(ic,ibuff);
    }
  else
    {
                                        /* a word or double word operand      */
      if (opsize32)
        {
                                        /* it is a double word instruction    */
          if (signext)
            {
                                        /* sign extend the (byte) operand     */
              prtbyte();
              f1 = (signed long) ((signed char) ic);
            }
          else
            prtdword();
          ibuff = hexdword(f1,ibuff);
        }
      else
        {
          if (signext)
            {
                                        /* sign extend the (byte) operand     */
              prtbyte();
              f1 = (signed long) ((signed char) ic);
            }
          else
            prtword();
          ibuff = hexword(f1,ibuff);
        }
    }
  #if defined(_ASM86_)
  if (masm)
  #endif
    *ibuff++ = 'H';
  #if defined(_ASM86_)
  else
    *ibuff++ = '\'';
  #endif
  return;
}
/******************************************************************************/
/****************************** putpar ****************************************/
/******************************************************************************/
/*                                                                            */
/*  DESCRIPTION:                                                              */
/*              outputs a left parenthesis (ASM/286) or left backet (MASM),   */
/*              and sets the flag, unless the flag is already set, in which   */
/*              case it outputs a comma (ASM/286) or plus (MASM)              */
/*                                                                            */
/*  PARAMETERS:                                                               */
/*              none                                                          */
/*                                                                            */
/*  RETURNS:                                                                  */
/*              none                                                          */
/*                                                                            */
/*  GLOBAL DATA:                                                              */
/*              ibuff                                                         */
/*                                                                            */
/******************************************************************************/
static void putpar()
{
  if (parflag == 0)
    {
                                        /* no parenthesis so far              */
      parflag = 1;
      #if defined(_ASM86_)
      if (masm)
      #endif
        *ibuff++ = '[';
      #if defined(_ASM86_)
      else
        *ibuff++ = '(';
      #endif
    }
  else
    {
                                        /* we have a parenthesis - output a
                                           comma                              */
      #if defined(_ASM86_)
      if (masm)
      #endif
        *ibuff++ = '+';
      #if defined(_ASM86_)
      else
        *ibuff++ = ',';
      #endif
    }
  return;
}
/******************************************************************************/
/****************************** putpar ****************************************/
/******************************************************************************/
/*                                                                            */
/*  DESCRIPTION:                                                              */
/*              print the specified string                                    */
/*                                                                            */
/*  PARAMETERS:                                                               */
/*              *str            Input:  string ->                             */
/*                                                                            */
/*  RETURNS:                                                                  */
/*              none                                                          */
/*                                                                            */
/*  GLOBAL DATA:                                                              */
/*              ibuff                                                         */
/*                                                                            */
/******************************************************************************/
static void putstr(char *str)
{
  while (*str != 0)
    *ibuff++ = *str++;
  return;
}
/******************************************************************************/
/****************************** r1 ********************************************/
/******************************************************************************/
/*                                                                            */
/*  DESCRIPTION:                                                              */
/*              init all parameters and assign buffer for output              */
/*                                                                            */
/*  PARAMETERS:                                                               */
/*              none                                                          */
/*                                                                            */
/*  RETURNS:                                                                  */
/*              none                                                          */
/*                                                                            */
/*  GLOBAL DATA:                                                              */
/*              hbuff, ibuff, mbuff, parm's                                   */
/*                                                                            */
/******************************************************************************/
static void r1()
{
  hbuff = parm->hbuffer;
  ibuff = parm->ibuffer;
  mbuff = parm->mbuffer;
  dbit = 0;
  parm->retbits = 0;
  parm->rettype = 0;
  parm->retreg = 0;
  parm->retseg = 0;
  parm->retoffset = 0;
  parm->retscale = 0;
  parm->retbase = 255;
  parm->retindex = 255;
  return;
}
/******************************************************************************/
/****************************** setdw *****************************************/
/******************************************************************************/
/*                                                                            */
/*  DESCRIPTION:                                                              */
/*              set the 'd' and 'w' bits from the instruction                 */
/*                                                                            */
/*  PARAMETERS:                                                               */
/*              none                                                          */
/*                                                                            */
/*  RETURNS:                                                                  */
/*              none                                                          */
/*                                                                            */
/*  GLOBAL DATA:                                                              */
/*                                                                            */
/*                                                                            */
/******************************************************************************/
static void setdw()
{
  wbit = (UCHAR)(instr & 1);            /* set 8/16/32 bit marker:
                                           0 => 8 bit, 1 => 16 or 32 bit      */
  dbit = (UCHAR)(instr & 2);            /* set direction bit:
                                               2 => mem -> regf
                                               0 => regf -> mem               */
  return;
}
/******************************************************************************/
/****************************** setrm *****************************************/
/******************************************************************************/
/*                                                                            */
/*  DESCRIPTION:                                                              */
/*              set trm-regf-mod from byte after instruction                  */
/*                                                                            */
/*  PARAMETERS:                                                               */
/*              none                                                          */
/*                                                                            */
/*  RETURNS:                                                                  */
/*              none                                                          */
/*                                                                            */
/*  GLOBAL DATA:                                                              */
/*                                                                            */
/*                                                                            */
/******************************************************************************/
static void setrm()
{
  prtbyte();
  rm = (UCHAR)(ic & 0x07);
  regf = (UCHAR)((UCHAR)(ic & 0x038) >> 3);    /* added uchar cast inside  521*/
  mod = (UCHAR)((UCHAR)(ic & 0xC0) >> 6);      /* added uchar cast inside  521*/
  sibbase5 = 0;
  if (addr32)
    {
                                        /* interpret this as a 32-bit instr   */
      if (mod != 3)
        basereg = (UCHAR)(rm + 1);
      indexreg = 0;
      scalefactor = 0;
      if (mod == 0 && rm == 5)
        basereg = 0;
      if (mod != 3 && rm == 4)
        {
                                        /* we have a SIB byte                 */
          prtbyte();
          basereg = (UCHAR)((ic & 0x07) + 1);
          indexreg = (UCHAR)(((UCHAR)(ic & 0x38) >> 3) + 1);             /*521*/
          scalefactor = (UCHAR)((UCHAR)(ic & 0xC0) >> 6);                /*521*/
          if (indexreg == 5)
            {
                                        /* index=4 -> no index                */
              indexreg = 0;
              scalefactor = 0;
            }
          if (mod == 0 && basereg == 6)
            {
                                        /* a special case, no base            */
              basereg = 0;
              sibbase5 = 1;             /* remember what we have done         */
            }
        }
        if (basereg == 6 ||             /* EBP                                */
          basereg == 5  )               /* ESP                                */
        defseg = 3;                     /* SS                                 */
    }
  else
    {
                                        /* interpret this as a 16-bit instr   */
       basereg = basereg16[rm];
       indexreg = indexreg16[rm];
       if (mod == 0 && rm == 6)
         basereg = 0;
       if (basereg == 6)                /* BP                                 */
         defseg = 3;                    /* SS                                 */
       scalefactor = 0;
    }
  return;
}
/******************************************************************************/
/****************************** DisAsm ****************************************/
/******************************************************************************/
/*                                                                            */
/*  DESCRIPTION:                                                              */
/*              main routine                                                  */
/*                                                                            */
/*  PARAMETERS:                                                               */
/*              parmptr              Input:  interface pointer                */
/*                                                                            */
/*  RETURNS:                                                                  */
/*              none                                                          */
/*                                                                            */
/*  GLOBAL DATA:                                                              */
/*                                                                            */
/*                                                                            */
/******************************************************************************/


void DisAsm (PARLIST * parmptr)         /* removed pascal                 521*/
{
  parm = parmptr;
  startiptr = parm->iptr;
  ovseg = 0;
  defseg = 4;                           /* DS                                 */
  flags.flagbytes = 0;
  if (parm->flagbits & use32mask)
    use32 = 1;                          /* 32-bit dis-assembly mode           */
  r1();

  restart:                              /* return to here if we find a segment
                                           override                           */
  parm->retbits = 0;
  opsize32 = use32 ^ opsizeover;
  if (opsize32)
    parm->retbits |= 1;
  addr32 = use32 ^ addrover;
  if (addr32)
    parm->retbits |= 2;
  #if defined(_ASM86_)
    if ((parm->flagbits & MASMmask) == 0)
      {
                                        /* ASM/86 decode                      */
      if (opsize32)
        p = (UCHAR *)&ASM286_32;
      else
        p = (UCHAR *)&ASM286_16;
      goto restart_1;
      }
  #endif
                                        /* use MASM                           */
  if (opsize32)
    p = (UCHAR *)&MASM_32;
  else
    p = (UCHAR *)&MASM_16;

  restart_1:

/*bcopy( p, &z, sizeof(z)); */          /* added by Joe C.                 504*/
  memcpy(&z, p, sizeof(z));             /* added by Joe C.                 504*/
  prtbyte();                            /* get next byte of instruction       */
  instr = ic;
                                        /* in many cases we can print the
                                           instruction mnemonic now           */
  if (mnemnum[instr] != odd)
      printmnem(mnemnum[instr]);
                                        /* jump according to the instruction
                                           opcode                             */
  switch (instr)
    {
      case   0:
      case   1:
      case   2:
      case   3:  goto memop;            /* 00 - 03                            */
      case   4:
      case   5:  goto aregimop;         /* 04, 05                             */
      case   6:
      case   7:  goto segop;            /* 06, 07                             */
      case   8:
      case   9:
      case  10:
      case  11:  goto memop;            /* 08 - 0B                            */
      case  12:
      case  13:  goto aregimop;         /* 0C, 0D                             */
      case  14:  goto segop;            /* 0E                                 */
      case  15:  goto zerofop;          /* 0F                                 */
      case  16:
      case  17:
      case  18:
      case  19:  goto memop;            /* 10 - 13                            */
      case  20:
      case  21:  goto aregimop;         /* 14, 15                             */
      case  22:
      case  23:  goto segop;            /* 16, 17                             */
      case  24:
      case  25:
      case  26:
      case  27:  goto memop;            /* 18 - 1B                            */
      case  28:
      case  29:  goto aregimop;         /* 1C, 1D                             */
      case  30:
      case  31:  goto segop;            /* 1E, 1F                             */
      case  32:
      case  33:
      case  34:
      case  35:  goto memop;            /* 20 - 23                            */
      case  36:
      case  37:  goto aregimop;         /* 24, 25                             */
      case  38:  goto segovop;          /* 26                                 */
      case  39:  goto nulop;            /* 27                                 */
      case  40:
      case  41:
      case  42:
      case  43:  goto memop;            /* 28 - 2B                            */
      case  44:
      case  45:  goto aregimop;         /* 2C, 2D                             */
      case  46:  goto segovop;          /* 2E                                 */
      case  47:  goto nulop;            /* 2F                                 */
      case  48:
      case  49:
      case  50:
      case  51:  goto memop;            /* 30 - 33                            */
      case  52:
      case  53:  goto aregimop;         /* 34, 35                             */
      case  54:  goto segovop;          /* 36                                 */
      case  55:  goto nulop;            /* 37                                 */
      case  56:
      case  57:
      case  58:
      case  59:  goto memop;            /* 38 - 3B                            */
      case  60:
      case  61:  goto aregimop;         /* 3C, 3D                             */
      case  62:  goto segovop;          /* 3E                                 */
      case  63:  goto nulop;            /* 3F                                 */
      case  64:
      case  65:
      case  66:
      case  67:
      case  68:
      case  69:
      case  70:
      case  71:
      case  72:
      case  73:
      case  74:
      case  75:
      case  76:
      case  77:
      case  78:
      case  79:
      case  80:
      case  81:
      case  82:
      case  83:
      case  84:
      case  85:
      case  86:
      case  87:
      case  88:
      case  89:
      case  90:
      case  91:
      case  92:
      case  93:
      case  94:
      case  95:  goto reg16op;          /* 40 - 5F                            */
      case  96:
      case  97:  goto nulop;            /* 60, 61                             */
      case  98:  goto boundop;          /* 62                                 */
      case  99:  goto ARPLop;           /* 63                                 */
      case 100:
      case 101:  goto segovfsgs;        /* 64, 65                             */
      case 102:  goto overoper;         /* 66                                 */
      case 103:  goto overaddr;         /* 67                                 */
      case 104:  goto pushiop;          /* 68                                 */
      case 105:  goto MSIop;            /* 69                                 */
      case 106:  goto pushiop2;         /* 6A                                 */
      case 107:  goto MSIop2;           /* 6B                                 */
      case 108:
      case 109:
      case 110:
      case 111:  goto nulop;            /* 6C - 6F                            */
      case 112:
      case 113:
      case 114:
      case 115:
      case 116:
      case 117:
      case 118:
      case 119:
      case 120:
      case 121:
      case 122:
      case 123:
      case 124:
      case 125:
      case 126:
      case 127:  goto jdispop;          /* 70 - 7F                            */
      case 128:
      case 129:  goto memimop;          /* 80, 81                             */
      case 130:  goto memimop;          /* 82                              526*/
      case 131:  goto memimop2;         /* 83                                 */
      case 132:
      case 133:
      case 134:
      case 135:
      case 136:
      case 137:
      case 138:
      case 139:  goto memop;            /* 84 - 8B                            */
      case 140:  goto memsegop;         /* 8C                                 */
      case 141:  goto laddrop;          /* 8D                                 */
      case 142:  goto memsegop;         /* 8E                                 */
      case 143:  goto mempopop;         /* 8F                                 */
      case 144:  goto nulop;            /* 90                                 */
      case 145:
      case 146:
      case 147:
      case 148:
      case 149:
      case 150:
      case 151:  goto aregxop;          /* 91 - 97                            */
      case 152:
      case 153:  goto nulop;            /* 98, 99                             */
      case 154:  goto c32op;            /* 9A                                 */
      case 155:
      case 156:
      case 157:
      case 158:
      case 159:  goto nulop;            /* 9B - 9F                            */
      case 160:
      case 161:
      case 162:
      case 163:  goto AXmemop;          /* A0 - A3                            */
      case 164:
      case 165:
      case 166:
      case 167:  goto lostrop;          /* A4 - A7                            */
      case 168:
      case 169:  goto aregimop;         /* A8, A9                             */
      case 170:
      case 171:  goto stoscanop;        /* AA, AB                             */
      case 172:
      case 173:  goto lostrop;          /* AC, AD                             */
      case 174:
      case 175:  goto stoscanop;        /* AE, AF                             */
      case 176:
      case 177:
      case 178:
      case 179:
      case 180:
      case 181:
      case 182:
      case 183:
      case 184:
      case 185:
      case 186:
      case 187:
      case 188:
      case 189:
      case 190:
      case 191:  goto regimop;          /* B0 - BF                            */
      case 192:
      case 193:  goto shiftop;          /* C0, C1                             */
      case 194:  goto retop;            /* C2                                 */
      case 195:  goto ret0op;           /* C3                                 */
      case 196:
      case 197:  goto laddrop;          /* C4, C5                             */
      case 198:
      case 199:  goto MVImemop;         /* C6, C7                             */      case 200:  goto enterop;          /* C8                                 */
      case 201:  goto nulop;            /* C9                                 */
      case 202:  goto retop;            /* CA                                 */
      case 203:  goto retfop;           /* CB                                 */
      case 204:  goto int3op;           /* CC                                 */
      case 205:  goto intop;            /* CD                                 */
      case 206:  goto intoop;           /* CE                                 */
      case 207:  goto intretop;         /* CF                                 */
      case 208:
      case 209:
      case 210:
      case 211:  goto shiftop;          /* D0 - D3                            */
      case 212:
      case 213:  goto AAop;             /* D4, D5                             */
      case 214:  goto illegop;          /* D6                                 */
      case 215:  goto xlatop;           /* D7                                 */
      case 216:
      case 217:
      case 218:
      case 219:
      case 220:
      case 221:
      case 222:
      case 223:  goto escapeop;         /* D8 - DF                            */
      case 224:
      case 225:
      case 226:
      case 227:  goto jdispop;          /* E0 - E3                            */
      case 228:
      case 229:
      case 230:
      case 231:  goto IOimop;           /* E4 - E7                            */
      case 232:  goto c16op;            /* E8                                 */
      case 233:  goto j16op;            /* E9                                 */
      case 234:  goto j32op;            /* EA                                 */
      case 235:  goto jdispop;          /* EB                                 */
      case 236:
      case 237:
      case 238:
      case 239:  goto IODXop;           /* EC - EF                            */
      case 240:  goto nulop;            /* F0                                 */
      case 241:  goto illegop;          /* F1                                 */
      case 242:
      case 243:  goto repop;            /* F2, F3                             */
      case 244:
      case 245:  goto nulop;            /* F4, F5                             */
      case 246:
      case 247:  goto memF6F7op;        /* F6, F7                             */
      case 248:
      case 249:
      case 250:
      case 251:
      case 252:
      case 253:  goto nulop;            /* F8 - FD                            */
      case 254:
      case 255:  goto FEFFop;           /* FE, FF                             */
  }
                                        /* D4, D5                             */
AAop:                                   /* AAD and AAM instructions           */
  prtbyte();
  if (ic != 0x0A)
    goto illegop;
                                        /* second byte is @0A                 */
  printmnem(mnemAA[instr-0xD4]);        /* for these instructions, we
                                            have not printed the mnemonic     */
  goto exit;
                                        /*  04, 05, 0C, 0D
                                            14, 15, 1C, 1D
                                            24, 25, 2C, 2D
                                            34, 35, 3C, 3D
                                            A8, A9                            */
aregimop:                               /* operations between AX/AL (EAX/AL in
                                           32-bit mode) and immediate operands*/
  wbit = (UCHAR)(instr & 1);            /* set 8/16/32 bit marker:
                                           0 => 8 bit, 1 => 16/32 bit.        */
  regf = 0;                             /* the register is EAX/AX/AL          */
aregiml1:                               /* jump to here for other
                                           register/immediate ops             */
  prt8reg16(regf);
  *ibuff++ = ',';
  #if defined(_ASM86_)
    if (!masm)
      {
        *ibuff++ = 'X';
        *ibuff++ = '\'';
        quoteflag = 1;
      }
  #endif

/* aregiml2:                                                                  */

  parm->retreg = regf;
  if (!wbit)
    parm->retreg += 16;
  else
    {
      if (opsize32)
        parm->retreg += 8;
    }
  parm->rettype = regimmedtype;
  if (!wbit)
    {
      prtbyte();                        /* get next byte of instruction       */
      ibuff = hexbyte(ic,ibuff);
      parm->retimmed = ic;
    }
  else
    {
                                        /* set f1 to next word (opsize32 = 0)
                                        or double word (orsize32 = 1) of the
                                        instruction, and print it in hex      */
      if (opsize32)
        {
          prtdword();
          ibuff = hexdword(f1,ibuff);
        }
      else
        {
          prtword();
          ibuff = hexword(f1,ibuff);
        }
      parm->retimmed = f1;
    }

  #if defined(_ASM86_)
    if (masm)
  #endif
      *ibuff++ = 'H';
  #if defined(_ASM86_)
    else
      {
        if (quoteflag)
          *ibuff++ = '\'';
      }
  #endif
  goto exit;
                                        /* 91 - 97                            */
aregxop:                                /* XCHG (E)AX,rr orders               */
  if (opsize32)
    ibuff = printitem((ULONG)0,reg32,ibuff);
  else
    ibuff = printitem((ULONG)0,reg16,ibuff);
  *ibuff++ = ',';
  goto reg16op;
                                        /* 63                                 */
ARPLop:                                 /* ARPL                               */
  dbit = 0;                             /* register is second operand         */
  wbit = 1;                             /* word operand                       */
  opsize32 = 0;                         /* this is always a 16-bit operation  */
  parm->retbits &= 0xFFFE;
  setrm();
  goto memop0;
                                        /* A0 - A3                            */
AXmemop:                                /* single byte MOV orders, with 16-bit
                                           displacement                       */
  setdw();
  #if defined(_ASM86_)
    if (masm)
  #endif
      dbit = (UCHAR)(dbit ^ 2);
  #if defined(_ASM86_)
    else                                /* force the register to be the first */
      dbit = 2;                         /* operand for these instructions     */
  #endif
  if (addr32)
    {
      prtdword();
      opdisp = f1;
    }
  else
    {
      prtword();
      opdisp = f1;
    }
  regf = 0;
  mod = 0;
  rm = 6;
  basereg = 0;
  indexreg = 0;
  scalefactor = 0;
  disppres = 1;
  goto memop2;
                                        /* 62                                 */
boundop:                                /* BOUND                              */
  dbit = 2;                             /* register is first operand          */
  wbit = 1;                             /* word operation                     */
  setrm();
  if (mod == 3)
    goto illegop;                       /* illegal if second operand
                                           would be a register                */
  parm->rettype = BOUNDtype;
  goto memop0;
                                        /* C8                                 */
enterop:                                /* procedure ENTER instruction        */
  wbit = 1;                             /* a word immediate operand           */
  nocomma = 1;                          /* no leading comma for operand field */
  opsize32 = 0;                         /* this is always a 16-bit operation  */
  prtimmed();
  wbit = 0;                             /* then a byte immediate operand      */
  nocomma = 0;                          /* leading comma now wanted           */
  prtimmed();
  goto exit;
                                        /* D8 - DF                            */
escapeop:                               /* escape orders - for 287/387        */
  setrm();
  wbit = 1;                             /* force a word operation             */
  if (mod == 3)
    {
      parm->rettype = escapetype;
      parm->retescape = (USHORT)(((((USHORT)(instr & 0x07) << 3) + regf) << 3) + rm);
    }                                   /* added ushort cast               521*/
  else
    {
      parm->rettype = escmemtype;
      parm->retescape = (USHORT)(((USHORT)(instr & 0x07) << 3) + regf);
    }                                   /* added ushort cast               521*/
  if ((parm->flagbits & N387mask) == 0)
    {
                                        /* do not perform 287/387 decode      */
    printmnem(escmnem);                 /* ESCAPE/ESC                         */
    dbit = 251;                         /* a special case                     */
    goto memop0;
  }
                                        /* we are to perform 287/387 decode   */
  ESCinstr = (UCHAR)(((UCHAR)(instr & 7) << 3) + regf);
                                        /* added uchar cast                521*/
  if (mod != 3)
  {
                                        /* it is an operation that accesses
                                           memory                             */
    if (m387mem[ESCinstr] == fillegmnem)
      goto illegop;
                                        /* trap the illegal cases             */
    print387m(m387mem[ESCinstr]);
    dbit = 249;                         /* so only the memory operand is
                                           printed                            */
    goto memop0;
  }
                                        /* it is a non-memory operation - jump
                                           according to opcode                */
  switch (ESCinstr)
    {
     case  0:
     case  1:  goto ESCreg1;            /* D8, 0,1                            */
     case  2:
     case  3:  goto ESCreg0;            /* D8, 2,3                            */
     case  4:
     case  5:
     case  6:
     case  7:  goto ESCreg1;            /* D8, 4-7                            */
     case  8:
     case  9:  goto ESCreg0;            /* D9, 0,1                            */
     case 10:  goto ESCD92;             /* D9, 2                              */
     case 11:  goto ESCreg0;            /* D9, 3                              */
     case 12:
     case 13:
     case 14:
     case 15:  goto ESCD94;             /* D9, 4-7                            */
     case 16:
     case 17:
     case 18:
     case 19:
     case 20:  goto illegop;            /* DA, 0-4                            */
     case 21:  goto ESCDA5;             /* DA, 5                              */
     case 22:
     case 23:  goto illegop;            /* DA, 6,7                            */
     case 24:
     case 25:
     case 26:
     case 27:  goto illegop;            /* DB, 0-3                            */
     case 28:  goto ESCDB4;             /* DB, 4                              */
     case 29:
     case 30:
     case 31:  goto illegop;            /* DB, 5-7                            */
     case 32:
     case 33:  goto ESCreg2;            /* DC, 0,1                            */
     case 34:
     case 35:  goto ESCreg0;            /* DC, 2,3                            */
     case 36:
     case 37:
     case 38:
     case 39:  goto ESCreg2;            /* DC, 4-7                            */
     case 40:
     case 41:
     case 42:
     case 43:
     case 44:
     case 45:  goto ESCreg0;            /* DD, 0-5                            */
     case 46:
     case 47:  goto illegop;            /* DD, 6,7                            */
     case 48:
     case 49:  goto ESCreg2;            /* DE, 0,1                            */
     case 50:  goto ESCreg0;            /* DE, 2                              */
     case 51:  goto ESCDE3;             /* DE, 3                              */
     case 52:
     case 53:
     case 54:
     case 55:  goto ESCreg2;            /* DE, 4-7                            */
     case 56:
     case 57:
     case 58:
     case 59:  goto ESCreg0;            /* DF, 0-3                            */
     case 60:  goto ESCDF4;             /* DF, 4                              */
     case 61:
     case 62:
     case 63:  goto illegop;            /* DF, 5-7                            */
  }
                                        /* instructions with a reg field      */
ESCreg0:  dbit = 0;                     /* no ST operand                      */
  goto ESCreg;
ESCreg1:  dbit = 1;                     /* ST is the first operand            */
  goto ESCreg;
ESCreg2:  dbit = 2;                     /* ST is the second operand           */
ESCreg:  print387m(m387reg[ESCinstr]);
  if (dbit == 1)
    {
                                        /* print ST as the first operand      */
      *ibuff++ = 'S';
      *ibuff++ = 'T';
      *ibuff++ = ',';
    }
  *ibuff++ = 'S';
  *ibuff++ = 'T';
  *ibuff++ = '(';
  ibuff = disbyte(rm, ibuff);           /* print register number              */
  *ibuff++ = ')';
  if (dbit == 2)
    {
                                        /* print ST as the second operand     */

      *ibuff++ = ',';
      *ibuff++ = 'S';
      *ibuff++ = 'T';
    }
  goto exit;
                                        /* the following are special cases    */
ESCD92:                                 /* D9 op code, mod=3, regf=2          */
  if (rm == 0)
    {
      rm = (UCHAR)(fnopmnem);

ESCnoopand:  print387m(rm);
      goto exit;
    }
  goto illegop;

ESCD94:                                 /* D9 op code, mod = 3, regf = 4-7    */
                                        /* jump according to the mod and reg
                                           field                              */
  rm = (UCHAR)(((regf-4) << 3) + rm);
  switch (rm)
    {
      case  0:
      case  1:  goto ESCD95;            /* regf=4, rm=0,1                     */
      case  2:
      case  3:  goto illegop;           /* regf=4, rm=2,3                     */
      case  4:
      case  5:  goto ESCD95;            /* regf=4, rm=4,5                     */
      case  6:
      case  7:  goto illegop;           /* regf=4, rm=6,7                     */
      case  8:
      case  9:
      case 10:
      case 11:
      case 12:
      case 13:
      case 14:  goto ESCD95;            /* regf=5, rm=0-6                     */
      case 15:  goto illegop;           /* regf=5, rm=7                       */
      case 16:
      case 17:
      case 18:
      case 19:
      case 20:
      case 21:
      case 22:
      case 23:  goto ESCD95;            /* regf=6, rm=0-7                     */
      case 24:
      case 25:
      case 26:
      case 27:
      case 28:
      case 29:
      case 30:
      case 31:  goto ESCD95;            /* regf=7, rm=0-7                     */
  }
ESCD95:  rm = (UCHAR)(mnemD94[rm]);
  goto ESCnoopand;
ESCDA5:  if (rm != 1) goto illegop;
  rm = (UCHAR)(fucomppmnem);
  goto ESCnoopand;
ESCDB4:                                 /* DB op code, mod = 3, regf = 4      */
                                        /* jump according to the mod and reg
                                           field                              */
  rm = (UCHAR)(((regf-4) << 3) + rm);
  if (rm < 5)                           /* regf=4, rm=0-4                     */
    goto ESCDB4a;
  else                                  /* regf=4, rm=5-7                     */
    goto illegop;
ESCDB4a:  rm = (UCHAR)(mnemDB4[rm]);
  goto ESCnoopand;
ESCDE3:                                 /* DE op code, mod=3, regf=3          */
  if (rm == (UCHAR)(fabsmnem))
    {
      rm = (UCHAR)(fcomppmnem);
      goto ESCnoopand;
    }
  goto illegop;
ESCDF4:                                 /* DF op code, mod=3, regf=4          */
  if (rm == 0)
    {
      print387m(fstswmnem);
      ibuff = printitem((ULONG)0, reg16, ibuff);
      goto exit;
    }
  goto illegop;
                                        /* FE, FF                             */
FEFFop:                                 /* miscellaneous operations:  all
                                            have mod/rm byte
                                            and possible displacement bytes   */
  setdw();
  setrm();
  if (regf == 0 || regf == 1)
    goto F6F7l1;
  if (instr == 0xFE || regf == 7)
    goto illegop;
  dbit = 254;                           /* no second operand                  */
  wbit = 1;                             /* force a word operation             */
  if (regf == 6)
    printmnem(pushmnem);
  else
    {
      if (mod == 3)
        {
          if (regf == 3 || regf == 5)
            goto illegop;
          if (regf == 2 )
            parm->rettype = cnearregtype;
          else                          /* regf = 4                           */
          parm->rettype = jnearregtype;
        }
      else
        {
          if (regf == 2)
            parm->rettype = cnearmemtype;
          if (regf == 3)
            parm->rettype = cfartype;
          if (regf == 4)
            parm->rettype = jnearmemtype;
          if (regf == 5)
            parm->rettype = jfartype;
        }
      printmnem(mnemFF[regf - 2]);
    }
  goto memop0;
                                        /* illegal instructions               */
illegop:
  parm->iptr = startiptr + 1;
  printmnem(illegmnem);
  parm->rettype = illegtype;
  goto exit;
                                        /* CC                                 */
int3op:                                 /* INT3  instruction                  */
  ic = 3;
  #if defined(_ASM86_)
    if (!masm)
      goto intop_2;
  #endif
  goto intop_1;
                                        /* CE                                 */
intoop:                                 /* INTO  instruction                  */
  ic = 4;
  goto intop_2;
                                        /* CD                                 */
intop:                                  /* INT n instructions                 */
  prtbyte();                            /* get next byte of instruction       */

intop_1:
  #if defined(_ASM86_)
    if (!masm)
      {
        *ibuff++ = 'X';
        *ibuff++ = '\'';
        quoteflag = 1;
      }
  #endif
  ibuff = hexbyte(ic,ibuff);
  #if defined(_ASM86_)
    if (masm)
  #endif
      *ibuff++ = 'H';
  #if defined(_ASM86_)
    else
      {
        if (quoteflag)
          *ibuff++ = '\'';
      }
  #endif

intop_2:
  parm->retoffset = ic;
  parm->rettype = intntype;
  goto exit;
                                        /* CF                                 */
intretop:                               /* IRET  instruction                  */
  parm->rettype = intrettype;
  parm->retoffset = 0;
  goto exit;
                                        /* EC - EF                            */
IODXop:                                 /* IN DX and OUT DX instructions      */
  IODX = 1;                             /* mark it as DX type                 */
IODXl1:
  setdw();

  #if defined(_ASM86_)
    if (masm)
      {
  #endif
        if (dbit == 0)
          {
             prt8reg16(0);
             *ibuff++ = ',';
          }
  #if defined(_ASM86_)
      }
    else
      {
        if (!IODX)
          {
            *ibuff++ = 'X';
            *ibuff++ = '\'';
            quoteflag = 1;
          }
      }
  #endif
  if (IODX)
    ibuff = printitem((ULONG)2,reg16,ibuff);
    else
      {
        ibuff = hexbyte(ic,ibuff);
        #if defined(_ASM86_)
          if (masm)
            {
        #endif
        *ibuff++ = 'H';
        if (dbit != 0)
          {
            *ibuff++ = ',';
            prt8reg16(0);          }
         #if defined(_ASM86_)
            }
          else
            {
              if (quoteflag)
                *ibuff++ = '\'';
            }
         #endif

      }
  goto exit;
                                        /* E4 - E7                            */
IOimop:                                 /* IN xx and OUT xx instructions      */
  prtbyte();
  goto IODXl1;
                                        /* E8                                 */
c16op:                                  /* single byte call instructions with
                                           16 bit relative displacements      */
  parm->rettype = creltype;
  goto j16op0;
                                        /* E9                                 */
j16op:                                  /* single byte jump instructions with
                                           16 bit relative displacements      */
  parm->rettype = jreltype;
j16op0:
  if (opsize32)
    prtdword();
  else
    prtword();
  if (parm->instr_EIP == 0xFFFFFFFF)
    *ibuff++ = '+';
  #if defined(_ASM86_)
    if (!masm)
      {
        *ibuff++ = 'X';
        *ibuff++ = '\'';
        quoteflag = 1;
      }
  #endif
  if (!opsize32)
    f1 = (signed long) ((short) f1);
                                        /* sign extend the lower 16 bits      */
  if (parm->instr_EIP == 0xFFFFFFFF)
    {
      if (opsize32)
        ibuff = hexdword(f1,ibuff);
      else
        {
          ibuff = hexword(f1,ibuff);
        }
    }
  else
    {
                                        /* we have an EIP value - print
                                           resultant address                  */
    f2 = (parm->iptr - startiptr) + parm->instr_EIP + f1;
    if (opsize32)
      ibuff = hexdword(f2,ibuff);
    else
      ibuff = hexword(f2,ibuff);
    }
j16op1:
  #if defined(_ASM86_)
    if (masm)
  #endif
      *ibuff++ = 'H';
  #if defined(_ASM86_)
    else
      {
        if (quoteflag)
          *ibuff++ = '\'';
      }
  #endif
  parm->retoffset = f1;
  goto exit;
                                        /* 9A                                 */
c32op:                                  /* call instructions with 32-bit
                                           operands                           */
  parm->rettype = cfarimmtype;
  goto j32op_1;
                                        /* EA                                 */
j32op:                                  /* jump instructions with 32-bit
                                           operands                           */
  parm->rettype = jfarimmtype;
j32op_1:                                /* get new (E)IP value                */
  if (opsize32)
    prtdword();
  else
    prtword();
  f2 = f1;                              /* save it so the next line does not
                                           overwrite it                       */
  prtword();                            /* get new CS value                   */
  #if defined(_ASM86_)
    if (!masm)
      {
        *ibuff++ = 'X';
        *ibuff++ = '\'';
        quoteflag = 1;
      }
  #endif
  ibuff = hexword(f1,ibuff);
  #if defined(_ASM86_)
    if (masm)
  #endif
      *ibuff++ = 'H';
  #if defined(_ASM86_)
    else
      {
        if (quoteflag)
          *ibuff++ = '\'';
      }
  #endif
  *ibuff++ = ':';
  #if defined(_ASM86_)
    if (!masm)
      {
        *ibuff++ = 'X';
        *ibuff++ = '\'';
        quoteflag = 1;
      }
  #endif
  if (opsize32)
    ibuff = hexdword(f2,ibuff);
  else
    ibuff = hexword(f2,ibuff);
  #if defined(_ASM86_)
    if (masm)
  #endif
      *ibuff++ = 'H';
  #if defined(_ASM86_)
    else
      {
        if (quoteflag) *ibuff++ = '\'';
      }
  #endif
  parm->retoffset = f2;
  parm->retseg = (USHORT)f1;
  goto exit;
                                        /* 70 - 7F, E0 - E3, EB               */
jdispop:                                /* single byte jump instructions with
                                           single byte signed displacements   */
  prtbyte();
  if (ic > 127)
    {
                                        /* the displacement is negative       */
      if (parm->instr_EIP == 0xFFFFFFFF)
        *ibuff++ = '-';
      ic = (UCHAR)(256 - ic);
      f1 = 0 - ic;
    }
  else
    {
      if (parm->instr_EIP == 0xFFFFFFFF)
        *ibuff++ = '+';
      f1 = ic;
    }
  #if defined(_ASM86_)
    if (!masm)
      {
        *ibuff++ = 'X';
        *ibuff++ = '\'';
        quoteflag = 1;
      }
  #endif
  if (parm->instr_EIP == 0xFFFFFFFF)
    {
      ibuff = hexbyte(ic,ibuff);
    }
  else
    {
                                        /* we have an EIP value - print
                                           resultant address                  */
jdisp_1:
    f2 = (parm->iptr - startiptr) + parm->instr_EIP + f1;
jdisp_2:
    if (opsize32)
      ibuff = hexdword(f2,ibuff);
    else
      ibuff = hexword(f2,ibuff);
    }
  parm->rettype = jreltype;
  goto j16op1;
                                        /* 0F 80-8F                           */
jfulldispop:                            /* two byte jump instructions with
                                           full signed displacements          */
  if (opsize32)
    {
                                        /* there is a 4-byte operand          */
      prtdword();
    }
  else
    {
                                        /* there is a 2-byte operand          */
      prtword();
      f1 = (signed long) ((short) f1);  /* sign extend the lower 16 bits      */
    }
  f2 = f1;
  if (f1 > 0x7FFFFFFF)
    {
                                        /* the displacement is negative       */
    if (parm->instr_EIP == 0xFFFFFFFF)
      *ibuff++ = '-';
    f2 = 0 - f2;
    }
  else
    {
      if (parm->instr_EIP == 0xFFFFFFFF)
        *ibuff++ = '+';
    }
  #if defined(_ASM86_)
    if (!masm)
      {
        *ibuff++ = 'X';
        *ibuff++ = '\'';
        quoteflag = 1;
      }
  #endif
  if (parm->instr_EIP != 0xFFFFFFFF)
    goto jdisp_1;
  goto jdisp_2;
                                        /* 8D, C4, C5                         */
laddrop:                                /* load address - also LES and LDS    */
  setrm();
  dbit = 2;                             /* register is first operand          */
  wbit = 1;                             /* force a word operation             */
  if (instr != 0x8D)
    parm->rettype = memwwtype;
  else
    parm->rettype = LEAtype;
  goto memop0;
                                        /* 0F 02,03                           */
LARop:                                  /* load access rights                 */
  dbit = 2;                             /* register is first operand          */
  wbit = 1;                             /* word operation                     */
  goto memop00;
                                        /* AC, AD                             */
lostrop:                                /* load string orders                 */
  if (ovseg == 0)
    parm->retreg = defseg;
  else
    parm->retreg = ovseg;
  if (addr32)
    parm->retbase = 14;                 /* ESI                                */
  else
    parm->retbase = 6;                  /* SI                                 */
  goto stscl1;
                                        /* F6, F7                             */
memF6F7op:                              /* further memory operations:  all have
                                           mod/rm byte and possible displacement
                                           or immediate operand               */
  setdw();
  setrm();
  if (regf == 1)
    goto illegop;
  if (regf == 0)
    {
                                        /* it is TIB/TIW(TEST) mem,xx         */
      dbit = 255;
      #if defined(_ASM86_)
        f1 = timnem;
        if (mod != 3)
          {
            if (wbit == 0)
              f1 = tibmnem;
            else
              f1 = tiwmnem;
          }
        if (masm)
      #endif
      f1 = testmnem;
      printmnem((USHORT)f1);
      goto memop0;
    }
                                        /* other versions of these operations
                                           have only one operand              */

F6F7l1:
  dbit = 254;
  #if defined(_ASM86_)
    f1 = wbit * 8;
    if (mod == 3)
      f1 = 16;
    if (masm)
  #endif
  f1 = 0;
  printmnem(mnemF6F7[regf + f1]);
  goto memop0;
                                        /* 82, 83                             */
memimop2:                               /* as memimop, but word operations have
                                           8-bit sign extended immediate
                                           operands (i.e. not a 16-bit
                                           operand)                           */
  setdw();                              /* d bit is not significant in this
                                           case                               */
  setrm();
  if (wbit != 0)
  signext = 1;                          /* if a word order, the immediate
                                           operand is 8 bit sign extended     */
  goto memimopa;
                                        /* 80, 81                             */
memimop:                                /* a single byte instruction with
                                           mem-regf-r/m, followed by 1 or 2
                                           byte immediate operand:
                                           the regf field further defines the
                                           operation                          */
  setdw();                              /* d bit is not significant in this
                                           case                               */
  setrm();
memimopa:
  dbit = 255;
  f2 = (ULONG)(instr & 1) << 3;         /* added ulong cast                521*/
  if (mod == 3) f2 = 16;
  printmnem(mnem8081[f2 + regf]);
  goto memop0;                                        /*  00 - 03, 08 - 0B
                                            10 - 13, 18 - 1B
                                            20 - 23, 28 - 2B
                                            30 - 33, 38 - 3B
                                            84 - 8B                           */

memop:                                  /* single byte instructions with
                                           mem-regf-r/m byte plus possible
                                           displacement bytes                 */
  setdw();
memop00:
  setrm();
  #if defined(_ASM86_)
    if (!masm && (instr == 0x88 || instr == 0x89))
      {
                                        /* bodge the d bit for these (STore)
                                           instructions                       */
        dbit = 2;
      }
  #endif
                                        /* look for operand displacement      */
memop0:
  opdisp = 0;
  signeddisp = 0;
  if ((mod == 2) || ((mod == 0) && (rm == 6) && !addr32) ||
       ((mod == 0) && (rm == 5) && addr32) ||
       (addr32 && mod == 0 && sibbase5))
    {
                                        /* we have a displacement             */
      if (addr32)
        prtdword();
      else
        prtword();
      opdisp = f1;
      disppres = 1;
    }
  if (mod == 1)
    {
                                        /* we have 8-bit signed displacement  */
      prtbyte();
      signedflag = 1;
      opdisp = (signed long) ((signed char) ic);
                                        /* sign extend operand                */
      signeddisp = ic;
      if (ic > 127)
        {

                                        /* it is negative                     */
          negflag = 1;
          signeddisp = 256 - ic;
        }
      disppres = 1;
    }

                                        /* apart from the above cases, there
                                           are no displacement bytes          */
memop2:
  if (mod != 3)
    {
      if (disppres)
        parm->retoffset = opdisp;
      if (parm->rettype == 0)
        {
          if (wbit == 0)
            parm->rettype = membtype;
          else
            parm->rettype = memwtype;
        }
      if (basereg != 0)
        {
          parm->retbase = (UCHAR)(basereg - 1);
          if (addr32)
            parm->retbase = (UCHAR)(parm->retbase + 8);
        }
      if (indexreg != 0)
        {
          parm->retindex = (UCHAR)(indexreg - 1);
          if (addr32)
            parm->retindex = (UCHAR)(parm->retindex + 8);
            parm->retscale = scalefactor;
        }
      if (ovseg == 0)
        parm->retreg = defseg;
      else
        parm->retreg = ovseg;
    }
  if (dbit == 2 || dbit == 245 || dbit == 246)
    {
                                        /* register is first operand - print
                                           register name                      */
      if (dbit == 245 || dbit == 246)
        wbit = 1;
                                        /* MOVZX/MOVSX
                                           always have 16/32 bit registers    */
      segsflag = segrflag;
      prt8reg16(regf);
      segrflag = 0;
      *ibuff++ = ',';
    }
  if (dbit == 251)
    {
                                        /* register field is printed as a
                                        number for the first operand this is
                                        used for escape instructions          */
      if (mod == 3)
        {
          ibuff = hexword((ULONG)((((ULONG)(instr & 0x07) << 3) + regf) << 3) + rm,
                          ibuff);       /* added ulong cast                521*/

          #if defined(_ASM86_)
            if (masm)
          #endif
              *ibuff++ = 'H';
          goto exit;
        }
      ibuff = hexbyte((UCHAR)(((UCHAR)(instr & 0x07) << 3) + regf),ibuff);
                                        /* added uchar cast                521*/
      #if defined(_ASM86_)
        if (masm)
      #endif
          *ibuff++ = 'H';
      *ibuff++ = ',';
    }
  if (dbit == 246)
    wbit = 0;                           /* MOVZX/MOVSX may have byte
                                           second operand                     */
  if (dbit == 245 || dbit == 246)
    opsize32 = 0;                       /* MOVZX/MOVSX always has 16-bit size
                                           second operand                     */
  if (mod == 3)
    {
                                        /* operand is a register              */
    prt8reg16(rm);
    }
  else
    {
                                        /* operand is a memory location       */
      if (masm && (dbit == 245 || dbit == 246 || dbit == 244))
        goto memop3;
      if (masm && (dbit == 250 || dbit >= 252) &&
         (instr != 0xFF || regf < 2 || regf > 5))
        {
                                        /* we have a MASM memory operand for an
                                           instruction with no register operand
                                           (and not ESC) - output BYTE or WORD.
                                           But not for CALL and JMP indirect,
                                           and PUSH memory                    */
memop3:
          if (wbit == 1)
            {
              if (opsize32)
                {
                  putstr(dwordvec);
                }
              else
                {
                  putstr(wordvec);
                }
            }
          else
            {
              putstr(bytevec);
            }
        }
        if (masm && ovseg != 0)
          {
            prtovseg();
            *ibuff++ = ':';
          }
       if (disppres == 1)
         {
                                        /* there is a displacement            */
           #if defined(_ASM86_)
             if (masm)
           #endif
           putpar();                    /* for MASM, entire memory operand is
                                           in brackets, for compatibility with
                                           the assembler                      */
           if (signedflag == 0)
             {
                                        /* simple 16-bit case                 */
               if (addr32)
                 ibuff = hexdword(opdisp,ibuff);
               else
                 ibuff = hexword(opdisp,ibuff);
               #if defined(_ASM86_)
               if (masm)
               #endif
                 *ibuff++ = 'H';
             }
           else
             {
                                        /* 8-bit signed case                  */
               if (negflag)
                 *ibuff++ = '-';
               else
                 *ibuff++ = '+';
               ibuff = disbyte((UCHAR)(signeddisp),ibuff);
               #if defined(_ASM86_)
                 if (masm)
               #endif
                   *ibuff++ = 'H';
             }
         }
      if (basereg)
        {
                                        /* there is a base register - print it*/
          putpar();
          if (addr32)
            ibuff = printitem((ULONG)(basereg-1),reg32,ibuff);
          else
            ibuff = printitem((ULONG)(basereg-1),reg16,ibuff);
        }
      if (indexreg != 0)
        {
                                        /* there is a base register - print it*/
          putpar();
          if (addr32)
            ibuff = printitem((ULONG)(indexreg-1),reg32,ibuff);
          else
            ibuff = printitem((ULONG)(indexreg-1),reg16,ibuff);
          if (scalefactor != 0)
            {
              *ibuff++ = '*';
              if (scalefactor == 1)
                *ibuff++ = '2';
              if (scalefactor == 2)
                *ibuff++ = '4';
              if (scalefactor == 3)
                *ibuff++ = '8';
            }
        }
      #if defined(_ASM86_)
        if (!masm && ovseg)
          {
            putpar();
            prtovseg();
          }
      #endif
      if (parflag == 1)
        {
                                        /* we have output a left parenthesis -
                                        therefore output a right parenthesis to
                                        match                                 */
          #if defined(_ASM86_)
            if (masm)
          #endif
              *ibuff++ = ']';
          #if defined(_ASM86_)
            else
              *ibuff++ = ')';
          #endif
        }
    }
  if (dbit == 0 || dbit == 244 || dbit == 247 || dbit == 248)
    {
                                        /* register is second operand - print
                                           register name                      */
      *ibuff++ = ',';
      segsflag = segrflag;
      prt8reg16(regf);
      segrflag = 0;
    }
  if (dbit == 252)
    {
                                        /* second operand is "1" (for shifts) */
      *ibuff++ = ',';
      *ibuff++ = '1';
    }
  if (dbit == 253 || dbit == 248)
    {
                                        /* second (or third) operand is "CL"
                                           (for shifts)                       */
      *ibuff++ = ',';
      ibuff = printitem((ULONG)1,reg8,ibuff);
    }
  if (dbit == 250 || dbit == 247)
    {
                                        /* shift n instruction                */
      wbit = 0;                         /* force to 8-bit operand             */
      dbit = 255;                       /* so it prints immediate operand     */
    }
  if (dbit == 255)
    {
                                        /* second operand is an immediate
                                           operand                            */
      prtimmed();    }
  if ((instr & 0xFD) == 0x69)
    prtimmed();                         /* print 3rd (immediate)
                                           operand for MSI                    */
  ovseg = 0;
  goto exit;
                                        /* 8F                                 */
mempopop:                               /* pop memory location (reg field = 0
                                           only - others are illegal          */
  setrm();
  dbit = 254;                           /* no register operand                */
  wbit = 1;                             /* force a word operation             */
  if (regf != 0)
    goto illegop;
  if (mod == 3)
    printmnem(popmnem);
  else
    {
      if (opsize32)
        printmnem(popmnem_32);
      else
        printmnem(popmnem_16);
    }
  goto memop0;
                                        /* 8C, 8E                             */
memsegop:                               /* load or store a segment register
                                           from or to memory or another
                                           register                           */
  setrm();
  if (regf > 5)
    goto illegop;
  #if defined(_ASM86_)
    if (!masm && mod != 3)
      {
                                        /* for operands to store, the register
                                           is always the first
                                           operand for ASM/86 format          */
        dbit = 2;
      }
    else
      {
  #endif
  dbit = (UCHAR)((instr & 2));
  #if defined(_ASM86_)
      }
  #endif
  wbit = 1;                             /* force to a word operand            */
  segrflag = 1;
  #if defined(_ASM86_)
    if (masm)
  #endif
      printmnem(movmnem);
  #if defined(_ASM86_)
    else
      {
        if (instr == 0x8E || mod == 3)
          printmnem(lmnem);
        else
          printmnem(stmnem);
      }
  #endif

  opsize32 = 0;                         /* these are always 16-bit operations */
  parm->retbits &= 0xFE;                /* clear reply 32-bit marker          */
  goto memop0;
                                        /* 6B                                 */
MSIop2:                                 /* multiply immediate, sign extended
                                           byte operand                       */
  signext = 1;                          /* sign extended                      */
                                        /* 69                                 */
MSIop:                                  /* multiply immediate, 16-bit operand */
  dbit = 2;                             /* register is first operand          */
  wbit = 1;                             /* word operation                     */
  setrm();
  goto memop0;
                                        /* C6, C7                             */
MVImemop:                               /* store immediate operations
                                           (regf = 0 only - others are illegal*/
  setrm();
  setdw();
  dbit = 255;                           /* immediate operand                  */
  if (regf != 0)
    goto illegop;
  #if defined(_ASM86_)
    if (masm)
  #endif
      printmnem(movmnem);
  #if defined(_ASM86_)
    else
      {
        if (mod == 3)                   /* it actually is a LI instruction,
                                           although this is not the best way of
                                           encoding this                      */
          printmnem(mnemC6C7[2]);
        else
          printmnem(mnemC6C7[wbit]);
      }
  #endif
  goto memop0;
                                        /* 67                                 */
overaddr:                               /* single byte address size override
                                           prefix                             */
  addrover = 1;                         /* note we have had this              */
  goto restart;
                                        /* 66                                 */
overoper:                               /* single byte operand size override
                                           prefix                             */
  opsizeover = 1;                       /* note we have had this              */
  goto restart;
                                        /* 6A                                 */
pushiop2:                               /* push immediate, sign extended byte
                                           operand                            */
  signext = 1;                          /* sign extended                      */
                                        /* 68                                 */
pushiop:                                /* push immediate, word operand       */
  wbit = 1;                             /* always a word operation            */
  nocomma = 1;                          /* no leading comma for operand field */
  prtimmed();
  goto exit;
                                        /* 40 - 4F, 50 - 5F                   */
reg16op:                                /* single byte 16-bit register orders */
  regf = (UCHAR)(instr & 0x07);         /* get register number                */
  wbit = 1;                             /* force 16 or 32 bit register        */
  prt8reg16(regf);
  goto exit;
                                        /* B0 - BF                            */
regimop:                                /* MOV immediate to 8/16 bit register */
  wbit = (UCHAR)(instr & 0x08);         /* get 8/16 bit operation marker      */
  regf = (UCHAR)(instr & 0x07);         /* get register number                */
  goto aregiml1;
                                        /* F2, F3                             */
repop:                                  /* single byte REP, REPZ, REPNZ orders*/
  f1 = 0;
  ic = *(parm->iptr);                   /* do not increment pointer since we
                                           wish to access the same instruction
                                           byte next time                     */
  if ((ic & 0xF6) == 0xA6)
    {
                                        /* the next instruction is scan or
                                           compare string order - hence this
                                           is a REPZ/REPNZ rather than just
                                           REP                                */
      f1 = (instr & 1) + 1;
    }
  printmnem(repmnem[f1]);
  parm->rettype = reptype;              /* add rettype for REP            1.01*/
  goto exit;
                                        /* C2, CA                             */
retop:                                  /* RET n instructions                 */
  prtword();

  #if defined(_ASM86_)
    if (!masm)
      {
        *ibuff++ = 'X';
        *ibuff++ = '\'';
        quoteflag = 1;
      }
  #endif
  ibuff = hexword(f1,ibuff);
  #if defined(_ASM86_)
    if (masm)
  #endif
      *ibuff++ = 'H';
  #if defined(_ASM86_)
    else
      {
        if (quoteflag) *ibuff++ = '\'';
      }
  #endif
  parm->retoffset = f1;
  if (instr == 0xC2  )                  /* RET                                */
    parm->rettype = retneartype;
  else
                                        /* assume instr = 0xCA - RETF/RET     */
    parm->rettype = retfartype;
  goto exit;
                                        /* C3                                 */
ret0op:                                 /* RET  instruction                   */
  parm->rettype = retneartype;
  parm->retoffset = 0;                  /* it is RET 0                        */
  goto exit;
                                        /* CB                                 */
retfop:                                 /* RETF  instruction                  */
  parm->rettype = retfartype;
  parm->retoffset = 0;                  /* it is RETF 0                       */
  goto exit;
                                        /* 06, 07, 0E, 16, 17, 1E, 1F         */
segop:                                  /* single byte segment register
                                           instructions                       */
  f1 = (ULONG)(instr & 0x18) >> 3;      /* get register number                */
segop_1:                                /* added ulong cast to above st    521*/
  segsflag = 1;                         /* so it prints segment register      */
  prt8reg16((UCHAR)f1);
  goto exit;
                                        /* 64, 65                             */
segovfsgs:                              /* single byte segment override
                                           instructions                       */
  if (ovseg != 0)
    goto exit;                          /* trap SEGOV on SEGOV                */
  ovseg = (UCHAR)(instr - 0x5F);        /* get register number                */
  goto restart;
                                        /* 26, 2E, 36, 3E                     */
segovop:                                /* single byte segment override
                                           instructions                       */
  if (ovseg != 0)
    goto exit;                          /* trap SEGOV on SEGOV                */
  ovseg = (UCHAR)(((UCHAR)(instr & 0x18) >> 3) + 1); /* get register number - +1
                                           since 0 signifies no extant
                                           override                           */
  goto restart;                         /* added uchar cast to above st    521*/
                                        /* D0 - D3, C0,  C1                   */
shiftop:                                /* shift (rotate) operations          */
  setdw();
  setrm();
  if (regf == 6)
    goto illegop;
  if (dbit == 0)
    dbit = 252;
  else
    dbit = 253;                         /* distinguish "1" and "CL" cases     */
  if (instr == 0xC0 || instr == 0xC1)
    dbit = 250;                         /* special case - shift n             */
  f2 = regf * 3;
  if (mod != 3)
    {
                                        /* it is a memory operand  */
      f2 = f2 + 1 + wbit;
    }
  printmnem(shiftmnem[f2]);
  goto memop0;
                                        /* AA, AB, AE, AF                     */

stoscanop:                              /* store and scan string orders       */
  if (ovseg == 0)
    parm->retreg = 1;                   /* ES                                 */
  else
    parm->retreg = ovseg;
  if (addr32)
    parm->retbase = 15;                 /* EDI                                */
  else
    parm->retbase = 7;                  /* DI                                 */
stscl1:
  setdw();
  if (wbit == 0)
    parm->rettype = strbtype;
  else
    parm->rettype = strwtype;
  goto exit;
                                        /* D7                                 */
xlatop:                                 /* XLAT instruction                   */
  if (ovseg == 0)
    parm->retreg = defseg;
  else
    parm->retreg = ovseg;
  if (addr32)
    parm->retbase = 11;                 /* EBX                                */
  else
  parm->retbase = 3;                    /* BX                                 */
  parm->rettype = xlattype;
  goto exit;
                                        /* 0F - two-byte opcodes              */
zerofop:
  prtbyte();                            /* get the second byte of the instr   */
  if (mnem0F[ic] != odd)
    {
                                        /* in many cases we can print the
                                           instruction mnemonic now           */
      printmnem(mnem0F[ic]);
    }
  switch (ic)
    {
      case   0:  goto zerof00;          /*  00                                */
      case   1:  goto zerof01;          /*  01                                */
      case   2:
      case   3:  goto LARop;            /*  02,03                             */
      case   4:
      case   5:  goto illegop;          /*  04,05                             */
      case   6:  goto nulop;            /*  06                                */
      case   7:  goto illegop;          /*  07                                */
      case   8:
      case   9:  goto nulop;            /*  08,09                             */
      case  32:  goto zerof20;          /*  20                                */
      case  33:  goto zerof21;          /*  21                                */
      case  34:  goto zerof22;          /*  22                                */
      case  35:  goto zerof23;          /*  23                                */
      case  36:  goto zerof24;          /*  24                                */
      case  37:  goto illegop;          /*  25                                */
      case  38:  goto zerof26;          /*  26                                */
      case 128:
      case 129:
      case 130:
      case 131:
      case 132:
      case 133:
      case 134:
      case 135:
      case 136:
      case 137:
      case 138:
      case 139:
      case 140:
      case 141:
      case 142:
      case 143:  goto jfulldispop;      /*  80-8F                             */
      case 144:
      case 145:
      case 146:
      case 147:
      case 148:
      case 149:
      case 150:
      case 151:
      case 152:
      case 153:
      case 154:
      case 155:
      case 156:
      case 157:
      case 158:
      case 159:  goto zerof90;          /*  90-9F                             */
      case 160:
      case 161:  goto zerofA0;          /*  A0,A1                             */
      case 162:  goto illegop;          /*  A2                                */
      case 163:  goto zerofA3;          /*  A3                                */
      case 164:
      case 165:  goto zerofA4;          /*  A4,A5                             */
      case 166:
      case 167:  goto zerofA6;          /*  A6,A7                             */
      case 168:
      case 169:  goto zerofA8;          /*  A8,A9                             */
      case 170:  goto illegop;          /*  AA                                */
      case 171:  goto zerofA3;          /*  AB                                */
      case 172:
      case 173:  goto zerofA4;          /*  AC,AD                             */
      case 174:  goto illegop;          /*  AE                                */
      case 175:  goto MSIop;            /*  AF                                */
      case 176:
      case 177:  goto illegop;          /*  B0,B1                             */
      case 178:  goto laddrop;          /*  B2                                */
      case 179:  goto zerofA3;          /*  B3                                */
      case 180:
      case 181:  goto laddrop;          /*  B4,B5                             */
      case 182:  goto zerofB6;          /*  B6                                */
      case 183:  goto zerofB7;          /*  B7                                */
      case 184:
      case 185:  goto illegop;          /*  B8,B9                             */
      case 186:  goto zerofBA;          /*  BA                                */
      case 187:  goto zerofA3;          /*  BB                                */
      case 188:
      case 189:  goto MSIop;            /*  BC,BD                             */
      case 190:  goto zerofBE;          /*  BE                                */
      case 191:  goto zerofBF;          /*  BF                                */
      case 192:
      case 193:  goto zerofA6;          /*  C0,C1                             */
      case 194:
      case 195:
      case 196:
      case 197:
      case 198:
      case 199:  goto illegop;          /*  C2-C7                             */
      case 200:
      case 201:
      case 202:
      case 203:
      case 204:
      case 205:
      case 206:
      case 207:  goto zerofC8;          /*  C8-CF                             */

      default:   goto illegop;          /*  0A-1F                             */
                                        /*  27-7F                             */
                                        /*  D0-FF                             */
    }
                                        /* 0F 00                              */
zerof00:
  dbit = 249;                           /* no register operand                */
  wbit = 1;                             /* word operation                     */
  setrm();
  if (regf > 5) goto illegop;
  opsize32 = 0;                         /* this is always a 16-bit operation  */
  parm->retbits &= 0xFE;
  printmnem(mnem0F00[regf]);
  goto memop0;
                                        /* 0F 01                              */
zerof01:
  dbit = 249;                           /* no register operand                */
  wbit = 1;                             /* word operation                     */
  setrm();
  if (regf == 5)
    goto illegop;
  if (regf == 7 && mod == 3)
    goto illegop;
  if (regf < 4 && mod == 3)
    goto illegop;
  if (regf == 4 || regf == 6)
    {
                                        /* this is always a 16-bit operation  */
      opsize32 = 0;
      parm->retbits &= 0xFE;
    }
  printmnem(mnem0F01[regf]);
  if (regf < 4)
    parm->rettype = LGDTtype;
  goto memop0;
                                        /* 0F 22                              */
zerof22:
  dbit = 2;                             /* GP register is first operand       */
                                        /* 0F 20                              */
zerof20:                                /* in this case, r1 initialise dbit
                                           to 0 (GP register is second operand)
                                                                              */
  prtbyte();                            /* get the third byte of the instr    */
  if ((ic >> 6) != 3)
    goto illegop;
  regf = (UCHAR)(ic & 7);
  eee = (UCHAR)((ic >> 3) & 7);
  if (eee == 1 || eee > 3)
    goto illegop;
zerof20_2:
  printmnem(lmnem);
  if (dbit == 2)
    {
                                        /* register is first operand - print
                                           register name                      */
      ibuff = printitem((ULONG)regf,reg32,ibuff);
      *ibuff++ = ',';
    }
  ibuff = printitem((ULONG)eee,specialreg,ibuff);
  if (dbit == 0)
    {
                                        /* register is second operand - print
                                           register name                      */
      *ibuff++ = ',';
      ibuff = printitem((ULONG)regf,reg32,ibuff);
    }
  goto exit;
                                        /* 0F 23                              */
zerof23:
  dbit = 2;                             /* GP register is first operand       */
                                        /* 0F 21                              */
zerof21:                                /* in this case, r1 initialise dbit to 0
                                           (GP register is second operand)    */
  prtbyte();                            /* get the third byte of the instr    */
  if ((ic >> 6) != 3)
    goto illegop;
  regf = (UCHAR)(ic & 7);
  eee = (UCHAR)((ic >> 3) & 7);
  if ((eee == 4) | (eee == 5))
    goto illegop;
  eee += 4;                             /* 0-3 are CRn, DR0 follows           */
  goto zerof20_2;
                                        /* 0F 26                              */
zerof26:
  dbit = 2;                             /* GP register is first operand       */
                                        /* 0F 24                              */
zerof24:                                /* in this case, r1 initialise dbit to 0
                                           (GP register is second operand)    */
  prtbyte();                            /* get the third byte of the instr    */
  if ((ic >> 6) != 3)
    goto illegop;
  regf = (UCHAR)(ic & 7);
  eee = (UCHAR)((ic >> 3) & 7);
  if ((eee != 6) & (eee != 7))
    goto illegop;
  eee += 6;                             /* 0-3 are CRn, 4-12 are DRn, and TR6,
                                           TR7 follow (TR0-TR5 are omitted)   */
  goto zerof20_2;
                                        /* 0F 90                              */
zerof90:
  dbit = 254;                           /* there is only 1 operand            */
  wbit = 0;                             /* these are always byte operands     */
  setrm();
  goto memop0;
                                        /* 0F A0,A1                           */
zerofA0:
  f1 = 4;                               /* register is FS                     */
  goto segop_1;
                                        /* 0F A3,B3,AB,BB                     */
zerofA3:
  dbit = 244;
  wbit = 1;
  setrm();
  goto memop0;
                                        /* 0F A4,A5,AC,AD                     */
zerofA4:
  dbit = (UCHAR)((ic & 1) + 247);       /* this gives 247 for immediate
                                           operand, 248 for CL                */
  wbit = 1;
  setrm();
  goto memop0;
                                        /* 0F A6,A7, 0F C0,C1                 */
zerofA6:                                /* special 486 instructions           */
  dbit = 0;
  wbit = (UCHAR)(ic & 1);
  setrm();
  goto memop0;
                                        /* 0F A8,A9                           */
zerofA8:                                /* special 386 instructions           */
  f1 = 5;                               /* register is GS                     */
  goto segop_1;
                                        /* 0F B6                              */
zerofB6:
  dbit = 246;                           /* always a byte operation            */
  setrm();
  if (mod != 3)
    parm->rettype = membtype;

zerofB6_1:
  parm->retbits &= 0xFE;

  #if defined(_ASM86_)
    if (!masm)
      {
                                        /* for ASM case, output mnemonic      */
        if (mod == 3)
          f1 = 2;
        else
          f1 = (dbit & 1);
        printmnem(a86mnem0FB6[f1]);
      }
  #endif
  goto memop0;
                                        /* 0F B7                              */
zerofB7:
  dbit = 245;                           /* a word operation                   */
  opsize32 = 1;                         /* always a 32-bit register           */
  setrm();
  if (mod != 3)
    parm->rettype = memwtype;
  goto zerofB6_1;
                                        /* 0F BA                              */
zerofBA:
  dbit = 250;                           /* always an 8-bit immediate operand  */
  wbit = 1;                             /* always a word/dword operation      */
  setrm();
  if (regf < 4)
    goto illegop;
  printmnem(mnem0FBA[regf-4]);
  goto memop0;
                                        /* 0F BE                              */
zerofBE:
  dbit = 246;                           /* always a byte operation            */
  setrm();
  if (mod != 3)
    parm->rettype = membtype;

zerofBE_1:
  parm->retbits &= 0xFE;
  #if defined(_ASM86_)
    if (!masm)
      {
                                        /* for ASM case, output mnemonic      */
        if (mod == 3)
          f1 = 2;
        else
          f1 = (dbit & 1);
        printmnem(a86mnem0FBE[f1]);
      }
  #endif
  goto memop0;
                                        /* 0F BF                              */

zerofBF:
  dbit = 245;                           /* a word operation/                  */
  opsize32 = 1;                         /* always a 32-bit register           */
  setrm();
  if (mod != 3)
    parm->rettype = memwtype;
  goto zerofBE_1;
                                        /* 0F C8 - 0F CF                      */
zerofC8:
  dbit = 245;                           /* a word operation                   */
  if (opsize32 != 1)
    goto illegop;                       /* always a 32-bit register           */
  printmnem(bswapmnem);
  regf = (UCHAR)(ic & 0x07);            /* get register number                */
  wbit = 1;                             /* force 16 or 32 bit register        */
  prt8reg16(regf);
  goto exit;
                                        /* 27, 2F
                                           37, 3F
                                           90, 98, 99, 9B - 9F
                                           A4 - A7
                                           F0, F4, F5, F8 - FD                */

nulop:                                  /* single byte instructions with no
                                           operands                           */
                                        /* drop throught to exit              */
/******************************************************************************/
/****************************** RETURN ****************************************/
/******************************************************************************/

exit:
  if (ovseg)
    {
                                        /* we had a segment override instruction
                                           which was not required - back track,
                                           and reply with that instead        */
      parm->iptr = startiptr;
      r1();
      prtbyte();                        /* get next byte of instruction       */
      instr = ic;
      printmnem(segovmnem);
                                        /* SEGOV or SEG                       */
      ibuff = printitem((ULONG)(ovseg-1),segreg,ibuff);
      parm->rettype = segovtype;
      parm->retreg = ovseg;
    }

                                        /* return updated buffer pointers     */

  parm->retleng = (UCHAR)(parm->iptr - startiptr);
  *mbuff = 0;
  parm->mbuffer = mbuff;
  if (hbuff)
    {
      *hbuff = 0;
      parm->hbuffer = hbuff;
    }
  *ibuff = 0;
  parm->ibuffer = ibuff;
  return;
}
