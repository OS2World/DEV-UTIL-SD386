/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   setwps.c                                                                */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Allow user to define watch points and put in the watch points.          */
/*                                                                           */
/* History:                                                                  */
/*                                                                           */
/*...Release 1.00 (03/03/92)                                                 */
/*...                                                                        */
/*... 03/10/92  602   Srinivas  Hooking up watch points.                     */
/*                                                                           */
/**Includes*******************************************************************/

#include "all.h"                        /* SD386 include files               */

extern DEBUG_REGISTER Debug_Regs[];     /* hardware debug registers          */
#define MASK1  0x0000000f               /* masks used to align address on    */
#define MASK2  0xfffffff0               /* double word boundary.             */

/*****************************************************************************/
/* Setwps()                                                                  */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*    Allocates buffer for the display of watch point window and calls       */
/*    function to process the key strokes.                                   */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   fp      -  pointer to the current afile structure.                      */
/*                                                                           */
/* Return:                                                                   */
/*   none                                                                    */
/*****************************************************************************/
void SetWps( AFILE *fp )
{
  Cua_SetWps( fp );
}

/*****************************************************************************/
/* EncodeSize()                                                              */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*    Converts a given size into a index value.                              */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   Size     -  size of the address.                                        */
/*                                                                           */
/* Return:                                                                   */
/*   index    - index into sizes array.                                      */
/*****************************************************************************/
uchar EncodeSize(uint size)
{
 uchar rc=0;

  switch (size)
  {
     case 2:
       rc = 1;
       break;

     case 4:
       rc = 2;
       break;

     default:
       rc = 0;
       break;
  }
 return(rc);
}

/*****************************************************************************/
/* AlignAddress()                                                            */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*    Aligns a given address to a word boundary if the size is two bytes     */
/*    or to a double word boundary if the size is four bytes long.           */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   Address  -  pointer to the address.                                     */
/*   Size     -  size of the address.                                        */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*    0    -    If address is not modified.                                  */
/*    1    -    If address is aligned.                                       */
/*****************************************************************************/
uchar AlignAddress(uint *Address,uchar Size)
{
  uchar AlignmentDone;
  uint  TempAddress;

  AlignmentDone = FALSE;

  switch (Size)
  {
     case 1:
       if ( ((*Address) % 2 ) != 0)
       {
          *Address = *Address - 1;
          AlignmentDone = TRUE;
       }
       break;

     case 2:
       if ( (*Address) & 3 )
       {
          TempAddress = (*Address) & MASK1;
          TempAddress = (TempAddress / 4) * 4;
          (*Address) &= MASK2;
          (*Address) |= TempAddress;
          AlignmentDone = TRUE;
       }
       break;
  }
  return(AlignmentDone);
}

/*****************************************************************************/
/* PutInWps()                                                                */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*    Put watch points in x-server and update client indexes.                */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*****************************************************************************/
void  PutInWps( void )
{
 ULONG  Indexes[NODEBUGREGS];
 int    i;

 memset(Indexes,0,sizeof(Indexes));
 xPutInWps( Indexes );

 for(i=0;i < NODEBUGREGS; i++ )
  Debug_Regs[i].Wpindex = Indexes[i];

}

/*****************************************************************************/
/* IsWatchPoint()                                                            */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*    Is there a watch point defined.                                        */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/* Return:                                                                   */
/*   TRUE or FALSE                                                           */
/*****************************************************************************/
int IsWatchPoint( void )
{
 int i;

 for(i=0;i < NODEBUGREGS; i++ )
  if( (Debug_Regs[i].Address != 0) && (Debug_Regs[i].Status == ENABLED) )
   return( TRUE );
 return(FALSE);
}
