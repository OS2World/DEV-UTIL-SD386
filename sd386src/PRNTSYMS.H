#include "all.h"

void   dumptext( char *, UINT);
void   dumpline( char *, UINT);
void   dumphbuf( char *, UINT, UINT);
void   dumphlin( char *, UINT);
void   dumphex( UCHAR);
void   PrintRegType( UINT);

typedef struct recsymnames
  {
    int     code;
    char    name[20];
  };

#define NoOfHLSRecs  8

/* symbol type codes */
static struct recsymnames
   recordsym[NoOfHLSRecs] = {
       SSBEGIN,           "Begin     ",
       SSPROC,            "Proc      ",
       SSEND,             "End       ",
       SSDEF,             "Auto      ",
       SSVAR,             "Static    ",
       SSREG,             "Reg       ",
       SSCHGDEF,          "ChDfSeg   ",
       SSUSERDEF,         "TypeDef   "
  };


#define NextHLSymRec(p) ((char *)(p) + RecordLen + totn)
