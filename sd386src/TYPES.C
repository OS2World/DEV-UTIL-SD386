/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   types.c                                                                 */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   $$types segment handling routines.                                      */
/*                                                                           */
/*                                                                           */
/* History:                                                                  */
/*                                                                           */
/*****************************************************************************/

#include "all.h"                        /* SD86 include files                */

extern CmdParms     cmd;                /* start command parms.           403*/

/*************************************************************************813*/
/* QTypeName()                                                            813*/
/*                                                                        813*/
/* Description:                                                           813*/
/*                                                                        813*/
/*   Get the tag name, the userdef name, or the primitive name of         813*/
/*   a typeno.                                                            813*/
/*                                                                        813*/
/* Parameters:                                                            813*/
/*                                                                        813*/
/*   typeno     input - primitive or complex type number.                 813*/
/*   mid        input - module in which complex type number is defined.   813*/
/*                                                                        813*/
/* Return:                                                                813*/
/*                                                                        813*/
/*   cp         -> to the type name.                                      813*/
/*              NULL if failure.                                          813*/
/*                                                                        813*/
/* Assumptions:                                                           813*/
/*                                                                        813*/
/*****************************************************************************/
static uchar basetypes[] =                                              /*813*/
{                                                                       /*813*/
 TYPE_CHAR,                             /* 8-bit  signed.                 813*/
 TYPE_SHORT,                            /* 16-bit signed.                 813*/
 TYPE_LONG,                             /* 32-bit signed.                 813*/
 TYPE_UCHAR,                            /* 8-bit  unsigned.               813*/
 TYPE_USHORT,                           /* 16-bit unsigned.               813*/
 TYPE_ULONG,                            /* 32-bit unsigned.               813*/
 TYPE_FLOAT,                            /* 32-bit real.                   813*/
 TYPE_DOUBLE,                           /* 64-bit real.                   813*/
 TYPE_LDOUBLE,                          /* 80-bit real.                   813*/
 TYPE_VOID,                             /* void.                          813*/
 TYPE_PCHAR,                            /* 0:32 near ptr to 8-bit  signed.813*/
 TYPE_PSHORT,                           /* 0:32 near ptr to 16-bit signed.813*/
 TYPE_PLONG,                            /* 0:32 near ptr to 32-bit signed.813*/
 TYPE_PUCHAR,                           /* 0:32 near ptr to 8-bit  unsigne813*/
 TYPE_PUSHORT,                          /* 0:32 near ptr to 16-bit unsigne813*/
 TYPE_PULONG,                           /* 0:32 near ptr to 32-bit unsigne813*/
 TYPE_PFLOAT,                           /* 0:32 near ptr to 32-bit real.  813*/
 TYPE_PDOUBLE,                          /* 0:32 near ptr to 64-bit real.  813*/
 TYPE_PLDOUBLE,                         /* 0:32 near ptr to 80-bit real.  813*/
 TYPE_PVOID,                            /* 0:32 near ptr to void.         813*/
 TYPE_FPCHAR,                           /* far ptr to 8-bit  signed.      813*/
 TYPE_FPSHORT,                          /* far ptr to 16-bit signed.      813*/
 TYPE_FPLONG,                           /* far ptr to 32-bit signed.      813*/
 TYPE_FPUCHAR,                          /* far ptr to 8-bit  unsigned.    813*/
 TYPE_FPUSHORT,                         /* far ptr to 16-bit unsigned.    813*/
 TYPE_FPULONG,                          /* far ptr to 32-bit unsigned.    813*/
 TYPE_FPFLOAT,                          /* far ptr to 32-bit real.        813*/
 TYPE_FPDOUBLE,                         /* far ptr to 64-bit real.        813*/
 TYPE_FPLDOUBLE,                        /* far ptr to 80-bit real.        813*/
 TYPE_FPVOID,                           /* far ptr to void.               813*/
 TYPE_N16PCHAR,                         /* 0:16 near ptr to 8-bit  signed.813*/
 TYPE_N16PSHORT,                        /* 0:16 near ptr to 16-bit signed.813*/
 TYPE_N16PLONG,                         /* 0:16 near ptr to 32-bit signed.813*/
 TYPE_N16PUCHAR,                        /* 0:16 near ptr to 8-bit  unsigne813*/
 TYPE_N16PUSHORT,                       /* 0:16 near ptr to 16-bit unsigne813*/
 TYPE_N16PULONG,                        /* 0:16 near ptr to 32-bit unsigne813*/
 TYPE_N16PFLOAT,                        /* 0:16 near ptr to 32-bit real.  813*/
 TYPE_N16PDOUBLE,                       /* 0:16 near ptr to 64-bit real.  813*/
 TYPE_N16PLDOUBLE,                      /* 0:16 near ptr to 80-bit real.  813*/
 TYPE_N16PVOID                          /* 0:16 near ptr to void.         813*/
};                                                                      /*813*/

static char *basenames[] =                                              /*813*/
{                                                                       /*813*/
 "\4\0char"     ,                       /* 8-bit  signed.                 813*/
 "\5\0short"    ,                       /* 16-bit signed.                 813*/
 "\4\0long"     ,                       /* 32-bit signed.                 813*/
 "\5\0uchar"    ,                       /* 8-bit  unsigned.               813*/
 "\6\0ushort"   ,                       /* 16-bit unsigned.               813*/
 "\5\0ulong"    ,                       /* 32-bit unsigned.               813*/
 "\5\0float"    ,                       /* 32-bit real.                   813*/
 "\6\0double"   ,                       /* 64-bit real.                   813*/
 "\7\0ldouble"  ,                       /* 80-bit real.                   813*/
 "\4\0void"     ,                       /* void.                          813*/
 "\5\0char*"    ,                       /* 0:32 near ptr to 8-bit  signed.813*/
 "\6\0short*"   ,                       /* 0:32 near ptr to 16-bit signed.813*/
 "\5\0long*"    ,                       /* 0:32 near ptr to 32-bit signed.813*/
 "\6\0uchar*"   ,                       /* 0:32 near ptr to 8-bit  unsigne813*/
 "\7\0ushort*"  ,                       /* 0:32 near ptr to 16-bit unsigne813*/
 "\6\0ulong*"   ,                       /* 0:32 near ptr to 32-bit unsigne813*/
 "\6\0float*"   ,                       /* 0:32 near ptr to 32-bit real.  813*/
 "\5\0dble*"    ,                       /* 0:32 near ptr to 64-bit real.  813*/
 "\6\0ldble*"   ,                       /* 0:32 near ptr to 80-bit real.  813*/
 "\5\0void*"    ,                       /* 0:32 near ptr to void.         813*/
 "\5\0char*"    ,                       /* far ptr to 8-bit  signed.      813*/
 "\6\0short*"   ,                       /* far ptr to 16-bit signed.      813*/
 "\5\0long*"    ,                       /* far ptr to 32-bit signed.      813*/
 "\6\0uchar*"   ,                       /* far ptr to 8-bit  unsigned.    813*/
 "\7\0ushort*"  ,                       /* far ptr to 16-bit unsigned.    813*/
 "\6\0ulong*"   ,                       /* far ptr to 32-bit unsigned.    813*/
 "\6\0float*"   ,                       /* far ptr to 32-bit real.        813*/
 "\5\0dble*"    ,                       /* far ptr to 64-bit real.        813*/
 "\6\0ldble*"   ,                       /* far ptr to 80-bit real.        813*/
 "\5\0void*"    ,                       /* far ptr to void.               813*/
 "\5\0char*"    ,                       /* 0:16 near ptr to 8-bit  signed.813*/
 "\6\0short*"   ,                       /* 0:16 near ptr to 16-bit signed.813*/
 "\5\0long*"    ,                       /* 0:16 near ptr to 32-bit signed.813*/
 "\6\0uchar*"   ,                       /* 0:16 near ptr to 8-bit  unsigne813*/
 "\6\0short*"   ,                       /* 0:16 near ptr to 16-bit unsigne813*/
 "\6\0ulong*"   ,                       /* 0:16 near ptr to 32-bit unsigne813*/
 "\6\0float*"   ,                       /* 0:16 near ptr to 32-bit real.  813*/
 "\5\0dble*"    ,                       /* 0:16 near ptr to 64-bit real.  813*/
 "\6\0ldble*"   ,                       /* 0:16 near ptr to 80-bit real.  813*/
 "\5\0void*"                            /* 0:16 near ptr to void.         813*/
};                                                                      /*813*/

static uchar RecTypes[] =                                               /*813*/
{                                                                       /*813*/
 T_SCALAR,                                                              /*813*/
 T_BITFLD,                                                              /*813*/
 T_TYPDEF,                                                              /*813*/
 T_PROC,                                                                /*813*/
 T_ENTRY,                                                               /*813*/
 T_FUNCTION,                                                            /*813*/
 T_ARRAY,                                                               /*813*/
 T_STRUCT,                                                              /*813*/
 T_PTR,                                                                 /*813*/
 T_ENUM,
};                                                                      /*813*/

static char *RecNames[] =                                               /*813*/
{                                                                       /*813*/
 "\6\0scalar"   ,                                                       /*813*/
 "\6\0bitfld"   ,                                                       /*813*/
 "\7\0typedef"  ,                                                       /*813*/
 "\4\0proc"     ,                                                       /*813*/
 "\5\0entry"    ,                                                       /*813*/
 "\4\0func"     ,                                                       /*813*/
 "\5\0array"    ,                                                       /*813*/
 "\6\0struct"   ,                                                       /*813*/
 "\1\0*"        ,                                                       /*813*/
 "\4\0enum"     ,                                                       /*813*/
 "\7\0unknown"                                                          /*813*/
};                                                                      /*813*/

#define MAXTYPNAMLEN 32                                                 /*813*/
static uchar ptrtypename[MAXTYPNAMLEN+3];                               /*813*/
                                                                        /*813*/
UCHAR * QtypeName(UINT mid,USHORT typeno)                               /*813*/
{                                                                       /*813*/
 UINT    n;                                                             /*813*/
 Trec   *tp;                                                            /*813*/
 uchar  *cp;                                                            /*813*/
 USHORT  len;                                                           /*813*/

 if( typeno < 512 )                                                     /*813*/
 {                                                                      /*813*/
  /***********************************************************************813*/
  /* Get the name of a primitive type no.                                 813*/
  /***********************************************************************813*/
  n = bindex(basetypes,sizeof(basetypes),typeno);                       /*813*/
  if( n < sizeof(basetypes)  )                                          /*813*/
  {                                                                     /*813*/
   cp = basenames[n];                                                   /*813*/
   return( cp );                                                        /*813*/
  }                                                                     /*813*/
 }                                                                      /*813*/
 else                                                                   /*813*/
 {                                                                      /*813*/
  /***********************************************************************813*/
  /* Get the tag/userdef name. If there is no tag or userdef name,        813*/
  /* then use a default name.                                             813*/
  /***********************************************************************813*/
  tp = QtypeRec(mid, typeno);                                           /*813*/
  if( tp )                                                              /*813*/
  {                                                                     /*813*/
   cp = QTagName( tp );                                                 /*813*/
   /**********************************************************************813*/
   /* Append a "*" or "[]" suffix for pointer/array types.                813*/
   /**********************************************************************813*/
   switch( tp->RecType )                                                /*813*/
   {                                                                    /*813*/
    case T_PTR:                                                         /*813*/
      typeno = ((TD_POINTER*)tp)->TypeIndex;                            /*813*/
      if( typeno < 512 )                                                /*813*/
       cp = QtypeName(mid,typeno);                                      /*813*/
                                                                        /*813*/
      len = *(USHORT*)cp + 2;                                           /*813*/
      if( len <= (MAXTYPNAMLEN-2) )                                     /*813*/
      {                                                                 /*813*/
       memcpy(ptrtypename, cp, len );                                   /*813*/
       *(ptrtypename + len) = '*';                                      /*813*/
       *(USHORT*)(cp = ptrtypename) = len + 1;                          /*813*/
      }                                                                 /*813*/
      break;                                                            /*813*/
                                                                        /*813*/
    case T_ARRAY:                                                       /*813*/
      typeno = ((TD_ARRAY*)tp)->ElemType;                               /*813*/
      if( typeno < 512 )                                                /*813*/
       cp = QtypeName(mid,typeno);                                      /*813*/
                                                                        /*813*/
      len = *(USHORT*)cp + 2;                                           /*813*/
      if( len <= (MAXTYPNAMLEN-2) )                                     /*813*/
      {                                                                 /*813*/
       memcpy(ptrtypename, cp, len );                                   /*813*/
       *(ptrtypename + len) = '[';                                      /*813*/
       *(ptrtypename + len+1) = ']';                                    /*813*/
       *(USHORT*)(cp = ptrtypename) = len + 2;                          /*813*/
      }                                                                 /*813*/
      break;                                                            /*813*/
   }                                                                    /*813*/
   return( cp );                                                        /*813*/
  }                                                                     /*813*/
 }                                                                      /*813*/
 return( NULL );                                                        /*813*/
}                                                                       /*813*/

 uint
QtypeGroup( uint mid, uint   typeno )
{
    Trec *tp;

ReCycle:
    if( (typeno == VALUE_CONSTANT) ||   /* if the type number is fake     243*/
        (typeno == ADDRESS_CONSTANT) )  /* addr const or value const      243*/
         return( TG_CONSTANT );         /* then return TG_CONSTANT        243*/
    if( typeno < 512 )
    {
     if( typeno & PRIMITIVE_PTR )                                       /*112*/
         return( TG_POINTER );
     if( bindex(basetypes, sizeof(basetypes), typeno) < sizeof(basetypes) )
         return( TG_SCALAR );
    }
    else
    {
     tp = QbasetypeRec(mid, typeno);
     if( tp )
     {
         switch( tp->RecType )
         {
           case T_PTR:
             return( TG_POINTER );
           case T_STRUCT:
             return( TG_STRUCT );
           case T_CLASS:
             return( TG_CLASS );
           case T_REF:
             return( TG_REF );
           case T_ARRAY:
             return( TG_ARRAY );
           case T_ENUM:
             return( TG_ENUM );
           case T_BITFLD:
             return( TG_BITFLD );
     /***  case T_LIST: ***/
     /***  case T_PROC: ***/
           /*******************************************************************/
           /* In case of a user def record get the type no from the user   512*/
           /* def record and go to top to process the type no.             512*/
           /*******************************************************************/
           case T_TYPDEF:
             typeno = ((TD_USERDEF*)tp)->TypeIndex;
             goto ReCycle;

           case T_CLSDEF:
             typeno = ((TD_CLSDEF*)tp)->TypeIndex;
             goto ReCycle;
         }
     }
    }
    return( TG_UNKNOWN );
}



 int
QstructField(uint mid, UCHAR*pFieldName,uint *offptr,uint *tnoptr)
{
 Trec        *pTrec;
 TD_CLASS    *pClass;
 int          item;
 UCHAR        NameBuf[256];
 BOOL         IsMangled;
 USHORT       NameLength;
 TD_TYPELIST *pClassItemList;

    int i, n, rc;                       /* was register.                  112*/
    TD_STRUCT *tp;                                                      /*813*/
    Trec *NameList;
    TD_TYPELIST *TypeList;                                              /*813*/
    uchar *cp;

    pTrec = (Trec *)QbasetypeRec( mid, *tnoptr );
    switch(pTrec->RecType)
    {
     case T_CLASS:
     {
      int NumMembers;

      pClass         = (TD_CLASS*)pTrec;
      NumMembers     = pClass->NumMembers;
      pClassItemList = (TD_TYPELIST*)QtypeRec(mid, pClass->ItemListIndex);

      for( item = 1 ; item <= NumMembers; ++item )
      {
       cp = NULL;
       cp = (UCHAR *)QClassItemList(mid, pClassItemList, NAME, item-1);
       if(cp)
       {
        IsMangled = unmangle( NameBuf + 2, cp + 2 );

        if( IsMangled )
        {
         *(USHORT*)NameBuf = strlen(NameBuf + 2 );
         cp = NameBuf;
        }

       /*****************************************************************/
       /* When comparing the field names, if the case sensitivity is    */
       /* turned off, compare the names without case sensitivity.       */
       /*****************************************************************/
        NameLength = *(USHORT*)pFieldName + 2;

        rc = memcmp(pFieldName, cp, NameLength);

        if (rc && !cmd.CaseSens)
          rc = memicmp(pFieldName, cp, NameLength);
        if (rc == 0)
        {
         *offptr = QClassItemList(mid, pClassItemList, MEMBEROFFSET, item-1);
         *tnoptr = QClassItemList(mid, pClassItemList, MEMBERTYPE,   item-1);
         return(TRUE);
        }
       }
      }
     }
     break;

     case T_STRUCT:
     {
        if( (tp = (TD_STRUCT*)pTrec)
         && (tp->RecType == T_STRUCT)
         && (TypeList = (TD_TYPELIST*)QtypeRec(mid, tp->TypeListIndex  ))
         && (NameList = QtypeRec(mid, tp->NameListIndex  ))
          ) for( i=1, n = tp->NumMembers; i <= n; ++i )
            {
                cp = (uchar *) QNameList(NameList,NAME,i-1);
                if (cp )
                /*****************************************************************/
                /* When comparing the field names, if the case sensitivity is    */
                /* turned off, compare the names without case sensitivity.       */
                /*****************************************************************/
                {
                 USHORT NameLength;

                 NameLength = *(USHORT*)pFieldName + 2;

                   rc = memcmp(pFieldName, cp, NameLength);
                   if (rc && !cmd.CaseSens)
                     rc = memicmp(pFieldName, cp, NameLength);
                   if (rc == 0)
                   {
                      *offptr = QNameList(NameList,MEMBEROFFSET,i-1);
                      *tnoptr = TypeList->TypeIndex[i-1];
                      return(TRUE);
                   }
                }
            }
     }
     break;
    }
    return(FALSE);
}

/* This procedure returns the address of the Nth element of an array */
 int
QarrayItem(uint mid,uint subscript,uint *offptr,uint *tnoptr)
{
    TD_ARRAY *tp;                                                       /*813*/
    uint itemsize;
    USHORT typeno = *(USHORT*)tnoptr;                                   /*813*/
    uint abs_addr;


    if( QderefType(mid, &typeno) )      /* &ptr[n] evaluates to (ptr + n)    */
    {                                   /*                                   */
     itemsize = QtypeSize(mid, typeno); /*                                   */
     if( itemsize )                     /*                                   */
     {                                  /*                                   */
      abs_addr = DerefPointer(*offptr,mid);                             /*245*/
      if( abs_addr )                                                    /*245*/
      {                                                                 /*245*/
       *offptr = abs_addr + (itemsize * subscript);                     /*245*/
       *tnoptr = typeno;
       return( TRUE );
      }
     }
    /* &array[n] evaluates to (&array[0] + n) */
    }else if( (tp = (TD_ARRAY*)QbasetypeRec(mid, typeno))               /*813*/
        && (tp->RecType == T_ARRAY)                                     /*813*/
        && (itemsize = QtypeSize(mid,typeno = tp->ElemType)) )          /*813*/
    {
        *offptr += itemsize * subscript;
        *tnoptr = typeno;
        return( TRUE );
    }

    return( FALSE );
}
/*****************************************************************************/
/* QderefType()                                                              */
/*                                                                           */
/* Description:                                                              */
/*   Dereference a type number.                                              */
/*                                                                           */
/* Parameters:                                                               */
/*   mid       input - module id.                                            */
/*   tnoptr    input/output - where to put the dereferenced type.            */
/*                                                                           */
/* Return:                                                                   */
/*   1         => *tnoptr is the type of the pointer.                        */
/*   0                                                                       */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
                                        /*                                   */
 int                                    /*                                   */
QderefType(uint mid,USHORT *tnoptr)
                                        /* executable module id.             */
                                        /* where to put type for the caller. */
{                                       /*                                   */
 uint typeno;                           /*                                   */
 Trec *tp;                              /*                                   */
                                        /*                                   */
 typeno = *tnoptr;                      /*                                   */
 if(  typeno < 512 )                    /* if this is a primitive type and   */
 {                                      /*                                   */
  if( typeno & PRIMITIVE_PTR )          /* if it's really a pointer to a/*112*/
  {                                     /* primitive type, then convert back */
   *tnoptr = PrimitiveType(typeno);     /* to type and return 1.             */
   return(1);                           /*                                   */
  }                                     /*                                   */
 }                                      /*                                   */
 else                                   /*                                   */
 {                                      /* if it's not primitive, then       */
  tp = QbasetypeRec(mid, typeno);       /* resolve tp to base type.          */
  if( tp && (tp->RecType == T_PTR) )    /* if it's really a pointer to a     */
  {                                     /* base type, then convert back      */
   *tnoptr = ((TD_POINTER*)tp)->TypeIndex;                              /*813*/
   *tnoptr = HandleUserDefs(mid,*tnoptr);/* Get primitive typeno in case  601*/
   return(1);                           /* of primitive user defs.        601*/
  }                                     /*                                   */
 }                                      /*                                   */
 return(0);                             /*                                   */
}                                       /*                                   */

/*****************************************************************************/
/* QtypeRec()                                                             813*/
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Get a pointer to the type record for a typeno.                          */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   mid        input - module id containing the the typeno.                 */
/*   typeno     input - primitive or complex typeno.                         */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   tp         -> to a type record.                                         */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
Trec *QtypeRec(uint mid, USHORT typeno)
{
 Trec    *p=NULL;
 uchar   *pend;
 uint     n;
 uint     seglen;
 DEBFILE *pdf;

 /****************************************************************************/
 /* If the typeno is primitive, return NULL.                                 */
 /****************************************************************************/
 if( (typeno < 512) ||
     (typeno == VALUE_CONSTANT) ||                                      /*243*/
     (typeno == ADDRESS_CONSTANT) )                                     /*243*/
  return( NULL );

 /****************************************************************************/
 /* - Get a ptr to the EXE/DLL with this mid.                                */
 /* - If there is no type segment or the type segment is empty return NULL.  */
 /****************************************************************************/
 pdf = DBFindPdf(mid);
 p = (Trec *)DBTypSeg(mid, &seglen,pdf);
 if( !p || (seglen == 0 ) )
  return( NULL );

 /****************************************************************************/
 /* Scan the type segment until we get a ptr to the type record for typeno.  */
 /****************************************************************************/
 for( n=512 , pend = (UCHAR*)p + seglen;
      (UCHAR*)p < pend;
      p = NextTrec(p),
      n = (p->RecType != T_SKIP)?n+1:((TD_SKIP*)p)->NewIndex - 1
    )
 {
  if( n == typeno )
   return(p);
 }
 return(NULL);
}

/*****************************************************************************/
/* QbasetypeRec()                                                            */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Get a pointer to the type record for a typeno. Scan through T_TYPEDEF   */
/*   records. Return a pointer to a T_TYPEDEF only if contains a primitive   */
/*   typeno.                                                                 */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   mid        input - module id containing the the typeno.                 */
/*   typeno     input - primitive or complex typeno.                         */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   tp         -> to a type record.                                         */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
Trec *QbasetypeRec(uint mid, USHORT typeno)
{
 Trec *tp;

 tp = QtypeRec(mid, typeno);

 for( ; tp; )
 {
  if( tp->RecType == T_TYPDEF )
  {
   typeno = ((TD_USERDEF*)tp)->TypeIndex;
   if( typeno >= 512 )
   {
    tp = QtypeRec(mid, typeno);
    continue;
   }
   else
    break;
  }
  else if( tp->RecType == T_CLSDEF )
  {
   typeno = ((TD_CLSDEF*)tp)->TypeIndex;
   if( typeno >= 512 )
   {
    tp = QtypeRec(mid, typeno);
    continue;
   }
   else
    break;
  }
  else
   break;
 }

 return(tp);
}

/*****************************************************************************/
/* Qbasetype()                                                               */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Get a pointer to the type record for a typeno. Scan through T_TYPEDEF   */
/*   records. Return a pointer to a T_TYPEDEF only if contains a primitive   */
/*   typeno.                                                                 */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   mid        module id containing the the typeno.                         */
/*   typeno     primitive or complex typeno.                                 */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   typeno                                                                  */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
USHORT Qbasetype(UINT mid, USHORT typeno)
{
 Trec *tp;

 tp = QtypeRec(mid, typeno);

 for( ; tp; )
 {
  if( tp->RecType == T_TYPDEF )
  {
   typeno = ((TD_USERDEF*)tp)->TypeIndex;
   if( typeno >= 512 )
   {
    tp = QtypeRec(mid, typeno);
    continue;
   }
   else
    break;
  }
  else if( tp->RecType == T_CLSDEF )
  {
   typeno = ((TD_CLSDEF*)tp)->TypeIndex;
   if( typeno >= 512 )
   {
    tp = QtypeRec(mid, typeno);
    continue;
   }
   else
    break;
  }
  else
   break;
 }

 return(typeno);
}

/*****************************************************************************/
/* QNameList()                                                            813*/
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Get an instance of an offset or a pointer to a name from a name list.   */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   tp         input - -> to a name list record.                            */
/*   Request    input - -> Name,offset, or value.                            */
/*   Instance   input - instance of the name, offset or value wanted.(0..N). */
/*                      This guy is 0 base like an array.                    */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   pname      -> to the length prefixed name in the record.                */
/*    or                                                                     */
/*   offset        Offset associated with the name. This is returned         */
/*                 if the Request is not NAME.                               */
/*   index         Index in the list of the member with the specified        */
/*                 value passed as a parameter.                              */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   The instance is within the record. There are no checks to validate      */
/*   that the instance is in range.                                          */
/*                                                                           */
/*****************************************************************************/

#define NextName(p)      ((p)+*(USHORT*)(p)+6)
#define MemberOffset(p)  (*  (ULONG*)( (p) + *(USHORT*)(p) + 2) )
#define MemberValue(p)   (*  (ULONG*)( (p) + *(USHORT*)(p) + 2) )

ULONG QNameList( Trec *tp, int Request , int InstanceOrValue )
{
 char  *pname;
 char  *pend;
 ULONG  i;
 int    Instance,Value;

 /****************************************************************************/
 /*                                                                          */
 /* - Find the desired instance of the name or offset.                       */
 /*                                                                          */
 /*        2          1      1                        4                      */
 /*       ---------- ------ ------ -------- ---//--- ------------            */
 /*      | namelist | 0x7F | flag | name   | name   | offset or  |           */
 /* tp-->| length   |      |      | length |        | value      |           */
 /*       ---------- ------ ------ -------- ---//--- ------------            */
 /*      |                 |       ^                                         */
 /*      |<----Trec------->|       |                                         */
 /*                                |                                         */
 /*            (initial) pname-----                                          */
 /*                                                                          */
 /****************************************************************************/
 pname = (char*)tp + sizeof(Trec) + 1;
 switch( Request )
 {
  case NAME:
  case MEMBEROFFSET:

  Instance = InstanceOrValue;
  for(i=0; i<Instance; i++,pname = NextName(pname) ){;}
  if( Request == NAME )
    return((ULONG)pname);
  return( MemberOffset(pname) );

  case VALUEINDEX:
  case VERIFYVALUE:
  /****************************************************************************/
  /*                                                                          */
  /* - Find the instance that contains the value passed.                      */
  /* - Return TRUE or FALSE if we're verifying that the value is in the list. */
  /*   We do this because there is no value available to use as a return      */
  /*   code that we can distinguish from a valid value.                       */
  /* - Or, return the index that contains the value.                          */
  /*                                                                          */
  /****************************************************************************/
  pend = (char*)tp + tp->RecLen + 2;

  Value = InstanceOrValue;
  for ( i=0 , pname; pname<pend; pname=NextName(pname), i++)
  {
   if( MemberValue(pname) == Value )
   {
    if( Request == VERIFYVALUE )
     return(TRUE);
    return(i);
   }
  }
 }
 return(FALSE);
}
/*****************************************************************************/
/* QtypeSize()                                                            813*/
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Get the storage size for a typeno.                                      */
/*                                                                           */
/*      Primitive types as taken from the MSC object module format doc.      */
/*                                                                           */
/*      Types 0-511 are reserved. Types 0-255(high byte = 0) have meaning    */
/*      according to the decoding of the following bits:                     */
/*                                                                           */
/*      xxxx xxxx x xx xxx xx                                                */
/*      xxxx xxxx i md typ sz                                                */
/*                                                                           */
/*      i=0 ==> special type don't interpret md,typ,and sz.                  */
/*      i=1 ==> interpret low order 7 bits as follows.                       */
/*                                                                           */
/*      md - Model                                                           */
/*      00 - Direct                                                          */
/*      01 - Near pointer                                                    */
/*      10 - Far pointer                                                     */
/*      11 - Huge pointer                                                    */
/*                                                                           */
/*      type - base type                                                     */
/*      000  - signed                                                        */
/*      001  - unsigned                                                      */
/*      010  - real                                                          */
/*      011  -                                                               */
/*      100  -                                                               */
/*      101  -                                                               */
/*      110  -                                                               */
/*      111  -                                                               */
/*                                                                           */
/*      sz   - size                                                          */
/*      00   - 8-bit                                                         */
/*      01   - 16-bit                                                        */
/*      10   - 32-bit                                                        */
/*      11   -                                                               */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   mid        input - module id containing the typeno.                     */
/*   typeno     input - primitive or complex typeno.                         */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   typesize           storage size for the type.                           */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   The mid contains the typeno.                                            */
/*                                                                           */
/*****************************************************************************/
uint QtypeSize( uint mid, USHORT typeno )
{
 uint typesize=0;
 Trec *p;

ReCycle:
 /****************************************************************************/
 /* Handle primitive types.                                                  */
 /****************************************************************************/
 if( typeno < 512 )
 {
  switch(typeno )
  {
    /*************************************************************************/
    /* Primitive types.                                                      */
    /*************************************************************************/
    case TYPE_CHAR:
    case TYPE_UCHAR:
     return(sizeof(char));

    case TYPE_SHORT:
    case TYPE_USHORT:
     return(sizeof(short));

    case TYPE_LONG:
    case TYPE_ULONG:
     return(sizeof(long));

    case TYPE_FLOAT:
     return(sizeof(float));

    case TYPE_DOUBLE:
     return(sizeof(double));

    case TYPE_LDOUBLE:
    {
     MODULE *pModule = GetPtrToModule( mid, NULL );

     if( pModule->DbgFormatFlags.Syms == TYPE104_C600 )
       return( 10 );
     else
       return( sizeof( long double ) );
    }

    case TYPE_VOID:
     return(0);

    /*************************************************************************/
    /* Primitive 0:32 pointers and 16:16 pointers.                           */
    /*************************************************************************/
    case TYPE_PCHAR    :
    case TYPE_PSHORT   :
    case TYPE_PLONG    :
    case TYPE_PUCHAR   :
    case TYPE_PUSHORT  :
    case TYPE_PULONG   :
    case TYPE_PFLOAT   :
    case TYPE_PDOUBLE  :
    case TYPE_PLDOUBLE :
    case TYPE_PVOID    :
    case TYPE_FPCHAR   :
    case TYPE_FPSHORT  :
    case TYPE_FPLONG   :
    case TYPE_FPUCHAR  :
    case TYPE_FPUSHORT :
    case TYPE_FPULONG  :
    case TYPE_FPFLOAT  :
    case TYPE_FPDOUBLE :
    case TYPE_FPLDOUBLE:
    case TYPE_FPVOID   :
     return(4);

    /*************************************************************************/
    /* 0:16 near pointers.                                                   */
    /*************************************************************************/
    case TYPE_N16PCHAR    :
    case TYPE_N16PSHORT   :
    case TYPE_N16PLONG    :
    case TYPE_N16PUCHAR   :
    case TYPE_N16PUSHORT  :
    case TYPE_N16PULONG   :
    case TYPE_N16PFLOAT   :
    case TYPE_N16PDOUBLE  :
    case TYPE_N16PLDOUBLE :
    case TYPE_N16PVOID    :
     return(2);

  }
 }

 /****************************************************************************/
 /* Handle complex types.                                                    */
 /****************************************************************************/
tryagain:                                                               /*912*/
 p = QbasetypeRec(mid, typeno);

 if(p == NULL )                                                         /*828*/
  return(0);/* size=0 */                                                /*828*/

 switch( p->RecType )
 {
  case T_BITFLD:
   break;

  case T_TYPDEF:
   typeno = ((TD_USERDEF*)p)->TypeIndex;
   goto ReCycle;

  case T_PROC:
   typesize = 16;
   break;

  case T_ARRAY:
   typesize = ((TD_ARRAY*)p)->ByteSize;
   if( typesize == 0 )                                                  /*912*/
   {                                                                    /*912*/
    typeno = ((TD_ARRAY*)p)->ElemType;                                  /*912*/
    goto tryagain;                                                      /*912*/
   }                                                                    /*912*/
   break;

  case T_STRUCT:
   typesize = ((TD_STRUCT*)p)->ByteSize;
   break;

  case T_CLASS:
   typesize = ((TD_STRUCT*)p)->ByteSize;
   break;

  case T_PTR:
   switch(  ((TD_POINTER*)p)->Flags)
   {
    case PTR_0_32:
    case PTR_16_16:
     typesize = 4;
     break;

    case PTR_0_16:
     typesize = 2;
     break;
   }
   break;

  case T_ENUM:
   typesize = ((TD_ENUM*)p)->DataType;
   break;

 }
 return(typesize);
}

/*****************************************************************************/
/* HandleUserDefs()                                                       512*/
/*                                                                           */
/* Description:                                                              */
/*   Returns the primitive typeno from the the userdef records.              */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   Mid        input - module Id                                            */
/*   TypeNo     input - type no                                              */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   TypeNo     returns the type number.                                     */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
UINT HandleUserDefs(UINT Mid, UINT TypeNo)
{
 Trec   *tp;
 USHORT  type;

 /**************************************************************************/
 /* if the type number is primitive return.                                */
 /**************************************************************************/
 if (TypeNo < 512)
   return(TypeNo);

 /**************************************************************************/
 /* get the base type record for this type number. If it is a userdef      */
 /* record and the type number is primitive return the primitive type      */
 /* number or else return the inputed typeno.                              */
 /**************************************************************************/
 tp = QbasetypeRec(Mid, TypeNo);
 if( tp )
 {
  if( tp->RecType == T_TYPDEF)
  {
   type = ((TD_USERDEF *)tp)->TypeIndex;
   if (type < 512)
    return((uint)type);
  }
  else if( tp->RecType == T_CLSDEF )
  {
   type = ((TD_CLSDEF *)tp)->TypeIndex;
   return(type);
  }
 }
 return(TypeNo);
}

/*****************************************************************************/
/* QTagName()                                                             813*/
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Get the name of a complex type.                                         */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   tp         input - -> to a name list record.                            */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   cp         -> to the length prefixed name.                              */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*  tp != NULL                                                               */
/*                                                                           */
/*****************************************************************************/

char *QTagName( Trec *tp )
{
 char *cp = NULL;

 /****************************************************************************/
 /* Return a pointer to the length prefixed name of the tag/userdef.         */
 /****************************************************************************/
 switch(tp->RecType)
 {
  case T_TYPDEF:
  {
   TD_USERDEF* tpx = (TD_USERDEF*)tp;

   if(tpx->NameLen)
     cp = (char*)&tpx->NameLen;
  }
  break;

  case T_ARRAY:
  {
   TD_ARRAY* tpx = (TD_ARRAY*)tp;

   if(tpx->NameLen)
     cp = (char*)&tpx->NameLen;
  }
  break;

  case T_STRUCT:
  {
   TD_STRUCT* tpx = (TD_STRUCT*)tp;

   if(tpx->NameLen)
     cp = (char*)&tpx->NameLen;
  }
  break;

  case T_CLASS:
  {
   TD_CLASS* tpx = (TD_CLASS*)tp;

   if(tpx->NameLen)
     cp = (char*)&tpx->NameLen;
  }
  break;

  case T_PTR:
  {
   TD_POINTER* tpx = (TD_POINTER*)tp;

   if(tpx->NameLen)
     cp = (char*)&tpx->NameLen;
  }
  break;

  case T_ENUM:
  {
   TD_ENUM* tpx = (TD_ENUM*)tp;

   if(tpx->NameLen)
     cp = (char*)&tpx->NameLen;
  }
  break;
 }
 /****************************************************************************/
 /* If we don't find a tag/userdef name,then stuff in the "record" name.     */
 /****************************************************************************/
 if( !cp )
 {
  int   i;

  i =  bindex(RecTypes,sizeof(RecTypes),tp->RecType);
  cp = RecNames[i];
 }
 return(cp);
}

/*****************************************************************************/
/* QTypeNumber()                                                          813*/
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Find the type number in a mid for a tag name or userdef.                */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   mid        input  - the module we're searching in.                      */
/*   cp         input  - -> to the tag/userdef we're searching for.          */
/*                          (length prefixed.)                               */
/*   mid4type   output - if mid==0, this is the mid where the user defined   */
/*                       was found.                                          */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   typeno     the typeno or 0.                                             */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
USHORT QtypeNumber(UINT mid, UCHAR *cp, UINT *pmid4type)
{
 USHORT   typeno;
 USHORT   len;
 UINT     seglen;
 DEBFILE *pdf;
 Trec    *tp;
 Trec    *tpend;
 UCHAR   *ptag;
 UINT     mid4type;

 /****************************************************************************/
 /* - If the mid==0, then we're probably trying to format an absolute        */
 /*   address displayed in the data window.                                  */
 /****************************************************************************/
 if( mid == 0 )
 {
  typeno = QtypeNumberAndMid( cp, &mid4type );

  if( typeno )
   *pmid4type = mid4type;

  return( typeno );

 }

 /****************************************************************************/
 /* - Find the pdf containing the mid.                                       */
 /* - Get a pointer to the type segment.                                     */
 /* - Scan the type records looking for the tag/userdef.                     */
 /****************************************************************************/
 if( ( pdf = DBFindPdf(mid) ) &&
     ( tp  = (Trec *)DBTypSeg(mid, &seglen,pdf) ) &&
       seglen
   )
 {
  typeno = 512;
  tpend = (Trec*)( (UCHAR*)tp + seglen);
  for( ; (tp < tpend); ++typeno, tp = NextTrec(tp) )
  {
   if( ( ptag = QTagName(tp)) &&
       ( *(USHORT*)ptag == ( len = *(USHORT*)cp) ) &&
       ( strnicmp(cp+2,ptag+2,len) == 0 )
     )
    return(typeno);
  }
 }
 return(0);
}

/*****************************************************************************/
/* QTypeNumberAndMid()                                                       */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Find the type number and mid for a type name or userdef.                */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   cp         -> the the type name string.                                 */
/*   midptr     -> to the receiver of the mid containing the type name.      */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   typeno     the typeno or 0.                                             */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
USHORT QtypeNumberAndMid( UCHAR *cp, UINT *midptr )
{
 USHORT   typeno;
 USHORT   len;
 UINT     seglen;
 DEBFILE *pdf;
 Trec    *tp;
 Trec    *tpend;
 UCHAR   *ptag;
 UINT     mid;
 AFILE   *fp;

 /****************************************************************************/
 /* Currenly, this function only gets called when the user is trying to      */
 /* format an area of memory displayed in the data window using an           */
 /* absolute address.  The address will usually be of the form               */
 /* 0xabcdef or 0xf:abcd.                                                    */
 /*                                                                          */
 /* - Get the view that is currently in focus.                               */
 /* - Get the executable containing the view.                                */
 /****************************************************************************/
 fp  = Getfp_focus();
 pdf = fp->pdf;

 /****************************************************************************/
 /* - Scan the modules linked into the executable looking for the            */
 /*   type name.                                                             */
 /****************************************************************************/
 for( mid = DBNextMod(0, pdf); mid != 0; mid = DBNextMod(mid, pdf) )
 {
  if( ( pdf = DBFindPdf(mid) ) &&
      ( tp  = (Trec *)DBTypSeg(mid, &seglen,pdf) ) &&
        seglen
    )
  {
   typeno = 512;
   tpend = (Trec*)( (UCHAR*)tp + seglen);
   for( ; (tp < tpend); ++typeno, tp = NextTrec(tp) )
   {
    if( ( ptag = QTagName(tp)) &&
        ( *(USHORT*)ptag == ( len = *(USHORT*)cp) ) &&
        ( strnicmp(cp+2,ptag+2,len) == 0 )
      )
    {
     /************************************************************************/
     /* - If we find the type name, then tell the caller which module        */
     /*   it was found in.                                                   */
     /************************************************************************/
     *midptr = mid;
     return(typeno);
    }
   }
  }
 }
 /****************************************************************************/
 /* - Type name was not found.                                               */
 /****************************************************************************/
 return(0);
}

/*****************************************************************************/
/* QClassItemList()                                                          */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Retrieve info for an item in a class member list.                       */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   mid                                                                     */
/*   tp         -> to a class item list.                                     */
/*   Request    What kind of info.                                           */
/*   ItemNo     Which item in the list.                                      */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   RetVal                                                                  */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
ULONG QClassItemList( UINT mid, TD_TYPELIST *tp, int Request , int ItemNo )
{
 char   *pList;
 USHORT  typeno;
 Trec   *pTrec;
 ULONG   RetVal = 0;

 /****************************************************************************/
 /*                                                                          */
 /*        2          1      1     2        2        2                       */
 /*       ---------- ------ ------ -------- -------- -------- --//           */
 /*      | list     | 0x7F | flag | list   | list   | list   |               */
 /* tp-->| length   |      |      | item 1 | item 2 | item 3 |               */
 /*       ---------- ------ ------ -------- -------- -------- --//           */
 /*      |                 |                                                 */
 /*      |<----Trec------->|                                                 */
 /*                                                                          */
 /****************************************************************************/
 pList  = (char*)tp + sizeof(Trec) + 1;
 typeno = ((USHORT*)pList)[ItemNo];

 if(typeno == 0 )
  return(0);

 pTrec  = QtypeRec(mid, typeno);

 switch( pTrec->RecType )
 {
  case T_MEMFNC:
  {
   TD_MEMFNC *pMemFnc = (TD_MEMFNC *)pTrec;

   switch( Request )
   {
    case NAME:
    {
     RetVal = (ULONG)&(pMemFnc->NameLen);
    }
    break;

    case MEMBEROFFSET:
    {
     RetVal = 0;
    }
    break;

    case MEMBERTYPE:
    {
     RetVal = typeno;
    }
    break;

   }
  }
  break;

  case T_CLSMEM:
  {
   TD_CLSMEM *pClsMem = (TD_CLSMEM *)pTrec;
   char       StaticName[256];
   char       PublicName[256];

   switch( Request )
   {
    case NAME:
    {
     RetVal = (ULONG)&(pClsMem->NameLen);
    }
    break;

    case MEMBEROFFSET:
    {
     if( pClsMem->TypeQual & CLSMEM_STATIC )
     {
      /***********************************************************************/
      /* - for static entries try:                                           */
      /*                                                                     */
      /*    - looking in symbols for static mangled name.                    */
      /*    - looking in publics for static unmangled name.                  */
      /*    - looking in symbols for static unmangled name.                  */
      /*                                                                     */
      /***********************************************************************/
      int len;

      len = pClsMem->NameLen + sizeof(pClsMem->NameLen);

      memset( StaticName, 0, sizeof(StaticName) );
      memcpy( StaticName, &(pClsMem->NameLen), len );
      RetVal = findsvar(mid, StaticName);


      if( RetVal == 0 )
      {
       char *cp;

       /**********************************************************************/
       /* - save the static name...we might need to search publics.          */
       /**********************************************************************/
       memset( PublicName, 0, sizeof(PublicName) );
       memcpy( PublicName, StaticName, len );

       cp  = ((char*)&(pClsMem->Name)) + pClsMem->NameLen;
       len = *(USHORT*)cp + sizeof(pClsMem->NameLen);

       memset(StaticName, 0, sizeof(StaticName) );
       memcpy(StaticName, cp, len );
       RetVal = findsvar(mid, StaticName);
      }

      if( RetVal == 0 )
      {
       DEBFILE *pdf;

       pdf=DBFindPdf(mid);
       if( pdf != NULL )
       {
        RetVal = DBPub(PublicName+2, pdf);
       }
      }
     }
     else
      RetVal = pClsMem->Offset;
    }
    break;

    case MEMBERTYPE:
    {
     RetVal = pClsMem->TypeIndex;
    }
    break;

   }
  }
  break;

  case T_BSECLS:
  {
   TD_BSECLS *pBseCls = (TD_BSECLS*)pTrec;
   TD_CLASS  *pClass;

   switch( Request )
   {
    case NAME:
    {
     typeno  = pBseCls->TypeIndex;
     pClass  = (TD_CLASS*)QtypeRec(mid, typeno);
     RetVal  = (ULONG)&(pClass->NameLen);
    }
    break;

    case MEMBEROFFSET:
    {
     RetVal = pBseCls->Offset;
    }
    break;

    case MEMBERTYPE:
    {
     RetVal = pBseCls->TypeIndex;
    }
    break;
   }
  }
  break;

  case T_CLSDEF:
  {
   TD_CLSDEF *pClsDef = (TD_CLSDEF*)pTrec;

   switch( Request )
   {
    case NAME:
    {
     typeno = pClsDef->TypeIndex;
     RetVal = (ULONG)QtypeName(mid, typeno);
    }
    break;

    case MEMBEROFFSET:
    {
     RetVal = 0;
    }
    break;

    case MEMBERTYPE:
    {
     RetVal = pClsDef->TypeIndex;
    }
    break;
   }
  }
  break;

  case T_FRIEND:
  {
   TD_FRIEND *pFriend;

   pFriend = (TD_FRIEND *)pTrec;

   switch( Request )
   {
    case NAME:
    {
     if( pFriend->TypeQual & FRIEND_CLASS )
     {
      /***********************************************************************/
      /* - should only come here if friend class.                            */
      /***********************************************************************/
      TD_CLASS  *pClass;

      typeno = pFriend->TypeIndex;
      pClass = (TD_CLASS*)QtypeRec(mid, typeno);
      RetVal = (ULONG)&(pClass->NameLen);
     }
     else
      RetVal = (ULONG)&(pFriend->NameLen);
    }
    break;

    case MEMBEROFFSET:
    {
     RetVal = 0;
    }
    break;

    case MEMBERTYPE:
    {
     RetVal = pFriend->TypeIndex;
    }
    break;

   }
  }
  break;
 }
 return(RetVal);
}
