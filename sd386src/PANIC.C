/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   panic.c                                                                 */
/*                                                                           */
/* Description:                                                              */
/*   dump panic message to user and clean-up                                 */
/*                                                                           */
/* Static Functions:                                                         */
/*                                                                           */
/* External Functions:                                                       */
/*                                                                           */
/*                                                                           */
/* History:                                                                  */
/*                                                                           */
/*   02/08/91 Creation of 32-bit SD86, from 16-bit version.                  */
/*                                                                           */
/*...16->32 port.                                                            */
/*...                                                                        */
/*... 02/08/91  100   Philip    port to 32 bit.                              */
/*... 02/12/92  521   Srinivas  Port to C-Set/2.                             */
/*...                                                                        */
/**Includes*******************************************************************/
#include "all.h"                        /* SD86 include files                */

extern uint      VideoRows;
 void
panic( uchar why )
{
    CSR DosCsr;

    switch( (int)why ){
      case OOquit:
        break;
      default:
        printf( "\nPanic %d -- Press any key to continue\n", why );     /*521*/
        GetKey();
    }
    DosCsr.col = 0;
    DosCsr.row = ( uchar )(VideoRows-1);/* (uchar) cast                   100*/
/** DosCsr.mode = **/
    PutCsr( &DosCsr );

/*  SvcTerm(); *//* Added for OS/2 1.1 (4.50 driver) hang at DOSEXIT */

    DosExit(1,why);  /* Terminate ALL threads with rc = why */
}
