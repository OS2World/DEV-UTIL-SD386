/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   findpro.c                                                               */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*  Find SD386.PRO.                                                          */
/*                                                                           */
/*                                                                           */
/* History:                                                                  */
/*                                                                           */
/*   02/08/91 Creation of 32-bit SD386, from 16-bit version.                 */
/*                                                                           */
/**Includes*******************************************************************/

#include "all.h"                        /* SD386 include files               */

/*****************************************************************************/
/* findpro()                                                                 */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   find SD386.PRO.                                                         */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   fs        input - the passed filespec.                                  */
/*   pn        input - -> to callers "fully qualified filespec" buffer.      */
/*   pnlen     input - length of the caller's buffer.                        */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   void                                                                    */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   none.                                                                   */
/*                                                                           */
/*****************************************************************************/
 void
findpro(char *fs,char *pn,uint pnlen)   /* fs :: the passed filespec         */
                                        /* pn :: the caller's buffer         */
                                        /* pnlen :: caller's buffer length   */
{                                       /*                                   */
 char       *fn;                        /* profile file name.                */
 USHORT     control;                    /* control word for DosSearchPath    */
 uint       rc;                         /* DosSearchPath return code         */
                                        /*                                   */
/*****************************************************************************/
/* First, look in the current directory.                                     */
/*****************************************************************************/
 fn = fs;                               /*                                   */
 control = 0;                           /* no impied current directory       */
 rc= DosSearchPath(control,".\\",fn,pn,pnlen); /* look for the file          */
 if ( rc==0 )                           /* if we found it then               */
  return;                               /* return.                           */
/*****************************************************************************/
/* Now look along the DPATH environment variable.                            */
/*****************************************************************************/
 control = 2;                          /* use path reference as env variable */
 rc=DosSearchPath(control,"DPATH",fn,pn,pnlen);    /* look for it            */
 if( rc==0)                             /*                                   */
  return;                               /*                                   */
 pn = NULL;                             /* OUT OF LUCK CHIEF.                */
}                                       /*                                   */
