/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   debug.c                                                              822*/
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*  Miscellaneous debugging routines.                                        */
/*                                                                           */
/* History:                                                                  */
/*                                                                           */
/*   05/04/93 Created.                                                       */
/*                                                                           */
/*...Release 1.04 (04/30/93)                                                 */
/*...                                                                        */
/*...                                                                        */
/*****************************************************************************/

/*****************************************************************************/
/* Includes.                                                                 */
/*****************************************************************************/
#include "all.h"

void DumpModuleLoadTable( UINT *pModuleLoadTable )
{
 MODULE_LOAD_ENTRY *p;
 OBJTABLEENTRY     *q = NULL;
 char              *cp;
 UINT               NumModuleEntries;
 int                NumObjectsInTable;

 FILE *dmp;


 if( pModuleLoadTable == NULL )
  return;

 dmp = fopen("log.dat","a");


 NumModuleEntries = *pModuleLoadTable++;
 fprintf(dmp,"\nNum of Modules  =%d\n",NumModuleEntries);

 p = (MODULE_LOAD_ENTRY*)pModuleLoadTable;
 for(; NumModuleEntries > 0; NumModuleEntries--,p = (MODULE_LOAD_ENTRY*)q)
 {
  fprintf(dmp,"\nLengthofMteEntry=%d",p->LengthOfMteEntry);
  fprintf(dmp,"\n             mte=%d",p->mte);
  fprintf(dmp,"\nModuleNameLength=%d",p->ModuleNameLength);
  if( p->ModuleNameLength==1 )
   fprintf(dmp,"\n      ModuleName=%s","freed module");
  else
   fprintf(dmp,"\n      ModuleName=%s",p->ModuleName);

  cp = (char *)p + sizeof(MODULE_LOAD_ENTRY) - 1 + p->ModuleNameLength;
  q  = (OBJTABLEENTRY*)cp;

  NumObjectsInTable = *(int*)q;
  fprintf(dmp,"\nNumObjectsInTable=%d",NumObjectsInTable);

  q = (OBJTABLEENTRY*)((cp=(char*)q)+4);
  for( ; NumObjectsInTable > 0; NumObjectsInTable--,q++)
  {
   fprintf(dmp,"\nObjNum=%d ",q->ObjNum);
   fprintf(dmp,"ObjLoadAddr=%#8x ",q->ObjLoadAddr);
   fprintf(dmp,"Sel:Off=%04x:%-04x ",q->ObjLoadSel, q->ObjLoadOff);
   fprintf(dmp,"ObjLoadSize=%#8x  ",q->ObjLoadSize);
   fprintf(dmp,"ObjType=%d  ",q->ObjType);
   fprintf(dmp,"ObjBitness=%d  ",q->ObjBitness);
  }
  fprintf(dmp,"\n");
 }
 fflush(dmp);
 fclose(dmp);
}

void DumpMteTable( void *p )
{
 MTE_TABLE_ENTRY *pMteTable = (MTE_TABLE_ENTRY *)p;
 MTE_TABLE_ENTRY *pMteTableEntry;
 FILE *dmp;
 ULONG thismte,lastmte;

 if( p == NULL )
  return;

 dmp = fopen("log.dat","a");

 pMteTableEntry = pMteTable ;
 lastmte = pMteTableEntry->mte;

 for( ;
      pMteTable &&
      ( (thismte=pMteTableEntry->mte) != ENDOFTABLE);
      pMteTableEntry++ )
 {
  if( thismte != lastmte )
  {
   fprintf(dmp,"\n");
   lastmte = thismte;
  }
  fprintf(dmp,"\nmte=%-#6x ",pMteTableEntry->mte);
  fprintf(dmp,"Objnum=%-2d ",pMteTableEntry->ObjNum);
  fprintf(dmp,"LoadAddr=%-#8x",pMteTableEntry->LoadAddr);
 }
 fflush(dmp);
 fclose(dmp);
}

void DumpPubRec16( void *pPubRec16 )
{
 FILE *dmp;

 PUBREC16 *p = (PUBREC16*)pPubRec16;

 USHORT ObjNum = p->Pub16Addr.RawAddr.ObjNum;
 USHORT Offset = p->Pub16Addr.RawAddr.Offset;
 USHORT Typeid = p->Typeid;
 UCHAR  Namelen= p->Namelen;
 char   pubname[256];

 memset(pubname,0,sizeof(pubname));
 memcpy(pubname, &p->Namelen+1,Namelen);


 dmp = fopen("log.dat","a");
 fprintf(dmp,"\n %04x:%04x",ObjNum,Offset);
 fprintf(dmp," %04x",Typeid);
 fprintf(dmp," %s",pubname);

 fflush(dmp);
 fclose(dmp);
}
/*****************************************************************************/
/* Dump the object tables attached to the pdf structures.                    */
/*                                                                           */
/*                                                                           */
/*****************************************************************************/

extern PROCESS_NODE *pnode;

void DumpObjectTable( DEBFILE *pdf )
{
 OBJTABLEENTRY     *q = NULL;
 int                NumObjectsInTable;

 FILE *dmp;
 dmp = fopen("log.dat","a");

 q  = (OBJTABLEENTRY*)pdf->CodeObjs;

 NumObjectsInTable = *(int*)q;
 fprintf(dmp,"\nModuleName=%s",pdf->DebFilePtr->fn);
 fprintf(dmp,"\nNumObjectsInTable=%d",NumObjectsInTable);

 q = (OBJTABLEENTRY*)( (char*)q + 4 );
 for( ; NumObjectsInTable > 0; NumObjectsInTable--,q++)
 {
  fprintf(dmp,"\nObjNum=%d ",q->ObjNum);
  fprintf(dmp,"ObjLoadAddr=%#8x ",q->ObjLoadAddr);
  fprintf(dmp,"ObjLoadSize=%#8x  ",q->ObjLoadSize);
  fprintf(dmp,"ObjType=%d  ",q->ObjType);
  fprintf(dmp,"ObjBitness=%d  ",q->ObjBitness);
 }
 fprintf(dmp,"\n");

 fflush(dmp);
 fclose(dmp);
}



static UCHAR  AlreadyOpen=0;

/*****************************************************************************/
/* Dump the unwind tables.                                                   */
/*                                                                           */
/*                                                                           */
/*****************************************************************************/
extern uint          NActFrames;
extern uchar         ActCSAtrs[];       /* 0=>16-bit frame 1=>32-bit      107*/
extern uint          ActFrames[];       /* value of frame pointer (bp)       */
extern uint          ActFaddrs[];       /* value of return "EIP" on stack 107*/
extern uint          ActFlines[MAXAFRAMES][2]; /* mid, lno for ActFaddrs  107*/
extern SCOPE         ActScopes[];       /* (mid,ssproc) for ...              */

void DumpUnwindTables( void )
{
 int n;

 FILE *dmp;

  if( AlreadyOpen )
    dmp = fopen("log.dat","a");
  else
  {
    dmp = fopen("log.dat","w");
    AlreadyOpen = 1;
  }
 for( n=0; n< NActFrames; n++)
 {
  fprintf(dmp,"\nActCSAtrs[%2d]=%#4x ",n,ActCSAtrs[n]);
  fprintf(dmp,"\nActFrames[%2d]=%#8x ",n,ActFrames[n]);
  fprintf(dmp,"\nActFaddrs[%2d]=%#8x ",n,ActFaddrs[n]);
  fprintf(dmp,"\nActFrames[%2d]/mid=%3d ",n,ActFlines[n][0]);
  fprintf(dmp,"\nActFrames[%2d]/lno=%3d ",n,ActFlines[n][1]);
  fprintf(dmp,"\nActScopes[%2d]=%#8x ",n,ActScopes[n]);
  fprintf(dmp,"\n");
 }
 fflush(dmp);
 fclose(dmp);
}


/*****************************************************************************/
/* Dump the instruction cache                                                */
/*                                                                           */
/*                                                                           */
/*****************************************************************************/
void DumpCache( INSTR *icache)
{
 int n=0;


 FILE *dmp;
 dmp = fopen("log.dat","a");
 while( icache[n].instaddr != ENDOFCACHE )
 {
/*fprintf(dmp,"\n%4d instaddr=%#8x ",n, icache[n].instaddr);fflush(0);*/
   printf(    "\n%4d instaddr=%#8x ",n, icache[n].instaddr);fflush(0);
  n++;
 }
 fflush(dmp);
 fclose(dmp);
}

void DumpSourceLineCache( UINT  *SourceLineCache)
{
 int n=0;


 FILE *dmp;
 dmp = fopen("logx.dat","a");
 while( SourceLineCache[n] != ENDOFTABLE )
 {
  fprintf(dmp,"\n%6d lno=%d ",n,SourceLineCache[n]);fflush(0);
  n++;
 }
 fflush(dmp);
 fclose(dmp);
}

/*****************************************************************************/
/* Dump break points.                                                        */
/*                                                                           */
/*****************************************************************************/
void DumpBrks( BRK *p)
{
 uchar *q;
 uint nbytes;

 FILE *dmp;
 dmp = fopen("log.dat","a");

 fprintf(dmp,"\n\n");
 for( ;p;p=p->next )
 {
  fprintf(dmp,"\nbrkat=%#8x ",p->brkat);
  q=Getnbytes(p->brkat,1,&nbytes);
  fprintf(dmp,"opcode=%2x ",*q);
 }
 fflush(dmp);
 fclose(dmp);
}

void DumpModuleStructure( void *mptr )
{

 FILE *dmp;
 MODULE *p  = (MODULE *)mptr;
 CSECT  *pc = p->pCsects;

 dmp = fopen("log.dat","a");
 fprintf(dmp,"\n mid=%d",p->mid);
#if 0
 fprintf(dmp,"\n Filename       =%08x",p->FileName);
 fprintf(dmp,"\n Publics        =%08x",p->Publics);
 fprintf(dmp,"\n PubLen         =%08x",p->PubLen);
 fprintf(dmp,"\n TypeDefs       =%08x",p->TypeDefs);
 fprintf(dmp,"\n TypeLen        =%08x",p->TypeLen);
 fprintf(dmp,"\n Symbols        =%08x",p->Symbols);
 fprintf(dmp,"\n Symlen         =%08x",p->SymLen);
 fprintf(dmp,"\n LineNums       =%08x",p->LineNums);
 fprintf(dmp,"\n LineNumsLen    =%08x",p->LineNumsLen);
 fprintf(dmp,"\n Format/Pubs    =%d",p->DbgFormatFlags.Pubs);
 fprintf(dmp,"\n Format/Syms    =%d",p->DbgFormatFlags.Syms);
 fprintf(dmp,"\n Format/Lins    =%d",p->DbgFormatFlags.Lins);
 fprintf(dmp,"\n Format/Typs    =%d",p->DbgFormatFlags.Typs);
 fprintf(dmp,"\n");
#endif
 while(pc)
 {
  fprintf(dmp,"\n");
  fprintf(dmp,"\n      Segnum      =%10d", pc->SegNum     );
  fprintf(dmp,"\n      SegFlatAddr =%10x", pc->SegFlatAddr);
  fprintf(dmp,"\n      CsectLo     =%10x", pc->CsectLo    );
  fprintf(dmp,"\n      CsectHi     =%10x", pc->CsectHi    );
  fprintf(dmp,"\n      CsectSize   =%10x", pc->CsectSize  );
  fprintf(dmp,"\n      NumEntries  =%10d", pc->NumEntries );
  fprintf(dmp,"\n");

  pc = pc->next;
 }

 fflush(dmp);
 fclose(dmp);

}

void DumpCsectMap( MODULE* pm, CSECTMAP *pmap, int type )
{

 FILE     *dmp;
 CSECT    *pCsect;
 CSECTMAP *pc;
 void     *pf;

 dmp = fopen("log.dat","a");
 fprintf(dmp,"\n mid=%d",pm->mid);
 for( pCsect = pm->pCsects; pCsect != NULL; pCsect=pCsect->next )
 {
  fprintf(dmp,"\n");
  fprintf(dmp,"\n     Segnum      =%10d", pCsect->SegNum     );
  fprintf(dmp,"\n     SegFlatAddr =%10x", pCsect->SegFlatAddr);
  fprintf(dmp,"\n     CsectLo     =%10x", pCsect->CsectLo    );
  fprintf(dmp,"\n     CsectHi     =%10x", pCsect->CsectHi    );
  fprintf(dmp,"\n     CsectSize   =%10x", pCsect->CsectSize  );
  fprintf(dmp,"\n     NumEntries  =%10d", pCsect->NumEntries );
  pc = pmap;
  while(pc)
  {
   if( pc->pCsect == pCsect )
   {
    pf = pc->pFirstEntry;
    switch( type )
    {
     case TYPE10B_HL04:
     {
      fprintf(dmp,"\n      NumEntries    =%10d", ((FIRST_ENTRY_HL04*)pf)->NumEntries);
      fprintf(dmp,"\n      SegmentNumber =%10d", ((FIRST_ENTRY_HL04*)pf)->SegNum    );
      fprintf(dmp,"\n      BaseOffset    =%10x", ((FIRST_ENTRY_HL04*)pf)->BaseOffset);
      fprintf(dmp,"\n");
     }
     break;

     case TYPE10B_HL03:
     {
      fprintf(dmp,"\n      NumEntries    =%10d", ((FIRST_ENTRY_HL03*)pf)->NumEntries);
      fprintf(dmp,"\n      SegmentNumber =%10d", ((FIRST_ENTRY_HL03*)pf)->SegNum    );
      fprintf(dmp,"\n");
     }
     break;

     case TYPE10B_HL01:
     {
      fprintf(dmp,"\n      NumEntries    =%10d", ((FIRST_ENTRY_HL01*)pf)->NumEntries);
      fprintf(dmp,"\n");
     }
     break;

     case TYPE109_16:
     {
      UCHAR *pFileName = pc->pFirstEntry;


      pf = pFileName + *pFileName + 1;
      fprintf(dmp,"\n      NumEntries    =%10d", ((FIRST_ENTRY_109_16*)pf)->NumEntries);
      fprintf(dmp,"\n      SegmentNumber =%10d", ((FIRST_ENTRY_109_16*)pf)->SegNum    );
      fprintf(dmp,"\n");
     }
     break;

     case TYPE109_32:
     {
      fprintf(dmp,"\n      NumEntries    =%10d", ((FIRST_ENTRY_109_32*)pf)->NumEntries);
      fprintf(dmp,"\n      SegmentNumber =%10d", ((FIRST_ENTRY_109_32*)pf)->SegNum    );
      fprintf(dmp,"\n");
     }
     break;

     case TYPE105:
     {
      UCHAR *pFileName = pc->pFirstEntry;


      pf = pFileName + *pFileName + 1;
      fprintf(dmp,"\n      NumEntries    =%10d", ((FIRST_ENTRY_105*)pf)->NumEntries);
      fprintf(dmp,"\n      sfi           =%10d", pc->mssfi);
      fprintf(dmp,"\n");
     }
     break;
    }
   }
   pc = pc->next;
  }
 }
 fflush(dmp);
 fclose(dmp);

}

void DumpAfileStructure( void *fp   )
{
 FILE *dmp;

 AFILE *p = (AFILE *)fp;

  if( AlreadyOpen )
    dmp = fopen("log.dat","a");
  else
  {
    dmp = fopen("log.dat","w");
    AlreadyOpen = 1;
  }
 fprintf(dmp,"\n mid            =%d",  p->mid);
 fprintf(dmp,"\n Tlines         =%d",  p->Tlines);
 fprintf(dmp,"\n Nlines         =%d",  p->Nlines );
 fprintf(dmp,"\n Nbias          =%d",  p->Nbias);
 fprintf(dmp,"\n topline        =%d",  p->topline);
 fprintf(dmp,"\n csrline        =%d",  p->csrline);
 fprintf(dmp,"\n hotline        =%d",  p->hotline);
 fprintf(dmp,"\n hotaddr        =%08x",p->hotaddr);
 fprintf(dmp,"\n skipcols       =%d",  p->skipcols);
 fprintf(dmp,"\n Nshown         =%d",  p->Nshown);
 fprintf(dmp,"\n topoff         =%08x",p->topoff);
 fprintf(dmp,"\n csr.col        =%d",  p->csr.col);
 fprintf(dmp,"\n csr.row        =%d",  p->csr.row);
 fprintf(dmp,"\n");
 fflush(dmp);
 fclose(dmp);
}

void DumpINSTRStructure( void *pinstr )
{
 FILE *dmp;

 INSTR *p = (INSTR *)pinstr;

  if( AlreadyOpen )
    dmp = fopen("log.dat","a");
  else
  {
    dmp = fopen("log.dat","w");
    AlreadyOpen = 1;
    fprintf(dmp,"\n");
    fprintf(dmp,"instaddr  ");
    fprintf(dmp,"len       ");
    fprintf(dmp,"mod_type  ");
    fprintf(dmp,"type      ");
    fprintf(dmp,"opsize    ");
    fprintf(dmp,"reg       ");
    fprintf(dmp,"offset    ");
    fprintf(dmp,"seg       ");
    fprintf(dmp,"base      ");
    fprintf(dmp,"index     ");
    fprintf(dmp,"scale     ");
  }

 fprintf(dmp,"\n");
 fprintf(dmp,"%-10x",  p->instaddr );
 fprintf(dmp,"%-10d",  p->len      );
 fprintf(dmp,"%-10d",  p->mod_type );
 fprintf(dmp,"%-10d",  p->type     );
 fprintf(dmp,"%-10d",  p->OpSize   );
 fprintf(dmp,"%-10d",  p->reg      );
 fprintf(dmp,"%-10x",  p->offset   );
 fprintf(dmp,"%-10d",  p->seg      );
 fprintf(dmp,"%-10d",  p->base     );
 fprintf(dmp,"%-10d",  p->index    );
 fprintf(dmp,"%-10d",  p->scale    );
 fflush(dmp);
 fclose(dmp);
}

void DumpTSTATEStructure( void *ptstate )
{
 FILE *dmp;

 TSTATE *p = (TSTATE *)ptstate;

  if( AlreadyOpen )
    dmp = fopen("log.dat","a");
  else
  {
    dmp = fopen("log.dat","w");
    AlreadyOpen = 1;
    fprintf(dmp,"\n");
    fprintf(dmp,"DbgState  ");
    fprintf(dmp,"ThdState  ");
    fprintf(dmp,"Priority  ");
    fprintf(dmp,"tid       ");
  }

 fprintf(dmp,"\n");
 fprintf(dmp,"%-10x",  p->ts.DebugState );
 fprintf(dmp,"%-10d",  p->ts.ThreadState  );
 fprintf(dmp,"%-10d",  p->ts.Priority );
 fprintf(dmp,"%-10d",  p->tid      );
 fflush(dmp);
 fclose(dmp);
}

void DumpString( void *cp   )
{
 FILE *dmp;


  if( AlreadyOpen )
    dmp = fopen("log.dat","a");
  else
  {
    dmp = fopen("log.dat","w");
    AlreadyOpen = 1;
  }
 fprintf(dmp,"\n %s", cp);
 fflush(dmp);
 fclose(dmp);
}

#if 0
void DumpFILE( void *fp )
{
 FILE *dmp;

 FILE *p = (FILE *)fp;

  if( AlreadyOpen )
    dmp = fopen("logx.dat","a");
  else
  {
    dmp = fopen("logx.dat","w");
    AlreadyOpen = 1;
    fprintf(dmp,"\n");
    fprintf(dmp,"bufPtr    ");
    fprintf(dmp,"count     ");
    fprintf(dmp,"userFlags ");
    fprintf(dmp,"bufLen    ");
    fprintf(dmp,"ungetCount");
    fprintf(dmp,"tempStore ");
    fprintf(dmp,"ungetbf[0]");
    fprintf(dmp,"ungetbf[1]");
    fprintf(dmp,"lastOp    ");
    fprintf(dmp,"filler    ");
  }

 fprintf(dmp,"\n");
 fprintf(dmp,"%-10p",  p->bufPtr   );
 fprintf(dmp,"%-10d",  p->count    );
 fprintf(dmp,"%-10x",  p->userFlags);
 fprintf(dmp,"%-10d",  p->bufLen   );
 fprintf(dmp,"%-10d",  p->ungetCount);
 fprintf(dmp,"%-10d",  p->tempStore);
 fprintf(dmp,"%-10d",  p->ungetBuf[0]);
 fprintf(dmp,"%-10d",  p->ungetBuf[1]);
 fprintf(dmp,"%-10d",  p->lastOp   );
 fprintf(dmp,"%-10d",  p->filler   );
 fflush(dmp);
 fclose(dmp);
}
void DumpEntryList( void *p,int type )
{
  FILE *dmp;
  extern ULONG __pFirstAllocMem;
  extern ULONG __pLastAllocMem;
  extern ULONG __pFirstFree;
  ULONG pu = (ULONG)p;
  ULONG *pe = (ULONG*)0x1e0000;


  if( AlreadyOpen )
    dmp = fopen("logx.dat","a");
  else
  {
    dmp = fopen("logx.dat","w");
    AlreadyOpen = 1;
    fprintf(dmp,"\n");
    fprintf(dmp,"type            ");
    fprintf(dmp,"pFirstFree      ");
    fprintf(dmp,"pLastAllocMem   ");
    fprintf(dmp,"pFirstAllocMem  ");
  }

 fprintf(dmp,"\n");
 if( type == 1 )
  fprintf(dmp,"alloc(%-9x)",pu);
 else
  fprintf(dmp,"free (%-9x)",pu);

 fprintf(dmp,"%-16x",__pFirstFree    );
 fprintf(dmp,"%-16x",__pLastAllocMem );
 fprintf(dmp,"%-16x",__pFirstAllocMem);

 for(;;)
 {
  fprintf(dmp,"\n");
       fprintf(dmp,"%-010x",pe   );
       fprintf(dmp,"%-010x",*pe );
  pe++;fprintf(dmp,"%-010x",*pe );
  pe++;fprintf(dmp,"%-010x",*pe );
  pe++;fprintf(dmp,"%-010x",*pe );
  if( *pe == 0 )
   break;
  pe++;
 }

 fprintf(dmp,"\n");
 fflush(dmp);
 fclose(dmp);
}
/*---------------------------------------------------------------------*/

typedef struct
{
 int   msgnum;
 char *msg;
}ESPQ_MSG;

static ESPQ_MSG  EspMsg[] =
{
 ESP_QMSG_END_SESSION    , "Esp_Qmsg_End_Session"    ,
 ESP_QMSG_NEW_SESSION    , "Esp_Qmsg_New_Session"    ,
 ESP_QMSG_CTRL_BREAK     , "Esp_Qmsg_Ctrl_Break"     ,
 ESP_QMSG_DISCONNECT     , "Esp_Qmsg_Disconnect"     ,
 ESP_QMSG_CONNECT_REQUEST, "Esp_Qmsg_Connect_Request",
 ESP_QMSG_QUE_TERM       , "Esp_Qmsg_Que_Term"       ,
 ESP_QMSG_ERROR          , "Esp_Qmsg_Error"          ,
 ESP_QMSG_EMPTY          , "Esp_Qmsg_Empty"          ,
 ESP_QMSG_NEW_PROCESS    , "Esp_Qmsg_New_Process"    ,
 ESP_QMSG_OPEN_CONNECT   , "Esp_Qmsg_OpenConnect"    ,
 ESP_QMSG_SELECT_SESSION , "Esp_Qmsg_Select_Session" ,
 ESP_QMSG_SELECT_ESP     , "Esp_Qmsg_Select_Esp"     ,
 ESP_QMSG_PARENT_TERM    , "Esp_Qmsg_Parent_Term"    ,
 ESP_QMSG_CHILD_TERM     , "Esp_Qmsg_Child_Term"     ,
  -1 , ""
};

void DumpQue( void *p,void *q)
{
 PREQUESTDATA     pqr = (PREQUESTDATA)p;
 ESP_QUE_ELEMENT *pqe = (ESP_QUE_ELEMENT*)q;
 FILE            *dmp;
 int              i;
 ESPQ_MSG          *pmsg;

  if( AlreadyOpen )
    dmp = fopen("log.dat","a");
  else
  {
    dmp = fopen("log.dat","w");
    AlreadyOpen = 1;
    fprintf(dmp,"\n");
    fprintf(dmp,"msg                  ");
    fprintf(dmp,"RqPid  ");
    fprintf(dmp,"Rqdata ");
    fprintf(dmp,"Ppid  ");
    fprintf(dmp,"Pid   ");
    fprintf(dmp,"Psid  ");
    fprintf(dmp,"Sid   ");
  }

 fprintf(dmp,"\n");
 for( i = 0 , pmsg = EspMsg; pmsg[i].msgnum != -1 ; i++ )
 {
  if( (pqr->ulData == pmsg[i].msgnum ) )
  {
   fprintf(dmp,"%-21s",  pmsg[i].msg );
   break;
  }
 }
 fprintf(dmp,"%-07x",  pqr->pid );
 fprintf(dmp,"%-07x",  pqr->ulData );


 if( pqe )
 {
  fprintf(dmp,"%-06x",  pqe->ParentPid );
  fprintf(dmp,"%-06x",  pqe->ChildPid );
  fprintf(dmp,"%-06x",  pqe->ParentSid );
  fprintf(dmp,"%-06x",  pqe->ChildSid );
 }

 if( pqe == NULL )
 {
  fflush(dmp);
  fclose(dmp);
  return;
 }

{
 #define BUFFER_SIZE 64*1024-1

 char      ModuleName[CCHMAXPATH];
 void     *pProcStatBuf;
 ULONG     flags;

 ULONG     pid = pqe->ChildPid;

 qsPrec_t        *pProcRec  = NULL;     /* ptr to process record section     */
 qsS16Headrec_t  *p16SemRec = NULL;     /* ptr to 16 bit sem section         */

 /****************************************************************************/
 /* - Allocate a 64k buffer. This is the recommended size since a large      */
 /*   system may generate this much. It's allocated on a 64k boundary        */
 /*   because DosQprocStatus() is a 16 bit call and we don't want the        */
 /*   buffer to overlap a 64k boundary.                                      */
 /****************************************************************************/
 flags = PAG_COMMIT|PAG_READ|PAG_WRITE|OBJ_TILE;
 DosAllocMem( &pProcStatBuf,BUFFER_SIZE,flags);
 DosQProcStatus( (ULONG*)pProcStatBuf , BUFFER_SIZE );

 /****************************************************************************/
 /* Define a pointer to the process subsection of information.               */
 /****************************************************************************/
 pProcRec   = (qsPrec_t       *)((qsPtrRec_t*)pProcStatBuf)->pProcRec;
 p16SemRec  = (qsS16Headrec_t *)((qsPtrRec_t*)pProcStatBuf)->p16SemRec;

 /****************************************************************************/
 /* - scan to the proc record for the pid.                                   */
 /****************************************************************************/
 for( ;pProcRec->pid != pid; )
 {
  /****************************************************************************/
  /* Get a pointer to the next process block and test for past end of block.  */
  /****************************************************************************/
  pProcRec = (qsPrec_t *)( (char*)(pProcRec->pThrdRec) +
                                  (pProcRec->cTCB)*sizeof(qsTrec_t));

  if((void*)pProcRec >= (void*)p16SemRec )
  {
   pProcRec = NULL;
   break;
  }
 }

 /****************************************************************************/
 /* - now get the module name for this pid.                                  */
 /* - scan the block of names checking to see if this is one of              */
 /*   the child processes that we want to debug.                             */
 /* - the names may be in one of the following formats:                      */
 /*                                                                          */
 /*      1. case1                                                            */
 /*      2. case1.exe                                                        */
 /*      3. d:\path1\path2\case1                                             */
 /*      4. d:\path1\path2\case1.exe                                         */
 /*                                                                          */
 /* - copy a name from the name block into a local buffer.                   */
 /* - append a .exe if needed.                                               */
 /* - if the exe name contains a path then perform an explicit compare.      */
 /* - if the exe name does not contain a path, then only compare it to       */
 /*   name part of the module name.                                          */
 /*                                                                          */
 /****************************************************************************/
 if(pProcRec != NULL )
 {
  memset(ModuleName,' ',sizeof(ModuleName) );
  DosQueryModuleName(pProcRec->hMte,sizeof(ModuleName),ModuleName);
  fprintf(dmp,"%s", ModuleName );
 }
}
 fflush(dmp);
 fclose(dmp);

}
#endif

void DumpLineNums( MODULE *pModule )
{
 int       n;
 LNOTAB   *p;
 USHORT    NumEntries;
 CSECT    *pCsect;
 FILENAME *pFile;

 FILE *dmp;
 dmp = fopen("log.dat","a");

 fprintf(dmp,"\n\nmid       =%d ",pModule->mid);
 for( pCsect = pModule->pCsects; pCsect != NULL; pCsect=pCsect->next )
 {
  NumEntries = pCsect->NumEntries;
  p          = pCsect->pLnoTab;
  fprintf(dmp,"\nNumEntries=%d", NumEntries );
  fprintf(dmp,"\n       sfi       lno       off");
  for( n=0; n < NumEntries; n++,p++)
  {
   fprintf(dmp,"\n%10d%10d%10x",p->sfi, p->lno, p->off);
  }
 }

 pFile = pModule->pFiles;
 for( ; pFile; pFile=pFile->next )
 {
  fprintf(dmp,"\n%d, %s", pFile->sfi, (char*)(pFile->FileName) + 1 );
 }

 fflush(dmp);
 fclose(dmp);
}

/*****************************************************************************/
/* - dump the internal class/struct record.                                  */
/*****************************************************************************/
void DumpClassRecord( TD_CLASS *pClassRecord )
{
 char  bigname[500];
 char *cp;

 printf("\n");
 printf("\nRecLen        =%d", pClassRecord->RecLen);
 printf("\nRecType       =%x", pClassRecord->RecType);
 printf("\nByteSize      =%d", pClassRecord->ByteSize);
 printf("\nNumMembers    =%d", pClassRecord->NumMembers);
 printf("\nItemListIndex =%d", pClassRecord->ItemListIndex);

 memset(bigname, 0, sizeof(bigname));

 strncpy( bigname, pClassRecord->Name, pClassRecord->NameLen );

 printf("\nName=         = %d,%s", pClassRecord->NameLen, bigname );
}

/*****************************************************************************/
/* - dump the internal class/struct record.                                  */
/*****************************************************************************/
void DumpMemFncRecord( TD_MEMFNC *pIntMemFnc )
{
 char  bigname[500];
 char *cp;

 printf("\n");
 printf("\nRecLen        =%d", pIntMemFnc->RecLen      );
 printf("\nRecType       =%x", pIntMemFnc->RecType     );
 printf("\nTypeQual      =%x", pIntMemFnc->TypeQual    );
 printf("\nProtection    =%x", pIntMemFnc->Protection  );
 printf("\nFuncType      =%d", pIntMemFnc->FuncType    );
 printf("\nSubRecIndex   =%d", pIntMemFnc->SubRecIndex );
 printf("\nvTableIndex   =%d", pIntMemFnc->vTableIndex );
 printf("\nNameLen       =%d", pIntMemFnc->NameLen     );

 memset(bigname, 0, sizeof(bigname));

 strncpy( bigname, pIntMemFnc->Name, pIntMemFnc->NameLen );

 printf("\nName=         = %d,%s", pIntMemFnc->NameLen, bigname );
}

/*****************************************************************************/
/* - dump the internal class member record.                                  */
/*****************************************************************************/
void DumpClsMemRecord( TD_CLSMEM *pIntClsMem )
{
 char  bigname[500];
 char *cp;
 char *pName;
 int   NameLen;

 printf("\n");
 printf("\nRecLen        =%d", pIntClsMem->RecLen      );
 printf("\nRecType       =%x", pIntClsMem->RecType     );
 printf("\nTypeQual      =%x", pIntClsMem->TypeQual    );
 printf("\nProtection    =%x", pIntClsMem->Protection  );
 printf("\nTypeIndex     =%d", pIntClsMem->TypeIndex   );
 printf("\nOffset        =%x", pIntClsMem->Offset      );
 printf("\nNameLen       =%d", pIntClsMem->NameLen     );

 NameLen = pIntClsMem->NameLen;
 pName   = pIntClsMem->Name;
 if( pIntClsMem->TypeQual & 0x01 )
 {
  memset(bigname, 0, sizeof(bigname));

  strncpy( bigname, pName, NameLen );

  printf("\nStaticName=%d,%s", NameLen, bigname );

  pName += NameLen;
  NameLen = *(USHORT*)pName;
  pName += 2;
 }

 memset(bigname, 0, sizeof(bigname));

 strncpy( bigname, pName, NameLen );

 printf("\nMemberName=%d,%s", NameLen, bigname );
}

/*****************************************************************************/
/* - dump the internal class member record.                                  */
/*****************************************************************************/
void DumpBseClsRecord( TD_BSECLS *pIntBseCls )
{
 char  bigname[500];
 char *cp;
 char *pName;
 int   NameLen;

 printf("\n");
 printf("\nRecLen        =%d", pIntBseCls->RecLen      );
 printf("\nRecType       =%x", pIntBseCls->RecType     );
 printf("\nTypeQual      =%x", pIntBseCls->TypeQual    );
 printf("\nProtection    =%x", pIntBseCls->Protection  );
 printf("\nTypeIndex     =%d", pIntBseCls->TypeIndex   );
 printf("\nOffset        =%x", pIntBseCls->Offset      );
}

