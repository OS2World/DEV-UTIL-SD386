/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   boxmenu.c                                                               */
/*                                                                           */
/* Description:                                                              */
/*  puts up a menu waiting on a key.                                         */
/*                                                                           */
/*                                                                           */
/* History:                                                                  */
/*                                                                           */
/*   04/09/91 Creation of 32-bit SD86, from 16-bit version.                  */
/*                                                                           */
/**Includes*******************************************************************/

#include "all.h"                        /* SD86 include files                */

extern uchar      VideoAtr;

static  uchar lolite[] = { RepCnt(00), Attrib(00), 0 };
static  uchar hilite[] = { RepCnt(00), Attrib(vaCallSel), 0 };

 uint
BoxMenu(uint row,uint col,uint rows,uint cols,BoxMenuData *ptr)
{
    int item = ptr->item, maxitem;      /* was register.                  112*/
    ulong mask = ptr->rowmask;
    uint key, absrow;

    lolite[1] = hilite[1] = ( uchar )cols;
    lolite[2] = ( uchar )Attrib(VideoAtr);

    for( maxitem = 8*sizeof(mask) ;; ){
        if( --maxitem < 0 )
            return(0);
        if( mask & (1L << maxitem) )
            break;
    }
    for(;;)
    {
        if( item2row(item, mask) >= rows )
            item = 1;
        putrc( absrow = row + item2row(item,mask), col, hilite );
        key = GetKey();
        switch( key )
        {
          case DOWN:
          case RIGHT:
          case SPACEBAR:
            ++item;
            break;

          case UP:
          case LEFT:
            if( --item < 1 )
                item = maxitem;
            break;

          case F1:
            Help( ptr->helpid );
            break;

          case ESC:
            return( key );

          case ENTER:
          case A_ENTER:
          case C_ENTER:
            if( ptr->selectmask & (1L << item2row(item,mask)) )
            {
                ptr->item = item;
                return( key );
            }
            beep();
            break;

          default:
            beep();
        }
        putrc( absrow, col, lolite );
    }
}

uint
item2row(int item,ulong mask)
{
    uint n;                             /* was register.                  112*/

    for( n=0 ; n < 8*sizeof(mask) ; ++n )
        if( mask & (1L << n) )
            if( --item <= 0 )
                break;
    return(n);
}
