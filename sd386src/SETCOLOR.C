/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   setcolor.c                                                              */
/*                                                                           */
/* Description:                                                              */
/*   allow users to modify color settings interactively, and optionally      */
/*   save the new settings to user profile, SD386.PRO.                       */
/*                                                                           */
/* History:                                                                  */
/*                                                                           */
/*   02/08/91 Creation of 32-bit SD386, from 16-bit version.                 */
/*                                                                           */
/*...Release 1.00                                                            */
/*...                                                                        */
/*... 02/08/91  100   made changes for 32-bit compilation.                   */
/*...                                                                        */
/*...Release 1.00 (Pre-release 107 11/13/91)                                 */
/*...                                                                        */
/*... 11/13/91  400   Srinivas  Vertical Register Display.                   */
/*...                                                                        */
/*...Release 1.00 (Pre-release 108 12/05/91)                                 */
/*...                                                                        */
/*... 02/12/92  521   Srinivas  Port to C-Set/2.                             */
/*...                                                                        */
/*...Release 1.01 (04/03/92)                                                 */
/*...                                                                        */
/*... 05/08/92  701   Srinivas  Cua Interface.                               */
/*...                                                                        */
/*...Release 1.05 (05/27/93)                                                 */
/*...                                                                        */
/*... 05/27/93  825   Joe       Reading SD386.PRO after a save causes trap.  */
/*...                                                                        */
/**Includes*******************************************************************/
                                        /*                                   */
#define INCL_16                         /* for 16-bit API                 101*/
#define INCL_SUB                        /* kbd, vio, mouse routines       101*/
#include "all.h"                        /* SD386 include files               */
#define __MIG_LIB__                                                     /*521*/
#include "io.h"                         /* for routines open, chsize, close  */
#include "fcntl.h"                      /* for O_WRONLY, O_TEXT              */
#include "diacolor.h"                   /* colors dialog data             701*/

/**Macros*********************************************************************/
                                        /*                                   */
#define BGColor(c) BGColors[(c)>>(4)]   /* c -> background color name        */
#define FGColor(c) FGColors[(c)&(0x0F)] /* c -> foreground color name        */
                                        /*                                   */
/**Variables defined**********************************************************/

#define VATTLENGTH   23                 /* attribute field length            */
#define BGLENGTH     11                 /* background color field length     */
#define FGLENGTH     14                 /* foreground color field length     */
#define DEMOLENGTH    7                 /* color demo field length           */
#define MAXWINDOWSIZE HEADERSIZE+MAXVIDEOATR*(VATTLENGTH+\
         BGLENGTH+FGLENGTH+DEMOLENGTH)+1 /* window text buffer size          */
#define FIELDSLENGTH  BGLENGTH+FGLENGTH+DEMOLENGTH+2                    /*701*/
#define MAXITEM       MAXVIDEOATR       /* max number of lines in window     */
#define FILENOTFOUND  2                 /* file-not-found return code        */

char *vaTypes[MAXVIDEOATR+1] =          /* video attribute types             */
{                                       /*                                   */
"",                                     /* null       0                      */
"ProgramLine",                          /* vaProgram  1                      */
"LinesOkayForBrkPoint",                 /* vaBptOk    2                      */
"LinesWithBrkPoint",                    /* vaBptOn    3                      */
"ExecLineWithNoBrkPt",                  /* vaXline    4                      */
"ExecLineWithBrkPt",                    /* vaXlineOn  5                      */
"MenuBar",                              /* vaMenuBar  6                      */
"MenuBarCursor",                        /* vaMenuCsr  7                      */
"StorageExpression",                    /* vaStgExp   8                      */
"StorageValues",                        /* vaStgVal   9                      */
"RegisterWindow",                       /* vaRegWind 10                      */
"MenuPrompts",                          /* vaPrompt  11                      */
"ErrorMessages",                        /* vaError   12                      */
"FileAndPosition",                      /* vaInfo    13                      */
"HelpPanels",                           /* vaHelp    14                      */
"StorageExprsnPrompts",                 /* vaStgPro  15                      */
"ClearScreenOnExit",                    /* vaClear   16                      */
"DataWindowStatusLine",                 /* vaStgStat 17                      */
"AsmWindowSourceLines",                 /* vaMxdSrc  18                      */
"MenuBarItemSelected",                  /* vaMenuSel 19                      */
"SelectionWindow",                      /* vaCallStk 20                      */
"SelectWinItemSelected"                 /* vaCallSel 21                      */
};                                      /*                                   */
char *BGColors[MAXBGCOLOR] =            /* background color names            */
{                                       /*                                   */
"BG_BLACK",                             /* 0x00  black                       */
"BG_BLUE",                              /* 0x10  blue                        */
"BG_GREEN",                             /* 0x20  green                       */
"BG_CYAN",                              /* 0x30  cyan                        */
"BG_RED",                               /* 0x40  red                         */
"BG_MAGENTA",                           /* 0x50  magenta                     */
"BG_BROWN",                             /* 0x60  brown                       */
"BG_WHITE"                              /* 0x70  white                       */
};                                      /*                                   */
char *FGColors[MAXFGCOLOR] =            /* foreground color names            */
{                                       /*                                   */
"FG_BLACK",                             /* 0x00  black                       */
"FG_BLUE",                              /* 0x01  blue                        */
"FG_GREEN",                             /* 0x02  green                       */
"FG_CYAN",                              /* 0x03  cyan                        */
"FG_RED",                               /* 0x04  red                         */
"FG_MAGENTA",                           /* 0x05  magenta                     */
"FG_BROWN",                             /* 0x06  brown                       */
"FG_WHITE",                             /* 0x07  white                       */
"FG_GREY",                              /* 0x08  light black   = grey        */
"FG_LT_BLUE",                           /* 0x09  light blue                  */
"FG_LT_GREEN",                          /* 0x0A  light green                 */
"FG_LT_CYAN",                           /* 0x0B  light cyan                  */
"FG_LT_RED",                            /* 0x0C  light red                   */
"FG_LT_MAGENTA",                        /* 0x0D  light magenta               */
"FG_YELLOW",                            /* 0x0E  light brown   = yellow      */
"FG_LT_WHITE"                           /* 0x0F  light white                 */
};                                      /*                                   */
                                        /*                                   */
/**Externs********************************************************************/
                                        /*                                   */
extern uchar   ExceptionMap[MAXEXCEPTIONS];
extern uchar   LocalExcepMap[MAXEXCEPTIONS];
extern uchar   VideoAtr;                /* logical screen attribute          */
extern uchar  *VideoMap;                /* logical->physical attribute map   */
extern uchar   ScrollShade1[];          /* scroll bar attributes          701*/
extern uchar   ScrollShade2[];          /* scroll bar attributes          701*/
extern uchar   hilite[];                /* high light field attributes    701*/
extern uchar   normal[];                /* normal field attributes        701*/
extern uchar   ClearField[];            /* clear field attributes         701*/
extern uchar   InScrollMode;            /* flag to tell we are in scrollmode */

extern CmdParms cmd;
extern VIOCURSORINFO  NormalCursor;     /* make underscore flashing cursor   */

static uchar  DemoColor[] = { RepCnt(DEMOLENGTH+1), Attrib(0), 0 };
                                        /* color for demo Sample field       */
static uchar LocalVaMap[MAXVIDEOATR+1]; /* local copy of va map              */
static uchar DefVaMap[MAXVIDEOATR+1];   /* constant copy of default va map   */
static enum  { AttrField,               /* user currently at attribute field */
               BGField,                 /* user currently at bgcolor field   */
               FGField                  /* user currently at fgcolor field   */
             } fld;                     /* user current field position       */

/**Begin Code*****************************************************************/

/*****************************************************************************/
/* SaveColors()                                                              */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Save the attribute names and their associated color names to            */
/*   profile SD386.PRO.                                                      */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   fn        input - filename to save color values                         */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   0         Save sucessful                                                */
/*   1         Save unsucessful                                              */
/*                                                                           */
/*****************************************************************************/
                                        /*                                   */
 uint                                   /*                                   */
SaveColors( uchar *fn )                 /*                                   */
{                                       /*                                   */
 FILE   *fp;                            /* file ptr to SD386.PRO             */
 char    _word[90];                     /* buffer to scan for Start_Of_Colors*/
 uchar   line[VATTLENGTH+BGLENGTH];     /* to contain attr and color names   */
 uchar  *pText;                         /* pointer to a text string          */
 fpos_t  pos;                           /* file position                     */
 int     fh;                            /* file handle for SD386.PRO         */
 uchar  *pAttByte = LocalVaMap + 1;     /* video attribute byte index        */
 uint    i;                             /* attribute item index              */
                                        /*                                   */
 if( (fp = fopen(fn, "r+")) == NULL )   /* if can not open SD386.PRO,        */
  return( FILENOTFOUND );               /* return bad code                   */
                                        /*                                   */
 while(fscanf(fp, "%s", _word) != EOF && /* scan while not end of file and   */
       stricmp(_word, "Start_Of_Colors") /* not found "Start_Of_Colors"      */
      ){;}                              /*                                   */
 if( feof( fp ) )                       /* if "Start_Of_Colors" not exist,   */
  fprintf( fp, "\n\n      Start_Of_Colors");/* put it into profile           */
 else                                   /* else found "Start_Of_Colors",     */
 {                                      /*                                   */
  if( fgetpos( fp, &pos) ||             /* initialize file header position   */
      fsetpos( fp, &pos)                /* for future writes to file         */
    )                                   /*                                   */
   return( 1 );                         /* if bad file access, return error  */
 }                                      /*                                   */
                                        /*                                   */


 /****************************************************************************/
 /* At this point, fscanf has left us pointing like so:                   825*/
 /*                                                                       825*/
 /*   Start_Of_Colors0D0A                                                 825*/
 /*                    |                                                  825*/
 /*  fscanf------------                                                   825*/
 /*                                                                       825*/
 /* - We need to bump the file position past the 0A.                      825*/
 /*                                                                       825*/
 /****************************************************************************/
 {                                                                      /*825*/
  pos.__fpos_elem[0] += 1;                                              /*825*/
  fsetpos( fp, &pos);                                                   /*825*/
 }                                                                      /*825*/
                                                                        /*825*/
 fprintf(fp, "/*\n         ATTRIBUTE TYPE     BACKGROUND  FOREGROUND");/*  */
 fprintf(fp,   "\n      --------------------  ---------- -------------\n*/");
                                        /* write text after "Start_Of_Colors"*/
 /****************************************************************************/
 /* From the first attribute item until last, write the attribute name and   */
 /* its background and foreground colors out to file SD386.PRO.              */
 /****************************************************************************/
 line[VATTLENGTH+BGLENGTH-1] = 00;      /* line terminator                   */
 for( i = 1;                            /* start with first attribute item   */
      i <= MAXVIDEOATR;                 /* until the last attribute item     */
      i++, pAttByte++                   /* increment attr and color indices  */
    )                                   /* loop for all attribute items      */
 {                                      /*                                   */
  memset(line, ' ', VATTLENGTH+BGLENGTH-1);/* pad buffer with all blanks     */
                                        /*                                   */
  pText = vaTypes[i];                   /* get attribute name                */
  strncpy(line, pText, strlen(pText));  /* copy attribute name to buffer     */
                                        /*                                   */
  pText = BGColor(*pAttByte);           /* get background color name         */
  strncpy(line + VATTLENGTH,            /* copy background color name        */
          pText, strlen(pText));        /* to buffer                         */
                                        /*                                   */
  fprintf(fp, "\n      %s %s",          /* write buffer and foreground color */
          line, FGColor(*pAttByte));    /* name out to file                  */
 }                                      /*                                   */
 /****************************************************************************/
 /* After we are finished writing to the file, we now want to be certain to  */
 /* erase all junk text after our writing to the file.                       */
 /****************************************************************************/
 if( fgetpos( fp, &pos)  ||             /* get last position after our write */
     fclose( fp ) == EOF ||             /* close SD386.PRO                   */
    (fh = open(fn, O_WRONLY | O_TEXT))  /* open SD386.PRO to adjust file size*/
        == EOF           ||             /*                                   */
     chsize(fh, pos.__fpos_elem[0])  || /* truncate junk after last write 521*/
     close( fh )                        /* close SD386.PRO                   */
   )                                    /*                                   */
  return( 1 );                          /* if bad file access, return error  */
 /****************************************************************************/
 /* copy the global exceptions map to local exception map array and call  521*/
 /* save exception to write the exception map to profile.                 521*/
 /****************************************************************************/
 memcpy(LocalExcepMap, ExceptionMap, MAXEXCEPTIONS);                    /*521*/
 SaveExceptions( fn ) ;                                                 /*521*/
 return( 0 );                           /* else, return okay                 */
}                                       /* end SaveColors                    */

/*****************************************************************************/
/* Setcolors()                                                            701*/
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*     Calls functions to display, process key & mouse events & remove the   */
/*  colors dialog.                                                           */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/* Return:                                                                   */
/*   none                                                                    */
/*****************************************************************************/
void SetColors()
{
  /***************************************************************************/
  /*  - Get the Screen write access.                                         */
  /*  - Display the Colors Dialog.                                           */
  /*  - Process the Events in the Colors Dialog.                             */
  /*  - Remove the Colors Dialog.                                            */
  /*  - Restore the Screen write access.                                     */
  /***************************************************************************/
  GetScrAccess();
  DisplayDialog( &Dia_Color, TRUE );
  ProcessDialog( &Dia_Color, &Dia_Color_Choices, TRUE, NULL );
  RemoveDialog( &Dia_Color );
  SetScrAccess();
}

/*****************************************************************************/
/* DisplayColorChoice()                                                      */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Displays the color names and the sample demo color in the dialog win.   */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   shell   ->  pointer to a dialog shell structure.                        */
/*   ptr     ->  pointer to a dialog choice structure.                       */
/*                                                                           */
/* Return:                                                                   */
/*   none                                                                    */
/*****************************************************************************/
void  DisplayColorChoice( DIALOGSHELL *shell, DIALOGCHOICE *ptr )
{
  uint   i;
  uint   StrLength;                     /*                                   */
  uchar  StartIndex;                    /*                                   */
  uchar *GlobalVaMap = VideoMap;        /* global va map                     */
  uchar *pAttByte;                      /* video attribute type index        */

  /***************************************************************************/
  /* init the variables required.                                            */
  /***************************************************************************/
  pAttByte = LocalVaMap + 1;
  StartIndex = 1;

  /***************************************************************************/
  /* Advance the startindex and attribute ptrs depending on the skip rows.   */
  /***************************************************************************/
  for( i = 0; i < ptr->SkipRows; i++ )
   {
    pAttByte++;
    StartIndex++;
   }

  /***************************************************************************/
  /* Display the rows that can fit in the window.                            */
  /***************************************************************************/
  for( i = 0; i < ptr->MaxRows; i++, StartIndex++, pAttByte++ )
   {
    ClearField[1] = 1;
    putrc( shell->row + i + shell->SkipLines, shell->col + 1, ClearField );
    putrc( shell->row + i + shell->SkipLines, shell->col + 2, vaTypes[StartIndex] );
    StrLength = strlen( vaTypes[StartIndex] );
    ClearField[1] = shell->width - 6 - StrLength;
    putrc( shell->row + i + shell->SkipLines, shell->col + 2 + StrLength, ClearField );
    putrc( shell->row + i + shell->SkipLines, shell->col + 2 + VATTLENGTH,
           BGColor(*pAttByte));
    putrc( shell->row + i + shell->SkipLines, shell->col + 2 + VATTLENGTH + BGLENGTH,
           FGColor(*pAttByte) );
    /*************************************************************************/
    /*  - Set up the demo sample color field.                                */
    /*  - Get the sample color from the local map.                           */
    /*  - Put the demo sample color field.                                   */
    /*  - Restore to orginal window color map.                               */
    /*************************************************************************/
    DemoColor[2] = Attrib(StartIndex);
    putrc( shell->row + i + shell->SkipLines,
           shell->col + 2 + VATTLENGTH + BGLENGTH + FGLENGTH, "Sample" );
    VideoMap = LocalVaMap;
    putrc( shell->row + i + shell->SkipLines,
           shell->col + 2 + VATTLENGTH + BGLENGTH + FGLENGTH - 1, DemoColor );
    VideoMap = GlobalVaMap;
  }
}

/*****************************************************************************/
/* ReDisplaySingleColor()                                                 701*/
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Re Display a single row of color names and attributes.                  */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   ScrRow   ->  Where to refresh the color names row position              */
/*   ScrCol   ->  Where to refresh the color names col position              */
/*   SelIndex ->  Index into various color selection array.                  */
/*   pattbyte ->  pointer to attributes in the map.                          */
/*                                                                           */
/* Return:                                                                   */
/*   none                                                                    */
/*****************************************************************************/
 void
ReDisplaySingleColor (uint ScrRow,uint ScrCol,uint SelIndex,uchar *pAttByte)
{
  uchar *GlobalVaMap = VideoMap;

  ClearField[1] = BGLENGTH+FGLENGTH;
  putrc(ScrRow,ScrCol+VATTLENGTH,ClearField);
  putrc(ScrRow,ScrCol+VATTLENGTH,BGColor(*pAttByte));
  putrc(ScrRow,ScrCol+VATTLENGTH+BGLENGTH,FGColor(*pAttByte));
  /***************************************************************************/
  /*  - Set up the demo sample color field.                                  */
  /*  - Get the sample color from the local map.                             */
  /*  - Put the demo sample color field.                                     */
  /*  - Restore to orginal window color map.                                 */
  /***************************************************************************/
  DemoColor[2] = Attrib(SelIndex);
  putrc(ScrRow,ScrCol+VATTLENGTH+BGLENGTH+FGLENGTH,"Sample");
  VideoMap = LocalVaMap;
  putrc(ScrRow,ScrCol+VATTLENGTH+BGLENGTH+FGLENGTH-1,DemoColor);
  VideoMap = GlobalVaMap;
}

/*****************************************************************************/
/* SetDefaultColorMap()                                                   701*/
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Copy Videocolors map to a default map used after wards.                 */
/*                                                                           */
/* Parameters:                                                               */
/*   none                                                                    */
/*                                                                           */
/* Return:                                                                   */
/*   none                                                                    */
/*****************************************************************************/
void SetDefaultColorMap()
{
  memcpy(DefVaMap, VideoMap, MAXVIDEOATR+1);
}

/*****************************************************************************/
/*                                                                           */
/* ColorDialogFunction()                                                     */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/* This is the function called by ProcessDialog (in pulldown.c) whenever an  */
/* event occurs in the dialog. This fuction handles the events in a way      */
/* specific to this dialog.                                                  */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   shell         input  - Pointer to DIALOGSHELL structure.                */
/*   ptr           input  - Pointer to DIALOGCHOICE structure.               */
/*   nEvent        input  - Pointer to EVENT structure.                      */
/*                                                                           */
/* Return:                                                                   */
/*        = 0     - The caller has to continue processing the dialog.        */
/*       != 0     - The caller can terminate processing the dialog.          */
/*****************************************************************************/
uint  ColorDialogFunction( DIALOGSHELL *shell, DIALOGCHOICE *ptr,
                           EVENT *nEvent, void *ParamBlock )
{
  uint   fldcol;
  uchar  *pAttByte;                     /* Pointer to color map.             */
  int    SelectIndex;                   /* Index into color map array.       */

  SelectIndex = shell->CurrentField + ptr->SkipRows;
  pAttByte = LocalVaMap + SelectIndex;
  for( ;; )
  {
    switch( nEvent->Value )
    {
      case INIT_DIALOG:
      {
        /*********************************************************************/
        /* This message is sent before displaying elements in display area.  */
        /* It is to initialise the locals in the dialog function.            */
        /*  - Make a copy of the global color map.                           */
        /*  - Set the field cursor and variables to the first field.         */
        /*  - Set the length of the field highlite bar in normal and hilite  */
        /*    arrays (used in the call to putrc).                            */
        /*  - Return zero to denote continue dialog processing.              */
        /*********************************************************************/
        memcpy( LocalVaMap, VideoMap, MAXVIDEOATR + 1 );
        fldcol = shell->col + 2;
        VioSetCurPos( (short)(shell->row + shell->SkipLines + shell->CurrentField - 1),
                      (short)fldcol, 0 );
        fld = AttrField;

        hilite[1] = normal[1] = (UCHAR)shell->width - SCROLLBARCOLOFFSET -
                                         (FIELDSLENGTH) - 2;
        return( 0 );
      }

      case ENTER:
      {
        /*********************************************************************/
        /* This message is sent if the user clicks on the ENTER button or    */
        /* presses the ENTER key.                                            */
        /*  - Copy the local color map to the global color map.              */
        /*  - Return non zero value to denote the end of dialog processing.  */
        /*********************************************************************/
        ptr->SkipRows = 0;
        memcpy( VideoMap, LocalVaMap, MAXVIDEOATR + 1 );
        return( ENTER );
      }

      case F1:
      {
        uchar  *HelpMsg;

        HelpMsg = GetHelpMsg( HELP_DLG_COLORS, NULL,0 );
        CuaShowHelpBox( HelpMsg );
        return( 0 );
      }

      case ESC:
      case F10:
      {
        /*********************************************************************/
        /* This message is sent if the user clicks on the CANCEL button or   */
        /* presses the ESC/F10 keys.                                         */
        /*  - Return non zero value to denote the end of dialog processing.  */
        /*********************************************************************/
        ptr->SkipRows = 0;
        return( nEvent->Value );
      }

      case MOUSEPICK:
      {
        /*********************************************************************/
        /* This message is sent if the clicks in the dialog display area.    */
        /*  - Determine on which field the user clicked.                     */
        /*  - Set the cursor on that field.                                  */
        /*  - If the field turns out to be either Background or Foreground   */
        /*    color fields,                                                  */
        /*     - Set the event value as SPACEBAR (simulate) and stay in the  */
        /*       outer for loop to process SPACEBAR.                         */
        /*    Else                                                           */
        /*     - Return zero to denote continue dialog processing.           */
        /*********************************************************************/
        if( ((uint)nEvent->Col >= (shell->col+2 + VATTLENGTH)) &&
            ((uint)nEvent->Col < (shell->col+2 + VATTLENGTH + BGLENGTH)))
        {
           fldcol = shell->col+ 2 + VATTLENGTH;
           fld = BGField;
        }
        else
        {
          if( ((uint)nEvent->Col >= (shell->col+ 2 + VATTLENGTH + BGLENGTH)) &&
              ((uint)nEvent->Col < (shell->col+ 2 + VATTLENGTH + BGLENGTH +
                                    FGLENGTH)))
          {
            fldcol = shell->col+2 + VATTLENGTH + BGLENGTH;
            fld = FGField;
          }
          else
          {
            fldcol = shell->col+2;
            fld = AttrField;
          }
        }

        VioSetCurPos( (short)(shell->row + shell->SkipLines +
                      shell->CurrentField - 1), (short)fldcol, 0 );

        if( (fld == BGField) || (fld == FGField) )
        {
          nEvent->Value = SPACEBAR;
          continue;
        }
        return( 0 );
      }

      case BGNEXT:
      {
        /*********************************************************************/
        /* This message is sent if the user clicks in the BGNEXT button or   */
        /* presses SPACEBAR when on background field.                        */
        /*  - Set the field variables.                                       */
        /*  - Set the event value as SPACEBAR (simulate) and stay in the     */
        /*    outer for loop to process SPACEBAR.                            */
        /*  - Set the cursor position.                                       */
        /*********************************************************************/
        fldcol = shell->col+2 + VATTLENGTH;
        fld = BGField;
        nEvent->Value = SPACEBAR;
        VioSetCurPos( (short)(shell->row + shell->SkipLines +
                       shell->CurrentField - 1), (short)fldcol, 0 );
        continue;
      }

      case FGNEXT:
      {
        /*********************************************************************/
        /* This message is sent if the user clicks in the FGNEXT button or   */
        /* presses SPACEBAR when on foreground field.                        */
        /*  - Set the field variables.                                       */
        /*  - Set the event value as SPACEBAR (simulate) and stay in the     */
        /*    outer for loop to process SPACEBAR.                            */
        /*  - Set the cursor position.                                       */
        /*********************************************************************/
        fldcol = shell->col+2 + VATTLENGTH + BGLENGTH;
        fld = FGField;
        nEvent->Value = SPACEBAR;
        VioSetCurPos( (short)(shell->row + shell->SkipLines +
                       shell->CurrentField - 1), (short)fldcol, 0 );
        continue;
      }

      case key_R:
      case key_r:
      {
        /*********************************************************************/
        /* This message is sent if the user clicks in the RESET button or    */
        /* presses 'R' / 'r' keys.                                           */
        /*  - Reset the current color to default color.                      */
        /*  - Change the current color attribute and display the same in the */
        /*    "Sample" area of the dialog.                                   */
        /*  - Return zero to denote continue dialog processing.              */
        /*********************************************************************/
        *pAttByte = DefVaMap[SelectIndex];
        ReDisplaySingleColor( shell->row + shell->SkipLines +
                              shell->CurrentField - 1, shell->col + 2,
                              SelectIndex, pAttByte );
        return( 0 );
      }

      case key_D:
      case key_d:
      {
        /*********************************************************************/
        /* This message is sent if the user clicks in the DEFAULT button or  */
        /* presses 'D' / 'd' keys.                                           */
        /*  - Copy the default color map to local color map.                 */
        /*  - Redisplay the display area of the dialog.                      */
        /*  - Return zero to denote continue dialog processing.              */
        /*********************************************************************/
        memcpy( LocalVaMap, DefVaMap, MAXVIDEOATR + 1 );
        DisplayColorChoice( shell, ptr );
        return( 0 );
      }

      case key_S:
      case key_s:
      {
        /*********************************************************************/
        /* This message is sent if the user clicks on the SAVE button or     */
        /* presses the 'S' / 's' keys.                                       */
        /*  - Find the fully qualified file name for the .pro                */
        /*  - Call function to save the colors.                              */
        /*  - If not successfull in saving colors, display the error box     */
        /*    with appropriate error message.                                */
        /*  - If successfull display a timmed message and copy the local     */
        /*    color map to default color map.                                */
        /*  - Return zero to denote continue dialog processing.              */
        /*********************************************************************/
        uchar  StatusMsg[100];
        uchar  fn[MAXFNAMELEN];
        int    rc;

        findpro( "SD386.PRO", fn, MAXFNAMELEN );
        rc = SaveColors( fn );
        if( rc )
        {
          if( rc == FILENOTFOUND )
            sprintf( StatusMsg, "Unable to find \"SD386.PRO\" in DPATH" );
          else
            sprintf( StatusMsg, "Unable to save to profile \"%s\"", fn );
        }
        else
        {
          sprintf( StatusMsg, "Saving Colors to profile \"%s\"", fn );
          memcpy( VideoMap, LocalVaMap, MAXVIDEOATR + 1 );
        }
        SayMsgBox2( StatusMsg, 1200UL );
        VioSetCurType( &NormalCursor, 0 );
        return( 0 );
      }

      case RIGHT:
      case TAB:
      {
        /*********************************************************************/
        /* This message is sent if the user presses TAB or right arrow  key.*/
        /*  - Depending on the current field determine to which field we have*/
        /*    to move to the right or loop around.                           */
        /*  - Set the field variables.                                       */
        /*  - Set the cursor position.                                       */
        /*  - Return zero to denote continue dialog processing.              */
        /*********************************************************************/
        switch( fld )
        {
          case AttrField:
            fldcol = shell->col+ 2 + VATTLENGTH;
            fld = BGField;
            break;

          case BGField:
            fldcol = shell->col+ 2 + VATTLENGTH + BGLENGTH;
            fld = FGField;
            break;

          case FGField:
            fldcol = shell->col+2;
            fld = AttrField;
            break;
        }
        VioSetCurPos( (short)(shell->row + shell->SkipLines +
                       shell->CurrentField - 1), (short)fldcol, 0 );
        return( 0 );
      }

      case LEFT:
      case S_TAB:
      {
        /*********************************************************************/
        /* This message is sent if the user presses SHIFT TAB or left arrow  */
        /*  key.                                                            */
        /*  - Depending on the current field determine to which field we have*/
        /*    to move to the left or loop around.                            */
        /*  - Set the field variables.                                       */
        /*  - Set the cursor position.                                       */
        /*  - Return zero to denote continue dialog processing.              */
        /*********************************************************************/
        switch( fld )
        {
          case AttrField:
            fldcol = shell->col+2 + VATTLENGTH + BGLENGTH;
            fld = FGField;
            break;

          case BGField:
            fldcol = shell->col+2;
            fld = AttrField;
            break;

          case FGField:
            fldcol = shell->col+2 + VATTLENGTH;
            fld = BGField;
            break;
        }
        VioSetCurPos( (short)(shell->row + shell->SkipLines +
                       shell->CurrentField - 1), (short)fldcol, 0 );
        return( 0 );
      }

      case SPACEBAR:
      {
        /*********************************************************************/
        /* This message is sent if the user presses the SPACEBAR.            */
        /*  - If the cursor is in the forground or background fields move on */
        /*    to the next color.                                             */
        /*  - Change the current color attribute and display the same in the */
        /*    "Sample" area of the dialog.                                   */
        /*  - Return zero to denote continue dialog processing.              */
        /*********************************************************************/
        switch( fld )
        {
          case BGField:
            *pAttByte = (uchar) ((*pAttByte + 0x10) & 0x7F);
            ReDisplaySingleColor( shell->row + shell->SkipLines +
                                  shell->CurrentField - 1, shell->col + 2,
                                  SelectIndex, pAttByte );
            break;

          case FGField:
            *pAttByte = (uchar) ((*pAttByte & 0xF0) +
                                ((*pAttByte + 0x01) & 0x0F));
            ReDisplaySingleColor( shell->row + shell->SkipLines +
                                  shell->CurrentField - 1, shell->col + 2,
                                  SelectIndex, pAttByte );
            break;
        }
        return( 0 );
      }

      case UP:
      case DOWN:
      case SCROLLUP:
      case SCROLLDOWN:
      case SCROLLBAR:
      {
        /*********************************************************************/
        /* These messages are sent if the user presses up  or down  arrow  */
        /* keys or the user clicks on the  or  in the scrollbar.           */
        /*  - Set the field variables.                                       */
        /*  - Set the cursor position.                                       */
        /*  - Return zero to denote continue dialog processing.              */
        /*********************************************************************/
        switch( fld )
        {
          case AttrField:
            fldcol = shell->col+2;
            break;

          case BGField:
            fldcol = shell->col+2 + VATTLENGTH;
            break;

          case FGField:
            fldcol = shell->col+2 + VATTLENGTH + BGLENGTH;
            break;
        }

        VioSetCurPos( (short)(shell->row + shell->SkipLines +
                       shell->CurrentField - 1), (short)fldcol, 0 );
        return( 0 );
      }

      default:
        /*********************************************************************/
        /* Any other message.                                                */
        /* - Return zero to denote continue dialog processing.               */
        /*********************************************************************/
        return( 0 );
    }
  }
}
