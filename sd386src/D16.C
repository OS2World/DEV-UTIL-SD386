/*****************************************************************************/
/* Get16BitDosExitEntry                                                      */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Get the DosExit entry point for a 16 bit application.                   */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   mte            This is the module handle for DOSCALLS.DLL               */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   usrc           Just a return code.                                      */
/*   DosExitEntryPt The sel:off address of the 16 bit DosExit entry.         */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   mte is valid.                                                           */
/*                                                                           */
/*                                                                           */
/* Notes:                                                                    */
/*                                                                           */
/* These are the options used to compile this module.                        */
/*                                                                           */
/* cl  /c /Zi /U_DATA /U_TEXT -G2s /MD -Od -W3 -AL -NTCODX -NDDATX  $*.c     */
/*                                                                           */
/* I used MSC 6.00a, but you could use C/2 1.1                               */
/*                                                                           */
/*****************************************************************************/
typedef unsigned long ulong;
typedef unsigned short ushort;

#define DOSEXITORDINAL16BIT   5

ushort  Get16BitDosExitEntry(ulong);
ulong   DosExitEntryPt16;
ushort  pascal far DosGetProcAddr(ushort, void *, void *);

ushort Get16BitDosExitEntry(ulong mte)
{
 ulong   ordinal;
 ushort  usrc;

 ordinal = DOSEXITORDINAL16BIT;

 usrc =  DosGetProcAddr((ushort)mte,(void *)ordinal,(void*)&DosExitEntryPt16);
 return(usrc);
}
