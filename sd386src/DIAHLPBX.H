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
#define HLPBOX_DLG_START_ROW         4
#define HLPBOX_DLG_START_COL         21
#define HLPBOX_DLG_LEN               16
#define HLPBOX_DLG_WIDTH             0
#define HLPBOX_DLG_BUTTONS           1
#define HLPBOX_DLG_SKIPLINES_ATTOP   3
#define HLPBOX_DLG_BUTTON_ROWS       1
#define HLPBOX_DLG_NOOF_ENTRIES      0
#define HLPBOX_DLG_MAX_ROWS          10
#define HLPBOX_DLG_BTN_ROW           18

#define HLPBOX_DLG_BTN_ENTER_COL     HLPBOX_DLG_START_COL + 11
#define HLPBOX_DLG_BTN_ENTER_TEXT    " Ok "
#define HLPBOX_DLG_BTN_ENTER_WIDTH   4
#define HLPBOX_DLG_BTN_ENTER_KEY     ENTER

static uchar   Dia_HelpBox_Title[]   = " Help ";
static BUTTON  Dia_HelpBox_Buttons[] =
{
  {
    HLPBOX_DLG_BTN_ROW,
    HLPBOX_DLG_BTN_ENTER_COL,
    HLPBOX_DLG_BTN_ENTER_WIDTH,
    HLPBOX_DLG_BTN_ENTER_TEXT,
    HLPBOX_DLG_BTN_ENTER_KEY
  }
};

DIALOGSHELL  Dia_HelpBox =
{
  HLPBOX_DLG_START_ROW,
  HLPBOX_DLG_START_COL,
  HLPBOX_DLG_LEN,
  HLPBOX_DLG_WIDTH,
  HLPBOX_DLG_BUTTONS,
  HLPBOX_DLG_BUTTON_ROWS,
  Dia_HelpBox_Title,
  HLPBOX_DLG_SKIPLINES_ATTOP,
  &Dia_HelpBox_Buttons[0],
  DisplayHelpBoxText,
  HelpBoxDialogFunction
};

DIALOGCHOICE Dia_HelpBox_Choices =
{
  HLPBOX_DLG_NOOF_ENTRIES,
  HLPBOX_DLG_MAX_ROWS,
  0,                                    /* Not initialised in the begining.  */
  NULL                                  /* Not initialised in the begining.  */
};

