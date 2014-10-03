#include "all.h"

#include "prnttyps.h"

#define GOODRECORD  0
#define BADRECORD   1

uchar  DebugVersion;

extern void   dumptext( char *, UINT);
extern void   dumpline( char *, UINT);
extern void   dumphbuf( char *, UINT, UINT);
extern void   dumphlin( char *, UINT);
extern void   dumphex( UCHAR);
extern void   PrintRegType( UINT);
void   PrintTypeIndex( ushort );

static uint  Prt_HLF_Span(uchar **, uchar *, uchar *, uchar);
static uint  Prt_HLF_Text(uchar **, uchar *, uchar *);
static uint  Prt_HLF_Index(uchar **, uchar *, uchar *);
static uint  Prt_HLF_Index2(uchar **, uchar *, uchar *);
static uint  Prt_HLF_String(uchar **, uchar *, uchar *);
static uint  Prt_HLF_String2(uchar **, uchar *, uchar *);
static uint  Prt_HLF_Precision(uchar **, uchar *, uchar *, uchar *);

void   DumpModules( char *, int, FILE *, long * );
void   DumpModule( FILE *, ulong, ulong );

void PrtHLType( UCHAR *, UCHAR *);
FILE *ostream;                         /* text-style output                  */

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
#define MAXNAMELENX 60

/****** PrtHLType ************************************************************/
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

void  PrtHLType( uchar *TempBufPtr, uchar *TempBufEnd)
{
  uint    TypeIndex = 512;              /* convention: 512=1st $$TYPES index  */
  uchar  *p, *pend, TypeQual, Type, flag, Name[20];
  uint    n, itemcount, i;
  uint    BadRecord;
  uint    totn;                        /* total number of bytes in rec   1.01*/

  ostream = fopen( "types.dmp", "a" );
  fprintf(ostream, "\n\nType information\n");
  fprintf(ostream, "================\n");

  fprintf(ostream, "\n");
  while (TempBufPtr < TempBufEnd)
    {
      fprintf(ostream, " %5u)", TypeIndex++);  /* move to line up        1.01*/
      BadRecord = FALSE;

      if (TempBufPtr + sizeof(HLTrec2) - 1 > TempBufEnd)
        {
          fclose( ostream );
          return;
        }
        n = (uint)(((HLTrec2 *)TempBufPtr)->reclen);
        p = TempBufPtr + 2;

      totn = n;

      /*
      ** make sure we don't have a record that exceeds what we're supposed
      ** to display.  If so, then dump the rest of the type information.
      */
      if (p + n > TempBufEnd)
        {
          fprintf(ostream, " ");
          dumptext(TempBufPtr, TempBufEnd-TempBufPtr);
          fprintf(ostream, "\n");
          return;
        }

      /* setup type record end */
      pend = p + n;

      Type = *(p);
      /* setup type qualifier */
      TypeQual = *(p+1);
      /* point to sub-record data */
      p += 2;

/*       don't skip recs with length 1 - handle as usual 1.03
      check to see if we have a primitive or a complex type
      if (n == 1)
        {
          fprintf(ostream, " 0x%X\n", Type);
          goto SkipTypeRecord;
        }
*/

      /* check for type in the table, if we find it then print type name */
      for (i=0; i<NoOfHLTRecs; i++)
        {
          if (Type == recordtype[i].code)
            {
              fprintf(ostream, " %s:", recordtype[i].name);
              break;
            }
        }

      /* check to see if didn't find the type in the table */
      if (i == NoOfHLTRecs)
      {
        fprintf( ostream, "\n" );
        goto SkipTypeRecord;
      }

      /* print out sub-record specific data */
      switch (Type)
        {
          case HLT_ARRAY:              /* array                              */
              if (TypeQual & 0x01)
                fprintf(ostream, " ColMjr");
              else
                fprintf(ostream, " RowMjr");

              if (TypeQual & 0x02)
                fprintf(ostream, " Packed");
              else
                fprintf(ostream, " Unpacked");

              if (TypeQual & 0x04)
                fprintf(ostream, " Desc-prov");
              else
                fprintf(ostream, " No-Desc-Req");

              if (p + 4 > pend)
                goto DumpTypeRecord;

              fprintf(ostream, " Bytes:%lu", *(DWORD *)p);
              p += 4;

              if (BadRecord = Prt_HLF_Index(&p, pend, "Bounds"))
                goto DumpTypeRecord;
              if (BadRecord = Prt_HLF_Index(&p, pend, "Type"))
                goto DumpTypeRecord;
              BadRecord = Prt_HLF_String(&p, pend,
                                         "\n                  Name");
              break;

          case HLT_BASECLASS:           /* baseclass                          */
#if 0
              if (TypeQual & 0x01)
                fprintf(ostream, " Is-virtual");
              else
                fprintf(ostream, " Not-virtual");

              if (p + 1 > pend)
                goto DumpTypeRecord;

              fprintf(ostream, " Prot:");
              switch (*p)
                {
                  case 0:
                    fprintf(ostream, "private");
                    break;
                  case 1:
                    fprintf(ostream, "protected");
                    break;
                  case 2:
                    fprintf(ostream, "public");
                    break;
                  default:
                    goto DumpTypeRecord;
                }
              p += 1;

              if (BadRecord = Prt_HLF_Index2(&p, pend, "Type"))
                goto DumpTypeRecord;
              if (BadRecord = Prt_HLF_Span(&p, pend, "Offset"))
                goto DumpTypeRecord;
#endif
              break;

          case HLT_BITSTRING:          /* bit string                         */

              if (TypeQual & 0x01)
                {
                  fprintf(ostream, " Varying");
                  strcpy(Name, "MaxSize");
                }
              else
                {
                  fprintf(ostream, " Fixed");
                  strcpy(Name, "Size");
                }

              if (TypeQual & 0x02)
                fprintf(ostream, " Signed");
              else
                fprintf(ostream, " Unsigned");

              if (TypeQual & 0x04)
                fprintf(ostream, " Word-Align");
              else
                fprintf(ostream, " Byte-Align");

              if (TypeQual & 0x08)
                fprintf(ostream, " Displ-as-Value");
              else
                fprintf(ostream, " Displ-as-0/1-Str");

              if (TypeQual & 0x10)
                fprintf(ostream, " Desc-prov");
              else
                {
                  fprintf(ostream, " No-Desc-Req");
                  fprintf(ostream, "\n                  Offset:%u", *(BYTE *)p);
                  p += 1;
                  BadRecord = Prt_HLF_Span(&p, pend, Name, 1);
                }
                p += 1;                 /* get past base type field.         */
              break;

#if 0
          case HLT_CHARSTRING:         /* character string                   */
              switch (TypeQual)
                {
                  case 0x00:
                    fprintf(ostream, " fixed");
                    BadRecord = Prt_HLF_Span(&p, pend, "Size");
                    break;
                  case 0x01:
                    fprintf(ostream, " len-prfx");
                    BadRecord = Prt_HLF_Span(&p, pend, "MaxSize");
                    break;
                  case 0x02:
                    fprintf(ostream, " adjustable");
                    break;
                  case 0x03:
                    fprintf(ostream, " null-term");
                    BadRecord = Prt_HLF_Span(&p, pend, "MaxSize");
                    break;
                  case 0x04:
                    fprintf(ostream, " DBCS");
                    BadRecord = Prt_HLF_Span(&p, pend, "MaxSize");
                    break;
                  case 0x05:
                    fprintf(ostream, " DBCS-edited");
                    break;
                  default:
                    /*
                    ** bad type qualifier, move pointer back and display
                    ** from type qualifier to the end of the record
                    */
                    p--;
                    break;
                }
              break;

          case HLT_CLASS:               /* class                              */
              if (TypeQual & 0x01)
                fprintf(ostream, " Is-a-struct");
              else
                fprintf(ostream, " Not-a-struct");

              if (p + 4 > pend)
                goto DumpTypeRecord;

              fprintf(ostream, " Bytes:%lu", *(DWORD *)p);
              p += 4;

              if (p + 2 > pend)
                goto DumpTypeRecord;

              fprintf(ostream, " N:%u", *(WORD *)p);
              p += 2;

              if (BadRecord = Prt_HLF_Index2(&p, pend, "ItemList"))
                goto DumpTypeRecord;
              BadRecord = Prt_HLF_String2(&p, pend, "Name");

              #ifdef STATS
              TypeStringSize[Type] += TempStringSize;
              #endif /* STATS */
              break;
          case HLT_CLASS_DEF:           /* class definition                   */
              if (p + 1 > pend)
                goto DumpTypeRecord;

              fprintf(ostream, " Prot:");
              switch (*p)
                {
                  case 0:
                    fprintf(ostream, "private");
                    break;
                  case 1:
                    fprintf(ostream, "protected");
                    break;
                  case 2:
                    fprintf(ostream, "public");
                    break;
                  default:
                    goto DumpTypeRecord;
                }
              p += 1;

              if (BadRecord = Prt_HLF_Index2(&p, pend, "Type"))
                goto DumpTypeRecord;
              if (DebugVersion >= 0x03)
                {
                  if (BadRecord = Prt_HLF_Index2(&p, pend, "DefClass"))
                    goto DumpTypeRecord;
                }
              break;
          case HLT_CLASS_MEMBER:        /* class member                       */
              if (TypeQual & 0x01)
                fprintf(ostream, " is-static");
              else
                fprintf(ostream, " not-static");

              if (TypeQual & 0x02)
                fprintf(ostream, " is-vtbl-ptr");
              else
                fprintf(ostream, " not-vtbl-ptr");

              if (TypeQual & 0x04)
                fprintf(ostream, " is-vbase-ptr");
              else
                fprintf(ostream, " not-vbase-ptr");

              if (TypeQual & 0x08)
                fprintf(ostream, " const");
              else
                fprintf(ostream, " non-const");

              if (TypeQual & 0x10)
                fprintf(ostream, " volatile");
              else
                fprintf(ostream, " non-volatile");

              if (p + 1 > pend)
                goto DumpTypeRecord;

              fprintf(ostream, "\n                  Prot:");
              switch (*p)
                {
                  case 0:
                    fprintf(ostream, "private");
                    break;
                  case 1:
                    fprintf(ostream, "protected");
                    break;
                  case 2:
                    fprintf(ostream, "public");
                    break;
                  default:
                    goto DumpTypeRecord;
                }
              p += 1;

              if (BadRecord = Prt_HLF_Index2(&p, pend, "Type"))
                goto DumpTypeRecord;
              if (BadRecord = Prt_HLF_Span(&p, pend, "Offset"))
                goto DumpTypeRecord;
              if (BadRecord = Prt_HLF_String2(&p, pend, "Name"))
                goto DumpTypeRecord;

              #ifdef STATS
              TypeStringSize[Type] += TempStringSize;
              #endif /* STATS */

              BadRecord = Prt_HLF_String2(&p, pend, "Name");

              #ifdef STATS
              TypeStringSize[Type] += TempStringSize;
              #endif /* STATS */
              break;
          case HLT_CODELABEL:          /* code label                         */
          case HLT_FORMATLABEL:        /* format label                       */
              switch (TypeQual & 0x03)
                {
                  case 0x00:
                    fprintf(ostream, " offset-16");
                    break;
                  case 0x01:
                    fprintf(ostream, " offset-32");
                    break;
                  case 0x02:
                    fprintf(ostream, " segment-32");
                    break;
                  case 0x03:
                    fprintf(ostream, " segment-48");
                    break;
                }
              break;
#endif
          case HLT_ENTRY:              /* entry                              */
          case HLT_FUNCTION:           /* function                           */
          case HLT_PROCEDURE:          /* procedure                          */
              if (TypeQual & 0x01)
                fprintf(ostream, " Args(Pushed R->L");
              else
                fprintf(ostream, " Args(Pushed L->R");

              if (TypeQual & 0x02)
                fprintf(ostream, ", Caller Pops)");
              else
                fprintf(ostream, ", Callee Pops)");

              switch (TypeQual & 0x0C)
                {
                  case 0x00:
                    fprintf(ostream, " offset-16");
                    break;
                  case 0x04:
                    fprintf(ostream, " offset-32");
                    break;
                  case 0x08:
                    fprintf(ostream, " segment-32");
                    break;
                  case 0x0C:
                    fprintf(ostream, " segment-48");
                    break;
                }

              if (p + 2 > pend)
                goto DumpTypeRecord;

              fprintf(ostream, " Args:%u", *(WORD *)p);
              p += 2;

              if (p + 2 > pend)
                goto DumpTypeRecord;

              fprintf(ostream, " MaxArgs:%u", *(WORD *)p);
              p += 2;

              if (BadRecord = Prt_HLF_Index(&p, pend,
                                            "\n                  RetType"))
                goto DumpTypeRecord;
              BadRecord = Prt_HLF_Index(&p, pend, "TypeList");
              break;

          case HLT_SCALAR:
              p--;
              if (BadRecord = Prt_HLF_Index(&p, pend, "Type"))
                goto DumpTypeRecord;
              break;

          case HLT_ENUM:               /* enumerated                         */
              p--;
              if (BadRecord = Prt_HLF_Index(&p, pend, "Type"))
                goto DumpTypeRecord;
              if (BadRecord = Prt_HLF_Index(&p, pend, "NdxLst"))
                goto DumpTypeRecord;
              if (BadRecord = Prt_HLF_Span(&p, pend, "MinNdx", 4))
                goto DumpTypeRecord;
              if (BadRecord = Prt_HLF_Span(&p, pend, "MaxNdx", 4))
                goto DumpTypeRecord;
              BadRecord = Prt_HLF_String(&p, pend, "Name");
              break;

          case HLT_FILE:               /* file                               */
              break;
#if 0
          case HLT_FRIEND:             /* friend                             */
              if (TypeQual & 0x01)
                fprintf(ostream, " Friend-class");
              else
                fprintf(ostream, " Friend-fcn");

              if (BadRecord = Prt_HLF_Index2(&p, pend, "Type"))
                goto DumpTypeRecord;
              BadRecord = Prt_HLF_String2(&p, pend, "Name");

              #ifdef STATS
              TypeStringSize[Type] += TempStringSize;
              #endif /* STATS */
              break;
          case HLT_GRAPHIC:            /* graphic                            */
              switch (TypeQual)
                {
                  case 0x00:
                    fprintf(ostream, " fixed");
                    BadRecord = Prt_HLF_Span(&p, pend, "Size");
                    break;
                  case 0x01:
                    fprintf(ostream, " len-prfx");
                    BadRecord = Prt_HLF_Span(&p, pend, "MaxSize");
                    break;
                  case 0x02:
                    fprintf(ostream, " adjustable");
                    break;
                  default:
                    /*
                    ** bad type qualifier, move pointer back and display
                    ** from type qualifier to the end of the record
                    */
                    p--;
                    break;
                }
              break;
#endif
          case HLT_LIST:               /* list                               */
              itemcount = 0;
              switch (TypeQual)
                {
                  case 0x01:           /* list of types                      */
                    while (!BadRecord && (p < pend))
                      {
                        itemcount++;
                        /* can usually fit 5 list types per line */
                        if ((itemcount % 6) == 0)
                          {
                            fprintf(ostream, "\n                 ");
                            itemcount = 1;
                          }
                        BadRecord = Prt_HLF_Index(&p, pend, "Type");
                      }
                    break;

                  case 0x02:           /* list of name-offsets               */
                    while (!BadRecord && (p < pend))
                      {
                        itemcount++;
                        /* can usually fit 3 list name/offsets per line */
                        if ((itemcount % 4) == 0)
                          {
                            fprintf(ostream, "\n                 ");
                            itemcount = 1;
                          }
                        if (BadRecord = Prt_HLF_String(&p, pend, "Name"))
                          goto DumpTypeRecord;

                        BadRecord = Prt_HLF_Span(&p, pend, "Offset", 4);
                        fprintf(ostream, " ");
                      }
                    break;
                  case 0x03:           /* list of name-indexes               */
#if 0
                    while (!BadRecord && (p < pend))
                      {
                        itemcount++;
                        /* can usually fit 5 list name-indexes per line */
                        if ((itemcount % 6) == 0)
                          {
                            fprintf(ostream, "\n                 ");
                            itemcount = 1;
                          }
                        if (BadRecord = Prt_HLF_String(&p, pend, "Name"))
                          goto DumpTypeRecord;

                        BadRecord = Prt_HLF_Span(&p, pend, "Ndx");
                        fprintf(ostream, " ");
                      }
#endif
                    break;
                  case 0x04:           /* list of parameter types            */
#if 0
                    while (!BadRecord && (p < pend))
                      {
                        itemcount++;
                        /* can usually fit 3 list parameter types per line */
                        if ((itemcount % 4) == 0)
                          {
                            fprintf(ostream, "\n                 ");
                            itemcount = 1;
                          }
                        flag = *p++;
                        if (BadRecord = Prt_HLF_Index(&p, pend, "Type"))
                          goto DumpTypeRecord;

                        if (flag & 0x01)
                          fprintf(ostream, " (Val,");
                        else
                          fprintf(ostream, " (Addr,");

                        if (flag & 0x02)
                          fprintf(ostream, " Dsc-Prov)");
                        else
                          fprintf(ostream, " No-Dsc-Req)");

                        fprintf(ostream, " ");
                      }
#endif
                    break;
                  default:
                    /*
                    ** bad type qualifier, move pointer back and display
                    ** from type qualifier to the end of the record
                    */
                    p--;
                    break;
                }
              break;
#if 0
          case HLT_LOGICAL:            /* logical                            */
              switch (TypeQual)
                {
                  case 0x01:
                    fprintf(ostream, " (Fortran 4)");
                    break;
                  case 0x02:
                    fprintf(ostream, " (Fortran 1)");
                    break;
                  case 0x04:
                    fprintf(ostream, " (RPG)");
                    break;
                  default:
                    /*
                    ** bad type qualifier, move pointer back and display
                    ** from type qualifier to the end of the record
                    */
                    p--;
                    break;
                }
              break;
          case HLT_MACRO:              /* macro                              */
              /* don't do anything with macros yet */
              break;
          case HLT_MEMBER_FUNCTION:     /* member function                    */
              if (DebugVersion >= 0x03)
                {
                  fprintf(ostream, " Attr(");

                  if (TypeQual & 0x01)
                    fprintf(ostream, "static");
                  else
                    fprintf(ostream, "non-static");

                  if (TypeQual & 0x02)
                    fprintf(ostream, ", inline");
                  else
                    fprintf(ostream, ", non-inline");

                  if (TypeQual & 0x04)
                    fprintf(ostream, ", const");
                  else
                    fprintf(ostream, ", non-const");

                  if (TypeQual & 0x08)
                    fprintf(ostream, ", volatile");
                  else
                    fprintf(ostream, ", non-volatile");

                  if (TypeQual & 0x10)
                    fprintf(ostream, "\n                  virtual)");
                  else
                    fprintf(ostream, "\n                  non-virtual)");

                  if (p + 1 > pend)
                    goto DumpTypeRecord;

                  fprintf(ostream, " Prot:");
                  switch (*p)
                    {
                      case 0:
                        fprintf(ostream, "private");
                        break;
                      case 1:
                        fprintf(ostream, "protected");
                        break;
                      case 2:
                        fprintf(ostream, "public");
                        break;
                      default:
                        goto DumpTypeRecord;
                    }
                  p += 1;

                  if (p + 1 > pend)
                    goto DumpTypeRecord;

                  fprintf(ostream, " FcnType:");
                  switch (*(BYTE *)p)
                    {
                      case 0:
                        fprintf(ostream, "regular");
                        break;
                      case 1:
                        fprintf(ostream, "constructor");
                        break;
                      case 2:
                        fprintf(ostream, "destructor");
                        break;
                      default:
                        goto DumpTypeRecord;
                    }
                  p += 1;

                  if (p + 2 > pend)
                    goto DumpTypeRecord;

                  if (BadRecord = Prt_HLF_Index2(&p, pend, "FcnInfo"))
                    goto DumpTypeRecord;

                  /* see if we have the virtual bit set                       */
                  if (TypeQual & 0x10)
                    {
                      if (p + 4 > pend)
                        goto DumpTypeRecord;

                      if (BadRecord = Prt_HLF_Span(&p, pend,
                                                 "\n                  VirtNo"))
                        goto DumpTypeRecord;

                      if (BadRecord = Prt_HLF_String2(&p, pend, "Name"))
                        goto DumpTypeRecord;
                    }
                  else
                    {
                      if (BadRecord = Prt_HLF_String2(&p, pend,
                                                     "\n                  Name"))
                        goto DumpTypeRecord;
                    }

                  #ifdef STATS
                  TypeStringSize[Type] += TempStringSize;
                  #endif /* STATS */
                }
              else
                {
                  if (TypeQual & 0x01)
                    fprintf(ostream, " Args(Pushed R->L");
                  else
                    fprintf(ostream, " Args(Pushed L->R");

                  if (TypeQual & 0x02)
                    fprintf(ostream, ", Caller Pops");
                  else
                    fprintf(ostream, ", Callee Pops");

                  switch (TypeQual & 0x0C)
                    {
                      case 0x00:
                        fprintf(ostream, " offset-16");
                        break;
                      case 0x04:
                        fprintf(ostream, " offset-32");
                        break;
                      case 0x08:
                        fprintf(ostream, " segment-32");
                        break;
                      case 0x0C:
                        fprintf(ostream, " segment-48");
                        break;
                    }

                  if (TypeQual & 0x10)
                    fprintf(ostream, ", var num parms");
                  else
                    fprintf(ostream, ", fixed num parms");

                  if (TypeQual & 0x20)
                    fprintf(ostream, ",\n                  OS/2 call conv)");
                  else
                    fprintf(ostream, ",\n                  priv call conv)");

                  if (p + 1 > pend)
                    goto DumpTypeRecord;

                  fprintf(ostream, " Prot:");
                  switch (*p)
                    {
                      case 0:
                        fprintf(ostream, "private");
                        break;
                      case 1:
                        fprintf(ostream, "protected");
                        break;
                      case 2:
                        fprintf(ostream, "public");
                        break;
                      default:
                        goto DumpTypeRecord;
                    }
                  p += 1;

                  if (p + 1 > pend)
                    goto DumpTypeRecord;

                  fprintf(ostream, " FcnType:");
                  switch (*(BYTE *)p)
                    {
                      case 0:
                        fprintf(ostream, "regular");
                        break;
                      case 1:
                        fprintf(ostream, "constructor");
                        break;
                      case 2:
                        fprintf(ostream, "destructor");
                        break;
                      default:
                        goto DumpTypeRecord;
                    }
                  p += 1;

                  if (p + 4 > pend)
                    goto DumpTypeRecord;

                  fprintf(ostream, " VirtNo:%lu", *(DWORD *)p);
                  p += 4;

                  if (p + 1 > pend)
                    goto DumpTypeRecord;

                  fprintf(ostream, "\n                  Attr(");
                  {
                    BYTE  attr;
                    attr = *(BYTE *)p;

                    if (attr & 0x01)
                      fprintf(ostream, "static");
                    else
                      fprintf(ostream, "non-static");

                    if (attr & 0x02)
                      fprintf(ostream, ", inline");
                    else
                      fprintf(ostream, ", non-inline");

                    if (attr & 0x04)
                      fprintf(ostream, ", const");
                    else
                      fprintf(ostream, ", non-const");

                    if (attr & 0x08)
                      fprintf(ostream, ", volatile");
                    else
                      fprintf(ostream, ", non-volatile");

                    if (attr & 0x10)
                      fprintf(ostream, "\n                  virtual)");
                    else
                      fprintf(ostream, "\n                  non-virtual)");
                  }
                  p += 1;

                  if (p + 2 > pend)
                    goto DumpTypeRecord;

                  fprintf(ostream, " Args:%u", *(WORD *)p);
                  p += 2;

                  if (p + 2 > pend)
                    goto DumpTypeRecord;

                  fprintf(ostream, " MaxArgs:%u", *(WORD *)p);
                  p += 2;

                  if (BadRecord = Prt_HLF_Index(&p, pend, "RetType"))
                    goto DumpTypeRecord;

                  if (BadRecord = Prt_HLF_Index(&p, pend, "TypeList"))
                    goto DumpTypeRecord;

                  if (BadRecord = Prt_HLF_String(&p, pend,
                                                 "\n                  Name"))
                    goto DumpTypeRecord;

                  #ifdef STATS
                  TypeStringSize[Type] += TempStringSize;
                  #endif /* STATS */

                  if (BadRecord = Prt_HLF_Text(&p, pend, "Body"))
                    goto DumpTypeRecord;
                }
              break;
          case HLT_MEMBER_POINTER:      /* member pointer                     */
              if (TypeQual & 0x01)
                fprintf(ostream, " has-vbases");
              else
                fprintf(ostream, " no-vbases");

              if (TypeQual & 0x02)
                fprintf(ostream, " mult-inh");
              else
                fprintf(ostream, " no-mult-inh");

              if (TypeQual & 0x04)
                fprintf(ostream, " const");
              else
                fprintf(ostream, " non-const");

              if (TypeQual & 0x08)
                fprintf(ostream, " volatile");
              else
                fprintf(ostream, " non-volatile");

              if (BadRecord = Prt_HLF_Index(&p, pend,
                                            "\n                  ChildType"))
                goto DumpTypeRecord;

              if (BadRecord = Prt_HLF_Index2(&p, pend, "ClassType"))
                goto DumpTypeRecord;

              if (BadRecord = Prt_HLF_Index2(&p, pend, "RepType"))
                goto DumpTypeRecord;
              break;
          case HLT_PICTURE:            /* picture                            */
              switch (TypeQual)
                {
                  case 0x00:
                    fprintf(ostream, " (char)");

                    if (p + 2 > pend)
                      goto DumpTypeRecord;

                    fprintf(ostream, " Len:%u", *(WORD *)p);
                    p += 2;
                    break;
                  case 0x01:
                    /* don't do this type yet */
                    fprintf(ostream, " (dec arith)");
                    break;
                  default:
                    /*
                    ** bad type qualifier, move pointer back and display
                    ** from type qualifier to the end of the record
                    */
                    p--;
                    break;
                }
              break;
#endif
          case HLT_POINTER:            /* pointer                            */
              switch (TypeQual & 0x03)
                {
                  case 0x00:
                    fprintf(ostream, " offset-16");
                    break;
                  case 0x01:
                    fprintf(ostream, " offset-32");
                    break;
                  case 0x02:
                    fprintf(ostream, " segment-32");
                    break;
                  case 0x03:
                    fprintf(ostream, " segment-48");
                    break;
                }

              if (BadRecord = Prt_HLF_Index(&p, pend, "Type"))
                goto DumpTypeRecord;
              BadRecord = Prt_HLF_String(&p, pend, "Name");
              break;
#if 0
          case HLT_REFERENCE:          /* reference                          */
              if (BadRecord = Prt_HLF_Index2(&p, pend, "Type"))
                goto DumpTypeRecord;
              break;

          case HLT_SCALAR:             /* scalar                             */
              if (TypeQual & 0x01)
                fprintf(ostream, " Packed");
              else
                fprintf(ostream, " Unpacked");

              if (TypeQual & 0x02)
                fprintf(ostream, " Complex");
              else
                fprintf(ostream, " Real");

              if (TypeQual & 0x04)
                fprintf(ostream, " Float");
              else
                fprintf(ostream, " Fixed");

              if (TypeQual & 0x08)
                fprintf(ostream, " Decimal");
              else
                fprintf(ostream, " Binary");

              if (BadRecord = Prt_HLF_Index(&p, pend, "Type"))
                goto DumpTypeRecord;
              BadRecord = Prt_HLF_Precision(&p, pend, "Precision",
                                            "\n                  ScaleFactor");
              break;
          case HLT_SET:                /* set                                */
              if (BadRecord = Prt_HLF_Index(&p, pend, "Type"))
                goto DumpTypeRecord;
              BadRecord = Prt_HLF_String(&p, pend, "Name");

              #ifdef STATS
              TypeStringSize[Type] += TempStringSize;
              #endif /* STATS */
              break;
          case HLT_STACK:              /* stack                              */
              switch (TypeQual & 0x03)
                {
                  case 0x00:
                    fprintf(ostream, " offset-16");
                    break;
                  case 0x01:
                    fprintf(ostream, " offset-32");
                    break;
                  case 0x02:
                    fprintf(ostream, " segment-32");
                    break;
                  case 0x03:
                    fprintf(ostream, " segment-48");
                    break;
                }

              if (p + 4 > pend)
                goto DumpTypeRecord;

              fprintf(ostream, " Bytes:%lu", *(DWORD *)p);
              p += 4;

              BadRecord = Prt_HLF_String(&p, pend, "Name");

              #ifdef STATS
              TypeStringSize[Type] += TempStringSize;
              #endif /* STATS */
              break;
#endif
          case HLT_STRUCTURE:          /* structure                          */
              p--;
              fprintf(ostream, " Unpacked");

              if (p + 4 > pend)
                goto DumpTypeRecord;

              fprintf(ostream, " Bytes:%lu", *(DWORD *)p);
              p += 4;

              if (p + 2 > pend)
                goto DumpTypeRecord;

              fprintf(ostream, " N:%u", *(WORD *)p);
              p += 2;

              if (BadRecord = Prt_HLF_Index(&p, pend, "TypeList"))
                goto DumpTypeRecord;
              if (BadRecord = Prt_HLF_Index(&p, pend, "NameList"))
                goto DumpTypeRecord;
              BadRecord = Prt_HLF_String(&p, pend,
                                         "\n                  Name");
              break;
#if 0
          case HLT_SUBRANGE:           /* subrange                           */
              if (BadRecord = Prt_HLF_Index(&p, pend, "Type"))
                goto DumpTypeRecord;
              if (BadRecord = Prt_HLF_Span(&p, pend, "Start"))
                goto DumpTypeRecord;
              if (BadRecord = Prt_HLF_Span(&p, pend, "End"))
                goto DumpTypeRecord;
              BadRecord = Prt_HLF_String(&p, pend, "Name");

              #ifdef STATS
              TypeStringSize[Type] += TempStringSize;
              #endif /* STATS */
              break;
#endif
          case HLT_USERDEF:            /* user-defined                       */
              p--;
              if (BadRecord = Prt_HLF_Index(&p, pend, "Type"))
                goto DumpTypeRecord;
              BadRecord = Prt_HLF_String(&p, pend, "Name");
              break;

          case HLT_SKIP:               /* Skip                               */
              p--;
              fprintf(ostream, " New Index : %d", *(ushort *)p);
              TypeIndex = *(ushort *)p;
              p += 2;
              break;

          default:
              fprintf(ostream, "\n");
              break;
        }

DumpTypeRecord:
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
          ** 1.01
          */
          if (o.opt.x)
            {
              fprintf(ostream, "                  ");
              if (DebugVersion >= 0x03)
                dumphbuf(TempBufPtr+2, totn, 18);
              else
                dumphbuf(TempBufPtr+3, totn, 18);
              fprintf(ostream, "\n\n");
            }
        }

SkipTypeRecord:
        TempBufPtr = (uchar *)NextHLTypeRec2(TempBufPtr);
    }
  fclose( ostream );
}

/****** Prt_HLF_Span *********************************************************/
/*                                                                           */
/*  Description:                                                             */
/*      Based on the value of the HLF_span, one, two, or four uchars of      */
/*      signed or unsigned value is extracted.  The pointer pointing at the  */
/*      FID's is advanced to the next field.                                 */
/*                                                                           */
/*  Parameters:                                                              */
/*      uchar **SubRecPtr           pointer at beginning of field            */
/*                                                                           */
/*  Returns:                                                                 */
/*      long    Value               the value extracted                      */
/*      0                           Error encountered                        */
/*                                                                           */
/*****************************************************************************/

  static uint
Prt_HLF_Span (uchar **SubRecPtr, uchar *SubRecEnd, uchar *SpanName, uchar size)
{
  if (*SubRecPtr > SubRecEnd)
    return(BADRECORD);

  if (*SubRecPtr == SubRecEnd)
    return(GOODRECORD);

  switch( size )
    {
      case 1:
        fprintf(ostream, " %s:%hd", SpanName, (short)*(signed char *)(*SubRecPtr));
        (*SubRecPtr) += 1;
        return(GOODRECORD);

      case 2:
        fprintf(ostream, " %s:%hd", SpanName, *(short *)(*SubRecPtr));
        (*SubRecPtr) += 2;
        return(GOODRECORD);

      case 4:
        fprintf(ostream, " %s:%hd", SpanName, *(long *)(*SubRecPtr));
        (*SubRecPtr) += 4;
        return(GOODRECORD);
    }
}

/****** Prt_HLF_Text *********************************************************/
/*                                                                           */
/*  Description:                                                             */
/*      based on the value of the HLF_span, one, two, or four uchars of      */
/*      signed or unsigned value is extracted.  the pointer pointing at the  */
/*      FID's is advanced to the next field.                                 */
/*                                                                           */
/*  Parameters:                                                              */
/*      uchar **SubRecPtr           pointer to start of field                */
/*                                                                           */
/*  Returns:                                                                 */
/*      uint                        good/bad record flag                     */
/*                                                                           */
/*  Global Data:                                                             */
/*                                                                           */
/*****************************************************************************/

  static uint
Prt_HLF_Text (uchar **SubRecPtr, uchar *SubRecEnd, uchar *SpanName)
{
  int  SpanLen;

  if (*SubRecPtr > SubRecEnd)
    return(BADRECORD);

  if (*SubRecPtr == SubRecEnd)
    return(GOODRECORD);

  switch (*((uchar*)*SubRecPtr))
    {
      case HLF_SPAN_8S:
        /* make sure we won't exceed the record */
        if ((*SubRecPtr + 2) > SubRecEnd)
          return(BADRECORD);

        if (*(char *)(*SubRecPtr+1) < 0)
          return(BADRECORD);

        if ((*SubRecPtr + 2 + *(char *)(*SubRecPtr+1)) > SubRecEnd)
          return(BADRECORD);

        SpanLen = *(char *)(*SubRecPtr+1);

        fprintf(ostream, " %s:%.*s", SpanName, SpanLen, (*SubRecPtr+2));
        (*SubRecPtr) += (2 + *(char *)(*SubRecPtr+1));
        return(GOODRECORD);
      case HLF_SPAN_16S:
        /* make sure we won't exceed the record */
        if ((*SubRecPtr + 3) > SubRecEnd)
          return(BADRECORD);

        if (*(short *)(*SubRecPtr+1) < 0)
          return(BADRECORD);

        if ((*SubRecPtr + 3 + *(short *)(*SubRecPtr+1)) > SubRecEnd)
          return(BADRECORD);

        SpanLen = *(short *)(*SubRecPtr+1);

        fprintf(ostream, " %s:%.*s", SpanName, SpanLen, (*SubRecPtr+3));
        (*SubRecPtr) += (3 + *(short *)(*SubRecPtr+1));
        return(GOODRECORD);
      case HLF_SPAN_32S:
        /* make sure we won't exceed the record */
        if ((*SubRecPtr + 5) > SubRecEnd)
          return(BADRECORD);

        if (*(long *)(*SubRecPtr+1) < 0)
          return(BADRECORD);

        if ((*SubRecPtr + 5 + *(long *)(*SubRecPtr+1)) > SubRecEnd)
          return(BADRECORD);

        SpanLen = *(long *)(*SubRecPtr+1);

        fprintf(ostream, " %s:%.*s", SpanName, SpanLen, (*SubRecPtr+5));
        (*SubRecPtr) += (5 + *(long *)(*SubRecPtr+1));
        return(GOODRECORD);
      case HLF_SPAN_8U:
        /* make sure we won't exceed the record */
        if ((*SubRecPtr + 2) > SubRecEnd)
          return(BADRECORD);

        if ((*SubRecPtr + 2 + *(uchar *)(*SubRecPtr+1)) > SubRecEnd)
          return(BADRECORD);

        SpanLen = *(uchar *)(*SubRecPtr+1);

        fprintf(ostream, " %s:%.*s", SpanName, SpanLen, (*SubRecPtr+2));
        (*SubRecPtr) += (2 + *(uchar *)(*SubRecPtr+1));
        return(GOODRECORD);
      case HLF_SPAN_16U:
        /* make sure we won't exceed the record */
        if ((*SubRecPtr + 3) > SubRecEnd)
          return(BADRECORD);

        if ((*SubRecPtr + 3 + *(ushort *)(*SubRecPtr+1)) > SubRecEnd)
          return(BADRECORD);

        SpanLen = *(ushort *)(*SubRecPtr+1);

        fprintf(ostream, " %s:%.*s", SpanName, SpanLen, (*SubRecPtr+3));
        (*SubRecPtr) += (3 + *(ushort *)(*SubRecPtr+1));
        return(GOODRECORD);
      case HLF_SPAN_32U:
        /* make sure we won't exceed the record */
        if ((*SubRecPtr + 5) > SubRecEnd)
          return(BADRECORD);

        if ((*SubRecPtr + 5 + *(ulong *)(*SubRecPtr+1)) > SubRecEnd)
          return(BADRECORD);

        SpanLen = *(ulong *)(*SubRecPtr+1);

        fprintf(ostream, " %s:%.*s", SpanName, SpanLen, (*SubRecPtr+5));
        (*SubRecPtr) += (5 + *(ulong *)(*SubRecPtr+1));
        return(GOODRECORD);
      default:
        /* we didn't get a Fid Span */
        return(BADRECORD);
    }
}

/****** Prt_HLF_Index ********************************************************/
/*                                                                           */
/*  Description:                                                             */
/*                                                                           */
/*  Parameters:                                                              */
/*      uchar **SubRecPtr           pointer at beginning of field            */
/*                                                                           */
/*  Returns:                                                                 */
/*                                                                           */
/*****************************************************************************/

  static uint
Prt_HLF_Index (uchar **SubRecPtr, uchar *SubRecEnd, uchar *IndexName)
{
  if (*SubRecPtr > SubRecEnd)
    return(BADRECORD);

  if (*SubRecPtr == SubRecEnd)
    return(GOODRECORD);

  /* make sure we won't exceed the record */
  if ((*SubRecPtr + 2) > SubRecEnd)
    return(BADRECORD);

  fprintf(ostream, " %s:", IndexName);
  PrintTypeIndex(*(ushort *)(*SubRecPtr));
  (*SubRecPtr) += 2;
  return(GOODRECORD);

}

/****** Prt_HLF_Index2 *******************************************************/
/*                                                                           */
/*  Description:                                                             */
/*                                                                           */
/*  Parameters:                                                              */
/*      uchar **SubRecPtr           pointer to start of field                */
/*                                                                           */
/*  Returns:                                                                 */
/*      uint                        good/bad record flag                     */
/*                                                                           */
/*  Global Data:                                                             */
/*                                                                           */
/*****************************************************************************/

  static uint
Prt_HLF_Index2 (uchar **SubRecPtr, uchar *SubRecEnd, uchar *IndexName)
{
  if (*SubRecPtr > SubRecEnd)
    return(BADRECORD);

  if (*SubRecPtr == SubRecEnd)
    return(GOODRECORD);

  /* make sure we won't exceed the record */
  if ((*SubRecPtr + 2) > SubRecEnd)
    return(BADRECORD);

  fprintf(ostream, " %s:", IndexName);
  PrintTypeIndex(*(ushort *)(*SubRecPtr));
  (*SubRecPtr) += 2;
  return(GOODRECORD);
}

/****** Prt_HLF_String *******************************************************/
/*                                                                           */
/*  Description:                                                             */
/*                                                                           */
/*  Parameters:                                                              */
/*      uchar  **SubRecPtr          pointer at beginning of field            */
/*      MTETAB  *pMteTabEntry       module table entry pointer               */
/*                                                                           */
/*  Returns:                                                                 */
/*                                                                           */
/*****************************************************************************/

  static uint
Prt_HLF_String (uchar **SubRecPtr, uchar *SubRecEnd, uchar *StringName)
{
  ushort NameSpan;

  #ifdef STATS
  TempStringSize = 0;
  #endif /* STATS */

  if (*SubRecPtr > SubRecEnd)
    return(BADRECORD);

  if (*SubRecPtr == SubRecEnd)
    return(GOODRECORD);

  if (NameSpan = *(ushort *)(*SubRecPtr))
    {
      /* make sure we won't exceed the record */
      if ((*SubRecPtr + NameSpan + 2) > SubRecEnd)
        return(BADRECORD);

      if (NameSpan <= MAXNAMELENX)
        fprintf(ostream, " %s:%.*s", StringName, NameSpan, *SubRecPtr+2);
      else
        fprintf(ostream, " %s:%.*s", StringName, MAXNAMELENX, *SubRecPtr+2);
    }
  else
    fprintf(ostream, " NoName");

  *SubRecPtr += NameSpan + 2;
  return(GOODRECORD);
}

/****** Prt_HLF_String2 ******************************************************/
/*                                                                           */
/*  Description:                                                             */
/*                                                                           */
/*  Parameters:                                                              */
/*      uchar  **SubRecPtr          pointer to start of field                */
/*      MTETAB  *pMteTabEntry       module table entry pointer               */
/*                                                                           */
/*  Returns:                                                                 */
/*      uint                        good/bad record flag                     */
/*                                                                           */
/*  Global Data:                                                             */
/*                                                                           */
/*****************************************************************************/

  static uint
Prt_HLF_String2 (uchar **SubRecPtr, uchar *SubRecEnd, uchar *StringName)
{
  uchar  NameSpan;
  uchar  NameLen = 1;

  if (DebugVersion < 0x03)
    return(Prt_HLF_String(SubRecPtr, SubRecEnd, StringName));

  #ifdef STATS
  TempStringSize = 0;
  #endif /* STATS */

  if (*SubRecPtr > SubRecEnd)
    return(BADRECORD);

  if (*SubRecPtr == SubRecEnd)
    return(GOODRECORD);

  if (NameSpan = **SubRecPtr)
    {
      if (NameSpan & 0x80)
        {
          if (*SubRecPtr + 1 > SubRecEnd)
            return(BADRECORD);

          NameSpan = ((NameSpan & 0x80) << 8) + *(*SubRecPtr + 1);
          NameLen++;
        }

      /* make sure we won't exceed the record                                 */
      if ((*SubRecPtr + NameSpan + NameLen) > SubRecEnd)
        return(BADRECORD);

      if (NameSpan <= MAXNAMELENX)
        fprintf(ostream, " %s:%.*s", StringName, NameSpan, *SubRecPtr+NameLen);
      else
        fprintf(ostream, " %s:%.*s", StringName, MAXNAMELENX, *SubRecPtr+NameLen);
    }
  else
    fprintf(ostream, " NoName");

  #ifdef STATS
  TempStringSize = NameSpan;
  if (NameSpan < 128)
    ShortTypeStrings++;
  #endif /* STATS */

  *SubRecPtr += NameSpan + NameLen;
  return(GOODRECORD);
}

/****** Prt_HLF_Precision****************************************************/
/*                                                                          */
/*  Description:                                                            */
/*                                                                          */
/*  Parameters:                                                             */
/*      uchar **SubRecPtr           pointer at beginning of field           */
/*                                                                          */
/*  Returns:                                                                */
/*                                                                          */
/****************************************************************************/

  static uint
Prt_HLF_Precision (uchar **SubRecPtr, uchar *SubRecEnd, uchar *PrecisionName,
                   uchar *ScaleName)
{
  uchar  Precision;

  if (*SubRecPtr > SubRecEnd)
    return(BADRECORD);

  if (*SubRecPtr == SubRecEnd)
    return(GOODRECORD);

  if (*((uchar *)*SubRecPtr) == HLF_PRECISION)
    {
      /* make sure we won't exceed the record */
      if ((*SubRecPtr + 3) > SubRecEnd)
        return(BADRECORD);

      fprintf(ostream, " %s:%u", PrecisionName, *(uchar *)(*SubRecPtr+1));
      fprintf(ostream, " %s:%hd", ScaleName, (short)(*(uchar *)(*SubRecPtr+2) - 128));
      *SubRecPtr += 3;
      return(GOODRECORD);
    }
  else
    {
      /* we didn't get a Fid Precision */
      return(BADRECORD);
    }
}

/****** PrintTypeIndex *******************************************************/
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
PrintTypeIndex (ushort n)
{

  typedef struct recsymnames
    {
      int     code;
      char    name[20];
    };

static struct recsymnames
       PrimTypeNames[9] = {
           0x80,   "CHAR",
           0x81,   "SHORT",
           0x82,   "LONG",
           0x84,   "UCHAR",
           0x85,   "USHORT",
           0x86,   "ULONG",
           0x88,   "FLOAT",
           0x89,   "DOUBLE",
           0x97,   "VOID"
  };

  uchar *cp;
  ushort baseN, primN, i;

  baseN = (n & 0x0060) >> 5;
  primN = n & 0xFF9F;

  for (i=0; i<9; i++)
    {
      if (primN == PrimTypeNames[i].code)
        {
          fprintf(ostream, "%s", PrimTypeNames[i].name);
          switch (baseN)
            {
              case 0x01:
                fprintf(ostream, "* NEAR");
                break;
              case 0x02:
                fprintf(ostream, "* FAR");
                break;
              case 0x03:
                fprintf(ostream, "* HUGE");
                break;
            }
          return;
        }
    }

  fprintf(ostream, "%u", n);
}
