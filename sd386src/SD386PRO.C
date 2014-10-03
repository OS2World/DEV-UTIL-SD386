/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   sd86pro.c                                                               */
/*                                                                           */
/* Description:                                                              */
/*   Define key map for windows.                                             */
/*                                                                           */
/* History:                                                                  */
/*                                                                           */
/*   02/08/91 Creation of 32-bit SD386, from 16-bit version                  */
/*                                                                           */
/*...Release 1.00                                                            */
/*...                                                                        */
/*... 02/08/91  116   rewritten for 32 bit.                                  */
/*                                                                           */
/*...Release 1.00 (Pre-release 105 10/10/91)                                 */
/*...                                                                        */
/*... 10/31/91  308   Srinivas  Menu for exceptions.                         */
/*...                                                                        */
/*...Release 1.00 (Pre-release 108 12/05/91)                                 */
/*...                                                                        */
/*... 02/12/92  521   Srinivas  Port to C-Set/2.                             */
/*... 01/12/93  808   Selwyn    Profile fixes/improvements.                  */
/*... 01/04/94  913   Joe       Trap when reading .pro and no 0x1a.          */
/*****************************************************************************/

#define INCL_PROFILE_ARRAYS

#include "all.h"                        /* SD386 include files               */
#include "diahelp.h"
#define __MIG_LIB__                                                     /*521*/
#include <io.h>
/**Variables defined**********************************************************/

extern   uchar    VideoAtr;
extern   uchar   *VideoMap;             /* video attribute map               */
extern   char    *vaTypes[];            /* video attribute types             */
extern   char    *FGColors[];           /* foreground color names            */
extern   char    *BGColors[];           /* background color names            */


extern   uchar    ExceptionMap[MAXEXCEPTIONS];
extern   char    *ExcepTypes[MAXEXCEPTIONS];
extern   char    *ExcepSel[];           /* notification names                */
extern   uchar   ClearField[];          /* clear field attributes         701*/
extern   uchar   hilite[];              /* high light field attributes    701*/
extern   uchar   normal[];              /* normal field attributes        701*/

extern   CmdParms cmd;
static   int   index[USERKEYS][2];

typedef  struct _keyshelp
{
  int   key;
  ULONG helpid;
} KEYSHELP;

#define NUMKEYSHELP 4

static KEYSHELP KeysHelp[] =
{
  { F10,  HELP_ABAR_ABAR },
  { ESC,  HELP_ABAR_ABAR },
  { C_F2, HELP_PULL_DATASHOW },
  { S_F9, HELP_PULL_DATABREAK }
};

#define READERROR      1
#define VERSIONERROR   2
#define DEFSECERROR    3
#define NOKEYERROR     4
#define NOFUNCERROR    5
#define NOATTRERROR    6
#define NOBGCLRERROR   7
#define NOFGCLRERROR   8
#define NOEXTYPERROR   9
#define NOEXCEPERROR   0
#define NESTEDCMT     10

/*****************************************************************************/
/* Profile                                                                   */
/*                                                                           */
/* Description:                                                              */
/*   See if the sd386.pro profile exists and alter scan code to function     */
/*   table accordingly.                                                      */
/*   Set user-defined color values to logical attributes.                    */
/*                                                                           */
/* Parameters:                                                               */
/*   defk2f     scan code to function table (similar to KEY2FUNC).           */
/*                                                                           */
/* Return:                                                                   */
/*   char *     Error message to be displayed.                               */
/*                                                                           */
/*****************************************************************************/
void  Profile( KEY2FUNC  defk2f[])
                                        /* scan code to function table       */
{                                       /* begin Profile                     */
  int         fh;                       /* pointer to profile file        808*/
  uint        bg, fg;                   /* bg and fg color indices           */
  uint        f, k, n, kf;              /* indices                           */
  uchar      *fn;                       /* fully qualified file name         */
  int         ErrorCode;                /* return code.                      */
  long        FileSize;                 /* Total file size of .pro        808*/
  long        LineNum;
  char        LineNumStr[10];
  long        CurrentChar;              /* Current character pos in file  808*/
  char        *FileBuffer, *BufferStart;/* Buffer to read the file.       808*/
  char        *newtext;                 /* string to look for either:        */
                                        /*   1) "Start_Of_Defs" indicator,   */
                                        /*   2) profile key,                 */
                                        /*   3) function definition,         */
                                        /*   4) "Start_Of_Colors" indicator, */
                                        /*   5) attribute name,              */
                                        /*   6) background color name, or    */
                                        /*   7) foreground color name,       */
/*****************************************************************************/
/* We want to find SD386.PRO according to a specified path search.           */
/*                                                                           */
/*****************************************************************************/
  fn=Talloc(MAXFNAMELEN);               /* allocate space for a fully     521*/
                                        /* qualified filename.               */
  findpro("SD386.PRO",fn,MAXFNAMELEN);  /* get fully qualified profile nam116*/
/*****************************************************************************/
/*                                                                           */
/* First we must see if the profile exists.  If it does not, the defaults    */
/* are as you see them in defk2f.  If it does, we shall read in a character  */
/* string key and convert that to a scan code.  We will then read in a       */
/* character string function definition, and convert that to a function code.*/
/* Then we will search defk2f for the scan code and replace that entry's     */
/* function code with the one read in.  We will do this for as many key code */
/* and function code definitions that exist realizing that comments and other*/
/* information character strings exist in the file.                          */
/*                                                                           */
/*****************************************************************************/
  fh = open( fn, 0x00000004 | 0x00008000, NULL );                       /*808*/
  if(fn) Tfree(fn);                                                      /*813*/
  if( fh == -1 )                        /* troubles opening it ?             */
    return;                             /* we are done; later, you may want  */
                                        /*   to notify the user              */
  FileSize = filelength( fh );                                          /*808*/

  FileBuffer = (char *)Talloc( FileSize + 1 );                          /*808*/
  BufferStart = FileBuffer;                                             /*808*/
  memset( FileBuffer, '\0', FileSize + 1 );                             /*808*/

  if( read( fh, FileBuffer, FileSize ) == -1 )                          /*808*/
  {                                                                     /*808*/
    ErrorCode = READERROR;                                              /*808*/
    goto error;                                                         /*808*/
  }                                                                     /*808*/

/*****************************************************************************/
/*                                                                           */
/* Here we search for the title area.  This allows the user to format his    */
/* informational front matter in any manner that he wants.                   */
/*                                                                           */
/*****************************************************************************/

  newtext = strtok( FileBuffer, "\n" );                                 /*808*/
  if( strncmp( newtext, "Version 2.00", 12 ) )                          /*808*/
  {                                                                     /*808*/
    ErrorCode = VERSIONERROR;                                           /*808*/
    goto error;                                                         /*808*/
  }                                                                     /*808*/

  newtext = strtok( NULL, " \n" );                                      /*808*/
  while( newtext )                                                      /*808*/
  {
    if ( !strncmp( newtext, "Start_Of_Defs", 13 ) )
      break;                            /* we are done searching             */
   newtext = strtok( NULL, " \n" );                                     /*808*/
  }

  if( !newtext )                                                        /*808*/
  {                                                                     /*808*/
    ErrorCode = DEFSECERROR;                                            /*808*/
    goto error;                                                         /*808*/
  }                                                                     /*808*/

/*****************************************************************************/
/*                                                                           */
/* Now we can process the key to function code area.  We next try to find a  */
/* key definition in the file and see if it is the key2code table.           */
/*                                                                           */
/*****************************************************************************/

keydef:
  while( (newtext = strtok( NULL, " \n" )) &&                           /*808*/
         strnicmp(newtext, "Start_Of_Colors", 15 ))                     /*808*/
  {                                     /* process key/func definitions      */
    if( *newtext == 0x0d )              /* skip blank lines.              808*/
      goto keydef;                                                      /*808*/

    if( *newtext == 0x1a )              /* if end of file break.          808*/
      break;                                                            /*808*/

    if( newtext[0] == '?' )             /* If you get a '?' as the first  701*/
    {                                   /* character, key is not defined  701*/
      newtext = strtok( NULL, " \n" );  /* for the function, so skip the  808*/
      if( newtext )                     /* full line.                     701*/
        goto keydef;                                                    /*701*/
    }                                                                   /*701*/

    if ( newtext[0] == '/' &&           /* did we find the start of a        */
         newtext[1] == '*'    )         /*   comment ?                       */
    {                                   /* begin processing comment          */
      while( !strstr( newtext, "*/" ) && /* not found comment end yet and we */
             newtext  )                   /*   still have room to read ?     */
      {                                                                 /*808*/
        newtext = strtok( NULL, " \n" );  /* read in potential end of cmt 808*/
        if( strstr( newtext, "/*" ) )                                   /*808*/
        {                                                               /*808*/
          ErrorCode = NESTEDCMT;                                        /*808*/
          goto error;                                                   /*808*/
        }                                                               /*808*/
      }
      goto keydef;                      /* try to get a key definition       */
    }                                   /* end processing comment            */

/*****************************************************************************/
/*                                                                           */
/* Now we see if the key definition matches any of our key names in the      */
/* key2code table.  If it does, we know the scan code for the definition.    */
/*                                                                           */
/*****************************************************************************/

    for(
         k = 0;                         /* will start w/1st key2code entry   */
         k < USERKEYS;                  /* stay within key2code         /*808*/
         k++                            /* bump to next key2code entry       */
       )
    {
      int CompareLen = strlen( newtext );                               /*808*/

      if( newtext[CompareLen - 1] == 0xd )                              /*808*/
        CompareLen--;                                                   /*808*/
      if( !strnicmp( newtext, key2code[k].key, CompareLen ) )           /*808*/
        break;                          /* then we are done looking          */
    }

      if ( k == USERKEYS )              /* did we not find this key def   808*/
      {                                 /* this is a problem!                */
        ErrorCode = NOKEYERROR;                                         /*808*/
        goto error;                                                     /*808*/
      }                                 /* end this is a problem             */

/*****************************************************************************/
/*                                                                           */
/* Now we try to find a function definition to pair up with our key scan     */
/* code.  We need to ensure that this function definition is legit, too.     */
/*                                                                           */
/*****************************************************************************/

funcdef:
    newtext = strtok( NULL, " \n" );    /* read in potential func def.    808*/
    if ( newtext[0] == '/' &&           /* did we find the start of a        */
         newtext[1] == '*'    )         /*   comment ?                       */
    {                                   /* begin processing comment          */
      while( !strstr( newtext, "*/" ) && /* not found comment end yet and we */
             newtext )                     /*   still have room to read ?    */
      {                                                                 /*808*/
        newtext = strtok( NULL, " \n" );   /* read in potential cmt end   808*/
        if( strstr( newtext, "/*" ) )                                   /*808*/
        {                                                               /*808*/
          ErrorCode = NESTEDCMT;                                        /*808*/
          goto error;                                                   /*808*/
        }                                                               /*808*/
      }

      if ( !newtext )                   /* still have room to read ?    /*808*/
        goto funcdef;                   /* try to get a func definition      */
    }                                   /* end processing comment            */

/*****************************************************************************/
/*                                                                           */
/* Now we see if the function definition mathces any of our names in the     */
/* func2code table.  If it does, we know the function code for the           */
/* definition.                                                               */
/*                                                                           */
/*****************************************************************************/

    for(
         f = 0;                         /* will start w/1st func2code entry  */
         f < USERFUNCS;                 /* stay within func2code          808*/
         f++                            /* bump to nest func2code entry      */
       )
    {
      int CompareLen = strlen( newtext );                               /*808*/

      if( newtext[CompareLen - 1] == 0xd )                              /*808*/
        CompareLen--;                                                   /*808*/
      if ( !strnicmp( newtext, func2code[f].func, CompareLen ))         /*808*/
        break;                          /* then we are done looking          */
    }

      if ( f == USERFUNCS )             /* did we not find this func def  808*/
      {                                 /* this is a problem!                */
        ErrorCode = NOFUNCERROR;                                        /*808*/
        goto error;                                                     /*808*/
      }                                 /* end this is a problem             */

/*****************************************************************************/
/*                                                                           */
/* At this point we have a scan code and a function code that the user wants */
/* associated.  We look within defk2f for the scan code, and, if found, we   */
/* replace the defk2f function code with the new one.                        */
/*                                                                           */
/*****************************************************************************/

    for(
         kf = 0;                        /* will start w/1st defk2f entry     */
         kf < KEYNUMSOC;                /* stay within defk2f                */
         kf++                           /* bump to next defk2f entry         */
       )
      if ( defk2f[kf].scode == key2code[k].scode ) /* find scan code match ? */
        break;                          /* then we are done looking          */

      if ( kf == KEYNUMSOC )            /* did we not find the scan code ?   */
      {                                 /* this is a problem!                */
        ErrorCode = NOKEYERROR;                                         /*808*/
        goto error;                                                     /*808*/
      }                                 /* end this is a problem             */
      defk2f[kf].fcode = func2code[f].fcode; /* put in the func def code     */
  }                                     /* end process key/func definitions  */


/*****************************************************************************/
/*                                                                           */
/* Now we can process the attribute to colors area.  We next try to find an  */
/* attribute definition in the file and see if it is in the vaTypes table.   */
/*                                                                           */
/*****************************************************************************/

 if( !strnicmp(newtext, "Start_Of_Colors", 15 ) )
colordef:                                                               /*308*/
  while( (newtext = strtok( NULL, " \n" )) &&                           /*808*/
         strnicmp(newtext, "Start_Of_Exceptions", 19 ))
                                        /* and not yet Start_Of_Exceptions308*/
  {                                     /* process key/func definitions      */
    if( *newtext == 0x0d )              /* skip blank lines.              808*/
      goto colordef;                                                    /*808*/

    if( *newtext == 0x1a )              /* if end of file break.          808*/
      break;                                                            /*808*/

    if ( newtext[0] == '/' &&           /* did we find the start of a        */
         newtext[1] == '*'    )         /*   comment ?                       */
    {                                   /* begin processing comment          */
      while( !strstr( newtext, "*/" ) && /* not found comment end yet and we */
             newtext  )                   /*   still have room to read ?     */
      {                                                                 /*808*/
        newtext = strtok( NULL, " \n" );  /* read in potential end of cmt 808*/
        if( strstr( newtext, "/*" ) )                                   /*808*/
        {                                                               /*808*/
          ErrorCode = NESTEDCMT;                                        /*808*/
          goto error;                                                   /*808*/
        }                                                               /*808*/
      }
      goto colordef;                    /* try to get a key definition       */
    }                                   /* end processing comment            */

/*****************************************************************************/
/*                                                                           */
/* Now we see if the attribute matches any of our attribute names in the     */
/* vaTypes table.  If it does, we know the index for the attribute.          */
/*                                                                           */
/*****************************************************************************/

   for(
        k = 1;                          /* will start w/1st vaTypes entry    */
        k <= MAXVIDEOATR &&             /* stay within vaTypes, and          */
        stricmp(newtext, vaTypes[k]);
        k++                             /* bump to next vaTypes entry        */
      ){;}

   if ( k > MAXVIDEOATR )               /* did we not find this attr def?    */
   {                                    /* this is a problem!                */
     ErrorCode = NOATTRERROR;                                           /*808*/
     goto error;                                                        /*808*/
   }                                    /* end this is a problem             */

/*****************************************************************************/
/*                                                                           */
/* Now we try to find a background color to match up with our attribute      */
/* index.  We need to ensure that this background color is legit, too.       */
/*                                                                           */
/*****************************************************************************/

  while( (newtext = strtok( NULL, " \n" )) &&                           /*808*/
      newtext[0] == '/' &&              /* did we find the start of a        */
      newtext[1] == '*'    )            /*   comment ?                       */
    while( !strstr( newtext, "*/" ) &&  /* while not found end of comment    */
            newtext )                                                   /*808*/
      {                                                                 /*808*/
        newtext = strtok( NULL, " \n" );                                /*808*/
        if( strstr( newtext, "/*" ) )                                   /*808*/
        {                                                               /*808*/
          ErrorCode = NESTEDCMT;                                        /*808*/
          goto error;                                                   /*808*/
        }                                                               /*808*/
      }

/*****************************************************************************/
/*                                                                           */
/* Now we see if the bg color matches any of our bg color names in the       */
/* BGColors table.  If it does, we know the index for the bg color.          */
/*                                                                           */
/*****************************************************************************/

   for(
        bg = 0;                         /* start w/1st BGColors entry        */
        bg < MAXBGCOLOR &&              /* stay within BGColors, and         */
        strnicmp(newtext, BGColors[bg], strlen( BGColors[bg] ));
        bg++                            /* bump to next BGColors entry       */
      ){;}

   if ( bg == MAXBGCOLOR )              /* did we not find this bg color?    */
   {                                    /* this is a problem!                */
     ErrorCode = NOBGCLRERROR;                                          /*808*/
     goto error;                                                        /*808*/
   }                                    /* end this is a problem             */

/*****************************************************************************/
/*                                                                           */
/* Now we try to find a foreground color to match up with our attribute      */
/* index.  We need to ensure that this foreground color is legit, too.       */
/*                                                                           */
/*****************************************************************************/

  while( (newtext = strtok( NULL, " \n" )) &&                           /*808*/
      newtext[0] == '/' &&              /* did we find the start of a        */
      newtext[1] == '*'    )            /*   comment ?                       */
    while( !strstr( newtext, "*/" ) &&  /* while not found end of comment    */
            newtext )                                                   /*808*/
      {                                                                 /*808*/
        newtext = strtok( NULL, " \n" );                                /*808*/
        if( strstr( newtext, "/*" ) )                                   /*808*/
        {                                                               /*808*/
          ErrorCode = NESTEDCMT;                                        /*808*/
          goto error;                                                   /*808*/
        }                                                               /*808*/
      }

/*****************************************************************************/
/*                                                                           */
/* Now we see if the fg color matches any of our fg color names in the       */
/* FGColors table.  If it does, we know the index for the fg color.          */
/*                                                                           */
/*****************************************************************************/

   for(
        fg = 0;                         /* start w/1st FGColors entry        */
        fg < MAXFGCOLOR &&              /* stay within FGColors, and         */
        strnicmp(newtext, FGColors[fg], strlen( FGColors[fg] ));
        fg++                            /* bump to next FGColors entry       */
      ){;}

   if ( fg == MAXFGCOLOR )              /* did we not find this fg color?    */
   {                                    /* this is a problem!                */
     ErrorCode = NOFGCLRERROR;                                          /*808*/
     goto error;                                                        /*808*/
   }                                    /* end this is a problem             */

/*****************************************************************************/
/*                                                                           */
/* At this point we have attribute, background, and foreground indices that  */
/* user wants associated.  We now assign the background and foreground       */
/* colors to the attribute indexed into the VideoMap table.                  */
/*                                                                           */
/*****************************************************************************/

   VideoMap[k] = ( uchar )(( bg << 4 ) + fg);/* Assign clrs to attribute     */
  }                                     /* end process attr/color defs       */

/*****************************************************************************/
/* Now we can process the notifications of the exceptions                 308*/
/*****************************************************************************/

 if( !strnicmp(newtext, "Start_Of_Exceptions", 19 ))
  for ( ; ; )                           /* Loop until end of file            */
  {                                     /* process attr/color definitions    */
   while( (newtext = strtok( NULL, " \n" ) ) &&                         /*808*/
      newtext[0] == '/' &&              /* did we find the start of a        */
      newtext[1] == '*'    )            /*   comment ?                       */
    while( !strstr( newtext, "*/" ) &&  /* while not found end of comment    */
            newtext )                                                   /*808*/
      {                                                                 /*808*/
        newtext = strtok( NULL, " \n" );                                /*808*/
        if( strstr( newtext, "/*" ) )                                   /*808*/
        {                                                               /*808*/
          ErrorCode = NESTEDCMT;                                        /*808*/
          goto error;                                                   /*808*/
        }                                                               /*808*/
      }

    if( newtext == NULL )                                               /*913*/
     break;                                                             /*913*/

    if( *newtext == 0x1a )              /* if end of file break.          808*/
      break;                                                            /*808*/

    if( *newtext == 0x0d )              /* skip blank lines.            /*808*/
      continue;

   if( !newtext )                       /* if no more attributes to read  808*/
    break;                              /* exit loop and close file          */

/*****************************************************************************/
/*                                                                           */
/* Now we see if the exception type matches any of our exception types in the*/
/* Excepypes table.  If it does, we know the index for the ExceptionType.    */
/*                                                                           */
/*****************************************************************************/

   for(
        k = 0;                          /* will start w/1st vaTypes entry    */
        k <  MAXEXCEPTIONS &&           /* stay within vaTypes, and          */
        strnicmp(newtext, ExcepTypes[k], strlen( ExcepTypes[k] ));
        k++                             /* bump to next vaTypes entry        */
      ){;}

   if ( k >= MAXEXCEPTIONS )            /* did we not find this attr def?    */
   {                                    /* this is a problem!                */
     ErrorCode = NOEXTYPERROR;                                          /*808*/
     goto error;                                                        /*808*/
   }                                    /* end this is a problem             */

/*****************************************************************************/
/*                                                                           */
/* Now we try to find notification choice for the exception type.            */
/* We need to ensure that notification choice is legal too.                  */
/*                                                                           */
/*****************************************************************************/

  while( (newtext = strtok( NULL, " \n" ) ) &&                          /*808*/
      newtext[0] == '/' &&              /* did we find the start of a        */
      newtext[1] == '*'    )            /*   comment ?                       */
    while( !strstr( newtext, "*/" ) &&  /* while not found end of comment    */
            newtext )                                                   /*808*/
      {                                                                 /*808*/
        newtext = strtok( NULL, " \n" );                                /*808*/
        if( strstr( newtext, "/*" ) )                                   /*808*/
        {                                                               /*808*/
          ErrorCode = NESTEDCMT;                                        /*808*/
          goto error;                                                   /*808*/
        }                                                               /*808*/
      }

/*****************************************************************************/
/*                                                                           */
/* Now verify that the notification is in the ExcepSel table. If it does     */
/* we know the index for it.                                                 */
/*                                                                           */
/*****************************************************************************/

   for(
        n  = 0;                         /* start w/1st entry                 */
        n < 2 &&                        /* stay within bounds, and           */
        strnicmp(newtext, ExcepSel[n], strlen( ExcepSel[n] ));
        n++                             /* bump to next entry                */
      ){;}

   if ( n == 2 )                        /* did we not find this entry        */
   {                                    /* this is a problem!                */
     ErrorCode = NOEXCEPERROR;                                          /*808*/
     goto error;                                                        /*808*/
   }                                    /* end this is a problem             */

/*****************************************************************************/
/*                                                                           */
/* At this point we have exception type and its notification choice indices  */
/* that user wants associated.  We now assign the notification choice into   */
/* the ExceptionMap table.                                                   */
/*                                                                           */
/*****************************************************************************/

   ExceptionMap[k] = n;                 /* Assign the notification choice    */
  }                                     /* end exception notifications       */
  close( fh );                          /* proper handshaking with close  808*/
  Tfree( BufferStart );                                                  /*808*/
  xSetExceptions(ExceptionMap,sizeof(ExceptionMap) );
  return;

error:                                                                  /*808*/
  {                                                                     /*808*/
    char *BufferScan = BufferStart;                                     /*808*/
    long  i;                                                            /*808*/

    CurrentChar = newtext - BufferStart;                                /*808*/
    for( LineNum = 1, i = 0; i < CurrentChar; i++ )                     /*808*/
      if( BufferScan[i] == 0xd )                                        /*808*/
        LineNum++;                                                      /*808*/

    /*************************************************************************/
    /* At this point, newtext may contain a 0x0D at the end. It shouldn't... */
    /* but it might and rather than go back and try to fix the logic         */
    /* flaw, we'll just filter the damn thing out. Thank you...Selwyn.       */
    /*************************************************************************/
    {
     char *cp = strchr(newtext,0x0D );

     if( cp ) *cp = ' ';
    }
    sprintf(LineNumStr,"%d",LineNum);
    close( fh );
    Tfree( BufferStart );
    switch( ErrorCode )                                                 /*808*/
    {                                                                   /*808*/
      case READERROR:                                                   /*808*/
        Error(ERR_PROFILE_READERROR,TRUE,0);
        break;                                                          /*808*/

      case VERSIONERROR:                                                /*808*/
        Error(ERR_PROFILE_VERSION,TRUE,0);
        break;                                                          /*808*/

      case DEFSECERROR:                                                 /*808*/
        Error(ERR_PROFILE_KEY_DEFS,TRUE,0);
        break;                                                          /*808*/

      case NOFUNCERROR:                                                 /*808*/
        Error(ERR_PROFILE_INVALID_FUNCTION,TRUE,2,newtext,LineNumStr);
        break;

      case NOATTRERROR:                                                 /*808*/
        Error(ERR_PROFILE_INVALID_ATTRIBUTE,TRUE,2,newtext,LineNumStr);
        break;                                                          /*808*/

      case NOBGCLRERROR:                                                /*808*/
        Error(ERR_PROFILE_BG_COLOR,TRUE,2,newtext,LineNumStr);
        break;                                                          /*808*/

      case NOFGCLRERROR:                                                /*808*/
        Error(ERR_PROFILE_FG_COLOR,TRUE,2,newtext,LineNumStr);
        break;                                                          /*808*/

      case NOEXTYPERROR:                                                /*808*/
        Error(ERR_PROFILE_EXCEPTION_TYPE,TRUE,2,newtext,LineNumStr);
        break;                                                          /*808*/

      case NOEXCEPERROR:                                                /*808*/
        Error(ERR_PROFILE_EXCEPTION,TRUE,2,newtext,LineNumStr);
        break;                                                          /*808*/

      case NOKEYERROR:                                                  /*808*/
        Error(ERR_PROFILE_INVALID_KEY,TRUE,2,newtext,LineNumStr);
        break;                                                          /*808*/

      case NESTEDCMT:                                                   /*808*/
        Error(ERR_PROFILE_NESTED_COMMENT,TRUE,1,LineNumStr);
        break;                                                          /*808*/

      default:                                                          /*808*/
        Error(ERR_PROFILE_GENERAL,TRUE,NULL,0);
        break;                                                          /*808*/
    }                                                                   /*808*/
  }
}                                       /* end Profile                       */

/*****************************************************************************/
/* HelpScreen                                                                */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Build and display a scrollable help panel box.  Scroll is accomplished  */
/*   by using UPCURSOR, DOWNCURSOR, PGDOWN, and PGUP.  ACTIONBAR returns you */
/*   to the caller.                                                          */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*****************************************************************************/
void HelpScreen( void )
{
 DisplayDialog( &Dia_Help, TRUE );
 ProcessDialog( &Dia_Help, &Dia_Help_Choices, TRUE, NULL );
 RemoveDialog( &Dia_Help );
}

/*****************************************************************************/
/* DisplayHelpChoice()                                                    701*/
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Displays the help information for the key assignments.                  */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   shell   ->  pointer to a dialog shell structure.                        */
/*   ptr     ->  pointer to a dialog choice structure.                       */
/*                                                                           */
/* Return:                                                                   */
/*          None                                                             */
/*                                                                           */
/*****************************************************************************/
#define KEYFIELDSIZE 15
void  DisplayHelpChoice( DIALOGSHELL *shell, DIALOGCHOICE *ptr )
{
  int idx, i;

  idx = ptr->SkipRows;
  for( i = 0; i < ptr->MaxRows; i++, idx++ )
  {
    int StrLength, k, f;

    k = index[idx][0];
    f = index[idx][1];

    ClearField[1] = KEYFIELDSIZE;
    putrc( shell->row + i + shell->SkipLines, shell->col + 1, ClearField );
    putrc( shell->row + i + shell->SkipLines, shell->col + 2, key2code[k].key );

    StrLength = strlen( func2code[f].func );
    ClearField[1] = shell->width - 6 - StrLength;
    putrc( shell->row + i + shell->SkipLines, shell->col + 2 + StrLength,
           ClearField );
    putrc( shell->row + i + shell->SkipLines, shell->col + 2 + KEYFIELDSIZE,
           func2code[f].func );
  }
}

/*****************************************************************************/
/*                                                                           */
/* HelpDialogFunction()                                                      */
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
uint  HelpDialogFunction( DIALOGSHELL *shell, DIALOGCHOICE *ptr,
                          EVENT *nEvent, void *ParamBlock )
{
  switch( nEvent->Value )
  {
    case INIT_DIALOG:
    {
      int   idx, k, f, kf;

      for( idx = 0, k = 0; k < USERKEYS; k++ )
      {
        for( kf = 0; kf < KEYNUMSOC; kf++ )
        {
          if( defk2f[kf].scode == key2code[k].scode )
            break;
        }
        if( defk2f[kf].fcode == DONOTHING )
         continue;

        for( f = 0; f < USERFUNCS; f++ )
        {
          if( defk2f[kf].fcode == func2code[f].fcode )
            break;
        }
        index[idx][0] = k;
        index[idx][1] = f;
        idx++;
      }
      ptr->entries = idx;
      hilite[1] = normal[1] = KEYFIELDSIZE;
      return( 0 );
    }

    case ESC:
      return( ESC );

    case ENTER:
    {
      int  i;

      for( i = 0; i < 9; i++ )
      {
        PULLDOWN  *PullPtr;
        uchar     *FuncCode;
        int        j;

        PullPtr  = GetPullPointer( i );
        FuncCode = PullPtr->funccodes;
        for( j = 0; j < PullPtr->entries; j++ )
        {
          if( func2code[index[ptr->SkipRows + shell->CurrentField - 1][1]].fcode
              == *(FuncCode + j) )
          {
            uchar *HelpMsg;
            ULONG  HelpId;

            HelpId  = *(PullPtr->help + j);
            HelpMsg = GetHelpMsg( HelpId, NULL,0 );
            CuaShowHelpBox( HelpMsg );
            return( 0 );
          }
        }
      }

      for( i = 0; i < NUMKEYSHELP; i++ )
      {
        if( key2code[index[ptr->SkipRows + shell->CurrentField - 1][0]].scode
            == KeysHelp[i].key )
        {
          uchar *HelpMsg;
          ULONG  HelpId;

          HelpId  = KeysHelp[i].helpid;
          HelpMsg = GetHelpMsg( HelpId, NULL,0);
          CuaShowHelpBox( HelpMsg );
          return( 0 );
        }
      }
      beep();
      return( 0 );
    }

    case F1:
    {
      uchar *HelpMsg;

      HelpMsg = GetHelpMsg( HELP_DLG_KEYSHELP, NULL,0 );
      CuaShowHelpBox( HelpMsg );
      return( 0 );
    }

    default:
      return( 0 );
  }
}
