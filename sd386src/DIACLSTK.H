/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   Diaclstk.h                                                              */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Data for call stack dialog.                                             */
/*                                                                           */
/*...Release 1.01 (04/03/92)                                                 */
/*...                                                                        */
/*... 05/08/92  701   Srinivas  Cua Interface.                               */
/*****************************************************************************/

#define CLS_DLG_START_ROW         2
#define CLS_DLG_START_COL         22
#define CLS_DLG_LEN               18
#define CLS_DLG_WIDTH             36
#define CLS_DLG_BUTTONS           6
#define CLS_DLG_SKIPLINES_ATTOP   3
#define CLS_DLG_BUTTON_ROWS       3
#define CLS_DLG_NOOF_ENTRIES      0
#define CLS_DLG_MAX_ROWS          10
#define CLS_DLG_BTN_ROW1          16
#define CLS_DLG_BTN_ROW2          17
#define CLS_DLG_BTN_ROW3          18

#define CLS_DLG_BTN_GETVU_COL     CLS_DLG_START_COL + 5
#define CLS_DLG_BTN_GETVU_TEXT    "Get View"
#define CLS_DLG_BTN_GETVU_WIDTH   8
#define CLS_DLG_BTN_GETVU_KEY     ENTER

#define CLS_DLG_BTN_CANCEL_COL    CLS_DLG_START_COL + 5
#define CLS_DLG_BTN_CANCEL_TEXT   "Cancel"
#define CLS_DLG_BTN_CANCEL_WIDTH  6
#define CLS_DLG_BTN_CANCEL_KEY    ESC

#define CLS_DLG_BTN_SHALL_COL     CLS_DLG_START_COL + 5
#define CLS_DLG_BTN_SHALL_TEXT    "Show All"
#define CLS_DLG_BTN_SHALL_WIDTH   8
#define CLS_DLG_BTN_SHALL_KEY     key_a

#define CLS_DLG_BTN_HELP_COL      CLS_DLG_START_COL + 20
#define CLS_DLG_BTN_HELP_TEXT     "Help"
#define CLS_DLG_BTN_HELP_WIDTH    4
#define CLS_DLG_BTN_HELP_KEY      F1

#define CLS_DLG_BTN_SHNMD_COL     CLS_DLG_START_COL + 20
#define CLS_DLG_BTN_SHNMD_TEXT    "Show Named"
#define CLS_DLG_BTN_SHNMD_WIDTH   10
#define CLS_DLG_BTN_SHNMD_KEY     key_n

#define CLS_DLG_BTN_RUNTO_COL     CLS_DLG_START_COL + 20
#define CLS_DLG_BTN_RUNTO_TEXT    "Run To"
#define CLS_DLG_BTN_RUNTO_WIDTH   6
#define CLS_DLG_BTN_RUNTO_KEY     A_ENTER

static uchar   Dia_Clstk_Title[] = " Call Stack ";
static BUTTON  Dia_Clstk_Buttons[] =
{
  {
    CLS_DLG_BTN_ROW1,
    CLS_DLG_BTN_GETVU_COL,
    CLS_DLG_BTN_GETVU_WIDTH,
    CLS_DLG_BTN_GETVU_TEXT,
    CLS_DLG_BTN_GETVU_KEY
  },
  {
    CLS_DLG_BTN_ROW1,
    CLS_DLG_BTN_RUNTO_COL,
    CLS_DLG_BTN_RUNTO_WIDTH,
    CLS_DLG_BTN_RUNTO_TEXT,
    CLS_DLG_BTN_RUNTO_KEY
  },
  {
    CLS_DLG_BTN_ROW2,
    CLS_DLG_BTN_SHALL_COL,
    CLS_DLG_BTN_SHALL_WIDTH,
    CLS_DLG_BTN_SHALL_TEXT,
    CLS_DLG_BTN_SHALL_KEY
  },
  {
    CLS_DLG_BTN_ROW2,
    CLS_DLG_BTN_SHNMD_COL,
    CLS_DLG_BTN_SHNMD_WIDTH,
    CLS_DLG_BTN_SHNMD_TEXT,
    CLS_DLG_BTN_SHNMD_KEY
  },
  {
    CLS_DLG_BTN_ROW3,
    CLS_DLG_BTN_CANCEL_COL,
    CLS_DLG_BTN_CANCEL_WIDTH,
    CLS_DLG_BTN_CANCEL_TEXT,
    CLS_DLG_BTN_CANCEL_KEY
  },
  {
    CLS_DLG_BTN_ROW3,
    CLS_DLG_BTN_HELP_COL,
    CLS_DLG_BTN_HELP_WIDTH,
    CLS_DLG_BTN_HELP_TEXT,
    CLS_DLG_BTN_HELP_KEY
  }
};

DIALOGSHELL  Dia_Clstk =
{
  CLS_DLG_START_ROW,
  CLS_DLG_START_COL,
  CLS_DLG_LEN,
  CLS_DLG_WIDTH,
  CLS_DLG_BUTTONS,
  CLS_DLG_BUTTON_ROWS,
  Dia_Clstk_Title,
  CLS_DLG_SKIPLINES_ATTOP,
  &Dia_Clstk_Buttons[0],
  DisplayClstkChoice,
  ClstkDialogFunction
};

DIALOGCHOICE Dia_Clstk_Choices =
{
  CLS_DLG_NOOF_ENTRIES,
  CLS_DLG_MAX_ROWS,
  0,                                    /* Not initialised in the begining.  */
  NULL                                  /* Not initialised in the begining.  */
};

