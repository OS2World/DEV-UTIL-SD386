#include "all.h"

ULONG exception_nos[MAXEXCEPTIONS] =
{
 XCPT_GUARD_PAGE_VIOLATION,
 XCPT_UNABLE_TO_GROW_STACK,
 XCPT_DATATYPE_MISALIGNMENT,
 XCPT_ACCESS_VIOLATION,
 XCPT_ILLEGAL_INSTRUCTION,
 XCPT_FLOAT_DENORMAL_OPERAND,
 XCPT_FLOAT_DIVIDE_BY_ZERO,
 XCPT_FLOAT_INEXACT_RESULT,
 XCPT_FLOAT_INVALID_OPERATION,
 XCPT_FLOAT_OVERFLOW,
 XCPT_FLOAT_STACK_CHECK,
 XCPT_FLOAT_UNDERFLOW,
 XCPT_INTEGER_DIVIDE_BY_ZERO,
 XCPT_INTEGER_OVERFLOW,
 XCPT_PRIVILEGED_INSTRUCTION,
 XCPT_IN_PAGE_ERROR,
 XCPT_PROCESS_TERMINATE,
 XCPT_ASYNC_PROCESS_TERMINATE,
 XCPT_NONCONTINUABLE_EXCEPTION,
 XCPT_INVALID_DISPOSITION,
 XCPT_INVALID_LOCK_SEQUENCE,
 XCPT_ARRAY_BOUNDS_EXCEEDED,
 XCPT_UNWIND,
 XCPT_BAD_STACK,
 XCPT_INVALID_UNWIND_TARGET,
 XCPT_SIGNAL,
 XCPT_PROGRAM
};

UCHAR ExceptionMap[MAXEXCEPTIONS] =
{
 NO_NOTIFY,     /* 0  XCPT_GUARD_PAGE_VIOLATION      */
 NOTIFY,        /* 1  XCPT_UNABLE_TO_GROW_STACK      */
 NOTIFY,        /* 2  XCPT_DATATYPE_MISALIGNMENT     */
 NOTIFY,        /* 3  XCPT_ACCESS_VIOLATION          */
 NOTIFY,        /* 4  XCPT_ILLEGAL_INSTRUCTION       */
 NOTIFY,        /* 5  XCPT_FLOAT_DENORMAL_OPERAND    */
 NOTIFY,        /* 6  XCPT_FLOAT_DIVIDE_BY_ZERO      */
 NOTIFY,        /* 7  XCPT_FLOAT_INEXACT_RESULT      */
 NOTIFY,        /* 8  XCPT_FLOAT_INVALID_OPERATION   */
 NOTIFY,        /* 9  XCPT_FLOAT_OVERFLOW            */
 NOTIFY,        /*10  XCPT_FLOAT_STACK_CHECK         */
 NOTIFY,        /*11  XCPT_FLOAT_UNDERFLOW           */
 NOTIFY,        /*12  XCPT_INTEGER_DIVIDE_BY_ZERO    */
 NOTIFY,        /*13  XCPT_INTEGER_OVERFLOW          */
 NOTIFY,        /*14  XCPT_PRIVILEGED_INSTRUCTION    */
 NOTIFY,        /*15  XCPT_IN_PAGE_ERROR             */
 NO_NOTIFY,     /*16  XCPT_PROCESS_TERMINATE         */
 NO_NOTIFY,     /*17  XCPT_ASYNC_PROCESS_TERMINATE   */
 NOTIFY,        /*18  XCPT_NONCONTINUABLE_EXCEPTION  */
 NOTIFY,        /*19  XCPT_INVALID_DISPOSITION       */
 NOTIFY,        /*20  XCPT_INVALID_LOCK_SEQUENCE     */
 NOTIFY,        /*21  XCPT_ARRAY_BOUNDS_EXCEEDED     */
 NO_NOTIFY,     /*22  XCPT_UNWIND                    */
 NOTIFY,        /*23  XCPT_BAD_STACK                 */
 NOTIFY,        /*24  XCPT_INVALID_UNWIND_TARGET     */
 NOTIFY,        /*25  XCPT_SIGNAL                    */
 NO_NOTIFY      /*26  XCPT_PROGRAM                   */
};

char *ExcepTypes[MAXEXCEPTIONS] =       /*                                   */
{                                       /*                                   */
 "GuardPageViolation",                  /* 0                                 */
 "UnableToGrowStack",                   /* 1                                 */
 "DataTypeMisAlignment",                /* 2                                 */
 "AccessViolation",                     /* 3                                 */
 "IllegalInstruction",                  /* 4                                 */
 "FloatingDenormalOperand",             /* 5                                 */
 "FloatingDivideByZero",                /* 6                                 */
 "FloatingInExactResult",               /* 7                                 */
 "FloatingInvalidOperation",            /* 8                                 */
 "FloatingOverflow",                    /* 9                                 */
 "FloatingStackCheck",                  /* 10                                */
 "FloatingUnderflow",                   /* 11                                */
 "IntegerDivideByZero",                 /* 12                                */
 "IntegerOverFlow",                     /* 13                                */
 "PrivilegedInstruction",               /* 14                                */
 "PageError",                           /* 15                                */
 "ProcessTerminate",                    /* 16                                */
 "AsyncProcessTerminate",               /* 17                                */
 "NonContinuableException",             /* 18                                */
 "InvalidDisposition",                  /* 19                                */
 "InvalidLockSequence",                 /* 20                                */
 "ArrayBoundsExceeded",                 /* 21                                */
 "UnwindException",                     /* 22                                */
 "BadStack",                            /* 23                                */
 "InvalidUnwindTarget",                 /* 24                                */
 "Signal",                              /* 25                                */
 "DosRaiseException/c++/throw"          /* 26                                */
};

char *ExcepSel[2]    = {
                        "NoNotify",
                        "Notify"
                       };

void XSrvSetExceptions( UCHAR *pExceptionMap, int length )
{
 memcpy( ExceptionMap,pExceptionMap,length );
}

/*****************************************************************************/
/* GetExceptionMap()                                                         */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Get a notify/nonotify specification from the exception map.             */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   ExceptionIndex  an index into the exception map.                        */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   NOTIFY/NONOTIFY                                                         */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   ExceptionIndex is within the bounds of the ExceptionMap array.          */
/*                                                                           */
/*****************************************************************************/
int  GetExceptionMap( int ExceptionIndex )
{
 return( ExceptionMap[ExceptionIndex] );
}

/*****************************************************************************/
/* GetExceptionIndex()                                                       */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Get the index in the exception number map of an exception.              */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   exception_no    the exception number.                                   */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   n               index in the exception_nos map.                         */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*                                                                           */
/*****************************************************************************/
int  GetExceptionIndex( int exception_no )
{
 int n;

 n = lindex(exception_nos, MAXEXCEPTIONS, exception_no);

 if( n == MAXEXCEPTIONS )
  n = lindex(exception_nos, MAXEXCEPTIONS, XCPT_PROGRAM);

 return( n );
}

/*****************************************************************************/
/* GetExceptionNumber()                                                      */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Get the number of the exception for an exception index.                 */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   ExceptionIndex  the index of the exception.    .                        */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   The exception number.                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   ExceptionIndex is within the bounds of the ExceptionMap array.          */
/*                                                                           */
/*****************************************************************************/
int  GetExceptionNumber( int ExceptionIndex )
{
 return( exception_nos[ExceptionIndex] );
}

/*****************************************************************************/
/* GetExceptionType()                                                        */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Get the string defining the exception.                                  */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   ExceptionIndex  the index of the exception.    .                        */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   Pointer to the defining string.                                         */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   ExceptionIndex is within the bounds of the ExceptionTypes array.        */
/*                                                                           */
/*****************************************************************************/
char *GetExceptionType( int ExceptionIndex )
{
 return( ExcepTypes[ExceptionIndex] );
}

/*****************************************************************************/
/* ResolveException()                                                        */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Handle exception notifications other than INT3 and Single-Step          */
/*   from DosDebug.                                                          */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   pptb          -> to the ptrace buffer.                                  */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   exception_no  the exception number from the the exception               */
/*                 registration record.                                      */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   none                                                                    */
/*                                                                           */
/*****************************************************************************/
ULONG ResolveException(PtraceBuffer *pptb)
{
 UINT                  exception_no;
 UINT                  rc;
 PtraceBuffer          LocalPtb;
 EXCEPTIONREPORTRECORD ExceptionReportRecord;
 UINT                  nbytes;

 /****************************************************************************/
 /* First, get the exception number.                                         */
 /****************************************************************************/
 exception_no = 0;
 nbytes = sizeof(EXCEPTIONREPORTRECORD);
 memset(&LocalPtb,0,sizeof(LocalPtb));

 LocalPtb.Pid    = pptb->Pid;
 LocalPtb.Cmd    = DBG_C_ReadMemBuf ;
 LocalPtb.Addr   =  (ULONG)pptb->Buffer;
 LocalPtb.Buffer = (ULONG)&ExceptionReportRecord;
 LocalPtb.Len    = (ULONG)nbytes ;
 rc              = DosDebug( &LocalPtb );

 if(rc || LocalPtb.Cmd!=DBG_N_Success)
   return(TRAP_EXP);                    /* return exception anomaly code     */

 exception_no = ExceptionReportRecord.ExceptionNum;

 /****************************************************************************/
 /* Now, read the "real" register values into AppPTB. DosDebug gives us      */
 /* a bogus set of registers so we have to patch them up.                    */
 /****************************************************************************/
 {
  #define LDTBIT 4
  CONTEXTRECORD XcptContextRecord;

  nbytes = sizeof(XcptContextRecord);
  memset(&LocalPtb,0,sizeof(LocalPtb));
  LocalPtb.Pid = pptb->Pid;
  LocalPtb.Cmd = DBG_C_ReadMemBuf ;
  LocalPtb.Addr =  (ULONG)pptb->Len;
  LocalPtb.Buffer = (ULONG)&XcptContextRecord;
  LocalPtb.Len = (ULONG)nbytes ;
  rc = DosDebug( &LocalPtb );
  if(rc || LocalPtb.Cmd!=DBG_N_Success)
    return(TRAP_EXP);

  if( XcptContextRecord.ContextFlags & CONTEXT_CONTROL )
  {
   pptb->EBP    = XcptContextRecord.ctx_RegEbp;
   pptb->EIP    = XcptContextRecord.ctx_RegEip;
   pptb->CS     = XcptContextRecord.ctx_SegCs;
   pptb->EFlags = XcptContextRecord.ctx_EFlags;
   pptb->ESP    = XcptContextRecord.ctx_RegEsp;
   pptb->SS     = XcptContextRecord.ctx_SegSs;

   pptb->CSAtr = 0xd0;
   if( pptb->CS & LDTBIT )
    pptb->CSAtr = 0;

   pptb->SSAtr = 0xd0;
   if( pptb->SS & LDTBIT )
    pptb->SSAtr = 0;
  }

  if( XcptContextRecord.ContextFlags & CONTEXT_INTEGER )
  {
   pptb->EDI = XcptContextRecord.ctx_RegEdi;
   pptb->ESI = XcptContextRecord.ctx_RegEsi;
   pptb->EAX = XcptContextRecord.ctx_RegEax;
   pptb->EBX = XcptContextRecord.ctx_RegEbx;
   pptb->ECX = XcptContextRecord.ctx_RegEcx;
   pptb->EDX = XcptContextRecord.ctx_RegEdx;
  }

  if( XcptContextRecord.ContextFlags & CONTEXT_SEGMENTS )
  {
   pptb->GS = XcptContextRecord.ctx_SegGs;
   pptb->FS = XcptContextRecord.ctx_SegFs;
   pptb->ES = XcptContextRecord.ctx_SegEs;
   pptb->DS = XcptContextRecord.ctx_SegDs;

   pptb->GSAtr = 0xd0;
   if( pptb->GS & LDTBIT )
    pptb->GSAtr = 0;

   pptb->FSAtr = 0xd0;
   if( pptb->FS & LDTBIT )
    pptb->FSAtr = 0;

   pptb->ESAtr = 0xd0;
   if( pptb->ES & LDTBIT )
    pptb->ESAtr = 0;

   pptb->DSAtr = 0xd0;
   if( pptb->DS & LDTBIT )
    pptb->DSAtr = 0;
  }
 }
 return(exception_no);
}
