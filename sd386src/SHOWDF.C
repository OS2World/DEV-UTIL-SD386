/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   showdf.c                                                                */
/*                                                                           */
/* Description:                                                              */
/*   Data File Display Formatting Routines                                   */
/*                                                                           */
/*                                                                           */
/* History:                                                                  */
/*                                                                           */
/*   02/08/91 Creation of 32-bit SD86, from 16-bit version.                  */
/*                                                                           */
/*...16->32 port.                                                            */
/*...                                                                        */
/*... 02/08/91  101   Joe       port to 32 bit.                              */
/*...                                                                        */
/*...Release 1.01 (04/03/92)                                                 */
/*...                                                                        */
/*... 05/08/92  701   Srinivas  Cua Interface.                               */
/**Includes*******************************************************************/

#include "all.h"                        /* SD86 include files                */

/**External declararions******************************************************/

extern uint        VideoCols;           /*  number of screen columns.        */
extern uint        DataFileTop;         /* top record displayed in data file.*/
extern uint        VioStartOffSet;      /* flag to tell were to start screen */
                                        /* display.                       701*/

/*****************************************************************************/
/*  fmtdwinstat()                                                            */
/*                                                                           */
/* Description:                                                              */
/*   Format the data window status line.                                     */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/* Return:                                                                   */
/*   void                                                                    */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
#define MAXFLDSIZE 8                    /* Size of "line xxxx of xxxx" field.*/
 void                                   /*                                   */
fmtdwinstat(uint dstatrow,int row)      /* dstatrow : data window status row */
                                        /* row:current record no in data file*/
{                                       /*                                   */
 uint  n;                               /* # of chars formatted(field size). */
 uchar buffer[ MAXFLDSIZE+4 ];          /*                                   */
 char  format[15];                      /* format for status line.           */
 int   recno;                           /* record # the cursor is on.        */
                                        /*                                   */
/*****************************************************************************/
/* First, clear the status line and turn on its color.                       */
/*****************************************************************************/
 ClrScr( dstatrow, dstatrow, vaStgStat);/*                                   */
                                        /*                                   */
/*****************************************************************************/
/* Format the "rec xxxx " field of the status line.                          */
/*                                                                           */
/*****************************************************************************/
 recno = DataFileTop + row + 1 - VioStartOffSet;                        /*701*/
 strcpy(format,"%crec %d");             /* Use this base format.             */
 n = sprintf( buffer,                   /*                                521*/
             format,                    /* Use this format                   */
             Attrib(vaStgStat),         /* and this attribute.               */
             recno                      /* The file record #. (1...N).       */
           );                           /*                                   */
 memset(buffer+n,' ',MAXFLDSIZE+1-n );  /* Pad end of buffer with blanks. 101*/
 buffer[ MAXFLDSIZE+1 ] = 0;            /* terminate the string.             */
 putrc( dstatrow,                       /* display the string.               */
        VideoCols-MAXFLDSIZE,
        buffer                          /*                                   */
      );                                /*                                   */
}
