/*****************************************************************************/
/* File:                                                                     */
/*   cpydata.c                                                               */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*  Router/formatter for data items for MSH interpreter.                     */
/*                                                                           */
/* History:                                                                  */
/*                                                                           */
/*   03/10/93 Created.                                                       */
/*                                                                           */
/*...Release 1.02 (10/22/92)                                                 */
/*...                                                                        */
/*... 01/26/93  809   Selwyn    HLL Level 2 support.                         */
/*... 03/03/93  813   Joe       Revised types handling for HL03.             */
/*... 03/06/93  814   Joe       Signed 8 bit char not displaying correct     */
/*...                           negative.                                    */
/*****************************************************************************/
#include "all.h"

extern MSHPUTOBJECT       MshPutObject;
extern MSHOBJECTSIZE      mshObjectSize;
extern MSHPUTSIMPLEOBJECT MshPutSimpleObject;
extern MSHINITOBJECT      MshInitObject;
extern MSHGET_DIRECT      mshget_direct;
extern MSHPUT_DIRECT      mshput_direct;

/*****************************************************************************/
/* CopyDataItem()                                                            */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   This routine routes the typeno to the correct formatter.                */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   cp         -> to be allocated to a structure                            */
/*              struct { int  len; int dummy; double *data; }                */
/*   data       location in user's address space of data                     */
/*              we want to format.                                           */
/*   mid        module in which the typeno is defined.                       */
/*   typeno     the type number of the record to be formatted.               */
/*   sfx        stack frame index for stack/parameter variables.             */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   void                                                                    */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*  cp -> to a buffer that has been pre-formatted with blanks.               */
/*  typeno is a valid type number within the mid.                            */
/*                                                                           */
/*****************************************************************************/

void CopyDataItem( char *name, COPYDATA **cp, UINT data, UINT mid,
    USHORT typeno, UINT sfx , UINT nitems ,
    MSHOBJECT *mshObject)
{
 MSHOBJECT *mshObjectHold=NULL, mshObjectTemp;
 Trec  *tp;

 mshObjectHold=mshObject;
 if(!mshObject) {   /*MSHGET pulldown*/
    mshObject=&mshObjectTemp;
    (*MshInitObject)(mshObject);
    (*MshPutSimpleObject)(mshObject,0,UNDEFINED,1,1);
 }
 if(((int)mshObject)==-1) { /*MSHPUT pulldown*/
    /*First call to get data item size.* */
    int m=1, n=1, t=UNDEFINED;
    char msg[80];
    mshObject=&mshObjectTemp;
    (*MshInitObject)(mshObject);

    /*Use t=UNDEFINED to query data size.*/
    (*mshget_direct)(name, &m, &n, &t, (char*)(mshObject->data) );

    /*Create dataless item.*/
    (*MshPutSimpleObject)(mshObject,2,t,m,n);

    mshObject=(MSHOBJECT *)T2alloc(sizeof(double)*(*mshObjectSize)(mshObject),SD386_MSH_CHAIN);
    if(!mshObject) {
       sprintf("CopyDataItem: cannot allocate memory for MSH object %s.",
           name);
       fmterr(msg);
       beep();
       return;
    }

    (*mshget_direct)(name, &m, &n, &t, (char*)(mshObject->data) );
    (*MshPutSimpleObject)(mshObject,2,t,m,n);

    /*Set accessfn field to -1 to indicate to Cpy functions that */
    /*data in being put.*/

    *((int *)&mshObject->accessfn)=-1;

 }
 if( typeno < 512 )
 {
recirculate:
  switch( typeno )
  {
   case TYPE_VOID:
    CpyBytes( cp, data , sfx , 16, mshObject);
    (*cp)->typeno=typeno;
    break;

   case TYPE_CHAR:
   case TYPE_UCHAR:
    CpyChar( cp, data , sfx , typeno, nitems, mshObject);
    break;

   case TYPE_SHORT:
   case TYPE_USHORT:
    CpyShort( cp, data , sfx , typeno, nitems, mshObject);
    break;

   case TYPE_LONG:
   case TYPE_ULONG:
    CpyLong( cp, data , sfx , typeno, nitems, mshObject);
    break;


   case TYPE_FLOAT:
   case TYPE_DOUBLE:
   case TYPE_LDOUBLE:
    CpyFloat( mid,cp, data , sfx , typeno, nitems, mshObject);
    break;

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
    typeno = CpyPtr(cp,(ULONG*)&data,0/*mid=0*/,typeno,sfx,mshObject);
    /*Return pointer.*/
    typeno=TYPE_ULONG;
    break;

  }
  /***************************************************************************/
  /* Remove the \0 inserted by the formatter.                                */
  /***************************************************************************/
#if 0
  *(strchr(cp,'\0')) = ' ';
#endif
msh:
  (*cp)->typeno=typeno;
  mshObject=mshObjectHold;
  if(!(*cp)->rc)
  {
   switch( typeno )
   {
    case TYPE_CHAR:
    case TYPE_UCHAR:
     if(!mshObject) {
         (*mshput_direct)( name, 1, (*cp)->len, 4, (char *) ((*cp)->data) );
     }
     else {
        if(mshObject->mshAttributes.type==UNDEFINED) {
            (*MshPutSimpleObject)(mshObject,0,4, 1, (*cp)->len, (char *) ((*cp)->data) );
        }
        else {
            (*MshPutObject)(mshObject,0,
                mshObject->mshAttributes.type,
                mshObject->ndimensions,
                mshObject->dimensions,
                (char *) ((*cp)->data) );
        }
     }
     break;

    case TYPE_SHORT:
    case TYPE_USHORT:
     if(!mshObject) {
         (*mshput_direct)( name,
             1, ((*cp)->len)/sizeof(SHORT) , 0, (char *) ((*cp)->data) );
     }
     else {
         if(mshObject->mshAttributes.type==UNDEFINED) {
         (*MshPutSimpleObject)(mshObject,0,
             0, 1, ((*cp)->len)/sizeof(SHORT) , (char *) ((*cp)->data) );
        }
        else {
            (*MshPutObject)(mshObject,0,
                mshObject->mshAttributes.type,
                mshObject->ndimensions,
                mshObject->dimensions,
                (char *) ((*cp)->data) );
        }
     }
     break;

    case TYPE_LONG:
    case TYPE_ULONG:
     if(!mshObject) {
         (*mshput_direct)( name,
             1, ((*cp)->len)/sizeof(LONG), 1,(char *) (*cp)->data );
     }
     else {
         if(mshObject->mshAttributes.type==UNDEFINED) {
         (*MshPutSimpleObject)(mshObject,0,
             1, 1, ((*cp)->len)/sizeof(LONG), (char *) (*cp)->data );
        }
        else {
            (*MshPutObject)(mshObject,0,
                mshObject->mshAttributes.type,
                mshObject->ndimensions,
                mshObject->dimensions,
                (char *) ((*cp)->data) );
        }
     }
     break;

    case TYPE_FLOAT:
     if(!mshObject) {
         (*mshput_direct)( name,
             1, ((*cp)->len)/sizeof(float), 3, (char *) (*cp)->data );
     }
     else {
         if(mshObject->mshAttributes.type==UNDEFINED) {
         (*MshPutSimpleObject)(mshObject,0,
             3, 1, ((*cp)->len)/sizeof(float), (char *) (*cp)->data );
        }
        else {
            (*MshPutObject)(mshObject,0,
                mshObject->mshAttributes.type,
                mshObject->ndimensions,
                mshObject->dimensions,
                (char *) ((*cp)->data) );
        }
     }
     break;

    case TYPE_DOUBLE:
    case TYPE_LDOUBLE:
     if(!mshObject) {
         (*mshput_direct)( name,
             1, ((*cp)->len)/sizeof(double), 2, (char *) (*cp)->data );
     }
     else {
         if(mshObject->mshAttributes.type==UNDEFINED) {
         (*MshPutSimpleObject)(mshObject,0,
             2, 1, ((*cp)->len)/sizeof(double), (char *) (*cp)->data );
        }
        else {
            (*MshPutObject)(mshObject,0,
                mshObject->mshAttributes.type,
                mshObject->ndimensions,
                mshObject->dimensions,
                (char *) ((*cp)->data) );
        }
     }
     break;

   }
  }
  FreeChain(SD386_MSH_CHAIN);
  return;
 }
 else
 /****************************************************************************/
 /* - Now, handle typenos > 512.                                             */
 /* - QbasetypeRec may return with tp pointing to a TD_USERDEF record with   */
 /*   a primitive type index. In this case, we grab the primitive typeno     */
 /*   and recirculate.                                                       */
 /****************************************************************************/
 {
  tp = QbasetypeRec(mid,typeno);
  if( tp->RecType == T_TYPDEF )
  {
   typeno = ((TD_USERDEF*)tp)->TypeIndex;
   goto recirculate;
  }
 }

 /****************************************************************************/
 /* When we get here, we have a tp pointing to a base type record and it     */
 /* is not a T_USERDEF record.                                               */
 /****************************************************************************/
 switch(tp->RecType)
 {
  default:
   break;

  case T_SCALAR:
  case T_PROC:
  case T_ENTRY:
  case T_FUNCTION:
  case T_ARRAY:
  case T_STRUCT:
   {
   struct {
      TD_STRUCT *tp;
      char name[80];
   } findStruct;
   strcpy(findStruct.name,name);
   findStruct.tp=(TD_STRUCT *) tp;
   FindStruct(NULL, (char *) &findStruct, mshObjectHold);
   }
   break;

  case T_ENUM:
   typeno = CpyEnums( tp, cp, data, mid, typeno, sfx, mshObject);
   /**************************************************************************/
   /* typeno will be returned as 0 in the event of an error.                 */
   /**************************************************************************/
   if( typeno == 0 )
    break;
   goto recirculate;

  case T_BITFLD:
   CpyBitField( tp, cp, data, mid, typeno, sfx, mshObject);
   break;

casepointer:
  case T_PTR:
   /**************************************************************************/
   /* See the documentation for Pointer handling. It will make the           */
   /* understanding of this much simpler.                                    */
   /**************************************************************************/
   (void)CpyPtr( cp, (ULONG*)&data , mid , typeno , sfx, mshObject);
   /*Here we force return of the pointer.*/
   typeno=TYPE_ULONG;
   goto msh;
 }
}
/*****************************************************************************/
/* CpyChar()                                                              813*/
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Format signed/unsigned character data for display.                      */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   cp         input - -> to the buffer we're formatting into.              */
/*   UserAddr   input - location in user's address space of data             */
/*                      we want to format.                                   */
/*   sfx        input - stack frame index for auto data.                     */
/*   typeno     input - primitive type no. Tells signed/unsigned.            */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   void                                                                    */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*  cp -> to a buffer that has been pre-formatted with blanks.               */
/*                                                                          */
/*****************************************************************************/
#define SIGNED   0x00
#define UNSIGNED 0x04
void CpyChar( COPYDATA  **cp,
              ULONG       UserAddr,
              UINT        sfx,
              USHORT      typeno ,
              UINT        nitems,
              MSHOBJECT  *mshObject)
{
 UINT   read;
 UINT   size;
 signed char *dp;

 size = sizeof(char);
 dp = (signed char *) MshAppData( UserAddr, &nitems, size, &read, sfx, mshObject);
 *cp=(COPYDATA *)T2alloc(sizeof(COPYDATA)+read-sizeof(double),CPY_CHAIN);
 if( !dp || (size*nitems != read) )
  {sprintf((*cp)->buffer,"Invalid Address");(*cp)->rc=-1;return;}

 memcpy((*cp)->data,dp,read);
 (*cp)->len=read;
 (*cp)->rc=0;
}
/*****************************************************************************/
/* CpyShort()                                                             813*/
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Format signed/unsigned primitive short data types.                      */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   cp         input - -> to the buffer we're formatting into.              */
/*   UserAddr   input - location in user's address space of data             */
/*                      we want to format.                                   */
/*   sfx        input - stack frame index for auto data.                     */
/*   typeno     input - primitive type no. Tells signed/unsigned.            */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   void                                                                    */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*  cp -> to a buffer that has been pre-formatted with blanks.               */
/*                                                                           */
/*****************************************************************************/
void CpyShort( COPYDATA  **cp,
               ULONG       UserAddr,
               UINT        sfx ,
               USHORT      typeno ,
               UINT        nitems,
               MSHOBJECT  *mshObject)
{
 UINT   read;
 UINT   size;
 signed short *dp;

 size = sizeof(short);

 dp = (signed short *)MshAppData( UserAddr, &nitems, size, &read, sfx, mshObject);

 *cp=(COPYDATA *)T2alloc(sizeof(COPYDATA)+read-sizeof(double),CPY_CHAIN);
 if( !dp || (size*nitems != read) )
  {sprintf((*cp)->buffer,"Invalid Address");(*cp)->rc=-1;return;}

 memcpy((*cp)->data,dp,read);
 (*cp)->len=read;
 (*cp)->rc=0;
}
/*****************************************************************************/
/* CpyLong()                                                              813*/
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Format signed/unsigned primitive long data types.                       */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   cp         input - -> to the buffer we're formatting into.              */
/*   UserAddr   input - location in user's address space of data             */
/*                      we want to format.                                   */
/*   sfx        input - stack frame index for auto data.                     */
/*   typeno     input - primitive type no. Tells signed/unsigned.            */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   void                                                                    */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*  cp -> to a buffer that has been pre-formatted with blanks.               */
/*                                                                           */
/*****************************************************************************/
void CpyLong( COPYDATA **cp, ULONG UserAddr, UINT sfx , USHORT typeno , UINT nitems, MSHOBJECT *mshObject)
{
 UINT   read;
 UINT   size;
 signed long *dp;
#if 0
 char   fs[11];
#endif
 size = sizeof(long);
 dp = (signed long *)MshAppData( UserAddr, &nitems, size, &read, sfx, mshObject);
 *cp=(COPYDATA *)T2alloc(sizeof(COPYDATA)+read-sizeof(double),CPY_CHAIN);
 if( !dp || (size*nitems != read) )
  {sprintf((*cp)->buffer,"Invalid Address");(*cp)->rc=-1;return;}
 memcpy((*cp)->data,dp,read);
 (*cp)->len=read;
 (*cp)->rc=0;
#if 0

 if( (typeno & TYP_BITS) == SIGNED )
 {
  strcpy(fs,"%-12ld%#lx");
  sprintf(cp,fs,*dp,(ULONG)*dp);
 }
 else
 {
  strcpy(fs,"%-12lu%#lx");
  sprintf(cp,fs,(ULONG)*dp,(ULONG)*dp);
 }
#endif
}

/*****************************************************************************/
/* CpyFloat()                                                             813*/
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Format primitive floating point types.                                  */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   cp         input - -> to the buffer we're formatting into.              */
/*   UserAddr   input - location in user's address space of data             */
/*                      we want to format.                                   */
/*   sfx        input - stack frame index for auto data.                     */
/*   typeno     input - primitive type no. Tells signed/unsigned.            */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   void                                                                    */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*  cp -> to a buffer that has been pre-formatted with blanks.               */
/*                                                                           */
/*****************************************************************************/
void CpyFloat( UINT mid,COPYDATA **cp, ULONG UserAddr, UINT sfx , USHORT typeno , UINT nitems, MSHOBJECT *mshObject)
{
 UINT   read;
 UINT   size;
 signed long *dp;

 size = QtypeSize(mid,typeno);
 dp = (signed long *)MshAppData( UserAddr, &nitems, size, &read, sfx, mshObject);
 *cp=(COPYDATA *)T2alloc(sizeof(COPYDATA)+read-sizeof(double),CPY_CHAIN);
 if( !dp || (size*nitems != read) )
  {sprintf((*cp)->buffer,"Invalid Address");(*cp)->rc=-1;return;}
 memcpy((*cp)->data,dp,read);
 (*cp)->len=read;
 (*cp)->rc=0;
#if 0

 /****************************************************************************/
 /* Mask all the floating point exceptions while we execute the sprintf.     */
 /* The debugger may trap if we don't do this.                               */
 /*                                                                          */
 /****************************************************************************/
 _control87(0x37f,0xffff);
 switch(typeno)
 {
  case TYPE_FLOAT:
   sprintf(cp,"%.10g",*(float*)dp);
   break;

  case TYPE_DOUBLE:
   sprintf(cp,"%.18g",*(double*)dp);
   break;

  case TYPE_LDOUBLE:
   sprintf(cp,"%.21Lg",*(long double*)dp);
   break;
 }
 /****************************************************************************/
 /* - Clear any exceptions that may have occurred.                           */
 /* - Reset the control word to the default value.                           */
 /****************************************************************************/
 _clear87();
 _control87(CW_DEFAULT,0xffff);
#endif
}

/*****************************************************************************/
/* CpyPtr()                                                               813*/
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Format pointers to signed/unsigned primitives.                          */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   pcp        input - -> to -> to the buffer we're formatting into.        */
/*   pUserAddr  input - -> to our variable containing the location in        */
/*                         in user's address space of the pointer we         */
/*                         we want to format.                                */
/*   mid        input - module in which the typeno is defined.               */
/*   typeno     input - the type number of the T_BITFLD recoard.             */
/*   sfx        input - stack frame index for auto data.                     */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   void                                                                    */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*  cp -> to a buffer that has been pre-formatted with blanks.               */
/*                                                                           */
/*****************************************************************************/
USHORT CpyPtr(COPYDATA **pcp,ULONG *pUserAddr,UINT mid, USHORT typeno,UINT sfx, MSHOBJECT *mshObject)
{
 UINT    read;
 UINT    size;
 signed  char  **dp;
 ULONG   UserAddr = *pUserAddr;
 COPYDATA **cp = pcp;
 UINT    type, nitems=1;

 type = GetPtrType(mid,typeno);
 size = QtypeSize(mid,typeno);

 switch( type )
 {
  case PTR_0_32:
  case PTR_16_16:
   dp = ( signed char**)MshAppData( UserAddr, &nitems, size, &read, sfx, mshObject);
 *cp=(COPYDATA *)T2alloc(sizeof(COPYDATA)+read-sizeof(double),CPY_CHAIN);
   if( !dp || (size != read) )
    {sprintf((*cp)->buffer,"Invalid Address");(*cp)->rc=-1;return(0);}
 memcpy((*cp)->data,dp,read);
 (*cp)->len=read;
 (*cp)->rc=0;
   if( type == PTR_0_32 )
   {
    *pUserAddr = (ULONG)*dp;
   }
   else /* case PTR_16_16 */
   {
    *pUserAddr = _SelOff2Flat( *((USHORT*)dp+1) , *(USHORT*)dp );
   }
#if 0

   if( type == PTR_0_32 )
   {
    sprintf(cp,fs1,*dp);
    *pUserAddr = (ULONG)*dp;
   }
   else /* case PTR_16_16 */
   {
    sprintf(cp,fs2, *((USHORT*)dp+1),*(USHORT*)dp);
    *pUserAddr = _SelOff2Flat( *((USHORT*)dp+1) , *(USHORT*)dp );
   }
#endif
   break;

  case PTR_0_16:
   /**************************************************************************/
   /* - Convert a BP Relative location of the near pointer to a flat address */
   /*   if the 0:16 pointer is a stack variable.                             */
   /* - Get 2 bytes of offset from the location.                             */
   /* - Format the 0:16 pointer.                                             */
   /* - Get the selector where the pointer was defined. Since the pointer is */
   /*   near, the target of the pointer has to have the same selector.       */
   /* - Form the flat address value of the pointer.                          */
   /*                                                                        */
   /**************************************************************************/
   if( TestBit(UserAddr,STACKADDRBIT) )
    UserAddr = StackBPRelToAddr( UserAddr , sfx );
   if ( UserAddr == 0 )
    {sprintf((*cp)->buffer,"Invalid Address");(*cp)->rc=-1;return(0);}

   dp = ( signed char**)MshAppData( UserAddr, &nitems, size, &read, sfx, mshObject);
 *cp=(COPYDATA *)T2alloc(sizeof(COPYDATA)+read-sizeof(double),CPY_CHAIN);
   if( !dp || (size != read) )
    {sprintf((*cp)->buffer,"Invalid Address");(*cp)->rc=-1;return(0);}
 memcpy((*cp)->data,dp,read);
 (*cp)->len=read;
 (*cp)->rc=0;
#if 0
   sprintf(cp,fs3, *(USHORT*)dp);
#endif

   {
    USHORT Sel;
    USHORT Off;

    _Flat2SelOff(UserAddr, &Sel,&Off);
    *pUserAddr = _SelOff2Flat( Sel, *(USHORT*)dp);
   }
 }

 /****************************************************************************/
 /* - If the typeno is primitive, then zero the model bits and return        */
 /*   the type of the pointer to the caller.                                 */
 /* - If the typeno is complex, then echo the typeno back to the caller.     */
 /*   What we return is irrelevant;however, we need to return something.     */
 /****************************************************************************/
 if( typeno < 512 )
   typeno &= (~MD_BITS);

 /****************************************************************************/
 /* Append strings to type char and type uchar pointers.                     */
 /****************************************************************************/
 if( typeno == TYPE_CHAR || typeno == TYPE_UCHAR )
 {
  char *cpend;

  CpyString(cp,cpend,*pUserAddr,sfx,mshObject);
 }
 return(typeno);
}

/*****************************************************************************/
/* CpyBytes()                                                             813*/
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Format bytes in hex.                                                    */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   cp         input - -> to the buffer we're formatting into.              */
/*   UserAddr   input - location in user's address space of data             */
/*                      we want to format.                                   */
/*   sfx        input - stack frame index for auto data.                     */
/*   size       input - number of byte to format.                            */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   void                                                                    */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*  cp -> to a buffer that has been pre-formatted with blanks.               */
/*                                                                           */
/*****************************************************************************/
void CpyBytes( COPYDATA  **cp,
               ULONG       UserAddr,
               UINT        sfx,
               UINT        size,
               MSHOBJECT  *mshObject)
{
 UINT   read;
 UCHAR *dp;
 UINT   nitems;

 nitems = size;
 dp = ( UCHAR *)MshAppData( UserAddr, &nitems, 1, &read, sfx, mshObject);
 *cp=(COPYDATA *)T2alloc(sizeof(COPYDATA)+read-sizeof(double),CPY_CHAIN);
 if( !dp || (size != read) )
  {sprintf((*cp)->buffer,"Invalid Address");(*cp)->rc=-1;return;}
 memcpy((*cp)->data,dp,read);
 (*cp)->len=read;
 (*cp)->rc=0;
}

/*****************************************************************************/
/* CpyString()                                                            813*/
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Format a string.                                                        */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   cp         input - -> to the buffer we're formatting into.              */
/*   cpend      input - -> to the end of the buffer.                         */
/*   UserAddr   input - location in user's address space of data             */
/*                      we want to format.                                   */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   void                                                                    */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*  cp -> to a buffer that has been pre-formatted with blanks.               */
/*                                                                           */
/*****************************************************************************/
void CpyString( COPYDATA **cp,
                char      *cpend,
                ULONG      UserAddr,
                UINT       sfx,
                MSHOBJECT *mshObject)
{
 UINT   read = 0;
 UINT   size;
 UCHAR *dp;
 UINT   nitems;

 /****************************************************************************/
 /*                                                                          */
 /*         read                                                             */
 /*      |<------------------>|                                              */
 /*      |                    |                                              */
 /*         size                                                             */
 /*      |<------------------------------------------------->|               */
 /*      |                                                   |               */
 /*       ---------------------------------------------------                */
 /*      |                                                   |               */
 /*       ---------------------------------------------------                */
 /*       |                   |                             |                */
 /*       |                   |                             |                */
 /*      cp                   cpend( if size<read)          cpend            */
 /*                                                                          */
 /****************************************************************************/

 dp = ( UCHAR *)MshAppData( UserAddr, &nitems, size, &read, sfx, mshObject);
 *cp=(COPYDATA *)T2alloc(sizeof(COPYDATA)+read-sizeof(double),CPY_CHAIN);
 if( !dp || read==0 )
  {sprintf((*cp)->buffer,"Invalid Address");(*cp)->rc=-1;return;}
 memcpy((*cp)->data,dp,read);
 (*cp)->len=read;
 (*cp)->rc=0;
}

/*****************************************************************************/
/* CpyEnums()                                                             813*/
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Format Enums.                                                           */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   tp         input - -> to the enum type record.                          */
/*   pcp        input - ->-> to the buffer we're formatting into.            */
/*   UserAddr   input - location in user's address space of data             */
/*                      we want to format.                                   */
/*   mid        input - module in which the typeno is defined.               */
/*   typeno     input - the type number of the T_BITFLD recoard.             */
/*   sfx        input - stack frame index for auto data.                     */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   typeno     the primitive data type for the enum.                        */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*  cp -> to a buffer that has been pre-formatted with blanks.               */
/*  tp -> to a valid enum record.                                            */
/*                                                                           */
/*****************************************************************************/
USHORT CpyEnums( Trec       *tp,
                 COPYDATA  **pcp,
                 ULONG       UserAddr,
                 UINT        mid,
                 USHORT      typeno,
                 UINT        sfx,
                 MSHOBJECT  *mshObject)
{
 char   *ename;
 UINT    size;
 USHORT  type;
 long    lvalue;
 long   *dp;
 UINT    read;
 ULONG   index;
 UINT    nitems;

 /****************************************************************************/
 /* - Get the type of the enum.                                              */
 /* - Get the size of the enum.                                              */
 /* - Get the value of the enum.                                             */
 /****************************************************************************/
 type = ((TD_ENUM*)tp)->DataType;
 size = QtypeSize(mid,type);

 dp = (signed long *)MshAppData( UserAddr, &nitems, size, &read, sfx, mshObject);
 *pcp=(COPYDATA *)T2alloc(sizeof(COPYDATA)+read-sizeof(double),CPY_CHAIN);
 if( !dp || (size != read) )
  {sprintf((*pcp)->buffer,"Invalid Address");return(0);}

 memcpy((*pcp)->data,dp,read);
 (*pcp)->len=read;
 (*pcp)->rc=0;

 if( size == 1 )
     lvalue = (long)(*(UCHAR *)dp);
 else if( size == 2)
     lvalue = (long)(*(USHORT *)dp);
 else if( size == 4)
     lvalue = *dp;

 /***************************************************************************/
 /* - Get the name list index.                                              */
 /* - Find the name associated with the enum value(lvalue) and format it.   */
 /***************************************************************************/
 typeno = ((TD_ENUM*)tp)->NameListIndex;

 tp = QbasetypeRec(mid, typeno);
 if( tp && QNameList(tp,VERIFYVALUE,lvalue) )
 {
  index =  QNameList(tp,VALUEINDEX,lvalue);
  ename =  (char*)QNameList(tp,NAME,index);
  memcpy(*pcp, ename+sizeof(USHORT/*2 byte name length*/), *ename );

  /***************************************************************************/
  /* bump the buffer pointer for the caller.                                 */
  /***************************************************************************/
  *pcp = *pcp + *ename;
 }
 return(type);
}

/*****************************************************************************/
/* CpyBitField()                                                          813*/
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Format a Bitield.                                                       */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   ptrec      input - -> to the enum type record.                          */
/*   cp         input - -> to the buffer we're formatting into.              */
/*   UserAddr   input - location in user's address space of data             */
/*                      we want to format.                                   */
/*   mid        input - module in which the typeno is defined.               */
/*   typeno     input - the type number of the T_BITFLD recoard.             */
/*   sfx        input - stack frame index for auto data.                     */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   void                                                                    */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*  cp -> to a buffer that has been pre-formatted with blanks.               */
/*  tp -> to a valid enum record.                                            */
/*                                                                           */
/*****************************************************************************/
#define MAXPLXBITSTRINGLENGTH 64
#define TYPE_ULONG_8          0
void CpyBitField( Trec       *ptrec,
                  COPYDATA  **cp,
                  ULONG       UserAddr,
                  UINT        mid,
                  USHORT      typeno,
                  UINT        sfx,
                  MSHOBJECT  *mshObject)
{
 TD_BITFLD *tp;                        /* -> to base type record.            */
 UCHAR      type;                      /* type of bitfld storage.            */
 UCHAR      bitfldsize;                /* size of bitfld storage.            */
 UCHAR      bitfldoffs;                /* offset of bitfld within stg.       */
 UCHAR      size;                      /* size of bitfld storage.            */
 UCHAR      offset;                    /* offset of bitfld within stg.       */
 UCHAR      width;                     /* width  of bitfld within stg.       */
 UINT       value;                     /* unsigned value for bitfld.         */
 UCHAR      byteval;                   /* byte value of bit field.           */
 UCHAR      bytemask;                  /* byte mask for bitfield.            */
 int        i;
 int        j;
 UCHAR      fmtmask;                   /* storage mask for bitfld.           */
 UCHAR     *dp;
 UINT       read;
 int        bitsconsumed;
 UINT       nitems;

 tp = (TD_BITFLD*)ptrec;

 bitfldsize = tp->BitSize;
 bitfldoffs = tp->Offset;
 type       = tp->BaseType;
 if( type == TYPE_ULONG && (tp->Flags & DISPLAY_AS_VALUE ) )
  type = TYPE_ULONG_8;

 size = bitfldsize;
 offset = bitfldoffs;
 switch( type )
 {
  case TYPE_UCHAR:
  /*************************************************************************/
  /* PL/X bit fields.                                                      */
  /*************************************************************************/
  {
   width = 8;

   bitsconsumed = 0;
   j = 0;
   for(;;)
   {
    if(size+offset>width)
    {
     size = width - offset;
    }

    for( i=0; i< (int)width ; i++ )
     if( (i+j)<MAXPLXBITSTRINGLENGTH )
      (*cp)->buffer[j+i] = '.';
    bytemask=(UCHAR)( (bytemask=0x7F)>>(size-1) );
    bytemask=(UCHAR)((UCHAR)(~bytemask)>>offset );
    dp = MshAppData(UserAddr,&nitems, 1,&read,sfx, mshObject);
    if( !dp || (read != 1 ) )
     {sprintf((*cp)->buffer,"Invalid Address");(*cp)->rc=-1;return;}

    byteval = bytemask & (*dp);
    fmtmask = 0x80;
    for( i=0;
         i< (int)width;
         fmtmask >>=1, i++)
    {
     if( bytemask & fmtmask )
     {
      if( (i+j)<MAXPLXBITSTRINGLENGTH )
       (*cp)->buffer[j+i] = (UCHAR)((fmtmask & byteval)?'1':'0');
     }
    }
    j += i;
    bitsconsumed += (int)size;
    if(bitsconsumed >= (int)bitfldsize)
     break;
    size = bitfldsize - (UCHAR)bitsconsumed;   /* ?????????????? */
    offset = 0;
    UserAddr++;
   }
  }
  break;

  case TYPE_USHORT:
  /**********************************************************************/
  /* C211/C600 bitfields.                                               */
  /**********************************************************************/
  {
   USHORT mask;
   USHORT usfmtmask;

   width = 16;
   for( i=0; i< (int)width ; i++ )
    (*cp)->buffer[i] = '.';
   mask = (~(mask = 1))<<(size-1);
   mask = (~mask)<<offset;

   dp = MshAppData(UserAddr,&nitems, 2,&read,sfx, mshObject);
   if( !dp || (read != 2 ) )
    {sprintf((*cp)->buffer,"Invalid Address");(*cp)->rc=-1;return;}

   value = mask & (*(USHORT*)dp);
   usfmtmask = 0x8000;
   for( i=0; i< (int)width ; usfmtmask >>=1, i++ )
   {
    if( mask & usfmtmask )
     (*cp)->buffer[i] = (UCHAR)((usfmtmask & value)?'1':'0');
   }
  }
  break;

  case TYPE_ULONG:
  /**********************************************************************/
  /* CL386 bitfields.                                                   */
  /**********************************************************************/
  {
   ULONG mask;
   ULONG ulfmtmask;

   width = 32;
   for( i=0; i< (int)width ; i++ )
    (*cp)->buffer[i] = '.';
   mask = (~(mask = 1))<<(size-1);
   mask = (~mask)<<offset;

   dp = MshAppData(UserAddr,&nitems, 4,&read,sfx, mshObject);
   if( !dp || (read != 4 ) )
    {sprintf((*cp)->buffer,"Invalid Address");(*cp)->rc=-1;return;}

   value = mask & (*(ULONG*)dp);
   ulfmtmask = 0x80000000;
   for( i=0; i< (int)width ; ulfmtmask >>=1, i++ )
   {
      if( mask & ulfmtmask )
       (*cp)->buffer[i] = (UCHAR)((ulfmtmask & value)?'1':'0');
   }
  }
  break;

  case TYPE_ULONG_8:
  /**********************************************************************/
  /* IBM C SET-2 Bitfields.                                             */
  /**********************************************************************/
  {
   if ( (size > 8) || ((offset+size) > 8) )
   {
    /********************************************************************/
    /* If the bit field spans across bytes then we will not be able     */
    /* show the exact memory lay out of bits since they are packed      */
    /* and filled from lower order bits to higher order bits.           */
    /* In this case we convert the bit string into Hex value and show   */
    /* it as a bit string.                                              */
    /********************************************************************/
    UINT uifmtmask;
    if (!BytesToValue(UserAddr,offset,size,sfx,&value))
       return;
    uifmtmask = 0x1;
    for( i=0; i< (int)size; uifmtmask <<=1, i++)
    {
     if( value & uifmtmask )
       (*cp)->buffer[size-i-1] = (UCHAR)'1';
     else
       (*cp)->buffer[size-i-1] = (UCHAR)'0';
    }
   }
   else
   {
    /********************************************************************/
    /* If the bit field doesn't span across a byte then show the        */
    /* exact memory lay out of bits.                                    */
    /********************************************************************/
    width = 8;
    for( i=0; i< (int)width ; i++ )
      (*cp)->buffer[i] = '.';
    bytemask=(UCHAR)( (~(bytemask = 1))<<(size-1) );
    bytemask=(UCHAR)((UCHAR)(~bytemask)<<offset );
    dp = MshAppData(UserAddr,&nitems,1,&read,sfx, mshObject);
    if( !dp || (read != 1 ) )
     {sprintf((*cp)->buffer,"Invalid Address");(*cp)->rc=-1;return;}

    byteval = bytemask & (*dp);
    fmtmask = 0x80;
    for( i=0; i< (int)width; fmtmask >>=1, i++)
    {
     if( bytemask & fmtmask )
       (*cp)->buffer[i] = (UCHAR)((fmtmask & byteval)?'1':'0');
    }
   }
  }
  break;
 }                                     /* end of switch{}.                  */
 return;
}
