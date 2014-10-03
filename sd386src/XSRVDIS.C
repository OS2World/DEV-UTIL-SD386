/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   xsrvdis.c                                                            822*/
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*  some x-server disassembly functions.                                     */
/*                                                                           */
/* History:                                                                  */
/*                                                                           */
/*   05/04/93 Created.                                                       */
/*                                                                           */
/*...                                                                        */
/*... 06/04/93  827   Joe       Add remote debug support.                    */
/*...                                                                        */
/*****************************************************************************/
#include "all.h"

/*****************************************************************************/
/* _InstLength()                                                             */
/*                                                                           */
/* Description:                                                              */
/*   Gets the length of an assembler instruction.                            */
/*                                                                           */
/* Parameters:                                                               */
/*   addr        input - address of instruction.                             */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*               length of instruction.                                      */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   addr is flat.                                                           */
/*                                                                           */
/*****************************************************************************/
UCHAR _InstLength( ULONG addr )
{
 DTIF    InstrPacket;
 int     PacketSize;
 UCHAR   type;
 UCHAR   bitness;

 PacketSize = sizeof(InstrPacket);
 memset(&InstrPacket,0,PacketSize );
 bitness = _GetBitness( addr );
 type = (bitness==BIT16)?USE16:USE32;
 InstrPacket.Flags.Use32bit = type;
 _GetInstrPacket( addr, (DTIF*)&InstrPacket );
 return(InstrPacket.retInstLen);
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
_GetInstrPacket( uint addr, DTIF *InstrPacket ) /*                     101*/
{                                       /*                                   */
 uchar    hexbuffer[HEXBUFFSIZE];       /* where disassembler puts hex.   108*/
 char     mnebuffer[MNEMBUFFSIZE];      /* where disassembler puts mne.   108*/
 uchar    textbuffer[TEXTBUFFSIZE];     /* where disassembler puts text.  108*/
 uchar   *tempptr;                      /* ->DBGet allocated memory.         */
 uint     read;                         /* bytes read in by Getnbytes.       */
                                        /*                                   */
 /****************************************************************************/
 /* tempptr allocate and free is managed by Getnbytes which allocates the    */
 /* buffer and just holds on to it until the next call by any caller.        */
 /****************************************************************************/
 tempptr =  Getnbytes(addr,BLEN,&read); /* read BLEN bytes from debuggee. 101*/
 InstrPacket->InstPtr = tempptr;        /* ->read in hex from user app       */
 InstrPacket->InstEIP  = 0xffffffff;    /* EIP value for this instr->        */
 InstrPacket->Flags.MASMbit=1;          /* 1 for masm disassembly.           */
 InstrPacket->Flags.N387bit  = 1;       /* not a 80x87 processor instr       */
 InstrPacket->Flags.Unused1 = 0;        /* make zero due to possible futur   */
 /****************************************************************************/
 /* We don't really care about these buffers at this time. We put them       */
 /* in to satisfy the disassembler.                                          */
 /****************************************************************************/
 InstrPacket->HexBuffer = hexbuffer;    /* hexbuffer will have instr strea   */
 InstrPacket->MneBuffer = mnebuffer;    /* -> disassembled mnemonic.         */
 InstrPacket->TextBuffer = textbuffer;  /* for disasembly text               */
 DisAsm( InstrPacket );                 /* disassemble current instruction   */
}                                       /* end GetInstrPacket.               */
