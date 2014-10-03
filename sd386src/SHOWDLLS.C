/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   ShowDlls.c                                                              */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*  Display names of DLLs                                                    */
/*                                                                           */
/* History:                                                                  */
/*                                                                           */
/*   02/08/91 Creation of 32-bit SD86, from 16-bit version.                  */
/*                                                                           */
/*...Release 1.00 (Pre-release 108 12/05/91)                                 */
/*...                                                                        */
/*... 02/21/92  521   Srinivas  Port to C-Set/2.                             */
/*...                                                                        */
/*...Release 1.01 (04/03/92)                                                 */
/*...                                                                        */
/*... 05/08/92  701   Samuel    Cua Interface.                               */
/**Includes*******************************************************************/
#include "all.h"                        /* SD86 include files                */
#include "diadll.h"                     /* dlls dialog data               701*/

/**External declararions******************************************************/
extern uint     VideoRows;              /* # of rows per screen              */
extern uint     VideoCols;              /* # of columns per screen           */
extern uchar    VideoAtr;               /* video attribute                   */
extern KEY2FUNC defk2f[];               /* init key to function lookup table */
extern uint     ProcessID;              /* process id.                       */
extern uchar    VideoAtr;               /* logical screen attribute          */
extern uchar    ScrollShade1[];         /* scroll bar attributes          701*/
extern uchar    ScrollShade2[];         /* scroll bar attributes          701*/
extern uchar    hilite[];               /* high light field attributes    701*/
extern uchar    normal[];               /* normal field attributes        701*/
extern uchar    ClearField[];           /* clear field attributes         701*/
extern uchar    InScrollMode;           /* flag to tell we are in scrollmode */
extern CmdParms cmd;
extern PROCESS_NODE *pnode;                                             /*827*/

/*****************************************************************************/
/* ShowDlls()                                                             701*/
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   show names of DLLs linked to EXE file in the new cua format.            */
/*                                                                           */
/* Parameters:                                                               */
/*   none.                                                                   */
/*                                                                           */
/* Return:                                                                   */
/*   none.                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*   1. The node for EXE file always exists (i.e. pdf is never NULL)         */
/*****************************************************************************/
void ShowDlls()
{

  /***************************************************************************/
  /* As the actual dimensions of the dialog box is determined at a later     */
  /* point of time, the display of the dialog is also deferred until that    */
  /* point (INIT_DIALOG in the dialog function of the dialog).               */
  /***************************************************************************/
  ProcessDialog( &Dia_Dll, &Dia_Dll_Choices, TRUE, NULL );
  RemoveDialog( &Dia_Dll );
}

/*****************************************************************************/
/*                                                                           */
/* DllDialogFunction()                                                       */
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
uint  DllDialogFunction( DIALOGSHELL *shell, DIALOGCHOICE *ptr,
                         EVENT *nEvent, void *ParamBlock )
{
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
      int      NumOfDll, StrLen, MaxStrLen = 0;
      DEBFILE *cur_dll;
      DEBFILE *pdf;

      pdf = pnode->ExeStruct;

      cur_dll = pdf->next;
      NumOfDll = 0;
      while( cur_dll != NULL )
      {
        NumOfDll++;
        StrLen = strlen( cur_dll->DebFilePtr->fn );
        MaxStrLen = MaxStrLen > StrLen ? MaxStrLen : StrLen;
        cur_dll = cur_dll->next;
      }

      shell->width = (MaxStrLen > (MAXSHDLLWINSIZE - 6)) ? MAXSHDLLWINSIZE : (MaxStrLen + 6);

      shell->col = (VideoCols - shell->width) / 2;

      if( shell->Buttons )
        shell->Buttons[0].col = shell->col +
                                (shell->width - shell->Buttons[0].length)/2;

      ptr->entries = NumOfDll;

      hilite[1] = normal[1] = (UCHAR)shell->width - SCROLLBARCOLOFFSET - 2;
      DisplayDialog( shell, TRUE );
    }
    return( 0 );

    case ESC:
    case F10:
    case ENTER:
    {
      /*********************************************************************/
      /* This message is sent if the user clicks on the ENTER button or    */
      /* presses the ENTER key.                                            */
      /*  - Return non zero value to denote the end of dialog processing.  */
      /*********************************************************************/
      ptr->SkipRows = 0;
      return( ENTER );
    }

    default:
      return( 0 );
  }
}

/*****************************************************************************/
/* DisplayDllChoice()                                                     701*/
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Displays the dll names in the dialog window.                            */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   shell   ->  pointer to a dialog shell structure.                        */
/*   ptr     ->  pointer to a dialog choice structure.                       */
/*                                                                           */
/* Return:                                                                   */
/*   none                                                                    */
/*****************************************************************************/
void  DisplayDllChoice (DIALOGSHELL *shell,DIALOGCHOICE *ptr)
{
  uint    i;
  uint    StartIndex = 1;               /*                                   */
  uint    StrLength;                    /*                                   */
  char    buffer[80], *buf;             /*                                   */
  DEBFILE *cur_dll;                     /* pointer to the EXE file info      */
  DEBFILE *pdf;                         /* pointer to debug file struct      */
  uint    BorderCols;

  BorderCols = 6;

  pdf = pnode->ExeStruct;               /* get -> to Exe structure.          */
  cur_dll = pdf->next;                  /* set ptr to 1st DLL node           */

  /***************************************************************************/
  /* Advance the startindex and -> the dll depending on the skip rows.       */
  /***************************************************************************/
  for( i = 0; i < ptr-> SkipRows; i++ )
  {
    StartIndex++;
    cur_dll = cur_dll->next;
  }

  /***************************************************************************/
  /* Display the names that can fit in the window.                           */
  /***************************************************************************/
  for( i = 0; i < ptr->MaxRows; i++ )
  {
    if( StartIndex <= ptr->entries )
    {
      ClearField[1] = 1;
      putrc( shell->row + i + shell->SkipLines, shell->col + 1, ClearField );
      StrLength = strlen( cur_dll->DebFilePtr->fn );
      if( StrLength > (shell->width - BorderCols) )
      {
        buf = cur_dll->DebFilePtr->fn;
        buf = buf + (StrLength - (shell->width - BorderCols));
        strcpy( buffer, buf );
        buffer[0] = '~';
        putrc( shell->row + i + shell->SkipLines, shell->col + 2, buffer );
        StrLength = strlen( buffer );
      }
      else
        putrc( shell->row + i + shell->SkipLines, shell->col + 2,
               cur_dll->DebFilePtr->fn );
      StartIndex++;
      ClearField[1] = shell->width - BorderCols - StrLength;
      putrc( shell->row + i + shell->SkipLines, shell->col + 2 + StrLength,
             ClearField );
      cur_dll = cur_dll->next;
    }
    else
    {
      ClearField[1] = shell->width-2-SCROLLBARCOLOFFSET;
      putrc( shell->row + i + shell->SkipLines, shell->col + 1, ClearField );
    }
  }
}
