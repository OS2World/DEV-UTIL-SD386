/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   unwind.c                                                                */
/*                                                                           */
/* Description:                                                              */
/*  unwind the user's call stack.                                            */
/*                                                                           */
/*                                                                           */
/* History:                                                                  */
/*                                                                           */
/*****************************************************************************/
#include "all.h"                        /* SD86 include files                */
#include "diaclstk.h"                   /* call stack dialog data         701*/

/*
 * The following vectors define active stack frames.  They do NOT include
 * the stack frame for the procedure currently executing.  They are ordered
 * from most recent (subscript 0) to least recent (subscript N).  The
 * total number of active frames is given by NActFrames.  The accuracy
 * of these tables depends on the application using the standard linkage
 * convention:
 *
 *   push bp
 *   mov  bp, sp
 */

UINT           NActFrames;
UCHAR          ActCSAtrs[ MAXAFRAMES ]; /* 0 => 16-bit frame 1=>32-bit       */
UINT           ActFrames[ MAXAFRAMES ]; /* value of frame pointer (bp).      */
ULONG          ActFaddrs[ MAXAFRAMES ]; /* value of return addr on stack.    */
UINT           ActFlines[ MAXAFRAMES ] [ 2 ] ; /* mid/lno for ret addr.      */
SCOPE          ActScopes[ MAXAFRAMES ]; /* (mid,ssproc) for return addr.     */

extern uchar         VideoAtr;
extern PtraceBuffer  AppPTB;
extern uint          VideoRows;
extern uint          VideoCols;
extern CmdParms      cmd;

/* The ActiveProcsMenu procedure displays a list of the active procedures.
 * An item from this list may be selected for a temporary breakpoint, or
 * for finding within the debugger.
 */

#define NEARRET         1
#define FARRET          0
#define ATR32           0xD0               /* stack frame is 32-bit       107*/
#define ATR16           0x00               /* stack frame 1s 16-bit       107*/
#define NUM_FRAME_BYTES 12              /* bumped to 12.                  706*/

static  int  ShowAllFlag   = FALSE;
static  SCOPE         ExecScope;

uint ActiveProcsMenu(AFILE **fpp)
{
 UINT key;

 key = Cua_ActiveProcsMenu( fpp );
 return(key);
}

/*****************************************************************************/
/*    Same as ActiveProcsMenu function except that it builds the buffer with */
/* with \0 instead of \r and calls dialog routines to process the keys.   701*/
/*****************************************************************************/
uint Cua_ActiveProcsMenu( AFILE **fpp )                                 /*701*/
{                                                                       /*701*/
  int     i, rc, MaxSymLen, SymLen, LongSymLen;                    /*811  701*/
  uint    key;                                                          /*701*/
  uchar   *cp, *buf, *SaveBuffer = NULL;                                /*701*/
  DEBFILE *pdf;                                                         /*701*/
  DIALOGCHOICE  *chptr;                                                 /*701*/
  CALLSTACKPARAM ClstkParam;                                            /*701*/
  int     buflen;
  int     ScrollBar;                                                    /*701*/

  for( ;; )                                                             /*701*/
  {                                                                     /*701*/
    chptr = &Dia_Clstk_Choices;                                         /*701*/

    MaxSymLen = VideoCols - 6;                                          /*811*/
    buflen = ( (NActFrames + 1)*(MaxSymLen+2) ) + 1 ;
    cp = buf = Talloc( buflen);                                    /*811  701*/

    LongSymLen = 0;                                                     /*811*/
    SymLen    = 0;                                                      /*811*/
    for( i = (short)NActFrames; --i >= 0;  )                            /*701*/
    {                                                                   /*701*/
     pdf = FindExeOrDllWithAddr( ActFaddrs[i] );                        /*701*/
     if(pdf != NULL )
      FormatProcName( cp, ActFaddrs[i], ActScopes[i], pdf,              /*824*/
                               ActCSAtrs[i], MaxSymLen);                /*824*/
     else
      sprintf(cp,"%08X Invalid Frame Address",ActFaddrs[i]);

     SymLen = strlen(cp);                                               /*824*/
     if( SymLen > MaxSymLen )                                           /*824*/
     {                                                                  /*824*/
      SymLen = MaxSymLen;                                               /*824*/
      cp[SymLen] = '\0';                                                /*824*/
     }                                                                  /*824*/
     if( SymLen > LongSymLen )                                          /*824*/
      LongSymLen = SymLen;                                              /*824*/
     cp += SymLen + 1;                                                  /*824*/
    }                                                                   /*824*/
    /* 906 if terminating check was removed */                          /*701*/
    {                                                                   /*824*/

     pdf = FindExeOrDllWithAddr( GetExecAddr() );                       /*901*/
     if( pdf != NULL )
      FormatProcName( cp, GetExecAddr(), ExecScope, pdf,                /*901*/
                           AppPTB.CSAtr, MaxSymLen);                    /*824*/
     else
      sprintf(cp,"%08X Invalid Frame Address",GetExecAddr() );
     SymLen = strlen(cp);                                               /*824*/
     if( SymLen > MaxSymLen )                                           /*824*/
     {                                                                  /*824*/
      SymLen = MaxSymLen;                                               /*824*/
      cp[SymLen] = '\0';                                                /*824*/
     }                                                                  /*824*/
     if( SymLen > LongSymLen )                                          /*824*/
      LongSymLen = SymLen;                                              /*824*/
    }                                                                   /*824*/

    chptr->entries = NActFrames + 1;
    chptr->labels  = buf;

    if( chptr->entries > 10 )
    {
      chptr->MaxRows = 10;
      Dia_Clstk.length = chptr->MaxRows + 8;
      ScrollBar = TRUE;
    }
    else
    {
      Dia_Clstk.length = chptr->entries + 8;
      chptr->MaxRows = chptr->entries;
      ScrollBar = FALSE;
    }

    /*************************************************************************/
    /* At this point, LongSymLen <= MaxSymLen.                               */
    /*************************************************************************/
    Dia_Clstk.width = LongSymLen + 6;
    if( Dia_Clstk.width < 36 )
      Dia_Clstk.width = 36;
    Dia_Clstk.row = (VideoRows - Dia_Clstk.length) / 2;
    Dia_Clstk.col = (VideoCols - Dia_Clstk.width) / 2;
    Dia_Clstk.Buttons[0].row = Dia_Clstk.row + chptr->MaxRows + 4;
    Dia_Clstk.Buttons[0].col = Dia_Clstk.col + 5;
    Dia_Clstk.Buttons[1].row = Dia_Clstk.row + chptr->MaxRows + 4;
    Dia_Clstk.Buttons[1].col = Dia_Clstk.col + Dia_Clstk.width - 16;
    Dia_Clstk.Buttons[2].row = Dia_Clstk.row + chptr->MaxRows + 5;
    Dia_Clstk.Buttons[2].col = Dia_Clstk.col + 5;
    Dia_Clstk.Buttons[3].row = Dia_Clstk.row + chptr->MaxRows + 5;
    Dia_Clstk.Buttons[3].col = Dia_Clstk.col + Dia_Clstk.width - 16;
    Dia_Clstk.Buttons[4].row = Dia_Clstk.row + chptr->MaxRows + 6;
    Dia_Clstk.Buttons[4].col = Dia_Clstk.col + 5;
    Dia_Clstk.Buttons[5].row = Dia_Clstk.row + chptr->MaxRows + 6;
    Dia_Clstk.Buttons[5].col = Dia_Clstk.col + Dia_Clstk.width - 16;

    if( SaveBuffer )
    {
      RemoveMsgBox( "Unwinding...", SaveBuffer );
      SaveBuffer = NULL;
    }
    DisplayDialog( &Dia_Clstk, ScrollBar );
    rc = 0;
    ClstkParam.rc = &rc;
    ClstkParam.fpp= fpp;
    key = ProcessDialog( &Dia_Clstk, &Dia_Clstk_Choices, ScrollBar,
                         (void *)&ClstkParam );
    RemoveDialog( &Dia_Clstk );

    if( rc != RECIRCULATE )
      return( key );
    else
    {
      SaveBuffer = SayMsgBox4( "Unwinding..." );
      if( key == key_a || key == key_A )
      {
        SetShowAll( TRUE );
        SetActFrames();
        SetShowAll( FALSE );
      }
      else
      {
        SetActFrames();
      }
    }
  }
}

void FormatProcName( uchar *buf, uint addr, SCOPE scope, DEBFILE *pdf,  /*824*/
                       uchar FrameAtr, int MaxSymLen)                   /*824*/
{                                                                       /*824*/
 uchar *cp;                                                             /*824*/
 ushort Sel;                                                            /*824*/
 ushort Off;                                                            /*824*/
                                                                        /*824*/
 /****************************************************************************/
 /* Stuff a z-string name in the buf.                                     824*/
 /* - If there is scope, then get the name from symbols.                  824*/
 /* - else get the name from publics.                                     824*/
 /* - else format as 32 address if 32 bit frame.                          824*/
 /* - else format as 16 address if 16 bit frame.                          824*/
 /****************************************************************************/
 if( scope != NULL)
  CopyProcName( scope, buf, MaxSymLen );
 else
 {
  cp = DBFindProcName(addr , pdf);
  if ( cp )
  {
   int  NameLen;
   BOOL IsMangled;
   char buffer[256];

   memset(buffer, 0, sizeof(buffer));

   IsMangled = FALSE;
   IsMangled = unmangle( buffer, cp + 2 );
   if( IsMangled == FALSE )
   {
    NameLen =  *(USHORT *)cp;
    if( NameLen > MaxSymLen )
     NameLen = MaxSymLen;
    memcpy(buf, cp+2, NameLen);
    buf[NameLen] = '\0';
   }
   else
   {
    NameLen =  strlen(buffer);
    if( NameLen > MaxSymLen )
     NameLen = MaxSymLen;
    memcpy(buf, buffer, NameLen);
    buf[NameLen] = '\0';
   }
  }
  else if ( FrameAtr == ATR32 )
  {
   sprintf(buf, "%08X", addr);
  }
  else /* FrameAtr == ATR16 */
  {
   Code_Flat2SelOff(addr,&Sel,&Off);
   sprintf( buf, "%04X:%04X",Sel,Off);
  }
 }
}

/*****************************************************************************/
/* SetActFrames()                                                            */
/*                                                                           */
/* Description:                                                              */
/*  1. Establishes the following arrays for the current state of the user's  */
/*     stack:                                                                */
/*                                                                           */
/*      ActFaddrs[]   the CS:IP or EIP values for the stack frames        107*/
/*      ActCSAtrs[]   0 => 16-bit stack frame  1 => 32-bit stack frame    107*/
/*      ActFlines[][] the mid/lno values for the CS:IP/EIP return values  107*/
/*      ActScopes[]   pointers to SSProc records for the frame CS:IP/EIP vals*/
/*      ActFrames[]   offsets in the stack of the stack frames ( EBP values )*/
/*                                                                           */
/* Parameters:                                                               */
/*   none                                                                    */
/*                                                                           */
/* Return:                                                                   */
/*   none                                                                    */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*    This routine has to guarantee the above arrays are valid. "Valid"      */
/*    implies, for example, that ActFaddrs contains valid addresses          */
/*    in the application code.                                               */
/*                                                                           */
/*****************************************************************************/
void SetActFrames( )
{
 STACK_PARMS  parms;
 UCHAR       *pActCSAtrs = NULL;
 UINT        *pActFrames = NULL;
 UINT        *pActFaddrs = NULL;
 ULONG        n;
 DEBFILE     *pdf;                                                      /*901*/

 parms.CS          = AppPTB.CS;
 parms.SS          = AppPTB.SS;
 parms.EBP         = AppPTB.EBP;
 parms.ESP         = AppPTB.ESP;
 parms.SSAtr       = AppPTB.SSAtr;
 parms.ShowAllFlag = ShowAllFlag;

 NActFrames = xGetCallStack( &parms,&pActCSAtrs,&pActFrames,&pActFaddrs );
 memcpy(ActCSAtrs,pActCSAtrs,NActFrames*sizeof(ActCSAtrs[0]));
 memcpy(ActFrames,pActFrames,NActFrames*sizeof(ActFrames[0]));
 memcpy(ActFaddrs,pActFaddrs,NActFrames*sizeof(ActFaddrs[0]));
 if(pActCSAtrs) Tfree(pActCSAtrs);
 if(pActFrames) Tfree(pActFrames);
 if(pActFaddrs) Tfree(pActFaddrs);

 for( n = 0; n < NActFrames; n++ )
 {
  ActScopes[n] = FindScope( ActFaddrs[n] , &ActFlines[n][0] );
 }
 pdf = FindExeOrDllWithAddr( GetExecAddr() );                            /*901*/
 ExecScope = LocateScope(GetExecAddr(),GetExecMid(),pdf);                /*901*/
}

 uint
StackFrameIndex(SCOPE scope)
{
    uint n;                             /* was register.                  112*/

    if( scope == ExecScope )        return(1);
    if( NActFrames
     && ((n = lindex(ActScopes, NActFrames, (ulong)scope)) < NActFrames)
      ) return( n+2 );
    return(0);
}

 uint
StackFrameAddress(uint index)
{
    if( index == 1 )
    {
     if( AppPTB.SSAtr == 0 )
      return( (uint)Data_SelOff2Flat( AppPTB.SS, LoFlat(AppPTB.EBP) ) );
     else
      return( AppPTB.EBP );
    }

    if( (index -= 2) < NActFrames )
        return( ActFrames[index] );            /* return 32-bit frame     107*/
    return( NULL);
}


/*****************************************************************************/
/* StackFrameMemModel()                                                   112*/
/*                                                                        112*/
/* Description:                                                           112*/
/*   Returns the memory model of the stack frame for the index.           112*/
/*                                                                        112*/
/* Parameters:                                                            112*/
/*   index       input - the stack frame index.                           112*/
/*                                                                        112*/
/* Return:                                                                112*/
/*   BIT16        0.                                                      112*/
/*   BIT32        1.                                                      112*/
/*                                                                        112*/
/* Assumptions:                                                           112*/
/*                                                                        112*/
/*   The index has been verified by caller so no need to check for error. 112*/
/*   Future callers may not do the verification so you may want to add    112*/
/*   the error checking, but this is currently valid.                     112*/
/*                                                                        112*/
/*****************************************************************************/
 uchar                                                                  /*112*/
StackFrameMemModel( uint index )                                        /*112*/
{                                                                       /*112*/
 if( index == 1 )                                                       /*112*/
 {                                                                      /*112*/
  if( AppPTB.SSAtr == 0 )                                               /*112*/
   return( BIT16 );                                                     /*112*/
  else                                                                  /*112*/
   return( BIT32 );                                                     /*112*/
 }                                                                      /*112*/
                                                                        /*112*/
 if ( ActCSAtrs[index-2] == 0 )         /* index should be decremented    532*/
   return( BIT16 );                                                     /*218*/
  else                                                                  /*218*/
   return( BIT32 );                                                     /*218*/

}                                       /* end StackFrameMemModel().      112*/


/*****************************************************************************
* name          bcopy -- Copy bytes
*
* synopsis      cp = bcopy( source, destination, Nbytes );
*
*               char *cp            -  pointer to 1st byte beyond dest
*               char *source        -  pointer to 1st byte to be copied
*               char *destination   -  pointer to 1st byte to receive copy
*               uint Nbytes         -  number of bytes to copy
*
* description   This routine copies N bytes (does not detect overlap).
*               It always copies bytes from left to right (lower to higher).
*               It returns the address of the 1st byte after the destination.
*/
 char*                                                                  /*215*/
bcopy(char *source,char *destination,uint Nbytes)
{
 memcpy( destination, source, Nbytes );  /* copy source to dest           107*/
 return(destination+Nbytes);             /* give back ptr to next slot    107*/

}

/*****************************************************************************/
/* DBDLLLoadInfo()                                                           */
/*                                                                           */
/* Description:                                                              */
/*   Returns a beginning address for a DLL code object.                      */
/*   This routine might ought to go into DBIF.C.                          107*/
/*                                                                           */
/* Parameters:                                                               */
/*   addr        input  - address within the DLL.                            */
/*   pdf         input  - DLL containing the instruction.                    */
/*                                                                           */
/* Return:                                                                   */
/*   loadaddr           - start address of the DLL object.                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   pdf is valid.                                                           */
/*                                                                           */
/*****************************************************************************/
 uint
DBDLLLoadInfo(uint addr,DEBFILE *pdf)
                                        /* current instruction addr          */
                                        /* EXE/DLL containing addr           */
{
 int           i;                       /* loop counter                      */
 uint          NumCodeObjs;             /* number of table entries        521*/
 uint          *p;                      /* working ptr                    521*/
 OBJTABLEENTRY *te;                     /* -> to a object table entry        */

 NumCodeObjs = *(p=pdf->CodeObjs);
 te = (OBJTABLEENTRY *)++p;
 for(i=1; i <= NumCodeObjs; i++,te++ )
 {
  if( (te->ObjType == CODE)     &&                                        /*706*/
      (addr >= te->ObjLoadAddr) &&
      (addr <  te->ObjLoadAddr + te->ObjLoadSize) )
   return( te->ObjLoadAddr );
 }
 return(0);
}                                       /* end DBDLLLoadInfo().              */

void  SetShowAll( int FlagOption )                                      /*701*/
{                                                                       /*701*/
  ShowAllFlag = FlagOption;                                             /*701*/
}                                                                       /*701*/
