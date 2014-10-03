/****** PrtHLSym *************************************************************/
/*                                                                           */
/*  Description:                                                             */
/*      print to output stream                                               */
/*                                                                           */
/*  Parameters:                                                              */
/*                                                                           */
/*  Returns:                                                                 */
/*      void                                                                 */
/*                                                                           */
/*  Global Data:                                                             */
/*      o - options specified by user                                        */
/*                                                                           */
/*****************************************************************************/
#include "prntsyms.h"

void PrtHLSym(UCHAR *, UCHAR * );
FILE *ostream;                         /* text-style output                  */
ULONG  bugpos = 0;                     /* offset into exe of debug info      */
ULONG  currpos = 0;                    /* file ptr of item being displayed   */
extern void   dumptext( char *, UINT);

typedef union                          /* input options from user            */
  {                                    /* what to display                    */
    struct                             /* if bit set then show it            */
    {
       UINT h :1;                       /* header (t.o.c.)                    */
       UINT f :1;                       /* files & libs                       */
       UINT p :1;                       /* pubs                               */
       UINT t :1;                       /* types                              */
       UINT s :1;                       /* symbols                            */
       UINT y :1;                       /* symbols - old format               */
       UINT l :1;                       /* line nums                          */
       UINT x :1;                       /* hex output                     1.01*/
       UINT q :1;                       /* dump debug info stats              */
       UINT o :1;                       /* option specified                   */
       UINT z :1;                       /* invalid option                     */
    } opt;
    UINT  w;
  } options;
       options o;                      /* input options from user            */

void PrtHLSym(UCHAR *TempBufPtr, UCHAR *TempBufEnd)
{
  UCHAR  *p, *pend, Type;
  UINT    n, i;
  ULONG   prevn;
  ULONG   thisoff;
  UINT    totn;                        /* total number of bytes in rec   1.01*/
  UINT    RecordLen;


  o.opt.y = 0;
  o.opt.x = 0;
  ostream = fopen( "symbols.dmp", "a" );
  fprintf(ostream, "\n\nSymbolic information\n");
  fprintf(ostream, "====================\n");
          fprintf(ostream,
            "  Offset           Rec/Frame                     Reg/  Relative   \n");
          fprintf(ostream,
            " in File  Record  Offset/Addr  Type Index   Len  Seg    Offset    \n");
          fprintf(ostream,
            "   (0x)    Type      (0x)       Dec  (Hex)  (0x) Num  Body Epilog Name\n");
          fprintf(ostream,
            " ======== ======= ============ ============ ==== ==== =========== ============\n");

  n = 0;
  prevn = 0;
  while (TempBufPtr < TempBufEnd)
    {
      RecordLen = 2;    /* xxx */
      fprintf(ostream,"\n");

      /*
      ** setup record len
      ** setup pointer to record beginning
      ** setup pointer to record end
      ** setup offset of this record in file
      */
      p = TempBufPtr + 2; /* xxx */
      n = *(USHORT*)TempBufPtr;   /* xxx */


      totn = n;
      pend = p + n;

      thisoff = currpos + bugpos + prevn;

      /*
      ** get symbol type
      ** point to symbol information
      */
      Type = *(p);
      p += 1;

      /*
      ** make sure we don't have a record that exceeds what we're supposed
      ** to display.  If so, then dump the rest of the symbol information.
      */
      if (pend > TempBufEnd)
        {
          fprintf(ostream, " ");
          dumptext(TempBufPtr, TempBufEnd-TempBufPtr);
          fprintf(ostream, "\n");
          fclose( ostream );
          return;
        }

      /* check for symbol in the table, if we find it then print symbol name */
      for (i=0; i<NoOfHLSRecs; i++)
        {
          if (Type == recordsym[i].code)
            {
              if (o.opt.y)             /* straight format                   */
                fprintf(ostream, "  %s:", recordsym[i].name);
              break;
            }
        }

      /* check to see if didn't find the symbol in the table */
      if (i == NoOfHLSRecs)
        {
          /* don't know what to do, so dump whole record */
          fprintf(ostream, " ");
          dumptext(TempBufPtr, pend-TempBufPtr);
          goto SkipSymRecord;
        }


      /* determine which symbol record we got */
      switch (Type)
        {
          case SSBEGIN:                /* begin                              */
            {
              SSBegin  *bp;

              /* check to see we have a valid record */
              if (n < sizeof(SSBegin) - 2) /* xxx */
                goto DumpSymRecord;

              /* display the record */
              bp = (SSBegin *)(TempBufPtr);/* xxx */
              if (o.opt.y)             /* straight format                    */
                {
                  fprintf(ostream, " Offset=%lu  Bytes=%lu  Name=", bp->BlockOffset,
                    bp->BlockLen);
                }
              else                     /* columnar format                    */
                {
                  fprintf(ostream,
                    " %s  Begin     %.8X               ",
                    "        ", bp->BlockOffset);
                  fprintf(ostream,
                    "%.4X",
                    (USHORT)bp->BlockLen);
                  fprintf(ostream, "                  ");
                }

              p = pend;
              break;
            }
          case SSPROC:                 /* procedure                          */
            {
              SSProc  *pp;
              USHORT     nlen;
              USHORT     nbytes = 0;
/* xxx        USHORT     nbytes = 1; */

              /* check to see we have a valid record */
              if (n < sizeof(SSProc)-2 ) /* xxx */
                goto DumpSymRecord;

              /* display the record */
              pp = (SSProc *)(TempBufPtr);
/* xxx        pp = (SSProc *)(TempBufPtr + RecordLen); */
              if (o.opt.y)             /* straight format                    */
                {
                  fprintf(ostream,
                    " Offset=%lu  Type=%u (0x%.4X)  Bytes=%lu\n",
                    pp->ProcOffset, pp->TypeIndex, pp->TypeIndex, pp->ProcLen);
                  fprintf(ostream,
                    "              BodyOff=%u  EpilogOff=%u  Name=",
                    pp->DebugStart, pp->DebugEnd);
                }
              else                     /* columnar format                    */
                {
                  if( pp->Flags == MEMBERFUNC )
                  {
                    fprintf(ostream,
                      " %s  MemFcn    %.8lX   %4u (%.4X) ",
                      "        ", pp->ProcOffset, pp->TypeIndex, pp->TypeIndex);
                    fprintf(ostream,
                      "%.4X      %.4X   ",
                      (ushort)pp->ProcLen, pp->DebugStart);
                    fprintf(ostream,
                      "%.4X ",
                      (ushort)pp->DebugEnd);
                  }
                  else
                  {
                    fprintf(ostream,
                      " %s  Proc      %.8lX   %4u (%.4X) ",
                      "        ", pp->ProcOffset, pp->TypeIndex, pp->TypeIndex);
                    fprintf(ostream,
                      "%.4X      %.4X   ",
                      (USHORT)pp->ProcLen, pp->DebugStart);
                    fprintf(ostream,
                      "%.4X ",
                      (USHORT)pp->DebugEnd);
                  }
                }

              /* check to see name doesn't exceed record length */
              p = pp->Name;
              nlen = pp->NameLen;
/* xxx        nlen = *p; */

              if (p + nlen + nbytes > pend)
                goto DumpSymRecord;

              /* check to see if we have a name */
              if (nlen)
                {

                  if (nlen <= MAXNAMELEN)
                    fprintf(ostream, "%.*s", nlen, p+nbytes);
                  else
                    fprintf(ostream, "%.*s", MAXNAMELEN, p+nbytes);
                }

              if (!o.opt.y && pp->Flags == MEMBERFUNC )
                {
                  fprintf(ostream,
                    "\n                                %4u (%.4X)",
                    pp->ClassType, pp->ClassType);
                }

              p = pend;
              break;
            }
          case SSEND:                /* end                                */
            {
              SSEnd  *ep;

              /* check to see we have a valid record */
              if (n < sizeof(SSEnd) - 2 ) /* xxx */
                break;

              /* display the record */
              ep = (SSEnd *)(TempBufPtr); /* xxx */
              if (!o.opt.y)            /* columnar format                    */
                fprintf(ostream, " %s  End  ", "        " );

              p = pend;
              break;
            }
          case SSDEF:               /* automatic variable                 */
            {
              SSDef  *ap;

              /* check to see we have a valid record */
              if (n < sizeof(SSDef) - 2 ) /* xxx */
                goto DumpSymRecord;

              /* display the record */
              ap = (SSDef *)(TempBufPtr); /* xxx */
              if (o.opt.y)             /* straight format                    */
                {
                  fprintf(ostream, " FrameOff=%ld  Type=%u (0x%.4X)  Name=",
                    ap->FrameOffset, ap->TypeIndex, ap->TypeIndex);
                }
              else                     /* columnar format                    */
                {
                  fprintf(ostream,
                    " %s  Auto      %.8lX   %4u (%.4X)",
                    "        ", ap->FrameOffset, ap->TypeIndex, ap->TypeIndex);
                  fprintf(ostream, "                       ");
                }

              /* check to see name doesn't exceed record length */
              p = ap->Name;
/* xxx        if ((p + *p + 1) > pend) */
              if ((p + ap->NameLen ) > pend)
                goto DumpSymRecord;

              if ( (ap->NameLen))
                {

                  if ( (ap->NameLen) <= MAXNAMELEN)
                    fprintf(ostream, "%.*s", (ap->NameLen), (ap->Name));
                  else
                    fprintf(ostream, "%.*s", MAXNAMELEN, (ap->Name));
                }

              p = pend;
              break;
            }
          case SSVAR:             /* static variable                    */
            {
              SSVar  *sp;
              USHORT       nlen;
              USHORT       nbytes = 1;

              /* check to see we have a valid record */
              if (n < sizeof(SSVar) - 2 ) /* xxx */
                goto DumpSymRecord;

              /* display the record */
              sp = (SSVar *)(TempBufPtr ); /* xxx */
              if (o.opt.y)             /* straight format                    */
                {
                  fprintf(ostream, " Addr=%4.4hX:%8.8lX  Type=%u (0x%.4X)  Name=",
                    sp->ObjectNum, sp->Offset, sp->TypeIndex, sp->TypeIndex);
                }
              else                     /* columnar format                    */
                {
                  fprintf(ostream,
                    " %s  Static %4.4hX:%8.8lX %4u (%.4X)",
                    "        ", sp->ObjectNum, sp->Offset, sp->TypeIndex,
                    sp->TypeIndex);
                  fprintf(ostream, "                       ");
                }

              /* check to see name doesn't exceed record length */
              p = sp->Name;
              nlen = sp->NameLen;


              if (p + nlen > pend)
                goto DumpSymRecord;

              /* check to see if we have a name */
              if (nlen)
                {

                  if (nlen <= MAXNAMELEN)
                    fprintf(ostream, "%.*s", nlen, p);
                  else
                    fprintf(ostream, "%.*s", MAXNAMELEN, p);
                }

              p = pend;
              break;
            }
          case SSREG:                /* register variable                  */
            {
              SSReg  *rp;

              /* check to see we have a valid record */
              if (n < sizeof(SSReg) - 4 ) /* xxx */
                goto DumpSymRecord;

              /* display the record */
              rp = (SSReg *)(TempBufPtr ); /* xxx */
              if (o.opt.y)             /* straight format                    */
                {
                  fprintf(ostream, " Type=%u (0x%.4X)  RegNum=",
                    rp->TypeIndex, rp->TypeIndex);
                  PrintRegType(rp->RegNum);
                  fprintf(ostream, "  Name=");
                }
              else                     /* columnar format                    */
                {
                  fprintf(ostream,
                    " %s  Reg                  %4u (%.4X)      %.4X",
                    "        ", rp->TypeIndex, rp->TypeIndex, rp->RegNum);
                  fprintf(ostream, "             ");
                }

              /* check to see name doesn't exceed record length */
              p = rp->Name;
              if ((p + rp->NameLen) > pend)
                goto DumpSymRecord;

              /* check to see if we have a name */
              if (rp->NameLen)
                {

                  if (rp->NameLen <= MAXNAMELEN)
                    fprintf(ostream, "%.*s", rp->NameLen, rp->Name);
                  else
                    fprintf(ostream, "%.*s", MAXNAMELEN, rp->Name);
                }

              p = pend;
              break;
            }
          case SSCHGDEF:           /* change default segment          1.04*/
            {
              SSChgDef *cp;

              /* check to see if we have a valid record */
              if (n < sizeof(SSChgDef) - 2 ) /* xxx */
                goto DumpSymRecord;

              /* display the record */
              cp = (SSChgDef *)(TempBufPtr );/* xxx */
              if (o.opt.y)             /* straight format                    */
                {
                  fprintf(ostream, " ChDfSeg: Seg=%u  Reserved=%u",
                    cp->SegNum , cp->Reserved );
                }
              else                     /* columnar format                    */
                {
                  fprintf(ostream,
                    " %s  ChDfSeg                               %.4X",
                    "        ", cp->SegNum);
                  fprintf(ostream,
                    "             Reserved=%u",
                    cp->Reserved );
                }

              p = pend;
              break;
            }
          case SSUSERDEF:            /* typedef                            */
            {
              SSUserDef  *tp;

              /* check to see we have a valid record */
              if (n < sizeof(SSUserDef) - 2 ) /* xxx */
                goto DumpSymRecord;

              /* display the record */
              tp = (SSUserDef *)(TempBufPtr ); /* xxx */
              if (o.opt.y)             /* straight format                    */
                {
                  fprintf(ostream, " Type=%u (0x%.4X)  Name=",
                    tp->TypeIndex, tp->TypeIndex);
                }
              else                     /* columnar format                    */
                {
                  fprintf(ostream,
                    " %s  TypeDef              %4u (%.4X)",
                    "        ", tp->TypeIndex, tp->TypeIndex);
                  fprintf(ostream, "                       ");
                }

              /* check to see name doesn't exceed record length */
              p = tp->Name;
              if ((p + tp->NameLen) > pend)
                goto DumpSymRecord;

              /* check to see if we have a name */
              if ( (tp->NameLen))
                {

                  if ( (tp->NameLen) <= MAXNAMELEN)
                    fprintf(ostream, "%.*s",  (tp->NameLen), (tp->Name));
                  else
                    fprintf(ostream, "%.*s", MAXNAMELEN, (tp->Name));
                }

              p = pend;
              break;
            }
        }

DumpSymRecord:
      if (p < pend)
        {
          fprintf(ostream, " ");
          dumptext(p, pend-p);
        }
      else
        {
          fprintf(ostream, "\n");
          /*
          ** dump record in hex if hex output wanted
          */
          if (o.opt.x)
            {
              if (o.opt.y)
                {
                  fprintf(ostream, "              ");
                  dumphbuf(TempBufPtr, totn + 1, 14);
                }
              else
                {
                  fprintf(ostream, "           ");
                  dumphbuf(TempBufPtr, totn + 1, 11);
                }
              fprintf(ostream, "\n\n");
            }

        }

SkipSymRecord:
      prevn = prevn + totn + RecordLen;
      TempBufPtr = (UCHAR *)NextHLSymRec(TempBufPtr);
    }
  fclose( ostream );
}

/****** dumptext *************************************************************/
/*                                                                           */
/*  Description:                                                             */
/*                                                                           */
/*  Parameters:                                                              */
/*                                                                           */
/*  Returns:                                                                 */
/*                                                                           */
/*  Global Data:                                                             */
/*                                                                           */
/*****************************************************************************/

  void
dumptext (char *p, UINT n)
{
  UINT i, once=0;
  char *pend;

  for(pend=p+n; p < pend; p+=i)
    {
      if (once++)
        fprintf(ostream, "\n       ");

      if ((i = pend-p) > 16)
        i = 16;
      dumpline(p, i);
    }
}

/****** dumpline *************************************************************/
/*                                                                           */
/*  Description:                                                             */
/*                                                                           */
/*  Parameters:                                                              */
/*                                                                           */
/*  Returns:                                                                 */
/*                                                                           */
/*  Global Data:                                                             */
/*                                                                           */
/*****************************************************************************/

  void
dumpline(char *buf, UINT n)
{
  char *cp;
  UINT i;

  for(i=0, cp=buf; i < 16; i++)
    {
      /* 1 is the spacer */
      if ((i > 0) && ((i & 1) == 0) )
        fprintf(ostream, " ");

      if (i < n)
        dumphex(*cp++);
      else
        fprintf(ostream, "  ");
    }

  fprintf(ostream, " ");

  for(i=0, cp=buf; i < n; i++, cp++)
    fprintf(ostream, "%c", (*cp >= 0x20 && *cp < 0x7F) ? *cp : '.');
  fprintf(ostream, "\n");
}

/****** dumphbuf *************************************************************/
/*                                                                           */
/*  Description:                                                             */
/*      Dumps a buffer of hex bytes (no ASCII side text).                    */
/*                                                                           */
/*  Parameters:                                                              */
/*      p        ptr to buffer to be dumped                                  */
/*      n        number of bytes to dump                                     */
/*      space    white space amount before dumping line                      */
/*                                                                           */
/*  Returns:                                                                 */
/*      void                                                                 */
/*                                                                           */
/*  Global Data:                                                             */
/*      none                                                                 */
/*                                                                           */
/*****************************************************************************/

  void
dumphbuf(char *p, UINT n, UINT space)
{
  char *cp;
  UINT i, once=0;
  char *pend;
  char fakeit;

  for(pend=p+n; p < pend; p+=i)
    {
      if (once++)
        {
          fprintf(ostream, "\n");
          fakeit = ' ';
          fprintf(ostream, "%*c", space, fakeit);
        }

      if ((i = pend-p) > 16)
        i = 16;
      dumphlin(p, i);
    }
}

/****** dumphlin *************************************************************/
/*                                                                           */
/*  Description:                                                             */
/*      Dumps a line of hex bytes (no ASCII side text).                      */
/*                                                                           */
/*  Parameters:                                                              */
/*      buf      ptr to buffer to be dumped                                  */
/*      n        number of bytes to dump                                     */
/*                                                                           */
/*  Returns:                                                                 */
/*      void                                                                 */
/*                                                                           */
/*  Global Data:                                                             */
/*      none                                                                 */
/*                                                                           */
/*****************************************************************************/

  void
dumphlin(char *buf, UINT n)
{
  char *cp;
  UINT i;

  for(i=0, cp=buf; i < 16; i++)
    {
      /* 1 is the spacer */
      if ((i > 0) && ((i & 1) == 0) )
        fprintf(ostream, " ");

      if (i < n)
        dumphex(*cp++);
      else
        fprintf(ostream, "  ");
    }

}

/****** dumphex **************************************************************/
/*                                                                           */
/*  Description:                                                             */
/*                                                                           */
/*  Parameters:                                                              */
/*                                                                           */
/*  Returns:                                                                 */
/*                                                                           */
/*  Global Data:                                                             */
/*                                                                           */
/*****************************************************************************/

  void
dumphex(UCHAR c)
{
  static char hexdig[16] = "0123456789ABCDEF";

  fprintf(ostream, "%c%c", hexdig[(c >> 4) & 0x0F], hexdig[c & 0x0F]);
}

/****** PrintRegType *********************************************************/
/*                                                                           */
/*  Description:                                                             */
/*      print to output stream                                               */
/*                                                                           */
/*  Parameters:                                                              */
/*                                                                           */
/*  Returns:                                                                 */
/*      void                                                                 */
/*                                                                           */
/*  Global Data:                                                             */
/*                                                                           */
/*****************************************************************************/

  void
PrintRegType (UINT n)
{

  typedef struct recsymnames
    {
      int     code;
      char    name[20];
    };

static struct recsymnames
       RegTypeNames[43] = {
           0x00,   "AL",
           0x01,   "CL",
           0x02,   "DL",
           0x03,   "BL",
           0x04,   "CH",
           0x05,   "DH",
           0x06,   "BH",
           0x07,   "AL",
           0x08,   "AX",
           0x09,   "CX",
           0x0A,   "DX",
           0x0B,   "BX",
           0x0C,   "SP",
           0x0D,   "BP",
           0x0E,   "SI",
           0x0F,   "DI",
           0x10,   "EAX",
           0x11,   "ECX",
           0x12,   "EDX",
           0x13,   "EBX",
           0x14,   "ESP",
           0x15,   "EBP",
           0x16,   "ESI",
           0x17,   "EDI",
           0x18,   "SS",
           0x19,   "CS",
           0x1A,   "SS",
           0x1B,   "DS",
           0x1C,   "FS",
           0x1D,   "GS",
           0x20,   "DX:AX",
           0x21,   "ES:BX",
           0x22,   "IP",
           0x23,   "FLAGS",
           0x24,   "EFLAGS",
           0x80,   "ST(0)",
           0x81,   "ST(1)",
           0x82,   "ST(2)",
           0x83,   "ST(3)",
           0x84,   "ST(4)",
           0x85,   "ST(5)",
           0x86,   "ST(6)",
           0x87,   "ST(7)"
  };

  UINT   i;

  for (i=0; i<43; i++)
    {
      if (n == RegTypeNames[i].code)
        {
          fprintf(ostream, "%s", RegTypeNames[i].name);
          return;
        }
    }

  fprintf(ostream, "%u", n);
}
