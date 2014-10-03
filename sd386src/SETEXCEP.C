/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   setexcep.c                                                              */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Allow users to modify exception notifications reporting and optionally  */
/*   save the new settings to user profile, SD386.PRO.                       */
/*                                                                           */
/* History:                                                                  */
/*                                                                           */
/*   10/29/91    308  Srinivas  Created to allow handling of exception       */
/*                              notifications.                               */
/*                                                                           */
/*...Release 1.00 (Pre-release 107 11/13/91)                                 */
/*...                                                                        */
/*... 11/13/91  400   Srinivas  Vertical Register Display.                   */
/*...                                                                        */
/*...Release 1.00 (Pre-release 108 12/05/91)                                 */
/*...                                                                        */
/*... 02/21/92  521   Srinivas  Port to C-Set/2.                             */
/*...                                                                        */
/*...Release 1.01 (04/03/92)                                                 */
/*...                                                                        */
/*... 05/08/92  701   Srinivas  Cua Interface.                               */
/*...                                                                        */
/*...Release 1.05 (05/27/93)                                                 */
/*...                                                                        */
/*... 05/27/93  825   Joe       Reading SD386.PRO after a save causes trap.  */
/*...                                                                        */
/**Includes*******************************************************************/
#define INCL_16                         /* for 16-bit API                    */
#define INCL_SUB                        /* kbd, vio, mouse routines          */
#include "all.h"                        /* SD386 include files               */
#define __MIG_LIB__                                                     /*521*/
#include "io.h"                         /* for routines open, chsize, close  */
#include "fcntl.h"                      /* for O_WRONLY, O_TEXT              */
#include "diaexcep.h"                   /* exceptions dialog data         701*/

/**Variables defined**********************************************************/

#define MAXFNAMELEN   129               /* 1 byte len+128 char for SD86.PRO  */
#define VATTLENGTH   30                 /* exception name length             */
#define NOTIFICATION 9                  /* notification field length         */
#define MAXWINDOWSIZE HEADERSIZE+MAXEXCEPTIONS*(VATTLENGTH+NOTIFICATION+1)
                                        /* window text buffer size           */
#define NOTIFYMSGWIDTH 10               /* notiyfield len in cua_interface701*/
#define FILENOTFOUND   2                /* file-not-found return code        */

/**Externs********************************************************************/
extern char   *ExcepTypes[MAXEXCEPTIONS];/* exception names                  */
extern uint    VideoRows;               /* logical screen rows               */
extern uchar   ExceptionMap[MAXEXCEPTIONS]; /* exception notify/nonotify.    */
extern uchar   VideoAtr;                /* logical screen attribute          */
extern uchar   ScrollShade1[];          /* scroll bar attributes             */
extern uchar   ScrollShade2[];          /* scroll bar attributes             */
extern uchar   hilite[];                /* high light field attributes       */
extern uchar   normal[];                /* normal field attributes           */
extern uchar   ClearField[];            /* clear field attributes            */
extern uchar   InScrollMode;            /* flag to tell we are in scrollmode */
extern VIOCURSORINFO  NormalCursor;     /* make underscore flashing cursor   */
extern CmdParms cmd;
extern char    *ExcepSel[];             /* notification names             825*/

uchar LocalExcepMap[MAXEXCEPTIONS];     /* local copy of exceptions map   521*/
                                        /* removed static                 521*/
static uchar DefExcepMap[MAXEXCEPTIONS];/* constant copy of exception map  */
static enum { ExpField,                 /* user currently at exception name  */
              NotifyField               /* user currently at notify  field   */
            } fld;                      /* user current field position       */

/*****************************************************************************/
/* SaveExceptions()                                                          */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Save the exception names and their exception notifications to           */
/*   profile SD386.PRO.                                                      */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   fn        input - filename to save color values                         */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   0         Save sucessful                                                */
/*   1         Save unsucessful                                              */
/*                                                                           */
/*****************************************************************************/
                                        /*                                   */
 uint                                   /*                                   */
SaveExceptions( uchar *fn )             /*                                   */
{                                       /*                                   */
 FILE   *fp;                            /* file ptr to SD386.PRO             */
 char    _word[90];                     /* buffer to scan for Start_Of_Excep */
 uchar   line[VATTLENGTH+NOTIFICATION]; /* to contain excep & excep notifi   */
 uchar  *pText;                         /* pointer to a text string          */
 fpos_t  pos;                           /* file position                     */
 int     fh;                            /* file handle for SD386.PRO         */
 uchar  *pAttByte = LocalExcepMap;      /* video attribute byte index        */
 uint    i;                             /* attribute item index              */
                                        /*                                   */
 if( (fp = fopen(fn, "r+")) == NULL )   /* if can not open SD386.PRO,        */
  return( FILENOTFOUND );               /* return bad code                   */
                                        /*                                   */
 while(fscanf(fp, "%s", _word) != EOF && /* scan while not end of file and   */
       stricmp(_word, "Start_Of_Exceptions")){;}                        /*521*/
                                        /* not found "Start_Of_Exceptions"   */
 if( feof( fp ) )                       /* if "Start_Of_Exceptions" not exist*/
  fprintf( fp, "\n\nStart_Of_Exceptions");/* put it into profile          825*/
 else                                   /* else found "Start_Of_Exceptions"  */
 {                                      /*                                   */
  if( fgetpos( fp, &pos) ||             /* initialize file header position   */
      fsetpos( fp, &pos)                /* for future writes to file         */
    )                                   /*                                   */
   return( 1 );                         /* if bad file access, return error  */
 }                                      /*                                   */
                                        /*                                   */
 fprintf(fp,"\n/*\n         EXCEPTION TYPE           NOTIFICATION");
 fprintf(fp,    "\n      --------------------       --------------\n*/");
                                    /* write text after "Start_Of_Exceptions"*/
 /****************************************************************************/
 /* From the first exception name until last, write the exception name and   */
 /* its notification value out to file SD386.PRO.                            */
 /****************************************************************************/
 line[VATTLENGTH+NOTIFICATION-1] = 00;  /* line terminator                   */
 for( i = 0;                            /* start with first exception name   */
      i <  MAXEXCEPTIONS;               /* until the last exception name     */
      i++, pAttByte++                   /* increment indices                 */
    )                                   /* loop for all exception names      */
 {                                      /*                                   */
  memset(line, ' ', VATTLENGTH+NOTIFICATION-1);/* pad buffer with all blanks     */
                                        /*                                   */
  pText = ExcepTypes[i];                /* get exception name                */
  strncpy(line, pText, strlen(pText));  /* copy excpetion name to buffer     */
                                        /*                                   */
  pText = ExcepSel[*pAttByte];          /* get exception notification        */
  strcpy(line + VATTLENGTH, pText );    /* copy exception notification       */
                                        /* to buffer                         */
                                        /*                                   */
  /***************************************************************************/
  /* - Removed the space at the end of the line.                          825*/
  /***************************************************************************/
  fprintf(fp, "\n      %s",line);       /* write buffer out to file.         */
 }                                      /*                                   */
 /****************************************************************************/
 /* After we are finished writing to the file, we now want to be certain to  */
 /* erase all junk text after our writing to the file.                       */
 /****************************************************************************/
 if( fgetpos( fp, &pos)  ||             /* get last position after our write */
     fclose( fp ) == EOF ||             /* close SD386.PRO                   */
    (fh = open(fn, O_WRONLY | O_TEXT))  /* open SD386.PRO to adjust file size*/
        == EOF           ||             /*                                   */
     chsize(fh, pos.__fpos_elem[0])  || /* truncate junk after last write 521*/
     close( fh )                        /* close SD386.PRO                   */
   )                                    /*                                   */
  return( 1 );                          /* if bad file access, return error  */


 /****************************************************************************/
 /* Add the last cr-lf-eof sequence to the end of the file.               825*/
 /****************************************************************************/
 fp = fopen(fn, "ab");                                                  /*825*/
 if( (fp == NULL ) ||                                                   /*825*/
     fseek(fp,0L,SEEK_END) )                                            /*825*/
  return(1);                                                            /*825*/
                                                                        /*825*/
 fprintf(fp,"\xD\xA\x1A");                                              /*825*/
 fclose(fp);                                                            /*825*/

 return( 0 );                           /* else, return okay                 */
}                                       /* end SaveExceptions                */


/*****************************************************************************/
/* SetExceptions()                                                        701*/
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*     Calls functions to display, process key & mouse events & remove the   */
/*  exception notification map dialog.                                       */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/* Return:                                                                   */
/*   none                                                                    */
/*****************************************************************************/
void SetExceptions()
{
  /***************************************************************************/
  /*  - Get the Screen write access.                                         */
  /*  - Display the Exceptions Dialog.                                       */
  /*  - Process the Events in the Exceptions Dialog.                         */
  /*  - Remove the Exceptions Dialog.                                        */
  /*  - Restore the Screen write access.                                     */
  /***************************************************************************/
  GetScrAccess();
  DisplayDialog( &Dia_Excep, TRUE );
  ProcessDialog( &Dia_Excep, &Dia_Excep_Choices, TRUE, NULL );
  RemoveDialog( &Dia_Excep );
  SetScrAccess();
  xSetExceptions(ExceptionMap,sizeof(ExceptionMap) );
}

/*****************************************************************************/
/* DisplayExcepChoice()                                                   701*/
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Displays the exception names and the notification type in the dialog    */
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
void  DisplayExcepChoice (DIALOGSHELL *shell,DIALOGCHOICE *ptr)
{
  uint   i;
  uint   StartIndex;                    /*                                   */
  uchar  *pExcepMap;                    /*                                   */

  /***************************************************************************/
  /* Clear the dialog.                                                       */
  /***************************************************************************/
  for( i=0; i<ptr->MaxRows; i++)
  {
   ClearField[1] = VATTLENGTH + NOTIFICATION;
   putrc(shell->row+i+shell->SkipLines,shell->col+1, ClearField);
  }

  /***************************************************************************/
  /* init the variables required.                                            */
  /***************************************************************************/
  pExcepMap = LocalExcepMap;
  StartIndex = 0;

  /***************************************************************************/
  /* Advance the startindex and notification ptrs depending on the skip rows.*/
  /***************************************************************************/
  for( i = 0; i < ptr->SkipRows; i++ )
  {
    StartIndex++;
    pExcepMap++;
  }

  for(i=0;i<ptr->MaxRows;i++)
  {
   putrc(shell->row+i+shell->SkipLines, shell->col+2,ExcepTypes[StartIndex]);
   putrc(shell->row+i+shell->SkipLines, shell->col+2+VATTLENGTH, ExcepSel[*pExcepMap]);

   StartIndex++;
   pExcepMap++;
  }
}

/*****************************************************************************/
/* SetDefaultExcepMap()                                                   701*/
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Copy exception notify map to a default map used after wards.            */
/*                                                                           */
/* Parameters:                                                               */
/*   none                                                                    */
/*                                                                           */
/* Return:                                                                   */
/*   none                                                                    */
/*****************************************************************************/
void SetDefaultExcepMap()
{
  memcpy( DefExcepMap, ExceptionMap, MAXEXCEPTIONS );
}

/*****************************************************************************/
/*                                                                           */
/* ExcepDialogFunction()                                                     */
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
uint  ExcepDialogFunction( DIALOGSHELL *shell, DIALOGCHOICE *ptr,
                           EVENT *nEvent, void *ParamBlock )
{
  uint   fldcol;
  uchar  *pExcepMap;                    /* Pointer to exception map.         */
  int    SelectIndex;                   /* Index into exception map array.   */

  SelectIndex = shell->CurrentField + ptr->SkipRows - 1;
  pExcepMap = LocalExcepMap + SelectIndex;
  for( ;; )
  {
    switch( nEvent->Value )
    {
      case INIT_DIALOG:
      {
        /*********************************************************************/
        /* This message is sent before displaying elements in display area.  */
        /* It is to initialise the locals in the dialog function.            */
        /*  - Make a copy of the global exception map.                       */
        /*  - Set the field cursor and variables to the first field.         */
        /*  - Set the length of the field highlite bar in normal and hilite  */
        /*    arrays (used in the call to putrc).                            */
        /*  - Return zero to denote continue dialog processing.              */
        /*********************************************************************/
        memcpy( LocalExcepMap, ExceptionMap, MAXEXCEPTIONS );
        fldcol = shell->col + 2;
        VioSetCurPos( (short)(shell->row + shell->SkipLines +
                       shell->CurrentField - 1), (short)fldcol, 0 );
        fld = ExpField;

        hilite[1] = normal[1] = (UCHAR)VATTLENGTH - 4;
        return( 0 );
      }

      case ENTER:
      {
        /*********************************************************************/
        /* This message is sent if the user clicks on the ENTER button or    */
        /* presses the ENTER key.                                            */
        /*  - Copy the local excetion map to the global exception map.       */
        /*  - Return non zero value to denote the end of dialog processing.  */
        /*********************************************************************/
        ptr->SkipRows = 0;
        memcpy( ExceptionMap, LocalExcepMap, MAXEXCEPTIONS );
        return( ENTER );
      }

      case F1:
      {
        uchar  *HelpMsg;

        HelpMsg = GetHelpMsg( HELP_DLG_EXCEPTIONS, NULL,0 );
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

      case MOUSEPICK:
      {
        /*********************************************************************/
        /* This message is sent if the clicks in the dialog display area.    */
        /*  - Determine on which field the user clicked.                     */
        /*  - Set the cursor on that field.                                  */
        /*  - If the field turns out to be notify field,                     */
        /*     - Set the event value as SPACEBAR (simulate) and stay in the  */
        /*       outer for loop to process SPACEBAR.                         */
        /*    Else                                                           */
        /*     - Return zero to denote continue dialog processing.           */
        /*********************************************************************/
        if( ((uint)nEvent->Col >= (shell->col + 2 + VATTLENGTH)) &&
            ((uint)nEvent->Col < (shell->col+ 2 + VATTLENGTH + NOTIFYMSGWIDTH)))
        {
           fldcol = shell->col+ 2 + VATTLENGTH;
           fld = NotifyField;
        }
        else
        {
           fldcol = shell->col+2;
           fld = ExpField;
        }

        VioSetCurPos( (short)(shell->row + shell->SkipLines +
                       shell->CurrentField - 1), (short)fldcol, 0 );

        if( fld == NotifyField )
        {
          nEvent->Value = SPACEBAR;
          continue;
        }

        return( 0 );
      }

      case key_R:
      case key_r:
      {
        /*********************************************************************/
        /* This message is sent if the user clicks in the RESET button or    */
        /* presses 'R' / 'r' keys.                                           */
        /*  - Reset the current exception notification to default.           */
        /*  - Display the new notification value in the field.               */
        /*  - Return zero to denote continue dialog processing.              */
        /*********************************************************************/
        *pExcepMap = DefExcepMap[ SelectIndex ];
        putrc( shell->row + shell->SkipLines + shell->CurrentField - 1,
               shell->col + 2 + VATTLENGTH, ExcepSel[*pExcepMap] );
        return( 0 );
      }

      case key_D:
      case key_d:
      {
        /*********************************************************************/
        /* This message is sent if the user clicks in the DEFAULT button or  */
        /* presses 'D' / 'd' keys.                                           */
        /*  - Copy the default exception map to the local exception map.     */
        /*  - Redisplay the display area of the dialog.                      */
        /*  - Return zero to denote continue dialog processing.              */
        /*********************************************************************/
        memcpy( LocalExcepMap, DefExcepMap, MAXEXCEPTIONS );
        DisplayExcepChoice( shell, ptr );
        return( 0 );
      }

      case key_S:
      case key_s:
      {
        /*********************************************************************/
        /* This message is sent if the user clicks on the SAVE button or     */
        /* presses the 'S' / 's' keys.                                       */
        /*  - Find the fully qualified file name for the .pro                */
        /*  - Call function to save the exceptions.                          */
        /*  - If not successfull in saving exceptions, display the error box */
        /*    with appropriate error message.                                */
        /*  - If successfull display a timmed message and copy the local     */
        /*    exception map to the default exception map.                    */
        /*  - Return zero to denote continue dialog processing.              */
        /*********************************************************************/
        uchar  StatusMsg[100];
        uchar  fn[MAXFNAMELEN];
        int    rc;

        findpro( "SD386.PRO", fn, MAXFNAMELEN );
        rc = SaveExceptions( fn );
        if( rc )
        {
          if( rc == FILENOTFOUND )
            sprintf( StatusMsg, "Unable to find \"SD386.PRO\" in DPATH" );
          else
            sprintf( StatusMsg, "Unable to save to profile \"%s\"", fn );
        }
        else
        {
          sprintf( StatusMsg, "Saving Exceptions to profile \"%s\"", fn );
          memcpy( ExceptionMap, LocalExcepMap, MAXEXCEPTIONS );
        }
        SayMsgBox2( StatusMsg, 1200UL );
        VioSetCurType( &NormalCursor, 0 );
        return( 0 );
      }

      case RIGHT:
      case TAB:
      {
        /*********************************************************************/
        /* This message is sent if the user presses TAB or right arrow  key.*/
        /*  - Depending on the current field determine to which field we have*/
        /*    to move to the right or loop around.                           */
        /*  - Set the field variables.                                       */
        /*  - Set the cursor position.                                       */
        /*  - Return zero to denote continue dialog processing.              */
        /*********************************************************************/
        switch( fld )
        {
          case ExpField:
          {
            fldcol = shell->col + 2 + VATTLENGTH;
            fld    = NotifyField;
            break;
          }

          case NotifyField:
          {
            fldcol = shell->col + 2;
            fld    = ExpField;
            break;
          }
        }
        VioSetCurPos( (short)(shell->row + shell->SkipLines +
                       shell->CurrentField - 1), (short)fldcol, 0 );
        return( 0 );
      }

      case LEFT:
      case S_TAB:
      {
        /*********************************************************************/
        /* This message is sent if the user presses SHIFT TAB or left arrow  */
        /*  key.                                                            */
        /*  - Depending on the current field determine to which field we have*/
        /*    to move to the left or loop around.                            */
        /*  - Set the field variables.                                       */
        /*  - Set the cursor position.                                       */
        /*  - Return zero to denote continue dialog processing.              */
        /*********************************************************************/
        switch( fld )
        {
          case ExpField:
          {
            fldcol = shell->col + 2 + VATTLENGTH;
            fld    = NotifyField;
            break;
          }

          case NotifyField:
          {
            fldcol = shell->col + 2;
            fld    = ExpField;
            break;
          }
        }
        VioSetCurPos( (short)(shell->row + shell->SkipLines +
                       shell->CurrentField - 1), (short)fldcol, 0 );
        return( 0 );
      }

      case SPACEBAR:
      {
        /*********************************************************************/
        /* This message is sent if the user presses the SPACEBAR.            */
        /*  - Toggle the notification value of the current field.            */
        /*  - Display the new notification value in the field.               */
        /*  - Return zero to denote continue dialog processing.              */
        /*********************************************************************/
        pExcepMap = LocalExcepMap + SelectIndex;
        *pExcepMap = (uchar)(*pExcepMap + 1);
        if( *pExcepMap > 1 )
          *pExcepMap  = 0;

        ClearField[1] = NOTIFICATION;
        putrc( shell->row + shell->SkipLines + shell->CurrentField - 1,
               shell->col + 2 + VATTLENGTH, ClearField );
        putrc( shell->row + shell->SkipLines + shell->CurrentField - 1,
               shell->col + 2 + VATTLENGTH, ExcepSel[*pExcepMap] );
        return( 0 );
      }

      case UP:
      case DOWN:
      case SCROLLUP:
      case SCROLLDOWN:
      case SCROLLBAR:
      {
        /*********************************************************************/
        /* These messages are sent if the user presses up  or down  arrow  */
        /* keys or the user clicks on the  or  in the scrollbar.           */
        /*  - Set the field variables.                                       */
        /*  - Set the cursor position.                                       */
        /*  - Return zero to denote continue dialog processing.              */
        /*********************************************************************/
        switch( fld )
        {
          case ExpField:
          {
            fldcol = shell->col + 2;
            break;
          }

          case NotifyField:
          {
            fldcol = shell->col + 2 + VATTLENGTH;
            break;
          }
        }

        VioSetCurPos( (short)(shell->row + shell->SkipLines +
                       shell->CurrentField - 1), (short)fldcol, 0 );
        return( 0 );
      }

      default:
        /*********************************************************************/
        /* Any other message.                                                */
        /* - Return zero to denote continue dialog processing.               */
        /*********************************************************************/
        return( 0 );
    }
  }
}
