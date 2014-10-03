/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   ldsource.c                                                              */
/*                                                                           */
/* Description:                                                              */
/*   load source                                                             */
/*                                                                           */
/* Static Functions:                                                         */
/*                                                                           */
/* External Functions:                                                       */
/*                                                                           */
/*                                                                           */
/* History:                                                                  */
/*                                                                           */
/*...16->32 port.                                                            */
/*...                                                                        */
/*... 02/08/91  100   Philip    port to 32 bit.                              */
/*... 02/08/91  101   Joe       port to 32 bit.                              */
/*... 02/08/91  111   Christina port to 32 bit.                              */
/*...                                                                        */
/*...Release 1.00 (After pre-release 1.08)                                   */
/*...                                                                        */
/*... 02/12/92  521   Joe       Port to C-Set/2.                             */
/*...                                                                        */
/**Includes*******************************************************************/

#define INCL_16                         /* 16-bit API                     100*/
#include "all.h"                        /* SD86 include files                */

extern  ushort FRlength;                /*                                101*/

/* name         LoadSource -- Loads (part of) a source file.
 *
 * synopsis     LoadSource( filename, srcbuf, offbuf, skiplines,
 *                  srclenptr, Nlinesptr, Tlinesptr );
 *
 *              uchar *filename;        Name of source file.
 *              uchar *srcbuf;          Buffer for lines of source file.
 *              uint  *offbuf;          Buffer for line offsets into srcbuf.
 *              uint  skiplines;        # of lines to skip at start of file.
 *              uint  *srclenptr;       Returns # bytes used from srcbuf.
 *              uint  *Nlinesptr;       Returns # lines in srcbuf and offbuf.
 *              uint  *Tlinesptr;       Returns # lines in file (optional).
 *
 * description  The specified file is opened, read into storage, and an array
 *              of line offsets is constructed.  If the file can not be found
 *              or read, or a source buffer can not be allocated, then an AFILE
 *              block without source is returned.
 *
 * assumptions  1.  Each line of the file ends with 0x0D and 0x0A.
 *              2.  0x1A immediately follows the last line of the file.
 *              3.  The srcbuf starts at segment offset zero, and is 64K.
 */

#define IOBUFLEN 0xFFF8
#define LIMIT (ushort)(0-MAXCOLS-1)     /* This is only a 2-byte constant 100*/

 void
LoadSource(uchar *filename,uchar *srcbuf,ushort *offbuf,uint skiplines,
           ushort *srclenptr,uint *Nlinesptr,uint *Tlinesptr )
{
    SEL      selector = 0;              /*  initialized.                  521*/
    uchar *buffer, *lp;
    uint Nlines=0, Tlines=0;            /* was register.                  112*/

    if( !DosAllocSeg((USHORT)IOBUFLEN, (PSEL)&selector, 0) ){  /* 111 */
        if( !FRopen(filename, buffer = (UCHAR *)Sel2Flat(selector),(USHORT)IOBUFLEN) )
        {                                                           /*100 111*/
            lp = FRread(buffer);
            while( lp )
            {
             if( ++Nlines > skiplines )
             {
                 *srcbuf++ = 0;
                 *offbuf++ = LoFlat(srcbuf); /* offsets are 2-bytes    100*/
                 srcbuf += Encode(lp, srcbuf,
                     ((FRlength > MAXCOLS) ? MAXCOLS : FRlength) );
                 if( LoFlat(srcbuf) > LIMIT )
                 {                             /* 2-byte limit         100*/
                  if( Tlinesptr )
                   for( Tlines = Nlines ; FRread(buffer) ; ++Tlines )
                    {;}
                  break;
                 }
             }
             lp = FRread(buffer);
            }
            FRclose(buffer);
            *srcbuf++ = 0;
            if( Nlines >= skiplines )
                Nlines -= skiplines;
            else
                Nlines = 0;
        }
        DosFreeSeg(selector);
    }
    *srclenptr = LoFlat(srcbuf);        /* length is 2-byte value         100*/
    *Nlinesptr = Nlines;
    if( Tlinesptr )
        *Tlinesptr = (Tlines ? Tlines : (Nlines + skiplines));
}
