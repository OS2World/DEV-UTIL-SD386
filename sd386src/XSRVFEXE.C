/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   xsrvfexe.c                                                              */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*  Find EXE file.( also used for browse )                                   */
/*                                                                           */
/* History:                                                                  */
/*                                                                           */
/*   11/30/92 Created.                                                       */
/*                                                                           */
/*...Release 1.02 (10/22/92)                                                 */
/*...                                                                        */
/*... 05/06/93  822   Joe       Add mte table handling.                      */
/*...                                                                        */
/*****************************************************************************/
#include "all.h"

/*****************************************************************************/
/* XSrvFindExe()                                                             */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Find EXE file.                                                          */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   progname input  - The filespec of the EXE we're looking for.            */
/*                                                                           */
/*   pn       input  - Pointer to the caller's buffer to receive filespec.   */
/*   pnlen    input  - Length of the caller's buffer.                        */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   rc       The return code from DosSearchPath().                          */
/*            The caller's buffer gets a fully qualified file spec.          */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*  The EXE extension has been added to the filespec.                        */
/*                                                                           */
/*****************************************************************************/
APIRET XSrvFindExe(char *progname,char *pn,UINT pnlen)
                                        /* progname :: the passed filespec   */
                                        /* pn :: the caller's buffer         */
                                        /* pnlen :: caller's buffer length   */
{                                       /*                                   */
 char      *fn;                         /* pointer to the filename           */
 ULONG      control;                    /* control word for DosSearchPath 822*/
 char       pathref[129];               /* path buffer for DosSearchPath     */
 char      *pref;                       /* a pointer to "\" in the filespec  */
 APIRET     rc;                         /* DosSearchPath return code         */
 char       fs[256];                    /* make local copy of progname.      */

/*****************************************************************************/
/* Look for the explicit filespec if there was one.                          */
/*                                                                           */
/* We have to extract the path part of the filespec and tell DosSearchPath   */
/* what it is. If we don't find the explicit filespec, then return with a    */
/* file not found code.                                                      */
/*****************************************************************************/
 strcpy(fs,progname);                   /* make local copy of progname.      */
 control = 0;                           /* use pathref as an ASCIIZ string   */
 pref=strrchr(fs,'\\');                 /* check again. Was there a path in  */
 if(pref)                               /* the filespec? If so, then         */
 {                                      /*                                   */
  *pref = '\0';                         /* copy path part of passed filespec */
  strcpy(pathref,fs);                   /* to the pathref                    */
  fn = pref+1;                          /* this is the filename less path    */
  rc=DosSearchPath(control,pathref,fn,pn,pnlen); /* look for it              */
  return(rc);                           /* return the result of our efforts  */
 }                                      /*                                   */
                                        /*                                   */
/*****************************************************************************/
/* Look in the current directory.                                            */
/*                                                                           */
/* At this point, there was no explicit filespec so we will look in the      */
/* current directory.                                                        */
/*****************************************************************************/
                                        /* or use filename as passed.        */
 rc= DosSearchPath(control,".\\",fs,pn,pnlen); /* look for the file          */
 if ( rc==0 )                           /* if we found it then               */
  return(rc);                           /* return.                           */
                                        /*                                   */
/*****************************************************************************/
/* Now look along the SD386SRC environment variable.                      116*/
/*****************************************************************************/
 control = 2;                           /* use path reference as env variable*/
 rc=DosSearchPath(control,"SD386SRC",fs,pn,pnlen); /* look for it         116*/
 if( rc==0)                             /*                                   */
  return(rc);                           /*                                   */
                                        /*                                   */
/*****************************************************************************/
/* Now look along the PATH environment variable.                             */
/*****************************************************************************/
 rc=DosSearchPath(control,"PATH",fs,pn,pnlen);    /* look for it             */
 return(rc);                            /*                                   */
}                                       /* end findexe()                     */
