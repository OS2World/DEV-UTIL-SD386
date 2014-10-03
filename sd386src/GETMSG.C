/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   getmsg.c                                                                */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Some basic message functions common to SD386 and ESP.                   */
/*                                                                           */
/* History:                                                                  */
/*                                                                           */
/*   11/11/93 Created.                                                       */
/*                                                                           */
/**Includes*******************************************************************/

#include "all.h"

/*****************************************************************************/
/* GetHelpMsg                                                                */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*  Get a message from the message file.                                     */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*    id         message number we want to get.                              */
/*    pIvTable   ->to table of pointers to substitution strings.             */
/*    IvCount    number of substitution strings.                             */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*    pMsgBuf    -> to the buffer that will hold the message.                */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*    Caller frees pMsgBuf.                                                  */
/*                                                                           */
/*****************************************************************************/
UCHAR *GetHelpMsg(ULONG id,UCHAR **pIvTable, ULONG IvCount)
{
 APIRET rc;
 ULONG  nbytes;
 char  *pMsgBuf;

 pMsgBuf = Talloc(MAXHELPMSG);

 rc = DosGetMessage(pIvTable,
                    IvCount,
                    pMsgBuf,
                    MAXHELPMSG,
                    id,
                    MSGFILE,
                    (ULONG*)&nbytes);
 if ( rc != 0 )
 {
  if( rc == ERROR_FILE_NOT_FOUND )
   nbytes = sprintf(pMsgBuf,"\n\nCan't find file \"%s\"\r\n", MSGFILE);
  else
   nbytes = sprintf(pMsgBuf, "\n\nDosGetMessage(rc=%d)\r\n",rc);
 }
 pMsgBuf[nbytes] = 0;

 return( pMsgBuf );
}

/*****************************************************************************/
/* SayMsg                                                                    */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Display a fatal message and then exit the debugger.                     */
/*   These messages are displayed before the debugger executes vioinit().    */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   MsgId         id of the message to display.                             */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
void SayMsg(ULONG MsgId)
{
 uchar *pMsgBuf;

 pMsgBuf = GetHelpMsg(MsgId, NULL,0);
 printf("%s",pMsgBuf);
 Tfree( pMsgBuf );
 exit(0);
}

/*****************************************************************************/
/* ErrorPrintf()                                                             */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Display a fatal message and then exit the debugger.                     */
/*                                                                           */
/*   This function allows for a substitution string and a substitution       */
/*   number to be tossed into the %1 and %2 fields of the message            */
/*   defined by the message id.                                              */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   MsgId          id of the message to display.                            */
/*   Fatal          TRUE=>kill the debugger. FALSE=>informational.           */
/*   SubStringCount String that will go into %1,%2,... substitutions.        */
/*   ...            list of ptrs to strings                                  */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   These messages are always fatal.                                        */
/*                                                                           */
/*****************************************************************************/
#include <stdarg.h>
void _System ErrorPrintf(ULONG MsgId, int SubStringCount,...)
{
 UCHAR   *SubStringTable[MAX_SUBSTRINGS];
 int      i;
 va_list  pSubString;
 UCHAR   *pMsgBuf;

 va_start(pSubString,SubStringCount);
 for( i = 0; i < SubStringCount; i++ )
  SubStringTable[i] = va_arg(pSubString,char *);

 pMsgBuf = GetHelpMsg(MsgId, SubStringTable,SubStringCount);
 printf(pMsgBuf);fflush(0);
 Tfree( pMsgBuf );
 MyExit();
}
