/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   dialog.c                                                                */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Common Cua routines to paint, remove and process keys and mouse events  */
/* for pulldowns, popups and dialogs.                                        */
/*                                                                           */
/*...Release 1.01 (04/03/92)                                                 */
/*...                                                                        */
/*... 05/08/92  701   Srinivas  Cua Interface.                               */
/*... 07/20/92  701   Selwyn    Revision.                                    */
/*...                                                                        */
/*...Release 1.02 (10/22/92)                                                 */
/*...                                                                        */
/*... 02/11/93  812   Selwyn    Wrong cursor position in callstack dialog.   */
/*... 04/15/93  821   Selwyn    Changes in processing clicks on buttons.     */
/*... 12/09/93  909   Joe       Fix for not erasing pulldowns in 132x60 mode.*/
/*****************************************************************************/
#include "all.h"

extern uchar   * _Seg16 VideoPtr;       /* -> to logical video buffer        */
extern uchar    hiatt[];                /* attributes string for highlight.  */
extern uchar    VideoAtr;               /* default logical video attribute.  */
extern uchar    ScrollShade1[];         /* attribute string for scroll bar.  */
extern uchar    ScrollShade2[];         /* attribute string for scroll bar.  */
extern uchar    ClearField[];           /* attrib str for clearing fields.   */
extern uchar    hilite[];               /* attrib str to highlight fields.   */
extern uchar    badhilite[];            /* attrib str to highlight bad fields*/
extern uchar    normal[];               /* attrib str to disp normal fields. */
extern uchar    badnormal[];            /* attrib str to disp bad fields.    */
extern uchar    Shadow[];               /* attrib str for shadowing around   */
extern uchar    InScrollMode;           /* flag to tell we are in scrollmode */
extern VIOCURSORINFO  NormalCursor;     /* make underscore flashing cursor   */
extern KEY2FUNC defk2f[];               /* keys to functions map.            */
extern CmdParms cmd;
extern uint     VideoCols;              /* # of columns per screen           */

#define ON   1
#define OFF  0

static PEVENT Event;
static UINT   MouseState;

/*****************************************************************************/
/* Strings used to display a pulldown.                                       */
/*****************************************************************************/
static uchar barstr[]    =  {V_BAR,0};
static uchar UpArrow[]   =  {UPARROW,0};
static uchar DnArrow[]   =  {DNARROW,0};

/*****************************************************************************/
/* Strings used to display a pop up.                                         */
/*****************************************************************************/
static uchar toppop[]  =  {TL_CORNER,RepCnt(1),H_BAR,TR_CORNER,0};
static uchar botpop[]  =  {BL_CORNER,RepCnt(1),H_BAR,BR_CORNER,0};
static uchar poptll[]  =  {L_JUNCTION,RepCnt(1),H_BAR,0};
static uchar poptlr[]  =  {RepCnt(1),H_BAR,R_JUNCTION,0};

/*****************************************************************************/
/*  Windowsv()                                                               */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*     save area of screen about to be used by a pulldown or popup.          */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   int x           x coordinate of top left corner                         */
/*   int y           y coordinate of top left corner                         */
/*   int rows        # of rows in window                                     */
/*   int cols        # of columns in window                                  */
/*   char *ptr       -> to save area                                         */
/*                                                                           */
/* Return:                                                                   */
/*   None                                                                    */
/*****************************************************************************/
void windowsv(int x,int y,int rows,int cols,char *ptr)
{
  int i;

  for (i = 0 ; i < rows ; i++)
  {
    memcpy(ptr,VideoPtr+((VideoCols * (y + i)) + x) * 2,cols << 1);     /*909*/
    ptr += cols * 2;
  }
}

/*****************************************************************************/
/*  Windowrst()                                                              */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*     Restore screen area behind a pulldown or popup.                       */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   int x           x coordinate of top left corner                         */
/*   int y           y coordinate of top left corner                         */
/*   int rows        # of rows in window                                     */
/*   int cols        # of columns in window                                  */
/*   char *ptr       -> to save area                                         */
/*                                                                           */
/* Return:                                                                   */
/*   None                                                                    */
/*****************************************************************************/
void windowrst(int x,int y,int rows,int cols,char *ptr)
{
  int i;

  for (i = 0 ; i < rows ; i++)
  {
    memcpy(VideoPtr + ((VideoCols * (y + i)) + x) * 2,ptr,cols << 1);   /*909*/
    ptr += cols * 2;
  }
  VioShowBuf(VideoCols*2*y,VideoCols*2*rows,0);                         /*909*/
}

/*****************************************************************************/
/*  DisplayDialog()                                                          */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*     Displays a dialog window along with buttons and scroll bar.           */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   DIALOGSHELL *ptr     ->  pointer to a dialog shell structure            */
/*                                                                           */
/* Return:                                                                   */
/*   None                                                                    */
/*****************************************************************************/
void DisplayDialog(DIALOGSHELL *ptr, int ScrollBar )
{
  int     i;                            /* counter                           */
  int     tlen;                         /* title length                      */
  BUTTON *ButtonPtr;                    /* -> to button struture.            */

  /***************************************************************************/
  /* Get the Screen Access to write.                                         */
  /***************************************************************************/
  GetScrAccess();

  /***************************************************************************/
  /* Allocate the memory required to save the screen that will be covered by */
  /* Dialog and save the screen area which will be covered.                  */
  /***************************************************************************/
  ptr->SaveArea = Talloc(ptr->length * ptr->width * 2);
  windowsv(ptr->col,ptr->row,ptr->length,ptr->width,ptr->SaveArea);

  /***************************************************************************/
  /* - set the default video attribute.                                      */
  /* - initialize color attribute for the clear cell.                        */
  /***************************************************************************/
  VideoAtr     = vaMenuBar;
  ClearField[1] = (uchar)ptr->width;

  /***************************************************************************/
  /* clear the screen area covered by the popup.                             */
  /***************************************************************************/
  for (i=0;i<ptr->length;i++)
    putrc(ptr->row+i,ptr->col,ClearField);

  /***************************************************************************/
  /* Put the length into topline string and then display the top line.       */
  /***************************************************************************/
  toppop[2] = (uchar)(ptr->width - 2);
  putrc(ptr->row,ptr->col,toppop);

  /***************************************************************************/
  /* display the title line in reverse video.                                */
  /***************************************************************************/
  tlen = strlen(ptr->title);
  poptll[2] = (char) ((ptr->width - tlen - 2) >> 1);
  poptlr[1] = (char) (ptr->width - tlen - poptll[2] - 2);
  putrc(ptr->row+1,ptr->col,poptll);
  VideoAtr = vaMenuCsr;
  putrc(ptr->row+1,ptr->col + poptll[2] + 1,ptr->title);
  VideoAtr = vaMenuBar;
  putrc(ptr->row+1,ptr->col + poptll[2] + tlen + 1,poptlr);

  /***************************************************************************/
  /* draw the sides of pop up window.                                        */
  /***************************************************************************/
  for(i = 2 ; i < (ptr->length - 1) ; i++)
  {
    putrc(ptr->row+i,ptr->col,barstr);
    putrc(ptr->row+i,ptr->col+ptr->width-1,barstr);
  }

  /***************************************************************************/
  /* Put the scroll bar.                                                     */
  /***************************************************************************/
  if( ScrollBar )
  {
    putrc(ptr->row+UPARROWROWOFFSET,
          ptr->col+ptr->width-SCROLLBARCOLOFFSET,UpArrow);
    for(i = 3 ; i < (ptr->length - ptr->NoOfButtonRows-DNARROWROWOFFSET) ; i++)
      putrc(ptr->row+i,ptr->col+ptr->width-SCROLLBARCOLOFFSET,ScrollShade1);
    putrc(ptr->row+ptr->length-ptr->NoOfButtonRows-DNARROWROWOFFSET,
            ptr->col+ptr->width-SCROLLBARCOLOFFSET,DnArrow);
  }
  /***************************************************************************/
  /* Put length into botline string and then display the bottom line.        */
  /***************************************************************************/
  botpop[2] = (char) (ptr->width - 2);
  putrc(ptr->row+ptr->length - 1,ptr->col,botpop);

  /***************************************************************************/
  /* draw the buttons on the pop up.                                         */
  /***************************************************************************/
  ButtonPtr = ptr->Buttons;
  for (i = 0 ; i < ptr->NoOfButtons ; i++)
  {
    putrc(ButtonPtr->row,ButtonPtr->col-1,"<");
    putrc(ButtonPtr->row,ButtonPtr->col,ButtonPtr->Name);
    putrc(ButtonPtr->row,ButtonPtr->col+ButtonPtr->length,">");
    ButtonPtr++;
  }

  /***************************************************************************/
  /* Reset the video attributes to that of window.                           */
  /***************************************************************************/
  VideoAtr = vaMenuBar;
}

/*****************************************************************************/
/*  RemoveDialog()                                                           */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*     Removes a Dialog window from the screen.                              */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   DIALOGSHELL *ptr     ->  pointer to a dialog shell structure            */
/*                                                                           */
/* Return:                                                                   */
/*   None                                                                    */
/*****************************************************************************/
void RemoveDialog(DIALOGSHELL *ptr)
{
  /***************************************************************************/
  /* - set the default video attribute.                                      */
  /***************************************************************************/
  VideoAtr = vaMenuBar;

  /***************************************************************************/
  /* Restore the screen area covered by the pop up window.                   */
  /* Release the memory allocated for screen save area.                      */
  /***************************************************************************/
  windowrst(ptr->col,ptr->row,ptr->length,ptr->width,ptr->SaveArea);
  Tfree(ptr->SaveArea);

  /***************************************************************************/
  /* Reset the Screen Access to write.                                       */
  /***************************************************************************/
  SetScrAccess();
}

/*****************************************************************************/
/*  VerfiyMousePtrinDialogButtons()                                          */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Check if the mouseclick is within the context of current dialog buttons */
/* if it is map click to the key value associated with the button & ret 1    */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   DIALOGSHELL *shell     -> to a dialog shell structure.                  */
/*   uint        *Key       -> to mapped key stroke.                         */
/*                                                                           */
/* Return:                                                                   */
/*   TRUE if the click is within the dialog or else FALSE                    */
/*****************************************************************************/
uchar  VerifyMouseinDialogButtons (DIALOGSHELL *shell,uint *Key)
{
  BUTTON *ButtonPtr;                    /* -> to button struture.            */
  int     i;

  Event = GetCurrentEvent();
  /***************************************************************************/
  /* Is click on the buttons row of the popup window.                        */
  /***************************************************************************/
  if ( (Event->Row >= (shell->row + shell->length - shell->NoOfButtonRows - 1)) &&
       (Event->Row <= (shell->row + shell->length - 1)) )
  {
     ButtonPtr = shell->Buttons;
     for (i = 0 ; i < shell->NoOfButtons ; i++)
     {
       /**********************************************************************/
       /* check if the click is in the button if yes return true and stuff   */
       /* in the key associated with that button.                            */
       /**********************************************************************/
       if ( (Event->Row == ButtonPtr->row) &&
            (Event->Col >= ButtonPtr->col-1) &&
            (Event->Col <= ButtonPtr->col+ButtonPtr->length))
       {
          *Key = ButtonPtr->Key;
          return(TRUE);
       }
       ButtonPtr++;
     }
  }
  return(FALSE);
}

/*****************************************************************************/
/*                                                                           */
/* GetEventPositionInDialog()                                                */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/* Checks if the current event has occurred in the dialog scrollbar, dialog  */
/* display area, dialog buttons or outside the dialog and returns the        */
/* corresponding code.                                                       */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   shell         input  - Pointer to DIALOGSHELL structure.                */
/*   ptr           input  - Pointer to DIALOGCHOICE structure.               */
/*   key           output - If the event is in the dialog buutons then the   */
/*                          key corresponding to the button is returned in   */
/*                          this uint *.                                     */
/* Return:                                                                   */
/*                                                                           */
/*   SCROLLBAR_UPARROW      Event in scrollbar on the Up  arrow.            */
/*   SCROLLBAR_DNARROW      Event in scrollbar on the Down  arrow.          */
/*   SCROLLBAR_ABOVE_SLIDER Event in scrollbar above the slider.             */
/*   SCROLLBAR_BELOW_SLIDER Event in scrollbar below the slider.             */
/*   SCROLLBAR_ON_SLIDER    Event in scrollbar on the slider.                */
/*   BUTTONS                Event in dialog buttons.                         */
/*   DISPLAY_AREA           Event in the display area of the dialog.         */
/*   OUTSIDE_DIALOG         Event outside the dialog.                        */
/*                                                                           */
/*****************************************************************************/
uint GetEventPositionInDialog( DIALOGSHELL *shell, DIALOGCHOICE *ptr,
                               uint *key, uchar ScrollBar )
{
  Event = GetCurrentEvent();
  /***************************************************************************/
  /* Is the click within the row and column boundries of the Dialog          */
  /***************************************************************************/
  if( ( Event->Row >= shell->row ) &&
      ( Event->Row <= (shell->row + shell->length + 1) ) &&
      ( Event->Col >= shell->col ) &&
      ( Event->Col <= (shell->col + shell->width) ) )
  {
    if( (Event->Col == (shell->col + shell->width - SCROLLBARCOLOFFSET)) &&
        ScrollBar )
    {
      if( Event->Row == (shell->row + UPARROWROWOFFSET) )
        return( SCROLLBAR_UPARROW );

      if( Event->Row ==
        (shell->row + shell->length - shell->NoOfButtonRows - DNARROWROWOFFSET))
        return( SCROLLBAR_DNARROW );

      if( ( Event->Row >= (shell->row + UPARROWROWOFFSET +1) ) &&
          ( Event->Row <= (shell->row+shell->length - shell->NoOfButtonRows -
                           DNARROWROWOFFSET - 1) ) )
      {
        if( Event->Row < ptr->SliderStartRow )
          return( SCROLLBAR_ABOVE_SLIDER );

        if( Event->Row > ptr->SliderStartRow + ptr->SliderSize )
          return( SCROLLBAR_BELOW_SLIDER );

        return( SCROLLBAR_ON_SLIDER );
      }
    }
    else
    {
      if( VerifyMouseinDialogButtons( shell, key ) )
        return( BUTTONS );

      if( (Event->Row >= shell->row + shell->SkipLines) &&
          (Event->Row <  shell->row + shell->SkipLines + ptr->MaxRows) )
        return( DISPLAY_AREA );
      return( ON_TITLE );
    }
  }
  return( OUTSIDE_DIALOG );
}

/*****************************************************************************/
/*                                                                           */
/* ProcessDialog()                                                           */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/* This is the common function to process all the dialogs. It uses two       */
/* fuctions in the shell structure. The two functions are                    */
/*                                                                           */
/*   1. Display function to display the elements in the display area of the  */
/*      dialog.                                                              */
/*   2. Dialog function which will process various events that occur in the  */
/*      dialog.                                                              */
/*                                                                           */
/* This function also maintains the scrollbar display. As and when an event  */
/* occurs in the dialog, it sends messages to the dialog function and calls  */
/* the display functions to refresh the display area.                        */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   shell         input  - Pointer to DIALOGSHELL structure.                */
/*   ptr           input  - Pointer to DIALOGCHOICE structure.               */
/*   Scrollbar     input  - Indicates whether the dialog needs a scrollbar.  */
/*   ParamBlock    i/o    - Void pointer which will be passed on to the      */
/*                          dialog function to facilitate parameter passing  */
/*                          between the caller of the dialog and the dialog  */
/*                          function.                                        */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/* This functions assumes that the two function pointers are set in the      */
/* dialog shell parameter passed to it.                                      */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*          None.                                                            */
/*****************************************************************************/
uint  ProcessDialog( DIALOGSHELL *shell, DIALOGCHOICE *ptr, uchar ScrollBar,
                     void *ParamBlock )
{
  int    CurHighFldNo;
  int    NewHighFldNo;
  int    SelectIndex;
  uint   key;
  uint   ButtonKey;                                                     /*821*/
  int    OldSkipRows;
  uint   NoOfSliderPos;
  uint   Resolution;
  uint   i, rc;
  uchar  PositionTable[10];
  int    PositionTbPtr;
  uchar  InDragMode;
  uint   WaitTime;
  uint   SliderDrag;
  uint   EventPosition;
  EVENT  Dummy;

  /***************************************************************************/
  /* - set default video attribute.                                          */
  /***************************************************************************/
  VideoAtr = vaMenuBar;

  /***************************************************************************/
  /* - Init the select Index and higlight indexs.                            */
  /* - set the fld type and field column position.                           */
  /* - set the cursor type.                                                  */
  /***************************************************************************/
  InDragMode    = FALSE;
  CurHighFldNo  = 1;
  NewHighFldNo  = 1;
  SelectIndex   = 1;
  PositionTbPtr = 0;

  VioSetCurType( &NormalCursor, 0 );

  /***************************************************************************/
  /* - Send INIT_DIALOG message to the dialog function.                      */
  /***************************************************************************/
  Dummy.Value = INIT_DIALOG;
  ptr->SkipRows = 0;                                                    /*812*/
  shell->CurrentField = CurHighFldNo;
  rc = (*(shell->DialogFunction))( shell, ptr, &Dummy, ParamBlock );
  if( rc )
    return( rc );

  if( (shell->CurrentField < ptr->SkipRows) ||
      (shell->CurrentField > (ptr->MaxRows + ptr->SkipRows)) )
  {
    ptr->SkipRows = shell->CurrentField - 1;
    ptr->SkipRows = min( ptr->SkipRows, (int)(ptr->entries - ptr->MaxRows) );
    ptr->SkipRows = (ptr->SkipRows < 0) ? 0 : ptr->SkipRows;
    CurHighFldNo  = shell->CurrentField - ptr->SkipRows;
    NewHighFldNo  = shell->CurrentField - ptr->SkipRows;
    SelectIndex   = shell->CurrentField;
  }
  else
    SelectIndex = CurHighFldNo = NewHighFldNo = shell->CurrentField;

  /***************************************************************************/
  /* If scrollbar is present calculate the various scrollbar attributes.     */
  /***************************************************************************/
  if( ScrollBar == TRUE )
  {
    /*************************************************************************/
    /* Calculate the slider row.                                             */
    /*************************************************************************/
    ptr->SliderStartRow = shell->row + UPARROWROWOFFSET + 1;

    /*************************************************************************/
    /* Calculate the Slider Size.                                            */
    /*  RoundUp((Page Size / Total Entries) * No of Chars in Scroll Bar)     */
    /*************************************************************************/
    ptr->SliderSize = (uint)( (ptr->MaxRows * ptr->MaxRows) / ptr->entries )
                      + 1;
    if( ptr->SliderSize > ptr->MaxRows )
       ptr->SliderSize = ptr->MaxRows;

    /*************************************************************************/
    /* Calculate the Resolution.                                             */
    /*  RoundUp(Total Entries / Page Size)                                   */
    /*************************************************************************/
    Resolution = (uint)( ptr->entries / ptr->MaxRows );

    /*************************************************************************/
    /* Calculate the Slider Positions.                                       */
    /*  No of Chars in Scroll Bar - Slider Size + 1                          */
    /*  Build the scroll bar slider vs skip entries map.                     */
    /*************************************************************************/
    NoOfSliderPos = ptr->MaxRows - ptr->SliderSize + 1;
    for( i = 0; i < NoOfSliderPos; i++ )
      PositionTable[i] = 1 + ( i * Resolution );

   if( ptr->SkipRows )
   {
      for( i = 0; i < NoOfSliderPos; i++ )
      {
        if( i != (NoOfSliderPos - 1) )
        {
          if( ((ptr->SkipRows+1) >= (int)PositionTable[i]) &&
              ((ptr->SkipRows+1) < (int)PositionTable[i+1]) )
          {
             PositionTbPtr = i;
             ptr->SliderStartRow = shell->row+UPARROWROWOFFSET + 1 + i;
             break;
          }
        }
        else
        {
          if( (ptr->SkipRows+1) >= (int)PositionTable[i] )
          {
            PositionTbPtr = i;
            ptr->SliderStartRow = shell->row+UPARROWROWOFFSET + 1 + i;
            break;
          }
        }
      }
    }
  }

  /***************************************************************************/
  /* Display the color entries in the dialog box.                            */
  /***************************************************************************/

  /***************************************************************************/
  /* - Call display function to display elements in the display area.        */
  /***************************************************************************/
  (*(shell->Display))( shell, ptr );

  /***************************************************************************/
  /* - Display the field cursor in the current field.                        */
  /* - If scrollbar is present display the slider in the scrollbar.          */
  /***************************************************************************/
  putrc( shell->row + shell->SkipLines + CurHighFldNo - 1, shell->col + 1,
         hilite );
  if( ScrollBar == TRUE )
  {
    for( i = 0; i < ptr->SliderSize; i++ )
      putrc( ptr->SliderStartRow + i, shell->col + shell->width -
             SCROLLBARCOLOFFSET, ScrollShade2 );
  }

  WaitTime = SEM_INDEFINITE_WAIT;

  for( ;; )
  {
    /*************************************************************************/
    /* - Get an event.                                                       */
    /*************************************************************************/
    Event = GetEvent( WaitTime );

    /*************************************************************************/
    /* Switch based on the type of the event.                                */
    /*************************************************************************/
    switch( Event->Type )
    {
      case TYPE_MOUSE_EVENT:
      {
        EventPosition = GetEventPositionInDialog( shell, ptr, &key, ScrollBar );
        switch( Event->Value )
        {
          /*******************************************************************/
          /* If the event is a left button down event.                       */
          /*******************************************************************/
          case EVENT_BUTTON_1_DOWN:
          {
            /*****************************************************************/
            /* - Set the state of the button to Button Pressed.              */
            /*   (The state will be over written at some places).            */
            /*****************************************************************/
            MouseState = STATE_BUTTON_PRESSED;
            switch( EventPosition )
            {
              case SCROLLBAR_UPARROW:
              {
                /*************************************************************/
                /* If the event was on the scrollbar up  arrow or on the    */
                /* down  arrow.                                             */
                /*  - Set mouse state to continuous press (the user might).  */
                /*  - Set the wait time to Initial wait (500ms) if it is a   */
                /*    real button one down event or if it is a fake event    */
                /*    set wait time to wait between two fake events (50ms).  */
                /*  - Set the key to scroll up or scroll down for futher     */
                /*    processing below.                                      */
                /*************************************************************/
                MouseState = STATE_CONTINUOUS_PRESS;
                key = SCROLLUP;
                if( Event->FakeEvent == TRUE )
                  WaitTime = SCROLL_REGULAR_WAIT;
                else
                  WaitTime = SCROLL_INITIAL_WAIT;
                break;
              }

              case SCROLLBAR_DNARROW:
              {
                MouseState = STATE_CONTINUOUS_PRESS;
                key = SCROLLDOWN;
                if( Event->FakeEvent == TRUE )
                  WaitTime = SCROLL_REGULAR_WAIT;
                else
                  WaitTime = SCROLL_INITIAL_WAIT;
                break;
              }

              case SCROLLBAR_ABOVE_SLIDER:
              {
                /*************************************************************/
                /* If the event was on the scrollbar above or below the      */
                /* slider                                                    */
                /*  -  Set key to page up or page down for further processing*/
                /*     below.                                                */
                /*************************************************************/
                key = PGUP;
                break;
              }

              case SCROLLBAR_BELOW_SLIDER:
              {
                key = PGDN;
                break;
              }

              case SCROLLBAR_ON_SLIDER:
              {
                /*************************************************************/
                /* If the event was on the scrollbar slider                  */
                /*  - Set mouse state to continuous press (the user might).  */
                /*  - Set the slider drag flag.                              */
                /*  - Set key to scroll bar for further processing below.    */
                /*************************************************************/
                MouseState = STATE_CONTINUOUS_PRESS;
                SliderDrag = ON;
                key = SCROLLBAR;
                break;
              }

              case BUTTONS:
                /*************************************************************/
                /* If the button down event was on any of the dialog buttons */
                /* save the "button key" so that we can check when the user  */
                /* lets up on the button.                                    */
                /*************************************************************/
                ButtonKey = key;                                        /*821*/
                continue;                                               /*821*/

              case DISPLAY_AREA:
              {
                /*************************************************************/
                /* If the event was on the display area of the dialog        */
                /*  - Calculate the new field to be high lighted.            */
                /*  - Set key to mouse pick and break for further processing.*/
                /*************************************************************/
                NewHighFldNo = (uint)Event->Row - shell->row -
                                     shell->SkipLines + 1;
                key = MOUSEPICK;
                break;
              }

              default:
                continue;
            }
            break;
          }

          case EVENT_BUTTON_1_DOWN_MOVE:
          {
            /*****************************************************************/
            /* If the event is a left button down move event.                */
            /*****************************************************************/
            switch( EventPosition )
            {
              case SCROLLBAR_UPARROW:
              {
                /*************************************************************/
                /* If the user is still holding the mouse button 1 down      */
                /*  - Set key to scroll up / scroll down.                    */
                /*  - Set wait time to wait between two fake events (50ms).  */
                /*  - Change the event value to button down, so that the     */
                /*    subsequent fake events will be treated as button down  */
                /*    events until the user lets up the button.              */
                /*  - Break for further processing below.                    */
                /* Else                                                      */
                /*  - Set the mouse state to button move and continue for    */
                /*    further processing.                                    */
                /*************************************************************/
                if( MouseState == STATE_CONTINUOUS_PRESS )
                {
                  key = SCROLLUP;
                  WaitTime = SCROLL_REGULAR_WAIT;
                  Event->Value = EVENT_BUTTON_1_DOWN;
                }
                else
                  MouseState = STATE_BUTTON_MOVE;
                break;
              }

              case SCROLLBAR_DNARROW:
              {
                if( MouseState == STATE_CONTINUOUS_PRESS )
                {
                  key = SCROLLDOWN;
                  WaitTime = SCROLL_REGULAR_WAIT;
                  Event->Value = EVENT_BUTTON_1_DOWN;
                }
                else
                  MouseState = STATE_BUTTON_MOVE;
                break;
              }

              case SCROLLBAR_ON_SLIDER:
              {
                /*************************************************************/
                /* If the mouse drag is on the slider of the scrollbar, check*/
                /* if the user has already picked up (a button down) the     */
                /* slider and still holding on to it (continuous press state)*/
                /*  - Set the key value as scroll bar and mouse state        */
                /*  - Break for further processing below.                    */
                /* Else                                                      */
                /*  - Continue for there is no processing needs to be done.  */
                /*************************************************************/
                if( SliderDrag == ON && MouseState == STATE_CONTINUOUS_PRESS )
                {
                  key = SCROLLBAR;
                  break;
                }
                else
                  continue;
              }

              case DISPLAY_AREA:
              {
                /*************************************************************/
                /* If the user has picked up the slider, or the Up or Down   */
                /* arrows in the scroll bar (clicks on the above, holds the  */
                /* button and moves), dont do any processing continue for the*/
                /* next event as nothing is to be done until he lets of the  */
                /* mouse button.                                             */
                /*************************************************************/
                if( MouseState == STATE_CONTINUOUS_PRESS )
                  continue;

                /*************************************************************/
                /* If the mouse state was button released, calculate the new */
                /* field row to be highlited, set the key to mouse pick and  */
                /* break for further processing below.                       */
                /* Else                                                      */
                /*  It might be a button release event, so reset the wait    */
                /*  time and continue for the next event.                    */
                /*************************************************************/
                if( MouseState == STATE_BUTTON_RELEASED )
                {
                  NewHighFldNo = (uint)Event->Row - shell->row -
                                       shell->SkipLines + 1;
                  key = MOUSEPICK;
                }
                else
                {
                  MouseState = STATE_BUTTON_MOVE;
                  WaitTime   = SEM_INDEFINITE_WAIT;
                  continue;
                }

                MouseState = STATE_BUTTON_MOVE;
                break;
              }

              default:
              {
                /*************************************************************/
                /* The event has occured outside the dialog. If the user is  */
                /* still holding the button while he has picked up something */
                /* (as explained above) dont do anything continue for the    */
                /* next event, else set mouse state and wait time and        */
                /* continue.                                                 */
                /*************************************************************/
                if( MouseState == STATE_CONTINUOUS_PRESS )
                  continue;

                MouseState = STATE_BUTTON_MOVE;
                WaitTime = SEM_INDEFINITE_WAIT;
                continue;
              }
            }
            break;
          }

          case EVENT_NO_BUTTONS_DOWN:
          case EVENT_NO_BUTTONS_DOWN_MOVE:
          {
            /*****************************************************************/
            /* If the user lets of the button outside the dialog box beep.   */
            /* - Reset the variables and then continue.                      */
            /*****************************************************************/
#if 0
            if( EventPosition == OUTSIDE_DIALOG )
              beep();
#endif
            /*****************************************************************/
            /* If the user has let up on the mouse button on a dialog button */
            /* check if he has already clicked on that button. If they match */
            /* break for further processing doen below. If not continue.     */
            /*****************************************************************/
            if( EventPosition == BUTTONS )                              /*821*/
              if( ButtonKey == key )                                    /*821*/
                break;                                                  /*821*/

            ButtonKey  = 0;                                             /*821*/
            MouseState = STATE_BUTTON_RELEASED;
            SliderDrag = OFF;
            InDragMode = FALSE;
            WaitTime = SEM_INDEFINITE_WAIT;
            continue;
          }

          default:
          {
            /*****************************************************************/
            /* Ideally, should not enter this code.                          */
            /*****************************************************************/
            WaitTime = SEM_INDEFINITE_WAIT;
            continue;
          }
        }
        break;
      }

      case TYPE_KBD_EVENT:
      {
        /*********************************************************************/
        /* If it is keyboard event, set the key value and break for further  */
        /* processing below.                                                 */
        /*********************************************************************/
        key = Event->Value;
        break;
      }
    }

    /*************************************************************************/
    /* - Set the wait time appropriately. (It does not look that clean to do */
    /*   this here, anyway it works and should try to clean later).          */
    /*************************************************************************/
    if( EventPosition != SCROLLBAR_UPARROW &&
        EventPosition != SCROLLBAR_DNARROW &&
        MouseState    != STATE_CONTINUOUS_PRESS )
      WaitTime = SEM_INDEFINITE_WAIT;

    /*************************************************************************/
    /* If the dialog has the length of its hilite bar as 0, the dialog       */
    /* contains text (help dialog), so map up arrow key and down arrow key   */
    /* as scroll up and scroll down.                                         */
    /*************************************************************************/
    if( (normal[1] == 0) && (hilite[1] == 0) )
    {
      if( key == UP )
        key = SCROLLUP;
      else
      if( key == DOWN )
        key = SCROLLDOWN;
    }

    switch( key )
    {
      case UP:
      {
        SelectIndex--;
        if( SelectIndex < 1 )
        {
           /******************************************************************/
           /* If we are going above the top reset the index and continue.    */
           /******************************************************************/
           SelectIndex = 1;
           continue;
        }
        else
        {
          /*******************************************************************/
          /* - Change the highlight row position.                            */
          /* - If the highlight row position is going above the window, then */
          /*   we need to scroll the display. Anchor to the top of the win.  */
          /*******************************************************************/
          NewHighFldNo = CurHighFldNo - 1;
          if( NewHighFldNo < 1 )
          {
            NewHighFldNo++;
            ptr->SkipRows--;
            ptr->SkipRows = (ptr->SkipRows < 0) ? 0 : ptr->SkipRows;
            (*(shell->Display))( shell, ptr );
          }
          shell->CurrentField = NewHighFldNo;
          Dummy = *Event;
          Dummy.Value = key;
          (*(shell->DialogFunction))( shell, ptr, &Dummy, ParamBlock );
        }
        break;
      }

      case SCROLLUP:
      {
        /*********************************************************************/
        /* If there are no rows to scroll up continue.                       */
        /*********************************************************************/
        if( ptr->SkipRows == 0 )
           continue;
        /*********************************************************************/
        /* Scroll the window.                                                */
        /*********************************************************************/
        ptr->SkipRows--;
        (*(shell->Display))( shell, ptr );
        /*********************************************************************/
        /* - Calculate the HighLight row position. It should keep following  */
        /*   the current selection. If it goes beyond the window ground it   */
        /*   to the bottom of the window and adjust the SelectIndex.         */
        /*********************************************************************/
        NewHighFldNo = SelectIndex - ptr->SkipRows;
        if( NewHighFldNo > ptr->MaxRows )
        {
          NewHighFldNo = ptr->MaxRows;
          SelectIndex--;
        }
        else
        {
          shell->CurrentField = NewHighFldNo;
          Dummy = *Event;
          Dummy.Value = key;
          (*(shell->DialogFunction))( shell, ptr, &Dummy, ParamBlock );
        }
        break;
      }

      case SCROLLDOWN:
      {
        /*********************************************************************/
        /* If there are no rows to scroll down continue.                     */
        /*********************************************************************/
        if( (ptr->SkipRows + ptr->MaxRows) >= ptr->entries )
           continue;
        /*********************************************************************/
        /* Scroll the window.                                                */
        /*********************************************************************/
        ptr->SkipRows++;
        ptr->SkipRows = min(ptr->SkipRows,(int)(ptr->entries-ptr->MaxRows));
        ptr->SkipRows = (ptr->SkipRows < 0) ? 0 : ptr->SkipRows;
        (*(shell->Display))( shell, ptr );
        /*********************************************************************/
        /* - Calculate the HighLight row position. It should keep following  */
        /*   the current selection. If it goes above the window ground it    */
        /*   to the top of the window and adjust the SelectIndex.            */
        /*********************************************************************/
        NewHighFldNo = SelectIndex - ptr->SkipRows;
        if( NewHighFldNo < 1 )
        {
          NewHighFldNo = 1;
          SelectIndex++;
        }
        else
        {
          shell->CurrentField = NewHighFldNo;
          Dummy = *Event;
          Dummy.Value = key;
          (*(shell->DialogFunction))( shell, ptr, &Dummy, ParamBlock );
        }
        break;
      }

      case PGUP:
      {
        /*********************************************************************/
        /* If there are no rows to scroll up continue.                       */
        /*********************************************************************/
        if( ptr->SkipRows == 0 )
           continue;
        OldSkipRows = ptr->SkipRows;
        /*********************************************************************/
        /* - Scroll the window.                                              */
        /* - CalCulate the select index.                                     */
        /*********************************************************************/
        ptr->SkipRows -= ptr->MaxRows;
        ptr->SkipRows  = (ptr->SkipRows < 0) ? 0 : ptr->SkipRows;
        SelectIndex    = (SelectIndex - (OldSkipRows - ptr->SkipRows));
        (*(shell->Display))( shell, ptr );
        shell->CurrentField = NewHighFldNo;
        Dummy = *Event;
        Dummy.Value = key;
        (*(shell->DialogFunction))( shell, ptr, &Dummy, ParamBlock );
        break;
      }

      case DOWN:
      {
        SelectIndex++;
        if( SelectIndex > ptr->entries )
        {
          /******************************************************************/
          /* If we are going below the botom reset the index and conitnue.  */
          /******************************************************************/
          SelectIndex = ptr->entries;
          continue;
        }
        else
        {
          /*******************************************************************/
          /* - Change the highlight row position.                            */
          /* - If the highlight row position is going below the window, then */
          /*   we need to scroll the display. Anchor to the bottom of the    */
          /*   window.                                                       */
          /*******************************************************************/
          NewHighFldNo = CurHighFldNo + 1;
          if( (NewHighFldNo > ptr->MaxRows) || (NewHighFldNo > ptr->entries) )
          {
            SelectIndex = min(SelectIndex,ptr->entries);
            NewHighFldNo--;
            ptr->SkipRows++;
            ptr->SkipRows = min(ptr->SkipRows,(int)(ptr->entries-ptr->MaxRows));
            ptr->SkipRows = (ptr->SkipRows < 0) ? 0 : ptr->SkipRows;
            (*(shell->Display))( shell, ptr );
          }
          shell->CurrentField = NewHighFldNo;
          Dummy = *Event;
          Dummy.Value = key;
          (*(shell->DialogFunction))( shell, ptr, &Dummy, ParamBlock );
        }
        break;
      }

      case PGDN:
      {
        /*********************************************************************/
        /* If there are no rows to scroll down continue.                     */
        /*********************************************************************/
        if( (ptr->SkipRows + ptr->MaxRows) >= ptr->entries )
           continue;
        /*********************************************************************/
        /* - Scroll the window.                                              */
        /* - CalCulate the select index.                                     */
        /*********************************************************************/
        OldSkipRows = ptr->SkipRows;
        ptr->SkipRows += ptr->MaxRows;
        ptr->SkipRows = min(ptr->SkipRows,(int)(ptr->entries-ptr->MaxRows));
        ptr->SkipRows = (ptr->SkipRows < 0) ? 0 : ptr->SkipRows;
        SelectIndex =  (SelectIndex + (ptr->SkipRows - OldSkipRows));
        (*(shell->Display))( shell, ptr );
        shell->CurrentField = NewHighFldNo;
        Dummy = *Event;
        Dummy.Value = key;
        (*(shell->DialogFunction))( shell, ptr, &Dummy, ParamBlock );
        break;
      }

      case SCROLLBAR:
      {
        int RowsToSkip;
        int SkipRows;
        static ushort MarkedMouseRow;
        SkipRows = ptr->SkipRows;

        if( Event->Value != EVENT_BUTTON_1_DOWN_MOVE )
        {
          /*******************************************************************/
          /* If the click is on the slider itself continue. If yes take a    */
          /* snap shot of mouse row and go into drag mode.                   */
          /*******************************************************************/
          if( ((uint)Event->Row >= ptr->SliderStartRow) &&
              ((uint)Event->Row <= (ptr->SliderStartRow + ptr->SliderSize - 1)) )
          {
            MarkedMouseRow = Event->Row;
            InDragMode = TRUE;
            continue;
          }

          /*******************************************************************/
          /* If the mouse position is below slider its a page down.          */
          /*******************************************************************/
          if( (uint)Event->Row > (ptr->SliderStartRow + ptr->SliderSize - 1) )
            RowsToSkip = ptr->MaxRows;

          /*******************************************************************/
          /* If the mouse position is above slider its a page up.            */
          /*******************************************************************/
          if( (uint)Event->Row < ptr->SliderStartRow )
             RowsToSkip = -(int)ptr->MaxRows;
        }
        else
        {
          if( InDragMode == TRUE )
          {
            /****************************************************************/
            /* If we are in drag mode determine on which side we are moving */
            /* and depending on direction determine the position table ptr. */
            /* Handle the boundries of the position table.                  */
            /****************************************************************/
            if( Event->Row > MarkedMouseRow )
              PositionTbPtr++;
            if( Event->Row < MarkedMouseRow )
              PositionTbPtr--;
            if( Event->Row == MarkedMouseRow )
              continue;

            MarkedMouseRow = Event->Row;
            if( PositionTbPtr < 0 )
              PositionTbPtr = 0;
            if( PositionTbPtr > (NoOfSliderPos - 1) )
              PositionTbPtr = NoOfSliderPos - 1;
          }
          else
            continue;
          /*******************************************************************/
          /* get the number of rows to skip from the postion table.          */
          /*******************************************************************/
          RowsToSkip = PositionTable[PositionTbPtr] - 1;
          SkipRows = 0;
        }
        /*********************************************************************/
        /*    Calculate the new number of rows to be skipped. Adjust it with */
        /*  respect to the boundries.                                        */
        /*********************************************************************/
        if( RowsToSkip > 0 )
        {
          SkipRows += RowsToSkip;
          SkipRows = min(SkipRows,(int)(ptr->entries-ptr->MaxRows));
          SkipRows = (SkipRows < 0) ? 0 : SkipRows;
        }
        else
        {
          SkipRows -= (-RowsToSkip);
          SkipRows  = (SkipRows < 0) ? 0 : SkipRows;
        }

        /*********************************************************************/
        /* If New number of rows to skip is same as old one simply conintue. */
        /*********************************************************************/
        if( SkipRows == ptr->SkipRows )
           continue;
        else
        {
          /******************************************************************/
          /* - Calculate the Highlight row no.                              */
          /* - Put in the new skip rows value into the structure.           */
          /******************************************************************/
          NewHighFldNo = SelectIndex - SkipRows;
          ptr->SkipRows = SkipRows;
        }

        if( NewHighFldNo < 1 )
        {
          /******************************************************************/
          /*   If the highlight row is going above the window, reset it to  */
          /* top of window and adjust the select index.                     */
          /******************************************************************/
          NewHighFldNo = 1;
          SelectIndex = ptr->SkipRows + 1;
        }

        if( NewHighFldNo > ptr->MaxRows )
        {
          /******************************************************************/
          /*   If the highlight row is going below the window, reset it to  */
          /* bot of window and adjust the select index.                     */
          /******************************************************************/
          NewHighFldNo = ptr->MaxRows;
          SelectIndex = ptr->SkipRows + ptr->MaxRows;
        }

        /*********************************************************************/
        /* scroll the window.                                                */
        /*********************************************************************/
        (*(shell->Display))( shell, ptr );
        shell->CurrentField = NewHighFldNo;
        Dummy = *Event;
        Dummy.Value = key;
        (*(shell->DialogFunction))( shell, ptr, &Dummy, ParamBlock );
      }
      break;

      case MOUSEPICK:
      {
        uint  NewSelectIndex;

        NewSelectIndex = SelectIndex - (CurHighFldNo - NewHighFldNo);

        /*********************************************************************/
        /* If the new selected item is same as the old one and we are in     */
        /* attribute field simply continue.                                  */
        /*********************************************************************/

        if( NewSelectIndex <= ptr->entries )
          SelectIndex = NewSelectIndex;
        else
          NewHighFldNo = CurHighFldNo;
      }                                 /* Intentional fall-through          */
      goto casedefault;
casedefault:

      default:
      {
        uint retcode;

        Dummy = *Event;
        Dummy.Value = key;
        shell->CurrentField = NewHighFldNo;

        retcode = (*(shell->DialogFunction))( shell, ptr, &Dummy , ParamBlock );
        if( retcode )
          return( retcode );
        break;
      }
    }

    if( ScrollBar == TRUE )
    {
      for( i = 0; i < NoOfSliderPos; i++ )
      {
        if( i != (NoOfSliderPos - 1) )
        {
          if( ((ptr->SkipRows+1) >= (int)PositionTable[i]) &&
              ((ptr->SkipRows+1) < (int)PositionTable[i+1]) )
          {
             PositionTbPtr = i;
             ptr->NewSliderStartRow = shell->row+UPARROWROWOFFSET + 1 + i;
             break;
          }
        }
        else
        {
          if( (ptr->SkipRows+1) >= (int)PositionTable[i] )
          {
            PositionTbPtr = i;
            ptr->NewSliderStartRow = shell->row+UPARROWROWOFFSET + 1 + i;
            break;
          }
        }
      }

      if( ptr->NewSliderStartRow != ptr->SliderStartRow )
      {
        for( i = 0; i < ptr->SliderSize; i++ )
          putrc( ptr->SliderStartRow + i,
                 shell->col + shell->width - SCROLLBARCOLOFFSET,
                 ScrollShade1 );
        ptr->SliderStartRow = ptr->NewSliderStartRow;
        for( i = 0; i < ptr->SliderSize; i++ )
          putrc( ptr->SliderStartRow + i,
                 shell->col + shell->width - SCROLLBARCOLOFFSET,
                 ScrollShade2 );
      }
    }

    if( CurHighFldNo != NewHighFldNo )
    {
      putrc( shell->row + shell->SkipLines + CurHighFldNo - 1, shell->col + 1,
             normal );
      CurHighFldNo = NewHighFldNo;
    }
    putrc( shell->row + shell->SkipLines + CurHighFldNo - 1, shell->col + 1,
           hilite );
  }
}

/*****************************************************************************/
/* ProcessYesNoBox()                                                         */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*  Handle a simple box with a yes/no response.                              */
/*                                                                           */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   shell         Pointer to DIALOGSHELL structure.                         */
/*   ptr           Pointer to DIALOGCHOICE structure.                        */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   key                                                                     */
/*                                                                           */
/*****************************************************************************/
UINT  ProcessYesNoBox( DIALOGSHELL *shell, DIALOGCHOICE *ptr )
{
 UINT   key;
 UINT   ButtonKey;
 UINT   WaitTime;
 UINT   EventPosition;

 /****************************************************************************/
 /* - set default video attribute.                                           */
 /****************************************************************************/
 VideoAtr = vaMenuBar;

 /****************************************************************************/
 /* - Call display function to display elements in the display area.         */
 /****************************************************************************/
 (*(shell->Display))( shell, ptr );

 WaitTime = SEM_INDEFINITE_WAIT;

 for( ;; )
 {
  /**************************************************************************/
  /* - Get an event.                                                        */
  /**************************************************************************/
  Event = GetEvent( WaitTime );

  /**************************************************************************/
  /* Switch based on the type of the event.                                 */
  /**************************************************************************/
  switch( Event->Type )
  {
   case TYPE_MOUSE_EVENT:
   {
    EventPosition = GetEventPositionInDialog( shell, ptr, &key, FALSE );
    switch( Event->Value )
    {
     /************************************************************************/
     /* If the event is a left button down event.                            */
     /************************************************************************/
     case EVENT_BUTTON_1_DOWN:
     {
      /***********************************************************************/
      /* - Set the state of the button to Button Pressed.                    */
      /*   (The state will be over written at some places).                  */
      /***********************************************************************/
      MouseState = STATE_BUTTON_PRESSED;

      /***********************************************************************/
      /* If the button down event was on any of the dialog buttons           */
      /* save the "button key" so that we can check when the user            */
      /* lets up on the button.                                              */
      /***********************************************************************/
      if( EventPosition == BUTTONS )
       ButtonKey = key;
      continue;
     }
   /*break;*/

     case EVENT_BUTTON_1_DOWN_MOVE:
     {
      MouseState = STATE_BUTTON_MOVE;
      WaitTime   = SEM_INDEFINITE_WAIT;
      continue;
     }
   /*break;*/

     case EVENT_NO_BUTTONS_DOWN:
     case EVENT_NO_BUTTONS_DOWN_MOVE:
     {
      /***********************************************************************/
      /* If the user has let up on the mouse button on a dialog button       */
      /* check if he has already clicked on that button. If they match       */
      /* break for further processing down below. If not continue.           */
      /***********************************************************************/
      if( EventPosition == BUTTONS )
      {
       if( ButtonKey == key )
         break;
      }

      ButtonKey  = 0;
      MouseState = STATE_BUTTON_RELEASED;
      WaitTime   = SEM_INDEFINITE_WAIT;
      continue;
     }
  /* break;*/

     default:
     {
      /***********************************************************************/
      /* Ideally, should not enter this code.                                */
      /***********************************************************************/
      WaitTime = SEM_INDEFINITE_WAIT;
      continue;
     }
  /* break; */
    }
   }
   break;

   case TYPE_KBD_EVENT:
   {
    /*************************************************************************/
    /* If it is keyboard event, set the key value and break for further      */
    /* processing below.                                                     */
    /*************************************************************************/
    key = Event->Value;
    break;
   }
  }

  switch( key )
  {
   case key_y:
   case key_Y:
   case key_n:
   case key_N:
   case ENTER:
   case ESC:
    return(key);

   default:
    break;
  }
 }
}
