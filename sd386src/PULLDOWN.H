/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   pulldown.h                                                              */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*  Structures of pulldowns, dialogs and popups along with related constants */
/* are defined here.                                                         */
/*                                                                           */
/*...Release 1.01 (04/03/92)                                                 */
/*...                                                                        */
/*... 05/08/92  701   Srinivas  Cua Interface.                               */
/*... 12/10/93  910   Joe   Clear fields on cursor sensitive prompting.      */
/*****************************************************************************/

/*****************************************************************************/
/* Graphic box characters.                                                   */
/*****************************************************************************/
#define  H_BAR       0xC4
#define  HH_BAR      0xCD
#define  V_BAR       0xB3
#define  VV_BAR      0xBA
#define  TR_CORNER  0xBF
#define  BR_CORNER  0xD9
#define  TL_CORNER  0xDA
#define  BL_CORNER  0xC0
#define  TR_CCORNER  0xBB
#define  BR_CCORNER  0xBC
#define  TL_CCORNER  0xC9
#define  BL_CCORNER  0xC8
#define  B_JUNCTION 0xC1
#define  L_JUNCTION 0xC3
#define  R_JUNCTION 0xB4
#define  UPARROW    0x18
#define  DNARROW    0x19
#define  SHADELIGHT 0xB0
#define  SHADEDARK  0xB1

#define  R_ARROW        0x10
#define  U_ARROW        0x18
#define  D_ARROW        0x19
#define  BULLET         0x07

/*****************************************************************************/
/* Button Structure.                                                         */
/*****************************************************************************/
typedef struct button_st
{
  uint   row;                           /* row position of button.           */
  uint   col;                           /* column position of button.        */
  uint   length;                        /* length of button.                 */
  uchar  *Name;                         /* button text.                      */
  uint   Key;                           /* Key code associated with button.  */
} BUTTON;

typedef struct cascade
{
  int  PulldownIndex;
  UINT Flag;                            /* Index in the pulldown array.      */
} CAS_PULLDOWN;                         /* Flag for the status of choices.   */

/*****************************************************************************/
/* Pull down window structure.                                               */
/*****************************************************************************/
typedef struct pulld
{
   uint          row;                   /* row to start pulldown             */
   uint          col;                   /* column to start pulldown          */
   uint          width;                 /* width of pulldown box             */
   uint          entries;               /* # of entries in pulldown          */
   UINT          BitMask;               /* bit mask of entries not available */
   uchar         *labels;               /* list of labels concat together    */
   uchar         *hotkeys;              /* list of single char selections    */
   uchar         *SelPos;               /* Single char postions in selection */
   ULONG         *help;                 /* list of help screens              */
   uchar         *funccodes;            /* function codes for each selection */
   STRING        *AccelKeys;            /* Names of Accel Keys.              */
   uint          separators;            /* bit mask of separators in pulldown*/
   CAS_PULLDOWN  *CasPulldown;          /* Pointer to cascaded pulldown.     */
   uchar         *SaveArea;             /* -> screen save area.              */
} PULLDOWN;

/*****************************************************************************/
/* pop up shell structure.                                                   */
/*****************************************************************************/
typedef struct pop_st
{
   uint    row;                         /* row to start popup                */
   uint    col;                         /* cloumn to start popup             */
   uint    length;                      /* length of popup                   */
   uint    width;                       /* width of popup                    */
   uint    NoOfButtons;                 /* no of buttons.                    */
   char   *title;                       /* specific title of the popup win.  */
   char   *instructions;                /* specific instructions for popup.  */
   ULONG   help;                        /* context sensitive help id.        */
   BUTTON *Buttons;                     /* -> to buttons                     */
   uint    Flags;                       /* popup flags.                   910*/
} POPUPSHELL;

/*****************************************************************************/
/* pop up window choice structure.                                           */
/*****************************************************************************/
typedef struct pop_ch
{
   uint   entries;                      /* number of entries                 */
   uint   disabled;                     /* bit mask of choices not available */
   uchar *labels;                       /* list of choice strings            */
   uchar *hotkeys;                      /* list of single char selections    */
   uchar *SelPos;                       /* Single char postions in selection */
   ULONG *help;                         /* list of help screen numbers       */
} POPUPCHOICE;

/*****************************************************************************/
/* Dialog window choice structure.                                           */
/*****************************************************************************/
typedef struct Dialog_ch
{
  uint   entries;                       /* number of entries                 */
  uint   MaxRows;                       /* maximum no of rows.               */
  int    SkipRows;                      /* number of rows to skip.           */
  uchar *labels;                        /* list of choice strings            */
  uint   SliderStartRow;
  uint   NewSliderStartRow;
  uint   SliderSize;
} DIALOGCHOICE;

/*****************************************************************************/
/* Dialog shell structure.                                                   */
/*****************************************************************************/
typedef struct Dialog_st
{
   uint    row;                         /* row to start popup                */
   uint    col;                         /* column to start popup             */
   uint    length;                      /* length of popup                   */
   uint    width;                       /* width of popup                    */
   uint    NoOfButtons;                 /* no of buttons.                    */
   uint    NoOfButtonRows;              /* no of rows buttons occupy.        */
   uchar  *title;                       /* popup title                       */
   uint    SkipLines;                   /* No of lines to skip at the top.   */
   BUTTON *Buttons;                     /* -> to buttons                     */
   void   (*Display)( struct Dialog_st *, struct Dialog_ch * );
   uint   (*DialogFunction)( struct Dialog_st *, struct Dialog_ch *, EVENT *,
                             void * );
   uchar  *SaveArea;                    /* -> screen save area.              */
   uint   CurrentField;
} DIALOGSHELL;
