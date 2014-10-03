#include "all.h"

char *GetProcessStatus( ULONG );
char *GetProcessState( ULONG );
char *GetShrMemName( qsMrec_t *, USHORT );
char *GetSysSemName( qsS16Headrec_t *, USHORT );
void  GetSlotOwner( qsPrec_t *, USHORT, USHORT *, char *, ULONG, void *);

 #define BUFFER_SIZE 64*1024-1

void QueryProcesses( void )
{
 APIRET      rc;
 char        ModuleName[CCHMAXPATH];
 int         n;
 USHORT     *p;
 USHORT      count;
 qsTrec_t   *pt;
 qsS16rec_t *ps;
 qsLrec_t   *pl;
 qsMrec_t   *pm;
 USHORT      SemOwner;
 USHORT     *pmte;
 char       *cp;
 ULONG       sleepid;
 USHORT      semindex;
 USHORT      semid;
 USHORT      slotnum;
 USHORT      tid;
 FILE       *dmp;
 ULONG       flags;
 void       *buffer;

 qsGrec_t        *pGlobalRec;           /* ptr to the global data section    */
 qsPrec_t        *pProcRec;             /* ptr to process record section     */
 qsS16Headrec_t  *p16SemRec;            /* ptr to 16 bit sem section         */
 qsMrec_t        *pShrMemRec;           /* ptr to shared mem section         */
 qsLrec_t        *pLibRec;              /* ptr to exe module recrd section   */


 dmp = fopen("log.dat","w");


 /****************************************************************************/
 /* Read the status buffer.                                                  */
 /****************************************************************************/
 flags = PAG_COMMIT|PAG_READ|PAG_WRITE|OBJ_TILE;
 DosAllocMem( &buffer,BUFFER_SIZE,flags);
 DosQProcStatus( (ULONG*)buffer , BUFFER_SIZE );

 /****************************************************************************/
 /* Define the pointers to the various sections of information.              */
 /****************************************************************************/
 pGlobalRec = (qsGrec_t       *)((qsPtrRec_t*)buffer)->pGlobalRec;
 pProcRec   = (qsPrec_t       *)((qsPtrRec_t*)buffer)->pProcRec;
 p16SemRec  = (qsS16Headrec_t *)((qsPtrRec_t*)buffer)->p16SemRec;
 pShrMemRec = (qsMrec_t       *)((qsPtrRec_t*)buffer)->pShrMemRec;
 pLibRec    = (qsLrec_t       *)((qsPtrRec_t*)buffer)->pLibRec;

 /****************************************************************************/
 /* Print the Global Data Section.                                           */
 /****************************************************************************/
 fprintf(dmp,"\nGlobal Data Section\n");
 fprintf(dmp,"\n   Number of Threads in Use = %d",pGlobalRec->cThrds);
 fprintf(dmp,"\n\n===============================================================");

 /****************************************************************************/
 /* Print the Process Data Section.                                          */
 /****************************************************************************/
 fprintf(dmp,"\nProcess Data Section\n");

 for(;;)
 {
  memset(ModuleName,' ',sizeof(ModuleName) );
  rc = DosQueryModuleName(pProcRec->hMte,sizeof(ModuleName),ModuleName);
  if(rc != 0 )
   fprintf(dmp,"\nDosQueryModuleName() Error rc=%d",rc);

  fprintf(dmp,"\n  %s",ModuleName);
  fprintf(dmp,"\n    Process ID                   = %-6d(%#04x)",pProcRec->pid,
                                                            pProcRec->pid);

  fprintf(dmp,"\n    Parent Process ID            = %-6d(%#04x)",pProcRec->ppid,
                                                            pProcRec->ppid);

  fprintf(dmp,"\n    Process Type                 = %-6d(%#04x)",pProcRec->type,
                                                            pProcRec->type);

  cp = GetProcessStatus(pProcRec->stat);
  fprintf(dmp,"\n    Process Status               = %-6d(%#04x) %s",pProcRec->stat,
                                                               pProcRec->stat,
                                                               cp);

  fprintf(dmp,"\n    Process Screen Grp           = %-6d(%#04x)",pProcRec->sgid,
                                                            pProcRec->sgid);

  fprintf(dmp,"\n    Process Mte                  = %-6d(%#04x)",pProcRec->hMte,
                                                            pProcRec->hMte);

  fprintf(dmp,"\n    Number of TCBs               = %d",pProcRec->cTCB);
  fprintf(dmp,"\n    Number of 16 bit Sema4s      = %d",pProcRec->c16Sem);
  fprintf(dmp,"\n    Number of Runtime Libs       = %d",pProcRec->cLib);
  fprintf(dmp,"\n    Number of Shared Mem Handles = %d",pProcRec->cShrMem);

  /****************************************************************************/
  /* Define the pointers to the sema4,lib, and shared memory blocks for       */
  /* this process.                                                            */
  /****************************************************************************/
  count = pProcRec->cLib;
  if(count != 0)
  {
   fprintf(dmp,"\n\n    DLL Handles/Names");
   for(p=pProcRec->pLibRec,n=1; n<=count ;n++,p++)
   {
    strcpy(ModuleName,"Invalid");
    DosQueryModuleName(*p,sizeof(ModuleName),ModuleName);
    fprintf(dmp,"\n      %-6d(%04x) %s",*p,*p,ModuleName);
   }
  }

  count = pProcRec->c16Sem;
  if(count != 0)
  {
   fprintf(dmp,"\n\n    16 Bit Sema4 Indices\n");
   for(p=pProcRec->p16SemRec,n=1; n<=count ;n++,p++)
    fprintf(dmp,"\n      %-6d(%#04x) %s",*p,*p,"xxxxx");
  }

  count = pProcRec->cShrMem;
  if(count != 0)
  {
   fprintf(dmp,"\n\n    Shared Memory Handles/Names\n");
   for(p=pProcRec->pShrMemRec,n=1; n<=count ;n++,p++)
   {
    cp = GetShrMemName(pShrMemRec,*p);
    fprintf(dmp,"\n      %-6d(%04x) %s",*p,*p,cp);
   }
  }
  fprintf(dmp,"\n");


  /***************************************************************************/
  /* Now, print the thread info for this process.                            */
  /***************************************************************************/
  fprintf(dmp,"\n\n    Thread Info\n");
  fprintf(dmp,"\n      tid   slot   sleepid  priority systime  usertime state");
  fprintf(dmp,"\n      (hex) (hex)  (hex)    (hex)    (hex)    (hex)    (hex)");

  count = pProcRec->cTCB;
  pt = pProcRec->pThrdRec;
  for(n=1; n <= count; n++ )
  {
   fprintf(dmp,"\n      %04x  ",pt->tid);
   fprintf(dmp,"%04x   " ,pt->slot);
   fprintf(dmp,"%08x "   ,pt->sleepid);
   fprintf(dmp,"%08x "   ,pt->priority);
   fprintf(dmp,"%08x "   ,pt->systime);
   fprintf(dmp,"%08x "   ,pt->usertime);
   fprintf(dmp,"%08x %s" ,pt->state,GetProcessState(pt->state));
   pt = (qsTrec_t*)( (char*)pt + sizeof(qsTrec_t) );
  }
  fprintf(dmp,"\n");
  /***************************************************************************/
  /* Print the name of the sema4 a thread is blocked on.                     */
  /***************************************************************************/
  count = pProcRec->cTCB;
  pt = pProcRec->pThrdRec;
  for(n=1; n <= count; n++ )
  {
   sleepid  = pt->sleepid;
   semindex = *(USHORT*)( (char*)&sleepid + 2);
   switch( semindex )
   {
    case 0x400:
     semid    = *(USHORT*)&sleepid;
     cp =  GetSysSemName( p16SemRec ,semid);
     fprintf(dmp,"\n      tid=%04x is blocked on system sema4 %s",pt->tid,cp);
     break;

    case 0xfffe:
    case 0xfffd:
     slotnum = *(USHORT*)&sleepid;
     memset(ModuleName,' ',sizeof(ModuleName) );
     GetSlotOwner( pProcRec, slotnum, &tid, ModuleName, sizeof(ModuleName),
                       (void*)p16SemRec );
     if( semindex == 0xfffe )
      fprintf(dmp,"\n      tid=%04x is blocked on Ramsem owned by tid=%04x of %s",
                                                                pt->tid,
                                                                tid,
                                                                ModuleName);
     else
      fprintf(dmp,"\n      tid=%04x is blocked on Muxsem owned by tid=%04x of %s",
                                                                 pt->tid,
                                                                 tid,
                                                                 ModuleName);
      break;

   }
   pt = (qsTrec_t*)( (char*)pt + sizeof(qsTrec_t) );
  }
  fprintf(dmp,"\n  -------------------------------------------------------------\n");
  /****************************************************************************/
  /* Get a pointer to the next process block and test for past end of block.  */
  /****************************************************************************/
  pProcRec = (qsPrec_t *)( (char*)(pProcRec->pThrdRec) +
                                  (pProcRec->cTCB)*sizeof(qsTrec_t));

  if((void*)pProcRec >= (void*)p16SemRec )
   break;
 }
 fprintf(dmp,"\n=================================================================");
 /****************************************************************************/
 /* Print the 16 Bit Semaphore Section.                                      */
 /****************************************************************************/
 fprintf(dmp,"\n16 Bit Semaphore Section\n");
 fprintf(dmp,"\nSema4  Owner  Sema4  Ref    Proc ");
 fprintf(dmp,"\nIndex  Slot   Flag   Count  count");
 fprintf(dmp,"\n(hex)  (hex)  (hex)  (dec)  (dec)\n");

 ps = (qsS16rec_t*)( (char*)p16SemRec + sizeof(qsS16Headrec_t) );
 SemOwner = ps->s_SysSemOwner;
 for(; ps != NULL ; ps = (qsS16rec_t*)ps->NextRec)
 {
  if( ps->s_SysSemOwner != 0 )
   SemOwner = ps->s_SysSemOwner;
  fprintf(dmp,"\n%04x   "  ,ps->s_SysSemNameIndex.SemIndex);
  fprintf(dmp,"%04x   "    ,SemOwner);
  fprintf(dmp,"%02x     "  ,ps->s_SysSemFlag);
  fprintf(dmp,"%-#7d"      ,ps->s_SysSemRefCnt);
  fprintf(dmp,"%-#7d"      ,ps->s_SysSemProcCnt);
  fprintf(dmp,"%s"         ,&(ps->s_SysSemNameIndex.SemName.SemName));

 }
 fprintf(dmp,"\n=================================================================");
 /****************************************************************************/
 /* Print the Executable Module Section.                                     */
 /****************************************************************************/
 fprintf(dmp,"\n\nExecutable Module Section");
 fprintf(dmp,"\n\nmte  Name\n");

 pl = (qsLrec_t*)pLibRec;
 for( ;pl != NULL; pl = pl->pNextRec)
 {
  fprintf(dmp,"\n\n%04X %s",pl->hmte,pl->pName);
  pmte = (USHORT*)&(pl->ImpModTable);
  for(n = 1; n <= (int)pl->ctImpMod; n++,pmte++ )
  {
   fprintf(dmp,"\n%04X",*pmte);
   strcpy(ModuleName,"Invalid");
   DosQueryModuleName(*pmte,sizeof(ModuleName),ModuleName);
   fprintf(dmp,"         %s",ModuleName);
  }
 }
 fprintf(dmp,"\n=================================================================");
 /****************************************************************************/
 /* Print the Shared Memory Section.                                         */
 /****************************************************************************/
 fprintf(dmp,"\n\nShared Memory Section\n");
 fprintf(dmp,"\nMem           Ref        ");
 fprintf(dmp,"\nHandle Sel    Count  Name");
 fprintf(dmp,"\n(hex)  (hex)  (dec)\n");

 pm = (qsMrec_t*)pShrMemRec;
 for(; pm != NULL ; pm = (qsMrec_t*)pm->pNextRec)
 {
  fprintf(dmp,"\n%04x   " ,pm->hmem);
  fprintf(dmp,"%04x   "   ,pm->sel);
  fprintf(dmp,"%-#7d"     ,pm->refcnt);
  fprintf(dmp,"%s"        ,&(pm->Memname));
 }
 fclose(dmp);
}

/*****************************************************************************/
/* Get the meaning of the status value.                                      */
/*****************************************************************************/
static STATUS statdefs[] =
{
 0x00,"Normal",
 0x01,"Doing ExitList Processing"     ,
 0x02,"Exiting thread 1"              ,
 0x04,"The whole process is exiting"  ,
 0x10,"Parent cares about termination",
 0x20,"Parent did an exec-and-wait"   ,
 0x40,"Process is dying"              ,
 0x80,"Process in embryonic state"    ,
 0xFF,NULL
};

char *GetProcessStatus( ULONG status)
{
 int    i;
 ULONG  temp;

 for( i=0,temp=statdefs[0].stat;
      (temp != 0xff) && (temp != status);
      temp=statdefs[++i].stat
    ){;}
 return(statdefs[i].statmsg);
}

/*****************************************************************************/
/* Get the meaning of the status value.                                      */
/*****************************************************************************/
static STATUS statedefs[] =
{
 0x01, "Ready to run" ,
 0x02, "Blocked"      ,
 0x05, "Running"      ,
 0x09, "Frozen"       ,
 0xFF,NULL
};

char *GetProcessState( ULONG state)
{
 int    i;
 ULONG  temp;

 for( i=0,temp=statedefs[0].stat;
      (temp != 0xff) && (temp != state);
      temp=statedefs[++i].stat
    ){;}
 return(statedefs[i].statmsg);
}

/*****************************************************************************/
/* Get the name of a shared memory segment.                                  */
/*****************************************************************************/
char *GetShrMemName( qsMrec_t *pShrMemRec, USHORT hmem)
{
 qsMrec_t *pm;

 pm = pShrMemRec;
 for(; pm != NULL ; pm = (qsMrec_t*)pm->pNextRec )
 {
  if( pm->hmem == hmem)
   return( &(pm->Memname) );
 }
 return(NULL);
}

/*****************************************************************************/
/* Get the name of a system semaphore.                                       */
/*****************************************************************************/
char *GetSysSemName( qsS16Headrec_t *p16SemRec, USHORT semid)
{
 qsS16rec_t *ps;

 ps = (qsS16rec_t*)( (char*)p16SemRec + sizeof(qsS16Headrec_t) );
 for(; ps != NULL ; ps = (qsS16rec_t*)ps->NextRec)
 {
  if( semid == ps->s_SysSemNameIndex.SemIndex)
   return( &(ps->s_SysSemNameIndex.SemName.SemName) );
 }
 return(NULL);
}

/*****************************************************************************/
/* Get the owner of a slot.                                                  */
/*****************************************************************************/
void GetSlotOwner( qsPrec_t *pProcRec,
                       USHORT slotnum ,
                       USHORT *ptid,
                       char   *pModuleName,
                       ULONG   buflen,
                       void   *pend
                 )
{
 qsTrec_t *pt;
 USHORT    count;
 int       n;

 for(;;)
 {
  count = pProcRec->cTCB;
  pt = pProcRec->pThrdRec;
  for(n=1; n <= count; n++ )
  {
   if(pt->slot == slotnum)
   {
    *ptid = pt->tid;
    DosQueryModuleName(pProcRec->hMte,buflen,pModuleName);
    return;
   }
   pt = (qsTrec_t*)( (char*)pt + sizeof(qsTrec_t) );
  }
  /****************************************************************************/
  /* Get a pointer to the next process block and test for past end of block.  */
  /****************************************************************************/
  pProcRec = (qsPrec_t *)( (char*)(pProcRec->pThrdRec) +
                                  (pProcRec->cTCB)*sizeof(qsTrec_t));

  if((void*)pProcRec >= (void*)pend )
   break;
 }
}
