/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   showd2.c                                                                */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*  data display routines.                                                   */
/*                                                                           */
/*                                                                           */
/* History:                                                                  */
/*                                                                           */
/*   02/08/91 Creation of 32-bit SD86, from 16-bit version.                  */
/*                                                                           */
/*...16->32 port.                                                            */
/*...                                                                        */
/*... 02/08/91  100   Philip    port to 32 bit.                              */
/*... 02/08/91  101   Joe       port to 32 bit.                              */
/*... 02/08/91  102   Pratima   port to 32 bit.                              */
/*... 02/08/91  103   Dave      port to 32 bit.                              */
/*... 02/08/91  104                                                          */
/*... 02/08/91  105   Christina port to 32 bit.                              */
/*... 02/08/91  106   Srinivas  port to 32 bit.                              */
/*... 02/08/91  107   Dave      port to 32 bit.                              */
/*... 02/08/91  108   Dave      port to 32 bit.                              */
/*... 02/08/91  109                                                          */
/*... 02/08/91  110   Srinivas  port to 32 bit.                              */
/*... 02/08/91  111   Christina port to 32 bit.                              */
/*... 02/08/91  112   Joe       port to 32 bit.                              */
/*... 02/08/91  113                                                          */
/*... 02/08/91  114                                                          */
/*... 02/08/91  115   Srinivas  port to 32 bit.                              */
/*... 02/08/91  116   Joe       port to 32 bit.                              */
/*                                                                           */
/*...Release 1.00 (Pre-release 1)                                            */
/*...                                                                        */
/*... 08/06/91  221   srinivas  Users app being started in back ground       */
/*... 08/14/91  215   Christina   also adding support HLL format             */
/*...                                                                        */
/*...Release 1.00 (Pre-release 108 12/05/91)                                 */
/*...                                                                        */
/*... 02/12/92  521   Srinivas  Port to C-Set/2.                             */
/*... 02/13/92  529   Srinivas  Truncate big member names of structures in   */
/*...                           data window.                                 */
/*...Release 1.02 (10/22/92)                                                 */
/*...                                                                        */
/*... 03/03/93  813   Joe       Revised types handling for HL03.             */
/*... 12/20/93  912   Joe       Integer divide by zero trap on 0 len arrays. */
/*... 02/16/94  915   Joe       Fix for not editing UCHAR and CHAR arrays.   */
/**Includes*******************************************************************/

#include "all.h"                        /* SD86 include files                */

/**Defines *******************************************************************/

#define TAB1 15                         /* first tab pos for data display    */
#define TAB2 27                         /* second tab pos for data display   */

/**External declararions******************************************************/

extern uint       ExprAddr;             /* Address value of expr          101*/
                                        /* ( Set by ParseExpr )              */

/*****************************************************************************/
/* ShowStruct()                                                              */
/*                                                                           */
/* Description:                                                              */
/*   Show a data structure.                                                  */
/*                                                                           */
/* Parameters:                                                               */
/*   dfp       input - pointer to dfile node.                                */
/*   row       input - where to start the display.                           */
/*   rows      input - rows available for display.                           */
/*   skip      input - # of logical records to skip.                         */
/*                                                                           */
/* Return:                                                                   */
/*   TRUE                                                                    */
/*   FALSE                                                                   */
/*                                                                           */
/*****************************************************************************/
uint                                    /*                                521*/
ShowStruct(DFILE *dfp,uint row,uint rows,uint skip)
                                        /* -> dfile node containing stucture.*/
                                        /* screen row.                       */
                                        /* rows available for display.       */
                                        /* # of logical data recs to skip.   */
{                                       /*                                   */
 uint   item;                           /*                                   */
 uint   rr;                             /*                                   */
 uint   n;                              /*                                   */
 TD_STRUCT   *tp;                       /*                                813*/
 TD_TYPELIST *typelist;                 /*                                813*/
 Trec  *namelist;                       /*                                   */
 uint   Nitems;                         /*                                   */
 uint   mid;                            /*                                   */
 uint   sfx;                            /*                                   */
 uchar *cp;                             /*                                   */
 uint   baseaddr;                       /*                                112*/
 uchar  buffer[2*DATALINESPAN];         /*                                   */
 uint   memberoffs;                     /* offset of member within structure.*/
 uint   membertype;                     /* type of the structure member.     */
 USHORT MaxLen;                         /* length of max structure name813529*/
                                        /*                                   */
 mid = dfp->mid;                        /*                                   */
 sfx = dfp->sfx;                        /*                                   */
 baseaddr = dfp->baseaddr;              /*                                   */

/*****************************************************************************/
/* The first thing we want to do is get a pointer to the base type record.   */
/* If the showtype is <= 512, indicating a primitive type, then we will      */
/* get a back a NULL.                                                        */
/*****************************************************************************/
 tp = (TD_STRUCT*)QbasetypeRec(mid, dfp->showtype); /*                    813*/
 if(!tp ||                              /* cutting through the typedefs.     */
    tp->RecType != T_STRUCT             /*                                   */
   )                                    /* cutting through the typedefs.     */
  return(FALSE);

/*****************************************************************************/
/* Now, we are going to get the number of structure members and quit if we   */
/* were told to skip more than the number of structure members.              */
/*****************************************************************************/
 Nitems = tp->NumMembers;               /*                                813*/
 if( Nitems <= skip )
  return(FALSE);

/*****************************************************************************/
/* Now, get pointers to the type list and the name list.                     */
/*****************************************************************************/
 typelist = (TD_TYPELIST*)QtypeRec(mid, tp->TypeListIndex);             /*813*/
 namelist = QtypeRec(mid, tp->NameListIndex);                           /*813*/
                                        /*                                   */
 if( skip )                             /*                                   */
     dfp = NULL;                        /*                                   */
                                        /*                                   */
 for( rr=0, item=skip+1;                /*                                813*/
      (rr < rows) &&                    /*                                   */
      (item <= Nitems);                 /*                                   */
      ++rr, ++item                      /*                                   */
    )                                   /*                                   */
 {                                      /*                                   */
  cp = (UCHAR *)QNameList(namelist,NAME,item-1);   /* find a member name. 813*/

  if(!cp)                               /* if there is no name then go home. */
    break;                              /*                                   */
                                        /*                                   */
  InitDataBuffer( buffer,               /* initialize the data buffer, i.e.  */
                  DATALINESPAN,         /* clear it, put expression name in  */
                  dfp                   /* it, put attributes in it, and  .  */
                );                      /* and terminate it with \0.         */
  dfp = NULL;                           /* kill dfp so we won't write the    */
                                        /* expression name again.            */
  MaxLen = *(USHORT*)cp;                /* length of structure name    813529*/
  MaxLen =                              /* limit the length of the name   529*/
  (MaxLen < (TAB2-2)) ? MaxLen : (TAB2-2);  /* till TAB2 position         529*/
  n = sprintf( buffer + STGCOL+1,       /* put the member name in the buffer.*/
              "%.*s: ",                 /*                                521*/
              MaxLen,                   /*                                529*/
              cp+sizeof(USHORT)         /*                                813*/
            );                          /*                                   */
                                        /*                                   */
                                        /* compute the offset of this member */
                                        /* within the data structure.        */
  memberoffs = QNameList(namelist,MEMBEROFFSET,item-1);                 /*813*/

                                        /* find the type of this member.     */
  membertype = typelist->TypeIndex[item-1];                          /*813112*/
                                        /*                                   */
                                        /*                                   */
  *(buffer + STGCOL+1 + n) = ' ';       /* remove null inserted by buffmt    */
                                        /*                                   */
  if ( n <= TAB1 )  n = TAB1;           /* move to a tab position based      */
  else  n = TAB2;                       /* on length of member name          */
                                        /*                                   */
                                        /*                                   */
                                        /* now format this item in the buffer*/
  FormatDataItem( buffer + STGCOL+1 + n,/*  starting position for the data.  */
                  baseaddr + memberoffs,/*  address of the member to format. */
                  mid,                  /*  mid containing the data.         */
                  membertype,           /*  type of the member.              */
                  sfx                   /*  stack frame index if there is one*/
                );                      /*                                   */
                                        /*                                   */
  buffer[ DATALINESPAN-1 ] = 0;         /* terminate the data buffer...again.*/
  putrc( row + rr, 0, buffer );         /* now display the data line.        */
 }                                      /*                                   */
                                        /*                                   */
 if( rr < rows )                        /* now clear the remaining rows.     */
   ClrScr( row + rr,                    /*                                   */
           row + rows - 1,              /*                                   */
           vaStgExp                     /*                                   */
         );                             /*                                   */
 return( TRUE );                        /*                                   */
}                                       /*                                   */

uint                                    /*                                521*/
ZapStruct(DFILE *dfp,uint row,uint skip )
{
    TD_STRUCT *tp;                                                      /*813*/

    TD_TYPELIST *typelist;                                              /*813*/

    Trec *namelist;
    uint itemtype, mid = dfp->mid, sfx = dfp->sfx;
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
}

/*****************************************************************************/
/* ShowArray()                                                            112*/
/*                                                                        112*/
/* Description:                                                           112*/
/*   Show a array.                                                        112*/
/*                                                                        112*/
/* Parameters:                                                            112*/
/*   dfp       input - pointer to dfile node.                             112*/
/*   row       input - where to start the display.                        112*/
/*   rows      input - rows available for display.                        112*/
/*   skip      input - # of logical records to skip.                      112*/
/*                                                                        112*/
/* Return:                                                                112*/
/*   TRUE                                                                 112*/
/*   FALSE                                                                112*/
/*                                                                        112*/
/*****************************************************************************/
uint                                    /*                                521*/
ShowArray( DFILE *dfp, uint row, uint rows, uint skip )                 /*112*/
{                                                                       /**12*/
 TD_ARRAY  *tp;                         /*                                813*/
 uint   item;                           /* array item to display.         112*/
 uint   rr;                             /*                                112*/
 uint   n;                              /* gp integer.                    112*/
 uint   Nitems;                         /* number of array items.         112*/
 uint   atomtype;                       /* type of array items.           112*/
 uint   atomsize;                       /* size of an array item.         112*/
 uint   mid;                            /* module id for the array var.   112*/
 uint   sfx;                            /* stack frame index if auto.     112*/
 uint   baseaddr;                       /* user addr where array starts.  112*/
 uchar  buffer[2*DATALINESPAN];         /* display buffer for array item. 112*/
 uchar *cp;                             /* gp char ptr.                   112*/
 uint   addr;                           /* gp address.                    112*/
                                                                        /*112*/
 mid = dfp->mid;                                                        /*112*/
 sfx = dfp->sfx;                                                        /*112*/
 baseaddr = dfp->baseaddr;                                              /*112*/
                                                                        /*112*/
 /****************************************************************************/
 /* - get a ptr to the defining type record in $$type seg.                112*/
 /* - get the atomic type of the array elements.                          112*/
 /* - get the size of the atomic type.                                    112*/
 /* - get the number of array items.                                      112*/
 /* - handle any errors in the above steps by simply returning false.     112*/
 /*                                                                       112*/
 /****************************************************************************/
 tp = (TD_ARRAY*)QbasetypeRec(mid, dfp->showtype);                   /*813112*/
 if( ( tp == NULL ) || ( tp->RecType != T_ARRAY ) )                  /*813112*/
  return( FALSE );                                                      /*112*/
                                                                        /*112*/
 atomtype = tp->ElemType;                                            /*813112*/
 atomtype = HandleUserDefs(mid,atomtype);                               /*813*/
                                                                        /*512*/
 if( atomtype == 0 )                                                    /*112*/
  return( FALSE );                                                      /*112*/
                                                                        /*112*/
 atomsize = QtypeSize(mid, atomtype);                                   /*112*/
 if( atomsize == 0 )                                                    /*112*/
  return( FALSE );                                                      /*112*/
                                                                        /*112*/
 Nitems = tp->ByteSize/atomsize;                                     /*813112*/
 if( Nitems == 0 )                                                      /*912*/
   Nitems = 1;                                                          /*912*/
 if( Nitems <= skip )                                                   /*112*/
  return( FALSE );                                                      /*112*/
                                                                        /*112*/
 /************************************************************************112*/
 /* simply show character array bytes as a string.                        112*/
 /************************************************************************112*/
 if( atomtype == TYPE_CHAR || atomtype == TYPE_UCHAR )                  /*112*/
 {                                                                      /*112*/
  ShowHexBytes(dfp, row, rows, skip);                                   /*112*/
  return( TRUE );                                                       /*112*/
 }                                                                      /*112*/
                                                                        /*112*/
 /************************************************************************112*/
 /* -we only want to associate the array name with the first element of   112*/
 /*  the array. We set dfp == null after the first element, or before the 112*/
 /*  first element if we have scrolled the array name off the screen.     112*/
 /* -format each element.                                                 112*/
 /* -display in int and hex format.                                       112*/
 /* -truncate long lines.                                                 112*/
 /*                                                                       112*/
 /************************************************************************112*/
 if( skip )                                                             /*112*/
  dfp = NULL;                                                           /*112*/
                                                                        /*112*/
 for( rr=0, item=skip+1; (rr < rows) && (item <= Nitems); ++rr, ++item) /*112*/
 {                                                                      /*112*/
  InitDataBuffer( buffer, DATALINESPAN, dfp );                          /*112*/
  dfp = NULL;                                                           /*112*/
  n = sprintf( buffer + STGCOL+1, "[%02u] ", item-1 );                  /*521*/
                                                                        /*112*/
  *(buffer + STGCOL+1 + n) = ' ';                                       /*112*/
                                                                        /*112*/
  if ( n <= TAB1 )  n = TAB1;                                           /*112*/
  else  n = TAB2;                                                       /*112*/
                                                                        /*112*/
                                                                        /*112*/
  cp   = buffer + STGCOL + 1 + n;                                       /*112*/
  addr = baseaddr+atomsize*(item-1);                                    /*112*/
  FormatDataItem( cp , addr, mid, atomtype, sfx);                       /*112*/
  buffer[ DATALINESPAN-1 ] = 0;                                         /*112*/
  putrc( row + rr, 0, buffer );                                         /*112*/
 }                                                                      /*112*/
 if( rr < rows )                                                        /*112*/
  ClrScr( row + rr, row + rows - 1, vaStgExp );                         /*112*/
 return( TRUE );                                                        /*112*/
}                                       /* end ShowArray().             /*112*/

uint                                    /*                                521*/
ZapArray(DFILE *dfp,uint row,uint skip)
{
    TD_ARRAY *tp;                                                       /*813*/
    uint  col;                          /* was register.                  112*/
    uint Nitems, atomsize, mid = dfp->mid, sfx = dfp->sfx;
    uint   atomtype;                                                    /*101*/
    uchar  buffer[10], *baseaddr = (uchar*) ExprAddr;

    if( (tp = (TD_ARRAY*)QbasetypeRec(mid, dfp->showtype))
     && (tp->RecType == T_ARRAY)
     && (atomtype = tp->ElemType)                                       /*813*/
     && (atomsize = QtypeSize(mid, atomtype))
     && (Nitems   = tp->ByteSize / atomsize) > skip                     /*813*/
     ){
        /*****************************************************************915*/
        /* At this point, tp could be pointing to a T_TYPEDEF record      915*/
        /* that defines a primitive type. This is the case with UCHAR     915*/
        /* and CHAR typedefs. So, we have to get the "true" atomtype.     915*/
        /*****************************************************************915*/
        atomtype = HandleUserDefs(mid,atomtype);                        /*915*/

        if( atomtype == TYPE_CHAR || atomtype == TYPE_UCHAR ){
            if( Nitems <= (uint)16*skip )
                return( FALSE );
            return( ZapHexBytes(dfp, row, skip) );
        }
        col = sprintf( buffer, "[%02u] ", skip );                       /*521*/
        return( (* PickKeyer( mid, atomtype))
            (mid, atomtype, sfx, row, col, baseaddr + atomsize * skip) );
    }
    return( FALSE );
}


/* Note: The order of "keyers" MUST match the TGROUP type */


static UINTFUNC keyers[] =
{
 NULL,                                  /* TG_UNKNOWN                        */
 (UINTFUNC)KeyScalar,                   /* TG_SCALAR                         */
 (UINTFUNC)KeyPointer,                  /* TG_POINTER                        */
 NULL,                                  /* TG_STRUCT                         */
 NULL,                                  /* TG_ARRAY                          */
 NULL,                                  /* TG_ENUM                           */
 NULL                                   /* TG_BITFLD                         */
};

 UINTFUNC
PickKeyer( uint mid, uint tno )         /*                                101*/
{
    UINTFUNC f;

    return( (f = keyers[QtypeGroup(mid,tno)]) ? f : (UINTFUNC)FalseKeyer );
}

 uint
FalseKeyer( )
{
    return( FALSE );
}
