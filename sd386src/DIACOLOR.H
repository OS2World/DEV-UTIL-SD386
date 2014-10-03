/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   Diacolor.h                                                              */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Data for colors dialog.                                                 */
/*                                                                           */
/*...Release 1.01 (04/03/92)                                                 */
/*...                                                                        */
/*... 05/08/92  701   Srinivas  Cua Interface.                               */
/*****************************************************************************/

#define CLR_DLG_START_ROW         3
#define CLR_DLG_START_COL         9
#define CLR_DLG_LEN               18
#define CLR_DLG_WIDTH             62
#define CLR_DLG_BUTTONS           8
#define CLR_DLG_SKIPLINES_ATTOP   3
#define CLR_DLG_BUTTON_ROWS       3
#define CLR_DLG_NOOF_ENTRIES      MAXVIDEOATR
#define CLR_DLG_MAX_ROWS          10
#define CLR_DLG_BTN_ROW1          17
#define CLR_DLG_BTN_ROW2          19

#define CLR_DLG_BTN_ENTER_COL     CLR_DLG_START_COL + 30
#define CLR_DLG_BTN_ENTER_TEXT    "Enter"
#define CLR_DLG_BTN_ENTER_WIDTH   5
#define CLR_DLG_BTN_ENTER_KEY     ENTER

#define CLR_DLG_BTN_CANCEL_COL    CLR_DLG_START_COL + 41
#define CLR_DLG_BTN_CANCEL_TEXT   "Cancel"
#define CLR_DLG_BTN_CANCEL_WIDTH  6
#define CLR_DLG_BTN_CANCEL_KEY    ESC

#define CLR_DLG_BTN_HELP_COL      CLR_DLG_START_COL + 53
#define CLR_DLG_BTN_HELP_TEXT     "Help"
#define CLR_DLG_BTN_HELP_WIDTH    4
#define CLR_DLG_BTN_HELP_KEY      F1

#define CLR_DLG_BTN_BGNEXT_COL    CLR_DLG_START_COL + 5
#define CLR_DLG_BTN_BGNEXT_TEXT   "BgNext"
#define CLR_DLG_BTN_BGNEXT_WIDTH  6
#define CLR_DLG_BTN_BGNEXT_KEY    BGNEXT

#define CLR_DLG_BTN_FGNEXT_COL    CLR_DLG_START_COL + 17
#define CLR_DLG_BTN_FGNEXT_TEXT   "FgNext"
#define CLR_DLG_BTN_FGNEXT_WIDTH  6
#define CLR_DLG_BTN_FGNEXT_KEY    FGNEXT

#define CLR_DLG_BTN_DEFAULT_COL   CLR_DLG_START_COL + 29
#define CLR_DLG_BTN_DEFAULT_TEXT  "Default"
#define CLR_DLG_BTN_DEFAULT_WIDTH 7
#define CLR_DLG_BTN_DEFAULT_KEY   key_D

#define CLR_DLG_BTN_RESET_COL     CLR_DLG_START_COL + 42
#define CLR_DLG_BTN_RESET_TEXT    "Reset"
#define CLR_DLG_BTN_RESET_WIDTH   5
#define CLR_DLG_BTN_RESET_KEY     key_R

#define CLR_DLG_BTN_SAVE_COL      CLR_DLG_START_COL + 53
#define CLR_DLG_BTN_SAVE_TEXT     "Save"
#define CLR_DLG_BTN_SAVE_WIDTH    4
#define CLR_DLG_BTN_SAVE_KEY      key_S

static uchar   Dia_Color_Title[] = " Colors ";
static BUTTON  Dia_Color_Buttons[] =
{
  {
    CLR_DLG_BTN_ROW2,
    CLR_DLG_BTN_ENTER_COL,
    CLR_DLG_BTN_ENTER_WIDTH,
    CLR_DLG_BTN_ENTER_TEXT,
    CLR_DLG_BTN_ENTER_KEY
  },
  {
    CLR_DLG_BTN_ROW2,
    CLR_DLG_BTN_CANCEL_COL,
    CLR_DLG_BTN_CANCEL_WIDTH,
    CLR_DLG_BTN_CANCEL_TEXT,
    CLR_DLG_BTN_CANCEL_KEY
  },
  {
    CLR_DLG_BTN_ROW2,
    CLR_DLG_BTN_HELP_COL,
    CLR_DLG_BTN_HELP_WIDTH,
    CLR_DLG_BTN_HELP_TEXT,
    CLR_DLG_BTN_HELP_KEY
  },
  {
    CLR_DLG_BTN_ROW1,
    CLR_DLG_BTN_BGNEXT_COL,
    CLR_DLG_BTN_BGNEXT_WIDTH,
    CLR_DLG_BTN_BGNEXT_TEXT,
    CLR_DLG_BTN_BGNEXT_KEY
  },
  {
    CLR_DLG_BTN_ROW1,
    CLR_DLG_BTN_FGNEXT_COL,
    CLR_DLG_BTN_FGNEXT_WIDTH,
    CLR_DLG_BTN_FGNEXT_TEXT,
    CLR_DLG_BTN_FGNEXT_KEY
  },
  {
    CLR_DLG_BTN_ROW1,
    CLR_DLG_BTN_DEFAULT_COL,
    CLR_DLG_BTN_DEFAULT_WIDTH,
    CLR_DLG_BTN_DEFAULT_TEXT,
    CLR_DLG_BTN_DEFAULT_KEY
  },
  {
    CLR_DLG_BTN_ROW1,
    CLR_DLG_BTN_RESET_COL,
    CLR_DLG_BTN_RESET_WIDTH,
    CLR_DLG_BTN_RESET_TEXT,
    CLR_DLG_BTN_RESET_KEY
  },
  {
    CLR_DLG_BTN_ROW1,
    CLR_DLG_BTN_SAVE_COL,
    CLR_DLG_BTN_SAVE_WIDTH,
    CLR_DLG_BTN_SAVE_TEXT,
    CLR_DLG_BTN_SAVE_KEY
  }
};

DIALOGSHELL  Dia_Color =
{
  CLR_DLG_START_ROW,
  CLR_DLG_START_COL,
  CLR_DLG_LEN,
  CLR_DLG_WIDTH,
  CLR_DLG_BUTTONS,
  CLR_DLG_BUTTON_ROWS,
  Dia_Color_Title,
  CLR_DLG_SKIPLINES_ATTOP,
  &Dia_Color_Buttons[0],
  DisplayColorChoice,
  ColorDialogFunction
};

DIALOGCHOICE Dia_Color_Choices =
{
  CLR_DLG_NOOF_ENTRIES,
  CLR_DLG_MAX_ROWS,
  0,                                    /* Not initialised in the begining.  */
  NULL                                  /* Not initialised in the begining.  */
};
