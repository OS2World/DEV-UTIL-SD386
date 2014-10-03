/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   browse.c                                                                */
/*                                                                           */
/* Description:                                                              */
/*   browse any text file.                                                   */
/*                                                                           */
/* History:                                                                  */
/*                                                                           */
/*   02/08/91 Creation of 32-bit SD86, from 16-bit version.                  */
/*                                                                           */
/*...16->32 port.                                                            */
/*...                                                                        */
/*... 02/08/91  100   Philip    port to 32 bit.                              */
/*... 02/08/91  101   Joe       port to 32 bit.                              */
/*... 02/08/91  105   Christina port to 32 bit.                              */
/*... 02/08/91  106   Srinivas  port to 32 bit.                              */
/*... 02/08/91  110   Srinivas  port to 32 bit.                              */
/*... 02/08/91  111   Christina port to 32 bit.                              */
/*... 02/08/91  112   Joe       port to 32 bit.                              */
/*...                                                                        */
/*... 08/22/91  234   Joe       PL/X gives "varname" is incorrect message    */
/*...                           when entering a parameter name in the data   */
/*...                           window.  This happens when the cursor is on  */
/*...                           an internal procedure definition statement   */
/*...                           and you use F2 to get into the data window   */
/*...                           and then type the name.                      */
/*... 02/12/92  521   Srinivas  Port to C-Set/2.                             */
/*...                                                                        */
/**Includes*******************************************************************/

#include "all.h"                        /* SD86 include files                */

/**Defines *******************************************************************/


/**External declararions******************************************************/

extern uint     LinesPer;               /* current # lines/screen for code   */
extern KEY2FUNC defk2f[];               /*                                   */
extern uint     TopLine;                /* current top line for code         */
extern uint     VideoRows;              /*                                   */
extern char     BrowseSearchBuffer[ PROMAX+1 ];
extern UINT     FnameRow;               /* screen row for file name (0..N)   */

/**Static definitions ********************************************************/
                                        /*                                   */
static uint   prohids[] =               /*                                   */
{
  HELP_DLG_BROWSE,                      /* context help for filename entry   */
  HELP_DLG_FIND,                        /* context help for find string.     */
  HELP_WIN_BROWSE                       /* context help for browse window.   */
};                                      /*                                   */
                                        /*                                   */
extern uint slen;                       /* length of search string        110*/
extern uint str_fnd_flag;               /* flag to signal reverse video   110*/

/*****************************************************************************/
/* browse                                                                    */
/*                                                                           */
/* Description:                                                              */
/*   Browse any file.                                                        */
/*   Overview:                                                               */
/*    1. Prompt the user for a filename.                                     */
/*    2. Build a fully qualified filespec from the filename.                 */
/*    3. Allocate memory for the afile.                                      */
/*    4. Allocate memory for the source and the source index.                */
/*    5. Load the source and build the index.                                */
/*    7. Build afile info.                                                   */
/*    8. Process user input keys.                                            */
/*    9. Free up temp memory allocations.                                    */
/*                                                                           */
/* Parameters:                                                               */
/*   none                                                                    */
/*                                                                           */
/* Return:                                                                   */
/*   ???????????????????????????????                                         */
/*                                                                           */
/*****************************************************************************/
  int                                   /*                                   */
browse( char *FileName, int InitialLineNumber  )
{                                       /*                                   */
 uint      n;                           /* just a number                     */
 uint      Nlines;                      /* num of lines in the source buffer */
 uint      Tlines;                      /* total lines in the file.          */
 SEL       srcsel;                      /* source buffer selector.        110*/
 SEL       offsel;                      /* offset buffer selector.        110*/
 uint      srcseglen;                   /* actual size of source buffer      */
 AFILE    *fp;                          /* temp afile structure.             */
 uchar    *fnfull;                      /* fully qualified filespec.         */
 int       fnlen;                       /* filename/filespec buffer length.  */
 int       func;                        /* function associated with a key.   */
 uint      rc;                          /* return code.                      */
 uchar     hlight[4];                   /* attributes for find string     110*/
 int       OffsetTableBufSize;          /* size alloc'd for offtab.       234*/
 ushort   *offtab;                      /* table of ptrs to src txt lines 234*/
                                        /*                                   */
/*****************************************************************************/
/* The first thing we do is get a filename from the user and build a fully   */
/* qualified filespec. The search order is:                                  */
/*                                                                           */
/*   1. explicit filespec.                                                   */
/*   2. current directory.                                                   */
/*   3. SD386SRC environment variable.                                    116*/
/*   4. PATH.                                                                */
/*                                                                           */
/*****************************************************************************/
 rc = 0;                                /* assume all is well.               */
 fp = NULL;                             /* intialize null afile block.       */
 fnlen=CCHMAXPATH;                      /* 1 byte of len + 128 of char + \0  */

 fnfull=Talloc(fnlen);                  /* allocate space for a filespec. 521*/
 rc=FindExe(FileName, fnfull+1 , fnlen);/* search for filename/build filespec*/
/* Tfree( (void*)fn);  */                /* done with the file name.       521*/
 if(rc)                                 /* if we could not locate the file   */
 {                                      /* name then                         */
  Tfree((void*)fnfull);                  /* free the storage allocated for 521*/
  return(rc);                           /* file name & return with can't     */
 }                                      /* find message.                     */
                                        /* build a length prefixed  filespec.*/
 n=strlen(fnfull+1);                    /* compute filespec length.          */
 *fnfull=(uchar)n;                      /* insert length in first byte.      */
 ClrScr( FnameRow, FnameRow, vaInfo);   /* clear fname row and show the user */
 putrc( FnameRow, 0, fnfull+1);         /* his fully qualified filespec.     */
                                        /*                                   */
/*****************************************************************************/
/* Now we will proceed to build an afile which can be used for browsing the  */
/* file. First, we load the source into some allocated buffers for the raw   */
/* source and an index of offsets to the text lines. We make an initial      */
/* guess as to how big these buffers should be, then, we reallocate them     */
/* after we know how big they should be for real.                            */
/*                                                                           */
/*****************************************************************************/
 fp = (AFILE*) Talloc(SizeOfAFILE(n));  /* allocate memory for this afile.521*/
 memcpy( fp->filename, fnfull, n+1 );   /* copy filename in afile struct. 101*/
 Nlines = 0;                            /* initialize # of source lines      */
 Tlines = 0;                            /* initialize # of file lines.       */
 srcsel = 0;                            /* "        " source buffer selector.*/
 offsel = 0;                            /* "        " offset buffer selector.*/
 rc=DosAllocSeg(0,(PSEL)&srcsel,0);     /* allocate 64k for source buffer.   */
 if(rc){rc=2;goto errorexit;}           /* error check.                      */
                                        /*                                   */
 rc=DosAllocSeg(20*1024,(PSEL)&offsel,0);/* allocate 20k for offset index.   */
 if(rc){rc=2;goto errorexit;}           /* error check.                      */
                                        /*                                   */
 LoadSource(fp->filename+1  ,           /* input: source filespec.           */
   ( uchar *) Sel2Flat(srcsel) ,        /* output: source buffer sel 111  110*/
   ( ushort *)Sel2Flat(offsel) ,        /* output: txt line offst buf sel 110*/
              0                ,        /* input: no skip lines at file start*/
   (ushort *) &srcseglen       ,        /* output: # of src buf bytes used111*/
              &Nlines          ,        /* output: # of src lines in buffer  */
              &Tlines                   /* output: "               " file    */
             );
 if(Nlines == 0)                        /* if there are no source lines for  */
 {rc=4;goto errorexit;}                 /* removed rc check                  */
                                        /*                                   */
                                        /*                                   */
 fp->flags = 0x00;                      /* clear the flags.                  */
 fp->Tlines = Tlines;                   /* put # of src file lines in afile. */
 if( Tlines > Nlines )                  /* does compressed source exceed 64k?*/
  fp->flags |= AF_HUGE;                 /* mark afile with huge file flag    */
 fp->Nlines = Nlines;                   /* # of lines in buffer into afile111*/
 fp->source = (uchar *)Sel2Flat(srcsel);/* -> to source text into afile111110*/

/*****************************************************************************/
/* Allocate the offtab[] buffer to hold Nlines + 1 offsets. We add the    234*/
/* so that we can make the offset table indices line up with the          234*/
/* source line numbers.                                                   234*/
/*****************************************************************************/
 OffsetTableBufSize = (Nlines+1)*sizeof(ushort);                        /*234*/
 offtab = (ushort*) Talloc(OffsetTableBufSize);                     /*521 234*/
 memcpy(offtab + 1, (uchar*)Sel2Flat(offsel), Nlines * sizeof(ushort) );/*234*/
 fp->offtab = offtab;                                                   /*234*/

 if( offsel )                           /* if there was an offset segment 234*/
  DosFreeSeg(offsel);                   /* allocated then free it up      234*/

 fp->topline = 1;                       /* no. put it on the top line     234*/
 fp->csrline = 1;                       /* no. put it on the top line     234*/

 if( InitialLineNumber != 0 )
  fp->csrline = InitialLineNumber;

 fp->pdf = NULL;                        /*                                110*/
 scrollwindow( fp, InitialLineNumber ); /* display the initial window.       */

/*****************************************************************************/
/* Now, we are going to process the user's keys.                             */
/*                                                                           */
/*****************************************************************************/
 for(;;)                                /* begin loop to process keys.       */
 {                                      /*                                   */
  fmtpos( fp );                         /* put "line xx of yy" message out   */
  fp->csr.row = ( uchar )( fp->csrline- /* set cursor line in source buffer  */
                  fp->topline +         /* - source line at top of display   */
                  TopLine );            /* + topline for the code            */
  PutCsr( ( CSR * )&fp->csr );          /* set the cursor position           */
  if (str_fnd_flag == 1)                /* if the string found flag is    110*/
  {                                     /* set by scan function then      110*/
     hlight[0] = 0xFF;                  /* attribute string for highlight 110*/
     hlight[1] = (uchar)slen;           /* attribute string for highlight 110*/
     hlight[2] = Attrib(vaXline);       /* attribute string for highlight 110*/
     hlight[3] = 0;                     /* attribute string for highlight 110*/
     putrc(fp->csr.row,fp->csr.col,&hlight[0]);  /*                       110*/
     str_fnd_flag = 0;                  /* highlight the string and reset 110*/
  }                                     /* the flag                       110*/

  SetMenuMask( BROWSEFILE );                                            /*701*/
  #define NOHANDLEESC 1                                                 /*701*/
  func = GetFuncsFromEvents( NOHANDLEESC, NULL ); /*                     701*/
  switch( func )                        /* switch on function selection      */
  {                                     /* begin function processing         */
   case QUIT:
   case ESCAPE:
    goto errorexit;                     /* go to exit processing.            */

   case UPCURSOR:                       /* move cursor up one line           */
    fp->csrline -= 1;                   /* update afile with current csr line*/
                                        /* removed call to dovcsr            */
    if( !IsOnCRT(fp) )                  /* if the cursor goes off the screen */
     {                                  /*                                   */
      fp->csrline += 1;                 /* reset afile value before update   */
      scrollwindow(fp, -1);             /* move the screen up   one line     */
     }                                  /*                                   */
    break;                              /*                                   */
                                        /*                                   */
   case DOWNCURSOR:                     /* move the cursor down one line     */
                                        /* removed call to dovcsr            */
    fp->csrline += 1;                   /* update afile with current csr line*/
    if( !IsOnCRT(fp) )                  /* if the cursor goes off the screen */
     {                                  /*                                   */
      fp->csrline -= 1;                 /* reset afile value before update   */
      scrollwindow(fp, 1);              /* move the screen down one line     */
     }                                  /*                                   */
    break;                              /*                                   */
                                        /*                                   */
   case PREVWINDOW:                     /* show previous screen              */
    scrollwindow(fp,-(int)LinesPer);    /* display the initial window.    521*/
    break;                              /*                                   */
                                        /*                                   */
   case NEXTWINDOW:                     /* show next screen                  */
    scrollwindow( fp,  LinesPer );      /* display the initial window.       */
    break;                              /*                                   */
                                        /*                                   */
   case FIRSTWINDOW:                    /* show top of file.                 */
    scrollwindow(fp,-(int)fp->Tlines);  /* display the initial window234521*/
    fp->csrline = fp->topline;          /*                                   */
    break;                              /*                                   */
                                        /*                                   */
   case LASTWINDOW:                     /* show the bottom of the file.   234*/
    scrollwindow( fp, fp->Tlines - LinesPer + 1);
    fp->csrline=fp->topline+fp->Nshown-1;/* cursor on bottom line            */
    break;                              /*                                   */
                                        /*                                   */
   case TOPOFWINDOW:                    /* move cursor to top line on screen */
    fp->csrline = fp->topline;          /* update afile t put cursor on top  */
    break;                              /*                                   */
                                        /*                                   */
   case BOTOFWINDOW:                    /* move cursor to last line on screen*/
    fp->csrline=fp->topline+fp->Nshown-1;/*                                  */
    break;                              /*                                   */
                                        /*                                   */
/*****************************************************************************/
/* These are the find and repeat find functions.                             */
/*                                                                           */
/*****************************************************************************/
   case FIND:
    FindStr( fp );
    scrollwindow(fp,0);
    break;

   case REPEATFIND:
    rc = (uint)ScanStr(fp);
    scrollwindow(fp,0);
    if( rc == 0 )
     fmterr( "Can't find that" );
    rc = 0;
    break;

                                        /*                                   */
   case GENHELP:                        /* General help                   701*/
   {                                                                    /*701*/
     Help(prohids[2]);
     break;                              /*                                  */
   }                                                                    /*701*/

   case HELP:                           /* move cursor to last line on scr   */
     HelpScreen( );
     break;                             /* for now, no context sensitive help*/
  }                                     /* end of switch on user functions.  */
 }                                      /* end of process key loop.          */
                                        /*                                   */
errorexit:                              /*                                   */
 if( fp->source  &&                     /* free the DosAlloc'd source buffer.*/
     DosFreeSeg(Flat2Sel(fp->source))   /* if it doesn't free then call it110*/

   )                                    /*                                   */
  rc = 2;                               /* a memory allocation error.        */
                                        /*                                   */
 if( fp->offtab )                       /* free the DosAlloc'd offset buffer.*/
     Tfree((char *)fp->offtab);          /* handle error same as above.521 234*/
                                        /*                                   */
 if( fnfull )                           /* free the malloc'd filespec.       */
  Tfree((void*)fnfull);                  /*                                521*/
 Tfree((void*)fp);                       /* free the temp afile structure. 521*/
 return(rc);                            /* display error msg; ret. to caller */
}                                       /*                                   */
                                        /*                                   */
/*****************************************************************************/
/*  scrollwindow()                                                           */
/*                                                                           */
/* Description:                                                              */
/*   move the screen vertically n lines.                                     */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   fp        input - the afile for this dfile node.                        */
/*   n         input - # of lines to scroll.                                 */
/*                     a large value means move a full screen.               */
/*                                                                           */
/* Return:                                                                   */
/*   void                                                                    */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
 void                                   /*                                   */
scrollwindow(AFILE *fp,int n)           /* number of lines to move screen    */
{                                       /*                                   */
 int  newtop;                           /* file line at top of screen.       */
 int  csrdelta;                         /* cursor offset from topline        */
 int  target;                           /* potential top line in file        */
 int  bias;                             /* lines skipped in the file for     */

                                        /* HUGE files.                       */
 newtop = fp->topline + n;              /* define the new top line           */
 csrdelta = fp->csrline - fp->topline;  /* define lines between csr and top  */
                                        /*                                   */
 Recheck:                               /*                                   */
                                        /*                                   */
 bias = fp->Nbias;                      /*                                   */
/*****************************************************************************/
/*                                                                           */
/* At this point we first check to see if we're going beyond the lower line  */
/* boundary of the source buffer.  If we are then there could still be source*/
/* left in the file above the lines contained in the source buffer.  This is */
/* the case of a HUGE file meaning that the 64k source buffer could not hold */
/* all of the source lines and we need to back up in the file.               */
/*                                                                           */
/* If we have a HUGE file and all lines have not been read in, then we need  */
/* to page the source buffer down in the file replenishing the source buffer.*/
/* We then need to update the afile block and resynchronize the display.     */
/*****************************************************************************/
 if( newtop < 1 )                       /* trying to go before first line 234*/
 {                                      /*                                   */
  if( (fp->flags & AF_HUGE) &&          /* is this a HUGE file and the       */
       fp->Nbias                        /* buffer was previously paged down ?*/
    )                                   /*                                   */
  {                                     /*                                   */
   target = newtop + fp->Nbias;         /* yes on both. define new target    */
   if( target < 1 )                     /* if target is before file begin 234*/
    target = 1;                         /* then clip it at 0.             234*/
                                        /*                                   */
   scrollfile( fp, target );            /* now page up in the file        234*/
                                        /*                                   */
   Resync:                              /*                                   */
                                        /*                                   */
   newtop = target - fp->Nbias;         /* define the new top of the file    */
   fp->hotline += bias - fp->Nbias;     /* define the new hotline            */
   goto Recheck;                        /* do it again just for being a jerk */
  }                                     /*                                   */
                                        /*                                   */
  newtop = 1;                           /* can't move before first line   234*/
                                        /*                                   */
 }
/*****************************************************************************/
/*At this point we first check to see if we have enough lines left in the    */
/*buffer to fill up the display.  If we don't, we could still have source    */
/*left in the file that has not been read into the source buffer.  This is   */
/*the case of a HUGE file meaning that the 64k source buffer could not hold  */
/*all of the source lines and we still need to read some in.                 */
/*                                                                           */
/*If we have a HUGE file and all lines have not been read in, then we need   */
/*to page the source buffer down in the file replenishing the source buffer. */
/*We then need to update the afile block and resynchronize the display.      */
/*                                                                           */
/*****************************************************************************/
 else                                   /*                                   */
 {                                      /*                                   */
  if( (uint)newtop + LinesPer -1 > fp->Nlines )
  {                                     /*need more lines than in buffer  234*/
   if((fp->flags & AF_HUGE) &&          /* yes. Is this a HUGE file and not  */
      (fp->Nlines+fp->Nbias < fp->Tlines)/* all lines have been read?        */
     )                                  /*                                   */
   {                                    /*                                   */
    target = newtop + fp->Nbias;        /* true again. define the target line*/
    if( (uint)target > fp->Tlines )     /* is the target past end of file?   */
     target = fp->Tlines - LinesPer+1;  /* yes. display last LinesPer in file*/
     scrollfile(fp,target+LinesPer-1);
                                        /*                                   */
    goto Resync;                        /* resync the display                */

   }                                    /*                                   */
   newtop = fp->Nlines - LinesPer + 1;  /* display last LinesPer in file  234*/
   if( newtop < 1 )                     /* don't let the newtop go above file*/
    newtop = 1;                         /* 0 is the minimum                  */
  }                                     /*                                   */
 }                                      /*                                   */
 fp->topline = newtop;                  /* put new topline in afile          */
 fmttxt( fp );                          /* format the text for this afile    */
 if( IsOnCRT(fp) == FALSE )
  fp->csrline = fp->topline + csrdelta; /* put new csrline in afile          */
}                                       /*                                   */
/*****************************************************************************/
/*  scrollfile()                                                             */
/*                                                                           */
/* Description:                                                              */
/*   move the file vertically.                                               */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   fp        input - the afile for this dfile node.                        */
/*   lno       input - file line # we're shooting for.                       */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*    none                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
 void                                   /*                                   */
scrollfile(AFILE *fp,uint lno)
{                                       /*                                   */
 SEL       srcsel;                      /* selector for source buffer.    110*/
 SEL       offsel;                      /* selector for offset buffer.    110*/
 uint      line;                        /* file line for new bias.           */
 uint      Nlines;                      /* # lines in the source buffer.     */
 uint      srcseglen;                   /*                                   */
 uint      retry;                       /* flag for retry loadsource.        */
 int       OffsetTableBufSize;          /* size alloc'd for offtab.       234*/
 ushort   *offtab;                      /* table of ptrs to src txt lines 234*/

/*****************************************************************************/
/* When we come in, lno is a line number beyond the range of lines currently */
/* in the source buffer. The first thing we do is make a few sanity checks   */
/* on lno.                                                                   */
/*                                                                           */
/*****************************************************************************/
                                        /* check the following:              */
 if( (lno == 0)  ||                     /*  1. a zero line number.           */
     ((lno > fp->Nbias) &&              /*  2. the line number falls in the  */
      (lno <= (fp->Nbias + fp->Nlines)) /*     bounds of the remaining file. */
     ) ||                               /*                                   */
     !(fp->flags & AF_HUGE)             /*  3. compressed source > 64K.      */
   ) return;                            /*                                   */

    if( lno > LINESBEFORE )
        line = lno - LINESBEFORE;
    else
        line = 1;
/*****************************************************************************/
/* Now, we want to allocate new segments for the source and offset buffers.  */
/*                                                                           */
/*                                                                           */
/*                                                                           */
/*****************************************************************************/
 srcsel = 0;                            /*                                   */
 offsel = 0;                            /*                                   */
 retry  = 0;                            /*                                   */
 Nlines = 0;                            /*                                   */
 DosAllocSeg(0,(PSEL)&srcsel,0);        /* allocate 64k for source buffer.   */
 DosAllocSeg(20*1024,(PSEL)&offsel,0);  /* allocate 20k for offset index.   */
                                        /*                                   */
 Retry:                                 /*                                   */
 LoadSource(fp->filename+1  ,           /* input: source filespec.           */
   (uchar *)  Sel2Flat(srcsel) ,        /* output: source buffer sel 111  110*/
   ( ushort *)Sel2Flat(offsel) ,        /* output: txt line offst buf sel 110*/
                line - 1       ,        /* input: no skip lines at file start*/
    (ushort *)&srcseglen       ,        /* output: # of src buf bytes used111*/
              &Nlines          ,        /* output: # of src lines in buffer  */
             NULL );                    /* output: don't cnt total lns again.*/
                                        /*                                   */
 fp->Nlines = Nlines;                   /* put # src buffer lines in afile.  */
                                        /*                                   */
 if( ( (lno+VideoRows)>(line+Nlines) )  /* this code is trying to sync the   */
     && !retry )                        /* end of the buffer, the end of the */
 {                                      /* file and the screen.              */
  if( lno > VideoRows )                 /* ???                               */
   line = lno - VideoRows;              /* ???                               */
  else                                  /* ???                               */
   line = 1;                            /* ???                               */
  retry = 1;                            /* ???                               */
  goto Retry;                           /* ???                               */
 }                                      /*                                   */
 fp->Nbias  = line - 1;                 /* establish the new top line in buf.*/
                                        /*                                   */
 if( fp->offtab )                       /* if necessary, free up the old     */
     Tfree((char *)fp->offtab);          /* offset buffer.             521 234*/
/*****************************************************************************/
/* Allocate the offtab[] buffer to hold Nlines + 1 offsets. We add the    234*/
/* so that we can make the offset table indices line up with the          234*/
/* source line numbers.                                                   234*/
/*****************************************************************************/
 OffsetTableBufSize = (Nlines+1)*sizeof(ushort);                        /*234*/
 offtab = (ushort*) Talloc(OffsetTableBufSize);                     /*521 234*/
 memcpy(offtab + 1, (uchar*)Sel2Flat(offsel), Nlines * sizeof(ushort) );/*234*/
 fp->offtab = offtab;                                                   /*234*/

 if( offsel )                           /* if there was an offset segment 234*/
  DosFreeSeg(offsel);                   /* allocated then free it up      234*/
 if( fp->source )                       /* if necessary, free up the old     */
     DosFreeSeg(Flat2Sel(fp->source) ); /* source buffer.                 110*/
 fp->source = (uchar *)Sel2Flat(srcsel); /* insert -> source buf into 110 111*/
 return;                                /*                                   */
}
