/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   freefp.c                                                                */
/*                                                                           */
/* Description:                                                              */
/*   Free an AFILE block.                                                    */
/*   The specified AFILE block is dequeued and freed.                        */
/*   The associated source buffer and offset table (if any)                  */
/*   are also freed.                                                         */
/*                                                                           */
/* History:                                                                  */
/*                                                                           */
/*   02/08/91 Creation of 32-bit SD86, from 16-bit version.                  */
/*                                                                           */
/*...16->32 port.                                                            */
/*...                                                                        */
/*... 02/08/91  106   Srinivas  port to 32 bit.                              */
/*... 02/08/91  112   Joe       port to 32 bit.                              */
/*...                                                                        */
/*...Release 1.00 (Pre-release 108 12/05/91)                                 */
/*...                                                                        */
/*... 02/21/92  521   Srinivas  Port to C-Set/2.                             */
/*****************************************************************************/

#define INCL_16                         /* 16-bit API                     106*/
#include "all.h"                        /* SD86 include files                */

extern uint        MaxActMods;
extern AFILE*      allfps;

 void
freefp(AFILE *fp)
{
    AFILE *p;                           /* was register.                  112*/

    /* Note: "next" must be 1st field in the AFILE block for this code */
    for( p = (AFILE *) &allfps; p; p = p->next ){
        if( fp == p->next ){
            p->next = fp->next;
            if( fp->source )
               DosFreeSeg(Flat2Sel(fp->source));                        /*106*/
            if( fp->offtab )
               Tfree((void *)(fp->offtab));                              /*521*/
            Tfree( ( void * )fp);                                        /*521*/
            return;
    }   }
    panic(OOfreefp);
}
