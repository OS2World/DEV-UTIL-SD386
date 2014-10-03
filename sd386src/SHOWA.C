/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   showa.c                                                                 */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*  Machine display routines.                                                */
/*                                                                           */
/* History:                                                                  */
/*                                                                           */
/*   02/08/91 Creation of 32-bit SD86, from 16-bit version.                  */
/*                                                                           */
/*...16->32 port.                                                            */
/*...                                                                        */
/*... 02/08/91  100   Philip    port to 32 bit.                              */
/*... 02/08/91  101   Joe       port to 32 bit.                              */
/*... 02/08/91  102   Pratima   port to 32 bit.                              */
/*... 02/08/91  103   Dave      port to 32 bit.                              */
/*... 02/08/91  104                                                          */
/*... 02/08/91  105   Christina port to 32 bit.                              */
/*... 02/08/91  106   Srinivas  port to 32 bit.                              */
/*... 02/08/91  107   Dave      port to 32 bit.                              */
/*... 02/08/91  108   Dave      port to 32 bit.                              */
/*... 02/08/91  109                                                          */
/*... 02/08/91  110   Srinivas  port to 32 bit.                              */
/*... 02/08/91  111   Christina port to 32 bit.                              */
/*... 02/08/91  112   Joe       port to 32 bit.                              */
/*... 02/08/91  113                                                          */
/*... 02/08/91  114                                                          */
/*... 02/08/91  115   Srinivas  port to 32 bit.                              */
/*... 02/08/91  116   Joe       port to 32 bit.                              */
/*                                                                           */
/*...Release 1.00 (Pre-release 1)                                            */
/*...                                                                        */
/*... 07/09/91  201   srinivas  single line disassembly when stepping into   */
/*                              a system dll.                                */
/*... 08/06/91  221   srinivas  Users app being started in back ground       */
/*... 08/19/91  229   srinivas  ESC key should take back to action bar from  */
/*                              showthds and editregs.                       */
/*... 08/30/91  235   Joe       Cleanup/rewrite ascroll() to fix several bugs*/
/*                                                                           */
/*...Release 1.00 (Pre-release 105 10/10/91)                                 */
/*...                                                                        */
/*... 10/16/91  301   Srinivas  Alt_F3 (change mnemonics) not when you come  */
/*                              into a c-runtime asm code.                   */
/*... 10/22/91  305   Srinivas  Trap on sizing the data window.              */
/*...                                                                        */
/*... 10/31/91  308   Srinivas  Menu for exceptions.                         */
/*...                                                                        */
/*...Release 1.00 (Pre-release 107 11/13/91)                                 */
/*...                                                                        */
/*... 11/13/91  400   Srinivas  Vertical Register Display.                   */
/*...                                                                        */
/*... 11/18/91  401   Srinivas  Floating point Register Display.             */
/*...                                                                        */
/*... 11/21/91  402   Joe       Fix source/disassembly synchronization.      */
/*...                                                                        */
/*...Release 1.00 (Pre-release 108 12/05/91)                                 */
/*...                                                                        */
/*... 01/28/92  508   Srinivas  Added Set Execution Line function.           */
/*... 02/05/92  511   Srinivas  Allow turning off screen swapping while      */
/*...                           executing.                                   */
/*... 02/11/92  518   Srinivas  Remove limitation on max no of screen rows.  */
/*... 02/12/92  521   Srinivas  Port to C-Set/2.                             */
/*... 02/17/92  531   Srinivas  Trap when we switch back to source in thunks */
/*...                           case.                                        */
/*...                                                                        */
/*...Release 1.00 (03/03/92)                                                 */
/*...                                                                        */
/*... 03/10/92  602   Srinivas  Hooking up watch points.                     */
/*...                                                                        */
/*...Release 1.01 (04/03/92)                                                 */
/*...                                                                        */
/*... 05/08/92  701   Srinivas  Cua Interface.                               */
/*... 10/05/92  701   Michelle  Help window.                                 */
/*... 01/12/93  808   Selwyn    Profile fixes/improvements.                  */
/*... 03/22/93  817   Selwyn    S_F3 from a mixed view with comment lines    */
/*...                           only hangs.                                  */
/*****************************************************************************/
/**Includes*******************************************************************/
                                        /*                                   */
#include "all.h"                        /* SD86 include files                */
static int iview=0;
                                        /*                                   */
/**Defines *******************************************************************/
                                        /*                                   */
#define N1KEYS 29                       /*                                   */
                                        /*                                   */
/**External declararions******************************************************/
                                        /*                                   */
extern AFILE      *allfps;              /*                                   */
extern AFILE      *fp_focus;            /*                                   */
extern uint        TopLine;             /*                                   */
extern uint        SaveTopLine;         /* saved top line for data window.   */
extern uint        HideDataWin;         /* data window hidden flag.          */
extern uint        LinesPer;            /* current # lines/screen for code   */
extern uint        MinPer;              /* minimum # of lines for code.      */
extern uint        MaxPer;              /* max # of lines for code.          */
extern uint        VideoCols;           /*                                   */
extern uint        VideoRows;           /* # of rows per screen           518*/
extern uint        SyncAddr;            /*                                   */
extern uchar      *ActFaddrs[];         /*                                   */
extern uchar      *ActCSIPs[];          /*                                   */
extern uint        TidTab[];            /*                                   */
extern KEY2FUNC    defk2f[];            /*                                   */
extern int         RegTop=0;            /*                                   */
extern uint        MneChange=0;         /* flag to tell DBDisa about a       */
                                        /* mnemonics change.                 */
extern uchar       Reg_Display;         /* Register display flag          400*/
extern uchar      *BoundPtr;            /* -> to screen bounds            518*/
extern uint        VioStartOffSet;      /* flag to tell were to start scr 701*/
extern CmdParms    cmd;
extern PROCESS_NODE *pnode;                                             /*827*/
extern UINT        MaxData;             /* max # of lines of Data            */
                                        /*                                   */

       uint    RowsUsed;                /* was static                     235*/
       int     AsmTop;                  /* was static                     235*/
       int     AsmRows=0;               /* was static                     235*/
extern uint   *AddrCache;                                               /*518*/
extern uint   *SourceLineCache;                                         /*518*/
extern int        BlowBy;               /* blow by GetFuncsFromEvents flag701*/
                                        /*                                   */

UINT   AL86orMASM=1;                    /* 1 if user wants MASM disassembly  */




/**Begin Code*****************************************************************/
                                        /*                                   */
/*****************************************************************************/
/* showA()                                                                   */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Disassembly window handling.                                            */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   fp        input - the afile for this dfile node.                        */
/*   msg       ????? - ????????????????????????????????????????????????????? */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*             a GO cmd of sorts, like GOSTEP, GOSTEPC, etc.                 */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   none.                                                                   */
/*                                                                           */
/*****************************************************************************/
    uint
showA(AFILE *fp, uchar *msg )
{
 AFILE     *Newfp;
 uint      CsrRow;
 uint      CsrInstAddr;
 uint      key;                         /* key returned from GetKey()        */
 int       func;                        /* function associated with a key    */


 Recalibrate(fp, 0);
 fmtasm( fp );

 if( fp->flags & AF_ZOOM )                                              /*235*/
 {                                                                      /*235*/
  if( !IsOnAsmCRT( fp->hotaddr) )                                       /*235*/
  {                                                                     /*235*/
   fp->topoff = fp->hotaddr;                                            /*235*/
  }                                                                     /*235*/
  ascroll(fp, 0) ;                                                      /*235*/
  if( fp->flags & ASM_VIEW_NEW )                                        /*301*/
    SetBitOff(fp->flags, ASM_VIEW_NEW);                                 /*301*/
 }                                                                      /*235*/
                                                                        /*235*/
 else if( fp->flags & ASM_VIEW_NEW )                                    /*235*/
 {                                                                      /*235*/
  ascroll(fp, 0) ;                                                      /*235*/
  SetBitOff(fp->flags, ASM_VIEW_NEW);                                   /*235*/
 }                                                                      /*235*/
                                                                        /*235*/
 else if( fp->flags & ASM_VIEW_TOGGLE )                                 /*235*/
 {                                                                      /*235*/
  ascroll(fp, 0) ;                                                      /*235*/
  SetBitOff(fp->flags, ASM_VIEW_TOGGLE );                               /*235*/
 }                                                                      /*235*/
                                                                        /*235*/
 else if( fp->flags & ASM_VIEW_CHANGE )                                 /*235*/
 {                                                                      /*235*/
  ascroll(fp, 0) ;                                                      /*235*/
  SetBitOff(fp->flags, ASM_VIEW_CHANGE );                               /*235*/
 }                                                                      /*235*/
                                                                        /*235*/
 else if( fp->flags & ASM_VIEW_NEXT   )                                 /*235*/
 {                                                                      /*235*/
  ascroll(fp, 0) ;                                                      /*235*/
  SetBitOff(fp->flags, ASM_VIEW_NEXT   );                               /*235*/
 }                                                                      /*235*/
                                                                        /*235*/
 /************************************************************************235*/
 /* set the initial cursor position.                                      235*/
 /************************************************************************235*/
 if( fp->flags & AF_ZOOM  )                                             /*235*/
 {
  for( CsrRow=0; AddrCache[CsrRow] != fp->hotaddr; CsrRow++){;}         /*235*/
  SetBitOff(fp->flags, AF_ZOOM   );                                     /*235*/
 }
 else                                                                   /*235*/
 {                                                                      /*235*/
  CsrRow = fp->csr.row - TopLine;                                       /*235*/
  if( (int)CsrRow < 0 )
   CsrRow = 0;
  if( CsrRow >= RowsUsed )                                              /*235*/
   CsrRow = RowsUsed - 1;                                               /*235*/
 }                                                                      /*235*/
                                                                        /*235*/
 /************************************************************************235*/
 /* clear the flags and display any messages.                             235*/
 /************************************************************************235*/
 fmterr( msg );                                                         /*235*/
                                                                        /*235*/
 /****************************************************************************/
 /* now, handle the keystrokes.                                           235*/
 /****************************************************************************/
 for( ;; )
 {
   fp->csr.row = CsrRow + TopLine;                                      /*701*/
   if( fp->csr.row > VideoRows )                                        /*701*/
     fp->csr.row = TopLine;
   PutCsr( ( CSR *)&fp->csr );                                          /*701*/
   SetMenuMask( ASSEMBLYVIEW );                                         /*701*/
   if( (fp == GetExecfp()) || (GetExecfp() == NULL) )                   /*701*/
    DisableDropFile();
   else
    EnableDropFile();

  /***************************************************************************/
  /* Get an event from the kbd/mouse.                                     701*/
  /***************************************************************************/
  if( (SerialParallel() == SERIAL) && (QueryConnectSema4() == SEMA4_RESET) )
  {
   fmterr("Disconnected...double click on window to reconnect.");
   _beginthread(WaitForInterrupt, NULL, 0x8000, NULL );
block:
   SerialConnect( JUST_WAIT, DbgGetProcessID(), _DBG, SendMsgToDbgQue );
   fmterr("Reconnected.");
   DosSelectSession(0);
  }
#if 0
  if(iview) {
      func=BROWSEMSH;
  }
  else
#endif
  {
     func = (BlowBy)?TOGGLESTORAGE:GetFuncsFromEvents(0 , (void *)fp );                           /*701*/
  }

  if( (SerialParallel() == SERIAL) && (QueryConnectSema4() == SEMA4_RESET) )
  {
   ConnectThisPid( INFORM_ESP );
   goto block;
  }

  /***************************************************************************/
  /* GetFuncsFromEvents will not always be able to return an executable   701*/
  /* function code, so we get back a raw LEFTMOUSECLICK or RIGHTMOUSECLICK701*/
  /* So, what we do is:                                                   701*/
  /*                                                                      701*/
  /*  -LEFTMOUSECLICK                                                     701*/
  /*    - if the click is in the source window, then set the cursor       701*/
  /*      position.                                                       701*/
  /*    - if the click is in the data window, then switch to the data     701*/
  /*      window.                                                         701*/
  /*                                                                      701*/
  /*  -RIGHTMOUSECLICK                                                    701*/
  /*    - bring up the object pulldown for the source window.             701*/
  /*                                                                      701*/
  /***************************************************************************/

   if( func == LEFTMOUSECLICK )                                         /*701*/
   {                                                                    /*701*/
     if( GetEventView() == DATAVIEW )                                   /*701*/
     {                                                                  /*701*/
       SetDataViewCsr();                                                /*701*/
       func = TOGGLESTORAGE;                                            /*701*/
     }                                                                  /*701*/
     else                                                               /*701*/
       func = SETCURSORPOS;                                             /*701*/
   }                                                                    /*701*/

ReCirculate:                                                            /*701*/
  /***************************************************************************/
  /* Process the function.                                                701*/
  /***************************************************************************/
  switch( func )                        /* switch on function selection      */
  {                                                                     /*701*/
/*****************************************************************************/
/* move the cursor up one line.                                              */
/*****************************************************************************/
   case UPCURSOR:                       /*                                   */
    if( CsrRow == 0 )                   /* move the cursor up one line.      */
     ascroll(fp, -1);                   /* scroll down one line if trying    */
    else                                /* to go above the screen.           */
     CsrRow--;                          /*                                   */
    break;                              /*                                   */
/*****************************************************************************/
/* move the curdor down one line.                                            */
/*****************************************************************************/
    case DOWNCURSOR:                    /*                                   */
     if( ++CsrRow >= RowsUsed )         /* move the cursor down one line.    */
     {                                  /* scroll up  one line if trying     */
      CsrRow = RowsUsed-1;              /* to go below the screen.           */
      if( RowsUsed == (uint)AsmRows )   /*                                   */
      {                                 /*                                   */
       ascroll(fp, 1);                  /*                                   */
      }                                 /*                                   */
     }                                  /*                                   */
     break;                             /*                                   */
/*****************************************************************************/
/* scroll the window up one line. Anchor cursor to its current line.         */
/***x*************************************************************************/
    case LEFTCURSOR:                    /*                                   */
     ascroll(fp,-1);                    /*                                   */
     if( CsrRow < RowsUsed - 1 )        /*                                   */
      CsrRow++;                         /*                                   */
      break;                            /*                                   */
/*****************************************************************************/
/* scroll the window one line down. Anchor curor to its current line.        */
/*****************************************************************************/
    case RIGHTCURSOR:                   /*                                   */
     ascroll(fp,1);                     /*                                   */
     if( CsrRow > 0 )                   /*                                   */
      CsrRow--;                         /*                                   */
      break;                            /*                                   */
/*****************************************************************************/
/* the infamous page up.                                                     */
/*****************************************************************************/
    case PREVWINDOW:                    /*                                   */
     ascroll(fp, 1 - AsmRows);          /*                                   */
     break;                             /*                                   */
/*****************************************************************************/
/* page down.                                                                */
/*****************************************************************************/
    case NEXTWINDOW:                    /*                                   */
     if( RowsUsed == (uint)AsmRows )    /*                                   */
      ascroll(fp, AsmRows - 1);         /*                                   */
     break;                             /*                                   */
/*****************************************************************************/
/*                                                                           */
/*****************************************************************************/
/*        case FIRSTWINDOW:               ???
            fp->topoff = dp->loIP;        ???
???         ascroll(fp, 0);          */
/*****************************************************************************/
/* move the cursor to the top of the window.                                 */
/*****************************************************************************/
    case TOPOFWINDOW:                   /*                                   */
     CsrRow = 0;                        /*                                   */
     break;                             /*                                   */
                                        /*                                   */
/*****************************************************************************/
/*                                                                           */
/*****************************************************************************/
/* ???    case LASTWINDOW:
   ???      fp->topoff = dp->hiIP;
   ???      ascroll(fp, 1 - AsmRows); */
/*****************************************************************************/
/* move the cursor to the bottom of the window.                              */
/*****************************************************************************/
    case BOTOFWINDOW:                   /*                                   */
     CsrRow = RowsUsed-1;               /*                                   */
     break;                             /*                                   */
/*****************************************************************************/
/* expand and shrink storage window.                                         */
/*****************************************************************************/
   case SHRINKSTORAGE:                  /* shrink the storage display area   */
   case EXPANDSTORAGE:                  /* Expand the storage display area   */
    if( HideDataWin == TRUE  )          /* if data window is hidden,         */
    {                                   /* turn it on.                       */
     Recalibrate(fp,-(int)SaveTopLine); /* resize the src/asm window.     521*/
     CsrRow -= TopLine;                 /* adjust the cursor.                */
     HideDataWin = FALSE;               /* TopLine gets reestablished in     */
     break;                             /*                                   */
    }                                   /* the recalibrate....puke!          */
   switch( func )                       /*                                   */
   {                                    /*                                   */
    case SHRINKSTORAGE:                 /*                                   */
     if( ShrinkStorage(fp,0) )          /*                                   */
      beep();                           /* beep if not shrinkable            */
     else                               /*                                   */
      if( RowsUsed == (uint)AsmRows )   /*                                305*/
       if(CsrRow < RowsUsed) CsrRow++;  /*                                   */
     break;                             /*                                   */
                                        /*                                   */
    case EXPANDSTORAGE:                 /*                                   */
     if( ExpandStorage(fp,0) )          /*                                   */
      beep();                           /* beep if not expandable            */
     else                               /*                                   */
      if( RowsUsed == (uint)AsmRows )   /*                                305*/
         if(CsrRow > 0) CsrRow--;       /*                                   */
     break;                             /*                                   */
   }                                    /*                                   */
   break;                               /*                                   */
/*****************************************************************************/
/* single step. step over calls.                                             */
/*****************************************************************************/
    case SSTEP:
    case SSTEPNOSWAP:

     if ( GetThreadState( GetExecTid() ) == TRC_C_ENDED )
     {
      beep();
      fmterr( "No threads or thread not found. Can't step anything.");
      break;
     }

     if ( GetThreadState( GetExecTid() ) == TRC_C_BLOCKED )
     {
      beep();
      fmterr( "Can't single step a blocked thread");
      break;
     }
     if ( GetThdDbgState( GetExecTid() ) == TS_FROZEN )
     {
      beep();
      fmterr( "Can't single step a frozen thread");
      break;
     }
     if( fp == GetExecfp() )
     {
       if( func == SSTEP )
         return( ASMSSTEP );
       else
         return( ASMSSTEPNOSWAP );
     }
     goto caseNOGO;
/*****************************************************************************/
/* single step. DO NOT step over calls.                                      */
/*****************************************************************************/
    case SSTEPINTOFUNC:
    case SSTEPINTOFUNCNOSWAP:

     if ( GetThreadState( GetExecTid() ) == TRC_C_ENDED )
     {
      beep();
      fmterr( "No threads or thread not found. Can't step anything.");
      break;
     }

     if ( GetThreadState( GetExecTid() ) == TRC_C_BLOCKED )
     {
      beep();
      fmterr( "Can't single step a blocked thread");
      break;
     }
     if ( GetThdDbgState( GetExecTid() ) == TS_FROZEN )
     {
      beep();
      fmterr( "Can't single step a frozen thread");
      break;
     }
     if( fp == GetExecfp() )
     {
       if( func == SSTEPINTOFUNC )
         return( ASMSSTEPINTOFUNC );
       else
         return( ASMSSTEPINTOFUNCNOSWAP );
     }
     return( ASMSSTEPINTOFUNC );
/*****************************************************************************/
/* Let 'er rip.                                                              */
/*****************************************************************************/
    case RUNNOSWAP:                     /* turn off screen swapping & go  511*/
    case RUN:                           /*                                   */
    caseRUN:                            /*                                   */
     if( GetExecfp() )                  /*                                   */
      return( func );                   /*                                   */
    caseNOGO:                           /*                                   */
     if( !GetExecfp() )                 /* if the afile is not valid, then   */
      fmt2err( "Can't continue:", msg );/* we cannot continue.               */
     goto complain;                     /* go cry about it.                  */
                                        /*                                   */
/*****************************************************************************/
/* Let 'er rip to the current cursor location.                               */
/*                                                                           */
/* If there is a breakpoint on the current cursor instruction address,       */
/* CsrInstAddr, then just run to it. Otherwise, try to set a breakpoint      */
/* on this address and run to it. Complain if a break cannot be set.         */
/*                                                                           */
/*****************************************************************************/
    case RUNTOCURSOR:                             /*                         */
    case RUNTOCURSORNOSWAP:                       /*                      827*/
     CsrInstAddr = AddrCache[CsrRow];             /*                      235*/
     if( IfBrkOnAddr(CsrInstAddr) )               /*                         */
      goto caseRUN;                               /*                         */
     if( CsrInstAddr == NULL )
      goto complain;
     SetAddrBRK(fp, CsrInstAddr, BRK_ONCE);
     goto caseRUN;

/*****************************************************************************/
/*                                                                           */
/*****************************************************************************/
     case GENHELP:                        /* General help                      */
     {
      uchar *HelpMsg;

      HelpMsg = GetHelpMsg( HELP_WIN_ASM, NULL,0);
      CuaShowHelpBox( HelpMsg );
      break;
     }

     case FUNCKEYSHELP:                                                 /*808*/
       HelpScreen( );              /* enter our help function ( ESC key */
                                   /*   to return here )                */
        break;                     /* for now, no context sensitive help*/
/*****************************************************************************/
/* Show the DLL names for EXE file.                                          */
/*****************************************************************************/
    case SHOWDLLS:                      /* show DLL names                    */
     ShowDlls();                        /*                                   */
     break;                             /*                                   */
/*****************************************************************************/
/* Find the executing addr and bring it onto the display.  What this guy does*/
/* is locate the executing line in the current afile if the cursor is not on */
/* the executing line.  If the cursor is on the executing line, then we start*/
/* back around the ring of stack frames showing the executing line in each   */
/* afile.                                                                    */
/*                                                                           */
/*****************************************************************************/
    case FINDEXECLINE:                  /*                                   */
     if( GetExecfp() )                  /*                                   */
     {                                  /*                                   */
      CsrInstAddr = AddrCache[CsrRow];  /*                                235*/
      fp_focus = FindExecAddr(fp, (uint)CsrInstAddr);/*                      */
      if( fp_focus )                    /*                                   */
      {
       fp_focus->flags  = AF_ZOOM;      /*                                235*/
       return(-1);                      /*                                   */
      }
     }                                  /*                                   */
     goto complain;                     /*                                   */
/*****************************************************************************/
/* bring up the thread menu.                                                 */
/*****************************************************************************/
    case SHOWTHREADS:                   /* show process threads              */
    {                                   /*                                   */
     uint  tid;                         /*                                   */
     tid  = GetExecTid();               /*                                   */
     showthds(&key);                    /*                                229*/
     if ( tid  != GetExecTid()  )       /*                                   */
      return( GOTID );                  /*                                   */
     break;                             /*                                   */
    }                                   /*                                   */

/*****************************************************************************/
/* bring up the process menu.                                                */
/*****************************************************************************/
   case SHOWPROCESSES:
    Cua_showproc();
    break;

/*****************************************************************************/
/* Hide the data window, but save the state for later return.                */
/*                                                                           */
/*****************************************************************************/
   case SHOWHIDESTORAGE:                /*                                   */
    if( HideDataWin == TRUE  )          /* if data window is hidden,         */
    {                                   /* turn it on.                       */
     Recalibrate(fp,-(int)SaveTopLine); /* resize the src/asm window.     521*/
     CsrRow -= (TopLine - VioStartOffSet);
                                        /* adjust the cursor.                */
     HideDataWin = FALSE;               /* TopLine gets reestablished in     */
    }                                   /* the recalibrate....puke!          */
    else                                /* else if the window is not         */
    {                                   /* hidden, then turn it off.         */
     SaveTopLine = TopLine - VioStartOffSet;
                                        /* hold top line for restore.        */
     CsrRow += (TopLine - VioStartOffSet);
                                        /* adjust the cursor.                */
     Recalibrate(fp,TopLine - VioStartOffSet);
                                        /* resize and refresh the window.    */
/*
     SaveTopLine = TopLine;
     CsrRow += TopLine;
     Recalibrate(fp,TopLine);
*/
                                        /* will set TopLine = 0              */
     HideDataWin = TRUE;                /* set hidden mode flag.             */
    }                                   /*                                   */
    break;                              /*                                   */
/*****************************************************************************/
/* Toggle to and from the storage window.                                    */
/*****************************************************************************/
    case TOGGLEHIDESTORAGE:
    case TOGGLESTORAGE:
     Newfp = NULL;
     func = typoS((SIZEFUNC)Recalibrate, fp, &Newfp);
     if( Newfp )
     {
      fp_focus = Newfp;
      return(-1);
     }
     if( func != 0 )                                                    /*701*/
      goto ReCirculate;                                                 /*701*/
     goto Refresh_Screen;                                               /*701*/

/*************************************************************************235*/
/* toggle between source view and disassembly view.                       235*/
/*                                                                        235*/
/*************************************************************************235*/
    case TOGGLEASM:                                                     /*235*/
                                                                        /*235*/
     if( fp->source == NULL )                                           /*235*/
     {                                                                  /*235*/
      beep();                                                           /*235*/
      break;                                                            /*235*/
     }                                                                  /*235*/
                                                                        /*235*/
     /********************************************************************235*/
     /* - come here if there is source for this file.                     235*/
     /* - SyncAddr allows the source view to be synchronized with the asm.235*/
     /********************************************************************235*/
     switch( fp->sview )                                                /*235*/
     {                                                                  /*235*/
      case MIXEDN:                                                      /*402*/
                                                                        /*402*/
       if( (AddrCache[CsrRow] != 0)                                     /*531*/
            && (CsrRow != 0) )                                          /*531*/
        for(; (int)CsrRow >= 0 && AddrCache[CsrRow] != 0 ; CsrRow--){;} /*531*/
                                                                        /*402*/
       fp->topline  = SourceLineCache[CsrRow] - CsrRow - fp->Nbias;     /*402*/
       fp->csrline  = SourceLineCache[CsrRow] - fp->Nbias;              /*402*/
       if( (int)fp->topline < 1 )                                       /*402*/
        fp->topline = 1;                                                /*402*/
       break;                                                           /*402*/
                                                                        /*402*/
      case NOSRC:                                                       /*402*/
                                                                        /*235*/
       fp->flags |= AF_SYNC;
       SyncAddr = AddrCache[CsrRow];                                    /*235*/
       break;                                                           /*235*/
     }                                                                  /*235*/

     (fp_focus = fp)->shower = showC;                                   /*235*/
     return(-1);                                                        /*235*/

    /*************************************************************************/
    /* If Displaying the register window                                  400*/
    /*  - Set the REPAINT bit.                                            400*/
    /* Toggle the REGS386BIT bit.                                         400*/
    /* If the coprocessor register window is already on                   400*/
    /*  - reset the screen bounds                                         400*/
    /*  - refresh the data window.                                        400*/
    /*  - reset the REGS387BIT bit.                                       400*/
    /* If the user is turning off the register display                    400*/
    /*  - reset the screen bounds                                         400*/
    /*  - refresh the data window.                                        400*/
    /* Format the screen                                                  400*/
    /*************************************************************************/
    case REGISTERDISPLAY:                                               /*400*/
     if (!TestBit(Reg_Display,REGS386BIT))                              /*400*/
        SetBit(Reg_Display,REPAINT);                                    /*400*/
     if (TestBit(Reg_Display,REGS387BIT) && !iview)                     /*401*/
     {                                                                  /*401*/
        memset(BoundPtr,VideoCols,VideoRows);                           /*518*/
        ResetBit(Reg_Display,REGS387BIT);                               /*401*/
        ShowData(TopLine);                                              /*400*/
     }                                                                  /*401*/
     Reg_Display ^= 0x2;                                                /*400*/
     if (!TestBit(Reg_Display,REGS386BIT))                              /*400*/
     {                                                                  /*400*/
        memset(BoundPtr,VideoCols,VideoRows);                           /*518*/
        ShowData(TopLine);                                              /*400*/
#if 0
        RemoveCollisionArea();                                          /*701*/
#endif
     }                                                                  /*400*/
     goto Refresh_Screen;                                               /*400*/

    /*************************************************************************/
    /* If Displaying the co processor registers                           401*/
    /*  - Set the REPAINT bit.                                            401*/
    /* Toggle the REGS387BIT bit.                                         401*/
    /* If the register window is already on                               401*/
    /*  - reset the screen bounds                                         401*/
    /*  - reset the REGS386BIT bit.                                       401*/
    /* If the user is turning off the coproessor register display         401*/
    /*  - reset the screen bounds                                         401*/
    /*  - refresh the data window.                                        401*/
    /* Format the screen                                                  401*/
    /*************************************************************************/
    case COREGISTERDISPLAY:                                             /*401*/
     if (!TestBit(Reg_Display,REGS387BIT))                              /*401*/
        SetBit(Reg_Display,REPAINT);                                    /*401*/
     if (TestBit(Reg_Display,REGS386BIT) && !iview)                     /*401*/
     {                                                                  /*401*/
        ResetBit(Reg_Display,REGS386BIT);                               /*401*/
        memset(BoundPtr,VideoCols,VideoRows);                           /*518*/
     }                                                                  /*401*/
     Reg_Display ^= 0x4;                                                /*401*/
     if (!TestBit(Reg_Display,REGS387BIT))                              /*401*/
     {                                                                  /*401*/
        memset(BoundPtr,VideoCols,VideoRows);                           /*518*/
        ShowData(TopLine);                                              /*401*/
#if 0
        RemoveCollisionArea();                                          /*701*/
#endif
     }                                                                  /*401*/
     goto Refresh_Screen;                                               /*401*/

    /*************************************************************************/
    /* Edit the 30386 registers.                                             */
    /* If the coprocessor register window is already on                      */
    /*  - reset the screen bounds                                            */
    /*  - reset the REGS387BIT bit.                                          */
    /*  - refresh the data window.                                           */
    /* If Register window is not present bring it up.                        */
    /* Process the editing of registers.                                     */
    /* If the register window was not up when we came here turn it off.      */
    /* Format the screen                                                     */
    /*************************************************************************/
    case EDITREGISTERS:
      if (TestBit(Reg_Display,REGS387BIT))
      {
        GetScrAccess();
        ResetBit(Reg_Display,REGS387BIT);
        ShowData(TopLine);
      }
      if (!TestBit(Reg_Display,REGS386BIT))
      {
        SetBit(Reg_Display,REPAINT);
        ShowvRegs();
      }
      Newfp = KeyvRegs(&key);
      if (!TestBit(Reg_Display,REGS386BIT))
      {
        memset(BoundPtr+VioStartOffSet,VideoCols,VideoRows-VioStartOffSet);
        ShowData(TopLine);
        fp->flags |= ASM_VIEW_REFRESH;
        ascroll( fp, 0 );
      }
#if 0
      RemoveCollisionArea();                                            /*701*/
#endif
      if (Newfp)
      {
        fp_focus = Newfp;
        return(-1);
      }
      break;

   /**************************************************************************/
   /* Set Execution Line Function.                                        508*/
   /*                                                                     508*/
   /*  - Check if the Line is executable, if not put error message.       508*/
   /*  - If the current address is a valid address, call function to      508*/
   /*    change the CS:IP and then refresh the screen.                    508*/
   /*                                                                     508*/
   /**************************************************************************/
   case SETEXECLINE:                                                    /*508*/
   {                                                                    /*508*/
    CsrInstAddr = AddrCache[CsrRow];                                    /*508*/
    if( CsrInstAddr != NULL )                                           /*508*/
    {                                                                   /*508*/
      if (SetExecLine(CsrInstAddr) == TRUE)                             /*508*/
         goto Refresh_Screen;                                           /*508*/
    }                                                                   /*508*/
    beep();                                                             /*508*/
    fmterr( "Can't set ip at this line" );                              /*508*/
   }                                                                    /*508*/
   break;                                                               /*508*/

/*****************************************************************************/
/* set global flag for MASM-AL86 mnemonics.                                  */
/*****************************************************************************/
    case TOGGLEDIS:                     /* user wants to toggle disassembly? */
     AL86orMASM ^= 1;                   /* toggle the AL/86 or MASM flag     */
     MneChange = 1;                     /* toggle the AL/86 or MASM flag     */
     fp->flags |= ASM_VIEW_MNE;         /* change mnemonic view.          235*/
     ascroll( fp, 0 );                  /* show the new disassembly mode     */
     SetBitOff(fp->flags, ASM_VIEW_MNE);                                /*235*/
     break;                             /* end user wants to toggle disasm   */
/*****************************************************************************/
/* show the users call stack and execute options.                            */
/*****************************************************************************/
    case SHOWCALLSTACK:                 /*                                   */
     switch( ActiveProcsMenu(&fp_focus))/*                                   */
      { case A_ENTER:                   /* run to the return csip of the     */
        case C_ENTER:                   /* selected function.                */
          func = RUN;                   /*                                701*/
          goto caseRUN;                 /*                                   */
        case ENTER:                     /* switch to context of the selected */
          return(-1);                   /* function.                         */
      }                                 /*                                   */
      break;                            /*                                   */
/*****************************************************************************/
/* show the user's application.                                              */
/*****************************************************************************/
   case TIMEDSHOWAPPWINDOW:
    if( IsEspRemote() )
     xSelectSession();
    else
     DosSelectSession(DbgGetSessionID());
    DosSleep(5000L);
    DosSelectSession( 0 );
    break;
/*****************************************************************************/
/* set/reset conditional breakpoint. not available at asm level yet.( Never?)*/
/*****************************************************************************/
    case SETCLEARCONDBKPT:
      beep();
      fmterr( "Function not available" );
      break;
/*****************************************************************************/
/*                                                                           */
/*****************************************************************************/
    case SETCLEARBKPT:                  /* set/reset breakpoint              */
    {
     CsrInstAddr = AddrCache[CsrRow];
     if( CsrInstAddr == NULL )
      goto complain;
     SetAddrBRK(fp, CsrInstAddr, BRK_SIMP);
     fp->flags |= ASM_VIEW_REFRESH;
     ascroll(fp,0);
     SetBitOff(fp->flags, ASM_VIEW_REFRESH );
     break;
    }

   case SAVEBKPTS:
       SaveBreakpoints();
       break;

   case RESTOREBKPTS:
       ResetBreakpointFileTime();
       RestoreBreakpoints();
       break;

   case EDITBKPTS:
       EditBreakpoints();
       break;
                                                                        /*701*/
/*****************************************************************************/
/* process  menu options.                                                    */
/*****************************************************************************/
#if 0
    case ACTIONBAR:                     /*                                   */
                                        /*  if changing context, then return */
     if( fp_focus = AsmActionBar(fp, func) )                            /*235*/
     {                                                                  /*235*/
      if( fp_focus->shower == showA && fp_focus->flags == 0 )           /*235*/
      {                                                                 /*235*/
       fp_focus->flags = ASM_VIEW_CHANGE;                               /*235*/
       fp_focus->csr.row = fp_focus->csrline - 1;                       /*235*/
       if(fp_focus->sview == NOSRC )                                    /*235*/
        fp_focus->csr.row = 0;                                          /*235*/
      }                                                                 /*235*/
      return(-1);                                                       /*235*/
     }                                                                  /*235*/
     goto Refresh_Screen;
     {
      if( fp_focus->flags == 0 )        /*                                235*/
       fp_focus->flags  = AF_SYNC;      /*                                235*/
      return(-1);                       /*  to Run() with the new fp.        */
     }
    fmtasm( fp );                       /*                                235*/
    fmterr( msg );                      /*                                235*/
    fp->flags |= ASM_VIEW_REFRESH;      /*                                235*/
    ascroll(fp,0);                      /*                                235*/
    SetBitOff(fp->flags, ASM_VIEW_REFRESH );  /*                          235*/
    break;                              /*                                235*/
#endif
                                        /*                                   */
/*****************************************************************************/
/* browse any file.                                                          */
/*****************************************************************************/
   case BROWSE:                         /* browse any file.                  */
    BrowseFile();                                                       /*701*/
    goto Refresh_Screen;
                                                                        /*701*/

/*****************************************************************************/
/* browse any MSH LOG FILE                                                   */
/*****************************************************************************/
#ifdef MSH
   case BROWSEMSH:
    {
    WINDOWEVENT *we;
    iview=1;
    we=BrowseMshFile(fp);
    iview=0;
    if(we->func) {
        iview=we->iview;
        func=we->func;
//      SrcCol = fp->skipcols + fp->csr.col;  /* set  column position              */
//      SrcLine = fp->csrline + fp->Nbias;    /* removed a +1.                  234*/
//      lp = fp->source +                     /* ->to top of source buffer +       */
//           fp->offtab[ fp->csrline ];       /* offset of cursorline              */
        CsrRow=fp->csr.row-we->Window->row;
        goto ReCirculate;
    }
    goto Refresh_Screen;
    }
#endif
/*****************************************************************************/
/* go to next file in the afile ring.                                        */
/*****************************************************************************/
   case NEXTFILE:                       /* switch to th next file in ring    */
    if( !(fp_focus = fp->next) )        /* is there a next one in the ring   */
     if( (fp_focus = allfps) == fp )    /* is there only one file?           */
     {                                  /*                                   */
      beep();                           /* yes only one file                 */
      break;                            /*                                   */
     }                                  /*                                   */
    if(fp_focus->shower == showA )                                      /*235*/
     fp_focus->flags = ASM_VIEW_NEXT;                                   /*235*/
    return(-1);                         /* return to run with recalled afi   */
#if 0
/*****************************************************************************/
/* toggle disasm line views.                                                 */
/*****************************************************************************/
   case TOGGLEASMLINE:                  /*                                   */
   {                                    /*                                   */
                                        /*                                   */
    fp->lview=(uchar)                   /* change to next asm line view.     */
              ( (fp->lview==ASMLINEVIEWS) /* change to next asm line view.   */
               ?1:++fp->lview  );       /*                                   */
    fmtasm( fp );                       /* reformat the screen.              */
    fmterr( msg );                      /* add error message if necessary.   */
    ascroll( fp, 0 );                   /* refresh the screen.               */
    break;                              /*                                   */
   }                                    /*                                   */
#endif
/*************************************************************************235*/
/* toggle source/asm views.                                               235*/
/*************************************************************************235*/
   case TOGGLEASMVIEW:                                                  /*235*/
   {                                                                    /*235*/
    if( fp->topoff == 0 )                                               /*817*/
      fp->topoff = GetExecAddr();                                       /*817*/
    if( fp->source == NULL )                                            /*235*/
     beep();                                                            /*235*/
    else                                                                /*235*/
    {                                                                   /*235*/
     fp->flags |= ASM_VIEW_TOGGLE;                                      /*235*/
     if( fp->sview == NOSRC )                                           /*235*/
     {                                                                  /*235*/
      {
       LNOTAB *pLnoTabEntry;

       DBMapInstAddr(AddrCache[CsrRow], &pLnoTabEntry, fp->pdf);
       if( pLnoTabEntry )
        fp->csrline = pLnoTabEntry->lno;
      }
      fp->topline = fp->csrline - CsrRow;                               /*235*/
      if( (int)fp->topline < 0 )                                        /*235*/
       fp->topline = 0;                                                 /*235*/
      fp->sview = MIXEDN;                                               /*235*/
     }                                                                  /*235*/
     else /* (fp->sview == MIXEDN ) */                                  /*235*/
      fp->sview = NOSRC;                                                /*235*/
                                                                        /*235*/
     return(-1);                                                        /*235*/
    }                                                                   /*235*/
                                                                        /*235*/
                                                                        /*235*/
                                                                        /*235*/
#if 0
    fmtasm( fp );                       /* reformat the screen.              */
    fmterr( msg );                      /* add error.                        */

    fp->flags |= AF_SYNC;               /*                                235*/
    ascroll(fp,0);                      /*                                235*/
    SetBitOff(fp->flags, AF_SYNC );     /*                                235*/
    if( CsrRow >= RowsUsed )            /*                                235*/
     CsrRow = RowsUsed - 1;             /*                                235*/
#endif
    break;                              /*                                   */
   }                                    /*                                   */

/*****************************************************************************/
/* clear all break points                                                    */
/*                                                                           */
/*****************************************************************************/
   case CLEARALLBKPTS:                  /* Clear all program breaks.      106*/
   {
    FreeAllBrks();                      /*                                106*/
    fp->flags |= ASM_VIEW_REFRESH;      /*                                235*/
    ascroll(fp,0);                      /*                                235*/
    SetBitOff(fp->flags, ASM_VIEW_REFRESH );  /*                          235*/
    break;                              /*                                235*/
   }

   /**************************************************************************/
   /* Set user colors.                                                       */
   /**************************************************************************/
   case SETCOLORS:                      /* set user colors.                  */
   {                                    /*                                   */
    SetColors();                        /*                                   */
    DisplayMenu();                      /* Redisplay the menu bar.           */
    if((TestBit(Reg_Display,REGS386BIT))/* If either 386 registers or 387 400*/
    ||(TestBit(Reg_Display,REGS387BIT)))/* registers window is on         401*/
        SetBit(Reg_Display,REPAINT);    /* set the reg win paint bit.     400*/
    goto Refresh_Screen;                /* update color changes              */
   }                                    /*                                   */

   /**************************************************************************/
   /* Set user exceptions notifications                                   308*/
   /**************************************************************************/
   case SETEXCEPTIONS:
     SetExceptions();
     break;

   /**************************************************************************/
   /* Set watch points                                                    602*/
   /**************************************************************************/
   case SETWATCHPOINTS:
     SetWps(fp);
     break;

   case SETCURSORPOS:                                                   /*701*/

     SetAsmViewCsr( fp, &CsrRow );                                      /*701*/
     break;                                                             /*701*/

   case DROPFILE:                                                       /*701*/
    {                                                                   /*701*/
     AFILE *fpnext;                                                     /*701*/
                                                                        /*701*/
     fpnext = fp->next;                                                 /*701*/
     if ( fpnext == NULL )                                              /*701*/
     fpnext = allfps;                                                   /*701*/
     freefp( fp );                                                      /*701*/
     fp = fpnext;                                                       /*701*/
     if( fp->shower == showA && fp->flags == 0 )                        /*701*/
     {                                                                  /*701*/
      fp->flags = ASM_VIEW_CHANGE;                                      /*701*/
      fp->csr.row = fp->csrline - 1;                                    /*701*/
      if(fp->sview == NOSRC )                                           /*701*/
       fp->csr.row = 0;                                                 /*701*/
     }                                                                  /*701*/
     fp_focus = fp;
     return(-1);                                                        /*701*/
    }                                                                   /*701*/
                                                                        /*701*/
   case RESTART:
     Restart();
     break;

   case QUIT:
     NormalQuit(TRUE);
     break;

   case SETFUNCTIONBKPT:
   case SETADDRESSBKPT:
   case SETDEFERREDBKPT:
   case SETADDRLOADBKPT:
   case SETDLLLOADBKPT:
    SetNameOrAddrBkpt( fp,func);                                        /*701*/
    goto Refresh_Screen;                                                /*701*/

   case GETFUNCTION:                                                    /*701*/
   case GETFILE:                                                        /*701*/
   case GETADDRESS:                                                     /*701*/
    {
     char  InitString[81];                                              /*701*/
     if( func == GETFUNCTION || func == GETADDRESS )                    /*701*/
     {                                                                  /*701*/
      /*******************************************************************701*/
      /* Cursor sensitive prompting is not turned on in the assembler     701*/
      /* view. If it's GETADDRESS, then help the use out by throwing in a 701*/
      /* 0x for him.                                                      701*/
      /*******************************************************************701*/
      InitString[0] = '\0';                                             /*701*/
      if( func == GETADDRESS )                                          /*701*/
       strcpy( InitString,"0x");                                        /*701*/
      fp_focus = GetFunction( InitString, func );                       /*701*/
     }                                                                  /*701*/
     else                                                               /*701*/
      fp_focus = GetF( );                                               /*701*/
                                                                        /*701*/
     if( fp_focus )                                                     /*701*/
     {                                                                  /*701*/
      if( fp_focus->shower == showA && fp_focus->flags == 0 )           /*701*/
      {                                                                 /*701*/
       fp_focus->flags = ASM_VIEW_CHANGE;                               /*701*/
       fp_focus->csr.row = fp_focus->csrline - 1;                       /*701*/
       if(fp_focus->sview == NOSRC )                                    /*701*/
        fp_focus->csr.row = 0;                                          /*701*/
      }                                                                 /*701*/
      return(-1);                                                       /*701*/
     }                                                                  /*701*/
    }
    goto Refresh_Screen;                                                /*701*/

   case ESCAPE:                                                         /*701*/
     goto Refresh_Screen;                                               /*701*/

   case DONOTHING:
     continue;

   default:
   complain:
    beep();
  }
  continue;

    Refresh_Screen:
        fp->flags |= ASM_VIEW_REFRESH;
        fmtasm( fp );
        fmterr( msg );
        /*********************************************************************/
        /* This will fake showa into thinking that there has been a change   */
        /* in mnemonics so that the disassembly will be unconditionally      */
        /* updated. We do this because code may have been patched and we     */
        /* want to force a refresh of the disassembly.                       */
        /*********************************************************************/
        MneChange = 1;                  /* force unconditional refresh       */
        ascroll( fp, 0 );               /* of the assembler window           */
        SetBitOff(fp->flags, ASM_VIEW_REFRESH );
 }
}

/*****************************************************************************/
/* Recalibrate()                                                             */
/*                                                                           */
/* Description:                                                              */
/*   Dereference a pointer.                                                  */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   fp        input - afile for this asm window.                            */
/*   adjust    input - how much to adjust this window.                       */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   none.                                                                   */
/*                                                                           */
/*****************************************************************************/
extern uint         MaxPer;             /* max # of lines for source code    */
int
Recalibrate(AFILE *fp, int  adjust )
{
 int n = adjust;                        /* was register.                  112*/
 int delta;                             /*                                235*/

 if( TopLine == VioStartOffSet &&       /* if the TopLine is maxed out and701*/
     adjust > 0                         /* caller wants it bigger, then      */
   )                                    /*                                   */
  return(0);                            /* don't do it.                      */
/*****************************************************************************/
/* We want to adjust the size of the asm window by n lines. It must remain   */
/* within its boundaries, else we don't adjust it. We also adjust the top    */
/* line of the source display.                                               */
/*****************************************************************************/
#if 0
 LinesPer += n;                         /* make adjustment and               */
#endif
 TopLine -= n;                          /* adjust the top line of the display*/
 if( TopLine > MaxData )                /* Limit to max data window size.    */
  TopLine = MaxData;                    /*                                   */
                                        /*                                   */
 LinesPer = MaxPer - TopLine;
 if( (int)(LinesPer) < (int)MinPer ||   /* check for in bounds.              */
     (int)LinesPer > (int)MaxPer )      /*                                   */
 {                                      /*                                   */
  LinesPer -= n;                        /* if not in bounds, then back out   */
  return(1);                            /* the adjustment and return error.  */
 }                                      /*                                   */
/*****************************************************************************/
/* now, size the lines available for disasm display lines and the register   */
/* variables.                                                                */
/*                                                                           */
/*****************************************************************************/
 AsmRows = LinesPer;                    /* rows available for assembly lines.*/
 AsmTop = TopLine;                      /* the top of the asm display.       */
 if( adjust )                           /*                                   */
 {                                      /*                                   */
  delta = -adjust;
  /***************************************************************************/
  /* We don't want to page down if there are no lines below the current      */
  /* display.                                                                */
  /***************************************************************************/
  if( delta > 0 && RowsUsed < AsmRows )
  {
   fp->flags |= ASM_VIEW_NEXT;                                          /*305*/
   ascroll( fp, 0 );                    /*                                235*/
   fmtasm( fp );                        /* display the window.               */
  }
  else
  /***************************************************************************/
  /* else, make the adjustment up or down as required.                       */
  /***************************************************************************/
  {
   ascroll( fp, delta   );
   fmtasm( fp );                        /* display the window.               */
  }
 }
 return(0);                             /* keep the compiler happy.          */
}

/*****************************************************************************/
/* SetAsmViewCsr()                                                        701*/
/*                                                                        701*/
/* Description:                                                           701*/
/*   Set the cursor position in the source window when a mouse event      701*/
/*   occurs.                                                              701*/
/*                                                                        701*/
/* Parameters:                                                            701*/
/*                                                                        701*/
/*   fp           -> to the afile structure that will be updated.         701*/
/*                -> CsrRow variable that keeps the current row in        701*/
/*                   the assembler view.                                  701*/
/*                                                                        701*/
/* Return:                                                                701*/
/*                                                                        701*/
/* Assumptions                                                            701*/
/*                                                                        701*/
/*  none                                                                  701*/
/*                                                                        701*/
/*****************************************************************************/
extern uint           FnameRow;                                         /*701*/
void  SetAsmViewCsr( AFILE *fp, uint *pCsrRow )                         /*701*/
{                                                                       /*701*/
 PEVENT  Event;                                                         /*701*/
                                                                        /*701*/
 Event = GetCurrentEvent();                                             /*701*/
                                                                        /*701*/
 if( Event->Row >= TopLine && Event->Row <  FnameRow )                  /*701*/
 {                                                                      /*701*/
  fp->csr.row = Event->Row;                                             /*701*/
  fp->csr.col = Event->Col;                                             /*701*/
  *pCsrRow = Event->Row - TopLine;                                      /*701*/
 }                                                                      /*701*/
}                                                                       /*701*/
