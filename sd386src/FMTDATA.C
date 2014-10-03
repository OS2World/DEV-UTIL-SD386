/*****************************************************************************/
/* File:                                                                     */
/*   fmtdata.c                                                            813*/
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*  Router/formatter for data items.                                         */
/*                                                                           */
/* History:                                                                  */
/*                                                                           */
/*****************************************************************************/

#include "all.h"

/*****************************************************************************/
/* FmtDataItem()                                                             */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   This routine routes the typeno to the correct formatter.                */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   cp         -> to the buffer we're formatting into.                      */
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
void FormatDataItem( UCHAR *cp, UINT data, UINT mid, USHORT typeno, UINT sfx )
{
 Trec  *tp;

startatthetop:

 if( typeno < 512 )
 {
recirculate:
  switch( typeno )
  {
   case TYPE_VOID:
    FmtBytes( cp, data , sfx , 16);
    break;

   case TYPE_CHAR:
   case TYPE_UCHAR:
    FmtChar( cp, data , sfx , typeno);
    break;

   case TYPE_SHORT:
   case TYPE_USHORT:
    FmtShort( cp, data , sfx , typeno);
    break;

   case TYPE_LONG:
   case TYPE_ULONG:
    FmtLong( cp, data , sfx , typeno);
    break;


   case TYPE_FLOAT:
   case TYPE_DOUBLE:
   case TYPE_LDOUBLE:
    FmtFloat( mid,cp, data , sfx , typeno);
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
    typeno = FmtPtr(&cp,(ULONG*)&data,0/*mid=0*/,typeno,sfx);
    if( typeno )
     goto recirculate;
    break;

  }
  /***************************************************************************/
  /* Remove the \0 inserted by the formatter.                                */
  /***************************************************************************/
  *(strchr(cp,'\0')) = ' ';
  return;
 }
 /****************************************************************************/
 /* - Now, handle typenos > 512.                                             */
 /* - QbasetypeRec may return with tp pointing to a TD_USERDEF record with   */
 /*   a primitive type index. In this case, we grab the primitive typeno     */
 /*   and recirculate.                                                       */
 /****************************************************************************/
 else
 {
  tp = QbasetypeRec(mid,typeno);
  if( tp && (tp->RecType == T_TYPDEF) )
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

  case T_MEMFNC:
  {
// FmtMemFncAttrs( tp, cp );
  }
  break;

  case T_BSECLS:
  {
   USHORT    typeno;
   TD_CLASS *pClass;

   typeno  = ((TD_BSECLS*)tp)->TypeIndex;
   pClass  = (TD_CLASS*)QtypeRec(mid, typeno);

   strncpy( cp, pClass->Name, pClass->NameLen );
  }
  break;

  case T_FRIEND:
   break;

  case T_CLSDEF:
  {
   typeno  = ((TD_CLSDEF*)tp)->TypeIndex;
   goto startatthetop;
  }
//break;

  case T_CLSMEM:
  {
   typeno = ((TD_CLSMEM*)tp)->TypeIndex;
   goto startatthetop;
  }
//break;

  case T_SCALAR:
  case T_PROC:
  case T_ENTRY:
  case T_FUNCTION:
  case T_ARRAY:
  case T_STRUCT:
   FmtBytes( cp, data , sfx , 16);
   *(strchr(cp,'\0')) = ' ';
   break;

  case T_CLASS:
   strcpy(cp, "...object");
   *(strchr(cp,'\0')) = ' ';
   break;

  case T_ENUM:
   typeno = FmtEnums( tp,&cp, data, mid, typeno, sfx);
   /**************************************************************************/
   /* typeno will be returned as 0 in the event of an error.                 */
   /**************************************************************************/
   if( typeno == 0 )
    break;
   *cp++ = ' ';
   *cp++ = '=';
   *cp++ = ' ';
   goto recirculate;

  case T_BITFLD:
   FmtBitField( tp, cp, data, sfx);
   break;

  case T_REF:
   FmtRef( cp, data , sfx );
   break;

casepointer:
  case T_PTR:
  {
   /**************************************************************************/
   /* See the documentation for Pointer handling. It will make the           */
   /* understanding of this much simpler.                                    */
   /**************************************************************************/
   (void)FmtPtr( &cp, (ULONG*)&data , mid , typeno , sfx);
   typeno = ((TD_POINTER*)tp)->TypeIndex;

   if( typeno < 512 )
    goto recirculate;

   tp = QbasetypeRec(mid,typeno);
   switch( tp->RecType )
   {
    case T_TYPDEF:
      typeno = ((TD_USERDEF*)tp)->TypeIndex;
      goto recirculate;

    case T_PTR:
     (void)FmtPtr( &cp, (ULONG*)&data , mid , typeno , sfx);
     typeno = ((TD_POINTER*)tp)->TypeIndex;
     if( (typeno < 512 ) )
     {
      if( !( typeno & MD_BITS) )
       goto recirculate;
      else
      {
       FmtBytes( cp, data , sfx , 16);
       break;
      }
     }
     tp = QbasetypeRec(mid,typeno);
     switch( tp->RecType )
     {
      case T_TYPDEF:
       typeno = ((TD_USERDEF*)tp)->TypeIndex;
       if( (typeno < 512 ) && !( typeno & MD_BITS) )
        goto recirculate;
       break;

      default:
      FmtBytes( cp, data , sfx , 16);
      break;
     }
     break;

    default:
     FmtBytes( cp, data , sfx , 16);
     break;
   }
   *(strchr(cp,'\0')) = ' ';
  }
  break;   /* end case T_PTR */
 }
}
/*****************************************************************************/
/* FmtChar()                                                              813*/
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
/*                                                                           */
/*****************************************************************************/
#define SIGNED   0x00
#define UNSIGNED 0x04
void FmtChar( char *cp, ULONG UserAddr, UINT sfx , USHORT typeno )
{
 UINT   read;
 UINT   size;
 signed char *dp;
 char   fs[10] = "%-12d";
 char   f2[4] = "%#x";
 char   f3[5] = "\'%c\'";

 size = sizeof(char);
 dp = (signed char *)GetAppData( UserAddr, size, &read, sfx);
 if( !dp || (size != read) )
  {sprintf(cp,"Invalid Address");return;}

 (*dp < 32 )? strcat(fs,f2):strcat(fs,f3);
 if( typeno == TYPE_CHAR )
  sprintf(cp,fs,*dp,(UCHAR)*dp);
 else
  sprintf(cp,fs,(UCHAR)*dp,(UCHAR)*dp);

}
/*****************************************************************************/
/* FmtShort()                                                             813*/
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
void FmtShort( char *cp, ULONG UserAddr, UINT sfx , USHORT typeno )
{
 UINT   read;
 UINT   size;
 signed short *dp;
 char   fs[10] = "%-12d%#hx";

 size = sizeof(short);
 dp = (signed short *)GetAppData( UserAddr, size, &read, sfx);
 if( !dp || (size != read) )
  {sprintf(cp,"Invalid Address");return;}

 if( (typeno & TYP_BITS) == SIGNED )
  sprintf(cp,fs,*dp,(USHORT)*dp);
 else
  sprintf(cp,fs,(USHORT)*dp,(USHORT)*dp);
}
/*****************************************************************************/
/* FmtLong()                                                              813*/
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
void FmtLong( char *cp, ULONG UserAddr, UINT sfx , USHORT typeno )
{
 UINT   read;
 UINT   size;
 signed long *dp;
 char   fs[11];

 size = sizeof(long);
 dp = (signed long *)GetAppData( UserAddr, size, &read, sfx);
 if( !dp || (size != read) )
  {sprintf(cp,"Invalid Address");return;}

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
}

/*****************************************************************************/
/* FmtFloat()                                                             813*/
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
void FmtFloat( UINT mid,char *cp, ULONG UserAddr, UINT sfx , USHORT typeno )
{
 UINT   read;
 UINT   size;
 signed long *dp;

 size = QtypeSize(mid,typeno);
 dp = (signed long *)GetAppData( UserAddr, size, &read, sfx);
 if( !dp || (size != read) )
  {sprintf(cp,"Invalid Address");return;}

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
}

/*****************************************************************************/
/* FmtPtr()                                                               813*/
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
static char fs1[]={'%','0','8','X',' ',0xCD,0x10,' ','\0'};
static char fs2[]={'%','0','4','X',':','%','0','4','X',' ',0xCD,0x10,' ','\0'};
static char fs3[]={'%','0','4','X',' ',0xCD,0x10,' ','\0'};
USHORT FmtPtr(char **pcp,ULONG *pUserAddr,UINT mid, USHORT typeno,UINT sfx)
{
 UINT    read;
 UINT    size;
 signed  char  **dp;
 ULONG   UserAddr = *pUserAddr;
 char   *cp = *pcp;
 UINT    type;

 type = GetPtrType(mid,typeno);
 size = QtypeSize(mid,typeno);

 switch( type )
 {
  case PTR_0_32:
  case PTR_16_16:
   dp = ( signed char**)GetAppData( UserAddr, size, &read, sfx);
   if( !dp || (size != read) )
    {sprintf(cp,"Invalid Address");return(0);}

   if( type == PTR_0_32 )
   {
    sprintf(cp,fs1,*dp);
    *pUserAddr = (ULONG)*dp;
   }
   else /* case PTR_16_16 */
   {
    sprintf(cp,fs2, *((USHORT*)dp+1),*(USHORT*)dp);
    *pUserAddr = Data_SelOff2Flat( *((USHORT*)dp+1) , *(USHORT*)dp );
   }
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
    {sprintf(cp,"Invalid Address");return(0);}

   dp = ( signed char**)GetAppData( UserAddr, size, &read, sfx);
   if( !dp || (size != read) )
    {sprintf(cp,"Invalid Address");return(0);}
   sprintf(cp,fs3, *(USHORT*)dp);

   {
    USHORT Sel;
    USHORT Off;

    Data_Flat2SelOff(UserAddr, &Sel,&Off);
    *pUserAddr = Data_SelOff2Flat( Sel, *(USHORT*)dp);
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
  extern UINT VideoCols;

  cpend = (*pcp-STGCOL-1) + VideoCols - 1;
  FmtString(cp+strlen(cp),cpend,*pUserAddr,sfx);
  typeno = 0;
 }
 else
  /***************************************************************************/
  /* Kill the \0 at the end of the string by converting to a space.          */
  /***************************************************************************/
  *(*pcp=strchr(cp,'\0')) = ' ';

 return(typeno);
}

/*****************************************************************************/
/* FmtBytes()                                                             813*/
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
void FmtBytes( char *cp, ULONG UserAddr, UINT sfx , UINT size )
{
 UINT   read;
 int    i;
 UCHAR *dp;

 dp = ( UCHAR *)GetAppData( UserAddr, size, &read, sfx);
 if( !dp || (size != read) )
 {
  sprintf(cp,"Invalid Address");
 }
 else
 {
  *cp = '\0';
  for( i=0; i < read; i++,dp++)
  {
   char buffer[4];

   sprintf(buffer, "%02X ", *dp);
   strcat( cp, buffer );
  }
 }
}

/*****************************************************************************/
/* FmtString()                                                            813*/
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
void FmtString( char *cp, char *cpend ,ULONG UserAddr, UINT sfx )
{
 UINT   read = 0;
 UINT   size;
 UCHAR *dp;
 int    c;

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

 size = cpend-cp;
 dp = ( UCHAR *)GetAppData( UserAddr, size, &read, sfx);
 if( !dp || read==0 )
  {sprintf(cp,"Invalid Address");return;}

 if( read < size )
  cpend = cp + read;

 *cp = '\"';
 for( cp++; cp<=cpend && *dp != '\0'; cp++,dp++)
 {
  c = *dp;
  /***************************************************************************/
  /* show graphic characters as little dots.                                 */
  /***************************************************************************/
  if( c < 0x20 || c > 0x7F )
   *cp = 0xFA;
  else
   *cp = c;
 }
 if( cp <= cpend )
  *cp = '\"';

 /****************************************************************************/
 /* null terminate the string just formatted.                                */
 /****************************************************************************/
 cp++; if( cp <= cpend ) *cp = '\0';
}

/*****************************************************************************/
/* FmtEnums()                                                             813*/
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
USHORT FmtEnums( Trec *tp,char **pcp,ULONG UserAddr,UINT mid,
                 USHORT typeno,UINT sfx)
{
 char   *ename;
 UINT    size;
 USHORT  type;
 long    lvalue;
 long   *dp;
 UINT    read;
 ULONG   index;

 /****************************************************************************/
 /* - Get the type of the enum.                                              */
 /* - Get the size of the enum.                                              */
 /* - Get the value of the enum.                                             */
 /****************************************************************************/
 type = ((TD_ENUM*)tp)->DataType;
 size = QtypeSize(mid,type);

 dp = (signed long *)GetAppData( UserAddr, size, &read, sfx);
 if( !dp || (size != read) )
 {
  sprintf(*pcp,"Invalid Address");
  *(strchr(*pcp,'\0')) = ' ';
  return(0);
 }

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
/* FmtBitField()                                                          813*/
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
void FmtBitField( Trec *ptrec, char *cp, ULONG UserAddr, UINT sfx)
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
      cp[j+i] = '.';
    bytemask=(UCHAR)( (bytemask=0x7F)>>(size-1) );
    bytemask=(UCHAR)((UCHAR)(~bytemask)>>offset );
    dp = GetAppData(UserAddr,1,&read,sfx);
    if( !dp || (read != 1 ) )
     {sprintf(cp,"Invalid Address");return;}

    byteval = bytemask & (*dp);
    fmtmask = 0x80;
    for( i=0;
         i< (int)width;
         fmtmask >>=1, i++)
    {
     if( bytemask & fmtmask )
     {
      if( (i+j)<MAXPLXBITSTRINGLENGTH )
       cp[j+i] = (UCHAR)((fmtmask & byteval)?'1':'0');
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
    cp[i] = '.';
   mask = (~(mask = 1))<<(size-1);
   mask = (~mask)<<offset;

   dp = GetAppData(UserAddr,2,&read,sfx);
   if( !dp || (read != 2 ) )
    {sprintf(cp,"Invalid Address");return;}

   value = mask & (*(USHORT*)dp);
   usfmtmask = 0x8000;
   for( i=0; i< (int)width ; usfmtmask >>=1, i++ )
   {
    if( mask & usfmtmask )
     cp[i] = (UCHAR)((usfmtmask & value)?'1':'0');
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
    cp[i] = '.';
   mask = (~(mask = 1))<<(size-1);
   mask = (~mask)<<offset;

   dp = GetAppData(UserAddr,4,&read,sfx);
   if( !dp || (read != 4 ) )
    {sprintf(cp,"Invalid Address");return;}

   value = mask & (*(ULONG*)dp);
   ulfmtmask = 0x80000000;
   for( i=0; i< (int)width ; ulfmtmask >>=1, i++ )
   {
      if( mask & ulfmtmask )
       cp[i] = (UCHAR)((ulfmtmask & value)?'1':'0');
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
       cp[size-i-1] = (UCHAR)'1';
     else
       cp[size-i-1] = (UCHAR)'0';
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
      cp[i] = '.';
    bytemask=(UCHAR)( (~(bytemask = 1))<<(size-1) );
    bytemask=(UCHAR)((UCHAR)(~bytemask)<<offset );
    dp = GetAppData(UserAddr,1,&read,sfx);
    if( !dp || (read != 1 ) )
     {sprintf(cp,"Invalid Address");return;}

    byteval = bytemask & (*dp);
    fmtmask = 0x80;
    for( i=0; i< (int)width; fmtmask >>=1, i++)
    {
     if( bytemask & fmtmask )
       cp[i] = (UCHAR)((fmtmask & byteval)?'1':'0');
    }
   }
  }
  break;
 }                                     /* end of switch{}.                  */
 return;
}

/*****************************************************************************/
/* - format member function attributes.                                      */
/*****************************************************************************/
#define MEMFNC_PRIVATE      0x00
#define MEMFNC_PROTECTED    0x01
#define MEMFNC_PUBLIC       0x02

#define MEMFNC_CONSTRUCTOR  0x1
#define MEMFNC_DESTRUCTOR   0x2

#define SMEMFNC_PRIVATE     "Pri"
#define SMEMFNC_PROTECTED   "Pro"
#define SMEMFNC_PUBLIC      "Pub"

#define SMEMFNC_CONSTRUCTOR "Ct"
#define SMEMFNC_DESTRUCTOR  "Dt"

#define MEMFNC_STATIC      0x01
#define MEMFNC_INLINE      0x02
#define MEMFNC_CONST       0x04
#define MEMFNC_VOL         0x08
#define MEMFNC_VIRTUAL     0x10

#define SMEMFNC_STATIC     "St"
#define SMEMFNC_INLINE     "In"
#define SMEMFNC_CONST      "Co"
#define SMEMFNC_VOL        "Vo"
#define SMEMFNC_VIRTUAL    "Vir"

void FmtMemFncAttrs( Trec *pTrec, UCHAR *cp )
{
 TD_MEMFNC *pMemFnc;

 pMemFnc = (TD_MEMFNC*)pTrec;

 switch(pMemFnc->Protection)
 {
  case  MEMFNC_PRIVATE:
  {
   strcpy(cp, SMEMFNC_PRIVATE);
  }
  break;

  case  MEMFNC_PROTECTED:
  {
   strcpy(cp, SMEMFNC_PROTECTED);
  }
  break;

  case  MEMFNC_PUBLIC:
  {
   strcpy(cp, SMEMFNC_PUBLIC);
  }
  break;
 }
 cp += strlen(cp);

 switch(pMemFnc->FuncType)
 {
  case  MEMFNC_CONSTRUCTOR:
  {
   strcpy(cp, SMEMFNC_CONSTRUCTOR);
  }
  break;

  case  MEMFNC_DESTRUCTOR:
  {
   strcpy(cp, SMEMFNC_DESTRUCTOR);
  }
  break;
 }

 if(pMemFnc->TypeQual & MEMFNC_STATIC   )
 {
  cp += strlen(cp);
  strcpy(cp, SMEMFNC_STATIC);
 }

 if(pMemFnc->TypeQual & MEMFNC_INLINE   )
 {
  cp += strlen(cp);
  strcpy(cp, SMEMFNC_INLINE);
 }

 if(pMemFnc->TypeQual & MEMFNC_CONST    )
 {
  cp += strlen(cp);
  strcpy(cp, SMEMFNC_CONST);
 }

 if(pMemFnc->TypeQual & MEMFNC_VOL      )
 {
  cp += strlen(cp);
  strcpy(cp, SMEMFNC_VOL);
 }

 if(pMemFnc->TypeQual & MEMFNC_VIRTUAL  )
 {
  cp += strlen(cp);
  strcpy(cp, SMEMFNC_VIRTUAL);
 }
}

/*****************************************************************************/
/* - format class member attributes.                                         */
/*****************************************************************************/
void FmtClsMemAttrs( Trec *pTrec, UCHAR *cp )
{
 TD_CLSMEM *pClsMem;

 pClsMem = (TD_CLSMEM*)pTrec;

 switch(pClsMem->Protection)
 {
  case  CLSMEM_PRIVATE:
  {
   strcpy(cp, SCLSMEM_PRIVATE);
  }
  break;

  case  CLSMEM_PROTECTED:
  {
   strcpy(cp, SCLSMEM_PROTECTED);
  }
  break;

  case  CLSMEM_PUBLIC:
  {
   strcpy(cp, SCLSMEM_PUBLIC);
  }
  break;
 }
 cp += strlen(cp);

 if(pClsMem->TypeQual & CLSMEM_STATIC   )
 {
  cp += strlen(cp);
  strcpy(cp, SCLSMEM_STATIC);
 }

 if(pClsMem->TypeQual & CLSMEM_VTABLE   )
 {
  cp += strlen(cp);
  strcpy(cp, SCLSMEM_VTABLE);
 }

 if(pClsMem->TypeQual & CLSMEM_VBASE    )
 {
  cp += strlen(cp);
  strcpy(cp, SCLSMEM_VBASE);
 }

 if(pClsMem->TypeQual & CLSMEM_CONST    )
 {
  cp += strlen(cp);
  strcpy(cp, SCLSMEM_CONST);
 }

 if(pClsMem->TypeQual & CLSMEM_VOL  )
 {
  cp += strlen(cp);
  strcpy(cp, SCLSMEM_VOL);
 }

 if(pClsMem->TypeQual & CLSMEM_SELF )
 {
  cp += strlen(cp);
  strcpy(cp, SCLSMEM_SELF);
 }
}

/*****************************************************************************/
/* - format classdef attributes.                                             */
/*****************************************************************************/
#define CLSDEF_PRIVATE      0x00
#define CLSDEF_PROTECTED    0x01
#define CLSDEF_PUBLIC       0x02

#define SCLSDEF_PRIVATE     "Pri"
#define SCLSDEF_PROTECTED   "Pro"
#define SCLSDEF_PUBLIC      "Pub"

void FmtClsDefAttrs( Trec *pTrec, UCHAR *cp )
{
 TD_CLSDEF *pClsDef;

 pClsDef = (TD_CLSDEF*)pTrec;

 switch(pClsDef->Protection)
 {
  case  CLSDEF_PRIVATE:
  {
   strcpy(cp, SCLSDEF_PRIVATE);
  }
  break;

  case  CLSDEF_PROTECTED:
  {
   strcpy(cp, SCLSDEF_PROTECTED);
  }
  break;

  case  CLSDEF_PUBLIC:
  {
   strcpy(cp, SCLSDEF_PUBLIC);
  }
  break;
 }
}

/*****************************************************************************/
/* - format base class attributes.                                           */
/*****************************************************************************/
#define BSECLS_PRIVATE      0x00
#define BSECLS_PROTECTED    0x01
#define BSECLS_PUBLIC       0x02

#define SBSECLS_PRIVATE     "Private "
#define SBSECLS_PROTECTED   "Protected "
#define SBSECLS_PUBLIC      "Public "

#define BSECLS_VIRTUAL      0x01

#define SBSECLS_VIRTUAL     "Virtual "

void FmtBseClsAttrs( Trec *pTrec, UCHAR *cp )
{
 TD_BSECLS *pBseCls;

 pBseCls = (TD_BSECLS*)pTrec;

 *cp++= ' ';
 *cp++= ':';
 *cp++= ' ';

 switch(pBseCls->Protection)
 {
  case  BSECLS_PRIVATE:
  {
   strcpy(cp, SBSECLS_PRIVATE);
  }
  break;

  case  BSECLS_PROTECTED:
  {
   strcpy(cp, SBSECLS_PROTECTED);
  }
  break;

  case  BSECLS_PUBLIC:
  {
   strcpy(cp, SBSECLS_PUBLIC);
  }
  break;
 }
 cp += strlen(cp);

 if(pBseCls->TypeQual & BSECLS_VIRTUAL  )
 {
  cp += strlen(cp);
  strcpy(cp, SBSECLS_VIRTUAL);
 }
}

/*****************************************************************************/
/* - format friend function attributes.                                      */
/*****************************************************************************/
void FmtFriendAttrs( Trec *pTrec, UCHAR *cp )
{
 TD_FRIEND *pFriend;

 pFriend = (TD_FRIEND*)pTrec;

 if(pFriend->TypeQual & FRIEND_CLASS )
 {
  strcpy(cp, SFRIEND_CLASS);
 }
 else
 {
  strcpy(cp, SFRIEND_FUNCTION);
 }
}

/*****************************************************************************/
/* FmtRef()                                                               813*/
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Format signed/unsigned primitive long data types.                       */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   cp         -> to the buffer we're formatting into.                      */
/*   UserAddr   location in user's address space of data                     */
/*              we want to format.                                           */
/*   sfx        stack frame index for auto data.                             */
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
void FmtRef( char *cp, ULONG UserAddr, UINT sfx )
{
 UINT   read;
 UINT   size;
 signed long *dp;

 size = sizeof(long);
 dp = (signed long *)GetAppData( UserAddr, size, &read, sfx);
 if( !dp || (size != read) )
 {
  sprintf(cp,"Invalid Address");
 }
 else
 {
  sprintf(cp, "%#lx...Reference", (ULONG)*dp );
 }
 *(strchr(cp,'\0')) = ' ';
}
