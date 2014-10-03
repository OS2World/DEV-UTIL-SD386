/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY813*/
/*   maptypes.c                                                              */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Mapping funtions for types from other formats to the internal format.   */
/*                                                                           */
/* History:                                                                  */
/*...                                                                        */
/*... 03/01/93  813   Selwyn    Changes for HL02/HL03.                       */
/*****************************************************************************/
#include "all.h"

/*****************************************************************************/
/* MapHLLTypes()                                                          813*/
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Map the HLL types records into the Internal Types format.               */
/*                                                                           */
/* (Unlike symbols and line numbers mapping functions, the internal types    */
/*  table is allocated within this function as the size of the internal      */
/*  table is difficult to find upfront because of the presence of various    */
/*  variable length fields)                                                  */
/*                                                                           */
/* Parameters:                                                               */
/*   RawTable       (input)  - Pointer to the raw types table.               */
/*   mptr           (input)  - Pointer to the MODULE structure.              */
/*                                                                           */
/* Return:                                                                   */
/*   None.                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/* - Raw symbol table has already been loaded into the memory.               */
/* - It is the responsibility of this function to allocate memory for the    */
/*   Internal types table and return a pointer to the table after mapping.   */
/* - This function would free the memory allocated for the Raw Table.        */
/*****************************************************************************/
#define CHUNKSIZE   0XFFFF
#define HWMARKSIZE  0X1000
uchar  *MapHLLTypes( uchar *RawTable, MODULE *pModule )
{
  Trec   *HLLptr=NULL;
  Trec   *Mapptr=NULL;
  uint   IntRecLength = 0;
  uint   HLLRecLength = 0;
  uint   IntTableLength = 0;
  uint   RecIDLen = 0;
  uchar  *IntTypeTable = NULL;
  uchar  *EndOfTable = NULL;
  static uint  UpperLimit;
  static uchar *Scratch = NULL;


  USHORT      *pNumMembers;
  BOOL         IsMappingClass;
  TD_TYPELIST *pListRec;
  ULONG        IntBufSize;

  /***************************************************************************/
  /* The Raw types table is first mapped to a scratch area and then copied to*/
  /* the internal table. This is done because the size of the internal table */
  /* is difficult to be determined upfront. A scratch area of 64k bytes is   */
  /* allocated at first and is extended whenever needed.                     */
  /***************************************************************************/
  if( Scratch != NULL )
   Tfree(Scratch);

  IntBufSize = CHUNKSIZE;
  UpperLimit = CHUNKSIZE - HWMARKSIZE;
  Scratch    = Talloc( IntBufSize );
  /***************************************************************************/
  /* Adjust for skipping the "01" at the start of each record in all formats */
  /* except HLL Level 3.                                                     */
  /***************************************************************************/
  RecIDLen = 1;
  if( (pModule->DbgFormatFlags.Typs == TYPE104_HL03) ||
      (pModule->DbgFormatFlags.Typs == TYPE104_HL04)
    )
    RecIDLen = 0;

  /***************************************************************************/
  /* Scan through from start to end of the raw table. Depending on the type  */
  /* of the record, copy the information accordingly to the internal types   */
  /* table.                                                                  */
  /***************************************************************************/
  HLLptr         = (Trec *)(RawTable + RecIDLen);
  Mapptr         = (Trec *)Scratch;
  EndOfTable     = RawTable + pModule->TypeLen;
  pNumMembers    = NULL;
  pListRec       = NULL;
  IsMappingClass = FALSE;

  while( ((uchar *)HLLptr) < EndOfTable )
  {
    IntRecLength = HLLRecLength = 0;
    switch( HLLptr->RecType )
    {
      case T_STRUCT:                    /* Structure record.          (0x79) */
      {
        TD_STRUCT *pIntStruct = (TD_STRUCT *)Mapptr;
        HL_STRUCT *pHLLStruct = (HL_STRUCT *)((uchar *)HLLptr + 3);

        /*********************************************************************/
        /* The Type qualifier field is not mapped.                           */
        /*********************************************************************/
        IntRecLength = HLLptr->RecLen - 3;
        HLLRecLength = HLLptr->RecLen;

        pIntStruct->RecLen        = IntRecLength;
        pIntStruct->RecType       = HLLptr->RecType;
        pIntStruct->ByteSize      = pHLLStruct->ByteSize;
        pIntStruct->NumMembers    = pHLLStruct->NumMembers;
        pIntStruct->TypeListIndex = pHLLStruct->TypeListIndex;
        pIntStruct->NameListIndex = pHLLStruct->NameListIndex;
        pIntStruct->NameLen       = pHLLStruct->NameLen;
        strncpy( pIntStruct->Name, pHLLStruct->Name, pHLLStruct->NameLen );

        Mapptr = (Trec *)((uchar *)Mapptr + IntRecLength + 2);
        HLLptr = (Trec *)((uchar *)HLLptr + HLLRecLength + 2 + RecIDLen);
        IntTableLength += (IntRecLength + 2);
      }
      break;

      case T_CLASS:
      {
       int       NumBytes;
       USHORT    EncLength;
       TD_CLASS *pIntClass;
       HL_CLASS *pHLLClass;

       pIntClass    = (TD_CLASS *)Mapptr;
       pHLLClass    = (HL_CLASS *)((uchar *)HLLptr + 3);
       HLLRecLength = HLLptr->RecLen;
       EncLength    = GetEncLength(&(pHLLClass->EncLen), &NumBytes);
       IntRecLength = sizeof(TD_CLASS) - 3 + EncLength;

       pIntClass->NameLen  = EncLength;
       strncpy( pIntClass->Name,
                 ( (char *)&(pHLLClass->EncLen) ) + NumBytes,
                 EncLength );

       pIntClass->RecLen        = IntRecLength;
       pIntClass->RecType       = HLLptr->RecType;
       pIntClass->TypeQual      = pHLLClass->TypeQual;
       pIntClass->ByteSize      = pHLLClass->ByteSize;
       pIntClass->NumMembers    = pHLLClass->NumMembers;
       pIntClass->ItemListIndex = pHLLClass->TypeListIndex;

//     DumpClassRecord( pIntClass );

       IsMappingClass = TRUE;
       pNumMembers    = &pIntClass->NumMembers;

       Mapptr = (Trec *)((uchar *)Mapptr + IntRecLength + 2);
       HLLptr = (Trec *)((uchar *)HLLptr + HLLRecLength + 2 + RecIDLen);
       IntTableLength += (IntRecLength + 2 );
      }
      break;

      case T_MEMFNC:                    /* Member Function             0x45  */
      {
       int        NumBytes;
       USHORT     EncLength;
       TD_MEMFNC *pIntMemFnc;
       HL_MEMFNC *pHLLMemFnc;
       UCHAR     *pEncLen = NULL;
       ULONG      VirtNo  = 0;

       pIntMemFnc   = (TD_MEMFNC *)Mapptr;
       pHLLMemFnc   = (HL_MEMFNC *)((uchar *)HLLptr + 3);
       HLLRecLength = HLLptr->RecLen;

       /**********************************************************************/
       /* - the member function may be virtual or non-virtual                */
       /* - if virtual, get the virtual index.                               */
       /* - define a ptr to the enc name.                                    */
       /**********************************************************************/
       if( IsVirtual(pHLLMemFnc) )
       {
        UCHAR *pVirtNo = NULL;
        UINT  junksize;

        pVirtNo = &(pHLLMemFnc->fid.vTableIndex.FidSpan);
        pEncLen  = GetField( pVirtNo, (void *)&VirtNo, &junksize);
       }
       else
        pEncLen = (UCHAR*)&pHLLMemFnc->fid.EncLen;

       /**********************************************************************/
       /* - calculate the enc length.                                        */
       /* - calculate the internal record length.                            */
       /**********************************************************************/
       EncLength    = GetEncLength((ENCLEN*)pEncLen, &NumBytes);
       IntRecLength = sizeof(TD_MEMFNC) - 3 + EncLength;

       /**********************************************************************/
       /* - fill in the internal record fields.                              */
       /**********************************************************************/
       pIntMemFnc->RecLen      = IntRecLength;
       pIntMemFnc->RecType     = HLLptr->RecType;
       pIntMemFnc->TypeQual    = pHLLMemFnc->TypeQual;
       pIntMemFnc->Protection  = pHLLMemFnc->Protection;
       pIntMemFnc->FuncType    = pHLLMemFnc->FuncType;
       pIntMemFnc->SubRecIndex = pHLLMemFnc->SubRecIndex;
       pIntMemFnc->vTableIndex = VirtNo;
       pIntMemFnc->NameLen     = EncLength;

       strncpy( pIntMemFnc->Name, pEncLen + NumBytes, EncLength );

//     DumpMemFncRecord( pIntMemFnc );

       Mapptr = (Trec *)((uchar *)Mapptr + IntRecLength + 2);
       HLLptr = (Trec *)((uchar *)HLLptr + HLLRecLength + 2 + RecIDLen);
       IntTableLength += (IntRecLength + 2 );
      }
      break;

      case T_CLSMEM:                    /* Class Member                0x46  */
      {
       int        NumBytes;
       TD_CLSMEM *pIntClsMem;
       HL_CLSMEM *pHLLClsMem;
       UCHAR     *pOffset;
       ULONG      Offset;
       char      *pName;
       char      *pStaticName;
       char      *pMemberName;
       USHORT     StaticNameLen;
       USHORT     MemberNameLen;
       UINT       junksize;

       pIntClsMem   = (TD_CLSMEM *)Mapptr;
       pHLLClsMem   = (HL_CLSMEM *)((uchar *)HLLptr + 3);
       HLLRecLength = HLLptr->RecLen;

       pOffset      = &(pHLLClsMem->FidSpan);
       Offset       = 0;
       pName        = GetField( pOffset, (void *)&Offset, &junksize);
       if( IsStatic(pHLLClsMem) || IsVtable(pHLLClsMem))
       {
        pStaticName    = pName;
        StaticNameLen  = GetEncLength((ENCLEN*)pStaticName, &NumBytes);
        pStaticName   += NumBytes;
        pName          = pStaticName + StaticNameLen;
       }
       else
       {
        pStaticName    = NULL;
        StaticNameLen  = 0;
        pName         += 1;
       }

       pMemberName    = pName;
       MemberNameLen  = GetEncLength((ENCLEN*)pMemberName, &NumBytes);
       pMemberName   += NumBytes;
       IntRecLength   = sizeof(TD_CLSMEM) -
                        sizeof(Trec) + 1  -
                        sizeof(pIntClsMem->Name) -
                        sizeof(pIntClsMem->NameLen);

       if( pStaticName )
        IntRecLength  += sizeof(pIntClsMem->NameLen) + StaticNameLen;

       if( pMemberName )
        IntRecLength  += sizeof(pIntClsMem->NameLen) + MemberNameLen;

       /**********************************************************************/
       /* - fill in the internal record fields.                              */
       /**********************************************************************/
       pIntClsMem->RecLen      = IntRecLength;
       pIntClsMem->RecType     = HLLptr->RecType;
       pIntClsMem->TypeQual    = pHLLClsMem->TypeQual;
       pIntClsMem->Protection  = pHLLClsMem->Protection;
       pIntClsMem->TypeIndex   = pHLLClsMem->TypeIndex;
       pIntClsMem->Offset      = Offset;
       pName = (char*)&pIntClsMem->NameLen;
       if( IsStatic(pHLLClsMem) )
       {
        *(USHORT*)pName  = StaticNameLen;
        strncpy( pName+2, pStaticName, StaticNameLen );
        pName += StaticNameLen + 2;
       }
       *(USHORT*)pName  = MemberNameLen;
       strncpy( pName+2, pMemberName, MemberNameLen );

//     DumpClsMemRecord( pIntClsMem );

       Mapptr = (Trec *)((uchar *)Mapptr + IntRecLength + 2);
       HLLptr = (Trec *)((uchar *)HLLptr + HLLRecLength + 2 + RecIDLen);
       IntTableLength += (IntRecLength + 2 );
      }
      break;

      case T_BSECLS:                    /* Base Class                  0x41  */
      {
       TD_BSECLS *pIntBseCls;
       HL_BSECLS *pHLLBseCls;
       UCHAR     *pOffset;
       ULONG      Offset;
       UINT       junksize;

       pIntBseCls   = (TD_BSECLS *)Mapptr;
       pHLLBseCls   = (HL_BSECLS *)((uchar *)HLLptr + 3);
       HLLRecLength = HLLptr->RecLen;

       pOffset      = &(pHLLBseCls->FidSpan);
       Offset       = 0;

       GetField( pOffset, (void *)&Offset, &junksize);

       IntRecLength   = sizeof(TD_BSECLS) - sizeof(Trec) + 1;

       /**********************************************************************/
       /* - fill in the internal record fields.                              */
       /**********************************************************************/
       pIntBseCls->RecLen      = IntRecLength;
       pIntBseCls->RecType     = HLLptr->RecType;
       pIntBseCls->TypeQual    = pHLLBseCls->TypeQual;
       pIntBseCls->Protection  = pHLLBseCls->Protection;
       pIntBseCls->TypeIndex   = pHLLBseCls->TypeIndex;
       pIntBseCls->Offset      = Offset;

//     DumpBseClsRecord( pIntBseCls );

       Mapptr = (Trec *)((uchar *)Mapptr + IntRecLength + 2);
       HLLptr = (Trec *)((uchar *)HLLptr + HLLRecLength + 2 + RecIDLen);
       IntTableLength += (IntRecLength + 2 );
      }
      break;

      case T_CLSDEF:                    /* ClassDef                    0x43  */
      {
       TD_CLSDEF *pIntClsDef;
       HL_CLSDEF *pHLLClsDef;

       pIntClsDef   = (TD_CLSDEF *)Mapptr;
       pHLLClsDef   = (HL_CLSDEF *)((uchar *)HLLptr + 3);
       HLLRecLength = HLLptr->RecLen;
       IntRecLength = sizeof(TD_CLSDEF) - sizeof(Trec) + 1;

       /**********************************************************************/
       /* - fill in the internal record fields.                              */
       /**********************************************************************/
       pIntClsDef->RecLen      = IntRecLength;
       pIntClsDef->RecType     = HLLptr->RecType;
       pIntClsDef->TypeQual    = pHLLClsDef->TypeQual;
       pIntClsDef->Protection  = pHLLClsDef->Protection;
       pIntClsDef->TypeIndex   = pHLLClsDef->TypeIndex;
       pIntClsDef->ClassType   = pHLLClsDef->ClassType;

//     DumpClsDefRecord( pIntClsDef );

       Mapptr = (Trec *)((uchar *)Mapptr + IntRecLength + 2);
       HLLptr = (Trec *)((uchar *)HLLptr + HLLRecLength + 2 + RecIDLen);
       IntTableLength += (IntRecLength + 2 );
      }
      break;

      case T_REF:                    /* Reference                   0x48  */
      {
       TD_REF *pIntRef;
       HL_REF *pHLLRef;

       pIntRef      = (TD_REF *)Mapptr;
       pHLLRef      = (HL_REF *)((uchar *)HLLptr + 3);
       HLLRecLength = HLLptr->RecLen;

       IntRecLength   = sizeof(TD_REF) - sizeof(Trec) + 1;

       /**********************************************************************/
       /* - fill in the internal record fields.                              */
       /**********************************************************************/
       pIntRef->RecLen      = IntRecLength;
       pIntRef->RecType     = HLLptr->RecType;
       pIntRef->TypeIndex   = pHLLRef->TypeIndex;

       Mapptr = (Trec *)((uchar *)Mapptr + IntRecLength + 2);
       HLLptr = (Trec *)((uchar *)HLLptr + HLLRecLength + 2 + RecIDLen);
       IntTableLength += (IntRecLength + 2 );
      }
      break;

      case T_FRIEND:
      {
       int       NumBytes;
       USHORT    EncLength;
       TD_FRIEND *pIntFriend;
       HL_FRIEND *pHLLFriend;

       pIntFriend   = (TD_FRIEND *)Mapptr;
       pHLLFriend   = (HL_FRIEND *)((uchar *)HLLptr + 3);
       HLLRecLength = HLLptr->RecLen;
       EncLength    = GetEncLength(&(pHLLFriend->EncLen), &NumBytes);
       IntRecLength = sizeof(TD_FRIEND) - 3 + EncLength;

       pIntFriend->NameLen  = EncLength;
       strncpy( pIntFriend->Name,
                 ( (char *)&(pHLLFriend->EncLen) ) + NumBytes,
                 EncLength );

       pIntFriend->RecLen    = IntRecLength;
       pIntFriend->RecType   = HLLptr->RecType;
       pIntFriend->TypeQual  = pHLLFriend->TypeQual;
       pIntFriend->TypeIndex = pHLLFriend->TypeIndex;

//     DumpFriendRecord( pIntFriend );

       Mapptr = (Trec *)((uchar *)Mapptr + IntRecLength + 2);
       HLLptr = (Trec *)((uchar *)HLLptr + HLLRecLength + 2 + RecIDLen);
       IntTableLength += (IntRecLength + 2 );
      }
      break;

      case T_BITFLD:                    /* Bit fields.                (0x5C) */
      {
        TD_BITFLD *pIntBitfld = (TD_BITFLD *)Mapptr;
        HL_BITFLD *pHLLBitfld = (HL_BITFLD *)((uchar *)HLLptr + 3);

        IntRecLength = sizeof( TD_BITFLD ) - 2;
        HLLRecLength = HLLptr->RecLen;

        /*********************************************************************/
        /* - Type qualifier field is mapped to the flags field.              */
        /* - Base type field in the internal types record is set to 0x86     */
        /*   (TYPE_ULONG 32 Bit).                                            */
        /*********************************************************************/
        pIntBitfld->RecLen   = IntRecLength;
        pIntBitfld->RecType  = HLLptr->RecType;
        pIntBitfld->Flags    = pHLLBitfld->TypeQual;
        pIntBitfld->Offset   = pHLLBitfld->Offset;
        pIntBitfld->BitSize  = pHLLBitfld->BitSize[0];
        pIntBitfld->BaseType = TYPE_ULONG;

        Mapptr = (Trec *)((uchar *)Mapptr + IntRecLength + 2);
        HLLptr = (Trec *)((uchar *)HLLptr + HLLRecLength + 2 + RecIDLen);
        IntTableLength += (IntRecLength + 2);
        break;
      }

      case T_TYPDEF:                    /* Typedef                    (0x5D) */
      {
        TD_USERDEF *pIntUserDef = (TD_USERDEF *)Mapptr;
        HL_USERDEF *pHLLUserDef = (HL_USERDEF *)((uchar *)HLLptr + 3);

        IntRecLength = HLLptr->RecLen - 2;
        HLLRecLength = HLLptr->RecLen;

        pIntUserDef->RecLen    = IntRecLength;
        pIntUserDef->RecType   = HLLptr->RecType;
        pIntUserDef->TypeIndex = pHLLUserDef->TypeIndex;
        pIntUserDef->NameLen   = pHLLUserDef->NameLen;
        strncpy( pIntUserDef->Name, pHLLUserDef->Name, pHLLUserDef->NameLen );

        Mapptr = (Trec *)((uchar *)Mapptr + IntRecLength + 2);
        HLLptr = (Trec *)((uchar *)HLLptr + HLLRecLength + 2 + RecIDLen);
        IntTableLength += (IntRecLength + 2);
        break;
      }

      case T_PROC:                      /* Procedure.                 (0x75) */
      case T_ENTRY:                     /* Entry.                     (0x53) */
      case T_FUNCTION:                  /* Function.                  (0x54) */
      {
        TD_PROC *pIntProc = (TD_PROC *)Mapptr;
        HL_PROC *pHLLProc = (HL_PROC *)((uchar *)HLLptr + 3);

        IntRecLength = HLLptr->RecLen - 2;
        HLLRecLength = HLLptr->RecLen;

        pIntProc->RecType       = T_PROC;
        pIntProc->RecLen        = IntRecLength;
        pIntProc->Flags         = pHLLProc->TypeQual;
        pIntProc->NumParams     = pHLLProc->NumParams;
        pIntProc->MaxParams     = pHLLProc->MaxParams;
        pIntProc->ReturnType    = pHLLProc->ReturnType;
        pIntProc->ParmListIndex = pHLLProc->ParmListIndex;

        Mapptr = (Trec *)((uchar *)Mapptr + IntRecLength + 2);
        HLLptr = (Trec *)((uchar *)HLLptr + HLLRecLength + 2 + RecIDLen);
        IntTableLength += (IntRecLength + 2);
        break;
      }

      case T_ARRAY:                     /* Array.                     (0x78) */
      {
        TD_ARRAY *pIntArray = (TD_ARRAY *)Mapptr;
        HL_ARRAY *pHLLArray = (HL_ARRAY *)((uchar *)HLLptr + 3);

        IntRecLength = HLLptr->RecLen - 2;
        HLLRecLength = HLLptr->RecLen;

        pIntArray->RecLen     = IntRecLength;
        pIntArray->RecType    = HLLptr->RecType;
        pIntArray->Flags      = pHLLArray->TypeQual;
        pIntArray->ByteSize   = pHLLArray->ByteSize;
        pIntArray->ElemType   = pHLLArray->ElemType;
        pIntArray->NameLen    = pHLLArray->NameLen;
        strncpy( pIntArray->Name, pHLLArray->Name, pHLLArray->NameLen );
        pIntArray->BoundsTypeIndex = pHLLArray->BoundsTypeIndex;

        Mapptr = (Trec *)((uchar *)Mapptr + IntRecLength + 2);
        HLLptr = (Trec *)((uchar *)HLLptr + HLLRecLength + 2 + RecIDLen);
        IntTableLength += (IntRecLength + 2);
        break;
      }

      case T_PTR:                       /* Pointer.                   (0x7A) */
      {
        TD_POINTER *pIntPointer = (TD_POINTER *)Mapptr;
        HL_POINTER *pHLLPointer = (HL_POINTER *)((uchar *)HLLptr + 3);

        if( HLLptr->RecLen > 5 )
          IntRecLength = HLLptr->RecLen - 1;
        else
          IntRecLength = HLLptr->RecLen + 1;
        HLLRecLength = HLLptr->RecLen;

        /*********************************************************************/
        /* - Type qualifier field is mapped to the flags field.              */
        /*********************************************************************/
        pIntPointer->RecLen    = IntRecLength;
        pIntPointer->RecType   = HLLptr->RecType;
        pIntPointer->Flags     = pHLLPointer->TypeQual;
        pIntPointer->TypeIndex = pHLLPointer->TypeIndex;

        if( HLLRecLength > 5 )
        {
          pIntPointer->NameLen   = pHLLPointer->NameLen;
          strncpy( pIntPointer->Name, pHLLPointer->Name, pHLLPointer->NameLen );
        }
        else
          pIntPointer->NameLen = 0;

        Mapptr = (Trec *)((uchar *)Mapptr + IntRecLength + 2);
        HLLptr = (Trec *)((uchar *)HLLptr + HLLRecLength + 2 + RecIDLen);
        IntTableLength += (IntRecLength + 2);
        break;
      }

      case T_SCALAR:                    /* Scalar.                    (0x51) */
      {
        TD_SCALAR *pIntScalar = (TD_SCALAR *)Mapptr;
        HL_SCALAR *pHLLEnum   = (HL_SCALAR *)((uchar *)HLLptr + 3);

        HLLRecLength = HLLptr->RecLen;
        pIntScalar->RecType  = HLLptr->RecType;
        pIntScalar->DataType = pHLLEnum->DataType;
        pIntScalar->RecLen   = IntRecLength = sizeof( TD_SCALAR ) - 2;

        Mapptr = (Trec *)((uchar *)Mapptr + IntRecLength + 2);
        HLLptr = (Trec *)((uchar *)HLLptr + HLLRecLength + 2 + RecIDLen);
        IntTableLength += (IntRecLength + 2);
        break;
      }

      case T_ENUM:                      /* Enums.                     (0x7B) */
      {
        #define  INTENUMSTRUCTLEN  15

        uchar  *TablePtr    = NULL;
        uint    FieldLength = 0;

        TD_ENUM *pIntEnum = (TD_ENUM *)Mapptr;
        HL_ENUM *pHLLEnum = (HL_ENUM *)((uchar *)HLLptr + 3);

        HLLRecLength = HLLptr->RecLen;
        pIntEnum->RecType       = HLLptr->RecType;
        pIntEnum->DataType      = pHLLEnum->DataType;
        pIntEnum->NameListIndex = pHLLEnum->MemListIndex;

        /*********************************************************************/
        /* - Lower and Upper bound fields are of variable size in the raw    */
        /*   table but are of fixed size (4 bytes) in the internal table.    */
        /* - Get field function is used to get the value of the variable size*/
        /*   fields and to increment the table pointer beyond those fields.  */
        /* - "INTENUMSTRUCTLEN" is the length of the internal Enum record    */
        /*   length without the name field.                                  */
        /*********************************************************************/
        TablePtr = ((uchar *)HLLptr + sizeof( HL_ENUM ) + sizeof( Trec ));
        pIntEnum->LBound = pIntEnum->UBound = 0;
        TablePtr = GetField( TablePtr, (void *)&(pIntEnum->LBound),
                             &FieldLength );
        TablePtr = GetField( TablePtr, (void *)&(pIntEnum->UBound),
                             &FieldLength );
        /*********************************************************************/
        /* - Skip the FID string (0x82).                                     */
        /* - Copy the name length and the name fields.                       */
        /*********************************************************************/
        TablePtr++;
        pIntEnum->NameLen = *TablePtr++;
        strncpy( pIntEnum->Name, TablePtr, pIntEnum->NameLen );
        IntRecLength = INTENUMSTRUCTLEN + pIntEnum->NameLen;
        pIntEnum->RecLen = IntRecLength;

        Mapptr = (Trec *)((uchar *)Mapptr + IntRecLength + 2);
        HLLptr = (Trec *)((uchar *)HLLptr + HLLRecLength + 2 + RecIDLen);
        IntTableLength += (IntRecLength + 2);
        break;
      }

      case T_LIST:                      /* List.                      (0x7F) */
      {
        uchar *HLLTablePtr = NULL;
        uchar *IntTablePtr = NULL;
        int    ListRecLen  = 0;

        uchar  TypeQual  = *((uchar *)HLLptr + 3);

        HLLRecLength = ListRecLen = HLLptr->RecLen;
        IntRecLength = 0;
        IntTablePtr = ((uchar *)Mapptr + sizeof( Trec ));
        HLLTablePtr = ((uchar *)HLLptr + sizeof( Trec ) + 2);
        pListRec    = (TD_TYPELIST*)Mapptr;

        /*********************************************************************/
        /* - Type qualifier field tells whether the list is a Namelist or    */
        /*   a Typelist.                                                     */
        /*********************************************************************/
        switch( TypeQual )
        {
          case LIST_ST_TYPES:           /* Type List                  (0x01) */
          {
            int  i = 0;
            TD_TYPELIST *pIntTypeList = (TD_TYPELIST *)Mapptr;

            pIntTypeList->RecType = HLLptr->RecType;
            pIntTypeList->Flags   = LIST_ST_TYPES;
            /*****************************************************************/
            /* - subtract type (0x7F), type qual and FID index field lengths.*/
            /*****************************************************************/
            ListRecLen -= 3;
            while( ListRecLen > 0 )
            {
              pIntTypeList->TypeIndex[i++] = *((ushort *)HLLTablePtr);
              /***************************************************************/
              /* - get past the index (2 bytes) and the FID index (1 byte).  */
              /***************************************************************/
              HLLTablePtr += 3;
              ListRecLen  -= 3;
            }
            /*****************************************************************/
            /* - Calculate the size of the internal record based on the num  */
            /*   of entries in the list + type (1 byte) + type qual (1 byte).*/
            /*****************************************************************/
            IntRecLength = pIntTypeList->RecLen = (2 * i) + 2;
            break;
          }

          case LIST_ST_ENUM:            /* Name List  (Enums)         (0x03) */
          case LIST_ST_NAMES:           /* Name List  (Structs)       (0x02) */
          {
            ushort  NameLen   = 0;
            uint    OffsetLen = 0;
            ulong   Offset    = 0;
            Trec    *pIntNameList = (Trec *)Mapptr;

            /*****************************************************************/
            /* - Name list does not have a structure defined like type list  */
            /*   because of the variable length name fields. So it has to be */
            /*   mapped into the table in a byte by byte basis.              */
            /*****************************************************************/
            pIntNameList->RecType = HLLptr->RecType;
            *IntTablePtr = LIST_ST_NAMES;
            IntTablePtr++;
            /*****************************************************************/
            /* - subtract type (0x7F), type qual and FID index field lengths.*/
            /*****************************************************************/
            ListRecLen -= 3;
            IntRecLength = 2;
            while( ListRecLen > 0 )
            {
              /***************************************************************/
              /* - Copy the name length and the name.                        */
              /* - Calculate the cummulative record length by adding lengths */
              /*   of each item in the list.                                 */
              /***************************************************************/
              *((ushort *)IntTablePtr) = NameLen = *HLLTablePtr;
              IntTablePtr += 2;
              HLLTablePtr++;
              ListRecLen--;

              strncpy( IntTablePtr, HLLTablePtr, NameLen );
              IntTablePtr = IntTablePtr + NameLen;
              HLLTablePtr = HLLTablePtr + NameLen;
              ListRecLen  = ListRecLen  - NameLen;

              Offset = 0;
              /***************************************************************/
              /* The offset field in the name list is of variable length, so */
              /* call getfield to get the value of the offset and to move    */
              /* HLLTablePtr to the next field.                              */
              /***************************************************************/
              {
               /**************************************************************/
               /* optimization bug workaround.                               */
               /**************************************************************/
               UCHAR *tptr = NULL;
               tptr = GetField( HLLTablePtr, (void *)&Offset, &OffsetLen );
               HLLTablePtr = tptr;
              }
              *((ulong *)IntTablePtr) = Offset;
              IntTablePtr += 4;
              IntRecLength += (NameLen + 6);
              ListRecLen -= (OffsetLen + 1);

              HLLTablePtr++;
              ListRecLen--;
            }
            pIntNameList->RecLen = IntRecLength;
            break;
          }
          default:
            break;
        }

        /*********************************************************************/
        /* IntRecLength will be zero in case of PROC LIST or ENUM LIST which */
        /* are not currently supported. so in those cases insert a NULL      */
        /* record into the internal table and skip the entire list record.   */
        /*********************************************************************/
        if( IntRecLength )
          Mapptr = (Trec *)((uchar *)Mapptr + IntRecLength + 2);
        else
        {
          IntRecLength = sizeof( Trec ) - 2;

          Mapptr->RecLen  = IntRecLength;
          Mapptr->RecType = T_NULL;
          Mapptr = (Trec *)((uchar *)Mapptr + IntRecLength + 2);
        }

        IntTableLength += (IntRecLength + 2);
        HLLptr = (Trec *)((uchar *)HLLptr + HLLRecLength + 2 + RecIDLen);
        if( IsMappingClass )
        {
         if( HLLptr->RecType != T_BSECLS )
         {
          UCHAR *pTempList;
          UCHAR *cp;
          int    TypeIndexSize;

          /*******************************************************************/
          /* Come here to add an item to a base class list.                  */
          /* - bump the number of members in the class.                      */
          /* - make a temp copy of the current list.                         */
          /* - stuff a 0 typeindex into the list.                            */
          /* - append the temp list.                                         */
          /* - adjust the list RecLen field.                                 */
          /* - adjust the internal table size.                               */
          /* - free temp memory.                                             */
          /*******************************************************************/
          TypeIndexSize = sizeof(pListRec->TypeIndex[0]);
          *pNumMembers += 1;

          cp = (UCHAR*)pListRec + sizeof(TD_TYPELIST) - TypeIndexSize;

          pTempList = Talloc(pListRec->RecLen);
          memcpy(pTempList, cp, pListRec->RecLen);

          *(USHORT*)cp = 0;

          memcpy(cp+TypeIndexSize, pTempList, pListRec->RecLen);

          pListRec->RecLen  += TypeIndexSize;
          IntTableLength    += TypeIndexSize;

          Mapptr = (Trec *)((UCHAR *)Mapptr + TypeIndexSize);

          Tfree(pTempList);
         }
         IsMappingClass = FALSE;
         pListRec       = NULL;
         pNumMembers    = NULL;
        }
      }
      break;                            /* T_LIST break                      */

      default:
      {
        /*********************************************************************/
        /* In case of the record types which are not currently supported, we */
        /* insert a NULL record into the internal table and skip the record. */
        /*********************************************************************/
        HLLRecLength = HLLptr->RecLen;
        IntRecLength = sizeof( Trec ) - 2;

        Mapptr->RecLen  = IntRecLength;
        Mapptr->RecType = T_NULL;

        HLLptr = (Trec *)((uchar *)HLLptr + HLLRecLength + 2 + RecIDLen);
        Mapptr = (Trec *)((uchar *)Mapptr + IntRecLength + 2);
        IntTableLength += (IntRecLength + 2);
        break;
      }
    }

    /*************************************************************************/
    /* If there is danger that the scratch buffer could overflow for this    */
    /* subsection, reallocate the scratch area and copy the internal table   */
    /* info into the new scratch area.                                       */
    /*************************************************************************/
    if( IntTableLength > UpperLimit )
    {
      UCHAR  *TempBuffer = NULL;

      IntBufSize +=  CHUNKSIZE;
      UpperLimit +=  CHUNKSIZE;
      TempBuffer  =  Talloc( IntBufSize );
      memcpy( TempBuffer, Scratch, IntTableLength );
      Tfree( Scratch );
      Scratch = TempBuffer;
      Mapptr = (Trec *)(Scratch + IntTableLength);
    }
  }
  /***************************************************************************/
  /* Once we are through with the mapping for a module,                      */
  /* - Change the length in the MODULE structure to the internal table length*/
  /* - Allocate memory for the internal types table.                         */
  /* - Copy the internal table from the scratch area.                        */
  /* - Return a pointer to the internal types table.                         */
  /***************************************************************************/
  pModule->TypeLen = IntTableLength;
  IntTypeTable = Talloc( IntTableLength );
  memcpy( IntTypeTable, Scratch, IntTableLength );
  Tfree( RawTable );
  return( IntTypeTable );
}

/*****************************************************************************/
/* GetField()                                                                */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Gets the value of a variable length numeric leaf.                       */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   Buffer       - Points to the start of the variable length leaf.         */
/*   FieldValue   - buffer in which the field value is returned.             */
/*   FieldLength  - buffer in which the field length is returned.            */
/*                                                                           */
/* Return:                                                                   */
/*   Buffer which points to the next field in the record.                    */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/* - The buffer points to the start of the variable length numeric leaf.     */
/*                                                                           */
/*****************************************************************************/
UCHAR *GetField( UCHAR *buffer, void *FieldValue, UINT *FieldLength )
{
  if( *buffer & 0x80 )
  {
    switch( *buffer )
    {
      case 0x88:
        *((char *)FieldValue) = *((char *)(buffer + 1));
        *FieldLength = 1;
        return( buffer + 2 );

      case 0x8B:
        *((uchar *)FieldValue) = *(buffer + 1);
        *FieldLength = 1;
        return( buffer + 2 );

      case 0x85:
        *((short *)FieldValue) = *((short *)(buffer + 1));
        *FieldLength = 2;
        return( buffer + 3 );

      case 0x89:
        *((ushort *)FieldValue) = *((ushort *)(buffer + 1));
        *FieldLength = 2;
        return( buffer + 3 );

      case 0x86:
        *((long *)FieldValue) = *((long *)(buffer + 1));
        *FieldLength = 4;
        return( buffer + 5 );

      case 0x8A:
        *((ulong *)FieldValue) = *((ulong *)(buffer + 1));
        *FieldLength = 4;
        return( buffer + 5 );
    }
  }
  else
  {
    *((uchar *)FieldValue) = *buffer;
    *FieldLength = 1;
    return( buffer + 1 );
  }
  return(0);
}

/*****************************************************************************/
/* MapMSTypes()                                                           813*/
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Map the Microsoft types records into the Internal Types format.         */
/*                                                                           */
/* (Unlike symbols and line numbers mapping functions, the internal types    */
/*  table is allocated within this function as the size of the internal      */
/*  table is difficult to find upfront because of the presence of various    */
/*  variable length fields)                                                  */
/*                                                                           */
/* Parameters:                                                               */
/*   RawTable       (input)  - Pointer to the raw types table.               */
/*   mptr           (input)  - Pointer to the MODULE structure.              */
/*                                                                           */
/* Return:                                                                   */
/*   None.                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/* - Raw symbol table has already been loaded into the memory.               */
/*                                                                           */
/*****************************************************************************/
uchar  *MapMSTypes( uchar *RawTable, MODULE *pModule )                  /*813*/
{
  Trec   *MSptr;
  Trec   *Mapptr;
  uint   IntRecLength;
  uint   MSRecLength;
  uint   IntTableLength = 0;
  uint   RecIDLen;
  uchar  *IntTypeTable;
  uchar  *EndOfTable;
  static uint  UpperLimit = 60000;
  static uchar *Scratch = NULL;

  /***************************************************************************/
  /* The Raw types table is first mapped to a scratch area and then copied to*/
  /* the internal table. This is done because the size of the internal table */
  /* is difficult to be determined upfront. A scratch area of 64k bytes is   */
  /* allocated at first and is extended whenever needed.                     */
  /***************************************************************************/
  if( !Scratch )
    Scratch = Talloc( 65535 );

  RecIDLen = 1;

  /***************************************************************************/
  /* Scan through from start to end of the raw table. Depending on the type  */
  /* of the record, copy the information accordingly to the internal types   */
  /* table.                                                                  */
  /***************************************************************************/
  MSptr  = (Trec *)(RawTable + RecIDLen);
  Mapptr = (Trec *)Scratch;
  EndOfTable = RawTable + pModule->TypeLen;

  while( ((uchar *)MSptr) < EndOfTable )
  {
    IntRecLength = MSRecLength = 0;
    switch( MSptr->RecType )
    {
      case T_STRUCT:                    /* Structure.                 (0x79) */
      {
        #define  INTSTRUCTLEN  13
        uchar  *TablePtr;
        uint   FieldLength;
        uint   BitLen = 0;
        TD_STRUCT *pIntStruct = (TD_STRUCT *)Mapptr;
        MS_STRUCT *pMSStruct;

        /*********************************************************************/
        /* - The structure record has "size" field in bits and is also a     */
        /*   variable length field. The "NumMembers" field is also variable  */
        /*   size. so call "GetField" to get the values of those fields.     */
        /* - A structure is defined for the remaining fields.                */
        /* - "INTSTRUCTLEN" gives the length of the internal structure record*/
        /*   without the name field.                                         */
        /*********************************************************************/
        MSRecLength  = MSptr->RecLen;
        TablePtr = ((uchar *)MSptr + sizeof( Trec ));

        pIntStruct->RecType = MSptr->RecType;
        TablePtr = GetField( TablePtr, (void *)&BitLen,
                             &FieldLength );
        pIntStruct->ByteSize = BitLen / 8;
        pIntStruct->NumMembers = 0;
        TablePtr = GetField( TablePtr, (void *)&(pIntStruct->NumMembers),
                             &FieldLength );
        pMSStruct = (MS_STRUCT *)TablePtr;

        pIntStruct->TypeListIndex = pMSStruct->TypeListIndex;
        pIntStruct->NameListIndex = pMSStruct->NameListIndex;
        pIntStruct->NameLen       = pMSStruct->NameLen;
        strncpy( pIntStruct->Name, pMSStruct->Name, pMSStruct->NameLen );
        IntRecLength = pIntStruct->RecLen = INTSTRUCTLEN + pIntStruct->NameLen;

        Mapptr = (Trec *)((uchar *)Mapptr + IntRecLength + 2);
        MSptr = (Trec *)((uchar *)MSptr + MSRecLength + 2 + RecIDLen);
        IntTableLength += (IntRecLength + 2);
        break;
      }

      case T_BITFLD:                    /* Bit Field.                 (0x5C) */
      {
        #define SIGNED_FLAG     0x02

        TD_BITFLD *pIntBitfld = (TD_BITFLD *)Mapptr;
        MS_BITFLD *pMSBitfld  = (MS_BITFLD *)((uchar *)MSptr + 3);

        IntRecLength = sizeof( TD_BITFLD ) - 2;
        MSRecLength = MSptr->RecLen;

        pIntBitfld->RecLen   = IntRecLength;
        pIntBitfld->RecType  = MSptr->RecType;

        pIntBitfld->Flags    = 0;
        pIntBitfld->Offset   = pMSBitfld->Offset;
        pIntBitfld->BitSize  = pMSBitfld->BitSize;

        switch( pMSBitfld->BaseType )
        {
          case 0x7D:
          case 0x7C:
            if( pModule->DbgFormatFlags.Typs == TYPE103_CL386 )
              pIntBitfld->BaseType = TYPE_ULONG;
            else
              pIntBitfld->BaseType = TYPE_USHORT;
            break;

          case 0x6F:
            pIntBitfld->BaseType = TYPE_UCHAR;
            break;
        }
        /*********************************************************************/
        /* Flags are set based on the "BaseType" field in the raw record.    */
        /*********************************************************************/
        if( pMSBitfld->BaseType == TFLD_INT )
          pIntBitfld->Flags |= SIGNED_FLAG;

        Mapptr = (Trec *)((uchar *)Mapptr + IntRecLength + 2);
        MSptr = (Trec *)((uchar *)MSptr + MSRecLength + 2 + RecIDLen);
        IntTableLength += (IntRecLength + 2);
        break;
      }

      case T_TYPDEF:                    /* TypeDef.                   (0x5D) */
      {
        TD_USERDEF *pIntUserDef = (TD_USERDEF *)Mapptr;
        MS_USERDEF *pMSUserDef  = (MS_USERDEF *)((uchar *)MSptr + 3);

        IntRecLength = MSptr->RecLen - 1;
        MSRecLength  = MSptr->RecLen;

        pIntUserDef->RecLen    = IntRecLength;
        pIntUserDef->RecType   = MSptr->RecType;

        /*********************************************************************/
        /* - Change the "A*" in the 0:16 pointer type indexes to "E*".       */
        /*********************************************************************/
        if( pModule->DbgFormatFlags.Typs != TYPE103_CL386 )
          pIntUserDef->TypeIndex =
                              GetInternal0_16PtrIndex( pMSUserDef->TypeIndex );
        else
          pIntUserDef->TypeIndex = pMSUserDef->TypeIndex;
        pIntUserDef->NameLen   = pMSUserDef->NameLen;
        strncpy( pIntUserDef->Name, pMSUserDef->Name, pMSUserDef->NameLen );

        Mapptr = (Trec *)((uchar *)Mapptr + IntRecLength + 2);
        MSptr = (Trec *)((uchar *)MSptr + MSRecLength + 2 + RecIDLen);
        IntTableLength += (IntRecLength + 2);
        break;
      }

      case T_PROC:                      /* Procedure.                 (0x75) */
      case T_ENTRY:                     /* Entry.                     (0x53) */
      case T_FUNCTION:                  /* Function.                  (0x54) */
      {
        #define  ARGSRTOL   0x01
        #define  CALLERPOPS 0x02
        #define  FUNC32BIT  0x04
        #define  FARCALL    0x08

        uchar  CallingConv;
        TD_PROC *pIntProc = (TD_PROC *)Mapptr;
        MS_PROC *pMSProc  = (MS_PROC *)((uchar *)MSptr + 3);

        IntRecLength = sizeof( TD_PROC ) - 2;
        MSRecLength = MSptr->RecLen;

        pIntProc->RecType = T_PROC;
        pIntProc->RecLen  = IntRecLength;
        pIntProc->Flags   = 0;

        /*********************************************************************/
        /* Take care of the FID_VOID (0x81) in case of C2 1.1. The return    */
        /* type field is not present in the raw table record.                */
        /*********************************************************************/
        if( pMSProc->FID_ReturnType == FLDID_VOID )
        {
          uchar *RecordPtr = (uchar *)&(pMSProc->ReturnType);

          CallingConv = *RecordPtr;
          pIntProc->ReturnType    = TYPE_VOID;
          pIntProc->NumParams     = *(RecordPtr + 1);
          pIntProc->ParmListIndex = *(ushort *)(RecordPtr + 3);
        }
        else
        {
          CallingConv = pMSProc->CallConv;
          pIntProc->NumParams     = pMSProc->NumParams;
          pIntProc->ReturnType    = pMSProc->ReturnType;
          pIntProc->ParmListIndex = pMSProc->ParmListIndex;
        }

        /*********************************************************************/
        /* The flags are set based on the "Calling Convention" field in the  */
        /* raw table record.                                                 */
        /*********************************************************************/
        switch( CallingConv )
        {
          case 0x63:
            pIntProc->Flags |= ARGSRTOL;
            pIntProc->Flags |= CALLERPOPS;
            break;

          case 0x96:
            pIntProc->Flags |= FARCALL;
            break;

          default:
            break;
        }

        /*********************************************************************/
        /* In case of CL386 do the following corrections:                    */
        /*                                                                   */
        /*   VOID PTR        0xBC  ===>  0xB7                                */
        /*   VOID            0x9C  ===>  0x9C                                */
        /*                   0xDC  ===>  0xD7                                */
        /*********************************************************************/
        if( pModule->DbgFormatFlags.Typs == TYPE103_CL386 )
        {
          pIntProc->Flags |= FUNC32BIT;
          switch( pIntProc->ReturnType )
          {
            case 0xBC:
              pIntProc->ReturnType = 0xB7;
              break;
            case 0x9C:
              pIntProc->ReturnType = 0x97;
              break;
            case 0xDC:
              pIntProc->ReturnType = 0xD7;
              break;
          }

          switch( pIntProc->ParmListIndex )
          {
            case 0xBC:
              pIntProc->ParmListIndex = 0xB7;
              break;
            case 0x9C:
              pIntProc->ParmListIndex = 0x97;
              break;
            case 0xDC:
              pIntProc->ParmListIndex = 0xD7;
              break;
          }
        }
        pIntProc->MaxParams = 0;

        Mapptr = (Trec *)((uchar *)Mapptr + IntRecLength + 2);
        MSptr  = (Trec *)((uchar *)MSptr + MSRecLength + 2 + RecIDLen);
        IntTableLength += (IntRecLength + 2);
        break;
      }

      case T_ARRAY:                     /* Array.                     (0x78) */
      {
        #define  INTARRAYLEN  12

        uchar  *TablePtr;
        uint   FieldLength;
        uint   BitLen = 0;
        TD_ARRAY *pIntArray = (TD_ARRAY *)Mapptr;
        MS_ARRAY *pMSArray;

        MSRecLength = MSptr->RecLen;

        /*********************************************************************/
        /* The length field is a variable size numeric leaf. so call GetField*/
        /* to get the value. A structure is defined for the remaining fields.*/
        /*********************************************************************/
        TablePtr = (uchar *)MSptr + 3;
        TablePtr = GetField( TablePtr, (void *)&BitLen, &FieldLength );
        pIntArray->ByteSize = BitLen / 8;
        if( FieldLength > 1 )
          FieldLength++;
        pMSArray = (MS_ARRAY *)TablePtr;

        pIntArray->RecLen   = MSptr->RecLen + (4 - FieldLength) + 4;
        pIntArray->RecType  = MSptr->RecType;
        /*********************************************************************/
        /* - Change the "A*" in the 0:16 pointer type indexes to "E*".       */
        /*********************************************************************/
        if( pModule->DbgFormatFlags.Typs != TYPE103_CL386 )
          pIntArray->ElemType =
                              GetInternal0_16PtrIndex( pMSArray->ElemType );
        else
          pIntArray->ElemType = pMSArray->ElemType;
        pIntArray->NameLen  = 0;

        /*********************************************************************/
        /* The name field in the array type record is an optional field. so  */
        /* copy the name only if the name field is present in the record.    */
        /*********************************************************************/
        if( MSRecLength > (FieldLength + 5) )
        {
          pIntArray->NameLen  = pMSArray->NameLen;
          strncpy( pIntArray->Name, pMSArray->Name, pMSArray->NameLen );
          pIntArray->RecLen   = MSptr->RecLen + (4 - FieldLength) - 1 +
                                pMSArray->NameLen;
        }
        IntRecLength = pIntArray->RecLen;
        pIntArray->BoundsTypeIndex = 0;
        pIntArray->Flags = 0;

        Mapptr = (Trec *)((uchar *)Mapptr + IntRecLength + 2);
        MSptr = (Trec *)((uchar *)MSptr + MSRecLength + 2 + RecIDLen);
        IntTableLength += (IntRecLength + 2);
        break;
      }

      case T_PTR:                       /* Pointer.                   (0x7A) */
      {
        TD_POINTER *pIntPointer = (TD_POINTER *)Mapptr;
        MS_POINTER *pMSPointer = (MS_POINTER *)((uchar *)MSptr + 3);

        IntRecLength = MSptr->RecLen - 1;
        MSRecLength = MSptr->RecLen;

        pIntPointer->RecLen  = IntRecLength;
        pIntPointer->RecType = MSptr->RecType;
        /*********************************************************************/
        /* The flags field is set based on the "Model" field and the type of */
        /* the compiler.                                                     */
        /*********************************************************************/
        switch( pMSPointer->Model )
        {
          case 0x74:
            if( pModule->DbgFormatFlags.Typs == TYPE103_CL386 )
              pIntPointer->Flags = PTR_0_32;
            else
              pIntPointer->Flags = PTR_0_16;
            break;

          case 0x5E:
          case 0x73:
            if( pModule->DbgFormatFlags.Typs == TYPE103_CL386 )
              pIntPointer->Flags = PTR_0_32;
            else
              pIntPointer->Flags = PTR_16_16;
            break;
        }

        /*********************************************************************/
        /* - Change the "A*" in the 0:16 pointer type indexes to "E*".       */
        /*********************************************************************/
        if( pModule->DbgFormatFlags.Typs != TYPE103_CL386 )
          pIntPointer->TypeIndex = GetInternal0_16PtrIndex(
                                     pMSPointer->TypeIndex );
        else
          pIntPointer->TypeIndex = pMSPointer->TypeIndex;
        /*********************************************************************/
        /* The Name field in the pointer record is optional. We still copy   */
        /* some junk into the internal table record. The redundant info is   */
        /* over written by the next record as the incrementing the internal  */
        /* table pointer according to the size of the record takes care of   */
        /* this problem !!!.                                                 */
        /*********************************************************************/
        pIntPointer->NameLen   = pMSPointer->NameLen;
        strncpy( pIntPointer->Name, pMSPointer->Name, pMSPointer->NameLen );

        Mapptr = (Trec *)((uchar *)Mapptr + IntRecLength + 2);
        MSptr = (Trec *)((uchar *)MSptr + MSRecLength + 2 + RecIDLen);
        IntTableLength += (IntRecLength + 2);
        break;
      }

      case T_ENUM:                      /* Enums.                     (0x7B) */
      {
        #define  INTSCLRSTRUCTLEN  15

        uchar  *TablePtr;
        uint   FieldLength;
        ushort *IntDataType;
        TD_SCALAR *pIntScalar = (TD_SCALAR *)Mapptr;
        TD_ENUM   *pIntEnum   = (TD_ENUM *)Mapptr;
        MS_SCALAR *pMSScalar  = (MS_SCALAR *)((uchar *)MSptr + 3);

        MSRecLength = MSptr->RecLen;
        /*********************************************************************/
        /* We have a special case of a enum record in Microsoft format (7B)  */
        /* which is similar to our internal scalar record. So if we come     */
        /* across such a enum record map it to our internal scalar record    */
        /* which is of type 0x51.                                            */
        /*********************************************************************/
        if( MSptr->RecLen == 3 )
        {
          pIntScalar->RecType = T_SCALAR;
          pIntScalar->RecLen  = IntRecLength = sizeof( TD_SCALAR ) - 2;
          IntDataType = &(pIntScalar->DataType);
        }
        else
        {
          pIntEnum->RecType = MSptr->RecType;
          IntDataType = &(pIntEnum->DataType);
        }
        /*********************************************************************/
        /* The Datatype field is set based on the "BaseType" field in the raw*/
        /* record and the length of the scalar.                              */
        /*********************************************************************/
        switch( pMSScalar->BaseType )
        {
          case TFLD_INT:
            switch( pMSScalar->BitLength )
            {
              case 0x08:
                *IntDataType = TYPE_CHAR;
                break;

              case 0x10:
                *IntDataType = TYPE_SHORT;
                break;

              case 0x20:
                *IntDataType = TYPE_LONG;
                break;
            }
            break;

          case TFLD_UINT:
            switch( pMSScalar->BitLength )
            {
              case 0x08:
                *IntDataType = TYPE_UCHAR;
                break;

              case 0x10:
                *IntDataType = TYPE_USHORT;
                break;

              case 0x20:
                *IntDataType = TYPE_ULONG;
                break;
            }
            break;

          case TFLD_CHAR:
            *IntDataType = TYPE_CHAR;
            break;
        }

        if( MSptr->RecLen > 3 )
        {
          pIntEnum->NameLen = pMSScalar->NameLen;
          strncpy( pIntEnum->Name, pMSScalar->Name, pIntEnum->NameLen );

          TablePtr = ((uchar *)MSptr + 7 + pMSScalar->NameLen );
          TablePtr++;
          pIntEnum->NameListIndex = *(ushort *)TablePtr;
          TablePtr += 2;
          /*******************************************************************/
          /* - Lower and Upper bound fields are of variable size in the raw  */
          /*   table but are of fixed size (4 bytes) in the internal table.  */
          /* - Get field function is used to get the value of the var size   */
          /*   fields and to increment the table pointer beyond those fields.*/
          /* - "INTSCLRSTRUCTLEN" is the length of the internal scalar record*/
          /*   length without the name field.                                */
          /*******************************************************************/
          pIntEnum->LBound = pIntEnum->UBound = 0;
          TablePtr = GetField( TablePtr, (void *)&(pIntEnum->LBound),
                               &FieldLength );
          TablePtr = GetField( TablePtr, (void *)&(pIntEnum->UBound),
                               &FieldLength );
          IntRecLength = INTSCLRSTRUCTLEN + pIntEnum->NameLen;
          pIntEnum->RecLen = IntRecLength;
        }

        Mapptr = (Trec *)((uchar *)Mapptr + IntRecLength + 2);
        MSptr = (Trec *)((uchar *)MSptr + MSRecLength + 2 + RecIDLen);
        IntTableLength += (IntRecLength + 2);
        break;
      }

      case T_LIST:                      /* Lists.                     (0x7F) */
      {
        uchar *MSTablePtr;
        uchar *IntTablePtr;
        int  ListRecLen;
        uchar FIDIndex = *((uchar *)MSptr + 3);

        MSRecLength = ListRecLen = MSptr->RecLen;
        IntRecLength = 0;
        IntTablePtr = ((uchar *)Mapptr + sizeof( Trec ));
        MSTablePtr  = ((uchar *)MSptr + sizeof( Trec ) + 1);

        /*********************************************************************/
        /* Unlike HLL records where lists are differentiated using "TypeQual"*/
        /* field, MS records are differentiated using FID indexes.           */
        /*********************************************************************/
        switch( FIDIndex )
        {
          case 0x83:                    /* Type List                  (0x83) */
          {
            int  i = 0;
            TD_TYPELIST *pIntTypeList = (TD_TYPELIST *)Mapptr;

            pIntTypeList->RecType = MSptr->RecType;
            pIntTypeList->Flags   = LIST_ST_TYPES;
            /*****************************************************************/
            /* - subtract type (0x7F) and FID index field lengths.           */
            /*****************************************************************/
            ListRecLen -= 2;
            while( ListRecLen > 0 )
            {
              /***************************************************************/
              /* - if the complex type == 0, then set it to void.            */
              /*                                                             */
              /* Note: This should not happen;however, complex types == 0    */
              /*       show up in T_LIST records in IBM C/2 1.1.             */
              /*                                                             */
              /***************************************************************/
              if( *(ushort *)MSTablePtr == 0 )
                *(ushort *)MSTablePtr = TYPE_VOID;
              /***************************************************************/
              /* - Change the "A*" in the 0:16 pointer type indexes to "E*". */
              /***************************************************************/
              if( pModule->DbgFormatFlags.Typs != TYPE103_CL386 )
                pIntTypeList->TypeIndex[i++] = GetInternal0_16PtrIndex(
                                               *((ushort *)MSTablePtr) );
              else
                pIntTypeList->TypeIndex[i++] = *((ushort *)MSTablePtr);
              MSTablePtr += 3;
              ListRecLen -= 3;
            }
            /*****************************************************************/
            /* - Calculate the size of the internal record based on the num  */
            /*   of entries in the list + type (1 byte) + type qual (1 byte).*/
            /*****************************************************************/
            IntRecLength = pIntTypeList->RecLen = (2 * i) + 2;
            break;
          }

          case 0x82:                    /* Name List.                 (0x82) */
          {
            ushort  NameLen;
            ushort  AdjustLen;
            uint    OffsetLen;
            ulong   Offset;
            Trec    *pIntNameList = (Trec *)Mapptr;

            /*****************************************************************/
            /* - Name list does not have a structure defined like type list  */
            /*   because of the variable length name fields. So it has to be */
            /*   mapped into the table in a byte by byte basis.              */
            /*****************************************************************/
            pIntNameList->RecType = MSptr->RecType;
            *IntTablePtr = LIST_ST_NAMES;
            IntTablePtr++;
            /*****************************************************************/
            /* - subtract type (0x7F) and FID index field lengths.           */
            /*****************************************************************/
            ListRecLen  -= 2;
            IntRecLength = 2;
            while( ListRecLen > 0 )
            {
              /***************************************************************/
              /* - Copy the name length and the name.                        */
              /* - Calculate the cummulative record length by adding lengths */
              /*   of each item in the list.                                 */
              /***************************************************************/
              *((ushort *)IntTablePtr) = NameLen = *MSTablePtr;
              if( NameLen == 0 )
              {
                MSptr = (Trec *)EndOfTable;
                break;
              }
              IntTablePtr += 2;
              MSTablePtr++;
              ListRecLen--;

              strncpy( IntTablePtr, MSTablePtr, NameLen );
              IntTablePtr += NameLen;
              MSTablePtr  += NameLen;
              ListRecLen  -= NameLen;

              Offset = 0;
              /***************************************************************/
              /* The offset field in the name list is of variable length, so */
              /* call getfield to get the value of the offset and to move    */
              /* MSTablePtr to the next field.                               */
              /***************************************************************/
              AdjustLen = 0;
              if( *MSTablePtr & 0x80 )
                AdjustLen = 1;

              MSTablePtr = GetField( MSTablePtr, (void *)&Offset, &OffsetLen );
              *((ulong *)IntTablePtr) = Offset;
              IntTablePtr += 4;
              IntRecLength += (NameLen + 6);
              ListRecLen -= (OffsetLen + AdjustLen);

              MSTablePtr++;
              ListRecLen--;
            }
            pIntNameList->RecLen = IntRecLength;
            break;
          }

          default:
            break;
        }

        /*********************************************************************/
        /* IntRecLength will be zero in case of PROC LIST or ENUM LIST which */
        /* are not currently supported. so in those cases insert a NULL      */
        /* record into the internal table and skip the entire list record.   */
        /*********************************************************************/
        if( IntRecLength )
          Mapptr = (Trec *)((uchar *)Mapptr + IntRecLength + 2);
        else
        {
          IntRecLength = sizeof( Trec ) - 2;

          Mapptr->RecLen  = IntRecLength;
          Mapptr->RecType = T_NULL;
          Mapptr = (Trec *)((uchar *)Mapptr + IntRecLength + 2);
        }

        IntTableLength += (IntRecLength + 2);
        if( (uchar *)MSptr != EndOfTable )
          MSptr = (Trec *)((uchar *)MSptr + MSRecLength + 2 + RecIDLen);
        break;
      }

      case T_SKIP:                      /* Skip record.               (0x90) */
      {
        TD_SKIP *pIntSkip = (TD_SKIP *)Mapptr;
        MS_SKIP *pMSSkip  = (MS_SKIP *)((uchar *)MSptr + 3);

        IntRecLength = sizeof( TD_SKIP ) - 2;
        MSRecLength  = MSptr->RecLen;

        pIntSkip->RecLen   = IntRecLength;
        pIntSkip->RecType  = MSptr->RecType;
        pIntSkip->NewIndex = pMSSkip->NewIndex;

        Mapptr = (Trec *)((uchar *)Mapptr + IntRecLength + 2);
        MSptr  = (Trec *)((uchar *)MSptr + MSRecLength + 2 + RecIDLen);
        IntTableLength += (IntRecLength + 2);
        break;
      }

      default:
      {
        /*********************************************************************/
        /* In case of the record types which are not currently supported, we */
        /* insert a NULL record into the internal table and skip the record. */
        /*********************************************************************/
        MSRecLength = MSptr->RecLen;
        IntRecLength = sizeof( Trec ) - 2;

        Mapptr->RecLen  = IntRecLength;
        Mapptr->RecType = T_NULL;

        MSptr = (Trec *)((uchar *)MSptr + MSRecLength + 2 + RecIDLen);
        Mapptr = (Trec *)((uchar *)Mapptr + IntRecLength + 2);
        IntTableLength += (IntRecLength + 2);
        break;
      }
    }

    /*************************************************************************/
    /* If there is danger that the scratch buffer could overflow for this    */
    /* subsection, reallocate the scratch area and copy the internal table   */
    /* info into the new scratch area.                                       */
    /*************************************************************************/
    if( IntTableLength > UpperLimit )
    {
      uchar  *TempBuffer;

      UpperLimit += 20000;
      TempBuffer = Talloc( UpperLimit + 5000 );
      memcpy( TempBuffer, Scratch, IntTableLength );
      Tfree( Scratch );
      Scratch = TempBuffer;
      Mapptr = (Trec *)(Scratch + IntTableLength);
    }
  }
  /***************************************************************************/
  /* Once we are through with the mapping for a module,                      */
  /* - Change the length in the MODULE structure to the internal table length*/
  /* - Allocate memory for the internal types table.                         */
  /* - Copy the internal table from the scratch area.                        */
  /* - Return a pointer to the internal types table.                         */
  /***************************************************************************/
  pModule->TypeLen = IntTableLength;
  IntTypeTable = Talloc( IntTableLength );
  memcpy( IntTypeTable, Scratch, IntTableLength );
  Tfree( RawTable );
  return( IntTypeTable );
}

/*****************************************************************************/
/* GetInternal0_16PtrIndex()                                              813*/
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Convert the NEAR (A* 0:16) pointers to the internal type indexes (E*).  */
/*   Convert the HUGE (E*) pointers to FAR (C* 16:16) pointers.              */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   TypeIndex    (input) - Type index input.                                */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* - NEAR (A* 0:16) pointers are converted to internal type indexes (E*).    */
/* - HUGE (E*) pointers are converted to FAR (C* 16:16) pointers.            */
/* - For the rest the original type index is returned.                       */
/*                                                                           */
/*****************************************************************************/
ushort  GetInternal0_16PtrIndex( ushort TypeIndex )
{
  if( TypeIndex >= 512 )
   return(TypeIndex);

  if( ((TypeIndex & 0xF0) == 0xA0) || (TypeIndex == 0xB7) )
    return( TypeIndex | 0x40 );
  else
  if( (TypeIndex & 0xF0) == 0xE0 )
    return( TypeIndex & 0xDF );
  else
    return( TypeIndex );
}

/*****************************************************************************/
/* GetEncLength()                                                            */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Decode an encoded length. Determine how many bytes(1/2) are in          */
/*   the encoded length.                                                     */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   pEncLen     -> to 1/2 byte encoded length                               */
/*   pNumBytes   -> to receiver of number(1/2) of bytes used in encoded len. */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   EncLength                                                               */
/*                                                                           */
/*****************************************************************************/
USHORT GetEncLength( ENCLEN *pEncLen, int *pNumBytes )
{
 UCHAR  ENC_1stByte;
 UCHAR  ENC_2ndByte;
 USHORT EncLength;
 int    NumBytes;

 EncLength   = 0;
 NumBytes    = 0;
 ENC_1stByte = pEncLen->ENC_1stByte;
 ENC_2ndByte = pEncLen->ENC_2ndByte;

 if(TestBit(ENC_1stByte, 7 ))
 {
  EncLength = (ENC_1stByte & 0x7f) + ENC_2ndByte;
  NumBytes  = 2;
 }
 else
 {
  EncLength = ENC_1stByte;
  NumBytes  = 1;
 }

 *pNumBytes = NumBytes;
 return(EncLength);
}

#define VIRTUAL_MASK  0x10
BOOL IsVirtual( HL_MEMFNC *pHLLMemFnc )
{
 if( pHLLMemFnc->TypeQual & VIRTUAL_MASK )
  return(TRUE);
 else
  return(FALSE);
}

#define STATIC_MASK  0x01
BOOL IsStatic( HL_CLSMEM *pHLLClsMem )
{
 if( pHLLClsMem->TypeQual & STATIC_MASK )
  return(TRUE);
 else
  return(FALSE);
}

#define VTABLE_MASK  0x02
BOOL IsVtable( HL_CLSMEM *pHLLClsMem )
{
 if( pHLLClsMem->TypeQual & VTABLE_MASK )
  return(TRUE);
 else
  return(FALSE);
}
