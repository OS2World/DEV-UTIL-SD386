/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   actbar.c                                                                */
/*                                                                           */
/* Description:                                                              */
/*  Source and Assembler action bar handling.                                */
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
/*... 02/08/91  105   Christina port to 32 bit.                              */
/*... 02/08/91  106   Srinivas  port to 32 bit.                              */
/*... 02/08/91  110   Srinivas  port to 32 bit.                              */
/*... 02/08/91  111   Christina port to 32 bit.                              */
/*... 02/08/91  112   Joe       port to 32 bit.                              */
/*                                                                           */
/*...Release 1.00 (Pre-release 1)                                            */
/*...                                                                        */
/*... 07/09/91  204   srinivas  Hooking up of defered break points.          */
/*... 08/19/91  229   srinivas  ESC key should take back to action bar from  */
/*                              showthds and editregs.                       */
/*... 08/19/91  231   srinivas  GETFUNC, GETFILE, BRKPT from the action bar  */
/*                              need not remember the previous values.       */
/*... 08/22/91  234   Joe       PL/X gives "varname" is incorrect message    */
/*...                           when entering a parameter name in the data   */
/*...                           window.  This happens when the cursor is on  */
/*...                           an internal procedure definition statement   */
/*...                           and you use F2 to get into the data window   */
/*...                           and then type the name.                      */
/*                                                                           */
/*...Release 1.00 (Pre-release 107 11/13/91)                                 */
/*...                                                                        */
/*... 11/13/91  400   Srinivas  Vertical Register Display.                   */
/*...                                                                        */
/*... 11/18/91  401   Srinivas  Floating point Register Display.             */
/*...                                                                        */
/*...Release 1.00 (Pre-release 108 12/05/91)                                 */
/*...                                                                        */
/*... 02/10/92  517   Srinivas  Cusrsor sensitive prompting for GetFunc.     */
/*... 02/11/92  518   Srinivas  Remove limitation on max no of screen rows.  */
/*... 02/12/92  521   Srinivas  Port to C-Set/2.                             */
/*...                                                                        */
/*...Release 1.01 (07/10/92)                                                 */
/*...                                                                        */
/*... 05/08/92  701   Joe       Cua Interface.                               */
/*...                                                                        */
/**Includes*******************************************************************/

#define INCL_16                         /* 16-bit API                     101*/
#include "all.h"                        /* SD86 include files                */
/* #define _MT             */                                           /*111*/
#include <process.h>                                                    /*111*/

/**Macros*********************************************************************/

#define NODROPOPT 0x08                  /* mask 2**2 (dropfile)              */
#define NOPROCESS 0x40                  /* mask for  (process )           106*/
#define NOSELECT   0                    /* no menu selection.                */
#define GETFUNC    1                    /* get a function.                   */
#define GETFILEx   2                    /* get a file.                       */
#define BKPT       3                    /* set immediate/deferred bkpt.      */
#define DROPFILEx  4                    /* drop a file from ring.            */
#define FINDSTRING 5                    /* find/repeat find a string.        */
#define EDITREGS   5                    /* edit registers in asm menu.       */
#define THREADS    6                    /* restart the application.          */
#define PROCESS    7                    /* restart the application.          */
#define RESTARTx   8                    /* restart the application.          */
#define QUITx      9                    /* quit SD86.                        */

/**Externs********************************************************************/

extern AFILE*        allfps;            /* -> to afile ring.                 */
extern uint          LinesPer;          /* currnt # lines/screen for Code.   */
extern uint          VideoCols;         /* # of columns per screen           */
extern uint          VideoRows;         /* # of rows per screen           518*/
/*****************************************************************************/
/* These externs are instanced for each process being debugged.              */
/*****************************************************************************/
extern char  GetFuncBuffer[PROMAX+1];   /* save buffer for GetFunc.          */
extern char  GetFileBuffer[PROMAX+1];   /* save buffer for GetFile.          */
extern char  BkptBuffer[PROMAX+1];      /* save buffer for Bkpt.             */
extern char  FindStringBuffer[PROMAX+1];/* save buffer for Find.             */
extern uchar Reg_Display;               /* Register display flag          400*/
extern uchar *BoundPtr;                 /* -> to screen bounds            518*/

/*****************************************************************************/
/* scan function                                                             */
/*****************************************************************************/
uint slen;                              /* length of search string        110*/
extern uint str_fnd_flag;               /* flag to signal reverse video   110*/

/* scan(AFILE *fp,uchar *str,uchar *buf) */
int scan(AFILE *fp,uchar *str)          /*                                701*/
{
    int togo;
    uint hit = 0;
    uint orglno = fp->csrline + fp->Nbias;
    uint n = fp->csrline;               /* was register. 112              110*/
    uint runlno = orglno;               /* was register. 112              110*/
    uchar *cp, c1st = str[0];
    uint adjust = fp->skipcols + fp->csr.col + 1;
    ushort *offtab = fp->offtab;           /* 2-byte table offsets        100*/
    uint xlc = 0, pass1 = 1;
    uchar pattern[ PROMAX+1 ];

 uchar            buf[ MAXCOLS+1 ];     /* buffer for source line decode.    */
  memset(    buf, 0, sizeof(   buf) );  /* clear the prompt buffer.       100*/


    str_fnd_flag = 0;                   /* reset the string found flag.   110*/

    /* If str is all lowercase, Then do case insensative match */
    if( (slen = strlen(str)) < sizeof(pattern) ){
        LowerCase(str, pattern);
        pattern[slen] = 00;                /* null-terminator             100*/
        xlc = !strcmp(str, pattern);
        if ( xlc ) {                       /* if all lowercase,           100*/
            c1st = *( str = pattern );
    }   }
    for(;;)
    {
        togo = (Decode(fp->source + offtab[n], buf))
             - slen + 1 - adjust;
        if( xlc )
            LowerCase( buf, buf );
     for( cp = buf + adjust,
          adjust = 0 ;
          togo > 0 ;
          ++cp,
          togo -= hit+1
        )
     {
      hit = bindex(cp, togo, c1st);
      cp += hit;
      if( (hit < (uint)togo) && !strncmp(cp, str, slen) )/*if match found,100*/
      {
          fp->csrline = n;
          fp->skipcols = ((n = cp - buf) / VideoCols) * VideoCols;
          fp->csr.col = ( uchar)(n % VideoCols);
          str_fnd_flag = 1;             /* set the flag to procede with   110*/
          return(1);                    /* highlighting the string        110*/
      }
     }

     n++;                                                               /*110*/

     if( (++runlno == orglno) && !pass1)
     {                                  /* if line no on which search     110*/
                                        /* started and current line no is 110*/
                                        /* same return with failure       110*/
         fp->csrline = n;                                               /*110*/
         return(0);
      }

     if( runlno > fp->Tlines )          /*  if the lineno goes beyond the 234*/
     {                                  /*  total lines fetch the top     110*/
         n = runlno = 1;                /*  portion of the source and     234*/
         pass1 = 0;                     /*  continue search.              110*/
         if (fp->pdf != NULL)
             pagefp(fp,1);              /* if not called from browse func 110*/
         else
            scrollfile(fp,1);           /* if called from browse function 110*/
         offtab = fp->offtab;           /* reinitiliase the offtab pointe 110*/
         /* Wrap Around */
                                        /*  infomration message           110*/
         beep();                        /*  issue a beep sound.           110*/
     }

     if( (runlno==1 && orglno==1) && !pass1 )
     {                                  /* if line no on which serach     234*/
                                        /* started is 1 and we have wrap- 110*/
                                        /* ed around return with failure  110*/

         return(0);
     }

     if( n > fp->Nlines )               /* if lineno exceeds the number   234*/
     {                                  /* of lines in the buffer, fetch  110*/
                                        /* the next portion of the source 110*/
        if (fp->pdf != NULL)
          pagefp(fp,n + fp->Nbias);     /* if not called from browse func 234*/
        else
          scrollfile(fp,n + fp->Nbias); /* if called from browse function 234*/
        offtab = fp->offtab;            /* reinitiliase the offtab pointe 110*/
        n = runlno - fp->Nbias;         /* set the line no from where to  110*/
                                        /* continue search                110*/
     }

    }
}
