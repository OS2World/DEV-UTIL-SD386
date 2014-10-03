/*****************************************************************************/
/* File:                                              IBM INTERNAL USE ONLY  */
/*      showvreg.c                                                           */
/*                                                                           */
/* Description:                                                              */
/*      Vertical Display and Edit Registers' and Flags' contents.            */
/*                                                                           */
/* History:                                                                  */
/*                                                                           */
/*...Release 1.00 (Pre-release 107 11/13/91)                                 */
/*...                                                                        */
/*... 11/13/91  400   Srinivas  Vertical Register Display.                   */
/*...                                                                        */
/*...Release 1.00 (Pre-release 108 12/05/91)                                 */
/*...                                                                        */
/*... 01/28/92  508   Srinivas  Added Set Execution Line function.           */
/*... 02/11/92  518   Srinivas  Remove limitation on max no of screen rows.  */
/*... 02/12/92  521   Srinivas  Port to C-Set/2.                             */
/*...                                                                        */
/*...Release 1.00 (03/03/92)                                                 */
/*...                                                                        */
/*... 03/20/92  607   Srinivas  CRMA fixes.                                  */
/*...                                                                        */
/*...Release 1.01 (04/03/92)                                                 */
/*...                                                                        */
/*... 05/08/92  701   Srinivas  Cua Interface.                               */
/*... 09/16/93  901   Joe       Add code to handle resource interlock error. */
/***Includes******************************************************************/

#include "all.h"

/***External declarations*****************************************************/

extern uint          VideoCols;         /*                                   */
extern uint          VideoRows;         /* # of rows per screen           518*/
extern uchar         *VideoMap;         /*                                   */
extern uchar         VideoAtr;          /*                                   */
extern uchar         AppRegsZapped;     /* Flag to say 'Regs Edited'         */
extern PtraceBuffer  AppPTB;            /* Appln Process Trace Buffer        */
extern uchar        *BoundPtr;          /* -> to screen bounds            518*/
extern uchar         Reg_Display;       /*                                   */
extern AFILE        *fp_focus;          /*                                521*/
extern uint          VioStartOffSet;    /* flag to tell were to start screen */
                                        /* display.                       701*/
static UINT          RegChgMask;        /*                                   */

static struct rfsv32 {                  /* 32-bit regs                       */
  uchar rn32[4];                        /* register name                     */
  uchar row;                            /* screen row                        */
  uchar ndx;                            /* index into userregs               */
} rfsv32[] = {                          /* the array of regs.                */
  { {'E','A','X',' '},1, 0},            /* Reg. EAX, row, its index          */
  { {'E','B','X',' '},2, 3},            /* Reg. EBX, row, its index          */
  { {'E','C','X',' '},3, 1},            /* Reg. ECX, row, its index          */
  { {'E','D','X',' '},4, 2},            /* Reg. EDX, row, its index          */
  { {'E','S','I',' '},5, 6},            /* Reg. ESI, row, its index          */
  { {'E','D','I',' '},6, 7},            /* Reg. EDI, row, its index          */
  { {'E','S','P',' '},7, 4},            /* Reg. ESP, row, its index          */
  { {'E','B','P',' '},8, 5},            /* Reg. EBP, row, its index          */
  { {'E','I','P',' '},9, 9},            /* Reg. EIP, row, its index          */
  { { 0 , 0 , 0 , 0 },0, 0}             /*                                   */
};                                      /*                                   */
                                        /*                                   */
static struct rfsv16 {                  /* 16-bit regs                       */
  uchar rn16[2];                        /* register name                     */
  uchar row;                            /* screen row                        */
  uchar ndx;                            /* index into userregs               */
} rfsv16[] = {                          /* the array of regs.                */
  { {'D','S'},10,06},                   /* Reg. DS, row, its index           */
  { {'E','S'},11,12},                   /* Reg. ES, row, its index           */
  { {'C','S'},12,00},                   /* Reg. CS, row, its index           */
  { {'S','S'},13,30},                   /* Reg. SS, row, its index           */
  { {'F','S'},14,18},                   /* Reg. FS, row, its index           */
  { {'G','S'},15,24},                   /* Reg. GS, row, its index           */
  { { 0 , 0 },00,00}                    /*                                   */
};                                      /*                                   */
                                        /*                                   */
static uchar regname[5];                /* register name                     */
static uchar x4[6] =                    /* parm for fn to convert 16bit to   */
  { Attrib(vaRegWind)  };               /* hex                               */
                                        /*                                   */
static uchar x8[10] =                   /* parm for fn to convert 32bit to   */
  { Attrib(vaRegWind) };                /* hex                               */
                                        /*                                   */
static struct ffs {                     /*                                   */
  uchar fn[2];                          /* flag name                         */
  uchar row;                            /* screen row                        */
  uchar col;                            /* screen col                        */
  uchar Nsh;                            /* shift count                       */
} ffs[] = {                             /* the array of flags                */
  { {'C','='}, 16,FC1, 0},              /* CARRY flag, pos, shift count      */
  { {'P','='}, 17,FC2, 2},              /* PARITY flg, pos, shift count      */
  { {'A','='}, 18,FC0, 4},              /* ACARRY flg, pos, shift count      */
  { {'Z','='}, 16,FC0, 6},              /* ZERO  flag, pos, shift count      */
  { {'S','='}, 16,FC2, 7},              /* SIGN  flag, pos, shift count      */
  { {'D','='}, 17,FC1,10},              /* DIRECTION flg, pos, shift count   */
  { {'O','='}, 17,FC0,11},              /* OVERFLOW flg, pos, shift count    */
  { { 0 , 0 }, 0, 0 ,  0}               /*                                   */
};                                      /*                                   */
                                        /*                                   */
static uchar flagvalue[3] =             /* flag value to display             */
  { Attrib(vaRegWind) };                /*                                   */
                                        /*                                   */
static uint FlagMask[] = {              /*                                   */
  ZERO_FLAG, CARRY_FLAG, SIGN_FLAG,     /* array of masks for all the flags  */
   OVERFLOW_FLAG, DIRECTION_FLAG,       /*                                   */
   PARITY_FLAG, ACARRY_FLAG, 0 };       /*                                   */
                                        /*                                   */
static uchar r16lolite[] =              /* param for PUTRC to display 16bit  */
  { RepCnt(4), Attrib(vaRegWind), 0 };  /* reg value with no highlighting    */
                                        /*                                   */
static uchar r16hilite[] =              /* param for PUTRC to display 16bit  */
  { RepCnt(4), Attrib(vaStgPro), 0 };   /* reg value highlighted             */
                                        /*                                   */
static uchar r32lolite[] =              /* param for PUTRC to display 32bit  */
  { RepCnt(8), Attrib(vaRegWind), 0 };  /* reg value with no highlighting    */
                                        /*                                   */
static uchar r32hilite[] =              /* param for PUTRC to display 32bit  */
  { RepCnt(8), Attrib(vaStgPro), 0 };   /* reg value highlighted             */
                                        /*                                   */
static uchar flolite[] =                /* param for PUTRC to display flag   */
  { RepCnt(1), Attrib(vaRegWind), 0 };  /* value with no highlighting        */
                                        /*                                   */
static uchar fhilite[] =                /* param for PUTRC to display flag   */
  { RepCnt(1), Attrib(vaStgPro), 0 };   /* value highlighted                 */

static PtraceBuffer  OldPTB;            /* Old Appln Process Trace Buffer    */
/*****************************************************************************/
/* ShowvRegs()                                                               */
/*                                                                           */
/* Description:                                                              */
/*      Vertical Display the registers' and flags' contents.                 */
/* Parameters:                                                               */
/*      none                                                                 */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*      none                                                                 */
/*****************************************************************************/
void ShowvRegs()
{
 uint  *userregs;                       /* user regs in the PTBuffer         */
 uint   Start_Col;                      /* column where to start display     */
 struct rfsv32 *p;                      /* -> to 32bit registers             */
 struct rfsv16 *q;                      /* -> to 16bit segment registers     */
 struct ffs *r;                         /* -> to flags                       */
 ushort flags;                          /* Holds the flags from AppPTB       */
 uchar  Attribute;                      /* attribute to hold reg win highlit */
 uint   Mask;                           /* mask position to check bits in    */
                                        /* RegChgMask                        */
 /****************************************************************************/
 /* Release the screen bounds.                                               */
 /* If it is the 1st invocation paint the whole register window Box.         */
 /****************************************************************************/
 memset(BoundPtr+VioStartOffSet,VideoCols,VideoRows-VioStartOffSet);
 Start_Col = VideoCols - REGSWINWIDTH + 2;
 Mask = 0;
 VideoAtr  = vaRegWind;
 if (TestBit(Reg_Display,REPAINT))
 {
   Vfmtbox( "",VioStartOffSet,                                          /*701*/
            VideoCols - REGSWINWIDTH,REGSWINLENGTH,REGSWINWIDTH );
   ResetBit(Reg_Display,REPAINT);
 }

#if 0
 MouseRect.row  = VioStartOffSet;                                       /*701*/
 MouseRect.col  = VideoCols - REGSWINWIDTH;                             /*701*/
 MouseRect.cRow = MouseRect.row + REGSWINLENGTH - 1;                    /*701*/
 MouseRect.cCol = MouseRect.col + REGSWINWIDTH - 1;                     /*701*/
 SetCollisionArea( &MouseRect );                                        /*701*/
#endif

 /****************************************************************************/
 /* Find the Toggle attribute of the existing color attribute for register   */
 /* window and put it in vaRegTogg position of videomap.                     */
 /****************************************************************************/
 Attribute = VideoMap[vaRegWind];
 Attribute ^= HIGHLIGHT;
 VideoMap[vaRegTogg] = Attribute;

 /****************************************************************************/
 /*  Get pointer to the register in AppPTB, loop for all 32bit registers.    */
 /*  Check the register change mask to determine if the attribute has to be  */
 /*  changed for the value being displayed.                                  */
 /****************************************************************************/
 userregs = (uint *)&(AppPTB.EAX);
 p = rfsv32;
 *(fourc *)regname = *(fourc *) p->rn32;
 for(; *(fourc *)regname ; ++p, Mask++, *(fourc *)regname = *(fourc *) p->rn32)
 {
    utox8( *(userregs + p->ndx ), x8 + 1 );
    putrc( p->row + VioStartOffSet, Start_Col, regname );               /*701*/
    if (TestBit(RegChgMask,Mask))
      x8[0] = Attrib(vaRegTogg);
    else
      x8[0] = Attrib(vaRegWind);
    putrc( p->row + VioStartOffSet, Start_Col + 4, x8 );                /*701*/
 }

 /****************************************************************************/
 /*  Get pointer to the segment registers in AppPTB, loop for all seg regs.  */
 /*  Check the register change mask to determine if the attribute has to be  */
 /*  changed for the value being displayed.                                  */
 /****************************************************************************/
 userregs = (uint *)&(AppPTB.CS);
 q = rfsv16;
 *(twoc *)regname = *(twoc *) q->rn16;
 for( ; *(twoc *)regname ; ++q, Mask++,*(twoc *)regname = *(twoc *) q->rn16 )
 {
    utox4( *((ushort *)userregs + q->ndx ), x4 + 1 );
    putrc( q->row + VioStartOffSet, Start_Col, regname );               /*701*/
    if (TestBit(RegChgMask,Mask))
      x4[0] = Attrib(vaRegTogg);
    else
      x4[0] = Attrib(vaRegWind);
    putrc( q->row + VioStartOffSet, Start_Col + 4, x4 );                /*701*/
 }

 /****************************************************************************/
 /* Get the flags from the AppPTB, Shift out each one by one from the flags. */
 /* The shift is indicated by the shift count for each of the flags in the   */
 /* flag structure.                                                          */
 /* Check the register change mask to determine if the attribute has to be   */
 /* changed for the value being displayed.                                   */
 /****************************************************************************/
 flags = ( ushort )AppPTB.EFlags;
 r = ffs;
 *(twoc *)regname = *(twoc *) r->fn;
 for( ; *(twoc *)regname ; ++r, Mask++,*(twoc *)regname = *(twoc *) r->fn )
 {
    flagvalue[1] = (uchar)'0' + (uchar)((flags>>r->Nsh) & 1);
    putrc( r->row + VioStartOffSet, Start_Col + r->col , regname );     /*701*/
    if (TestBit(RegChgMask,Mask))
      flagvalue[0] = Attrib(vaRegTogg);
    else
      flagvalue[0] = Attrib(vaRegWind);
    putrc( r->row + VioStartOffSet, Start_Col + r->col + 2,flagvalue);  /*701*/
 }

 /****************************************************************************/
 /* Set the screen bounds.                                                   */
 /****************************************************************************/
 memset(BoundPtr+VioStartOffSet,VideoCols-REGSWINWIDTH,REGSWINLENGTH);  /*518*/
                                                                        /*701*/
}

/*****************************************************************************/
/* GetScrAccess()                                                            */
/*                                                                           */
/* Description:                                                              */
/*      Gets the access to write to screen.                                  */
/* Parameters:                                                               */
/*      none                                                                 */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*      none                                                                 */
/*****************************************************************************/
void GetScrAccess()
{
  if ( (TestBit(Reg_Display,REGS386BIT)) ||
       (TestBit(Reg_Display,REGS387BIT)) )
  {
    memset(BoundPtr+VioStartOffSet,VideoCols,VideoRows-VioStartOffSet); /*701*/
  }
}

/*****************************************************************************/
/* SetScrAccess()                                                            */
/*                                                                           */
/* Description:                                                              */
/*      Sets the bounds to prevent writing to screen.                        */
/* Parameters:                                                               */
/*      none                                                                 */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*      none                                                                 */
/*****************************************************************************/
void SetScrAccess()
{
  if (TestBit(Reg_Display,REGS386BIT))
  {
    memset(BoundPtr+VioStartOffSet,
           VideoCols-REGSWINWIDTH,REGSWINLENGTH);                /*518*//*701*/
  }
  if (TestBit(Reg_Display,REGS387BIT))
  {
    memset(BoundPtr+VioStartOffSet,
           VideoCols-COREGSWINWIDTH,COREGSWINLENGTH);            /*518*//*701*/
  }
}

/*****************************************************************************/
/* KeyvRegs()                                                                */
/*                                                                           */
/* Description:                                                              */
/*      Display the registers' and flags' contents.                          */
/* Parameters:                                                               */
/*      none                                                                 */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*      none                                                                 */
/*****************************************************************************/

AFILE *KeyvRegs(uint *key)
{
 uint   n;
 uint   OldSS, OldSP, OldBP;
 uchar  flagvals[16][2], *mbuf;
 char  *Ivtable[7];
 ULONG  execaddr = 0;
 AFILE *execfp;

 /****************************************************************************/
 /* save curretn stack frame registers SS , ESP and EBP                      */
 /****************************************************************************/
 OldSS = AppPTB.SS;
 OldSP = AppPTB.ESP;
 OldBP = AppPTB.EBP;

 /****************************************************************************/
 /* Get the current values of flags into an array to be used for the help    */
 /* window for flags.                                                        */
 /****************************************************************************/
 memset(flagvals,0,sizeof(flagvals));
 for( n=0 ; FlagMask[n] ; ++n )
     flagvals[n][0] = (uchar)((AppPTB.EFlags & FlagMask[n]) ? '1' : '0');

 /****************************************************************************/
 /* Fill the buffer to be used for displaying help info for flags            */
 /****************************************************************************/

 for(n=0;n<7;n++)
  Ivtable[n] = &flagvals[n][0];
 mbuf = GetHelpMsg(HELP_REGS_FLAGS, Ivtable,7);

 /****************************************************************************/
 /* Display the help info for flags and let use edit registers and flags     */
 /* Set flag to indicate that registers are editied.                         */
 /****************************************************************************/
 VideoAtr = vaRegWind;
 *key = ShowHelpBox(mbuf,(UINTFUNC)ZapvRegs,NULL,NULL);
 Tfree( (void*)mbuf);                                                    /*521*/
 AppRegsZapped = TRUE;

 /****************************************************************************/
 /* - write the registers to esp.                                            */
 /****************************************************************************/
 xWriteRegs( &execaddr,&AppPTB);
 if( execaddr != GetExecAddr() )
 {
  SetExecAddr(execaddr);
  SetExecValues( GetExecTid(), FALSE );
 }

 /****************************************************************************/
 /* If any of the stack registers or CS IP have changed, establish the       */
 /* necessary arrays for the current state of users stack.                   */
 /****************************************************************************/
 if ( OldSS != AppPTB.SS || OldSP != AppPTB.ESP|| OldBP != AppPTB.EBP )
    SetActFrames();

 /****************************************************************************/
 /* Following the changes set the flag to show the current executing line    */
 /****************************************************************************/
 execfp = SetExecfp();
 if ( execfp != NULL )
 {
  execfp->flags |= AF_ZOOM;
  return( execfp );
 }

 return(fp_focus);                      /* return the current fp which    521*/
}

/*****************************************************************************/
/* ZapvRegs()                                                                */
/*                                                                           */
/* Description:                                                              */
/*      Let the user edit the registers and flags.                           */
/* Parameters:                                                               */
/*      none                                                                 */
/* Return:                                                                   */
/*      key code of the last key.                                            */
/* Assumptions:                                                              */
/*      none                                                                 */
/*****************************************************************************/
uint ZapvRegs()
{
 uchar   regval16[5];                  /* field for reg16 edited values     */
 uchar   regval32[9];                  /* field for reg32 edited values     */
 uchar   flagval[2];                   /* field for flags edited values     */
 uint    mode;                         /* what type of field we are in      */
 struct  rfsv16 *p16;                  /* -> array of 16-bit reg struct     */
 struct  rfsv32 *p32;                  /* -> array of 32-bit reg struct     */
 struct  ffs *q;                       /* -> array of flag struct           */
 uint    fldlen;                       /* length of field being edited      */
 uint    flagmask;                     /* mask for updating flag in AppPTB  */
 uint    bitpos;                       /* bit position of edited flag       */
 int     csrrow;                       /* cursor row position               */
 int     csrcol;                       /* cursor col position               */
 uint    Start_Col;                    /* column where to start display     */
 uint   *userregs32;                   /* -> to 32bit regs in AppPTB        */
 ushort *userregs16;                   /* -> to 16bit seg regs in AppPTB    */
 uint   *flags;                        /* -> to flags of AppPTB structure   */
 uint    n, key, csroff;               /*                                   */

 /***************************************************************************/
 /* Initialise the pointers to the AppPTB structure.                        */
 /***************************************************************************/
 userregs32 = (uint *)&(AppPTB.EAX);   /* cast it to uint *              521*/
 userregs16 = (ushort *)&(AppPTB.CS);
 flags      = (uint *)&(AppPTB.EFlags);/* cast it to uint *              521*/

 /***************************************************************************/
 /* Initialise the variables.                                               */
 /***************************************************************************/
 csrcol    = 0;
 csrrow    = 1 + VioStartOffSet;                                       /*701*/
 Start_Col = VideoCols - REGSWINWIDTH + 2;

 for(;;)
 {
   /**************************************************************************/
   /* Depending on the cursor row set the mode which indicates what we are   */
   /* editing.                                                               */
   /**************************************************************************/
   if ((csrrow >= (1 + VioStartOffSet)) &&                              /*701*/
       (csrrow <= (9 + VioStartOffSet)))                                /*701*/
      mode = REG32MODE;
   else
   {
      if ((csrrow >= (10 + VioStartOffSet)) &&                          /*701*/
          (csrrow <= (15 + VioStartOffSet)))                            /*701*/
         mode = REG16MODE;
      else
         mode = FLAGSMODE;
   }
   csroff = 0;

   /**************************************************************************/
   /* Release the screen bounds. Clear the csrcol field, if we are not in    */
   /* flags mode as it does not make any sense when we are in registers mode */
   /**************************************************************************/
   memset(BoundPtr + VioStartOffSet,VideoCols,VideoRows-VioStartOffSet);/*701*/
   if (mode != FLAGSMODE)
      csrcol = 0;

   /**************************************************************************/
   /* If a flag is to be edited, hilite it and init required fields.         */
   /*  - find the flag to be edited                                          */
   /*  - hilite the flag field                                               */
   /*  - set up the field length, shift count and mask for updating flags    */
   /*  - get the key from the KEYSTR function and value into flagval         */
   /*  - if the key is not ESC key and if it is not a blank field update     */
   /*    the flags value                                                     */
   /*  - remove the hilite from the flag field                               */
   /**************************************************************************/
   if (mode == FLAGSMODE)
   {
     for( q = ffs, n = 0; n++ < NOFREGS; ++q )
       if ((csrrow == (q->row + VioStartOffSet)) && (csrcol == q->col))/*701*/
          break;

     putrc(csrrow, Start_Col+q->col + 2, fhilite);

     fldlen   = 1;
     bitpos   = q->Nsh;
     flagmask = 1;

     key = GetString( csrrow, Start_Col + q->col + 2, fldlen, fldlen,
                      &csroff, flagval, BINKEYFLD,NULL );

     if ((key != ESC) && (strlen(flagval) != 0))
     {
       if (atou(flagval) == 0)
         *flags  = (*flags) & ~(flagmask << bitpos);
       else
         *flags |= (flagmask << bitpos);
     }

     putrc(csrrow,Start_Col+q->col+2, flolite);
   }

   /**************************************************************************/
   /* If a 32bit register is to be edited, hilite it and init req fields.    */
   /*  - find the 32bit register to be edited                                */
   /*  - hilite the register field                                           */
   /*  - set up the field length                                             */
   /*  - get the key from the KEYSTR function and value into regval32        */
   /*  - if the key is not ESC key and if it is not a blank field update     */
   /*    the register value                                                  */
   /*  - remove the hilite from the 32bit register field                     */
   /**************************************************************************/
   if ( mode == REG32MODE )
   {
      for( p32 = rfsv32, n = 0; n++ < NO32REGS; ++p32 )
        if (csrrow == (p32->row + VioStartOffSet))                      /*701*/
           break;

      putrc(csrrow, Start_Col+4, r32hilite);
      fldlen = 8;

      key = GetString( csrrow, Start_Col + 4, fldlen, fldlen,
                       &csroff, regval32, HEXKEYFLD,NULL);

      if ((key != ESC) && (strlen(regval32) != 0))
        x8tou( regval32, userregs32 + p32->ndx );

      putrc(csrrow, Start_Col+4, r32lolite);
   }

   /**************************************************************************/
   /* If a 16bit register is to be edited, hilite it and init req fields.    */
   /*  - find the 16bit register to be edited                                */
   /*  - hilite the register field                                           */
   /*  - set up the field length                                             */
   /*  - get the key from the KEYSTR function and value into regval16        */
   /*  - if the key is not ESC key and if it is not a blank field update     */
   /*    the register value                                                  */
   /*  - remove the hilite from the 16bit register field                     */
   /**************************************************************************/
   if ( mode == REG16MODE )
   {
      for( p16 = rfsv16, n = 0; n++ < NO16REGS; ++p16 )
         if (csrrow == (p16->row + VioStartOffSet))                     /*701*/
            break;

      putrc(csrrow, Start_Col+4, r16hilite);
      fldlen = 4;

      key = GetString( csrrow, Start_Col + 4, fldlen, fldlen,
                       &csroff, regval16, HEXKEYFLD,NULL );

      if ((key != ESC) && (strlen(regval16) != 0))
        x4tou( regval16,userregs16 + p16->ndx);

      putrc(csrrow, Start_Col+4, r16lolite);
   }

   switch (key)
   {
     case TAB:
     case RIGHT:
     case DATAKEY:
       if (mode == FLAGSMODE)           /* If in FLAGSMODE incr column       */
          csrcol += FCincr;             /*                                   */
       if (csrrow == (REGSWINLENGTH - 2 + VioStartOffSet))              /*701*/
                                        /* If on last row incr column such   */
          csrcol = 12;                  /* that we go back to top.           */
       if ((mode != FLAGSMODE)          /* If we are in register mode or     */
           || (csrcol > 8))             /* colunm is greater than 8 , treat  */
       {                                /* the keys as down key.             */
         if (csrcol > 8)                /* If column is > 8 wrap around to   */
            csrcol = 0;                 /* 1st column.                       */
         goto casedown;
casedown:
     case DOWN:                         /*                                   */
         if (csrrow == (REGSWINLENGTH - 2 + VioStartOffSet))            /*701*/
                                        /* Is it last row                    */
            csrrow = 1 + VioStartOffSet;/*  - yes go back to top          701*/
         else                           /*                                   */
            csrrow += 1;                /*  - no bump to next row            */
       }                                /*                                   */
       break;

     case LEFT:
     case S_TAB:
       if (mode == FLAGSMODE)           /* If in FLAGSMODE decr column       */
          csrcol -= FCincr;             /*                                   */
       if ((mode != FLAGSMODE)          /* If we are in register mode or     */
           || (csrcol < 0))             /* colunm is smaller than 0 , treat  */
       {                                /* the keys as up key.               */
         if (csrcol < 0)                /* If column is < 0 wrap around to   */
            csrcol = 8;                 /* last column.                      */
         goto caseup;
caseup:
     case UP:                           /*                                   */
         if (csrrow == (1 + VioStartOffSet))/* Is it first row            701*/
             csrrow = REGSWINLENGTH - 2 + VioStartOffSet;               /*701*/
                                        /*  - yes go back to last            */
         else                           /*                                   */
             csrrow -= 1;               /*  - no bump to previous row        */
       }                                /*                                   */
       break;

     case ENTER:
     case ESC:
     case PADPLUS:
       ShowvRegs();
       return(key);

     case F1:
       Help(HELP_DLG_EDITREGS);  break;

     default:
       beep();
   }                                    /* end switch                        */
   ShowvRegs();                         /* display the regsisters            */
 }                                      /* end of forever loop               */
}                                       /* end ZapRegs                       */

/*****************************************************************************/
/* SetRegChgMask()                                                           */
/*                                                                           */
/* Description:                                                              */
/*      Sets the bits in the mask to indicate the register changed after     */
/*      any go.                                                              */
/* Parameters:                                                               */
/*      none                                                                 */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*      none                                                                 */
/*****************************************************************************/
void SetRegChgMask()
{
 ulong *userregs;                       /* user regs in the PTBuffer      521*/
 ulong *Oldregs;                        /* Old regs in the PTBuffer       521*/
 struct rfsv32 *p;                      /* -> to 32bit registers             */
 struct rfsv16 *q;                      /* -> to 16bit segment registers     */
 struct ffs *r;                         /* -> to flags                       */
 ushort Newflags;                       /* Holds the flags from AppPTB       */
 ushort Oldflags;                       /* Holds the flags from OldPTB       */
 uchar  NewflagValue;                   /* Holds the new flag value          */
 uchar  OldflagValue;                   /* Holds the Old flag value          */
 uint   Mask;                           /* mask to set the bits in RegChgmsk */

 RegChgMask = 0;
 Mask       = 0x1;
 /****************************************************************************/
 /*  Get pointer to the register in PTB, loop for all 32bit registers.       */
 /*  Set the bit in the mask if the value changes.                           */
 /****************************************************************************/
 userregs = &(AppPTB.EAX);
 Oldregs  = &(OldPTB.EAX);
 for( p = rfsv32; *(fourc *) p->rn32; ++p , Mask <<= 1)
 {
    if ( *(userregs + p->ndx) != *(Oldregs + p->ndx) )
       RegChgMask |= Mask;
 }

 /****************************************************************************/
 /*  Get pointer to the segment registers in PTB, loop for all seg regs.     */
 /*  Set the bit in the mask if the value changes.                           */
 /****************************************************************************/
 userregs = (ulong *)&(AppPTB.CS);      /* cast to ulong *                521*/
 Oldregs  = (ulong *)&(OldPTB.CS);      /* cast to ulong *                521*/
 for( q = rfsv16; *(twoc *)q->rn16; ++q,Mask <<= 1)
 {
    if ( *(userregs + p->ndx) != *(Oldregs + p->ndx) )
       RegChgMask |= Mask;
 }

 /****************************************************************************/
 /* Get the flags from the PTB, Shift out each one by one from the flags.    */
 /* The shift is indicated by the shift count for each of the flags in the   */
 /* flag structure.                                                          */
 /* Set the bit in the mask if the value changes.                            */
 /****************************************************************************/
 Newflags = ( ushort )AppPTB.EFlags;
 Oldflags = ( ushort )OldPTB.EFlags;
 for( r = ffs; *(twoc *)r->fn; ++r,Mask <<= 1)
 {
    NewflagValue = (uchar)'0' + (uchar)((Newflags>>r->Nsh) & 1);
    OldflagValue = (uchar)'0' + (uchar)((Oldflags>>r->Nsh) & 1);
    if ( NewflagValue != OldflagValue )
       RegChgMask |= Mask;
 }

 /****************************************************************************/
 /* Update the old set of register values with the new set of values         */
 /****************************************************************************/
 OldPTB = AppPTB;
}

/*****************************************************************************/
/* SetExecLine()                                                          508*/
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Updates the CS:IP and all related global variables.                     */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   NewIp   -> New Instruction Pointer.                                     */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   TRUE    success                                                         */
/*   FALSE   failure                                                         */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*      none                                                                 */
/*                                                                           */
/*****************************************************************************/
uint SetExecLine(uint NewIp)
{
 APIRET  rc;

 /****************************************************************************/
 /* - if the address didn't change don't do anything.                        */
 /****************************************************************************/
 if(NewIp == GetExecAddr() )
  return( TRUE );

 if( xSetExecAddr( NewIp ) )
  return(FALSE);

 /************************************************************************/
 /* - update the exec values after forcing an update of the AppPTB       */
 /*   buffer.                                                            */
 /************************************************************************/
 SetExecValues( GetExecTid(), TRUE );

 if(SetExecfp())
    rc = TRUE;
 else
    rc = FALSE;

 return(rc);
}
#ifdef MSH
void showvRegs(WINDOW *win)
{
 uint  *userregs;                       /* user regs in the PTBuffer         */
 struct rfsv32 *p;                      /* -> to 32bit registers             */
 struct rfsv16 *q;                      /* -> to 16bit segment registers     */
 struct ffs *r;                         /* -> to flags                       */
 ushort flags;                          /* Holds the flags from AppPTB       */
 uchar  Attribute;                      /* attribute to hold reg win highlit */
 uint   Mask;                           /* mask position to check bits in    */
                                        /* RegChgMask                        */
 char   **screen=(char **)win->user_data;
 --screen;
 /****************************************************************************/
 /* Release the screen bounds.                                               */
 /* If it is the 1st invocation paint the whole register window Box.         */
 /****************************************************************************/
 Mask = 0;
 VideoAtr  = vaRegWind;

 /****************************************************************************/
 /* Find the Toggle attribute of the existing color attribute for register   */
 /* window and put it in vaRegTogg position of videomap.                     */
 /****************************************************************************/
 Attribute = VideoMap[vaRegWind];
 Attribute ^= HIGHLIGHT;
 VideoMap[vaRegTogg] = Attribute;

 /****************************************************************************/
 /*  Get pointer to the register in AppPTB, loop for all 32bit registers.    */
 /*  Check the register change mask to determine if the attribute has to be  */
 /*  changed for the value being displayed.                                  */
 /****************************************************************************/
 userregs = (uint *)&(AppPTB.EAX);
 p = rfsv32;
 *(fourc *)regname = *(fourc *) p->rn32;
 for(; *(fourc *)regname ; ++p, Mask++, *(fourc *)regname = *(fourc *) p->rn32)
 {
    utox8( *(userregs + p->ndx ), x8 + 1 );
    memset(screen[p->row],' ',80);
    screen[p->row][0]=Attrib(vaRegWind);
    strcpy(screen[p->row]+1, regname );
    if (TestBit(RegChgMask,Mask))
      x8[0] = Attrib(vaRegTogg);
    else
      x8[0] = Attrib(vaRegWind);
    strcpy(screen[p->row]+5, x8 );                /*701*/
 }

 /****************************************************************************/
 /*  Get pointer to the segment registers in AppPTB, loop for all seg regs.  */
 /*  Check the register change mask to determine if the attribute has to be  */
 /*  changed for the value being displayed.                                  */
 /****************************************************************************/
 userregs = (uint *)&(AppPTB.CS);
 q = rfsv16;
 *(twoc *)regname = *(twoc *) q->rn16;
 for( ; *(twoc *)regname ; ++q, Mask++,*(twoc *)regname = *(twoc *) q->rn16 )
 {
    utox4( *((ushort *)userregs + q->ndx ), x4 + 1 );
    memset(screen[q->row],' ',80);
    screen[q->row][0]=Attrib(vaRegWind);
    strncpy(screen[q->row]+1, regname , 2);                         /*701*/
    if (TestBit(RegChgMask,Mask))
      x4[0] = Attrib(vaRegTogg);
    else
      x4[0] = Attrib(vaRegWind);
    strcpy(screen[q->row] + 5, x4 );                /*701*/
 }

 /****************************************************************************/
 /* Get the flags from the AppPTB, Shift out each one by one from the flags. */
 /* The shift is indicated by the shift count for each of the flags in the   */
 /* flag structure.                                                          */
 /* Check the register change mask to determine if the attribute has to be   */
 /* changed for the value being displayed.                                   */
 /****************************************************************************/
 flags = ( ushort )AppPTB.EFlags;
 r = ffs;
 *(twoc *)regname = *(twoc *) r->fn;
 for( ; *(twoc *)regname ; ++r,*(twoc *)regname = *(twoc *) r->fn )
 {
    memset(screen[r->row],' ',80);
    screen[r->row][0]=Attrib(vaRegWind);
 }

 r = ffs;
 *(twoc *)regname = *(twoc *) r->fn;
 for( ; *(twoc *)regname ; ++r, Mask++,*(twoc *)regname = *(twoc *) r->fn )
 {
    flagvalue[1] = (uchar)'0' + (uchar)((flags>>r->Nsh) & 1);
    strncpy(screen[r->row] + (5*r->col/4) + 1, regname, strlen(regname) );     /*701*/
    if (TestBit(RegChgMask,Mask))
      flagvalue[0] = Attrib(vaRegTogg);
    else
      flagvalue[0] = Attrib(vaRegWind);
    strncpy( screen[r->row]+(5*r->col/4)+3,flagvalue,strlen(flagvalue));  /*701*/
 }
 r = ffs;
 *(twoc *)regname = *(twoc *) r->fn;
 for( ; *(twoc *)regname ; ++r,*(twoc *)regname = *(twoc *) r->fn )
 {
  int i;
  for(i=79;i>=0;--i)
      if(screen[r->row][i]==' ')screen[r->row][i]='\0';else break;
 }
 PaintWindow(win);
}
#endif
