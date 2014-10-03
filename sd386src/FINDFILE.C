/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   findfile.c                                                              */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*  Find EXE or DLL source files.                                            */
/*                                                                           */
/*                                                                           */
/* History:                                                                  */
/*                                                                           */
/*   02/08/91 Creation of 32-bit SD86, from 16-bit version.                  */
/*                                                                           */
/*...16->32 port.                                                            */
/*...                                                                        */
/*... 02/08/91  116   Joe       port to 32 bit.                              */
/*...                                                                        */
/*...Release 1.00 (Pre-release 108 12/05/91)                                 */
/*...                                                                        */
/*... 12/19/91  500   Srinivas  HPFS file name problems.                     */
/*...                                                                        */
/**Includes*******************************************************************/

#include "all.h"                        /* SD86 include files                */

/*****************************************************************************/
/* findsrc()                                                                 */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   find exe or dll source.                                                 */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   fs       Pointer to filespec.                                           */
/*   pn       Pointer to the caller's buffer for the path.                   */
/*   pnlen    Length of the caller's buffer.                                 */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*****************************************************************************/
 void
findsrc(char *fs,char *pn,uint pnlen)   /* fs :: the passed filespec         */
                                        /* pn :: the caller's buffer         */
                                        /* pnlen :: caller's buffer length   */
{                                       /*                                   */
 char      *fn;                         /* pointer to the filename           */
 USHORT     control;                    /* control word for DosSearchPath    */
 char       pathref[129];               /* path buffer for DosSearchPath     */
 char      *pref;                       /* a pointer to "\" in the filespec  */
 uint       rc;                         /* DosSearchPath return code         */
 uint       FileNameLen,i;              /*                                500*/
 uint       PathFlag = 0;               /*                                500*/
                                        /*                                   */
/*****************************************************************************/
/* First, look in the current directory.                                     */
/*                                                                           */
/* If the filespec has a path prefix, then we want to strip off the filename */
/* part of it. If the filespec is just a filename, then use it as is.        */
/*****************************************************************************/
 FileNameLen = strlen(fs);                                              /*500*/
 i = 0;                                                                 /*500*/
 fn = fs + FileNameLen;                                                 /*500*/
 while (i++ < FileNameLen)                                              /*500*/
 {                                                                      /*500*/
   if ( (*fn == '\\') || (*fn == '/') )                                 /*500*/
   {                                                                    /*500*/
     fn = fn + 1;                                                       /*500*/
     break;                                                             /*500*/
   }                                                                    /*500*/
   else                                                                 /*500*/
     fn--;                                                              /*500*/
 }                                                                      /*500*/
 control = 0;                           /* no impied current directory       */
 rc= DosSearchPath(control,".\\",fn,pn,pnlen); /* look for the file          */
 if ( rc==0 )                           /* if we found it then               */
  return;                               /* return.                           */
                                        /*                                   */
/*****************************************************************************/
/* If not in the current directory, then try the filespec as passed.         */
/*                                                                           */
/* We have to extract the path part of the filespec and tell DosSearchPath   */
/* what it is.                                                               */
/*****************************************************************************/
                                        /*                                   */
 i = 0;                                                                 /*500*/
 pref = fs + FileNameLen;                                               /*500*/
 while (i++ < FileNameLen)                                              /*500*/
 {                                                                      /*500*/
   if ( (*pref == '\\') || (*pref == '/') )                             /*500*/
   {                                                                    /*500*/
     PathFlag = 1;                                                      /*500*/
     break;                                                             /*500*/
   }                                                                    /*500*/
   else                                                                 /*500*/
     pref--;                                                            /*500*/
 }                                                                      /*500*/
 if(PathFlag)                           /* the filespec? If so, then      500*/
 {                                      /*                                   */
  *pref = '\0';                         /* copy path part of passed filespec */
  strcpy(pathref,fs);                   /* to the pathref                    */
  fn = pref+1;                          /* this is the filename less path    */
  control = 0;                          /* use pathref as an ASCIIZ string   */
  rc=DosSearchPath(control,pathref,fn,pn,pnlen); /* look for it              */
  if ( rc==0 )                          /* if we found it then               */
   return;                              /* return.                           */
 }
/*****************************************************************************/
/* Now look along the SD386SRC environment variable.                      116*/
/*****************************************************************************/
 control = 2;                          /* use path reference as env variable */
 rc=DosSearchPath(control,"SD386SRC",fn,pn,pnlen); /* look for it         116*/
 if( rc==0)                             /*                                   */
  return;                               /*                                   */
                                        /*                                   */
/*****************************************************************************/
/* Now look along the PATH environment variable.                             */
/*****************************************************************************/
 rc=DosSearchPath(control,"PATH",fn,pn,pnlen);    /* look for it             */
 if( rc==0)                             /*                                   */
  return;                               /*                                   */
 pn = NULL;                             /* OUT OF LUCK CHIEF.                */
}                                       /*                                   */

/*****************************************************************************/
/* FindExe()                                                                 */
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
UINT FindExe(char *progname,char *pn,UINT pnlen)
                                        /* progname :: the passed filespec   */
                                        /* pn :: the caller's buffer         */
                                        /* pnlen :: caller's buffer length   */
{                                       /*                                   */
 char      *fn;                         /* pointer to the filename           */
 ULONG      control;                    /* control word for DosSearchPath 822*/
 char       pathref[129];               /* path buffer for DosSearchPath     */
 char      *pref;                       /* a pointer to "\" in the filespec  */
 UINT       rc;                         /* DosSearchPath return code         */
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