/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY813*/
/*   mapsyms.                                                                */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Mapping funtions for symbols from other formats to the internal format. */
/*                                                                           */
/* History:                                                                  */
/*...                                                                        */
/*... 03/01/93  813   Selwyn    Changes for HL02/HL03.                       */
/*****************************************************************************/
#include "all.h"                        /* SD86 include files                */
#include "mapsyms.h"                    /*                                   */

/*****************************************************************************/
/* MapMS16Syms()                                                          809*/
/*                                                                           */
/* Description:                                                              */
/*   Map a 16 bit MSC 6.0/IBM C2 1.1 raw symbol table to the internal symbol */
/*   table.                                                                  */
/*                                                                           */
/* Parameters:                                                               */
/*   RawTable       (input)  - Pointer to the raw symbol table.              */
/*   IntSymTable    (output) - Pointer to the internal symbol table.         */
/*   symlen         (input)  - Length of the raw symbol table.               */
/*                                                                           */
/* Return:                                                                   */
/*   None.                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/* - Raw symbol table has already been loaded into the memory.               */
/* - Memory for the internal symbol table has already been allocated.        */
/*                                                                           */
/*****************************************************************************/
void  MapMS16Syms( uchar *RawTable, SSRec *IntSymTable, uint symlen )   /*809*/
{
  int LenSymSectionMS16;
  SSRec *mapptr;
  SSrec *MS16ptr;
  uchar  MS16len;

  LenSymSectionMS16 = symlen;
  MS16ptr = (SSrec *)RawTable;
  mapptr = IntSymTable;

  /***************************************************************************/
  /* Scan the Raw symbol table. Depending on the type of each raw symbol     */
  /* table record, copy the information to the internal symbol table record. */
  /***************************************************************************/
  while( LenSymSectionMS16 > 0 )
  {
    LenSymSectionMS16 -= (MS16ptr->RecLen + 1);
    switch( MS16ptr->RecType )
    {
      case SSPROC:
      {
        SSProc    *procmapptr  = (SSProc  *)mapptr;
        SSProc16  *proc16ptr   = (SSProc16 *)MS16ptr;

        MS16len = proc16ptr->RecLen;
        procmapptr->RecLen     = MS16len + 8;
        procmapptr->Flags      = PROCEDURE;
        procmapptr->RecType    = SSPROC;

        procmapptr->ProcOffset = proc16ptr->ProcOffset;
        /*********************************************************************/
        /* - Change the "A*" in the 0:16 pointer type indexes to "E*".       */
        /*********************************************************************/
        procmapptr->TypeIndex  = GetInternal0_16PtrIndex(
                                   proc16ptr->TypeIndex );
        procmapptr->ProcLen    = proc16ptr->ProcLen;

        procmapptr->DebugStart = proc16ptr->DebugStart;
        procmapptr->DebugEnd   = proc16ptr->DebugEnd;

        procmapptr->ClassType  = proc16ptr->Reserved;
        procmapptr->NearFar    = proc16ptr->NearFar;
        procmapptr->NameLen    = (ushort)proc16ptr->Name[0];

        strncpy( procmapptr->Name, &(proc16ptr->Name[1]),
                 proc16ptr->Name[0] );
        MS16ptr = (SSrec *)((uchar *)MS16ptr + MS16len + 1);
        mapptr  = (SSRec *)((uchar *)mapptr + procmapptr->RecLen + 2);
        break;
      }

      case SSBEGIN:
      {
        SSBegin16 *Begin16ptr  = (SSBegin16 *)MS16ptr;
        SSBegin   *Beginmapptr = (SSBegin *)mapptr;

        MS16len = Begin16ptr->RecLen;
        Beginmapptr->RecLen      = sizeof( SSBegin ) - 2;
        Beginmapptr->RecType     = SSBEGIN;

        Beginmapptr->BlockOffset = Begin16ptr->BlockOffset;
        Beginmapptr->BlockLen    = Begin16ptr->BlockLen;

        MS16ptr = (SSrec *)((uchar *)MS16ptr + MS16len + 1);
        mapptr = (SSRec *)((uchar *)mapptr + Beginmapptr->RecLen + 2);
        break;
      }

      case SSEND:
      {
        SSEnd16 *End16ptr  = (SSEnd16 *)MS16ptr;
        SSEnd   *Endmapptr = (SSEnd  *)mapptr;

        MS16len = End16ptr->RecLen;
        Endmapptr->RecLen   = sizeof( SSEnd ) - 2;
        Endmapptr->RecType  = SSEND;
        mapptr = (SSRec *)((uchar *)mapptr + Endmapptr->RecLen + 2);
        MS16ptr = (SSrec *)((uchar *)MS16ptr + MS16len + 1);
        break;
      }

      case SSDEF:
      {
        SSDef16 *Def16ptr  = (SSDef16 *)MS16ptr;
        SSDef   *Defmapptr = (SSDef  *)mapptr;
        short    FrameOffset = 0;

        MS16len = Def16ptr->RecLen;
        Defmapptr->RecLen   = MS16len + 3;
        Defmapptr->RecType  = SSDEF;
        FrameOffset = Def16ptr->FrameOffset;
        if( FrameOffset < 0 )
          Defmapptr->FrameOffset = -(int)(- FrameOffset);
        else
          Defmapptr->FrameOffset = (int)FrameOffset;
        /*********************************************************************/
        /* - Change the "A*" in the 0:16 pointer type indexes to "E*".       */
        /*********************************************************************/
        Defmapptr->TypeIndex   = GetInternal0_16PtrIndex( Def16ptr->TypeIndex );
        Defmapptr->NameLen     = (ushort)Def16ptr->Name[0];

        strncpy( Defmapptr->Name, &(Def16ptr->Name[1]), Def16ptr->Name[0] );
        mapptr = (SSRec *)((uchar *)mapptr + Defmapptr->RecLen + 2);
        MS16ptr = (SSrec *)((uchar *)MS16ptr + MS16len + 1);
        break;
      }

      case SSVAR:
      {
        SSVar16  *Var16ptr   = (SSVar16 *)MS16ptr;
        SSVar    *Varmapptr  = (SSVar  *)mapptr;

        MS16len = Var16ptr->RecLen;
        Varmapptr->RecLen    = MS16len + 4;
        Varmapptr->Flags     = STATIC;
        Varmapptr->RecType   = SSVAR;
        Varmapptr->Offset    = Var16ptr->Offset;
        Varmapptr->ObjectNum = Var16ptr->ObjectNum;
        /*********************************************************************/
        /* - Change the "A*" in the 0:16 pointer type indexes to "E*".       */
        /*********************************************************************/
        Varmapptr->TypeIndex = GetInternal0_16PtrIndex( Var16ptr->TypeIndex );
        Varmapptr->NameLen   = (ushort)Var16ptr->Name[0];

        strncpy( Varmapptr->Name, &(Var16ptr->Name[1]), Var16ptr->Name[0] );
        mapptr = (SSRec *)((uchar *)mapptr + Varmapptr->RecLen + 2);
        MS16ptr = (SSrec *)((uchar *)MS16ptr + MS16len + 1);
        break;
      }

      case SSCHGDEF:
      {
        SSChgDef16  *ChgDef16ptr  = (SSChgDef16 *)MS16ptr;
        SSChgDef    *ChgDefmapptr = (SSChgDef  *)mapptr;

        MS16len = ChgDef16ptr->RecLen;
        ChgDefmapptr->RecLen   = sizeof( SSChgDef ) - 2;
        ChgDefmapptr->RecType  = SSCHGDEF;
        ChgDefmapptr->SegNum   = ChgDef16ptr->SegNum;
        ChgDefmapptr->Reserved = ChgDef16ptr->Reserved;

        mapptr = (SSRec *)((uchar *)mapptr + ChgDefmapptr->RecLen + 2);
        MS16ptr = (SSrec *)((uchar *)MS16ptr + MS16len + 1);
        break;
      }

      case SSUSERDEF:
      {
        SSUserDef16 *UserDef16ptr  = (SSUserDef16 *)MS16ptr;
        SSUserDef   *UserDefmapptr = (SSUserDef *)mapptr;

        MS16len = UserDef16ptr->RecLen;
        UserDefmapptr->RecLen    = MS16len + 1;
        UserDefmapptr->RecType   = SSUSERDEF;
        /*********************************************************************/
        /* - Change the "A*" in the 0:16 pointer type indexes to "E*".       */
        /*********************************************************************/
        UserDefmapptr->TypeIndex = GetInternal0_16PtrIndex(
                                     UserDef16ptr->TypeIndex );
        UserDefmapptr->NameLen   = (ushort)UserDef16ptr->Name[0];
        strncpy( UserDefmapptr->Name, &(UserDef16ptr->Name[1]),
                 UserDef16ptr->Name[0] );

        mapptr = (SSRec *)((uchar *)mapptr + UserDefmapptr->RecLen + 2);
        MS16ptr = (SSrec *)((uchar *)MS16ptr + MS16len + 1);
        break;
      }

      case SSREG:
      {
        SSReg16 *Reg16ptr  = (SSReg16 *)MS16ptr;
        SSReg   *Regmapptr = (SSReg  *)mapptr;

        MS16len = Reg16ptr->RecLen;
        Regmapptr->RecLen    = MS16len + 1;
        Regmapptr->RecType   = SSREG;
        /*********************************************************************/
        /* - Change the "A*" in the 0:16 pointer type indexes to "E*".       */
        /*********************************************************************/
        Regmapptr->TypeIndex = GetInternal0_16PtrIndex( Reg16ptr->TypeIndex );
        Regmapptr->RegNum    = Reg16ptr->RegNum;
        Regmapptr->NameLen   = (ushort)Reg16ptr->Name[0];
        strncpy( Regmapptr->Name, &(Reg16ptr->Name[1]),
                 Reg16ptr->Name[0] );

        mapptr = (SSRec *)((uchar *)mapptr + Regmapptr->RecLen + 2);
        MS16ptr = (SSrec *)((uchar *)MS16ptr + MS16len + 1);
        break;
      }

      default:
      {                                                                 /*810*/
        SSReg16 *Reg16ptr = (SSReg16 *)MS16ptr;                         /*810*/
        MS16ptr = (SSrec *)((uchar *)MS16ptr + Reg16ptr->RecLen + 1);   /*810*/
        break;                                                          /*810*/
      }                                                                 /*810*/
    }
  }
}

/*****************************************************************************/
/* MapMS32Syms()                                                          809*/
/*                                                                           */
/* Description:                                                              */
/*   Map a 32 bit MSC 6.0 raw symbol table to the internal symbol table.     */
/*                                                                           */
/* Parameters:                                                               */
/*   RawTable       (input)  - Pointer to the raw symbol table.              */
/*   IntSymTable    (output) - Pointer to the internal symbol table          */
/*   symlen         (input)  - Length of the raw symbol table.               */
/*                                                                           */
/* Return:                                                                   */
/*   None.                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/* - Raw symbol table has already been loaded into the memory.               */
/* - Memory for the internal symbol table has already been allocated.        */
/*                                                                           */
/*****************************************************************************/
void  MapMS32Syms( uchar *RawTable, SSRec *IntSymTable, uint symlen )   /*809*/
{
  int LenSymSectionMS32;
  SSRec *mapptr;
  SSrec *MS32ptr;
  uchar  MS32len;

  LenSymSectionMS32 = symlen;
  MS32ptr = (SSrec *)RawTable;
  mapptr = IntSymTable;

  /***************************************************************************/
  /* Scan the Raw symbol table. Depending on the type of each raw symbol     */
  /* table record, copy the information to the internal symbol table record. */
  /***************************************************************************/
  while( LenSymSectionMS32 > 0 )
  {
    LenSymSectionMS32 -= (MS32ptr->RecLen + 1);
    switch( MS32ptr->RecType )
    {
      case SSPROC32:
      {
        SSProc    *procmapptr  = (SSProc  *)mapptr;
        SSProc32  *proc32ptr   = (SSProc32 *)MS32ptr;

        MS32len = proc32ptr->RecLen;
        procmapptr->RecLen     = MS32len + 6;
        procmapptr->Flags      = PROCEDURE;
        procmapptr->RecType    = SSPROC;

        procmapptr->ProcOffset = proc32ptr->ProcOffset;
        procmapptr->TypeIndex  = proc32ptr->TypeIndex;
        procmapptr->ProcLen    = proc32ptr->ProcLen;

        procmapptr->DebugStart = proc32ptr->DebugStart;
        procmapptr->DebugEnd   = proc32ptr->DebugEnd;

        procmapptr->ClassType  = proc32ptr->Reserved;
        procmapptr->NearFar    = proc32ptr->NearFar;
        procmapptr->NameLen    = (ushort)proc32ptr->Name[0];

        strncpy( procmapptr->Name, &(proc32ptr->Name[1]),
                 proc32ptr->Name[0] );
        MS32ptr = (SSrec *)((uchar *)MS32ptr + MS32len + 1);
        mapptr = (SSRec *)((uchar *)mapptr + procmapptr->RecLen + 2);
        break;
      }

      case SSBEGIN32:
      {
        SSBegin32 *Begin32ptr  = (SSBegin32 *)MS32ptr;
        SSBegin   *Beginmapptr = (SSBegin *)mapptr;

        MS32len = Begin32ptr->RecLen;
        Beginmapptr->RecLen      = sizeof( SSBegin ) - 2;
        Beginmapptr->RecType     = SSBEGIN;

        Beginmapptr->BlockOffset = Begin32ptr->BlockOffset;
        Beginmapptr->BlockLen    = Begin32ptr->BlockLen;

        MS32ptr = (SSrec *)((uchar *)MS32ptr + MS32len + 1);
        mapptr = (SSRec *)((uchar *)mapptr + Beginmapptr->RecLen + 2);
        break;
      }

      case SSEND32:
      {
        SSEnd32 *End32ptr  = (SSEnd32 *)MS32ptr;
        SSEnd   *Endmapptr = (SSEnd  *)mapptr;

        MS32len = End32ptr->RecLen;
        Endmapptr->RecLen   = sizeof( SSEnd ) - 2;
        Endmapptr->RecType  = SSEND;
        mapptr = (SSRec *)((uchar *)mapptr + Endmapptr->RecLen + 2);
        MS32ptr = (SSrec *)((uchar *)MS32ptr + MS32len + 1);
        break;
      }

      case SSDEF32:
      {
        SSDef32 *Def32ptr  = (SSDef32 *)MS32ptr;
        SSDef   *Defmapptr = (SSDef  *)mapptr;
        short    FrameOffset = 0;

        MS32len = Def32ptr->RecLen;
        Defmapptr->RecLen   = MS32len + 1;
        Defmapptr->RecType  = SSDEF;
        FrameOffset = Def32ptr->FrameOffset;
        if( FrameOffset < 0 )
          Defmapptr->FrameOffset = -(int)(- FrameOffset);
        else
          Defmapptr->FrameOffset = (int)FrameOffset;
        Defmapptr->TypeIndex   = Def32ptr->TypeIndex;
        Defmapptr->NameLen     = (ushort)Def32ptr->Name[0];

        strncpy( Defmapptr->Name, &(Def32ptr->Name[1]), Def32ptr->Name[0] );
        mapptr = (SSRec *)((uchar *)mapptr + Defmapptr->RecLen + 2);
        MS32ptr = (SSrec *)((uchar *)MS32ptr + MS32len + 1);
        break;
      }

      case SSVAR32:
      {
        SSVar32  *Var32ptr   = (SSVar32 *)MS32ptr;
        SSVar    *Varmapptr  = (SSVar  *)mapptr;

        MS32len = Var32ptr->RecLen;
        Varmapptr->RecLen    = MS32len + 2;
        Varmapptr->Flags     = STATIC;
        Varmapptr->RecType   = SSVAR;
        Varmapptr->Offset    = Var32ptr->Offset;
        Varmapptr->ObjectNum = Var32ptr->ObjectNum;
        Varmapptr->TypeIndex = Var32ptr->TypeIndex;
        Varmapptr->NameLen   = (ushort)Var32ptr->Name[0];

        strncpy( Varmapptr->Name, &(Var32ptr->Name[1]), Var32ptr->Name[0] );
        mapptr = (SSRec *)((uchar *)mapptr + Varmapptr->RecLen + 2);
        MS32ptr = (SSrec *)((uchar *)MS32ptr + MS32len + 1);
        break;
      }

      case SSCHGDEF32:
      {
        SSChgDef32  *ChgDef32ptr  = (SSChgDef32 *)MS32ptr;
        SSChgDef    *ChgDefmapptr = (SSChgDef  *)mapptr;

        MS32len = ChgDef32ptr->RecLen;
        ChgDefmapptr->RecLen   = sizeof( SSChgDef ) - 2;
        ChgDefmapptr->RecType  = SSCHGDEF;
        ChgDefmapptr->SegNum   = ChgDef32ptr->SegNum;
        ChgDefmapptr->Reserved = ChgDef32ptr->Reserved;

        mapptr = (SSRec *)((uchar *)mapptr + ChgDefmapptr->RecLen + 2);
        MS32ptr = (SSrec *)((uchar *)MS32ptr + MS32len + 1);
        break;
      }

      case SSUSERDEF32:
      {
        SSUserDef32 *UserDef32ptr  = (SSUserDef32 *)MS32ptr;
        SSUserDef   *UserDefmapptr = (SSUserDef *)mapptr;

        MS32len = UserDef32ptr->RecLen;
        UserDefmapptr->RecLen    = MS32len + 1;
        UserDefmapptr->RecType   = SSUSERDEF;
        UserDefmapptr->TypeIndex = UserDef32ptr->TypeIndex;
        UserDefmapptr->NameLen   = (ushort)UserDef32ptr->Name[0];
        strncpy( UserDefmapptr->Name, &(UserDef32ptr->Name[1]),
                 UserDef32ptr->Name[0] );

        mapptr = (SSRec *)((uchar *)mapptr + UserDefmapptr->RecLen + 2);
        MS32ptr = (SSrec *)((uchar *)MS32ptr + MS32len + 1);
        break;
      }

      case SSREG32:
      {
        SSReg32 *Reg32ptr  = (SSReg32 *)MS32ptr;
        SSReg   *Regmapptr = (SSReg  *)mapptr;

        MS32len = Reg32ptr->RecLen;
        Regmapptr->RecLen    = MS32len + 1;
        Regmapptr->RecType   = SSREG;
        Regmapptr->TypeIndex = Reg32ptr->TypeIndex;
        Regmapptr->RegNum    = Reg32ptr->RegNum;
        Regmapptr->NameLen   = (ushort)Reg32ptr->Name[0];
        strncpy( Regmapptr->Name, &(Reg32ptr->Name[1]),
                 Reg32ptr->Name[0] );

        mapptr = (SSRec *)((uchar *)mapptr + Regmapptr->RecLen + 2);
        MS32ptr = (SSrec *)((uchar *)MS32ptr + MS32len + 1);
        break;
      }

      default:
      {                                                                 /*810*/
        SSReg32 *reg32ptr = (SSReg32 *)MS32ptr;                         /*810*/
        MS32ptr = (SSrec *)((uchar *)MS32ptr + reg32ptr->RecLen + 1);   /*810*/
        break;                                                          /*810*/
      }                                                                 /*810*/
   }
 }
}

/*****************************************************************************/
/* MapHLLSyms()                                                           809*/
/*                                                                           */
/* Description:                                                              */
/*   Map a 32 bit IBM HLL raw symbol table to the internal symbol table.     */
/*                                                                           */
/* Parameters:                                                               */
/*   RawTable       (input)  - Pointer to the raw symbol table.              */
/*   IntSymTable    (output) - Pointer to the internal symbol table          */
/*   mptr           (input)  - Pointer to the MODULE structure.              */
/*                                                                           */
/* Return:                                                                   */
/*   None.                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/* - Raw symbol table has already been loaded into the memory.               */
/* - Memory for the internal symbol table has already been allocated.        */
/*                                                                           */
/*****************************************************************************/
void  MapHLLSyms( uchar *RawTable, SSRec *IntSymTable, MODULE *mptr )   /*809*/
{
  int LenSymSectionHLL;
  SSRec *mapptr;
  SSrec *HLLptr;
  uchar  HLLlen;
  uchar  RecordType;
  uint   Adjust;

  LenSymSectionHLL = mptr->SymLen;
  HLLptr = (SSrec *)RawTable;
  mapptr = IntSymTable;

  /***************************************************************************/
  /* Scan the Raw symbol table. Depending on the type of each raw symbol     */
  /* table record, copy the information to the internal symbol table record. */
  /*                                                                         */
  /* The record length in HLL level 3 format could be in encoded format, so  */
  /* call functions "GetRecordLength" and "GetRecordType" to get the length  */
  /* and type for each raw symbol table record.                              */
  /***************************************************************************/
  while( LenSymSectionHLL > 0 )
  {
    HLLlen     = GetRecordLength( (uchar *)HLLptr, mptr );
    RecordType = GetRecordType( (uchar *)HLLptr, mptr );

    Adjust = 1;
    if( TestBit( *(uchar *)HLLptr, 7 ) &&
                               (
                                (mptr->DbgFormatFlags.Syms == TYPE104_HL03) ||
                                (mptr->DbgFormatFlags.Syms == TYPE104_HL04)
                               )
      )
      Adjust = 2;

    LenSymSectionHLL -= (HLLlen + Adjust);
    switch( RecordType )
    {
      case SSPROC:
      case SSENTRY:
      case SSPROCCPP:
      case SSMEMFUNC:
      {
        SSProc    *procmapptr  = (SSProc  *)mapptr;
        SSProcHLL *procHLLptr;

        procHLLptr = (SSProcHLL *)( (uchar *)HLLptr + Adjust + 1 );
        procmapptr->RecLen     = HLLlen + 2;
        switch( RecordType )
        {
          case SSPROC:
            procmapptr->Flags = PROCEDURE;
            break;
          case SSENTRY:
            procmapptr->Flags = SECONDENTRY;
            break;
          case SSPROCCPP:
            procmapptr->Flags = CPPPROCEDURE;
            break;
          case SSMEMFUNC:
            procmapptr->Flags = MEMBERFUNC;
            break;
        }

        procmapptr->RecType    = SSPROC;
        procmapptr->ProcOffset = procHLLptr->ProcOffset;
        procmapptr->TypeIndex  = procHLLptr->TypeIndex;
        procmapptr->ProcLen    = procHLLptr->ProcLen;

        procmapptr->DebugStart = procHLLptr->DebugStart;
        procmapptr->DebugEnd   = procHLLptr->DebugEnd;

        procmapptr->ClassType  = procHLLptr->Reserved;
        procmapptr->NearFar    = procHLLptr->NearFar;
        procmapptr->NameLen    =
             GetRecordLength( (uchar *)&(procHLLptr->Name[0]), mptr );

        strncpy( procmapptr->Name, &(procHLLptr->Name[1]),
                 procmapptr->NameLen );
        mapptr = (SSRec *)((uchar *)mapptr + procmapptr->RecLen + 2);
        HLLptr = (SSrec *)((uchar *)HLLptr + HLLlen + Adjust);

        break;
      }

      case SSBEGIN:
      {
        SSBeginHLL *BeginHLLptr;
        SSBegin    *Beginmapptr  = (SSBegin *)mapptr;

        BeginHLLptr = (SSBeginHLL *)( (uchar *)HLLptr + Adjust + 1 );

        Beginmapptr->RecLen      = sizeof( SSBegin ) - 2;
        Beginmapptr->RecType     = SSBEGIN;
        Beginmapptr->BlockOffset = BeginHLLptr->BlockOffset;
        Beginmapptr->BlockLen    = BeginHLLptr->BlockLen;

        mapptr = (SSRec *)((uchar *)mapptr + Beginmapptr->RecLen + 2);
        HLLptr = (SSrec *)((uchar *)HLLptr + HLLlen + Adjust);

        break;
      }

      case SSEND:
      {
        SSEnd    *Endmapptr = (SSEnd  *)mapptr;

        Endmapptr->RecLen   = sizeof( SSEnd ) - 2;
        Endmapptr->RecType  = SSEND;
        mapptr = (SSRec *)((uchar *)mapptr + Endmapptr->RecLen + 2);
        HLLptr = (SSrec *)((uchar *)HLLptr + HLLlen + Adjust);

        break;
      }

      case SSDEF:
      {
        SSDefHLL *DefHLLptr;
        SSDef    *Defmapptr = (SSDef  *)mapptr;

        DefHLLptr = (SSDefHLL *)( (uchar *)HLLptr + Adjust + 1 );

        Defmapptr->RecLen   = HLLlen + 1;
        Defmapptr->RecType  = SSDEF;
        Defmapptr->FrameOffset = DefHLLptr->FrameOffset;
        Defmapptr->TypeIndex   = DefHLLptr->TypeIndex;
        Defmapptr->NameLen     = (ushort)DefHLLptr->Name[0];

        strncpy( Defmapptr->Name, &(DefHLLptr->Name[1]), DefHLLptr->Name[0] );
        mapptr = (SSRec *)((uchar *)mapptr + Defmapptr->RecLen + 2);
        HLLptr = (SSrec *)((uchar *)HLLptr + HLLlen + Adjust);

        break;
      }

      case SSCHGDEF:
      {
        SSChgDefHLL *ChgDefHLLptr;
        SSChgDef    *ChgDefmapptr = (SSChgDef  *)mapptr;

        ChgDefHLLptr = (SSChgDefHLL *)( (uchar *)HLLptr + Adjust + 1 );

        ChgDefmapptr->RecLen   = sizeof( SSChgDef ) - 2;
        ChgDefmapptr->RecType  = SSCHGDEF;
        ChgDefmapptr->SegNum   = ChgDefHLLptr->SegNum;
        ChgDefmapptr->Reserved = ChgDefHLLptr->Reserved;

        mapptr = (SSRec *)((uchar *)mapptr + ChgDefmapptr->RecLen + 2);
        HLLptr = (SSrec *)((uchar *)HLLptr + HLLlen + Adjust);

        break;
      }

      case SSVAR:
      case SSVARCPP:
      {
        SSVarHLL *VarHLLptr;
        SSVar    *Varmapptr  = (SSVar  *)mapptr;

        VarHLLptr = (SSVarHLL *)( (uchar *)HLLptr + Adjust + 1 );

        Varmapptr->RecLen    = HLLlen + 2;
        switch( RecordType )
        {
          case SSVAR:
            Varmapptr->Flags = STATIC;
            break;
          case SSVARCPP:
            Varmapptr->Flags = CPPSTATIC;
            break;
        }
        Varmapptr->RecType   = SSVAR;
        Varmapptr->Offset    = VarHLLptr->Offset;
        Varmapptr->ObjectNum = VarHLLptr->ObjectNum;
        Varmapptr->TypeIndex = VarHLLptr->TypeIndex;
        Varmapptr->NameLen   =
             GetRecordLength( (uchar *)&(VarHLLptr->Name[0]), mptr );

        strncpy( Varmapptr->Name, &(VarHLLptr->Name[1]), Varmapptr->NameLen );
        mapptr = (SSRec *)((uchar *)mapptr + Varmapptr->RecLen + 2);
        HLLptr = (SSrec *)((uchar *)HLLptr + HLLlen + Adjust);

        break;
      }

      case SSUSERDEF:
      {
        SSUserDefHLL *UserDefHLLptr;
        SSUserDef    *UserDefmapptr = (SSUserDef *)mapptr;

        UserDefHLLptr = (SSUserDefHLL *)( (uchar *)HLLptr + Adjust + 1 );

        UserDefmapptr->RecLen    = HLLlen + 1;
        UserDefmapptr->RecType   = SSUSERDEF;
        UserDefmapptr->TypeIndex = UserDefHLLptr->TypeIndex;
        UserDefmapptr->NameLen   = (ushort)UserDefHLLptr->Name[0];
        strncpy( UserDefmapptr->Name, &(UserDefHLLptr->Name[1]),
                 UserDefHLLptr->Name[0] );

        mapptr = (SSRec *)((uchar *)mapptr + UserDefmapptr->RecLen + 2);
        HLLptr = (SSrec *)((uchar *)HLLptr + HLLlen + Adjust);

        break;
      }

      case SSREG:
      {
        SSRegHLL *RegHLLptr;
        SSReg    *Regmapptr  = (SSReg  *)mapptr;

        RegHLLptr = (SSRegHLL *)( (uchar *)HLLptr + Adjust + 1 );

        Regmapptr->RecLen    = HLLlen + 1;
        Regmapptr->RecType   = SSREG;
        Regmapptr->TypeIndex = RegHLLptr->TypeIndex;
        Regmapptr->RegNum    = RegHLLptr->RegNum;
        Regmapptr->NameLen   = (ushort)RegHLLptr->Name[0];
        strncpy( Regmapptr->Name, &(RegHLLptr->Name[1]),
                 RegHLLptr->Name[0] );

        mapptr = (SSRec *)((uchar *)mapptr + Regmapptr->RecLen + 2);
        HLLptr = (SSrec *)((uchar *)HLLptr + HLLlen + Adjust);

        break;
      }

      default:
      {                                                                 /*810*/
        HLLptr = (SSrec *)((uchar *)HLLptr + HLLlen + Adjust );         /*810*/
        break;                                                          /*810*/
      }                                                                 /*810*/
   }
 }
}

/*****************************************************************************/
/* GetRecordLength()                                                      809*/
/*                                                                           */
/* Description:                                                              */
/*   Get the symbol record length from the encoded length.                   */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   Buffer   (input)  - Pointer to the start of the symbol record.          */
/*   pModule  (input)  - Pointer to the module structure.                    */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   Length of the symbol record.                                            */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/* -  The buffer passed points to the start of the symbol record.            */
/*                                                                           */
/*****************************************************************************/
ushort  GetRecordLength( uchar *Buffer, MODULE *pModule )               /*809*/
{
  ushort RecordLength;

  /***************************************************************************/
  /* - Set the default record length.                                        */
  /* - Check to see if the 7th bit in the first byte is set and the symbol   */
  /*   type is HL03. If so calculate the record length.                      */
  /* - Return the record length.                                             */
  /***************************************************************************/
  RecordLength = *Buffer;
  if( *Buffer & 0x80 )
  {
    if( (pModule->DbgFormatFlags.Syms == TYPE104_HL03) ||
        (pModule->DbgFormatFlags.Syms == TYPE104_HL04)
      )
      RecordLength = ((*Buffer & 0x7F) << 8) + *(Buffer + 1);
  }
  return( RecordLength );
}

/*****************************************************************************/
/* GetRecordType()                                                        809*/
/*                                                                           */
/* Description:                                                              */
/*   Get the symbol record type which is present after an encoded length.    */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   Buffer   (input)  - Pointer to the start of the symbol record.          */
/*   pModule  (input)  - Pointer to the module structure.                    */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   Type of the symbol record.                                              */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/* -  The buffer passed points to the start of the symbol record.            */
/*                                                                           */
/*****************************************************************************/
uchar  GetRecordType( uchar *Buffer, MODULE *pModule )                  /*809*/
{
  uchar RecordType;

  /***************************************************************************/
  /* - Set the default record type.                                          */
  /* - Check to see if the 7th bit in the first byte is set and the symbol   */
  /*   type is HL03. If so calculate the record type.                        */
  /* - Return the record type.                                               */
  /***************************************************************************/
  RecordType = *(Buffer + 1);
  if( *Buffer & 0x80 )
  {
    if( (pModule->DbgFormatFlags.Syms == TYPE104_HL03) ||
        (pModule->DbgFormatFlags.Syms == TYPE104_HL04)
      )
      RecordType = *(Buffer + 2);
  }
  return( RecordType );
}
