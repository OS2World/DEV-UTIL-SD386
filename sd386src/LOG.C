/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   log.c                                                                822*/
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*  Where the DosDebug calls are really made. They come here                 */
/*  so that we can implement the logging of DosDebug traffic.                */
/*                                                                           */
/* History:                                                                  */
/*                                                                           */
/*   02/02/94 Created.                                                       */
/*                                                                           */
/*****************************************************************************/
#include "all.h"
#undef DosDebug

/*****************************************************************************/
/* Dump DosDebug Registers.                                                  */
/*****************************************************************************/
char    *DbgCmds[32] = {
   "DBG_C_Null",
   "DBG_C_ReadMem_I",
   "DBG_C_ReadMem_D",
   "DBG_C_ReadReg",
   "DBG_C_WriteMem_I",
   "DBG_C_WriteMem_D",
   "DBG_C_WriteReg",
   "DBG_C_Go",
   "DBG_C_Term",
   "DBG_C_SStep",
   "DBG_C_Stop",
   "DBG_C_Freeze",
   "DBG_C_Resume",
   "DBG_C_NumToAddr",
   "DBG_C_ReadCoRegs",
   "DBG_C_WriteCoRegs",
   "Reserved ***",
   "DBG_C_ThrdStat",
   "DBG_C_MapROAlias",
   "DBG_C_MapRWAlias",
   "DBG_C_UnMapAlias",
   "DBG_C_Connect",
   "DBG_C_ReadMemBuf",
   "DBG_C_WriteMemBuf",
   "DBG_C_SetWatch",
   "DBG_C_ClearWatch",
   "DBG_C_RangeStep",
   "DBG_C_Continue",
   "DBG_C_AddrToObject",
   "DBG_C_XchgOpcode",
   "DBG_C_LinToSel",
   "DBG_C_SelToLin"
};

char    *DBG_Notif[18] = {
   "DBG_N_Success",
   "DBG_N_Error",
   "Invalid Notification",
   "Invalid Notification",
   "Invalid Notification",
   "Invalid Notification",
   "DBG_N_ProcTerm",
   "DBG_N_Exception",
   "DBG_N_ModuleLoad",
   "DBG_N_CoError",
   "DBG_N_ThreadTerm",
   "DBG_N_AsyncStop",
   "DBG_N_NewProc",
   "DBG_N_AliasFree",
   "DBG_N_Watchpoint",
   "DBG_N_ThreadCreate",
   "DBG_N_ModuleFree",
   "DBG_N_RangeStep"
};

/*****************************************************************************/
/* Table of exceptions                                                       */
/*****************************************************************************/
typedef struct
{
  uint  XcptNum;
  char *pXcptName;
} XCPTTABLEENTRY;

#define MAX_EXCEPTIONS 29
XCPTTABLEENTRY XcptTable[MAX_EXCEPTIONS] =                       /* current  */
{                                                                /* values   */
 XCPT_GUARD_PAGE_VIOLATION    , "XCPT_GUARD_PAGE_VIOLATION"     ,/*0x80000001*/
 XCPT_UNABLE_TO_GROW_STACK    , "XCPT_UNABLE_TO_GROW_STACK"     ,/*0x80010001*/
 XCPT_DATATYPE_MISALIGNMENT   , "XCPT_DATATYPE_MISALIGNMENT"    ,/*0xC000009E*/
 XCPT_BREAKPOINT              , "XCPT_BREAKPOINT"               ,/*0xC000009F*/
 XCPT_SINGLE_STEP             , "XCPT_SINGLE_STEP"              ,/*0xC00000A0*/
 XCPT_ACCESS_VIOLATION        , "XCPT_ACCESS_VIOLATION"         ,/*0xC0000005*/
 XCPT_ILLEGAL_INSTRUCTION     , "XCPT_ILLEGAL_INSTRUCTION"      ,/*0xC000001C*/
 XCPT_FLOAT_DENORMAL_OPERAND  , "XCPT_FLOAT_DENORMAL_OPERAND"   ,/*0xC0000094*/
 XCPT_FLOAT_DIVIDE_BY_ZERO    , "XCPT_FLOAT_DIVIDE_BY_ZERO"     ,/*0xC0000095*/
 XCPT_FLOAT_INEXACT_RESULT    , "XCPT_FLOAT_INEXACT_RESULT"     ,/*0xC0000096*/
 XCPT_FLOAT_INVALID_OPERATION , "XCPT_FLOAT_INVALID_OPERATION"  ,/*0xC0000097*/
 XCPT_FLOAT_OVERFLOW          , "XCPT_FLOAT_OVERFLOW"           ,/*0xC0000098*/
 XCPT_FLOAT_STACK_CHECK       , "XCPT_FLOAT_STACK_CHECK"        ,/*0xC0000099*/
 XCPT_FLOAT_UNDERFLOW         , "XCPT_FLOAT_UNDERFLOW"          ,/*0xC000009A*/
 XCPT_INTEGER_DIVIDE_BY_ZERO  , "XCPT_INTEGER_DIVIDE_BY_ZERO"   ,/*0xC000009B*/
 XCPT_INTEGER_OVERFLOW        , "XCPT_INTEGER_OVERFLOW"         ,/*0xC000009C*/
 XCPT_PRIVILEGED_INSTRUCTION  , "XCPT_PRIVILEGED_INSTRUCTION"   ,/*0xC000009D*/
 XCPT_IN_PAGE_ERROR           , "XCPT_IN_PAGE_ERROR"            ,/*0xC0000006*/
 XCPT_PROCESS_TERMINATE       , "XCPT_PROCESS_TERMINATE"        ,/*0xC0010001*/
 XCPT_ASYNC_PROCESS_TERMINATE , "XCPT_ASYNC_PROCESS_TERMINATE " ,/*0xC0010002*/
 XCPT_NONCONTINUABLE_EXCEPTION, "XCPT_NONCONTINUABLE_EXCEPTION" ,/*0xC0000024*/
 XCPT_INVALID_DISPOSITION     , "XCPT_INVALID_DISPOSITION"      ,/*0xC0000025*/
 XCPT_INVALID_LOCK_SEQUENCE   , "XCPT_INVALID_LOCK_SEQUENCE"    ,/*0xC000001D*/
 XCPT_ARRAY_BOUNDS_EXCEEDED   , "XCPT_ARRAY_BOUNDS_EXCEEDED"    ,/*0xC0000093*/
 XCPT_B1NPX_ERRATA_02         , "XCPT_B1NPX_ERRATA_02"          ,/*0xC0010004*/
 XCPT_UNWIND                  , "XCPT_UNWIND"                   ,/*0xC0000026*/
 XCPT_BAD_STACK               , "XCPT_BAD_STACK"                ,/*0xC0000027*/
 XCPT_INVALID_UNWIND_TARGET   , "XCPT_INVALID_UNWIND_TARGET"    ,/*0xC0000028*/
 XCPT_SIGNAL                  , "XCPT_SIGNAL"                    /*0xC0010003*/
};

static int DbgTrace = FALSE;
static int AlreadyOpen = FALSE;

APIRET Dbg(PtraceBuffer *p)
{
  ULONG  rc;
  FILE  *dmp = NULL;

  if( DbgTrace == FALSE )
   return(DosDebug( p ));

  if( AlreadyOpen == FALSE )
  {
   dmp = fopen("log.dat","w");
   AlreadyOpen = TRUE;
  }
  else
   dmp = fopen("log.dat","a");

  fprintf( dmp,"\nDosDebug Command is %s...Cmd=%d",DbgCmds[p->Cmd],p->Cmd);
  if (p->Cmd == 27)
  {
    switch((ULONG)(p->Value))
    {
      case XCPT_CONTINUE_SEARCH :
        fprintf( dmp,"\n  Value is        XCPT_CONTINUE_SEARCH");
        break;
      case XCPT_CONTINUE_EXECUTION :
        fprintf( dmp,"\n  Value is        XCPT_CONTINUE_EXECUTION");
        break;
      case XCPT_CONTINUE_STOP :
        fprintf( dmp,"\n  Value is        XCPT_CONTINUE_STOP");
        break;
    }
  }

  DumpPtraceBuffer( dmp,p);

  rc = DosDebug( p );

  fprintf( dmp,"\nDosDebug Notification is %s...Cmd=%d",DBG_Notif[abs(p->Cmd)],p->Cmd);
  if( p->Cmd == DBG_N_ModuleLoad )
  {
    char modname[256];
    DosQueryModuleName( p->Value, 256, modname );
    fprintf( dmp,"\n  Dll name is %s",modname);
  }
  DumpPtraceBuffer(dmp,p);

  if( p->Cmd == DBG_N_Exception  )
  {
   if (p->Buffer == XCPT_BREAKPOINT)
   {
     fprintf(dmp,"\n Exception is XCPT_BREAKPOINT\n" );
     fprintf(dmp," Exception Address = %x\n",p->Addr);
     fprintf(dmp," Exception Number  = %x\n",p->Buffer);
     fprintf(dmp," Exception Chance  = %d\n",p->Value);
   }
   else if (p->Buffer == XCPT_SINGLE_STEP)
   {
     fprintf(dmp,"\n Exception is XCPT_SINGLE_STEP\n" );
     fprintf(dmp," Exception Address   = %x\n",p->Addr);
     fprintf(dmp," Exception Number    = %x\n",p->Buffer);
     fprintf(dmp," Exception Chance    = %d\n",p->Value);
   }
   else
   {
    char         *pXcptName;
    ULONG         XcptNumber;
    ULONG         XcptAddr;

    XcptAddr = 0;
    pXcptName = NULL;
    XcptNumber = DbgResolveException( p , &XcptAddr, &pXcptName );

    fprintf(dmp,"\n Exception is      = %s\n" , pXcptName );
    fprintf(dmp," Exception Address = %x\n" , XcptAddr);
    fprintf(dmp," Exception Number  = %x\n" , XcptNumber );
    fprintf(dmp," Exception Chance  = %d\n" , p->Value  );
   }

  }

  fflush( dmp );
  fclose(dmp);
  return(rc);
}

void DumpPtraceBuffer( FILE *dmp,PtraceBuffer *p)
{

 fprintf( dmp,"\n Pid   = %-8u Tid   = %-8u Mte   = %08X Value = %08X\n",
                  p->Pid,      p->Tid,      p->MTE,      p->Value);

 fprintf( dmp," Addr  = %08X Buffer= %08X Len   = %08X Index = %08X\n",
                p->Addr,     p->Buffer,   p->Len,      p->Index );

 fprintf( dmp," EAX   = %08X EBX   = %08X ECX   = %08X EDX   = %08X\n",
                p->EAX,      p->EBX,      p->ECX,      p->EDX );

 fprintf( dmp," ESI   = %08X EDI   = %08X EBP   = %08X ESP   = %08X\n",
                p->ESI,      p->EDI,      p->EBP,      p->ESP );

 fprintf( dmp," EIP   = %08X\n",
                p->EIP);

 fprintf( dmp," CS    = %-08.4X DS    = %-08.4X ES    = %-08.4X SS    = %-08.4X\n",
                p->CS,       p->DS,             p->ES,          p->SS );

 fprintf( dmp," CSLim = %08X DSLim = %08X ESLim = %08X SSLim = %08X\n",
                p->CSLim,    p->DSLim,    p->ESLim,    p->SSLim );

 fprintf( dmp," CSBase= %08X DSBase= %08X ESBase= %08X SSBase= %08X\n",
                p->CSBase,   p->DSBase,   p->ESBase,   p->SSBase );

 fprintf( dmp," CSAcc = %-08.2X DSAcc = %-08.2X ESAcc = %-08.2X SSAcc = %-08.2X\n",
                p->CSAcc,    p->DSAcc,    p->ESAcc,    p->SSAcc );

 fprintf( dmp," CSAtr = %-08.2X DSAtr = %-08.2X ESAtr = %-08.2X SSAtr = %-08.2X\n",
                p->CSAtr,    p->DSAtr,    p->ESAtr,    p->SSAtr );


}

void EspSetDbgTrace( int TorF)
{
 DbgTrace = TorF;
}

ULONG DbgResolveException(PtraceBuffer *pptb,ULONG *pXcptAddr,char **pXcptName)
{
 EXCEPTIONREPORTRECORD *pXcptReportRec;
 uint                   XcptNumber;
 uint                   rc;
 uchar                 *p;
 PtraceBuffer           ptb;
 uint                   nbytes;
 int                    i;

 nbytes = sizeof(EXCEPTIONREPORTRECORD);
 p = (UCHAR*)Talloc( nbytes );

 ptb.Cmd    = DBG_C_ReadMemBuf;
 ptb.Pid    = pptb->Pid;
 ptb.Addr   = pptb->Buffer;/* -> to exception report record */
 ptb.Buffer = (ULONG)p;
 ptb.Len    = (ULONG)nbytes ;

 rc = DosDebug( &ptb );
 if( (rc==0) && (ptb.Cmd==DBG_N_Success) )
 {
  pXcptReportRec = (EXCEPTIONREPORTRECORD *)p;
  *pXcptAddr     = (ulong)pXcptReportRec->ExceptionAddress;
  XcptNumber     = pXcptReportRec->ExceptionNum;

  i = 0;
  while( i < MAX_EXCEPTIONS )
  {
    if( XcptNumber == XcptTable[i].XcptNum )
    {
      *pXcptName = XcptTable[i].pXcptName;
      break;
    }
    i++;
  }
 }

 if(p)Tfree(p);
 return(XcptNumber);
}
