/*****************************************************************************/
/* File:                                                                     */
/*   getfile.c                                                               */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*  Get the file specified by the user                                       */
/*                                                                           */
/*                                                                           */
/* History:                                                                  */
/*                                                                           */
/*   02/29/96 Revides.                                                       */
/*                                                                           */
/*****************************************************************************/

#include "all.h"

extern AFILE*   allfps;
extern UINT     LinesPer;

/*****************************************************************************/
/* GetFile                                                                   */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Build a view for the file specified by the user.                        */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   fname    pointer to filename string                                     */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   cur_afp  pointer to AFILE struct for the file                           */
/*   NULL     file not found                                                 */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   1. The AFILE structure for EXE file always exists (i.e. allfps <> NULL) */
/*                                                                           */
/*****************************************************************************/
AFILE *GetFile(UCHAR *fname)
{
 AFILE *cur_afp;
 AFILE *fp;
 AFILE *plast;
 uchar *fn;
 uchar *afn;
 uchar *apn;

 uint PathInfoFlag;


 PathInfoFlag = FALSE;


 if( strchr(fname,0x5C) )
    PathInfoFlag = TRUE;

 fn = strrchr(fname, '\\');
 fn = (fn) ? (fn + 1) : fname ;

/*****************************************************************************/
/* Search the list of AFILE structures to find the file name in it.          */
/* If found then return pointer to the AFILE struct.                         */
/* If not then search MODULE list of each DEBFILE struct  to locate          */
/* lineno info area and then filename within it and try to match user's fname*/
/* with lineno info filename.                                                */
/*****************************************************************************/
 cur_afp = allfps;
 while (cur_afp != NULL)
 {
    apn = (cur_afp->filename);
    afn = strrchr(apn, '\\');
    afn = (afn) ? (afn + 1) : apn;
    if (PathInfoFlag == TRUE)
    {
      if (stricmp(fname,apn+1) == 0)
         break;
    }
    else
    {
      if (stricmp(fn,afn) == 0)
         break;
    }
    cur_afp = cur_afp->next;
 }

/*****************************************************************************/
/* User's filename not found in the AFILE list so search MODULE list for     */
/* each executable file (exe + dll)                                          */
/*****************************************************************************/
 if(cur_afp == NULL)
 {
  /***************************************************************************/
  /* - build a view and add it to the ring of views.                         */
  /***************************************************************************/
  for( plast=(AFILE*)&allfps, fp=allfps; fp != NULL; plast=fp, fp=fp->next ){;}
  plast->next = cur_afp = makefp( NULL, 0, 0, fname );
 }

 /****************************************************************************/
 /* -set the csr line.                                                       */
 /****************************************************************************/
 if(cur_afp != NULL)
 {
  int LinesAvailableFromToplineToCsrline = 0;

  LinesAvailableFromToplineToCsrline = LinesPer>>1;

  if( cur_afp->csrline >= LinesAvailableFromToplineToCsrline )
   cur_afp->topline = cur_afp->csrline - LinesAvailableFromToplineToCsrline + 1;
  else
   cur_afp->topline = 1;
 }
 return(cur_afp);
}
