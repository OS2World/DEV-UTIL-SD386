/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   ptrace.h                                                                */
/*                                                                           */
/* Description:                                                              */
/*   Contains all of the DosDebug commands and notification values           */
/*                                                                           */
/* History:                                                                  */
/*                                                                           */
/*   02/08/91 Creation of 32-bit SD86, from 16-bit version.                  */
/*                                                                           */
/*****************************************************************************/

typedef struct  {
  ULONG   Pid;                          /* Debuggee Process id               */
  ULONG   Tid;                          /* Debuggee Thread id                */
  long    Cmd;                          /* Command or Notification           */
  long    Value;                        /* Generic Data Value                */
  ULONG   Addr;                         /* Debuggee Address                  */
  ULONG   Buffer;                       /* Debugger Buffer Address           */
  ULONG   Len;                          /* Length of Range                   */
  ULONG   Index;                        /* Generic Identifier Index          */
  ULONG   MTE;                          /* Module Table Entry Handle         */
  ULONG   EAX;                          /* Register Set                      */
  ULONG   ECX;
  ULONG   EDX;
  ULONG   EBX;
  ULONG   ESP;
  ULONG   EBP;
  ULONG   ESI;
  ULONG   EDI;
  ULONG   EFlags;
  ULONG   EIP;
  ULONG   CSLim;
  ULONG   CSBase;
  UCHAR   CSAcc;
  UCHAR   CSAtr;
  USHORT  CS;
  ULONG   DSLim;
  ULONG   DSBase;
  UCHAR   DSAcc;
  UCHAR   DSAtr;
  USHORT  DS;
  ULONG   ESLim;
  ULONG   ESBase;
  UCHAR   ESAcc;
  UCHAR   ESAtr;
  USHORT  ES;
  ULONG   FSLim;
  ULONG   FSBase;
  UCHAR   FSAcc;
  UCHAR   FSAtr;
  USHORT  FS;
  ULONG   GSLim;
  ULONG   GSBase;
  UCHAR   GSAcc;
  UCHAR   GSAtr;
  USHORT  GS;
  ULONG   SSLim;
  ULONG   SSBase;
  UCHAR   SSAcc;
  UCHAR   SSAtr;
  USHORT  SS;
} PtraceBuffer;


#define DBG_L_386         1
#define DBG_C_Continue    27
#define DBG_CO_387        1

#define DBG_W_Global      0x00000001
#define DBG_W_Local       0x00000002
#define DBG_W_Execute     0x00010000
#define DBG_W_Write       0x00020000
#define DBG_W_ReadWrite   0x00030000
