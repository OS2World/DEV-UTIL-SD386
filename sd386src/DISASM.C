/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   disasm.c                                                                */
/*                                                                           */
/* Description:                                                              */
/*   Contains functions that deal with disassembly for SD/86.                */
/*                                                                           */
/* History:                                                                  */
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
/*... 07/09/91  201   srinivas  single line disassembly when stepping into   */
/*                              a system dll.                                */
/*... 08/30/91  235   Joe       Cleanup/rewrite ascroll() to fix several bugs*/
/*                              a system dll.                                */
/*...                                                                        */
/*...Release 1.00 (Pre-release 108 12/05/91)                                 */
/*...                                                                        */
/*... 02/18/92  534   Srinivas  Double execution lines in disassembly view   */
/*                              when we have REP instruction.                */
/*...                                                                        */
/*...Release 1.00 (03/03/92)                                                 */
/*...                                                                        */
/*... 03/18/92  606   Srinivas  Handle multiple segment numbers in lno table */
/*...                           due to alloc_text pragma.                    */
/*... 12/14/93  911   Joe       Hang showing PL/X disassembly of select stmt.*/
/** Includes *****************************************************************/
                                        /*                                   */
#include "all.h"                        /* SD86 include files                */
                                        /*                                   */
/** Macros *******************************************************************/
                                        /*                                   */
#define VBAR         179                /* vertical bar character            */
#define ROLLBACK     300                /* bytes to start back scrolling.    */
#define CHUNKOFBYTES 300                /* number of bytes to read at a   235*/
                                        /* time while forming instr cache.235*/
#define MAXINSTLENGTH 25                /*                                235*/
                                        /*                                   */
/** Externs ******************************************************************/
                                        /*                                   */
extern PtraceBuffer AppPTB;             /*                                   */
extern uint         AL86orMASM;         /* AL86 or MASM mnemonics flag.      */
extern int          CacheAnchorIndex;   /*                                235*/
extern int          InstrCacheSize;     /*                                235*/
extern INSTR       *icache;             /* instruction cache.             235*/
extern uint         MneChange;          /* flag to tell DBDisa about a       */
extern uchar        AllocTextFlag;      /* flag to know of alloc_text     606*/
                                        /*                                   */
static UINT MidLastOff;                 /* last offset of mod in segment.    */
static UINT MidFirstOff;                /* first offset of module in segment.*/

UINT   CurrentMid;                      /* current disasm mid.               */
/*****************************************************************************/
/* DBDisa                                                complete rewrite 235*/
/*                                                                           */
/* Description:                                                              */
/*   returns an index in the instruction cache of addr and ensures           */
/*   that there will be rows instructions after addr. The delta              */
/*   is a bias that is used for backing up in the instruction stream.        */
/*   It is the number of instructions "before" the addr.                     */
/*                                                                           */
/* Parameters:                                                               */
/*   mid        input - module id.                                           */
/*   addr       input - instruction address in cache.                        */
/*   rows       input - number of instructions to disassemble.               */
/*   delta      input - number of rows to adjust before addr.                */
/*                                                                           */
/* Return:                                                                   */
/*   index      the index of addr in the cache.                              */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*  -addr is valid and its pdf can be found.                                 */
/*  -addr will always be in the current cache-if this were not true, the     */
/*   last disassembly would not exist.                                       */
/*                                                                           */
/*****************************************************************************/
  int
DBDisa( uint mid, uint addr, uint rows, int delta )
{
 uint     midoff;                       /* base addr of module.              */
 uint     modlen;                       /* length of module.                 */
 uint     instaddr;                     /* just an instruction address.      */
 int      index;                        /* just a counter.                   */
 DEBFILE *pdf;                          /* ->pdf containing the mid.         */
 int      found;                        /* flag for find of addr in cache.   */
 uint     FirstAddrInCache;             /*                                   */
 int      RowsLeftInCache;              /*                                   */

/*****************************************************************************/
/* Handle special case to rebuild cache. This is currently only called       */
/* for a change in mnemonics. The AL86orMASM global determines which         */
/* set of mnemonics we use.                                                  */
/*****************************************************************************/
 if( addr == 0 && rows == 0)
 {
  cache( icache[0].instaddr, MidLastOff );
  return(0);
 }

/*****************************************************************************/
/* setup new static values if the module changed sinve we were here last.    */
/* - get base address and length of the module.                              */
/* - setup some static values.                                               */
/* - do the initial cache.                                                   */
/*****************************************************************************/
 pdf         = FindExeOrDllWithAddr( addr );
 midoff      = DBLsegInfo( addr, &modlen, pdf );
 MidFirstOff = midoff;
 MidLastOff  = midoff + modlen - 1;
 if( (mid != CurrentMid) || (mid == FAKEMID) )
 /****************************************************************************/
 /* If the mid is a FAKEMID then rebuild the tables, since the FAKEMID       */
 /* corresponds to all the unknown address space.                            */
 /****************************************************************************/
 {
  CurrentMid = mid;
  cache(addr,MidLastOff);
 }

FINDADDRINCACHE:
  /***************************************************************************/
  /* find add in cache.                                                      */
  /***************************************************************************/
  found = TRUE;
  index = FindAddrInCache( addr );
  if( index == ADDRNOTFOUND )
    found = FALSE;

  /***************************************************************************/
  /* - If delta = 0 and we found the addr in the cache, then test to         */
  /*   see if there are enough instructions left in the cache to quench      */
  /*   rows. If so, return the index. If the rows can't be quenched, then    */
  /*   move the cache down in the instruction stream. If addr is not         */
  /*   currently in the cache, then build a new cache.                       */
  /*                                                                         */
  /* - after go back and you will be successful this time.                   */
  /*                                                                         */
  /***************************************************************************/
  if( delta == 0 )
  {
   RowsLeftInCache = InstrCacheSize - index;
   if( found == TRUE )
   {
    if( rows <= (uint)RowsLeftInCache )
     return(index);
    /*************************************************************************/
    /* - It's possible that there may not be enough lines in the cache    911*/
    /*   to quench the request for rows. If that's the case,then          911*/
    /*   cache the addr so that it will be at the 0 index in the cache.   911*/
    /*   This is only likely to happen in PL/X.                           911*/
    /*************************************************************************/
    if( index != 0 )                                                    /*911*/
     cache(addr,MidLastOff);                                            /*911*/
    return(0); /* return index = 0 */                                   /*911*/
   }
   if( found == TRUE )
    cache( icache[CacheAnchorIndex].instaddr , MidLastOff );
   else
   {
    FirstAddrInCache = rollback ( addr, -CacheAnchorIndex, MidFirstOff);
    cache(FirstAddrInCache,MidLastOff);
   }
   goto FINDADDRINCACHE;
  }

  /*****************************************************************************/
  /* Handle delta < 0. Here we test to see if there are delta instructions     */
  /* available in the cache before the addr. If so, we simply return an index. */
  /* Other wise we backup in the instruction stream and re-cache.              */
  /*                                                                           */
  /*****************************************************************************/
  if( delta < 0 )
  {
   if( index < -delta )
   {
    instaddr = rollback ( addr, -CacheAnchorIndex, MidFirstOff);
    cache(instaddr,MidLastOff);
    index =  FindAddrInCache( addr );
    for( ; delta < 0 && index > 0 ; index--,delta++){;}
   }
   else
    index += delta;
   return( index );
  }
 return(0);
}

/*****************************************************************************/
/* FindAddrInCache()                                  ( function added )  235*/
/*                                                                           */
/* Description:                                                              */
/*   Finds the index of an address in the instruction cache.                 */
/*                                                                           */
/* Parameters:                                                               */
/*   addr       input - address to be found.                                 */
/*                                                                           */
/* Return:                                                                   */
/*   index      index of the addr.                                           */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*                                                                           */
/*****************************************************************************/
 int
FindAddrInCache( uint addr )
{
 int index;
 for (index = 0; icache[index].instaddr != ENDOFCACHE &&
                 index < InstrCacheSize; index++)
 {
  if( icache[index].instaddr==addr )
   return( index );
 }
 return( ADDRNOTFOUND );
}

/*****************************************************************************/
/* rollback                                                                  */
/*                                                                           */
/* Description:                                                              */
/*   adjusts an address backwards in the instruction stream.                 */
/*                                                                           */
/* Parameters:                                                               */
/*   iap        ->current instruction (where we are sitting in showA window).*/
/*   deltai     number of instructions to adjust iap by.                     */
/*   fbyte      offset of first byte in current module.                      */
/*   lbyte      offset of last byte in current module.                       */
/*                                                                           */
/* Return:                                                                   */
/*   p          iap modified by deltai worth of instructions (when possible).*/
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   deltai is negative.                                                     */
/*                                                                           */
/*****************************************************************************/
  uint
rollback(uint iap,int deltai,uint fbyte )
                                        /* ->current instruction             */
                                        /* # of instsrs to adj for disasm    */
                                        /* offset of first byte in curr mid  */
{                                       /* begin instdelta                   */
  int   lentab[ROLLBACK];               /* retained instruction lengths      */
  int   i;                              /* index into lentab                 */
  uint  trialiap;                       /* experimental ptr to scroll     107*/
  uchar *streamptr;                     /* ->read in instruction stream   107*/
  uchar type;                           /* indicates 16- or 32-bit code   107*/
  uint  read;                           /* number of bytes read in by DBGetCS*/
  UCHAR bitness;

/*****************************************************************************/
/*                                                                           */
/* Scrolling assembly instructions backward is tricky.    The idea is to     */
/* start disassembling (for instruction length only) at a point well behind  */
/* where you currently are and keep track of these lengths.  At some point,  */
/* this stream of disassembly will (about 99.999% of the time!) meet back at */
/* the current instruction.  You can then back track thru an array of lengths*/
/* to figure out the proper address to scroll back to.                       */
/*                                                                           */
/*****************************************************************************/
  if ( deltai < 0 &&                    /* want to delta backward and        */
       iap != fbyte )                   /*   we can ?                     107*/
  {                                     /* begin delta backward              */
    bitness = GetBitness( iap );           /* set type for InstLengthGlob    107*/
    type = (bitness==BIT16)?USE16:USE32;
    trialiap = fbyte;                   /* assume scroll back to mid start107*/
    if ( fbyte + ROLLBACK <  iap )      /* just need rollback amount ?    107*/
      trialiap = iap - ROLLBACK;        /* scroll back shorter amt        107*/

    streamptr=GetCodeBytes(trialiap,    /* read in all bytes up thru addr 827*/
                    iap-trialiap,
                    &read);

    i = 0;                              /* initialize index into lentab      */
    while( trialiap < iap )             /* still need disasm lengths ?    107*/
    {                                   /* disasm forward till we converge   */
      lentab[i] = InstLengthGlob( streamptr, type ); /* gimme inst len !  107*/
      trialiap += lentab[i];            /* bump to next instr address     107*/
      streamptr += lentab[i++];         /* bump to next instr & next entry107*/
    }                                   /* end disasm frwd till we converge  */

    if ( trialiap == iap )              /* did we converge ?              107*/
      for(
           i--;                         /* back up to last instr entry       */
           i >= 0;                      /* make sure we still have entries   */
           i--                          /* back up another entry             */
         )
      {                                 /* add up all instr lengths for delta*/
        iap -= lentab[i];               /* back up by this entry's length 107*/
        if ( !( ++deltai ) )            /* done scrolling back ?             */
          break;                        /* finished adjusting iap            */
      }                                 /* end add up all instr lens for delt*/
  }                                     /* end delta backward                */

  return( iap );                        /* give back new deltad address      */
}                                       /* end instdelta                     */
/*****************************************************************************/
/* cache()                                                                   */
/*                                                                           */
/* Description:                                                              */
/*   cache assembler instructions beginning at some address.                 */
/*                                                                           */
/* Parameters:                                                               */
/*   addr       address where to begin disassembly.                          */
/*   lastoff    offset of last byte in current module.                       */
/*                                                                           */
/* Return:                                                                   */
/*   index      number of instructions cached. Could be less than cache size */
/*              when we hit the end of the module.                           */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   addr is a valid instruction.                                            */
/*                                                                           */
/*****************************************************************************/
 int
cache(uint addr ,uint lastoff)
{
  UCHAR    bitness;
  uchar   *tempptr;                     /* ->DBGet allocated memory space    */
  int     index;                        /*                                   */
  uint    instaddr;                     /*                                   */
  uint    read;                         /* number of bytes read in by DBGet  */
  uint    instlen;
  uchar   hexbuffer[HEXBUFFSIZE];       /* where disassembler puts hex.   108*/
  char    mnebuffer[MNEMBUFFSIZE];      /* where disassembler puts mne.   108*/
  uchar   textbuffer[TEXTBUFFSIZE];     /* where disassembler puts text.  108*/
  uchar   type;                         /* type of module 16 or 32 bit    106*/
  uint    Bytes_Consumed = 0;           /* counter to keep track of no    235*/
  DTIF    packet;                       /* disassembler comm packet          */
                                        /* bytes consumed in instr stream 235*/
   tempptr =  GetCodeBytes(             /* ->instruction stream which is  827*/
                     addr,              /* read in starting at current addr  */
                     CHUNKOFBYTES,      /* for a max of CHUNKOFBYTES bytes   */
                     &read );           /*   (but maybe less available)   106*/
   memset(&packet,0,sizeof(DTIF));      /* clear the comm packet.         101*/
   packet.InstPtr = tempptr;            /* ->read in hex from user app       */
   packet.InstEIP  = 0xffffffff;        /* EIP value for this instr.         */
   packet.Flags.MASMbit=AL86orMASM;     /* use AL/86 or MASM (user decides)  */
   packet.Flags.N387bit  = 1;           /* not a 80x87 processor instr       */
   bitness = GetBitness(addr);
   type = (bitness==BIT16)?USE16:USE32;
   packet.Flags.Use32bit = (ushort) type;
   packet.Flags.Unused1 = 0;            /* make zero due to possible future  */
   instaddr = addr;
   for (index = 0; index < InstrCacheSize; index++)
   {                                    /* disassemble one line at a pop     */


/*****************************************************************************/
/*                                                                           */
/* Use Dave Toll's disassembler to disassemble the instruction.              */
/*                                                                           */
/*****************************************************************************/

    memset(hexbuffer, 0, sizeof(hexbuffer));
    memset(mnebuffer, 0, sizeof(mnebuffer));
    memset(textbuffer, 0, sizeof(textbuffer));


    packet.HexBuffer = hexbuffer;       /* hexbuffer will have instr stream h*/
    packet.MneBuffer = mnebuffer;       /* -> disassembled mnemonic.         */
    packet.TextBuffer = textbuffer;     /* for disasembly text               */
    DisAsm( ( DTIF * ) &packet );       /* disassemble current instruction   */

    icache[index].instaddr = instaddr;
    strcpy(icache[index].mne,mnebuffer);
    strcpy(icache[index].text,textbuffer);
    strcpy(icache[index].hex,hexbuffer);
    instlen = packet.retInstLen;          /* instruction length         */
    icache[index].type     = packet.retType;   /* type of operand info       */
    if(packet.retType == REPETC )         /* instr is of repeat form ?    108*/
    {
     packet.HexBuffer = hexbuffer;
     packet.MneBuffer = mnebuffer;
     packet.TextBuffer = textbuffer;
     DisAsm( ( DTIF * ) &packet );
     strcpy(icache[index].text,mnebuffer);
     strcat(icache[index].hex,hexbuffer);
     instlen  += packet.retInstLen;            /* instruction length         */
    }
    icache[index].len      = instlen;          /* instruction length         */
    icache[index].reg      = packet.retReg;    /* register field             */
    icache[index].offset   = packet.retOffset; /* instrn's offset/displacemnt*/
    icache[index].seg      = packet.retSeg;    /* instrn's selector field    */
    icache[index].base     = packet.retBase;   /* base register field        */
    icache[index].index    = packet.retIndex;  /* index register field       */
    icache[index].scale    = packet.retScale;  /* index register scale    108*/
    icache[index].mod_type = type;             /* module type             106*/
    icache[index].OpSize   = 0;                /* assume 16-bit op size   108*/
    if (packet.retbits.retOpSize)              /* 32-bit op size ?        108*/
      icache[index].OpSize   = 1;

    instaddr += instlen;
    Bytes_Consumed += instlen;          /* update the bytes consumed cntr 235*/

    if ( instaddr  > lastoff )          /* disassembling past end of mod ?   */
    {
     index++;
     break;                             /* terminate cache with null selector*/
    }

    if ( Bytes_Consumed > (CHUNKOFBYTES - MAXINSTLENGTH) )              /*235*/
    {                                                                   /*235*/
      tempptr =  GetCodeBytes(          /* ->instruction stream which is  827*/
                       instaddr,        /* read starting at current addr  235*/
                     CHUNKOFBYTES,      /* for a max of CHUNKOFBYTES bytes235*/
                     &read );           /*   (but maybe less available)   235*/
      Bytes_Consumed = 0;               /* reset the bytes consumed cntr  235*/
      packet.InstPtr = tempptr;         /* ->read in hex from user app    235*/
    }                                                                   /*235*/
/* DumpINSTRStructure( (void*)(&icache[index]) ); */
   }                                    /* end for{} to disassemble.         */
   icache[index].instaddr = NULL;       /* terminate the cache.              */
/* DumpCache( icache); */
 return( index );
}
/*****************************************************************************/
/* GetInstrPacket                                                            */
/*                                                                           */
/* Description:                                                              */
/*   Get the disassembler packet for an instruction.                         */
/*                                                                           */
/* Parameters:                                                               */
/*   addr        input - address of instruction.                             */
/*   packet      input - -> to instruction packet.                           */
/*                       gets filled in by this function.                    */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*  addr is valid.                                                           */
/*                                                                           */
/*****************************************************************************/
#define BLEN 15                         /* bytes of debuggee code to read 107*/
                                        /* for call instruction disassembl   */
 void                                   /*                                   */
GetInstrPacket( uint addr, DTIF *InstrPacket ) /*                         101*/
{                                       /*                                   */
 uchar    hexbuffer[HEXBUFFSIZE];       /* where disassembler puts hex.   108*/
 char     mnebuffer[MNEMBUFFSIZE];      /* where disassembler puts mne.   108*/
 uchar    textbuffer[TEXTBUFFSIZE];     /* where disassembler puts text.  108*/
 uchar   *tempptr;                      /* ->DBGet allocated memory.         */
 uint     read;                         /* bytes read in by DBGet.           */
                                        /*                                   */
 /****************************************************************************/
 /* tempptr allocate and free is managed by DBGet which allocates the        */
 /* buffer and just holds on to it until the next call by any caller.        */
 /****************************************************************************/
 tempptr =  GetCodeBytes(addr,BLEN,&read); /* read BLEN bytes from debugge827*/
 InstrPacket->InstPtr = tempptr;        /* ->read in hex from user app       */
 InstrPacket->InstEIP  = 0xffffffff;    /* EIP value for this instr->        */
 InstrPacket->Flags.MASMbit=1;          /* 1 for masm disassembly.           */
 InstrPacket->Flags.N387bit  = 1;       /* not a 80x87 processor instr       */
 InstrPacket->Flags.Unused1 = 0;        /* make zero due to possible futur   */
 /****************************************************************************/
 /* We don't really care about these buffers at this time. We put them       */
 /* in to satisfy the disassembler.                                          */
 /****************************************************************************/
 memset(hexbuffer, 0, sizeof(hexbuffer));
 memset(mnebuffer, 0, sizeof(mnebuffer));
 memset(textbuffer, 0, sizeof(textbuffer));
 InstrPacket->HexBuffer = hexbuffer;    /* hexbuffer will have instr strea   */
 InstrPacket->MneBuffer = mnebuffer;    /* -> disassembled mnemonic.         */
 InstrPacket->TextBuffer = textbuffer;  /* for disasembly text               */
 DisAsm( InstrPacket );                 /* disassemble current instruction   */
}                                       /* end GetInstrPacket.               */
/*************************************************************************101*/
/* InstLength()                                                           101*/
/*                                                                        101*/
/* Description:                                                           101*/
/*   Gets the length of an assembler instruction.                         101*/
/*                                                                        101*/
/* Parameters:                                                            101*/
/*   addr        input - address of instruction.                          101*/
/*                                                                        101*/
/* Return:                                                                101*/
/*               length of instruction.                                   101*/
/*                                                                        101*/
/* Assumptions:                                                           101*/
/*                                                                        101*/
/*   addr is flat.                                                        101*/
/*                                                                        101*/
/*************************************************************************101*/
 uchar                                                                  /*101*/
InstLength( uint addr )                                                 /*101*/
{                                                                       /*101*/
 DTIF    InstrPacket;                                                   /*101*/
 int     PacketSize;                                                    /*101*/
 UCHAR   bitness;                                                       /*101*/
 UCHAR   type;                                                          /*101*/

                                                                        /*101*/
 PacketSize = sizeof(InstrPacket);                                      /*101*/
 memset(&InstrPacket,0,PacketSize );                                    /*101*/
 bitness = GetBitness( addr );
 type = (bitness==BIT16)?USE16:USE32;
 InstrPacket.Flags.Use32bit = type;                                     /*101*/
 GetInstrPacket( addr, (DTIF*)&InstrPacket );                           /*101*/
 return(InstrPacket.retInstLen);                                        /*101*/
}                                                                       /*101*/
/*************************************************************************107*/
/* InstLengthGlob()                                                       107*/
/*                                                                        107*/
/* Description:                                                           107*/
/*   Gets the length of an assembler instruction where the instruction    107*/
/*   stream is already in our memory (no more DBGets!).                   107*/
/*                                                                        107*/
/* Parameters:                                                            107*/
/*   inststreamptr  input - points to instruction stream.                 107*/
/*   type           input - 0=>USE16, 1=>USE32.                           107*/
/*                                                                        107*/
/* Return:                                                                107*/
/*                  length of instruction.                                107*/
/*                                                                        107*/
/* Assumptions:                                                           107*/
/*                                                                        107*/
/*    instruction stream already read in via DBGet!                       107*/
/*************************************************************************107*/
 uchar
InstLengthGlob( uchar* inststreamptr, uchar type )
{
 DTIF     InstrPacket;
 int      PacketSize;
 uchar    InstrLen;                     /* instruction length             534*/
 uchar    hexbuffer[HEXBUFFSIZE];       /* where disassembler puts hex.   108*/
 char     mnebuffer[MNEMBUFFSIZE];      /* where disassembler puts mne.   108*/
 uchar    textbuffer[TEXTBUFFSIZE];     /* where disassembler puts text.  108*/

 PacketSize = sizeof(InstrPacket);
 memset(&InstrPacket,0,PacketSize );
 InstrPacket.InstPtr=inststreamptr;     /* ->read in hex from user app       */
 InstrPacket.InstEIP=0xffffffff;        /* EIP value for this instr->        */
 InstrPacket.Flags.Use32bit=type;       /* based upon address type           */
 InstrPacket.Flags.MASMbit=1;           /* 1 for masm disassembly.           */
 InstrPacket.Flags.N387bit=1;           /* not a 80x87 processor instr       */
 InstrPacket.Flags.Unused1=0;           /* make zero due to possible futur   */
 /****************************************************************************/
 /* We don't really care about these buffers at this time. We put them       */
 /* in to satisfy the disassembler.                                          */
 /****************************************************************************/
 memset(hexbuffer, 0, sizeof(hexbuffer));
 memset(mnebuffer, 0, sizeof(mnebuffer));
 memset(textbuffer, 0, sizeof(textbuffer));
 InstrPacket.HexBuffer=hexbuffer;       /* hexbuffer will have instr strea   */
 InstrPacket.MneBuffer=mnebuffer;       /* -> disassembled mnemonic.         */
 InstrPacket.TextBuffer=textbuffer;     /* for disasembly text               */
 DisAsm( &InstrPacket );                /* disassemble current instruction   */
 InstrLen = InstrPacket.retInstLen;     /*                                534*/
 if(InstrPacket.retType == REPETC )     /* instr is of repeat form ?      534*/
 {                                      /*                                534*/
   InstrPacket.HexBuffer=hexbuffer;     /* hexbuffer will have instr strea534*/
   InstrPacket.MneBuffer=mnebuffer;     /* -> disassembled mnemonic.      534*/
   InstrPacket.TextBuffer=textbuffer;   /* for disasembly text            534*/
   DisAsm( &InstrPacket );              /* disassemble current instruction534*/
   InstrLen += InstrPacket.retInstLen;  /*                                534*/
 }                                      /*                                534*/
 return(InstrLen);                      /* caller gets instruction length 534*/
}                                       /* end GetInstrPacket.               */
