/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   diaproc.h                                                               */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Data for process dialog.                                                */
/*                                                                           */
/*...Created 08/29/94                                                        */
/*...                                                                        */
/*****************************************************************************/

#define PRC_DLG_START_ROW         6
#define PRC_DLG_START_COL         7
#define PRC_DLG_LEN               11
#define PRC_DLG_WIDTH             67
#define PRC_DLG_BUTTONS           3
#define PRC_DLG_SKIPLINES_ATTOP   3
#define PRC_DLG_BUTTON_ROWS       1
#define PRC_DLG_NOOF_ENTRIES      0
#define PRC_DLG_MAX_ROWS          5
#define PRC_DLG_BTN_ROW1          15

#define PRC_DLG_BTN_ENTER_COL     PRC_DLG_START_COL + 4
#define PRC_DLG_BTN_ENTER_TEXT    "Enter"
#define PRC_DLG_BTN_ENTER_WIDTH   5
#define PRC_DLG_BTN_ENTER_KEY     ENTER

#define PRC_DLG_BTN_CANCEL_COL    PRC_DLG_START_COL + 14
#define PRC_DLG_BTN_CANCEL_TEXT   "Cancel"
#define PRC_DLG_BTN_CANCEL_WIDTH  6
#define PRC_DLG_BTN_CANCEL_KEY    ESC

#define PRC_DLG_BTN_HELP_COL      PRC_DLG_START_COL + 24
#define PRC_DLG_BTN_HELP_TEXT     "Help"
#define PRC_DLG_BTN_HELP_WIDTH    4
#define PRC_DLG_BTN_HELP_KEY      F1

static uchar   Dia_Proc_Title[] = " Processes ";
static BUTTON  Dia_Proc_Buttons[] =
{
  {
    PRC_DLG_BTN_ROW1,
    PRC_DLG_BTN_ENTER_COL,
    PRC_DLG_BTN_ENTER_WIDTH,
    PRC_DLG_BTN_ENTER_TEXT,
    PRC_DLG_BTN_ENTER_KEY
  },
  {
    PRC_DLG_BTN_ROW1,
    PRC_DLG_BTN_CANCEL_COL,
    PRC_DLG_BTN_CANCEL_WIDTH,
    PRC_DLG_BTN_CANCEL_TEXT,
    PRC_DLG_BTN_CANCEL_KEY
  },
  {
    PRC_DLG_BTN_ROW1,
    PRC_DLG_BTN_HELP_COL,
    PRC_DLG_BTN_HELP_WIDTH,
    PRC_DLG_BTN_HELP_TEXT,
    PRC_DLG_BTN_HELP_KEY
  }
};

DIALOGSHELL  Dia_Proc =
{
  PRC_DLG_START_ROW,
  PRC_DLG_START_COL,
  PRC_DLG_LEN,
  PRC_DLG_WIDTH,
  PRC_DLG_BUTTONS,
  PRC_DLG_BUTTON_ROWS,
  Dia_Proc_Title,
  PRC_DLG_SKIPLINES_ATTOP,
  &Dia_Proc_Buttons[0],
  DisplayProcChoice,
  ProcessDialogFunction
};

DIALOGCHOICE Dia_Proc_Choices =
{
  PRC_DLG_NOOF_ENTRIES,
  PRC_DLG_MAX_ROWS,
  0,                                    /* Not initialised in the begining.  */
  NULL                                  /* Not initialised in the begining.  */
};

