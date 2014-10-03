/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   popups.h                                                                */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Definitions for popups.                                                 */
/*                                                                           */
/*****************************************************************************/

#define  MAXPROMPTSTRING           81

#define  POP_START_ROW             8
#define  POP_START_COL            16
#define  POP_LEN                   9
#define  POP_WIDTH                46
#define  POP_BUTTONS               3
#define  POP_BUTTON_ROWS           1

#define  POP_PROMPT_START_ROW     POP_START_ROW + 5
#define  POP_PROMPT_START_COL     POP_START_COL + 2
#define  POP_PROMPT_LENGTH        POP_WIDTH     - 4

#define  POP_BTN_ROW              POP_START_ROW +7

#define  POP_BTN_ENTER_COL        POP_START_COL + 11
#define  POP_BTN_ENTER_TEXT       "Enter"
#define  POP_BTN_ENTER_WIDTH       5
#define  POP_BTN_ENTER_KEY        ENTER

#define  POP_BTN_CANCEL_COL       POP_START_COL + 21
#define  POP_BTN_CANCEL_TEXT      "Cancel"
#define  POP_BTN_CANCEL_WIDTH      6
#define  POP_BTN_CANCEL_KEY       ESC

#define  POP_BTN_HELP_COL         POP_START_COL + 31
#define  POP_BTN_HELP_TEXT        "Help"
#define  POP_BTN_HELP_WIDTH        4
#define  POP_BTN_HELP_KEY         F1
