/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   cuaclstk.c                                                              */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Cua call stack dialog functions.                                        */
/*                                                                           */
/*...Release 1.01 (04/03/92)                                                 */
/*...                                                                        */
/*... 05/08/92  701   Samuel    Cua Interface.                               */
/*****************************************************************************/
#include "all.h"

extern uint    NActFrames;
extern uint    ActFaddrs[];

extern uchar   VideoAtr;                /* logical screen attribute          */
extern uchar   ScrollShade1[];          /* scroll bar attributes          701*/
extern uchar   ScrollShade2[];          /* scroll bar attributes          701*/
extern uchar   hilite[];                /* high light field attributes    701*/
extern uchar   normal[];                /* normal field attributes        701*/
extern uchar   ClearField[];            /* clear field attributes         701*/

extern CmdParms      cmd;
extern PtraceBuffer  AppPTB;
/*****************************************************************************/
/*                                                                           */
/* ClstkDialogFunction()                                                     */
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
uint  ClstkDialogFunction( DIALOGSHELL *shell, DIALOGCHOICE *ptr,
                           EVENT *nEvent, void *ParamBlock )
{
  int     SelectIndex;
  AFILE   *fp;
  uint    addr;

  SelectIndex = shell->CurrentField + ptr->SkipRows;
  switch( nEvent->Value )
  {
    case INIT_DIALOG:
    {
      /*********************************************************************/
      /* This message is sent before displaying elements in display area.  */
      /* It is to initialise the locals in the dialog function.            */
      /*  - Set the length of the field highlite bar in normal and hilite  */
      /*    arrays (used in the call to putrc).                            */
      /*  - Return zero to denote continue dialog processing.              */
      /*********************************************************************/
      hilite[1] = normal[1] = (UCHAR)shell->width - SCROLLBARCOLOFFSET - 2;

      shell->CurrentField = ptr->entries;
      return( 0 );
    }

    case ENTER:
    {
      AFILE   **fpp;

      fpp = ((CALLSTACKPARAM *)ParamBlock)->fpp;

      addr = ( (SelectIndex <= NActFrames) ?
               ActFaddrs[ NActFrames - SelectIndex ] : GetExecAddr() );

      if( (fp = findfp( addr )) != NULL )
      {
        fp->flags |= AF_ZOOM;
        *fpp = fp;
      }
      return( ENTER );
    }

    case key_a:
    case key_n:
    case key_A:
    case key_N:
    {
      int  *ip = ((CALLSTACKPARAM *)ParamBlock)->rc;

      *ip = RECIRCULATE;
      return( nEvent->Value );
    }

    case A_ENTER:
    case C_ENTER:
    {
      addr = ( (SelectIndex <= NActFrames) ?
               ActFaddrs[ NActFrames - SelectIndex ] : GetExecAddr() );

      if( (SelectIndex <= NActFrames)  &&
          (fp = findfp(addr) )
        )
      {
       SetAddrBRK( fp, addr, BRK_ONCE);
       return( A_ENTER );
      }

      beep();
      return( 0 );
    }

    case F1:
    {
      uchar  *HelpMsg;

      HelpMsg = GetHelpMsg( HELP_DLG_CALLSTACK, NULL,0);
      CuaShowHelpBox( HelpMsg );
      return( 0 );
    }

    case ESC:
    case F10:
    {
      /*********************************************************************/
      /* This message is sent if the user clicks on the ENTER button or    */
      /* presses the ENTER key.                                            */
      /*  - Return non zero value to denote the end of dialog processing.  */
      /*********************************************************************/
      ptr->SkipRows = 0;
      return( ESC );
    }

    default:
      return( 0 );
  }
}

/*****************************************************************************/
/* DisplayClstkChoice()                                                   701*/
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Displays the function names in the dialog window.                       */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   shell   ->  pointer to a dialog shell structure.                        */
/*   ptr     ->  pointer to a dialog choice structure.                       */
/*                                                                           */
/* Return:                                                                   */
/*   none                                                                    */
/*****************************************************************************/
void  DisplayClstkChoice( DIALOGSHELL *shell, DIALOGCHOICE *ptr )
{
  uint   i;
  uint   StartIndex = 1;
  uint   StrLength;
  uchar  *str;
  uint   BorderCols;

  BorderCols = 6;

  str = ptr->labels;

  /***************************************************************************/
  /* Advance the startindex and -> to func names depending on the skip rows  */
  /***************************************************************************/
  for( i = 0; i < ptr->SkipRows; i++ )
  {
    str += (strlen( str ) + 1);
    StartIndex++;
  }

  /***************************************************************************/
  /* Display the rows that can fit in the window.                            */
  /***************************************************************************/
  for( i = 0; i < ptr->MaxRows; i++ )
  {
    if( StartIndex <= ptr->entries )
    {
      ClearField[1] = 1;
      putrc( shell->row + i + shell->SkipLines, shell->col + 1, ClearField );
      putrc( shell->row + i + shell->SkipLines, shell->col + 2, str );
      StrLength = strlen( str );
      str += (StrLength + 1);
      StartIndex++;
      ClearField[1] = shell->width - BorderCols - StrLength;
      putrc( shell->row + i + shell->SkipLines, shell->col + 2 + StrLength,
             ClearField );
    }
    else
    {
      ClearField[1] = shell->width-2-SCROLLBARCOLOFFSET;
      putrc( shell->row + i + shell->SkipLines, shell->col + 1, ClearField );
    }
  }
}
