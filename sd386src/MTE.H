
/*****************************************************************************/
/* Module Load Table.                                                        */
/*****************************************************************************/
#define    CODE   0                     /* a code type object flag           */
#define    DATA   1                     /* a data type object flag           */

typedef struct _ObjTableEntry           /*  struct for internal object       */
{                                       /*  table for EXE or DLL.            */
 UINT   ObjNum;                         /*  the object number.               */
 UINT   ObjLoadAddr;                    /*  address where loader put it.     */
 UINT   ObjLoadSize;                    /*  size of the object.              */
 UCHAR  ObjType;                        /*  code or data object.             */
 UCHAR  ObjBitness;                     /*  16 bit or 32 bit object.         */
 USHORT ObjLoadSel;                     /*  selector for 16:16 access.       */
 USHORT ObjLoadOff;                     /*  offset   for 16:16 access.       */
}OBJTABLEENTRY;                         /*                                   */

typedef struct _Module_Load_Entry
{
 int  LengthOfMteEntry;
 UINT mte;
 UINT ModuleNameLength;
 char ModuleName[1];
}MODULE_LOAD_ENTRY;

/*****************************************************************************/
/* Mte Table .                                                               */
/*****************************************************************************/
#define ENDOFTABLE      0xFFFFFFFF
#define TABLE_INCREMENT 100

typedef struct _MteTable
{
  ULONG mte;
  ULONG ObjNum;
  ULONG LoadAddr;
}MTE_TABLE_ENTRY;

/*****************************************************************************/
/* X-server object table list entry header. Note that the OBJTABLEENTRY      */
/* may be repeated NumEntries times.                                         */
/*****************************************************************************/
typedef struct _XSrvObjTableEntry
{
 struct _XSrvObjTableEntry *pnext;
 ULONG         mte;
 ULONG         NumEntries;
 OBJTABLEENTRY ObjEntry;
}XSRVOBJTABLEENTRY;
