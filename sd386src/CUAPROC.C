/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   cuaproc.c                                                               */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Cua process dialog functions.                                           */
/*                                                                           */
/*...Created 08/29/94                                                        */
/*...                                                                        */
/*****************************************************************************/
#include "all.h"
#include "diaproc.h"                    /* threads dialog data               */

#define PIDLEN       7                  /* process id field length.          */

/*****************************************************************************/
/* - define frame overhead - 2 spaces for the scroll bar + 2 spaces          */
/*                           for the left and right sides.                   */
/*****************************************************************************/
#define FRAME_OVER_HEAD   6

extern uchar   VideoAtr;                /* logical screen attribute          */
extern uchar   ScrollShade1[];          /* scroll bar attributes          701*/
extern uchar   ScrollShade2[];          /* scroll bar attributes          701*/
extern uchar   hilite[];                /* high light field attributes    701*/
extern uchar   normal[];                /* normal field attributes        701*/
extern uchar   ClearField[];            /* clear field attributes         701*/

static uchar Header[] = " Pid    Filename";

/*****************************************************************************/
/* Cua_showproc()                                                            */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*  Calls functions to handle the process dialog.                            */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/* Return:                                                                   */
/*****************************************************************************/
void Cua_showproc( void )
{
  GetScrAccess();
  DisplayDialog( &Dia_Proc, TRUE );
  ProcessDialog( &Dia_Proc, &Dia_Proc_Choices, TRUE, NULL );
  RemoveDialog( &Dia_Proc );
  SetScrAccess();
  return;
}

/*****************************************************************************/
/* DisplayProcChoice()                                                    701*/
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
void  DisplayProcChoice( DIALOGSHELL *shell, DIALOGCHOICE *ptr )
{
  uint     i;
  uint     StartIndex;
  ALLPIDS *pPid;
  char    *cp;
  int      len;
  int      bufsize;
  int      FileNameFieldLen;
  char    *pbuf;

  /***************************************************************************/
  /* - allocate memory needed for a single display row.                      */
  /* - define the size of the filename field.                                */
  /***************************************************************************/
  bufsize = shell->width - FRAME_OVER_HEAD;

  FileNameFieldLen = bufsize - PIDLEN ;
              pbuf = Talloc( bufsize + 1 );   /* add one for terminating \0. */

  /***************************************************************************/
  /* Put the header text and init variables.                                 */
  /***************************************************************************/
  putrc( shell->row + 2, shell->col + 1, Header );
  StartIndex = 1;

  /***************************************************************************/
  /* Advance the startindex and -> to process node depending on the skip rows*/
  /***************************************************************************/
  pPid    = GetAllpids();
  for( i = 0; i < ptr->SkipRows; i++ )
  {
    pPid = pPid->next;
    StartIndex++;
  }

  /***************************************************************************/
  /* Display the rows that can fit in the window.                            */
  /***************************************************************************/
  for( i = 0; i < ptr->MaxRows; i++, StartIndex++ )
  {
    /*************************************************************************/
    /* Clear the display buffer.                                             */
    /*************************************************************************/
    memset( pbuf, ' ', bufsize );
    if( StartIndex <= ptr->entries )
    {
      /**********************************************************************/
      /*  - Put the pid and name into the buffer using this layout:         */
      /*                                                                    */
      /*   PIDLEN FILENAMELEN                                               */
      /*   |     ||                                                         */
      /*   Pid----Filename----------------------------------------          */
      /*             1112222222222333333333344444444445555555555666         */
      /*   01234567890123456789012345678901234567890123456789012345         */
      /**********************************************************************/
      sprintf( pbuf, "%-6d ", pPid->pid );

      cp  = pPid->pFileSpec;
      len = strlen(cp);
      if( len > FileNameFieldLen )
      {
       cp += strlen(cp);
       cp -= FileNameFieldLen;
      }
      strncpy( pbuf+PIDLEN, cp, strlen(cp) );

      /***********************************************************************/
      /* - add the buffer termination.                                       */
      /***********************************************************************/
      pbuf[bufsize] = '\0';

      /***********************************************************************/
      /* bump to the next process.                                           */
      /***********************************************************************/
      pPid = pPid->next;
    }
    putrc( shell->row + i + shell->SkipLines, shell->col + 2, pbuf );
  }                                                /*      |                */
  Tfree( pbuf );                                    /*      |                */
}                                                  /* offset from left frame*/

/*****************************************************************************/
/*                                                                           */
/* ProcessDialogFunction()                                                   */
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
uint  ProcessDialogFunction( DIALOGSHELL *shell, DIALOGCHOICE *ptr,
                             EVENT *nEvent, void *ParamBlock )
{
  int     SelectIndex;
  ALLPIDS *plastproc;


  SelectIndex = shell->CurrentField + ptr->SkipRows;
  ptr->entries = 0;
  plastproc = GetAllpids( );

  while( plastproc != NULL )
  {
   ptr->entries++;
   plastproc = plastproc->next;
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

    case ENTER:
     /************************************************************************/
     /* This message is sent if the user clicks on the ENTER button or       */
     /* presses the ENTER key.                                               */
     /*  - Return non zero value to denote the end of dialog processing.     */
     /************************************************************************/
     {
      ALLPIDS         *pGoTo;
      ALLPIDS         *pThisPid;
      DBG_QUE_ELEMENT  Qelement;
      USHORT           ThisPid;
      USHORT           GoToPid;

      ThisPid  = (ULONG)DbgGetProcessID();
      pThisPid = GetPid( ThisPid );

      pGoTo    = GetPidIndex( SelectIndex );
      GoToPid  = pGoTo->pid;

      if( GoToPid != ThisPid )
      {
       Qelement.pid = GoToPid;
       Qelement.sid = ThisPid;

       if( (SerialParallel() == SERIAL) && (pGoTo->PidFlags.Executing==FALSE) )
       {
        Qelement.DbgMsgFlags.InformEsp = INFORM_ESP;

        SetConnectSema4( &pThisPid->ConnectSema4, FALSE );

        SendMsgToDbgQue( DBG_QMSG_CONNECT_ESP, &Qelement, sizeof(Qelement) );
       }
       Qelement.pid = GoToPid;
       SendMsgToDbgQue( DBG_QMSG_SELECT_SESSION, &Qelement, sizeof(Qelement) );
      }
      ptr->SkipRows = 0;
      return( ENTER );
     }

    case F1:
    {
      uchar  *HelpMsg;

      HelpMsg = GetHelpMsg( HELP_DLG_PROCESSES, NULL,0 );
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

    default:
     return( 0 );
  }
}
