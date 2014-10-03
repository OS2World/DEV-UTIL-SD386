/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY813*/
/*   mapsyms.h                                                               */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Record type definitions for symbols mapping functions.                  */
/*                                                                           */
/* History:                                                                  */
/*...                                                                        */
/*... 03/01/93  813   Selwyn    Changes for HL02/HL03.                       */
/*****************************************************************************/

/*****************************************************************************/
/* HLL symbol structures.                                                    */
/*****************************************************************************/
typedef struct                          /* Type Number : 0x00               */
{
  uint   BlockOffset;
  uint   BlockLen;
} SSBeginHLL;

typedef struct                          /* Type Number : 0x01               */
{
  uint   ProcOffset;
  ushort TypeIndex;
  uint   ProcLen;
  ushort DebugStart;
  uint   DebugEnd;
  ushort Reserved;
  uchar  NearFar;
  varstr Name;
} SSProcHLL;

typedef struct                          /* Type Number : 0x04               */
{
  int    FrameOffset;
  ushort TypeIndex;
  varstr Name;
} SSDefHLL;

typedef struct                          /* Type Number : 0x05               */
{
  uint   Offset;
  ushort ObjectNum;
  ushort TypeIndex;
  varstr Name;
} SSVarHLL;

typedef struct                          /* Type Number : 0x0D               */
{
  ushort TypeIndex;
  uchar  RegNum;
  varstr Name;
} SSRegHLL;

typedef struct                          /* Type Number : 0x11               */
{
  ushort SegNum;
  ushort Reserved;
} SSChgDefHLL;

typedef struct                          /* Type Number : 0x12               */
{
  ushort TypeIndex;
  varstr Name;
} SSUserDefHLL;

/*****************************************************************************/
/* MSC C 6.0/IBM C2 1.1 16 bit structures.                                   */
/*****************************************************************************/

typedef struct                          /* Type Number : 0x00               */
{
  uchar  RecLen;
  uchar  RecType;
  ushort BlockOffset;
  ushort BlockLen;
} SSBegin16;

typedef struct                          /* Type Number : 0x01               */
{
  uchar  RecLen;
  uchar  RecType;
  ushort ProcOffset;
  ushort TypeIndex;
  ushort ProcLen;
  ushort DebugStart;
  ushort DebugEnd;
  ushort Reserved;
  uchar  NearFar;
  varstr Name;
} SSProc16;

typedef struct                          /* Type Number : 0x04               */
{
  uchar  RecLen;
  uchar  RecType;
  ushort FrameOffset;
  ushort TypeIndex;
  varstr Name;
} SSDef16;

typedef struct                          /* Type Number : 0x05               */
{
  uchar  RecLen;
  uchar  RecType;
  ushort Offset;
  ushort ObjectNum;
  ushort TypeIndex;
  varstr Name;
} SSVar16;

typedef struct                          /* Type Number : 0x0D               */
{
  uchar  RecLen;
  uchar  RecType;
  ushort TypeIndex;
  uchar  RegNum;
  varstr Name;
} SSReg16;

typedef struct                          /* Type Number : 0x11               */
{
  uchar  RecLen;
  uchar  RecType;
  ushort SegNum;
  ushort Reserved;
} SSChgDef16;

typedef struct                          /* Type Number : 0x12               */
{
  uchar  RecLen;
  uchar  RecType;
  ushort TypeIndex;
  varstr Name;
} SSUserDef16;

typedef struct                          /* Type Number : 0x02               */
{
  uchar RecLen;
  uchar RecType;
} SSEnd16;

/*****************************************************************************/
/* MSC C 6.0 32 bit structures.                                              */
/*****************************************************************************/
#define SSBEGIN32   0x80
#define SSPROC32    0x81
#define SSEND32     0x82
#define SSDEF32     0x84
#define SSVAR32     0x85
#define SSREG32     0x8D
#define SSCHGDEF32  0x91
#define SSUSERDEF32 0x12

#define SSREGBX32 19                    /*                                605*/
#define SSREGSI32 22                    /*                                605*/
#define SSREGDI32 23                    /*                                605*/

typedef struct                          /* Type Number : 0x80               */
{
  uchar  RecLen;
  uchar  RecType;
  uint   BlockOffset;
  ushort BlockLen;
} SSBegin32;

typedef struct                          /* Type Number : 0x81               */
{
  uchar  RecLen;
  uchar  RecType;
  uint   ProcOffset;
  ushort TypeIndex;
  ushort ProcLen;
  ushort DebugStart;
  ushort DebugEnd;
  ushort Reserved;
  uchar  NearFar;
  varstr Name;
} SSProc32;

typedef struct                          /* Type Number : 0x84               */
{
  uchar  RecLen;
  uchar  RecType;
  int    FrameOffset;
  ushort TypeIndex;
  varstr Name;
} SSDef32;

typedef struct                          /* Type Number : 0x85               */
{
  uchar  RecLen;
  uchar  RecType;
  uint   Offset;
  ushort ObjectNum;
  ushort TypeIndex;
  varstr Name;
} SSVar32;

typedef struct                          /* Type Number : 0x8D               */
{
  uchar  RecLen;
  uchar  RecType;
  ushort TypeIndex;
  uchar  RegNum;
  varstr Name;
} SSReg32;

typedef struct                          /* Type Number : 0x91               */
{
  uchar  RecLen;
  uchar  RecType;
  ushort SegNum;
  ushort Reserved;
} SSChgDef32;

typedef struct                          /* Type Number : 0x92               */
{
  uchar  RecLen;
  uchar  RecType;
  ushort TypeIndex;
  varstr Name;
} SSUserDef32;

typedef struct                          /* Type Number : 0x82               */
{
  uchar RecLen;
  uchar RecType;
} SSEnd32;

#define SSproc   void                   /*                                101*/

typedef struct
{
  uchar RecLen;
  uchar RecType;
} SSrec;

