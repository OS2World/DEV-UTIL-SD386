/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   makefp.c                                                                */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Make a view.                                                            */
/*                                                                           */
/* History:                                                                  */
/*                                                                           */
/*   02/21/96 Modified.                                                      */
/*                                                                           */
/*****************************************************************************/

#define INCL_16
#include "all.h"

extern PROCESS_NODE *pnode;

/*****************************************************************************/
/* makefp                                                                    */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Creates an AFILE block for the specified module.                        */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   mid         the module id we're making this view for.                   */
/*   instaddr    build the view around this address.                         */
/*   fname       z-string filename from GetFile.                             */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   AFILE *fp   -> to afile view structure.                                 */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
AFILE *makefp(DEBFILE *pdf, ULONG mid, ULONG instaddr, UCHAR *fname)
{
 uchar    *cp = NULL;
 uint      segbase;
 uchar    *srcbuf;
 uint      Nlines;
 uint      Tlines;
 SEL       srcsel;
 SEL       offsel;

 ushort    srcseglen;
 ushort   *offtab;
 AFILE    *fp;
 int       ftype;
 int       OffsetTableBufSize;
 char      FileNameBuffer[CCHMAXPATH];
 char      FileSpecBuffer[CCHMAXPATH];
 UCHAR     FileNameLength;
 LNOTAB   *pLnoTabEntry;
 int       sfi;
 MODULE   *pModule;
 CSECT    *pCsect;
 UCHAR    *pFileName;
 UCHAR    *fn;
 BOOL      found;
 ULONG     junk;
 int       lno;

 if( fname )
 {
  /***************************************************************************/
  /* - if fname is specified, then the view is to be build using the source  */
  /*   file name supplied by the user as in the case of getfile.             */
  /* - we need to find an sfi to associate with the file.                    */
  /* - build a length-prefixed z-string from the supplied name.              */
  /***************************************************************************/
  fn         = strrchr(fname, '\\');
  fn         = (fn) ? (fn + 1) : fname ;
  pFileName  = Talloc( strlen(fn) + 2 );
  *pFileName = (UCHAR)strlen(fn);
  strcpy( pFileName+1, fn );

  found = FALSE;
  for( sfi=mid=0, pdf = pnode->ExeStruct; pdf != NULL; pdf=pdf->next )
  {
   mid = MapSourceFileToMidSfi( pFileName, &sfi, pdf );
   if( mid && sfi )
   {
    found = TRUE;
    memset(FileNameBuffer, 0, sizeof(FileNameBuffer) );
    strcpy(FileNameBuffer+1, fname );
    FileNameBuffer[0] = (UCHAR)strlen(fname);
    break;
   }
  }

  Tfree( pFileName );

  if( found == FALSE )
   return(NULL);

  instaddr = DBMapLno(mid, 0, sfi, &junk , pdf );
  DBMapInstAddr(instaddr, &pLnoTabEntry, pdf);
  lno = 0;
  if( pLnoTabEntry )
   lno = pLnoTabEntry->lno;
 }
 else
 {
  /***************************************************************************/
  /* - we're going to build the view from an instruction address.            */
  /***************************************************************************/
  if ( pdf->SrcOrAsm == SOURCE_LEVEL )
  {
   mid = DBMapInstAddr(instaddr, &pLnoTabEntry, pdf);
   lno = sfi = 0;
   if( pLnoTabEntry )
   {
    lno = pLnoTabEntry->lno;
    sfi = pLnoTabEntry->sfi;
   }
   if( (pLnoTabEntry != NULL) && (sfi != 0) )
   {
    memset(FileNameBuffer, 0, sizeof(FileNameBuffer) );
    cp = GetFileName( mid, sfi );
    if( cp )
     strcpy(FileNameBuffer, cp);
   }
  }
 }

 if(ftype)
 {
  findsrc(FileNameBuffer+1,FileSpecBuffer+1,sizeof(FileSpecBuffer) );
  FileSpecBuffer[0] = (UCHAR)strlen(FileSpecBuffer+1);
  memcpy(FileNameBuffer,FileSpecBuffer,sizeof(FileSpecBuffer));
 }

 FileNameLength = FileNameBuffer[0];

 fp = (AFILE*) Talloc(SizeOfAFILE(FileNameLength));
 memcpy( fp->filename, FileNameBuffer, FileNameLength+1);

 /****************************************************************************/
 /* - allocate 64k for the source buffer.                                    */
 /* - allocate 20k for the offset buffer.                                    */
 /* - load source file into a buffer and define:                             */
 /*    - srcsel    =  source buffer selector.                                */
 /*    - offsel    =  offset buffer selector.                                */
 /*    - 0         =  number of lines to skip at the beginning of the file.  */
 /*    - srcseglen =  source buffer bytes actually used by the load.         */
 /*    - Nlines    =  number of source lines in the buffer.                  */
 /*    - Tlines    =  number of source lines in the entire file.             */
 /* - reallocate the source buffer to size really needed.                    */
 /****************************************************************************/
 Nlines = srcsel = offsel = 0;
 if( !DosAllocSeg(0,(PSEL)&srcsel,0) &&
     !DosAllocSeg(20*1024,(PSEL)&offsel, 0) )
 {
  LoadSource( fp->filename+1, (UCHAR *)Sel2Flat(srcsel),
             (USHORT *)Sel2Flat(offsel), 0, &srcseglen, &Nlines, &Tlines);
  if( Nlines )
   DosReallocSeg(srcseglen, srcsel);
 }

 /****************************************************************************/
 /* - now, define the view structure.                                        */
 /****************************************************************************/
 fp->shower   = showA;
 fp->pdf      = pdf;
 fp->mid      = mid;
 fp->sfi      = sfi;
 fp->mseg     = segbase;
 fp->Tlines   = Tlines;                 /* number of lines in file.          */
 fp->Nlines   = Nlines;                 /* number of lines in source buffer. */
 fp->Nbias    = 0;                      /* number of lines skipped in source.*/
 fp->topline  = 1;                      /* init to 1st line in the file.     */
 fp->csrline  = lno;                    /*  " "                              */
 fp->hotline  = lno;                    /*  " "                              */
 fp->hotaddr  = instaddr;
 fp->skipcols = 0;                      /* columns skipped on left.          */
 fp->Nshown   = 0;
 fp->topoff   = instaddr;
 fp->csr.row  = 0;
 fp->csr.col  = 0;
 fp->csr.mode = CSR_NORMAL;
 fp->sview    = NOSRC;                  /* assume no source disassembler view*/
 fp->flags    = ASM_VIEW_NEW;

 if( Nlines )
 {
  srcbuf = (uchar*)Sel2Flat(srcsel);
  fp->source = srcbuf;

  /**************************************************************************/
  /* Allocate the offtab[] buffer to hold Nlines + 1 offsets. We add the 1  */
  /* so that we can make the offset table indices line up with the          */
  /* source line numbers.                                                   */
  /**************************************************************************/
  OffsetTableBufSize = (Nlines+1)*sizeof(USHORT);
  offtab             = (USHORT*) Talloc(OffsetTableBufSize);

  memcpy(offtab + 1, (uchar*)Sel2Flat(offsel), Nlines*sizeof(USHORT) );

  fp->offtab = offtab;
  fp->flags  = 0;                      /* clear asm flag.                   */

  if( Tlines > Nlines )                /* does compressed source exceed 64k?*/
   fp->flags |= AF_HUGE;               /* mark afile with huge file flag    */

  fp->shower = showC;                  /*  display source                   */
  fp->sview  = MIXEDN;                 /* assume no source disassembler view*/

/*
  Flag all text lines for which a (line #, offset) pair exists.
*/

  pModule = GetModuleWithAddr( instaddr, pdf );
  if( pModule )
  {
   for(pCsect = pModule->pCsects; pCsect != NULL; pCsect=pCsect->next )
   {
    int NumEntries;

    NumEntries   = pCsect->NumEntries;
    pLnoTabEntry = pCsect->pLnoTab;

    if( (pLnoTabEntry != NULL) && ( NumEntries > 0 ) )
    {
     for( ; NumEntries; pLnoTabEntry++, NumEntries-- )
     {
      if( pLnoTabEntry->sfi == sfi )
      {
       int lno;

       lno = pLnoTabEntry->lno;
       if( (lno != 0) && (lno <= Nlines) )
       {
        *(srcbuf + offtab[lno] - 1) |= LINE_OK;
       }
      }
     }
    }
   }
  }
  MarkLineBRKs( fp );                   /* mark the active breakpoints       */
 }

 if( offsel )                           /* if there was an offset segment    */
  DosFreeSeg(offsel);                   /* allocated then free it up         */
 if( !Nlines && srcsel )                /* if there was no source then       */
  DosFreeSeg(srcsel);                   /* free up the temp source buffer    */

 return( fp );
}

    AFILE *
fakefp(uint base,uint span,DEBFILE *pdf)
{
    uint n, mlen;
    uchar modname[CCHMAXPATH];
    AFILE *fp;

    strcpy(modname,pdf->DebFilePtr->fn);

    mlen = sprintf( modname, "Code Segment %04X (%04X thru %04X)",
            SelOf(base), OffOf(base), OffOf(base) + (uint)span - 1 );
    n = SizeOfAFILE(mlen);
    fp = (AFILE *) Talloc(n);
    fp->pdf = pdf;
    fp->mseg = base;
    fp->topoff=base;                  /* off of base for top disasm disp     */
    fp->mid = FAKEMID;
    fp->shower = showA;
    memcpy( fp->filename + 1, modname,
           (fp->filename[0] = ( uchar )mlen) + 1 );

    return( fp );
}
