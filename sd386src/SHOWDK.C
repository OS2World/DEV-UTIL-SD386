/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   showdk.c                                                                */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*  data formatting routines.                                                */
/*                                                                           */
/*                                                                           */
/* History:                                                                  */
/*                                                                           */
/*****************************************************************************/

#include "all.h"
static int iview=0;
#define FmtErr fmterr
/**Defines *******************************************************************/

#define N1KEYS 17

/**External declararions******************************************************/


extern uint        DstatRow;            /* data window status row.           */
extern uint        ExprMid;             /* Set by ParseExpr                  */
extern uint        ExprLno;             /* Set by ParseExpr                  */
extern uint        ExprTid;             /* Set by ParseExpr                  */
extern SCOPE       ExprScope;           /* Set by ParseExpr                  */
extern uchar*      ParseError;          /* Set by ParseExpr                  */
extern uchar       BadExprMsg[];        /*                                   */
extern KEY2FUNC    defk2f[];            /*                                   */
extern uint        ParseMshExpr;
extern uint        DataFileTop;         /* top record displayed in data file.*/
extern uint        DataFileBot;         /* bottom rec displayed in data file.*/
extern uint        TopLine;             /* current top line for Code (0..N)  */
extern uint        LinesPer;            /* current # lines/screen for code  */
extern uint        SaveTopLine;         /* saved top line.                   */
extern int         HideDataWin;         /* data window hidden flag.          */
extern uint        ExprAddr;            /* Set by ParseExpr.              112*/
extern uchar       Re_Parse_Data;       /* flag to indicate wether all    244*/
                                        /* variables in datawindow are to 244*/
                                        /* be reparsed.                   244*/
extern uint        VioStartOffSet;      /* flag to tell were to start screen */
                                        /* display.                       701*/
extern uint        VideoCols;
extern uint        VideoRows;
extern uchar      *BoundPtr;            /* -> to screen bounds            701*/
extern CmdParms   cmd;
extern UINT       MaxData;              /* max # of lines of Data            */

int    BlowBy = NO;                     /* blow by GetFuncsFromEvents flag701*/
int    HideDataWin;                     /* data window hidden flag.          */
DFILE *DataFileQ;                       /* data file pointer.                */
DFILE *dfpfirst={(DFILE*)(void*)&DataFileQ};
DFILE *dfplast ={(DFILE*)(void*)&DataFileQ};

/**Static definitions ********************************************************/

static  CSR stgcsr[3];                 /* data window cursor definition.    */


/*****************************************************************************/
/*  InitDataWinCsr()                                                      701*/
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Inits the data window cursor value to one.                              */
/*                                                                           */
/* Return:                                                                   */
/*   None                                                                    */
/*****************************************************************************/
void InitDataWinCsr()                                                   /*701*/
{                                                                       /*701*/
  stgcsr[0].row = 1;                                                    /*701*/
  stgcsr[1].row = 0;                                                    /*701*/
  stgcsr[2].row = 0;                                                    /*701*/
}                                                                       /*701*/

int typoS(SIZEFUNC resizefunc, AFILE *fp, AFILE **fpp)
{
 int    row;                            /* display row in the data window.   */
 int    mem;                            /* the member of a dfile node.       */
 uint   key;                            /* the user's input key.             */
 uint   span;                           /* span of records in data window.   */
 int    nexttop;                        /* next top line of data display.    */
 uint   absrow;                         /* the absolute data file record #.  */
 int    func;                           /* function associated with a key.   */
 DFILE *dfp;                            /* -> to a dfile node.               */
 uint   filelen;                        /* length of the data file.          */
 char   msgbuf[80];                     /* buffer for message.               */
 uint   rc;                             /* retrun code.                      */
 uint   view_flag;                      /* flag to indicate source or asm 220*/
                                        /* view.                             */
 uchar  Data_Refresh_flag;              /* flag to indicate refesh of     244*/
                                        /* data window.                   244*/
 AFILE  *newfp;
/*****************************************************************************/
/* The first thing we want to do is open the data window if one is not open. */
/* We will open a full size, blank window and resize the src/asm window      */
/* as necessary.                                                             */
/*                                                                           */
/*****************************************************************************/
 if(HideDataWin)                        /* we may need to unhide the         */
 {                                      /* data window.                      */
  (* resizefunc)(fp,-(int)SaveTopLine); /* resize the src/asm window.     521*/
  HideDataWin = FALSE;                  /* TopLine gets reestablished in     */
 }                                      /* the resizefunc()...puke!          */
 DstatRow = TopLine;                    /* establish status line (0..N).     */
 if( !(TopLine-VioStartOffSet))         /* if there is not a window open, 701*/
 {                                      /*                                   */
  DstatRow = MaxData;                   /* establish status line (0..N).     */
  (* resizefunc)(fp,-(int)MaxData);     /* resize the src/asm window.     521*/
 }                                      /*                                   */
                                        /*                                   */
/*****************************************************************************/
/* Establish the cursor row in the data window. Do not wrap.                 */
/*                                                                           */
/*****************************************************************************/
 row = stgcsr[iview].row;                      /* retrieve saved cursor position.   */
 if( row >= (int)TopLine )              /* if below the window, then         */
   row = TopLine - 1 + VioStartOffSet;  /* peg it at the bottom.          701*/
                                        /*                                   */
/*****************************************************************************/
/* Now refresh the data in the window and add the status line.               */
/*                                                                           */
/*****************************************************************************/
Refresh:
 ShowData( iview?LinesPer:TopLine );
 FmtErr(NULL);

 /************************************************************************701*/
 /* - The BlowBy flag is used to skip the GetFuncsFromEvents call in      701*/
 /*   showc() and showa so that we can execute functions from the         701*/
 /*   data window and come back into the data window without stopping     701*/
 /*   in showc() or showa().                                              701*/
 /* - To avoid an annoying blink when running functions from the data     701*/
 /*   window we set the BoundPtr[TopLine] = 0 so that this line could     701*/
 /*   not be written to. What we have to do here is reset the BoundPtr    701*/
 /*   back to what it should be.                                          701*/
 /*                                                                       701*/
 /************************************************************************701*/
 if(iview==VIEW_NORMAL)
 {                                                                      /*701*/
  extern uchar  Reg_Display;                                            /*701*/
  BoundPtr[TopLine] = VideoCols;                                        /*701*/
  if(TestBit(Reg_Display,REGS386BIT))                                   /*701*/
   BoundPtr[TopLine] = VideoCols-REGSWINWIDTH;                          /*701*/
  if(TestBit(Reg_Display,REGS387BIT))                                   /*701*/
   BoundPtr[TopLine] = VideoCols-COREGSWINWIDTH;                        /*701*/
 }                                                                      /*701*/
  BlowBy = NO;                                                          /*701*/

/*****************************************************************************/
/* We will now handle the following functions.                               */
/*                                                                           */
/* case UPCURSOR:                       -- move cursor up in data window/drag*/
/* case DOWNCURSOR:                     -- move cursor dn in data window/drag*/
/* case FIRSTWINDOW:                    -- move to top of data file.         */
/* case PREVWINDOW:                     -- move back one window.             */
/* case TOPOFWINDOW:                    -- move cursor to top of window.     */
/* case BOTOFWINDOW:                    -- move cursor to bottom of window.  */
/* case LASTWINDOW:                     -- move to last window in file.      */
/* case NEXTWINDOW:                     -- move to the next window in file.  */
/* case SHRINKSTORAGE:                  -- reduce the window size.           */
/* case EXPANDSTORAGE:                  -- increase the window size.         */
/* case EXPANDVAR:                      -- expand  the dfile node member.    */
/* case TABRIGHT:                       -- edit the dfile record.            */
/* case RIGHTCURSOR:                    -- edit the dfile record.            */
/* case HELP:                           -- help.                             */
/* case SHOWSEGS:                       -- show the EXE header segments.     */
/* case TOGGLESTORAGE:                  -- go back to the source window.     */
/* case ACTIONBAR:                      -- display the options menu.         */
/* case FORMATVAR:                      -- display data record in user format*/
/* case INSERTLINE:                     -- add a blank line in the data win. */
/* case DELETELINE:                     -- delete an object from the data win*/
/* case EDITFORMULA:                    -- edit a data window expression.    */
/* case BROWSE:                         -- browse any file.                  */
/* case TOGGLEHIDESTORAGE:              -- go back to the src window&hide    */
/* case SHOWHIDESTORAGE:                -- go back to the src window&hide    */
/* case SETCOLORS:                      -- set colors from colors menu       */
/*                                                                           */
/*****************************************************************************/
 Data_Refresh_flag = FALSE;             /* reset the data refresh flag.      */
 for( ;; )                              /* begin loop to process user input. */
 {                                      /*                                   */
  if (Data_Refresh_flag)                /* if data refreh flag is set     244*/
  {                                     /* then call show data.           244*/
    ShowData( iview?LinesPer:TopLine );
    Data_Refresh_flag = FALSE;          /* reset the data refresh flag.   244*/
  }
  absrow = DataFileTop +                /* establish absolute data file rec. */
      (stgcsr[iview].row = ( uchar )row);      /*                                   */
  {
    CSR csr;
    csr.row=RowStart+stgcsr[iview].row;
    csr.col=ColStart+stgcsr[iview].col;
    PutCsr( &csr );                     /* refresh the cursor.               */
  }
  if( TopLine )                         /* if there is a data window, then   */
   if(!iview) fmtdwinstat(DstatRow, row );         /* put in a status line.             */

  /***************************************************************************/
  /* Set the masked items for the source view.                            701*/
  /***************************************************************************/
  if( fp->shower == showA )                                             /*701*/
    SetMenuMask( DATAVIEWASM );                                         /*701*/
  else                                                                  /*701*/
    SetMenuMask( DATAVIEWSRC );                                         /*701*/
                                                                        /*701*/
  if( (fp == GetExecfp()) || (GetExecfp() == NULL) )                    /*701*/
   DisableDropFile();
  else
   EnableDropFile();

  /***************************************************************************/
  /* Get an event from the kbd/mouse.                                     701*/
  /***************************************************************************/
GETAFUNCTION:
#define  RETURNESC  1                                                   /*818*/

  if( (SerialParallel() == SERIAL) && (QueryConnectSema4() == SEMA4_RESET) )
  {
   FmtErr("Disconnected...double click on window to reconnect.");
   _beginthread(WaitForInterrupt, NULL, 0x8000, NULL );
block:
   SerialConnect( JUST_WAIT, DbgGetProcessID(), _DBG, SendMsgToDbgQue );
   FmtErr("Reconnected.");
   DosSelectSession(0);
  }

  func = GetFuncsFromEvents( RETURNESC, (void *)fp );              /*818  701*/

  if( (SerialParallel() == SERIAL) && (QueryConnectSema4() == SEMA4_RESET) )
  {
   ConnectThisPid( INFORM_ESP );
   goto block;
  }
HANDLEEVENTS:
  /***************************************************************************/
  /* GetFuncsFromEvents will not always be able to return an executable   701*/
  /* function code, so we get back a raw LEFTMOUSECLICK or RIGHTMOUSECLICK701*/
  /* So, what we do is:                                                   701*/
  /*                                                                      701*/
  /*  -LEFTMOUSECLICK                                                     701*/
  /*    - if the click is in the data window, then set the cursor         701*/
  /*      position or edit storage.                                       701*/
  /*    - if the click is in the source window, then switch to the source 701*/
  /*      window.                                                         701*/
  /*                                                                      701*/
  /*  -RIGHTMOUSECLICK                                                    701*/
  /*    - bring up the object pulldown for the data window.               701*/
  /*                                                                      701*/
  /***************************************************************************/
  if( func == LEFTMOUSECLICK )                                          /*701*/
  {                                                                     /*701*/
   switch( GetEventView() )                                             /*701*/
   {                                                                    /*701*/
    case SOURCEVIEW:                                                    /*701*/
     SetSrcViewCsr( fp );                                               /*701*/
     func = TOGGLESTORAGE;                                              /*701*/
     break;                                                             /*701*/
                                                                        /*701*/
    case DATAVIEW:                                                      /*701*/
    {                                                                   /*701*/
     PEVENT  Event;                                                     /*701*/
                                                                        /*701*/
     Event = GetCurrentEvent();                                         /*701*/
                                                                        /*701*/
     if(Event->Col >= STGCOL - 1)                                       /*701*/
     {                                                                  /*701*/
      SetDataViewCsr();                                                 /*701*/
      row = stgcsr[iview].row;                                                 /*701*/
      absrow = DataFileTop + ( uchar )row ;                             /*701*/
      func = TABRIGHT;                                                  /*701*/
     }                                                                  /*701*/
     else                                                               /*701*/
      func = SETCURSORPOS;                                              /*701*/
     break;                                                             /*701*/
    }                                                                   /*701*/
                                                                        /*701*/
    case ACTBARVIEW:                                                    /*701*/
     goto GETAFUNCTION;                                                 /*701*/
                                                                        /*701*/
                                                                        /*701*/
    default:                                                            /*701*/
     func = DONOTHING;                                                  /*701*/
     break;                                                             /*701*/
   }                                                                    /*701*/
  }                                                                     /*701*/
  else if( func == RIGHTMOUSECLICK )                                    /*701*/
  {                                                                     /*701*/
    int    MouseState;                                                  /*701*/
    MouseState = STATE_BUTTON_PRESSED;                                  /*701*/
    func = GetObjectPullChoice( 13, &MouseState );                      /*701*/
  }                                                                     /*701*/

#if 0
  if(key == ESC)                        /*                                   */
   func = TOGGLESTORAGE;                /*                                   */
#endif

  switch( func )                        /* branch on the user input.         */
  {                                     /*                                   */
   case UPCURSOR:                       /* begin UPCURSOR.                   */
                                        /*                                   */
    if( --row == (VioStartOffSet-1) )   /* if we're trying to drag the    701*/
    {                                   /* window up in the file, then       */
     row = VioStartOffSet;              /* put the cursor on the 1st line 701*/
     if(DataFileTop > 0)                /* If the window is not topped out   */
      DataFileTop--;                    /* then drag the window up one line. */
     span = DataFileBot - DataFileTop;  /* if the data file is too big to fit*/
     if( span == TopLine )              /* in the window, then adjust the    */
      DataFileBot--;                    /* size of the window so it will fit.*/
     Data_Refresh_flag = TRUE;          /* set the data refresh flag.     244*/
    }                                   /*                                   */
    break;                              /* end UPCURSOR.                     */
                                        /*                                   */
   case DOWNCURSOR:                     /* begin DOWNCURSOR.                 */
                                        /*                                   */
    if( ++row == (int)TopLine )         /* if we're trying to drag the       */
    {                                   /* window down, then leave the cursor*/
     row--;                             /* on the bottom line.               */
     DataFileTop++;                     /* move the window top down one rec- */
     DataFileBot++;                     /* move the bottom of the window     */
     Data_Refresh_flag = TRUE;          /* set the data refresh flag.     244*/
    }                                   /*                                   */
    break;                              /* end DOWNCURSOR.                   */
                                        /*                                   */
   case FIRSTWINDOW:                    /* begin FIRSTWINDOW, alias C-Home.  */
    if( DataFileTop > 0 )               /* if window is not already pegged   */
    {                                   /* against the top, then             */
     DataFileTop = 0;                   /* peg window to top of data file.   */
     DataFileBot=DataFileTop+TopLine-1; /* establish window bottom.          */
     Data_Refresh_flag = TRUE;          /* set the data refresh flag.     244*/
    }                                   /*                                   */
    row = VioStartOffSet;               /* put cursor on the top line.    701*/
    break;                              /* end FIRSTWINDOW.                  */
                                        /*                                   */
   case PREVWINDOW:                     /* begin PREVWINDOW, alias PgUp.     */
    if( DataFileTop > 0 )               /* if the window is not pegged to the*/
    {                                   /* top of the file, then             */
     nexttop = DataFileTop-TopLine + 1; /* compute the next top of file and  */
     DataFileTop = nexttop;             /* clip it at the top of the file.   */
     if( nexttop < 0 )                  /*                                   */
      DataFileTop = 0;                  /*                                   */
     DataFileBot=DataFileTop+TopLine-1; /* compute the data file bottom.     */
     Data_Refresh_flag = TRUE;          /* set the data refresh flag.     244*/
    }                                   /*                                   */
    row = VioStartOffSet;               /* put the cursor on the top row. 701*/
    break;                              /* end PREVWINDOW.                   */
                                        /*                                   */
   case TOPOFWINDOW:                    /* TOPOFWINDOW, alias C-PgUp.        */
    row = VioStartOffSet;               /* put the cursor on the top row. 701*/
    break;                              /*                                   */
                                        /*                                   */
   case LASTWINDOW:                     /* begin LASTWINDOW, alias c-end.    */
    dfp = (DFILE *)&DataFileQ;          /* ->to dfile ring pointer.          */

    /*************************************************************************/
    /* scan to the last node in the dfile ring.                              */
    /*************************************************************************/
    for( ; dfp && dfp->next; dfp = dfp->next){;}

    if( dfp != NULL )                   /* a node.                           */
    {                                   /*                                   */
     filelen = dfp->lrn+dfp->lines;     /* compute length of the data file.  */
     nexttop = filelen - TopLine;       /* compute last window top rec.      */
     if( nexttop > (int)DataFileTop )   /* if it's past the current top rec  */
     {                                  /* then compute the new top and      */
      DataFileTop = nexttop;            /* bottom.                           */
      DataFileBot = filelen - 1;        /*                                   */
      Data_Refresh_flag = TRUE;         /* set the data refresh flag.     244*/
     }                                  /*                                   */
     row = TopLine - 1;                 /* move cursor to top of window.     */
    }                                   /*                                   */
    break;                              /* end LASTWINDOW.                   */

   case NEXTWINDOW:                     /* begin NEXTWINDOW, alias PgDn.     */
    DataFileTop += TopLine-1;           /*                                   */
    DataFileBot += TopLine-1;           /*                                   */
    Data_Refresh_flag = TRUE;           /* set the data refresh flag.     244*/
    row = TopLine-1;                    /* put cursor on the bottom of window*/
    break;                              /* end NEXTWINDOW.                   */
                                        /*                                   */
   case BOTOFWINDOW:                    /* ctrl-PgDn.                        */
     row = TopLine-1;                   /* put cursor on bottom of window.   */
     break;                             /*                                   */

   case SETCURSORPOS:                                                   /*701*/
    SetDataViewCsr();                                                   /*701*/
    row = stgcsr[iview].row;                                                   /*701*/
    break;                                                              /*701*/
                                        /*                                   */
   case SHRINKSTORAGE:                  /* SHRINKSTORAGE.                    */
    if( fp->shower == showC)            /* set a view flag for the call   220*/
     view_flag = 1;                     /* to shrinkstorage.              220*/
    else                                /*                                220*/
     view_flag = 0;                     /*                                220*/
    if( ShrinkStorage(fp,view_flag) )   /* shrink it.                     220*/
     beep();                            /*                                   */
    DstatRow = TopLine;                 /* reset the status line.            */
    if(row >= (int)TopLine)             /* don't let cursor fall into the    */
     row = TopLine -1;                  /* source window.                    */
    if(TopLine != VioStartOffSet)       /* if we have closed up the data  701*/
     break;                             /* window, then go back to the       */
    FmtErr( NULL );                     /* source/asm window.                */
    return(0);                          /*                                   */

/*****************************************************************************/
/* These three functions handling leaving the data window.                   */
/*                                                                           */
/*****************************************************************************/
   case TOGGLEHIDESTORAGE:              /* hide the data window when         */
   case SHOWHIDESTORAGE:                /* leaving.                          */
    SaveTopLine = TopLine - VioStartOffSet;/* hold top line for restore.  701*/
    (* resizefunc)(fp,TopLine-VioStartOffSet);/* resize and refresh win   701*/
                                        /* will set TopLine = 0              */
    HideDataWin = TRUE;                 /* set hidden mode flag.             */
    return(-2);                         /*                                   */
                                        /*                                   */
   case ESCAPE:                                                         /*818*/
   case TOGGLESTORAGE:                  /* return to src/asm window.         */
     FmtErr( NULL );                    /*                                   */
     return(0);                         /*                                   */

   case EXPANDSTORAGE:                  /* EXPANDSTORAGE                     */
    if( fp->shower == showC)            /* check in what type of view are 701*/
       view_flag = 1;                   /* we in the source window. Depen 220*/
    else                                /* ding on that pass a flag to    220*/
       view_flag = 0;                   /* expand storage.                220*/
    if( ExpandStorage(fp,view_flag) )   /*                                220*/
     beep();                            /* beep if not expandable            */
    DstatRow = TopLine;                 /*                                   */
    break;                              /* reset the status line.            */
                                        /*                                   */
   case EXPANDVAR:                      /*                                   */
    {                                   /*                                   */
     uint recno;                        /* the data file record number.      */
     recno = DataFileTop + row;         /* establish the record number.      */
     dfp=FindNearDrec( recno );         /* find the containing dfile node.   */
     if(!dfp ||                         /* if no node or the member number   */
        recno-dfp->lrn + 1 > dfp->lines /* is beyond the end of the node     */
       )                                /* then                              */
     {                                  /*                                   */
      beep();                           /* alert the user.                   */
      FmtErr("No data to expand");      /* format an error message.          */
      break;                            /* go try again.                     */
     }                                  /*                                   */
                                        /* now, verify active function if    */
     /************************************************************************/
     /* If this node is for a stack variable, the stack frame may not        */
     /* be active. We need to check for this and send a message.             */
     /* The scope is in the node info.                                       */
     /************************************************************************/
     if( dfp->scope && !dfp->sfx )      /* if stack var and no active        */
     {                                  /* frame, then...                    */
      char   procname[64];              /* buffer for procedure name.        */
                                        /*                                   */
      CopyProcName(dfp->scope,          /* get function defining this        */
                   procname,            /* stack variable.                   */
                   sizeof(procname)     /*                                   */
                  );                    /*                                   */
      sprintf( msgbuf,                  /* build a  "function not active" 521*/
              "\"%s\" not active",      /* message.                          */
              procname);                /*                                   */
                                        /*                                   */
      beep();                           /* beep.                             */
      putmsg(msgbuf);                   /* display the message.              */
      break;                            /*                                   */
     }                                  /*                                   */
     mem = recno - dfp->lrn + 1;        /* compute the node member.          */
     newfp = NULL;
     rc = zoomrec( fp, dfp, mem, &newfp );
     if( newfp )
     {
      *fpp = newfp;
      return(0);
     }
     if(rc && rc != ESC )               /* back all the way out on ESC.      */
     {                                  /*                                   */
      beep();                           /*                                   */
      putmsg("can't expand this data"); /*                                   */
     }                                  /*                                   */

     if( fp->shower == showC )          /*                                701*/
       dovscr( fp, 0 );                 /* refresh source part of the     220*/
     else                               /* screen or else refresh the asm 220*/
     {                                  /* part of the screen             220*/
       fp->flags |= ASM_VIEW_REFRESH;   /* set the flag for refreshing    302*/
       ascroll( fp, 0 );                /* asm view                       302*/
     }
     break;
    }

caseTABRIGHT:
   case TABRIGHT:
   case RIGHTCURSOR:
   { uchar *cp;

     if( (dfp = FindNearDrec(absrow)) && dfp->zapper )
     {
      if( !(cp=ParseExpr(dfp->expr,0x10,dfp->mid,dfp->lno, dfp->sfi)) || *cp )
      {
       FmtErr("Sorry, I can't do that right now.");
      }
      else
      {
       rc = (*dfp->zapper)(dfp, row, absrow - dfp->lrn);                /*701*/
       if(  rc )                                                        /*701*/
       {                                                                /*701*/
        if( rc == LEFTMOUSECLICK )                                      /*701*/
        {                                                               /*701*/
         func = LEFTMOUSECLICK;                                         /*701*/
         goto HANDLEEVENTS;                                             /*701*/
        }                                                               /*701*/
        if( (rc == RECIRCULATE) || (rc == UP) || (rc == DOWN) )         /*701*/
        {                                                               /*701*/
         row = stgcsr[iview].row;                                              /*701*/
         absrow = DataFileTop + ( uchar )row ;                          /*701*/
         goto caseTABRIGHT;                                             /*701*/
        }                                                               /*701*/

        Re_Parse_Data = TRUE;      /* Set the parse var flag         244*/

        /*********************************************************************/
        /* - set the DataRecalc flag.                                     903*/
        /*********************************************************************/
        {                                                               /*903*/
         extern int DataRecalc;                                         /*903*/
                                                                        /*903*/
         DataRecalc = TRUE;                                             /*903*/
        }                                                               /*903*/

        goto Refresh;
       }
      }
     }
     beep();break;
   }

   case GENHELP:                        /* General help                   701*/
    {
      uchar *HelpMsg;

      HelpMsg = GetHelpMsg( HELP_WIN_DATA, NULL,0 );
      CuaShowHelpBox( HelpMsg );
    }                                  /* else fall through...              */
    break;

   case HELP:
     HelpScreen( );              /* enter our help function ( ESC key */
                                 /*   to return here )                */
      break;                     /* for now, no context sensitive help*/

   case TOGGLEASM:                                                      /*701*/
    return(func);                                                       /*701*/

   case FORMATVAR:
     dfp = FindDrec(absrow);
     if( dfp != NULL )
     {
      SetType(dfp);
      goto Refresh;
     }
     break;

   case INSERTLINE:
     InsDataFile( absrow, NULL );
     goto Refresh;

   case DELETELINE:
     DelDataFile( absrow );
     goto Refresh;

   case FIND:                                                           /*701*/
   case REPEATFIND:                                                     /*701*/
   {                                                                    /*701*/
    PEVENT Event;                                                       /*701*/
                                                                        /*701*/
    Event = GetCurrentEvent();                                          /*701*/
    if( Event->Type != TYPE_KBD_EVENT )                                 /*701*/
    {                                                                   /*701*/
      if( func == FIND )                                                /*701*/
       BlowBy = YES;                                                    /*701*/
      else                                                              /*701*/
       BoundPtr[TopLine] = 0;                                           /*701*/
      goto caseBlowBy;                                                  /*701*/
    }                                   /* else fall through...           701*/
   }
/*****************************************************************************/
/* The following block of code handles the entry and editing of data objects.*/
/* Typing and valid character or the EditFormula function will take us into  */
/* this mode.                                                                */
/*                                                                           */
/*****************************************************************************/
{                                       /*                                   */
 uint   csroff=0;                       /* offset of cursor in entry field.  */
 uint   retkey;                         /* key code returned from keystr().  */
 uchar *cp;                             /* -> to error msg from ZapExpr().   */
 uchar probuf[EXPRSIZE];                /* the data object expression buffer.*/
                                        /*                                   */
   default:                             /* case DEFAULT:                     */
   case EDITFORMULA:                    /* case EDITFORMULA:                 */

    if( func != EDITFORMULA )
    {
     /************************************************************************/
     /* We need a copy of the raw key for this function.                  701*/
     /************************************************************************/
     {                                                                  /*701*/
      PEVENT CurrentEvent;                                              /*701*/
                                                                        /*701*/
      CurrentEvent = GetCurrentEvent();                                 /*701*/
      key = CurrentEvent->Value;
     }                                                                  /*701*/
                                           /*                                   */
       if( key == ESC )
         break;

       key = key & 0xFF;                   /*                                   */
       if( key != 0x15 )                   /* let's please the italians.        */
        if( key < 0x20 ||                  /* check for wacky character and     */
            key > 0x7F                     /* inform the user.                  */
          )                                /*                                   */
        {                                  /*                                   */
         beep();
         break;
        }                                  /*                                   */
    }                                     /*                                   */

   /* case EDITFORMULA:    */
                                        /*                                   */
    dfp = FindDrec(row+DataFileTop);    /* find scoping dfile node for row.  */
    if(dfp)                             /* if a node is defined here then    */
     memcpy(probuf,                     /* copy the expression into the   101*/
            dfp->expr,                  /* expression buffer.                */
            sizeof(probuf)              /*                                   */
           );                           /*                                   */
    else                                /* else, initialize the expression   */
     *(uint *)probuf = 0;               /* buffer to double NULL.            */
                                        /*                                   */
    if( func != EDITFORMULA)            /* if a valid character was typed    */
    {                                   /*                                   */
     probuf[0] = ( uchar )key;          /* then put it the 1st char position.*/
     csroff = 1;                        /*                                   */
/*   csroff = 2;   */                   /*                                   */
    }                                   /*                                   */
                                        /*                                   */
    ClrScr( row, row, vaStgPro );       /* clear the screen.                 */
    putrc( row, 0, probuf );            /* display the initial expr buffer.  */
    retkey = '\0';                      /* initialize retkey from keystr().  */

    while( (retkey != ENTER) && (retkey != ESC) )
    {
     retkey = GetString(row,0,sizeof(probuf)-1, sizeof(probuf)-1,
                                                   &csroff, probuf, 0,NULL);


     switch( retkey )                   /* now process the expression buffer.*/
     {                                  /*                                   */
      case ENTER:                       /* if the user said go for it, then  */
       cp = ZapExpr(row+DataFileTop,    /* build a new node or process an    */
                    probuf,             /* old node.                         */
                    &csroff,            /*                                   */
                    fp                  /*                                   */
                   );                   /* tell the variable context.        */
       if(cp)                           /* if there was a problem with the   */
       {                                /* expression in the buffer then     */
        FmtErr( cp );                   /* post a bad expression message     */
        retkey = '\0';                  /* reset the retkey.                 */
        beep();                         /* alert the user.                   */
       }                                /*                                   */
       break;                           /* and resume in the while loop.     */
                                        /*                                   */
      case ESC:                         /* escape out of this mess.          */
        break;                          /*                                   */
                                        /*                                   */
      case F1:                          /* context sensitive help.           */
        Help( HELP_FORMULA );
        break;                          /*                                   */
                                        /*                                   */
      default:                          /*                                   */
        beep();                         /*                                   */
     }                                  /* end expression buffer processing. */
     csroff = 0;                        /* set cursor offset to 1st char.    */
    }                                   /* end loop til ENTER.               */
                                        /*                                   */
    if( retkey == ENTER )               /* if user left above loop with enter*/
     if( ++row == (int)TopLine )        /* then move the cursor down a row   */
      row -= 1;                         /* if possible.                      */
    goto Refresh;                       /* go refresh the display.           */
}                                       /* end case default & EDITFORMULA.   */

   case SETWATCHPOINTS:                                                 /*701*/
    SetWps(fp);                                                         /*701*/
    break;                                                              /*701*/
                                                                        /*701*/
   case FINDFUNCTION:                                                   /*701*/
   case FINDEXECLINE:                                                   /*701*/
   case NEXTFILE:                                                       /*701*/
   case DROPFILE:                                                       /*701*/
   case RESTART:                                                        /*701*/
   case QUIT:                                                           /*701*/
   case RUN:                                                            /*701*/
   case SSTEP:                                                          /*701*/
   case SSTEPINTOFUNC:                                                  /*701*/
   case RUNTOCURSOR:                                                    /*701*/
   case RUNNOSWAP:                                                      /*701*/
   case SSTEPNOSWAP:                                                    /*701*/
   case SSTEPINTOFUNCNOSWAP:                                            /*701*/
   case RUNTOCURSORNOSWAP:                                              /*701*/
   case SETCLEARBKPT:                                                   /*701*/
   case SETCLEARCONDBKPT:                                               /*701*/
   case CLEARALLBKPTS:                                                  /*701*/
   case TOGGLEASMVIEW:                                                  /*701*/
   case TOGGLEDIS:                                                      /*701*/
    BoundPtr[TopLine] = 0;                                              /*701*/
    goto caseBlowBy;

   /**************************************************************************/
   /* Functions requiring popups cannot block the purple line from        701*/
   /* blinking.                                                           701*/
   /**************************************************************************/
   case GETFUNCTION:                                                    /*701*/
   case GETADDRESS:                                                     /*701*/
   case GETFILE:                                                        /*701*/
   case BROWSE:                                                         /*701*/
   case SETFUNCTIONBKPT:                                                /*701*/
   case SETADDRESSBKPT:                                                 /*701*/
   case SETEXCEPTIONS:                                                  /*701*/
   case SHOWCALLSTACK:                                                  /*701*/
   case SHOWTHREADS:                                                    /*701*/
   case SHOWPROCESSES:                                                  /*701*/
   case SHOWDLLS:                                                       /*701*/
   case REGISTERDISPLAY:                                                /*701*/
   case EDITREGISTERS:                                                  /*701*/
   case COREGISTERDISPLAY:                                              /*701*/
   case SETCOLORS:                                                      /*701*/

    /*************************************************************************/
    /* Set up a fence to prevent the display routines from writing and    701*/
    /* rewriting the purple line that divides the source/assembler window 701*/
    /* and the data window.                                               701*/
    /*                                                                    701*/
    /* Set a flag to blow by GetFuncsFromEvents so that we can come       701*/
    /* directly back into the data window.                                701*/
    /*************************************************************************/
caseBlowBy:
    BlowBy = YES;                                                       /*701*/
    return((int)func);                                                  /*701*/
  }                                     /* end of switch on user input func. */
 }                                      /* end for(;;) loop to handle funcs. */
}                                       /* end typos();                      */
/*****************************************************************************/
/* AppendDataFile()                                                          */
/*                                                                           */
/* Description:                                                              */
/*   Add a record at the end of the dfile chian.                             */
/*                                                                           */
/* Parameters:                                                               */
/*   insdfp    input - ->to the dfile structure to insert.                   */
/*                                                                           */
/* Return:                                                                   */
/*   dfp       pointer to the appended node.                                 */
/*                                                                           */
/*                                                                           */
/* Assumptions:                                                              */
/*  insdfp points to a dfile built on the stack by the caller.               */
/*  dfplast and dfpfirst in same segment to validate pointer comparison.     */
/*                                                                           */
/*****************************************************************************/
extern DFILE *dfpfirst;
extern DFILE *dfplast;

 DFILE *
AppendDataFile(DFILE *insdfp)
{
 DFILE *dfp;                            /* -> to dfile node.                 */
/*****************************************************************************/
/* We first make a copy of the caller's dfile node in dynamic storage.       */
/* Then we scan for the last node in the dfile chain and append the          */
/* new node.                                                                 */
/*                                                                           */
/*****************************************************************************/
 dfp = (DFILE*) Talloc(sizeof(DFILE));  /* allocate dfile node.           521*/
 memcpy( dfp,insdfp, sizeof(DFILE) );   /* copy stack dfile info into it.    */
                                        /*                                   */
 for( dfplast=dfpfirst;                 /* update the pointer to the last    */
      dfplast->next;                    /* dfile node.                       */
      dfplast=dfplast->next             /*                                   */
    ){;}                                /*                                   */
                                        /*                                   */
 if( dfplast == dfpfirst )              /* if this is first node in chain,   */
  dfp->lrn = VioStartOffSet;            /* then init first rec number.    701*/
 else                                   /* otherwise,                        */
  dfp->lrn=dfplast->lrn +               /* use last node info to compute     */
           dfplast->lines;              /* loc of this node in the data file.*/
                                        /*                                   */
 dfplast->next=dfp;                     /* put the node into the chain.      */
 dfplast=dfp;                           /*                                   */
 dfp->next = NULL;                      /*                                   */
                                        /*                                   */
 return(dfp);                           /* return -> to added node.          */
}                                       /*                                   */

/*****************************************************************************/
/* InsDataFile()                                                             */
/*                                                                           */
/* Description:                                                              */
/*   Insert a record or node into the data file.                             */
/*                                                                           */
/* Parameters:                                                               */
/*   rec       input - where in the data file to put this record.            */
/*   insdfp    input - ->to the dfile structure to insert.                   */
/*                                                                           */
/* Return:                                                                   */
/*   void                                                                    */
/*                                                                           */
/*                                                                           */
/* Assumptions:                                                              */
/*  insdfp points to a dfile built on the stack by the caller.               */
/*                                                                           */
/*****************************************************************************/
 /* Note: This code assumes "next" is the 1st field in the DFILE struct. */
                      /* record number in data file to make insertion (0..N) */
                      /* data file record to insert, or NULL if none */


 void
InsDataFile(uint rec,DFILE *insdfp)
                                        /* insert dfile after this rec.      */
                                        /* -> do dfile struct to insert.     */
{                                       /*                                   */
 DFILE *dfp;                            /* -> to dfile node.                 */
                                        /*                                   */
 /****************************************************************************/
 /* - scan for the insert position.                                          */
 /****************************************************************************/
 dfp = (DFILE*)DataFileQ ;
 for( ; dfp ; dfp = dfp->next )
 {
  if( dfp->lrn >= rec )                 /*                                   */
   dfp->lrn += 1;                       /*                                   */
 }                                       /*                                   */
 if( insdfp )                           /* insert a node into the dfile.     */
 {                                      /*                                   */
  dfp = (DFILE*) Talloc(sizeof(*dfp));  /*                                521*/
  memcpy(dfp , insdfp,  sizeof(*dfp) ); /*                                101*/
  dfp->lrn = rec;                       /*                                   */
  QueueDrec( dfp );                     /*                                   */
 }                                      /*                                   */
                                        /*                                   */
}                                       /*                                   */


 void
DelDataFile(uint rec)
{
    DFILE *dfp;

    /* Note: This code assumes "next" is the 1st field in the DFILE struct. */
    dfp = (DFILE*)DataFileQ ;
    for( ; dfp ; dfp = dfp->next )
    {
        if( dfp->lrn == rec )
        {
            DropDrec( dfp );
            break;
        }
    }

    /* Note: This code assumes "next" is the 1st field in the DFILE struct. */
    dfp = (DFILE*)DataFileQ ;
    for( ; dfp ; dfp = dfp->next )
    {
        if( dfp->lrn > rec )
            dfp->lrn -= 1;
    }
}



 DFILE *
FindDrec(uint recno)
{
    DFILE *dfp;

    /* Note: This code assumes "next" is the 1st field in the DFILE struct. */
    dfp = (DFILE*)DataFileQ ;
    for( ; dfp ; dfp = dfp->next )
    {
        if( dfp->lrn == recno )
            break;
        if( dfp->lrn > recno )
            return( NULL );
    }
    return( dfp );
}

/*****************************************************************************/
/* FindNearDrec()                                                            */
/*                                                                           */
/* Description:                                                              */
/*   Find the dfile node containing a specific record.                       */
/*                                                                           */
/* Parameters:                                                               */
/*   recno     input - data file record.                                     */
/*                                                                           */
/* Return:                                                                   */
/*   dfpx      -> to dfile node or NULL.                                     */
/*                                                                           */
/*****************************************************************************/
 DFILE *
FindNearDrec(uint recno)                /* data file record number.          */
{                                       /*                                   */
 DFILE *dfp;                            /* dfile node pointer.               */
 DFILE *dfpx;                           /* dfile last node pointer.          */
                                        /*                                   */
 for( dfp = DataFileQ,                  /* scan the list of nodes for recno. */
      dfpx = NULL ;                     /*                                   */
      dfp ;                             /*                                   */
      dfp = (dfpx = dfp) -> next        /*                                   */
    )                                   /*                                   */
 {                                      /*                                   */
  if( dfp->lrn > recno )                /*                                   */
   break;                               /*                                   */
 }                                      /*                                   */
 return( dfpx );                        /*                                   */
}                                       /* end FindNearDrec().               */

 void
QueueDrec(DFILE *dfp)
{
    DFILE *this, *prev;                 /* was register.                  112*/

    /* Note: This code assumes "next" is the 1st field in the DFILE struct. */
    prev = (DFILE*)&DataFileQ ;
    this = (DFILE*)DataFileQ ;
    for( ; this ; this = (prev = this) -> next  )
    {
        if( this->lrn == dfp->lrn )
            panic(OOqdatar);
        if( this->lrn > dfp->lrn )
            break;
    }
    dfp->next = prev->next;
    prev->next = dfp;
}
 void
DropDrec(DFILE *dfp)
{
    DFILE *ptr;                         /* was register.                  112*/

    /* Note: This code assumes "next" is the 1st field in the DFILE struct. */
    for( ptr = (DFILE*)&DataFileQ; ptr ; ptr = ptr->next ){
        if( ptr->next == dfp ){
            ptr->next = dfp->next;
            Tfree( ( void * )dfp);                                       /*521*/
            return;
    }   }
    panic(OOdropdr);
}

 uchar*
ZapExpr(uint lrn,uchar *expr,uint *csroffptr,AFILE *fp)
{
    uchar *cp;                          /* was register.                  112*/
    DFILE *dfp;
    uint new;
    uint mid = fp->mid;                 /* we'll tell parseexpr() the mid.   */
    uint lno = fp->csrline+fp->Nbias;   /* add Nbias to get correct lno.  310*/
    int  sfi = fp->sfi;                 /* add Nbias to get correct lno.  310*/

    new = ((dfp = FindDrec(lrn)) == NULL);

    if( strlen(expr) )
    {
#ifdef MSH
        ParseMshExpr = 1; /*This is reset by ParseExpr.*/
#endif
        cp = ParseExpr(expr,0x10,mid,lno, sfi);
        if( cp )
        {
            extern int DataRecalc;                                         /*903*/
            if(((uint) cp) < 30)  {/*MSH return code.*/
                int iret=((uint)cp)-10;
                switch (iret)  {
                case -3:
                   break;

                case -2:
                   return((uchar *) "MSH semantic error.");

                case -1:
                   return((uchar *) "MSH syntax error.");

                case  0:
                   FmtErr((uchar *) "MSH Return code 0.");
                   Re_Parse_Data = TRUE;      /* Set the parse var flag         244*/
                   DataRecalc = TRUE;
                   return(NULL);

                case  1:
                   FmtErr((uchar *) "MSH Return code 1.");
                   Re_Parse_Data = TRUE;      /* Set the parse var flag         244*/
                   DataRecalc = TRUE;
                   return(NULL);

                case  8:
                   FmtErr((uchar *) "REXX Semantic error.");
                   Re_Parse_Data = TRUE;      /* Set the parse var flag         244*/
                   DataRecalc = TRUE;
                   return(NULL);

                case  10:
                   FmtErr((uchar *) "REXX Return code 0.");
                   Re_Parse_Data = TRUE;      /* Set the parse var flag         244*/
                   DataRecalc = TRUE;
                   return(NULL);

                case  11:
                   FmtErr((uchar *) "REXX Return code non-zero.");
                   Re_Parse_Data = TRUE;      /* Set the parse var flag         244*/
                   DataRecalc = TRUE;
                   return(NULL);

                default:
                   return((uchar *) "MSH Unknown return code.");

                } /* endswitch */
            }
            if( !*cp ){
                if( new )
                    dfp = (DFILE*) Talloc(sizeof(*dfp));                /*521*/
                memcpy(dfp->expr, expr, sizeof(dfp->expr) );            /*101*/
                dfp->lrn = lrn;
                dfp->mid = ExprMid;
                dfp->lno = ExprLno;
                dfp->sfi = sfi;
                dfp->scope = ExprScope;
                dfp->datatype = HandleUserDefs(ExprMid,ExprTid);        /*512*/
                                        /* Get primitive typeno in case   512*/
                                        /* of primitive user defs.        512*/
                dfp->baseaddr = ExprAddr;                               /*112*/
                SetShowType( dfp, dfp->datatype );
                SetShowLines( dfp );
                if( new )
                    QueueDrec(dfp);
            }else{
                *csroffptr = cp - expr;
                return( (uchar*)BadExprMsg );
            }
        }else{
            *csroffptr = 0;
            return( ParseError ? ParseError : (uchar*)BadExprMsg );
        }
    }else if( dfp )
        DropDrec(dfp);

    return( NULL );
}


/*****************************************************************************/
/* SetType()                                                              813*/
/*                                                                        813*/
/* Description:                                                           813*/
/*                                                                        813*/
/*   Set the typing of a data window variable to be reformatted.          813*/
/*                                                                        813*/
/* Parameters:                                                            813*/
/*                                                                        813*/
/*   dfp        input - -> the dfile structure defining the data item.    813*/
/*   fp         input - -> the view structure defining the data item.     813*/
/*                                                                        813*/
/* Return:                                                                813*/
/*                                                                        813*/
/*   void                                                                 813*/
/*                                                                        813*/
/* Assumptions:                                                           813*/
/*                                                                        813*/
/*************************************************************************813*/
#define MAXTYPNAMLEN 32  /* This is the # of underscores in the userdef   813*/
                         /* slot of the menuitems[].                      813*/
#define NMENUITEMS 12                                                   /*813*/
static  uint menutypes[ NMENUITEMS] =                                   /*813*/
{                                                                       /*813*/
 0,                                     /* slot for ?.                  /*813*/
 0,                                     /* slot for hex.                /*813*/
 TYPE_CHAR,                                                             /*813*/
 TYPE_SHORT,                                                            /*813*/
 TYPE_LONG,                                                             /*813*/
 TYPE_UCHAR,                                                            /*813*/
 TYPE_USHORT,                                                           /*813*/
 TYPE_ULONG,                                                            /*813*/
 TYPE_FLOAT,                                                            /*813*/
 TYPE_DOUBLE,                                                           /*813*/
 TYPE_LDOUBLE,                                                          /*813*/
 0                                      /* slot for user defined type.   *813*/
};                                                                      /*813*/
                                                                        /*813*/
void SetType(DFILE *dfp)                                                /*813*/
{                                                                       /*813*/
 UINT      n;                                                           /*813*/
 USHORT    len;                                                         /*813*/
 USHORT    typeno;                                                      /*813*/
 UINT      item;                                                        /*813*/
 UCHAR    *cp;                                                          /*813*/
 UCHAR    *mp;                                                          /*813*/
 PULLDOWN *PullPtr;
 PEVENT    Event;
 USHORT    row;
 USHORT    col;

 typeno = len = 0;
 /************************************************************************813*/
 /* 1.Get a ptr to the string of menuitems. It will look like so:         813*/
 /*                                                                       813*/
 /*   index        name     string terminator                             813*/
 /*     0          .........0                                             813*/
 /*     1          .........0                                             813*/
 /*     .          .........0                                             813*/
 /*     .          .........0                                             813*/
 /*     .          .........0                                             813*/
 /*     .          .........0                                             813*/
 /*   NMENUITEMS-1 0______________________________00<--user defined slot. 813*/
 /*                .                                                      813*/
 /*                ^                                                      813*/
 /*                |                                                      813*/
 /*                 --The "0" will be replace by a "." after the first    813*/
 /*                   time through the routine.                           813*/
 /*                                                                       813*/
 /*                   There are MAXTYPNAMLEN of the underscores.          813*/
 /*                                                                       813*/
 /****************************************************************************/
 PullPtr  = GetPullPointer( 12 );                                    /*813701*/
 mp = PullPtr->labels;                                               /*813701*/

 /****************************************************************************/
 /* 1.Get a ptr to the beginning of the user defined slot.                813*/
 /* 2.Initialize the typeno of the user defined slot.                     813*/
 /* 3.Get the type name.                                                  813*/
 /* 4.Replace the 0 prefix in the user defined slot with a '.'.           813*/
 /* 5.Set the type of the user defined slot.                              813*/
 /* 6.Copy the type name into the user defined slot and terminate it.     813*/
 /*                                                                       813*/
 /************************************************************************813*/
 for( n=NMENUITEMS-1; n ; n--, mp += strlen(mp)+1){;}                   /*813*/
 menutypes[ NMENUITEMS-1 ] = 0;                                         /*813*/
 if( (typeno = dfp->datatype) &&                                        /*813*/
     (cp = QtypeName(dfp->mid, typeno)) )                               /*813*/
 {                                                                      /*813*/
  *mp = '.';                                                            /*813*/
  menutypes[ NMENUITEMS-1 ] = typeno;                                   /*813*/
  if( (len = *(USHORT*)cp) > MAXTYPNAMLEN )                             /*813*/
   len = MAXTYPNAMLEN;                                                  /*813*/
  memcpy(mp+1, cp+2, len );                                             /*813*/
  *(USHORT*)(mp+len+1) = 0;                                             /*813*/
 }                                                                      /*813*/

 /************************************************************************813*/
 /* - Define the width of the pulldown. The width will be based on the    813*/
 /*   length of the predefined primitive names or the length of the       813*/
 /*   user defined name whichever is larger.                              813*/
 /* - Define the number of entries in the pulldown.                       813*/
 /****************************************************************************/
 PullPtr->width = 11;                                                /*813701*/
 if( (len + 5) > PullPtr->width )                                       /*813*/
   PullPtr->width = len + 5;                                            /*813*/
 PullPtr->entries = NMENUITEMS;                                      /*813806*/

 /************************************************************************813*/
 /* Define the starting upper left row,col position for the start of      813*/
 /* the pulldown. Test for the pulldown going below the screen and adjust 813*/
 /* upwards if it does.                                                   813*/
 /************************************************************************813*/
 Event = GetCurrentEvent();                                          /*813701*/
 if( Event->Value == C_ENTER )                                       /*813701*/
 {                                                                   /*813701*/
   VioGetCurPos( &row, &col, 0 );                                    /*813701*/
   if( (row + PullPtr->entries + 3 + 1) < VideoRows )                /*813701*/
     PullPtr->row = row + 1;                                         /*813701*/
   else                                                              /*813701*/
     PullPtr->row = VideoRows - PullPtr->entries - 3;                /*813701*/
   PullPtr->col = col;                                               /*813701*/
 }                                                                   /*813701*/
 else                                                                /*813701*/
 {                                                                   /*813701*/
   PullPtr->row = 11;                                                /*813701*/
   PullPtr->col = 35;                                                /*813701*/
 }                                                                   /*813701*/

 /****************************************************************************/
 /* - Get the primitive item or the tag/userdef name from the user.          */
 /* - Set the type of the shower in the dfile structure for this data        */
 /*   item.                                                                  */
 /****************************************************************************/
 {
  int  MouseState = STATE_BUTTON_RELEASED;

  item = GetObjectPullChoice( 12, &MouseState );

  switch( item )
  {
   default:
    SetShowType( dfp, menutypes[item-1] );
    break;

   case SETCURSORPOS:
   case DONOTHING:
    break;

   case 1:
    GetFormatType(dfp);
    break;

  }
  return;
 }
}

/* Note: The order of "showers" and "zappers" MUST match the TGROUP type */

static  SHOWER  showers[] = {
                             ShowHexBytes, /*  TG_UNKNOWN                    */
                             ShowScalar  , /*  TG_SCALAR                     */
                             ShowScalar  , /*  TG_POINTER                    */
                             ShowStruct  , /*  TG_STRUCT                     */
                             ShowArray   , /*  TG_ARRAY                      */
                             ShowScalar  , /*  TG_ENUM                       */
                             ShowScalar  , /*  TG_BITFLD                     */
                             ShowConstant, /*  TG_CONSTANT                   */
                             ShowClass   , /*  TG_CLASS                      */
                             ShowScalar    /*  TG_REF                        */
                            };
#ifdef MSH
static  FINDER  finders[] = {
                             FindHexBytes, /*  TG_UNKNOWN                    */
                             FindScalar  , /*  TG_SCALAR                     */
                             FindScalar  , /*  TG_POINTER                    */
                             FindStruct  , /*  TG_STRUCT                     */
                             FindArray   , /*  TG_ARRAY                      */
                             FindScalar  , /*  TG_ENUM                       */
                             FindScalar  , /*  TG_BITFLD                     */
                             FindConstant  /*  TG_CONSTANT                243*/
                            };
#endif
static  ZAPPER   zappers[] =
{
  ZapHexBytes ,                         /* TG_UNKNOWN                      */
  ZapScalar   ,                         /* TG_SCALAR                       */
  ZapPointer  ,                         /* TG_POINTER                      */
  ZapStruct   ,                         /* TG_STRUCT                       */
  ZapArray    ,                         /* TG_ARRAY                        */
  NULL        ,                         /* TG_ENUM                         */
  NULL        ,                         /* TG_BITFLD                       */
  NULL        ,                         /* TG_CONSTANT                     */
  ZapClass                              /* TG_CLASS                        */
};

 void
SetShowType( DFILE *dfp, uint tid )
{
    uint  tgroup = QtypeGroup(dfp->mid, tid);

    dfp->showtype = tid;

    if( (dfp->datatype != ADDRESS_CONSTANT) &&
      (dfp->datatype != VALUE_CONSTANT))/* if data type is not constant   243*/
    {                                   /* then set the required shower   243*/
      dfp->shower = showers[tgroup];    /* zapper, and finder.            243*/
#ifdef MSH
      dfp->finder = finders[tgroup];
#endif
      dfp->zapper = zappers[tgroup];    /*                                243*/
    }                                   /*                                243*/
    else                                /*                                243*/
    {                                   /* if data type is constant set   243*/
      dfp->shower = ShowConstant;       /* the shower to showconstant &   243*/
#ifdef MSH
      dfp->finder = FindConstant;       /* the finder to findconstant &   243*/
#endif
      dfp->zapper = NULL;               /* zapper to null.                243*/
    }                                   /*                                243*/

}
/*****************************************************************************/
/*SetShowLines()                                                             */
/*                                                                           */
/* Description:                                                              */
/*   Insert "lines" value into the dfile node.                               */
/*                                                                           */
/* Parameters:                                                               */
/*   dfp       input - -> to the dfile node we are working on.               */
/*                                                                           */
/* Return:                                                                   */
/*   void                                                                    */
/*                                                                           */
/* Assumptions:                                                              */
/*  dfp is valid and not NULL.                                               */
/*                                                                           */
/*****************************************************************************/
 int
SetShowLines(DFILE *dfp)
{
 uint   atomtype;                       /* type of array element.         101*/
 uint   atomsize;                       /* size of array element in bits.    */
 Trec  *tp;                             /* -> to type record for a typeno.   */

/*****************************************************************************/
/* We want to get the number of lines needed to display this data object.  We*/
/* will assume 1 line and make adjustments as necessary based on the complex */
/* data type.  Char arrays will be treated as strings and we will have one   */
/* line for each 16 bytes of string.                                         */
/*                                                                           */
/**u**************************************************************************/
  if( (dfp->datatype <  512) ||         /* handle primitive and constant  243*/
    (dfp->datatype == VALUE_CONSTANT)|| /* data types                     243*/
    (dfp->datatype == ADDRESS_CONSTANT))/*                                243*/
   return(dfp->lines = 1);              /* assume 1 line.                    */
                                        /* handle complex data types.        */
  tp=QbasetypeRec(dfp->mid,dfp->datatype); /* get base type record.          */
  if( !tp )                             /*                                   */
   return(0);                           /*                                   */
  switch(tp->RecType)                   /* handle each type individually. 813*/
  {                                     /*                                   */
   case T_CLASS:
   {
    dfp->lines = ((TD_CLASS*)tp)->NumMembers;

    if( GetViewMemFncs() == FALSE )
    {
     dfp->lines = GetLinesNoMemFncs( dfp->mid, tp );
    }
   }
   break;

   case T_STRUCT:                       /* type 121                          */
    dfp->lines = ((TD_STRUCT*)tp)->NumMembers;                          /*813*/
    break;                              /*                                   */
                                        /*                                   */
   case T_BITFLD:                       /* type 92.                          */
    dfp->lines = 1;                     /* assume 1 line.                    */
    break;                              /*                                   */
                                        /*                                   */
   case T_ARRAY:                        /* type 120.                         */
    atomtype = ((TD_ARRAY*)tp)->ElemType;                               /*813*/
    atomtype =  HandleUserDefs(dfp->mid,atomtype);                      /*813*/
    if(atomtype == TYPE_CHAR ||         /* if the type is char or            */
       atomtype == TYPE_UCHAR           /* unsigned char then                */
      )                                 /*                                   */
    {                                   /* treat the array as a string.      */
     uint nbytes;                       /* temp for array bytes.             */
     nbytes = ((TD_ARRAY*)tp)->ByteSize;                                /*813*/
     dfp->lines = nbytes/16;            /* this is base number of lines.     */
     if( nbytes-dfp->lines*16 > 0 )     /* if there is a vestigial, then     */
      dfp->lines += 1;                  /* add a line for it.                */
    }                                   /*                                   */
    else                                /* if the type is NOT char, then     */
    {                                   /*                                   */
     atomsize = QtypeSize(dfp->mid,     /* get the number of chars in the    */
                         atomtype);     /* string.                           */
     dfp->lines = ((TD_ARRAY*)tp)->ByteSize;                            /*813*/
     dfp->lines /= atomsize;            /* compute number of lines to display*/
    }                                   /*                                   */
    break;                              /* end case T_ARRAY:                 */
                                        /*                                   */
   case T_TYPDEF:                       /* type  93                          */
   case T_PROC  :                       /*      117                          */
   case T_PTR   :                       /*      122                          */
   case T_ENUM  :                       /*      123                          */
   case T_LIST  :                       /*      127                          */
   case T_NULL  :                       /*      128                          */
   default      :                       /*                                   */
    dfp->lines = 1;                     /*                                   */
    break;                              /*                                   */
  }                                     /*                                   */
  return(dfp->lines);                   /*                                   */
}                                       /* end SetShowLines()                */

/*****************************************************************************/
/* SetDataViewCsr()                                                       701*/
/*                                                                        701*/
/* Description:                                                           701*/
/*   Set the cursor position in the data window when a mouse event        701*/
/*   occurs.                                                              701*/
/*                                                                        701*/
/* Parameters:                                                            701*/
/*                                                                        701*/
/* Return:                                                                701*/
/*                                                                        701*/
/* Assumptions                                                            701*/
/*                                                                        701*/
/*****************************************************************************/
void  SetDataViewCsr( )                                                 /*701*/
{                                                                       /*701*/
  PEVENT  Event;                                                        /*701*/
                                                                        /*701*/
  Event = GetCurrentEvent();                                            /*701*/
  if( Event->Row >= TopLine )                                           /*701*/
   stgcsr[iview].row = TopLine-1;                                              /*701*/
  else if( Event->Row < VioStartOffSet )                                /*701*/
   stgcsr[iview].row = VioStartOffSet;                                         /*701*/
  else                                                                  /*701*/
   stgcsr[iview].row = Event->Row;                                             /*701*/
}                                                                       /*701*/
