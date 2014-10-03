/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   viewasm.c                                                               */
/*                                                                           */
/* Description:                                                              */
/*   assembler display formatting routines.                                  */
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
/*... 07/09/91  213   srinivas  one byte memory operand problem.             */
/*... 08/30/91  235   Joe       Cleanup/rewrite ascroll() to fix several bugs*/
/*...                                                                        */
/*...Release 1.08 (Pre-release 108 )                                         */
/*...                                                                        */
/*... 01/17/92  504   Joe       Can't step into 16 bit dll on 6.177H.        */
/*...                                                                        */
/**Includes*******************************************************************/
                                        /*                                   */
#include "all.h"                        /* SD86 include files                */
                                        /*                                   */
/**Variables defined**********************************************************/
/*                                                                           */
/*                                                                           */
/**Externs********************************************************************/
/*                                                                           */
extern uint VideoCols;
extern INSTR    *icache;                /* +1 for a NULL terminating sel. 235*/
extern PtraceBuffer AppPTB;
extern CmdParms      cmd;
/*
/**Begin Code*****************************************************************/
/*                                                                           */
/*****************************************************************************/
/* buildasmline()                                                            */
/*                                                                           */
/* Description:                                                              */
/*   build disassembly text line.                                            */
/*                                                                           */
/* Parameters:                                                               */
/*   pbuf       -> buffer where text goes.                                   */
/*   pinstr     -> cache structure for this line.                            */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*  The cached instruction info is a valid instruction. It cannot be the     */
/*  terminating cache entry.                                                 */
/*                                                                           */
/*****************************************************************************/
#define ADDRSIZE 9+1                    /* addr field size for ssss:oooo or  */
                                        /*   oooooooo  format             108*/
#define HEXSIZE  15+1                   /* hex field size                 108*/
#define MNESIZE  MNEMBUFFSIZE+1         /* mnem field size                108*/
#define TXTSIZE  22+1                   /* text field size                   */
#define MEMSIZE  11                     /* memory reference field size.      */
#define DATACOL  45                     /* memory reference for side source. */
#define WALL     DATACOL + 11           /* vertical bar location.            */
#define VBAR     179                    /* vertical bar location.            */

 void
buildasmline(uchar *pbuf,INSTR *pinstr)
{
  uchar   *bp;                          /* ->start of line in text           */
  uchar  *cp;                           /*                                   */
  int     offset;                       /*                                   */
  uchar  *s;                            /* ->into line in formatted text     */
  ushort       Sel;                                                     /*607*/
  ushort       Off;                                                     /*607*/

    bp = pbuf;                          /* initialize first line start       */

/*****************************************************************************/
/*                                                                           */
/* Fill in the text buffer with Sel:Off data in format  XXXX:XXXXbb  .       */
/*                                                                           */
/*****************************************************************************/

    s = bp;                             /* start ->into text at line's edge  */
    if (pinstr->mod_type)               /* 32-bit instruction ?           108*/
     utox8(pinstr->instaddr, s);        /* 8-byte address as an offset    108*/
    else                                /* 16-bit instruction             108*/
    {
      Code_Flat2SelOff(pinstr->instaddr,&Sel,&Off);
      utox4(Sel,s);                     /* sel portion of ssss:oooo   607 504*/
      s += 4;                           /* bump past selector portion        */
      *s++ = ':';                       /* put in ':' and bump to offset     */
      utox4(Off,s);                     /* offset portion of ssss:ooo 607 504*/
    }
    offset = ADDRSIZE;

/*****************************************************************************/
/*                                                                           */
/* Fill in the hex instruction.                                              */
/*                                                                           */
/*****************************************************************************/

     s = bp + offset;
     strcpy( s,pinstr->hex);
     cp = s;
     while(*cp++ != '\0'){;}
     *--cp = ' ';
     offset += HEXSIZE;                                                 /*108*/

/*****************************************************************************/
/*                                                                           */
/* Fill in the mnemonic.                                                     */
/*                                                                           */
/*****************************************************************************/

    s = bp + offset;                    /* bump past offset and two blanks   */
    strcpy(s,pinstr->mne);
    cp = s;
    while(*cp++ != '\0'){;}
    *--cp = ' ';
    offset += MNESIZE;

/*****************************************************************************/
/*                                                                           */
/* Fill in the text.                                                         */
/*                                                                           */
/*****************************************************************************/

    s = bp + offset;
    strcpy(s,pinstr->text);
    while(*s++ != '\0'){;}
    *--s = ' ';
    offset += TXTSIZE;

/*****************************************************************************/
/*                                                                           */
/* Fill in the memory operand.                                               */
/*                                                                           */
/*****************************************************************************/

    s = bp + offset + 5;               /* pad line out to midline with ' 's */

    if( IsEspRemote() == FALSE )
     buildopnd( s , pinstr );

}

/*****************************************************************************/
/* buildopnd()                                                               */
/*                                                                           */
/* Description:                                                              */
/*   build operand part of the disassembly.                                  */
/*                                                                           */
/* Parameters:                                                               */
/*   s          -> to buffer location where operand is to be written.        */
/*   pinstr     -> cache structure containing the instruction info.          */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*  pinstr->instaddr != NULL.  This implies a termination cache entry.       */
/*                                                                           */
/*****************************************************************************/
  void
buildopnd(uchar *s,INSTR *pinstr)
{
  uchar   type;                         /*                                   */
  uchar  *appopptr;                     /* ->operand for current instruction */
  uint    off;

    type = pinstr->type;
    switch( type )
    {                                   /* begin work on operand information */
      case MEM8BIT:                     /* 8-bit memory operand ?            */
        *( s += 11) = '[';              /* right justify for 8-bit operand   */
        appopptr = grabopnd(            /* get a pointer to the operand hex  */
                             pinstr,    /*   using Toll's packet information */
                             1 );       /*  and indicate call is for 8 bits  */
        if( appopptr != NULL )
         utox2( *appopptr, ++s );       /* put in 8-bit operand           213*/
        else
         {++s;sprintf(s,"..");}
        *( s + 2 ) = ']';               /* left justify for 8-bit operand    */
        break;                          /* end 8-bit memory operand          */


      case MEM16OR32BIT:                /* 16-bit memory operand ?           */
        if (pinstr->OpSize)             /* 32-bit memory operand ?        108*/
        {
          *( s += 5 ) = '[';            /* right justify for operand         */
          appopptr = grabopnd(          /* get a pointer to the operand hex  */
                               pinstr,  /*   using Toll's packet information */
                               4 );     /*  and indicate call is for 32 bits */
          if( appopptr != NULL )
           utox8( *( uint* ) appopptr, ++s ); /* put in 32-bit operand    108*/
          else
           {++s;sprintf(s,"........");}
          *( s + 8 ) = ']';             /* left justify for 32-bit operand   */
        }
        else                            /* truly a 16-bit operand         108*/
        {
          *( s += 9 ) = '[';            /* right justify for 16-bit operand  */
          appopptr = grabopnd(          /* get a pointer to the operand hex  */
                               pinstr,  /*   using Toll's packet information */
                               2 );     /*  and indicate call is for 16 bits */
          if( appopptr != NULL )
           utox4( *( uint* ) appopptr, ++s ); /* put in 16-bit operand       */
          else
           {++s;sprintf(s,"....");}
          *( s + 4 ) = ']';             /* left justify for 16-bit operand   */
        }
        break;                          /* end 16-bit memory operand         */

      case MEM1616OR3216BIT:            /* 16:16 memory operand ?            */
        if (pinstr->OpSize)             /* 32-bit memory operand ?        108*/
        {
          *( s ) = '[';                 /* right justify for 16:16 operand   */
          appopptr = grabopnd(          /* get a pointer to the operand hex  */
                               pinstr,  /*   using Toll's packet information */
                               6 );     /*  and indicate call is for 16 bits */
          if( appopptr != NULL )
          {
           utox4( *(( USHORT* ) appopptr + 2), ++s ); /* put in 16-bit sel opnd*/
           *( s += 4 ) = ':';              /* put in colon separator         */
           utox8( *( uint* ) appopptr, ++s ); /* put in 32-bit offset operand*/
          }
          else
           {++s;sprintf(s,"....:........");}
          *( s + 8 ) = ']';             /* left justify for 16:16 operand    */
        }
        else                            /* truly 16-bit mem opnd             */
        {
          appopptr = grabopnd(pinstr,4);
          s+=4;
          sprintf(s,"[....:....]");
          if( appopptr != NULL )
          {
           utox4( *( (USHORT*)appopptr + 1), s+1);
           utox4( *(USHORT*)appopptr, s+6);
          }
        }
        break;                          /* end 16:16 memory operand          */

      case JMPIPDISP:                   /* jump to IP + displacement instr ? */
      case CALLIPDISP:                  /* call to IP + displacement instr ? */
        if (pinstr->mod_type)           /* 32-bit instruction ?           108*/
        {
          *( s +=5 ) = '[';             /* right justify for operand         */
          off=pinstr->instaddr+         /* offset of the instruction.        */
              (uint)pinstr->offset +    /* relative offset in instruction.   */
              (uint)pinstr->len;        /* adjustment for instruction length.*/
          utox8( off, ++s );            /* target EIP                        */
          *( s + 8 ) = ']';             /* left justify for 32-bit opnd   108*/
        }
        else                            /* 16-bit instruction             108*/
        {
          *( s+=4 ) = '[';              /* right justify for operand         */
          utox4( AppPTB.CS, ++s );      /* put in cs selector             101*/
          *( s += 4 ) = ':';            /* colon separator                   */
          off=LoFlat(pinstr->instaddr) + /* offset of the instruction.       */
              (uint)pinstr->offset +    /* relative offset in instruction.   */
              (uint)pinstr->len;        /* adjustment for instruction length.*/
          utox4( off , ++s);            /* put in target ip.                 */
          *( s + 4 ) = ']';             /* left justify for 16:16 operand    */
        }
        break;                          /* end jump/call to IP + disp instr  */

      case ILLEGINST:                   /* illegal instruction ?             */
      case NOOPND:                      /* no operand information ?          */
        break;                          /* end no operand information        */
    }                                   /* end work on operand information   */
}

/*****************************************************************************/
/* selreg                                                                    */
/*                                                                           */
/* Description:                                                              */
/*   returns a selector register value.                                      */
/*                                                                           */
/* Parameters:                                                               */
/*   retreg     Dave Toll's register code for the selector register.         */
/*                                                                           */
/* Return:                                                                   */
/*   selvalue   contents of the indicated selector register.                 */
/*                                                                           */
/*****************************************************************************/
  uint
selreg(uint retreg)                     /* register code for a selector reg  */
{                                       /* begin selreg                      */
  switch( retreg )
  {                                     /* begin deciphering register code   */
    case RETREGDS:                      /* selector is the DS register ?     */
      return( AppPTB.DS );              /* give the bugger the DS value   101*/

    case RETREGSS:                      /* selector is the SS register ?     */
      return( AppPTB.SS );              /* give the bugger the SS value   101*/

    case RETREGES:                      /* selector is the ES register ?     */
      return( AppPTB.ES );              /* give the bugger the ES value   101*/

    case RETREGCS:                      /* selector is the CS register ?     */
      return( AppPTB.CS );              /* give the bugger the CS value   101*/

    default:
      return(0);

  }                                     /* end deciphering register code     */

}                                       /* end selreg                        */

/*****************************************************************************/
/* BaseVal()                                                                 */
/*                                                                           */
/* Description:                                                              */
/*   returns a segment(for 16 bit) + base offset.                            */
/*                                                                           */
/* Parameters:                                                               */
/*   pinstr     structure describing the instruction from Dave Toll's        */
/*              disassembler.                                                */
/*                                                                           */
/* Return:                                                                   */
/*   base       segment + base.                                              */
/*                                                                           */
/*****************************************************************************/
ULONG BaseVal(INSTR *pinstr)
{                                       /* begin offreg                      */
 ULONG basevalue;
  switch( pinstr->base )
  {
    case RETREGAX:
    case RETREGBX:
    case RETREGCX:
    case RETREGDX:
    case RETREGBP:
    case RETREGSP:
    case RETREGSI:
    case RETREGDI:
base16:
     basevalue = selreg(pinstr->reg);
     basevalue = Data_SelOff2Flat( basevalue,0);
     switch( pinstr->base )
     {
      case RETREGAX:
        return( basevalue + (AppPTB.EAX&0x0000FFFF));

      case RETREGBX:
        return( basevalue + (AppPTB.EBX&0x0000FFFF));

      case RETREGCX:
        return( basevalue + (AppPTB.ECX&0x0000FFFF));

      case RETREGDX:
        return( basevalue + (AppPTB.EDX&0x0000FFFF));

      case RETREGBP:
        return( basevalue + (AppPTB.EBP&0x0000FFFF));

      case RETREGSP:
        return( basevalue + (AppPTB.ESP&0x0000FFFF));

      case RETREGSI:
        return( basevalue + (AppPTB.ESI&0x0000FFFF));

      case RETREGDI:
        return( basevalue + (AppPTB.EDI&0x0000FFFF));

      case RETREGNONE:
        return(basevalue);
     }
     break;

    case RETREGEAX:                     /* offset is the EAX register ?   108*/
      return( AppPTB.EAX );             /* give the bugger the EAX value  108*/

    case RETREGEBX:                     /* offset is the EBX register ?   108*/
      return( AppPTB.EBX);              /* give the bugger the EBX value  108*/

    case RETREGECX:                     /* offset is the ECX register ?   108*/
      return( AppPTB.ECX );             /* give the bugger the ECX value  108*/

    case RETREGEDX:                     /* offset is the EDX register ?   108*/
      return( AppPTB.EDX);              /* give the bugger the EDX value  108*/

    case RETREGEBP:                     /* offset is the EBP register ?   108*/
      return( AppPTB.EBP);              /* give the bugger the EBP value  108*/

    case RETREGESP:                     /* offset is the ESP register ?   108*/
      return( AppPTB.ESP);              /* give the bugger the ESP value  108*/

    case RETREGESI:                     /* offset is the ESI register ?   108*/
      return( AppPTB.ESI);              /* give the bugger the ESI value  108*/

    case RETREGEDI:                     /* offset is the EDI register ?   108*/
      return( AppPTB.EDI);              /* give the bugger the EDI value  108*/

    case RETREGNONE:                    /* no offset register used ?         */
      if( pinstr->mod_type == BIT16 )
       goto base16;
      return( 0 );                      /* => this offset is zero            */
  }                                     /* end deciphering register code     */
  return(0);
}                                       /* end offreg                        */

/*****************************************************************************/
/* offreg                                                                    */
/*                                                                           */
/* Description:                                                              */
/*   returns an offset register value.                                       */
/*                                                                           */
/* Parameters:                                                               */
/*   retreg     Dave Toll's register code for the offset register.           */
/*                                                                           */
/* Return:                                                                   */
/*   offvalue   contents of the indicated offset register.                   */
/*                                                                           */
/*****************************************************************************/
  uint
offreg(uint retreg)                     /* register code for a offset reg    */
{                                       /* begin offreg                      */
  switch( retreg )
  {                                     /* begin deciphering register code   */
    case RETREGAX:                      /* offset is the AX register ?       */
      return( AppPTB.EAX&0x0000FFFF);   /* give the bugger the AX value   108*/

    case RETREGBX:                      /* offset is the BX register ?       */
      return( AppPTB.EBX&0x0000FFFF);   /* give the bugger the BX value   108*/

    case RETREGCX:                      /* offset is the CX register ?       */
      return( AppPTB.ECX&0x0000FFFF);   /* give the bugger the CX value   108*/

    case RETREGDX:                      /* offset is the DX register ?       */
      return( AppPTB.EDX&0x0000FFFF);   /* give the bugger the DX value   108*/

    case RETREGBP:                      /* offset is the BP register ?       */
      return( AppPTB.EBP&0x0000FFFF);   /* give the bugger the BP value   108*/

    case RETREGSP:                      /* offset is the SP register ?       */
      return( AppPTB.ESP&0x0000FFFF);  /* give the bugger the SP value   108*/

    case RETREGSI:                      /* offset is the SI register ?       */
      return( AppPTB.ESI&0x0000FFFF);   /* give the bugger the SI value   108*/

    case RETREGDI:                      /* offset is the DI register ?       */
      return( AppPTB.EDI&0x0000FFFF);   /* give the bugger the DI value   108*/

    case RETREGEAX:                     /* offset is the EAX register ?   108*/
      return( AppPTB.EAX );             /* give the bugger the EAX value  108*/

    case RETREGEBX:                     /* offset is the EBX register ?   108*/
      return( AppPTB.EBX);              /* give the bugger the EBX value  108*/

    case RETREGECX:                     /* offset is the ECX register ?   108*/
      return( AppPTB.ECX );             /* give the bugger the ECX value  108*/

    case RETREGEDX:                     /* offset is the EDX register ?   108*/
      return( AppPTB.EDX);              /* give the bugger the EDX value  108*/

    case RETREGEBP:                     /* offset is the EBP register ?   108*/
      return( AppPTB.EBP);              /* give the bugger the EBP value  108*/

    case RETREGESP:                     /* offset is the ESP register ?   108*/
      return( AppPTB.ESP);              /* give the bugger the ESP value  108*/

    case RETREGESI:                     /* offset is the ESI register ?   108*/
      return( AppPTB.ESI);              /* give the bugger the ESI value  108*/

    case RETREGEDI:                     /* offset is the EDI register ?   108*/
      return( AppPTB.EDI);              /* give the bugger the EDI value  108*/

    case RETREGNONE:                    /* no offset register used ?         */
      return( 0 );                      /* => this offset is zero            */
  }                                     /* end deciphering register code     */
  return(0);
}                                       /* end offreg                        */

/*****************************************************************************/
/* grabopnd                                                                  */
/*                                                                           */
/* Description:                                                              */
/*   returns a pointer to the "read in" hex operand.                         */
/*                                                                           */
/* Parameters:                                                               */
/*   pcktptr    ->Dave Toll's disassembler communications packet.            */
/*   opndsize   size of our operand (in bytes).                              */
/*                                                                           */
/* Return:                                                                   */
/*   p          ->hex operand bytes read in.                                 */
/*                                                                           */
/*****************************************************************************/
  uchar*
grabopnd(INSTR *cptr,uint opndsize)     /* cptr : -> cached instruction info */
                                        /* opndsize : size of operand to grab*/
{                                       /* begin grabopnd                    */
  uint    opndptr;                      /* ->operand in user's app space  101*/
  uchar   *appcdptr;                    /* ->application code read in        */
  uint    read;                         /* number of bytes read in by DBGetCS*/
  ULONG   basevalue;                    /* base register value            108*/
  uint    indexvalue;                   /* index register value           108*/

  basevalue  = BaseVal( cptr );         /* get a base value.                 */
  indexvalue = offreg( cptr->index );   /* get index register value       108*/
  indexvalue = indexvalue<<cptr->scale; /* scale the index reg            108*/
  opndptr = basevalue +                 /* calculate operand's address    108*/
            indexvalue +
            ( uint ) cptr->offset;

  appcdptr = DBGet(                     /* ->memory operand which is read i  */
                    opndptr,            /*   starting at operand address for */
                    opndsize,           /*   a maximum of opndsize bytes     */
                    &read );            /*   (but maybe less available)      */





  return( appcdptr );                   /* give caller ->operand hex         */
}                                       /* end grabopnd                      */


/*****************************************************************************/
/* buildasmsrc()                                                             */
/*                                                                           */
/* Description:                                                              */
/*   display source code along with assembler.                               */
/*                                                                           */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   cp        input - -> to start of text buffer line.                      */
/*   fp        input - afile for this asm window.                            */
/*   lno       input - executable line number for the source.                */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   lno is a valid executable line number.                                  */
/*                                                                           */
/*****************************************************************************/
 void
buildasmsrc(uchar *cp,AFILE *fp,uint lno)
{                                       /*                                   */
 uchar     line[ MAXCOLS+1 ];           /* a source line buffer.             */
 uchar    *ptr;                         /* a ptr.                            */
 uint      i;                           /* just an i.                        */

/*****************************************************************************/
/* The lno value at this point may need to be adjusted for a HUGE, > 64k,    */
/* source file. The Nbias value is the number of lines in the source file    */
/* that have been skipped prior to what is currently loaded in the source    */
/* buffer for this afile.                                                    */
/*                                                                           */
/*****************************************************************************/

 lno -= fp->Nbias ;                     /* adjust for HUGE source.        235*/
 ptr = fp->source + fp->offtab[lno];    /* -> to text line in source buffer. */
 i  = Decode(ptr,line);                 /* decode/copy src text to line buf. */
 i = (i < VideoCols) ? i : VideoCols;   /* limit to video cols               */
 strncpy(cp , line, i );

}
