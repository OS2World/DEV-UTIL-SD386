/*****************************************************************************/
/* File:                                                                     */
/*   DiaBkPnt.h                                                              */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Data for watch points dialog.                                           */
/*                                                                           */
/*...Revised (06/13/95)                                                      */
/*...                                                                        */
/*****************************************************************************/

#define BKP_DLG_START_ROW         14
#define BKP_DLG_START_COL         19
#define BKP_DLG_LEN               0
#define BKP_DLG_WIDTH             60
#define BKP_DLG_BUTTONS           2
#define BKP_DLG_SKIPLINES_ATTOP   3
#define BKP_DLG_BUTTON_ROWS       1
#define BKP_DLG_NOOF_ENTRIES      3
#define BKP_DLG_MAX_ROWS          3
#define BKP_DLG_BTN_ROW           BKP_DLG_START_ROW + 7

#define BKP_DLG_BTN_ENTER_COL     BKP_DLG_START_COL + 9
#define BKP_DLG_BTN_ENTER_TEXT    "Yes"
#define BKP_DLG_BTN_ENTER_WIDTH   3
#define BKP_DLG_BTN_ENTER_KEY     key_y

#define BKP_DLG_BTN_CANCEL_COL    BKP_DLG_START_COL + 20
#define BKP_DLG_BTN_CANCEL_TEXT   "No"
#define BKP_DLG_BTN_CANCEL_WIDTH  2
#define BKP_DLG_BTN_CANCEL_KEY    key_n

#define BKP_DLG_BTN_HELP_COL      BKP_DLG_START_COL + 32
#define BKP_DLG_BTN_HELP_TEXT     "Help"
#define BKP_DLG_BTN_HELP_WIDTH    4
#define BKP_DLG_BTN_HELP_KEY      F1

static uchar   Dia_BkPnt_Title[]   = " Do you want to defer this breakpoint? ";
static BUTTON  Dia_BkPnt_Buttons[] =
{
  {
    BKP_DLG_BTN_ROW,
    BKP_DLG_BTN_ENTER_COL,
    BKP_DLG_BTN_ENTER_WIDTH,
    BKP_DLG_BTN_ENTER_TEXT,
    BKP_DLG_BTN_ENTER_KEY
  },
  {
    BKP_DLG_BTN_ROW,
    BKP_DLG_BTN_CANCEL_COL,
    BKP_DLG_BTN_CANCEL_WIDTH,
    BKP_DLG_BTN_CANCEL_TEXT,
    BKP_DLG_BTN_CANCEL_KEY
  },
  {
    BKP_DLG_BTN_ROW,
    BKP_DLG_BTN_HELP_COL,
    BKP_DLG_BTN_HELP_WIDTH,
    BKP_DLG_BTN_HELP_TEXT,
    BKP_DLG_BTN_HELP_KEY
  }
};

DIALOGSHELL  Dia_BkPnt =
{
  BKP_DLG_START_ROW,
  BKP_DLG_START_COL,
  BKP_DLG_LEN,
  BKP_DLG_WIDTH,
  BKP_DLG_BUTTONS,
  BKP_DLG_BUTTON_ROWS,
  Dia_BkPnt_Title,
  BKP_DLG_SKIPLINES_ATTOP,
  &Dia_BkPnt_Buttons[0],
  DisplayBkPntChoice,
  NULL
/*BkPntDialogFunction */
};

DIALOGCHOICE Dia_BkPnt_Choices =
{
  BKP_DLG_NOOF_ENTRIES,
  BKP_DLG_MAX_ROWS,
  0,                                    /* Not initialised in the begining.  */
  NULL                                  /* Not initialised in the begining.  */
};
