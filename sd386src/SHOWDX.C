/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   showdx.c                                                                */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/* History:                                                                  */
/*                                                                           */
/*   02/08/91 Creation of 32-bit SD86, from 16-bit version.                  */
/*                                                                           */
/*...16->32 port.                                                            */
/*...                                                                        */
/*... 02/08/91  101   Joe       port to 32 bit.                              */
/*... 02/08/91  111   Christina port to 32 bit.                              */
/*... 02/08/91  112   Joe       port to 32 bit.                              */
/*                                                                           */
/*...Release 1.00 (Pre-release 1)                                            */
/*...                                                                        */
/*... 07/09/91  202   srinivas  change address box when editing in data      */
/*                              window to show flat address.                 */
/*                                                                           */
/*...Release 1.00 (Pre-release 105 10/10/91)                                 */
/*...                                                                        */
/*... 11/07/91  311   Srinivas  EAX in Register window disappears some times.*/
/*...                                                                        */
/*...Release 1.00 (Pre-release 107 11/13/91)                                 */
/*...                                                                        */
/*... 11/13/91  400   Srinivas  Vertical Register Display.                   */
/*...                                                                        */
/*...Release 1.00 (Pre-release 108 12/05/91)                                 */
/*...                                                                        */
/*... 02/10/92  514   Srinivas  TAB and S_TAB keys required to function to   */
/*...                           move between the hex and ascii fields.       */
/*... 02/11/92  518   Srinivas  Remove limitation on max no of screen rows.  */
/*... 02/21/92  521   Srinivas  Port to C-Set/2.                             */
/*...                                                                        */
/*...Release 1.01 (04/03/92)                                                 */
/*...                                                                        */
/*... 08/03/92  701   Joe       Cua Interface.                               */
/*...                                                                        */
/**Includes*******************************************************************/

#include "all.h"

extern uchar*      VideoMap;
extern uint        ExprAddr;  /* Set by ParseExpr -- Address value of expr101*/
extern uchar       CantZapMsg[];
extern uchar       BadDataMsg[];
extern uchar      *BoundPtr;            /* -> to screen bounds            518*/
extern uint        VideoCols;           /* # of columns per screen        514*/


static  uchar hexbuf[2+2]  = { Attrib(vaStgPro) };
static  uchar ascbuf[16+2] = { Attrib(vaStgPro) };
static  uchar chrbuf[16+2] = { Attrib(vaStgVal) };
static  uchar hexout[] = { RepCnt(2), Attrib(vaStgVal), 0 };

#define ABOXROWS 3
#define ABOXCOLS 12                                                     /*202*/


static  uchar AboxFormat [] = "³ ÚÄÄÄÄÄÄÄÄ¿ÀÄ´########³  ÀÄÄÄÄÄÄÄÄÙ";   /*311*/
/***********************/               /* This array needs to a null     311*/
/*  "³ ÚÄÄÄÄÄÄÄÄ¿"     */               /* terminated string. Because of  311*/
/*  "ÀÄ´########³"     */               /* this reason it has been changed311*/
/*  "  ÀÄÄÄÄÄÄÄÄÙ"     */               /* to a single dim array.         311*/
/*   123456789012      */               /*                                311*/
/***********************/               /*                                311*/


 uint
ZapHexBytes(DFILE *dfp,uint row,uint skip)
{
    int ith, incr, i;                   /* was register.                  112*/
    uint col, key, csroff, nbytes, sfx = dfp->sfx;
    uint absptr,ptr, varptr = ExprAddr + 16*skip;
    uchar *dp;
    uchar hexval, buffer[16+2], rawdata[16];
    uchar *original;
    uchar *addrbox;
    uint  limit = 16;                   /* max no of bytes                521*/
    PEVENT Event;                       /* -> to current event structure. 701*/
    uint   EntryRow;                    /* the row we came in on.         701*/
    uint   rc = TRUE;                   /*                                701*/
    uint   InitCsrOff = 0;              /* initial cursor pos for ASCII.  701*/

    if( varptr < ExprAddr )
        return( FALSE );

    /*************************************************************************/
    /* Limit the display to the size of array or a struct.                521*/
    /*************************************************************************/
    if( dfp->showtype )                                                 /*521*/
    {                                                                   /*521*/
        limit = QtypeSize(dfp->mid, dfp->showtype);                     /*521*/
        if( limit > 16)                                                 /*521*/
           limit = 16;                                                  /*521*/
    }                                                                   /*521*/

    dp = GetAppData(varptr,limit, &nbytes, sfx);
    if( !dp || (nbytes != limit))
        return( FALSE );

    memcpy( rawdata,dp, nbytes );                                       /*101*/


    ith = incr = key = col = 0;
    /*************************************************************************/
    /* - if we come in with a mouse event then we need to get the index   701*/
    /*   of the entry field that we're going to edit.                     701*/
    /* - if the event did not occur in a valid entry field, i.e.,         701*/
    /*   it occurred in the white space between fields or in the          701*/
    /*   edit area, then we just return.                                  701*/
    /* - if the event occurred in the ASCII area, then we get the         701*/
    /*   initial offset of the cursor position in the field.              701*/
    /*************************************************************************/
    Event = GetCurrentEvent();                                          /*701*/
    EntryRow = Event->Row;                                              /*701*/
    if( Event->Type == TYPE_MOUSE_EVENT )                               /*701*/
    {                                                                   /*701*/
     int area;                                                          /*701*/
                                                                        /*701*/
     ith = GetIthHexField(Event);                                       /*701*/
     area = GetStorageArea(Event);                                      /*701*/
     if( (area == STORAGEAREA && ith >= nbytes) ||                      /*701*/
          area == EDITAREA )                                            /*701*/
      return( TRUE );                                                   /*701*/
     if( area == ASCIIAREA )                                            /*701*/
      InitCsrOff = Event->Col-(ASCIICOL-1);                             /*701*/
    }                                                                   /*701*/

    addrbox =  2*ABOXROWS*ABOXCOLS +
        (original = Talloc(2*2*ABOXROWS*ABOXCOLS));                     /*521*/

    GotoNewByte:
        if( (ith += incr) < 0 )
            goto Fini;


        /*********************************************************************/
        /* If we are going to the right beyond the screen limits , beep   514*/
        /* and depending on the direction take action.                    514*/
        /* - If we are going forward decrement the current byte ->.       514*/
        /* - If we are going backward stay at the 1st byte.               514*/
        /*********************************************************************/
        if(((ith * 4)+3) >= BoundPtr[row])                              /*514*/
        {                                                               /*514*/
           beep();                                                      /*514*/
           if (incr > 0)                                                /*514*/
             ith -= 1;                                                  /*514*/
           else                                                         /*514*/
             ith = 0;                                                   /*514*/
        }                                                               /*514*/

        /*********************************************************************/
        /* If we have less than 16 bytes on the screen and Registers dis- 514*/
        /* play is on, remain on the current byte.                        514*/
        /*********************************************************************/
        if( (ith >= (int)nbytes) && (BoundPtr[row] < VideoCols))        /*514*/
             ith--;                                                     /*514*/

        if( ith >= (int)nbytes ){
            ith = 0;
            csroff = InitCsrOff;                                        /*701*/
            col = ASCIICOL-1;
        }else{
            csroff = (key == LEFT);
            col = STGCOL-1 + 3*ith + ith/4;
        }
        ptr = varptr + ith;

        Vgetbox( original, row+1, col, ABOXROWS, ABOXCOLS );
        memcpy(  addrbox,original, 2*ABOXROWS*ABOXCOLS );

        if( GetAbsAddr(ptr, sfx, &absptr) ){
            FmtAdrBox( addrbox, absptr );
            Vputbox( addrbox, row+1, col, ABOXROWS, ABOXCOLS );
        }
        if( col == ASCIICOL-1 )
            goto KeyAscii;

        utox2( rawdata[ith], hexbuf+1 );
        putrc( row, col, hexbuf );
        for( i=0 ; i < (int)nbytes ; ++i )
            chrbuf[i+1] = graphic(rawdata[i]);
        chrbuf[i+1] = 0;
        putrc( row, ASCIICOL-1, chrbuf );

    for(;;)
    {
        key = GetString(row,col,2, 2,&csroff, buffer,HEXKEYFLD,NULL);
        switch(key)
        {

          case DATAKEY:
          case RIGHT:
          case TAB:
            incr = 1;
            goto UpdateByte;

          case LEFT:
          case S_TAB:
            incr = -1;
            goto UpdateByte;

          case UP:                                                      /*701*/
          case DOWN:                                                    /*701*/
           Event->Type =  TYPE_MOUSE_EVENT;                             /*701*/
           Event->Col  =  col;                                          /*701*/
           Event->Row  =  (key == DOWN )?row+1:row-1;                   /*701*/
           SetDataViewCsr();                                            /*701*/
           goto UpdateByte;

          case ENTER:
          UpdateByte:
            if( *(twoc*)buffer != *(twoc*)(hexbuf+1) ){
                if( !x2tou(buffer, &hexval) ){
                    fmterr( BadDataMsg );
                    goto Complain;  }
/*110 remove !*/if(  PutAppData(ptr, 1, &hexval, sfx) ){
                    fmterr( CantZapMsg );
                    goto Complain;  }
                rawdata[ith] = hexval;
            }
           goto caseESC;

caseESC:
          case ESC:
            putrc( row, col, hexout );
            Vputbox( original, row+1, col, ABOXROWS, ABOXCOLS );
            if( (key==ENTER) || (key==ESC) || (key==UP) || (key==DOWN) )/*701*/
                goto Fini;
            goto GotoNewByte;

          case F1:
            Help( HELP_DATA_HEX );
            break;

          case LEFTMOUSECLICK:                                          /*701*/
           {                                                            /*701*/
                                                                        /*701*/
            /*****************************************************************/
            /* - get the current event.                                   701*/
            /* - if it's not in the storage area on a valid entry field   701*/
            /*   then come back in ( recirculate) on the new row.         701*/
            /* - otherwise get the new ith and proceed to the next entry  701*/
            /*   field.                                                   701*/
            /*****************************************************************/
            Event = GetCurrentEvent();                                  /*701*/
            if( GetEventView() != DATAVIEW )                            /*701*/
            {                                                           /*701*/
             rc = key;                                                  /*701*/
             key = ENTER;                                               /*701*/
             goto UpdateByte;                                           /*701*/
            }                                                           /*701*/
            if(  (GetStorageArea(Event) != STORAGEAREA ) ||             /*701*/
                 (Event->Row != EntryRow)                ||             /*701*/
                 (GetIthHexField(Event) >= nbytes)                      /*701*/
              )                                                         /*701*/
            {                                                           /*701*/
             SetDataViewCsr();                                          /*701*/
             rc = RECIRCULATE;                                          /*701*/
             key = ENTER;                                               /*701*/
             goto UpdateByte;                                           /*701*/
            }                                                           /*701*/
            ith = GetIthHexField(Event);                                /*701*/
            incr = 0;                                                   /*701*/
            goto UpdateByte;                                            /*701*/
          /*  break;   */                                               /*701*/
          }                                                             /*701*/

          default:
          Complain:
            beep();
    }   }

    KeyAscii:;{
        uint NewChr, IsSame;

        for( i=0 ; i < (int)nbytes ; ++i )
            ascbuf[i+1] = graphic(rawdata[i]);
        ascbuf[i+1] = 0;
        putrc( row, col, ascbuf );

        strcpy(buffer,ascbuf);          /* init the buffer for GetString. 701*/
        for(;;){
            key = GetString(row,col,nbytes,nbytes,&csroff,buffer+1,1,NULL);
            switch( key )
            {
              case F1:
                Help( HELP_DATA_ASCII );
                break;
              case TAB:                                                 /*514*/
              case S_TAB:                                               /*514*/
              case RIGHT:                                               /*514*/
              case LEFT:                                                /*514*/
              case DATAKEY:                                             /*514*/
              case UP:                                                  /*701*/
              case DOWN:                                                /*701*/
               Event->Type =  TYPE_MOUSE_EVENT;                         /*701*/
               Event->Col  =  col;                                      /*701*/
               Event->Row  =  (key == DOWN )?row+1:row-1;               /*701*/
               SetDataViewCsr();                                        /*701*/
               goto caseENTER;

caseENTER:
              case ENTER:
                for( ith=0, NewChr=1, ptr = varptr ; ++ith <= (int)nbytes ; ++ptr ){
                    if( NewChr ){
                        IsSame = ( (NewChr = buffer[ith]) == ascbuf[ith] );
                        switch( NewChr ){
                          case ACHAR_NULL:
                          case ACHAR_NONA:
                            NewChr = 0x100;  /* use 0x00 and keep going */
                    }   }
                    if( NewChr && IsSame )
                        continue;
/*110 remove ! */   if(  PutAppData(ptr, 1, (uchar*)&NewChr, sfx)  ){
                        fmterr( CantZapMsg );
                        goto Grumble;
                }   }
               goto caseesc;

caseesc:
              case ESC:

                buffer[0] = Attrib(vaStgVal);                           /*701*/
                putrc( row, ASCIICOL-1, buffer );                       /*701*/

                Vputbox( original, row+1, col, ABOXROWS, ABOXCOLS );
                goto Fini;

              case LEFTMOUSECLICK:                                      /*701*/
              /***************************************************************/
              /* if we get a left button click while in the data window   701*/
              /* then simply recirculate and come back in at the new      701*/
              /* cursor position.                                         701*/
              /***************************************************************/
                if( GetEventView() != DATAVIEW )                        /*701*/
                {                                                       /*701*/
                 rc = key;                                              /*701*/
                 key = ENTER;                                           /*701*/
                 goto caseENTER;                                        /*701*/
                }                                                       /*701*/
                SetDataViewCsr();                                       /*701*/
                rc = RECIRCULATE;                                       /*701*/
                key = ENTER;                                            /*701*/
                goto caseENTER;                                         /*701*/
                                                                        /*701*/

              default:
              Grumble:
                beep();
            }
        }
    }
    Fini:
        /*********************************************************************/
        /* Before finishing check the key, if it is ENTER key return back 514*/
        /* or else depending on the key set up the index and incr values  514*/
        /* since for all other keys we go back to hexfield editing we have514*/
        /* to update the data read and we refresh that row on the screen  514*/
        /* with a call to show hex bytes.                                 514*/
        /*********************************************************************/
        switch (key)                                                    /*514*/
        {                                                               /*514*/
          case TAB:                                                     /*514*/
          case DATAKEY:                                                 /*514*/
          case RIGHT:                                                   /*514*/
            ith = 0;                                                    /*514*/
            incr = 0;                                                   /*514*/
            goto GoBkstart;                                             /*514*/
          case S_TAB:                                                   /*514*/
          case LEFT:                                                    /*514*/
            if ( ith == -1)             /* are we at the Starting ??      514*/
               ith = 17;                /* if yes go to ascii field       514*/
            else                        /* else                           514*/
               ith--;                   /* move back one byte position.   514*/
            incr = -1;                                                  /*514*/
GoBkstart:                                                              /*514*/
            dp = GetAppData(varptr, limit, &nbytes, sfx);           /*521 514*/
            memcpy( rawdata, dp, nbytes );                              /*514*/
            ShowHexBytes(dfp, row, 1, skip);                            /*514*/
            goto GotoNewByte;                                           /*514*/
          default:                                                      /*514*/
            Tfree( (void*)original);                                 /*521 514*/
            if( (key==UP) || (key==DOWN) )                              /*701*/
             return(key);                                               /*701*/
            return( rc );                                               /*514*/
        }                                                               /*514*/
}

  void
FmtAdrBox(uchar *boxptr,uint addr)
{
    uchar c;                            /* was register.                  112*/
    uchar *fmtptr, *bufptr, buffer[9];

    bufptr  = buffer;                                                   /*202*/
    utox8(addr, buffer);                                                /*202*/

    fmtptr = (uchar*) AboxFormat;

    for(;;)
    {
        c = *fmtptr++;
        switch( c )
        {
          case 0:
            return;

          case ' ':
            boxptr += 2;
            break;

          case '#':
          default:
           if( c == '#')
            c = *bufptr++;

          {
           uchar attr1;
           uchar attr2;
            *boxptr++ = c;  /* set character */
            attr1 = FG_white|FG_light;
            attr2 = VideoMap[vaStgVal] & (uchar)~FG_attribute;
            *boxptr++ = attr2 | attr1;
          }
    }   }
}

x2tou(uchar *hexstr,uchar *valptr)
{
    uint c, n, mask, shift;             /* was register.                  112*/

    shift = 4;
    mask = 0xF0;
    for( n=0 ; n < 2 ; ++n, shift -= 4, mask >>= 4 ){
        if( ((c = hexstr[n] - '0') > 9)
         && (((c = hexstr[n] - ('A'-10)) < 10) || (c > 15))
          ) return( FALSE );
        *valptr &= ~mask;
        *valptr |= (c << shift);
    }
    return( TRUE );
}


/*****************************************************************************/
/* GetIthHexField()                                                       701*/
/*                                                                        701*/
/* Description:                                                           701*/
/*   Get the ith field that a mouse event has occurred in.                701*/
/*                                                                        701*/
/* Parameters:                                                            701*/
/*   Event      The event that we're testing.                             701*/
/*                                                                        701*/
/* Return:                                                                701*/
/*                                                                        701*/
/*  ith         The hex field that the event occurred in.                 701*/
/*                                                                        701*/
/* Assumptions                                                            701*/
/*  none                                                                  701*/
/*                                                                        701*/
/*****************************************************************************/
#define FIELDSIZE 3                                                     /*701*/
#define FRAMESIZE (4*FIELDSIZE+1)                                       /*701*/
#define EVENTNOTINFIELD 99                                              /*701*/
#define ASCII_ITH       16                                              /*701*/
                                                                        /*701*/
int GetIthHexField( PEVENT Event )                                      /*701*/
{                                                                       /*701*/
 int    frame;                                                          /*701*/
 int    column;                                                         /*701*/
 int    FieldStartCol;                                                  /*701*/
 int    delta;                                                          /*701*/
 int    ith;                                                            /*701*/
 int    area;                                                           /*701*/
                                                                        /*701*/
 /************************************************************************701*/
 /* If the event is in the ascii field, we only need to return the        701*/
 /* initial index of the field simply to distinguish it from an index     701*/
 /* in the storage area.                                                  701*/
 /************************************************************************701*/
 area = GetStorageArea(Event);                                          /*701*/
 if( area == ASCIIAREA )                                                /*701*/
  return(ASCII_ITH);                                                    /*701*/
 /************************************************************************701*/
 /* - get absolute column number relative to 0 index.                     701*/
 /* - convert to column number relative to STGCOL, the start of editable  701*/
 /*   storage.                                                            701*/
 /* - get the frame the event occurred in.                                701*/
 /* - get the ith field that the event occurred in.                       701*/
 /* - test to see if the event occurred in the spaces between the fields  701*/
 /*   and if it did, then return a "not in field" indication.             701*/
 /* -                                                                     701*/
 /* -                                                                     701*/
 /* - storage layout is like so:                                          701*/
 /*                                                                       701*/
 /*        __STGCOL - 1 ( the -1 makes STGCOL relative to a 0 index.)     701*/
 /*       |                                                               701*/
 /*       |                                                               701*/
 /*       |-frame 0---||-frame 1---||-frame 2---||-frame 3---|            701*/
 /*       bb bb bb bb  bb bb bb bb  bb bb bb bb  bb bb bb bb              701*/
 /*       ----------------------------------------------------            701*/
 /* ith   0  1  2  3   4  5  6  7   8  9  1  1   1  1  1  1               701*/
 /*                                       0  1   2  3  4  5               701*/
 /*                 111111111122222222223333333333444444444455            701*/
 /*       0123456789012345678901234567890123456789012345678901            701*/
 /*                                                                       701*/
 /************************************************************************701*/
 column  = Event->Col;                                                  /*701*/
 column = ( column - (STGCOL-1) );                                      /*701*/
                                                                        /*701*/
 frame = column/FRAMESIZE;                                              /*701*/
 ith = frame*4 + (column - frame*FRAMESIZE)/FIELDSIZE;                  /*701*/
                                                                        /*701*/
 FieldStartCol = ith*FIELDSIZE + ith/4;                                 /*701*/
 delta =  column - FieldStartCol;                                       /*701*/
 if( delta != 0 && delta != 1 )                                         /*701*/
  ith = EVENTNOTINFIELD;                                                /*701*/
 return(ith);                                                           /*701*/
                                                                        /*701*/
}                                                                       /*701*/
/*****************************************************************************/
/* GetStorageArea()                                                       701*/
/*                                                                        701*/
/* Description:                                                           701*/
/*   Get the area of the data window that an event has occurred in.       701*/
/*                                                                        701*/
/* Parameters:                                                            701*/
/*   Event      The event that we're testing.                             701*/
/*                                                                        701*/
/* Return:                                                                701*/
/*                                                                        701*/
/*  area         - edit area                                              701*/
/*               - storage area                                           701*/
/*               - ascii area                                             701*/
/*                                                                        701*/
/* Assumptions                                                            701*/
/*  none                                                                  701*/
/*                                                                        701*/
/*****************************************************************************/
int GetStorageArea( PEVENT Event )                                      /*701*/
{                                                                       /*701*/
 int    area = 0;                                                       /*701*/
                                                                        /*701*/
 if( GetEventView() != DATAVIEW )                                       /*701*/
  return(0);                                                            /*701*/
                                                                        /*701*/
 else if( Event->Col < STGCOL - 1 )                                     /*701*/
  area = EDITAREA;                                                      /*701*/
                                                                        /*701*/
 else if( Event->Col < ASCIICOL - 1 )                                   /*701*/
  area = STORAGEAREA;                                                   /*701*/
                                                                        /*701*/
 else                                                                   /*701*/
  area = ASCIIAREA;                                                     /*701*/
 return( area );                                                        /*701*/
}                                                                       /*701*/
