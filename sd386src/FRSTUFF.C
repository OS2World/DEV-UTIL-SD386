/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   frstuff.c                                                               */
/*                                                                           */
/* Description:                                                              */
/*   fr DosOpen, etc. routines collection point.                             */
/*                                                                           */
/* Static Functions:                                                         */
/*                                                                           */
/* External Functions:                                                       */
/*                                                                           */
/*                                                                           */
/* History:                                                                  */
/*                                                                           */
/*   03/28/91 Culled out of ldsource.c to compile routines w/no INCL-16.     */
/*                                                                           */
/*...Release 1.00                                                            */
/*...                                                                        */
/*... 03/28/91  103   first attempts...                                      */
/*... 05/21/91  105   at least one editor (system E) does not add a newline  */
/*                    automatically at the end of file marker. The program   */
/*                    previously assumed this.                               */
/*...                                                                        */
/*...Release 1.00 (Pre-release 1.08 12/05/91)                                */
/*...                                                                        */
/*... 12/27/91  503   Srinivas  Last char of each line missing.              */
/*...                                                                        */
/**Includes*******************************************************************/

#include "all.h"                        /* SD86 include files                */

ushort FRlength = 0;                                                    /*101*/

typedef struct {
  HFILE  handle;  /* OS/2 file handle */ /* changed to HFILE              100*/
  ushort offset;  /* buffer offset of next unused byte */    /* to ushort 100*/
  ushort buflen;  /* # of bytes in buffer (less FRINFO) */   /* to ushort 100*/
  ushort togo;    /* # of bytes unused in buffer */          /* to ushort 100*/
} FRINFO;

 uint
FRopen(uchar *filename,uchar *buffer,ushort buflen)
{
    int rc;
    ulong   action;                     /*                                103*/
    FRINFO *p = (FRINFO*)buffer;        /* was register.                  112*/

    rc = DosOpen(filename, (PHFILE)&(p->handle), (PULONG)&action,       /*103*/
        0L,     /* primary allocation size for file */
        0x00,   /* file attributes */
        0x0001, /* open existing file */
        0x00A0, /* no inherit, deny write, read only */
        0L);    /* reserved */

    if( rc )
     return( rc );

    p->togo = 0;
    p->offset = sizeof(FRINFO);
    p->buflen = ( ushort )(buflen - sizeof(FRINFO));

    return( 0 );
}

    uint
FRclose(uchar *buffer )
{
    return( DosClose( ((FRINFO*)buffer)->handle ) );
}

    uchar *
FRread(uchar *buffer)
{
    uchar *cat;
    ushort n, togo;                     /* these are 2-byte variables     100*/
    ulong  BytesRead = 0;               /*                                103*/
    FRINFO *p = (FRINFO*)buffer;

    APIRET  rc;                         /*                                101*/
    HFILE   hFile;                      /*                                101*/
    PVOID   pBuffer;                    /*                                101*/
    ULONG   cbRead;                     /*                                101*/
    PULONG  pcbActual;                  /*                                101*/
    uchar   transcat[80];               /* 105 */

    for(;;){
        cat = (uchar*)p + p->offset;
        togo = p->togo;
        if( togo )
        {
            n = bindex(cat, togo, '\n');
            if( n < togo )
            {
                p->togo -= n+1;
                p->offset += n+1;
                /*************************************************************/
                /* In some cases we don't have a \r preceding \n, so the  503*/
                /* earlier assumption of \r preceding \n is not always    503*/
                /* valid.                                                 503*/
                /*************************************************************/
                if (cat[n-1] == '\r')                                   /*503*/
                {                                                       /*503*/
                  cat[ n-1 ] = 0;                                       /*503*/
                  FRlength = n-1;                                       /*503*/
                }                                                       /*503*/
                else                                                    /*503*/
                {                                                       /*503*/
                  cat[ n ] = 0;                                         /*503*/
                  FRlength = n;                                         /*503*/
                }                                                       /*503*/
                return( cat );
            }
/* With at least one editor (I found this in the E system editor), a newline*/
/* character is NOT put immediately before the EOF marker. The code was     */
/* modified to handle this problem - we assumed that there was always a     */
/* newline. */

            /* if the end of file is the last character, and there are still*/
            /* other characters left 105 */
            else if ((cat[togo-1] == 0x001A) &&                         /*105*/
                     (togo > 1))                                        /*105*/
                {                                                       /*105*/
                /* put it in a different string so we don't mess with the    */
                /* original string */
                strcpy(transcat,cat);                                   /*105*/
                transcat[togo -1] = 0;  /* return up to EOF char          105*/
                p->offset += togo - 1;  /* point to EOF                   105*/
                p->togo = 1;            /* just the EOF char to go        105*/
                FRlength = (ushort)(togo - 1);                          /*105*/
                return(transcat);                                       /*105*/
                }                                                       /*105*/
        }
        if( togo )
            memcpy( (uchar*)p + sizeof(FRINFO), cat, togo );            /*100*/


        p->offset = sizeof(FRINFO);                                     /*101*/
        hFile = p->handle;                                              /*101*/
        pBuffer = (uchar*)p + togo + p->offset;                         /*101*/
        cbRead = p->buflen - togo;                                      /*101*/
        pcbActual = &BytesRead;                                         /*101*/
        rc = DosRead(hFile, pBuffer, cbRead,pcbActual);                 /*101*/
        if ( rc || (BytesRead == 0) )                                   /*101*/
         return( NULL );                                                /*101*/
        p->togo += BytesRead;                                           /*101*/
    }
}
