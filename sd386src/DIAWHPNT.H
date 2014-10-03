/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   Diawhpnt.h                                                              */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Data for watch points dialog.                                           */
/*                                                                           */
/*...Release 1.01 (04/03/92)                                                 */
/*...                                                                        */
/*... 05/08/92  701   Srinivas  Cua Interface.                               */
/*****************************************************************************/

#define WPT_DLG_START_ROW         6
#define WPT_DLG_START_COL         0
#define WPT_DLG_LEN               12
#define WPT_DLG_WIDTH             80
#define WPT_DLG_BUTTONS           7
#define WPT_DLG_SKIPLINES_ATTOP   3
#define WPT_DLG_BUTTON_ROWS       4
#define WPT_DLG_NOOF_ENTRIES      4
#define WPT_DLG_MAX_ROWS          4
#define WPT_DLG_BTN_ROW1          14
#define WPT_DLG_BTN_ROW2          16

#define WPT_DLG_BTN_SZNEXT_COL    WPT_DLG_START_COL + 11
#define WPT_DLG_BTN_SZNEXT_TEXT   "Size"
#define WPT_DLG_BTN_SZNEXT_WIDTH  4
#define WPT_DLG_BTN_SZNEXT_KEY    SZNEXT

#define WPT_DLG_BTN_SPNEXT_COL    WPT_DLG_START_COL + 27
#define WPT_DLG_BTN_SPNEXT_TEXT   "Scope"
#define WPT_DLG_BTN_SPNEXT_WIDTH  5
#define WPT_DLG_BTN_SPNEXT_KEY    SPNEXT

#define WPT_DLG_BTN_TYNEXT_COL    WPT_DLG_START_COL + 45
#define WPT_DLG_BTN_TYNEXT_TEXT   "Type"
#define WPT_DLG_BTN_TYNEXT_WIDTH  4
#define WPT_DLG_BTN_TYNEXT_KEY    TYNEXT

#define WPT_DLG_BTN_STATUS_COL    WPT_DLG_START_COL + 61
#define WPT_DLG_BTN_STATUS_TEXT   "Status"
#define WPT_DLG_BTN_STATUS_WIDTH  6
#define WPT_DLG_BTN_STATUS_KEY    STNEXT

#define WPT_DLG_BTN_ENTER_COL     WPT_DLG_START_COL + 27
#define WPT_DLG_BTN_ENTER_TEXT    "Enter"
#define WPT_DLG_BTN_ENTER_WIDTH   5
#define WPT_DLG_BTN_ENTER_KEY     ENTER

#define WPT_DLG_BTN_CANCEL_COL    WPT_DLG_START_COL + 43
#define WPT_DLG_BTN_CANCEL_TEXT   "Cancel"
#define WPT_DLG_BTN_CANCEL_WIDTH  6
#define WPT_DLG_BTN_CANCEL_KEY    ESC

#define WPT_DLG_BTN_HELP_COL      WPT_DLG_START_COL + 63
#define WPT_DLG_BTN_HELP_TEXT     "Help"
#define WPT_DLG_BTN_HELP_WIDTH    4
#define WPT_DLG_BTN_HELP_KEY      F1

static uchar   Dia_WhPnt_Title[]   = " Watch Points ";
static BUTTON  Dia_WhPnt_Buttons[] =
{
  {
    WPT_DLG_BTN_ROW1,
    WPT_DLG_BTN_SZNEXT_COL,
    WPT_DLG_BTN_SZNEXT_WIDTH,
    WPT_DLG_BTN_SZNEXT_TEXT,
    WPT_DLG_BTN_SZNEXT_KEY
  },
  {
    WPT_DLG_BTN_ROW1,
    WPT_DLG_BTN_SPNEXT_COL,
    WPT_DLG_BTN_SPNEXT_WIDTH,
    WPT_DLG_BTN_SPNEXT_TEXT,
    WPT_DLG_BTN_SPNEXT_KEY
  },
  {
    WPT_DLG_BTN_ROW1,
    WPT_DLG_BTN_TYNEXT_COL,
    WPT_DLG_BTN_TYNEXT_WIDTH,
    WPT_DLG_BTN_TYNEXT_TEXT,
    WPT_DLG_BTN_TYNEXT_KEY
  },
  {
    WPT_DLG_BTN_ROW1,
    WPT_DLG_BTN_STATUS_COL,
    WPT_DLG_BTN_STATUS_WIDTH,
    WPT_DLG_BTN_STATUS_TEXT,
    WPT_DLG_BTN_STATUS_KEY
  },
  {
    WPT_DLG_BTN_ROW2,
    WPT_DLG_BTN_ENTER_COL,
    WPT_DLG_BTN_ENTER_WIDTH,
    WPT_DLG_BTN_ENTER_TEXT,
    WPT_DLG_BTN_ENTER_KEY
  },
  {
    WPT_DLG_BTN_ROW2,
    WPT_DLG_BTN_CANCEL_COL,
    WPT_DLG_BTN_CANCEL_WIDTH,
    WPT_DLG_BTN_CANCEL_TEXT,
    WPT_DLG_BTN_CANCEL_KEY
  },
  {
    WPT_DLG_BTN_ROW2,
    WPT_DLG_BTN_HELP_COL,
    WPT_DLG_BTN_HELP_WIDTH,
    WPT_DLG_BTN_HELP_TEXT,
    WPT_DLG_BTN_HELP_KEY
  }
};

DIALOGSHELL  Dia_WhPnt =
{
  WPT_DLG_START_ROW,
  WPT_DLG_START_COL,
  WPT_DLG_LEN,
  WPT_DLG_WIDTH,
  WPT_DLG_BUTTONS,
  WPT_DLG_BUTTON_ROWS,
  Dia_WhPnt_Title,
  WPT_DLG_SKIPLINES_ATTOP,
  &Dia_WhPnt_Buttons[0],
  DisplayWhPntChoice
};

DIALOGCHOICE Dia_WhPnt_Choices =
{
  WPT_DLG_NOOF_ENTRIES,
  WPT_DLG_MAX_ROWS,
  0,                                    /* Not initialised in the begining.  */
  NULL                                  /* Not initialised in the begining.  */
};

