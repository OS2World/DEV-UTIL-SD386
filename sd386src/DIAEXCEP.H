/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   Diaexcep.h                                                              */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Data for exceptions dialog.                                             */
/*                                                                           */
/*...Release 1.01 (04/03/92)                                                 */
/*...                                                                        */
/*... 05/08/92  701   Srinivas  Cua Interface.                               */
/*****************************************************************************/

#define EXP_DLG_START_ROW         4
#define EXP_DLG_START_COL         16
#define EXP_DLG_LEN               18
#define EXP_DLG_WIDTH             48
#define EXP_DLG_BUTTONS           7
#define EXP_DLG_SKIPLINES_ATTOP   3
#define EXP_DLG_BUTTON_ROWS       3
#define EXP_DLG_NOOF_ENTRIES      MAXEXCEPTIONS
#define EXP_DLG_MAX_ROWS          10
#define EXP_DLG_BTN_ROW1          18
#define EXP_DLG_BTN_ROW2          20

#define EXP_DLG_BTN_ENTER_COL     EXP_DLG_START_COL + 17
#define EXP_DLG_BTN_ENTER_TEXT    "Enter"
#define EXP_DLG_BTN_ENTER_WIDTH   5
#define EXP_DLG_BTN_ENTER_KEY     ENTER

#define EXP_DLG_BTN_CANCEL_COL    EXP_DLG_START_COL + 27
#define EXP_DLG_BTN_CANCEL_TEXT   "Cancel"
#define EXP_DLG_BTN_CANCEL_WIDTH  6
#define EXP_DLG_BTN_CANCEL_KEY    ESC

#define EXP_DLG_BTN_HELP_COL      EXP_DLG_START_COL + 39
#define EXP_DLG_BTN_HELP_TEXT     "Help"
#define EXP_DLG_BTN_HELP_WIDTH    4
#define EXP_DLG_BTN_HELP_KEY      F1

#define EXP_DLG_BTN_NEXT_COL      EXP_DLG_START_COL + 5
#define EXP_DLG_BTN_NEXT_TEXT     "Next"
#define EXP_DLG_BTN_NEXT_WIDTH    4
#define EXP_DLG_BTN_NEXT_KEY      SPACEBAR

#define EXP_DLG_BTN_DEFAULT_COL   EXP_DLG_START_COL + 15
#define EXP_DLG_BTN_DEFAULT_TEXT  "Default"
#define EXP_DLG_BTN_DEFAULT_WIDTH 7
#define EXP_DLG_BTN_DEFAULT_KEY   key_D

#define EXP_DLG_BTN_RESET_COL     EXP_DLG_START_COL + 28
#define EXP_DLG_BTN_RESET_TEXT    "Reset"
#define EXP_DLG_BTN_RESET_WIDTH   5
#define EXP_DLG_BTN_RESET_KEY     key_R

#define EXP_DLG_BTN_SAVE_COL      EXP_DLG_START_COL + 39
#define EXP_DLG_BTN_SAVE_TEXT     "Save"
#define EXP_DLG_BTN_SAVE_WIDTH    4
#define EXP_DLG_BTN_SAVE_KEY      key_S

static uchar   Dia_Excep_Title[] = " Exceptions ";
static BUTTON  Dia_Excep_Buttons[] =
{
  {
    EXP_DLG_BTN_ROW2,
    EXP_DLG_BTN_ENTER_COL,
    EXP_DLG_BTN_ENTER_WIDTH,
    EXP_DLG_BTN_ENTER_TEXT,
    EXP_DLG_BTN_ENTER_KEY
  },
  {
    EXP_DLG_BTN_ROW2,
    EXP_DLG_BTN_CANCEL_COL,
    EXP_DLG_BTN_CANCEL_WIDTH,
    EXP_DLG_BTN_CANCEL_TEXT,
    EXP_DLG_BTN_CANCEL_KEY
  },
  {
    EXP_DLG_BTN_ROW2,
    EXP_DLG_BTN_HELP_COL,
    EXP_DLG_BTN_HELP_WIDTH,
    EXP_DLG_BTN_HELP_TEXT,
    EXP_DLG_BTN_HELP_KEY
  },
  {
    EXP_DLG_BTN_ROW1,
    EXP_DLG_BTN_NEXT_COL,
    EXP_DLG_BTN_NEXT_WIDTH,
    EXP_DLG_BTN_NEXT_TEXT,
    EXP_DLG_BTN_NEXT_KEY
  },
  {
    EXP_DLG_BTN_ROW1,
    EXP_DLG_BTN_DEFAULT_COL,
    EXP_DLG_BTN_DEFAULT_WIDTH,
    EXP_DLG_BTN_DEFAULT_TEXT,
    EXP_DLG_BTN_DEFAULT_KEY
  },
  {
    EXP_DLG_BTN_ROW1,
    EXP_DLG_BTN_RESET_COL,
    EXP_DLG_BTN_RESET_WIDTH,
    EXP_DLG_BTN_RESET_TEXT,
    EXP_DLG_BTN_RESET_KEY
  },
  {
    EXP_DLG_BTN_ROW1,
    EXP_DLG_BTN_SAVE_COL,
    EXP_DLG_BTN_SAVE_WIDTH,
    EXP_DLG_BTN_SAVE_TEXT,
    EXP_DLG_BTN_SAVE_KEY
  }
};

DIALOGSHELL  Dia_Excep =
{
  EXP_DLG_START_ROW,
  EXP_DLG_START_COL,
  EXP_DLG_LEN,
  EXP_DLG_WIDTH,
  EXP_DLG_BUTTONS,
  EXP_DLG_BUTTON_ROWS,
  Dia_Excep_Title,
  EXP_DLG_SKIPLINES_ATTOP,
  &Dia_Excep_Buttons[0],
  DisplayExcepChoice,
  ExcepDialogFunction
};

DIALOGCHOICE Dia_Excep_Choices =
{
  EXP_DLG_NOOF_ENTRIES,
  EXP_DLG_MAX_ROWS,
  0,                                    /* Not initialised in the begining.  */
  NULL                                  /* Not initialised in the begining.  */
};

