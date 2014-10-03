/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   keystr.c                                                                */
/*                                                                           */
/* Description:                                                              */
/*   Edit a screen field.                                                    */
/*                                                                           */
/* History:                                                                  */
/*                                                                           */
/*   02/08/91 Creation of 32-bit SD86, from 16-bit version.                  */
/*                                                                           */
/*...16->32 port.                                                            */
/*...                                                                        */
/*... 02/08/91  100   Philip    port to 32 bit.                              */
/*... 02/08/91  101   Joe       port to 32 bit.                              */
/*... 02/08/91  108   Dave      port to 32 bit.                              */
/*                                                                           */
/*...Release 1.00 (Pre-release 1)                                            */
/*...                                                                        */
/*... 08/16/91  227   srinivas  Insert key problems while entering user      */
/*                              response.                                    */
/*...                                                                        */
/*...Release 1.00 (Pre-release 108 12/05/91)                                 */
/*...                                                                        */
/*... 02/13/92  525   Srinivas  Prompt for editing variables not working     */
/*...                           properly when register display is on.        */
/*...                                                                        */
/*...Release 1.00 (03/03/92)                                                 */
/*...                                                                        */
/*... 03/10/92  602   Srinivas  Hooking up watch points.                     */
/*...                                                                        */
/*...Release 1.01 (04/03/92)                                                 */
/*...                                                                        */
/*... 05/08/92  701   Srinivas  Cua Interface.                               */
/*...                                                                        */
/*...Release 1.02 (10/22/92)                                                 */
/*...                                                                        */
/*... 04/14/93  820   Selwyn    Add /u option to not flush k/b buffer.       */
/*... 04/15/93  821   Selwyn    Changes in processing clicks on buttons.     */
/*... 12/10/93  910   Joe       Clear fields on cursor sensitive prompting.  */
/*****************************************************************************/
/**Includes*******************************************************************/
                                        /*                                   */
#define INCL_16                         /* for 16-bit API                 101*/
#define INCL_SUB                        /* kbd, vio, mouse routines       101*/
#include "all.h"                        /* SD86 include files                */
static int iview=0;                     /*                                   */
/**Externs********************************************************************/
/*                                                                           */
/*                                                                           */
extern uchar*      VideoPtr;            /* Pointer to logical video buffer   */
extern uint        VideoCols;           /* # of columns per row on screen    */
extern uchar       *BoundPtr;           /* -> to screen bounds            525*/
extern CmdParms     cmd;                /* pointer to CmdParms structure  701*/
extern ushort       MouseRow;           /* Mouse Screen Row position      701*/
extern ushort       MouseCol;           /* Mouse Screen Column position   701*/

extern VIOCURSORINFO      NormalCursor;
extern VIOCURSORINFO      InsertCursor;
extern VIOCURSORINFO      HiddenCursor;

#define RCBREAK  0x0001      /* exit field */
#define RCREDRAW 0x0002      /* redraw screen */
#define RCDATAXT 0x0004      /* data key exit */

static uchar    *FieldBase=NULL;        /*-> to 1st byte of fld in video buf */
static uint      FieldType=0;           /* field type flags                  */
static uint      RightScrollOffset;
static uint      LeftScrollOffset;
static uint      FieldDisplayLength;
static uchar    *Buffer;

static PEVENT    Event;

/*****************************************************************************/
/* GetString()                                                               */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/* Get a string from the user of a specified length in a specified location  */
/* in the screen. This function provides horizontal scrolling facility.      */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/* FieldRow      (input) - Starting row of the string.                       */
/* FieldCol      (input) - Starting column of the string.                    */
/* length        (input) - Total length of the string buffer.                */
/* displen       (input) - Length of the string to be displayed on screen.   */
/* cursor (input/output) - Position of the cursor within the string.         */
/* buffer       (output) - Buffer to hold the string typed in by the user.   */
/* pShell        - -> to control structure for the popup.                 910*/
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Mouse clicks in the form of keys.                                         */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/* This function assumes the memory needed for storing the user string has   */
/* already been allocated by the caller.                                     */
/*                                                                           */
/*****************************************************************************/
#define MOU_STATE_UP   1
#define MOU_STATE_DOWN 2
uint GetString( uint FieldRow, uint FieldCol, uint length, uint displen,
                uint *cursor, uchar *pInbuf, uint InFlags, POPUPSHELL *pShell)
{
  /***************************************************************************/
  /* Diagram to explain local variables.                                     */
  /*                                                                         */
  /*  0  ...     10        ...                   40       ...          200   */
  /*                                                                         */
  /*             +-------------------------------+                           */
  /*  +----------|                               |---------------------+     */
  /*  |          |  Portion of the string shown  |                     |     */
  /*  |          |  in the screen.               |                     |     */
  /*  +----------|                               |---------------------+     */
  /*             +-------------------------------+                           */
  /*             |<---- Display Length --------->|                           */
  /*                                                                         */
  /*  |<------------ length (length of the entire buffer) ------------>|     */
  /*                                                                         */
  /*   In the above example:                                                 */
  /*                                                                         */
  /*   length            ===>  200                                           */
  /*   DisplayLength     ===>  30                                            */
  /*   DisplayOffset     ===>  10                                            */
  /*                                                                         */
  /***************************************************************************/
  uint        key;
  int         n, i;
  ushort      rc;
  int         voff;
  uint        IsInsertMode = 0, scratch, BufLen;
  uint        DisplayOffset;
  uint        CursorRow, CursorCol;
  int         FirstKeyEntry = TRUE;                                     /*910*/
  uint        flags = InFlags;                                          /*910*/
  int         state = MOU_STATE_UP; /* assume this! */                  /*910*/
  int         NextState = 0;                                            /*910*/
  BUTTON     *pButtonDown = NULL;                                       /*910*/

  /***************************************************************************/
  /*                                                                         */
  /* flags  - Flags which indicate type of the field.                        */
  /*          (AUTOEXIT / HEXONLY / BINONLY)                                 */
  /***************************************************************************/
  if(pShell)
   flags = pShell->Flags;                                               /*910*/

  /***************************************************************************/
  /* - Set keyboard flush buffer flag to indiacte not to flush the keyboard  */
  /*   buffer while we are in getstring.                                     */
  /* - Initialise the local variables.                                       */
  /***************************************************************************/
  SetFlushBufferFlag( NOFLUSHNOW );                                     /*820*/
  FieldDisplayLength = displen;
  DisplayOffset = 0;
  CursorRow = FieldRow;

  CursorCol = ( *cursor > displen ) ? FieldCol : FieldCol + *cursor;

#ifdef MSH
  if(iview) {
    CursorRow+=RowStart;
    FieldRow+=RowStart;
    CursorCol+=ColStart;
    FieldCol+=ColStart;
  }
#endif

  /***************************************************************************/
  /* Allocate memory for the local buffer.                                   */
  /***************************************************************************/
  Buffer = (uchar *)Talloc( (length+1) * sizeof( uchar ) );
  memset( Buffer, '\0', length+1 );
  if(!iview) {
  /***************************************************************************/
  /* Adjust display length if the length specified by the caller goes out of */
  /* the screen.                                                             */
  /***************************************************************************/
  if( FieldCol + FieldDisplayLength > VideoCols )
    FieldDisplayLength = VideoCols - FieldCol;

  /***************************************************************************/
  /* Check to see if the field would over lap some other window, if so adjust*/
  /* the display length accordingly.                                         */
  /***************************************************************************/
  if( FieldDisplayLength + FieldCol > BoundPtr[FieldRow] )
    FieldDisplayLength = BoundPtr[FieldRow] - FieldCol;
  }
  else
  {
#ifdef MSH
      if( FieldDisplayLength + FieldCol - ColStart> VideoWidth )
        FieldDisplayLength = VideoWidth - FieldCol + ColStart;
#endif

  }/* End if*/
  /***************************************************************************/
  /* RightScrollOffset is the amount of scrolling to be done when the user   */
  /* goes out of the display length. It is 2/3 rds of the display length.    */
  /***************************************************************************/
  RightScrollOffset  = (FieldDisplayLength * 2)/3;
  LeftScrollOffset   = FieldDisplayLength - RightScrollOffset;

  FieldBase = (uchar *)Sel2Flat( HiFlat(VideoPtr) ) +
              ( voff = 2*(FieldRow*VideoCols + FieldCol) );
  FieldType = flags;

  /***************************************************************************/
  /* Depending on the type of the string copy the initial string to the local*/
  /* buffer. Display the string.                                             */
  /***************************************************************************/
  if( (FieldType & HEXONLY) || (FieldType & BINONLY) )
  {
    for( i = 0; i < length; i++ )
      Buffer[i] = FieldBase[i*2];
  }
  else
    strcpy( Buffer, pInbuf );

  DisplayField( DisplayOffset );
  VioShowBuf( (ushort)voff,(ushort) (2*FieldDisplayLength), 0 );

  VioSetCurType( &NormalCursor, 0 );

  for(;;)
  {
    VioSetCurPos( (ushort)CursorRow, (ushort)CursorCol, 0 );

    Event = GetEvent( SEM_INDEFINITE_WAIT );

    switch( Event->Type )
    {
      case TYPE_MOUSE_EVENT:                                            /*910*/
      {                                                                 /*910*/
       /******************************************************************910*/
       /* Test for mouse event between the string brackets.               910*/
       /******************************************************************910*/
       if( ( Event->Row == (ushort)FieldRow ) &&                        /*910*/
           ( Event->Col >= (ushort)FieldCol ) &&                        /*910*/
           ( Event->Col <= ((ushort)FieldCol+(ushort)FieldDisplayLength-1))
         )                                                              /*910*/
       {                                                                /*910*/
        /*****************************************************************910*/
        /* - If it's a button down event then set the cursor at the       910*/
        /*   end of the string or on the character that was clicked on.   910*/
        /*****************************************************************910*/
        if( Event->Value == EVENT_BUTTON_1_DOWN)                        /*910*/
        {                                                               /*910*/
          BufLen = strlen( Buffer );                                    /*910*/
          if( (DisplayOffset + (Event->Col - FieldCol)) > BufLen )      /*910*/
          {                                                             /*910*/
            keybeep();                                                  /*910*/
            CursorCol = FieldCol + (BufLen - DisplayOffset);            /*910*/
          }                                                             /*910*/
          else                                                          /*910*/
            CursorCol = (uint)Event->Col;                               /*910*/
        }                                                               /*910*/
        /*****************************************************************910*/
        /* - Ignore events between the brackets that are not button       910*/
        /*   down events.                                                 910*/
        /* - Any mouse event between []s turns off the erasure of a       910*/
        /*   cursor sensitive prompt.                                     910*/
        /*****************************************************************910*/
        FieldType &= ~CLEAR1ST;                                         /*910*/
        continue;                                                       /*910*/
       }                                                                /*910*/
                                                                        /*910*/
       /**********************************************************************/
       /* - handle strings that are not in the context of a popup.           */
       /**********************************************************************/
       if(pShell == NULL )                                              /*910*/
       {                                                                /*910*/
        NextState = GetMouseState( Event );                             /*910*/
        if( (state == MOU_STATE_UP) &&                                  /*910*/
            (NextState == MOU_STATE_DOWN)                               /*910*/
          )                                                             /*910*/
        {                                                               /*910*/
         key = LEFTMOUSECLICK;                                          /*910*/
         break;                                                         /*910*/
        }                                                               /*910*/
                                                                        /*910*/
        state = NextState;                                              /*910*/
        continue;                                                       /*910*/
       }                                                                /*910*/
       /******************************************************************910*/
       /* - Now, handle mouse events outside the []s within the context   910*/
       /*   of a popup.                                                   910*/
       /*                                                                 910*/
       /* - Button events are valid on the release event.                 910*/
       /*                                                                 910*/
       /* - Transitions from up to down "outside" the <>s and []s         910*/
       /*   are returned to the caller as a LEFTMOUSECLICK.               910*/
       /*                                                                 910*/
       /*  ------------------------                                       910*/
       /* |                        |                                      910*/
       /* |                       -----------                             910*/
       /* |         <------------|           |<----------------           910*/
       /* |        |              -----------                  |          910*/
       /* |      ----                                        ----         910*/
       /* |     |    |---                                ---|    |---     910*/
       /* |     | UP |   | BU,BUM                BD,BDM |   | DN |   |    910*/
       /* |     |    |<--                                -->|    |<--     910*/
       /* |      ----                                        ----         910*/
       /* |        |                                           |          910*/
       /* |        |                                           |          910*/
       /* |        |              -----------                  |          910*/
       /* |         ------------>|           |---------------->           910*/
       /* |                       -----------                             910*/
       /* |                               |                               910*/
       /* |                               |                               910*/
       /* | BU  - Button up event.        |                               910*/
       /* | BUM - Button up move event.    -(1) If BD or BDM event        910*/
       /* | BD  - Button down event.            occurs in a button(<>s),  910*/
       /* | BDM - Button down move event.       then set pButtonDown      910*/
       /* |                                     to point to the BUTTON    910*/
       /* |                                     structure. This is        910*/
       /* |                                     effectively a "pending"   910*/
       /*  ----(1) If BU or BUM event           button event. If the      910*/
       /*          && if the event occurs       up event occurs within    910*/
       /*          in the same button as        the same <>s, then we've  910*/
       /*          a "pending" button           got ourselves a valid     910*/
       /*          then we have a button        button event.             910*/
       /*          event.                                                 910*/
       /*                                   (2) If BD or BDM do not       910*/
       /*                                       occur within a button,    910*/
       /*                                       then set pButtonDown=NULL 910*/
       /*                                       and go back to the        910*/
       /*                                       caller with a             910*/
       /*                                       LEFTMOUSECLICK.           910*/
       /*                                                                 910*/
       /******************************************************************910*/
       NextState = GetMouseState( Event );                              /*910*/
       if( (state == MOU_STATE_DOWN) &&                                 /*910*/
           (NextState == MOU_STATE_UP) &&                               /*910*/
           (pButtonDown != NULL) &&                                     /*910*/
           (pButtonDown == GetButtonPtr(pShell,Event))                  /*910*/
         )                                                              /*910*/
       {                                                                /*910*/
        key = pButtonDown->Key;                                         /*910*/
        break;                                                          /*910*/
       }                                                                /*910*/
                                                                        /*910*/
       if( (state == MOU_STATE_UP) &&                                   /*910*/
           (NextState == MOU_STATE_DOWN)                                /*910*/
         )                                                              /*910*/
       {                                                                /*910*/
        pButtonDown = GetButtonPtr( pShell,Event);                      /*910*/
        if( pButtonDown == NULL )                                       /*910*/
        {                                                               /*910*/
         key = LEFTMOUSECLICK;                                          /*910*/
         break;                                                         /*910*/
        }                                                               /*910*/
       }                                                                /*910*/
                                                                        /*910*/
       state = NextState;                                               /*910*/
       continue;                                                        /*910*/
      }

      case TYPE_KBD_EVENT:
      {
        key = Event->Value;
        break;
      }
    }

    if( (FieldType & CLEAR1ST) && (FirstKeyEntry == TRUE) )             /*910*/
     FirstKeyEntry = FALSE;                                             /*910*/

    switch( key )
    {
      case F1:
      case ESC:
      case ENTER:
      case UP:
      case DOWN:
      case A_ENTER:
      case MOUSECLICK:
      /***********************************************************************/
      /* - these keys are specific to the watchpoint dialog.              910*/
      /***********************************************************************/
      case TYNEXT:                      /* watchpoint type button.        910*/
      case SPNEXT:                      /* watchpoint scope button.       910*/
      case SZNEXT:                      /* watchpoint size button.        910*/
      case STNEXT:                      /* watchpoint status button.      910*/
      {
        /*********************************************************************/
        /* All the above keys cannot be proccessed by getstring, so return   */
        /* the key to the caller.                                            */
        /*********************************************************************/
        rc = RCBREAK;
        break;
      }

      case C_HOME:
      {
        /*********************************************************************/
        /* Control-Home takes you to the start of the string.                */
        /*********************************************************************/
        DisplayOffset = 0;
        CursorCol = FieldCol;
        rc = RCREDRAW;
        break;
      }

      case C_END:
      {
        /*********************************************************************/
        /* Control-End takes you to the end of the string (to the last char  */
        /* the user has types in).                                           */
        /*********************************************************************/
        BufLen = strlen( Buffer );
        if( BufLen > FieldDisplayLength )
        {
          DisplayOffset = BufLen - FieldDisplayLength;
          CursorCol = FieldCol + FieldDisplayLength - 1;
        }
        else
        {
          DisplayOffset = 0;
          CursorCol = FieldCol + BufLen;
        }
        rc = RCREDRAW;
        break;
      }

      case TAB:
      case S_TAB:
      {
        /*********************************************************************/
        /* If the user has keyed in a tab  and the field type is AUTOEXIT,   */
        /* return to the caller.                                             */
        /*********************************************************************/
        if( FieldType & AUTOEXIT )
          rc = RCBREAK;
        else
        {
          keybeep();
          rc = 0;
        }
        break;
      }

      case LEFT:
      {
        /*********************************************************************/
        /* The user has pressed the left arrow key.                          */
        /*  - If the cursor is not in the starting column move the cursor    */
        /*    one column to the left.                                        */
        /*********************************************************************/
        if( CursorCol > FieldCol )
        {
          CursorCol--;
          rc = 0;
        }
        else
        {
          /*******************************************************************/
          /* If the cursor is in the starting column, a non zero value in the*/
          /* display offset would indicate we have some characters to the    */
          /* left to be displayed (see diagram above). If you have enough    */
          /* characters to scroll to LeftScrollOffset amount, do so. If not  */
          /* scroll to the start of the string.                              */
          /*******************************************************************/
          if( DisplayOffset )
          {
            if( (int)(DisplayOffset - LeftScrollOffset) >= 0 )
            {
              DisplayOffset -= LeftScrollOffset;
              CursorCol += (LeftScrollOffset - 1);
            }
            else
            {
              CursorCol += DisplayOffset;
              DisplayOffset = 0;
            }
            rc = RCREDRAW;
          }
          else
          {
            if( FieldType & AUTOEXIT )
              rc = RCBREAK;
            else
            {
               keybeep();
               rc = 0;
            }
          }
        }
        break;
      }

      case RIGHT:
      {
        /*********************************************************************/
        /* The user has pressed the right arrow key.                         */
        /*  - If the cursor is not in the ending column of display and not   */
        /*    end of the string, move the cursor one column to the right.    */
        /*********************************************************************/
        BufLen = strlen( Buffer );
        if( CursorCol < ( FieldCol + FieldDisplayLength - 1 ) )
        {
          if( (DisplayOffset + (CursorCol - FieldCol)) < BufLen )
          {
            CursorCol++;
            rc = 0;
          }
          else
          {
            keybeep();
            rc = 0;
          }
        }
        else
        {
          /*******************************************************************/
          /* If the cursor is at the ending column of display, check to see  */
          /* if we are within the buffer length. If we have enough space to  */
          /* scroll by RightScrollOffset amount do so, if not scroll till the*/
          /* end of the string.                                              */
          /*******************************************************************/
          if( DisplayOffset + FieldDisplayLength < BufLen )
          {
            DisplayOffset += (FieldDisplayLength - RightScrollOffset);
            if( (DisplayOffset + (CursorCol - FieldCol)) > BufLen )
              CursorCol = FieldCol + (BufLen - DisplayOffset);
            else
              CursorCol = FieldCol + RightScrollOffset;
            rc = RCREDRAW;
          }
          else
          {
            /*****************************************************************/
            /* If we reached the ending column and the end of the buffer, if */
            /* the field type is AUTOEXIT return to the caller.              */
            /*****************************************************************/
            if( FieldType & AUTOEXIT )
              rc = RCBREAK;
            else
            {
              keybeep();
              rc = 0;
            }
          }
        }
        break;
      }

      case HOME:
      {
        /*********************************************************************/
        /* Home takes to the starting column of the string.                  */
        /*********************************************************************/
        CursorCol = FieldCol;
        break;
      }

      case END:
      {
        /*********************************************************************/
        /* End takes you to the ending column of the string. If the string   */
        /* is not long enough till the ending column of the display, End     */
        /* takes you to the end of the string.                               */
        /*********************************************************************/
        CursorCol = FieldCol + FieldDisplayLength - 1;
        scratch = DisplayOffset + ( CursorCol - FieldCol );
        BufLen = strlen( Buffer );
        if( scratch > BufLen )
          CursorCol = FieldCol + (BufLen - DisplayOffset);
        break;
      }

      case INS:
      {
        /*********************************************************************/
        /* Insert key is pressed. This toggles the InsertMode flag. If the   */
        /* current length of the string is less than the buffer length,      */
        /* InsertMode could be allowed. Set the cusror type accordingly.     */
        /*********************************************************************/
        if( IsInsertMode )
        {
          IsInsertMode = 0;
          VioSetCurType( &NormalCursor, 0 );
        }
        else
        {
          if( length != strlen( Buffer ) )
          {
            IsInsertMode = 1 - IsInsertMode;
            IsInsertMode ? VioSetCurType( &InsertCursor, 0 ) :
                           VioSetCurType( &NormalCursor, 0 );
          }
          else                           /* Display proper error message...   */
            keybeep();
        }
        break;
      }

      case DEL:
      {
        /*********************************************************************/
        /* Delete the current character. The rest of the string is shifted   */
        /* left.                                                             */
        /*********************************************************************/
        scratch = DisplayOffset + ( CursorCol - FieldCol );
        ( scratch < strlen( Buffer ) ) ? ShiftLeft( scratch ) :
                                       keybeep();
        rc = RCREDRAW;
        break;
      }

      case BACKSPACE:
      {
        /*********************************************************************/
        /* The previous character to the cursor is deleted. By default the   */
        /* string is shifted left by one character. But if the cursor is on  */
        /* the starting column then the string is shifted by LeftScrollOffset*/
        /* amount.                                                           */
        /*********************************************************************/
        scratch = DisplayOffset + (CursorCol - FieldCol);
        if( scratch )
        {
          ShiftLeft( scratch - 1 );
          CursorCol--;
          if( CursorCol < FieldCol )
          {
            if( (int)(DisplayOffset - LeftScrollOffset) >= 0 )
            {
              DisplayOffset -= LeftScrollOffset;
              CursorCol += LeftScrollOffset;
            }
            else
            {
              CursorCol += (DisplayOffset + 1);
              DisplayOffset = 0;
            }
          }
        }
        else
          keybeep();
        rc = RCREDRAW;
        break;
      }

      default:
      {
        /*********************************************************************/
        /* Any other character key is pressed. Verify the validity of the    */
        /* character depending on the type of the string (HEX/BIN).          */
        /*********************************************************************/
        key &= 0xFF;
        if( key != 0x15 )
        if( (key < 0x20) || (key > 0x7E) )
        {
          rc = RCBREAK;
          break;
        }

        if( FieldType & HEXONLY )
        {
          if( (key < '0') || (key > '9') )
          {
            key &= 0xDF;
            if( (key < 'A') || (key > 'F') )
            {
              keybeep();
              rc = 0;
              break;
            }
          }
        }

        if( FieldType & BINONLY )
        {
          if( (key != '0') && (key != '1') )
          {
            keybeep();
            rc = 0;
            break;
          }
        }


        if( FieldType & CLEAR1ST )                                      /*910*/
        {                                                               /*910*/
         memset(Buffer,'\0',strlen(Buffer) );                           /*910*/
         *cursor = 0;                                                   /*910*/
         CursorCol = (*cursor > displen)?FieldCol : FieldCol + *cursor; /*910*/
        }                                                               /*910*/

        /*********************************************************************/
        /* Scratch gives you the exact number of characters so far typed in  */
        /* by the user. Accept the last character only if scratch is less    */
        /* than the total length of the string.                              */
        /*********************************************************************/
        scratch = DisplayOffset + ( CursorCol - FieldCol );
        if( scratch < length )
        {
          if( IsInsertMode )
          {
            if( (strlen( Buffer ) + 1) > length )
            {
              keybeep();
              rc = 0;
              break;      /* reject */
            }
            else
               ShiftRight( scratch );
          }

          Buffer[scratch] = ( uchar )key;

          /*******************************************************************/
          /* Adjust the cursor position accordingly so that it stays ahead   */
          /* of the last character.                                          */
          /*******************************************************************/
          if( CursorCol >= (FieldCol + FieldDisplayLength) )
          {
            DisplayOffset += (FieldDisplayLength - RightScrollOffset - 1);
            CursorCol = FieldCol + RightScrollOffset + 2;
          }
          else
            CursorCol++;

          /*******************************************************************/
          /* It the type of the field is AUTOEXIT return to the caller once  */
          /* the buffer is full.                                             */
          /*******************************************************************/
          BufLen = strlen( Buffer );
          if( (BufLen == length) &&
              (FieldType & AUTOEXIT) &&
              ((CursorCol - FieldCol) == FieldDisplayLength) )
            rc = RCREDRAW + RCBREAK + RCDATAXT;
          else
            rc = RCREDRAW;
          break;
        }
        else
        {
          keybeep();
          rc = 0;
          break;
        }
      }
    }

    /*************************************************************************/
    /* - Turn off the clear field flag after any event.                      */
    /*************************************************************************/
    FieldType &= ~CLEAR1ST;                                             /*910*/

   if( rc & RCREDRAW )
   {
     DisplayField( DisplayOffset );
     VioShowBuf( (ushort)voff, (ushort)( 2 * FieldDisplayLength ), 0 );
   }
   if( rc & RCBREAK )
     break;
  }                                     /* end of for loop to handle keystrks*/
    VioSetCurType( &HiddenCursor, 0 );

    /*************************************************************************/
    /* - Copy the local buffer to the caller supplied buffer.                */
    /* - Free the local buffer.                                              */
    /* - Reset the keyboard buffer flush flag.                               */
    /*************************************************************************/
    for( n = 0; n < strlen( Buffer ); n++ )
        *pInbuf++ = Buffer[n];

    *pInbuf = 0;
    *cursor = CursorCol;
    Tfree( Buffer );
    ResetFlushBufferFlag();                                             /*820*/
    return( (rc & RCDATAXT) ? DATAKEY : key );
}

void ShiftRight( uint offset )
{
  int   i, BufLen;

  BufLen = strlen( Buffer );

  for( i = BufLen; i > 0 && i > offset; i-- )
    Buffer[i] = Buffer[i-1];

  Buffer[i] = ' ';
}

void ShiftLeft( uint offset )
{
  int   i, BufLen;

  BufLen = strlen( Buffer );

  for( i = offset; i < BufLen; i++ )
    Buffer[i] = Buffer[i+1];

  Buffer[i] = '\0';
}

void DisplayField( uint offset )
{
  int    i, j;
  uchar *bp;

  bp = Buffer + offset;

  for( i = j = 0; j < FieldDisplayLength; i += 2, j++ )
    FieldBase[i] = (*bp) ? *bp++ : '\0';
}

void keybeep()
{
    DosBeep( 1000/*Hz*/, 1/*Milliseconds*/ );
}
/*****************************************************************************/
/* GetMouseState()                                                           */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Get the next state of mouse button 1.                                   */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   pEvent         -> the new event.                                        */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   NextState                                                               */
/*                                                                           */
/*****************************************************************************/
int GetMouseState( PEVENT pEvent )
{
 int NextState;

 switch(pEvent->Value)
 {
  case EVENT_NO_BUTTONS_DOWN:
  case EVENT_NO_BUTTONS_DOWN_MOVE:
   NextState = MOU_STATE_UP;
   break;

  case EVENT_BUTTON_1_DOWN:
  case EVENT_BUTTON_1_DOWN_MOVE:
   NextState = MOU_STATE_DOWN;
   break;

  /***************************************************************************/
  /* - any other events don't change the state.                              */
  /***************************************************************************/
  default:
   break;
 }
 return(NextState);
}
