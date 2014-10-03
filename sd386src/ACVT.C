/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   acvt.c                                                                  */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Converts flat address to sel:off form and vice versa.                   */
/*                                                                           */
/*   !!!!use these for data conversions only!!!!                             */
/*   !!!!call dosdebug to convert code addresses!!!!                         */
/*                                                                           */
/*                                                                           */
/* History:                                                                  */
/*                                                                           */
/*****************************************************************************/
#include "all.h"

#define Flat2Sel(p)         ((SEL)((HiFlat(p)<<3) | 7))

void Data_Flat2SelOff(ULONG addr,USHORT *pSelector,USHORT *pOffset)
{
 USHORT Selector = 0;
 USHORT Offset   = 0;


 Code_Flat2SelOff(addr, &Selector, &Offset);
 if( Selector == 0 )
 {
  *pSelector = Flat2Sel( addr );
  *pOffset   = LoFlat  ( addr );
 }
}

#define SelOff2Flat(s,o)    (((ULONG)(((USHORT)s)>>3))<<16)|(ULONG)o

ULONG Data_SelOff2Flat (USHORT Selector, USHORT Offset)
{
 ULONG    addr = 0;

 addr = Code_SelOff2Flat(Selector, Offset);
 if( addr == 0 )
  addr = SelOff2Flat( Selector, Offset );

 return( addr );
}

void Code_Flat2SelOff(ULONG addr,USHORT *pSelector,USHORT *pOffset)
{
 DEBFILE *pdf;
 ULONG    LoadAddr;
 USHORT   LoadSel;
 USHORT   LoadOff;

 *pSelector = 0;
 *pOffset   = 0;

 pdf = FindExeOrDllWithAddr(addr);
 if( pdf )
  if(MapFlatAddrToBase(pdf, addr, &LoadAddr, &LoadSel, &LoadOff) == 0 )
  {
   *pSelector = LoadSel;
   *pOffset = LoadOff + (USHORT)(addr - LoadAddr);
  }
}

ULONG Code_SelOff2Flat (ushort Selector,ushort Offset)
{
 DEBFILE *pdf;
 ULONG    addr = 0;
 ULONG    LoadAddr;
 USHORT   LoadOff;

 pdf = FindExeOrDllWithSelOff(Selector,Offset);
 if( pdf )
  if( MapSelOffToBase(pdf, Selector, Offset, &LoadAddr, &LoadOff) == 0)
   addr = LoadAddr + (Offset - LoadOff);

 return( addr );
}
