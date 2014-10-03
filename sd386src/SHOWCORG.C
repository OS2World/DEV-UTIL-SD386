/*****************************************************************************/
/* File:                                              IBM INTERNAL USE ONLY  */
/*      showcorg.c                                                           */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*      Display Co Processor registers                                       */
/*                                                                           */
/* History:                                                                  */
/*                                                                           */
/*...Revised 08/29/95                                                        */
/*...                                                                        */
/*****************************************************************************/

#include "all.h"

extern UINT          VideoCols;
extern UINT          VideoRows;
extern UCHAR         *VideoMap;
extern UCHAR         VideoAtr;
extern UCHAR        *BoundPtr;
extern UCHAR         Reg_Display;
extern UINT          ProcessID;
extern UINT          VioStartOffSet;

static struct coregs
{
 UCHAR row;
}regs[] = { 1,3,5,7,9,11,13,15,0};

static UCHAR Status[]   = {" Status:"};
static UCHAR Control[]  = {" Control:"};
static UCHAR Status1[]  = {"  C   CCCESPUOZDI"};
static UCHAR Status2[]  = {" B3TOP210SFEEEEEE"};
static UCHAR Control1[] = {"     RCPC  PUOZDI"};

static UCHAR BitString[18]     = { Attrib(vaRegWind) };
static UCHAR HexDig[]          = "0123456789ABCDEF";
static UCHAR RegValue[23]      = { Attrib(vaRegWind) };
static UCHAR Blank[PRINTLEN+2] = { Attrib(vaRegWind) };

static COPROCESSORREGS  coproc_regs;

/*****************************************************************************/
/* ShowCoRegs()                                                              */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*      Display the Co processor registers                                   */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
void ShowCoRegs()
{
 UINT   Start_Col;
 struct coregs  *p;
 UCHAR  Attribute;
 UINT   x,y;
 USHORT TagValue;
 UCHAR  Buffer[PRINTLEN+2];
 long double Value387;
 int    Len;
 APIRET rc;

 int    i,j;
 USHORT tag[8];
 char   DosDebugRegValue[8][23];
 char   PhysRegValueString[8][23];
 char   PhysRegValue[8][10];
 char   RelativeRegString[8][4];
 USHORT PhysRegStackTop;
 UCHAR  PhysRegIndex[8];

 /****************************************************************************/
 /* - Get the coprocessor registers.                                         */
 /****************************************************************************/
 memset( &coproc_regs, 0, sizeof(COPROCESSORREGS) );

 rc = xGetCoRegs(&coproc_regs);
 if( rc != 0 )
 {
  SayMsgBox2("CoProcessor Not Loaded", 800UL);
  Reg_Display = 0;
  return;
 }

 /****************************************************************************/
 /* - Define tag fields. See 80387 reference.                                */
 /****************************************************************************/
 tag[0] = (coproc_regs.TagWord & 0x0003 );
 tag[1] = (coproc_regs.TagWord & 0x000C );
 tag[2] = (coproc_regs.TagWord & 0x0030 );
 tag[3] = (coproc_regs.TagWord & 0x00C0 );
 tag[4] = (coproc_regs.TagWord & 0x0300 );
 tag[5] = (coproc_regs.TagWord & 0x0C00 );
 tag[6] = (coproc_regs.TagWord & 0x3000 );
 tag[7] = (coproc_regs.TagWord & 0xC000 );

 /****************************************************************************/
 /* - Convert the coprocessor register values to displayable hex strings.    */
 /****************************************************************************/
 memset( DosDebugRegValue, 0 , sizeof(DosDebugRegValue) );
 Converttox( (UCHAR*)(&coproc_regs.Stack[0]), DosDebugRegValue[0]);
 Converttox( (UCHAR*)(&coproc_regs.Stack[1]), DosDebugRegValue[1]);
 Converttox( (UCHAR*)(&coproc_regs.Stack[2]), DosDebugRegValue[2]);
 Converttox( (UCHAR*)(&coproc_regs.Stack[3]), DosDebugRegValue[3]);
 Converttox( (UCHAR*)(&coproc_regs.Stack[4]), DosDebugRegValue[4]);
 Converttox( (UCHAR*)(&coproc_regs.Stack[5]), DosDebugRegValue[5]);
 Converttox( (UCHAR*)(&coproc_regs.Stack[6]), DosDebugRegValue[6]);
 Converttox( (UCHAR*)(&coproc_regs.Stack[7]), DosDebugRegValue[7]);

 /****************************************************************************/
 /* - Define the physical register that contains the top of the stack.       */
 /****************************************************************************/
 PhysRegStackTop =  (USHORT)((coproc_regs.StatusWord >> 11) & 0x7);

 /****************************************************************************/
 /* - Now, associate the display hex strings with their physical registers.  */
 /*   We wiil be displaying physical registers from 0-7.                     */
 /* - Define the physical display hex value. This will be converted by       */
 /*   sprintf to a displayable real value.                                   */
 /* - Define the logical register index associated with a physical register. */
 /****************************************************************************/
 memset( PhysRegValueString, 0 , sizeof(PhysRegValueString) );
 i = PhysRegStackTop;
 for( j=0; j<8; j++ )
 {
  strcpy(PhysRegValueString[i], DosDebugRegValue[j]);
  memcpy(PhysRegValue[i], &coproc_regs.Stack[j], sizeof(REG80BIT) );
  sprintf(RelativeRegString[i], "ST%c", HexDig[j] );

  (i==7)?i=0:i++;
 }

/*****************************************************************************/
/* The following block of code will printf the coprocessor registers.        */
/* Remove the ifdefs and build to get it to print.                           */
/*---------------------------------------------------------------------------*/
#if 0
 printf("\n\n");
 printf("\nControlWord %x ",coproc_regs.ControlWord);
 printf("\nReserved1   %x ",coproc_regs.Reserved1  );
 printf("\nStatusWord  %x ",coproc_regs.StatusWord );
 printf("\nReserved2   %x ",coproc_regs.Reserved2  );
 printf("\nTagWord     %x ",coproc_regs.TagWord    );
 printf("\nReserved3   %x ",coproc_regs.Reserved3  );
 printf("\nIpOffset    %x ",coproc_regs.IpOffset   );
 printf("\nCsSelector  %x ",coproc_regs.CsSelector );
 printf("\nReserved4   %x ",coproc_regs.Reserved4  );
 printf("\nOpOffset    %x ",coproc_regs.OpOffset   );
 printf("\nOpSelector  %x ",coproc_regs.OpSelector );
 printf("\nReserved5   %x ",coproc_regs.Reserved5  );

 printf("\n\nPhysical Register Stack Top = %d", PhysRegStackTop );
 printf("\n\nLogical Registers");
 for( i=0; i<8; i++ )
 {
  printf("\n%d, %s, ", i, DosDebugRegValue[i] );
 }

 printf("\n\nPhysical Registers");
 for( i=0; i<8; i++ )
 {
  int    bitval1;
  int    bitval2;

  bitval1 =  (tag[i] >> (i*2)  )&0x0001 ;
  bitval2 =  (tag[i] >> (i*2+1))&0x0001;

  printf("\n%d, %s, %1d%1d ", i, PhysRegValueString[i], bitval2, bitval1 );
 }
 fflush(0);
#endif
/*--end of print coprocessor registers--------------------------------------*/

 /****************************************************************************/
 /* - Release the screen bounds.                                             */
 /* - Display the register box + headers/footers.                            */
 /****************************************************************************/
 memset(Blank+1, SPACE, sizeof(Blank)-2);
 memset(Buffer, 0, sizeof(Buffer) );
 memset(BoundPtr+VioStartOffSet,VideoCols,VideoRows-VioStartOffSet);
 Start_Col = VideoCols - COREGSWINWIDTH + 4;
 VideoAtr  = vaRegWind;
 if (TestBit(Reg_Display,REPAINT))
 {
  Vfmtbox( "",VioStartOffSet,
           VideoCols - COREGSWINWIDTH,COREGSWINLENGTH,COREGSWINWIDTH );

  ResetBit(Reg_Display,REPAINT);
  Status[0]   = Attrib(vaRegWind);
  Status1[0]  = Attrib(vaRegWind);
  Status2[0]  = Attrib(vaRegWind);
  Control[0]  = Attrib(vaRegWind);
  Control1[0] = Attrib(vaRegWind);
  putrc( StatusRow + VioStartOffSet, Start_Col-2, Status );
  putrc( StatusRow-2+VioStartOffSet, Start_Col+9, Status1 );
  putrc( StatusRow-1+VioStartOffSet, Start_Col+9, Status2 );
  putrc( ControlRow+VioStartOffSet, Start_Col-2, Control );
  putrc( ControlRow-1+VioStartOffSet, Start_Col+9, Control1 );
 }

 /****************************************************************************/
 /* Find the Toggle attribute of the existing color attribute for register   */
 /* window and put it in vaRegTogg position of videomap.                     */
 /****************************************************************************/
 Attribute            = VideoMap[vaRegWind];
 Attribute           ^= HIGHLIGHT;
 VideoMap[vaRegTogg]  = Attribute;

 /****************************************************************************/
 /* - Now, display the registers.                                            */
 /****************************************************************************/
 for( p=regs, i=0; i < 8 ; p++, i++)
 {
  TagValue  = tag[i] >> (i*2) ;
  TagValue &= 0x3;
  switch(TagValue)
  {
   case TAG_VALID:
   case TAG_INFIN:
    strcpy( RegValue + 1, PhysRegValueString[i] );
    break;

   case TAG_ZERO:
    FillIn(RegValue + 1,ZERO);
    break;

   case TAG_EMPTY:
    FillIn(RegValue + 1,EMPTY);
    break;
  }

  /***************************************************************************/
  /* - highlight the top-of-stack                                            */
  /***************************************************************************/
  if(PhysRegStackTop == i )
  {
    Buffer[0]   = Attrib(vaRegTogg);
    RegValue[0] = Attrib(vaRegTogg);
  }
  else
  {
    Buffer[0]   = Attrib(vaRegWind);
    RegValue[0] = Attrib(vaRegWind);
  }

  memset(PhysRegIndex, 0, sizeof(PhysRegIndex) );
  PhysRegIndex[0] = Attrib(vaRegWind);
  PhysRegIndex[1] = HexDig[i];

  putrc( p->row+VioStartOffSet, Start_Col-2, PhysRegIndex );
  putrc( p->row+VioStartOffSet, Start_Col ,  RelativeRegString[i] );
  putrc( p->row+VioStartOffSet, Start_Col+4, RegValue );

  /***************************************************************************/
  /* - display a blank or the floating register value in real format.        */
  /***************************************************************************/
  if (TagValue == TAG_EMPTY)
  {
   putrc( p->row+1+VioStartOffSet, Start_Col , Blank );
  }
  else
  {
   /************************************************************************/
   /* - Store the value of long double from co proceesor into a local var  */
   /* - Call sprintf.                                                      */
   /* - stuff in the spaces at the end of the buffer.                      */
   /************************************************************************/
   Value387 = *(long double*)&PhysRegValue[i];
   Len = sprintf(Buffer+1,"%+1.18LE",Value387);
   while (Len < PRINTLEN)
     Buffer[Len++ + 1] = SPACE;
   putrc( p->row+1+VioStartOffSet, Start_Col , Buffer );
  }
 }

 /****************************************************************************/
 /* Move the statuswords bit pattern into a temp array and display it        */
 /****************************************************************************/
 for (x = 0,y = 16; x < 16; x++, y--)
    BitString[y] = (UCHAR)'0' + (UCHAR)((coproc_regs.StatusWord>>x) & 1);
 putrc( StatusRow+VioStartOffSet, Start_Col+9, BitString );

 /****************************************************************************/
 /* Move the Controlwords bit pattern into a temp array and display it       */
 /****************************************************************************/
 for (x = 0,y = 16; x < 16; x++, y--)
    BitString[y] = (UCHAR)'0' + (UCHAR)((coproc_regs.ControlWord>>x) & 1);
 putrc( ControlRow+VioStartOffSet, Start_Col+9, BitString );

 /****************************************************************************/
 /* Set the screen bounds.                                                   */
 /****************************************************************************/
 memset(BoundPtr+VioStartOffSet, VideoCols-COREGSWINWIDTH, COREGSWINLENGTH);
}

/*****************************************************************************/
/* Converttox()                                                              */
/*                                                                           */
/* Description:                                                              */
/*      Convert an 80 bit real to an hex string.                             */
/* Parameters:                                                               */
/*      Input  -> to input value                                             */
/*      x      -> to Hex String                                              */
/* Return:                                                                   */
/*      none                                                                 */
/* Assumptions:                                                              */
/*      none                                                                 */
/*****************************************************************************/
void Converttox(UCHAR *Input,UCHAR *x)
{
    UINT n;
    UINT IpIndex;
    UINT OpIndex;
    UCHAR Temp;
    OpIndex = 20;
    IpIndex = 0;

    for( n=0 ; n < 10 ; n++)
    {
        Temp = Input[IpIndex++];
        x[OpIndex--] = HexDig[Temp & 0xF];
        Temp >>= 4;
        x[OpIndex--] = HexDig[Temp & 0xF];
        if (OpIndex == (UINT)4)
          x[OpIndex--] = SPACE;
    }
}

/*****************************************************************************/
/* FillIn()                                                                  */
/*                                                                           */
/* Description:                                                              */
/*      Fills a string with given character.                                 */
/* Parameters:                                                               */
/*      Input  -> to input string.                                           */
/*      x      character to copy.                                            */
/* Return:                                                                   */
/*      none                                                                 */
/* Assumptions:                                                              */
/*      none                                                                 */
/*****************************************************************************/
void FillIn(UCHAR *Input,UCHAR x)
{
    UINT n;
    UINT OpIndex = 0;

    for( n=0 ; n < 20 ; n++)
    {
        Input[OpIndex++] = x;
        if (OpIndex == (UINT)4)
          Input[OpIndex++] = SPACE;
    }
}
#ifdef MSH
void showCoRegs(WINDOW *win)
{
 struct coregs  *p;
 UCHAR  Attribute;
 UINT   x,y;
 USHORT TagValue;
 UCHAR  Buffer[PRINTLEN+2];
 long double Value387;
 int    Len;
 APIRET rc;

 int    i,j;
 USHORT tag[8];
 char   DosDebugRegValue[8][23];
 char   PhysRegValueString[8][23];
 char   PhysRegValue[8][10];
 char   RelativeRegString[8][4];
 USHORT PhysRegStackTop;
 UCHAR  PhysRegIndex[8];

 char   **screen=(char **)win->user_data;
 --screen;
 /****************************************************************************/
 /* - Get the coprocessor registers.                                         */
 /****************************************************************************/
 memset( &coproc_regs, 0, sizeof(COPROCESSORREGS) );

 rc = xGetCoRegs(&coproc_regs);
 if( rc != 0 )
 {
  SayMsgBox2("CoProcessor Not Loaded", 800UL);
  Reg_Display = 0;
  return;
 }

 /****************************************************************************/
 /* - Define tag fields. See 80387 reference.                                */
 /****************************************************************************/
 tag[0] = (coproc_regs.TagWord & 0x0003 );
 tag[1] = (coproc_regs.TagWord & 0x000C );
 tag[2] = (coproc_regs.TagWord & 0x0030 );
 tag[3] = (coproc_regs.TagWord & 0x00C0 );
 tag[4] = (coproc_regs.TagWord & 0x0300 );
 tag[5] = (coproc_regs.TagWord & 0x0C00 );
 tag[6] = (coproc_regs.TagWord & 0x3000 );
 tag[7] = (coproc_regs.TagWord & 0xC000 );

 /****************************************************************************/
 /* - Convert the coprocessor register values to displayable hex strings.    */
 /****************************************************************************/
 memset( DosDebugRegValue, 0 , sizeof(DosDebugRegValue) );
 Converttox( (UCHAR*)(&coproc_regs.Stack[0]), DosDebugRegValue[0]);
 Converttox( (UCHAR*)(&coproc_regs.Stack[1]), DosDebugRegValue[1]);
 Converttox( (UCHAR*)(&coproc_regs.Stack[2]), DosDebugRegValue[2]);
 Converttox( (UCHAR*)(&coproc_regs.Stack[3]), DosDebugRegValue[3]);
 Converttox( (UCHAR*)(&coproc_regs.Stack[4]), DosDebugRegValue[4]);
 Converttox( (UCHAR*)(&coproc_regs.Stack[5]), DosDebugRegValue[5]);
 Converttox( (UCHAR*)(&coproc_regs.Stack[6]), DosDebugRegValue[6]);
 Converttox( (UCHAR*)(&coproc_regs.Stack[7]), DosDebugRegValue[7]);

 /****************************************************************************/
 /* - Define the physical register that contains the top of the stack.       */
 /****************************************************************************/
 PhysRegStackTop =  (USHORT)((coproc_regs.StatusWord >> 11) & 0x7);

 /****************************************************************************/
 /* - Now, associate the display hex strings with their physical registers.  */
 /*   We wiil be displaying physical registers from 0-7.                     */
 /* - Define the physical display hex value. This will be converted by       */
 /*   sprintf to a displayable real value.                                   */
 /* - Define the logical register index associated with a physical register. */
 /****************************************************************************/
 memset( PhysRegValueString, 0 , sizeof(PhysRegValueString) );
 i = PhysRegStackTop;
 for( j=0; j<8; j++ )
 {
  strcpy(PhysRegValueString[i], DosDebugRegValue[j]);
  memcpy(PhysRegValue[i], &coproc_regs.Stack[j], sizeof(REG80BIT) );
  sprintf(RelativeRegString[i], "ST%c", HexDig[j] );

  (i==7)?i=0:i++;
 }

/*****************************************************************************/
/* The following block of code will printf the coprocessor registers.        */
/* Remove the ifdefs and build to get it to print.                           */
/*---------------------------------------------------------------------------*/
#if 0
 printf("\n\n");
 printf("\nControlWord %x ",coproc_regs.ControlWord);
 printf("\nReserved1   %x ",coproc_regs.Reserved1  );
 printf("\nStatusWord  %x ",coproc_regs.StatusWord );
 printf("\nReserved2   %x ",coproc_regs.Reserved2  );
 printf("\nTagWord     %x ",coproc_regs.TagWord    );
 printf("\nReserved3   %x ",coproc_regs.Reserved3  );
 printf("\nIpOffset    %x ",coproc_regs.IpOffset   );
 printf("\nCsSelector  %x ",coproc_regs.CsSelector );
 printf("\nReserved4   %x ",coproc_regs.Reserved4  );
 printf("\nOpOffset    %x ",coproc_regs.OpOffset   );
 printf("\nOpSelector  %x ",coproc_regs.OpSelector );
 printf("\nReserved5   %x ",coproc_regs.Reserved5  );

 printf("\n\nPhysical Register Stack Top = %d", PhysRegStackTop );
 printf("\n\nLogical Registers");
 for( i=0; i<8; i++ )
 {
  printf("\n%d, %s, ", i, DosDebugRegValue[i] );
 }

 printf("\n\nPhysical Registers");
 for( i=0; i<8; i++ )
 {
  int    bitval1;
  int    bitval2;

  bitval1 =  (tag[i] >> (i*2)  )&0x0001 ;
  bitval2 =  (tag[i] >> (i*2+1))&0x0001;

  printf("\n%d, %s, %1d%1d ", i, PhysRegValueString[i], bitval2, bitval1 );
 }
 fflush(0);
#endif
/*--end of print coprocessor registers--------------------------------------*/

 /****************************************************************************/
 /* - Release the screen bounds.                                             */
 /* - Display the register box + headers/footers.                            */
 /****************************************************************************/
 Blank[0]=Attrib(vaRegWind);
 memset(Blank+1, SPACE, sizeof(Blank)-2);
 memset(Buffer, 0, sizeof(Buffer) );
 memset(BoundPtr+VioStartOffSet,VideoCols,VideoRows-VioStartOffSet);
 VideoAtr  = vaRegWind;
#if 0
 if (TestBit(Reg_Display,REPAINT))
 {
  Vfmtbox( "",VioStartOffSet,
           VideoCols - COREGSWINWIDTH,COREGSWINLENGTH,COREGSWINWIDTH );

  ResetBit(Reg_Display,REPAINT);
  Status[0]   = Attrib(vaRegWind);
  Status1[0]  = Attrib(vaRegWind);
  Status2[0]  = Attrib(vaRegWind);
  Control[0]  = Attrib(vaRegWind);
  Control1[0] = Attrib(vaRegWind);
 }
#endif

 /****************************************************************************/
 /* Find the Toggle attribute of the existing color attribute for register   */
 /* window and put it in vaRegTogg position of videomap.                     */
 /****************************************************************************/
 Attribute            = VideoMap[vaRegWind];
 Attribute           ^= HIGHLIGHT;
 VideoMap[vaRegTogg]  = Attribute;

 /****************************************************************************/
 /* - Now, display the registers.                                            */
 /****************************************************************************/
 for( p=regs, i=0; i < 8 ; p++, i++)
 {
  TagValue  = tag[i] >> (i*2) ;
  TagValue &= 0x3;
  switch(TagValue)
  {
   case TAG_VALID:
   case TAG_INFIN:
    strcpy( RegValue + 1, PhysRegValueString[i] );
    break;

   case TAG_ZERO:
    FillIn(RegValue + 1,ZERO);
    break;

   case TAG_EMPTY:
    FillIn(RegValue + 1,EMPTY);
    break;
  }

  /***************************************************************************/
  /* - highlight the top-of-stack                                            */
  /***************************************************************************/
  if(PhysRegStackTop == i )
  {
    Buffer[0]   = Attrib(vaRegTogg);
    RegValue[0] = Attrib(vaRegTogg);
  }
  else
  {
    Buffer[0]   = Attrib(vaRegWind);
    RegValue[0] = Attrib(vaRegWind);
  }

  memset(PhysRegIndex, 0, sizeof(PhysRegIndex) );
  PhysRegIndex[0] = Attrib(vaRegWind);
  PhysRegIndex[1] = HexDig[i];

  strcpy( screen[p->row], PhysRegIndex );
  strcat( screen[p->row], " " );
  strcat( screen[p->row], RelativeRegString[i] );
  strcat( screen[p->row], " " );
  strcat( screen[p->row], RegValue );

  /***************************************************************************/
  /* - display a blank or the floating register value in real format.        */
  /***************************************************************************/
  if (TagValue == TAG_EMPTY)
  {
   strcpy( screen[p->row+1], "  " );
   strcpy( screen[p->row+1]+2, Blank );
  }
  else
  {
   /************************************************************************/
   /* - Store the value of long double from co proceesor into a local var  */
   /* - Call sprintf.                                                      */
   /* - stuff in the spaces at the end of the buffer.                      */
   /************************************************************************/
   Value387 = *(long double*)&PhysRegValue[i];
   Len = sprintf(Buffer+1,"  %+1.18LE",Value387);
   while (Len < PRINTLEN)
     Buffer[Len++ + 1] = SPACE;
   strcpy( screen[p->row+1], Buffer);

  }
 }

 /****************************************************************************/
 /* Move the statuswords bit pattern into a temp array and display it        */
 /****************************************************************************/
 for (x = 0,y = 16; x < 16; x++, y--)
    BitString[y] = (UCHAR)'0' + (UCHAR)((coproc_regs.StatusWord>>x) & 1);
   memset( screen[StatusRow], ' ', 80);
   strncpy( screen[StatusRow]+11, BitString,strlen(BitString));

 /****************************************************************************/
 /* Move the Controlwords bit pattern into a temp array and display it       */
 /****************************************************************************/
 for (x = 0,y = 16; x < 16; x++, y--)
    BitString[y] = (UCHAR)'0' + (UCHAR)((coproc_regs.ControlWord>>x) & 1);
   memset( screen[ControlRow], ' ', 80);
   strncpy( screen[ControlRow]+11, BitString,strlen(BitString));

 /****************************************************************************/
 /****************************************************************************/
  strncpy(screen[StatusRow   ], Status ,strlen(Status));
  strncpy(screen[ControlRow  ], Control , strlen(Control));

  memset( screen[StatusRow-1], ' ', 80);
  strncpy(screen[StatusRow-1 ]+10, Status2 , strlen(Status2));

  memset( screen[StatusRow-2], ' ', 80);
  strncpy(screen[StatusRow-2 ]+10, Status1 , strlen(Status1));

  memset( screen[ControlRow-1], ' ', 80);
  strncpy(screen[ControlRow-1]+10, Control1 ,strlen(Control1));
  for(i=79;i>=0;--i)
      if(screen[ControlRow-1][i]==' ')screen[ControlRow-1][i]='\0';else break;
  for(i=79;i>=0;--i)
      if(screen[ControlRow][i]==' ')screen[ControlRow][i]='\0';else break;
  for(i=79;i>=0;--i)
      if(screen[StatusRow][i]==' ')screen[StatusRow][i]='\0';else break;
  for(i=79;i>=0;--i)
      if(screen[StatusRow-1][i]==' ')screen[StatusRow-1][i]='\0';else break;
  for(i=79;i>=0;--i)
      if(screen[StatusRow-2][i]==' ')screen[StatusRow-2][i]='\0';else break;
}
#endif
