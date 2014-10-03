/*****************************************************************************/
/* File:                                                                     */
/*   exe.h                                                                   */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   EXE  include stuff                                                      */
/*                                                                           */
/* History:                                                                  */
/*                                                                           */
/*   04/06/95 Updated.                                                       */
/*                                                                           */
/*****************************************************************************/

#define    LFO_OF_EXE_HEADER_LFO 0x3C   /* Offset of where to find the       */
                                        /* offset of the start of the exe    */
                                        /* header.                           */

/*****************************************************************************/
/* Executable type flags.                                                    */
/*****************************************************************************/
#define    NE     1                     /* 16 bit executable OS/2 1.x        */
#define    LX     2                     /* 32 bit executable OS/2 2.x        */

/*****************************************************************************/
/* - Some general bitness flags.                                             */
/*****************************************************************************/
#define    BITUNKNOWN  0
#define    BIT16       1
#define    BIT32       2

/*****************************************************************************/
/* Flags for the NBxx debug information signature.                           */
/*****************************************************************************/
#define    NB00     1
#define    NB02     2
#define    NB04     3

/*****************************************************************************/
/* - HLL signature in the FileName records.                                  */
/*****************************************************************************/
#define    HLL    0x4C48                /* debug style value "HL"            */

/*****************************************************************************/
/* - Directory subsection types.                                             */
/*****************************************************************************/
#define    SSTMODULES   0x0101          /* File Name.                        */
#define    SSTPUBLICS   0x0102          /* Public Symbols.                   */
#define    SSTTYPES     0x0103          /* Types.                            */
#define    SSTSYMBOLS   0x0104          /* Symbols.                          */
#define    SSTLIBRARIES 0x0106          /* Libraries.                        */
#define    SSTSRCLINES  0x0105          /* Line Numbers - (IBM C/2 1.1)      */
#define    SSTSRCLNSEG  0x0109          /* Line Numbers - (MSC 6.00)         */
#define    SSTIBMSRC    0x010B          /* Line Numbers - IBM HLL            */

/*****************************************************************************/
/* - Public symbol types                                                     */
/*****************************************************************************/
#define    TYPE_PUB_16  1
#define    TYPE_PUB_32  2

/*****************************************************************************/
/* Flags for the line number sections.                                       */
/*****************************************************************************/
#define    TYPE109_16     1             /* 16 bit MS/(C211 linked link386)   */
#define    TYPE109_32     2             /* 32 bit MSC.                       */
#define    TYPE105        3             /* IBM C211                          */
#define    TYPE10B_HL01   4             /* IBM HLL Level 1                   */
#define    TYPE10B_HL02   5             /* IBM HLL Level 2                   */
#define    TYPE10B_HL03   6             /* IBM HLL Level 3                   */
#define    TYPE10B_HL04   7             /* IBM HLL Level 4                   */

/*****************************************************************************/
/* Flags for the symbols sections.                                           */
/*****************************************************************************/
#define    TYPE104_C211   1
#define    TYPE104_C600   2
#define    TYPE104_HL01   3
#define    TYPE104_HL02   4
#define    TYPE104_CL386  5
#define    TYPE104_HL03   6
#define    TYPE104_HL04   7

/*****************************************************************************/
/* Flags for the types sections.                                             */
/*****************************************************************************/
#define    TYPE103_C211   1
#define    TYPE103_C600   2
#define    TYPE103_HL01   3
#define    TYPE103_HL02   4
#define    TYPE103_CL386  5
#define    TYPE103_HL03   6
#define    TYPE103_HL04   7

/*****************************************************************************/
/* - Structures defining the entries in the debug information directory.     */
/*****************************************************************************/
typedef struct _hdrentry
{
 USHORT  SubSectType;
 USHORT  ModIndex;
 ULONG   SubSectOff;
 USHORT  SubSectLen;
}HDRENTRY;

typedef struct _hdrentryhll
{
 USHORT  SubSectType;
 USHORT  ModIndex;
 ULONG   SubSectOff;
 ULONG   SubSectLen;
}HDRENTRYHLL;

/*****************************************************************************/
/* - Structures defining the entries in the filename records/linked modules. */
/*****************************************************************************/
typedef struct _frec_nb00_lx
{
 USHORT  SegNum;                        /* containing segment number         */
 ULONG   ImOff;                         /* offset into the load image        */
 ULONG   ImLen;                         /* length of load image              */
 USHORT  overlay;                       /* not used                          */
 USHORT  LibNum;                        /* what library it came from         */
 char    NoSegs;                        /* Number of segments                */
 char    Reserved;                      /* 1 byte pad                        */
/* char   *fnlength;*/                  /* filename length                   */
/* char    fname[]; */                  /* filename                          */
}FREC_NB00_LX;

typedef struct _frec_nb04_segobject
{
 USHORT  SegNum;                        /* containing segment number         */
 ULONG   ImOff;                         /* offset into the load image        */
 ULONG   ImLen;                         /* length of load image              */
}FREC_NB04_SEGOBJECT;

typedef struct _frec_nb04
{
 FREC_NB04_SEGOBJECT SegObject;         /* description of a segment object.  */
 USHORT              overlay;           /* not used                          */
 USHORT              LibNum;            /* what library it came from         */
 UCHAR               NoSegs;            /* Number of segments.               */
 char                reserved;          /* 1 byte pad                        */
 USHORT              DebugStyle;        /* debug style                       */
 UCHAR               version[2];        /* version (debug)                   */
 UCHAR               fnlength;          /* filename length                   */
/* char              fname[]; */        /* filename                          */
}FREC_NB04;

typedef struct _frec_ne_segobject
{
 USHORT  SegNum;                        /* containing segment number         */
 USHORT  ImOff;                         /* offset into the load image        */
 USHORT  ImLen;                         /* length of load image              */
}FREC_NE_SEGOBJECT;

typedef struct _frec_ne
{
 FREC_NE_SEGOBJECT SegObject;           /* description of a segment object.  */
 USHORT            overlay;             /* not used                          */
 USHORT            LibNum;              /* what library it came from         */
 UCHAR             NoSegs;              /* Number of segments                */
 char              Reserved;            /* 1 byte pad                        */
 UCHAR             fnlength;            /* filename length                   */
/* char    fname[]; */                  /* filename                          */
}FREC_NE;

/*****************************************************************************/
/* - Public symbol records.                                                  */
/*****************************************************************************/
typedef struct
{
 ULONG   Offset;
 USHORT  ObjNum;
 USHORT  Typeid;
 UCHAR   Namelen;
}PUBREC32;

typedef struct
{
 union
 {
  ULONG FlatAddr;
  struct
  {
   USHORT Offset;
   USHORT ObjNum;
  }RawAddr;
 }Pub16Addr;
 USHORT  Typeid;
 UCHAR   Namelen;
}PUBREC16;

#define NextPubRec16(p) ( (UCHAR *)(p)+sizeof(PUBREC16)-1 + *((UCHAR *)(p)+sizeof(PUBREC16)-1) + 1)
#define NextPubRec32(p) ( (UCHAR *)(p)+sizeof(PUBREC32)-1 + *((UCHAR *)(p)+sizeof(PUBREC32)-1) + 1)

/*****************************************************************************/
/* - Line number records.                                                    */
/*****************************************************************************/

/*****************************************************************************/
/* - First Entry record entry types. There are others but they're not        */
/*   supported.                                                              */
/*****************************************************************************/
#define FILE_NAMES_TABLE_ENTRY_TYPE_HL04  0x03

#define LINE_NUMBER_TABLE_ENTRY_TYPE_HL01 0x00
#define LINE_NUMBER_TABLE_ENTRY_TYPE_HL02 LINE_NUMBER_TABLE_ENTRY_TYPE_HL01
#define LINE_NUMBER_TABLE_ENTRY_TYPE_HL03 LINE_NUMBER_TABLE_ENTRY_TYPE_HL01
#define LINE_NUMBER_TABLE_ENTRY_TYPE_HL04 LINE_NUMBER_TABLE_ENTRY_TYPE_HL01

/*****************************************************************************/
/* - Line number data structures.                                            */
/*****************************************************************************/
typedef struct _sourcefilenametype105
{
 UCHAR   SourceFileNameLength;
}SOURCE_FILE_NAME_TYPE105;

typedef struct _109_16tableheader
{
 USHORT SegNum;
 USHORT NumEntries;
}FIRST_ENTRY_109_16;

typedef struct _109_16tableentry
{
 USHORT LineNumber;
 USHORT Offset;
}LINE_NUMBER_TABLE_ENTRY_109_16;

typedef struct _105tableheader
{
 USHORT NumEntries;
}FIRST_ENTRY_105;

typedef struct _105tableentry
{
 USHORT LineNumber;
 USHORT Offset;
}LINE_NUMBER_TABLE_ENTRY_105;

typedef struct _109_32tableheader
{
 USHORT SegNum;
 USHORT NumEntries;
}FIRST_ENTRY_109_32;

typedef struct _109_32tableentry
{
 USHORT LineNumber;
 ULONG  Offset;
}LINE_NUMBER_TABLE_ENTRY_109_32;

/*****************************************************************************/
/* - Pertinent HL01 structures                                               */
/*****************************************************************************/
typedef struct _firstentryhl01
{
 USHORT LineNumber;
 UCHAR  EntryType;
 UCHAR  Reserved;
 USHORT NumEntries;
 USHORT PathTableEntries;
}FIRST_ENTRY_HL01;

typedef struct _filenametableentryhl01
{
 ULONG  StartOfSource;
 ULONG  NumberOfSourceRecords;
 ULONG  NumberOfSourceFiles;
 UCHAR  SourceFileNameLength;           /* first source file name.           */
}FILE_NAME_TABLE_ENTRY_HL01;

typedef struct _linenumbertableentryhl01
{
 USHORT  LineNumber;
 UCHAR   SourceFileIndex;
 UCHAR   Flags;
 ULONG   Offset;
}LINE_NUMBER_TABLE_ENTRY_HL01;

typedef struct _pathtableeentryhl01
{
 ULONG  Offset;
 USHORT PathCode;
}PATH_TABLE_ENTRY_HL01;

/*****************************************************************************/
/* - Pertinent HL03 structures.(HL02 was never released.)                    */
/*****************************************************************************/

typedef struct _firstentryhl03
{
 USHORT LineNumber;
 UCHAR  EntryType;
 UCHAR  Reserved;
 USHORT NumEntries;
 USHORT SegNum;
 ULONG  FileNameTableSize;
}FIRST_ENTRY_HL03;

typedef struct _filenametableentryhl03
{
 ULONG  StartOfSource;
 ULONG  NumberOfSourceRecords;
 ULONG  NumberOfSourceFiles;
 UCHAR  SourceFileNameLength;           /* first source file name.           */
}FILE_NAME_TABLE_ENTRY_HL03;

typedef struct _linenumbertableentryhl03
{
 USHORT LineNumber;
 USHORT SourceFileIndex;
 ULONG  Offset;
}LINE_NUMBER_TABLE_ENTRY_HL03;

/*****************************************************************************/
/* - Pertinent HL04 structures.                                              */
/*****************************************************************************/
typedef struct _headerhl04
{
 char   junk[8];
 ULONG  FileNameTableSize;
}HEADER_HL04;

typedef struct _firstentryhl04
{
 USHORT LineNumber;
 UCHAR  EntryType;
 UCHAR  Reserved;
 USHORT NumEntries;
 USHORT SegNum;
 ULONG  BaseOffset;
}FIRST_ENTRY_HL04;

#define FILE_NAME_TABLE_ENTRY_HL04 FILE_NAME_TABLE_ENTRY_HL01

typedef struct _linenumbertableentryhl04
{
 USHORT  LineNumber;
 USHORT  SourceFileIndex;
 ULONG   Offset;
}LINE_NUMBER_TABLE_ENTRY_HL04;

typedef struct _csectmap
{
 struct _csectmap *next;
 struct _csect    *pCsect;
 void             *pFirstEntry;
 USHORT            NumEntries;
 int               mssfi;               /* sfi index contrived for ms types. */
 BOOL              IsFileUnique;
}CSECTMAP;

typedef struct _filenametablemap
{
 struct _filenametablemap *next;
 void                     *pFirstEntry;
}FILE_NAME_TABLE_MAP;

/*****************************************************************************/
/* - Internal Table Entry.                                                   */
/*****************************************************************************/
typedef struct
{
 int    sfi;
 USHORT lno;
 ULONG  off;
}LINNUM_ENTRY;

typedef struct _filename
{
 struct _filename *next;
 int               sfi;
 char              FileName[1];         /* length prefixed z-string.         */
}FILENAME;

#define LNOTAB LINNUM_ENTRY

typedef struct
{
 USHORT  lno;
 USHORT  off;
}LNOTAB16;

typedef struct
{
 USHORT  lno;
 UCHAR   srcfileind;
 UCHAR   flags;
 ULONG  off;
}LNOTABHLL;

typedef struct _sring
{
 struct _sring *next;
 ULONG          mid;
 ULONG          symlen;
 SSRec         *symptr;
}SYMNODE;

typedef struct _typenode
{
 struct _typenode *next;
 ULONG             mid;
 ULONG             typlen;
 UCHAR            *typptr;
}TYPENODE;

typedef struct _lring
{
 struct _lring *next;
 ULONG          mid;
 ULONG          tablents;
 UINT          *ltabptr;
}LNONODE;

typedef struct _pubnode
{
 struct _pubnode *next;
 ULONG            mid;
 ULONG            publen;
 UCHAR           *pubptr;
}PUBNODE;

/*****************************************************************************/
/* - Structure(s) and constants defining a linked module(.obj).              */
/*****************************************************************************/
#define    FAKEMID    0xffff

typedef struct _csect
{
 struct _csect *next;
 struct  MOD   *pModule;                /* -> to parent module.              */
 USHORT         SegNum;
 ULONG          SegFlatAddr;
 ULONG          CsectLo;
 ULONG          CsectHi;
 ULONG          CsectSize;
 ULONG          NumEntries;             /* lnotab entries.                   */
 LNOTAB        *pLnoTab;
}CSECT;

typedef struct _modflags
{
 int   LnoTabLoaded :1;                 /* line number tables are loaded.    */
 int   reserved     :31;
}MOD_FLAGS;

typedef struct MOD
{
 struct  MOD       *NextMod;
 struct _DEBFILE   *pdf;                /* -> back to owning exe/dll.        */
 ULONG              mid;
 MOD_FLAGS          modflags;
 ULONG              FileName;
 ULONG              FileNameLen;
 ULONG              Publics;
 ULONG              PubLen;
 ULONG              TypeDefs;
 ULONG              TypeLen;
 ULONG              Symbols;
 ULONG              SymLen;
 ULONG              LineNums;
 ULONG              LineNumsLen;
 CSECT             *pCsects;
 FILENAME          *pFiles;
 struct
 {
  UCHAR  Pubs;
  UCHAR  Syms;
  UCHAR  Lins;
  UCHAR  Typs;
 } DbgFormatFlags;
}MODULE;

/*****************************************************************************/
/* - Structure(s) defining an executable(EXE/DLL).                           */
/*****************************************************************************/
typedef struct _exeflags
{
 int   ExeType   :2;        /* flag indicating NE(16-bit) or LX(32-bit)       */
 int   NBxxType  :2;        /* NB00, NB02, or NB04.                           */
}EXE_FLAGS;

typedef struct
{
 HFILE fh;
 char *fn;
}MFILE;

#define SOURCE_LEVEL 0
#define ASM_LEVEL    1
typedef struct _DEBFILE
{
 struct   _DEBFILE *next;
 MFILE    *DebFilePtr;
 UINT      pid;
 UINT      mte;
 UCHAR     SrcOrAsm;                    /* source level = 0; asm level = 1   */
 ULONG     DebugOff;                    /* lfo of debug info.                */
 MODULE   *MidAnchor;
 SYMNODE  *psyms;
 TYPENODE *ptyps;
 PUBNODE  *ppubs;
 UINT     *CodeObjs;
 ULONG     EntryExitPt;
 EXE_FLAGS ExeFlags;
}DEBFILE;

typedef DEBFILE EXEFILE;

/*****************************************************************************/
/* - structure defining a process.                                           */
/*****************************************************************************/
typedef struct _process_node
{
 UINT              pid;
 UINT              sid;
 struct _DEBFILE  *ExeStruct;
 struct BRKstruct *allbrks;
}PROCESS_NODE;

/*****************************************************************************/
/* - some defines/structures used to support the alloc_text pragma.          */
/*****************************************************************************/
typedef struct
{
 USHORT  SegNum;
 USHORT  ImOff;
 USHORT  ImLen;
}SEGREC16;
