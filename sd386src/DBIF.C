/*****************************************************************************/
/* File:                                                                     */
/*                                                                           */
/*   dbif.c                                                                  */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Interface functions.                                                    */
/*                                                                           */
/* History:                                                                  */
/*                                                                           */
/*   04/10/95 Updated.                                                       */
/*                                                                           */
/*****************************************************************************/

#include "all.h"

extern PtraceBuffer  AppPTB;
extern CmdParms      cmd;

extern PROCESS_NODE *pnode;

UINT DataTid;
UINT DataMid;

/*****************************************************************************/
/*  DBPub                                                                    */
/*                                                                           */
/* Description:                                                              */
/*   returns information (type id, module id, and * ) about a given public   */
/*   name.                                                                   */
/*                                                                           */
/* Parameters:                                                               */
/*   pub       pointer to a public name.                                     */
/*   pdf        pointer to debug file structure.                             */
/*                                                                           */
/* Return:                                                                   */
/*   pubaddr    public address.                                              */
/*              NULL for not found.                                          */
/*                                                                           */
/*****************************************************************************/
UINT DBPub( UCHAR *pub , DEBFILE *pdf)
{
 uint      pubaddr;
 MODULE   *mptr;
 uchar     pubname[256];
 uchar     temppub[256];
 uint      publen;
 PUBREC32 *precptr;
 PUBREC16 *precptr16;
 uchar    *pubend;
 int       rc;

/*****************************************************************************/
/* Scan public segments looking for a public name.                           */
/*                                                                           */
/*  1. Scan through the modules in this pdf until we find a match.           */
/*  2. Branch on 16 or 32 bit modules.                                       */
/*  3. Scan through the publics for each module.                             */
/*  4. Convert public names to Z strings and compare case sensitive.         */
/*  5. If no compare, then compare case insensitive unless user said not     */
/*     to.                                                                   */
/*  6. If we find a match, then break and return pubaddr.                    */
/*                                                                           */
/*****************************************************************************/

 pubaddr = NULL;
 for( mptr = pdf->MidAnchor; mptr != NULL; mptr = mptr->NextMod )
 {
  switch( mptr->DbgFormatFlags.Pubs )
  {
   case TYPE_PUB_32:
    precptr=(PUBREC32 *)DBPubSeg(mptr->mid,&publen,pdf);
    if ( precptr != NULL )
    {
     pubend = (uchar *)precptr + publen;
     for( ; (uchar *)precptr < pubend; )
     {
      strncpy(pubname,(uchar *)(&precptr->Namelen) + 1,precptr->Namelen);
      pubname[precptr->Namelen] = '\0';
      rc = strcmp( pubname, pub );
      if ( rc && !cmd.CaseSens )
      {
       strcpy( temppub , pub );
       strlwr(temppub);
       strlwr(pubname );
       rc = strcmp( pubname, temppub );
      }

      if ( !rc )
      {
/***?*/DataMid = mptr->mid;
/***?*/DataTid = precptr->Typeid;
       pubaddr = precptr->Offset;
       return(pubaddr);
      }
      precptr = (PUBREC32 *)NextPubRec32(precptr);
     }
    }
    break;

   case TYPE_PUB_16:
    precptr16=(PUBREC16 *)DBPubSeg(mptr->mid,&publen,pdf);
    if ( precptr16 != NULL )
    {
     pubend = (uchar *)precptr16 + publen;
     for( ; (uchar *)precptr16 < pubend; )
     {
      strncpy( pubname,(uchar *)(&precptr16->Namelen)+1,precptr16->Namelen);
      pubname[precptr16->Namelen] = '\0';
      rc = strcmp( pubname, pub );
      if ( rc && !cmd.CaseSens )
      {
       strcpy( temppub , pub );
       strlwr(temppub);
       strlwr(pubname );
       rc = strcmp( pubname, temppub );
      }
      if ( !rc )
      {
/***?*/DataMid = mptr->mid;
/***?*/DataTid = GetInternal0_16PtrIndex( precptr16->Typeid );
       pubaddr =  precptr16->Pub16Addr.FlatAddr;
       return(pubaddr);
      }
      precptr16 = (PUBREC16 *)NextPubRec16(precptr16);
     }
    }
    break;
  }
 }
 return ( pubaddr );
}

/*****************************************************************************/
/* DBPut                                                                     */
/*                                                                           */
/* Description:                                                              */
/*   puts data into the user's application address space.                    */
/*                                                                           */
/* Parameters:                                                               */
/*   address    pointer to address space in user's application.              */
/*   nbytes     number of bytes to copy to the user's application.           */
/*   source     pointer to buffer of data that is targeted for the app.      */
/*                                                                           */
/* Return:                                                                   */
/*   rc         whether it worked (0) or not (1).                            */
/*                                                                           */
/*****************************************************************************/
  uint
DBPut( uint   address,
       uint  nbytes,
       uchar *source)
{
  uint  rc;
  PtraceBuffer ptb;

    if( (address >> REGADDCHECKPOS) == REGISTERTYPEADDR )
    {
     *(uint*)( (uint)&AppPTB + (address & REGISTERADDRMASK) )
                = *(uint*)source;
     return( rc=0 );
    }

  memset(&ptb,0,sizeof(ptb));
  ptb.Pid = DbgGetProcessID();
  ptb.Cmd = DBG_C_WriteMemBuf;
  ptb.Addr =  (ulong)address;
  ptb.Buffer = (ulong)source;
  ptb.Len = (ulong)nbytes;
  rc = xDosDebug( &ptb );
  if(rc || (ptb.Cmd != DBG_N_Success ))
    return ( 1 );

  return ( 0 );
}

/*****************************************************************************/
/* DBNextMod()                                                               */
/*                                                                           */
/* Description:                                                              */
/*   Return the next sequential mid.                                         */
/*                                                                           */
/* Parameters:                                                               */
/*   mid        we want the mid after this one.                              */
/*              for mid = 0 return fisrt mid in chain.                       */
/*              for mid = non-zero return next mid in the chain.             */
/*              return mid = 0 if no more mids.                              */
/*   pdf        pointer to debug file structure.                             */
/*                                                                           */
/* Return:                                                                   */
/*   pModule->mid  the next mid in the chain.                                */
/*                                                                           */
/* Assumptions:                                                              */
/*   There exists at least one mid.                                          */
/*   The mid you're looking for exists.                                      */
/*                                                                           */
/*****************************************************************************/
UINT DBNextMod( UINT mid, DEBFILE *pdf )
{
 MODULE   *pModule;

 pModule = pdf->MidAnchor;
 if ( mid == 0 )
  return( pModule->mid );

 for( ;
    pModule->mid != mid;
    pModule = pModule->NextMod
   ){;}

 if ( pModule->NextMod )
  return( pModule->NextMod->mid );

 return( 0 );
}

/*****************************************************************************/
/* DBGet                                                                     */
/*                                                                           */
/* Description:                                                              */
/*   gets data from the user's application address space.                    */
/*                                                                           */
/* Parameters:                                                               */
/*   address    pointer to address space in user's application.              */
/*   nbytes     number of bytes to copy from the user's application.         */
/*   totlptr    pointer to number of bytes that were read in.                */
/*                                                                           */
/* Return:                                                                   */
/*   p          pointer to DBGet buffer that holds the bytes read in.        */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   address  is a flat address.                                             */
/*                                                                           */
/*****************************************************************************/
  uchar*
DBGet( uint address, uint nbytes, uint *totlptr  )
{
 UCHAR *pbytes;

/*****************************************************************************/
/* Fisrt check if the address is a register address, to do this we check     */
/* the 8 most significant bits for the bit pattern 01000000 if it is equal   */
/* then return the pointer to the appropriate register field in the app      */
/* ptrace buffer. stuff in the length of size of reg in the bytes read       */
/* pointer.                                                                  */
/*                                                                           */
/* NOTE:                                                                     */
/*   The checking for register address had been changed from checking two    */
/* MSB's to 8 MSB's because you can have a junk address which has the two    */
/* MSB's set and you fall into trap.                                         */
/*****************************************************************************/

    if( (address >> REGADDCHECKPOS) == REGISTERTYPEADDR )
    {
       *totlptr = sizeof(AppPTB.EAX);
       return( (uchar *)(&AppPTB) + (address & REGISTERADDRMASK) );
    }


  pbytes = GetDataBytes( address, nbytes, totlptr  );
 return(pbytes);
}

/*****************************************************************************/
/*  DBFindProcName()                                                         */
/*                                                                           */
/* Description:                                                              */
/*   Get a pointer to the function name.                                     */
/*                                                                           */
/* Parameters:                                                               */
/*   addr       an address within some function.                             */
/*   pdf        pointer to debug file containing this address.               */
/*                                                                           */
/* Return:                                                                   */
/*              pointer to the function name.                                */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*  pdf contains the addr.                                                   */
/*  addr != null.                                                            */
/*                                                                           */
/*****************************************************************************/
#define ASMLEVEL 1

UCHAR *DBFindProcName( ULONG addr , DEBFILE *pdf )
{
  static  char  ProcNameBuffer[MAXNAMELEN];
  uint     mid;
  uchar    *precptr;
  uchar    *saveptr;
  uchar  *pubend;
  uint    publen;
  void   *scope;
  MODULE *pModule;
  uint    delta;
  uint    NewDelta;
  uint    PubOffset;
/*****************************************************************************/
/* What we're going to do is find the function name that contains an address.*/
/* If we don't have symbol information for the module, then we'll use the    */
/* public info and take a guess.                                             */
/*                                                                           */
/* compile|link switch|comment        | logic                                */
/* switch |switch     |               |                                      */
/*   Zi   |  CO       |               |                                      */
/* _______|___________|_______________|___________________________           */
/* 1.yes  |  no       | no debug info |                                      */
/* 2.yes  |  yes      | use symbols   |                                      */
/* 3.no   |  yes      | use publics   | best fit public name. not guaranteed.*/
/* 4.no   |  no       | no debug info |                                      */
/*                                                                           */
/* 1. Check for cases 1 and 4. If no debug info, then can't find a name.     */
/* 2. Get the mid for this address in cases 2 and 3.                         */
/* 3. Get a ptr to the scope record if this falls in case 2.                 */
/* 4. Return a ptr to the function name.                                     */
/* 5. Or, fall through to case 3.                                            */
/*                                                                           */
/*****************************************************************************/
 if( pdf->SrcOrAsm == ASMLEVEL )
  return( NULL );

 mid=GetMid(addr,pdf);
 if( mid == 0 )
  return( NULL );

 scope = LocateScope(addr,mid,pdf);
 if( scope != NULL )
 {
  pModule = GetPtrToModule( mid, pdf );
  if( pModule != NULL )
  {
   switch( pModule->DbgFormatFlags.Syms )
   {
    case TYPE104_C211:
    case TYPE104_C600:
    case TYPE104_CL386:
    case TYPE104_HL01:
    case TYPE104_HL02:
    case TYPE104_HL03:
    case TYPE104_HL04:
     return((uchar*)&(((SSProc *)scope)->NameLen) );
   }
  }
 }

/*****************************************************************************/
/* If we get here, then we have public information for the module but no     */
/* symbols.                                                                  */
/*                                                                           */
/* 1. Get a ptr the publics for this module.                                 */
/* 2. Get a ptr the module structure for the mid.                            */
/* 3. Branch on 16 or 32 bit public segment.                                 */
/* 4. Scan the publics updating the closest one to addr.                     */
/* 5. Return a pointer to the name in the public record.                     */
/*                                                                           */
/*****************************************************************************/
 precptr = DBPubSeg(mid, &publen, pdf);
 if ( !precptr )
  return( NULL );
 pubend = precptr + publen;
 saveptr = NULL;
 delta =  0xFFFFFFFF;
 NewDelta = 0;
 pModule = GetPtrToModule( mid, pdf );

 switch( pModule->DbgFormatFlags.Pubs )
 {
  case TYPE_PUB_32:
  {
   PUBREC32 *pubrec32;

   for( ; precptr < pubend; )
   {
    PubOffset = ((PUBREC32 *)precptr)->Offset;
    if( addr >= PubOffset )
    {
     NewDelta = addr - PubOffset;
     if ( NewDelta < delta )
     {
      delta = NewDelta;
      saveptr = precptr;
     }
    }
    precptr = NextPubRec32(precptr);
   }

   if ( !saveptr )
    return( NULL);

   pubrec32 = (PUBREC32 *)saveptr;
   *(ushort *)&ProcNameBuffer[0] = pubrec32->Namelen;
   memcpy(&ProcNameBuffer[2], &pubrec32->Namelen+1, pubrec32->Namelen);
   return( &ProcNameBuffer[0] );
  }


  case TYPE_PUB_16:
  {
   PUBREC16 *pubrec16;

   for( ; precptr < pubend; )
   {
    PubOffset = ((PUBREC16 *)precptr)->Pub16Addr.FlatAddr;
    if( addr >= PubOffset )
    {
     NewDelta = addr - PubOffset;
     if ( NewDelta < delta )
     {
      delta = NewDelta;
      saveptr = precptr;
     }
    }
    precptr = NextPubRec16(precptr);
   }

   if ( !saveptr )
    return( NULL);

   pubrec16 = (PUBREC16 *)saveptr;
   *(ushort *)&ProcNameBuffer[0] = pubrec16->Namelen;
   memcpy(&ProcNameBuffer[2], &pubrec16->Namelen+1, pubrec16->Namelen);
   return( &ProcNameBuffer[0] );
  }
 }
 return(NULL);
}
/*****************************************************************************/
/* GetMid()                                                                  */
/*                                                                           */
/* Description:                                                              */
/*   get module id for a given instruction address within a pdf.             */
/*                                                                           */
/* Parameters:                                                               */
/*   address    instruction address for which a mid is to be returned.       */
/*   pdf        pointer to debug file structure.                             */
/*                                                                           */
/* Return:                                                                   */
/*   mid        module id for the given instruction.  0 => instruction       */
/*              address was out of range of our collection of mids.          */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   none.                                                                   */
/*                                                                           */
/*****************************************************************************/
UINT GetMid( ULONG addr, DEBFILE *pdf )
{
 MODULE  *pModule;
 UINT     mid;

 /****************************************************************************/
 /* scan the module ring and check for the containment of the parameter      */
 /* addr. return 0 if no containment.                                        */
 /****************************************************************************/
 mid = 0;
 for( pModule = pdf->MidAnchor; pModule != NULL; pModule = pModule->NextMod )
 {
  if( GetCsectWithAddr( pModule, addr ) != NULL )
  {
   mid = pModule->mid;
   break;
  }
 }
 return ( mid );
}

/*****************************************************************************/
/* GetPtrToModule()                                                          */
/*                                                                           */
/* Description:                                                              */
/*   Get a ptr to the MODULE structure for a mid.                            */
/*                                                                           */
/* Parameters:                                                               */
/*   mid        input - module id.                                           */
/*   pdf        input - the debug file that we're to look in.                */
/*                    - NULL ==> do a global search cause don't know pdf.    */
/*                                                                           */
/* Return:                                                                   */
/*   pModule       -> to the module structure.                               */
/*   NULL       didn't find it.                                              */
/*                                                                           */
/*****************************************************************************/
 MODULE *
GetPtrToModule( uint mid, DEBFILE *pdf )
{
 MODULE *pModule;
 /****************************************************************************/
 /* Jump to the inner loop if we already know the pdf we want to look in.    */
 /****************************************************************************/
 if( pdf != NULL )
  goto PDFKNOWN;
 for(pdf = pnode->ExeStruct; pdf != NULL ; pdf=pdf->next )
 {
PDFKNOWN:
  for( pModule = pdf->MidAnchor; pModule != NULL; pModule = pModule->NextMod )      /*101*/
  {
   if ( pModule->mid == mid )
    return(pModule);
  }
 }
 return( NULL );
}
/*****************************************************************************/
/* IsNearPtr16Or32()                                                         */
/*                                                                           */
/* Description:                                                              */
/*   determine whether a near pointer is 0:16 or 0:32.                       */
/*                                                                           */
/* Parameters:                                                               */
/*   mid        input - module id.                                           */
/*   ptr        input - near pointer we're trying to resolve.                */
/*   sfx        input - stack frame index for a stack variable.              */
/*                                                                           */
/* Return:                                                                   */
/*   BIT16                                                                   */
/*   BIT32                                                                   */
/*   BITUNKNOWN                                                              */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
 uchar
IsNearPtr16Or32( uint mid , uint ptr , uint sfx )
{
 uchar   bitness;
 MODULE *pModule;
 /****************************************************************************/
 /* We have to determine whether the pointer is 0:16 or 0:32.                */
 /* This is a hole in the debug format in that a 0:16 and 0:32               */
 /* should have a different type specifier. Currently, they both             */
 /* have an A* format.                                                       */
 /*                                                                          */
 /* So, let's probe in this order:                                           */
 /*  Stack   - mappable to a stack frame.                                    */
 /*  Data    - mappable to a data object.                                    */
 /*  Dynamic - if it doesn't map to either of the above.                     */
 /*            ( use $$type format flag for the mid ).                       */
 /*                                                                          */
 /****************************************************************************/
 if( TestBit(ptr,STACKADDRBIT) )
  bitness = StackFrameMemModel( sfx );
 else
  bitness = GetBitness( ptr );
 /****************************************************************************/
 /* Must be a ptr in dynamic memory.                                         */
 /****************************************************************************/
 if( bitness == (uchar)BITUNKNOWN )
 {
  pModule = GetPtrToModule( mid , NULL );
  if( pModule == NULL )
   return( BITUNKNOWN );
  bitness = BIT16;
  if( (pModule->DbgFormatFlags.Typs == TYPE103_CL386) ||
      (pModule->DbgFormatFlags.Typs == TYPE103_HL01)  ||
      (pModule->DbgFormatFlags.Typs == TYPE103_HL02) )
   bitness = BIT32;
 }
 return( bitness );
}


/*****************************************************************************/
/* GetCsectWithAddr()                                                        */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   get a pointer to the memory object that contains this address.          */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   pdf        pointer to debug file with the data.                         */
/*   addr       contained address.                                           */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   pCsect -> csect.                                                        */
/*              NULL.                                                        */
/*                                                                           */
/*****************************************************************************/
CSECT *GetCsectWithAddr(  MODULE *pModule, ULONG addr )
{
 CSECT  *pCsect;
 ULONG   addrlo;
 ULONG   addrhi;

 pCsect = NULL;
 if( pModule )
 {
  pCsect = pModule->pCsects;

  for( ; pCsect != NULL; pCsect=pCsect->next )
  {
   addrlo = pCsect->CsectLo;
   addrhi = pCsect->CsectHi;

   if( (addr >= addrlo) && (addr <= addrhi) )
    break;
  }
 }
 return( pCsect );
}
