/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   popup.c                                                                 */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Common Cua routines to handle popups.                                   */
/*                                                                           */
/*...Release 1.01 (07/20/92)                                                 */
/*...                                                                        */
/*... 07/20/92  701   Joe       Cua Interface.                               */
/*... 12/10/93  910   Joe       Clear fields on cursor sensitive prompting.  */
/*****************************************************************************/
#include "all.h"
#include "popups.h"

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
extern KEY2FUNC defk2f[];               /* keys to functions map.            */


static BUTTON PopButton[] =
{
 {
  POP_BTN_ROW         ,
  POP_BTN_ENTER_COL   ,
  POP_BTN_ENTER_WIDTH ,
  POP_BTN_ENTER_TEXT  ,
  POP_BTN_ENTER_KEY
 },
 {
  POP_BTN_ROW         ,
  POP_BTN_CANCEL_COL  ,
  POP_BTN_CANCEL_WIDTH,
  POP_BTN_CANCEL_TEXT ,
  POP_BTN_CANCEL_KEY
 },
 {
  POP_BTN_ROW         ,
  POP_BTN_HELP_COL    ,
  POP_BTN_HELP_WIDTH  ,
  POP_BTN_HELP_TEXT   ,
  POP_BTN_HELP_KEY
 }
};

static POPUPSHELL Popup = {
                           POP_START_ROW   ,
                           POP_START_COL   ,
                           POP_LEN         ,
                           POP_WIDTH       ,
                           POP_BUTTONS     ,
                           NULL            ,
                           NULL            ,
                           0               ,
                           &PopButton[0]
                          };

/*****************************************************************************/
/* Strings used to display a pulldown.                                       */
/*****************************************************************************/
static UCHAR barstr[]    =  {V_BAR,0};

/*****************************************************************************/
/* Strings used to display a pop up.                                         */
/*****************************************************************************/
static UCHAR toppop[]  =  {TL_CORNER,RepCnt(1),H_BAR,TR_CORNER,0};
static UCHAR botpop[]  =  {BL_CORNER,RepCnt(1),H_BAR,BR_CORNER,0};
static UCHAR poptll[]  =  {L_JUNCTION,RepCnt(1),H_BAR,0};
static UCHAR poptlr[]  =  {RepCnt(1),H_BAR,R_JUNCTION,0};

/*****************************************************************************/
/*  DisplayPop()                                                             */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*     Displays a pop up window with title and buttons.                      */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   POPUPSHELL *ptr   ->  to a popup shell structure.                       */
/*                                                                           */
/* Return:                                                                   */
/*   None                                                                    */
/*****************************************************************************/
void DisplayPop(POPUPSHELL *ptr )
{                                       /*                                   */
  int    i;                             /* counter                           */
  int    tlen;                          /* title length                      */
  BUTTON *ButtonPtr;                    /* -> to button struture.            */

  /***************************************************************************/
  /* Get the Screen Access to write.                                         */
  /***************************************************************************/
  GetScrAccess();

  /***************************************************************************/
  /* - set the default video attribute.                                      */
  /* - initialize color attribute for the clear cell.                        */
  /***************************************************************************/
  VideoAtr      = vaMenuBar;
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
  /* put the instructions into the popup.                                    */
  /***************************************************************************/
  if (ptr->instructions)
    putrc(ptr->row + 3,
          ptr->col + ((ptr->width - strlen(ptr->instructions)) >> 1),
          ptr->instructions);

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
  /* Reset the video attributes to that of window and show the mouse         */
  /***************************************************************************/
  VideoAtr = vaMenuBar;
  /***************************************************************************/
  /* Restore Screen Access to write.                                         */
  /***************************************************************************/
  SetScrAccess();
}

/*****************************************************************************/
/*  GetPopStr()                                                              */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Gets a user input string from a pop up window and returns either an     */
/* ENTER or an ESCAPE key                                                    */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   POPUPSHELL  *shell     -> to a popup shell structure.                   */
/*   uint         row       row for user input                               */
/*   uint         col       column to start on                               */
/*   uint         length    length of input field                            */
/*   uchar       *str       -> to entered string                             */
/*                                                                           */
/* Return:                                                                   */
/*   The key code by which we are exiting.                                   */
/*****************************************************************************/
uint GetPopStr(POPUPSHELL *shell,uint row,uint col,
               uint length,uchar *str)
{
  uint cursor;                          /* cursor location in input field    */
  uint key;                             /* key pressed to end input          */
  uint help;                            /* help index.                       */

  /***************************************************************************/
  /*  - put brackets around the input field.                                 */
  /*  - put the initial string in the input field.                           */
  /***************************************************************************/
  putrc(row,col,"[");
  putrc(row,col + length - 1,"]");

  for(;;)
  {
    cursor = strlen( str );
    /*************************************************************************/
    /* get the string from the user.                                         */
    /*************************************************************************/
    key = GetString(row,col+1,PROMAX,length-2,&cursor,str,0,shell);     /*910*/

    switch( key )
    {
      /** Enter and Escape return the pressed key to caller **/
      case ENTER:
      case PADENTER:
      case ESC:
      /***********************************************************************/
      /* On a Enter and Escape key return the key and leading blank stripped */
      /* string.                                                             */
      /***********************************************************************/
        stripblk(str);
        return(key);

      case F1:
      /***********************************************************************/
      /* process the help key and then get input again.                      */
      /***********************************************************************/
      {
        uchar  *HelpMsg;

        help = shell->help;
        HelpMsg = GetHelpMsg( help, NULL,0 );
        CuaShowHelpBox( HelpMsg );
        break;
      }

      default:
      /***********************************************************************/
      /* unknown keys, beep and then try again.                              */
      /***********************************************************************/
        beep();
        break;
    }
  }
}

/*****************************************************************************/
/*  StripBlk()                                                               */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Removes leading blanks from a string                                    */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   uchar       *str       -> to string to be striped of leading blanks     */
/*                                                                           */
/* Return:                                                                   */
/*   None                                                                    */
/*****************************************************************************/
void stripblk (uchar *str)
{                                       /*                                   */
  uint   i;                             /* counter                           */
  uint   len;                           /* string length                     */
  uchar *ptr;                           /* work pointer                      */

  /***************************************************************************/
  /* If 1st char is not a blank, there are no leading blanks so return.      */
  /***************************************************************************/
  if (*str != ' ')
    return;

  /***************************************************************************/
  /* get pointer to the 1st non blank character.                             */
  /***************************************************************************/
  ptr = str;
  while (*ptr == ' ')
    ptr++;
  len = strlen(ptr) + 1;

  /***************************************************************************/
  /* copy the string back into the same string from starting.                */
  /***************************************************************************/
  for (i=0;i<len;i++)
    *(str+i) = *ptr++;
}


/*****************************************************************************/
/* GetButtonPtr()                                                         910*/
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Get a ptr to the button structure for the given popup and mouse         */
/*   event.                                                                  */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   shell       -> to a popup shell structure.                              */
/*   Event       mouse event.                                                */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   ButtonPtr   -> to a button structure.                                   */
/*   NULL           if the mouse event did not occur in the <>s.             */
/*                                                                           */
/*****************************************************************************/
BUTTON *GetButtonPtr(POPUPSHELL *shell, PEVENT Event)
{
  BUTTON *ButtonPtr;
  int     i;

  /***************************************************************************/
  /* Is click on the buttons row of the popup window.                        */
  /***************************************************************************/
  ButtonPtr = shell->Buttons;
  for (i = 0 ; i < shell->NoOfButtons ; i++)
  {
    /*************************************************************************/
    /* check if the click is in the button if yes return true and stuff      */
    /* in the key associated with that button.                               */
    /*************************************************************************/
    if ( (Event->Row == ButtonPtr->row) &&
         (Event->Col >= ButtonPtr->col-1) &&
         (Event->Col <= ButtonPtr->col+ButtonPtr->length))
       return(ButtonPtr);
    ButtonPtr++;
  }
  return(NULL);
}

/*****************************************************************************/
/* GetPopArea()                                                              */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Get the piece of the screen that the popup will overlay.                */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   SaveArea   -> to buffer to put the stuff in.                            */
/*                                                                           */
/* Return                                                                    */
/*                                                                           */
/*   none                                                                    */
/*                                                                           */
/*****************************************************************************/
void GetPopArea( char *SaveArea )
{
 windowsv(Popup.col,Popup.row,Popup.length,Popup.width,SaveArea);
}

/*****************************************************************************/
/* PutPopArea()                                                              */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Restore the piece of the screen grabbed previously.                     */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   SaveArea   -> to buffer to put the stuff in.                            */
/*                                                                           */
/* Return                                                                    */
/*                                                                           */
/*   none                                                                    */
/*                                                                           */
/*****************************************************************************/
void PutPopArea( char *SaveArea )
{
 windowrst(Popup.col,Popup.row,Popup.length,Popup.width,SaveArea);
}
