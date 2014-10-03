/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   vio.c                                                                   */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*  VIO subsystem initialization.                                            */
/*                                                                           */
/*                                                                           */
/* History:                                                                  */
/*                                                                           */
/*   02/08/91 Creation of 32-bit SD86, from 16-bit version.                  */
/*                                                                           */
/*...16->32 port.                                                            */
/*...                                                                        */
/*... 02/08/91  100   Philip    port to 32 bit.                              */
/*... 02/08/91  112   Joe       changed size of cursor so it could be seen   */
/*                              on different size screens                    */
/*                                                                           */
/*...Release 1.00 (Pre-release 1)                                            */
/*...                                                                        */
/*... 07/09/91  200   srinivas  changed logo screen from RPGS to PRGS.       */
/*                                                                           */
/*...Release 1.00 (Pre-release 107 11/13/91)                                 */
/*...                                                                        */
/*... 11/13/91  400   Srinivas  Vertical Register Display.                   */
/*...                                                                        */
/*...Release 1.00 (Pre-release 108 12/05/91)                                 */
/*...                                                                        */
/*... 02/11/92  518   Srinivas  Remove limitation on max no of screen rows.  */
/*... 02/12/92  521   Srinivas  Port to C-Set/2.                             */
/*...                                                                        */
/*...Release 1.01 (04/03/92)                                                 */
/*...                                                                        */
/*... 05/08/92  701   Srinivas  Cua Interface.                               */
/**Includes*******************************************************************/
                                        /*                                   */
#define INCL_16                         /* 16-bit API                     100*/
#define INCL_SUB                        /* vio, kbd, mouse routines       100*/
#include "all.h"                        /* SD86 include files                */
static int iview=0;
                                        /*                                   */
VIOCURSORINFO  NormalCursor;            /* normal thin line at bottom.       */
VIOCURSORINFO  InsertCursor;            /* block style cursor.               */
VIOCURSORINFO  HiddenCursor;            /* invisible cursor.                 */

/* logical to physical video attribute maps */
static  uchar vaColor[] = { 0,
  /* vaProgram  1 */ BG_blue |FG_white,
  /* vaBptOk    2 */ BG_blue |FG_white|FG_light,
  /* vaBptOn    3 */ BG_red  |FG_white|FG_light,
  /* vaXline    4 */ BG_white|FG_black|FG_light,
  /* vaXlineOn  5 */ BG_white|FG_red,
  /* vaMenuBar  6 */ BG_cyan |FG_white|FG_light,
  /* vaMenuCsr  7 */ BG_black|FG_white|FG_light,
  /* vaStgExp   8 */ BG_white|FG_black,
  /* vaStgVal   9 */ BG_white|FG_black,
  /* vaRegWind 10 */ BG_blue |FG_white,
  /* vaPrompt  11 */ BG_brown|FG_white|FG_light,
  /* vaError   12 */ BG_black|FG_red  |FG_light,
  /* vaInfo    13 */ BG_cyan |FG_white|FG_light,
  /* vaHelp    14 */ BG_white|FG_black,
  /* vaStgPro  15 */ BG_magenta|FG_white|FG_light,
  /* vaClear   16 */ BG_black|FG_white,
  /* vaStgStat 17 */ BG_cyan |FG_white|FG_light,
  /* vaMxdSrc  18 */ BG_blue |FG_white,
  /* vaMenuSel 19 */ BG_cyan |FG_yellow,
  /* vaCallStk 20 */ BG_brown|FG_white|FG_light,
  /* vaCallSel 21 */ BG_white|FG_brown,
  /* vaRegTogg 22 */ BG_blue |FG_white|FG_light,
  /* vaTemp1   23 */ BG_blue |FG_white|FG_light,
  /* vaBadAct  24 */ BG_white|FG_black|FG_light,                        /*701*/
  /* vaBadSel  25 */ BG_black|FG_white|FG_light,                        /*701*/
  /* vaShadow  26 */ BG_black|FG_black|FG_light                         /*701*/
};
static  uchar vaMono[] = { 0,
  /* vaProgram  1 */ BG_black|FG_white,
  /* vaBptOk    2 */ BG_black|FG_white|FG_light,
  /* vaBptOn    3 */ BG_black|FG_uline|FG_light,
  /* vaXline    4 */ BG_white|FG_black,
  /* vaXlineOn  5 */ BG_white|FG_black,
  /* vaMenuBar  6 */ BG_white|FG_black,
  /* vaMenuCsr  7 */ BG_black|FG_white,
  /* vaStgExp   8 */ BG_white|FG_black,
  /* vaStgVal   9 */ BG_white|FG_black,
  /* vaRegWind 10 */ BG_black|FG_white,
  /* vaPrompt  11 */ BG_white|FG_black,
  /* vaError   12 */ BG_black|FG_white|FG_light,
  /* vaInfo    13 */ BG_black|FG_white,
  /* vaHelp    14 */ BG_white|FG_black,
  /* vaStgPro  15 */ BG_black|FG_white|FG_light,
  /* vaClear   16 */ BG_black|FG_white,
  /* vaStgStat 17 */ BG_black|FG_white,
  /* vaMxdSrc  18 */ BG_black|FG_white,
  /* vaMenuSel 19 */ BG_black|FG_white,
  /* vaCallStk 20 */ BG_white|FG_black,
  /* vaCallSel 21 */ BG_black|FG_white,
  /* vaRegTogg 22 */ BG_black|FG_white|FG_light,
  /* vaTemp1   23 */ BG_black|FG_white|FG_light,
  /* vaBadAct  24 */ BG_white|FG_black|FG_light,                        /*701*/
  /* vaBadSel  25 */ BG_black|FG_white,                                 /*701*/
  /* vaShadow  26 */ BG_black|FG_black|FG_light                         /*701*/
};

extern CmdParms       cmd;              /* pointer to CmdParms structure  701*/
                                        /* screen display.                701*/

UINT   MsgRow;                          /* screen row for messages (0..N)    */
UINT   MaxData;                         /* max # of lines of Data            */
UINT   MaxPer;                          /* max # of lines of Code            */
UCHAR *VideoMap;                        /* -> logical-to-phys attrb.map      */
UCHAR *BoundPtr;                        /* -> to screen bounds               */
UINT   ProRow;                          /* screen row for prompt (0..N)      */
UINT   VideoCols;                       /* # of columns per screen           */

#ifdef MSH
UINT   VideoWidth;                      /* # of columns per screen           */
#endif

UINT   FnameRow;                        /* screen row for file name (0..N)   */
UCHAR  VideoAtr = 0;                    /* default logical video attribute.  */
UINT   HelpRow;                         /* screen row for menu help (0..N)   */
UINT   LinesPer;                        /* currnt # of lines/screen for Code */
UINT   VideoRows;                       /* # of rows per screen              */
UINT   VioStartOffSet;                  /* flag to tell were to start     701*/
UINT   MinPer;                          /* min # of lines of Code            */
UINT   MenuRow;                         /* screen row for menu bar (0..N)    */
UCHAR *VideoPtr = NULL;                 /* -> to logical video buffer     100*/
UINT   TopLine;                         /* current top line for Code (0..N)  */
USHORT ColStart=0, RowStart=0;

/* Attrib is a macro in fmt.h           */

static uchar clrrow[] = { Attrib(00), RepCnt(00), ' ', 0 };

    void
ClrScr(uint toprow,uint botrow,uint attr )
/*         toprow  : 1st line to clear (0..N) */
/*         botrow  : last line to clear (0..N) */
/*         attr    : logical attribute used to clear screen */
{
  VideoAtr = (uchar)attr;               /* Set the default video attribute   */
  clrrow[0] = (uchar)Attrib(attr);
  clrrow[2] = (uchar)(VideoCols-ColStart);
  while( toprow <= botrow )
      putrc( toprow++, 0, clrrow );
}

static  uchar *banner[] = {
 "O S / 2  S o u r c e - l e v e l   D e b u g g e r",
 "",
 "Version 5.00",
 "08/23/96",
 "",
 "Copyright (C) IBM Corp. 1987 - 1996",
 "",
 "",
 "",
 NULL
};
#define BANNER_ROW 4

void VioInit( )
{
 APIRET      rc;
 UCHAR     **cpp;
 UINT        vlen;
 UINT        row;
 VIOMODEINFO md;

 memset(&md, 0, sizeof(md) );
 md.cb = sizeof(md);

 if( VioGetMode(&md, 0) )
 {
  printf( "Vio error" );
  exit(0);
 }

 VideoRows = md.row;
 VideoCols = md.col;
 MaxPer    = VideoRows - 2 ;            /* max # of lines of Code            */
 MaxData   = VideoRows * 2 / 3;         /* max # of rows for data window     */
 MinPer    = MaxPer - MaxData;          /* min # of lines of Code            */
 ProRow    = VideoRows - 3;             /* screen row for prompt (0..N)      */
 MenuRow   = VideoRows - 3;             /* screen row for menu bar (0..N)    */
 FnameRow  = VideoRows - 2;             /* screen row for file name (0..N)   */
 HelpRow   = VideoRows - 1;             /* screen row for menu help (0..N)   */
 MsgRow    = VideoRows - 1;             /* screen row for messages (0..N)    */
 LinesPer  = MaxPer - TopLine;          /* current # of lines per screen     */
                                        /* for source                        */
 VideoMap  = vaColor;

 if( md.color == 1 )
  VideoMap = vaMono;


    rc = VioGetBuf((PULONG)&VideoPtr, (PUSHORT)&vlen, 0);
    if( rc != 0 ){
        printf( "VioGetBuf() returned %d\n", rc );
        panic(OOquit);
    }
    if( ! HiddenCursor.attr ){
        rc = VioGetCurType(&NormalCursor,0);
        if( rc != 0 ){
            printf( "VioGetCurType() returned %d\n", rc );
            panic(OOquit);  }

        /*********************************************************************/
        /* If the cursor is in insert mode when we enter here then convert   */
        /* the parms of cursorinfo into a normal mode cursor.                */
        /*********************************************************************/
        if ((NormalCursor.cEnd - NormalCursor.yStart) > 2)
          NormalCursor.yStart = NormalCursor.cEnd -
                           ((NormalCursor.cEnd - NormalCursor.yStart - 1) / 4);
        NormalCursor.yStart -= 1;                                       /*112*/
        memcpy( &InsertCursor, &NormalCursor, sizeof(InsertCursor) );   /*100*/
        InsertCursor.yStart = 0;
        memcpy( &HiddenCursor, &NormalCursor, sizeof(HiddenCursor) );   /*100*/
        HiddenCursor.attr = -1;
    }

    /*************************************************************************/
    /* - Allocate memory for screen bounds                                400*/
    /* - set the bounds to the video cols                                 400*/
    /*************************************************************************/
    BoundPtr = Talloc(VideoRows);                                   /*521 518*/
    if (BoundPtr)                                                       /*518*/
       memset(BoundPtr,VideoCols,VideoRows);                            /*518*/
    else                                                                /*518*/
    {                                                                   /*518*/
        printf( "malloc failed\n");                                     /*518*/
        panic(OOquit);                                                  /*518*/
    }                                                                   /*518*/

    VioStartOffSet = 1;
    TopLine = 1;
    MenuRow  = 0;
    LinesPer = MaxPer - TopLine;
    InitDataWinCsr();
    AccessMouse();

    HideCursor();
    ClrScr( 0, VideoRows-1, vaBptOk );
    cpp = banner;
    for( row = BANNER_ROW; *cpp; cpp++,row++ )
    {
     putrc( row, (VideoCols-strlen(*cpp)) / 2, *cpp );
    }
}

    void
ClrPhyScr(uint top,uint bot,uint attr )
                                        /* 1st line to clear (0..N)          */
                                        /* last line to clear (0..N)         */
                                        /* logical attr used to clr scrn     */
{
    twoc fillchar = ( twoc )((VideoMap[attr] << 8) | ' ');/* cast         100*/

    if( VioScrollUp((ushort)top, 0, (ushort)bot, /* cast                  100*/
                    (ushort)(VideoCols-1),       /* cast                  100*/
                    (ushort)(bot - top + 1),     /* cast                  100*/
                    (char*) &fillchar, 0)  )
        panic(OOvioclr);
}

    void
PutPhyScr(uint top,uint bot )
                        /* 1st line to show (0..N) */
                        /* last line to show (0..N) */
{
    if( VioShowBuf((ushort)(top * 2*VideoCols),              /* cast      100*/
                   (ushort)((bot - top + 1) * 2*VideoCols),  /* cast      100*/
                   0) )
        panic(OOvioshow);
}

/* Interface for 32-bit assembly routines to call 16-bit VioShowBuf API   100*/
void                                                                    /*100*/
Vio32ShowBuf(ushort VioOff, ushort VioLen, ushort VioHan)               /*100*/
{                                                                       /*100*/
 Vio16ShowBuf(VioOff, VioLen, VioHan);                                  /*100*/
}                                                                       /*100*/

/*****************************************************************************/
/* Putrc()                                                                400*/
/*                                                                           */
/* Description:                                                              */
/*      Filters the parameters being passed to putrcx (asm routine)          */
/*      so that we donot write to the parts of the screen where we           */
/*      do not have access to it.                                            */
/* Parameters:                                                               */
/*      row   - Row where to write the string                                */
/*      col   - col where to write the string                                */
/*      ptr   - -> to the char string with embeded attributes.               */
/* Return:                                                                   */
/*      none                                                                 */
/* Assumptions:                                                              */
/*      It is assumed that this routine is expected to handle one row        */
/*      at a time. It can't handle a big string spanning across more than    */
/*      screen columns.                                                      */
/*****************************************************************************/
void putrc(uint row, uint col, char *ptr)
{
 char  Save_Byte;                       /* Byte saved                     400*/
 char *Save_Ptr;                        /* -> location which was modified 400*/
 char *String_Ptr;                      /* -> string                      400*/
 char  Restore_Flag;                    /* flag to indicate any modificat 400*/
 uchar First_Char;                      /* 1st character                  400*/
 uint  String_Len,temp;                 /*                                400*/
 uchar Count;                           /*                                400*/
 uint  No_Of_Attributes;                /* count of attributes            400*/
 extern int iview;
 /****************************************************************************/
 /* Initialise the flags.                                                    */
 /****************************************************************************/
 No_Of_Attributes =  0;
 Restore_Flag     =  0;
 String_Ptr       =  ptr;
 First_Char       = *String_Ptr;

 /****************************************************************************/
 /* If the column where we want to write the string is beyond the screen     */
 /* bounds then return without doing any thing.                              */
 /****************************************************************************/
 if (iview==2) /*This is used to turn off video writes when in MSH Browse mode.*/
    return;
 if(iview) {
 if (col > BoundPtr[row])
    return;

 if (row + RowStart >= VideoRows  - 2)
    return;
 }
 else
 {
 if (col > BoundPtr[row])
    return;

 if (row > VideoRows )
    return;

 }/* End if*/
 /****************************************************************************/
 /* If the 1st char of the i/p string is 0xFF it tells us that the i/p       */
 /* string is of format as follows                                           */
 /*    0XFF xx  .....                                                        */
 /*     |   |------->  Tells the repeat count or if it is 0xFF it a special  */
 /*     |              case                                                  */
 /*      ----------->  Tells us repeat count is following.                   */
 /*                                                                          */
 /* - Check if the repeat count when added to the col postion where to start */
 /*   painting excceds the screen bounds.                                    */
 /* - If it does set up the flag to restore the contents, save the repeat    */
 /*   count, modify repeat count such that it does not exceed the screen     */
 /*   bounds.                                                                */
 /****************************************************************************/
 if (First_Char == (uchar)0xFF)
 {
    Count = *(++String_Ptr);
    if (Count < (uchar)0xFF)
    {
       temp = (uint)Count + col;
       if (temp > BoundPtr[row])
       {
          Restore_Flag = 1;
          Save_Ptr = String_Ptr;
          Save_Byte = *Save_Ptr;
          *String_Ptr = BoundPtr[row] - col;
       }
    }
 }
 else
 {
    /*************************************************************************/
    /* If the 1st char of the i/p string is less than 0x80 it says that the  */
    /* i/p string is a ordinary string with embeded attributes whose values  */
    /* are greater than 0x7F.                                                */
    /*    xx xx xx xx xx xx xx 0x81 xx xx xx xx                              */
    /*     |                    |---> It is a attribute.                     */
    /*      ----------->  Ordinary charater.                                 */
    /*                                                                       */
    /* - Check when the length of the string added to the col postion where  */
    /*   to start painting excceds the screen bounds.                        */
    /* - If it does scan the string for determining the count of attributes. */
    /* - Set up a flag to restore the string contents                        */
    /* - Find the location where we have to end the string so that it does   */
    /*   cross the screen bounds.                                            */
    /* - save the byte at that location.                                     */
    /* - stuff is a null at that location.                                   */
    /*************************************************************************/
    if (First_Char < (uchar)0x80)
    {
       String_Len = strlen(String_Ptr);
       if ((String_Len + col) > BoundPtr[row])
       {
         Save_Ptr = String_Ptr;
         while (*Save_Ptr)
         {
            if (((uchar)*(Save_Ptr) > (uchar)0x7F) &&
                ((uchar)*(Save_Ptr) < (uchar)0x99) )
               No_Of_Attributes++;
               Save_Ptr++;
         }
         Restore_Flag = 1;
         Save_Ptr = String_Ptr + BoundPtr[row] - col + No_Of_Attributes;
         Save_Byte = *Save_Ptr;
         *Save_Ptr = '\0';
       }
    }
    else
    {
       /**********************************************************************/
       /* If the 1st char of the i/p string is greater than 0x80 & the next  */
       /* char is 0xFF then the format of the i/p string is as follows       */
       /*  0x81  0XFF xx  .....                                              */
       /*   |     |   |------->  Tells the repeat count or if it is 0xFF it  */
       /*   |     |              a special case.                             */
       /*   |      ----------->  Tells us repeat count is following.         */
       /*    ----------------->  Attribute.                                  */
       /*                                                                    */
       /* - Check if the repeat count when added to the col postion where to */
       /*   start painting excceds the screen bounds.                        */
       /* - If it does set up the flag to restore the contents, save the rep */
       /*   count, modify repeat count such that it does not exceed the scr  */
       /*   bounds.                                                          */
       /**********************************************************************/
       if (*(++String_Ptr) == (char)0xFF)
       {
          Count = *(++String_Ptr);
          if (Count < (uchar)0xFF)
          {
             temp = (uint)Count + col;
             if (temp > BoundPtr[row])
             {
                Restore_Flag = 1;
                Save_Ptr = String_Ptr;
                Save_Byte = *Save_Ptr;
                *String_Ptr = BoundPtr[row] - col;
             }
          }
       }
       else
       {
    /*************************************************************************/
    /* If the 1st char of the i/p string is greater than 0x80 it says that   */
    /* i/p string is a ordinary string with embeded attributes whose values  */
    /* are greater than 0x7F.                                                */
    /*  0x81 xx xx xx xx xx xx xx 0x81 xx xx xx xx                           */
    /*   |    |                     |---> It is a attribute.                 */
    /*   |     -------------------------> Ordinary charater.                 */
    /*    ------------------------------> It is a attribute.                 */
    /* - Check when the length of the string added to the col postion where  */
    /*   to start painting excceds the screen bounds.                        */
    /* - If it does scan the string for determining the count of attributes. */
    /* - Set up a flag to restore the string contents                        */
    /* - Find the location where we have to end the string so that it does   */
    /*   cross the screen bounds.                                            */
    /* - save the byte at that location.                                     */
    /* - stuff is a null at that location.                                   */
    /*************************************************************************/
          String_Len = strlen(String_Ptr);
          if ((String_Len + col) > BoundPtr[row])
          {
             Save_Ptr = String_Ptr;
             while (*Save_Ptr)
             {
               if (((uchar)*(Save_Ptr) > (uchar)0x7F) &&
                  ((uchar)*(Save_Ptr) < (uchar)0x99) )
                  No_Of_Attributes++;
                  Save_Ptr++;
             }
             Restore_Flag = 1;
             Save_Ptr = String_Ptr + BoundPtr[row] - col + No_Of_Attributes;
             Save_Byte = *Save_Ptr;
             *Save_Ptr = '\0';
          }
       }
    }
 }

 /****************************************************************************/
 /* Call putrcx to display the string                                        */
 /****************************************************************************/
/* putrcx(row, col, ptr, 1); */
 {
  putrcx(row, col, ptr);
 }
 /****************************************************************************/
 /* If restore flag is set, restore the string its orginal contents          */
 /****************************************************************************/
 if (Restore_Flag)
    *Save_Ptr = Save_Byte;
 }
