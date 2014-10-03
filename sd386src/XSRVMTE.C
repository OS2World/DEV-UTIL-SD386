/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   srvrmte.c                                                            822*/
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*  Mte table handling functions.                                            */
/*                                                                           */
/* History:                                                                  */
/*                                                                           */
/*   05/04/93 Created.                                                       */
/*                                                                           */
/*...Release 1.04 (04/30/93)                                                 */
/*...                                                                        */
/*... 05/04/93  822   Joe       Add mte table handling.                      */
/*...                                                                        */
/*****************************************************************************/
#include "all.h"

/*****************************************************************************/
/* AddToModuleLoadTable()                                                    */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Add a module load notification link to a linked list of                 */
/*   notifications. There will be a link for each load and free.             */
/*                                                                           */
/*             --------       --------                                       */
/*      ----->|        |---->|        |----                                  */
/*            | next   |     | next   |    |                                 */
/*             --------       --------     -                                 */
/*            | sizeof |     | sizeof |                                      */
/*            | entry  |     | entry  |                                      */
/*             --------       --------     -                                 */
/*            |        |     |        |                                      */
/*            | Table  |     | Table  |                                      */
/*            | Entry  |     | Entry  |                                      */
/*            |(see    |     |        |                                      */
/*            | below) |     |        |                                      */
/*            |        |     |        |                                      */
/*             --------       --------                                       */
/*                                                                           */
/*   Table Entries look like so:                                             */
/*                                                                           */
/*   Note: "record length" does not include the length of the                */
/*         "record length" field.                                            */
/*                                                                           */
/*     4        4     4              var            var                      */
/*    --------------------------------------//-----------//-------           */
/*   |        |      |             |              |               |          */
/*   | record | mte  | module name | module name  | object table  |          */
/*   | length |      | length      |              |               |          */
/*    --------------------------------------//-----------//-------           */
/*   |        |      |             |              |               |          */
/*   | record | mte  | module name | module name  | object table  |          */
/*   | length |      | length      |              |               |          */
/*    --------------------------------------//-----------//-------           */
/*        .                                                                  */
/*        .                                                                  */
/*        .                                                                  */
/*                                                                           */
/*                                                                           */
/*   The object table will look like so:                                     */
/*                                                                           */
/*     4            4        4          4          1     1                   */
/*    ------------------------------------------------------------           */
/*   | Number of  |        |          |          |      |         |          */
/*   | objects in | Object | LoadAddr | LoadSize | Type | Bitness |          */
/*   | the table  | Number |          |          |      |         |          */
/*    ------------------------------------------------------------           */
/*                |        |          |          |      |         |          */
/*                | Object | LoadAddr | LoadSize | Type | Bitness |          */
/*                | Number |          |          |      |         |          */
/*                 -----------------------------------------------           */
/*                   .                                                       */
/*                   .                                                       */
/*                   .                                                       */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*  ppModuleList input - Address of a pointer to the module list.            */
/*  Pid          input - Process id owning the mte.                          */
/*  mte          input - EXE/DLL handle of freed dll.                        */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   A link will be added to the end of the list pointed to by               */
/*   ppModuleList.                                                           */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*  Pid and mte are valid.                                                   */
/*                                                                           */
/*****************************************************************************/
void AddToModuleLoadTable( UINT **ppModuleList, ULONG Pid, ULONG mte)
{
 char  ModuleName[CCHMAXPATH];
 FILE *fpModule;
 UINT *pObjectTable;
 UINT *ptr;
 UINT *p;
 int  NumObjects;
 int  SizeOfObjectTable;
 int  ModuleNameLength;
 int  TableSize;

 /****************************************************************************/
 /* - Get the module name.                                                   */
 /* - Open the module and if it fails then we'll simply proceed              */
 /*   as if we never received the notification.                              */
 /* - Get the object table.                                                  */
 /****************************************************************************/
 DosQueryModuleName( mte,CCHMAXPATH,ModuleName);
 fpModule = fopen( ModuleName,"rb");
 if( fpModule == NULL )
  return;
 pObjectTable = GetObjectTable( fpModule, Pid, mte );
 if(pObjectTable )
 {

  /***************************************************************************/
  /* - Add the table to the x-server list of tables.                         */
  /***************************************************************************/
  AddObjTableToList( pObjectTable,mte);

  /***************************************************************************/
  /* We're going to add a table to the linked list of module tables.         */
  /* - Get the size of the module table.                                     */
  /* - Get the length of the module name + 1 for z-string termination.       */
  /* - Get the table size not including the "next" pointer or the            */
  /*   size of the rest of the table.                                        */
  /* - Allocate the storage needed for the table.                            */
  /* - Fill in the table entries.                                            */
  /* - Get a ptr to the last module table in the list.                       */
  /* - Add the table to the list of tables.                                  */
  /***************************************************************************/
  NumObjects = *pObjectTable;
  SizeOfObjectTable =  sizeof(NumObjects) + NumObjects*sizeof(OBJTABLEENTRY);
  ModuleNameLength = strlen(ModuleName);
  ModuleNameLength += 1;

  TableSize = 4 +                       /* size of mte.                      */
              4 +                       /* size of module name length.       */
              ModuleNameLength +        /* length of the module name.        */
              SizeOfObjectTable;        /* size of object table. duh!        */

  p=ptr=Talloc(TableSize +
               4 +                      /* size of next->.                   */
               4 );                     /* size of length of rest of table.  */

  *(p)++ = NULL;                        /* ground the node.                  */
  *p++ = TableSize;                     /* size of the rest of the table.    */
  *p++ = mte;
  *p++ = ModuleNameLength;
  strcpy((char*)p,ModuleName);
  p = (UINT*)( (UCHAR*)p + ModuleNameLength);
  memcpy(p,pObjectTable,SizeOfObjectTable);
  Tfree(pObjectTable);
  for(p=(UINT*)ppModuleList;*p!=NULL;p= (UINT*)*p){;}
  *p = (UINT)ptr;
 }
 fclose(fpModule);
}

/*****************************************************************************/
/* AddFreeToModuleLoadTable()                                             822*/
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*  Add a module free notification link  to the module load table.           */
/*  (See the comments under AddToModuleLoadTable() for a description         */
/*   of linked list. )                                                       */
/*                                                                           */
/*  The entry has the following format:                                      */
/*                                                                           */
/*                                                |->object table            */
/*                                                |                          */
/*     4        4     4              1              4                        */
/*    ------------------------------------------------------------           */
/*   |        |      |             |              |               |          */
/*   | record | mte  | 1           | '\0'         | 0 objects     |          */
/*   | length |      | name len    | null z-string|               |          */
/*    ------------------------------------------------------------           */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*  ppModuleList input - Address of a pointer to the module list.            */
/*  Pid          input - Process id owning the mte.                          */
/*  mte          input - EXE/DLL handle of freed dll.                        */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   A link will be added to the end of the list pointed to by               */
/*   ppModuleList.                                                           */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*  Pid and mte are valid.                                                   */
/*                                                                           */
/*****************************************************************************/
void AddFreeToModuleLoadTable( UINT **ppModuleList, ULONG mte)
{
 UINT *ptr;
 UINT *p;
 int  TableSize;

 /****************************************************************************/
 /* - First, remove from x-server list of tables.                            */
 /****************************************************************************/
 DelObjTableFromList( mte );

 TableSize = 4 +                        /* size of mte.                      */
             4 +                        /* size of module name length.       */
             1 +                        /* length of the module name.        */
             4;                         /* size of object table.             */

 p=ptr=Talloc(TableSize +
              4 +                       /* size of next->.                   */
              4 );                      /* size of length of rest of table.  */

 /****************************************************************************/
 /* - Since Talloc() zeros the allocated space we only have to add the       */
 /*   non-zero entries.                                                      */
 /****************************************************************************/
 *(p)++ = NULL;                         /* ground the node.                  */
 *p++ = TableSize;                      /* size of the rest of the table.    */
 *p++ = mte;
 *p++ = 1;

 /****************************************************************************/
 /* Add the entry to the linked list of modules.                             */
 /****************************************************************************/
 for(p=(UINT*)ppModuleList;*p!=NULL;p= (UINT*)*p){;}
 *p = (UINT)ptr;
}

/*****************************************************************************/
/* GetObjectTable()                                                       822*/
/*                                                                           */
/* Description:                                                              */
/*  Get the memory object table from the EXE/DLL header.                     */
/*                                                                           */
/*  The table has the following format:                                      */
/*                                                                           */
/*     4            4        4          4          1     1                   */
/*    ------------------------------------------------------------           */
/*   | Number of  |        |          |          |      |         |          */
/*   | objects in | Object | LoadAddr | LoadSize | Type | Bitness |          */
/*   | the table  | Number |          |          |      |         |          */
/*    ------------------------------------------------------------           */
/*                |        |          |          |      |         |          */
/*                | Object | LoadAddr | LoadSize | Type | Bitness |          */
/*                | Number |          |          |      |         |          */
/*                 -----------------------------------------------           */
/*                   .                                                       */
/*                   .                                                       */
/*                   .                                                       */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*  fp        input - file handle for the exe/dll.                           */
/*  Pid       input - Process id owning the mte.                             */
/*  mte       input - EXE/DLL handle.                                        */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*  tableptr  pointer to the table.                                          */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*  Pid and mte are valid.                                                   */
/*                                                                           */
/*****************************************************************************/
#define OFFSETOFOS2HDR   0x3CL          /* loc contains offset of start of   */
                                        /* OS/2 exe header.                  */
                                        /*                                   */
/* The following offsets are relative to the start of the EXE header         */
/* for 16 bit EXE/DLLs.                                                      */
                                        /*                                   */
#define OFFNUMOBJS16      0x1CL         /* loc with # of 16 bit segs.        */
#define OFFOBJTAB16       0x22L         /* loc with # start of 16 bit        */
                                        /* segment table.                    */
#define SIZENUMOBJS16     2             /* bytes to read for num of segs.    */
#define SIZEOBJTAB16      2             /* bytes to read for segment         */
                                        /* table offset.                     */
#define SIZETABENTRY16    8             /* table entry size in file.         */
#define CODEBIT16         0             /* flag bit for code/data object.    */
#define OFFOFFLAGS16      4             /* offs of flags field in tbl ent.   */
#define OFFOFOBJSIZE16    2             /* offs of size  field in tbl ent.   */

/* The following offsets are relative to the start of the EXE header         */
/* for 32 bit EXE/DLLs.                                                      */
                                        /*                                   */
#define OFFNUMOBJS32      0x44L         /* loc with # of 32 bit objects.     */
#define OFFOBJTAB32       0x40L         /* loc with # start of 32 bit        */
                                        /* object table.                     */
#define SIZENUMOBJS32     4             /* bytes to read for num of objs.    */
#define SIZEOBJTAB32      4             /* bytes to read for obj table       */
                                        /* offset.                           */
#define SIZETABENTRY32    24            /* table entry size in file.         */
#define CODEBIT32         2             /* flag bit for code object.         */
#define OFFOFFLAGS32      8             /* offs of flags field in tbl ent.   */
#define OFFOFOBJSIZE32    0             /* offs of size  field in tbl ent.   */
#define BIGDEFAULT        13            /* bit for 32 or 16 bitness.         */
#define MAXBUFFERSIZE     SIZETABENTRY32/* max read buffer size.             */


 UINT *
GetObjectTable( FILE *fp , ULONG Pid, ULONG mte )
{
 char            buffer[MAXBUFFERSIZE]; /* file table entry read buffer.     */
 ULONG           StartOS2Hdr;           /* loc of the OS/2 EXE/DLL hdr.      */
 UINT            NumOfObjs;             /* number of memory objects.         */
 UINT            StartOfTable;          /* start of object table in file.    */
 UINT            flags;                 /* object flags.                     */
 UINT            ObjNum;                /* ordinal object number.            */
 int             NumObjs;               /* number of code objects.           */
 UINT           *tableptr;              /* -> to table we're building.       */
 OBJTABLEENTRY  *tp;                    /* -> to object table entries.       */
 PtraceBuffer    ptb;                   /* DosDebug buffer.                  */
 UCHAR           ExeType;               /* 16 bit/32 bit EXE/DLL.            */
 ULONG           OffOfNumOfObjects;     /* location containing number of     */
                                        /* objects in object table.          */
 ULONG           OffOfObjTable;         /* location containing the offset    */
                                        /* of the object table.              */
 int             NumBytesToRead;        /* bytes to read from the file.      */
 int             OffOfFlags;            /* offs of flags field in tbl ent.   */
 int             OffOfObjSize;          /* offs of size  field in tbl ent.   */
 int             SizeOfTable;           /* size of table we're building.     */
 APIRET          rc;                    /* DosDebug return code.             */

/*****************************************************************************/
/* - Get the location of the OS/2 EXE/DLL header.                            */
/* - Establish 16 or 32 bitness.                                             */
/* - Read the location containing the number of memory objects.              */
/* - Read the number of memory objects.                                      */
/* - Read the location containing the offset of the object table.            */
/* - Read the location of the object table.                                  */
/* - Read the object table to get the number of objects.                     */
/* - Allocate the table.                                                     */
/* - Build the table and convert object numbers to load addresses.           */
/*                                                                           */
/*****************************************************************************/
 fseek(fp,OFFSETOFOS2HDR,SEEK_SET);
 fread( buffer,1,4,fp);
 StartOS2Hdr = *( ULONG *)buffer;

 /****************************************************************************/
 /* Establish 16 or 32 bitness.                                              */
 /****************************************************************************/
 fseek(fp, StartOS2Hdr,SEEK_SET);
 fread( buffer , 1,1 , fp);
 ExeType = BIT16;
 if( buffer[0] == 'L' )
  ExeType = BIT32;

 /****************************************************************************/
 /* Read the number of memory ojbects.                                       */
 /****************************************************************************/
 OffOfNumOfObjects = OFFNUMOBJS16;
 NumBytesToRead    = SIZENUMOBJS16;
 if ( ExeType == BIT32 )
 {
  OffOfNumOfObjects = OFFNUMOBJS32;
  NumBytesToRead    = SIZENUMOBJS32;
 }
 memset(buffer,0,sizeof(buffer) );
 fseek(fp, StartOS2Hdr + OffOfNumOfObjects,SEEK_SET);
 fread( buffer , 1,NumBytesToRead , fp);
 NumOfObjs  = *( int *)buffer;

 /****************************************************************************/
 /* Get the file offset start of the table.                                  */
 /****************************************************************************/
 OffOfObjTable     = OFFOBJTAB16;
 NumBytesToRead    = SIZEOBJTAB16;
 if ( ExeType == BIT32 )
 {
  NumBytesToRead    = SIZEOBJTAB32;
  OffOfObjTable     = OFFOBJTAB32;
 }
 memset(buffer,0,sizeof(buffer) );
 fseek(fp, StartOS2Hdr + OffOfObjTable,SEEK_SET);
 fread( buffer,1,NumBytesToRead,fp);
 StartOfTable = *( UINT *)buffer;

 /****************************************************************************/
 /* Read the table once to get the number of objects. We have to have this   */
 /* before we know how much memory to allocate.                              */
 /*                                                                          */
 /*  - read table entries and check the appropriate flags for code or data   */
 /*  - if no objects, then return a null pointer.                            */
 /*                                                                          */
 /* Note:                                                                    */
 /*                                                                          */
 /*  The high order part of the flags on a 16 bit read are garbage but       */
 /*  we're not doing anything with it so it doesn't matter.                  */
 /*                                                                          */
 /****************************************************************************/
 NumObjs = 0;
 fseek(fp,StartOS2Hdr+StartOfTable,SEEK_SET);
 NumBytesToRead = SIZETABENTRY16;
 OffOfFlags = OFFOFFLAGS16;
 if( ExeType == BIT32 )
 {
  NumBytesToRead = SIZETABENTRY32;
  OffOfFlags = OFFOFFLAGS32;
 }
 for ( ObjNum = 1; ObjNum <= NumOfObjs; ObjNum++ )
 {
  memset(buffer,0,sizeof(buffer) );
  fread( buffer,1,NumBytesToRead,fp);
  flags = *(UINT *)&buffer[OffOfFlags];
  NumObjs++;
 }

 /****************************************************************************/
 /* Add this code if we don't want dlls with 0 objects.                      */
 /****************************************************************************/

 if(NumObjs == 0 )
  return(NULL);


 /****************************************************************************/
 /* Now read the table entries and fill in the valuse.                       */
 /*                                                                          */
 /* - Allocate the memory for the table.( number of objects + size. )        */
 /* - Make first table entry the number of objects in the table.             */
 /* - Read the table entries.                                                */
 /* - Convert object numbers to addresses.                                   */
 /*                                                                          */
 /****************************************************************************/
 SizeOfTable = NumObjs*sizeof(OBJTABLEENTRY) + sizeof(int);
 tableptr    = (void *)Talloc( SizeOfTable );
 *tableptr   = NumObjs;
 tp          = (OBJTABLEENTRY *)(tableptr + 1);

 fseek(fp,StartOS2Hdr+StartOfTable,SEEK_SET);
 NumBytesToRead = SIZETABENTRY16;
 OffOfFlags     = OFFOFFLAGS16;
 OffOfObjSize   = OFFOFOBJSIZE16;
 if( ExeType == BIT32 )
 {
  NumBytesToRead = SIZETABENTRY32;
  OffOfFlags     = OFFOFFLAGS32;
  OffOfObjSize   = OFFOFOBJSIZE32;
 }

 for ( ObjNum = 1; ObjNum <= NumOfObjs; ObjNum++,tp++)
 {
  memset(buffer,0,sizeof(buffer) );
  fread( buffer,1,NumBytesToRead,fp);
  flags = *(UINT *)&buffer[OffOfFlags];
  switch( ExeType )
  {
   case BIT32:
    if( TestBit(flags,CODEBIT32) == FALSE )
      tp->ObjType = DATA;
    else
      tp->ObjType = CODE;
    tp->ObjLoadAddr = ObjNum;
    tp->ObjLoadSize = *(UINT *)&buffer[OffOfObjSize];
    tp->ObjBitness = (UCHAR)( (TestBit(flags,BIGDEFAULT) )?BIT32:BIT16);
    break;

   case BIT16:
    if( TestBit(flags,CODEBIT16) == TRUE  )
      tp->ObjType = DATA;
    else
      tp->ObjType = CODE;
    tp->ObjLoadAddr = ObjNum;
    tp->ObjLoadSize = (UINT)(*(USHORT *)&buffer[OffOfObjSize]);
    tp->ObjBitness  = BIT16;
    break;
  }

  /***************************************************************************/
  /* Convert object number to a load address. If there is an error, then     */
  /* assign a 0 load address to the object number.                           */
  /***************************************************************************/
  memset(&ptb,0,sizeof(ptb));
  ptb.Pid   = Pid;
  ptb.MTE   = mte;
  ptb.Cmd   = DBG_C_NumToAddr;
  ptb.Value = ObjNum;
  rc=DosDebug(&ptb);
  if( rc || ptb.Cmd != 0)
  {
   tp->ObjNum      =  0;
   tp->ObjLoadAddr =  0;
   tp->ObjLoadSize =  0;
   tp->ObjBitness  =  BIT32;
  }
  else
  {
   tp->ObjNum = ObjNum;
   tp->ObjLoadAddr =  ptb.Addr;
  }

  /***************************************************************************/
  /* Now put in the 16:16 base address for 16 bit objects.                   */
  /***************************************************************************/

  if( tp->ObjBitness == BIT16 )
   Sys_Flat2SelOff(tp->ObjLoadAddr,&tp->ObjLoadSel,(USHORT*)&tp->ObjLoadOff);
 }
 return( tableptr );
}                                       /* end GetObjectTable().             */

/*****************************************************************************/
/* PackModuleLoadTable()                                                  822*/
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Take the linked list of module load information and pack it for         */
/*   return to the caller.                                                   */
/*                                                                           */
/*   The packed table will look like so:                                     */
/*                                                                           */
/*                                                                           */
/*     4                                                                     */
/*     -------                                                               */
/*    |       |<--The number of mte entries in this table                    */
/*    |       |                                                              */
/*     -------                                                               */
/*       .                                                                   */
/*       .                                                                   */
/*       .                                                                   */
/*                                                                           */
/*     4        4     4              var            var                      */
/*    --------------------------------------//-----------//-------           */
/*   |        |      |             |              |               |          */
/*   | record | mte  | module name | module name  | object table  |          */
/*   | length |      | length      |              |               |          */
/*    --------------------------------------//-----------//-------           */
/*   |        |      |             |              |               |          */
/*   | record | mte  | module name | module name  | object table  |          */
/*   | length |      | length      |              |               |          */
/*    --------------------------------------//-----------//-------           */
/*        .                                                                  */
/*        .                                                                  */
/*        .                                                                  */
/*                                                                           */
/*                                                                           */
/*   The object table will look like so:                                     */
/*                                                                           */
/*     4            4        4          4          1     1                   */
/*    ------------------------------------------------------------           */
/*   | Number of  |        |          |          |      |         |          */
/*   | objects in | Object | LoadAddr | LoadSize | Type | Bitness |          */
/*   | the table  | Number |          |          |      |         |          */
/*    ------------------------------------------------------------           */
/*                |        |          |          |      |         |          */
/*                | Object | LoadAddr | LoadSize | Type | Bitness |          */
/*                | Number |          |          |      |         |          */
/*                 -----------------------------------------------           */
/*                   .                                                       */
/*                   .                                                       */
/*                   .                                                       */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   pModuleList        input  - -> to the linked list to be packed.         */
/*   pModuleTableLength output - -> to the length of the table built.        */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   pModuleTable  -> to the packed module table.                            */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   none                                                                    */
/*                                                                           */
/*****************************************************************************/
UINT *PackModuleLoadTable(UINT *pModuleList , int *pModuleTableLength )
{
 int    ModuleTableLength = 0;
 int    NumBytesToCopy;
 UCHAR *ptr;
 UINT  *pModuleTable;
 UINT   NumModules;

 typedef struct _link
 {
  struct _link *pnext;
  UINT          LinkSize;
  UINT          mte;
 }LINK;

 LINK *p;

 /****************************************************************************/
 /* - Compute the size of the block needed to store the table and            */
 /*   the number of modules that will be in the table.                       */
 /****************************************************************************/
 NumModules = 0;
 for( p=(LINK*)pModuleList; p!=NULL; p=p->pnext)
  if(p->mte != 0 )
   {
    ModuleTableLength += 4 + p->LinkSize;
    NumModules++;
   }

 ModuleTableLength += sizeof(NumModules);
 /****************************************************************************/
 /* Now pack the table.                                                      */
 /*  - Allocate the storage.                                                 */
 /*  - Put the number of modules in the table.                               */
 /*  - Copy the individual tables to the big block.                          */
 /****************************************************************************/
 pModuleTable = Talloc(ModuleTableLength);
 *pModuleTable = NumModules;

 ptr = (UCHAR*)pModuleTable + sizeof(UINT);
 for( p=(LINK*)pModuleList; p!=NULL; p=p->pnext)
  if(p->mte != 0 )
  {
   NumBytesToCopy = p->LinkSize + sizeof(p->LinkSize);
   memcpy(ptr,&p->LinkSize,NumBytesToCopy);
   ptr += NumBytesToCopy;
  }

 /****************************************************************************/
 /* Now, free the storage allocated for the linked list.                     */
 /****************************************************************************/
 p=(LINK*)pModuleList;
 while( p!=NULL)
 {
  LINK *pnext = p->pnext;
  Tfree(p);
  p=pnext;
 }
 /****************************************************************************/
 /* return the pointer to THE module table.                                  */
 /****************************************************************************/
 *pModuleTableLength = ModuleTableLength;
 return( pModuleTable);
}
/*****************************************************************************/
/* AddModuleLoadTables()                                                     */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Combine two load tables.                                                */
/*                                                                           */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*  p1           -> to a module load table.                                  */
/*  len1         length of table p1 points to.                               */
/*  p2           -> to a module load table.                                  */
/*  len2         length of table p2 points to.                               */
/*  plen         -> to receiver of combined table length.                    */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   p           -> to the combined table.                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
UINT *AddModuleLoadTables(UINT *p1,int len1, UINT *p2,int len2 ,int *plen)
{
 int   len;
 UINT *p;
 UINT *q;
 int   nbytes;

 /****************************************************************************/
 /* - compute the length of the combined table. Each table contains an       */
 /*   initial 4 byte field containing the number of entries in the table.    */
 /*   Subtract 4 bytes to adjust the combined length.                        */
 /* - allocate the combined table.                                           */
 /* - put in the combined number of entries.                                 */
 /* - copy the tables.                                                       */
 /* - give the caller the combined length.                                   */
 /* - return a pointer to the combined table.                                */
 /****************************************************************************/
 len = len1 + len2 - 4;
 p = q = Talloc(len);
 *p++ = *p1++ + *p2++;
 nbytes = len1 - 4;
 memcpy(p,p1,nbytes);
 p = (UINT*)((UCHAR*)p + nbytes);
 nbytes = len2 - 4;
 memcpy(p,p2,nbytes);
 *plen = len;
 return(q);
}

/*****************************************************************************/
/* AddObjTableToList()                                                       */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Add an object table to the list of tables in the x-server.              */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*  pObjectTable    -> to the table to be added.                             */
/*  mte             mte handle of exe/dll.                                   */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   void                                                                    */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*  pObjectTable != NULL                                                     */
/*                                                                           */
/*****************************************************************************/
static XSRVOBJTABLEENTRY *pObjTableList;
void AddObjTableToList( UINT *pObjectTable, ULONG mte )
{
 XSRVOBJTABLEENTRY *ptr;
 ULONG              NumEntries;
 int                size;


 /****************************************************************************/
 /* - compute size & allocate the link.                                      */
 /****************************************************************************/
 NumEntries = *(ULONG *)pObjectTable;
 size = sizeof(XSRVOBJTABLEENTRY) + (NumEntries-1)*sizeof(OBJTABLEENTRY);
 ptr = Talloc( size );

 /****************************************************************************/
 /* - copy the info to the link.                                             */
 /****************************************************************************/
 ptr->NumEntries = NumEntries;
 ptr->mte        = mte;
 memcpy( &ptr->ObjEntry , ++pObjectTable , NumEntries*sizeof(OBJTABLEENTRY) );

 /****************************************************************************/
 /* - add this link to the list.                                             */
 /****************************************************************************/
 ptr->pnext = pObjTableList;
 pObjTableList = ptr;
}

void FreeObjectList( void )
{
 XSRVOBJTABLEENTRY *ptr;
 XSRVOBJTABLEENTRY *ptrnext;

 for( ptr = pObjTableList; ptr != NULL; )
 {
  ptrnext=ptr->pnext;
  Tfree((void*)ptr);
  ptr=ptrnext;
 }
 pObjTableList = NULL;
}

/*****************************************************************************/
/* DelObjTableFromList()                                                     */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Delete an object table to the list of tables in the x-server.           */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*  mte             mte handle of exe/dll.                                   */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   void                                                                    */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
void DelObjTableFromList( ULONG mte )
{
 XSRVOBJTABLEENTRY *p;
 XSRVOBJTABLEENTRY *q;

 p=pObjTableList;
 q=(XSRVOBJTABLEENTRY *)&pObjTableList;
 for( ; p && (p->mte != mte ); q=p,p=p->pnext ){;}

 if( p )
 {
  q->pnext = p->pnext;
  Tfree(p);
 }
}

/*****************************************************************************/
/* _GetBitness()                                                             */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Get the 16 or 32 bitness of the address parameter.                      */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   addr       address.                                                     */
/*                                                                           */
/* Return:                                                                   */
/*   BIT32      address is 32 bit.                                           */
/*   BIT16      address is 16 bit.                                           */
/*   BITUNKNOWN can't determine.                                             */
/*                                                                           */
/*****************************************************************************/
UCHAR _GetBitness( ULONG addr )
{
 XSRVOBJTABLEENTRY *ptr;
 ULONG              NumEntries;
 OBJTABLEENTRY     *te;
 int                i;

 ptr = pObjTableList;
 for( ; ptr; ptr=ptr->pnext )
 {
  NumEntries = ptr->NumEntries;
  if ( NumEntries == 0 )
   continue;
  te = (OBJTABLEENTRY *)&ptr->ObjEntry;
  for(i=1; i <= NumEntries; i++,te++ )
  {
   if( addr >= te->ObjLoadAddr &&
       addr <  te->ObjLoadAddr + te->ObjLoadSize )
    return(te->ObjBitness);
  }
 }
 return(BITUNKNOWN);
}

/*****************************************************************************/
/* GetBaseAddr()                                                             */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Get the 16 or 32 bitness of the address parameter.                      */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   addr       address.                                                     */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   BaseAddr   base address of the object containing the addr.              */
/*                                                                           */
/*****************************************************************************/
ULONG GetBaseAddr( ULONG addr )
{
 XSRVOBJTABLEENTRY *ptr;
 ULONG              NumEntries;
 OBJTABLEENTRY     *te;
 int                i;

 ptr = pObjTableList;
 for( ; ptr; ptr=ptr->pnext )
 {
  NumEntries = ptr->NumEntries;
  if ( NumEntries == 0 )
   continue;
  te = (OBJTABLEENTRY *)&ptr->ObjEntry;
  for(i=1; i <= NumEntries; i++,te++ )
  {
   if( addr >= te->ObjLoadAddr &&
       addr <  te->ObjLoadAddr + te->ObjLoadSize )
    return(te->ObjLoadAddr);
  }
 }
 return(0);
}

/*****************************************************************************/
/* _GetLoadAddr()                                                            */
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
ULONG _GetLoadAddr( ULONG mte, UINT ObjectNumber)
{
 XSRVOBJTABLEENTRY *ptr;
 ULONG              NumEntries;
 OBJTABLEENTRY     *te;
 int                i;
 ULONG              LoadAddr = 0;

 /****************************************************************************/
 /* - scan to the object tables for this mte.                                */
 /****************************************************************************/
 for(ptr = pObjTableList; ptr && (ptr->mte != mte); ptr=ptr->pnext ){;}

 /****************************************************************************/
 /* - scan the object table and get the ObjectNumber.                        */
 /****************************************************************************/
 if( ptr &&
     (ptr->mte == mte) &&
     (NumEntries = ptr->NumEntries)
   )
 {
  te = (OBJTABLEENTRY *)(&ptr->ObjEntry);
  for(i=1; i <= NumEntries; i++,te++ )
  {
   if( te->ObjNum == ObjectNumber )
   {
    LoadAddr = te->ObjLoadAddr;
    break;
   }
  }
 }
 return( LoadAddr );
}

/*****************************************************************************/
/* GetPtrToObjectTable( ULONG mte )                                          */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Get a pointer to the object table for an mte. Skip past the             */
/*   NumEntries field.                                                       */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   mte          mte of the module the object is in.                        */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   te           -> to object table for this mte.                           */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
OBJTABLEENTRY *GetPtrToObjectTable( ULONG mte )
{
 XSRVOBJTABLEENTRY *ptr;
 OBJTABLEENTRY     *te = NULL;

 /****************************************************************************/
 /* - scan to the object tables for this mte.                                */
 /****************************************************************************/
 for(ptr = pObjTableList; ptr && (ptr->mte != mte); ptr=ptr->pnext ){;}

 /****************************************************************************/
 /* - scan the object table and get the ObjectNumber.                        */
 /****************************************************************************/
 if(  ptr &&
     (ptr->mte == mte) &&
      ptr->NumEntries
   )
 {
  te = (OBJTABLEENTRY *)(&ptr->ObjEntry);
 }
 return( te );
}

/*****************************************************************************/
/* CoalesceTables( )                                                         */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Coalesce two object tables.                                             */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   pModuleLoadTableAccum     previously coalesced table.                   */
/*   ModuleTableLengthAccum    previously coalesced table length.            */
/*   pModuleLoadTable          table to be coalesced.                        */
/*   ModuleTableLength,        table to be coalesced length.                 */
/*   pModuleTableLengthAccum   length of the resultant table.                */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   pModuleLoadTableAccum     ->to resultant table.                         */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
UINT *CoalesceTables( UINT *pModuleLoadTableAccum,
                      int   ModuleTableLengthAccum,
                      UINT *pModuleLoadTable,
                      int   ModuleTableLength,
                      int  *pModuleTableLengthAccum )
{
 int   length = 0;
 UINT *p;

 if(pModuleLoadTableAccum == NULL)
 {
  pModuleLoadTableAccum = pModuleLoadTable;
  length= ModuleTableLength;
 }
 else
 {
  /*************************************************************************/
  /* - coalesce the load tables.                                           */
  /*************************************************************************/
  p =pModuleLoadTableAccum;
  pModuleLoadTableAccum = AddModuleLoadTables( pModuleLoadTableAccum,
                                               ModuleTableLengthAccum,
                                               pModuleLoadTable,
                                               ModuleTableLength,
                                               &length );
  Tfree(pModuleLoadTable);
  Tfree(p);
 }
 *pModuleTableLengthAccum = length;
 return(pModuleLoadTableAccum);
}
