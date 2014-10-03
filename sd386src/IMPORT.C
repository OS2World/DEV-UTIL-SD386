/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   import.c                                                                */
/*                                                                           */
/* Description:                                                              */
/*   Functions for handling imports.                                         */
/*                                                                           */
/*                                                                           */
/* History:                                                                  */
/*   06/18/90 Created                                                        */
/*                                                                           */
/*...Release 4.00                                                            */
/*...                                                                        */
/*... 05/23/90  604   add handling for imported variables.                   */
/*                                                                           */
/*...Release 5.00                                                            */
/*...                                                                        */
/*... 11/23/92  803   Selwyn - 32 bit porting.                               */
/*...                                                                        */
/*... 03/22/93  815   Selwyn - Workaround for compiler bug. 0:32 should be   */
/*                             16:16.                                        */
/**Includes*******************************************************************/
/*                                                                           */
/**Includes*******************************************************************/
                                        /*                                   */
#include "all.h"                        /* SD86 include files                */
                                        /*                                   */
/**Defines *******************************************************************/
                                        /*                                   */
 #define     MRT    0X28                /* location of Module Reference Table*/
 #define     INTa   0X2A                /* location of Import Name Table.    */
 #define     RNT    0X26                /* location of Resident Name Table.  */
 #define     ET     0X04                /* location of Entry Table.          */
                                        /*                                   */
/**External declararions******************************************************/
                                        /*                                   */
extern PROCESS_NODE *pnode;                                             /*827*/
                                        /*                                   */
/**External definitions*******************************************************/
                                        /*                                   */
/**Static definitions ********************************************************/
                                        /*                                   */
/**Begin Code*****************************************************************/
                                        /*                                   */
/*****************************************************************************/
/* ResolveImport()                                                           */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Resolve a data reference imported by name from a 16 bit DLL.            */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   import     input - -> to the import name. name=length specified string. */
/*   ImportPdf  input - -> to the "importing" debug file.                    */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   addr       the resolved address of the import.                          */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   ImportPdf is valid and is the correct importing file.                   */
/*                                                                           */
/*****************************************************************************/
uint   ResolveImport( char *import, DEBFILE *ImportPdf )
{                                       /*                                   */
 char     buffer[2];                    /* file read buffer.                 */
 ulong    StartOS2Hdr;                  /* location of the OS/2 EXE header.  */
 uint     MRToffset;                    /* MRT offset relative to start of   */
                                        /* OS/2 header.                      */
 uint     INToffset;                    /* INT offset relative to start of   */
                                        /* OS/2 header.                      */
 ulong    MRTloc;                       /* absolute file location of MRT.    */
 ulong    INTloc;                       /* absolute file location of INT.    */
 ulong    mrtoff;                       /* offset into  MRT.                 */
 ulong    intoff;                       /* offset into  INT.                 */
 HFILE    fh;                           /* -> to c-runtime file structure.   */
 int      namelen;                      /* length of exporting dll/exe.      */
 char     modname[129];                 /* z string dll/exe filename.        */
 DEBFILE *ExportPdf;                    /* -> to "exporting" deb file        */
 uint     addr;                         /* resolved import address.          */
 int      rc = 0;
 uchar    ExeType;                                                      /*803*/
/*****************************************************************************/
/* We have an import name and the dll/exe that imported it.                  */
/* Our job is to find who exported this guy.                                 */
/*                                                                           */
/* Here's what we do to resolve this import:                                 */
/*                                                                           */
/* 1. Get the file offset of the Module Reference Table(MRT) for the         */
/*    importing file.  The OS/2 EXE header contains the offset of the MRT at */
/*    location 28H.  The offset given is a word offset relative to the       */
/*    beginning of the OS/2 header.  The table is terminated with a null     */
/*    byte.                                                                  */
/*                                                                           */
/* 2. For each MRT entry we will get the name of a dll/exe from the Imported */
/*    Name Table(INT).  We will search the Resident Name Table(RNT) for each */
/*    of these INT files to find the one that exported our import.           */
/*                                                                           */
/* 3. Once we have found the RNT entry in the dll/exe that exported our      */
/*    import, we will extract the index part of the RNT entry. The index     */
/*    is an entry table(ET) index.                                           */
/*                                                                           */
/* 4. We will search the ET for the bundle containing the index of the       */
/*    import/export.  The ET bundle will contain the segment # and offset    */
/*    location of the import.  We will call DosDebug  to convert the         */
/*    segment # to a selector for this process.                              */
/*                                                                           */
/* 5. Some pertinent locations relative to start of OS/2 header:             */
/*                                                                           */
/*    MRT   Module Reference Table  28H word offset.                         */
/*    INT   Imported Name Table     2AH word offset.                         */
/*    RNT   Resident Name Table     26H word offset.                         */
/*    ET    Entry Table             04H word offset.                         */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*                                                                           */
/*                                                                           */
/*****************************************************************************/
/* First, we check the dll/exe to see if it has debug info.  If it does, then*/
/* the file will be open.  If it doesn't, we will have to open it and close  */
/* it when we're done.                                                       */
/*****************************************************************************/
 if(ImportPdf->DebugOff == 0L )         /* if no debug info, then            */
 {                                      /*                                   */
  rc=opendos(ImportPdf->DebFilePtr->fn,"rb",&fh);
                                        /* open the file for binary read */
  if ( rc != NO_ERROR )                 /* if it doesn't open then           */
   return( NULL );                      /* we're sol.                        */
  ImportPdf->DebFilePtr->fh = fh;       /* update debug file node.           */
 }                                      /*                                   */
                                        /*                                   */
/*****************************************************************************/
/* Find the absolute locations in the importing file of the MRT and the INT. */
/*****************************************************************************/
 StartOS2Hdr = FindOS2hdr(ImportPdf);   /* find start of OS2 EXE header.     */
 if(StartOS2Hdr == 0L)                  /* if we can't read the header       */
  goto error;                           /* then we're done.                  */
                                        /*                                   */
 seekf(ImportPdf,StartOS2Hdr + MRT);    /* seek loc containing MRT offset.   */
 if(readf( buffer,2,ImportPdf))         /* read MRT offset.                  */
  goto error;                           /*                                   */
                                        /*                                   */
 MRToffset = *(ushort *)buffer;         /* offset of MRT from OS/2 header.   */
 MRTloc = StartOS2Hdr + MRToffset;      /* absolute location of MRT.         */
                                        /*                                   */
 seekf(ImportPdf,StartOS2Hdr + INTa);   /* seek loc containing INT offset.   */
 if(readf( buffer,2,ImportPdf))         /* read INT offset.                  */
  goto error;                           /*                                   */
                                        /*                                   */
 INToffset = *(ushort *)buffer;         /* offset of INT from OS/2 header.   */
 INTloc = StartOS2Hdr + INToffset;      /* absolute location of INT.         */
                                        /*                                   */
 /****************************************************************************/
 /* If there is no MRT, then the import must be bogus.                       */
 /****************************************************************************/
 if( INTloc == MRTloc )                 /* test for empty MRT.               */
  return( NULL );                       /* return no can resolve.            */
/*****************************************************************************/
/*                                                                           */
/* 1. Read the MRT entries and get the associated dll/exe name from the INT. */
/*    The MRT is an array of offsets to dll/exe names in the INT. Since the  */
/*    INT begins with a 0 byte we can consider the MRT as being terminated   */
/*    by a zero byte.                                                        */
/* 2. Retrieve exports from this dll/exe looking for one that matches        */
/*     the import.                                                           */
/* 3. Repeat for all MRT entries until a match is found.                     */
/*                                                                           */
/*****************************************************************************/
 mrtoff = MRTloc;                       /* init pointer to MRT.              */
 addr = NULL;                           /* init the address.                 */
 for(;; mrtoff += 2)                    /* scan MRT.                         */
 {                                      /*                                   */
  seekf(ImportPdf,mrtoff );             /* seek MRT.                         */
  if(readf( buffer,2,ImportPdf))        /* read MRT entry for dll/exe.       */
   goto error;                          /*                                   */
  if( *(uchar*)buffer == 0)             /* end of MRT?                       */
   break;                               /*                                   */
  intoff = INTloc + *(ushort *)buffer;  /* calc offset of modname in INT.    */
  seekf(ImportPdf,intoff);              /* seek offset of modname.     .     */
  if(readf( buffer,1,ImportPdf))        /* read modname length.              */
   goto error;                          /*                                   */
  namelen = *(uchar*)buffer;            /* calc modname length.              */
  if(readf(modname,namelen,ImportPdf))  /* read the modname string.          */
   goto error;                          /*                                   */
  modname[namelen] = '\0';              /* make modname a z string.          */
                                        /* find deb file  for modname.       */
  /*********************************************************************/
  /* Get the PDF for the module which exported the variable.           */
  /*  - call findexport32 if the module which exported the variable is */
  /*    a 32 bit module.                                               */
  /*  - call findexport if the module which exported the variable is a */
  /*    16 bit module.                                                 */
  /*********************************************************************/
  ExportPdf = findpdf( modname );                                       /*803*/
  if( ExportPdf )                                                       /*803*/
  {                                                                     /*803*/
    ulong OS2Header;                                                    /*803*/
    HFILE fHandle;                                                      /*803*/

    if( ExportPdf->DebugOff == 0L )                                     /*803*/
    {                                                                   /*803*/
      rc = opendos( ExportPdf->DebFilePtr->fn, "rb", &fHandle );        /*803*/

     if ( rc != NO_ERROR )                                              /*803*/
      return( NULL );                                                   /*803*/
     ExportPdf->DebFilePtr->fh = fHandle;                               /*803*/
    }                                                                   /*803*/

    OS2Header = FindOS2hdr( ExportPdf );                                /*803*/
    if( OS2Header == 0L )                                               /*803*/
     goto error;                                                        /*803*/

    seekf( ExportPdf, OS2Header );                                      /*803*/
    readf( buffer, 1, ExportPdf );                                      /*803*/
    ExeType = BIT16;                                                    /*803*/
    if( buffer[0] == 'L' )                                              /*803*/
     ExeType = BIT32;                                                   /*803*/

    if( ExportPdf->DebugOff == 0L )                                     /*803*/
      closedos( fHandle );                                              /*803*/

    if( ExeType == BIT32 )                                              /*803*/
      addr = findexport32( ExportPdf, import );                         /*803*/
    else                                                                /*803*/
      addr = findexport( ExportPdf, import );                           /*803*/
   }                                                                    /*803*/

   if( addr )                                 /* look for matching export.   */
    break;                              /* if we find it, then we're done.   */
 }                                      /* end scan MRT.                     */
 if(ImportPdf->DebugOff == 0L )         /* if no debug info, then            */
  closedos(fh);                         /* close the file.                   */
 return( addr );                        /*                                   */
/*****************************************************************************/
/*error handling for ResolveImport().                                        */
/*****************************************************************************/
error:                                  /*                                   */
 if(ImportPdf->DebugOff == 0L )         /* if no debug info, then            */
  closedos(fh);                         /* close the file.                   */
 return( NULL );                        /*                                   */
}                                       /* end ResolveImport()               */

/*****************************************************************************/
/* findpdf()                                                                 */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Find a debug file node given a dll or exe modname.  The modname may     */
/*   include an extension.  The nodes contain the full path                  */
/*   specification for the filename and we have to dig out the modname       */
/*   part.                                                                   */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   modname    -> to modname part of modname.ext file name.                 */
/*                 ext can be dll or exe.                                    */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   pdf         -> to the debug file node for this name.                    */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   modname is a z string.                                                  */
/*   The dll has been initialized and attached.                              */
/*                                                                           */
/*****************************************************************************/
DEBFILE *findpdf( char *modname )
{
 DEBFILE *pdf;
 char    *fn;
 int      fnlen;

 for(pdf = pnode->ExeStruct;
     pdf;
     pdf=pdf->next
    )
 {
  fn = strrchr(pdf->DebFilePtr->fn,'\\');
  fn++;
  if( strchr(modname, '.') != NULL )
  {
   /**************************************************************************/
   /* - come here if the modname has an extension.                           */
   /**************************************************************************/
   if( stricmp(modname, fn) == 0 )
    return(pdf);
  }
  else
  {
   /**************************************************************************/
   /* - come here if the modname does not contain an extension.              */
   /**************************************************************************/
   fnlen = strcspn(fn,".");
   if( strlen(modname) == fnlen )
    if(strnicmp(fn,modname,fnlen) == 0 )
     return(pdf);
  }
 }
 return(NULL);
}

/*****************************************************************************/
/* findexport()                                                              */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Look for an export within this debug file RNT that matches the          */
/*   import passed as a parameter.                                           */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   pdf        input - -> to debug file node to search.                     */
/*   import     input - -> to the import name. The import name is a          */
/*                         length specified string.                          */
/* Return:                                                                   */
/*                                                                           */
/*   addr       address of the import.                                       */
/*   NULL       if address not found.                                        */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   import is a z string.                                                   */
/*   The dll has been initialized and attached.                              */
/*   The RNT will contain at least one name - the module or EXE name if      */
/*   nothing else.                                                           */
/*                                                                           */
/*****************************************************************************/
uint findexport( DEBFILE *pdf, char *import )
                                        /* find the debug file node.         */
{                                       /*                                   */
 HFILE    fh;                           /* c-runtime file structure.         */
 char     buffer[2];                    /* file read buffer.                 */
 ulong    StartOS2Hdr;                  /* location of the OS/2 EXE header   */
 uint     RNToffset;                    /* RNT offset relative to start of   */
                                        /* OS/2 header.                      */
 ulong    RNTloc;                       /* absolute file location of RNT.    */
 ulong    rntoff;                       /* offset into  RNT.                 */
 uchar    explen;                       /* length of export name.            */
 uchar    adjlen;                       /* export length adjusted for '_'.   */
 char     export[129];                  /* z string export name.             */
 char    *en;                           /* -> to export name in record.      */
 uint     etindex;                      /* ET index.                         */
 int      match;                        /* import/export match flag.         */
 uint     EToffset;                     /* ET  offset relative to start of   */
                                        /* OS/2 header.                      */
 ulong    ETloc;                        /* absolute file location of ET.     */
 ulong    etoff;                        /* offset into  ET.                  */
 uchar    entries;                      /* # entries in a bundle.            */
 uchar    bundletype;                   /* type of the bundle entry.         */
                                        /* 0  - NULL bundle.                 */
                                        /* FF - Movable segment records.     */
                                        /* nn - Fixed Segment and nn is      */
                                        /*      the segment #.               */
 uchar    segnum;                       /* segment number of ET entry.       */
 uint     off;                          /* offset of the export.             */
 int      rc = 0;                       /* return code.                      */
 int      hiindex;                      /* hi record index in bundle.        */
 int      loindex;                      /* lo record index in bundle.        */
 int      bunrecsize;                   /* size of bundle in records.        */
/*****************************************************************************/
/* First, we check the dll/exe to see if it has debug info.  If it does,     */
/* then the file will be open.  If it doesn't, we will have to open it       */
/* and close it when we're done.                                             */
/*****************************************************************************/
 if(pdf->DebugOff == 0L )               /* if no debug info, then            */
 {                                      /*                                   */
  rc=opendos(pdf->DebFilePtr->fn,"rb",&fh);
                                        /* open the file for binary read */
  if ( rc != NO_ERROR )                 /* if it doesn't open then           */
   return( NULL );                      /* we're sol.                        */
  pdf->DebFilePtr->fh = fh;             /* update debug file node.           */
 }                                      /*                                   */
/*****************************************************************************/
/* Find the absolute locations in the exporting dll/exe of the RNT.          */
/*****************************************************************************/
 StartOS2Hdr = FindOS2hdr(pdf);         /* find start of OS2 EXE header.     */
 if(StartOS2Hdr == 0L)                  /* if we can't read the header       */
  return(NULL);                         /* then we're done.                  */
                                        /*                                   */
 seekf(pdf,StartOS2Hdr + RNT);          /* seek loc containing RNT offset.   */
 if(readf( buffer,2,pdf))               /* read RNT offset.                  */
  goto error;                           /*                                   */
                                        /*                                   */
 RNToffset = *(ushort *)buffer;         /* offset of RNT from OS/2 header.   */
 RNTloc = StartOS2Hdr + RNToffset;      /* absolute location of RNT.         */
/*****************************************************************************/
/* Now, scan the RNT looking for the matching export. If any read errors     */
/* occur, then brancg to error handling. We're done reading the table        */
/* when the namelength is 0. This is part of the EXE format.                 */
/*                                                                           */
/* Note: underscore handling.                                                */
/* The name in the static import record will not contain an underscore.      */
/* The export record in the RNT will. The export record name will also       */
/* be upper case. We ignore the underscore and do case insensitive compar.   */
/*****************************************************************************/
 rntoff = RNTloc;                       /* init RNT location.                */
 match = FALSE;                         /* assume no match.                  */
 for(;;)                                /* scan the RNT.                     */
 {                                      /*                                   */
  seekf(pdf,rntoff );                   /* seek RNT.                         */
  if(readf( export,1,pdf))              /* read length of export name.       */
   goto error;                          /*                                   */
  explen = adjlen = *(uchar *)export;   /*                                   */
  if(explen == 0)                       /* we're finished reading the tabl   */
   break;                               /* when the length is 0.             */
  if(readf(export+1,explen,pdf))        /* read the name part of the         */
   goto error;                          /* record.                           */
  en = export + 1;                      /* init ptr to export name.          */
  if( *(import + 1) != '_' )            /* if the import name has an '_' no  */
  {                                     /* adjustment is needed. if not make */
   if(*(char *)(en) == '_' )            /* an adjustment because the debug   */
   {                                    /* info may not contain '_'.         */
    en++;                               /*                                   */
    adjlen -= 1;                        /*                                   */
   }                                    /*                                   */
  }                                     /*                                   */
  if( adjlen == *(uchar *)import )      /* if export and import lengths      */
  {                                     /* match then we compare names.      */
   if(readf(buffer,2,pdf))              /* read Entry Table index.           */
    goto error;                         /*                                   */
   etindex = *(ushort *)buffer;         /*                                   */
   if(strnicmp(en,import+1,adjlen)==0)  /* if export matches the import,     */
    {match = TRUE; break;}              /* set the matching flag.            */
  }                                     /*                                   */
  rntoff += explen+3;                   /* update to offset of next entry.   */
 }                                      /*                                   */
/*****************************************************************************/
/* At this point, we have either found a match in the RNT of the exporting   */
/* file or we haven't. If we haven't, then there is no need to continue.     */
/* If we have a match, then we will find the etindex in the ET table and     */
/* build our addr.                                                           */
/*****************************************************************************/
 if( !match )                           /*                                   */
  goto error;                           /*                                   */
/*****************************************************************************/
/* Find the absolute locations in the exporting dll/exe of the ET.           */
/*****************************************************************************/
 seekf(pdf,StartOS2Hdr + ET);           /* seek loc containing ET  offset.   */
 if(readf( buffer,2,pdf))               /* read ET  offset.                  */
  goto error;                           /*                                   */
                                        /*                                   */
 EToffset = *(ushort *)buffer;          /* offset of ET  from OS/2 header.   */
 ETloc = StartOS2Hdr + EToffset;        /* absolute location of ET.          */
                                        /*                                   */
/*****************************************************************************/
/* What we're going to do is find the bundle in the entry table that         */
/* contains the etindex parameter.                                           */
/*****************************************************************************/
 etoff  = ETloc;                        /* init ET location.                 */
 hiindex = 0;                           /* init ET table index.              */
 for(;;)                                /* scan the RNT.                     */
 {                                      /*                                   */
  seekf(pdf,etoff );                    /* seek offset within ET.            */
  if(readf( buffer,1,pdf))              /* read the number of entries in     */
   goto error;                          /* the ET. break on error.           */
  entries = *(uchar *)buffer;           /* # of entries in this bundle.      */
  if(entries == 0 )                     /* table terminates with double      */
   break;                               /* byte of zeros. This should        */
                                        /* get it.                           */
  loindex = hiindex + 1;                /* calc span of indices for this     */
  hiindex = loindex + entries - 1 ;     /* bundle.                           */
                                        /*                                   */
  if( etindex >= loindex &&             /* if our etindex is in this         */
      etindex <= hiindex )              /* bundle, then we can move on.      */
   break;                               /*                                   */
                                        /*                                   */
  if(readf( buffer,1,pdf))              /* if not read the bundle type and   */
   goto error;                          /* break on error.                   */
                                        /*                                   */
  bundletype = *(uchar *)buffer;        /* type of the bundle entry.         */
  switch( bundletype )                  /*                                   */
  {                                     /*                                   */
   case 0:    bunrecsize = 0;break;     /* NULL bundle.                      */
   case 0xFF: bunrecsize = 6;break;     /* Movable Segment records.          */
   default:   bunrecsize = 3;break;     /* Fixed Segment records.            */
  }                                     /*                                   */
                                        /*                                   */
  etoff += bunrecsize*entries + 2;      /* calc offset of next bundle.       */
 }                                      /*                                   */
/*****************************************************************************/
/* At this point, etoff should be pointing at the bundle. We will now        */
/* get the segment # and the offset. The segment # will have to be           */
/* converted to a selector. You should be positioned to read the segment#.   */
/* We will handle Single Fixed segments only.                                */
/*****************************************************************************/
 if(readf( buffer,1,pdf))               /* read the number of entries in     */
  goto error;                           /* the ET. break on error.           */
 segnum = *(uchar *)buffer;             /* calc the segment number.          */
 if( segnum == 0 ||                     /* if it's null or movable, then     */
     segnum == 0xFF )                   /* we can't handle it. it must be    */
                                        /* a Single Fixed segment.           */
  goto error;                           /*                                   */
 etoff += 2 + 3*(etindex-loindex);      /* compute offset within bundle      */
                                        /* of the etindex we want.           */
 seekf(pdf,etoff );                     /* seek offset of etindex record.    */
 if(readf( buffer,3,pdf))               /* read the record.                  */
  goto error;                           /*                                   */
 off = *(ushort *)(buffer + 1);         /* extract offset part of record.    */

 if(pdf->DebugOff == 0L )               /* if no debug info, then            */
  closedos(fh);                         /* close the file.                   */

 return( GetLoadAddr(pdf->mte,segnum) + off );  /* return the address. 822803*/
                                        /*                                   */
/*****************************************************************************/
/*error handling for getexportaddr().                                        */
/*****************************************************************************/
error:                                  /*                                   */
 if(pdf->DebugOff == 0L )               /* if no debug info, then            */
  closedos(fh);                         /* close the file.                   */
 return( NULL );                        /*                                   */
}                                       /* end getexportaddr().              */


/*****************************************************************************/
/* Locations for the various table offsets.                                  */
/*****************************************************************************/
#define  FPT32OFFSETLOC   0x68          /* Fixup Page Table offset.          */
#define  FRT32OFFSETLOC   0x6C          /* Fixup Record Table offset.        */
#define  IMT32OFFSETLOC   0x70          /* Import Module name Table offset.  */
#define  IPT32OFFSETLOC   0x78          /* Import Procedure name Table offset*/

#define  NOOFPAGEOFFSET   0x14          /* No. of pages offset               */
#define  FIXUPSIZEOFFSET  0x30          /* Fixup section size offset.        */

/*****************************************************************************/
/* Source flag bit definitions.                                              */
/*****************************************************************************/
#define  BIT16SELFIX      0x02          /* 16-Bit selector fixup.            */
#define  SRCLISTFLAG      0x20          /* Source List flag.                 */

/*****************************************************************************/
/* Target flag bit definitions.                                              */
/*****************************************************************************/
#define  INTERNALFIX      0             /* Internal reference fixup.         */
#define  IMPORDFLAG       0x01          /* Imported reference by Ordinal.    */
#define  IMPNAMEFLAG      0x02          /* Imported reference by Name.       */
#define  INTENTRYFIXBIT1  0x01          /* Internal reference via Entry Table*/
#define  INTENTRYFIXBIT2  0x02          /* bits 1 and 2 combined (0x3).      */
#define  ADDFIXFLAG       0x04          /* Additive fixup flag.              */
#define  TARGT32FLAG      0x10          /* 32 Bit Target offset flag.        */
#define  ADFIX32FLAG      0x20          /* 32 Bit Additive fixup flag.       */
#define  OBNUM16FLAG      0x40          /* 16 Bit Object number/Mod ord flag.*/
#define  BIT8ORDFLAG      0x80          /* 8 Bit Ordinal flag.               */

/*****************************************************************************/
/* ResolveImport32()                                                      803*/
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Resolve a data reference imported by name from a 32 bit DLL.            */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   import     input - -> to the import name. name=length specified string. */
/*   ImportPdf  input - -> to the "importing" debug file.                    */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   addr       the resolved address of the import.                          */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   ImportPdf is valid and is the correct importing file.                   */
/*   The address returned will always be a flat address.                     */
/*                                                                           */
/* (Read the documentation on the 32 bit EXE for the exact layouts of the    */
/*  various tables).                                                         */
/*                                                                           */
/*****************************************************************************/
uint   ResolveImport32( char *import, DEBFILE *ImportPdf, uchar *ExpModType )
{                                                                       /*815*/
  char     buffer[4];                   /* file read buffer.                 */
  ulong    StartOS2Hdr;                 /* location of the OS/2 EXE header.  */
  int      NoOfPages;                   /* Number of pages for the Module.   */
  int      EndOfFRT;                    /* End of Fixup Record Table.        */
  int      FPTOffset;                   /* Fixup Page Table offset.          */
  int      FRTOffset;                   /* Fixup Record Table offset.        */
  int      FRTTableOffset;              /* Offset within Fixup Record Table. */
  int      FixupSectSize;               /* Fixup section size.               */
  int      IMTOrdNum;                   /* Import Module Table ordinal number*/
  int      IMTOffset;                   /* Import Module Table offset.       */
  int      IMTTableOffset;              /* Offset within Import Module Table.*/
  int      IPTOffset;                   /* Import Procedure Table offset.    */
  int      IPTSize;                     /* Import Procedure Table size.      */
  int      IPTTableOffset;              /* Offset within IPT.                */
  int      CountOrOffset;
  uint     _addrss;
  char     FRTSourceFlag, FRTTargetFlag;/* Target and Source Flags.          */
  char     ModName[129], ProcName[129];
  int      ModNameLen, ProcNameLen, OffsetIncrement;
  int      rc, i;
  HFILE    fh;
  DEBFILE  *ExportPdf;
  uchar    ExeType;

  /***************************************************************************/
  /* The following are the steps involved in resolving the import.           */
  /*                                                                         */
  /* 1. Read the number of pages for the importing module.                   */
  /*                                                                         */
  /* 2. Get the file offset for the Fixup Page Table (FPT) in the importing  */
  /*    file. We can get the start and end of Fixup Record Table (FRT) from  */
  /*    FPT.                                                                 */
  /*                                                                         */
  /* 3. Scan from the top to bottom of FRT. For each entry which has the     */
  /*    Imported by Name bit set in the FRT, we get the offset in the        */
  /*    Import Procedure Name Table (IPT). If the name in the IPT matches    */
  /*    with the one we are looking for, we get the corresponding Import     */
  /*    Module Table ordinal number for the entry from FRT.                  */
  /*                                                                         */
  /* 4. From the Module ordinal number got from step 3, we scan Import       */
  /*    Module Table to get the name of the module which exported the        */
  /*    variable.                                                            */
  /*                                                                         */
  /* 5. We call findexport32 or findexport with the import and the export    */
  /*    pdf of the module got from step 4.                                   */
  /*                                                                         */
  /***************************************************************************/


  /***************************************************************************/
  /* Check the DLL/EXE if it has debug info. If yes, the file will be open.  */
  /* If not, we have to open and close once we are through.                  */
  /***************************************************************************/
  if( ImportPdf->DebugOff == 0L )
  {
    rc = opendos( ImportPdf->DebFilePtr->fn, "rb", &fh );

    if ( rc != NO_ERROR )
      return( NULL );
    ImportPdf->DebFilePtr->fh = fh;
  }

  /***************************************************************************/
  /* Get the start of the OS2 EXE/DLL header. If it fails report the error.  */
  /***************************************************************************/
  StartOS2Hdr = FindOS2hdr( ImportPdf );
  if( StartOS2Hdr == 0L )
   return( NULL );

  /***************************************************************************/
  /* Get the number of pages for the module.                                 */
  /***************************************************************************/
  seekf( ImportPdf, StartOS2Hdr + NOOFPAGEOFFSET );
  if( readf( buffer, 4, ImportPdf ) )
    goto error;

  NoOfPages = *(int *)buffer;

  /***************************************************************************/
  /* Get the offsets for FPT, FRT, IMT and IPT.                              */
  /***************************************************************************/
  seekf( ImportPdf, StartOS2Hdr + FPT32OFFSETLOC );
  if( readf( buffer, 4, ImportPdf ) )
    goto error;
  FPTOffset = *(int *)buffer;

  seekf( ImportPdf, StartOS2Hdr + FIXUPSIZEOFFSET );
  if( readf( buffer, 4, ImportPdf ) )
    goto error;
  FixupSectSize = *(int *)buffer;

  seekf( ImportPdf, StartOS2Hdr + FRT32OFFSETLOC );
  if( readf( buffer, 4, ImportPdf ) )
    goto error;
  FRTOffset = *(int *)buffer;

  seekf( ImportPdf, StartOS2Hdr + IMT32OFFSETLOC );
  if( readf( buffer, 4, ImportPdf ) )
    goto error;
  IMTOffset = *(int *)buffer;

  seekf( ImportPdf, StartOS2Hdr + IPT32OFFSETLOC );
  if( readf( buffer, 4, ImportPdf ) )
    goto error;
  IPTOffset = *(int *)buffer;

  /***************************************************************************/
  /* - Get the size of the Import Procedure Name table.                      */
  /* - Find whether the import name we are looking for is present in the IPT.*/
  /*   If the import name is not present in IPT, then it is not imported by  */
  /*   name. Since we do not support import by ordinal number report that as */
  /*   an error.                                                             */
  /***************************************************************************/
  IPTSize = (FPTOffset + FixupSectSize) - IPTOffset;
  if( !FindImportName( import, ImportPdf, StartOS2Hdr + IPTOffset, IPTSize ) )
    goto error;

  /***************************************************************************/
  /* Get the end of FRT from FPT. FPT contains the offsets for each page in  */
  /* the FRT and has the end of FRT at the end.                              */
  /***************************************************************************/
  seekf( ImportPdf, StartOS2Hdr + FPTOffset + (NoOfPages * 4) );
  if( readf( buffer, 4, ImportPdf ) )
    goto error;
  EndOfFRT = *(int *)buffer;

  /***************************************************************************/
  /* Get the start of FRT. (Offset of the first page).                       */
  /***************************************************************************/
  seekf( ImportPdf, StartOS2Hdr + FPTOffset );
  if( readf( buffer, 4, ImportPdf ) )
    goto error;

  FRTTableOffset = *(int *)buffer;

  /***************************************************************************/
  /* Scan through FRT.                                                       */
  /***************************************************************************/
  for( ;; )
  {
    /*************************************************************************/
    /* If the offset into FRT has come to the end of FRT, stop.              */
    /*************************************************************************/
    if( FRTTableOffset >= EndOfFRT )
      break;

    /*************************************************************************/
    /* Read Source and Target flags for the FRT entry.                       */
    /*************************************************************************/
    seekf( ImportPdf, StartOS2Hdr + FRTOffset + FRTTableOffset );
    if( readf( buffer, 1, ImportPdf ) )
      goto error;
    FRTSourceFlag = *(char *)buffer;

    seekf( ImportPdf, StartOS2Hdr + FRTOffset + FRTTableOffset + 1 );
    if( readf( buffer, 1, ImportPdf ) )
      goto error;
    FRTTargetFlag = *(char *)buffer;

    /*************************************************************************/
    /* If the source list flag is set, we have a list of offsets attached to */
    /* the end, so get the number of offsets.                                */
    /*************************************************************************/
    if( FRTSourceFlag & SRCLISTFLAG )
    {
      seekf( ImportPdf, StartOS2Hdr + FRTOffset + FRTTableOffset + 2 );
      if( readf( buffer, 1, ImportPdf ) )
        goto error;
      CountOrOffset = *(char *)buffer;
    }
    else
    /*************************************************************************/
    /* If there is no list attached the count field will have an offset      */
    /* (2 bytes) for entries which cross page boundaries which currently is  */
    /* not taken care.                                                       */
    /*************************************************************************/
    {
      seekf( ImportPdf, StartOS2Hdr + FRTOffset + FRTTableOffset + 2 );
      if( readf( buffer, 2, ImportPdf ) )
        goto error;
      CountOrOffset = *(short *)buffer;
    }

    /*************************************************************************/
    /* We are interested only on the entries for Imported by Name. So if we  */
    /* come across any other entries we call corresponding functions to      */
    /* compute the offset increment to skip the record.                      */
    /*************************************************************************/
    if( FRTTargetFlag == INTERNALFIX )
    {
      /***********************************************************************/
      /* The record is for Internal Reference fix. Skip and continue.        */
      /***********************************************************************/
      OffsetIncrement = PassbyInternalFix( FRTSourceFlag, FRTTargetFlag,
                                           CountOrOffset );
      FRTTableOffset += OffsetIncrement;
      continue;
    }

    if( FRTTargetFlag & IMPORDFLAG )
    {
      /***********************************************************************/
      /* The record is for Import by Ordinal number. (We dont support import */
      /* by ordinal number currently). Skip and continue.                    */
      /***********************************************************************/
      OffsetIncrement = PassbyImportByOrdFix( FRTSourceFlag, FRTTargetFlag,
                                              CountOrOffset );
      FRTTableOffset += OffsetIncrement;
      continue;
    }

    if( (FRTTargetFlag & INTENTRYFIXBIT1) &&
        (FRTTargetFlag & INTENTRYFIXBIT2) )
    {
      /***********************************************************************/
      /* The record is for Internal fix via entry table. Skip and continue.  */
      /***********************************************************************/
      OffsetIncrement = PassbyInternalEntFix( FRTSourceFlag, FRTTargetFlag,
                                              CountOrOffset );
      FRTTableOffset += OffsetIncrement;
      continue;
    }

    if( FRTTargetFlag & IMPNAMEFLAG )
    {
      /***********************************************************************/
      /* The record is for Import by Name. We gotto process it.              */
      /***********************************************************************/
      int  ProcOffsetLen, CountLen, ModOrdLen, Offset;

      /***********************************************************************/
      /* If the source list flag is set, we have an one byte count or else   */
      /* we have a two byte offset.                                          */
      /***********************************************************************/
      CountLen = 2;
      if( FRTSourceFlag & SRCLISTFLAG )
        CountLen = 1;
      /***********************************************************************/
      /* If the 16 bit object number flag is set, the module ordinal number  */
      /* is two bytes or else it is one byte length.                         */
      /***********************************************************************/
      ModOrdLen = 1;
      if( FRTTargetFlag & OBNUM16FLAG )
        ModOrdLen = 2;
      /***********************************************************************/
      /* If the 32 bit target flag is set, the procedure offset length is of */
      /* 4 bytes length or else it is 2 bytes in length.                     */
      /***********************************************************************/
      ProcOffsetLen = 2;
      if( FRTTargetFlag & TARGT32FLAG )
        ProcOffsetLen = 4;

      /***********************************************************************/
      /* Get the offset into the Import Procedure names Table (IPT) for this */
      /* record.                                                             */
      /***********************************************************************/
      seekf( ImportPdf, StartOS2Hdr + FRTOffset + FRTTableOffset +
                        CountLen + ModOrdLen + 2 );
      if( readf( buffer, ProcOffsetLen, ImportPdf ) )
        goto error;
      if( ProcOffsetLen == 2 )
        IPTTableOffset = *(short *)buffer;
      else
        IPTTableOffset = *(int *)buffer;

      /***********************************************************************/
      /* Get the procedure name length and read the procedure name.          */
      /***********************************************************************/
      seekf( ImportPdf, StartOS2Hdr + IPTOffset + IPTTableOffset );
      if( readf( buffer, 1, ImportPdf ) )
        goto error;
      ProcNameLen = *(char *)buffer;
      if( readf( ProcName, ProcNameLen, ImportPdf ) )
        goto error;
      ProcName[ProcNameLen] = '\0';

      /***********************************************************************/
      /* Compare the procedure name with the import we are looking for.      */
      /***********************************************************************/
      if( !strnicmp( import+1, ProcName, *import ) )
      {
        /*********************************************************************/
        /* If they both match, get the ordinal number into the Import Module */
        /* Table (IMT) for this record.                                      */
        /*********************************************************************/
        seekf( ImportPdf, StartOS2Hdr + FRTOffset + FRTTableOffset +
                          CountLen + 2 );
        if( readf( buffer, ModOrdLen, ImportPdf ) )
          goto error;
        if( ModOrdLen == 1 )
          IMTOrdNum = *(char *)buffer;
        else
          IMTOrdNum = *(short *)buffer;

        IMTTableOffset = 0;
        /*********************************************************************/
        /* As we get only the ordinal number and not the offset into IMT, we */
        /* skip that many entries in the IMT.                                */
        /*********************************************************************/
        Offset = 0;
        for( i = 0; i < IMTOrdNum; i++ )
        {
          seekf( ImportPdf, StartOS2Hdr + IMTOffset + IMTTableOffset );
          if( readf( buffer, 1, ImportPdf ) )
            goto error;
          ModNameLen = *(char *)buffer;
          IMTTableOffset = Offset;
          Offset += (ModNameLen + 1);
        }

        /*********************************************************************/
        /* Get the Module name length and read the module name.              */
        /*********************************************************************/
        seekf( ImportPdf, StartOS2Hdr + IMTOffset + IMTTableOffset );
        if( readf( buffer, 1, ImportPdf ) )
          goto error;
        ModNameLen = *(char *)buffer;
        if( readf( ModName, ModNameLen, ImportPdf ) )
          goto error;
        ModName[ModNameLen] = '\0';

        /*********************************************************************/
        /* Get the PDF for the module which exported the variable.           */
        /*  - call findexport32 if the module which exported the variable is */
        /*    a 32 bit module.                                               */
        /*  - call findexport if the module which exported the variable is a */
        /*    16 bit module.                                                 */
        /*********************************************************************/
        ExportPdf = findpdf( ModName );
        if( ExportPdf )
        {
          ulong OS2Header;
          HFILE fHandle;

          if( ExportPdf->DebugOff == 0L )
          {
            rc = opendos( ExportPdf->DebFilePtr->fn, "rb", &fHandle );

           if ( rc != NO_ERROR )
            return( NULL );
           ExportPdf->DebFilePtr->fh = fHandle;
          }

          OS2Header = FindOS2hdr( ExportPdf );
          if( OS2Header == 0L )
           goto error;

          seekf( ExportPdf, OS2Header );
          readf( buffer, 1, ExportPdf );
          ExeType = BIT16;
          if( buffer[0] == 'L' )
           ExeType = BIT32;

          if( ExportPdf->DebugOff == 0L )
            closedos( fHandle );

          *ExpModType = ExeType;                                        /*815*/
          if( ExeType == BIT32 )
            _addrss = findexport32( ExportPdf, import );
          else
            _addrss = findexport( ExportPdf, import );
        }

        if( _addrss )
        {
          if( ImportPdf->DebugOff == 0L )
            closedos( fh );
          return( _addrss );
        }
      }
    }
    /*************************************************************************/
    /* We have not been able to get the address of the import. So skip to    */
    /* the next record in FRT.                                               */
    /*************************************************************************/
    OffsetIncrement = PassbyImportByNameFix( FRTSourceFlag, FRTTargetFlag,
                                             CountOrOffset );
    FRTTableOffset += OffsetIncrement;
  }

error:
  if( ImportPdf->DebugOff == 0L )
    closedos( fh );
  return( NULL );
}

/*****************************************************************************/
/* PassbyInternalFix()                                                    803*/
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Calculates the increment to skip a internal reference fix record.       */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   SourceFlag    -  Source flag for the record.                            */
/*   TargetFlag    -  Target flag for the record.                            */
/*   CountOrOffset -  It has a count ot an offset depending the source list  */
/*                    bit in the source flag.                                */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   Increment  to skip the current record.                                  */
/*                                                                           */
/*****************************************************************************/
int  PassbyInternalFix( int SourceFlag, int TargetFlag, int CountOrOffset )
{
  int  Increment = 2;

  /***************************************************************************/
  /* The length of the Internal reference fix record is calcuated as follows:*/
  /*                                                                         */
  /*  - 2 bytes for the source and target flags.                             */
  /*  - if the source list bit is set, the count is 1 byte or it is a 2 byte */
  /*    offset.                                                              */
  /*  - if the 16 bit object number bit is set, the object number is 2 bytes */
  /*    or else it is 1 byte.                                                */
  /*  - if the source flag does not specify a 16 bit selector fixup, we have */
  /*    a target offset field. It is of 2 bytes if the 32 bit target offset  */
  /*    flag bit is clear or else it is 4 bytes.                             */
  /*  - if the source list bit is set in the source is set, the record is    */
  /*    followed by a list of source offsets as specified in the count.      */
  /*                                                                         */
  /***************************************************************************/
  if( SourceFlag & SRCLISTFLAG )
    Increment++;
  else
    Increment += 2;

  if( TargetFlag & OBNUM16FLAG )
    Increment += 2;
  else
    Increment++;

  if( (SourceFlag & 0x0000000F) != BIT16SELFIX )
  {
    if( TargetFlag & TARGT32FLAG )
      Increment += 4;
    else
      Increment += 2;
  }

  if( SourceFlag & SRCLISTFLAG )
    Increment += ( CountOrOffset * 2 );

  return( Increment );
}

/*****************************************************************************/
/* PassbyInternalEntFix()                                                 803*/
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Calculates the increment to skip a internal reference via entry table   */
/*   fixup record.                                                           */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   SourceFlag    -  Source flag for the record.                            */
/*   TargetFlag    -  Target flag for the record.                            */
/*   CountOrOffset -  It has a count ot an offset depending the source list  */
/*                    bit in the source flag.                                */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   Increment  to skip the current record.                                  */
/*                                                                           */
/*****************************************************************************/
int  PassbyInternalEntFix( int SourceFlag, int TargetFlag, int CountOrOffset )
{
  int  Increment = 2;

  /***************************************************************************/
  /* The length of the Internal reference via entry table fixup record is    */
  /* calculated as follows:                                                  */
  /*                                                                         */
  /*  - 2 bytes for the source and target flags.                             */
  /*  - if the source list bit is set, the count is 1 byte or it is a 2 byte */
  /*    offset.                                                              */
  /*  - if the 16 bit object number bit is set, the object number is 2 bytes */
  /*    or else it is 1 byte.                                                */
  /*  - if the additive fixup bit in the target flag is set, we will have a  */
  /*    additive field. It is of 2 bytes length if the 32 bit additive flag  */
  /*    bit is clear or else it is 4 bytes.                                  */
  /*  - if the source list bit is set in the source is set, the record is    */
  /*    followed by a list of source offsets as specified in the count.      */
  /*                                                                         */
  /***************************************************************************/
  if( SourceFlag & SRCLISTFLAG )
    Increment++;
  else
    Increment += 2;

  if( TargetFlag & OBNUM16FLAG )
    Increment += 2;
  else
    Increment++;

  if( TargetFlag & ADDFIXFLAG )
  {
    if( TargetFlag & ADFIX32FLAG )
      Increment += 4;
    else
      Increment += 2;
  }

  if( SourceFlag & SRCLISTFLAG )
    Increment += ( CountOrOffset * 2 );

  return( Increment );
}

/*****************************************************************************/
/* PassbyImportByOrdFix()                                                 803*/
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Calculates the increment to skip a import by ordinal fixup record.      */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   SourceFlag    -  Source flag for the record.                            */
/*   TargetFlag    -  Target flag for the record.                            */
/*   CountOrOffset -  It has a count ot an offset depending the source list  */
/*                    bit in the source flag.                                */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   Increment  to skip the current record.                                  */
/*                                                                           */
/*****************************************************************************/
int  PassbyImportByOrdFix( int SourceFlag, int TargetFlag, int CountOrOffset )
{
  int  Increment = 2;

  /***************************************************************************/
  /* The length of the Import by ordinal fixup record is calculated as       */
  /* follows:                                                                */
  /*                                                                         */
  /*  - 2 bytes for the source and target flags.                             */
  /*  - if the source list bit is set, the count is 1 byte or it is a 2 byte */
  /*    offset.                                                              */
  /*  - if the 16 bit object number bit is set, the object number is 2 bytes */
  /*    or else it is 1 byte.                                                */
  /*  - if the 8 bit ordinal bit in the target flag is set, we have a 1 byte */
  /*    import ordinal, or if the 32 bit target offset is set it is 4 bytes, */
  /*    or else it is 2 bytes.                                               */
  /*  - if the additive fixup bit in the target flag is set, we will have a  */
  /*    additive field. It is of 2 bytes length if the 32 bit additive flag  */
  /*    bit is clear or else it is 4 bytes.                                  */
  /*  - if the source list bit is set in the source is set, the record is    */
  /*    followed by a list of source offsets as specified in the count.      */
  /*                                                                         */
  /***************************************************************************/
  if( SourceFlag & SRCLISTFLAG )
    Increment++;
  else
    Increment += 2;

  if( TargetFlag & OBNUM16FLAG )
    Increment += 2;
  else
    Increment++;

  if( TargetFlag & BIT8ORDFLAG )
    Increment ++;
  else
  {
    if( TargetFlag & TARGT32FLAG )
      Increment += 4;
    else
      Increment += 2;
  }

  if( TargetFlag & ADDFIXFLAG )
  {
    if( TargetFlag & ADFIX32FLAG )
      Increment += 4;
    else
      Increment += 2;
  }

  if( SourceFlag & SRCLISTFLAG )
    Increment += ( CountOrOffset * 2 );

  return( Increment );
}

/*****************************************************************************/
/* PassbyImportByNameFix()                                                803*/
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Calculates the increment to skip a import by name fixup record.         */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   SourceFlag    -  Source flag for the record.                            */
/*   TargetFlag    -  Target flag for the record.                            */
/*   CountOrOffset -  It has a count ot an offset depending the source list  */
/*                    bit in the source flag.                                */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   Increment  to skip the current record.                                  */
/*                                                                           */
/*****************************************************************************/
int  PassbyImportByNameFix( int SourceFlag, int TargetFlag, int CountOrOffset )
{
  int  Increment = 2;

  /***************************************************************************/
  /* The length of the Import by name fixup record is calculated as follows: */
  /*                                                                         */
  /*  - 2 bytes for the source and target flags.                             */
  /*  - if the source list bit is set, the count is 1 byte or it is a 2 byte */
  /*    offset.                                                              */
  /*  - if the 16 bit object number bit is set, the object number is 2 bytes */
  /*    or else it is 1 byte.                                                */
  /*  - if the 32 bit target offset flag is set, we have 4 byte procedure    */
  /*    name offset or else it is of 2 bytes.                                */
  /*  - if the additive fixup bit in the target flag is set, we will have a  */
  /*    additive field. It is of 2 bytes length if the 32 bit additive flag  */
  /*    bit is clear or else it is 4 bytes.                                  */
  /*  - if the source list bit is set in the source is set, the record is    */
  /*    followed by a list of source offsets as specified in the count.      */
  /*                                                                         */
  /***************************************************************************/
  if( SourceFlag & SRCLISTFLAG )
    Increment++;
  else
    Increment += 2;

  if( TargetFlag & OBNUM16FLAG )
    Increment += 2;
  else
    Increment++;

  if( TargetFlag & TARGT32FLAG )
    Increment += 4;
  else
    Increment += 2;

  if( TargetFlag & ADDFIXFLAG )
  {
    if( TargetFlag & ADFIX32FLAG )
      Increment += 4;
    else
      Increment += 2;
  }

  if( SourceFlag & SRCLISTFLAG )
    Increment += ( CountOrOffset * 2 );

  return( Increment );
}

/*****************************************************************************/
/* Table offset locations.                                                   */
/*****************************************************************************/
#define  RNT32OFFSETLOC   0x58          /* Resident Name Table offset.       */
#define  ENT32OFFSETLOC   0x5C          /* Entry Table offset.               */

/*****************************************************************************/
/* Bundle types.                                                             */
/*****************************************************************************/
#define UNUSEDENTRY    0x00
#define BIT16ENTRY     0x01
#define CALLGATE286    0x02
#define BIT32ENTRY     0x03
#define FORWARDER      0x04

/*****************************************************************************/
/* findexport32()                                                         803*/
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Look for an export within this 32 bit debug file RNT that matches the   */
/*   import passed as a parameter and calculate the address for the import.  */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   pdf        input - -> to debug file node to search.                     */
/*   import     input - -> to the import name. The import name is a          */
/*                         length specified string.                          */
/* Return:                                                                   */
/*                                                                           */
/*   addr       address of the import.                                       */
/*   NULL       if address not found.                                        */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   import is a z string.                                                   */
/*   The dll has been initialized and attached.                              */
/*   The RNT will contain at least one name - the module or EXE name if      */
/*   nothing else.                                                           */
/*                                                                           */
/*****************************************************************************/
uint findexport32( DEBFILE *pdf, char *import )
{
  HFILE    fh;
  int      rc = 0;
  int      Match;
  char     buffer[4];                   /* file read buffer.                 */
  ulong    StartOS2Hdr;                 /* location of the OS/2 EXE header.  */
  char     ExportName[129];
  int      ExportNameLen;
  int      EntryTableIndex;             /* Index into the Entry Table.       */
  int      RNTOffset;                   /* Offset of Resident Name Table.    */
  int      RNTTableOffset;              /* Offset within RNT.                */
  int      ENTOffset;                   /* Offset of Entry Table.            */
  int      ENTTableOffset;              /* Offset within Entry Table.        */
  int      HighIndex;
  int      LowIndex;
  int      Offset;                      /* offset for the import/export.     */
  int      ObjectNumber;
  short    BundleCount;
  short    BundleType;
  char    *ExportNameBuffer;

  /***************************************************************************/
  /* The following are the steps involved in calculating the address of the  */
  /* exported variable.                                                      */
  /*                                                                         */
  /* 1. Search the Resident Names Table (RNT) to find the record for the     */
  /*    import/export.                                                       */
  /*                                                                         */
  /* 2. The record from step 1 contains an index into the Entry Table (ET)   */
  /*    for the import/export.                                               */
  /*                                                                         */
  /* 3. Search the Entry Table for the bundle that contains the index for    */
  /*    the import/export. The bundle entry contains the object number and   */
  /*    offset of the import/export. We will then call DosDebug to convert   */
  /*    the object number to an address and add the offset to get a flat     */
  /*    address.                                                             */
  /***************************************************************************/


  /***************************************************************************/
  /* Check the DLL if it has debug info. If yes, the file will be open. If   */
  /* not, we have to open and close once we are through.                     */
  /***************************************************************************/
  if( pdf->DebugOff == 0L )
  {
    rc = opendos( pdf->DebFilePtr->fn, "rb", &fh );

    if( rc != NO_ERROR )
      return( NULL );
    pdf->DebFilePtr->fh = fh;
  }

  /***************************************************************************/
  /* Get the start of the OS2 EXE/DLL header. If it fails report the error.  */
  /***************************************************************************/
  StartOS2Hdr = FindOS2hdr( pdf );
  if( StartOS2Hdr == 0L )
    return( NULL );

  /***************************************************************************/
  /* Get the offset for the Resident Names Table (RNT).                      */
  /***************************************************************************/
  seekf( pdf, StartOS2Hdr + RNT32OFFSETLOC );
  if( readf( buffer, 4, pdf ) )
    goto error;

  RNTOffset = *(int *)buffer;
  Match = FALSE;

  RNTTableOffset = 0;
  /***************************************************************************/
  /* Scan through the Resident Names Table.                                  */
  /***************************************************************************/
  for( ;; )
  {
    /*************************************************************************/
    /* Read the export name length and the export name.                      */
    /*************************************************************************/
    seekf( pdf, RNTOffset + StartOS2Hdr + RNTTableOffset );
    if( readf( buffer, 1, pdf ) )
      goto error;
    ExportNameLen = *(char *)buffer;

    if( !ExportNameLen )
      break;

    if( readf( ExportName, ExportNameLen, pdf ) )
      goto error;

    /*************************************************************************/
    /* If the import doesnt have an '_' in the begining, remove the '_' from */
    /* the export name before comparison.                                    */
    /*************************************************************************/
    ExportNameBuffer = &ExportName[0];
    if( *(import + 1) != '_' )
    {
      if( ExportName[0] == '_' )
        ExportNameBuffer = &ExportName[1];
    }

    /*************************************************************************/
    /* Compare the export name with the import name we are interested. If we */
    /* get a match read the Entry Table index for the import.                */
    /*************************************************************************/
    if( !strnicmp( import+1, ExportNameBuffer, *import ) )
    {
      Match = TRUE;
      if( readf( buffer, 2, pdf ) )
        goto error;
      EntryTableIndex = *(short *)buffer;
      break;
    }
    /*************************************************************************/
    /* If no match was found, go to the next entry in RNT.                   */
    /*************************************************************************/
    RNTTableOffset += (ExportNameLen + 3);
  }

  /***************************************************************************/
  /* If we dont get a match in RNT, report it as an error.                   */
  /***************************************************************************/
  if( !Match )
    goto error;

  /***************************************************************************/
  /* Get the Entry Table offset.                                             */
  /***************************************************************************/
  seekf( pdf, StartOS2Hdr + ENT32OFFSETLOC );
  if( readf( buffer, 4, pdf ) )
    goto error;
  ENTOffset = *(int *)buffer;

  ENTTableOffset = 0;
  HighIndex = 0;
  /***************************************************************************/
  /* Scan through the Entry Table bundles for the bundle containing the ET   */
  /* index for the import/export.                                            */
  /***************************************************************************/
  for( ;; )
  {
    /*************************************************************************/
    /* Get the number of entries in the bundle.                              */
    /*************************************************************************/
    seekf( pdf, ENTOffset + StartOS2Hdr + ENTTableOffset );
    if( readf( buffer, 1, pdf ) )
      goto error;
    BundleCount = *(char *)buffer;

    /*************************************************************************/
    /* Get the type of the entries in the bundle. (All entries in a bundle   */
    /* are homogeneous).                                                     */
    /*************************************************************************/
    seekf( pdf, ENTOffset + StartOS2Hdr + ENTTableOffset + 1 );
    if( readf( buffer, 1, pdf ) )
      goto error;
    BundleType = *(char *)buffer;

    /*************************************************************************/
    /* Calculate the lower and higher index for the bundle.                  */
    /*************************************************************************/
    LowIndex  = HighIndex + 1;
    HighIndex = LowIndex + BundleCount - 1;

    /*************************************************************************/
    /* If the Entry Table index we are looking for, falls within the lower   */
    /* and higher index for the bundle, we have found the entry for the      */
    /* import/export. So break out of the loop.                              */
    /*************************************************************************/
    if( EntryTableIndex >= LowIndex &&
        EntryTableIndex <= HighIndex )
       break;

    /*************************************************************************/
    /* If the Entry Table index does not lie in this bundle, skip to the next*/
    /* bundle. Depending on the type of the bundle and the number of bundle  */
    /* entries, calculate the offset of the next bundle.                     */
    /*************************************************************************/
    switch( BundleType & 0x000F )
    {
      case UNUSEDENTRY: ENTTableOffset += (BundleCount * 1) + 1; break;
      case BIT16ENTRY:  ENTTableOffset += (BundleCount * 3) + 4; break;
      case CALLGATE286: ENTTableOffset += (BundleCount * 5) + 4; break;
      case BIT32ENTRY:  ENTTableOffset += (BundleCount * 5) + 4; break;
      case FORWARDER:   ENTTableOffset += (BundleCount * 7) + 4; break;
    }
  }

  /***************************************************************************/
  /* Get the object number of the import/export.                             */
  /***************************************************************************/
  if( readf( buffer, 2, pdf ) )
    goto error;
  ObjectNumber = *(short *)buffer;

  /***************************************************************************/
  /* If the bundle type for the entry is 16 bit, we will have 2 byte offset  */
  /* for the import/export. Read the offset which we will add to the address */
  /* got for the object.                                                     */
  /***************************************************************************/
  if( (BundleType & 0x000F) == 0x01 )
  {
    int BundleOffset = 0;

    BundleOffset = (EntryTableIndex - LowIndex) * 3 + 4;
    seekf( pdf, ENTOffset + StartOS2Hdr + ENTTableOffset + BundleOffset );
    if( readf( buffer, 3, pdf ) )
      goto error;
    Offset = *(short *)(buffer + 1);
  }
  /***************************************************************************/
  /* If the bundle type for the entry is not 16 bit, we will have 2 byte     */
  /* offset for the export. Read the offset which we will add to the address */
  /* got for the object.                                                     */
  /***************************************************************************/
  else
  {
    int BundleOffset = 0;

    BundleOffset = (EntryTableIndex - LowIndex) * 5 + 4;
    seekf( pdf, ENTOffset + StartOS2Hdr + ENTTableOffset + BundleOffset );
    if( readf( buffer, 5, pdf ) )
      goto error;
    Offset = *(int *)(buffer + 1);
  }

  if( pdf->DebugOff == 0L )
    closedos( fh );

  /***************************************************************************/
  /* Add the offset with the address got from DosDebug.                      */
  /***************************************************************************/
  return( GetLoadAddr(pdf->mte,ObjectNumber) + Offset );/* retn the addr  822*/

error:

  if( pdf->DebugOff == 0L )
    closedos( fh );
  return( NULL );
}

/*****************************************************************************/
/* FindImportName()                                                          */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Checks whether the given import name is present in the Import Procedure */
/*   Name table.                                                             */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   Import        -  Name of the import to be searched.                     */
/*                    (one byte length prefixed string).                     */
/*   ImportPdf     -  -> to the "importing" debug file.                      */
/*   IPTOffset     -  Start offset for Import Procedure Name Table.          */
/*   IPTSize       -  Size of the Import Procedure Name table.               */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   0 - Import name not found in IPT.                                       */
/*   1 - Import name found in IPT.                                           */
/*                                                                           */
/*****************************************************************************/
int  FindImportName( char *Import, DEBFILE *ImportPdf, ulong IPTOffset,
                     int IPTSize )
{
  uchar *IPT, *IPTEnd, *IPTPtr;

  /***************************************************************************/
  /* - Allocate memory for IPT.                                              */
  /* - Read IPT into the memory.                                             */
  /* - Search for the import name in IPT.                                    */
  /* - Return result of the search after freeing IPT.                        */
  /***************************************************************************/
  IPT = Talloc( IPTSize );

  seekf( ImportPdf, IPTOffset );
  if( readf( IPT, IPTSize, ImportPdf ) )
    return( 0 );

  IPTEnd = IPT + IPTSize;
  /***************************************************************************/
  /* The first byte in IPT was always "0x00" and so skip the first byte.     */
  /***************************************************************************/
  IPTPtr = IPT + 1;

  while( IPTPtr < IPTEnd )
  {
    char NameLen;

    NameLen = *IPTPtr;
    if( NameLen == *Import )
      if( !strnicmp( Import + 1, IPTPtr + 1, NameLen ) )
      {
        Tfree( IPT );
        return( 1 );
      }
    IPTPtr += NameLen;
  }
  Tfree( IPT );
  return( 0 );
}
