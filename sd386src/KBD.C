/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   kbd.c                                                                   */
/*                                                                           */
/* Description:                                                              */
/*   keyboard processing                                                     */
/*                                                                           */
/* Static Functions:                                                         */
/*                                                                           */
/* External Functions:                                                       */
/*                                                                           */
/*                                                                           */
/* History:                                                                  */
/*                                                                           */
/*   02/08/91 Creation of 32-bit SD86, from 16-bit version.                  */
/*                                                                           */
/*...16->32 port.                                                            */
/*...                                                                        */
/*... 02/08/91  100   Philip    port to 32 bit.                              */
/*... 02/08/91  101   Joe       port to 32 bit.                              */
/*...                                                                        */
/*...Release 1.01 (07/10/92)                                                 */
/*...                                                                        */
/*... 05/08/92  701   Joe       Cua Interface.                               */
/*...                                                                        */
/*...Release 1.02 (10/22/92)                                                 */
/*...                                                                        */
/*... 01/12/93  808   Selwyn    Profile fixes/improvements.                  */
/*... 03/29/93  818   Selwyn    Changes in the useability of ESC/ALT keys.   */
/*... 04/14/93  819   Selwyn    Add /k option for keyboard only use.         */
/*... 04/14/93  820   Selwyn    Add /u option to not flush k/b buffer.       */
/**Includes*******************************************************************/
#define INCL_16                         /* for 16-bit API                 100*/
#define INCL_SUB                        /* kbd, vio, mouse routines       100*/
#include "all.h"                        /* SD86 include files                */

/*
Keydata structure is defined in INCLUDE\SUBCALLS.H
CursorData structure is defined in INCLUDE\SUBCALLS.H
KBDFLUSHBUFFER is a dll in KBDCALLS.DLL and part of DOSCALLS.LIB.
KBDCHARIN is a dll in KBDCALLS.DLL and part of DOSCALLS.LIB.

*/

extern VIOCURSORINFO      NormalCursor;
extern VIOCURSORINFO      InsertCursor;
extern VIOCURSORINFO      HiddenCursor;

extern CmdParms cmd;
/*****************************************************************************/
/* GetKey()                                                                  */
/*                                                                           */
/* Description:                                                              */
/*   Get a key from the k/b and encode for our purposes.                     */
/*                                                                           */
/* Parameters:                                                               */
/*   none                                                                    */
/*                                                                           */
/* Return:                                                                   */
/*    k         encoded key.                                                 */
/*                                                                           */
/* Assumptions                                                               */
/*                                                                           */
/*                                                                           */
/* typedef struct _KBDKEYINFO {                                              */
/*        UCHAR    chChar;              --  ASCII character code             */
/*        UCHAR    chScan;              --  Scan Code                        */
/*        UCHAR    fbStatus;            --  State of the character           */
/*        UCHAR    bNlsShift;           --  Reserved (must equal 0)          */
/*        USHORT   fsState;             --  state of the shift keys          */
/*        ULONG    time;                --  time stamp of key stroke         */
/*        }KBDKEYINFO;                                                       */
/*                                                                           */
/*                                                                           */
/*  if chChar = 0  then chScan contains extended codes. (for example alt-esc)*/
/*              E0 then chScan contains extended codes for:                  */
/*                 Insert,Home,PgUp,Delete,End,PgDn,left,right,up,down       */
/*  if chScan = E0 then we have the PadEnter key.                            */
/*                                                                           */
/*  if chChar is not 0 or E0, then chChar contains the ASCII representation  */
/*  of the key and chScan "may", I repeat, "may" contain the scan code.      */
/*                                                                           */
/*****************************************************************************/

#define SHIFT_REPORT    0x0001                                          /*701*/
#define ALT_KEYDOWN     0x0008                                          /*701*/


uint                                    /* return an int for the key      101*/
GetKey( )                               /* get a character from the K/B      */
{                                       /* begin main loop                   */
 uint   k;                              /* encoded key.                   101*/
 KBDKEYINFO  key;                       /* define k/b parms                  */
 KBDINFO     KbdInfo;
 USHORT      fWait;                     /* wait/nowait on character.      101*/
 HKBD        hkbd;                      /* keyboard handle. ( = 0 )       101*/
 static uint StatusSet = FALSE;                                         /*701*/

/*****************************************************************************/
/* First we flush the keyboard buffer and get key packet.                    */
/*                                                                           */
/*****************************************************************************/
 hkbd  = 0;                             /* default kbd handle.            101*/
 fWait = 0;                             /* wait on the key.               101*/

 if( !NoFlushBuffer() )                                                 /*820*/
   KbdFlushBuffer(hkbd);                /* clear the keystroke buffer     101*/

 if( StatusSet == FALSE )                                               /*701*/
 {                                                                      /*701*/
   KbdInfo.cb = 10;                     /* length of record (always 10).  701*/
   KbdInfo.fsMask = 0x0006;             /*                            818 701*/
   KbdInfo.fsInterim = 0;                                               /*701*/
   KbdInfo.fsState = 0;                                                 /*701*/

   KbdSetStatus( &KbdInfo, hkbd );                                      /*701*/
   StatusSet = TRUE;                                                    /*701*/
 }                                                                      /*701*/

 for( ;; )
 {
  (void)KbdCharIn((PKBDKEYINFO)&key,fWait,hkbd);      /* read a char.     101*/
  /***************************************************************************/
  /* We are going to encode the key for our usage.                           */
  /*                                                                         */
  /*     chScan     chChar                                                   */
  /* k = ________   ________                                                 */
  /*                                                                         */
  /* Yes, we have flopped them. DO NOT ASK ME WHY!                           */
  /*                                                                         */
  /*                                                                         */
  /***************************************************************************/
  if( key.chScan == 0xE0 )              /* if this is the PadEnter key       */
     key.chScan = 0;                    /* then clear scan code.             */
  if( key.chChar == 0xE0 )              /* if this is insert,home,etc. as    */
     key.chChar = 0;                    /* explained above, then clear the   */
                                        /* char code.                        */
  k = ( key.chScan << 8) | key.chChar;  /* encode the key...right.           */

/*****************************************************************************/
/* Our encoding scheme will encode all spacebars the same do we have to      */
/* check individual shift states and modify coding accordingly.              */
/*                                                                           */
/*****************************************************************************/
  if( k == SPACEBAR )                   /* if the key encodes to a spacebar  */
  {                                     /* then we want to check for alt and */
   if( key.fsState & 0x0A00 )           /* shift states. check for alt keys. */
    k = A_SPACEBAR;                     /*                                   */
   if( key.fsState & 0x0500 )           /* check for ctrl keys.              */
    k = C_SPACEBAR;                     /*                                   */
   if( key.fsState & 0x0003 )           /* check for ctrl keys.           808*/
    k = S_SPACEBAR;                     /*                                808*/
  }                                     /*                                   */
/*****************************************************************************/
/* Insert and Shift-insert will return the same chChar and chScan. We have   */
/* to look at the shift state to distinguish.                                */
/*                                                                           */
/*****************************************************************************/
  if( k == INS )                        /* if the key encodes to an insert   */
  {                                     /* then we want to check for the     */
   if( key.fsState & 0x0003 )           /* shift states and change to        */
    k = S_INS;                          /* shift insert if necessary.        */
   if( key.fsState & 0x0A00 )           /* shift states and change to     808*/
    k = A_INS;                          /* Alt   insert if necessary.     808*/
  }                                     /*                                   */
  if( !NoFlushBuffer() )                                                /*820*/
    KbdFlushBuffer(hkbd);               /* clear the keystroke buffer     101*/
  return( k );                          /* return k/b character k            */
 }
}

    void
PutCsr( CSR *csr )
{
 VIOCURSORINFO  *p;                     /*                                101*/
 USHORT          usRow;                 /*                                101*/
 USHORT          usColumn;              /*                                101*/
 HVIO            hvio;                  /*                                101*/
                                        /*                                101*/
 usRow    = csr->row;                   /*                                101*/
 usColumn = csr->col;                   /*                                101*/
 hvio     = 0;                          /*                                101*/
                                        /*                                101*/
 VioSetCurPos( usRow, usColumn, hvio ); /*                                101*/

    switch( csr->mode ){
      default:
      case CSR_NORMAL:
        p = &NormalCursor;  break;
      case CSR_INSERT:
        p = &InsertCursor;  break;
      case CSR_HIDDEN:
        p = &HiddenCursor;  break;
    }
    VioSetCurType(p,hvio);              /*                                101*/
}

    void
HideCursor( )
{
 HVIO            hvio;                  /*                                101*/
 VIOCURSORINFO  *p;                     /*                                101*/

 hvio = 0;                              /*                                101*/
 p = (PVIOCURSORINFO)&HiddenCursor;     /*                                101*/
 VioSetCurType( p, hvio );              /*                                101*/
}

    void
beep( )
{
    DosBeep( 1800/*Hz*/ , 1/*millisecond*/ );
}

/*****************************************************************************/
/* NoFlushBuffer()                                                        820*/
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Checks whether the keyboard buffer flush flag is set.                   */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   None.                                                                   */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   TRUE       Keyboard flush buffer flag is turned on.                     */
/*   FALSE      Keyboard flush buffer flag is turned off.                    */
/*                                                                           */
/*****************************************************************************/
int  NoFlushBuffer()
{
  if( cmd.KBDBufferFlush )
    return( TRUE );
  else
    return( FALSE );
}

/*****************************************************************************/
/* SetFlushBufferFlag()                                                   820*/
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Sets the keyboard flush buffer flag.                                    */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   Flag       Value of the keyboard flush buffer flag.                     */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   None.                                                                   */
/*                                                                           */
/*****************************************************************************/
void SetFlushBufferFlag( int Flag )
{
  /***************************************************************************/
  /* If the flag was set to "No buffer flush all times" dont change it.      */
  /***************************************************************************/
  if( cmd.KBDBufferFlush != NOFLUSHALLTIMES )
    cmd.KBDBufferFlush = Flag;
}

/*****************************************************************************/
/* ResetFlushBufferFlag()                                                 820*/
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Resets the keyboard flush buffer flag.                                  */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   None.                                                                   */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   None.                                                                   */
/*                                                                           */
/*****************************************************************************/
void ResetFlushBufferFlag()
{
  /***************************************************************************/
  /* If the flag was set to "No buffer flush all times" dont reset it.       */
  /***************************************************************************/
  if( cmd.KBDBufferFlush != NOFLUSHALLTIMES )
    cmd.KBDBufferFlush = 0;
}

/*****************************************************************************/
/* SetHotKey()                                                               */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Turn off/on the alt-esc and ctrl-esc keys.                              */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
void SetHotKey()
{
 APIRET rc;
 HFILE  FileHandle = 0;

 /****************************************************************************/
 /* - First, open the keyboard.                                              */
 /****************************************************************************/
 {
  ULONG ActionTaken   = 0;
  ULONG FileSize      = 0;
  ULONG FileAttribute = FILE_SYSTEM;
  ULONG OpenFlag      = OPEN_ACTION_OPEN_IF_EXISTS;
  ULONG OpenMode      = OPEN_SHARE_DENYNONE | OPEN_ACCESS_READWRITE;

  rc = DosOpen((PSZ)"KBD$",
               (PHFILE)&FileHandle,
               (PULONG)&ActionTaken,
               (ULONG)FileSize,
               (ULONG)FileAttribute,
               (ULONG)OpenFlag,
               (ULONG)OpenMode,
               (PEAOP2) NULL) ;
 }


 /****************************************************************************/
 /* - Turn off ctrl-esc and alt-esc.                                         */
 /****************************************************************************/
 if( rc == 0 )
 {
  HOTKEY HotKey;
  ULONG  ParmLengthInOut;
  ULONG  DataLengthInOut;

  HotKey.fsHotKey         = 0;
  HotKey.uchScancodeMake  = 0;
  HotKey.uchScancodeBreak = 0;
  HotKey.idHotKey         = 0xFFFF;

  ParmLengthInOut = sizeof (HotKey);
  DataLengthInOut = 0;

  rc = DosDevIOCtl( FileHandle,
            (ULONG) IOCTL_KEYBOARD,
            (ULONG) KBD_SETSESMGRHOTKEY,
            (PVOID) &HotKey,
            (ULONG) ParmLengthInOut,
            (PULONG) &ParmLengthInOut,
            (PVOID) NULL,
            (ULONG) 0L,
            (PULONG) &DataLengthInOut);


 }

 /****************************************************************************/
 /* - Close the keyboard.                                                    */
 /****************************************************************************/
 if( FileHandle != 0) DosClose( FileHandle ) ;
}

/*****************************************************************************/
/* Convert                                                                   */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Convert a key scan code to its appropriate function code.               */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   keycode    scan code passed in to be converted.                         */
/*                                                                           */
/* Return:                                                                   */
/*   funcode    function code associated with the scan code passed in.       */
/*                                                                           */
/*****************************************************************************/
extern KEY2FUNC defk2f[];
int Convert(uint keycode)
{
  uint i;

  for(
       i = 0;
       i < KEYNUMSOC;
       i++
     )
    if ( defk2f[i].scode == keycode )
      return( defk2f[i].fcode );

  return( DONOTHING );
}
