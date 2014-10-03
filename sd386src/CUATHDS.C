/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   cuathds.c                                                               */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Cua threads dialog functions.                                           */
/*                                                                           */
/*...Release 1.01 (04/03/92)                                                 */
/*...                                                                        */
/*... 05/08/92  701   Srinivas  Cua Interface.                               */
/*****************************************************************************/
#include "all.h"
#include "diaThds.h"                    /* threads dialog data            701*/

#define TIDLEN      4                   /* thread id field length            */
#define DBGLEN      4                   /* debug state field length          */
#define THDLEN      4                   /* thread state field length         */
#define FNLEN      18                   /* file name field length            */
#define LINELEN    11                   /* line number field length          */
#define FUNCLEN    21                   /* function name field length        */

extern uchar   VideoAtr;                /* logical screen attribute          */
extern uchar   ScrollShade1[];          /* scroll bar attributes          701*/
extern uchar   ScrollShade2[];          /* scroll bar attributes          701*/
extern uchar   hilite[];                /* high light field attributes    701*/
extern uchar   normal[];                /* normal field attributes        701*/
extern uchar   ClearField[];            /* clear field attributes         701*/

extern TSTATE   *thead;                 /* thread list head pointer          */
extern AFILE    *fp_focus;              /* a holder for an afile node.       */
extern CmdParms cmd;

static uchar Header[] = "Tid Dbg Thd FileName          Line       Function";
static char *Tvals[4] =                 /* thread state values               */
{                                       /*                                   */
 "Run",                                 /* Runnable                          */
 "Sus",                                 /* Suspended                         */
 "Blk",                                 /* Blocked                           */
 "Cri"                                  /* Critical section                  */
};                                      /*                                   */
                                        /*                                   */
static char *Dvals[3] =                 /* debug state values                */
{                                       /*                                   */
 "Thw",                                 /* thawed                            */
 "Frz",                                 /* frozen                            */
 "Trm"                                  /* terminating (DBG_N_ThreadTerm)    */
};                                      /* from DosDebug                     */
                                        /*                                   */
static TSTATE  *nplast;                 /* last display node                 */
static uchar    allinfo;                /* toggle to display all threads info*/

AFILE *showthds( uint *Dummy )
{
  return( Cua_showthds() );
}

/*****************************************************************************/
/* Cua_showthds()                                                         701*/
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*     Calls functions to display, process key & mouse events & remove the   */
/*  threads dialog.                                                          */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/* Return:                                                                   */
/*   none                                                                    */
/*****************************************************************************/
AFILE  *Cua_showthds()
{
  GetScrAccess();

  DisplayDialog( &Dia_Thds, TRUE );
  ProcessDialog( &Dia_Thds, &Dia_Thds_Choices, TRUE, NULL );
  RemoveDialog( &Dia_Thds );
  SetScrAccess();
  return( fp_focus );
}

/*****************************************************************************/
/* DisplayThdsChoice()                                                    701*/
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Displays the Thread names and other related information in the dialog   */
/* window.                                                                   */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   shell   ->  pointer to a dialog shell structure.                        */
/*   ptr     ->  pointer to a dialog choice structure.                       */
/*                                                                           */
/* Return:                                                                   */
/*   none                                                                    */
/*****************************************************************************/
void  DisplayThdsChoice( DIALOGSHELL *shell, DIALOGCHOICE *ptr )
{
  UINT    i,j;
  UINT    StartIndex;
  UCHAR  *buf,*fn;
  UCHAR  *pbuf;
  ULONG   bufsize;

  /***************************************************************************/
  /* set pointer to head of the threads linked list & allocate memory requi- */
  /* red display a sinle row.                                                */
  /***************************************************************************/
  nplast  = thead;
  bufsize = shell->width - 1 - SCROLLBARCOLOFFSET;
  buf     = Talloc( bufsize );

  /***************************************************************************/
  /* Put the header text and init variables.                                 */
  /***************************************************************************/
  putrc( shell->row + 2, shell->col + 2, Header );
  StartIndex = 1;

  /***************************************************************************/
  /* Advance the startindex and -> to thread node depending on the skip rows */
  /***************************************************************************/
  for( i = 0; i < ptr->SkipRows; i++ )
  {
    nplast = nplast->next;
    StartIndex++;
  }

  /***************************************************************************/
  /* Display the rows that can fit in the window.                            */
  /***************************************************************************/
  for( i = 0; i < ptr->MaxRows; i++, StartIndex++ )
  {
    pbuf = buf + 1;
    /*************************************************************************/
    /* Clear the display buffer.                                             */
    /*************************************************************************/
    memset( buf, ' ', shell->width - 2 - SCROLLBARCOLOFFSET );
    if( StartIndex <= ptr->entries )
    {
      /**********************************************************************/
      /*  - Put the TID (thread id) and DBG (debug state ) into the buffer  */
      /*  - advance the pointer.                                            */
      /**********************************************************************/
      j = sprintf( pbuf, "%-3d %s ", nplast->tid,
                   Dvals[nplast->ts.DebugState] );
      *(pbuf +j) = ' ';
      pbuf += TIDLEN + DBGLEN;

      /**********************************************************************/
      /*  If the thread is not terminated and all info flag is set build    */
      /* all the info ie filename , line no and function name.              */
      /**********************************************************************/
      if( allinfo )                                                     /*906*/
      {
        memcpy( pbuf, Tvals[nplast->ts.ThreadState], THDLEN - 1 );
        pbuf += THDLEN;

        /*********************************************************************/
        /* If not fake get the moudle or file name                           */
        /*********************************************************************/
        if( nplast->mid != FAKEMID )
        {
          int junk;

          fn = DBModName( nplast->mid, nplast->sfi, nplast->pdf, &junk );
          if( fn )
           strncpy( pbuf, fn + 1, min( *fn, FNLEN ) );
        }
        else
        {
          uchar buffer[CCHMAXPATH];

          strcpy(buffer,nplast->pdf->DebFilePtr->fn);

          memcpy( pbuf, buffer, min( FNLEN, strlen(buffer) ) );
        }
        pbuf += FNLEN;

        /*********************************************************************/
        /* put the lno stopped info into the buffer                          */
        /*********************************************************************/
        sprintf( pbuf - 1, " line %-5u ", nplast->lno );
        pbuf += LINELEN;

        /*********************************************************************/
        /* put the function name into the buffer.                            */
        /*********************************************************************/
        fn = DBFindProcName( nplast->ip, nplast->pdf );
        if( fn )
          strncpy( pbuf, fn+2, min( *(ushort *)fn, FUNCLEN ) );
      }
      /***********************************************************************/
      /* if this is the current execthd put in the arrow mark in front       */
      /***********************************************************************/
      if( nplast == GetExecThread() )
        buf[0] = '>';
      /***********************************************************************/
      /* bump to the next thread.                                            */
      /***********************************************************************/
      nplast = nplast->next;
    }
    buf[bufsize-1] = 0; /* terminate the string */
    putrc( shell->row + i + shell->SkipLines, shell->col + 1, buf );
  }
  Tfree( buf );
}

/*****************************************************************************/
/*                                                                           */
/* ThreadsDialogFunction()                                                   */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/* This is the function called by ProcessDialog (in pulldown.c) whenever an  */
/* event occurs in the dialog. This fuction handles the events in a way      */
/* specific to this dialog.                                                  */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   shell         input  - Pointer to DIALOGSHELL structure.                */
/*   ptr           input  - Pointer to DIALOGCHOICE structure.               */
/*   nEvent        input  - Pointer to EVENT structure.                      */
/*                                                                           */
/* Return:                                                                   */
/*        = 0     - The caller has to continue processing the dialog.        */
/*       != 0     - The caller can terminate processing the dialog.          */
/*****************************************************************************/
uint  ThreadsDialogFunction( DIALOGSHELL *shell, DIALOGCHOICE *ptr,
                             EVENT *nEvent, void *ParamBlock )
{
  int     SelectIndex;
  uint    Counter;
  TSTATE *np;
  UINT    tid;

  SelectIndex = shell->CurrentField + ptr->SkipRows;
  ptr->entries = 0;
  nplast = thead;
  Counter = 1;
  while( nplast != 0 )
  {
    ptr->entries++;
    if( nEvent->Value == INIT_DIALOG )
    {
      if( nplast == GetExecThread() )
        SelectIndex = Counter;
    }
    else
    {
      if( Counter == SelectIndex )
        np = nplast;
    }
    nplast = nplast->next;
    Counter++;
  }

  switch( nEvent->Value )
  {
    case INIT_DIALOG:
    {
      /*********************************************************************/
      /* This message is sent before displaying elements in display area.  */
      /* It is to initialise the locals in the dialog function.            */
      /*  - Set the field cursor and variables to the first field.         */
      /*  - Set the length of the field highlite bar in normal and hilite  */
      /*    arrays (used in the call to putrc).                            */
      /*  - Return zero to denote continue dialog processing.              */
      /*********************************************************************/
      hilite[1] = normal[1] = (UCHAR)shell->width - SCROLLBARCOLOFFSET - 2;
      putrc( shell->row + 2, shell->col + 2, Header );
      return( 0 );
    }

    case key_H:
    case key_h:
    {
      xThawThread( 0 );                                                 /*827*/
      BuildThreadList();
      DisplayThdsChoice( shell, ptr );
      return( 0 );
    }

    case key_R:
    case key_r:
    {
      xFreezeThread( 0 );                                               /*827*/
      BuildThreadList();
      DisplayThdsChoice( shell, ptr );
      return( 0 );
    }

    case key_F:
    case key_f:
    {
      tid = np->tid;
      xFreezeThread( tid );                                             /*827*/
      BuildThreadList();
      /***********************************************************************/
      /* - BuildThreadList wipes out the thread list so we have to update.   */
      /***********************************************************************/
      np = GetThreadTSTATE( tid );
      putrc( shell->row + shell->SkipLines + shell->CurrentField - 1,
             shell->col + 2 + TIDLEN,
            Dvals[np->ts.DebugState] );
      return( 0 );
    }

    case key_T:
    case key_t:
    {
      tid = np->tid;
      xThawThread( tid );                                               /*827*/
      BuildThreadList();
      /***********************************************************************/
      /* - BuildThreadList wipes out the thread list so we have to update.   */
      /***********************************************************************/
      np = GetThreadTSTATE( tid );
      putrc( shell->row + shell->SkipLines + shell->CurrentField - 1,
             shell->col + 2 + TIDLEN,
            Dvals[np->ts.DebugState] );
      return( 0 );
    }

    case key_A:
    case key_a:
    {
      allinfo ^= 0x1;
      DisplayThdsChoice( shell, ptr );
      return( 0 );
    }

    case ENTER:
    {
      /*********************************************************************/
      /* This message is sent if the user clicks on the ENTER button or    */
      /* presses the ENTER key.                                            */
      /*  - Return non zero value to denote the end of dialog processing.  */
      /*********************************************************************/
      ptr->SkipRows = 0;
      return( ENTER );
    }

    case F1:
    {
      uchar  *HelpMsg;

      HelpMsg = GetHelpMsg( HELP_DLG_THREADS, NULL,0 );
      CuaShowHelpBox( HelpMsg );
      return( 0 );
    }

    case ESC:
    case F10:
    {
      /*********************************************************************/
      /* This message is sent if the user clicks on the CANCEL button or   */
      /* presses the ESC/F10 keys.                                         */
      /*  - Return non zero value to denote the end of dialog processing.  */
      /*********************************************************************/
      ptr->SkipRows = 0;
      return( nEvent->Value );
    }

    case SPACEBAR:
    {
      /* 906 removed a check for thread in terminating state.*/
      {
        RunThread( np->tid );
        DisplayThdsChoice( shell, ptr );
      }
      return( 0 );
    }

    default:
      return( 0 );
  }
}
