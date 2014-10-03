/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   Diadll.h                                                                */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Data for dlls dialog.                                                   */
/*                                                                           */
/*...Release 1.01 (04/03/92)                                                 */
/*...                                                                        */
/*... 05/08/92  701   Srinivas  Cua Interface.                               */
/*****************************************************************************/

#define HELPCOLS   35                   /* number of cols in our help box    */

#define HLP_DLG_START_ROW         4
#define HLP_DLG_START_COL         21
#define HLP_DLG_LEN               16
#define HLP_DLG_WIDTH             HELPCOLS + 4
#define HLP_DLG_BUTTONS           3
#define HLP_DLG_SKIPLINES_ATTOP   3
#define HLP_DLG_BUTTON_ROWS       1
#define HLP_DLG_NOOF_ENTRIES      USERKEYS
#define HLP_DLG_MAX_ROWS          10
#define HLP_DLG_BTN_ROW           18

#define HLP_DLG_BTN_ENTER_COL     HLP_DLG_START_COL + 5
#define HLP_DLG_BTN_ENTER_TEXT    "Enter"
#define HLP_DLG_BTN_ENTER_WIDTH   5
#define HLP_DLG_BTN_ENTER_KEY     ENTER

#define HLP_DLG_BTN_CANCEL_COL    HLP_DLG_START_COL + 16
#define HLP_DLG_BTN_CANCEL_TEXT   "Cancel"
#define HLP_DLG_BTN_CANCEL_WIDTH  6
#define HLP_DLG_BTN_CANCEL_KEY    ESC

#define HLP_DLG_BTN_HELP_COL      HLP_DLG_START_COL + 28
#define HLP_DLG_BTN_HELP_TEXT     "Help"
#define HLP_DLG_BTN_HELP_WIDTH    4
#define HLP_DLG_BTN_HELP_KEY      F1

static uchar   Dia_Help_Title[]   = " Keys help ";
static BUTTON  Dia_Help_Buttons[] =
{
  {
    HLP_DLG_BTN_ROW,
    HLP_DLG_BTN_ENTER_COL,
    HLP_DLG_BTN_ENTER_WIDTH,
    HLP_DLG_BTN_ENTER_TEXT,
    HLP_DLG_BTN_ENTER_KEY
  },
  {
    HLP_DLG_BTN_ROW,
    HLP_DLG_BTN_CANCEL_COL,
    HLP_DLG_BTN_CANCEL_WIDTH,
    HLP_DLG_BTN_CANCEL_TEXT,
    HLP_DLG_BTN_CANCEL_KEY
  },
  {
    HLP_DLG_BTN_ROW,
    HLP_DLG_BTN_HELP_COL,
    HLP_DLG_BTN_HELP_WIDTH,
    HLP_DLG_BTN_HELP_TEXT,
    HLP_DLG_BTN_HELP_KEY
  }
};

DIALOGSHELL  Dia_Help =
{
  HLP_DLG_START_ROW,
  HLP_DLG_START_COL,
  HLP_DLG_LEN,
  HLP_DLG_WIDTH,
  HLP_DLG_BUTTONS,
  HLP_DLG_BUTTON_ROWS,
  Dia_Help_Title,
  HLP_DLG_SKIPLINES_ATTOP,
  &Dia_Help_Buttons[0],
  DisplayHelpChoice,
  HelpDialogFunction
};

DIALOGCHOICE Dia_Help_Choices =
{
  HLP_DLG_NOOF_ENTRIES,
  HLP_DLG_MAX_ROWS,
  0,                                    /* Not initialised in the begining.  */
  NULL                                  /* Not initialised in the begining.  */
};
