/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   pagefp.c                                                                */
/*                                                                           */
/* Description:                                                              */
/*   Recreates an AFILE block for the specified module.                      */
/*                                                                           */
/* Static Functions:                                                         */
/*                                                                           */
/* External Functions:                                                       */
/*                                                                           */
/*                                                                           */
/* History:                                                                  */
/*                                                                           */
/*   02/08/91 Creation of 32-bit SD86, from 16-bit version.                  */
/*                                                                           */
/*...16->32 port.                                                            */
/*...                                                                        */
/*... 02/08/91  100   made changes for 32-bit compilation.                   */
/*... 06/02/91  111   fix warnings                                           */
/*...                                                                        */
/*...Release 1.00 (Pre-release 1)                                            */
/*...                                                                        */
/*... 08/22/91  234   Joe       PL/X gives "varname" is incorrect message    */
/*...                           when entering a parameter name in the data   */
/*...                           window.  This happens when the cursor is on  */
/*...                           an internal procedure definition statement   */
/*...                           and you use F2 to get into the data window   */
/*...                           and then type the name.                      */
/*...                                                                        */
/*...Release 1.00 (Pre-release 108 12/05/91)                                 */
/*...                                                                        */
/*... 02/21/92  521   Srinivas  Port to C-Set/2.                             */
/**Includes*******************************************************************/
                                        /*                                   */
#define INCL_16                         /* 16-bit API                     100*/
#include "all.h"                        /* SD86 include files                */


extern uint      VideoRows;

/*****************************************************************************/
/*                                                                           */
/* synopsis     pagefp( mid, lno );                                          */
/*                                                                           */
/*              uint  mid;              (* module ID *)                      */
/*              uint  lno;              (* required line number *)           */
/*                                                                           */
/* description  Recreates an AFILE block for the specified module.           */
/*                                                                           */
/*****************************************************************************/
 void
pagefp(AFILE *fp,uint lno)
{
    uchar *srcbuf;
    SEL srcsel, offsel;                 /* changed to SEL                 100*/
    ushort srcseglen;                   /* 2-byte value                   100*/
    ushort *offtab;                     /* 2-byte values                  100*/
    uint    Nlines, retry;
    int       OffsetTableBufSize;       /* size alloc'd for offtab.       234*/
    uint FirstLineInSrcBuf;             /* First "real" line # in srcbuf. 234*/
    uint LastLineInSrcBuf;              /* Last  "real" line # in srcbuf. 234*/

    /* Just return if the line # is zero, or the line is already */
    /* in the buffer, or the source is complete in the buffer. */
    if( (lno == 0)
     || ((lno > fp->Nbias) && (lno <= (fp->Nbias + fp->Nlines)))
     || !(fp->flags & AF_HUGE)
      ) return;

    if( lno > LINESBEFORE )
        FirstLineInSrcBuf = lno - LINESBEFORE;
    else
        FirstLineInSrcBuf = 1;

    /* Get the table of (line number, segment offset) pairs,     */
    /* and allocate new segments for the source and line offsets */
    srcsel = offsel = 0;                /* clear SEL variables            100*/
    retry = 0;
    if( !DosAllocSeg(0,(PSEL)&srcsel,0) &&
        !DosAllocSeg(20*1024,(PSEL)&offsel,0)  ){

        Retry:
            LoadSource( fp->filename+1, (uchar *)Sel2Flat(srcsel),/* changed macro 100 111*/
                       (ushort *)Sel2Flat(offsel),       /* ushort ptr    100*/
                FirstLineInSrcBuf - 1, &srcseglen, &Nlines, NULL );

        if( Nlines ){
            if( ((lno + VideoRows) > (FirstLineInSrcBuf + Nlines)) && !retry ){
                if( lno > VideoRows )
                    FirstLineInSrcBuf = lno - VideoRows;
                else
                    FirstLineInSrcBuf = 1;
                retry = 1;
                goto Retry;
            }
            DosReallocSeg(srcseglen, srcsel);

            if( fp->offtab )
                Tfree(( void * ) fp->offtab);                            /*521*/

   /**************************************************************************/
   /* Allocate the offtab[] buffer to hold Nlines + 1 offsets. We add the 234*/
   /* so that we can make the offset table indices line up with the       234*/
   /* source line numbers.                                                234*/
   /**************************************************************************/
   OffsetTableBufSize = (Nlines+1)*sizeof(ushort);                      /*234*/
   offtab = (ushort*) Talloc(OffsetTableBufSize);                   /*521 234*/
   memcpy(offtab + 1, (uchar*)Sel2Flat(offsel), Nlines*sizeof(ushort) );/*234*/
   fp->offtab = offtab;                                                 /*234*/

            fp->Nlines = Nlines;

            if( fp->source )
                DosFreeSeg( Flat2Sel(fp->source) );/* convert to sel      100*/
            fp->source = srcbuf = (uchar *)Sel2Flat(srcsel);/* changed ptr macro   100 111 */

            fp->Nbias  = FirstLineInSrcBuf - 1;

            /* Flag all text lines for which a (line #, offset) pair exists  */
            /*****************************************************************/
            /* Scan the line number table looking for line numbers that      */
            /* are within the range of lines loaded in the source buffer.    */
            /* Flag these lines as ok to set a breakpoint on.                */
            /*****************************************************************/
            LastLineInSrcBuf = FirstLineInSrcBuf + Nlines - 1;

            MarkLineBrksOK( fp->pdf, fp->mid, fp->sfi, srcbuf, offtab,
                            FirstLineInSrcBuf, LastLineInSrcBuf);
            MarkLineBRKs( fp );
    }   }
    if( offsel )
        DosFreeSeg(offsel);
    if( !Nlines && srcsel )
        DosFreeSeg(srcsel);
}

void MarkLineBrksOK( DEBFILE *pdf,
                     ULONG    mid,
                     int      sfi,
                     UCHAR   *srcbuf,
                     USHORT  *offtab,
                     UINT     FirstLineInSrcBuf,
                     UINT     LastLineInSrcBuf)
{
 int     NumEntries;
 int     lno;
 CSECT  *pCsect;
 MODULE *pModule;
 LNOTAB *pLnoTabEntry;

 pModule = GetPtrToModule( mid, pdf );
 if( pModule == NULL )
  return;

 for(pCsect = pModule->pCsects; pCsect != NULL; pCsect=pCsect->next )
 {
  NumEntries   = pCsect->NumEntries;
  pLnoTabEntry = pCsect->pLnoTab;

  if( (pLnoTabEntry != NULL) && ( NumEntries > 0 ) )
  {
   for( ; NumEntries; pLnoTabEntry++, NumEntries-- )
   {
    if( pLnoTabEntry->sfi == sfi )
    {
     lno = pLnoTabEntry->lno;
     if( lno >= FirstLineInSrcBuf && lno <= LastLineInSrcBuf )
     {
      lno = lno - FirstLineInSrcBuf + 1;
      *(srcbuf + offtab[lno] - 1) |= LINE_OK;
     }
    }
   }
  }
 }
}
