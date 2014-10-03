/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   showc.c                                                                 */
/*                                                                           */
/* Description:                                                              */
/*   Source window processing.                                               */
/*                                                                           */
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
/*... 08/06/91  221   srinivas  Users app being started in back ground       */
/*... 08/19/91  229   srinivas  ESC key should take back to action bar from  */
/*...                           showthds and editregs.                       */
/*... 08/22/91  234   Joe       PL/X gives "varname" is incorrect message    */
/*...                           when entering a parameter name in the data   */
/*...                           window.  This happens when the cursor is on  */
/*...                           an internal procedure definition statement   */
/*...                           and you use F2 to get into the data window   */
/*...                           and then type the name.                      */
/*... 08/30/91  235   Joe       Cleanup/rewrite ascroll() to fix several bugs*/
/*                                                                           */
/*...Release 1.00 (Pre-release 105 10/10/91)                                 */
/*...                                                                        */
/*... 10/31/91  308   Srinivas  Menu for exceptions.                         */
/*...                                                                        */
/*...Release 1.00 (Pre-release 107 11/13/91)                                 */
/*...                                                                        */
/*... 11/13/91  400   Srinivas  Vertical Register Display.                   */
/*...                                                                        */
/*... 11/18/91  401   Srinivas  Floating point Register Display.             */
/*...                                                                        */
/*...Release 1.00 (Pre-release 108 12/05/91)                                 */
/*...                                                                        */
/*... 01/28/92  508   Srinivas  Added Set Execution Line function.           */
/*... 02/05/92  511   Srinivas  Allow turning off screen swapping while      */
/*...                           executing.                                   */
/*... 02/10/92  517   Srinivas  Cusrsor sensitive prompting for GetFunc.     */
/*... 02/11/92  518   Srinivas  Remove limitation on max no of screen rows.  */
/*... 02/12/92  521   Srinivas  Port to C-Set/2.                             */
/*...                                                                        */
/*...Release 1.00 (03/03/92)                                                 */
/*...                                                                        */
/*... 03/10/92  602   Srinivas  Hooking up watch points.                     */
/*... 03/12/92  603   Srinivas  Problem with files longer than 80 cols.      */
/*...                                                                        */
/*...Release 1.01 (04/03/92)                                                 */
/*...                                                                        */
/*... 08/03/92  701   Joe       Cua Interface.                               */
/*... 10/05/92  701   Michelle  Help window.                                 */
/*... 01/12/93  808   Selwyn    Profile fixes/improvements.                  */
/*... 03/22/93  816   Selwyn    PgUp/PgDn not working after F6 in a large    */
/*...                           file.                                        */
/**Includes*******************************************************************/
                                        /*                                   */
#include "all.h"                        /* SD86 include files                */
static int   iview=0;   /*Shared with showa.c*/

uint str_fnd_flag;                      /* flag to indicate the wether to 110*/
                                        /* mark the word in reverse video 110*/

/**Externs********************************************************************/
extern uint slen;                       /* len of string found with FIND  110*/
extern AFILE      *allfps;              /* all known files                   */
extern AFILE      *fp_focus;            /* if return(-1)                     */
extern uint        VideoCols;           /* # of cols per screen              */
extern uint        VideoRows;           /* # of rows per screen           518*/
extern uint        HideDataWin;         /* data window hidden flag.          */
extern uint        LinesPer;            /* current # lines/screen for code   */
extern KEY2FUNC    defk2f[];            /*                                   */
extern uchar       Reg_Display = 0;     /* Register display flag          400*/
extern uchar      *BoundPtr;            /* -> to screen bounds            518*/
extern char        GetFuncBuffer[];     /* save buffer for GetFunc.       517*/
extern uint        VioStartOffSet;      /* flag to tell were to start scr 701*/
extern int        BlowBy;               /* blow by GetFuncsFromEvents flag701*/
extern CmdParms   cmd;
extern PROCESS_NODE *pnode;                                             /*827*/
extern UINT       TopLine;              /* current top line for code         */
extern UINT       MaxData;              /* max # of lines of Data            */

static AFILE      *showfp;              /* static var to hold current fp  701*/
static uchar      *lp;                  /* pointer line in source buffer  701*/
static uint       SrcCol;               /* cursor column position         701*/
static uint       SrcLine;              /* cursor source line position    701*/

UINT  SyncAddr;                         /* used to sync ASM and SOURCE views */
UINT  NoPaint;                          /* used by formatting routines also  */
UINT  DataFileBot;                      /* last rec shown.            (0..N) */
UINT  DataFileTop;                      /* 1st  rec shown.            (0..N) */
UINT  SaveTopLine;                      /* saved top line for data window.   */


/*                                                                           */
/**Begin Code*****************************************************************/
/*                                                                           */
/*****************************************************************************/
uint                                    /* chg return value to uint       521*/
showC( AFILE *fp , uchar *msg )         /* show the source & handle k/b      */
                                        /* AFILE block pointer               */
                                        /* message pointer                   */
{
 AFILE  *Newfp;
 uint   n;                              /* just an unsigned integer          */
 uint   key;                            /* key returned from GetKey()     101*/
 uint   span;                           /* # of bytes spanned by source line */
 ULONG  xline;                          /* disasm display sync line          */
 int    ScrollAmt;                      /* how much to scroll the screen     */
 int    CsrAdjust;                      /* how much to adjust the cursor     */
 int    func;                           /* function associated with a key    */
 int    rc;                             /* return code from ShowAppScr    521*/
 uchar     hlight[4];                   /* attributes for find string     110*/
 int    MouseState;
 char   InitString[81];

 LNOTAB *pLnoTabEntry;
                                        /*                                   */
 showfp = fp;

 if( fp->flags & AF_SYNC )              /* sync asm and source views         */
 {
  SetBitOff(fp->flags, AF_SYNC);        /* set "views synchronized"  in AFILE*/

  if(DBMapInstAddr(SyncAddr, &pLnoTabEntry, fp->pdf))
  {
   xline = 0;
   if(pLnoTabEntry) xline = pLnoTabEntry->lno;

   xline -= fp->Nbias;                  /* make relative to source buf begin */

   if( xline  > fp->Nlines )            /* if past end of the source buffer  */
    xline = fp->Nlines;                 /* then set it to last buffer line   */

   fp->topline += xline-1;              /* define the new topline            */

   fp->hotline = xline;                 /* define the hotline                */
   fp->flags |= AF_ZOOM;                /* set flag to show current exec line*/
  }
 }                                      /* end of sync asm and source views  */
/*
  The hotline is the source line number where execution has stopped.
*/

 NoPaint = (fp->flags & AF_ZOOM);       /* set "show current exec line" flag */
 if(!iview) fmtscr( fp );               /* show the screen for this afile    */

 if( NoPaint )
 {
/*****************************************************************************/
/* If the hotline falls above the currently shown screen, then scroll        */
/* up until the hotline is in view. The top margin is "ADJUST" lines down    */
/* from the top.                                                             */
/*                                                                           */
/*****************************************************************************/
  #define ADJUST  4                     /*                                   */
if(!iview) {
  if( (fp->hotline) < fp->topline )     /* if the hotline is above the    234*/
  {                                     /* screen, then scroll the screen    */
                                        /* to bring the hotline into view.   */
   ScrollAmt=fp->hotline-1 -            /*                                   */
             fp->topline;               /* adjust below top  of screen.      */
   ScrollAmt-=ADJUST;                   /*                                   */
   dovscr( fp, ScrollAmt );             /* now perform the scroll.           */
   }
}
else {
#ifdef MSH
  if( (fp->hotline) < fp->topline )     /* if the hotline is above the    234*/
  {                                     /* screen, then scroll the screen    */
                                        /* to bring the hotline into view.   */
   ScrollAmt=fp->hotline-1 -            /*                                   */
             fp->topline;               /* adjust below top  of screen.      */
   ScrollAmt-=ADJUST;                   /*                                   */
   Window[SOURCE_WIN]->user_data=(void *) fp;
   DoVscr(Window[SOURCE_WIN],ScrollAmt);
   }
#endif
}
/*****************************************************************************/
/* If the hotline falls below the currently shown screen, then scroll down   */
/* to bring the hotline into view. Also, provide margin adjustment.          */
/*                                                                           */
/*****************************************************************************/
if(!iview) {
  if( (fp->hotline) >=                  /* if the hot line is below the      */
      (fp->topline+fp->Nshown)          /* screen, then scroll screen down   */
    )                                   /*                                   */
  {                                     /*                                   */
   ScrollAmt=fp->hotline - fp->topline; /*                                234*/
   ScrollAmt-=ADJUST;                   /* adjust below the top of screen.   */
   dovscr( fp, ScrollAmt );             /* now perform the scroll.           */
  }                                     /*                                   */
}
else {
#ifdef MSH
  if( (fp->hotline) >=                  /* if the hot line is below the      */
      (fp->topline+fp->Nshown)          /* screen, then scroll screen down   */
    )                                   /*                                   */
  {                                     /*                                   */
   ScrollAmt=fp->hotline - fp->topline; /*                                234*/
   ScrollAmt-=ADJUST;                   /* adjust below the top of screen.   */
   Window[SOURCE_WIN]->user_data=(void *) fp;
   DoVscr(Window[SOURCE_WIN],ScrollAmt);
  }                                     /*                                   */
#endif
}

                                        /*                                   */
/*****************************************************************************/
/*                                                                           */
/*                                                                           */
/*                                                                           */
/*****************************************************************************/

  NoPaint = 0;                          /* ??????????????????????????????????*/
  SetBitOff(fp->flags, AF_ZOOM);        /* do the same in the AFILE struct   */

  CsrAdjust = fp->hotline - fp->csrline; /* set cursor adjustment amount  234*/
  if(!iview)
  dovcsr( fp, CsrAdjust );              /* then adjust it.                   */
  else {
#ifdef MSH
      Window[SOURCE_WIN]->user_data=(void *) fp;
      DoVcsr( Window[SOURCE_WIN], CsrAdjust );          /* then adjust it.                   */
#endif
  }

  fp->csrline = fp->hotline;            /*                                234*/
  if(!iview) fmtscr( fp );               /* show the screen for this afile    */
 }
 fmterr( msg );                         /* display the error message         */

 if( !IsOnCRT(fp) )                     /* is the cursor on the screen?      */
     fp->csrline = fp->topline;         /* no. put it on the top line        */

/*
Keyboard processing loop
*/
 for( ;; )
 {                                      /* begin k/b processing loop         */
  fmtpos( fp );                         /* put "line xx of yy" message out   */
  SrcCol = fp->skipcols + fp->csr.col;  /* set  column position              */
  SrcLine = fp->csrline + fp->Nbias;    /* removed a +1.                  234*/
  lp = fp->source +                     /* ->to top of source buffer +       */
       fp->offtab[ fp->csrline ];       /* offset of cursorline              */
  fp->csr.row = ( uchar )( fp->csrline- /* set cursor line in source buffer  */
                  fp->topline +         /* - source line at top of display   */
                  TopLine );            /* + topline for the code            */
  PutCsr( ( CSR * )&fp->csr );          /* set the cursor position           */

  if (str_fnd_flag == 1)                /* if string found highlight the  110*/
  {                                     /*  string on the screen at the   110*/
      hlight[0] = 0xFF;                 /* attribute string for highlight 110*/
      hlight[1] = (uchar)slen;          /* attribute string for highlight 110*/
      hlight[2] = Attrib(vaXline);      /* attribute string for highlight 110*/
      hlight[3] = 0;                    /* attribute string for highlight 110*/
      putrc(fp->csr.row,fp->csr.col,    /*  fp->csr.col .                 110*/
      &hlight[0]);                      /*                                110*/
      str_fnd_flag = 0;                 /*                                110*/
  }


  /***************************************************************************/
  /* Set the masked items for the source view.                            701*/
  /***************************************************************************/
  SetMenuMask( SOURCEVIEW );
  if( (fp == GetExecfp()) || (GetExecfp() == NULL) )
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
  if( func == LEFTMOUSECLICK )                                          /*701*/
  {                                                                     /*701*/
    if( GetEventView() == DATAVIEW )                                    /*701*/
    {                                                                   /*701*/
     SetDataViewCsr();                                                  /*701*/
     func = TOGGLESTORAGE;                                              /*701*/
    }                                                                   /*701*/
    else                                                                /*701*/
     func = SETCURSORPOS;                                               /*701*/
  }                                                                     /*701*/
  else                                                                  /*701*/
  if( func == RIGHTMOUSECLICK )                                         /*701*/
  {                                                                     /*701*/
    MouseState = STATE_BUTTON_PRESSED;                                  /*701*/
    func = GetObjectPullChoice( 11, &MouseState );                      /*701*/
  }                                                                     /*701*/
                                                                        /*701*/
ReCirculate:                                                            /*701*/
  /***************************************************************************/
  /* Process the function.                                                701*/
  /***************************************************************************/
  switch( func )                                                        /*701*/
                                                                        /*701*/
  {                                                                     /*701*/
                                                                        /*701*/
   case GETFUNCTION:                                                    /*701*/
   case GETFILE:                                                        /*701*/
   case GETADDRESS:                                                     /*701*/
    if( func == GETFUNCTION || func == GETADDRESS )                     /*701*/
    {                                                                   /*701*/
     /********************************************************************701*/
     /* If the function is GetFunction, then implement cursor sensitive   701*/
     /* prompting and initialize a token. If it's GETADDRESS, then        701*/
     /* help the use out by throwing in a 0x for him.                     701*/
     /********************************************************************701*/
     InitString[0] = '\0';                                              /*701*/
     if( func == GETADDRESS )                                           /*701*/
      strcpy( InitString,"0x");                                         /*701*/
     else                                                               /*701*/
      token( lp, SrcCol,InitString);                                    /*701*/
     fp_focus = GetFunction( InitString, func );                        /*701*/
    }                                                                   /*701*/
    else                                                                /*701*/
     fp_focus = GetF( );                                                /*701*/
                                                                        /*701*/
    if( fp_focus )                                                      /*701*/
    {                                                                   /*701*/
     if( fp_focus->shower == showA && fp_focus->flags == 0 )            /*701*/
     {                                                                  /*701*/
      fp_focus->flags = ASM_VIEW_CHANGE;                                /*701*/
      fp_focus->csr.row = fp_focus->csrline - 1;                        /*701*/
      if(fp_focus->sview == NOSRC )                                     /*701*/
       fp_focus->csr.row = 0;                                           /*701*/
     }                                                                  /*701*/
     return(-1);                                                        /*701*/
    }                                                                   /*701*/
    fmttxt( fp );                                                       /*701*/
    fmterr( msg );                                                      /*701*/
    break;                                                              /*701*/
                                                                        /*701*/
   case ESCAPE:                                                         /*701*/
    fmttxt( fp );                                                       /*701*/
    fmterr( msg );                                                      /*701*/
    break;                                                              /*701*/
                                                                        /*701*/
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
        fmttxt( fp );
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

   case FIND:                                                           /*701*/
    FindStr(fp);                                                        /*701*/
    fmtscr(fp);                                                         /*701*/
    break;                                                              /*701*/
                                                                        /*701*/
   case REPEATFIND:                                                     /*701*/
    rc = ScanStr(fp);                                                   /*701*/
    fmtscr(fp);                                                         /*701*/
    if( rc == 0 )                                                       /*701*/
     fmterr( "Can't find that" );                                       /*701*/
    break;                                                              /*701*/
                                                                        /*701*/
   case BROWSE:                                                         /*701*/
    BrowseFile();                                                       /*701*/
    fmtscr(fp);                                                         /*701*/
    break;                                                              /*701*/

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
        SrcCol = fp->skipcols + fp->csr.col;  /* set  column position              */
        SrcLine = fp->csrline + fp->Nbias;    /* removed a +1.                  234*/
        lp = fp->source +                     /* ->to top of source buffer +       */
             fp->offtab[ fp->csrline ];       /* offset of cursorline              */
        goto ReCirculate;
    }
    fmtscr(fp);                                                         /*701*/
    }
    break;                                                              /*701*/
#endif
                                                                        /*701*/
   case SETCLEARCONDBKPT:                                               /*701*/
    if( !(*(lp-1) & LINE_OK) )                                          /*701*/
    {                                                                   /*701*/
     beep();                                                            /*701*/
     fmterr( "Can't stop at this line" );                               /*701*/
     break;                                                             /*701*/
    }                                                                   /*701*/
    SetCondBrk( fp, SrcLine, lp );                                      /*701*/
    fmtscr( fp );                                                       /*701*/
    fmterr( "" );                                                       /*701*/
    break;                                                              /*701*/
                                                                        /*701*/
   case SAVEBKPTS:
    SaveBreakpoints();
    fmtscr(fp);
    break;

   case RESTOREBKPTS:
   {
    int mid;

    mid=fp->mid;
    ResetBreakpointFileTime();
    RestoreBreakpoints();
    fp=mid2fp(mid); /*fp may be replaced by RestoreBreakpoints, if the */
                    /*user designated source file disagrees with the */
                    /*file already loaded.*/
   }
   MarkLineBRKs( fp );
   fmtscr(fp);
   break;

   case EDITBKPTS:
       EditBreakpoints();
       break;

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
     fp_focus = fp;                                                     /*701*/
     return(-1);                                                        /*701*/
    }                                                                   /*701*/
                                                                        /*701*/
   case RESTART:                                                        /*701*/
    iview=0;
    Restart();                                                          /*701*/
    break;                                                              /*701*/
                                                                        /*701*/
   case QUIT:                                                           /*701*/
    NormalQuit(TRUE);                                                   /*701*/
    break;                                                              /*701*/
                                                                        /*701*/
   case SETFUNCTIONBKPT:
   case SETADDRESSBKPT:
   case SETDEFERREDBKPT:
   case SETADDRLOADBKPT:
   case SETDLLLOADBKPT:
    SetNameOrAddrBkpt(fp, func);
    if(!iview) fmttxt(fp);
    break;

   case UPCURSOR:                       /* move cursor up one line           */
    dovcsr( fp, -1 );                   /*                                   */
    break;                              /*                                   */
                                        /*                                   */
   case DOWNCURSOR:                     /* move the cursor down one line     */
    dovcsr( fp, 1 );                    /*                                   */
    break;                              /*                                   */
                                        /*                                   */
   case RIGHTCURSOR:                    /* move the cursor right one column  */
    dofwdtab( fp, lp, SrcCol );         /*                                   */
    break;                              /*                                   */
                                        /*                                   */
   case LEFTCURSOR:                     /* move the cursor left  one column  */
    dobwdtab( fp, lp, SrcCol );         /*                                   */
    break;                              /*                                   */
                                        /*                                   */
   case TABRIGHT:                       /* forward tab                       */
    dofwdtab( fp, lp, SrcCol );         /* same as RIGHT for now             */
    break;                              /*                                   */
                                        /*                                   */
   case TABLEFT:                        /* left tab                          */
    dobwdtab( fp, lp, SrcCol );         /* same as LEFT for now              */
    break;                              /*                                   */
                                        /*                                   */
   case PREVWINDOW:                     /* show previous screen              */
    dovscr( fp, -(int)LinesPer );       /*                                521*/
    break;                              /*                                   */
                                        /*                                   */
   case NEXTWINDOW:                     /* show next screen                  */
    dovscr( fp, LinesPer );             /*                                   */
    break;                              /*                                   */
                                        /*                                   */
   case FIRSTWINDOW:                    /*                                   */
    dovscr( fp, -(int)fp->Tlines );     /* display first screen       234 521*/
    fp->csrline = fp->topline;          /* update afile t put cursor on top  */
    break;                              /*                                   */
                                        /*                                   */
   case LASTWINDOW:                     /*                                   */
    dovscr( fp,fp->Tlines-LinesPer+1);  /* dispay last screen             234*/
    fp->csrline = fp->topline + fp->Nshown - 1;                         /*234*/
    break;

   case TOPOFWINDOW:                    /* move cursor to top line on screen */
    fp->csrline = fp->topline;          /* update afile t put cursor on top  */
    break;

   case BOTOFWINDOW:                    /* move cursor to last line on screen*/
    fp->csrline = fp->topline + fp->Nshown - 1;
    break;

   case SETCURSORPOS:                                                   /*701*/
    SetSrcViewCsr( fp );                                                /*701*/
    break;                                                              /*701*/

   /**************************************************************************/
   /* Source line step - step over calls.                                 701*/
   /**************************************************************************/
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

    if( fp == GetExecfp() )             /* is this the executing afile?      */
/*   return( GOSTMTC );  */             /* go back to run()                  */
     return( func );
    fmt2err( "Can't continue:", msg );
    beep();
    break;

   /**************************************************************************/
   /* Source line step - step into calls.                                 701*/
   /**************************************************************************/
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
/*   return( GOSTMT );    */
     return( func );
    fmt2err( "Can't continue:", msg );
    beep();
    break;

   /**************************************************************************/
   /* Run to the line the cursor is on.                                      */
   /*                                                                        */
   /*  - test for a break already on the line.                               */
   /*  - test for an executable line.                                        */
   /*  - set a one time breakpoint.                                          */
   /*  - run.                                                                */
   /**************************************************************************/
   case RUNTOCURSOR:
   case RUNTOCURSORNOSWAP:
    if( (*(lp-1) & LINE_BP) )
     if( GetExecfp() )
       return( func );

    if( !(*(lp-1) & LINE_OK) )
    {
     beep();
     fmterr( "Can't stop at this line" );
     break;
    }

    if( SetLineBRK(fp, SrcLine, lp, BRK_ONCE,NULL) ) /* single show bp   701*/
    {
     beep();
     fmterr( "Can't set breakpoint " );
     break;
    }

    if( GetExecfp() )
     return( func );
    break;

   /**************************************************************************/
   /* Let 'er rip.                                                        701*/
   /**************************************************************************/
   case RUNNOSWAP:
   case RUN:
    if( GetExecfp() )
     return( func );
    beep();
    fmt2err( "Can't continue:", msg );
    break;

   case GENHELP:                        /* General help                   701*/
    {
     uchar *HelpMsg;

     HelpMsg = GetHelpMsg( HELP_PULL_GENHELP, NULL,0 );
     CuaShowHelpBox( HelpMsg );
    }
    break;

   case MSHHELP:                        /* General help                   701*/
    {
#if 0 /*xxxxjc*/
     STARTDATA sd;
     ULONG     sid;
     ULONG     pid;
     char      buffer[256];
     char *format="%s\\help\\MSH.INF",PgmInputs[80];

     sprintf(PgmInputs,format,mshgetenv("MSHHOME"));

     memset( &sd, 0, sizeof(sd) );
     sd.Length        = sizeof(sd);
     sd.Related       = SSF_RELATED_INDEPENDENT;
     sd.FgBg          = SSF_FGBG_BACK;
     sd.TraceOpt      = SSF_TRACEOPT_NONE;
     sd.InheritOpt    = SSF_INHERTOPT_PARENT;
     sd.PgmName       = "VIEW.EXE";
     sd.PgmInputs     = PgmInputs;
     sd.SessionType   = SSF_TYPE_PM;
     sd.PgmTitle      = "Msh Help";
     sd.PgmControl    = SSF_CONTROL_VISIBLE;
     sd.ObjectBuffer  = buffer;
     sd.ObjectBuffLen = sizeof(buffer);

     DosStartSession( ( PSTARTDATA )&sd, &sid, &pid );

     fmterr("Math Shell Help");
#endif/*xxxxjc*/
    }
    break;

   case FUNCKEYSHELP:                                                   /*808*/
     HelpScreen( );
     break;                             /* for now, no context sensitive help*/

   case SHOWDLLS:                       /* show names of DLLs                */
     ShowDlls();                        /*                                   */
     break;                             /*                                   */

   case FINDEXECLINE:                   /* help                              */
    fp_focus = FindExecLine(fp, SrcLine); /* find current executing afile    */
    if( GetExecfp() && fp_focus )       /* if both afiles exist then         */
    {
     fp_focus->flags |= AF_ZOOM;        /*                            816 235*/
     return(-1);                        /* return to run()                   */
    }
    beep();
    break;

   case SHOWTHREADS:                    /* show process threads              */
    if( showthds((uint *)&key) )        /* if user selected new thread,   229*/
     return( -1 );                      /* return to run()                   */
    break;

   case SHOWPROCESSES:
    Cua_showproc();
    break;

   case TOGGLEHIDESTORAGE:              /*                                   */
   case TOGGLESTORAGE:                  /* Toggle to and from storage window */
   {
    Newfp = NULL;
    func = typoS((SIZEFUNC)ResizeWindow, fp, &Newfp);
    if( Newfp )
    {
     fp_focus = Newfp;
     return(-1);
    }
    if( func != 0 )                                                     /*701*/
     goto ReCirculate;                                                  /*701*/
    fmttxt( fp );                                                       /*701*/
    break;


#if 0
    rc = typoS((SIZEFUNC)ResizeWindow,fp); /* go to storage window handlin521*/
    if(rc != -1 )                       /* fall thru to toggleasm from       */
    {                                   /* storage window if -1 return.      */
     fmttxt( fp );                      /* come back from storage and update */
     break;
    }
#endif
   }
   case TOGGLEASM:                      /* toggle between source and asm     */
    SyncAddr = DBMapLno(fp->mid,        /* find synchronizing addr           */
                        SrcLine,        /* in this debug file.               */
                        fp->sfi,        /*                                   */
                        &span ,         /*                                   */
                        fp->pdf );      /*                                   */
    if(SyncAddr == NULL)                /* if no addr for this line then  106*/
     SyncAddr = DBMapNonExLine(fp->mid, /* try to map this line to an        */
                               SrcLine, /* executable line.                  */
                               fp->sfi,
                               fp->pdf);/*                                   */
    if( SyncAddr )                      /* is there any source available?    */
     {                                  /* yes.                              */
      fp->flags |= ASM_VIEW_TOGGLE;     /* tell afile that views are in sy235*/
      (fp_focus = fp) -> shower = showA;/* define asm shower function        */
      return(-1);                       /* return to run()                   */
     }
    beep();                             /* cannot display source             */
    break;

   case SHOWCALLSTACK:                  /* show call stack                   */
    switch( ActiveProcsMenu(&fp_focus) )/* show active procedures menu and   */
    {                                   /* return further instructions       */
     case A_ENTER:                      /* break on rtn to selected func     */
     case C_ENTER:                      /*                                   */
     if( GetExecfp() )                  /* is there an executing afile       */
       return( RUN );                   /* return to run() and run to func701*/
     fmt2err( "Can't continue:", msg ); /* no afile so can't execute         */
     beep();                            /* complain a little                 */
     break;                             /* break out sister                  */

     case ENTER:                        /* show the context of the call      */
      return(-1);
    }
    break;                              /* we just wanted to see the stack   */

   case FINDFUNCTION:                   /* find the module defining the      */
                                        /* procedure indicated by cursor.    */
    fp_focus = locatefp(lp, SrcCol );                                   /*521*/
    if( fp_focus )                                                      /*521*/
    {                                                                   /*235*/
     if( fp_focus->shower == showA )                                    /*235*/
     {                                                                  /*235*/
      fp_focus->flags = ASM_VIEW_CHANGE;                                /*235*/
      fp_focus->csr.row = fp_focus->csrline - 1;                        /*235*/
      if(fp_focus->sview == NOSRC )                                     /*235*/
       fp_focus->csr.row = 0;                                           /*235*/
     }                                                                  /*235*/
     return(-1);                                                        /*235*/
    }                                                                   /*235*/
    beep();
    break;

   case NEXTFILE:                       /* switch to th next file in ring    */
    if( !(fp_focus = fp->next) )        /* is there a next one in the ring?  */
     if( (fp_focus = allfps) == fp )    /* is there only one file?           */
     {
      beep();                           /* yes only one file                 */
      break;
     }
    if(fp_focus->shower == showA )                                      /*235*/
     fp_focus->flags = ASM_VIEW_NEXT;                                   /*235*/
    return(-1);                         /* return to run with recalled afile */

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
     if(!iview)fmttxt( fp );
     break;                                                             /*400*/


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
     if(!iview) fmttxt( fp );
     break;                                                             /*401*/

    /*************************************************************************/
    /* Set Execution Line Function.                                       508*/
    /*                                                                    508*/
    /*  - Check if the Line is executable, if not put error message.      508*/
    /*  - map the line number to address, if not able to map put error    508*/
    /*    message.                                                        508*/
    /*  - If able to map the line number to an address, call function to  508*/
    /*    change the CS:IP and then reformat the text.                    508*/
    /*                                                                    508*/
    /*************************************************************************/
    case SETEXECLINE:                                                   /*508*/
    {                                                                   /*508*/
       uint  addr;                                                      /*508*/
       uint  Tempspan;                                                  /*521*/
                                                                        /*508*/
       if( *(lp-1) & LINE_OK )                                          /*508*/
       {                                                                /*508*/
          addr = DBMapLno(fp->mid, SrcLine, fp->sfi, &Tempspan, fp->pdf);
          if( addr )                                                    /*508*/
          {                                                             /*508*/
            if( SetExecLine(addr) == TRUE )                             /*508*/
            {                                                           /*508*/
               fmttxt( fp );                                            /*508*/
               break;                                                   /*508*/
            }                                                           /*508*/
          }                                                             /*508*/
       }                                                                /*508*/
       beep();                                                          /*508*/
       fmterr( "Can't set ip at this line" );                           /*508*/
    }                                                                   /*508*/
    break;                                                              /*508*/

   case TIMEDSHOWAPPWINDOW:
    if( IsEspRemote() )
     xSelectSession();
    else
     DosSelectSession(DbgGetSessionID());
    DosSleep(5000L);
    DosSelectSession( 0 );
    break;

   case SETCLEARBKPT:                   /* set/reset breakpoint              */
    if( !(*(lp-1) & LINE_OK) )          /* breakable line?                   */
    {                                   /*                                   */
     beep();                            /*                                   */
     fmterr( "Can't stop at this line" );/* no. tell 'em so                  */
     break;
    }
    (void)SetLineBRK(fp,SrcLine,lp,BRK_SIMP,NULL);                      /*701*/
    fmttxt( fp );                       /* update the display to show bp.    */
    break;

   case SHRINKSTORAGE:                  /* shrink the storage display area   */
   case EXPANDSTORAGE:                  /* Expand the storage display area   */
    if( HideDataWin == TRUE  )          /* if data window is hidden,         */
    {                                   /* turn it on.                       */
     ResizeWindow(fp,-(int)SaveTopLine);/* resize the src/asm window.     521*/
     HideDataWin = FALSE;               /* TopLine gets reestablished in     */
     break;                             /*                                   */
    }                                   /* the recalibrate....puke!          */
   switch( func )
   {
    case SHRINKSTORAGE:                 /* shrink the storage display area   */
     if( ShrinkStorage(fp,1) )
      beep();                           /* beep if not shrinkable            */
     break;

    case EXPANDSTORAGE:                 /* Expand the storage display area   */
     if( ExpandStorage(fp,1) )
      beep();                           /* beep if not expandable            */
     break;
   }
   break;

   case SHOWVAR:                        /* show the contents of a variable   */
   case SHOWVARPTSTO:                   /* show contents var points to       */
   case PUTVARINSTG:                    /* put a variable in the storage win */
   case PUTVARPTSTOINSTG:               /* put what variable points to in stg*/
   case EXPANDVAR:                      /* expand variable                   */
   case MSHGET:                         /* mshget variable                   */
   case MSHPUT:                         /* mshget variable                   */
    {                                   /*                                   */
     DFILE  *dfp;                       /* dfile node pointer.               */
     if( HideDataWin == TRUE  )         /* if data window is hidden,         */
     {                                  /* turn it on.                       */
      ResizeWindow(fp,-(int)SaveTopLine);/* resize the src/asm window.    521*/
      HideDataWin = FALSE;              /* TopLine gets reestablished in     */
     }                                  /* the recalibrate....puke!          */
     Newfp = NULL;
     dfp = dumpvar(fp,                  /* afile pointer                     */
                   lp,                  /* pointer to source line            */
                   SrcLine,             /* source line number                */
                   SrcCol,              /* source column number              */
                   func,                /* which show function               */
                   &Newfp
                  );
     if( Newfp )
     {
      fp_focus = Newfp;
      return(-1);
     }
     if ( dfp )                         /* if true returned from dumpvar()   */
     {
      if( func==PUTVARINSTG ||          /* data window.                      */
          func==PUTVARPTSTOINSTG        /*                                   */
        )                               /*                                   */
      {                                 /*                                   */
       int adjust;                      /* amount to adjust the source window*/
       uint recspan;                    /* span of records.(0...N)           */
       uint winsize;                    /* size of the window(1..N).         */
/*****************************************************************************/
/* At this point, if the node is too big to fit in a full data window, then  */
/* show the top MaxData lines of the node.                                   */
/*                                                                           */
/*****************************************************************************/
       if( dfp->lines > MaxData )       /* if this is a big node, then       */
       {                                /*                                   */
        DataFileTop = dfp->lrn;         /* display first MaxData lines of it.*/
        DataFileBot = DataFileTop +     /*                                   */
                      MaxData - 1;      /*                                   */
        ResizeWindow( fp, -(int)MaxData);/* resize the window.            521*/
        break;                          /* done.                             */
       }                                /*                                   */
/*****************************************************************************/
/* At this point, we know that the node will fit in the window. We want to   */
/* put it in the current window without expanding if possible.               */
/*                                                                           */
/*****************************************************************************/
       winsize = TopLine-VioStartOffSet;/* current size of the window.    701*/
       DataFileBot = dfp->lrn +         /* next bottom of  recs in window.   */
                     dfp->lines -1;     /*                                   */
       recspan=DataFileBot-DataFileTop; /* compute span of recs in window.   */
                                        /* Note: respan = 0...N              */
       if(recspan < winsize)            /* if the lines will fit w/o         */
       {                                /* expanding the window, then do it. */
        ShowData(winsize);              /* refresh the window.               */
        break;                          /* done.                             */
       }                                /*                                   */
/*****************************************************************************/
/* At this point, we know that the node will NOT fit in the current window.  */
/* We will try to expand the window so that the node will fit.               */
/*                                                                           */
/*****************************************************************************/
       if(recspan < MaxData )           /* if the window can be expanded to  */
       {                                /* hold the data, then do it.        */
        adjust = recspan - TopLine + 1;
        ResizeWindow( fp , -adjust);    /* resize the window.                */
        break;                          /* done.                             */
       }                                /*                                   */
/*****************************************************************************/
/* At this point, we know that the node will NOT fit in the current window   */
/* and the window cannot be expanded to fit. We have to scroll.              */
/*                                                                           */
/*****************************************************************************/
       DataFileTop=DataFileBot-MaxData+1;/* adjust top rec to be top rec in  */
       adjust=MaxData-TopLine;          /* win. compute win expansion        */
       ResizeWindow( fp , -adjust);     /* adjustment and then               */
                                        /* resize the window.                */
      }                                 /* end of PUTVARINSTG || PUTVARPTS...*/
     }                                  /* end if(dfp) block.                */
     break;                             /* done with these cases.            */
    }                                   /* end of {} for these cases.        */
                                        /*                                   */
   case CLEARALLBKPTS:                  /* Clear all program breaks.         */
    FreeAllBrks();                      /*                                   */
    dovscr(fp,0);                       /* refresh the screen.               */
    break;                              /*                                   */
/*****************************************************************************/
/* Hide the data window, but save the state for later return.                */
/*                                                                           */
/*****************************************************************************/
   case SHOWHIDESTORAGE:                /*                                   */
    if( HideDataWin == TRUE  )          /* if data window is hidden,         */
    {                                   /* turn it on.                       */
     ResizeWindow(fp,-(int)SaveTopLine);/* resize the src/asm window.     521*/
     HideDataWin = FALSE;               /* TopLine gets reestablished in     */
    }                                   /* the recalibrate....puke!          */
    else                                /* else if the window is not         */
    {                                   /* hidden, then turn it off.         */
     SaveTopLine = TopLine - VioStartOffSet;/* hold top line for restore. 701*/
     ResizeWindow(fp,TopLine-VioStartOffSet);/* resize and refresh the win701*/
                                        /* will set TopLine = 0              */
     HideDataWin = TRUE;                /* set hidden mode flag.             */
    }                                   /*                                   */
    break;                              /*                                   */

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
    fmtscr( fp );                       /* reset the colors for this AFILE   */
    break;                              /*                                   */
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

   case  key_0:                      /* 0  0x0b30                         */
   case  key_1:                      /* 1  0x0231                         */
   case  key_2:                      /* 2  0x0332                         */
   case  key_3:                      /* 3  0x0433                         */
   case  key_4:                      /* 4  0x0534                         */
   case  key_5:                      /* 5  0x0635                         */
   case  key_6:                      /* 6  0x0736                         */
   case  key_7:                      /* 7  0x0837                         */
   case  key_8:                      /* 8  0x0938                         */
   case  key_9:                      /* 9  0x0a39                         */
   {
    typedef struct
    {
     char  key;
     UINT  code;
    }KEYTOCHAR;
    static KEYTOCHAR KeyToChar[]  = {'0',0x0b30,
                                     '1',0x0231,
                                     '2',0x0332,
                                     '3',0x0433,
                                     '4',0x0534,
                                     '5',0x0635,
                                     '6',0x0736,
                                     '7',0x0837,
                                     '8',0x0938,
                                     '9',0x0a39
                                    };
    for(n=0;n < 10 && (func != KeyToChar[n].code);n++){;}
    if( n < 10 )
    {
     fp->topline = GetLineNumber(KeyToChar[n].key);
     fp->csrline = fp->topline;
     dovscr(fp,0);
     break;
    }
   }
   beep();
   break;

   /**************************************************************************/
   /* Turn on/off the display of member functions.                           */
   /**************************************************************************/
   case TOGGLEMEMFNCS:
     SetViewMemFncs();
     break;

#ifdef MSH
   case MSHCOMMAND:                                                     /*701*/
    { static int MSHCommandCount=0;
      char msg[80];
      MSHOBJECT  *mshObject=*(((MSHOBJECT **)&mshinfo->mshfunction)+1);
      /*mshObject should be in the shared memory space.*/
      if(!strcmp(commandLine.parms[0],"MSHget")){
          MshParseExpr(
              commandLine.parms[1],
              commandLine.parms[2],
              0, 0, 0, mshObject);
      }
      else if(!strcmp(commandLine.parms[0],"MSHput")){
          extern int DataRecalc;                                         /*903*/
          extern uchar       Re_Parse_Data;
          MshParseExpr(
              commandLine.parms[1],
              commandLine.parms[2],
              0, 0, 0, mshObject);

          DataRecalc = TRUE;                                             /*903*/
          Re_Parse_Data = TRUE;
          ShowData(TopLine);
      }
      /*mshObject should be in the shared memory space.*/
      else {
          sprintf(msg,"O.K. MSH command (%s) semaphore processing %4.4d. ",
              commandLine.parms[0],
              ++MSHCommandCount);
      }
      fmterr( msg );
    }
    break;                                                              /*701*/
#endif

   default:
    beep();
    break;
  }                                     /* end of switch on functions        */
 }                                      /* end of keyboard processing loop   */
}                                       /* end of function showC             */

/***************************************************************************/
/*dovcsr                                                                   */
/* move the cursor down n lines                                            */
/*                                                                         */
/***************************************************************************/
 void
dovcsr(AFILE *fp,int n )                /* pointer to the afile block        */
                                        /* number of lines to move the cursor*/
{
 fp->csrline += n;                      /* update afile with current csr line*/
 if( !IsOnCRT(fp) )                     /* if the cursor goes off the screen */
 {
  fp->csrline -= n;                     /* reset afile value before update   */
  dovscr( fp, n );                      /* move the screen down n lines      */
 }
}

/***************************************************************************/
/*dohcsr                                                                   */
/* move the cursor over n columns                                          */
/*                                                                         */
/***************************************************************************/
 void
dohcsr(AFILE *fp, int n)
{
    if( (fp->csr.col += (uchar)n) >= ( uchar )VideoCols ){
        if( n > 0 ){
            n = (int)(fp->csr.col - (uchar)VideoCols + 1);
            fp->csr.col = ( uchar )(VideoCols - 1);
        }else{
        /* Convert csr.col from an 8-bit unsigned int to a 16-bit signed int */
            n = (int)(signed char)( fp->csr.col );                      /*603*/
            /*****************************************************************/
            /* changed cast from char to signed char                      603*/
            /*****************************************************************/
            fp->csr.col = 0;
        }
        dohscr( fp, n ); }
}

/***************************************************************************/
/*dohscr                                                                   */
/* move the screen horizontally n columns.                                 */
/*                                                                         */
/***************************************************************************/
 void
dohscr(AFILE *fp, int n)
{
    if( (int)(fp->skipcols += n) < 0 )
        fp->skipcols = 0;
    else if( fp->skipcols + VideoCols > 255 )
        fp->skipcols = 255 - VideoCols;

    fmttxt( fp );
}

/***************************************************************************/
/*dovscr                                                                   */
/* move the screen vertically n lines. n=large number moves a full screen. */
/*                                                                         */
/***************************************************************************/
 void
dovscr(AFILE *fp, int n)                /* pointer to afile                  */
                                        /* number of lines to move screen    */
{
 int newtop;                            /* new top of the screen             */
 int csrdelta;                          /* cursor offset from topline        */
 int target;                            /* potential top line in file        */
 int bias;                              /* file lines before start of src buf*/

 if(fp->Tlines==0) return;
 newtop = fp->topline + n;              /* define the new top line           */
 csrdelta = fp->csrline - fp->topline;  /* define lines between csr and top  */

 Recheck:

 bias = fp->Nbias;
/*

  At this point we first check to see if we're going beyond the lower
  line boundary of the source buffer.  If we are then there could still
  be source left in the file above the lines contained in the source
  buffer.  This is the case of a HUGE file meaning that the 64k source
  buffer could not hold all of the source lines and we need to back up
  in the file.

  If we have a HUGE file and all lines have not been read in, then we need
  to page the source buffer down in the file replenishing the source buffer.
  We then need to update the afile block and resynchronize the display.

*/

 if( newtop < 1 )                       /* trying to go before first line?234*/
 {
  if( (fp->flags & AF_HUGE) &&          /* is this a HUGE file and the       */
       fp->Nbias                        /* buffer was previously paged down ?*/
    )
  {
   target = newtop + fp->Nbias;         /* yes on both. define new target    */
   if( target < 1 )                     /* if target is before file begin 234*/
    target = 1;                         /* then clip it at 1.             234*/

   pagefp( fp, target );                /* removed a +1.                  234*/

   Resync:

   newtop = target - fp->Nbias;         /* define the new top of the file    */
   fp->hotline += bias - fp->Nbias;     /* define the new hotline            */
   goto Recheck;                        /* do it again just for being a jerk */
  }

  newtop = 1;                           /* can't move before first line   234*/

 }
 else
/*

  At this point we first check to see if we have enough lines left in the
  buffer to fill up the display.  If we don't, we could still have source
  left in the file that has not been read into the source buffer.  This is
  the case of a HUGE file meaning that the 64k source buffer could not hold
  all of the source lines and we still need to read some in.

  If we have a HUGE file and all lines have not been read in, then we need
  to page the source buffer down in the file replenishing the source buffer.
  We then need to update the afile block and resynchronize the display.


*/                                      /*need more lines than in buffer?*/
 {
  if( (uint)newtop + LinesPer - 1 > fp->Nlines )
  {
   if( (fp->flags & AF_HUGE) &&         /* yes. Is this a HUGE file and not  */
       (fp->Nlines + fp->Nbias < fp->Tlines) /* all lines have been read?    */
     )
   {
    target = newtop + fp->Nbias;        /* true again. define the target line*/
    if( target > (int)fp->Tlines )      /* is the target past end of file?   */
     target = fp->Tlines - LinesPer + 1;/* yes. display last LinesPer in file*/
    pagefp( fp, target + LinesPer - 1); /*                                234*/
    goto Resync;                        /* resync the display                */
   }
   newtop = fp->Nlines - LinesPer + 1;  /* display last LinesPer in file     */
   if( newtop < 1 )                     /* don't let the newtop go abv fil234*/
    newtop = 1;                         /* 1 is the minimum               234*/
  }
 }
 fp->topline = newtop;                  /* put new topline in afile          */
 fp->csrline = fp->topline + csrdelta;  /* put new csrline in afile          */

 fmttxt( fp );                          /* format the text for this afile    */
}

/***************************************************************************/
/*dofwdtab                                                                 */
/* move to the next c identifier in the line                               */
/*                                                                         */
/***************************************************************************/
 uint
dofwdtab(AFILE *fp,uchar *lp,uint col)  /* -> to afile                       */
                                        /* pointer to line cursor is on      */
                                        /* current cursor column             */
{
 uchar *cp;                             /* -> to char in source line         */
 uint   len;                            /* length of uncompressed source line*/
 int    delta=0;                        /* # of columns to next identifier   */
 uchar  line[MAXCOLS+1];                /* line buffer                       */

 len = Decode(lp, line);                /* define the uncompressed line len  */
 if( len > col )                        /* if the line is longer than column */
 {                                      /* then                              */
  cp = line + col;                      /* ->to character on the line        */
  line[len] = 0 ;                       /* init it to null                   */
  for( ;; )
  {
   while( IsOKchar(*cp++) ){;}          /* advance to end of current c ident */
   --cp;                                /* need to back up one position      */
   while( *cp && !IsOKchar(*cp) )       /* move to next identifier begin     */
    ++cp;
   if( *cp < '0' || *cp > '9' )         /* if we stopped on a constant then  */
    break;                              /* continue                          */
  }
  delta = (cp - line) - col ;           /* this is how much we gonna move    */
  dohcsr( fp, delta );                  /* now move it                       */
 }
 return( delta );                       /* inform caller how much we moved it*/
}
/***************************************************************************/
/*dobwdtab                                                                 */
/* move to the previous word in the line.                                  */
/*                                                                         */
/***************************************************************************/
 uint
dobwdtab(AFILE *fp,uchar *lp,uint col ) /* -> to afile                       */
                                        /* pointer to line cursor is on      */
                                        /* current cursor column             */
{
 uchar *cp;                             /* -> to char in source line         */
 uint  len;                             /* length of uncompressed source line*/
 int   delta=0;                         /* # of columns to next identifier   */
 uchar line[MAXCOLS+1];                 /* line buffer                       */

 len = Decode(lp, line);                /* define the uncompressed line len  */
 if( col && len )                       /* if the line is longer than column */
 {                                      /* then                              */
  cp = line + ( (col < len) ? col : len ); /* -> to column or end of line    */
  for( ;; )                             /* scan backward to previous identifr*/
  {
   for( ;; )                            /* scan left for OK char             */
   {
    if( cp == line )                    /* at the line begin?                */
     goto endfor;                       /* yes. we're done. get out          */
    if( IsOKchar(*--cp) )               /* is this a valid c identifier?     */
     break;                             /* yes. quit scan left for char      */
   }                                    /* end scan left for char            */

   while( IsOKchar(*cp) )               /* scan left for not-OK char         */
    if( --cp < line )                   /* at the line begin?                */
     break;                             /* yes. we're done. get out.         */
   if( *++cp < '0' || *cp > '9' )       /* valid identifier character ?      */
     break;                             /* yes. we're done, else a constant  */
  }                                     /* end scan backward for valid ident */
  endfor:
  delta = (cp - line) - col ;           /* how much to move the cursor       */
  dohcsr( fp, delta );                  /* move the cursor                   */
 }
 return( delta );                       /* return the amount moved           */
}                                       /* end dobwdtab()                    */

/***************************************************************************/
/*IsOnCRT                                                                  */
/* is the cursor on the screen.                                            */
/*                                                                         */
/***************************************************************************/
 uint
IsOnCRT(AFILE *fp )
{
 uint TorF;                             /* is it or isn't it                 */

/* we simply check to see if the current cursor line for the fp is within
   the line boundaries of the displayed source.
*/

 TorF =( fp->csrline >= fp->topline &&
         fp->csrline < (fp->topline + fp->Nshown)
       );

 return( TorF );                        /* return the truth                  */
}

/***************************************************************************/
/*IsOKchar                                                                 */
/* check for valid c identifier.                                           */
/*                                                                         */
/***************************************************************************/
 uint
IsOKchar(uchar c)                       /* valid char for a C identifier     */
{
 uint ctr = c;                          /* was register.                  112*/

 return
 (
  (ctr >= '0' && ctr <= '9') ||         /* a digit ?                         */
  (ctr >= 'a' && ctr <= 'z') ||         /* a lowercase alpha ?               */
  (ctr >= 'A' && ctr <= 'Z') ||         /* an uppercase alpha ?              */
  (ctr == '_')||                        /* an underscore                     */
  (ctr == '@')||                        /* a @ for PL/86                     */
  (ctr == '$')||                        /* a $ for PL/86                     */
  (ctr == '#')||                        /* a # for PL/86                     */
  (ctr == 0x15)                         /* the italian character            */
 );
}

/*****************************************************************************/
/*  ResizeWindow()                                                           */
/*                                                                           */
/* Description:                                                              */
/*   Adjust the source and storage window sizes.                             */
/*                                                                           */
/* Parameters:                                                               */
/*   fp         input - afile for the source view.                           */
/*   adjust     input - amount to shrink the source window.                  */
/*                                                                           */
/* Return:                                                                   */
/*   1 or 0     0=ok. 1=can't do it.                                         */
/*                                                                           */
/* Assumptions                                                               */
/*              fp is valid.                                                 */
/*                                                                           */
/*****************************************************************************/
extern uint         MaxPer;             /* max # of lines for source code    */
extern uint        DataFileTop;         /* 1st rec shown of data file (0..N) */
 int                                    /* removed static                 701*/
ResizeWindow(AFILE *fp, int adjust )    /* afile for this view.              */
                                        /* source window adjustment amt.     */
{                                       /*                                   */
 int n = adjust;                        /* amount to adjust window size.     */

/*****************************************************************************/
/* The first thing to do is adjust the size of the window.                   */
/*                                                                           */
/*****************************************************************************/
 if( TopLine == VioStartOffSet &&       /* if the TopLine is maxed out and701*/
     adjust > 0                         /* caller wants it bigger, then      */
   )                                    /*                                   */
  return(0);                            /* don't do it.                      */
                                        /*                                   */
 TopLine -= adjust;                     /*                                   */
 if( TopLine > MaxData )                /* Limit to max data window size.    */
  TopLine = MaxData;                    /*                                   */

/*****************************************************************************/
/* Now we want to fit the data in the window and scroll the data if need be. */
/*                                                                           */
/*****************************************************************************/
 LinesPer = MaxPer - TopLine;           /* adjust top of source from bot.    */
 ShowData( TopLine );                   /* display the storage window.       */
 n = fp->csrline;                       /* hold the cursor line.             */
 dovscr( fp, -adjust );                 /* scroll the screen.                */
 fp->csrline = n;                       /* put the cursor line back.         */
 if( !IsOnCRT(fp) )                     /* if the cursor goes off screen,    */
  fp->csrline = fp->topline;            /* then put it on the top of source. */
 return(0);                             /*                                   */
}                                       /*                                   */
/*****************************************************************************/
/* ShrinkStorage()                                                           */
/*                                                                           */
/* Description:                                                              */
/*   Reduce the size of the storage window one line.                         */
/*                                                                           */
/* Parameters:                                                               */
/*   fp         input - afile structure for this source.                     */
/*   flag       input - source or asm flag.                                  */
/*                      1 = source.                                          */
/*                      0 = asm.                                             */
/*                                                                           */
/* Return:                                                                   */
/*              1 = can't do it.                                             */
/*              0 = dunnit.                                                  */
/*                                                                           */
/* Assumptions                                                               */
/*              will shrink it if I can.                                     */
/*                                                                           */
/*****************************************************************************/
 int                                    /*                                   */
ShrinkStorage(AFILE *fp , uint flag)    /* the afile structure for this src. */
                                        /* called from source or asm flag.   */
                                        /* 1 =>source. 0 =>asm.              */
{                                       /*                                   */
 uint span;                             /* span of data records in window.   */
                                        /*                                   */
 if( TopLine == VioStartOffSet)         /* if no data window we're done.  701*/
  return(1);                            /*                                   */
 span = DataFileBot - DataFileTop +1;   /* compute # recs in window.         */
 if(span == TopLine)                    /* if window is full, then           */
  if(DataFileBot)                       /* decrement the bottom display rec  */
    DataFileBot--;                      /*                                   */
 if(flag)                               /*                                   */
  ResizeWindow(fp,  1);                 /* resize the source window.         */
 else                                   /*                                   */
  Recalibrate(fp,  1);                  /* resize the asm window.            */
 return(0);                             /* return a success code.            */
}                                       /* end ShrinkStorage().              */
/*****************************************************************************/
/* ExpandStorage()                                                           */
/*                                                                           */
/* Description:                                                              */
/*   Increase the size of the storage window one line.                       */
/*                                                                           */
/* Parameters:                                                               */
/*   fp         input - afile structure for this source.                     */
/*   flag       input - source or asm flag.                                  */
/*                      1 = source.                                          */
/*                      0 = asm.                                             */
/*                                                                           */
/* Return:                                                                   */
/*              1 = can't do it.                                             */
/*              0 = dunnit.                                                  */
/*                                                                           */
/* Assumptions                                                               */
/*              will grow it if I can.                                       */
/*                                                                           */
/*****************************************************************************/
 int                                    /*                                   */
ExpandStorage(AFILE *fp , uint flag)    /* the afile structure for this src. */
                                        /* called from source or asm flag.   */
                                        /* 1 =>source. 0 =>asm.              */
{                                       /*                                   */
 if(TopLine == MaxData)                 /* if maxed out we can't do it       */
  return(1);                            /*                                   */
 DataFileBot++;                         /* expand window down.               */
 if(flag)                               /*                                   */
  ResizeWindow(fp, -1);                 /* resize the source window.         */
 else                                   /*                                   */
  Recalibrate(fp, -1);                  /* resize the asm window.            */
 return(0);                             /* return a success code.            */
}                                       /* end ExpandStorage().              */

void  SetCursorPos()                                                    /*701*/
{                                                                       /*701*/
  PEVENT  Event;                                                        /*701*/
                                                                        /*701*/
  Event = GetCurrentEvent();                                            /*701*/
  if( ((uint)Event->Row >= TopLine) &&                                  /*701*/
      ((uint)Event->Row <= (TopLine + showfp->Nshown - 1)) )            /*701*/
  {                                                                     /*701*/
    showfp->csrline = showfp->topline + (uint)Event->Row - TopLine;     /*701*/
    showfp->csr.col = (uchar)Event->Col;                                /*701*/
    showfp->csr.row = ( uchar )( showfp->csrline - showfp->topline + TopLine );
    lp = showfp->source + showfp->offtab[ showfp->csrline ];            /*701*/
    SrcCol  = showfp->skipcols + showfp->csr.col;                       /*701*/
    SrcLine = showfp->csrline  + showfp->Nbias;                         /*701*/
    PutCsr( ( CSR * )&showfp->csr );                                    /*701*/
  }                                                                     /*701*/
  else                                                                  /*701*/
    beep();                                                             /*701*/
}                                                                       /*701*/
/*****************************************************************************/
/* GetEventView()                                                         701*/
/*                                                                        701*/
/* Description:                                                           701*/
/*   Get the current view that a mouse click has occurred in.             701*/
/*                                                                        701*/
/* Parameters:                                                            701*/
/*  none                                                                  701*/
/*                                                                        701*/
/*                                                                        701*/
/* Return:                                                                701*/
/*  view        SOURCEVIEW ( or assembler ) it's the same.                701*/
/*              DATAVIEW                                                  701*/
/*              ACTIONBAR                                                 701*/
/*                                                                        701*/
/* Assumptions                                                            701*/
/*  none                                                                  701*/
/*                                                                        701*/
/*****************************************************************************/
extern uint           FnameRow;                                         /*701*/
                                                                        /*701*/
int GetEventView( )                                                     /*701*/
{                                                                       /*701*/
 PEVENT CurrentEvent;                                                   /*701*/
 int    view = 0;                                                       /*701*/
                                                                        /*701*/
 CurrentEvent = GetCurrentEvent();                                      /*701*/

 if(!iview) {                                                           /*701*/
 if( CurrentEvent->Row == 0 )                                           /*701*/
  view = ACTIONBAR;                                                     /*701*/
                                                                        /*701*/
 if( CurrentEvent->Row >= TopLine && CurrentEvent->Row <  FnameRow )    /*701*/
  view = SOURCEVIEW;                                                    /*701*/
                                                                        /*701*/
 else if( CurrentEvent->Row != 0 &&  CurrentEvent->Row < TopLine )      /*701*/
  view = DATAVIEW;                                                      /*701*/
 }                                                                      /*701*/
 else
 {
#ifdef MSH
  extern WINDOW *root, *Window[]; WINDOW *win;
  win=GetEventWindow(root);
  if( CurrentEvent->Row == 0 )                                           /*701*/
  view = ACTIONBAR;                                                      /*701*/
  else if(win==Window[DATA_WIN]) view=DATAVIEW;
  else view=SOURCEVIEW;
#endif
 }/* End if*/
 return(view);                                                          /*701*/
}                                                                       /*701*/
/*****************************************************************************/
/* SetSrcViewCsr()                                                        701*/
/*                                                                        701*/
/* Description:                                                           701*/
/*   Set the cursor position in the source window when a mouse event      701*/
/*   occurs.                                                              701*/
/*                                                                        701*/
/* Parameters:                                                            701*/
/*                                                                        701*/
/* Return:                                                                701*/
/*                                                                        701*/
/* Assumptions                                                            701*/
/*  The caller has already verified that the cursor is in the             701*/
/*  Source window.                                                        701*/
/*                                                                        701*/
/*****************************************************************************/
void  SetSrcViewCsr( AFILE *fp )                                        /*701*/
{                                                                       /*701*/
  PEVENT  Event;                                                        /*701*/
                                                                        /*701*/
  Event = GetCurrentEvent();                                            /*701*/
  if( ((uint)Event->Row >= TopLine) &&                                  /*701*/
      ((uint)Event->Row <= (TopLine + fp->Nshown - 1)) )                /*701*/
  {                                                                     /*701*/
   fp->csrline = fp->topline + Event->Row - TopLine;                    /*701*/
   fp->csr.col = Event->Col;                                            /*701*/
  }                                                                     /*701*/
}                                                                       /*701*/

void WaitForInterrupt( void *dummy )
{
 EVENT           *pEvent;


 for(;;)
 {
  pEvent = GetEvent( 500 );
  if( pEvent->FakeEvent == TRUE )
  {
   if( QueryConnectSema4() == SEMA4_POSTED )
    break;

   continue;
  }

  if( QueryConnectSema4() == SEMA4_RESET )
   ConnectThisPid( INFORM_ESP );
  break;
 }
 _endthread();
}

void ConnectThisPid( int InformEsp )
{
 ALLPIDS         *pYieldPid;
 DBG_QUE_ELEMENT  Qelement;
 USHORT           ThisPid;
 USHORT           YieldPid;

 ThisPid  = (USHORT)DbgGetProcessID();

 YieldPid  = 0;
 pYieldPid = GetPidConnected();
 if( pYieldPid )
  YieldPid  = pYieldPid->pid;

 if( ThisPid != YieldPid )
 {
  if( pYieldPid )
  {
   pYieldPid->PidFlags.ConnectYielded = TRUE;
   SetConnectSema4( &pYieldPid->ConnectSema4, TRUE );
  }

  Qelement.pid = ThisPid;
  Qelement.sid = (ULONG)YieldPid;

  Qelement.DbgMsgFlags.InformEsp = TRUE;
  if( InformEsp == NO_INFORM_ESP )
   Qelement.DbgMsgFlags.InformEsp = FALSE;

  SendMsgToDbgQue( DBG_QMSG_CONNECT_ESP, &Qelement, sizeof(Qelement) );
 }
}
