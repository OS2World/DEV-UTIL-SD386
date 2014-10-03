/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   mouse.c                                                                 */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Mouse Related Functions.                                                */
/*                                                                           */
/*...Release 1.01 (04/03/92)                                                 */
/*...                                                                        */
/*... 05/08/92  701   Srinivas  Cua Interface.                               */
/*...                                                                        */
/*...Release 1.02 (10/22/92)                                                 */
/*...                                                                        */
/*... 04/14/93  819   Selwyn    Add /k option for keyboard only use.         */
/*****************************************************************************/
#include "all.h"
#include <process.h>

/*****************************************************************************/
/* this needs to be returned to static.                                      */
/*****************************************************************************/
static EVENT Event;                     /* The i/o event.                    */
static HEV     EventSem;                /* An event is ready sema4.          */
static HEV     GetItSem;                /* Go get an event sema4.            */
static HMTX    EventWrite;              /* Mutex sema4 for serializing       */
                                        /* access to writing the event.      */
static  HMOU    MouHandle = NULL;       /* Handle to mouse                   */

extern  uint    VideoRows;
extern  uint    VideoCols;

extern  CmdParms cmd;

/*****************************************************************************/
/* StartThreads()                                                            */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Starts the i/o threads for the keyboard and the mouse. Starts the       */
/*   semaphores needed to serialize access to the i/o event.                 */
/*                                                                           */
/* Parameters:                                                               */
/*  None                                                                     */
/*                                                                           */
/* Return:                                                                   */
/*  None                                                                     */
/*****************************************************************************/

#define STACKSIZE    0x8000             /* Stack size for the i/o threads.   */
#define INITSEMSTATE FALSE              /* Initial sema4 state is "set" for  */
                                        /* an event and "unowned" for mutex. */
#define ATTRFLAGS    0L                 /* Sema4s are unnamed and non-shared.*/

void StartThreads( void )
{
  /***************************************************************************/
  /* 1. Start a thread to handle the mouse events.                           */
  /* 2. Start a thread to handle the keyboard events.                        */
  /* 3. Create a sema4 to signal the posting of an event.                    */
  /* 4. Create a sema4 to trigger the i/o threads to go get an event.        */
  /***************************************************************************/
  _beginthread( KbdThread,   NULL, STACKSIZE, NULL );
  if( !cmd.KeyboardOnly )
   _beginthread( MouseThread, NULL, STACKSIZE, NULL );
#if 0
  DosCreateEventSem( (PSZ)NULL, &EventSem, ATTRFLAGS, INITSEMSTATE );
  DosCreateEventSem( (PSZ)NULL, &GetItSem, ATTRFLAGS, INITSEMSTATE );
  DosCreateMutexSem( NULL, &EventWrite, ATTRFLAGS, INITSEMSTATE );
#else
  {
     char semaphorename[80];
     USHORT         pid=getpid();

/*Allow other processes to communicate by simulating Mouse Events.*/
  sprintf(semaphorename,"\\SEM32\\SD386\\EventSem\\%4.4hd",pid);
  DosCreateEventSem( (PSZ) semaphorename,
      &EventSem, ATTRFLAGS, INITSEMSTATE );
  sprintf(semaphorename,"\\SEM32\\SD386\\GetItSem\\%4.4hd",pid);
  DosCreateEventSem( (PSZ)semaphorename,
      &GetItSem, ATTRFLAGS, INITSEMSTATE );
  sprintf(semaphorename,"\\SEM32\\SD386\\EventWrite\\%4.4d",pid);
  DosCreateMutexSem( semaphorename, &EventWrite, ATTRFLAGS, INITSEMSTATE );
  }
#endif
}

/*****************************************************************************/
/* AccessMouse()                                                             */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*  Opens the mouse and starts the threads for handling the i/o events.      */
/*                                                                           */
/* Parameters:                                                               */
/*   None                                                                    */
/*                                                                           */
/* Return:                                                                   */
/*   None                                                                    */
/*****************************************************************************/

#define BUTTON1PRESS    0x0004          /* Button 1 press/release events     */
#define BUTTON1MOVE     0x0002          /* Button 1 press/release + motion   */
#define BUTTON2PRESS    0x0010          /* Button 2 press/release events     */
#define BUTTON2MOVE     0x0008          /* Button 2 press/release + motion   */

void AccessMouse( void )
{
  ushort MouseMask;

  /***************************************************************************/
  /* Define the mask for the "actions" that we want to cause mouse "events"  */
  /* to be generated. MouState will tell us the "state" of the mouse at      */
  /* the "time" of the event. With this mask, we should see four states:     */
  /*                                                                         */
  /*  1. No buttons down.                                                    */
  /*  2. No buttons down AND the mouse is moving.                            */
  /*  3. Button 1 down.                                                      */
  /*  2. Button 1 down AND the mouse is moving.                              */
  /*                                                                         */
  /***************************************************************************/
  MouseMask = BUTTON1PRESS | BUTTON1MOVE | BUTTON2PRESS | BUTTON2MOVE;

  /***************************************************************************/
  /*  - Open the mouse.                                                      */
  /*  - Set up the event mask.                                               */
  /*  - Draw the mouse.                                                      */
  /*  - Start the i/o threads.                                               */
  /***************************************************************************/
  if( !cmd.KeyboardOnly )
  {
    if( MouOpen( 0L, &MouHandle ) || MouSetEventMask( &MouseMask, MouHandle) )
    {
      extern  uchar  VideoAtr;
char msg[] = { "\rMouse open failure. Only keyboard will be active.\r\
Press any key to continue..." };

      VideoAtr = vaMenuBar;
      ShowHelpBox( msg, (UINTFUNC)GetKey, NULL, NULL );
      cmd.KeyboardOnly = TRUE;
    }
  }
  StartThreads();
}

/*****************************************************************************/
/* GetEvent()                                                                */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*  Triggers the i/o threads to post an event and then waits until           */
/*  the event gets posted. If a wait time is specified, then a timeout       */
/*  will occur and the last event will essentially be recycled.              */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*  WaitTime    input - How long to wait before timing out.                  */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*  &Event      ptr to the event structure containing the posted event.      */
/*                                                                           */
/*****************************************************************************/
PEVENT GetEvent( uint WaitTime )
{
  ulong count;

  /***************************************************************************/
  /* 1. Set the RepeatEvent flag. If the event semaphore times out on        */
  /*    WaitTime, then the caller will use this flag to know that an         */
  /*    event is repeated. This us used mainly for scroll bars.              */
  /* 2. Set the i/o event semaphore. ( We don't really care about the        */
  /*    count at this time. )                                                */
  /* 3. Trigger the i/o threads to post an event.                            */
  /* 4. Wait for an event to be posted.                                      */
  /***************************************************************************/
  APIRET rc;
  ShowMouse();
  Event.FakeEvent  = TRUE;

  DosResetEventSem( EventSem, &count );
  DosPostEventSem( GetItSem );
#ifndef MSH
  DosWaitEventSem( EventSem, WaitTime );
#else
  rc=DosWaitEventSem( EventSem, WaitTime );
  if(rc==ERROR_TIMEOUT) {
      Event.FakeEvent = TRUE;
  }
  else if(Event.FakeEvent)  /*MSH Event must have interrupted */
  {
      Event.FakeEvent = FALSE;
      Event.Type  = TYPE_MSH_EVENT;
  }/* End if*/
#endif
  HideMouse();
  return( &Event );
}

/*****************************************************************************/
/* MouseThread()                                                             */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*  Waits for a trigger and then reads one event from the mouse queue.       */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*  Dummy       this is just a dummy parm to keep the compiler happy.        */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*  None                                                                     */
/*                                                                           */
/*****************************************************************************/

#define WAIT_EMPTYQ   1
#define NOWAIT_EMPTYQ 0
#define BUTTON1FILTER 0x0007
#define BUTTON2FILTER 0x0019

void MouseThread( void *Dummy )
{
  ushort        WaitOption;
  MOUEVENTINFO  MouEvent;
  ulong         count;

  /***************************************************************************/
  /* 1. Wait for a trigger to go get an event.                               */
  /* 2. Read the mouse queue.                                                */
  /* 3. Reset the trigger.                                                   */
  /* 4. Filter out any mouse states such as "button 2" that we're not        */
  /*    interested in.                                                       */
  /* 5. Acquire the mutex semaphore to serialize writing the event.          */
  /* 6. Write the event. We test to see if an event has already been         */
  /*    written since we could have a race between the keyboard and the      */
  /*    mouse to post the event. We handle this as first one to get the      */
  /*    mutex semaphore gets to post the event.                              */
  /* 7. Set a flag to indicate this is not a fake event.                     */
  /* 8. Release the mutex.                                                   */
  /* 9. Post the event.                                                      */
  /*                                                                         */
  /***************************************************************************/
  WaitOption = WAIT_EMPTYQ;
  for( ;; )
  {
    DosWaitEventSem( GetItSem, SEM_INDEFINITE_WAIT );
    MouReadEventQue( &MouEvent, &WaitOption, MouHandle );
    DosResetEventSem( GetItSem, &count );

    MouEvent.fs &= (BUTTON1FILTER | BUTTON2FILTER);

    DosRequestMutexSem( EventWrite, SEM_INDEFINITE_WAIT );
    if( Event.FakeEvent == TRUE )
    {
     Event.Row   = MouEvent.row;
     Event.Col   = MouEvent.col;
     Event.Type  = TYPE_MOUSE_EVENT;
     Event.Value = MouEvent.fs;
     Event.FakeEvent  = FALSE;
    }
    DosReleaseMutexSem( EventWrite );

    DosPostEventSem( EventSem );
  }
}

/*****************************************************************************/
/*  HideMouse()                                                              */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*  Hides the mouse.                                                         */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*  None                                                                     */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*  None                                                                     */
/*                                                                           */
/*****************************************************************************/
void HideMouse( void )
{
  NOPTRRECT NoPtrRect;

  NoPtrRect.row  = 0;
  NoPtrRect.col  = 0;
  NoPtrRect.cRow = VideoRows - 1;
  NoPtrRect.cCol = VideoCols - 1;

  MouRemovePtr( &NoPtrRect, MouHandle );
}

/*****************************************************************************/
/* ShowMouse()                                                              */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*  Redraws the mouse.                                                       */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*  None                                                                     */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*  None                                                                     */
/*                                                                           */
/*****************************************************************************/
void ShowMouse( void )
{
  MouDrawPtr( MouHandle );
}

/*****************************************************************************/
/* CloseMouse()                                                             */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*  Redraws the mouse.                                                       */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*  None                                                                     */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*  None                                                                     */
/*                                                                           */
/*****************************************************************************/
void CloseMouse( void )
{
  if( MouHandle )
    MouClose( MouHandle );
}

/*****************************************************************************/
/*  KbdThread()                                                              */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*  Gets a key from the keyboard and posts as an event.                      */
/*                                                                           */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*  None                                                                     */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*  None                                                                     */
/*                                                                           */
/*****************************************************************************/
void KbdThread( void *Dummy )
{
  ulong  count;
  uint   key;

  /***************************************************************************/
  /* 1. Wait for a trigger to go get a key.                                  */
  /* 2. Read the key.                                                        */
  /* 3. Reset the trigger.                                                   */
  /* 5. Acquire the mutex semaphore to serialize writing the event.          */
  /* 6. Write the event. We test to see if an event has already been         */
  /*    written since we could have a race between the keyboard and the      */
  /*    mouse to post the event. We handle this as first one to get the      */
  /*    mutex semaphore gets to post the event.                              */
  /* 7. Set a flag to indicate this is not a fake event.                     */
  /* 8. Release the mutex.                                                   */
  /* 9. Post the event.                                                      */
  /*                                                                         */
  /***************************************************************************/
  for( ;; )
  {
    DosWaitEventSem( GetItSem, SEM_INDEFINITE_WAIT );
    key = GetKey();
    DosResetEventSem( GetItSem, &count );

    DosRequestMutexSem( EventWrite, SEM_INDEFINITE_WAIT );
    if( Event.FakeEvent == TRUE )
    {
     Event.Type  = TYPE_KBD_EVENT;
     Event.Value = key;
     Event.FakeEvent  = FALSE;
    }
    DosReleaseMutexSem( EventWrite );

    DosPostEventSem( EventSem );
  }
}

/*****************************************************************************/
/* GetCurrentEvent()                                                         */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*  Get a ptr to the current i/o event. This is used mainly to get the       */
/*  current row and column of the last mouse state.                          */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*  None                                                                     */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*  &Event      ptr to the event structure.                                   /
/*                                                                           */
/*****************************************************************************/
PEVENT  GetCurrentEvent()
{
  return( &Event );
}

void CloseEventSemaphores(void) {
  DosClose(EventSem);   /*Do these first so that MSH will not submit any */
                        /*more commands.*/
  DosClose(EventWrite);
  DosPostEventSem( GetItSem ); /*MSH may be waiting.*/
  DosClose(GetItSem);
}
