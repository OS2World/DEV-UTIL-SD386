/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   asminit.c                                                               */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Initialize for debugging at assembler level.                            */
/*                                                                           */
/* History:                                                                  */
/*                                                                           */
/*   12/17/93 Rewritten.                                                     */
/*                                                                           */
/**Includes*******************************************************************/

#include "all.h"                        /* SD86 include files                */

/*****************************************************************************/
/* AsmInit()                                                                 */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*  Initialize a dll/exe for debugging at assembler level only.              */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*  pdf       input - the EXE/DLL structure.                                 */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*  pdf != NULL.                                                             */
/*                                                                           */
/*****************************************************************************/
void asminit ( DEBFILE *pdf )
{
 uint           NumCodeObjs;
 OBJTABLEENTRY *te;
 MODULE         *mptr;
 MODULE         *lptr;
 int             i;
 uint           *p;

 /****************************************************************************/
 /* - build a module chain based on the code objects.                        */
 /****************************************************************************/
 pdf->MidAnchor = NULL;
 lptr = (MODULE *)&pdf->MidAnchor;
 NumCodeObjs = *(p=pdf->CodeObjs);
 te = (OBJTABLEENTRY *)++p;
 for(i=1; i <= NumCodeObjs; i++,te++ )
 {
  if(te->ObjType == CODE )
  {
   mptr          = (MODULE *)Talloc(sizeof(MODULE)) ;
   mptr->pCsects = (CSECT*)Talloc(sizeof(CSECT));

   mptr->pCsects->pModule = mptr;

   mptr->mid = i;
   mptr->pCsects->SegFlatAddr = te->ObjLoadAddr;
   mptr->pCsects->CsectLo     = te->ObjLoadAddr;
   mptr->pCsects->CsectHi     = te->ObjLoadAddr + te->ObjLoadSize - 1;
   mptr->pCsects->CsectSize   = te->ObjLoadSize;
   lptr->NextMod = mptr;
   lptr = mptr;
  }
 }
}
