/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   cuamenu.c                                                               */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Cua Action bar related functions.                                       */
/*                                                                           */
/*****************************************************************************/

#include "all.h"
#include <ctype.h>
#include "cuamenu.h"

/*****************************************************************************/
/* #defines for various positions in the menu.                               */
/*****************************************************************************/
#define POS_ACTIONBAR    1              /* Event in the action bar.          */
#define POS_PULLDOWN     2              /* Event in the current pulldown.    */
#define POS_OUTSIDEMENU  3              /* Event outside the menu.           */

#define NO_PULLDOWN      -1

#define CONTINUE         1
#define RETURN           2

#define MENU_PULLDOWN    1
#define MENU_ACTIONBAR   2
#define MENU_PULLACTION  3

/*****************************************************************************/
/* Externals.                                                                */
/*****************************************************************************/
extern uchar    hiatt[];                /* attributes string for highlight.  */
extern uchar    VideoAtr;               /* default logical video attribute.  */
extern uchar    ClearField[];           /* attrib str for clearing fields.   */
extern uchar    hilite[];               /* attrib str to highlight fields.   */
extern uchar    badhilite[];            /* attrib str to highlight bad fields*/
extern uchar    normal[];               /* attrib str to disp normal fields. */
extern uchar    badnormal[];            /* attrib str to disp bad fields.    */
extern uchar    Shadow[];               /* attrib str for shadowing around   */
extern KEY2FUNC defk2f[];               /* keys to functions map.            */
extern KEYSCODE key2code[];
extern uint     VideoCols;
extern uint     VideoRows;
extern uchar    *VideoMap;
extern UINT     MenuRow;                /* screen row for menu bar (0..N)    */

/*****************************************************************************/
/* Statics.                                                                  */
/*****************************************************************************/
static PEVENT   Event;
static int      MouseState = STATE_BUTTON_RELEASED;
static int      MenuState;
static UCHAR    icol[ NUM_PULLDOWNS ];
static UCHAR    ilen[ NUM_PULLDOWNS ];
static int      CurrentMenuChoice;

static UCHAR    uarrow[]    = { U_ARROW, 0 };
static UCHAR    darrow[]    = { D_ARROW, 0 };

static UCHAR    barstr[]    = {V_BAR,0};
static UCHAR    rarrow[]    = { R_ARROW, 0 };
static UCHAR    topline[]   = { TL_CORNER, RepCnt(1), H_BAR, TR_CORNER, 0 };
static UCHAR    botline[]   = { BL_CORNER, RepCnt(1), H_BAR, BR_CORNER, 0 };

static UCHAR    separator[] = { L_JUNCTION, RepCnt(1), H_BAR, R_JUNCTION, 0 };
static UCHAR    bullet[]    = { BULLET, 0 };

typedef struct keychoice
{
  int Key;
  int Choice;
} KEY2CHOICE;

/*****************************************************************************/
/* CuaMenuBar()                                                              */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   We come into this function when we have received an event on the        */
/*   action bar. The user is wants to select a function to execute.          */
/*   The purpose of this function is to provide the interface necessary to   */
/*   select the function and return a function code to the caller.           */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   InitialOption  - Action bar selection before entry.                     */
/*                                                                           */
/* Return Value:                                                             */
/*                                                                           */
/*   FuncCode       - function code of the selected function.                */
/*                                                                           */
/*****************************************************************************/
int  CuaMenuBar( int InitialOption , void *pParm )
{
  int    FuncCode, rc, InitialPullChoice = 0;

  /***************************************************************************/
  /* This code will manage two cursors. The cursor highlighting the          */
  /* Action Bar selection and another highlighting the pulldown selection.   */
  /*                                                                         */
  /* -  Initialise the flags.                                                */
  /***************************************************************************/

  /***************************************************************************/
  /* If the function has been called with initial option as No pulldown, ie  */
  /* only the action bar cursor is to be shown.                              */
  /***************************************************************************/
  if( InitialOption == NO_PULLDOWN )
  {
    /*************************************************************************/
    /* - Set current pulldown flag.                                          */
    /* - Set CurrentMenuChoice which will get the selection on the action bar*/
    /* - Display the action bar cursor.                                      */
    /*************************************************************************/
    CurrentMenuChoice = 0;
    DisplayMenuCursor( CurrentMenuChoice );
    MenuState = MENU_ACTIONBAR;
  }
  /***************************************************************************/
  /* If the function has been called with a valid action bar option.         */
  /***************************************************************************/
  else
  {
    /*************************************************************************/
    /* - Set CurrentMenuChoice which will get the selection on the action bar*/
    /* - Display the action bar cursor.                                      */
    /* - Display the pulldown for the action bar selection.                  */
    /* - Display the pulldown cursor on the top pulldown selection.          */
    /*************************************************************************/
    if( (CurrentMenuChoice = InitialOption - 1) >= NUM_PULLDOWNS )
       CurrentMenuChoice = 0;

    DisplayMenuCursor( CurrentMenuChoice );
    DisplayPulldown( CurrentMenuChoice );

    Event = GetCurrentEvent();
    if( Event->Type == TYPE_MOUSE_EVENT )
    {
      MenuState = MENU_PULLACTION;
      switch( Event->Value )
      {
        case EVENT_BUTTON_1_DOWN:
        {
          MouseState = STATE_BUTTON_PRESSED;
          break;
        }

        case EVENT_BUTTON_1_DOWN_MOVE:
        {
          MouseState = STATE_BUTTON_MOVE;
          break;
        }

        case EVENT_NO_BUTTONS_DOWN:
        case EVENT_NO_BUTTONS_DOWN_MOVE:
        {
          MouseState = STATE_BUTTON_RELEASED;
          break;
        }
      }
    }
    else
    {
      MenuState  = MENU_PULLDOWN;
      MouseState = NULL;
    }
  }

  /***************************************************************************/
  /* Now, we go into a loop until we get a function code to return to the    */
  /* caller.                                                                 */
  /***************************************************************************/
  for( ;; )
  {
    switch( MenuState )
    {
      case MENU_PULLDOWN:
      {
        rc = GetPulldownChoice( CurrentMenuChoice, &MouseState, &FuncCode,
                                InitialPullChoice );
        switch( rc )
        {
          case ESC:                                                     /*818*/
          case ENTER:
          case PADENTER:
          {
            RemoveMenuCursor( CurrentMenuChoice );
            Event->Type=TYPE_KBD_EVENT; /*Pretend function returned by KBD*/
            return( FuncCode );
          }

          case RIGHT:
          {
            RemoveMenuCursor( CurrentMenuChoice );
            if( CurrentMenuChoice < (NUM_PULLDOWNS - 1) )
              CurrentMenuChoice++;
            else
              CurrentMenuChoice = 0;
            DisplayMenuCursor( CurrentMenuChoice );
            DisplayPulldown( CurrentMenuChoice );
            MenuState = MENU_PULLDOWN;
            break;
          }

          case LEFT:
          {
            RemoveMenuCursor( CurrentMenuChoice );
            if( CurrentMenuChoice > 0 )
              CurrentMenuChoice--;
            else
              CurrentMenuChoice = NUM_PULLDOWNS - 1;
            DisplayMenuCursor( CurrentMenuChoice );
            DisplayPulldown( CurrentMenuChoice );
            MenuState = MENU_PULLDOWN;
            break;
          }

          case RETURN:
          {
            RemoveMenuCursor( CurrentMenuChoice );
            return( FuncCode );
          }

          case MOUSECLICK:
          {
            int store;

            store = CurrentMenuChoice;
            if( MouseEventInActionBar( &CurrentMenuChoice ) )
            {
              CurrentMenuChoice--;
              if( store != CurrentMenuChoice )
              {
                RemoveMenuCursor( store );
                DisplayPulldown( CurrentMenuChoice );
                DisplayMenuCursor( CurrentMenuChoice );
              }

              if( MouseState == STATE_BUTTON_PRESSED ||
                  MouseState == STATE_BUTTON_MOVE )
                MenuState = MENU_PULLACTION;
              else
              {
                MenuState = MENU_PULLDOWN;
                MouseState = STATE_BUTTON_RELEASED;
                InitialPullChoice = 1;
              }
            }
            else
            {
              RemoveMenuCursor( store );
              return( SETCURSORPOS );
            }
            break;
          }
        }
        break;
      }

      case MENU_ACTIONBAR:
      case MENU_PULLACTION:
      {
        rc = GetActionbarChoice( &CurrentMenuChoice, &MenuState, &MouseState,
                                 &InitialPullChoice );
        switch( rc )
        {
          case ENTER:
          case PADENTER:
          {
            MenuState = MENU_PULLDOWN;
            break;
          }

          case ESC:
            return( DONOTHING );

          case MOUSECLICK:
          case SETCURSORPOS:
            return( SETCURSORPOS );

          case CONTINUE:
            break;
        }
        break;
      }
    }                                   /* switch( MenuState );              */
  }                                     /* for( ;; ) loop                    */
}

/*****************************************************************************/
/* DisplayMenu()                                                             */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*  Displays the action bar and builds arrays containing the length and      */
/*  column position of the action bar elements.                              */
/*                                                                           */
/* Parameters:                                                               */
/*             None                                                          */
/*                                                                           */
/* Return:                                                                   */
/*             None                                                          */
/*                                                                           */
/*****************************************************************************/
#define SPACING 1

void  DisplayMenu()
{
  int   Index, col, n;
  uchar *ip;

  ClrScr( MenuRow, MenuRow, vaMenuBar );

  for( Index = 0, col = SPACING, ip = MenuNames; *ip; ++Index )
  {
    icol[Index] = (uchar)col;
    ilen[Index] = (uchar)( (n = strlen( ip ) ) + 2 );
    col += n + SPACING + 1;
    ip  += n + 1;
  }

  for( Index = 0, ip = MenuNames; Index < NUM_PULLDOWNS; ++Index )
  {
    putrcx( MenuRow, icol[Index] + 1, ip);
    putrcx( MenuRow, icol[Index] + MenuCols[Index], hiatt);
    ip += ilen[Index] - 1;
  }

  putrcx( MenuRow, SHRINKUPARROWCOL, uarrow);
  putrcx( MenuRow, SHRINKUPARROWCOL, hiatt);
  putrcx( MenuRow, EXPANDDNARROWCOL, darrow);
  putrcx( MenuRow, EXPANDDNARROWCOL, hiatt);
}

/*****************************************************************************/
/*                                                                           */
/* GetEventPositionInMenu()                                                  */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/* Checks if the current event has occurred in the action bar,the            */
/* current pulldown, or neither. If the current event is in an               */
/* action bar or pulldown, then an index of the particular selection         */
/* of where the event occurred is returned.                                  */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   CurrPullIndex input  - the index of the pulldown currently selected.    */
/*   Choice        output - if the event is in the pulldown or the action    */
/*                          bar, then this is the index of the selection.    */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   POS_ACTIONBAR    Current event is in the action bar.                    */
/*   POS_PULLDOWN     Current event is in a pulldown.                        */
/*   POS_OUTSIDEMENU  Current event is not in pulldown or the action bar.    */
/*                                                                           */
/*****************************************************************************/
int  GetEventPositionInMenu( int CurrPullIndex, int *Choice )
{
  if( MouseEventInActionBar( Choice ) )
    return( POS_ACTIONBAR );

  if( MouseEventInPulldown( CurrPullIndex, Choice ) )
    return( POS_PULLDOWN );

  return( POS_OUTSIDEMENU );
}


/*****************************************************************************/
/* MouseEventInActionBar()                                                   */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Checks if the current event has occurred in the action bar, and if      */
/*   so sets the choice for the caller.                                      */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   Choice     output - selection if the event is in the action bar.        */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   TRUE       Event is in action bar.                                      */
/*   FALSE      Event is not in action bar.                                  */
/*                                                                           */
/*****************************************************************************/
int  MouseEventInActionBar( int *Choice )
{
  int    Index;
  PEVENT CurrentEvent;

  /***************************************************************************/
  /* As this function is called outside this module we have to get the       */
  /* current event even though this is available as a static variable (Event)*/
  /* in this module.                                                         */
  /***************************************************************************/
  CurrentEvent = GetCurrentEvent();

  if( CurrentEvent->Row == MenuRow )
  {
    for( Index = 0; Index < NUM_PULLDOWNS; Index++ )
    {
      if ( (CurrentEvent->Col >= icol[Index]) &&
           (CurrentEvent->Col <= (icol[Index] + ilen[Index])) )
      {
        *Choice = Index + 1;
        return( TRUE );
      }
    }
  }
  *Choice = 0;
  return( FALSE );
}

/*****************************************************************************/
/* MouseEventInPulldown()                                                    */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Checks if the current event has occurred in the pulldown, and if        */
/*   so sets the choice for the caller.                                      */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   PulldownIndex input  - Index corresponding to the current pulldown.     */
/*   Choice        output - Index corresponding to the choice in the         */
/*                          pulldown if the event is in the pulldown.        */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   TRUE       Event is in pulldown.                                        */
/*   FALSE      Event is not in pulldown.                                    */
/*                                                                           */
/*****************************************************************************/
int MouseEventInPulldown( int PulldownIndex, int *Choice )
{
  PULLDOWN  *PullPtr;
  PEVENT    CurrentEvent;

  CurrentEvent = GetCurrentEvent();
  PullPtr = pullarray[ PulldownIndex ];

  if( (CurrentEvent->Row > PullPtr->row ) &&
      (CurrentEvent->Row < (PullPtr->row + PullPtr->entries + 1)) )
  {
    if( (CurrentEvent->Col >= PullPtr->col) &&
        (CurrentEvent->Col <= PullPtr->col + PullPtr->width) )
    {
      *Choice = (int)CurrentEvent->Row - PullPtr->row;
      return( TRUE );
    }
  }
  return( FALSE );
}

/*****************************************************************************/
/* RemoveMenuCursor()                                                        */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Removes the action bar cursor.                                          */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   Index     - current selection in the action bar that the cursor is on.  */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   None                                                                    */
/*                                                                           */
/*****************************************************************************/
static uchar optOFF[] = {RepCnt(0),Attrib(vaMenuBar),0};
#define optOFFlen optOFF[1]

void  RemoveMenuCursor( int Index )
{
  /***************************************************************************/
  /* - Remove the hilight from the choice.                                   */
  /* - Display the hot key in hi attribute.                                  */
  /***************************************************************************/
  optOFFlen = ilen[Index];
  putrcx( MenuRow, icol[Index], optOFF);
  putrcx( MenuRow, icol[Index] + MenuCols[Index], hiatt);
}

/*****************************************************************************/
/* DiaplayMenuCursor()                                                       */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Adds the action bar cursor.                                             */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   Index     - selection in the action bar to put the cursor on.           */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   None                                                                    */
/*****************************************************************************/
static uchar  optON[] = {RepCnt(0),Attrib(vaMenuCsr),0};
#define optONlen optON[1]

void  DisplayMenuCursor( int Index )
{
  optONlen  = ilen[Index];
  putrcx( MenuRow, icol[Index], optON);
}

/*****************************************************************************/
/* GetActionbarChoice()                                                      */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Gets a user choice in the action bar when the pulldown is not displayed.*/
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   ActionbarIndex - position of the action bar cursor.                     */
/*                   (pulldown index into the array)                         */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*                                                                           */
/*****************************************************************************/
int  GetActionbarChoice( int *CurrentMenuChoice, int *MenuState,
                         int *MouseState, int *Choice )
{
  int PullDisplay = FALSE;

  if( *MenuState == MENU_PULLACTION )
    PullDisplay = TRUE;

  for( ;; )
  {
    Event = GetEvent( SEM_INDEFINITE_WAIT );
    switch( Event->Type )
    {
      case TYPE_MOUSE_EVENT:
      {
        int EventPosition;

        EventPosition = GetEventPositionInMenu( *CurrentMenuChoice,
                                                 Choice );
        switch( Event->Value )
        {
          case EVENT_NO_BUTTONS_DOWN:
          case EVENT_NO_BUTTONS_DOWN_MOVE:
          {
            switch( EventPosition )
            {
              case POS_PULLDOWN:
              case POS_ACTIONBAR:
              {
                if( *MouseState == STATE_BUTTON_PRESSED ||
                    *MouseState == STATE_BUTTON_MOVE )
                {
                  if( EventPosition == POS_ACTIONBAR )
                  {
#if 0
                    *CurrentMenuChoice = *Choice - 1;
#endif
                    *Choice = 1;
                  }
                  *MenuState = MENU_PULLDOWN;
                  return( CONTINUE );
                }
                break;
              }

              case POS_OUTSIDEMENU:
              {
                if( PullDisplay )
                  RemovePulldown( *CurrentMenuChoice );
                RemoveMenuCursor( *CurrentMenuChoice );
                return( SETCURSORPOS );
              }
            }
            break;
          }

          case EVENT_BUTTON_1_DOWN:
#if 0
          case EVENT_BUTTON_2_DOWN:
#endif
          {
            *MouseState = STATE_BUTTON_PRESSED;
            switch( EventPosition )
            {
              case POS_ACTIONBAR:
              {
                if( *Choice != (*CurrentMenuChoice + 1) )
                {
                  RemoveMenuCursor( *CurrentMenuChoice );
                  if( PullDisplay )
                    RemovePulldown( *CurrentMenuChoice );
                  *CurrentMenuChoice = *Choice - 1;
                  if( PullDisplay )
                    DisplayPulldown( *CurrentMenuChoice );
                  DisplayMenuCursor( *CurrentMenuChoice );
                }
                if( !PullDisplay )
                {
                  DisplayPulldown( *CurrentMenuChoice );
                  PullDisplay = TRUE;
                }
                break;
              }

              default:
              {
                if( PullDisplay )
                  RemovePulldown( *CurrentMenuChoice );
                RemoveMenuCursor( *CurrentMenuChoice );
                return( MOUSECLICK );
              }
            }
            break;
          }

          case EVENT_BUTTON_1_DOWN_MOVE:
#if 0
          case EVENT_BUTTON_2_DOWN_MOVE:
#endif
          {
            *MouseState = STATE_BUTTON_MOVE;
            switch( EventPosition )
            {
              case POS_ACTIONBAR:
              {
                if( *Choice != (*CurrentMenuChoice + 1) )
                {
                  if( PullDisplay == TRUE )
                    RemovePulldown( *CurrentMenuChoice );
                  RemoveMenuCursor( *CurrentMenuChoice );
                  *CurrentMenuChoice = *Choice - 1;
                  DisplayPulldown( *CurrentMenuChoice );
                  PullDisplay = TRUE;
                  DisplayMenuCursor( *CurrentMenuChoice );
                }
                break;
              }

              case POS_PULLDOWN:
              {
                if( PullDisplay == TRUE )
                {
                  *MenuState = MENU_PULLDOWN;
                  return( CONTINUE );
                }
                else
                {
                  RemoveMenuCursor( *CurrentMenuChoice );
                  return( MOUSECLICK );
                }
              }

              case POS_OUTSIDEMENU:
              {
                if( PullDisplay )
                  continue;
                else
                {
                  RemoveMenuCursor( *CurrentMenuChoice );
                  return( MOUSECLICK );
                }
              }

              default:
                break;
            }
            break;
          }
        }
        break;
      }

      case TYPE_KBD_EVENT:
      {
        switch( Event->Value )
        {
          case F1:
          {
            UCHAR    *HelpMsg, NoHelpMsg[50];
            ULONG    HelpId;

            HelpId = Menuhids[*CurrentMenuChoice];
            if( HelpId )
              HelpMsg = GetHelpMsg( HelpId, NULL,0 );
            else
            {
              strcpy( NoHelpMsg, "\rNo help available currently. Sorry!\r" );
              HelpMsg = NoHelpMsg;
            }
            CuaShowHelpBox( HelpMsg );
            break;
          }

          case RIGHT:
          {
            RemoveMenuCursor( *CurrentMenuChoice );
            if( *CurrentMenuChoice < (NUM_PULLDOWNS - 1) )
              (*CurrentMenuChoice)++;
            else
              *CurrentMenuChoice = 0;
            DisplayMenuCursor( *CurrentMenuChoice );
            break;
          }

          case LEFT:
          {
            RemoveMenuCursor( *CurrentMenuChoice );
            if( *CurrentMenuChoice > 0 )
              (*CurrentMenuChoice)--;
            else
              *CurrentMenuChoice = NUM_PULLDOWNS - 1;
            DisplayMenuCursor( *CurrentMenuChoice );
            break;
          }

          case ENTER:
          case PADENTER:
          {
            DisplayPulldown( *CurrentMenuChoice );
            return( Event->Value );
          }

          case ESC:
          {
            RemoveMenuCursor( *CurrentMenuChoice );
            return( Event->Value );
          }

          default:
          {
            int ActionBarChoice;

            ActionBarChoice = bindex( Menukeys, NUM_PULLDOWNS,
                                     (Event->Value & 0xFF) | 0x20 );
            if( ActionBarChoice < NUM_PULLDOWNS )
            {
              RemoveMenuCursor( *CurrentMenuChoice );
              *CurrentMenuChoice = ActionBarChoice;
              DisplayMenuCursor( *CurrentMenuChoice );
              DisplayPulldown( *CurrentMenuChoice );
              return( ENTER );
            }
            else
              beep();
            break;
          }
        }                               /* switch( Event->Value )            */
      }                                 /* case TYPE_KBD_EVENT               */
    }                                   /* switch( Event->Type )             */
  }                                     /* for( ;; ) loop                    */
}

/*****************************************************************************/
/* DisplayPullCursor()                                                       */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Displays the pulldown cursor.                                           */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   PullIndex - position of the action bar choice.                          */
/*               (pulldown index into the array)                             */
/*   Index     - position of the pulldown cursor choice.                     */
/*                                                                           */
/* Return:                                                                   */
/*   None                                                                    */
/*                                                                           */
/*****************************************************************************/
void  DisplayPullCursor( int PullIndex, int Index )
{
  PULLDOWN *PullPtr = pullarray[ PullIndex ];

  badhilite[1] = (uchar)PullPtr->width - 2;
  hilite[1]    = (uchar)PullPtr->width - 2;

  VideoMap[vaBadSel] = (VideoMap[vaMenuCsr] & 0xF0) | FG_black | FG_light;

  if( TestBit( PullPtr->BitMask, Index - 1 ) )
    putrcx( PullPtr->row + Index, PullPtr->col + 1, hilite);
  else
    putrcx( PullPtr->row + Index, PullPtr->col + 1, badhilite);
}

/*****************************************************************************/
/*                                                                           */
/* RemovePullCursor()                                                        */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Removes the pulldown cursor.                                            */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   PullIndex - position of the action bar choice.                          */
/*               (pulldown index into the array)                             */
/*   Index     - position of the pulldown cursor choice.                     */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   None                                                                    */
/*                                                                           */
/*****************************************************************************/
void  RemovePullCursor( int PullIndex, int Index )
{
  PULLDOWN *PullPtr = pullarray[ PullIndex ];

  badnormal[1] = (uchar)PullPtr->width - 2;
  normal[1]    = (uchar)PullPtr->width - 2;

  if( TestBit(PullPtr->BitMask, Index - 1 ) )
  {
    putrcx( PullPtr->row + Index, PullPtr->col + 1, normal);
    putrcx( PullPtr->row + Index, PullPtr->col + 2 +
           *( PullPtr->SelPos + Index - 1 ), hiatt);
  }
  else
    putrcx( PullPtr->row + Index, PullPtr->col + 1, badnormal);

}

/*****************************************************************************/
/* DisplayPulldown()                                                         */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*     Displays a pulldown window along with selections.                     */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   PullIndex - position of the action bar choice.                          */
/*               (pulldown index into the array)                             */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   None                                                                    */
/*                                                                           */
/*****************************************************************************/
void  DisplayPulldown( int PullIndex )
{
  int      i;
  char     *str;
  PULLDOWN *PullPtr;
  CAS_PULLDOWN *Cascade;

  PullPtr = pullarray[ PullIndex ];

  /***************************************************************************/
  /* Allocate the memory required to save the screen that will be covered by */
  /* Pulldown and save the screen area which will be covered.                */
  /***************************************************************************/
  PullPtr->SaveArea = (uchar *)Talloc( (PullPtr->entries + 3) *
                                       (PullPtr->width + 1) * 2 );
  windowsv( PullPtr->col, PullPtr->row, PullPtr->entries + 3,
            PullPtr->width + 1, PullPtr->SaveArea );

  /***************************************************************************/
  /* - set the default video attribute.                                      */
  /* - set up string for attributes of bad entries.                          */
  /* - initialize color attribute for the clear cell.                        */
  /***************************************************************************/
  VideoAtr      = vaMenuBar;
  badnormal[1]  = (uchar)PullPtr->width - 2;
  VideoMap[vaBadAct] = (VideoMap[vaMenuBar] & 0xF0) | FG_black | FG_light;
  ClearField[1] = (uchar)PullPtr->width;

  /***************************************************************************/
  /* clear the screen area covered by the pulldown.                          */
  /***************************************************************************/
  for( i = 0; i < PullPtr->entries + 2; i++ )
    putrcx( PullPtr->row + i, PullPtr->col, ClearField);

  /***************************************************************************/
  /* Put bar characters on the action bar.                                   */
  /***************************************************************************/
  topline[2] = (char)PullPtr->width - 2;
  putrcx( PullPtr->row, PullPtr->col, topline);

  /***************************************************************************/
  /* Put the entries into the pull down box. If the item is disabled then    */
  /* change the attributes of that item. Highlight the Character by which    */
  /* the item can be selected.                                               */
  /***************************************************************************/
  Cascade = &(PullPtr->CasPulldown[0]);
  for( i = 0, str = PullPtr->labels; i < PullPtr->entries ; i++ )
  {
    putrcx( PullPtr->row + i + 1, PullPtr->col, barstr);
    if( str[0] == '-' )
    {
      separator[2] = (char)PullPtr->width - 2;
      putrcx( PullPtr->row + i + 1, PullPtr->col, separator);
    }
    else
    {
      putrcx( PullPtr->row + i + 1, PullPtr->col + 2, str);
      if( Cascade )
        putrcx( PullPtr->row + i + 1, PullPtr->col + PullPtr->width - 8,
                (uchar *)(PullPtr->AccelKeys + i));
      else
      if( PullPtr->AccelKeys )
        putrcx( PullPtr->row + i + 1, PullPtr->col + PullPtr->width - 7,
                (uchar *)(PullPtr->AccelKeys + i));

      if( TestBit( PullPtr->BitMask, i ) )
        putrcx( PullPtr->row + i + 1, PullPtr->col + 2 + *(PullPtr->SelPos + i),
                hiatt);
      else
        putrcx( PullPtr->row + i + 1, PullPtr->col + 1, badnormal);

      if( Cascade &&  Cascade[i].PulldownIndex )
        putrcx( PullPtr->row + i + 1, PullPtr->col + PullPtr->width - 2,
                rarrow);
      putrcx( PullPtr->row + i + 1, PullPtr->col + PullPtr->width - 1, barstr);
    }
    str += strlen( str ) + 1;
  }

  /***************************************************************************/
  /* Put length into botline string and then display the bottom line.        */
  /***************************************************************************/
  botline[2] = (char)PullPtr->width - 2;
  putrcx( PullPtr->row + PullPtr->entries + 1, PullPtr->col, botline);


  /***************************************************************************/
  /* Display the shadowing stuff.                                            */
  /***************************************************************************/
  Shadow[1] = (char)PullPtr->width;
  putrcx( PullPtr->row + PullPtr->entries + 2, PullPtr->col + 1, Shadow);
  Shadow[1] = (char)1;

  for( i = 1; i < PullPtr->entries + 3; i++ )
    putrcx( PullPtr->row + i, PullPtr->col + PullPtr->width, Shadow);
}

/*****************************************************************************/
/* RemovePulldown()                                                          */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*     Removes a pulldown window from the screen.                            */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   PullIndex - position of the action bar choice.                          */
/*               (pulldown index into the array)                             */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   None                                                                    */
/*                                                                           */
/*****************************************************************************/
void  RemovePulldown( int PullIndex )
{
  PULLDOWN *PullPtr;

  PullPtr = pullarray[ PullIndex ];
  /***************************************************************************/
  /* - set the default video attribute.                                      */
  /* - Hide the mouse.                                                       */
  /***************************************************************************/
  VideoAtr = vaMenuBar;

  /***************************************************************************/
  /* Restore the screen area covered by the pulldown                         */
  /* Release the memory allocated for screen save area.                      */
  /***************************************************************************/
  windowrst( PullPtr->col, PullPtr->row, PullPtr->entries + 3,
             PullPtr->width + 1, PullPtr->SaveArea );

  Tfree( PullPtr->SaveArea );
}

/*****************************************************************************/
/* MouseEventInArrow()                                                       */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Checks if the current event has occurred in the arrow indicating the    */
/*   cascaded pulldown for the current pulldown choice.                      */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   Choice     output - selection if the event is in the action bar.        */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   TRUE       Event is in pulldown arrow.                                  */
/*   FALSE      Event is not in pulldown arrow.                              */
/*                                                                           */
/*****************************************************************************/
int  MouseEventInArrow( PULLDOWN *PullPtr, int CurrentPullChoice )
{
  PEVENT CurrentEvent;

  CurrentEvent = GetCurrentEvent();
  if( CurrentEvent->Col == (PullPtr->col + PullPtr->width - 2) )
    return( TRUE );
  else
    return( FALSE );
}

/*****************************************************************************/
/* GetPulldownChoice()                                                       */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Gets a user choice after a pulldown is displayed.                       */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   PullIndex - position of the action bar choice.                          */
/*               (pulldown index into the array)                             */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   This function assumes the pulldown is displayed already, and the caller */
/* need not remove the pulldown which is done inside this function itself.   */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*                                                                           */
/*****************************************************************************/
int  GetPulldownChoice( int PullIndex, int *MouseState, int *FuncCode,
                        int InitialPullChoice )
{
  int       CurrentPullChoice, PrevPullChoice, key;
  PULLDOWN *PullPtr;
  PEVENT    CurrentEvent;

  PullPtr = pullarray[ PullIndex ];
  if( InitialPullChoice )
    CurrentPullChoice = PrevPullChoice = InitialPullChoice;
  else
    CurrentPullChoice = PrevPullChoice = 1;

  DisplayPullCursor( PullIndex, CurrentPullChoice );
#if 0
  *MouseState = STATE_BUTTON_RELEASED;
#endif

  for( ;; )
  {
    CurrentEvent = GetEvent( SEM_INDEFINITE_WAIT );
    switch( CurrentEvent->Type )
    {
      case TYPE_MOUSE_EVENT:
      {
        int EventPosition;

        if( CurrentPullChoice )
          PrevPullChoice = CurrentPullChoice;
        EventPosition = GetEventPositionInMenu( PullIndex, &CurrentPullChoice );
        switch( CurrentEvent->Value )
        {
          case EVENT_NO_BUTTONS_DOWN:
          case EVENT_NO_BUTTONS_DOWN_MOVE:
          {
            switch( EventPosition )
            {
              case POS_PULLDOWN:
              {
                if( (*MouseState == STATE_BUTTON_PRESSED ||
                     *MouseState == STATE_BUTTON_MOVE) &&
                    (TestBit(PullPtr->BitMask, CurrentPullChoice - 1))
                  )
                {
                  CAS_PULLDOWN *Cascade;

                  Cascade = &(PullPtr->CasPulldown[0]);
                  if( Cascade && Cascade[CurrentPullChoice - 1].PulldownIndex &&
                      MouseEventInArrow( PullPtr, CurrentPullChoice ) )
                    key = RIGHT;
                  else
                    key = ENTER;
                }
                else
                {
                  if( *MouseState == STATE_BUTTON_PRESSED ||
                      *MouseState == STATE_BUTTON_MOVE )
                    beep();
                  continue;
                }
                break;
              }

              default:
              {
                if( *MouseState == STATE_BUTTON_PRESSED ||
                    *MouseState == STATE_BUTTON_MOVE )
                {
                  *FuncCode = SETCURSORPOS;
                  RemovePulldown( PullIndex );
                  return( MOUSECLICK );
                }
                else
                {
                  if( !CurrentPullChoice )
                    CurrentPullChoice = 1;
                  continue;
                }
              }
            }
            break;
          }

#if 0
          case EVENT_BUTTON_2_DOWN:
          {
            if( EventPosition == POS_OUTSIDEMENU )
            {
              SetCursorPos();
              continue;
            }
            else
            {
              *MouseState = STATE_BUTTON_RELEASED;
              continue;
            }
          }
#endif

          case EVENT_BUTTON_1_DOWN:
          case EVENT_BUTTON_1_DOWN_MOVE:
#if 0
          case EVENT_BUTTON_2_DOWN_MOVE:
#endif
          {
            if( CurrentEvent->Value == EVENT_BUTTON_1_DOWN )
              *MouseState = STATE_BUTTON_PRESSED;

            switch( EventPosition )
            {
              case POS_PULLDOWN:
              {
                if( CurrentEvent->Value != EVENT_BUTTON_1_DOWN )
                  *MouseState = STATE_BUTTON_MOVE;
                if( CurrentPullChoice != PrevPullChoice )
                {
                  if( PrevPullChoice && (!(PullPtr->separators &
                      (0x0001 << (PrevPullChoice - 1)))) )
                    RemovePullCursor( PullIndex, PrevPullChoice );
                  PrevPullChoice = CurrentPullChoice;
                  if( !(PullPtr->separators &
                        (0x0001 << (PrevPullChoice - 1))) )
                    DisplayPullCursor( PullIndex, CurrentPullChoice );
                }
                continue;
              }

              case POS_OUTSIDEMENU:
              {
                if( CurrentEvent->Value == EVENT_BUTTON_1_DOWN ||
                    ( *MouseState == STATE_BUTTON_RELEASED &&
                      CurrentEvent->Value != EVENT_BUTTON_2_DOWN_MOVE ) )
                {
                  *FuncCode = SETCURSORPOS;
                  RemovePulldown( PullIndex );
                  return( MOUSECLICK );
                }
                else
                {
                  if( PrevPullChoice )
                    RemovePullCursor( PullIndex, PrevPullChoice );
                  PrevPullChoice = CurrentPullChoice;
                  continue;
                }
              }

              default:
              {
                RemovePullCursor( PullIndex, PrevPullChoice );
                if( CurrentPullChoice != (PullIndex + 1) )
                  RemovePulldown( PullIndex );

                if( CurrentEvent->Value != EVENT_BUTTON_1_DOWN )
                  *MouseState = STATE_BUTTON_MOVE;
                *FuncCode = SETCURSORPOS;
                return( MOUSECLICK );
              }
            }
          }
      /*  break; */

          default:
          {
            *MouseState = STATE_BUTTON_RELEASED;
            continue;
          }
        }
        break;
      }

      case TYPE_KBD_EVENT:
      {
        key = CurrentEvent->Value;
        break;
      }
    }

    switch( key )
    {
      case F1:
      {
        uchar    *HelpMsg, NoHelpMsg[50];
        ULONG    HelpId;

        HelpId  = PullPtr->help[ CurrentPullChoice - 1 ];
        if( HelpId )
          HelpMsg = GetHelpMsg( HelpId, NULL,0 );
        else
        {
          strcpy( NoHelpMsg, "\rNo help available currently. Sorry!\r" );
          HelpMsg = NoHelpMsg;
        }
        CuaShowHelpBox( HelpMsg );
        break;
      }

      case UP:
      {
        RemovePullCursor( PullIndex, CurrentPullChoice );
        if( CurrentPullChoice > 1 )
          CurrentPullChoice--;
        else
          CurrentPullChoice = pullarray[ PullIndex ]->entries;

        if( PullPtr->separators & (0x0001 << (CurrentPullChoice - 1)) )
          CurrentPullChoice--;
        DisplayPullCursor( PullIndex, CurrentPullChoice );
        break;
      }

      case DOWN:
      {
        RemovePullCursor( PullIndex, CurrentPullChoice );
        if( CurrentPullChoice < pullarray[ PullIndex ]->entries )
          CurrentPullChoice++;
        else
          CurrentPullChoice = 1;
        if( PullPtr->separators & (0x0001 << (CurrentPullChoice - 1)) )
          CurrentPullChoice++;
        DisplayPullCursor( PullIndex, CurrentPullChoice );
        break;
      }

      case ESC:
      case LEFT:
      {
        *FuncCode = DONOTHING;
        RemovePulldown( PullIndex );
        return( CurrentEvent->Value );
      }

      default:
      {
        uchar    *LocPtr;

        LocPtr = strchr( PullPtr->hotkeys, toupper(CurrentEvent->Value & 0x00FF) );
        if( LocPtr && *LocPtr )
        {
          key = ENTER;
          PrevPullChoice = CurrentPullChoice;
          CurrentPullChoice = (int)( LocPtr - PullPtr->hotkeys + 1 );
          if( CurrentPullChoice != PrevPullChoice )
          {
            RemovePullCursor( PullIndex, PrevPullChoice );
            DisplayPullCursor( PullIndex, CurrentPullChoice );
          }
        }
        else
        {
          beep();
          break;
        }
      }                                 /* Intentional fall-through          */

      goto caseenter;
caseenter:

      case ENTER:
      case PADENTER:
      {
       if( TestBit(pullarray[ PullIndex ]->BitMask, CurrentPullChoice - 1) )
       {
        uchar *FuncCodes;

        RemovePulldown( PullIndex );
        FuncCodes = pullarray[ PullIndex ]->funccodes;
        if( FuncCodes )
          *FuncCode = (int)FuncCodes[ CurrentPullChoice - 1 ];
        else
          *FuncCode = CurrentPullChoice - 1;

        return( key );
       }
       else
        beep();
      }
      break;

      case RIGHT:
      {
        CAS_PULLDOWN *Cascade;
        PULLDOWN     *CasPullPtr;
        int  rc;

        Cascade = &(PullPtr->CasPulldown[0]);
        if( Cascade && Cascade[CurrentPullChoice - 1].PulldownIndex  &&
            TestBit( pullarray[PullIndex]->BitMask, CurrentPullChoice - 1 )
          )

        {
          CasPullPtr =
                      pullarray[ Cascade[CurrentPullChoice - 1].PulldownIndex ];

          CasPullPtr->row = PullPtr->row + CurrentPullChoice;
          if( (PullPtr->col + PullPtr->width + CasPullPtr->width) > VideoCols )
            CasPullPtr->col = PullPtr->col - (CasPullPtr->width + 1);
          else
            CasPullPtr->col = PullPtr->col + PullPtr->width + 1;
          DisplayCasPulldown( Cascade[CurrentPullChoice - 1] );
          rc = GetCasPulldownChoice( &Cascade[CurrentPullChoice - 1],
                                      FuncCode, MouseState );
          switch( rc )
          {
            case MOUSECLICK:
            {
              int Pos;

              PrevPullChoice = CurrentPullChoice;
              Pos = GetEventPositionInMenu(PullIndex, &CurrentPullChoice);
              if( Pos == POS_PULLDOWN )
              {
                if( CurrentPullChoice != PrevPullChoice )
                {
                  RemovePullCursor( PullIndex, PrevPullChoice );
                  PrevPullChoice = CurrentPullChoice;
                  DisplayPullCursor( PullIndex, CurrentPullChoice );
                }

                CurrentEvent = GetCurrentEvent();
                if( CurrentEvent->Value == EVENT_NO_BUTTONS_DOWN ||
                    CurrentEvent->Value == EVENT_NO_BUTTONS_DOWN_MOVE )
                  *MouseState = STATE_BUTTON_RELEASED;
                else
                {
                  *MouseState = STATE_BUTTON_PRESSED;
                  continue;
                }
              }
              else
              {
                CurrentEvent = GetCurrentEvent();
                if( CurrentEvent->Value == EVENT_NO_BUTTONS_DOWN ||
                    CurrentEvent->Value == EVENT_NO_BUTTONS_DOWN_MOVE )
                  *MouseState = STATE_BUTTON_RELEASED;
                else
                {
                  RemovePullCursor( PullIndex, PrevPullChoice );
                  *MouseState = STATE_BUTTON_PRESSED;
                }

                if( CurrentPullChoice != (PullIndex + 1) )
                  RemovePulldown( PullIndex );
                return( MOUSECLICK );
              }
              break;
            }

            case PADENTER:
            case ENTER:
            {
              uchar *FuncCodes;

              RemovePulldown( PullIndex );
              FuncCodes = pullarray[ PullIndex ]->funccodes;
              if( FuncCodes )
                *FuncCode = (int)FuncCodes[ CurrentPullChoice - 1 ];
              else
                *FuncCode = CurrentPullChoice - 1;

              return( rc );
            }

            case RETURN:
            {
              RemovePulldown( PullIndex );
              return( RETURN );
            }

            case ESC:
            case LEFT:
            case CONTINUE:
              continue;

            default:
            {
              RemovePulldown( PullIndex );
              return( rc );
            }
          }
        }
        RemovePulldown( PullIndex );
        *FuncCode = DONOTHING;                                          /*807*/
        return( key );
      }
    }
  }                                     /* end of for( ;; ) loop.            */
}

/*****************************************************************************/
/* DisplayCasPulldown()                                                      */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*     Displays a cascaded pulldown with appropriate default selections      */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   PullIndex - position of the action bar choice.                          */
/*               (pulldown index into the array)                             */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   None                                                                    */
/*                                                                           */
/*****************************************************************************/
void  DisplayCasPulldown( CAS_PULLDOWN Cascade )
{
  int       i;
  PULLDOWN *PullPtr;

  DisplayPulldown( Cascade.PulldownIndex );
  PullPtr = pullarray[ Cascade.PulldownIndex ];
  for( i = 0; i < PullPtr->entries; i++ )
  {
    if( TestBit( Cascade.Flag, i ) )
      putrcx( PullPtr->row + i + 1, PullPtr->col + 1, bullet);
  }
}

/*****************************************************************************/
/* GetCasPulldownChoice()                                                    */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Gets a user choice after a cascaded pulldown is displayed.              */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   PullIndex - position of the action bar choice.                          */
/*               (pulldown index into the array)                             */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   This function assumes the pulldown is displayed already, and the caller */
/* need not remove the pulldown which is done inside this function itself.   */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*                                                                           */
/*****************************************************************************/
int  GetCasPulldownChoice( CAS_PULLDOWN *Cascade, int *Choice, int *MouseState )
{
  int rc;

  *MouseState = STATE_BUTTON_RELEASED;
  rc = GetPulldownChoice( Cascade->PulldownIndex, MouseState, Choice, 1 );
  switch( rc )
  {
    case ENTER:
    {
      Cascade->Flag = 0;
      SetBit( Cascade->Flag, *Choice );
      return( rc );
    }

    case LEFT:
      return( CONTINUE );

    default:
      return( rc );
  }
}

/*****************************************************************************/
/*  UpdatePullDownsWithAccelKeys()                                           */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Updates the static pull down structures with the accel key names.       */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   None                                                                    */
/*                                                                           */
/*****************************************************************************/
void  UpdatePullDownsWithAccelKeys()
{
  int      i, j, k, no;
  PULLDOWN  *ptr;
  int       Funccode;

  /***************************************************************************/
  /* for all the pulldowns.                                                  */
  /***************************************************************************/
  for( no = 0; no < NUM_PULLDOWNS; no++ )
  {
    ptr = pullarray[no];
    /*************************************************************************/
    /* for all the choices.                                                  */
    /*************************************************************************/
    for( j = 0; j < ptr->entries; j++ )
    {
      /***********************************************************************/
      /* - get the funccode.                                                 */
      /* - Search for the function code in the defk2f map.                   */
      /* - If found search the key2code map to find the name of the accel    */
      /*   key and put the name in the pull down struture.                   */
      /***********************************************************************/
      Funccode = (int)*(ptr->funccodes+j);
      for( i = 0 ; i < KEYNUMSOC ; i++ )
      {
        if (defk2f[i].fcode == Funccode)
        {
          for( k = 0 ; k < USERKEYS ; k++ )
          {
            if (defk2f[i].scode == key2code[k].scode )
            {
              sprintf( (char *)(ptr->AccelKeys + j), "%+5s", key2code[k].key,
                       '\0' );
              break;
            }
          }
          break;
        }
      }
    }
  }
}

/*****************************************************************************/
/* KeyInActionBarExpressKeys()                                               */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Checks whether the current keyboard event has a key which is among the  */
/* the express keys for the action bar options.                              */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   Choice   (output) - The function returns the action bar option's index  */
/*                       in this int *, if the key is among the express keys.*/
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   TRUE              - The key is among the action bar express keys.       */
/*   FALSE             - The key is not among the action bar express keys.   */
/*                                                                           */
/*****************************************************************************/
uint KeyInActionBarExpressKeys( int *Choice )
{
  KEY2CHOICE ExpkeyChoices[NUM_PULLDOWNS] =
  {
    { A_F, 1 },
    { A_R, 2 },
    { A_B, 3 },
    { A_S, 4 },
    { A_D, 5 },
    { A_V, 6 },
    { A_E, 7 },
    { A_M, 8 },
    { A_H, 9 }
  };
  PEVENT CurrentEvent;
  int    i;

  CurrentEvent = GetCurrentEvent();

  for( i = 0; i < NUM_PULLDOWNS; i++ )
  {
    if( ExpkeyChoices[i].Key == CurrentEvent->Value )
    {
      *Choice = ExpkeyChoices[i].Choice;
      return( TRUE );
    }
  }
  return( FALSE );
}

/*****************************************************************************/
/* IsSwapFlag()                                                              */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Checks whether the swap flag for the Run pulldown is set for a given    */
/* pulldown choice. Swap flags are in the cascaded pulldown structures of the*/
/* pulldown options.                                                         */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   FuncCode (input)  - Function code of the given pulldown option. Used to */
/*                       identify the current pulldown option.               */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   SWAPFLAG          - Swap flag is set for the given pulldown option.     */
/*   NOSWAPFLAG        - Swap flag is not set for the given pulldown option. */
/*                                                                           */
/* Restrictions:                                                             */
/*                                                                           */
/*   This function is purely for the Run pulldown of the action bar. It may  */
/* not work for other cascaded pulldowns.                                    */
/*                                                                           */
/*****************************************************************************/
#define  RUNPULLDOWN   1
#define  SWAPFLAG      1
#define  NOSWAPFLAG    0

int  IsSwapFlag( int FuncCode )
{
  PULLDOWN  *PullPtr;

  PullPtr = pullarray[RUNPULLDOWN];     /* only run pulldown is our concern. */
  switch( FuncCode )
  {
    case RUN:
    case SSTEP:
    case SSTEPINTOFUNC:
    case RUNTOCURSOR:
    {
      int  i;
      CAS_PULLDOWN  *Cascade = PullPtr->CasPulldown;

      for( i = 0; i < PullPtr->entries; i++ )
      {
        if( PullPtr->funccodes[i] == FuncCode )
          if(TestBit( Cascade[i].Flag, 0 ) )
            return( SWAPFLAG );
          else
            return( NOSWAPFLAG );
      }
      return( NOSWAPFLAG );
    }

    default:
      return( NOSWAPFLAG );
  }
}

/*****************************************************************************/
/* GetObjectPullChoice()                                                     */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Displays an object pulldown and gets the user choice from the pulldown. */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   PullIndex - position of the action bar choice.                          */
/*               (pulldown index into the array)                             */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*****************************************************************************/
int  GetObjectPullChoice( int PullIndex, int *MouseState )
{
  PEVENT    CurrentEvent;
  PULLDOWN *PullPtr;
  int       FuncCode;

  CurrentEvent = GetCurrentEvent();
  PullPtr = pullarray[PullIndex];

  if( CurrentEvent->Type == TYPE_MOUSE_EVENT )
  {
    if( (CurrentEvent->Row + PullPtr->entries + 3) < VideoRows )
      PullPtr->row = CurrentEvent->Row + 1;
    else
      PullPtr->row = VideoRows - PullPtr->entries - 3;

    if( (CurrentEvent->Col + PullPtr->width + 3) < VideoCols )
      PullPtr->col = CurrentEvent->Col + 1;
    else
      PullPtr->col = VideoCols - PullPtr->width - 3;
  }

  DisplayPulldown( PullIndex );
  *MouseState = STATE_BUTTON_RELEASED;
  GetPulldownChoice( PullIndex, MouseState, &FuncCode, 0 );
  return( FuncCode );
}

/*****************************************************************************/
/* ReSetSelectBit()                                                          */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Sets the select bit for a selection in a pulldown.                      */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   PullIndex       - which pulldown.                                       */
/*   PullChoiceIndex - which pulldown entry.                                 */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   None.                                                                   */
/*                                                                           */
/*****************************************************************************/
void ReSetSelectBit( int PullIndex, int PullChoiceIndex )
{
 ResetBit(pullarray[PullIndex]->BitMask, PullChoiceIndex);
}

/*****************************************************************************/
/* SetSelectBit()                                                            */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Sets the select bit for a selection in a pulldown.                      */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   PullIndex       - which pulldown.                                       */
/*   PullChoiceIndex - which pulldown entry.                                 */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   None.                                                                   */
/*                                                                           */
/*****************************************************************************/
void SetSelectBit( int PullIndex, int PullChoiceIndex )
{
 SetBit(pullarray[PullIndex]->BitMask, PullChoiceIndex);
}

/*****************************************************************************/
/* SetMenuMask()                                                             */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Sets the mask bits for the various views in the debugger.               */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   View   which view the mask bits are to be set for.                      */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   None.                                                                   */
/*                                                                           */
/*****************************************************************************/
void  SetMenuMask( int View )
{
 /****************************************************************************/
 /* enable all of the pulldown choices.                                      */
 /****************************************************************************/
 pullarray[PULLDOWN_FILE       ]->BitMask = 0xFFFF;
 pullarray[PULLDOWN_RUN        ]->BitMask = 0xFFFF;
 pullarray[PULLDOWN_BREAKPOINT ]->BitMask = 0xFFFF;
 pullarray[PULLDOWN_SEARCH     ]->BitMask = 0xFFFF;
 pullarray[PULLDOWN_DATA       ]->BitMask = 0xFFFF;
 pullarray[PULLDOWN_VIEW       ]->BitMask = 0xFFFF;
 pullarray[PULLDOWN_SETTINGS   ]->BitMask = 0xFFFF;
 pullarray[PULLDOWN_MISC       ]->BitMask = 0xFFFF;
 pullarray[PULLDOWN_HELP       ]->BitMask = 0xFFFF;
 pullarray[PULLDOWN_RUN_CASCADE]->BitMask = 0xFFFF;
 pullarray[PULLDOWN_OBJ_SOURCE ]->BitMask = 0xFFFF;
 pullarray[PULLDOWN_OBJ_FORMAT ]->BitMask = 0xFFFF;
 pullarray[PULLDOWN_OBJ_DATA   ]->BitMask = 0xFFFF;

 switch( View )
 {
  case SOURCEVIEW:
  {
   /************************************************************************/
   /* disable Data pulldown choices                                        */
   /************************************************************************/
   ResetBit(pullarray[PULLDOWN_DATA]->BitMask, PULLDOWN_DATA_EDITEXPRESSION);
   ResetBit(pullarray[PULLDOWN_DATA]->BitMask, PULLDOWN_DATA_EDITSTORAGE   );
   ResetBit(pullarray[PULLDOWN_DATA]->BitMask, PULLDOWN_DATA_FORMATVAR     );
   ResetBit(pullarray[PULLDOWN_DATA]->BitMask, PULLDOWN_DATA_REMOVEVAR     );
   ResetBit(pullarray[PULLDOWN_DATA]->BitMask, PULLDOWN_DATA_INSERT        );

   /************************************************************************/
   /* disable View pulldown choices                                        */
   /************************************************************************/
   ResetBit(pullarray[PULLDOWN_VIEW]->BitMask, PULLDOWN_VIEW_SOURCE );
  }
  break;

  case ASSEMBLYVIEW:
  {
   ResetBit(pullarray[PULLDOWN_FILE]->BitMask, PULLDOWN_FILE_FINDFUNCTION);
   ResetBit(pullarray[PULLDOWN_FILE]->BitMask, PULLDOWN_FILE_DROPFILE);

   pullarray[PULLDOWN_SEARCH]->BitMask = 0x0000;
   pullarray[PULLDOWN_DATA  ]->BitMask = 0x0000;

   ResetBit(pullarray[PULLDOWN_VIEW]->BitMask, PULLDOWN_VIEW_ASSEMBLER);
  }
  break;

  case DATAVIEWSRC:
  {
   ResetBit(pullarray[PULLDOWN_FILE]->BitMask, PULLDOWN_FILE_FINDFUNCTION);
   ResetBit(pullarray[PULLDOWN_FILE]->BitMask, PULLDOWN_FILE_DROPFILE);

   ResetBit(pullarray[PULLDOWN_RUN]->BitMask, PULLDOWN_RUN_RUNTOCURSOR);
   ResetBit(pullarray[PULLDOWN_RUN]->BitMask, PULLDOWN_RUN_RUNTOCURSORNOSWAP);
   ResetBit(pullarray[PULLDOWN_RUN]->BitMask, PULLDOWN_RUN_SETEXECLINE);

   ResetBit(pullarray[PULLDOWN_BREAKPOINT]->BitMask, PULLDOWN_BREAKPOINT_SETCLRBKP);
   ResetBit(pullarray[PULLDOWN_BREAKPOINT]->BitMask, PULLDOWN_BREAKPOINT_SETCONDBKP);

   ResetBit(pullarray[PULLDOWN_DATA]->BitMask, PULLDOWN_DATA_SHOWVAR      );
   ResetBit(pullarray[PULLDOWN_DATA]->BitMask, PULLDOWN_DATA_SHOWVARPTSTO );
   ResetBit(pullarray[PULLDOWN_DATA]->BitMask, PULLDOWN_DATA_WATCHVAR     );
   ResetBit(pullarray[PULLDOWN_DATA]->BitMask, PULLDOWN_DATA_WATCHVARPTSTO);

   ResetBit(pullarray[PULLDOWN_VIEW]->BitMask, PULLDOWN_VIEW_SOURCE);
   ResetBit(pullarray[PULLDOWN_VIEW]->BitMask, PULLDOWN_VIEW_DATA);

   ResetBit(pullarray[PULLDOWN_SETTINGS]->BitMask, PULLDOWN_SETTINGS_ASSEMBLYSOURCE);
   ResetBit(pullarray[PULLDOWN_SETTINGS]->BitMask, PULLDOWN_SETTINGS_MNEMONICS     );
  }
  break;

  case DATAVIEWASM:
  {
   ResetBit(pullarray[PULLDOWN_FILE]->BitMask, PULLDOWN_FILE_FINDFUNCTION);
   ResetBit(pullarray[PULLDOWN_FILE]->BitMask, PULLDOWN_FILE_DROPFILE);

   ResetBit(pullarray[PULLDOWN_RUN]->BitMask, PULLDOWN_RUN_RUNTOCURSOR);
   ResetBit(pullarray[PULLDOWN_RUN]->BitMask, PULLDOWN_RUN_RUNTOCURSORNOSWAP);
   ResetBit(pullarray[PULLDOWN_RUN]->BitMask, PULLDOWN_RUN_SETEXECLINE);

   ResetBit(pullarray[PULLDOWN_BREAKPOINT]->BitMask, PULLDOWN_BREAKPOINT_SETCLRBKP);
   ResetBit(pullarray[PULLDOWN_BREAKPOINT]->BitMask, PULLDOWN_BREAKPOINT_SETCONDBKP);

   pullarray[PULLDOWN_SEARCH]->BitMask = 0x0000;

   ResetBit(pullarray[PULLDOWN_DATA]->BitMask, PULLDOWN_DATA_SHOWVAR      );
   ResetBit(pullarray[PULLDOWN_DATA]->BitMask, PULLDOWN_DATA_SHOWVARPTSTO );
   ResetBit(pullarray[PULLDOWN_DATA]->BitMask, PULLDOWN_DATA_WATCHVAR     );
   ResetBit(pullarray[PULLDOWN_DATA]->BitMask, PULLDOWN_DATA_WATCHVARPTSTO);

   ResetBit(pullarray[PULLDOWN_VIEW]->BitMask, PULLDOWN_VIEW_ASSEMBLER);
   ResetBit(pullarray[PULLDOWN_VIEW]->BitMask, PULLDOWN_VIEW_DATA);
  }
  break;

  case EXPANDDATA:
  {
   pullarray[PULLDOWN_FILE       ]->BitMask = 0x0000;
   pullarray[PULLDOWN_RUN        ]->BitMask = 0x0000;
   pullarray[PULLDOWN_BREAKPOINT ]->BitMask = 0x0000;
   pullarray[PULLDOWN_SEARCH     ]->BitMask = 0x0000;
   pullarray[PULLDOWN_DATA       ]->BitMask = 0x0000;
   pullarray[PULLDOWN_VIEW       ]->BitMask = 0x0000;
   pullarray[PULLDOWN_SETTINGS   ]->BitMask = 0x0000;
   pullarray[PULLDOWN_MISC       ]->BitMask = 0x0000;
   pullarray[PULLDOWN_HELP       ]->BitMask = 0x0000;
   pullarray[PULLDOWN_RUN_CASCADE]->BitMask = 0x0000;
   pullarray[PULLDOWN_OBJ_SOURCE ]->BitMask = 0x0000;
   pullarray[PULLDOWN_OBJ_FORMAT ]->BitMask = 0x0000;
   pullarray[PULLDOWN_OBJ_DATA   ]->BitMask = 0x0000;

   SetBit(pullarray[PULLDOWN_FILE]->BitMask, PULLDOWN_FILE_QUIT);
   SetBit(pullarray[PULLDOWN_DATA]->BitMask, PULLDOWN_DATA_EXPANDVAR);
  }
  break;

  case BROWSEFILE:
  {
   pullarray[PULLDOWN_FILE       ]->BitMask = 0x0000;
   pullarray[PULLDOWN_RUN        ]->BitMask = 0x0000;
   pullarray[PULLDOWN_BREAKPOINT ]->BitMask = 0x0000;
   pullarray[PULLDOWN_SEARCH     ]->BitMask = 0x0000;
   pullarray[PULLDOWN_DATA       ]->BitMask = 0x0000;
   pullarray[PULLDOWN_VIEW       ]->BitMask = 0x0000;
   pullarray[PULLDOWN_SETTINGS   ]->BitMask = 0x0000;
   pullarray[PULLDOWN_MISC       ]->BitMask = 0x0000;
   pullarray[PULLDOWN_HELP       ]->BitMask = 0x0000;
   pullarray[PULLDOWN_RUN_CASCADE]->BitMask = 0x0000;
   pullarray[PULLDOWN_OBJ_SOURCE ]->BitMask = 0x0000;
   pullarray[PULLDOWN_OBJ_FORMAT ]->BitMask = 0x0000;
   pullarray[PULLDOWN_OBJ_DATA   ]->BitMask = 0x0000;

   SetBit(pullarray[PULLDOWN_FILE]->BitMask, PULLDOWN_FILE_QUIT);

   pullarray[PULLDOWN_SEARCH]->BitMask = 0xFFFF;
  }
  break;

#if MSH
  case BROWSEMSHFILE:
  {
    SetSelectMask( 5, 1, ON );

    for( i = 8; i < 12; i++ )
      SetSelectMask( 4, i, ON );
    break;
  }
#endif

  default:
    break;
 }
}

PULLDOWN  *GetPullPointer( int Index )
{
  return( pullarray[ Index ] );
}

void DisableDropFile( void )
{
 ReSetSelectBit( PULLDOWN_FILE, PULLDOWN_FILE_DROPFILE );
}

void EnableDropFile( void )
{
 SetSelectBit( PULLDOWN_FILE, PULLDOWN_FILE_DROPFILE );
}
