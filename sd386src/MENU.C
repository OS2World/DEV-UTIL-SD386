/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   menu.c                                                                  */
/*                                                                           */
/* Description:                                                              */
/*   Action bar menu items handling.                                         */
/*                                                                           */
/*                                                                           */
/* History:                                                                  */
/*                                                                           */
/*   02/08/91 Creation of 32-bit SD86, from 16-bit version.                  */
/*                                                                           */
/*...16->32 port.                                                            */
/*...                                                                        */
/*... 02/08/91  100   Philip    port to 32 bit.                              */
/*... 02/08/91  112   Joe       port to 32 bit.                              */
/*...                                                                        */
/**Includes*******************************************************************/
#include "all.h"

#define IMAX 15         /* max # of items per menu (caller must not exceed) */
#define ISPACE 4        /* # of spaces between menu items                   */

extern UINT VideoCols;
extern UINT MenuRow;                    /* screen row for menu bar (0..N)    */
extern UINT HelpRow;                    /* screen row for menu help (0..N)   */

static  uchar optON[] = {RepCnt(0),Attrib(vaMenuCsr),0};
#define optONlen optON[1]
static  uchar optOFF[] = {RepCnt(0),Attrib(vaMenuBar),0};
#define optOFFlen optOFF[1]
static  uchar hiatt[] = { RepCnt(1), Attrib(vaMenuSel) , 0 };

/*****************************************************************************/
/*  MenuBar()                                                                */
/*                                                                           */
/* Description:                                                              */
/*   Displays a menu bar with help, allosw selection, and returns            */
/*   the number (1..N) of the item selected.                                 */
/*                                                                           */
/*                                                                           */
/* synopsis    n = MenuBar( items, helps, helpids, keys, selpos, hot, mask );*/
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   uchar *items;           (* concantenation of item strings *)            */
/*   uchar *helps;           (* concantenation of help strings *)            */
/*   ULONG *helpids;         (* array of help ids *)                         */
/*   uchar *keys;            (* array of action keys *)                      */
/*   unit  *selpos;          (* array of select key positions  *)            */
/*   uint   hot;             (* item # initially selected *)                 */
/*   uint   mask;            (* bit mask of disabled options *)              */
/*                                                                           */
/* Return:                                                                   */
/*   uint   n;               (* item # selected, or zero *)                  */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
 uint
MenuBar(uchar *items,uchar *helps, ULONG *helpids,uchar *keys,
        uint selpos[],uint hot,uint  mask)
{
    uint n, max, col, key, x, spacing;  /* was register.                  112*/
    uchar *cp, *ip;                     /* was register.                  112*/
    uchar icol[IMAX], ilen[IMAX];
    uint helper[IMAX];

    HideCursor();
    ClrScr( MenuRow, MenuRow, vaMenuBar );

    for( spacing = ISPACE+1; --spacing > 0; ){
        for( max=0, col = spacing-1, ip = items, cp = helps; *ip; ++max ){
            icol[max] = ( uchar )col;
            ilen[max] = ( uchar )( (n = strlen(ip)) + 2 );
            col += n + spacing;
            ip += n+1;
            helper[max] = cp - helps;
            cp += strlen(cp) + 1;
        }
        if( col <= (VideoCols - spacing) )
            break;
    }

    for( n=0, ip = items; n < max; ++n){
        putrc( MenuRow, icol[n] + 1, ip );
        /* Put highlight attribute on appropriate character of menu item     */
        putrc(MenuRow, icol[n] + selpos[n], hiatt);
        ip += ilen[n] - 1;
    }


    n = hot-2;

/* goto AdvanceOption;*/

    for( ; ; ){
        optONlen = optOFFlen = ilen[n];
        ClrScr( HelpRow, HelpRow, vaHelp );
        putrc( HelpRow, 0, helps + helper[n] );
        putrc( MenuRow, icol[n], optON );
        key = GetKey();
        putrc( MenuRow, icol[n], optOFF );
        /* Put highlight attribute on appropriate character of menu item     */
        putrc(MenuRow, icol[n] + selpos[n], hiatt);
        ClrScr( HelpRow, HelpRow, vaHelp );
        switch( key ){
            case TAB:
            case RIGHT:
            case SPACEBAR:
            AdvanceOption:
               do{ if( ++n >= max ) n = 0;
                }while( mask & (1<<n) );
                break;
            case LEFT:
            case S_TAB:
               do{ if( --n >= max ) n = max-1;
                }while( mask & (1<<n) );
                break;
            case UP:
            case ESC:
            case PADPLUS:
            case PADENTER:
                n = -1;
                goto fini;

            case ENTER:
                goto fini;

            case F1:
            {
              uchar *HelpMsg;

              putrc( MenuRow, icol[n], optON );
              HelpMsg = GetHelpMsg( helpids[n], NULL,0 );
              CuaShowHelpBox( HelpMsg );
              break;
            }
            default:
                if( (x = bindex(keys, max, (key & 0xFF) | 0x20)) < max  &&
                    !(mask & (1<<x))  ){
                    n = x;
                    goto fini; }
                beep();
        }
    }
    fini:

    return( n+1 );
}
