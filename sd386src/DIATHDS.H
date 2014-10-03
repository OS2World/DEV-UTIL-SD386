/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   Diathds.h                                                               */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Data for threads dialog.                                                */
/*                                                                           */
/*...Release 1.01 (04/03/92)                                                 */
/*...                                                                        */
/*... 05/08/92  701   Srinivas  Cua Interface.                               */
/*****************************************************************************/

#define TRD_DLG_START_ROW         6
#define TRD_DLG_START_COL         7
#define TRD_DLG_LEN               13
#define TRD_DLG_WIDTH             67
#define TRD_DLG_BUTTONS           9
#define TRD_DLG_SKIPLINES_ATTOP   3
#define TRD_DLG_BUTTON_ROWS       3
#define TRD_DLG_NOOF_ENTRIES      0
#define TRD_DLG_MAX_ROWS          5
#define TRD_DLG_BTN_ROW1          15
#define TRD_DLG_BTN_ROW2          17

#define TRD_DLG_BTN_ENTER_COL     TRD_DLG_START_COL + 32
#define TRD_DLG_BTN_ENTER_TEXT    "Enter"
#define TRD_DLG_BTN_ENTER_WIDTH   5
#define TRD_DLG_BTN_ENTER_KEY     ENTER

#define TRD_DLG_BTN_CANCEL_COL    TRD_DLG_START_COL + 43
#define TRD_DLG_BTN_CANCEL_TEXT   "Cancel"
#define TRD_DLG_BTN_CANCEL_WIDTH  6
#define TRD_DLG_BTN_CANCEL_KEY    ESC

#define TRD_DLG_BTN_HELP_COL      TRD_DLG_START_COL + 55
#define TRD_DLG_BTN_HELP_TEXT     "Help"
#define TRD_DLG_BTN_HELP_WIDTH    4
#define TRD_DLG_BTN_HELP_KEY      F1

#define TRD_DLG_BTN_SELECT_COL    TRD_DLG_START_COL + 4
#define TRD_DLG_BTN_SELECT_TEXT   "Select"
#define TRD_DLG_BTN_SELECT_WIDTH  6
#define TRD_DLG_BTN_SELECT_KEY    SPACEBAR

#define TRD_DLG_BTN_NAMES_COL     TRD_DLG_START_COL + 14
#define TRD_DLG_BTN_NAMES_TEXT    "Names"
#define TRD_DLG_BTN_NAMES_WIDTH   5
#define TRD_DLG_BTN_NAMES_KEY     key_A

#define TRD_DLG_BTN_FREEZE_COL    TRD_DLG_START_COL + 23
#define TRD_DLG_BTN_FREEZE_TEXT   "Freeze"
#define TRD_DLG_BTN_FREEZE_WIDTH  6
#define TRD_DLG_BTN_FREEZE_KEY    key_F

#define TRD_DLG_BTN_THAW_COL      TRD_DLG_START_COL + 33
#define TRD_DLG_BTN_THAW_TEXT     "Thaw"
#define TRD_DLG_BTN_THAW_WIDTH    4
#define TRD_DLG_BTN_THAW_KEY      key_T

#define TRD_DLG_BTN_AFREEZE_COL   TRD_DLG_START_COL + 41
#define TRD_DLG_BTN_AFREEZE_TEXT  "Freeze All"
#define TRD_DLG_BTN_AFREEZE_WIDTH 10
#define TRD_DLG_BTN_AFREEZE_KEY   key_R

#define TRD_DLG_BTN_ATHAW_COL     TRD_DLG_START_COL + 55
#define TRD_DLG_BTN_ATHAW_TEXT    "Thaw All"
#define TRD_DLG_BTN_ATHAW_WIDTH   8
#define TRD_DLG_BTN_ATHAW_KEY     key_H

static uchar   Dia_Thds_Title[] = " Threads ";
static BUTTON  Dia_Thds_Buttons[] =
{
  {
    TRD_DLG_BTN_ROW2,
    TRD_DLG_BTN_ENTER_COL,
    TRD_DLG_BTN_ENTER_WIDTH,
    TRD_DLG_BTN_ENTER_TEXT,
    TRD_DLG_BTN_ENTER_KEY
  },
  {
    TRD_DLG_BTN_ROW2,
    TRD_DLG_BTN_CANCEL_COL,
    TRD_DLG_BTN_CANCEL_WIDTH,
    TRD_DLG_BTN_CANCEL_TEXT,
    TRD_DLG_BTN_CANCEL_KEY
  },
  {
    TRD_DLG_BTN_ROW2,
    TRD_DLG_BTN_HELP_COL,
    TRD_DLG_BTN_HELP_WIDTH,
    TRD_DLG_BTN_HELP_TEXT,
    TRD_DLG_BTN_HELP_KEY
  },
  {
    TRD_DLG_BTN_ROW1,
    TRD_DLG_BTN_SELECT_COL,
    TRD_DLG_BTN_SELECT_WIDTH,
    TRD_DLG_BTN_SELECT_TEXT,
    TRD_DLG_BTN_SELECT_KEY
  },
  {
    TRD_DLG_BTN_ROW1,
    TRD_DLG_BTN_NAMES_COL,
    TRD_DLG_BTN_NAMES_WIDTH,
    TRD_DLG_BTN_NAMES_TEXT,
    TRD_DLG_BTN_NAMES_KEY
  },
  {
    TRD_DLG_BTN_ROW1,
    TRD_DLG_BTN_FREEZE_COL,
    TRD_DLG_BTN_FREEZE_WIDTH,
    TRD_DLG_BTN_FREEZE_TEXT,
    TRD_DLG_BTN_FREEZE_KEY
  },
  {
    TRD_DLG_BTN_ROW1,
    TRD_DLG_BTN_THAW_COL,
    TRD_DLG_BTN_THAW_WIDTH,
    TRD_DLG_BTN_THAW_TEXT,
    TRD_DLG_BTN_THAW_KEY
  },
  {
    TRD_DLG_BTN_ROW1,
    TRD_DLG_BTN_AFREEZE_COL,
    TRD_DLG_BTN_AFREEZE_WIDTH,
    TRD_DLG_BTN_AFREEZE_TEXT,
    TRD_DLG_BTN_AFREEZE_KEY
  },
  {
    TRD_DLG_BTN_ROW1,
    TRD_DLG_BTN_ATHAW_COL,
    TRD_DLG_BTN_ATHAW_WIDTH,
    TRD_DLG_BTN_ATHAW_TEXT,
    TRD_DLG_BTN_ATHAW_KEY
  }
};

DIALOGSHELL  Dia_Thds =
{
  TRD_DLG_START_ROW,
  TRD_DLG_START_COL,
  TRD_DLG_LEN,
  TRD_DLG_WIDTH,
  TRD_DLG_BUTTONS,
  TRD_DLG_BUTTON_ROWS,
  Dia_Thds_Title,
  TRD_DLG_SKIPLINES_ATTOP,
  &Dia_Thds_Buttons[0],
  DisplayThdsChoice,
  ThreadsDialogFunction
};

DIALOGCHOICE Dia_Thds_Choices =
{
  TRD_DLG_NOOF_ENTRIES,
  TRD_DLG_MAX_ROWS,
  0,                                    /* Not initialised in the begining.  */
  NULL                                  /* Not initialised in the begining.  */
};

