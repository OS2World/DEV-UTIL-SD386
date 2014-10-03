/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   cuasetwp.c                                                              */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Cua watch point dialog functions.                                       */
/*                                                                           */
/*...Release 1.01 (04/03/92)                                                 */
/*...                                                                        */
/*... 05/08/92  701   Samuel    Cua Interface.                               */
/*...                                                                        */
/*...Release 1.02 (10/22/92)                                                 */
/*...                                                                        */
/*... 04/15/93  821   Selwyn    Changes in processing clicks on buttons.     */
/*... 12/10/93  910   Joe       Clear fields on cursor sensitive prompting.  */
/*****************************************************************************/
#include "all.h"
#include "diawhpnt.h"                   /* data for watch point dialog.      */

#define MAXLINESIZE   74
#define DISPPROMPTLEN 35

extern uchar   msgbuf[];                /* msgbuf to hold info msg           */
extern uint    ExprAddr;                /* Set by ParseExpr                  */
extern uint    ExprMid;                 /* Set by ParseExpr                  */
extern uint    ExprTid;                 /* Set by ParseExpr                  */
extern SCOPE   ExprScope;               /* Set by ParseExpr                  */
extern uint    ProcessID;               /* Process id of debuggee.           */
extern uchar   VideoAtr;                /* logical screen attribute          */
extern uchar   normal[];                /* normal field attributes        701*/
extern uchar   ClearField[];            /* clear field attributes         701*/
extern VIOCURSORINFO  NormalCursor;     /* make underscore flashing cursor   */
extern CmdParms cmd;

DEBUG_REGISTER Debug_Regs[4];           /* hardware debug registers          */

static uchar Header[] = "RegNo      Expression                Size   Addr   Scope    Type    Status";

static char *Sizes[3] =                 /* size of the watch points          */
{                                       /*                                   */
  "1",                                  /*  - one byte long                  */
  "2",                                  /*  - two bytes long                 */
  "4"                                   /*  - four bytes long                */
};                                      /*                                   */

static char *Scopes[2] =                /* type of scope for watch points    */
{                                       /*                                   */
  "Local ",                             /*  - Local watchpoint               */
  "Global"                              /*  - Global watchpoint              */
};                                      /*                                   */

static char *Types[3] =                 /* type of watch points              */
{                                       /*                                   */
  "Readwrite",                          /*  - Readwrite watchpoint           */
  "Write    ",                          /*  - Write watchpoint               */
  "Execute  "                           /*  - Execute watchpoint             */
};                                      /*                                   */

static char *Status[2] =                /* status of the watch points        */
{                                       /*                                   */
  "Disabled",                           /*  - Watch point not set            */
  "Enabled "                            /*  - Watch point hit                */
};                                      /*                                   */

static enum { ExprField,
              SizeField,
              ScopeField,
              TypeField,
              StatusField
            } Field;

static DEBUG_REGISTER LDebug_Regs[4];
static PEVENT Event;

/*****************************************************************************/
/* Cua_Setwps()                                                           701*/
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*  Calls functions to display, process choices and remove watchpoint dialog */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   fp      -  pointer to the current afile structure.                      */
/*                                                                           */
/* Return:                                                                   */
/*   none                                                                    */
/*****************************************************************************/
void Cua_SetWps( AFILE *fp )
{
  GetScrAccess();
  DisplayDialog( &Dia_WhPnt, FALSE );
  GetDialogWhPntChoice( &Dia_WhPnt, &Dia_WhPnt_Choices, fp );
  RemoveDialog( &Dia_WhPnt );
  fmterr( msgbuf );
  SetScrAccess();
}

#define NOSCROLLBAR FALSE
/*****************************************************************************/
/* GetDialogWhPntChoice:  Selects an entry from a list on a dialog window    */
/*                                                                           */
/*   shell   ->  pointer to a dialog shell structure.                        */
/*   ptr     ->  pointer to a dialog choice structure.                       */
/*   choice  ->  field number that was selected.                             */
/*****************************************************************************/
uint GetDialogWhPntChoice( DIALOGSHELL *shell, DIALOGCHOICE *ptr, AFILE *fp )
{
  uint   key;                           /* the entered key                   */
  uint   skip;                          /* # of lines to choices             */
  uint   fldcol;
  uint   ButtonKey;                                                     /*821*/
  uint   fldcursor;
  uint   fldrow;
  uint   DebugRegNo;
  uint   CursorStartRow;
  uint   i, sfx;
  uchar  TempPrompt[ PROMPTLEN + 1 ];
  uchar  buffer[ MAXSYM+1 ], msg[80];
  uchar  ExprChgFlag;
  uchar  *cp;
  uint   Pos;

  POPUPSHELL PopShell;                                                  /*910*/

  /****************************************************************************/
  /* copy the Global debug register to local copy.                            */
  /****************************************************************************/
  for ( i = 0 ; i < 4 ; i++)
     LDebug_Regs[i] = Debug_Regs[i];

  /***************************************************************************/
  /* - set up string length for the arttribute strings.                      */
  /* - set default video attribute.                                          */
  /***************************************************************************/
  normal[1]    = (UCHAR) shell->width - 2 - SCROLLBARCOLOFFSET;
  VideoAtr     = vaMenuBar;


  /***************************************************************************/
  /* Start one line past the title.                                          */
  /***************************************************************************/
  skip = 3;

  /***************************************************************************/
  /* Put the watch point entries into the dialog box.                        */
  /***************************************************************************/
  DisplayWhPntChoice (shell,ptr);

  /***************************************************************************/
  /* - Put up the header text.                                               */
  /* - Set the cursor type.                                                  */
  /***************************************************************************/
  putrc( shell->row+2, shell->col+2, Header );
  VioSetCurType( &NormalCursor, 0 );

  /***************************************************************************/
  /* Init the various variables.                                             */
  /***************************************************************************/
  DebugRegNo   = 1;
  Field        = ExprField;
  fldcol       = shell->col + 5;
  CursorStartRow = fldrow = shell->row + skip;
  /***************************************************************************/
  /* - Build a local POPUPSHELL from the DIALOGSHELL. GetString requires  910*/
  /*   this structure after fix 910.                                      910*/
  /***************************************************************************/
  PopShell.row          = shell->row;                                   /*910*/
  PopShell.col          = shell->col;                                   /*910*/
  PopShell.length       = shell->length;                                /*910*/
  PopShell.width        = shell->width;                                 /*910*/
  PopShell.NoOfButtons  = shell->NoOfButtons;                           /*910*/
  PopShell.title        = NULL;                                         /*910*/
  PopShell.instructions = NULL;                                         /*910*/
  PopShell.help         = 0;                                            /*910*/
  PopShell.Buttons      = shell->Buttons;                               /*910*/
  PopShell.Flags        = AUTOEXIT;                                     /*910*/

  for(;;)
  {
GetExpr:

    VioSetCurPos( (short)fldrow, (short)fldcol+1, 0 );

    if( Field == ExprField )
    {
      ExprChgFlag = FALSE;
      strcpy(TempPrompt,LDebug_Regs[DebugRegNo - 1].Prompt);

      fldcursor = 0;
      key = GetString( fldrow, fldcol, PROMPTLEN, DISPPROMPTLEN, &fldcursor,
               LDebug_Regs[ DebugRegNo-1 ].Prompt, AUTOEXIT,&PopShell); /*910*/

      if( key == LEFTMOUSECLICK )
      {
        Pos = GetEventPositionInDialog( shell, ptr, &key, NOSCROLLBAR );
        if( !Pos )
          continue;
        else
        {                                                               /*821*/
         goto ValidateExpr;                                             /*910*/
        }                                                               /*821*/
      }
      else
      if( key == ESC )
        goto ProcessKey;
      else
      {
ValidateExpr:

        if( strcmp( TempPrompt, LDebug_Regs[DebugRegNo - 1].Prompt ) )
           ExprChgFlag = TRUE;

        cp = LDebug_Regs[DebugRegNo - 1].Prompt;

        if ( *cp )
        {
       /***********************************************************************/
       /* If the Expression in not empty.                                     */
       /***********************************************************************/
          if( ExprChgFlag == TRUE ||
              LDebug_Regs[DebugRegNo - 1].Status == DISABLED )
          {
       /*********************************************************************/
       /* Expression has changed. So parse the expression with the current  */
       /* context.                                                          */
       /*********************************************************************/
            cp = ParseExpr(LDebug_Regs[DebugRegNo - 1].Prompt,0x10,
                           fp->mid, fp->csrline+fp->Nbias, fp->sfi);

#if 0
            if(((uint) cp) < 20)  {/*MSH return code.*/
                int iret=((uint)cp)-10;
                switch (iret)  {
                case -3:
                   return((uchar *) "MSH exit.");
                case -2:
                   return((uchar *) "MSH semantic error.");
                case -1:
                   return((uchar *) "MSH syntax error.");
                case  0:
                   return((uchar *) "MSH Return code 0.");
                case  1:
                   return((uchar *) "MSH Return code 1.");
                default:
                   return((uchar *) "MSH Unknown return code.");
                } /* endswitch */
            }
#endif
       /*********************************************************************/
       /* Handle errors from parsing the expression.                        */
       /*********************************************************************/
            if ( (!cp) || (*cp) || (!ExprAddr) )
            {
              beep();
              fmterr( "Invalid Expression" );
              LDebug_Regs[DebugRegNo - 1].Wpindex = 0;
              LDebug_Regs[DebugRegNo - 1].Size  = 0;
              LDebug_Regs[DebugRegNo - 1].Address = 0;
              LDebug_Regs[DebugRegNo - 1].Aligned = FALSE;
              LDebug_Regs[DebugRegNo - 1].Scope = WPS_LOCAL;
              LDebug_Regs[DebugRegNo - 1].Type  = READWRITE;
              LDebug_Regs[DebugRegNo - 1].Status = DISABLED;
              DisplayWhPntChoice( shell, ptr );
              goto GetExpr;
            }
       /*********************************************************************/
       /* Filter out the register addresses.                                */
       /*********************************************************************/
            if( (ExprAddr >> REGADDCHECKPOS) == REGISTERTYPEADDR )
            {
               beep();
               fmterr( "Can't set watch points on register variables" );
               goto GetExpr;
            }
       /*********************************************************************/
       /* If the address is stack address, check for the scope stuff and if */
       /* every thing is O.k convert the address to actual address.         */
       /*********************************************************************/
            if ( TestBit(ExprAddr,STACKADDRBIT) )
            {
              sfx = StackFrameIndex( ExprScope );
              if( ExprScope && !sfx )
              {
                 beep();
                 CopyProcName(ExprScope,buffer,sizeof(buffer));
                 sprintf(msg,"\"%s\" not active",buffer);
                 fmterr(msg);
                 goto GetExpr;
              }
              ExprAddr = StackBPRelToAddr(ExprAddr,sfx);
            }
       /*********************************************************************/
       /* Get the size of the expression and put the size in encoded form   */
       /* into the debug registers.                                         */
       /*********************************************************************/
            if( ExprTid )
            {
               switch( QtypeGroup(ExprMid, ExprTid) )
               {
                  case TG_SCALAR:
                  case TG_POINTER:
                    LDebug_Regs[DebugRegNo-1].Size = EncodeSize(QtypeSize(ExprMid,ExprTid));
                    break;
                  case TG_CONSTANT:
                    LDebug_Regs[DebugRegNo - 1].Size = EncodeSize(sizeof(uint));
                    break;
                  default:
                     beep();
                     goto GetExpr;
               }
            }
            else
               LDebug_Regs[DebugRegNo - 1].Size = EncodeSize(sizeof(uint));

      /*********************************************************************/
      /* Assign the address and align it if neccessary.                    */
      /*********************************************************************/
            LDebug_Regs[DebugRegNo - 1].Address = ExprAddr;
            LDebug_Regs[DebugRegNo - 1].Status = ENABLED;
            if (LDebug_Regs[DebugRegNo - 1].Type != EXECUTE)
            {
               if (AlignAddress( &(LDebug_Regs[DebugRegNo - 1].Address),
                                  LDebug_Regs[DebugRegNo - 1].Size))
                  LDebug_Regs[DebugRegNo - 1].Aligned = TRUE;
               else
                  LDebug_Regs[DebugRegNo - 1].Aligned = FALSE;
            }
            else
            {
               LDebug_Regs[DebugRegNo - 1].Size = 0;
               LDebug_Regs[DebugRegNo - 1].Aligned = FALSE;
            }
            DisplayWhPntChoice( shell, ptr );
            VioSetCurType( &NormalCursor, 0 );
            VioSetCurPos( (short)fldrow, (short)fldcol+1, 0 );
            goto ProcessKey;
          }
        }
        else
        {
      /************************************************************************/
      /* If the expression is Empty, then reset all other field values to     */
      /* defaults and refresh the display.                                    */
      /* Don't allow the user to go to attribute fields with empty expression */
      /************************************************************************/
          LDebug_Regs[DebugRegNo - 1].Wpindex = 0;
          LDebug_Regs[DebugRegNo - 1].Size  = 0;
          LDebug_Regs[DebugRegNo - 1].Address = 0;
          LDebug_Regs[DebugRegNo - 1].Aligned = FALSE;
          LDebug_Regs[DebugRegNo - 1].Scope = WPS_LOCAL;
          LDebug_Regs[DebugRegNo - 1].Type  = READWRITE;
          LDebug_Regs[DebugRegNo - 1].Status = DISABLED;
          switch( key )
          {
              case ENTER:               /* If all the expressions are empty  */
              {                         /* the user might want to clear all  */
                int  Flag = FALSE;      /* watch points, so go process the   */
                                        /* key (ENTER).                      */
                for( i = 0; i < NODEBUGREGS; i++ )
                {
                  cp = LDebug_Regs[DebugRegNo - 1].Prompt;
                  if( *cp )
                    Flag = TRUE;
                }
                if( !Flag )
                  goto ProcessKey;
              }                         /* intentional fall-through          */
              goto caseright;
caseright:

              case RIGHT:
              case TAB:
              case LEFT:
              case S_TAB:
              case TYNEXT:
              case STNEXT:
              case SZNEXT:
              case SPNEXT:
                 DisplayWhPntChoice( shell, ptr );
                 beep();
                 goto GetExpr;

              default:
                 DisplayWhPntChoice( shell, ptr );
                 VioSetCurType( &NormalCursor, 0 );
                 goto ProcessKey;
          }
        }
        DisplayWhPntChoice( shell, ptr );
        VioSetCurType( &NormalCursor, 0 );
        goto ProcessKey;
    }                                   /* endif ( Field == ExprField )      */
  }

    /*************************************************************************/
    /* Get the data from keyboard or mouse.                                  */
    /*************************************************************************/
GetEvents:                                                              /*821*/
    Event = GetEvent( SEM_INDEFINITE_WAIT );
    switch( Event->Type )
    {
      case TYPE_MOUSE_EVENT:
      {
        switch( Event->Value )
        {
          case EVENT_BUTTON_1_DOWN:
          {
            Pos = GetEventPositionInDialog( shell, ptr, &key, NOSCROLLBAR );
            if( Pos == BUTTONS )
            {                                                           /*821*/
              ButtonKey = key;                                          /*821*/
              goto GetEvents;                                           /*821*/
            }                                                           /*821*/
            else
              key = LEFTMOUSECLICK;
            break;
          }

          case EVENT_NO_BUTTONS_DOWN:                                   /*821*/
          case EVENT_NO_BUTTONS_DOWN_MOVE:                              /*821*/
          {                                                             /*821*/
            Pos = GetEventPositionInDialog( shell, ptr, &key, NOSCROLLBAR );
            if( Pos == BUTTONS )                                        /*821*/
             if( ButtonKey == key )                                     /*821*/
               break;                                                   /*821*/
            ButtonKey = 0;                                              /*821*/
            continue;                                                   /*821*/
          }                                                             /*821*/

          default:
            continue;
        }
        break;
      }

      case TYPE_KBD_EVENT:
        key = Event->Value;
        break;
    }

    /*************************************************************************/
    /* switch on the key stroke.                                             */
    /*************************************************************************/
ProcessKey:
    switch( key )
    {
      case DOWN:
        ++DebugRegNo;
        fldrow++;
        if( DebugRegNo > NODEBUGREGS )
        {
          DebugRegNo = 1;
          fldrow = CursorStartRow;
        }
        break;

      case UP:
        --DebugRegNo;
        fldrow--;
        if( DebugRegNo < 1 )
        {
          DebugRegNo = NODEBUGREGS;
          fldrow = CursorStartRow + NODEBUGREGS - 1;
        }
        break;

      case TYNEXT:
         Field = TypeField;
         key = SPACEBAR;
         fldcol = shell->col + 10 + DISPPROMPTLEN + ADDRESSLEN + SCOPELEN;
         goto ProcessKey;

      case STNEXT:
         Field = StatusField;
         key = SPACEBAR;
         fldcol = shell->col + 11 + DISPPROMPTLEN + ADDRESSLEN + SCOPELEN +
                  TYPELEN;
         goto ProcessKey;

      case SZNEXT:
         Field = SizeField;
         key = SPACEBAR;
         fldcol = shell->col + 5 + DISPPROMPTLEN;
         goto ProcessKey;

      case SPNEXT:
         Field = ScopeField;
         key = SPACEBAR;
         fldcol = shell->col + 9 + DISPPROMPTLEN + ADDRESSLEN;
         goto ProcessKey;

      case SPACEBAR:                      /* if space,                         */
        switch( Field )                   /* see which field user is in        */
        {
          case SizeField:
            /*******************************************************************/
            /*  - Increment the Size index in the debug register.              */
            /*  - If the index exceeds the limit reset it to start.            */
            /*  - Parse the expression to arrive at the address.               */
            /*  - Align the address with the new selected address.             */
            /*  - Depending on alignment set the flag.                         */
            /*******************************************************************/
            LDebug_Regs[DebugRegNo - 1].Size++;
            if (LDebug_Regs[DebugRegNo - 1].Size > 2)
                LDebug_Regs[DebugRegNo - 1].Size = 0;
            ParseExpr(LDebug_Regs[DebugRegNo - 1].Prompt,0x10,
                           fp->mid,fp->csrline+fp->Nbias, fp->sfi);
            if( TestBit(ExprAddr,STACKADDRBIT) )
                ExprAddr = StackBPRelToAddr(ExprAddr,sfx);
            LDebug_Regs[DebugRegNo - 1].Address = ExprAddr;
            if (AlignAddress( &(LDebug_Regs[DebugRegNo - 1].Address),
                                LDebug_Regs[DebugRegNo - 1].Size))
               LDebug_Regs[DebugRegNo - 1].Aligned = TRUE;
            else
               LDebug_Regs[DebugRegNo - 1].Aligned = FALSE;

            break;

          case ScopeField:
            /*******************************************************************/
            /*  - Increment the Scope index in the debug register.             */
            /*  - If the index exceeds the limit reset it to start.            */
            /*******************************************************************/
            LDebug_Regs[DebugRegNo - 1].Scope++;
            if (LDebug_Regs[DebugRegNo - 1].Scope > 1)
                LDebug_Regs[DebugRegNo - 1].Scope = 0;
            break;

          case StatusField:

            LDebug_Regs[DebugRegNo - 1].Status++;
            if( LDebug_Regs[DebugRegNo - 1].Status > 1 )
                LDebug_Regs[DebugRegNo - 1].Status = 0;
            break;

          case TypeField:
           /*******************************************************************/
           /*  - Increment the type index in the debug register.              */
           /*  - If the index exceeds the limit reset it to start.            */
           /*  - Reset the parameters accordingly if the user selected        */
           /*    execute watch point.                                         */
           /*******************************************************************/
            LDebug_Regs[DebugRegNo - 1].Type++;
            if (LDebug_Regs[DebugRegNo - 1].Type > 2)
                LDebug_Regs[DebugRegNo - 1].Type = 0;
            if (LDebug_Regs[DebugRegNo - 1].Type == EXECUTE)
            {
                ParseExpr(LDebug_Regs[DebugRegNo - 1].Prompt,0x10,
                         fp->mid,fp->csrline+fp->Nbias, fp->sfi);
                if( TestBit(ExprAddr,STACKADDRBIT) )
                    ExprAddr = StackBPRelToAddr(ExprAddr,sfx);
                LDebug_Regs[DebugRegNo - 1].Size = 0;
                LDebug_Regs[DebugRegNo - 1].Aligned = FALSE;
                LDebug_Regs[DebugRegNo - 1].Address = ExprAddr;
            }
            break;
        }
        DisplayWhPntChoice( shell, ptr );
        break;

      case RIGHT:                        /* if right or tab,                  */
      case TAB:                          /*                                   */
        switch( Field )                  /* see which field user is in        */
        {                                /*                                   */
          case ExprField:                /* if user in expression field       */
            fldcol = shell->col + 5 + DISPPROMPTLEN;
            Field = SizeField;           /* move user right to Size field     */
            LDebug_Regs[DebugRegNo - 1].Status = ENABLED;
            DisplayWhPntChoice( shell, ptr );
            break;                       /*                                   */

          case SizeField:                /* if user in size field             */
            fldcol = shell->col + 9 + DISPPROMPTLEN + ADDRESSLEN;
            Field = ScopeField;          /* move user right to Scope field    */
            break;                       /*                                   */

          case ScopeField:               /* if user in scope field            */
            fldcol = shell->col + 10 + DISPPROMPTLEN + ADDRESSLEN + SCOPELEN;
            Field = TypeField;           /* move user right to type field     */
            break;                       /*                                   */

          case TypeField:                /* if user in scope field            */
            fldcol = shell->col + 11 + DISPPROMPTLEN + ADDRESSLEN + SCOPELEN +
                     TYPELEN;
            Field = StatusField;         /* move user right to type field     */
            break;                       /*                                   */

          case StatusField:              /* if user in status field           */
            fldcol = shell->col + 5;
            Field = ExprField;           /* move user right to expr field     */
            break;                       /*                                   */
        }                                /*                                   */
        break;                           /*                                   */

      case LEFT:                         /* if left or shift_tab,             */
      case S_TAB:                        /*                                   */
        switch( Field )                  /* see which field user is in        */
        {                                /*                                   */
          case ExprField:                /* if user in scope field            */
            fldcol = shell->col + 11 + DISPPROMPTLEN + ADDRESSLEN + SCOPELEN +
                     TYPELEN;
            Field = StatusField;         /* move user right to type field     */
            break;                       /*                                   */

          case StatusField:              /* if user in expression field       */
            fldcol = shell->col + 10 + DISPPROMPTLEN + ADDRESSLEN + SCOPELEN;
            Field = TypeField;           /* move user left to type field      */
            break;                       /*                                   */

          case SizeField:                /* if user in size field             */
            fldcol = shell->col + 5;
            Field = ExprField;           /* move user left to expr field      */
            break;                       /*                                   */

          case ScopeField:               /* if user in scope field            */
            fldcol = shell->col + 5 + DISPPROMPTLEN;
            Field = SizeField;           /* move user left to size field      */
            break;                       /*                                   */

          case TypeField:                /* if user in type field             */
            fldcol = shell->col + 9 + DISPPROMPTLEN + ADDRESSLEN;
            Field = ScopeField;          /* move user left to Scope field     */
            break;                       /*                                   */
        }                                /*                                   */
        break;                           /*                                   */

      case F1:
      {
        uchar  *HelpMsg;

        HelpMsg = GetHelpMsg( HELP_DLG_WATCH, NULL,0 );
        CuaShowHelpBox( HelpMsg );
        break;
      }


      case C_ENTER:
      case ENTER:
#if 0
      /***********************************************************************/
      /* Pull out the watch points.                                          */
      /***********************************************************************/
      rc = PullOutWps(&Debug_Regs[0]);

      /***********************************************************************/
      /* - Put in the watch points.                                          */
      /* - If we get a error setting the watch points display error message  */
      /*   and go to the watch point which caused it.                        */
      /***********************************************************************/
      rc = PutInWps(&LDebug_Regs[0]);
      if (rc)
      {
         sprintf(msg,"Error Setting Watch point %1d",rc);
         fmterr(msg);
         beep();
         DebugRegNo = rc;               /* put rc into debugRegNo            */
         fldcol = shell->col + 5;       /* move to 1st expression field      */
         fldrow = shell->row + 3 + DebugRegNo - 1;
         Field  = ExprField;            /* Start user in Expression field    */
         goto GetExpr;
      }
#endif

      /***********************************************************************/
      /* copy the local debug registers to global registers.                 */
      /***********************************************************************/
      for ( i = 0 ; i < NODEBUGREGS ; i++)
          Debug_Regs[i] = LDebug_Regs[i];

      /***********************************************************************/
      /* - define the watchpoints to the x-server.                        827*/
      /***********************************************************************/
      {                                                                 /*827*/
       WP_REGISTER  regs[NODEBUGREGS];                                  /*827*/
                                                                        /*827*/
       for ( i = 0 ; i < NODEBUGREGS ; i++)                             /*827*/
       {                                                                /*827*/
        regs[i].Address = Debug_Regs[i].Address;                        /*827*/
        regs[i].Size    = Debug_Regs[i].Size   ;                        /*827*/
        regs[i].Scope   = Debug_Regs[i].Scope  ;                        /*827*/
        regs[i].Type    = Debug_Regs[i].Type   ;                        /*827*/
        regs[i].Status  = Debug_Regs[i].Status ;                        /*827*/
        regs[i].Wpindex = Debug_Regs[i].Wpindex;                        /*827*/
        regs[i].IsSet   = FALSE;                                        /*827*/
       }                                                                /*827*/
       xDefWps(regs,sizeof(regs));                                                   /*827*/
      }                                                                 /*827*/
      return(key);

      case ESC:
      case F10:
      {
#if 0
        int i;

        for( i = 0; i < NODEBUGREGS; i++ )
        {
          if( strcmp( Debug_Regs[i].Prompt, LDebug_Regs[i].Prompt ) )
          {
            int j;
            for( j = 0 ; j < NODEBUGREGS ; j++ )
              LDebug_Regs[j] = Debug_Regs[j];
            DisplayWhPntChoice( shell, ptr );
            goto GetExpr;
          }
        }
#endif
        if( memcmp( Debug_Regs, LDebug_Regs, sizeof( LDebug_Regs ) ) )
        {
          int j;
          for( j = 0 ; j < NODEBUGREGS ; j++ )
            LDebug_Regs[j] = Debug_Regs[j];
          DisplayWhPntChoice( shell, ptr );
          Field = ExprField;
          fldcol = shell->col + 5;
          goto GetExpr;
        }
        return( key );
      }

      case LEFTMOUSECLICK:
      /***********************************************************************/
      /*  - Item was selected by clicking on the choices with mouse.         */
      /***********************************************************************/
      {
        uint  PrevDebugRegNo;
        uchar *sp;

        Event = GetCurrentEvent();
        PrevDebugRegNo = DebugRegNo;
        DebugRegNo = Event->Row - shell->row - 2;

        if( (!DebugRegNo) || (DebugRegNo > 4) )
        {
          DebugRegNo = PrevDebugRegNo;
          beep();
          break;
        }

        sp  = LDebug_Regs[DebugRegNo - 1].Prompt;

        /*********************************************************************/
        /* Depending on the mouse position set the fld col and field we are  */
        /* in.                                                               */
        /*********************************************************************/
        if ( ((uint)Event->Col >= ( shell->col + 5 )) &&
             ((uint)Event->Col < ( shell->col + 5 + DISPPROMPTLEN )))
        {
#if 0
          if( *sp || *psp )
          {
#endif
            fldcol = shell->col + 5;
            fldrow = (uint)Event->Row;
            Field = ExprField;
#if 0
          }
          else
            DebugRegNo = PrevDebugRegNo;
#endif
          goto GetExpr;
        }
        else
        if( ((uint)Event->Col > ( shell->col + 5 + DISPPROMPTLEN )) &&
            ((uint)Event->Col < ( shell->col + 5 + DISPPROMPTLEN + 4 )) )
        {
#if 0
          if( *psp )
          {
#endif
            if( *sp )
            {
              fldcol = shell->col + 5 + DISPPROMPTLEN;
              Field = SizeField;
              fldrow = (uint)Event->Row;
              key = SPACEBAR;
              goto ProcessKey;
            }
            else
            {
              DebugRegNo = PrevDebugRegNo;
              goto GetExpr;
            }
#if 0
          }
          else
          {
            DebugRegNo = PrevDebugRegNo;
            goto GetExpr;
          }
#endif
        }
        else
        if( ((uint)Event->Col > ( shell->col + 9 + DISPPROMPTLEN + ADDRESSLEN )) &&
            ((uint)Event->Col < ( shell->col + 9 + DISPPROMPTLEN + ADDRESSLEN +
                                  SCOPELEN )) )

        {
#if 0
          if( *psp )
          {
#endif
            if( *sp )
            {
              fldcol = shell->col + 9 + DISPPROMPTLEN + ADDRESSLEN;
              Field = ScopeField;
              fldrow = (uint)Event->Row;
              key = SPACEBAR;
              goto ProcessKey;
            }
            else
            {
              DebugRegNo = PrevDebugRegNo;
              goto GetExpr;
            }
#if 0
          }
          else
          {
            DebugRegNo = PrevDebugRegNo;
            goto GetExpr;
          }
#endif
        }
        else
        if ( ((uint)Event->Col > ( shell->col + 9 + DISPPROMPTLEN + ADDRESSLEN +
                                 SCOPELEN )) &&
             ((uint)Event->Col < ( shell->col + 9 + DISPPROMPTLEN + ADDRESSLEN +
                                 SCOPELEN + 9 )))
        {
#if 0
          if( *psp )
          {
#endif
            if( *sp )
            {
              fldcol = shell->col + 10 + DISPPROMPTLEN + ADDRESSLEN + SCOPELEN;
              Field = TypeField;
              fldrow = (uint)Event->Row;
              key = SPACEBAR;
              goto ProcessKey;
            }
            else
            {
              DebugRegNo = PrevDebugRegNo;
              goto GetExpr;
            }
#if 0
          }
          else
          {
            DebugRegNo = PrevDebugRegNo;
            goto GetExpr;
          }
#endif
        }
        else
        if( ((uint)Event->Col > ( shell->col + 10 + DISPPROMPTLEN + ADDRESSLEN +
                                  SCOPELEN + TYPELEN )) &&
            ((uint)Event->Col < ( shell->col + shell->width - 1 )))
        {
#if 0
          if( *psp )
          {
#endif
            if( *sp )
            {
              fldcol = shell->col + 11 + DISPPROMPTLEN + ADDRESSLEN + SCOPELEN +
                       TYPELEN;
              Field = StatusField;
              fldrow = (uint)Event->Row;
              key = SPACEBAR;
              goto ProcessKey;
            }
            else
            {
              DebugRegNo = PrevDebugRegNo;
              goto GetExpr;
            }
#if 0
          }
#endif
        }
        DebugRegNo = PrevDebugRegNo;
        beep();
        goto GetExpr;
      }

      default:
        beep();
        break;
    }
  }
}

/*****************************************************************************/
/* DisplayWhpntChoice()                                                   701*/
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Displays the details of the watch points in the watch point dialog.     */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   shell   ->  pointer to a dialog shell structure.                        */
/*   ptr     ->  pointer to a dialog choice structure.                       */
/*                                                                           */
/* Return:                                                                   */
/*   none                                                                    */
/*****************************************************************************/
void  DisplayWhPntChoice (DIALOGSHELL *shell,DIALOGCHOICE *ptr)
{
  uint   skip, i, n;
  uchar  *buf;
  uchar  Line[ MAXLINESIZE+1 ];

  skip = 3;

  for(i=0;i<ptr->MaxRows;i++)
  {
    memset( Line, ' ', MAXLINESIZE );
    Line[ MAXLINESIZE ] = '\0';
    buf = Line;

    n = sprintf( buf, "%1d.", i+1 );
    *( buf+n ) = ' ';
    buf = buf + 3;

    strncpy( buf, LDebug_Regs[i].Prompt, strlen( LDebug_Regs[i].Prompt ) );
    buf = buf + DISPPROMPTLEN + 1;

    strncpy( buf, Sizes[LDebug_Regs[i].Size], 1 );
    buf =  buf + 3;
    if( LDebug_Regs[i].Aligned == TRUE )
       strncpy( buf-1, "*", 1 );

    n = sprintf( buf, "%08X", LDebug_Regs[i].Address );
    *( buf+n ) = ' ';
    buf = buf + ADDRESSLEN + 1;

    sprintf( buf, "%s %s %s",
                  Scopes[LDebug_Regs[i].Scope],
                  Types[LDebug_Regs[i].Type],
                  Status[LDebug_Regs[i].Status] );

    putrc( shell->row+i+skip, shell->col+2, Line );
  }
}
