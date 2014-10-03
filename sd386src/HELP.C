/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   help.c                                                                  */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   various routines for displaying help and messages.                      */
/*                                                                           */
/*                                                                           */
/* History:                                                                  */
/*                                                                           */
/*   02/08/91 Creation of 32-bit SD86, from 16-bit version.                  */
/*                                                                           */
/*...16->32 port.                                                            */
/*...                                                                        */
/*... 02/08/91  100   Philip    port to 32 bit.                              */
/*... 02/08/91  112   Joe       port to 32 bit.                              */
/*...                                                                        */
/*...Release 1.00 (Pre-release 105 10/10/91)                                 */
/*...                                                                        */
/*... 10/31/91  308   Srinivas  Menu for exceptions.                         */
/*...                                                                        */
/*...Release 1.00 (Pre-release 107 11/13/91)                                 */
/*...                                                                        */
/*... 11/13/91  400   Srinivas  Vertical Register Display.                   */
/*... 11/18/91  401   Srinivas  Co processor Register Display.               */
/*...                                                                        */
/*...Release 1.00 (After pre-release 1.08)                                   */
/*...                                                                        */
/*... 02/12/92  521   Joe       Port to C-Set/2.                             */
/*...                                                                        */
/*...Release 1.00 (03/03/92)                                                 */
/*...                                                                        */
/*... 03/10/92  602   Srinivas  Hooking up watch points.                     */
/*...                                                                        */
/*...Release 1.01 (04/03/92)                                                 */
/*...                                                                        */
/*... 05/08/92  701   Srinivas  Cua Interface.                               */
/**Includes*******************************************************************/

#include "all.h"                        /* SD86 include files                */
#include "diahlpbx.h"

/**External declararions******************************************************/

extern uint     VideoRows;
extern uint     VideoCols;
extern uchar    VideoAtr;
extern uchar    hilite[];               /* high light field attributes    701*/
extern uchar    normal[];               /* normal field attributes        701*/
extern uchar    ClearField[];           /* attrib str for clearing fields.   */
extern CmdParms cmd;
extern UINT     HelpRow;                /* screen row for menu help (0..N)   */
extern UINT     ProRow;                 /* screen row for prompt (0..N)      */
extern UINT     MsgRow;                 /* screen row for messages (0..N)    */

/**Static definitions ********************************************************/

static  uchar *HelpMessage;

/*****************************************************************************/
/* Help()                                                                    */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Display help window, wait for key press, restore window.                */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   id            input - help file message id.                             */
/*   fillchars     input - optional fill in the blank chars.                 */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   key           user reponse from the help box.                           */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   none.                                                                   */
/*                                                                           */
/*****************************************************************************/

 void
Help(ULONG id)
{
    uchar *mbuf;

    mbuf = GetHelpMsg(id, NULL,0);
    VideoAtr = vaMenuBar;
    CuaShowHelpBox( mbuf );
    Tfree( mbuf);                                                        /*521*/
}

    uint
ShowHelpBox(uchar *helptext, UINTFUNC helpfunc,uint HeaderRows,void *args)
{
    uint  row, rows, cols;
    uchar *cp, *msg;
    uint  n, msglen, ans;
    uchar *vbuf;
    int   StartRow;
    uint  TotalRows, TotalCols;
    uint  ScreenRows;
    uchar *SavePtr;
    uint   col = 0;                     /* was not initialized.           512*/

    n = strlen(cp = helptext);
/*****************************************************************************/
/*  Calculate the number of rows and colums required to display the window308*/
/*  by scanning the text buffer for control characters.                   308*/
/*****************************************************************************/
    for( rows = cols = 0 ; n ; ++rows, n -= col, cp += col )
    {
        if( rows == 0 )
            msg = cp;
        if( (col = strcspn(cp, "\r")) > cols )
            cols = col;
        if( (col == 0) && (rows == 0) )
            rows -= 1;
        if( col < n )
            cp[col++] = 0;
        if( (col < n) && (cp[col] == '\n') )
            col += 1;
    }

/*****************************************************************************/
/* Calculate the following values:                                        308*/
/*                                                                           */
/*   StartRow   : Index into the text buffer from where to display the stuff.*/
/*   TotalRows  : Total rows required including the border rows.             */
/*   TotalCols  : Total cols required including the border Cols.             */
/*   ScreenRows : Total number of rows available for display the entries     */
/*                excluding the header lines and border lines.               */
/*****************************************************************************/
    StartRow   = 1;
    TotalRows  = rows + 2;
    TotalCols  = cols + 4;
    ScreenRows = VideoRows - 2 - HeaderRows;

/*****************************************************************************/
/*   If the total rows cannot fit on the screen then put the max possible.308*/
/*****************************************************************************/
    if ( (TotalRows ) > VideoRows )
       TotalRows = VideoRows;

/*****************************************************************************/
/*  Calculate the screen co-ordinates where to start the window.          308*/
/*****************************************************************************/
    row = (VideoRows - TotalRows) >> 1;
    col = (VideoCols - TotalCols) >> 1;

    vbuf = Talloc(2*TotalRows*TotalCols);                               /*521*/
    Vgetbox( vbuf, row, col,TotalRows, TotalCols );

    SavePtr = msg;

    Vfmtbox( "", row, col, TotalRows, TotalCols );

Reformat:

    msg = SavePtr;


/*****************************************************************************/
/* Display the header rows of the window.                                 308*/
/*****************************************************************************/
    for( n = 0 ; ++n <= HeaderRows ; )
    {
        msglen = strlen(msg);
        if( *msg == '\t' )
            putrc( row + n, col+2 + ((TotalCols-3 - msglen) >> 1), msg + 1 );
        else
            putrc( row + n, col+2, msg );
        if( *(msg += msglen + 1) == '\n' )
            msg += 1;
    }


/*****************************************************************************/
/* Skip the rows in the text buffer which are not to be displayed.        308*/
/*****************************************************************************/
    for( n = 1 ; n < StartRow ; n++)
    {
        msglen = strlen(msg);
        if( *(msg += msglen + 1) == '\n' )
            msg += 1;
    }


/*****************************************************************************/
/* Display the rows in the window from the desired postion in the text buff. */
/*****************************************************************************/
    for( n=HeaderRows ; ++n < TotalRows - 1 ; )
    {
        msglen = strlen(msg);
        if( *msg == '\t' )
            putrc( row + n, col+2 + ((TotalCols-3 - msglen) >> 1), msg + 1 );
        else
            putrc( row + n, col+2, msg );
        if( *(msg += msglen + 1) == '\n' )
            msg += 1;
    }

    HideCursor();


/*****************************************************************************/
/* Call the menu box function.                                            308*/
/*****************************************************************************/
    ans =  ( * helpfunc ) (row+1, col+1, TotalRows-2, TotalCols-2, args);


/*****************************************************************************/
/* If HeaderRows is zero it indicates a help message so donot do any process */
/* on the feed back keys.                                                 308*/
/*****************************************************************************/
    if (HeaderRows)
    {
         switch(ans)
         {
         /********************************************************************/
         /* Depending on the keys set the startrow value. Ensure that the 308*/
         /* screen is displayed with full rows ie on PgUp and PGDn anchor    */
         /* the last line to the bottom of the screen. Then go to Reformat   */
         /* to display the new rows in the window.                           */
         /********************************************************************/
           case UP:
             StartRow--;
             StartRow = (StartRow < 1) ? 1 : StartRow;
             goto Reformat;
           case DOWN:
             StartRow++;
             StartRow = min(StartRow,(int)(rows-HeaderRows-ScreenRows+1));
             StartRow = (StartRow < 1) ? 1 : StartRow;
             goto Reformat;
           case PGUP:
             StartRow = StartRow - ScreenRows;
             StartRow = (StartRow < 1) ? 1 : StartRow;
             goto Reformat;
           case PGDN:
             StartRow = StartRow + ScreenRows;
             StartRow = min(StartRow,(int)(rows-HeaderRows-ScreenRows+1));
             StartRow = (StartRow < 1) ? 1 : StartRow;
             goto Reformat;
           default:
             break;
         }
    }

    Vputbox( vbuf, row, col, TotalRows, TotalCols );
    Tfree( vbuf );                                                       /*521*/
    return( ans );
}

 void
fmterr(uchar *msg)
{
  char SaveVideoAtr;                    /* holds the current video attr   701*/
  SaveVideoAtr = VideoAtr;              /* save the current video attr    701*/
  ClrScr( MsgRow, MsgRow, vaError );
  if( msg ) putrc( MsgRow, 0, msg );
  VideoAtr = SaveVideoAtr;              /* Restore saved video attribute  701*/
}

void
fmt2err(uchar *msg1,uchar *msg2)
{
    fmterr( msg1 );
    if( msg1 && msg2 ) putrc( MsgRow, strlen(msg1) + 1, msg2 );
}

void
putmsg(uchar *msg)
{
    ClrScr( MsgRow, MsgRow, vaError );
    if( msg ) putrc( MsgRow, 0, msg );
}

 void
SayMsgBox1(uchar *msg )
{
    uint row, rows, col, cols;

    rows = 1;
    cols = strlen(msg);

    row = (VideoRows - (rows += 2)) >> 1;
    col = (VideoCols - (cols += 4)) >> 1;

    VideoAtr = vaInfo;
    Vfmtbox( "", row, col, rows, cols );
    putrc( row+1, col+2, msg );
}

 void
SayMsgBox2( uchar *msg, ulong delay )
{
    uchar *vbuf, vacopy = VideoAtr;
    uint cols=strlen(msg), rows=1, col, row;   /* was register.           112*/

    HideCursor();
    row = (VideoRows - (rows += 2)) >> 1;
    col = (VideoCols - (cols += 4)) >> 1;
    vbuf = Talloc(2*rows*cols);                                         /*521*/
    Vgetbox( vbuf, row, col, rows, cols );

    VideoAtr = vaInfo;
    Vfmtbox( "", row, col, rows, cols );
    putrc( row+1, col+2, msg );

    DosSleep(delay);

    Vputbox( vbuf, row, col, rows, cols );
    VideoAtr = vacopy;
    Tfree( vbuf );                                                       /*521*/
}

/*****************************************************************************/
/*  This function is same as saymsgbox1 except that it shifts the box to  401*/
/*  the left by 2 columns to avoid the over lap with the register windows,401*/
/*  its used only by the restart function                                 401*/
/*****************************************************************************/
 void
SayMsgBox3(uchar *msg )
{
    uint row, rows, col, cols;

    rows = 1;
    cols = strlen(msg);

    row = (VideoRows - (rows += 2)) >> 1;
    col = (VideoCols - (cols += 4) - 4 ) >> 1;

    VideoAtr = vaInfo;
    Vfmtbox( "", row, col, rows, cols );
/*msh  putrcx( row+1, col+2, msg , 0); */
    putrcx( row+1, col+2, msg);
}

uchar *SayMsgBox4( uchar *msg )
{
    uchar *buf, vacopy = VideoAtr;
    uint  cols = strlen( msg ), rows = 1, col, row;  /* was register.     112*/

    HideCursor();
    row = (VideoRows - (rows += 2)) >> 1;
    col = (VideoCols - (cols += 4)) >> 1;
    buf = Talloc(2*rows*cols);                                         /*521*/
    Vgetbox( buf, row, col, rows, cols );

    VideoAtr = vaInfo;
    Vfmtbox( "", row, col, rows, cols );
    putrc( row + 1, col + 2, msg );

    VideoAtr = vacopy;
    return( buf );
}

void RemoveMsgBox( uchar *msg, uchar *buf )
{
    uchar vacopy = VideoAtr;
    uint  cols = strlen( msg ), rows = 1, col, row;  /* was register.     112*/

    HideCursor();
    row = (VideoRows - (rows += 2)) >> 1;
    col = (VideoCols - (cols += 4)) >> 1;

    VideoAtr = vaInfo;

    Vputbox( buf, row, col, rows, cols );
    VideoAtr = vacopy;
}


#define BUTTONROW  (StartRow + TotalRows - 2)
#define BUTTONCOL  (StartCol + (TotalCols / 2) - 3)
#define BUTTONLEN  6

/*****************************************************************************/
/* CuaShowHelpBox()                                                       701*/
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Displays a given text in a dialog box.                                  */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   HelpText  -> pointer to a buffer containing the text to be displayed.   */
/*                                                                           */
/* Return:                                                                   */
/*          None                                                             */
/*                                                                           */
/*****************************************************************************/
void  CuaShowHelpBox( uchar *HelpText )
{
  uint   RowCount, ColCount;
  uint   TotalRows, TotalCols;
  uint   n = 0, MsgLen,  ScrollBar = FALSE;
  uchar  *Buffer;

  HelpMessage = HelpText;

  MsgLen = strlen( Buffer = HelpText );

  /***************************************************************************/
  /* Count the number of max rows and columns needed to display the text.    */
  /***************************************************************************/
  for( RowCount = ColCount = 0; MsgLen; ++RowCount, MsgLen -= n, Buffer += n )
  {
      if( (n = strcspn( Buffer, "\r" )) > ColCount )
          ColCount = n;
      if( (n == 0) && (RowCount == 0) )
          RowCount -= 1;
      if( n < MsgLen )
          Buffer[ n++ ] = 0;
      if( (n < MsgLen) && (Buffer[ n ] == '\n') )
          n += 1;
  }

  TotalRows = RowCount + 6;
  TotalCols = ColCount + 6;

  TotalCols = (TotalCols < VideoCols) ? TotalCols : VideoCols;

  /***************************************************************************/
  /* Set the various attributes of the dialog box depending on the max rows  */
  /* and columns.                                                            */
  /***************************************************************************/
  Dia_HelpBox_Choices.entries = RowCount;
  if( TotalRows > VideoRows )
  {
    Dia_HelpBox_Choices.MaxRows = VideoRows - 10;
    Dia_HelpBox.length = VideoRows - 4;
    ScrollBar = TRUE;
  }
  else
  {
    Dia_HelpBox_Choices.MaxRows = RowCount;
    Dia_HelpBox.length = TotalRows;
  }

  Dia_HelpBox.width = TotalCols;

  Dia_HelpBox.col    = (VideoCols - TotalCols) >> 1;
  Dia_HelpBox.row    = (VideoRows - Dia_HelpBox.length) >> 1;
  Dia_HelpBox.Buttons[0].row = Dia_HelpBox.row +
                               Dia_HelpBox_Choices.MaxRows + 4;    /*701*/
  Dia_HelpBox.Buttons[0].col = ( Dia_HelpBox.width -
                                 Dia_HelpBox.Buttons[0].length ) / 2 +
                               Dia_HelpBox.col;

  ClearField[1] = (uchar)Dia_HelpBox.width;
  DisplayDialog( &Dia_HelpBox, ScrollBar );
  ProcessDialog( &Dia_HelpBox, &Dia_HelpBox_Choices, ScrollBar, NULL );
  RemoveDialog( &Dia_HelpBox );
}

/*****************************************************************************/
/* DisplayHelpBoxText()                                                   701*/
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Displays the help text.                                                 */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   shell   ->  pointer to a dialog shell structure.                        */
/*   ptr     ->  pointer to a dialog choice structure.                       */
/*                                                                           */
/* Return:                                                                   */
/*          None                                                             */
/*                                                                           */
/*****************************************************************************/
void  DisplayHelpBoxText( DIALOGSHELL *shell, DIALOGCHOICE *ptr )
{
  int    i, idx, MsgLen;
  uchar  *message;

  message = HelpMessage;
  for( i = 0; i <= ptr->SkipRows; i++ )
  {
    MsgLen = strlen( message );
    if( *(message += MsgLen + 1) == '\n' )
      message += 1;
  }

  ClearField[1] = shell->width - 5;
  for( i = 0, idx = ptr->SkipRows; i < ptr->MaxRows; idx++, i++ )
  {
    putrcx( shell->row + i + shell->SkipLines, shell->col + 2, ClearField);
    MsgLen = strlen( message );

    if( *message == '\t' )
        putrcx( shell->row + i + shell->SkipLines,
                shell->col + 2 + ((shell->width - 3 - MsgLen) >> 1),
                message + 1 );
    else
        putrcx( shell->row + i + shell->SkipLines, shell->col + 2, message);

    if( *(message += MsgLen + 1) == '\n' )
      message += 1;
  }
}

/*****************************************************************************/
/* HelpBoxDialogFunction()                                                701*/
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
uint  HelpBoxDialogFunction( DIALOGSHELL *shell, DIALOGCHOICE *ptr,
                             EVENT *nEvent, void *ParamBlock )
{
  static uchar  Save;

  switch( nEvent->Value )
  {
    case INIT_DIALOG:
    {
      Save = hilite[1];
      hilite[1] = normal[1] = 0;
      return( 0 );
    }

    case ENTER:
    case ESC:
    {
      hilite[1] = normal[1] = Save;
      return( ESC );
    }

    default:
      return( 0 );
  }
}

/*****************************************************************************/
/* Error()                                                                   */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Display a fatal message and then exit the debugger.                     */
/*                                                                           */
/*   This function allows for a substitution string and a substitution       */
/*   number to be tossed into the %1 and %2 fields of the message            */
/*   defined by the message id.                                              */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   MsgId          id of the message to display.                            */
/*   Fatal          TRUE=>kill the debugger. FALSE=>informational.           */
/*   SubStringCount String that will go into %1,%2,... substitutions.        */
/*   ...            list of ptrs to strings                                  */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   Vio has been initialized.                                               */
/*                                                                           */
/*****************************************************************************/
#include <stdarg.h>
void _System Error(ULONG MsgId, int Fatal,int SubStringCount,...)
{
 UCHAR   *SubStringTable[MAX_SUBSTRINGS];
 int      i;
 va_list  pSubString;
 UCHAR   *pMsgBuf;
 CSR      DosCsr;

 va_start(pSubString,SubStringCount);
 for( i = 0; i < SubStringCount; i++ )
  SubStringTable[i] = va_arg(pSubString,char *);

 pMsgBuf = GetHelpMsg(MsgId, SubStringTable,SubStringCount);

 DosBeep( 1000/*Hz*/, 3/*Millisecs*/ );

 CuaShowHelpBox(pMsgBuf);
 Tfree( pMsgBuf );
 if( Fatal == FALSE )
  return;
 UnRegisterExceptionHandler( );
 ClrPhyScr( 0, VideoRows-1, vaClear );
 DosCsr.col = 0;
 DosCsr.row = ( uchar )(VideoRows-1);
 PutCsr( &DosCsr );
 exit(0);
}

/*****************************************************************************/
/* Message()                                                                 */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Display a fatal message and then exit the debugger.                     */
/*                                                                           */
/*   This function allows for a substitution string and a substitution       */
/*   number to be tossed into the %1 and %2 fields of the message            */
/*   defined by the message id.                                              */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   MsgId          id of the message to display.                            */
/*   Beep           TRUE=>scream about it. FALSE=>just a quiet bit of info.  */
/*   SubStringCount String that will go into %1,%2,... substitutions.        */
/*   ...            list of ptrs to strings                                  */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
void _System Message(ULONG MsgId, int Beep,int SubStringCount,...)
{
 UCHAR   *SubStringTable[MAX_SUBSTRINGS];
 int      i;
 va_list  pSubString;
 UCHAR   *pMsgBuf;

 va_start(pSubString,SubStringCount);
 for( i = 0; i < SubStringCount; i++ )
  SubStringTable[i] = va_arg(pSubString,char *);

 pMsgBuf = GetHelpMsg(MsgId, SubStringTable,SubStringCount);
 if(Beep)
  beep();
 SayStatusMsg(pMsgBuf);
 Tfree( pMsgBuf );
 return;
}

/*****************************************************************************/
/* SayStatusMsg()                                                            */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Display an informational message on the message line.                   */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   pMsg           -> to Z-String message.                                  */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*  Message is less than 80 bytes.                                           */
/*                                                                           */
/*                                                                           */
/*****************************************************************************/
void SayStatusMsg(UCHAR *msg)
{
 char  SaveVideoAtr;
 char *cp;

 /****************************************************************************/
 /* - The message may contain cr/lf characters. We need to replace them      */
 /*   with spaces.                                                           */
 /****************************************************************************/
 for( cp = msg; *cp ; cp++ )
 {
  if( (*cp == '\r') || (*cp == '\n') )
   *cp = ' ';
 }

 SaveVideoAtr = VideoAtr;
 ClrScr( MsgRow, MsgRow, vaError );
 if( msg ) putrc( MsgRow, 0, msg );
 VideoAtr = SaveVideoAtr;
}

/*****************************************************************************/
/* MsgYorN()                                                                 */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Display a prompt for a yes or no keyboard response.                     */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   MsgId          id of the message to display.                            */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   key                                                                     */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
UINT MsgYorN(ULONG MsgId)
{
 UCHAR   *pMsgBuf;
 uint     key;

 pMsgBuf = GetHelpMsg(MsgId, NULL,0 );
 key = SayMsgBox(pMsgBuf);
 Tfree( pMsgBuf );
 return(key);
}

/*****************************************************************************/
/* SayMsgBox()                                                               */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Display a prompt.                                                       */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   msg            ->to display message.                                    */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
UINT SayMsgBox(uchar *msg)
{
 UINT  row, rows, col, cols;
 char *cp=NULL;
 char *cpp;
 int   len;
 int   msglen = strlen(msg);
 int   numrows;
 int   n;
 char *vbuf;
 UINT  key;

 /****************************************************************************/
 /* - Count the rows and cols needed to display by tokenizing on \n.         */
 /****************************************************************************/
 numrows = 0;
 rows = 0;
 cols = 0;
 cp = strtok(msg,"\n");
 do
 {
  numrows++;
  len = 0;
  cpp = cp;
  while(*cpp++ != '\r') len++;
  cols = (cols>len)?cols:len;
  cp = strtok(NULL,"\n");
 }
 while( cp );

 /****************************************************************************/
 /* - tokenization left \0s where the \ns were. Replace \0s with spaces.     */
 /****************************************************************************/
 for( cp=msg;msglen;msglen--,cp++ )
  if( *cp == '\r')
   *cp = ' ';

 /****************************************************************************/
 /* - compute the top left origin of the display box and format.             */
 /****************************************************************************/
 rows = numrows;
 row = (VideoRows - (rows += 2)) >> 1;
 col = (VideoCols - (cols += 4)) >> 1;

 VideoAtr = vaInfo;
 vbuf = Talloc(2*rows*cols);
 Vgetbox( vbuf, row, col,rows, cols );
 Vfmtbox( "", row, col, rows, cols );
 /****************************************************************************/
 /* - write the message to the display.                                      */
 /****************************************************************************/
 for( cp = msg, n=row ; numrows ; numrows--,n++ )
 {
  putrc( n+1, col+2, cp );
  cp += strlen(cp) + 1;
 }

 {
  PEVENT pEvent;

  pEvent = GetEvent( SEM_INDEFINITE_WAIT );
  key = pEvent->Value;
  pEvent->Value = 0;
 }

 Vputbox( vbuf, row, col, rows, cols );
 Tfree( vbuf );
 return( key );
}

/*****************************************************************************/
/* SayMsgBox5()                                                              */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Display a prompt.                                                       */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   msg            ->to display message.                                    */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
void _System SayMsgBox5(ULONG MsgId, int SubStringCount,...)
{
 UCHAR   *SubStringTable[MAX_SUBSTRINGS];
 int      i;
 va_list  pSubString;
 UCHAR   *pMsgBuf;

 UINT  row, rows, col, cols;
 char *cp=NULL;
 char *cpp;
 int   len;
 int   msglen;
 int   numrows;
 int   n;
 char *vbuf;

 va_start(pSubString,SubStringCount);
 for( i = 0; i < SubStringCount; i++ )
  SubStringTable[i] = va_arg(pSubString,char *);

 pMsgBuf = GetHelpMsg(MsgId, SubStringTable,SubStringCount);

 msglen = strlen(pMsgBuf);

 /****************************************************************************/
 /* - Count the rows and cols needed to display by tokenizing on \n.         */
 /****************************************************************************/
 numrows = 0;
 rows = 0;
 cols = 0;
 cp = strtok(pMsgBuf,"\n");
 do
 {
  numrows++;
  len = 0;
  cpp = cp;
  while(*cpp++ != '\r') len++;
  cols = (cols>len)?cols:len;
  cp = strtok(NULL,"\n");
 }
 while( cp );

 /****************************************************************************/
 /* - tokenization left \0s where the \ns were. Replace \0s with spaces.     */
 /****************************************************************************/
 for( cp=pMsgBuf;msglen;msglen--,cp++ )
  if( *cp == '\r')
   *cp = ' ';

 /****************************************************************************/
 /* - compute the top left origin of the display box and format.             */
 /****************************************************************************/
 rows = numrows;
 row = (VideoRows - (rows += 2)) >> 1;
 col = (VideoCols - (cols += 4)) >> 1;

 VideoAtr = vaInfo;
 vbuf = Talloc(2*rows*cols);
 Vgetbox( vbuf, row, col,rows, cols );
 Vfmtbox( "", row, col, rows, cols );
 /****************************************************************************/
 /* - write the message to the display.                                      */
 /****************************************************************************/
 for( cp = pMsgBuf, n=row ; numrows ; numrows--,n++ )
 {
  putrc( n+1, col+2, cp );
  cp += strlen(cp) + 1;
 }
 DosSleep(1000);
 Vputbox( vbuf, row, col, rows, cols );
 Tfree( vbuf );
 return;
}

/*---------------------------------------------------------------------------*/
/* - Below here added for MSH support.                                       */
/*---------------------------------------------------------------------------*/
#ifdef MSH
void FmtErr(uchar *msg) {
  if(iview) {
      VioContext *vioContext=SaveVioContext(NULL);
      RestoreVioContext(&MainVioContext);
      fmterr(msg);
      RestoreVioContext(vioContext);
      Tfree(vioContext);
  }
  else
  {
      fmterr(msg);
  }/* End if*/
}
#endif
