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
#define MAXSHDLLWINSIZE 60

#define DLL_DLG_START_ROW         4
#define DLL_DLG_START_COL         10
#define DLL_DLG_LEN               16
#define DLL_DLG_WIDTH             MAXSHDLLWINSIZE
#define DLL_DLG_BUTTONS           1
#define DLL_DLG_SKIPLINES_ATTOP   3
#define DLL_DLG_BUTTON_ROWS       1
#define DLL_DLG_NOOF_ENTRIES      0
#define DLL_DLG_MAX_ROWS          10
#define DLL_DLG_BTN_ROW           18

#define DLL_DLG_BTN_CANCEL_COL    DLL_DLG_START_COL + 15
#define DLL_DLG_BTN_CANCEL_TEXT   "Cancel"
#define DLL_DLG_BTN_CANCEL_WIDTH  6
#define DLL_DLG_BTN_CANCEL_KEY    ESC

static uchar   Dia_Dll_Title[]   = " DLL List ";
static BUTTON  Dia_Dll_Buttons[] =
{
  {
    DLL_DLG_BTN_ROW,
    DLL_DLG_BTN_CANCEL_COL,
    DLL_DLG_BTN_CANCEL_WIDTH,
    DLL_DLG_BTN_CANCEL_TEXT,
    DLL_DLG_BTN_CANCEL_KEY
  }
};

DIALOGSHELL  Dia_Dll =
{
  DLL_DLG_START_ROW,
  DLL_DLG_START_COL,
  DLL_DLG_LEN,
  DLL_DLG_WIDTH,
  DLL_DLG_BUTTONS,
  DLL_DLG_BUTTON_ROWS,
  Dia_Dll_Title,
  DLL_DLG_SKIPLINES_ATTOP,
  &Dia_Dll_Buttons[0],
  DisplayDllChoice,
  DllDialogFunction
};

DIALOGCHOICE Dia_Dll_Choices =
{
  DLL_DLG_NOOF_ENTRIES,
  DLL_DLG_MAX_ROWS,
  0,                                    /* Not initialised in the begining.  */
  NULL                                  /* Not initialised in the begining.  */
};
