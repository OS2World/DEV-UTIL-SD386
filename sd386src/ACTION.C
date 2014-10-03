/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   action.c                                                                */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Functions requiring prompt strings from the user.                       */
/*                                                                           */
/*...Release 1.01 (07/10/92)                                                 */
/*...                                                                        */
/*... 10/05/92  707   Selwyn    Function entry/address breaks not getting    */
/*...                           marked.                                      */
/*... 11/16/92  802   Selwyn    Fix browse dialog remaining after "*" entry. */
/*****************************************************************************/

#include "all.h"
#include "popups.h"

static int iview=0;

/*****************************************************************************/
/* Externals.                                                                */
/*****************************************************************************/
extern CmdParms      cmd;               /* pointer to CmdParms structure     */
extern AFILE        *allfps;

static char   FindString[MAXPROMPTSTRING];

/*****************************************************************************/
/* Data for getfunc popup.                                                   */
/*****************************************************************************/
#define GETFUNCHELP HELP_DLG_GETFUNC
#define GETADDRHELP HELP_DLG_GETADDR

static uchar  CantFind[]         = "Can't find that";
static char   GF_Title[]         = " Search for a function ";
static char   GF_Instructions[]  = "Enter the function name";
static char   GA_Title[]         = " Search for an address ";
static char   GA_Instructions[]  = "Enter an address";


static BUTTON PopButton[] =
{
 {
  POP_BTN_ROW         ,
  POP_BTN_ENTER_COL   ,
  POP_BTN_ENTER_WIDTH ,
  POP_BTN_ENTER_TEXT  ,
  POP_BTN_ENTER_KEY
 },
 {
  POP_BTN_ROW         ,
  POP_BTN_CANCEL_COL  ,
  POP_BTN_CANCEL_WIDTH,
  POP_BTN_CANCEL_TEXT ,
  POP_BTN_CANCEL_KEY
 },
 {
  POP_BTN_ROW         ,
  POP_BTN_HELP_COL    ,
  POP_BTN_HELP_WIDTH  ,
  POP_BTN_HELP_TEXT   ,
  POP_BTN_HELP_KEY
 }
};

static POPUPSHELL Popup = {
                           POP_START_ROW   ,
                           POP_START_COL   ,
                           POP_LEN         ,
                           POP_WIDTH       ,
                           POP_BUTTONS     ,
                           NULL            ,
                           NULL            ,
                           0               ,
                           &PopButton[0]
                          };

/*****************************************************************************/
/* Data for getfile popup.                                                   */
/*****************************************************************************/
#define GETFILEHELP HELP_DLG_GETFILE

static char   GE_Title[]         = " Search for a file ";
static char   GE_Instructions[]  = "Enter a source file name";

/*****************************************************************************/
/* Data for find popup.                                                      */
/*****************************************************************************/
#define FINDSTRHELP HELP_DLG_FIND

static char   FI_Title[]         = " Search for a string ";
static char   FI_Instructions[]  = "Enter the string ";

/*****************************************************************************/
/* Data for browse popup.                                                    */
/*****************************************************************************/
#define BROWSEHELP HELP_DLG_BROWSE

static char   BR_Title[]         = " Browse a file ";
static char   BR_Instructions[]  = "Enter the file name ";

/*****************************************************************************/
/* Data for browse popup.                                                    */
/*****************************************************************************/
#define CONDBPHELP HELP_DLG_SETCONDBKP

static char   CB_Title[]         = " Set a conditional break";
static char   CB_Instructions[]  = "Enter an expression";

/*****************************************************************************/
/* Data for Name or Addr popup.                                              */
/*****************************************************************************/
#define IDFUNCHELP  HELP_DLG_FUNCENTRY
#define IDADDRHELP  HELP_DLG_ADDRESS
#define IDDEFRHELP  HELP_DLG_DEFERRED
#define IDLOADHELP  HELP_DLG_ADDRLOAD
#define IDDLLHELP   HELP_DLG_DLLLOAD

static char   IF_Title[]         = " Set a function break ";
static char   IF_Instructions[]  = "Enter [dllname.]funcname";

static char   IA_Title[]         = " Set an address break ";
static char   IA_Instructions[]  = "Enter an address";

static char   ID_Title[]         = " Set a deferred break ";
static char   ID_Instructions[]  = "Enter [dllname.]funcname";

static char   IO_Title[]         = " Set a load address break ";
static char   IO_Instructions[]  = "Enter an address";

static char   IL_Title[]         = " Set a dll load break ";
static char   IL_Instructions[]  = "Enter dll name(w/o path or extension)";
/*****************************************************************************/
/* Data for GetLineNumber.                                                   */
/*****************************************************************************/
#define LNFUNCHELP  HELP_DLG_GETLINENUMBER

static char   LN_Title[]         = " Go to a line number ";
static char   LN_Instructions[]  = "Enter the line number";

/*****************************************************************************/
/* Data for getformattype popup.                                             */
/*****************************************************************************/
#define GETTYPEHELP HELP_DLG_GETTYPE

static char   GT_Title[]         = " Get Format Type ";
static char   GT_Instructions[]  = "Enter data type";

/*****************************************************************************/
/* GetFunction()                                                             */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Prompt the user for a function name and then try to build a view        */
/*   for that function.                                                      */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   InitString  -> to a string to use as an initial prompt.                 */
/*                                                                           */
/* Return Value:                                                             */
/*                                                                           */
/*   fp          -> to AFILE structure containing the new view.              */
/*   NULL        couldn't find the specified function. Or, escaped.          */
/*                                                                           */
/*  Assumptions:                                                             */
/*                                                                           */
/*   none                                                                    */
/*                                                                           */
/*****************************************************************************/
 AFILE *
GetFunction( char *InitString , uint func )
{
 char  *hp;
 uint   key;
 char   PromptString[MAXPROMPTSTRING];
 AFILE *fp;
 uchar *SaveArea;
 int    IsAddr;
 static int _iview;

 /****************************************************************************/
 /* - copy the initstring ( cursor sensitive prompt ) into the prompt string.*/
 /* - put the title, instructions, and help appropriate for this popup       */
 /*   into the popup structure.                                              */
 /* - save the screen area.                                                  */
 /* - get a function name from the user.                                     */
 /* - handle user string and keys.                                           */
 /* - handle errors.                                                         */
 /* - restore the screen save area.                                          */
 /*                                                                          */
 /****************************************************************************/
 fp = NULL;

#if defined(MSH)
 if(commandLine.nparms>1) {
    IsAddr = (func==GETADDRESS)?TRUE:FALSE;
    fp = FindFuncOrAddr(commandLine.parms[1],IsAddr);
    if(fp)
    {
#if defined(SD386LOG)
        SD386Log(STRINGCODE," ");
        SD386Log(STRINGCODE,commandLine.parms[1]);
#endif
        return(fp);
    }
 }
#endif
 _iview=iview; iview=0;
 strcpy(PromptString,InitString);
 Popup.Flags = 0;
 if( func == GETFUNCTION )
 {
  Popup.title = GF_Title;
  Popup.instructions = GF_Instructions;
  Popup.help = GETFUNCHELP;
  Popup.Flags = CLEAR1ST;
 }
 else /* func == GETADDRESS */
 {
  Popup.title = GA_Title;
  Popup.instructions = GA_Instructions;
  Popup.help = GETADDRHELP;
 }

 SaveArea = (uchar *)Talloc(Popup.length * Popup.width * 2);
 GetPopArea(SaveArea);
 DisplayPop( &Popup );

 /****************************************************************************/
 /* loop to handle user entry/errors.                                        */
 /****************************************************************************/
 for(;;)
 {
  if( func == GETFUNCTION )
   hp = GF_Instructions;
  else
   hp = GA_Instructions;

  key = PopPrompt( PromptString, hp);

  stripblk(PromptString);


  if( key == ESC )
  {
#if 0
    SD386Log(STRINGCODE," ");
   SD386Log(KEYCODE|key,"");
#endif
   break;
  }
  else if ( key == ENTER )
  {
   if( strlen(PromptString) == 0 )
    continue;
   IsAddr = (func==GETADDRESS)?TRUE:FALSE;
   fp = FindFuncOrAddr(PromptString,IsAddr);
   if( fp )
   {
#if 0
    SD386Log(STRINGCODE," ");
    SD386Log(STRINGCODE,PromptString);
#endif
    break;
   }

   /**************************************************************************/
   /* display an error message and then ask for another name.                */
   /**************************************************************************/
   beep();
   hp = CantFind;
   fmterr( hp);
   continue;
  }
 }

 PutPopArea(SaveArea);
 Tfree(SaveArea );
 iview=_iview;
 return(fp);
}
/*****************************************************************************/
/* GetF()                                                                    */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Prompt the user for a file name and then try to build a view            */
/*   for that function.                                                      */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   none                                                                    */
/*                                                                           */
/* Return Value:                                                             */
/*                                                                           */
/*   fp          -> to AFILE structure containing the new view.              */
/*   NULL        couldn't find the specified file. Or, escaped.              */
/*                                                                           */
/*  Assumptions:                                                             */
/*                                                                           */
/*                                                                           */
/*****************************************************************************/
 AFILE *
GetF( )
{
 char  *hp;
 uint   key;
 char   PromptString[MAXPROMPTSTRING];
 AFILE *fp;
 uchar *SaveArea;

 static int _iview;
 /****************************************************************************/
 /* - put the title, instructions, and help appropriate for this popup       */
 /*   into the popup structure.                                              */
 /* - save the screen area used by the popup if CUA interface.               */
 /* - get a function name from the user.                                     */
 /* - handle user string and keys.                                           */
 /* - handle errors.                                                         */
 /* - restore the screen save area if CUA interface.                         */
 /*                                                                          */
 /****************************************************************************/
 fp = NULL;
#if defined(MSH)
 if(commandLine.nparms>1) {
    fp=GetFile(commandLine.parms[1]);
    if(fp)
    {
#if defined(SD386LOG)
        SD386Log(STRINGCODE," ");
        SD386Log(STRINGCODE,commandLine.parms[1]);
#endif
        return fp;
    }
 }
#endif
 _iview=iview; iview=0;
 *PromptString = '\0';

 Popup.title = GE_Title;
 Popup.instructions = GE_Instructions;
 Popup.help = GETFILEHELP;

 SaveArea = (uchar *)Talloc(Popup.length * Popup.width * 2);
 GetPopArea(SaveArea);
 DisplayPop( &Popup );

 /****************************************************************************/
 /* loop to handle user entry/errors.                                        */
 /****************************************************************************/
 for(;;)
 {
  hp = GE_Instructions;
  key = PopPrompt( PromptString, hp);

  stripblk(PromptString);

  if( key == ESC )
  {
#if 0
    SD386Log(STRINGCODE," ");
   SD386Log(KEYCODE|key,"");
#endif
   break;
  }
  else if ( key == ENTER )
  {
   if( strlen(PromptString) == 0 )
    continue;
   fp = GetFile(PromptString);
   if( fp )
   {
#if 0
        SD386Log(STRINGCODE," ");
        SD386Log(STRINGCODE,PromptString);
#endif
        break;
   }

   /**************************************************************************/
   /* display an error message and then ask for another name.                */
   /**************************************************************************/
   beep();
   hp = CantFind;
   fmterr( hp);
   continue;
  }
 }

 PutPopArea(SaveArea);
 Tfree(SaveArea );
 iview=_iview;
 return(fp);
}

/*****************************************************************************/
/* FindStr()                                                                 */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Get a string to find from the user in the current view.                 */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   fp          -> to the view we're searching in.                          */
/*                                                                           */
/* Return Value:                                                             */
/*                                                                           */
/*   none                                                                    */
/*                                                                           */
/*  Assumptions:                                                             */
/*                                                                           */
/*****************************************************************************/
void FindStr( AFILE *fp )
{
 char  *hp;
 uint   key;
 uchar *SaveArea;

 static int _iview;
 /****************************************************************************/
 /* - put the title, instructions, and help appropriate for this popup       */
 /*   into the popup structure.                                              */
 /* - save the screen area used by the popup if CUA interface.               */
 /* - get the string to find from the user.                                  */
 /* - handle user string and keys.                                           */
 /* - handle errors.                                                         */
 /* - restore the screen save area if CUA interface.                         */
 /*                                                                          */
 /****************************************************************************/
#if defined(MSH)
 if(commandLine.nparms>1) {
    strcpy(FindString,commandLine.parms[1]);
    if(ScanStr(fp)) {
#if defined(SD386LOG)
        SD386Log(STRINGCODE," ");
        SD386Log(STRINGCODE,commandLine.parms[1]);
#endif
        return;
    }
 }
#endif

 Popup.title = FI_Title;
 Popup.instructions = FI_Instructions;
 Popup.help = FINDSTRHELP;
 Popup.Flags = CLEAR1ST;

 SaveArea = (uchar *)Talloc(Popup.length * Popup.width * 2);
 GetPopArea(SaveArea);
 DisplayPop( &Popup );

 /****************************************************************************/
 /* - loop to handle user entry/errors.                                      */
 /*                                                                          */
 /*                                                                          */
 /* NOTE:                                                                    */
 /*   FindString is a static variable that will be saved for repeat finds and*/
 /*   subsequent prompts.                                                    */
 /****************************************************************************/
 for(;;)
 {
  hp = FI_Instructions;
  key = PopPrompt( FindString, hp);

  stripblk(FindString);

  if( key == ESC )
  {
#if 0
    SD386Log(STRINGCODE," ");
    SD386Log(KEYCODE|key,"");
#endif
    break;
  }
  else if ( key == ENTER )
  {
   if( strlen(FindString) == 0 )
    continue;

   if( ScanStr(fp) )
   {
#if 0
    SD386Log(STRINGCODE," ");
    SD386Log(KEYCODE|key,"");
#endif
    break;
   }

   /**************************************************************************/
   /* display an error message and then ask for another name.                */
   /**************************************************************************/
   beep();
   hp = CantFind;
   fmterr( hp);
   continue;
  }
 }

 PutPopArea(SaveArea);
 Tfree(SaveArea );
 iview=_iview;
}

/*****************************************************************************/
/* ScanStr()                                                                 */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Find a string in the view and modify the view structure to              */
/*   reflect the find.                                                       */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   none                                                                    */
/*                                                                           */
/* Return                                                                    */
/*                                                                           */
/*   rc         1 => success ( I know...it's backwards!! )                   */
/*              0 => failure                                                 */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   fp != NULL                                                              */
/*                                                                           */
/*****************************************************************************/
extern uint   LinesPer;

int ScanStr( AFILE *fp )
{
 uint          delta;
 int           rc = 0;

 rc = scan(fp, FindString);
 if( rc )
 {
  delta = fp->csrline - fp->topline;
  if( delta >= LinesPer )
  {
   fp->topline=fp->csrline-LinesPer/2;
   if( (int)fp->topline < 1 )
    fp->topline = 1;
  }
 }
 return(rc);
}

/*****************************************************************************/
/* BrowseFile()                                                              */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Find a string in the view and update the view.                          */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   none                                                                    */
/*                                                                           */
/* Return Value:                                                             */
/*                                                                           */
/*   none                                                                    */
/*                                                                           */
/*  Assumptions:                                                             */
/*                                                                           */
/*****************************************************************************/
 void
BrowseFile( )
{
 char  *hp;
 uint   key;
 char   PromptString[MAXPROMPTSTRING];
 uchar *SaveArea;


 static int _iview;
 /****************************************************************************/
 /* - put the title, instructions, and help appropriate for this popup       */
 /*   into the popup structure.                                              */
 /* - save the screen area used by the popup if CUA interface.               */
 /* - get the file to browse from the user.                                  */
 /* - handle user string and keys.                                           */
 /* - handle errors.                                                         */
 /* - restore the screen save area if CUA interface.                         */
 /*                                                                          */
 /****************************************************************************/

#if defined(MSH)
 if(commandLine.nparms>1)
 {
   uchar    *fnfull;                    /* fully qualified filespec.         */
   int       fnlen, rc;                 /* filename/filespec buffer length.  */
   fnlen = CCHMAXPATH;
   fnfull = Talloc( fnlen );
   rc = FindExe( commandLine.parms[1], fnfull+1 , fnlen );
   if(!rc) {
       if( !browse(commandLine.parms[1], 0) ) {
#if defined(SD386LOG)
         SD386Log(STRINGCODE," ");
         SD386Log(STRINGCODE,commandLine.parms[1]);
#endif
         return;
       }
    }
 }
#endif

 _iview=iview; iview=0;
 *PromptString = '\0';

 Popup.title = BR_Title;
 Popup.instructions = BR_Instructions;
 Popup.help = BROWSEHELP;

 SaveArea = (uchar *)Talloc(Popup.length * Popup.width * 2);
 GetPopArea(SaveArea);
 DisplayPop( &Popup );

 /****************************************************************************/
 /* - loop to handle user entry/errors.                                      */
 /****************************************************************************/
 for(;;)
 {
  hp = BR_Instructions;
  key = PopPrompt( PromptString, hp);

  stripblk(PromptString);

  if( key == ESC )
  {
#if 0
   SD386Log(STRINGCODE," ");
   SD386Log(KEYCODE|key,"");
#endif
   break;
  }

  else if ( key == ENTER )
  {
   uchar    *fnfull;                    /* fully qualified filespec.         */
   int       fnlen, rc;                 /* filename/filespec buffer length.  */

   if( strlen(PromptString) == 0 )
    continue;

   if( strpbrk( PromptString, "*?" ) )                                  /*802*/
   {                                                                    /*802*/
     hp = CantFind;                                                     /*802*/
     fmterr( hp );                                                      /*802*/
     beep();                                                            /*802*/
     continue;                                                          /*802*/
   }                                                                    /*802*/

   fnlen = CCHMAXPATH;
   fnfull = Talloc( fnlen );
   rc = FindExe( PromptString, fnfull+1 , fnlen );
   if( !rc )
   {
    PutPopArea(SaveArea);
    Tfree(SaveArea );
    SaveArea = NULL;
    if( !browse(PromptString, 0) )
    {
#if 0
      SD386Log(STRINGCODE," ");
      SD386Log(STRINGCODE,PromptString);
#endif
      break;
    }
   }

   /**************************************************************************/
   /* display an error message and then ask for another name.                */
   /**************************************************************************/
   beep();
   hp = CantFind;
   fmterr( hp);
   continue;
  }
 }
 if( SaveArea != NULL )
 {
  PutPopArea(SaveArea);
  Tfree(SaveArea );
  iview=_iview;
 }
}


#ifdef MSH
/*****************************************************************************/
/* BrowseMshFile()                                                           */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Find a string in the view and update the view.                          */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   none                                                                    */
/*                                                                           */
/* Return Value:                                                             */
/*                                                                           */
/*   none                                                                    */
/*                                                                           */
/*  Assumptions:                                                             */
/*                                                                           */
/*****************************************************************************/
WINDOWEVENT *BrowseMshFile(AFILE *fp)
{
 char *hp;
 static WINDOWEVENT we;

 /****************************************************************************/
 /* - put the title, instructions, and help appropriate for this popup       */
 /*   into the popup structure.                                              */
 /* - save the screen area used by the popup if CUA interface.               */
 /* - get the file to browse from the user.                                  */
 /* - handle user string and keys.                                           */
 /* - handle errors.                                                         */
 /* - restore the screen save area if CUA interface.                         */
 /*                                                                          */
 /****************************************************************************/

    if( !browsem(0,fp,&we) )
    {
#if defined(SD386LOG)
      SD386Log(STRINGCODE," ");
      SD386Log(STRINGCODE,PromptString);
#endif
      return(&we);
    }
    else {
   /**************************************************************************/
   /* display an error message and then ask for another name.                */
   /**************************************************************************/
   beep();
   hp = CantFind;
   fmterr( hp);
   }
}

#endif

/*****************************************************************************/
/* SetCondBreak()                                                            */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Set a conditional breakpoint.                                           */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   fp             -> to AFILE structure containing the current view.       */
/*   SrcLnNumOfCsr  the source line number that the cursor is currently      */
/*                  on.                                                      */
/*   pSrcLnInBuf    -> to the source line in the source buffer.              */
/*                                                                           */
/* Return                                                                    */
/*                                                                           */
/*   void                                                                    */
/*                                                                           */
/*  Assumptions:                                                             */
/*                                                                           */
/*                                                                           */
/*****************************************************************************/
 void
SetCondBrk( AFILE *fp, uint SrcLnNumOfCsr, uchar *pSrcLnInBuf )
{
 char  *hp;
 uint   key;
 char   PromptString[MAXPROMPTSTRING];
 char  *msg;
 uchar  SrcLnFlag;
 BRK   *pBrk;
 uchar *SaveArea;
 static int _iview;

#if defined(MSH)
 if(commandLine.nparms>1) {
    msg = SetLineBRK( fp,SrcLnNumOfCsr,pSrcLnInBuf,BRK_COND,commandLine.parms[1]);
#if defined(SD386LOG)
    SD386Log(STRINGCODE," ");
    SD386Log(STRINGCODE,commandLine.parms[1]);
#endif
 }
#endif

 _iview=iview; iview=0;
 *PromptString = '\0';
 /***************************************************************************/
 /* If there is a conditional bp already on this line, then we need to      */
 /* initialize the PromptString to the last prompt.                         */
 /***************************************************************************/
 SrcLnFlag = *(pSrcLnInBuf-1);
 if( (SrcLnFlag & LINE_BP) &&
     (pBrk=IsBrk(fp , SrcLnNumOfCsr) ) &&
      pBrk->cond )
  strcpy(PromptString, pBrk->cond->pCondition);

 /****************************************************************************/
 /* - put the title, instructions, and help appropriate for this popup       */
 /*   into the popup structure.                                              */
 /* - save the screen area used by the popup if CUA interface.               */
 /* - get the file to browse from the user.                                  */
 /* - handle user string and keys.                                           */
 /* - handle errors.                                                         */
 /* - restore the screen save area if CUA interface.                         */
 /*                                                                          */
 /****************************************************************************/
 Popup.title = CB_Title;
 Popup.instructions = CB_Instructions;
 Popup.help = CONDBPHELP;

 SaveArea = (uchar *)Talloc(Popup.length * Popup.width * 2);
 GetPopArea(SaveArea);
 DisplayPop( &Popup );

 /****************************************************************************/
 /* - loop to handle user entry/errors.                                      */
 /****************************************************************************/
 for(;;)
 {
  hp = CB_Instructions;
  key = PopPrompt( PromptString, hp);

  stripblk(PromptString);

  if( key == ESC )
  {
#if 0
   SD386Log(STRINGCODE," ");
   SD386Log(KEYCODE|key,"");
#endif
   break;
  }
  else if ( key == ENTER )
  {
   if( strlen(PromptString) == 0 )
    continue;

   msg = SetLineBRK( fp,SrcLnNumOfCsr,pSrcLnInBuf,BRK_COND,PromptString);
   if( !msg )
   {
#if 0
     SD386Log(STRINGCODE," ");
     SD386Log(STRINGCODE,PromptString);
#endif
    break;
   }

   /**************************************************************************/
   /* display an error message and then ask for another name.                */
   /**************************************************************************/
   beep();
   hp = msg;
   fmterr( msg );
   continue;
  }
 }

 PutPopArea(SaveArea);
 Tfree(SaveArea );
  iview=_iview;
}

/*****************************************************************************/
/* SetNameOrAddrBkpt()                                                       */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Set a name or address breakpoint including deferred breaks.             */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/* Return                                                                    */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*                                                                           */
/*****************************************************************************/
void  SetNameOrAddrBkpt( AFILE *fp , uint func )
{
 char   *hp;
 uint    key;
 char    PromptString[MAXPROMPTSTRING];
 uchar  *SaveArea;
 int     DorI;
 int     DefineType;
 char   *pmsg;
 APIRET  rc;
 static int _iview;

#if defined(MSH) && 0
 if(commandLine.nparms>1) {
    msg = SetIDBrk( pnode, commandLine.parms[1], &Brk );                         /*707*/
    if( !msg )
    {
      AFILE *lfp;                                                        /*707*/

      for( lfp = allfps; lfp; lfp = lfp->next )                          /*707*/
        if( lfp->mid == Brk.mid )                                        /*707*/
          MarkLineBRKs( lfp );                                           /*707*/
#if defined(SD386LOG)
      SD386Log(STRINGCODE," ");
      SD386Log(STRINGCODE,commandLine.parms[1]);
#endif
      return;
    }
 }
#endif

 _iview=iview; iview=0;
 *PromptString = '\0';
 /****************************************************************************/
 /* - put the title, instructions, and help appropriate for this popup       */
 /*   into the popup structure.                                              */
 /* - save the screen area used by the popup if CUA interface.               */
 /* - get the file to browse from the user.                                  */
 /* - handle user string and keys.                                           */
 /* - handle errors.                                                         */
 /* - restore the screen save area if CUA interface.                         */
 /*                                                                          */
 /****************************************************************************/
 switch( func )
 {
  case SETFUNCTIONBKPT:
   Popup.title        = IF_Title;
   Popup.instructions = IF_Instructions;
   Popup.help         = IDFUNCHELP;
   DorI               = BP_IMMEDIATE;
   DefineType         = BP_FUNC_NAME;
   hp                 = IF_Instructions;
   break;

  case SETADDRESSBKPT:
   Popup.title        = IA_Title;
   Popup.instructions = IA_Instructions;
   Popup.help         = IDADDRHELP;
   DorI               = BP_IMMEDIATE;
   DefineType         = BP_ADDR;
   hp                 = IA_Instructions;

   strcpy( PromptString, "0x" );
   break;

  case SETDEFERREDBKPT:
   Popup.title        = ID_Title;
   Popup.instructions = ID_Instructions;
   Popup.help         = IDDEFRHELP;
   DorI               = BP_DEFR;
   DefineType         = BP_FUNC_NAME;
   hp                 = ID_Instructions;
   break;

  case  SETADDRLOADBKPT:
   Popup.title        = IO_Title;
   Popup.instructions = IO_Instructions;
   Popup.help         = IDLOADHELP;
   DorI               = BP_DEFR;
   DefineType         = BP_LOAD_ADDR;
   hp                 = IO_Instructions;

   strcpy( PromptString, "0x" );
   break;

  case  SETDLLLOADBKPT:
   Popup.title        = IL_Title;
   Popup.instructions = IL_Instructions;
   Popup.help         = IDDLLHELP;
   DorI               = BP_DEFR;
   DefineType         = BP_DLL_LOAD;
   hp                 = IL_Instructions;
   break;
 }

 SaveArea = (uchar *)Talloc(Popup.length * Popup.width * 2);
 GetPopArea(SaveArea);
 DisplayPop( &Popup );

 /****************************************************************************/
 /* - loop to handle user entry/errors.                                      */
 /****************************************************************************/
 for(;;)
 {

  key = PopPrompt( PromptString, hp);

  stripblk(PromptString);

  if( key == ESC )
  {
#if 0
   SD386Log(STRINGCODE," ");
   SD386Log(KEYCODE|key,"");
#endif
   break;
  }
  else if ( key == ENTER )
  {
   if( strlen(PromptString) == 0 )
    continue;

   pmsg = NULL;
   rc = SetIDBrk( PromptString, DorI, DefineType, &pmsg );

   if( rc )
   {
    beep();
    hp = pmsg;
    fmterr( pmsg );
    continue;

#if 0
     AFILE *lfp;                                                        /*707*/
     SD386Log(STRINGCODE," ");
     SD386Log(STRINGCODE,PromptString);

     for( lfp = allfps; lfp; lfp = lfp->next )                          /*707*/
       if( lfp->mid == Brk.mid )                                        /*707*/
         MarkLineBRKs( lfp );                                           /*707*/
#endif
   }
   break;
   /**************************************************************************/
   /* display an error message and then ask for another name.                */
   /**************************************************************************/
  }
 }

 PutPopArea(SaveArea);
 Tfree(SaveArea );
  iview=_iview;
}

/*****************************************************************************/
/* PopPrompt                                                                 */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Get a popup string and return a key( alias mouse event )                */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   PromptString   where the prompt string will be put.                     */
/*   msg            -> for compatibility with old prompt method.             */
/*                                                                           */
/* Return                                                                    */
/*                                                                           */
/*   key        a key code                                                   */
/*                                                                           */
/*****************************************************************************/
uint PopPrompt( char *PromptString, char *msg )
{
 uint   key;
 static int _iview;
 _iview=iview; iview=0;
 key = GetPopStr( &Popup, POP_PROMPT_START_ROW, POP_PROMPT_START_COL,
                   POP_PROMPT_LENGTH, PromptString);
 iview=_iview;

 return(key);
}

/*****************************************************************************/
/* GetLineNumber()                                                           */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Prompt the user for a source line number within the current view.       */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   InitString  -> to a string to use as an initial prompt.                 */
/*                                                                           */
/* Return Value:                                                             */
/*                                                                           */
/*   int         a line number.                                              */
/*                                                                           */
/*  Assumptions:                                                             */
/*                                                                           */
/*   none                                                                    */
/*                                                                           */
/*****************************************************************************/
int GetLineNumber( char c )
{
 char  *hp;
 uint   key;
 char   PromptString[MAXPROMPTSTRING];
 uchar *SaveArea;
 int    LineNumber;
 static int _iview;

 LineNumber = 0;
 sprintf(PromptString,"%c",c);
 Popup.title = LN_Title;
 Popup.instructions = LN_Instructions;
 Popup.help = LNFUNCHELP;

 SaveArea = (uchar *)Talloc(Popup.length * Popup.width * 2);
 GetPopArea(SaveArea);
 DisplayPop( &Popup );

 /****************************************************************************/
 /* loop to handle user entry/errors.                                        */
 /****************************************************************************/
 for(;;)
 {
  hp = LN_Instructions;

  key = PopPrompt( PromptString, hp);

  stripblk(PromptString);

  if( key == ESC )
   break;
  else if ( key == ENTER )
  {
   if( strlen(PromptString) == 0 )
    continue;
   LineNumber = atoi(PromptString);
   break;
  }
 }

 PutPopArea(SaveArea);
 Tfree(SaveArea );
  iview=_iview;
 return(LineNumber);
}

/*****************************************************************************/
/* GetFormatType()                                                           */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Prompt the user for a file name and then try to build a view            */
/*   for that function.                                                      */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   none                                                                    */
/*                                                                           */
/* Return Value:                                                             */
/*                                                                           */
/*   fp          -> to AFILE structure containing the new view.              */
/*   NULL        couldn't find the specified file. Or, escaped.              */
/*                                                                           */
/*  Assumptions:                                                             */
/*                                                                           */
/*                                                                           */
/*****************************************************************************/
UINT GetFormatType( DFILE *dfp )
{
 char  *hp;
 uint   key;
 char   PromptString[MAXPROMPTSTRING];
 uchar *SaveArea;
 int    LeaveLoop;
 static int _iview;

 /****************************************************************************/
 /* - put the title, instructions, and help appropriate for this popup       */
 /*   into the popup structure.                                              */
 /* - save the screen area used by the popup if CUA interface.               */
 /* - get a function name from the user.                                     */
 /* - handle user string and keys.                                           */
 /* - handle errors.                                                         */
 /* - restore the screen save area if CUA interface.                         */
 /*                                                                          */
 /****************************************************************************/
 _iview=iview; iview=0;
 *PromptString = '\0';

 Popup.title = GT_Title;
 Popup.instructions = GT_Instructions;
 Popup.help = GETTYPEHELP;

 SaveArea = (uchar *)Talloc(Popup.length * Popup.width * 2);
 GetPopArea(SaveArea);
 DisplayPop( &Popup );

 /****************************************************************************/
 /* loop to handle user entry/errors.                                        */
 /****************************************************************************/
 for( LeaveLoop=FALSE; LeaveLoop==FALSE;)
 {
  hp = GT_Instructions;
  key = PopPrompt( PromptString, hp);

  stripblk(PromptString);

  if(  (key == ESC) ||
       ( (key == ENTER) && (strlen(PromptString)==0) )
    )
   LeaveLoop = TRUE;
  else
  {
   switch( key )
   {
    case ENTER:
    case PADENTER:
    {
     UINT    mid4type = 0;
     USHORT  typeno;
     UCHAR  *hp;
     UCHAR   buffer[PROMAX+2];

     memset(buffer, 0, sizeof(buffer));
     strcpy(buffer+2, PromptString);

     *(USHORT*)buffer = (USHORT)strlen(PromptString);

     typeno = QtypeNumber(dfp->mid, buffer, &mid4type );

     if( mid4type != 0 )
      dfp->mid = mid4type;

     if(typeno)
     {
      SetShowType( dfp, (UINT)typeno );
      LeaveLoop = TRUE;
     }
     else
     {
      beep();
      hp = "Incorrect type name";
      fmterr( hp);
     }
    }
    break;

    case ESC:
     LeaveLoop = TRUE;
     break;
   }
  }
 }

 PutPopArea(SaveArea);
 Tfree(SaveArea );
  iview=_iview;
 return(0);
}
