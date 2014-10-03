/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   zoomdata.c                                                              */
/*                                                                           */
/* Description:                                                              */
/*   expand data records in the data window.                                 */
/*                                                                           */
/*                                                                           */
/* History:                                                                  */
/*                                                                           */
/*   02/08/91 Creation of 32-bit SD86, from 16-bit version.                  */
/*                                                                           */
/*****************************************************************************/

#include "all.h"
static int iview=0;

extern uint        TopLine;
extern KEY2FUNC    defk2f[];
extern uint        DstatRow;
extern uint        VideoRows;
extern uint        TopLine;
extern uint        LinesPer;
extern uint        LinesPer;
extern UINT        FnameRow;

/*****************************************************************************/
/*  zoomrec()                                                                */
/*                                                                           */
/* Description:                                                              */
/*   expand a data window record within a dfile node.                        */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   fp        input - the afile for this dfile node.                        */
/*   dfp       input - -> to the dfile node.                                 */
/*   mem       input - the member within the dfile node.                     */
/*                     = 1 for primitive types                               */
/*                     = ? for complex types.                                */
/*                                                                           */
/* Return:                                                                   */
/*   0         all ok.                                                       */
/*   1         bad.                                                          */
/*   key = ESC escape key to caller.                                         */
/*   rc  = ESC to pass escape key up the recursive calling chain.            */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   The contents of the parent node located by dfp is assumed to be         */
/*   valid. This also assumes that for a stack variable that the scope       */
/*   is valid and that the sfx != 0. This implies "inactive".                */
/*                                                                           */
/*****************************************************************************/
uint zoomrec(AFILE *fp, DFILE *dfp, int mem, AFILE **fpp)
                                        /* -> afile structure for this node. */
                                        /* -> data file node containing mem. */
                                        /* dfile node member number or 1.    */
{                                       /*                                   */
 uint   memtypeno;                      /* member type number.               */
 uint   typeno;                         /* type number.                   101*/
 Trec  *tp;                             /* -> base rec for parent dfile node.*/
 Trec  *tpmem;                          /* -> type record for child member.  */
 TD_TYPELIST *typelist;                 /* -> typelist record.            813*/
 Trec  *namelist;                       /* -> namelist record.               */
 uint   mid;                            /* module id.                        */
 DFILE *memdfp;                         /* -> child dfile node.              */
 uchar *memname;                        /* -> to child member name.          */
 int    memnamelen;                     /* length of the child member name.  */
 uint   memberoffs;                     /* offset of child member in parent. */
 uint   rows;                           /* rows needed to display the child. */
 uint   atomtype;                       /* type of array element.         101*/
 uint   atomsize;                       /* size of array element in bits.    */
 uint   nbytes;                         /* # of bytes read by GetAppData().  */
 uchar *dp;                             /* -> to buf read from user space.101*/
 AFILE *newfp;

/*****************************************************************************/
/* We are going to build a child dfile node from the member within the       */
/* parent dfile node. The parent node is taken from the data window.         */
/* The first thing we do is establish those items that are inherited from    */
/* the parent.                                                               */
/*                                                                           */
/*****************************************************************************/
 mid = dfp->mid;                        /* module id.                        */
 memdfp=(DFILE*)Talloc(sizeof(DFILE));  /*clear the stack space for child.521*/
 memdfp->mid      = dfp->mid;           /* inherit: mid - module id.         */
 memdfp->lno      = dfp->lno;           /*          lno - source line number.*/
 memdfp->sfx      = dfp->sfx;           /*          sfx - stack frame index. */
 memdfp->scope    = dfp->scope;         /*          scope - symbol scope.    */
 memdfp->baseaddr = dfp->baseaddr;      /*                                   */
                                        /*                                   */
/*****************************************************************************/
/* The parent dfile record will contain a datatype and a showtype.           */
/* If the datatype is < 512, then the parent is primitive else the parent is */
/* complex.                                                                  */
/*                                                                           */
/*****************************************************************************/
 if(dfp->datatype < 512 ||              /* if parent is primitive, or if the */
    mem == 0 )                          /* child is to be an exact image of  */
                                        /* the parent, then                  */
 {                                      /*                                   */
  memcpy(memdfp, dfp,sizeof(DFILE));    /* inherit everything and go make 101*/
 }                                      /* new copy for possible reformat.   */
 else                                   /*                                   */
 {                                      /* parent is complex.                */
  tp = QbasetypeRec(mid, dfp->datatype);/* get -> to parent base type rec.   */
  if(!tp ) goto error;                  /* if this fails, then abort.        */
/*****************************************************************************/
/* At this point, we have a pointer to the base type record for the parent   */
/* dfile node. The complex types handled are as follows:                     */
/*                                                                           */
/*        T_PTR     yes                                                      */
/*        T_STRUCT  yes                                                      */
/*        T_ARRAY   yes                                                      */
/*        T_ENUM    no                                                       */
/*        T_BITFLD  no                                                       */
/*        T_TYPDEF  yes                                                      */
/*        T_PROC    not applicable                                           */
/*        T_LIST    not applicable                                           */
/*        T_NULL    not applicable                                           */
/*****************************************************************************/
  switch(tp->RecType)                   /* switch on complex type of prnt.813*/
  {                                     /*                                   */
   case T_PTR:
   case T_REF:
   /**************************************************************************/
   /* Get the type of the pointer and establish the datatype and             */
   /* showtype fields of the child node.                                     */
   /*                                                                        */
   /* Resolve the pointer.                                                   */
   /* Establish the number of lines needed for display.                      */
   /*                                                                        */
   /*                                                                        */
   /**************************************************************************/
   if(tp->RecType == T_PTR )
    typeno = ((TD_POINTER*)tp)->TypeIndex;
   else
    typeno = ((TD_REF*)tp)->TypeIndex;

    memdfp->datatype = typeno;
    memdfp->showtype = typeno;

    dp = GetAppData(memdfp->baseaddr,   /* resolve the pointer by getting    */
                    sizeof(uint),       /* the address of what the pointer101*/
                    &nbytes,            /* points to.                        */
                    memdfp->sfx         /*                                   */
                   );                   /*                                   */
    if(!dp) goto error;                 /* abort if ptr won't resolve.       */
    memdfp->baseaddr = *(uint*)dp;      /* establish base address of chld.101*/
                                        /* now get # lines needed for display*/
    memdfp->baseaddr = ResolveAddr(memdfp->baseaddr,dfp,dfp->datatype);
                                        /* resolve address into flat addr 219*/
    if(typeno < 512)                    /* if the pointer resolved to a      */
     break;                             /* primitive, then we're done.       */
    tpmem=QbasetypeRec(mid, typeno );   /* get -> to base type record.       */
                                        /*                                   */
    switch(tpmem->RecType)              /* select the appropriate method for */
    {                                   /* number of lines calculation.      */
     case T_CLASS:                      /* child is a structure.             */
     {
      USHORT  NameLen;
      char   *pName;

      memdfp->lines = ((TD_CLASS*)tpmem)->NumMembers;
      if( GetViewMemFncs() == FALSE )
      {
       memdfp->lines = GetLinesNoMemFncs( memdfp->mid, tpmem );
      }
      NameLen       = ((TD_CLASS*)tpmem)->NameLen;
      pName         = ((TD_CLASS*)tpmem)->Name;

      strncpy(memdfp->expr, pName, NameLen );

      memdfp->expr[NameLen]  = '\0';

     }
     break;

     case T_STRUCT:
     {
      int   NameLen;
      char *pName;

      memdfp->lines = ((TD_STRUCT*)tpmem)->NumMembers;
      NameLen       = ((TD_STRUCT*)tpmem)->NameLen;
      pName         = ((TD_STRUCT*)tpmem)->Name;

      strncpy(memdfp->expr, pName, NameLen );

      memdfp->expr[NameLen]  = '\0';
     }
     break;

     case T_ARRAY:                      /* child is an array.                */
                                        /*                                   */
      atomtype = ((TD_ARRAY*)tpmem)->ElemType;                          /*813*/
                                        /* get the type of array elements.   */
                                        /*                                   */
      atomsize = QtypeSize(mid,         /* get size of row in bytes.         */
                           atomtype);   /*                                   */
                                        /*                                   */
      nbytes = ((TD_ARRAY*)tpmem)->ByteSize;                         /*813520*/
      if(atomtype == TYPE_CHAR ||       /* if the type is char or         520*/
         atomtype == TYPE_UCHAR )       /* unsigned char then             520*/
      {                                 /* treat the array as a string.   520*/
        memdfp->lines = nbytes/16;      /* this is base number of lines.  520*/
        if( nbytes-memdfp->lines*16 > 0)/* if there is a vestigial, then  520*/
          memdfp->lines += 1;           /* add a line for it.             520*/
      }                                 /*                                520*/
      else                              /* if the type is NOT char, then  520*/
      {                                 /*                                520*/
        memdfp->lines = nbytes;         /*                                520*/
        memdfp->lines /= atomsize;      /* compute no of lines to display 520*/
      }                                 /*                                520*/
      break;                            /*                                   */
                                        /*                                   */
    }                                   /*                                   */
    break;                              /*                                   */
                                        /*                                   */
   case T_STRUCT:                       /*                                   */
   /**************************************************************************/
   /* Get the typelist of the parent dfile node.                             */
   /* Get the namelist of the parent dfile node.                             */
   /* Compute the base address for the child node.                           */
   /* Get a pointer to the base type record for the child.                   */
   /* If the child type is a pointer, then resolve the pointer and type.     */
   /* Establish the datatype and showtype of the child.                      */
   /* Add the child member name.                                             */
   /* Compute child display lines.                                           */
   /*                                                                        */
   /**************************************************************************/
    typeno = ((TD_STRUCT*)tp)->TypeListIndex;                           /*813*/

    typelist = (TD_TYPELIST*)QtypeRec(mid, typeno); /* -> to typelist.    813*/
                                        /*                                   */
    typeno = ((TD_STRUCT*)tp)->NameListIndex;                           /*813*/
                                        /*                                   */
    namelist = QtypeRec(mid, typeno);   /* establish -> to namelist.         */
                                        /*                                   */
    memtypeno = typelist->TypeIndex[mem-1];                             /*813*/

    if( memtypeno == 0 ) goto error;    /* abort if ptr won't resolve.    405*/

    memberoffs = QNameList(namelist,MEMBEROFFSET, mem-1 );              /*813*/

    memdfp->baseaddr = dfp->baseaddr +  /* compute base address of child.    */
                      memberoffs;       /*                                   */
                                        /*                                   */
    tpmem = QbasetypeRec(mid,memtypeno);/* get -> to child type.             */
    if( tpmem &&                        /*                                   */
       tpmem->RecType == T_PTR          /* if the child is a pointer, then   */
      )                                 /* resolve base address and type.    */
    {                                   /*                                   */
     dp = GetAppData(memdfp->baseaddr,  /* resolve the base address.         */
                     sizeof(uint),      /*                                101*/
                     &nbytes,           /*                                   */
                     memdfp->sfx        /*                                   */
                    );                  /*                                   */
     if(!dp) goto error;                /* abort if ptr won't resolve.       */
     memdfp->baseaddr = *(uint*)dp;     /* update child base address.        */
     memdfp->baseaddr = ResolveAddr(memdfp->baseaddr,dfp,memtypeno);

                                        /* resolve the child type number.    */
     memtypeno = ((TD_POINTER*)tpmem)->TypeIndex;                       /*813*/
    }                                   /*                                   */
    memdfp->datatype = memtypeno;       /* establish datatype and showtype   */
    memdfp->showtype = memtypeno;       /* of the child.                     */
                                        /*                                   */
                                        /* get the member name from namelist.*/
    memname=(uchar *)QNameList(namelist, NAME, mem-1);                  /*813*/

    memnamelen = (int)*memname;         /* compute member name length.       */
    memname += 2;                       /*                                907*/
    strncpy(memdfp->expr,               /* put the name in the child node.   */
            memname,                    /*                                   */
            memnamelen                  /*                                   */
           );                           /*                                   */
    memdfp->expr[memnamelen] = '\0';    /* terminate the name.               */
                                        /*                                   */
                                        /* now, compute lines for display.   */
    memdfp->lines = 1;                  /* assume 1.                         */
    if(memdfp->datatype >= 512 )        /* if child is a complex data type,  */
    {                                   /* then compute real number required.*/
     tpmem=QbasetypeRec(mid,            /* get -> to base rec for child.     */
                       memdfp->datatype);/*                                  */
     switch(tpmem->RecType)             /* select the appropriate method for */
     {                                  /* number of lines calculation.      */
      case T_STRUCT:                    /* child is a structure.             */



       /**********************************************************************/
       /* - put in a check for a structure with a reference to an            */
       /*   undefined structure.                                             */
       /**********************************************************************/
       typeno = ((TD_STRUCT*)tpmem)->TypeListIndex;                     /*908*/
       if(typeno == 0 )                                                 /*908*/
       {                                                                /*908*/
        Message(ERR_DATA_INVALID_STRUCT,TRUE,1,memdfp->expr);           /*908*/
        return(0);                                                      /*908*/
       }                                                                /*908*/

       memdfp->lines = ((TD_STRUCT*)tpmem)->NumMembers;                 /*813*/
                                        /*                                   */
       break;                           /*                                   */
                                        /*                                   */
      case T_ARRAY:                     /* child is an array.                */
       atomtype = ((TD_ARRAY*)tpmem)->ElemType;                         /*813*/
       atomsize = QtypeSize(mid, atomtype);
       nbytes = ((TD_ARRAY*)tpmem)->ByteSize;                        /*813520*/
       if(atomtype == TYPE_CHAR ||      /* if the type is char or         520*/
          atomtype == TYPE_UCHAR )      /* unsigned char then             520*/
       {                                /* treat the array as a string.   520*/
         memdfp->lines = nbytes/16;     /* this is base number of lines.  520*/
         if(nbytes-memdfp->lines*16 > 0)/* if there is a vestigial, then  520*/
           memdfp->lines += 1;          /* add a line for it.             520*/
       }                                /*                                520*/
       else                             /* if the type is NOT char, then  520*/
       {                                /*                                520*/
         if(nbytes == 0 )                                               /*912*/
          nbytes = atomsize;                                            /*912*/
         memdfp->lines = nbytes;        /*                                520*/
         memdfp->lines /= atomsize;     /* compute no of lines to display 520*/
       }                                /*                                520*/
       break;                           /*                                   */
     }                                  /*                                   */
    }                                   /* end case T_STRUCT                 */
    break;                              /*                                   */

   case T_CLASS:                        /*                                   */
   {
    USHORT       ItemListIndex;
    TD_TYPELIST *pClassItemList;
    ULONG        memberoffs;
    ULONG        membertype;
    TD_CLASS    *pClass;

    pClass           = (TD_CLASS*)tp;
    ItemListIndex    = pClass->ItemListIndex;

    if(ItemListIndex == 0 )
     return(1);

    pClassItemList   = (TD_TYPELIST*)QtypeRec(mid, ItemListIndex);
    memberoffs       = QClassItemList(mid, pClassItemList, MEMBEROFFSET, mem-1);
    membertype       = QClassItemList(mid, pClassItemList, MEMBERTYPE,   mem-1);

    if(membertype == 0 )
     return(1);

    memdfp->baseaddr = dfp->baseaddr + memberoffs;
    tpmem            = QbasetypeRec(mid, membertype);
    membertype       = Qbasetype(mid, membertype);
    memname          = (UCHAR*)QClassItemList(mid, pClassItemList, NAME, mem-1);

    if(memname != NULL )
    {
     memnamelen = (int)*memname;
     memname += 2;
     strncpy(memdfp->expr, memname, memnamelen );
     memdfp->expr[memnamelen] = '\0';
    }

    if( tpmem && (tpmem->RecType == T_PTR) )
    {
     dp = GetAppData(memdfp->baseaddr, sizeof(uint), &nbytes, memdfp->sfx );
     if(!dp)
      goto error;
     memdfp->baseaddr = *(uint*)dp;
     memdfp->baseaddr = ResolveAddr(memdfp->baseaddr, dfp, membertype);

     membertype = ((TD_POINTER*)tpmem)->TypeIndex;
    }
    memdfp->datatype = membertype;
    memdfp->showtype = membertype;

    memdfp->lines = 1;
    if(memdfp->datatype >= 512 )
    {
     tpmem=QtypeRec(mid, memdfp->datatype);
     switch(tpmem->RecType)
     {
      case T_MEMFNC:
      {
       AFILE     *fp;
       TD_MEMFNC *pMemFnc;
       char       buffer[256];
       char      *cp;
       char      *cpstart;
       char      *cpend;
       char      *cpp;

       memset( buffer, 0, sizeof(buffer) );

       pMemFnc  = (TD_MEMFNC*)tpmem;
       cpstart  = pMemFnc->Name;
       cpend    = cpstart + pMemFnc->NameLen - 1;
       cpp      = buffer;
       cp       = strstr(pMemFnc->Name, "__");
       cp      += 2;

       /**********************************************************************/
       /* - put in the first part of the public name.                        */
       /**********************************************************************/
       switch(pMemFnc->FuncType)
       {
        case FUNCTYPE_REGULAR:
         strncpy(cpp, cpstart, cp - cpstart );
         break;

        case FUNCTYPE_CTOR:
         strcpy(cpp, "__ct__");
         break;

        case FUNCTYPE_DTOR:
         strcpy(cpp, "__dt__");
         break;
       }
       cpp += strlen(cpp);

       /**********************************************************************/
       /* - put in the class name length.                                    */
       /**********************************************************************/
       sprintf(cpp, "%d", pClass->NameLen);
       cpp += strlen(cpp);

       /**********************************************************************/
       /* - put in the class name.                                           */
       /**********************************************************************/
       strncpy(cpp, pClass->Name, pClass->NameLen);
       cpp += strlen(cpp);

       /**********************************************************************/
       /* - put in the suffix.                                               */
       /* - it it's a const, throw in a C.                                   */
       /**********************************************************************/
       if( pMemFnc->TypeQual & FUNCQUAL_CONST )
        *cpp++ = 'C';

       strncpy( cpp, cp, cpend - cp + 1 );

       fp = FindFuncOrAddr(buffer, FALSE);
       if( fp )
       {
        fp->csr.col = 0;
        *fpp = fp;
        return(0);
       }
       else
        return(1);
      }
//    break;

      case T_CLASS:
      {
       TD_TYPELIST *pClassItemList;
       USHORT       FirstItemInList;
       USHORT       ItemListIndex;
       TD_CLASS    *pClass;

       pClass        = (TD_CLASS*)tpmem;
       memdfp->lines = pClass->NumMembers;
       if( GetViewMemFncs() == FALSE )
       {
        memdfp->lines = GetLinesNoMemFncs( memdfp->mid, tpmem );
       }

       strncpy(memdfp->expr, pClass->Name, pClass->NameLen );
       memdfp->expr[pClass->NameLen] = '\0';

       /**********************************************************************/
       /* - set a flag for base/derived class.                               */
       /* - at map time, an extra item was added to the list for             */
       /*   base classes.                                                    */
       /**********************************************************************/
       ItemListIndex    = pClass->ItemListIndex;

       if(ItemListIndex == 0 )
        return(1);

       pClassItemList   = (TD_TYPELIST*)QtypeRec(mid, ItemListIndex);
       FirstItemInList  = pClassItemList->TypeIndex[0];

       if( FirstItemInList == 0 )
        memdfp->DfpFlags.ClassType = DFP_BASE_CLASS;
       else
        memdfp->DfpFlags.ClassType = DFP_DERIVED_CLASS;

       /**********************************************************************/
       /* - put in a check for a class with a reference to an                */
       /*   undefined class.                                                 */
       /**********************************************************************/
       typeno = ((TD_CLASS*)tpmem)->ItemListIndex;
       if(typeno == 0 )
       {
        Message(ERR_DATA_INVALID_STRUCT, TRUE, 1, memdfp->expr);
        return(0);
       }
      }
      break;

      case T_ARRAY:
      {
       atomtype = ((TD_ARRAY*)tpmem)->ElemType;
       atomsize = QtypeSize(mid, atomtype);
       nbytes = ((TD_ARRAY*)tpmem)->ByteSize;
       if(atomtype == TYPE_CHAR ||
          atomtype == TYPE_UCHAR )
       {
         memdfp->lines = nbytes/16;
         if(nbytes-memdfp->lines*16 > 0)
           memdfp->lines += 1;
       }
       else
       {
         if(nbytes == 0 )
          nbytes = atomsize;
         memdfp->lines = nbytes;
         memdfp->lines /= atomsize;
       }
      }
      break;
     }
    }
   }
   break;


   case T_ARRAY:
   /**************************************************************************/
   /* get the atom type from the base type record.                           */
   /* get the atom size.                                                     */
   /* get the base address of this member of the array.                      */
   /* get the pointer to the base type record for this member.               */
   /* if this member is a pointer then resolve it:                           */
   /*   get new type.                                                        */
   /*   get new base address.                                                */
   /* set the data type and showtype.                                        */
   /* compute the number of lines needed for show.                           */
   /*                                                                        */
   /**************************************************************************/
    atomtype = ((TD_ARRAY*)tp)->ElemType;   /* get array element type.    813*/
    atomsize = QtypeSize(mid, atomtype);/* get array row size in bytes.      */
                                        /*                                   */
    memdfp->baseaddr=dfp->baseaddr +    /* establish child base address.     */
                       (mem-1)*atomsize;/*                                   */
                                        /*                                   */
    tpmem = QbasetypeRec(mid,atomtype); /* get -> to child type.             */
                                        /*                                   */
    if( tpmem &&                        /* if child is a pointer, then       */
       tpmem->RecType == T_PTR          /* resolve base address and type.    */
      )                                 /*                                   */
    {                                   /*                                   */
                                        /*                                   */
     dp = GetAppData(memdfp->baseaddr,  /* resolve base address.             */
                     sizeof(uint),      /*                                101*/
                     &nbytes,           /*                                   */
                     memdfp->sfx        /*                                   */
                    );                  /*                                   */
     if(!dp) goto error;                /* abort if ptr won't resolve.       */
     memdfp->baseaddr = *(uint*)dp;     /* establish resolved child base addr*/
     memdfp->baseaddr = ResolveAddr(memdfp->baseaddr,dfp,atomtype);
                                        /* resolve address into flat addr 219*/
     atomtype = ((TD_POINTER*)tpmem)->TypeIndex;/* get type of array elmnt813*/
    }                                   /*                                   */
                                        /*                                   */
    memdfp->datatype = atomtype;        /*                                   */
    memdfp->showtype = atomtype;        /*                                   */
                                        /*                                   */
    memdfp->lines = 1;                  /*                                   */
    if(memdfp->datatype >= 512 )        /* if child is a complex data type,  */
    {                                   /* then compute real number required.*/
     tpmem=QbasetypeRec(mid,            /* get -> to base rec for child.     */
                       memdfp->datatype);/*                                  */
     switch(tpmem->RecType)             /* select the appropriate method for */
     {                                  /* number of lines calculation.      */
      case T_STRUCT:
       memdfp->lines = ((TD_STRUCT*)tpmem)->NumMembers;
       break;

      case T_CLASS:
       memdfp->lines = ((TD_CLASS*)tpmem)->NumMembers;
       if( GetViewMemFncs() == FALSE )
       {
        memdfp->lines = GetLinesNoMemFncs( memdfp->mid, tpmem );
       }
       break;

      case T_ARRAY:
       atomtype = ((TD_ARRAY*)tpmem)->ElemType;
       atomsize = QtypeSize(mid, atomtype);

       nbytes = ((TD_ARRAY*)tpmem)->ByteSize;                        /*813520*/
       if(atomtype == TYPE_CHAR ||      /* if the type is char or         520*/
          atomtype == TYPE_UCHAR )      /* unsigned char then             520*/
       {                                /* treat the array as a string.   520*/
         memdfp->lines = nbytes/16;     /* this is base number of lines.  520*/
         if(nbytes-memdfp->lines*16 > 0)/* if there is a vestigial, then  520*/
           memdfp->lines += 1;          /* add a line for it.             520*/
       }                                /*                                520*/
       else                             /* if the type is NOT char, then  520*/
       {                                /*                                520*/
         memdfp->lines = nbytes;        /*                                520*/
         memdfp->lines /= atomsize;     /* compute no of lines to display 520*/
       }                                /*                                520*/
       break;                           /*                                   */
     }                                  /*                                   */
    }                                   /* end compute # lines for cmplx type*/
    break;                              /* end case T_ARRAY.                 */

   case T_ENUM:
   case T_BITFLD:
   case T_TYPDEF:
   case T_PROC:
   case T_LIST:
   case T_NULL:
    break;
  }                                     /* end of switch                     */
 }                                      /* end of else - handle complex types*/
/*****************************************************************************/
/* At this point, we have finally built the child node. We are going to      */
/* display the info and then handle user input.                              */
/*****************************************************************************/
 {                                      /* begin k/b handling.               */
  int    func;                          /* function associated with a key.   */
  int    cursormem;                     /* cursor dfile node member.         */
  int    firstmem;                      /* first member displayed in window. */
  int    lastmem;                       /* last  member displayed in window. */
  CSR    stgcsr;                        /* storage cursor.                   */
  int    show;                          /* show flag.                        */
  int    toobig;                        /* data too big to fit in window.    */
  uint   rc;                            /* return code.                      */
  int    ShowSource;                    /* show source flag.              519*/

  stgcsr.col = 0;                       /* initialize the cursor.            */
  stgcsr.row = (uchar)(TopLine + 1);    /* column 0 & 1st row in window.     */
  stgcsr.mode = CSR_NORMAL;             /* use a normal cursor.              */
                                        /*                                   */
  rows = VideoRows-4-TopLine;           /* number of rows in exp window.  519*/
                                        /*                                   */
  toobig = FALSE;                       /* assume the node will fit in the   */
  if( memdfp->lines > rows )            /* window. If it doesn't, then set   */
   toobig = TRUE;                       /* a flag.                           */
                                        /*                                   */
                                        /* initialize the entry display.     */
  cursormem = 1;                        /* cursor on first member.           */
  firstmem  = 1;                        /*                                   */
  lastmem   = memdfp->lines;            /* last member = last member in node */
  if( toobig )                          /* unless node is too big to fit the */
   lastmem = rows;                      /* window.                           */
  else                                  /*                                   */
   rows = lastmem - firstmem + 1;       /* limit to the size of the struct519*/
  if (rows == 0)                        /* if the number of rows is zero  521*/
     rows = 1;                          /* display at least oneline.      521*/
  memdfp->datatype = HandleUserDefs(memdfp->mid,memdfp->datatype);      /*512*/
                                        /* Get primitive typeno in case   512*/
                                        /* of primitive user defs.        512*/
  SetShowType( memdfp,memdfp->datatype);/* set show function for child node. */
  show = TRUE;                          /* set show flag for loop entry.     */
  ShowSource = (iview!=VIEW_SHOW);                 /* set show source flag           519*/
  SetMenuMask( EXPANDDATA );                                            /*701*/
  for( ;; )                             /* begin user processing loop.       */
  {                                     /*                                   */
   if(show)                             /* redisplay the window if the show  */
   {                                    /* flag is set.                      */
    /*************************************************************************/
    /* Is show source flag is set then depending on the shower of the fp  519*/
    /* call functions to refresh part of the screen.                      519*/
    /*************************************************************************/
    if (ShowSource == TRUE)                                             /*519*/
    {                                                                   /*519*/
     int  StartRow, RowsToPaint;                                        /*801*/
                                                                        /*801*/
     if( fp->shower == showC )                                          /*701*/
     {                                                                  /*801*/
       StartRow    = TopLine + rows + 1;
       RowsToPaint = LinesPer - rows - 1;
       if( (StartRow + fp->topline) < fp->Tlines )                      /*801*/
       {                                                                /*801*/
         RefreshSrcPart( fp, StartRow, RowsToPaint, rows+1);            /*519*/
       }                                                                /*801*/
       else
        ClrScr( StartRow, StartRow + RowsToPaint - 1, vaProgram );
     }
     else
       RefreshAsmPart( TopLine + rows + 1, rows + 1);
     ShowSource = FALSE;
    }
    (* memdfp->shower)(memdfp,          /* show the child node.              */
                       TopLine+1,       /*                                   */
                       rows,            /*                                   */
                       firstmem - 1     /*                                   */
                      );                /*                                   */
    fmtdwinstat(DstatRow, TopLine);     /* refresh the status line.          */
    show = FALSE;                       /* turn off the show flag.           */
   }                                    /*                                   */
   PutCsr( &stgcsr );                   /* put the cursor in the window.     */

#define RETURNESC 1                                                     /*701*/
   func = GetFuncsFromEvents( RETURNESC, (void *)fp );                  /*701*/
   switch( func )                       /* switch on function selection.     */
   {                                    /*                                   */
    case UPCURSOR:                      /* begin UPCURSOR.                   */
     if(cursormem > firstmem)           /*                                   */
     {                                  /* decrement the member.             */
      cursormem--;                      /* decrement the cursor.             */
      stgcsr.row -= 1;                  /*                                   */
      break;                            /*                                   */
     }                                  /*                                   */
     switch( toobig )                   /* if the node fits inside the window*/
     {                                  /* then we're done.                  */
      case FALSE:                       /*                                   */
       break;                           /*                                   */
                                        /*                                   */
      case TRUE:                        /* if it doesn't then                */
       if( firstmem > 1 )               /* if we're trying to go above the   */
       {                                /* the window, then we need to move  */
        firstmem--;                     /* the window up. if we're at the    */
        cursormem--;                    /* top of the window( firstmem=1)    */
        lastmem  = firstmem + rows - 1; /* then don't do anything.           */
        show = TRUE;                    /*                                   */
       }                                /*                                   */
       break;                           /*                                   */
     }                                  /*                                   */
     break;                             /* end UPCURSOR.                     */
                                        /*                                   */
    case DOWNCURSOR:                    /* begin DOWNCURSOR.                 */
     if(cursormem < lastmem )           /*                                   */
     {                                  /*                                   */
      cursormem++;                      /* increment the cursor member.      */
      stgcsr.row += 1;                  /* increment the cursor.             */
      break;                            /*                                   */
     }                                  /*                                   */
                                        /*                                   */
     switch( toobig )                   /* if the node fits in the window    */
     {                                  /* then we're done.                  */
      case FALSE:                       /*                                   */
       break;                           /*                                   */
                                        /*                                   */
      case TRUE:                        /* if it doesn't we come here.       */
       if( lastmem < (int)memdfp->lines)/*                                   */
       {                                /* we now drag the window down one   */
        lastmem++;                      /* line unless we're already bottomed*/
        cursormem++;                    /* out(lastmem = memdfp->lines).     */
        firstmem = lastmem - rows + 1;  /*                                   */
        show = TRUE;                    /*                                   */
       }                                /*                                   */
       break;                           /*                                   */
     }                                  /*                                   */
     break;                             /* end of DOWNCURSOR.                */
                                        /*                                   */
    case LEFTCURSOR:                    /* LEFTCURSOR.                       */
     Tfree((void*)memdfp);               /*  give memory back to OS/2.     521*/
     return(0);                         /*                                   */
                                        /*                                   */
    case LEFTMOUSECLICK:                                                /*701*/
    case RIGHTMOUSECLICK:                                               /*701*/
    {                                                                   /*701*/
      PEVENT Event;                                                     /*701*/
                                                                        /*701*/
      Event = GetCurrentEvent();                                        /*701*/
      if( (Event->Row <= TopLine) || (Event->Row > TopLine + rows) )    /*701*/
      {                                                                 /*701*/
        if( func == RIGHTMOUSECLICK )                                   /*701*/
          return( ESC );                                                /*701*/
        beep();                                                         /*701*/
        break;                                                          /*701*/
      }                                                                 /*701*/
      cursormem  = Event->Row - TopLine;                                /*701*/
      stgcsr.row = Event->Row;                                          /*701*/
      if( func == LEFTMOUSECLICK )                                      /*701*/
        break;                                                          /*701*/
    }                                                                   /*701*/
    goto caseexpandvar;
caseexpandvar:

    case EXPANDVAR:                     /* Expand variable in expansion win. */
    case RIGHTCURSOR:                   /* RIGHTCURSOT.                      */
     if(memdfp->datatype < 512)         /* if the data type is primitive  516*/
     {                                  /* then there is no point in      516*/
      beep();                           /* expanding it again.            516*/
      putmsg("Nothing more to expand"); /*                                516*/
      break;                            /*                                516*/
     }                                  /*                                516*/
     newfp = NULL;
     rc=zoomrec(fp, memdfp, cursormem, &newfp);
     if( newfp )
     {
      *fpp = newfp;
      return(0);
     }
     if(rc==ESC)
      return(rc);
     else if(rc)
     {
      beep();
      putmsg("can't expand this data"); /*                                   */
     }                                  /*                                   */
     show = TRUE;                       /* set the refresh flag.             */
     ShowSource = TRUE;                 /* set show source flag           519*/
     break;                             /*                                   */
                                        /*                                   */
    case PREVWINDOW:                    /* begin PREVWINDOW, alias PgUp.     */
     if(!toobig)                        /* if node fits in window, then fall */
      goto caseTopOfWindow;             /* thru to TOPOFWINDOW, alias C-PgUp.*/
     lastmem = firstmem;                /* otherwise, drag the window up one */
     firstmem = lastmem - rows + 1;     /* line unless already topped out.   */
     if( firstmem < 1 )                 /*                                   */
     {                                  /*                                   */
      firstmem = 1;                     /*                                   */
      lastmem = firstmem + rows - 1;    /*                                   */
     }                                  /*                                   */
     show = TRUE;                       /* set the refresh flag.             */
     cursormem = firstmem;              /* put the cursor on the 1st member. */
     stgcsr.row = (uchar)(TopLine + 1); /* set curosr on top row.            */
     break;                             /* end PREVWINDOW, alias PgUp.       */
                                        /*                                   */
    case NEXTWINDOW:                    /* begin NEXTWINDOW, alias PgDn.     */
     if(!toobig)                        /* if node fits in window fall thru  */
      goto caseBotOfWindow;             /* to BOTOFWINDOW, alias C-PgDn.     */
     firstmem = lastmem;                /* otherwise, move down one page.    */
     lastmem = firstmem + rows - 1;     /* anchor last page at bottom of the */
     if( lastmem > (int)memdfp->lines ) /* node.                             */
     {                                  /*                                   */
      lastmem = memdfp->lines;          /*                                   */
      firstmem = lastmem - rows + 1;    /*                                   */
     }                                  /*                                   */
     show = TRUE;                       /* set the refresh flag.             */
     cursormem = firstmem;              /* put the cursor on the 1st member. */
     stgcsr.row = (uchar)(TopLine + 1); /* set curosr on top row.            */
     break;                             /* end NEXTWINDOW, alias PgDn.       */
                                        /*                                   */
    case FIRSTWINDOW:                   /* begin FIRSTWINDOW, alias C-Home.  */
     if(!toobig)                        /* if node fits in window, then fall */
      goto caseTopOfWindow;             /* thru to TOPOFWINDOW.              */
     stgcsr.row = (uchar)(TopLine + 1); /* otherwise, reset to the first page*/
     cursormem = 1;                     /*                                   */
     firstmem  = 1;                     /*                                   */
     lastmem = rows;                    /*                                   */
     show = TRUE;                       /* set the refresh flag.             */
     break;                             /*                                   */
                                        /*                                   */
    caseTopOfWindow:                    /*                                   */
    case TOPOFWINDOW:                   /* begin TOPOFWINDOW, alias C-PgUp.  */
     cursormem = firstmem;              /* set node to first node member.    */
     stgcsr.row = (uchar)(TopLine + 1); /* set curosr on top row.            */
     break;                             /* end TOPOFWINDOW, alias C-PgUp.    */
                                        /*                                   */
    case LASTWINDOW:                    /* begin LASTWINDOW, alias C-end.    */
     if(!toobig)                        /* if node fits in window, then      */
      goto caseBotOfWindow;             /* fall thru to BOTOFWINDOW.         */
     lastmem = memdfp->lines;           /* otherwise, move to the window to  */
     firstmem = lastmem - rows + 1;     /* the bottom of the node.           */
     stgcsr.row=(uchar)(TopLine + rows);/*                                   */
     cursormem = lastmem;               /*                                   */
     show = TRUE;                       /* set the refresh flag.             */
     break;                             /* end LASTWINDOW, alias C-end.      */
                                        /*                                   */
    caseBotOfWindow:                    /*                                   */
    case BOTOFWINDOW:                   /* begin BOTOFWINDOW, alias C-PgDn.  */
     stgcsr.row += (uchar)(lastmem -    /* put cursor on last node member.   */
                           cursormem);  /*                                   */
     cursormem = lastmem;               /* set the cursor member.            */
     break;                             /* end BOTOFWINDOW, alias C-PgDn.    */
                                        /*                                   */
    case QUIT:                                                          /*701*/
    case ESCAPE:                        /*                                   */
      return( ESC );                    /*                                   */
   }                                    /* end switch( func ).               */
  }                                     /* end user processing loop.         */
 }                                      /* end of k/b handling.              */
/*****************************************************************************/
/* error handling                                                            */
/*****************************************************************************/
error:
 if(memdfp)
  Tfree((void*)memdfp);
 return(1);

}
