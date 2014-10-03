/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   pointer.c                                                               */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   pointer handling routines.                                              */
/*                                                                           */
/*                                                                           */
/* History:                                                                  */
/*                                                                           */
/*   08/15/91 Creation of 32-bit SD386.                                      */
/*                                                                           */
/*...Release 1.00 (Pre-release 1)                                            */
/*...                                                                        */
/*... 07/09/91  205   srinivas  Hooking up of register variables.            */
/*... 07/26/91  219   srinivas  handling near pointers.                      */
/*... 09/25/91  240   Srinivas  recursive PLX based variables.               */
/*...                                                                        */
/*...Release 1.00 (Pre-release 1.08 10/10/91)                                */
/*...                                                                        */
/*... 02/07/92  512   Srinivas  Handle Toronto "C" userdefs.                 */
/*...                                                                        */
/*...Release 1.02 (10/22/92)                                                 */
/*...                                                                        */
/*... 03/03/93  813   Joe       Revised types handling for HL03.             */
/*...                                                                        */
/*****************************************************************************/

#include "all.h"                        /* SD386 include files               */

/**External declararions******************************************************/

extern uint         ExprTid;            /* Set by ParseExpr and findlvar     */
extern SCOPE        ExprScope;          /* Set by ParseExpr and findlvar.    */
extern ushort       DgroupDS;           /* ds for handling near pointers  219*/

/*****************************************************************************/
/*  DerefPointer()                                                           */
/*                                                                           */
/* Description:                                                              */
/*   Dereference a pointer.                                                  */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   ptr       input - pointer that we want to dereference.                  */
/*   mid       input - module id.                                            */
/*                                                                           */
/* Return:                                                                   */
/*   *p        the new pointer.                                              */
/*   NULL                                                                    */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   none.                                                                   */
/*                                                                           */
/*****************************************************************************/
 uint                                   /*                                   */
DerefPointer( uint ptr , uint mid )     /*                            240 112*/
{                                       /*                                   */
 uint     sfx;                          /* stack frame index.                */
 uint    *p;                            /*                                   */
 uint     read;                         /* number of bytes read by DBGet     */
 uint     size;                         /*                                   */
 uchar    type;                         /* type of pointer                219*/
 uint     derefPtr;                     /* dereferneced pointer           219*/
 int      IsStackAddr = 0;

 sfx = StackFrameIndex( ExprScope );
 IsStackAddr = TestBit(ptr,STACKADDRBIT);
 if( IsStackAddr )
 {
   if( sfx == 0 )
     goto BadAddr;
   ptr = StackBPRelToAddr( ptr , sfx );
   if( ptr == NULL )
     goto BadAddr;
 }
 else
  if( (ptr >> REGADDCHECKPOS) == REGISTERTYPEADDR ){                    /*205*/
     if( sfx != 1 )                     /* If not in the executing frame  112*/
         goto BadAddr;                                                  /*112*/
 }                                                                      /*112*/

 type = GetPtrType(mid,ExprTid);                                     /*813240*/
                                        /* get the type of pointer        219*/
 if ( type == PTR_0_16 )                /*                                219*/
   size = 2;                            /* set the size of ptr depending  219*/
 else                                   /* on type of ptr.                219*/
   size = 4;                            /*                                219*/

/*****************************************************************************/
/* At this point, we are ready to return the fruits of our labor. We get     */
/* "pointer size" bytes from the users stack at location ptr if we can.      */
/*                                                                           */
/*****************************************************************************/
 p = (uint *)DBGet(ptr, size, &read );
 if( p == NULL )
  goto BadAddr;


 derefPtr = (uint) *p;                 /* get the value of deref pointer 219*/

/*****************************************************************************/
/* convert the deref pointer into correct flat adddress depending on type 219*/
/*****************************************************************************/
 switch (type)                                                          /*219*/
 {                                                                      /*219*/
   case PTR_0_16:                                                       /*219*/
     derefPtr = Data_SelOff2Flat(DgroupDS,LoFlat(derefPtr));
     break;                                                             /*219*/
                                                                        /*219*/
   case PTR_16_16:                                                      /*219*/
     derefPtr = Data_SelOff2Flat( HiFlat(derefPtr) , LoFlat(derefPtr) );
     break;                                                             /*219*/
 }                                                                      /*219*/
                                                                        /*219*/
 return( derefPtr );                                                    /*219*/

BadAddr:
 return( NULL );
}

/*****************************************************************************/
/*  ResolveAddr()                                                            */
/*                                                                           */
/* Description:                                                              */
/*   Resolves a address to the correct flat address.                         */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   addr      input - the address to be resolved.                           */
/*   dfp       input - -> to the dfile node.                                 */
/*   typeno    input - the typeno of the variable.                           */
/*                                                                           */
/* Return:                                                                   */
/*   FlatAddr  resolved correct address.                                     */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
 uint                                                                   /*219*/
ResolveAddr( uint Addr , DFILE *dfp , uint typeno)                      /*219*/
{                                                                       /*219*/
  uint FlatAddr;                                                        /*219*/
                                                                        /*219*/
  FlatAddr = Addr;                                                      /*219*/
  switch (GetPtrType(dfp->mid,typeno) )                              /*813219*/
  {                                                                     /*219*/
    case PTR_0_16:                                                      /*219*/
      FlatAddr = Data_SelOff2Flat(DgroupDS,LoFlat(FlatAddr));
      break;                                                            /*219*/
                                                                        /*219*/
    case PTR_16_16:                                                     /*219*/
      FlatAddr = Data_SelOff2Flat( HiFlat(Addr),LoFlat(Addr) );
      break;                                                            /*219*/
  }                                                                     /*219*/
  return(FlatAddr);                                                     /*219*/
}


/*****************************************************************************/
/* GetPtrType()                                                           813*/
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Resolve a typeno to its pointer type.                                   */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   mid        input - module id.                                           */
/*   typeno     input - primitive or complex pointer typeno.                 */
/*                                                                           */
/* Return:                                                                   */
/*              PTR_0_16.         16 bit near.                               */
/*              PTR_0_32.         32 bit near.                               */
/*              PTR_16_16.        16 bit far.                                */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   typeno must be a primitive pointer type or a complex TD_POINTER.        */
/*                                                                           */
/*****************************************************************************/
UINT GetPtrType( UINT mid, USHORT typeno )
{
 TD_POINTER  *PtrToBaseTypeRec;

ReCycle:
 if( typeno < 512 )
 {
  /***************************************************************************/
  /* Handle primitive pointer types.                                         */
  /***************************************************************************/
  switch(typeno)
  {

    case TYPE_PCHAR:
    case TYPE_PSHORT:
    case TYPE_PLONG:
    case TYPE_PUCHAR:
    case TYPE_PUSHORT:
    case TYPE_PULONG:
    case TYPE_PFLOAT:
    case TYPE_PDOUBLE:
    case TYPE_PLDOUBLE:
    case TYPE_PVOID:
     return(PTR_0_32);

    case TYPE_N16PCHAR:
    case TYPE_N16PSHORT:
    case TYPE_N16PLONG:
    case TYPE_N16PUCHAR:
    case TYPE_N16PUSHORT:
    case TYPE_N16PULONG:
    case TYPE_N16PFLOAT:
    case TYPE_N16PDOUBLE:
    case TYPE_N16PLDOUBLE:
    case TYPE_N16PVOID:
     return(PTR_0_16);

   case TYPE_FPCHAR:
   case TYPE_FPSHORT:
   case TYPE_FPLONG:
   case TYPE_FPUCHAR:
   case TYPE_FPUSHORT:
   case TYPE_FPULONG:
   case TYPE_FPFLOAT:
   case TYPE_FPDOUBLE:
   case TYPE_FPLDOUBLE:
   case TYPE_FPVOID:
    return(PTR_16_16);

   default:
    return(PTR_0_32);
  }
 }

 /****************************************************************************/
 /* Handle TD_POINTER type.                                                  */
 /****************************************************************************/
 PtrToBaseTypeRec = (TD_POINTER*)QbasetypeRec(mid, typeno);
 return( PtrToBaseTypeRec->Flags );
}
