/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   findexec.c                                                              */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*  Routines to backup one stack frame.                                      */
/*                                                                           */
/* History:                                                                  */
/*                                                                           */
/*   02/08/91 Creation of 32-bit SD86, from 16-bit version.                  */
/*                                                                           */
/*...16->32 port.                                                            */
/*...                                                                        */
/*... 02/08/91  100   Philip    port to 32 bit.                              */
/*... 02/08/91  101   Joe       port to 32 bit.                              */
/*... 02/08/91  107   Dave      port to 32 bit.                              */
/*... 02/08/91  112   Joe       port to 32 bit.                              */
/*                                                                           */
/*...Release 1.00 (Pre-release 1)                                            */
/*...                                                                        */
/*    08/13/81  224   srinivas  call stack blowing up if there are 10 funcs  */
/**Includes*******************************************************************/

#include "all.h"                        /* SD86 include files                */

extern uint        NActFrames;
extern uint        ActFaddrs[];                                         /*107*/
extern uint        ActFlines[MAXAFRAMES][2];                            /*107*/

#define MIDCOL 0                                                        /*224*/
#define LNOCOL 1                                                        /*224*/

    AFILE *
FindExecLine(AFILE *fp,uint lno)
{
    uint n;
    AFILE *p;

    if( (fp == GetExecfp()) && (lno == GetExecLno()) ){
        if( NActFrames == 0 )
            return( NULL );
        n = 0;  /* return 0th active frame */
        goto ReturnNthFrame;
    }
    if( (NActFrames > 0)
     && (n = IndexOfMidLnoForFrames( fp->mid, lno )) /* see if mid/lno match */
        < NActFrames-1 )
    {                                   /*   up w/some frame 107*/
        n+=1;                           /* return n+1th frame             107*/
        goto ReturnNthFrame;
    }
    p = findfp(GetExecAddr());                                          /*101*/
    if( p )                                                             /*101*/
        p->flags |= AF_ZOOM;
    return( p );

    ReturnNthFrame:

    p = findfp(ActFaddrs[n]);
    if( p )
    {
        p->flags |= AF_ZOOM;
        /* We update ActFlines because findfp may have added an AFILE */
        ActFlines[n][MIDCOL] = p->mid;
        ActFlines[n][LNOCOL] = p->hotline + p->Nbias;
    }
    return( p );
}

    AFILE *
FindExecAddr(AFILE *fp,uint addr)
{
    uint n;                             /* was register.                  112*/
    AFILE *p;

    if( (fp == GetExecfp()) && (addr == GetExecAddr() ) ){                    /*101*/
        if( NActFrames == 0 )
            return( NULL );
        n = 0;                          /* return 0th active frame           */
        goto ReturnNthFrame;
    }

    if( (NActFrames > 0) )
    {
     n = lindex(ActFaddrs, NActFrames-1, (ulong)addr);
     if( n < (NActFrames-1) )
     {
      n += 1;                         /* return (n+1)th active frame       */
      goto ReturnNthFrame;
     }
    }

    p = findfp(GetExecAddr());                                          /*101*/
    if( p )                                                             /*101*/
        p->flags |= AF_ZOOM;
    return( p );

    ReturnNthFrame:

    p = findfp(ActFaddrs[n]);
    if( p )
    {
        p->flags |= AF_ZOOM;
        /* We update ActFlines because findfp may have added an AFILE */
        ActFlines[n][MIDCOL] = p->mid;
        ActFlines[n][LNOCOL] = p->hotline + p->Nbias;
    }
    return( p );
}

/*****************************************************************************/
/* IndexOfMidLnoForFrames                                                    */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*  Looks to match up the current mid and lno with those in ActFlines[][].   */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*  mid   -   module id of interest.                                         */
/*  lno   -   line number of interest.                                       */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*  n     -   row index into ActFlines where the match was found.  If the    */
/*            mid/lno pair were not found, n == NActFrames => failure.       */
/*                                                                        107*/
/*****************************************************************************/
    uint
IndexOfMidLnoForFrames(uint mid,uint lno)
{
    uint n;

    for ( n = 0; n < NActFrames; n++ )       /* try to match mid/lno         */
    {
      if ( ActFlines[n][MIDCOL] == mid  &&   /* mid matches and              */
           ActFlines[n][LNOCOL] == lno     ) /*   lno matches ?              */
        break;                               /* stop with n at proper index  */
    }
    return( n );
}
