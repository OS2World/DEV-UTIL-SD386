
#define CHAR_READY             0x40

/*****************************************************************************/
/* Mouse states of interest as reported by MouReadEventQue.                  */
/*****************************************************************************/
#define MOUSTATE_NO_BUTTONS_DOWN        0x0
#define MOUSTATE_NO_BUTTONS_DOWN_MOVE   0x1
#define MOUSTATE_BUTTON_1_DOWN          0x4
#define MOUSTATE_BUTTON_1_DOWN_MOVE     0x2
#define MOUSTATE_BUTTON_2_DOWN          0x10
#define MOUSTATE_BUTTON_2_DOWN_MOVE     0x8

/*****************************************************************************/
/* Temporary alias for MOUSTATE_BUTTON_1_DOWN. !!!!!!!!!!!!!!!!!!!!!!        */
/*****************************************************************************/
#define EVENT_NO_BUTTONS_DOWN        0x0
#define EVENT_NO_BUTTONS_DOWN_MOVE   0x1
#define EVENT_LEFT_BUTTON_DOWN_MOVE  0x2
#define EVENT_LEFT_BUTTON_DOWN       0x4

#define EVENT_BUTTON_1_DOWN          0x4
#define EVENT_BUTTON_1_DOWN_MOVE     0x2
#define EVENT_BUTTON_2_DOWN          0x10
#define EVENT_BUTTON_2_DOWN_MOVE     0x8

/*****************************************************************************/
/* Another set of TEMPORARY aliases from Srinivas mouse.h !!!!!!!!!!!!!!!!   */
/*****************************************************************************/
#define NO_BUTTONS_DOWN        0x0
#define LEFT_BUTTON_DOWN_MOVE  0x2
#define LEFT_BUTTON_DOWN       0x4
#define FAKE_BUTTON_DOWN       0x6
#define BUTTON_DOWN            0x1
#define KBD_INPUT              10
#define SEMA4_TIMEOUT          640


/*****************************************************************************/
/* Mouse states                                                              */
/*****************************************************************************/
#define STATE_BUTTON_PRESSED   1
#define STATE_BUTTON_MOVE      2
#define STATE_BUTTON_RELEASED  3
#define STATE_CONTINUOUS_PRESS 4

#define SCROLL_REGULAR_WAIT     50
#define SCROLL_INITIAL_WAIT     500

#ifndef ERROR_TIMEOUT
  #define ERROR_TIMEOUT  640
#endif



/*****************************************************************************/
/* Valid entries for the "Value" member of the EVENT structure.              */
/*****************************************************************************/
#define TYPE_MOUSE_EVENT            1
#define TYPE_KBD_EVENT              2

#define TYPE_MSH_EVENT              3

typedef struct _EVENT
{
  ushort  Row;                          /* row of the mouse event.           */
  ushort  Col;                          /* col of the mouse event.           */
  uchar   Type;                         /* type of the event(kbd or mouse).  */
  uint    Value;                        /* MouState at time of event or key. */
  uchar   FakeEvent;                    /* Flag to designate this is an      */
                                        /* event( mouse ) resulting from a   */
                                        /* timeout while waiting for an      */
                                        /* i/o event. Can be TRUE or FALSE.  */
} EVENT, *PEVENT;
