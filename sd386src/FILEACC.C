/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   fileacc.c  (name changed 7/25/89 from exefile.c)                        */
/*                                                                           */
/* Description:                                                              */
/*   File handling functions for debug info file.                            */
/*                                                                           */
/*                                                                           */
/* History:                                                                  */
/*                                                                           */
/*   02/08/91 Creation of 32-bit SD86, from 16-bit version.                  */
/*                                                                           */
/*...16->32 port.                                                            */
/*...                                                                        */
/*... 05/14/91  105   Christina port to 32 bit.                              */
/*...                                                                        */
/*...Release 1.00 (Pre-release 108 12/05/91)                                 */
/*...                                                                        */
/*... 02/21/92  521   Srinivas  Port to C-Set/2.                             */
/**Includes*******************************************************************/

#include "all.h"                        /* SD86 include files                */
/*****************************************************************************/
/* opendos()                                                                 */
/*                                                                           */
/* Description:  The fopen() function is mimicked by using the DosOpen       */
/*    function. Fopen() is not used because it sets a limit on the number of */
/*    files that can be open at one time. The number of files can be set by  */
/*    the DosSetMaxFH function, and this is done in SD86.C. The only         */
/*    conversion that needs to be done is interpreting the attr string (which*/
/*    is the open mode that is usually passed to fopen, ex. "r+").           */
/*                                                                           */
/* Parameters:                                                               */
/*    filename      name of the file to open                                 */
/*    attr          open mode for file                                       */
/*    FileHandle    handle that will be used to operate on this file         */
/*                                                                           */
/* Return:                                                                   */
/*    zero if no error, non-zero if error                                    */
/*                                                                           */
/*****************************************************************************/
/* translate a function that simulates stream i/o into a dos function */
int opendos(char *filename,char *attr,HFILE *FileHandle)                /*105*/
{                                                                       /*105*/
                                        /*                                   */
   ULONG ActionTaken;                   /* action taken on file           105*/
   ULONG FileSize = 800L;               /* size of file to open; ignored  105*/
                                        /* if the file already exists        */
   ULONG FileAttr = FILE_ARCHIVED;      /* attributes chosen for the file 105*/
   ULONG OpenFlag;                      /* to handle new/existing files   105*/
   ULONG OpenMode;                      /* opening for read,write, append 105*/
   LONG Distance = 0L;                  /* distance from EOF to write     105*/
   ULONG NewPtr;                        /* new -> location after seek     105*/
   int rc;                              /* error code                     105*/

   OpenMode = OPEN_FLAGS_NOINHERIT;                                     /*105*/
    /* the flag constants are defined in <stdio.h>, the others in <bsedos.h> */
   if (*attr == 'r')                                                    /*105*/
       {
       OpenFlag = OPEN_ACTION_FAIL_IF_NEW | OPEN_ACTION_OPEN_IF_EXISTS; /*105*/
       }
   else if (*attr == 'w')                                               /*105*/
       {
       OpenFlag = OPEN_ACTION_CREATE_IF_NEW | OPEN_ACTION_REPLACE_IF_EXISTS;
       }
   else if (*attr == 'a')                                               /*105*/
       {
       OpenFlag = OPEN_ACTION_CREATE_IF_NEW | OPEN_ACTION_OPEN_IF_EXISTS;
       }
   else /* this is an error - first char must be r,w, or a */           /*105*/
       return 87;                                                       /*105*/

   if ((*(attr+1) == '+') || (*(attr+2) == '+'))                        /*105*/
       {
       OpenMode = OpenMode | OPEN_SHARE_DENYNONE | OPEN_ACCESS_READWRITE;
       }
   else
       if (*attr == 'r')                                                /*105*/
           OpenMode = OpenMode | OPEN_SHARE_DENYWRITE | OPEN_ACCESS_READONLY;
       else                             /* write or append operation      105*/
           OpenMode = OpenMode | OPEN_SHARE_DENYREAD | OPEN_ACCESS_WRITEONLY;

   rc = DosOpen(filename,FileHandle,&ActionTaken,FileSize,FileAttr,
                OpenFlag,OpenMode,NULL);                                /*105*/

   if (rc != NO_ERROR)                                                  /*105*/
       return rc;                                                       /*105*/

   /* if this was an append to an existing file, move the file ptr           */
   if ((*attr == 'a') && (ActionTaken == FILE_EXISTED))                 /*105*/
       rc = DosChgFilePtr(*FileHandle,Distance,SEEK_END,&NewPtr);       /*105*/

   return rc;                                                           /*105*/
}

/*****************************************************************************/
/* closedos()                                                                */
/*                                                                           */
/* Description:                                                              */
/*   mimic fclose() with DosClose, using a file handle. See description of   */
/*   opendos() for why.                                                      */
/*                                                                           */
/* Parameters:                                                               */
/*    FileHandle     file handle for file to close                           */
/*                                                                           */
/* Return: zero if no error, non-zero if error                               */
/*                                                                           */
/*****************************************************************************/
int closedos(HFILE FileHandle)                                          /*105*/
{
    return(DosClose(FileHandle));                                       /*105*/
}

/*****************************************************************************/
/* readdos()                                                                 */
/*                                                                           */
/* Description:                                                              */
/*    mimic the fread() function using DosRead with file handles. See        */
/*    description of OpenDos as to why.                                      */
/*                                                                           */
/* Parameters:                                                               */
/*    buffer     the buffer to put the bytes read into                       */
/*    size       what size of chunks to get                                  */
/*    count      how many chunks to get                                      */
/*    FileHandle file handle for this file                                   */
/*                                                                           */
/* Return:                                                                   */
/*   zero if no error, non-zero if an error. Does NOT return the number of   */
/*   bytes read to the caller, but this is in BytesRead if you want it.      */
/*                                                                           */
/*****************************************************************************/
int readdos(void *buffer,int size,int count,HFILE FileHandle)           /*105*/
{
    ULONG BytesRead;                    /* no of bytes read, should be    105*/
                                        /* same as size * count           105*/
    return(DosRead(FileHandle,buffer,size*count,&BytesRead));           /*105*/
}

/*****************************************************************************/
/* seekdos()                                                                 */
/*                                                                           */
/* Description:                                                              */
/*   mimics fseek() function using DosChgFilePtr with file handles. See      */
/*   description of OpenDos above to see why.                                */
/*                                                                           */
/* Parameters:                                                               */
/*   FileHandle   file handle for file to seek in                            */
/*   offset       offset to seek to from 'origin'                            */
/*   origin       start at this point (current, end, or beginning) and       */
/*                add offset                                                 */
/*                                                                           */
/* Return: zero if no error, non-zero if error.                              */
/*                                                                           */
/*****************************************************************************/
int seekdos(HFILE FileHandle,long offset,int origin)                    /*105*/
{
    ULONG Local;                                                        /*105*/

    return(DosChgFilePtr(FileHandle,offset,origin,&Local));             /*105*/
}

/*****************************************************************************/
/* seekf()                                                                   */
/*                                                                           */
/* Description:                                                              */
/*  seek file location.                                                      */
/*                                                                           */
/* Parameters:                                                               */
/*  pdf         debug file pointer.                                          */
/*  offset      file offset we're seeking.                                   */
/*                                                                           */
/* Return:                                                                   */
/*   void                                                                    */
/*                                                                           */
/*****************************************************************************/
  void
seekf(DEBFILE *pdf,ulong offset)
{
 HFILE  FileHandle;                     /* file struct for this debug file105*/
 int    rc = 0;                         /* return code. assume success.      */
 ULONG    Local;
 FileHandle = pdf->DebFilePtr->fh;      /* -> file structure for this pdf 105*/
 rc = DosChgFilePtr(FileHandle,         /* pointer to the debug info file 105*/
             offset,                    /* where to seek                     */
             SEEK_SET,                  /* offset from file beginning        */
             &Local);                   /* returns addr of new -> location105*/
 if ( rc )
   Error(ERR_FILE_READ,TRUE,1,pdf->DebFilePtr->fn);
 return;                                /* return.                           */
}                                       /* end of seekf()                    */

/*****************************************************************************/
/* readf()                                                                   */
/*                                                                           */
/* Description:                                                              */
/*  read a buffer from file.                                                 */
/*                                                                           */
/* Parameters:                                                               */
/*  bp          buffer pointer where caller wants the bytes stuffed.         */
/*  num         number of bytes the caller wants to read.                    */
/*  fp          file pointer.                                                */
/*                                                                           */
/* Return:                                                                   */
/*   void                                                                    */
/*                                                                           */
/*****************************************************************************/
  uint
readf(uchar *bp,uint num,DEBFILE *pdf)
{
 HFILE  FileHandle;                     /* file struct for this debug file105*/
 uint   bytes = 0;                      /* bytes returned.                105*/
 uint   rc=0;                           /* return code.                      */

 FileHandle = pdf->DebFilePtr->fh;      /* -> file structure for this pdf 105*/
 rc = DosRead( FileHandle,              /* file handle                    105*/
                 bp,                    /* buffer                         105*/
                num,                    /* size                           105*/
                (ulong *)&bytes );      /* number of bytes read           521*/
 if ( (rc != 0) || (bytes != num) )
   Error(ERR_FILE_READ,TRUE,1,pdf->DebFilePtr->fn);
 return(rc);
}                                       /* end of readf()                    */
