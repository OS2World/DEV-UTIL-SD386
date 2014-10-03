/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   xsrvacvt.c                                                              */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Converts flat address to sel:off form and vice versa using dosdebug.    */
/*   !!!!use these to convert code addresses!!!!                             */
/*                                                                           */
/* History:                                                                  */
/*                                                                           */
/*...Release 1.00 (03/03/92)                                                 */
/*...                                                                        */
/*****************************************************************************/
#include "all.h"

void Sys_Flat2SelOff(ULONG Address,USHORT *Selector,USHORT *Offset)
{
   PtraceBuffer  Ptb;
   memset(&Ptb,0,sizeof(Ptb));

   Ptb.Pid  = GetEspProcessID();
   Ptb.Cmd  = DBG_C_LinToSel;
   Ptb.Addr = Address;
   DosDebug(&Ptb);
   *Selector = (ushort)Ptb.Value;
   *Offset   = (ushort)Ptb.Index;
}

ULONG Sys_SelOff2Flat(USHORT Selector,USHORT Offset)
{
   PtraceBuffer  Ptb;
   memset(&Ptb,0,sizeof(Ptb));

   Ptb.Pid   = GetEspProcessID();
   Ptb.Cmd   = DBG_C_SelToLin;
   Ptb.Value = Selector;
   Ptb.Index = Offset;
   DosDebug(&Ptb);
   return( Ptb.Addr );
}
