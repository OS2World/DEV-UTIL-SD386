/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   invoke.c                                                                */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Parent/Child debugger invocation handling.                              */
/*                                                                           */
/* History:                                                                  */
/*                                                                           */
/*   11/04/93 Created.                                                       */
/*                                                                           */
/*****************************************************************************/

#include "all.h"

/*****************************************************************************/
/* ParseInvocationOptions                                                    */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*  Parse the invocation line entered by the user.                           */
/*                                                                           */
/*   Syntax:                                                                 */
/*                                                                           */
/*   SD386 [SD386 invocation options] [program name] [ program options ]     */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*    argc           number of invocation parameters.                        */
/*    argv        -> to array of ptrs to invocation parameters.              */
/*    pcmd        -> to the structure that will define the invocation parms. */
/*    pConnection -> to the structure that will define the connection parms. */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
void ParseInvocationOptions( int argc,
                             char**argv,
                             CmdParms *pcmd,
                             CONNECTION *pConnection )
{
 char  ParmString[512];
 int   i;
 char *pOption;
 char *cp;
 int   len;

 /****************************************************************************/
 /* - Handle the argc==1 case.                                               */
 /****************************************************************************/
 if( argc == 1 )
  SayMsg(HELP_INVOCATION_SD386);

 /****************************************************************************/
 /* - set default options.                                                   */
 /****************************************************************************/
 pConnection->DbgOrEsp    = _DBG;
 pConnection->ConnectType = BOUND;

 pcmd->DbgFlags.Verbose        = FALSE;
 pcmd->DbgFlags.IsParentDbg    = TRUE;
 pcmd->DbgFlags.UseExecPgm     = FALSE;
 pcmd->DbgFlags.DebugChild     = FALSE;
 pcmd->DbgFlags.SingleMultiple = SINGLE;
 pcmd->DbgFlags.DosDebugTrace  = FALSE;

 /****************************************************************************/
 /* - Build a string of tokens delimited by a single blank.                  */
 /****************************************************************************/
 ParmString[0] = '\0';
 for( i=1 ; i<argc; i++)
 {
  strcat(ParmString,argv[i]);
  strcat(ParmString," ");
 }
 strcat(ParmString,"\0");

 /****************************************************************************/
 /* - Now, parse the SD386 invocation options.                               */
 /*   At this point, the case where argc==1 has already been handled.        */
 /****************************************************************************/
 pOption = strtok(ParmString," ");
 do
 {
  switch( *pOption )
  {
   case '?':
    /*************************************************************************/
    /* - print the message file help.                                        */
    /*************************************************************************/
    SayMsg(HELP_INVOCATION_SD386);
    break;


   default:
    /*************************************************************************/
    /* - pOption will be pointing to the program name at this point. The     */
    /*   strtok function will have converted the blank at the end of         */
    /*   the name to a \0.                                                   */
    /* - convert the name to upper case.                                     */
    /* - append the .EXE to the program name if user did not enter it.       */
    /*************************************************************************/
    strupr(pOption);
    len = strlen(pOption);
    if( !strstr( pOption , ".EXE"))
     len += 4;
    cp  = Talloc(len+1);
    strcpy(cp,pOption);
    if( !strstr( pOption , ".EXE"))
     strcpy(cp+strlen(pOption),".EXE");
    cp[len] = '\0';
    pcmd->pUserExe   = cp;

    /*************************************************************************/
    /* - parse the user parameters exactly as they were entered by the user. */
    /*************************************************************************/
    pcmd->pUserParms = ParseUserParameters();
    return;

   case '/':
   case '-':

    switch( tolower(*(pOption + 1)) )
    {
     default:
      ErrorPrintf( ERR_BAD_INVOCATION_OPTION, 1, pOption );
      break;

     case 'h':
     case '?':
      /***********************************************************************/
      /* - print the message file help.                                      */
      /***********************************************************************/
      SayMsg(HELP_INVOCATION_SD386);
      break;

     case 'p':
      /***********************************************************************/
      /* - TRUE says read the user profile.                                  */
      /***********************************************************************/
      pcmd->Profile = TRUE;
      break;

     case 'c':
      /***********************************************************************/
      /* - TRUE says be case sensitive.                                      */
      /***********************************************************************/
      pcmd->CaseSens = TRUE;
      break;

     case 'i':
      /***********************************************************************/
      /* - TRUE says stop at the first exe/dll entry point.                  */
      /***********************************************************************/
      pcmd->ShowInit = TRUE;
      break;

     case 'f':
      /***********************************************************************/
      /* - 1 ==> start the application in a full screen session.             */
      /***********************************************************************/
      pcmd->SessionType  = SSF_TYPE_FULLSCREEN;
      break;

     case 'w':
      /***********************************************************************/
      /* - 2 ==> start the application in a text window.                     */
      /***********************************************************************/
      pcmd->SessionType  = SSF_TYPE_WINDOWABLEVIO;
      break;

     case 'k':
      /***********************************************************************/
      /* - TRUE says ignore mouse input and use keyboard only.               */
      /***********************************************************************/
      pcmd->KeyboardOnly = TRUE;
      break;

     case 'u':
      /***********************************************************************/
      /* - don't flush the keyboard buffer before getting a keystroke.       */
      /***********************************************************************/
      pcmd->KBDBufferFlush = NOFLUSHALLTIMES;
      break;

     case 'm':
      /************************************************************************/
      /* - Use imports or the math shell.                                     */
      /************************************************************************/
      if( strnicmp( pOption+1, "msh", 3 ) == 0 )
      {
       pcmd->DbgFlags.UseMsh  = TRUE;
      }
      else
       pcmd->ResolveImports = TRUE;
      break;

     case 'o':
      /***********************************************************************/
      /* - pull off the modem control file name.                             */
      /***********************************************************************/
      pConnection->modem = TRUE;
      pConnection->pModemFile = NULL;
      len = strlen(pOption+2);
      if( len != 0 )
      {
       cp  = Talloc(len+1);
       strcpy(cp,pOption + 2);
       cp[len] = '\0';
       pConnection->pModemFile = cp;
      }
      break;

     case 'r':
      /***********************************************************************/
      /* - TRUE says we want to remote debug.                                */
      /* - Parse the bit rate.                                               */
      /* - Set a flag for esp.                                               */
      /***********************************************************************/
      pConnection->ConnectType = ASYNC;
      pConnection->BitRate     = ParseBitRate( *(pOption+2) );

      break;

     case 'a':
      /***********************************************************************/
      /* - Parse the com port.                                               */
      /***********************************************************************/
      pConnection->ComPort = ParseComPort( *(pOption+2) );
      break;

     case 's':
      /***********************************************************************/
      /* - Blow by main.                                                     */
      /***********************************************************************/
      pcmd->NoStopAtMain = TRUE;
      break;

     case 'e':
      /***********************************************************************/
      /* - Use DosExecPgm() to start the debuggee.                           */
      /***********************************************************************/
      pcmd->DbgFlags.UseExecPgm   = TRUE;
      break;

     case 'b':
      /***********************************************************************/
      /* - Debug the children of the parent debuggee.                        */
      /* - Build a filespec for the name(s) of the children to debug.        */
      /***********************************************************************/
      pcmd->DbgFlags.DebugChild = TRUE;
      pcmd->pChildProcesses     = NULL;

      len = strlen(pOption+2);
      if( len != 0 )
      {
       cp = Talloc(strlen(pOption+2) + 1);
       strcpy(cp,strupr(pOption+2));
       pcmd->pChildProcesses = cp;
      }
      else
      {
       ErrorPrintf( ERR_BAD_INVOCATION_OPTION, 1, pOption );
      }
      break;

     case '@':
      if( strlen( pOption+1 ) != 1 )
       ErrorPrintf( ERR_BAD_INVOCATION_OPTION, 1, pOption );
      /***********************************************************************/
      /* - turn DosDebug() tracing on.                                       */
      /***********************************************************************/
      pcmd->DbgFlags.DosDebugTrace = TRUE;
      break;

     case '+':
      pcmd->DbgFlags.Verbose = TRUE;
      break;

     case '!':
      pcmd->DbgFlags.UseDebug  = TRUE;
      break;

     case 'n':
      /***********************************************************************/
      /* - connect using netbios.                                            */
      /* - get the logical adapter name.                                     */
      /***********************************************************************/
      pConnection->ConnectType = _NETBIOS;
      len = strlen(pOption+2);
      if( len != 0 )
      {
       if( len > (MAX_LSN_NAME - LSN_RES_NAME) )
        len = MAX_LSN_NAME - LSN_RES_NAME;
       cp = Talloc(len + 1);
       strncpy( cp, pOption+2, len );
       pConnection->pLsnName = cp;
      }
      break;

     case 't':
     {
      /***********************************************************************/
      /* - connect using sockets.                                            */
      /***********************************************************************/
      pConnection->ConnectType = SOCKET;
      len = strlen(pOption+2);
      cp  = Talloc(len + 1);

      strncpy( cp, pOption+2, len );

      pConnection->pLsnName = cp;
     }
     break;

     case 'x':
      pcmd->DbgFlags.HotKey = TRUE;
      SetHotKey();
      break;

     case '$':
     {
      int    cols;
      int    rows;

      VIOMODEINFO md;

      sscanf(pOption+2,"%dx%d",&cols, &rows);

      memset(&md, 0, sizeof(md));
      md.cb  = sizeof(md);
      VioGetMode(&md, 0);

      md.row = rows;
      md.col = cols;

      if( VioSetMode(&md, 0) )
      {
       printf("Unable to set mode %dx%d", cols, rows );
       exit(0);
      }
     }
     break;

    }
  }
  pOption = strtok(NULL," " );
 }
 while( pOption );
 return;
}

/*****************************************************************************/
/* ParseUserParameters                                                       */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*  Get a pointer to the user parameters exactly as they were entered        */
/*  by the user.                                                             */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*  pUserParms   -> to allocated block of user parameters.                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
char *ParseUserParameters( void )
{
 TIB  *pTib;
 PIB  *pPib;
 char *pCmdInvocation;
 char  UserParms[512];
 char *pUserParms;
 char *cp;
 int   len;

 /****************************************************************************/
 /*                                                                          */
 /* Call DosGetInfoBlocks() to get a pointer to command line invocation.     */
 /* We will get a pointer to a block formatted like so:                      */
 /*                                                                          */
 /*   string1\0string2\0\0.                                                  */
 /*                                                                          */
 /*   where:                                                                 */
 /*                                                                          */
 /*     string1=fully qualified invocation such as D:\SOMEDIR\SD386.EXE.     */
 /*     string2=everything entered by the user after string1.                */
 /*                                                                          */
 /****************************************************************************/
 DosGetInfoBlocks(&pTib,&pPib);
 pCmdInvocation = pPib->pib_pchcmd;

 /****************************************************************************/
 /* - blow by string1.                                                       */
 /****************************************************************************/
 while ( (*pCmdInvocation++) != '\0'){;}

 /****************************************************************************/
 /* - make a local copy of "string2"                                         */
 /* - and, blow by any sd386 invocation options.                             */
 /****************************************************************************/
 memset( UserParms,0,sizeof(UserParms) );
 strcpy(UserParms,pCmdInvocation);
 cp = strtok(UserParms," ");
 while( *cp=='/' || *cp=='-' ) cp=strtok(NULL," ");

 /****************************************************************************/
 /* At this point, cp will be pointing to the debuggee program name.         */
 /*                                                                          */
 /* - Blow by the program name.                                              */
 /* - If there are really some parms, then return a pointer to them.         */
 /****************************************************************************/
 while ( (*cp++) != '\0' ){;}
 len = strlen(cp);
 if( len == 0 )
  pUserParms = NULL;
 else
 {
  pUserParms = Talloc(len+1);
  strcpy(pUserParms,cp);
 }
 return(pUserParms);
}
/*****************************************************************************/
/* ParseBitRate                                                              */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*  Get the bit rate for remote debugging.                                   */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*  c            user specified bitrate 0-6.                                 */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
int ParseBitRate( char c )
{
 int BitRate;

 switch( c )
 {
  case '0':
   BitRate = 300;
   break;

  case '1':
   BitRate = 1200;
   break;

  case '2':
   BitRate = 2400;
   break;

  case '3':
   BitRate = 4800;
   break;

  case '4':
   BitRate = 9600;
   break;

  case '5':
   BitRate = 19200;
   break;

  case '6':
   BitRate = 38400;
   break;

  default:
   BitRate = 0;
   break;
 }
 return(BitRate);
}

/*****************************************************************************/
/* ParseComPort                                                              */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*  Get the com port to use for remote debugging.                            */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*  c            user specified com port 1-4.                                */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*  ComPort      user specified Com Port 1-4.                                */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
int ParseComPort( char c )
{
 int ComPort;

 switch( c )
 {
  case '1':
   ComPort = 1;
   break;

  case '2':
   ComPort = 2;
   break;

  case '3':
   ComPort = 3;
   break;

  case '4':
   ComPort = 4;
   break;

  default:
   SayMsg(ERR_COM_PORT_INVALID);
   break;
 }
 return(ComPort);
}

/*****************************************************************************/
/* ParseChildInvocationOptions                                               */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*  Parse the invocation line passed to a child debug session from           */
/*  the parent.                                                              */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*    argc           number of invocation parameters.                        */
/*    argv        -> to array of ptrs to invocation parameters.              */
/*    pcmd        -> to the structure that will define the invocation parms. */
/*    pConnection -> to the structure that will define the connection parms. */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
void ParseChildInvocationOptions( int         argc,
                                  char       **argv,
                                  CmdParms    *pcmd,
                                  CONNECTION  *pConnection )
{
 char  ParmString[512];
 int   i;
 char *pOption;
 char *cp;
 int   len;

 /****************************************************************************/
 /* - set any default options.                                               */
 /****************************************************************************/
 pConnection->DbgOrEsp    = _DBG;
 pConnection->ConnectType = BOUND;

 pcmd->DbgFlags.IsParentDbg    = FALSE;
 pcmd->DbgFlags.SingleMultiple = MULTIPLE;
 pcmd->DbgFlags.UseExecPgm     = FALSE;

 /****************************************************************************/
 /* - Build a string of tokens delimited by a single blank.                  */
 /****************************************************************************/
 ParmString[0] = '\0';
 for( i=1 ; i<argc; i++)
 {
  strcat(ParmString,argv[i]);
  strcat(ParmString," ");
 }
 strcat(ParmString,"\0");

 /****************************************************************************/
 /* - Now, parse the SD386 invocation options.                               */
 /*   At this point, the case where argc==1 has already been handled.        */
 /****************************************************************************/
 pOption = strtok(ParmString," ");
 do
 {
  switch( *pOption )
  {

   default:
    break;

   case '/':
   case '-':

    switch( tolower(*(pOption + 1)) )
    {
     default:
      break;

     case 'p':
      /***********************************************************************/
      /* - TRUE says read the user profile.                                  */
      /***********************************************************************/
      pcmd->Profile = TRUE;
      break;

     case 'c':
      /***********************************************************************/
      /* - TRUE says be case sensitive.                                      */
      /* - handle the /child= option which MUST be argv[1];                  */
      /***********************************************************************/
      if( strlen( pOption+1 ) == 1 )
       pcmd->CaseSens = TRUE;
      else if( strstr( pOption+1, "child" ) )
       pcmd->ProcessID = atoi( strchr( argv[1], '=' ) + 1 );
      break;

     case 'i':
      /***********************************************************************/
      /* - TRUE says stop at the first exe/dll entry point.                  */
      /***********************************************************************/
      pcmd->ShowInit = TRUE;
      break;

     case 'k':
      /***********************************************************************/
      /* - TRUE says ignore mouse input and use keyboard only.               */
      /***********************************************************************/
      pcmd->KeyboardOnly = TRUE;
      break;

     case 'u':
      /***********************************************************************/
      /* - don't flush the keyboard buffer before getting a keystroke.       */
      /***********************************************************************/
      pcmd->KBDBufferFlush = NOFLUSHALLTIMES;
      break;

     case 'm':
      /***********************************************************************/
      /* - TRUE says display imported variables.                             */
      /***********************************************************************/
      pcmd->ResolveImports = TRUE;
      break;

     case 's':
      /***********************************************************************/
      /* - Blow by main and                                                  */
      /* - Set the shared heap memory handle.                                */
      /***********************************************************************/
      if( strlen( pOption+1 ) == 1 )
       pcmd->NoStopAtMain = TRUE;
      else if( strstr( pOption+1, "shr" ) )
      {
       ULONG *pshrmem;

       pshrmem = (ULONG*)atol( strchr( pOption+1, '=' ) + 1 );
       SetShrMem( pshrmem );
      }
      break;

     case 'q':
      /***********************************************************************/
      /* - Set the queue name for sending messages.                          */
      /***********************************************************************/
      if( strstr( pOption+1, "qname" ) )
      {
       char *pQueName;

       pQueName = strchr( pOption+1, '=' ) + 1 ;
       SetDbgQueName( pQueName );
      }
      break;

     case 'h':
      /***********************************************************************/
      /* - Set the start address of the shared heap.                         */
      /***********************************************************************/
      if( strstr( pOption+1, "heap" ) )
      {
       ULONG *heap;

       heap = (ULONG*)atol( strchr( pOption+1, '=' ) + 1 );
       SetShrHeap( heap );
      }
      else if( strstr( pOption+1, "handle" ) )
      {
       pcmd->handle = (LHANDLE)atol( strchr( pOption+1, '=' ) + 1 );
      }
      break;

     case 'r':
      /***********************************************************************/
      /* - TRUE says we want to remote debug.                                */
      /* - Parse the bit rate.                                               */
      /* - Set a flag for esp.                                               */
      /***********************************************************************/
      pConnection->ConnectType = ASYNC;
      break;

     case 'n':
      /***********************************************************************/
      /* - connect using netbios.                                            */
      /* - get the logical adapter name.                                     */
      /***********************************************************************/
      pConnection->ConnectType = _NETBIOS;
      len = strlen(pOption+2);
      if( len != 0 )
      {
       if( len > (MAX_LSN_NAME - LSN_RES_NAME) )
        len = MAX_LSN_NAME - LSN_RES_NAME;
       cp = Talloc(len + 1);
       strncpy( cp, pOption+2, len );
       pConnection->pLsnName = cp;
      }
      break;

     case 't':
     {
      /***********************************************************************/
      /* - connect using sockets.                                            */
      /***********************************************************************/
      pConnection->ConnectType = SOCKET;
      len = strlen(pOption+2);
      cp  = Talloc(len + 1);

      strncpy( cp, pOption+2, len );

      pConnection->pLsnName = cp;
     }
     break;

     case '+':
      pcmd->DbgFlags.Verbose = TRUE;
      break;

     case 'e':
      /***********************************************************************/
      /* - DosExecPgm() was used to start the parent. This is inherited      */
      /*   from the parent debug session.                                    */
      /***********************************************************************/
      pcmd->DbgFlags.UseExecPgm = TRUE;
      break;

    }
  }
  pOption = strtok(NULL," " );
 }
 while( pOption );

 /****************************************************************************/
 /* - After parsing the child options, if the connection type is bound       */
 /*   then we're debugging multiple processes on a single machine and        */
 /*   the connection type has to be local pipe.                              */
 /****************************************************************************/
 if(  pConnection->ConnectType == BOUND )
  pConnection->ConnectType = LOCAL_PIPE;

 return;
}
