/*****************************************************************************/
/* File:                                              IBM INTERNAL USE ONLY  */
/*      showaf.c                                                             */
/*                                                                           */
/* Description:                                                              */
/*      Display and Edit Registers' and Flags' contents.                     */
/*                                                                           */
/*                                                                           */
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
/*...                                                                        */
/***Includes******************************************************************/

#include "all.h"                        /*   SD86 Include Files              */

/***External declarations*****************************************************/

extern UINT TopLine;
extern UINT VideoCols;
extern UINT FnameRow;


static uchar hexdig[] = "0123456789ABCDEF";

 void
fmtasm(AFILE *fp)
{
    ShowData( TopLine );
    fmtfname( fp );
    fmtthread();
    fmterr( NULL );
    HideCursor();
}

#define  MAXTHDLEN  19
  void
fmtthread( )
{
    uchar buffer[MAXTHDLEN];

    sprintf(buffer, "%cThread %u       ", Attrib(vaInfo), GetExecTid());
    putrc( FnameRow, VideoCols - MAXTHDLEN, buffer );
}


/*****************************************************************************/
/* utox2()                                                                   */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*      Convert an unsigned integer to a 2-char hex string.                  */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*      u     unsigned integer to be converted                               */
/*                                                                           */
/* Return:                                                                   */
/*      x2    pointer to converted 2-char hex output                         */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*      none                                                                 */
/*                                                                           */
/*****************************************************************************/

 void
utox2(uchar u,uchar *x2)                /* unsigned input char            213*/
                                        /* ptr to resulting 2-char hex output*/
{

    *(x2+1) = hexdig[u & 0x0F];
    *x2     = hexdig[(u >> 4) & 0x0F];
}
/*****************************************************************************/
/* utox4()                                                                   */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*      Convert an unsigned integer to a 4-char hex string.                  */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*      u     unsigned integer to be converted                               */
/*                                                                           */
/* Return:                                                                   */
/*      x4    pointer to converted 4-char hex output                         */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*      none                                                                 */
/*                                                                           */
/*****************************************************************************/

 void
utox4(uint u,uchar *x4)                 /* unsigned input word               */
                                        /* ptr to resulting 4-char hex output*/
{
    uint n;

    for( n=4 ; n-- ; u >>= 4 )
        x4[n] = hexdig[u & 0xF];
}
/*************************************************************************112*/
/* utox8()                                                                112*/
/*                                                                        112*/
/* Description:                                                           112*/
/*                                                                        112*/
/*      Convert an unsigned integer to an 8-char hex string.              112*/
/*                                                                        112*/
/* Parameters:                                                            112*/
/*                                                                        112*/
/*      u     unsigned integer to be converted                            112*/
/*                                                                        112*/
/* Return:                                                                112*/
/*      x8    pointer to converted 8-char hex output                      112*/
/*                                                                        112*/
/* Assumptions:                                                           112*/
/*                                                                        112*/
/*      none                                                              112*/
/*                                                                        112*/
/*************************************************************************112*/
                                                                        /*112*/
 void                                                                   /*112*/
utox8(uint u,uchar *x8)                                                 /*112*/
{                                                                       /*112*/
    uint n;                                                             /*112*/
                                                                        /*112*/
    for( n=8 ; n-- ; u >>= 4 )                                          /*112*/
        x8[n] = hexdig[u & 0xF];                                        /*112*/
}                                                                       /*112*/

/*****************************************************************************/
/* atou()                                                                    */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*      Convert a char to an unsigned integer.                               */
/*                                                                           */
/* Parameters:                                                               */
/*      cp     pointer to the char to be converted                           */
/*                                                                           */
/* Return:                                                                   */
/*      n      unsigned integer resulting from the conversion                */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*      none                                                                 */
/*                                                                           */
/*****************************************************************************/

 uint
atou(uchar *cp)
{
    uint n;


    for( n=0 ; (*cp >= '0') && (*cp <= '9') ; ++cp )
        n = 10*n + (*cp - '0');
    return( n );
}
