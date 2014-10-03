/*****************************************************************************/
/* File:                                                                     */
/*                                                                           */
/*   showclas.c                                                              */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*  Display c++ objects.                                                     */
/*                                                                           */
/*                                                                           */
/* History:                                                                  */
/*                                                                           */
/*   03/21/96 Created.                                                       */
/*                                                                           */
/*****************************************************************************/
#include "all.h"
#include "demangle.h"

extern UINT VideoCols;
extern UINT ExprAddr;                   /* Address value of expr             */
                                        /* ( Set by ParseExpr )              */

static BOOL ViewMemFncs = TRUE;

void SetViewMemFncs( void ) {ViewMemFncs ^= 1;}
BOOL GetViewMemFncs( void ) {return(ViewMemFncs);}

/*****************************************************************************/
/* ShowClass()                                                               */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Show a c++ object.                                                      */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   dfp       pointer to dfile node.                                        */
/*   row       where to start the display.                                   */
/*   rows      rows available for display.                                   */
/*   skip      # of logical records to skip.                                 */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   TRUE                                                                    */
/*   FALSE                                                                   */
/*                                                                           */
/*****************************************************************************/
#define ISA_STRUCT 0x01
UINT ShowClass(DFILE *dfp, UINT row, UINT rows, UINT skip)
{
 UINT      item;
 UINT      rr;
 Trec     *tp;
 UINT      Nitems;
 UINT      mid;
 UINT      sfx;
 UCHAR    *cp;
 UCHAR    *pBuf;
 UINT      baseaddr;
 UCHAR     buffer[1024];         /* adjust this when you know max chars/line */
 ULONG     memberoffs;                  /* offset of member within structure.*/
 ULONG     membertype;                  /* type of the class member.         */
 ULONG     itemtypeno;                  /* type list number.                 */
 BOOL      IsStruct;
 BOOL      IsMangled;
 int       ColonColumn = 0;

 TD_CLASS    *pClass;
 TD_TYPELIST *pClassItemList;

 mid      = dfp->mid;
 sfx      = dfp->sfx;
 baseaddr = dfp->baseaddr;

 /****************************************************************************/
 /* - verify that this is a class record and that it has members.            */
 /* - some classes are null.                                                 */
 /****************************************************************************/
 pClass = (TD_CLASS*)QbasetypeRec(mid, dfp->showtype);
 if( (pClass == NULL)                ||
     (pClass->RecType    != T_CLASS) ||
     (pClass->NumMembers == 0)
   )
  return(FALSE);

 /****************************************************************************/
 /* - format structures as in c.                                             */
 /****************************************************************************/
 IsStruct = FALSE;
 if(pClass->TypeQual & ISA_STRUCT )
  IsStruct = TRUE;

 /****************************************************************************/
 /* - get the number of class members.                                       */
 /* - quit if we were told to skip more than the number of class members.    */
 /****************************************************************************/
 Nitems = pClass->NumMembers;
 if( Nitems <= skip )
  return(FALSE);

 /****************************************************************************/
 /* - get a ptr to the class item list.                                      */
 /* - loop formatting each item.                                             */
 /****************************************************************************/
 pClassItemList = (TD_TYPELIST*)QtypeRec(mid, pClass->ItemListIndex);

 for( rr=0, item=skip+1;
      (rr < rows) &&
      (item <= Nitems);
      ++rr, ++item
    )
 {
  /***************************************************************************/
  /* - init a display buffer for the class item.                             */
  /* - put tabstops in for formatting purposes.                              */
  /***************************************************************************/
  InitClassItem( buffer, sizeof(buffer) );
  PutInTabStops( buffer);

  pBuf = buffer;

  /***************************************************************************/
  /* - get a ptr to the type record for this item.                           */
  /* - we can have an item typeno == 0 because we added a "0" type index     */
  /*   as the first item in the list when mapping base classes.              */
  /*********************************************w*****************************/
  itemtypeno = pClassItemList->TypeIndex[item-1];
  tp         = NULL;

  if(itemtypeno != 0 )
   tp = (Trec *)QtypeRec(mid, itemtypeno );

  /***************************************************************************/
  /* - if not viewing member functions, then continue. don't format.         */
  /***************************************************************************/
  if( (IsStruct         == FALSE) &&
      (GetViewMemFncs() == FALSE) &&
       tp                         &&
      (tp->RecType == T_MEMFNC)
    )
  {
   rr--;
   continue;
  }

  /***************************************************************************/
  /* - we want to display the expr name for the first display item.          */
  /* - if this is really a struct, then display as in c.                     */
  /* - if it's a class, put in the class name.                               */
  /* - if it's a derived class, then proceed to format it's base class.      */
  /* - if it's a base class, then we're done formatting this item.           */
  /***************************************************************************/
  if( (item==1) && dfp && dfp->expr )
  {
   strncpy( pBuf+1, dfp->expr, strlen(dfp->expr) );

   pBuf = NextTab(pBuf);

   if( IsStruct == FALSE )
   {
    pClass = (TD_CLASS*)QtypeRec(mid, dfp->showtype);

    strncpy( pBuf, pClass->Name, pClass->NameLen );

    if( dfp->DfpFlags.ClassType == DFP_DERIVED_CLASS )
     pBuf += pClass->NameLen;
   }
  }

  /***************************************************************************/
  /* - format member attributes, i.e. public, static, etc..                  */
  /***************************************************************************/
  if( tp && (IsStruct == FALSE) )
  {
   switch(tp->RecType)
   {
    case T_FRIEND:
    {
     FmtFriendAttrs( tp, pBuf + 4 );
     *(pBuf + strlen(pBuf)) = ' ';
     pBuf = NextTab(pBuf);
    }
    break;

    case T_MEMFNC:
    {
     FmtMemFncAttrs( tp, pBuf + 4 );
     *(pBuf + strlen(pBuf)) = ' ';
     pBuf = NextTab(pBuf);
    }
    break;

    case T_CLSMEM:
    {
     FmtClsMemAttrs( tp, pBuf + 4 );
     *(pBuf + strlen(pBuf)) = ' ';
     pBuf = NextTab(pBuf);
    }
    break;

    case T_CLSDEF:
    {
     FmtClsDefAttrs( tp, pBuf + 4 );
     *(pBuf + strlen(pBuf)) = ' ';
     pBuf = NextTab(pBuf);
    }
    break;

    case T_BSECLS:
    {
     int len;

     /************************************************************************/
     /* - put in some formatting so that we can line up the multiple         */
     /*   inheritance classes.                                               */
     /************************************************************************/
     if( item == 1 )
      ColonColumn = pBuf - buffer;
     else if( ColonColumn == 0 )
      pBuf = NextTab(pBuf);
     else
      pBuf += ColonColumn;

     FmtBseClsAttrs( tp, pBuf );

     len            = strlen(pBuf);
     *(pBuf + len)  = ' ';
     pBuf          += len;
    }
    break;
   }
  }
  else
   pBuf = NextTab(pBuf);

  /***************************************************************************/
  /* - format the item name and demangle it if need be.                      */
  /* - a base class record will not have a name.                             */
  /***************************************************************************/
  cp = (UCHAR *)QClassItemList(mid, pClassItemList, NAME, item-1);
  if( cp != NULL )
  {
   IsMangled = FALSE;
   IsMangled = unmangle( pBuf, cp + 2 );
   if( IsMangled == FALSE )
   {
    int len = *(USHORT*)cp;

    strncpy(pBuf, cp+2, len);
   }
   pBuf = NextTab(pBuf);
  }

  /***************************************************************************/
  /* - format class member records.                                          */
  /* - format class def records.                                             */
  /***************************************************************************/
  if(  tp &&
      ( (tp->RecType == T_CLSMEM) ||
        (tp->RecType == T_CLSDEF)
      )
    )
  {
   /**************************************************************************/
   /* - set the base address location for the object.                        */
   /* - if this is a static member, then kill the baseaddr.                  */
   /**************************************************************************/
   baseaddr = dfp->baseaddr;

   if( tp->RecType == T_CLSMEM )
   {
    TD_CLSMEM *pClsMem;

    pClsMem = (TD_CLSMEM *)tp;

    if( pClsMem->TypeQual & CLSMEM_STATIC )
     baseaddr = 0;
   }

   /**************************************************************************/
   /* - get the offset and type of this member within the object.            */
   /* - for a static member, the offset will be a flat address.              */
   /**************************************************************************/
   memberoffs = QClassItemList(mid, pClassItemList, MEMBEROFFSET, item-1);
   membertype = QClassItemList(mid, pClassItemList, MEMBERTYPE,   item-1);

   if( tp->RecType == T_CLSDEF )
   {
    /*************************************************************************/
    /* - if this is a class def then simply append a suffix for typedefs     */
    /*   and enums. These members do not occupy any storage.                 */
    /*************************************************************************/
    tp = (Trec*)QtypeRec(mid, membertype);
    if( tp->RecType == T_TYPDEF )
    {
     strcpy(pBuf, "...typedef");
     *(strchr(pBuf, '\0')) = ' ';
    }
    else if( tp->RecType == T_ENUM )
    {
     strcpy(pBuf, "...enum");
     *(strchr(pBuf, '\0')) = ' ';
    }
    else
     FormatDataItem( pBuf, baseaddr + memberoffs, mid, membertype, sfx );
   }
   else
    FormatDataItem( pBuf, baseaddr + memberoffs, mid, membertype, sfx );
  }
  /***************************************************************************/
  /* - chop off the buffer at the max screen width.                          */
  /* - take out the tabstops that have not been written on.                  */
  /***************************************************************************/
  buffer[ VideoCols + 2 ] = 0;
  PullOutTabStops( buffer );
  putrc( row + rr, 0, buffer );
 }

 /****************************************************************************/
 /* - clear the remaining display lines.                                     */
 /****************************************************************************/
 if( rr < rows )
   ClrScr( row + rr, row + rows - 1, vaStgExp );
 return( TRUE );
}

UINT ZapClass(DFILE *dfp,UINT row,UINT skip )
{
#if 0
    TD_STRUCT *tp;                                                      /*813*/

    TD_TYPELIST *typelist;                                              /*813*/

    Trec *namelist;
    UINT itemtype, mid = dfp->mid, sfx = dfp->sfx;
    uchar *cp, *baseaddr = (uchar*) ExprAddr;

    if( (tp = (TD_STRUCT*)QbasetypeRec(mid, dfp->showtype))
     && (tp->RecType == T_STRUCT)
     && (tp->NumMembers) > skip                                         /*813*/
     && (typelist = (TD_TYPELIST*)QtypeRec(mid,tp->TypeListIndex))      /*813*/
     && (namelist = QtypeRec(mid, tp->NameListIndex))                   /*813*/
     && (cp = (UCHAR*)QNameList(namelist,NAME,skip))                    /*813*/
     && (itemtype = typelist->TypeIndex[skip] )                         /*813*/
     )
    {
     return( (* PickKeyer( mid, itemtype))
            (mid, itemtype, sfx, row, *cp + 2,
            baseaddr + QNameList(namelist,MEMBEROFFSET,skip)  ) );   /*813215*/
    }
    return( FALSE );
#endif
}


/*****************************************************************************/
/* - insert tabstops into the buffer. i guess a tab stop is 0x09...don't     */
/*   really know...it doesn't matter.                                        */
/*****************************************************************************/
#define FIRSTTAB 20
#define TABSTOP  10
#define TABCHAR  0x09
void PutInTabStops( char *pbuf )
{
 char *cp = pbuf;
 int   i,j;

 cp[FIRSTTAB-1] =  TABCHAR;
 j              =  FIRSTTAB;
 cp             = &cp[FIRSTTAB-1];
 i = 0;
 while(*cp++)
 {
  if( i == TABSTOP )
  {
   pbuf[j-1]=TABCHAR;
   i = 0;
  }
  i++;
  j++;
 }
}

/*****************************************************************************/
/* - extract tabstops.                                                       */
/*****************************************************************************/
void PullOutTabStops( char *cp )
{
 for( ; *cp; cp++ )
 {
  if(*cp == TABCHAR)
   *cp = ' ';
 }
}

/*****************************************************************************/
/* - return a ptr to the next tabstop.                                       */
/*****************************************************************************/
char *NextTab( char *cp )
{
 while(*cp != TABCHAR) cp++;
 return(cp);
}

/*****************************************************************************/
/* InitClassItem()                                                           */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Initialize a buffer for the display of a c++ class member.              */
/*                                                                           */
/*    --- ------------------------------ ---                                 */
/*   |   |                              |   |                                */
/*   |atr| blanks                       | 0 |                                */
/*    --- ------------------------------ ---                                 */
/*                                                                           */
/*    atr is the display attribute for the class item.                       */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   cp                                                                      */
/*   bufsize                                                                 */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
void InitClassItem( UCHAR *cp, int bufsize )
{
 memset( cp, ' ' , bufsize );

 cp[0]           = Attrib(vaStgExp);
 cp[bufsize-1]   = 0;
}

/*****************************************************************************/
/* unmabgle()                                                                */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*  Demangle a mangled name.                                                 */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   pDeMangledName  ->to receiver of demangled name.                        */
/*   pMangledName                                                            */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   IsMangled                                                               */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*  pMangledName   != NULL                                                   */
/*  pDeMangledName != NULL                                                   */
/*                                                                           */
/*****************************************************************************/
BOOL unmangle( UCHAR *pDeMangledName, UCHAR *pMangledName )
{
 BOOL  IsMangled;
 char *cp;
 Name *pName;
 char *pRest;

 IsMangled = FALSE;
 pName = demangle( pMangledName, &pRest, RegularNames );
 if( pName != NULL )
 {
  IsMangled = TRUE;
  switch(kind(pName))
  {
   case MemberFunction:
   case Function:
   {
    cp = text(pName);
    strncpy(pDeMangledName, cp, strlen(cp) );
   }
   break;

   case MemberVar:
    cp = varName(pName);
    strncpy(pDeMangledName, cp, strlen(cp) );
    break;

   default:
    break;
  }
 }
 return(IsMangled);
}

/*****************************************************************************/
/* - only count the non-member function lines if the member function         */
/*   display is turned off.                                                  */
/*****************************************************************************/
UINT GetLinesNoMemFncs( UINT mid, Trec *tp )
{
 TD_CLASS    *pClass = (TD_CLASS*)tp;
 TD_TYPELIST *pClassItemList;
 UINT         Nitems;
 UINT         NitemsNoFncs;
 UINT         item;
 ULONG        itemtypeno;

 pClassItemList = (TD_TYPELIST*)QtypeRec(mid, pClass->ItemListIndex);
 Nitems         = pClass->NumMembers;

 NitemsNoFncs = 0;
 for( item = 1; item <= Nitems; ++item )
 {
  itemtypeno = pClassItemList->TypeIndex[item-1];
  tp         = NULL;

  if(itemtypeno != 0 )
   tp = (Trec *)QtypeRec(mid, itemtypeno );

  if( tp && (tp->RecType != T_MEMFNC) )
   NitemsNoFncs++;
 }
 return( NitemsNoFncs );
}
