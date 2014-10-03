/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   breakpnt.c                                                              */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Breakpoint file handling.                                               */
/*                                                                           */
/* History:                                                                  */
/*                                                                           */
/*   11/30/95 Created.                                                       */
/*                                                                           */
/*****************************************************************************/

#include "all.h"
#include "breakpnt.h"

static int  BreakpointFileTime;

/*****************************************************************************/
/* SaveBreakpoints()                                                         */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Save the currently defined breakpoints to a file.                       */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
void SaveBreakpoints( void )
{

 extern PROCESS_NODE *pnode;

 char    *pEnvSD386Brk;
 UCHAR   *pHelpMsg;
 FILE    *BkptFile_fp;
 int      irc;
 BRK     *pBrk;
 DEBFILE *pdf;
 char    *cp;
 char    *cpp;
 int      len;

 /****************************************************************************/
 /* - Get the breakpoint file environment variable.                          */
 /****************************************************************************/
 pEnvSD386Brk = GetSD386Brk();

 /****************************************************************************/
 /* - Get some information about the breakpoint file.  If it can't be        */
 /*   accessed then display a message.                                       */
 /****************************************************************************/
 BkptFile_fp = fopen(pEnvSD386Brk,"wb");
 if( BkptFile_fp == NULL )
   Error( ERR_BKPT_FILE_OPEN, FALSE, 1, pEnvSD386Brk );

 /****************************************************************************/
 /* - write syntax info to the file.                                         */
 /****************************************************************************/
 pHelpMsg = GetHelpMsg( HELP_BKPTS_SYNTAX, NULL, 0 );
 irc = fprintf(BkptFile_fp, pHelpMsg);
 if( irc < 0 )
   Error( ERR_BKPT_FILE_WRITE, FALSE, 1, pEnvSD386Brk );

 if( pHelpMsg ) Tfree(pHelpMsg);

 SayMsgBox5( HELP_BKPTS_SAVE_MSG, 1, pEnvSD386Brk );

 /****************************************************************************/
 /* - Reverse linked list so that breakpoints are stored in the same order   */
 /*   they came in. pBrk is left pointing to the tail node.                  */
 /* - Just return if the list is empty.                                      */
 /****************************************************************************/
 pBrk = pnode->allbrks;

 if( pBrk == NULL )
  goto fini;

 for( pBrk->prev = NULL; pBrk->next; )
 {
  pBrk->next->prev = pBrk;
  pBrk             = pBrk->next;
 }

 /****************************************************************************/
 /* - Now, write the breakpoints to a file.                                  */
 /****************************************************************************/
 for( ; pBrk ; pBrk = pBrk->prev )
 {
  if( pBrk->brkat )
  {
   /**************************************************************************/
   /* - Handle converted breakpoints.                                        */
   /**************************************************************************/
   pdf = FindExeOrDllWithAddr(pBrk->brkat);
   if( pdf == NULL )
    continue;  /* bad breakpoint...blow by */

   /**************************************************************************/
   /* - check/get a valid dllname.                                           */
   /**************************************************************************/
   if( pBrk->dllname )
   ;
   else
   {
    cp = strrchr( pdf->DebFilePtr->fn, '\\' );
    cp++;
    strlwr(cp);
    pBrk->dllname = Talloc(strlen(cp) + 1);
    strcpy(pBrk->dllname, cp);
   }

   /**************************************************************************/
   /* - now, check/build a valid (file name, line number ).                  */
   /**************************************************************************/
   if( pBrk->srcname && pBrk->lno )
   ;
   else if( pBrk->srcname == NULL )
   {
    cp = GetFileName( pBrk->mid, pBrk->sfi );
    if( cp )
    {
     len = *cp;
     cpp = strrchr( cp+1, '\\' );

     cpp = ( cpp == NULL )?cp+1:cpp+1;
     len = ( cpp == NULL )?len:len-(cpp-cp)+1;

     pBrk->srcname = Talloc(len+1);

     strncpy(pBrk->srcname, cpp, len);
    }
   }
   else /* if( pBrk->lno == 0 ) */
   {
    LNOTAB *pLnoTabEntry;

    DBMapInstAddr(pBrk->brkat, &pLnoTabEntry, pdf);
    if( pLnoTabEntry != NULL )
     pBrk->lno = pLnoTabEntry->lno;
   }
   /**************************************************************************/
   /* - now, abort if we still don't have a srcname and an lno.              */
   /**************************************************************************/
   if( pBrk->srcname && pBrk->lno )
   ;
   else
    continue;  /* abort this breakpoint. */

   fprintf(BkptFile_fp, "\n{");
   fprintf(BkptFile_fp, "%s",  pBrk->dllname );

   if( pBrk->flag.DefineType == BP_FUNC_NAME)
    fprintf(BkptFile_fp, ",%s", pBrk->funcname );
   else
   {
    fprintf(BkptFile_fp, ",%s", pBrk->srcname );
    fprintf(BkptFile_fp, ",%d", pBrk->lno );
   }

   if( pBrk->flag.DorI == BP_DEFR )
    fprintf( BkptFile_fp, ",D" );

   if( pBrk->cond && pBrk->cond->pCondition )
    fprintf(BkptFile_fp, ",%s", pBrk->cond->pCondition );

   fprintf(BkptFile_fp, "}");
  }
  else
  {
   /**************************************************************************/
   /* - handle unconverted breakpoints.                                      */
   /**************************************************************************/
   fprintf(BkptFile_fp, "\n{");
   fprintf(BkptFile_fp, "%s",  pBrk->dllname );
   if( pBrk->srcname )
   {
    fprintf(BkptFile_fp, ",%s", pBrk->srcname );
    fprintf(BkptFile_fp, ",%d", pBrk->lno );
   }
   else
    fprintf(BkptFile_fp, ",%s", pBrk->funcname );

   if( pBrk->flag.DorI == BP_DEFR )
    fprintf( BkptFile_fp, ",D" );

   if( pBrk->cond && pBrk->cond->pCondition )
    fprintf(BkptFile_fp, ",%s", pBrk->cond->pCondition );

   fprintf(BkptFile_fp, "}");
  }
 }

fini:

 if(BkptFile_fp)
 {
  fprintf(BkptFile_fp, "%c", 0x1A );
  /***************************************************************************/
  /* - update the time stamp for the breakpoint file.                        */
  /***************************************************************************/
  {
   struct stat statbuffer;

   fstat( fileno(BkptFile_fp), &statbuffer );

   BreakpointFileTime = statbuffer.st_mtime;
  }
  fclose(BkptFile_fp);
 }


 SetUsingBreakpointFileFlag();
 return;
}

/*****************************************************************************/
/* InitBreakpoints                                                           */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Check for breakpoint file usage and set initial time stamp.             */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
static BOOL UsingBreakpointFileFlag;

void ResetBreakpointFileTime(void){BreakpointFileTime = -1;}

void SetUsingBreakpointFileFlag(void)  {UsingBreakpointFileFlag = TRUE; }
void ResetUsingBreakpointFileFlag(void){UsingBreakpointFileFlag = FALSE;}
BOOL IsUsingBreakpointFile(void)       {return(UsingBreakpointFileFlag);}

void InitBreakpoints( void )
{
 FILE *BkptFile_fp;
 char *pEnvSD386Brk;

 /****************************************************************************/
 /* - Get the breakpoint file environment variable. If it's not defined      */
 /*   then the user doesn't want to use a breakpoint file.                   */
 /****************************************************************************/
 pEnvSD386Brk = GetSD386Brk();

 /****************************************************************************/
 /* - Get some information about the breakpoint file.  If it can't be        */
 /*   accessed then display a message.                                       */
 /****************************************************************************/
 BkptFile_fp = fopen(pEnvSD386Brk,"rb");
 if( BkptFile_fp != NULL )
 {
  SetUsingBreakpointFileFlag();
  fclose(BkptFile_fp);
 }
 else
  ResetUsingBreakpointFileFlag();

 ResetBreakpointFileTime();
 return;
}

/*****************************************************************************/
/* MergeBreakpoints()                                                        */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Make a decision aboute whether or not to restore the breakpoints        */
/*   from a file.                                                            */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
void MergeBreakpoints( void )
{
 if( IsUsingBreakpointFile() == TRUE )
 {
  AFILE *fp;

  RestoreBreakpoints();

  fp = Getfp_focus();
  if(fp)
   MarkLineBRKs(fp);
 }
}

/*****************************************************************************/
/* RestoreBreakpoints                                                        */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Read a file of breakpoints and define them.                             */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
void RestoreBreakpoints( void )
{
 APIRET   rc;
 int      Length;
 char    *pEnvSD386Brk;
 off_t    BkptFileSize;
 FILE    *BkptFile_fp;
 char    *pBkptFileText;
 char    *pBkptFileTextEnd;
 char    *pBkptDefinition;
 char    *pTempCopyBkptDefinition;
 char    *pCopyBkptDefinition;
 char    *pToken;
 int      BkptIndex;
 int      NumBreaks;
 int      errlno;
 char    *pMsg;
 BOOL     SyntaxErr;
 char    *cp;
 int      CommaCount;
 ULONG    addr;
 ULONG    mid;
 ULONG    junk;
 DEBFILE *pdf;
 BRK     *pBrk = NULL;
 ULONG    lno;
 int     *pBkptLinNumInfo;
 int      DefineType;
 int      sfi;

 struct stat statbuffer;

 struct _syntax
 {
  char *pexe;
  char *pfunc;
  char *pfile;
  char *plno;
  char *pdefr;
  char *pcond;
 }syntax;

 pCopyBkptDefinition = NULL;
 pBkptFileText       = NULL;

 /****************************************************************************/
 /* - Get the breakpoint file environment variable. If it's not defined      */
 /*   then the user said he didn't want to use a breakpoint file.            */
 /****************************************************************************/
 pEnvSD386Brk = GetSD386Brk();

 /****************************************************************************/
 /* Get some information about the breakpoint file. If it can't be accessed  */
 /* then display a message.                                                  */
 /****************************************************************************/
 BkptFile_fp = fopen(pEnvSD386Brk,"rb");
 if( BkptFile_fp == NULL )
 {
   Error( ERR_BKPT_FILE_OPEN, FALSE, 1, pEnvSD386Brk );
   return;
 }

 /****************************************************************************/
 /* - Get the file status. We assume this will work since the fopen worked.  */
 /* - If it hasn't changed then don't update it.                             */
 /****************************************************************************/
 fstat( fileno(BkptFile_fp), &statbuffer );

 if( BreakpointFileTime == -1 )
 ;
 else
 {
  /***************************************************************************/
  /* - check the time stamp and if it hasn't changed don't do anything.      */
  /***************************************************************************/
  if( statbuffer.st_mtime <= BreakpointFileTime )
   goto fini;
 }

 /****************************************************************************/
 /* - update the time stamp.                                                 */
 /****************************************************************************/
 BreakpointFileTime = statbuffer.st_mtime;

 /****************************************************************************/
 /* - Allocate/read the breakpoint file.                                     */
 /****************************************************************************/
 BkptFileSize = statbuffer.st_size;

 if( BkptFileSize == 0 )
   return;

 SayMsgBox5( HELP_BKPTS_RESTORE_MSG, 1, pEnvSD386Brk );

 pBkptFileText = Talloc(BkptFileSize + 1);

 fread( pBkptFileText, BkptFileSize, 1, BkptFile_fp);

 /****************************************************************************/
 /* - Strip out the comments.                                                */
 /* - Remove the crlfs.                                                      */
 /* - Get the number of breakpoints and build a table of breakpoint          */
 /*   line number locations.                                                 */
 /* - Blank out crlfs.                                                       */
 /****************************************************************************/
 pBkptFileTextEnd = pBkptFileText + BkptFileSize - 1;

 rc = StripComments( pBkptFileText, pBkptFileTextEnd );
 if( rc != 0 )
  Error( ERR_BKPT_FILE_COMMENT, FALSE, 1, pEnvSD386Brk );

 NumBreaks = 0;
 pBkptLinNumInfo = BuildBkptLinNumInfo(  pBkptFileText,
                                         pBkptFileTextEnd,
                                        &NumBreaks
                                      );

 BlankOutCRLFs( pBkptFileText, pBkptFileTextEnd );

 /****************************************************************************/
 /* - Tokenize/process the breakpoints.                                      */
 /****************************************************************************/
 BkptIndex = 0;
 SyntaxErr = FALSE;
 pBkptDefinition = strtok( pBkptFileText, "}");
 do
 {
  char *cps;
  char *cpd;

  /***************************************************************************/
  /* - Allocate space for a copy of the breakpoint.                          */
  /* - Allocate space for a temp copy of the breakpoint.                     */
  /* - Copy the break point to the temp copy.                                */
  /* - Compress the temp copy.                                               */
  /* - Copy the temp copy to the "real" copy adding spaces in front of       */
  /*   any commas.                                                           */
  /*                                                                         */
  /*    This is done to force strtok() to work in the desired manner.        */
  /*    The additional 4 bytes of allocation is for the maximum number       */
  /*    of comma delimiters.                                                 */
  /*                                                                         */
  /* - free the temp copy.                                                   */
  /*                                                                         */
  /***************************************************************************/
  Length = strlen(pBkptDefinition) + 1 + 4;
  cps    = pTempCopyBkptDefinition = Talloc( Length );
  cpd    = pCopyBkptDefinition     = Talloc( Length );

  strcpy(pTempCopyBkptDefinition, pBkptDefinition );
  Strip( pTempCopyBkptDefinition, '.', '.', 'c');
  for(; *cpd++ = *cps++ ;) {if(*cps == ',') *cpd++ = ' ';}
  Tfree(pTempCopyBkptDefinition);

  /***************************************************************************/
  /* - We will build a structure of pointers to the tokens.                  */
  /***************************************************************************/
  memset( &syntax, 0, sizeof(syntax) );
  pToken = strtok( pCopyBkptDefinition, ",");

  /***************************************************************************/
  /* - process dll/exe token                                                 */
  /***************************************************************************/
  if( (pToken == NULL) || (*pToken != '{') )
  {
   Error( ERR_BKPT_FILE_SYNTAX_BRACE, FALSE, 1, pEnvSD386Brk );
   SyntaxErr = TRUE; goto error;
  }
  else if( *(pToken+1) == ' ' )
  {
   pMsg = GetHelpMsg(ERR_BKPT_FILE_SYNTAX_DLLEXE, NULL, 0);
   beep(); SayStatusMsg(pMsg);

   errlno = pBkptLinNumInfo[BkptIndex];
   browse(pEnvSD386Brk, errlno );
   SyntaxErr = TRUE; goto error;
  }
  else
  {
   syntax.pexe = pToken + 1;

   /**************************************************************************/
   /* - process function name/ (filename,line number) token.                 */
   /**************************************************************************/
   pToken = strtok(NULL, ",");

   if( (pToken == NULL) || (*pToken == ' ') )
   {
    pMsg = GetHelpMsg(ERR_BKPT_FILE_SYNTAX_FUNC, NULL, 0);
    beep(); SayStatusMsg(pMsg);

    errlno = pBkptLinNumInfo[BkptIndex];
    browse(pEnvSD386Brk, errlno );
    SyntaxErr = TRUE; goto error;
   }
   else if( strchr(pToken, '.') == NULL )
     syntax.pfunc = pToken;
   else
   {
    syntax.pfile = pToken;

    /*************************************************************************/
    /* - process the line number.                                            */
    /*************************************************************************/
    pToken = strtok(NULL, "," );
    if( (pToken == NULL) || (*pToken == ' ') )
    {
     pMsg = GetHelpMsg(ERR_BKPT_FILE_SYNTAX_LNO, NULL, 0);
     beep(); SayStatusMsg(pMsg);

     errlno = pBkptLinNumInfo[BkptIndex];
     browse(pEnvSD386Brk, errlno );
     SyntaxErr = TRUE; goto error;
    }
    else
    {
     /************************************************************************/
     /* - Verify that the line number is a number.                           */
     /************************************************************************/
     for( cp = pToken ; *cp ; cp++ )
     {
      if( ((*cp < '0') || (*cp > '9')) && (*cp != ' ') )
      {
       pMsg = GetHelpMsg(ERR_BKPT_FILE_SYNTAX_LNO, NULL, 0);
       beep(); SayStatusMsg(pMsg);

       errlno = pBkptLinNumInfo[BkptIndex];
       browse(pEnvSD386Brk, errlno );
       SyntaxErr = TRUE; goto error;
      }
     }
     syntax.plno = pToken;
    }
   }

   /**************************************************************************/
   /* - now look for a type and condition.                                   */
   /**************************************************************************/
   pToken = strtok(NULL, "," );
   if( (pToken == NULL) || (*pToken == ' ') )
   ;/* done parsing */
   else
   {
    if( (stricmp(pToken, "d ") != 0) && (stricmp(pToken, "d") != 0) )
     syntax.pcond = pToken;
    else
    {
     syntax.pdefr = pToken;

     pToken = strtok(NULL, "," );
     if( (pToken == NULL) || (*pToken == ' ') )
     ;/* done parsing */
     else
      syntax.pcond = pToken;
    }
   }
  }

  /***************************************************************************/
  /*  - At this point, we're done parsing.                                   */
  /*  - The conditional text may have been compressed for the parsing;so now,*/
  /*    if there was a condition, then we have to define a pointer to        */
  /*    the real condition in the original breakpoint definition.            */
  /*                                                                         */
  /***************************************************************************/
  if( syntax.pcond != NULL )
  {
   /**************************************************************************/
   /* - Count the number of commas that were been tokenized.                 */
   /* - Move to the location of the nth comma in the original breakpoint     */
   /*   text and define this as the pointer to the breakpoint condition.     */
   /**************************************************************************/
   CommaCount = 0;
   cp         = pCopyBkptDefinition;
   for( ; cp < syntax.pcond; cp++ ) if( *cp == '\0' ) CommaCount++;

   for( cp = pBkptDefinition; CommaCount; cp++ )
    if( *cp == ',' )
     CommaCount--;

   syntax.pcond = cp;
  }

  /****************************************************************************/
  /* - Now, define the breakpoint.                                            */
  /****************************************************************************/
  Strip( syntax.pexe, '.', '.', 'c');

  DefineType = BP_FUNC_NAME;
  if( syntax.pfunc == NULL )
   DefineType = BP_SRC_LNO;

  /***************************************************************************/
  /* - kill a .dll extension if there is one.                                */
  /***************************************************************************/
  if( syntax.pexe && ((cp = strchr( syntax.pexe, '.' )) != NULL) )
   *cp = '\0';

  pdf = findpdf( syntax.pexe );
  if( pdf == NULL )
  {
   /***************************************************************************/
   /* - force the deferred condition.                                         */
   /***************************************************************************/
   syntax.pdefr = (void*)1UL;

   if( DefineType == BP_FUNC_NAME )
   {
    Strip( syntax.pfunc, '.', '.', 'c');

    pBrk                  = DefBrk( NULL, FALSE );
    pBrk->flag.DorI       = BP_DEFR;
    pBrk->flag.DefineType = BP_FUNC_NAME;
    pBrk->flag.ActionType = BRK_SIMP;
    pBrk->flag.File       = TRUE;

    if( strstr(syntax.pexe , "(null)") )
     pBrk->dllname = NULL;
    else
    {
     pBrk->dllname = Talloc(strlen(syntax.pexe)+1);
     strcpy(pBrk->dllname, syntax.pexe);
    }

    if( strstr(syntax.pfunc, "(null)") )
    {
     pBrk->funcname        = NULL;
     pBrk->flag.DefineType = BP_DLL_LOAD;
    }
    else
    {
     pBrk->funcname = Talloc(strlen(syntax.pfunc)+1);
     strcpy(pBrk->funcname, syntax.pfunc);
    }
   }
   else
   {
    Strip( syntax.pfile, '.', '.', 'c');

    pBrk                  = DefBrk( NULL, FALSE );
    pBrk->flag.DorI       = BP_DEFR;
    pBrk->flag.DefineType = BP_SRC_LNO;
    pBrk->flag.ActionType = BRK_SIMP;
    pBrk->flag.File       = TRUE;
    pBrk->lno             = atol( syntax.plno );
    pBrk->srcname         = Talloc(strlen(syntax.pfile)+1);
    pBrk->dllname         = Talloc(strlen(syntax.pexe)+1);

    strcpy(pBrk->dllname, syntax.pexe);
    strcpy(pBrk->srcname, syntax.pfile);
   }

   if( syntax.pcond )
   {
    cp = syntax.pcond;
    while( *cp == ' ') cp++;
    if( *cp == '!' )
     pBrk->cond->relation = COND_MSH;
   }
  }
  else /* pdf != NULL */
  /****************************************************************************/
  /* - define an immediate breakpoint.                                        */
  /****************************************************************************/
  {
   if( DefineType == BP_FUNC_NAME )
   {
    /**************************************************************************/
    /* - function name entry point/immediate.                                 */
    /**************************************************************************/
    Strip( syntax.pfunc, '.', '.', 'c');

    addr = DBPub(syntax.pfunc, pdf);
    if( addr == 0 )
    {
     /************************************************************************/
     /* - check for microsoft underscore names.                              */
     /************************************************************************/
     int   len = strlen(syntax.pfunc);
     char *cp  = Talloc(len + 2);

     cp[0] = '_';
     strcpy(cp+1, syntax.pfunc);
     addr = DBPub(cp, pdf);
     Tfree(cp);
    }

    {
     LNOTAB *pLnoTabEntry;

     lno = 0;
     mid = DBMapInstAddr( addr, &pLnoTabEntry, pdf);
     if( pLnoTabEntry != NULL )
     {
      lno = pLnoTabEntry->lno;
      sfi = pLnoTabEntry->sfi;
     }
    }

    if( addr == NULL )
    {
     pMsg = GetHelpMsg(ERR_BKPT_FILE_SYNTAX_NOFUNC, NULL, 0);
     beep(); SayStatusMsg(pMsg);

     errlno = pBkptLinNumInfo[BkptIndex];
     browse(pEnvSD386Brk, errlno );
     SyntaxErr = TRUE; goto error;
    }
   }
   else
   {
    /**************************************************************************/
    /* - (filename, line number) breakpoint/immediate.                        */
    /**************************************************************************/
    Strip( syntax.pfile, '.', '.', 'c');

    cp = strrchr( syntax.pfile, '\\' );
    cp = ( cp == NULL )?syntax.pfile:cp+1;
    {
     /************************************************************************/
     /* - need to build an lpz. ( added at linnum re-write.)                 */
     /************************************************************************/
     UCHAR len;
     char  *cpp;

     len     = strlen(cp);
     cpp     = Talloc(len + 2);
     cpp[0]  = len;

     strcpy(cpp+1, cp );

     mid = MapSourceFileToMidSfi( cpp, &sfi, pdf );
     if(cpp) Tfree(cpp);
    }
    lno = atol( syntax.plno );

    if( mid == 0 )
    {
     pMsg = GetHelpMsg(ERR_BKPT_FILE_SYNTAX_NOFILE, NULL, 0);
     beep(); SayStatusMsg(pMsg);

     errlno = pBkptLinNumInfo[BkptIndex];
     browse(pEnvSD386Brk, errlno );
     SyntaxErr = TRUE; goto error;
    }

    addr = DBMapLno(mid, lno, sfi, &junk , pdf );
    if( addr == NULL )
    {
     uchar plno[80];
     UCHAR   *SubStringTable[MAX_SUBSTRINGS];
     sprintf(plno,"%d",lno);
     SubStringTable[0]=plno;
     SubStringTable[1]=cp;
     pMsg = GetHelpMsg(ERR_BKPT_DEFN_LINENO, SubStringTable,2);
     beep();
     SayStatusMsg(pMsg);

     errlno = pBkptLinNumInfo[BkptIndex];
     browse(pEnvSD386Brk, errlno );
     SyntaxErr = TRUE; goto error;
    }
   }

   /**************************************************************************/
   /* - Check for a breakpoint already defined at this address and undefine  */
   /*   it if there is one.                                                  */
   /**************************************************************************/
   pBrk = IfBrkOnAddr( addr );
   if( pBrk )
    UndBrk( addr, TRUE );

   pBrk                  = DefBrk( addr, TRUE );
   pBrk->flag.DorI       = BP_IMMEDIATE;
   pBrk->flag.DefineType = DefineType;
   pBrk->flag.ActionType = BRK_SIMP;
   pBrk->flag.File       = TRUE;
   pBrk->lno             = lno;
   pBrk->mid             = mid;
   pBrk->sfi             = sfi;
   pBrk->dllname         = Talloc(strlen(syntax.pexe)+1);

   strcpy(pBrk->dllname, syntax.pexe);

   if( DefineType == BP_FUNC_NAME )
   {
    pBrk->funcname = Talloc(strlen(syntax.pfunc) + 1);
    strcpy(pBrk->funcname, syntax.pfunc);
   }
   else
   {
    pBrk->srcname = Talloc(strlen(syntax.pfile) + 1);
    strcpy(pBrk->srcname, syntax.pfile);
   }
  }

  /***************************************************************************/
  /* - add conditional information to the breakpoint.                        */
  /***************************************************************************/
  if( syntax.pcond )
  {
   pBrk->cond             = (BRKCOND*) Talloc(sizeof(BRKCOND));
   pBrk->cond->pCondition = Talloc(strlen(syntax.pcond)+1);

   strcpy(pBrk->cond->pCondition, syntax.pcond);

   cp = syntax.pcond;
   while( *cp == ' ') cp++;

   if( *cp == '!' )
    pBrk->cond->relation = COND_MSH;

   if( syntax.pdefr == NULL )
   {
    /************************************************************************/
    /* - If this is not a deferred break, then we can parse the condition   */
    /*   and report any errors. If the breakpoint is deferred, we have to   */
    /*   wait until convertdefbrks() gets called at module load time.       */
    /************************************************************************/
    cp = ParseCbrk( pBrk->cond, mid, lno, sfi );
    if(cp)
    {
     pMsg = GetHelpMsg(ERR_BKPT_FILE_SYNTAX_COND, &cp, 1 );
     beep(); SayStatusMsg(pMsg);

     errlno = pBkptLinNumInfo[BkptIndex];
     browse(pEnvSD386Brk, errlno );
     SyntaxErr = TRUE; goto error;
    }
   }
  }

  Tfree(pCopyBkptDefinition);
  pCopyBkptDefinition = NULL;


  pBkptDefinition += strlen(pBkptDefinition) + 1;
  pBkptDefinition  = strtok(pBkptDefinition, "}" );
  BkptIndex++;
 }
 while( (BkptIndex < NumBreaks) && (pBkptDefinition != NULL) );

/*****************************************************************************/
/* - come here after all break have been processed.                          */
/* - come here on syntax errors.                                             */
/*****************************************************************************/
fini:
error:
 if( BkptFile_fp )
  fclose( BkptFile_fp );

 if( pBkptFileText )       Tfree(pBkptFileText);
 if( pCopyBkptDefinition ) Tfree(pCopyBkptDefinition);

 if( (SyntaxErr == TRUE) && pBrk )
 {
  if( syntax.pdefr )
   UndBrk( pBrk->brkat, FALSE );
  else
   UndBrk( pBrk->brkat, TRUE );
 }
 return;
}

/*****************************************************************************/
/* EditBreakpoints                                                           */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Edit the breakpoint file.                                               */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
static ULONG EditorSessionID;
void EditBreakpoints( void )
{
 APIRET  rc;
 char   *pEnvSD386Editor;
 char   *pEnvSD386Brk;
 FILE   *EditFile_fp;
 char    Editor[CCHMAXPATH];
 BOOL    EditAccess;
 FILE    *BkptFile_fp;

 struct stat statbuffer;

 /****************************************************************************/
 /* - Get the editor environment variable.                                   */
 /****************************************************************************/
 pEnvSD386Editor = GetSD386Editor();

 if( pEnvSD386Editor == NULL )
 {
  Error( ERR_BKPT_FILE_ENV_EDITOR, FALSE, NULL, 0 );
  return;
 }

 /****************************************************************************/
 /* - now, make sure that we can access the editor.                          */
 /****************************************************************************/
 EditFile_fp = NULL;
 EditAccess  = TRUE;
 if( strrchr(pEnvSD386Editor, '\\' ) )
 {
  /***************************************************************************/
  /* - test with fopen if an explicit path was given.                        */
  /***************************************************************************/
  EditFile_fp = fopen(pEnvSD386Editor,"r");
  if( EditFile_fp == NULL )
   EditAccess = FALSE;
 }
 else
 {
  rc=DosSearchPath(SEARCH_IGNORENETERRS|SEARCH_ENVIRONMENT|SEARCH_CUR_DIRECTORY,
                   "PATH",
                   pEnvSD386Editor,
                   Editor,
                   sizeof(Editor) );
  if( rc )
   EditAccess = FALSE;
 }

 /****************************************************************************/
 /* - now, are we able to access the editor?                                 */
 /****************************************************************************/
 if( EditAccess == FALSE )
 {
  Error( ERR_BKPT_FILE_EDITOR_OPEN, FALSE, 1, pEnvSD386Editor );
  if( EditFile_fp ) fclose(EditFile_fp);
  return;
 }

 if( EditFile_fp ) fclose(EditFile_fp);

 /****************************************************************************/
 /* - Get the breakpoint file environment variable. If it's not defined      */
 /*   then the user said he didn't want to use a breakpoint file.            */
 /****************************************************************************/
 pEnvSD386Brk = GetSD386Brk();

 if( pEnvSD386Brk == NULL )
 {
  Error( ERR_BKPT_FILE_ENV_SD386BRK, FALSE, NULL, 0 );
  return;
 }

 /****************************************************************************/
 /* Get some information about the breakpoint file. If it can't be accessed  */
 /* then display a message.                                                  */
 /****************************************************************************/
 BkptFile_fp = fopen(pEnvSD386Brk,"rb");
 if( BkptFile_fp == NULL )
 {
   Error( ERR_BKPT_FILE_OPEN, FALSE, 1, pEnvSD386Brk );
   return;
 }

 /****************************************************************************/
 /* - If an editor session was previously started, then:                     */
 /*                                                                          */
 /*    1.the user may have edited the file and saved it.                     */
 /*    2.the user may have simply quit without modifying.                    */
 /*    3.the edit session may still be out there.                            */
 /*                                                                          */
 /* - We want to detect cases 1 and 2 and invalidate the session id.         */
 /* - In case 3, we simply want to select the session.                       */
 /*                                                                          */
 /****************************************************************************/
 if( EditorSessionID != 0 )
 {
  fstat( fileno(BkptFile_fp), &statbuffer );
  if( statbuffer.st_mtime <= BreakpointFileTime )
  {
   rc = DosSelectSession( EditorSessionID );
   if( rc )
    EditorSessionID = 0;
  }
  else
   EditorSessionID = 0;
 }

 if( EditorSessionID == 0 )
 {
  extern CmdParms cmd;

  STARTDATA  sd;
  char       MsgBuf[CCHMAXPATH];
  ULONG      pid;
  ULONG      EditorType;
  ULONG      SessionType;

  /***************************************************************************/
  /* - Define the session type.                                              */
  /***************************************************************************/
  SessionType = SSF_TYPE_DEFAULT;
  if( cmd.ProcessType != SSF_TYPE_PM )
  ;
  else
  {
   DosQueryAppType( pEnvSD386Editor, &EditorType );
   EditorType = EditorType & 0x7;
   if( EditorType == SSF_TYPE_PM )
   {
    Error( ERR_BKPT_FILE_EDITOR_PMTYPE, FALSE, NULL, 0 );
    return;
   }
   SessionType = SSF_TYPE_FULLSCREEN;
  }

  /***************************************************************************/
  /* - time stamp the file.                                                  */
  /***************************************************************************/
  if(BkptFile_fp)
  {
   fstat( fileno(BkptFile_fp), &statbuffer );
   BreakpointFileTime = statbuffer.st_mtime;
   fclose(BkptFile_fp);
  }

  memset(&sd, 0, sizeof(sd) );
  memset(MsgBuf, 0, sizeof(MsgBuf) );

  sd.Length        = sizeof(sd);
  sd.Related       = SSF_RELATED_CHILD;
  sd.FgBg          = SSF_FGBG_FORE;
  sd.TraceOpt      = SSF_TRACEOPT_NONE;
  sd.InheritOpt    = SSF_INHERTOPT_PARENT;
  sd.PgmName       = pEnvSD386Editor;
  sd.PgmInputs     = pEnvSD386Brk;
  sd.SessionType   = SessionType;
  sd.PgmTitle      = "SD386 Breakpoint Editor";
  sd.PgmControl    = SSF_CONTROL_VISIBLE;
  sd.ObjectBuffLen = sizeof(MsgBuf);
  sd.ObjectBuffer  = MsgBuf;

  rc = DosStartSession( (PSTARTDATA)&sd, &EditorSessionID, &pid );
  if(rc)
  {
   char rcstring[20];

   sprintf(rcstring, "%d", rc);
   Error( ERR_BKPT_FILE_EDITOR_START, FALSE, 2, rcstring, MsgBuf );
  }
 }
}


/*****************************************************************************/
/* Strip()                                                                   */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Strip characters from a string with optional compression.               */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   cp             -> to the string.                                        */
/*   c              character to be removed.                                 */
/*   copt           which ones to remove:                                    */
/*                   'l' ==>Leading                                          */
/*                   't' ==>Trailing                                         */
/*                   'a' ==>All                                              */
/*                   '.' ==>ignore                                           */
/*   xopt           compression:                                             */
/*                   'c' ==>compress(left justify)                           */
/*                   '.' ==>no compression                                   */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   The string is a null terminated.                                        */
/*                                                                           */
/*   Removed characters are replaced with blanks.                            */
/*                                                                           */
/*   Compression means removing the spaces after the characters are          */
/*   removed and left-justifying the string.                                 */
/*                                                                           */
/*****************************************************************************/
#define BLANK ' '
void Strip(char *cp, char c, char copt, char xopt)
{
 char *cpp;
 char *cpq;
 char *cpr;
 char *cpend;

 switch( tolower(copt) )
 {
  case 't':
   /**************************************************************************/
   /* - Scan to the last non-blank character in the string.                  */
   /* - Replace it with a blank.                                             */
   /**************************************************************************/
   for( cpp=cp+strlen(cp)-1; (cpp >= cp) && (*cpp==BLANK); cpp--){;}
   if( (cpp >= cp) && (*cpp == c) )
    *cpp = BLANK;
   break;

  case 'l':
   /**************************************************************************/
   /* - Scan to the first non-blank character in the string.                 */
   /* - Replace it with a blank.                                             */
   /**************************************************************************/
   for( cpp=cp, cpend=cp+strlen(cp); (cpp < cpend) && (*cpp==BLANK); cpp++){;}
   if( (cpp < cpend) && (*cpp == c) )
    *cpp = BLANK;
   break;

  case 'a':
   /**************************************************************************/
   /* - Scan the string replacing all matching chars with blanks.            */
   /**************************************************************************/
   for( cpp=cp, cpend=cp+strlen(cp); cpp < cpend ; cpp++)
   {
    if( *cpp == c )
      *cpp = BLANK;
   }
   break;

  case '.':
   break;
 }

 /****************************************************************************/
 /* Now, do the compression.                                                 */
 /****************************************************************************/
 if( tolower(xopt) == 'c' )
 {
  for( cpp=cp; ; cpp++)
  {
   if( *cpp == BLANK )
   {
    for( cpq=cpp; *cpq == BLANK; cpq++ ){;}
    for( cpr=cpp; *cpr++ = *cpq++; ){;}
   }
   if( *cpp == '\0' )
    break;
  }
 }
}


/*****************************************************************************/
/* StripComments()                                                           */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Strip comments from a block of text. Allow for nested comments.         */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   pTextBlk    ->to the block of text.                                     */
/*   pEndOfText  ->to the end of the text block.                             */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   rc     0 ==>success                                                     */
/*          1 ==>failure( unmatched comment at this time )                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
APIRET StripComments( char *pTextBlk, char *pEndOfText )
{
 char *cp;
 char *cpp;
 char *cpend;

 /****************************************************************************/
 /* - scan for the start of a comment.                                       */
 /****************************************************************************/
 cpp   = pTextBlk;
 cpend = pEndOfText;
 while( cpp <= pEndOfText )
 {
  for( cp = cpp; (cp <= cpend) && (*cp != '/'); cp++ ){;}

  cpp = FindEndOfComment( cp+1, pEndOfText );

  /***************************************************************************/
  /* - check for unmatched comment error.                                    */
  /***************************************************************************/
  if( cpp == NULL )
   return(1);

  /***************************************************************************/
  /* - blank out the comment.                                                */
  /***************************************************************************/
  if( cpp > (cp+1) )
   BlankOut(cp, cpp);
 }
 return(0);
}

/*****************************************************************************/
/* FindEndOfComment()                                                        */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Find the end of a comment in a block of text. The comment may contain   */
/*   nested comments.                                                        */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   pTextBlk    ->to the block of text.                                     */
/*   pEndOfText  ->to the end of the text block.                             */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   cp  -> to the end of the comment.                                       */
/*       NULL if unmatched comment.                                          */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*            ......./...                                                    */
/*                    |                                                      */
/*                    |                                                      */
/*  pStartOfComment-->   points to this position in the text.                */
/*                                                                           */
/*****************************************************************************/
char *FindEndOfComment( char *pStartOfComment, char *pEndOfText )
{
 char *cp;
 int   NestCount = 0;

 /****************************************************************************/
 /*                                                                          */
 /*                       ----------------------------------                 */
 /*                      |                                  |*               */
 /*                      |                                 -----------       */
 /*                      |                                |Start of   |      */
 /*                      |                          ----->|Comment    |      */
 /*                      |                   c     |      |           |      */
 /*                      |                  ----   |       -----------       */
 /*                      |                 |    |  |/         |!*            */
 /*         -----------  |   -----------   |    |  |          |              */
 /* begin  |           |  ->|           |  |   ---------      |              */
 /* ------>|Start of   |*   |+Transition|   ->|         |     |              */
 /*        |Comment    |--->|           |     |         |<----               */
 /*   <----|           |    | Nested++  |---->| Comment |                    */
 /*     !*  -----------      -----------      |         |   end(nested=0)    */
 /*                                           |         |-------->           */
 /*                              ------------>|         |                    */
 /*                             |              ---------                     */
 /*                             |               |  |*                        */
 /*                             |               |  |                         */
 /*                             |               |  |       -----------       */
 /*                             |               |   ----->|End of     |      */
 /*                             |               |         |Comment    |      */
 /*                             |                <--------|           |      */
 /*                             |                     !/   -----------       */
 /*                          -----------                    |                */
 /*                         |-Transition|                   |/               */
 /*                         |           |<------------------                 */
 /*                         | Nested--  |                                    */
 /*                          -----------                                     */
 /*                                                                          */
 /****************************************************************************/
 cp = pStartOfComment;

 if( *cp != '*' )
  return(cp);

 NestCount++;
 cp++;

 for( ; cp < pEndOfText; )
 {
  switch( *cp )
  {
   case '*':
    if( *(cp+1) == '/' )
     {NestCount -= 1; cp++;}
    break;

   case '/':
    if( *(cp+1) == '*' )
     {NestCount += 1; cp++;}
    break;

   default:
    break;
  }

  if( NestCount == 0 )
   break;

  cp++;

 }

 if( NestCount != 0 )
  cp = NULL;
 return(cp);
}

/*****************************************************************************/
/* Fill with blanks. Leave in the newlines.                                  */
/*****************************************************************************/
void BlankOut( char *cpstart, char *cpend)
{
 char *cp;

 for( cp = cpstart; cp <= cpend; cp++ )
  if( *cp != '\n' )
   *cp = ' ';
}

/*****************************************************************************/
/* Replace crs, lfs, and end-of-file.                                        */
/*****************************************************************************/
void BlankOutCRLFs( char *cpstart, char *cpend)
{
 char *cp;

 for( cp = cpstart; cp <= cpend; cp++ )
 {
  if( (*cp == '\r') || (*cp == '\n') || (*cp == 0x1A) )
  {
   *cp = ' ';
  }
 }
}

/*****************************************************************************/
/* - Build a table of line numbers and breakpoint indices.                   */
/* - The table is allocated here but is freed by the caller.                 */
/*****************************************************************************/
int *BuildBkptLinNumInfo( char *cpstart, char *cpend, int *pNumBreaks)
{
 char *cp;
 int   NumBreaks = 0;
 int   BkptLineNumber;
 int   BkptIndex;

 int  *bp;

 for( cp = cpstart; cp <= cpend; cp++ )
 {
  if( *cp == '{' )
  {
   NumBreaks++;
  }
 }

 bp = Talloc( NumBreaks * sizeof(int) );

 BkptLineNumber = 1;
 BkptIndex      = 0;

 for( cp = cpstart; cp <= cpend; cp++ )
 {
  if( *cp == '\n' )
  {
   BkptLineNumber++;
  }
  if( *cp == '{' )
  {
   bp[BkptIndex] = BkptLineNumber;
   BkptIndex++;
  }
 }
 *pNumBreaks = NumBreaks;
 return( bp );
}

/*****************************************************************************/
/* GetSD386Brk()/FreeSD386Brk()                                              */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Define/Retrieve/Free the breakpoint file location.                      */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   pSD386Brk   -> user specified environment variable.                     */
/*               NULL ==> user doesn't want to work with breakpoint files.   */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
static char *pSD386Brk = NULL;
char *GetSD386Brk( void )
{
 char *p;
 char  exename[13];

 if( pSD386Brk == NULL )
 {
  pSD386Brk = getenv(ENV_SD386BRK_NAME);

  if( pSD386Brk == NULL )
  {
   extern CmdParms cmd;
   ALLPIDS *p;

   char *cp;
   char *cpp;
   int   len;

   /**************************************************************************/
   /* - massage the exe name to build a default breakpoint file name.        */
   /**************************************************************************/
   p   = GetPid( cmd.ProcessID);
   cp  = strrchr( p->pFileSpec, '\\' );
   cpp = strchr(  cp, '.');
   len = cpp - cp - 1;
   memset( exename, 0, sizeof(exename) );
   strncpy( exename, cp+1, len);
   strcat(  exename, ".BRK" );
   strlwr(  exename );

   pSD386Brk = Talloc(strlen(exename) + 1);

   strcpy(pSD386Brk, exename );
  }
  else
  {
   /**************************************************************************/
   /* - make a local copy and hold on to it.                                 */
   /**************************************************************************/
   p         = pSD386Brk;
   pSD386Brk = Talloc(strlen(p) + 1);

   strcpy(pSD386Brk, p);

   Strip(pSD386Brk,';','t','.');
   Strip(pSD386Brk,'\\','t','c');
  }
 }
 return(pSD386Brk);
}

void FreeSD386Brk( void )
{
 if(pSD386Brk)
 {
  Tfree(pSD386Brk);
  pSD386Brk = NULL;
 }
 FreeSD386Editor();
}

/*****************************************************************************/
/* GetSD386Editor()/FreeSD386Editor()                                        */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Define/Retrieve/Free the editor for the breakpoint file.                */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   pSD386Editor -> user specified environment variable.                    */
/*                NULL ==> user doesn't want to work with breakpoint files.  */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
static char *pSD386Editor = NULL;
char *GetSD386Editor( void )
{
 char *cp;
 int   len;

 if( pSD386Editor == NULL )
 {
  cp = getenv(ENV_SD386EDITOR_NAME);

  if( cp == NULL )
   return( NULL );
  else
  {
   /**************************************************************************/
   /* - allocate space for the editor environment variable and make an       */
   /*   allowance for adding a .exe extension if need be.                    */
   /**************************************************************************/
   len          = strlen(cp) + 4;
   pSD386Editor = Talloc(len + 1);

   strcat(pSD386Editor, cp);
   Strip (pSD386Editor, ';' , 't', '.' );
   Strip (pSD386Editor, '\\', 't', 'c' );

   strlwr(pSD386Editor);
   if( strstr(pSD386Editor, ".exe") == NULL )
    strcat(pSD386Editor, ".exe");
  }
 }
 return(pSD386Editor);
}

void FreeSD386Editor( void )
{
 if(pSD386Editor)
 {
  Tfree(pSD386Editor);
  pSD386Editor = NULL;
 }
}

void KillEditorSession( void )
{
 if( EditorSessionID )
 {
  DosStopSession(STOP_SESSION_ALL, EditorSessionID);
 }
}

