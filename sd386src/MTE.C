/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   mte.c                                                                822*/
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*  Handle mte table functions.                                              */
/*                                                                           */
/* History:                                                                  */
/*                                                                           */
/*... 01/29/93 Created.                                                      */
/*... 05/06/93  822   Joe       Add mte table handling.                      */
/*... 09/09/93  900   Joe       Fix for trap when  ...\EXE\.. in path name.  */
/*...                                                                        */
/*****************************************************************************/
#include "all.h"

#define NextMod(p)  ((UCHAR*)p+*(UINT*)p + sizeof(UINT) )
#define NextObj(p) ( ((UCHAR*)p+2*sizeof(UINT)) + *((UCHAR*)p+2*sizeof(UINT)) \
                     + sizeof(UINT) )

static MTE_TABLE_ENTRY *pMteTable;
static UINT             TableSize;

extern PROCESS_NODE *pnode;

/*****************************************************************************/
/* UpdateMteTable()                                                          */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Add information to the mte table.                                       */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   pModuleTable input - -> to a module table of dll load information.      */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   void* pMteTable                                                         */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
void *UpdateMteTable( UINT *pModuleTable )
{

 UINT   NumModules;

 MODULE_LOAD_ENTRY      *pMteEntry;
 OBJTABLEENTRY  *pObjectTable;
 int             ModIndex;

 /****************************************************************************/
 /* First scan the module table to see if there are any module frees. If     */
 /* there have been, then scan the current mte table and mark the freed      */
 /* entries. These entries may be cleaned up later if the table needs to     */
 /* expand. Also, dump the data strucures allocated for this mte.            */
 /****************************************************************************/
 NumModules = *pModuleTable++;

 pMteEntry = (MODULE_LOAD_ENTRY*)pModuleTable;
 for(ModIndex=1;ModIndex<=NumModules;ModIndex++,
            pMteEntry=(MODULE_LOAD_ENTRY*)NextMod(pMteEntry) )
 {
  if(strlen(pMteEntry->ModuleName) == 0 )
  {
   MarkTheFreedMtes(pMteEntry->mte);
   FreeMte(pMteEntry->mte);
  }
 }

 /****************************************************************************/
 /* Now, scan the module table and update the mte table.                     */
 /****************************************************************************/
 pMteEntry = (MODULE_LOAD_ENTRY*)pModuleTable;
 for(ModIndex=1;ModIndex<=NumModules;ModIndex++ ,
            pMteEntry=(MODULE_LOAD_ENTRY*)NextMod(pMteEntry) )
 {
  if(strlen(pMteEntry->ModuleName) != 0 )
  {
   pObjectTable = (OBJTABLEENTRY *)NextObj(pMteEntry);
   AddObjectsToMteTable( pMteEntry->mte, pObjectTable );
  }
 }
/*DumpMteTable( pMteTable );*/
 return((void*)pMteTable );
}

/*****************************************************************************/
/* MarkTheFreedMtes()                                                        */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Mark the entries in the module table as freed. We simply replace        */
/*   the mtes with a 0.                                                      */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   mte          input - the target executable module table handle.         */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   void                                                                    */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/

void MarkTheFreedMtes( UINT mte )
{
 MTE_TABLE_ENTRY *pMteTableEntry;

 /****************************************************************************/
 /* Scan the mte table and zero out all the mte entries for this mte.        */
 /****************************************************************************/
 for(pMteTableEntry = pMteTable ; pMteTableEntry->mte != ENDOFTABLE;
             pMteTableEntry++ )
  if(pMteTableEntry->mte == mte)
   pMteTableEntry->mte = 0;

}

/*****************************************************************************/
/* AddMtesToTable()                                                          */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Update the mte table with the entries from an object table.             */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   pObjectTable input - -> to a Module  table module table handle.         */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   void                                                                    */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   The mte table pointer, pMteTable, will be NULL on the first             */
/*   entry to this function.                                                 */
/*                                                                           */
/*****************************************************************************/
void AddObjectsToMteTable( UINT mte, OBJTABLEENTRY *pObjectTable )
{
 MTE_TABLE_ENTRY *pMteTableEntry;
 UINT NumObjects;
 int  i;

 /****************************************************************************/
 /*  - Get the number of objects to be added to the table.                   */
 /*  - Bump the object table pointer past the NumObjects field.              */
 /****************************************************************************/
 NumObjects = *(UINT*)pObjectTable;
 pObjectTable = (OBJTABLEENTRY*)((UCHAR*)pObjectTable + sizeof(UINT));

tryagain:
 /****************************************************************************/
 /* Scan the mte table to see if this mte is already there. If it is         */
 /* then just don't put it in. We will also come here after the mte          */
 /* table has been expanded.                                                 */
 /****************************************************************************/
 for(pMteTableEntry = pMteTable ;
                      pMteTable &&
                      ( pMteTableEntry->mte != ENDOFTABLE);
                      pMteTableEntry++ )
  if(pMteTableEntry->mte == mte)
   return;

 /****************************************************************************/
 /* At this point, pMteTableEntry is pointing to the ENDOFTABLE entry.       */
 /*                                                                          */
 /* - If there's room in the table, then add the objects.                    */
 /* - If there isn't then go expand the table and come back.                 */
 /****************************************************************************/
 if( (TableSize - (pMteTableEntry - pMteTable)) > NumObjects )
 {
  for( i=1;i<=NumObjects;pObjectTable++,pMteTableEntry++ , i++ )
  {
   pMteTableEntry->mte      = mte;
   pMteTableEntry->ObjNum   = pObjectTable->ObjNum;
   pMteTableEntry->LoadAddr = pObjectTable->ObjLoadAddr;
  }
  pMteTableEntry->mte = ENDOFTABLE;
 }
 else
 /****************************************************************************/
 /* - Expand the table to hold the growth.                                   */
 /* - If there is an old table ( i.e. not the first time through ) then      */
 /*    - copy the new table to the expanded block.                           */
 /*    - free the old table.                                                 */
 /* - If it's the first time through, then mark the end of the table.        */
 /* - Update the table pointer and go back and try again.                    */
 /****************************************************************************/
 {
  MTE_TABLE_ENTRY *ptr;
  UINT             size;

  size = TableSize;
  TableSize += TABLE_INCREMENT;
  ptr = (MTE_TABLE_ENTRY*)Talloc(TableSize*sizeof(MTE_TABLE_ENTRY) );
  if(pMteTable)
  {
   memcpy(ptr, pMteTable, size*sizeof(MTE_TABLE_ENTRY) );
   Tfree(pMteTable);
  }
  if( pMteTable == NULL)
   ptr->mte = ENDOFTABLE;
  pMteTable = ptr;
  goto tryagain;
 }
}

/*****************************************************************************/
/* ExeDllInit()                                                              */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Initialize the data structures for module loads.                        */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   pModuleTable input - -> to a module table of dll load information.      */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
APIRET ExeDllInit( UINT *pModuleTable )
{
 UINT            NumModules;
 MODULE_LOAD_ENTRY      *pMteEntry;
 OBJTABLEENTRY  *pObjectTable;
 int             ModIndex;
 char           *pModuleName;
 UINT            NumObjects;
 UINT            ObjectTableLength;
 UCHAR          *p;
 APIRET          rc = 0;

 NumModules = *pModuleTable++;
 /****************************************************************************/
 /* - Scan the module table.                                                 */
 /* - If we're debugging remote, then we have to find the image of the       */
 /*   executable.                                                            */
 /* - If we can't find the image, the free up allocated storage and go on.   */
 /*                                                                          */
 /****************************************************************************/
 pMteEntry = (MODULE_LOAD_ENTRY*)pModuleTable;
 for(ModIndex=1;ModIndex<=NumModules;ModIndex++ ,
            pMteEntry=(MODULE_LOAD_ENTRY*)NextMod(pMteEntry) )
 {
  if(strlen(pMteEntry->ModuleName) != 0 )
  {
   pModuleName = pMteEntry->ModuleName;
   /**************************************************************************/
   /* Allocate space for the object table. The module table will be          */
   /* freed eventually so we have to get the object table out of it          */
   /* before it goes away.                                                   */
   /**************************************************************************/
   pObjectTable = (OBJTABLEENTRY *)NextObj(pMteEntry);
   NumObjects = *(UINT*)pObjectTable;
   ObjectTableLength = NumObjects*sizeof(OBJTABLEENTRY) + sizeof(UINT);
   p = Talloc(ObjectTableLength);
   memcpy(p,pObjectTable,ObjectTableLength);
   pObjectTable = (OBJTABLEENTRY*)p;

   /**************************************************************************/
   /* - the module name will already have been converted to upper case.   900*/
   /**************************************************************************/
   if( strstr(pModuleName,".EXE") != NULL)                              /*900*/
   {
    rc = exeinit(pMteEntry->mte,pModuleName,pObjectTable);
    if( rc != 0 )
     Error(rc,TRUE,1,pModuleName);
   }
   else
   {
    rc = dllinit(pMteEntry->mte,pModuleName,pObjectTable);
    if( (rc == TRAP_ADDR_LOAD) ||
        (rc == TRAP_DLL_LOAD)
      )
     return(rc);

    if( rc != 0 )
     Error(rc,TRUE,1,pModuleName);
   }
  }
 }
 return(0);
}
/*****************************************************************************/
/* GetLoadAddr()                                                             */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Get the load address of an object number in an mte.                     */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   mte          input - mte of the module the object is in.                */
/*   ObjectNumber input - the object number. duh.                            */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   The base address of the Object or 0 if the ObjNum is not in the table.  */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
ULONG GetLoadAddr( ULONG mte, UINT ObjectNumber)
{
 MTE_TABLE_ENTRY *pMteTableEntry;
 ULONG            LoadAddr = 0;

 /****************************************************************************/
 /*  - Scan the mte table to find this mte and object number.                */
 /****************************************************************************/
 for(pMteTableEntry = pMteTable ;
                      pMteTable &&
                      ( pMteTableEntry->mte != ENDOFTABLE);
                      pMteTableEntry++ )
 {
  if( (pMteTableEntry->mte == mte) &&
      (pMteTableEntry->ObjNum == ObjectNumber)
    )
  {
   LoadAddr = pMteTableEntry->LoadAddr;
   break;
  }
 }
 /****************************************************************************/
 /* We come here with ObjNum=0. One time this happens is when the FileName   */
 /* record in the debug info has an ObjNum=0. Here's a typical dump:         */
 /*                                                                          */
 /* Absolute       Code          Length    SSTLib   Debug       Module       */
 /* Address     Seg Offset        (0x)     Index    Style    Index  Name     */
 /* ========   =============    ========   ======   =====   ================ */
 /*    .             .             .          .      .              .        */
 /*    .             .             .          .      .              .        */
 /* 00001123     0000:0000        0000        1     None     12 DOSGETDBCSEV */
 /*                                                                          */
 /*                                                                          */
 /****************************************************************************/

 return(LoadAddr);
}

/*****************************************************************************/
/* FreeMteTable()                                                            */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Free the mte table.                                                     */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   none                                                                    */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   none                                                                    */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
void FreeMteTable( void )
{
 if( pMteTable )
 {
  Tfree( pMteTable );
  pMteTable = NULL;
  TableSize = 0;
 }
}

/*****************************************************************************/
/* CopySharedMteTab()                                                        */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Free the mte table.                                                     */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   none                                                                    */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   none                                                                    */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
void CopySharedMteTab( ULONG *pMteTabShared )
{
 ULONG len;

 len = *pMteTabShared++;
 TableSize = *pMteTabShared++;
 len -= 12;

 pMteTable = Talloc(len);
 memcpy( pMteTable, pMteTabShared, len);
}

/*****************************************************************************/
/* FreeMte()                                                                 */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Free the data structures allocated for this mte and take the            */
/*   structures out of the list.                                             */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   mte   the mte we're freeing up.                                         */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   void                                                                    */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
void FreeMte( HMODULE mte )
{
 DEBFILE *pdf;
 DEBFILE *pdfprev;
 BRK     *p;
 BRK     *pnext;

 pdf     = pnode->ExeStruct->next;
 pdfprev = pnode->ExeStruct;
 for (;pdf;pdfprev = pdf, pdf = pdf->next )
 {
  if (pdf->mte == mte )
  {
   pdfprev->next = pdf->next;
   freepdf(pdf);
   Tfree((void*)pdf);
   break;
  }
 }

 /****************************************************************************/
 /* - now scan the list of breakpoints and remove any breakpoints            */
 /*   defined for this mte.                                                  */
 /****************************************************************************/
 for( p = pnode->allbrks; p ; )
 {
  pnext = p->next;
  if( p->mte == mte )
  {
   if(p->flag.DefineType == BP_LOAD_ADDR )
   {
    p->flag.Reported = 0;
    p->mte           = 0;
   }
   else
    UndBrk(p->brkat, TRUE);
  }
  p = pnext;
 }
}
