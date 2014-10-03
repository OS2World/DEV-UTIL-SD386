/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   showdz.c                                                                */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*  data editing routines.                                                   */
/*                                                                           */
/*                                                                           */
/* History:                                                                  */
/*                                                                           */
/*   02/08/91 Creation of 32-bit SD86, from 16-bit version.                  */
/*                                                                           */
/*...16->32 port.                                                            */
/*...                                                                        */
/*... 02/08/91  101   Joe       port to 32 bit.                              */
/*... 02/08/91  108   Dave      port to 32 bit.                              */
/*... 02/08/91  111   Christina port to 32 bit.                              */
/*... 02/08/91  112   Joe       port to 32 bit.                              */
/*                                                                           */
/*...Release 1.00 (Pre-release 1)                                            */
/*...                                                                        */
/*... 07/09/91  205   srinivas  Hooking up of register variables.            */
/*... 07/09/91  207   srinivas  modifying data in the data window.           */
/*... 08/15/91  226   srinivas  modifying pointers in data window.           */
/*... 08/19/91  228   srinivas  padding zeros in the hex fields.             */
/*... 08/19/91  230   srinivas  Blank field in a hex field followed by ENTER */
/*                              key should restore the previous contents.    */
/*                                                                           */
/*...Release 1.00 (Pre-release 1.08 10/10/91)                                */
/*...                                                                        */
/*... 02/07/92  512   Srinivas  Handle Toronto "C" userdefs.                 */
/*... 02/12/92  521   Joe       Port to C-Set/2.                             */
/*...                                                                        */
/*...Release 1.01 (04/03/92)                                                 */
/*...                                                                        */
/*... 08/03/92  701   Joe       Cua Interface.                               */
/*...                                                                        */
/*...Release 1.02 (10/22/92)                                                 */
/*...                                                                        */
/*... 03/03/93  813   Joe       Revised types handling for HL03.             */
/*...                           (Moved FormatdataItem to it's own file.)     */
/*...                                                                        */
/*****************************************************************************/
#include "all.h"

#define TAB1 15                         /* first tab pos for data display    */
#define TAB2 27                         /* second tab pos for data display   */

UCHAR CantZapMsg[] = "Can't modify this data!";
UCHAR BadDataMsg[] = "Invalid value for data type";

extern uint   VideoCols;
extern uchar  VideoAtr;
extern uint   ExprAddr;  /* Set by ParseExpr -- Address value of expr 101*/

#define NTYPES 9
ushort dtab[NTYPES] = {
                       TYPE_CHAR   ,    /* 0x80                           101*/
                       TYPE_SHORT  ,    /* 0X81                           101*/
                       TYPE_LONG   ,    /* 0x82                           101*/
                       TYPE_UCHAR  ,    /* 0x84                           101*/
                       TYPE_USHORT ,    /* 0x85                           101*/
                       TYPE_ULONG  ,    /* 0x86                           101*/
                       TYPE_FLOAT  ,    /* 0x88                           101*/
                       TYPE_DOUBLE ,    /* 0x89                           101*/
                       TYPE_LDOUBLE     /* 0x8A                        813101*/
                      };


uint                                    /*                                521*/
ZapPointer(DFILE *dfp ,uint  row, uint skip )
{
    return( skip ? FALSE :
      KeyPointer(dfp->mid, dfp->showtype, dfp->sfx, row , 0 , ExprAddr) );
}

 int
KeyPointer(uint mid,uint tno,uint sfx,uint row,uint col,uint varptr )
{
    uint keycol, segcol, offcol, csroff;
    ULONG helpid;
    uint n;
    uchar type;
    uint ptr;
    uchar buffer[DATALINESPAN];
    PEVENT  Event;                      /*                                701*/
    uint    key;                        /*                                701*/
    uint    rc = TRUE;                  /*                                701*/

    memset( buffer, 0, sizeof(buffer) );
    FormatDataItem( buffer+1, varptr, mid, tno, sfx );                  /*226*/

#if 0
    /*************************************************************************/
    /*   if we come in with a mouse event that is not in the storage      701*/
    /*   area then just kick it back out.                                 701*/
    /*************************************************************************/
    Event = GetCurrentEvent();                                          /*701*/
    if( Event->Type == TYPE_MOUSE_EVENT )                               /*701*/
    {                                                                   /*701*/
     if( GetStorageArea(Event) != STORAGEAREA )                         /*701*/
      return( TRUE );                                                   /*701*/
    }                                                                   /*701*/
#endif
    if (col)                            /* if the var is part of an array 226*/
       {                                /* or struct indicated by its name226*/
        if ( col <= TAB1 )  col = TAB1; /* displayed beyond STGCOL, move  226*/
        else  col = TAB2;               /* to a TAB pos based on length of226*/
       }                                /* name.                          226*/

    col += STGCOL-1;                                                    /*226*/
    csroff = 0;                                                         /*226*/
    helpid = HELP_DATA_OFFSET;                                          /*226*/


    type = GetPtrType( mid,tno);
    switch (type )

    {
      case PTR_0_16:
        buffer[0] = Attrib(vaStgPro);
        buffer[5] = 0;
        putrc( row, col, buffer );
        for(;;)
        {

          key=GetString(row,col,4, 4,&csroff, buffer,HEXKEYFLD,NULL);
          switch( key )
          {
            case F1:
              Help( helpid );
              break;

            /*****************************************************************/
            /* Treat up/down key like a left mouse click.                    */
            /* Set up a fake event.                                          */
            /*****************************************************************/
            case UP:
            case DOWN:
            case LEFTMOUSECLICK:
            case ENTER:
            case DATAKEY:

            if( (key==UP) || (key==DOWN) )
            {
             Event = GetCurrentEvent();
             Event->Type =  TYPE_MOUSE_EVENT;
             Event->Col  =  col;
             Event->Row  =  (key == DOWN )?row+1:row-1;
            }

            if( (key==UP) || (key==DOWN) ||
                (key==LEFTMOUSECLICK) || (key==ENTER)
              )
            {
              if( strlen(buffer) == 0 )
                return( TRUE );
            }

            if( !ZapHexWord(varptr, sfx, buffer) )
            {
             beep();
             break;
            }

            /****************************************************************/
            /* On a left mouse click:                                       */
            /*                                                              */
            /*  - format the data item.                                     */
            /*  - put the storage field into it's normal color.             */
            /*  - if the event is not in the data window, then              */
            /*    simply return the key, else recirculate it.               */
            /****************************************************************/
            if( (key==LEFTMOUSECLICK) || (key==UP) || (key==DOWN) )
            {
             FormatDataItem( buffer+1, varptr, mid, tno, sfx );
             buffer[0] = Attrib(vaStgVal);
             putrc( row, col, buffer );
             SetDataViewCsr();
             rc = RECIRCULATE;
             if( (GetEventView() != DATAVIEW) )
              rc = key;
            }
            return(rc);

            case ESC:
              return( rc );
            default:
              beep();
          }
        }
     /* break; */

      case PTR_0_32:
        buffer[0] = Attrib(vaStgPro);
        buffer[10] = 0;
        putrc( row, col, buffer );
        for(;;)
        {
          key=GetString(row,col,8,8,&csroff,buffer,HEXKEYFLD,NULL);
          switch( key )
          {
            case F1:
              Help( helpid );
              break;

            /*****************************************************************/
            /* Treat up/down key like a left mouse click.                    */
            /* Set up a fake event.                                          */
            /*****************************************************************/
            case UP:
            case DOWN:
            case LEFTMOUSECLICK:
            case ENTER:
            case DATAKEY:

            if( (key==UP) || (key==DOWN) )
            {
             Event = GetCurrentEvent();
             Event->Type =  TYPE_MOUSE_EVENT;
             Event->Col  =  col;
             Event->Row  =  (key == DOWN )?row+1:row-1;
            }

            if( (key==UP) || (key==DOWN) ||
                (key==LEFTMOUSECLICK) || (key==ENTER)
              )
            {
              if( strlen(buffer) == 0 )
                return( TRUE );
            }

            if( !ZapHexDWord(varptr, sfx, buffer) )
            {
               beep();
               break;
            }

            /****************************************************************/
            /* On a left mouse click:                                       */
            /*                                                              */
            /*  - format the data item.                                     */
            /*  - put the storage field into it's normal color.             */
            /*  - if the event is not in the data window, then              */
            /*    simply return the key, else recirculate it.               */
            /****************************************************************/
            if( (key==LEFTMOUSECLICK) || (key==UP) || (key==DOWN) )
            {
             FormatDataItem( buffer+1, varptr, mid, tno, sfx );
             buffer[0] = Attrib(vaStgVal);
             putrc( row, col, buffer );
             SetDataViewCsr();
             rc = RECIRCULATE;
             if( (GetEventView() != DATAVIEW) )
              rc = key;
            }
            return(rc);


            case ESC:
              return( rc);
            default:
              beep();
          }
        }
    /*  break; */

      case PTR_16_16:
        buffer[0] = Attrib(vaStgPro);
        buffer[5] = 0;
        putrc( row, segcol = col, buffer );
        buffer[5] = Attrib(vaStgPro);
        buffer[10] = 0;
        putrc( row, offcol = col + 5, buffer + 5 );
        keycol = offcol;

        FlipFlop:
        if( keycol == segcol )  {
          keycol = offcol;
          ptr = varptr;
          helpid = HELP_DATA_OFFSET; }
        else                    {
          keycol = segcol;
          ptr = varptr + sizeof(OFFSET);
          helpid = HELP_DATA_SEGMENT; }
        for(;;)
        {
          key=GetString(row,keycol,4,4,&csroff,buffer,HEXKEYFLD,NULL);
          switch( key )
          {
            case LEFT:
              n=3;  goto UpdatePointer;
            case DATAKEY:
            case RIGHT:
            case TAB:
            case S_TAB:
              n=0;  goto UpdatePointer;
            case F1:
              Help( helpid );
              break;

            /*****************************************************************/
            /* Treat up/down key like a left mouse click.                    */
            /* Set up a fake event.                                          */
            /*****************************************************************/
            case UP:
            case DOWN:
            case LEFTMOUSECLICK:
            case ENTER:
            if( (key==UP) || (key==DOWN) )
            {
             Event = GetCurrentEvent();
             Event->Type =  TYPE_MOUSE_EVENT;
             Event->Col  =  col;
             Event->Row  =  (key == DOWN )?row+1:row-1;
            }

            if( strlen(buffer) == 0 )
              return( TRUE );
            if( !ZapHexWord(ptr, sfx, buffer) )
              goto Complain;

            /****************************************************************/
            /* On a left mouse click:                                       */
            /*                                                              */
            /*  - format the data item.                                     */
            /*  - put the storage field into it's normal color.             */
            /*  - if the event is not in the data window, then              */
            /*    simply return the key, else recirculate it.               */
            /****************************************************************/
            if( (key==LEFTMOUSECLICK) || (key==UP) || (key==DOWN) )
            {
             if( key==LEFTMOUSECLICK )
             {
              Event = GetCurrentEvent();                                          /*701*/

              if( Event->Row == row )
              {
               if( (segcol<=Event->Col) && (Event->Col<(segcol+4)))
               {
                keycol = offcol;
                csroff = Event->Col - segcol;
                goto FlipFlop;
               }
               else if((offcol<=Event->Col) && (Event->Col<(offcol+4)))
               {
                keycol = segcol;
                csroff = Event->Col - offcol;
                goto FlipFlop;
               }
              }
             }

             SetDataViewCsr();
             FormatDataItem( buffer+1, varptr, mid, tno, sfx );
             buffer[0] = Attrib(vaStgVal);
             buffer[5] = 0;
             putrc( row, segcol, buffer );
             buffer[5] = Attrib(vaStgVal);
             buffer[10] = 0;
             putrc( row, offcol, buffer + 5 );
             rc = RECIRCULATE;
             if( (GetEventView() != DATAVIEW) )
              rc = key;
            }
            return(rc);

            case ESC:
              return( rc );

            UpdatePointer:
              if( ZapHexWord(ptr, sfx, buffer) )
              {
               csroff = n;
               goto FlipFlop;
              }
              beep();
              break;

            default:
            Complain:
              beep();
              break;
          }
        }
    /*  break; */
    }
    return(FALSE);
}


 uint
ZapHexWord(uint ptr, uint sfx, uchar *hexstr )
{
    ushort hexval;

    x4tou(hexstr, &hexval);
    if(  PutAppData(ptr, sizeof(hexval), ( uchar * )&hexval, sfx) ){
        fmterr( CantZapMsg );
        return( FALSE );  }
    return( TRUE );
}

 uint
ZapHexDWord(uint ptr, uint sfx, uchar *hexstr )                         /*226*/
{                                                                       /*226*/
    uint hexval;                                                        /*226*/
                                                                        /*226*/
    x8tou(hexstr, &hexval);                                             /*226*/
    if(  PutAppData(ptr, sizeof(hexval), ( uchar * )&hexval, sfx) ){    /*226*/
        fmterr( CantZapMsg );                                           /*226*/
        return( FALSE );  }                                             /*226*/
    return( TRUE );                                                     /*226*/
}

 uint
x4tou(uchar *hexstr,ushort *valptr )
{
    uint c, n, mask, shift;             /* was register.                  112*/

    shift = 12;
    mask = 0xF000;
    for( n=0 ; n < strlen(hexstr) ; ++n, shift -= 4, mask >>= 4 ){      /*228*/
        if( ((c = hexstr[n] - '0') > 9)
         && (((c = hexstr[n] - ('A'-10)) < 10) || (c > 15)) )
          c = 0;                                                        /*228*/
        *valptr &= ~mask;
        *valptr |= (c << shift);
     }
    for(; n < 4 ; ++n, shift -= 4, mask >>= 4 )                         /*228*/
        *valptr &= ~mask;                                               /*228*/
    return( TRUE );
}

 uint
x8tou(uchar *hexstr, uint *valptr )                  /* added for ZapRegs 108*/
{
    uint c, n, mask, shift;             /* was register.                  112*/

    shift = 28;
    mask = 0xF0000000;
    for( n=0 ; n < strlen(hexstr) ; ++n, shift -= 4, mask >>= 4 ){      /*228*/
        if( ((c = hexstr[n] - '0') > 9)
         && (((c = hexstr[n] - ('A'-10)) < 10) || (c > 15)) )
          c = 0;                                                        /*228*/
        *valptr &= ~mask;
        *valptr |= (c << shift);
    }
    for(; n < 8 ; ++n, shift -= 4, mask >>= 4 )                         /*228*/
        *valptr &= ~mask;                                               /*228*/
    return( TRUE );
}


#define NTYPES 9
static uint dhid[NTYPES] = {
                             HELP_DATA_CHAR,
                             HELP_DATA_UCHAR,
                             HELP_DATA_INT,
                             HELP_DATA_UINT,
                             HELP_DATA_LONG,
                             HELP_DATA_ULONG,
                             HELP_DATA_FLOAT,
                             HELP_DATA_DOUBLE,
                             HELP_DATA_LDOUBLE
                           };

uint                                    /*                                521*/
ZapScalar(DFILE *dfp , uint row, uint skip )
{
    return( skip ? FALSE :
      KeyScalar(dfp->mid, dfp->showtype, dfp->sfx, row , 0 , ExprAddr) );
}

/* Procedure:   KeyScalar
 *
 * Input:
 *
 * Description:
 */

static  uchar ClrSfld[] = { Attrib(vaStgPro), RepCnt(0), ' ', 0 };
#define ClrSfldLen ClrSfld[2]


 int
KeyScalar(uint mid,uint tno,uint sfx,uint row,uint col,uint varptr )
{
    uint n;                             /* was register.                  112*/
    uint nbytes, csroff=0;
    ULONG helpid;
    uchar buffer[DATALINESPAN];
    uchar answer[ sizeof(long double) ];/* was sizeof(double).            813*/
    uint rc = TRUE;                     /*                                701*/
    uint key;                           /*                                701*/
    PEVENT Event;                       /*                                701*/

    /*************************************************************************/
    /*   if we come in with a mouse event that is not in the storage      701*/
    /*   area then just kick it back out.                                 701*/
    /*************************************************************************/
    Event = GetCurrentEvent();                                          /*701*/
    if( Event->Type == TYPE_MOUSE_EVENT )                               /*701*/
    {                                                                   /*701*/
     if( GetStorageArea(Event) != STORAGEAREA )                         /*701*/
      return( TRUE );                                                   /*701*/
    }                                                                   /*701*/
    ClrSfld[0] = Attrib(vaStgPro);                                      /*701*/

    tno = HandleUserDefs(mid,tno);      /* Get primitive typeno in case   512*/
                                        /* of primitive user defs.        512*/


    if( !(nbytes = QtypeSize(mid, tno))
     || ((n = windex(dtab, NTYPES, tno)) == NTYPES)
      ) return( FALSE );
    helpid = dhid[n];

    if (col)                            /* if the var is part of an array    */
       {                                /* or struct, indicated by its nam   */
        if ( col <= TAB1 )  col = TAB1; /* displayed beyond STGCOL, move     */
        else  col = TAB2;               /* to a TAB pos based on length of   */
       }                                /* name.                             */

    col += STGCOL-1;

    ClrSfldLen = ( uchar )(VideoCols - col);
    putrc( row, col, ClrSfld );

    memset( buffer,0, sizeof(buffer) );                                 /*101*/
    FormatDataItem( buffer, varptr, mid, tno, sfx );
    *(strchr(buffer,' ')) = '\0';                                           /*813*/
    VideoAtr = vaStgPro;
    putrc( row, col, buffer );

    for(;;){
        key=GetString(row,col,ClrSfldLen,ClrSfldLen,&csroff,buffer,0, NULL);
        switch( key )
        {
          case UP:
          case DOWN:
          case LEFTMOUSECLICK:
          case ENTER:
          if( (key==UP) || (key==DOWN) )
          {
           Event->Type =  TYPE_MOUSE_EVENT;
           Event->Col  =  col;
           Event->Row  =  (key == DOWN )?row+1:row-1;
          }

          if( strlen(buffer) == 0 )
              goto Fini;
          if( !ParseScalar(buffer, tno, ( ulong * )answer) ){
              fmterr( BadDataMsg );
          }else if(  PutAppData(varptr, nbytes, answer, sfx) ){
              fmterr( CantZapMsg );
          }else
          {
           /****************************************************************/
           /* On a left mouse click:                                       */
           /*                                                              */
           /*  - put the storage field into it's normal color.             */
           /*  - set the default video attribute and write buffer.         */
           /*  - format the data item.                                     */
           /*  - set the cursor to the location of the click in the        */
           /*    storage window.                                           */
           /*  - recirculate or return the a LEFTMOUSECLICK.               */
           /****************************************************************/
           if( (key==LEFTMOUSECLICK) || (key==UP) || (key==DOWN) )
           {
            extern int DataRecalc;

              ClrSfld[0] = Attrib(vaStgVal);
              putrc( row, col, ClrSfld );
              VideoAtr = vaStgVal;
              memset(buffer,' ',sizeof(buffer));
              buffer[0] = Attrib(vaStgExp);
              buffer[STGCOL] = Attrib(vaStgVal);
              buffer[sizeof(buffer)-1] = 0;
              DataRecalc = TRUE;
              FormatDataItem( buffer, varptr, mid, tno, sfx );
              putrc( row, col, buffer );
              SetDataViewCsr();
              rc = RECIRCULATE;
              if( key == LEFTMOUSECLICK )
               rc = key;
           }
           goto Fini;
          }
          break;

          case F1:
            Help( helpid );
            break;
          case ESC:
            goto Fini;
          default:
            beep();
    }   }
    Fini:
        if( (key==UP) || (key==DOWN) )                                  /*701*/
         return(key);                                                   /*701*/
        return( rc   );                 /*                                701*/
}


static jmp_buf ConversionError = {0};

 int
ParseScalar(uchar *strval,uint datatype,ulong *valptr )
{
    ulong ans, limit;                   /* was register.                  112*/

    if( setjmp(ConversionError) ){
        goto Failed;  }

    switch( datatype ){
      case TYPE_CHAR:
        limit = 127;  goto SignedNumber;
      case TYPE_UCHAR:
        if( (ans = cvtnum(strval, FALSE)) > 255 )  goto Failed;  break;
      case TYPE_SHORT:                  /*                                101*/
        limit = MAXINT;  goto SignedNumber;
      case TYPE_USHORT:                 /*                                101*/
        if( (ans = cvtnum(strval, FALSE)) > MAXUINT )  goto Failed;  break;
      case TYPE_LONG:
        ans = cvtnum(strval, TRUE);  break;
      case TYPE_ULONG:
        ans = cvtnum(strval, FALSE);  break;
      SignedNumber:
        ans = cvtnum(strval, TRUE);
        if( ((long)ans > (long)limit) || ((long)ans < (long)(-(int)(limit+1))) )
            goto Failed;
        break;

      case TYPE_FLOAT:                                                  /*813*/
      case TYPE_DOUBLE:                                                 /*813*/
      case TYPE_LDOUBLE:                                                /*813*/
       /******************************************************************813*/
       /* Mask all the floating point exceptions while we execute the .   813*/
       /* sprintf. The debugger may trap if we don't do this.             813*/
       /*                                                                 813*/
       /******************************************************************813*/
       _control87(0x37f,0xffff);                                        /*813*/
       switch(datatype)                                                 /*813*/
       {                                                                /*813*/
        case TYPE_FLOAT:                                                /*813*/
         *(float*)valptr = (float) atof(strval);                        /*813*/
         break;                                                         /*813*/
                                                                        /*813*/
        case TYPE_DOUBLE:                                               /*813*/
         *(double*)valptr = atof(strval);                               /*813*/
         break;                                                         /*813*/
                                                                        /*813*/
        case TYPE_LDOUBLE:                                              /*813*/
         *(long double*)valptr = _atold(strval);                        /*813*/
         break;                                                         /*813*/
       }                                                                /*813*/
       /******************************************************************813*/
       /* - Clear any exceptions that may have occurred.                  813*/
       /* - Reset the control word to the default value.                  813*/
       /******************************************************************813*/
       _clear87();                                                      /*813*/
       _control87(CW_DEFAULT,0xffff);                                   /*813*/
       return( TRUE );                                                  /*813*/
    }
    *valptr = ans;
    return( TRUE );

    Failed:
        return( FALSE );
}


#define BASE16PREFIX ('0'|(256*'x'))
#define CVTLIMIT ((ulong)0xFFFFFFFF/10)

     ulong
cvtnum(uchar *cp,uint IsSigned )
{
    ulong ans = 0;                      /* was register.                  112*/
    uint  Negate = 0;                   /* was register.                  112*/

    while( *cp == ' ' )
        ++cp;
    if( IsSigned && (*cp == '-') )
        Negate = *cp++;
    if( *(twoc*)cp == BASE16PREFIX ){
        int  c;                         /* was register.                  521*/
        for( ++cp , c = *++cp; c  ;c = *++cp  )
        {
            if( (((c = tolower(c) - '0') < 0) || (c > 9))
             && (((c -= 'a' - '0' - 10) < 10) || (c > 15))
              ) break;
            if( ans & 0xF0000000 )
                longjmp( ConversionError, 3 );
            ans = (ans << 4) | c;
        }
    }else{
        while( (*cp >= '0') && (*cp <= '9') ){
            if( (ans > CVTLIMIT) || ((ans == CVTLIMIT) && (*cp > '5')) )
                longjmp( ConversionError, 2 );
            ans = 10*ans + (*cp++ - '0');
    }   }
    while( *cp == ' ' )
        ++cp;
    if( *cp )  /* extra characters following number */
        longjmp( ConversionError, 1 );

    return( Negate ? -(int)ans : ans );                                 /*521*/
}

 int
PutAppData(uint addr, uint  nbytes, uchar *pdata, uint sfx )            /*207*/
{

    if( TestBit(addr,STACKADDRBIT) )                                    /*207*/
    {                                                                   /*207*/
     addr = StackBPRelToAddr( addr , sfx );                             /*207*/
     if( addr == NULL )                                                 /*207*/
            return( FALSE );                                            /*207*/
    }else if( (addr >> REGADDCHECKPOS) == REGISTERTYPEADDR ){           /*205*/
        if( sfx != 1 ){                     /* If not in the executing frame */
            return( FALSE );
    }   }
    return( DBPut(addr, nbytes, pdata) );
}
