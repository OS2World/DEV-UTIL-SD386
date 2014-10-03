typedef unsigned char  BYTE;
typedef unsigned short WORD;
typedef unsigned long  DWORD;
/*****************************************************************************/
/*  HLL Format Type Information                                              */
/*****************************************************************************/

/* High-Level FID definitions */
#define   HLF_NIL         0x80
#define   HLF_VOID        0x81
#define   HLF_STRING      0x82
#define   HLF_INDEX       0x83
#define   HLF_SPAN_16U    0x85
#define   HLF_SPAN_32U    0x86
#define   HLF_SPAN_8S     0x88
#define   HLF_SPAN_16S    0x89
#define   HLF_SPAN_32S    0x8A
#define   HLF_SPAN_8U     0x8B
#define   HLF_PRECISION   0x8C

/* High-Level Type definitions */
#define   HLT_AREA            0x55
#define   HLT_ARRAY           0x78
#define   HLT_BASECLASS       0x41
#define   HLT_BITSTRING       0x5C
#define   HLT_CHARSTRING      0x60
#define   HLT_CLASS           0x40
#define   HLT_CLASS_DEF       0x43
#define   HLT_CLASS_MEMBER    0x46
#define   HLT_CODELABEL       0x72
#define   HLT_ENTRY           0x53
#define   HLT_ENUM            0x7B
#define   HLT_FILE            0x67
#define   HLT_FORMATLABEL     0x65
#define   HLT_FRIEND          0x42
#define   HLT_FUNCTION        0x54
#define   HLT_GRAPHIC         0x62
#define   HLT_LIST            0x7F
#define   HLT_LOGICAL         0x56
#define   HLT_MACRO           0x59
#define   HLT_MEMBER_FUNCTION 0x45
#define   HLT_MEMBER_POINTER  0x49
#define   HLT_PICTURE         0x61
#define   HLT_POINTER         0x7A
#define   HLT_PROCEDURE       0x75
#define   HLT_REFERENCE       0x48
#define   HLT_SCALAR          0x51
#define   HLT_SET             0x52
#define   HLT_STACK           0x57
#define   HLT_STRUCTURE       0x79
#define   HLT_SUBRANGE        0x6F
#define   HLT_USERDEF         0x5D
#define   HLT_SKIP            0x90

typedef struct
  { BYTE    recid;                     /* 01                                 */
    WORD    reclen;
    BYTE    rectype;
    BYTE    recqual;
    BYTE    rectext[1];                /* variable length                    */
  } HLTrec;

typedef struct
  { WORD    reclen;
    BYTE    rectype;
    BYTE    recqual;
    BYTE    rectext[1];                /* variable length                    */
  } HLTrec2;

#define NextHLTypeRec(p)  ((HLTrec *)  ((char *)(p)+((HLTrec *)(p))->reclen+3))
#define NextHLTypeRec2(p) ((HLTrec2 *) ((char *)(p)+((HLTrec2 *)(p))->reclen+2))

typedef struct rectypenames
  {
    int     code;
    char    name[20];
  };

#define NoOfHLTRecs  31

/* record type codes */
static struct rectypenames
   recordtype[NoOfHLTRecs] = {
       HLT_AREA,            "Area     ",
       HLT_ARRAY,           "Array    ",
       HLT_BASECLASS,       "BaseClass",
       HLT_BITSTRING,       "BitString",
       HLT_CHARSTRING,      "CharStr  ",
       HLT_CLASS,           "Class    ",
       HLT_CLASS_DEF,       "ClassDef ",
       HLT_CLASS_MEMBER,    "ClassMem ",
       HLT_CODELABEL,       "CodeLabel",
       HLT_ENTRY,           "Entry    ",
       HLT_ENUM,            "Enum     ",
       HLT_FILE,            "File     ",
       HLT_FORMATLABEL,     "FmtLabel ",
       HLT_FRIEND,          "Friend   ",
       HLT_FUNCTION,        "Procedure",
       HLT_GRAPHIC,         "Graphic  ",
       HLT_LIST,            "List     ",
       HLT_LOGICAL,         "Logical  ",
       HLT_MACRO,           "Macro    ",
       HLT_MEMBER_FUNCTION, "MemberFcn",
       HLT_MEMBER_POINTER,  "MemberPtr",
       HLT_PICTURE,         "Picture  ",
       HLT_POINTER,         "Pointer  ",
       HLT_PROCEDURE,       "Procedure",
       HLT_REFERENCE,       "Reference",
       HLT_SCALAR,          "Scalar   ",
       HLT_SET,             "Set      ",
       HLT_STACK,           "Stack    ",
       HLT_STRUCTURE,       "Structure",
       HLT_SUBRANGE,        "Subrange ",
       HLT_USERDEF,         "UserDef  ",
  };

