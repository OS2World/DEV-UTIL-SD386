/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   exe.c                                                                   */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*  EXE handling functions.                                                  */
/*                                                                           */
/*                                                                           */
/* History:                                                                  */
/*                                                                           */
/*   02/08/91 Creation of 32-bit SD86, from 16-bit version.                  */
/*                                                                           */
/*...16->32 port.                                                            */
/*...                                                                        */
/*... 02/08/91  101   Joe       port to 32 bit.                              */
/*... 02/08/91  107   Dave      port to 32 bit.                              */
/*... 02/08/91  112   Joe       port to 32 bit.                              */
/*... 02/08/91  105   Christina port to 32 bit.                              */
/*                                                                           */
/*...Release 1.00 (Pre-release 108 12/05/91)                                 */
/*...                                                                        */
/*... 02/21/92  521   Srinivas  Port to C-Set/2.                             */
/*...                                                                        */
/*...Release 1.00 (03/03/92)                                                 */
/*...                                                                        */
/*... 03/26/92  609   Srinivas  Handle /Gp- calling convention for C-Set/2.  */
/*...           609             (removed by joe)                             */
/*... 01/26/93  809   Selwyn    HLL Level 2 support.                         */
/*...                                                                        */
/**Includes ******************************************************************/
                                        /*                                   */
#include "all.h"                        /* SD86 include files                */
                                        /*                                   */
/**Defines *******************************************************************/
                                        /*                                   */
/**External declararions******************************************************/
                                        /*                                   */
extern PROCESS_NODE *pnode;             /* -> to process list.            827*/
extern CmdParms      cmd;               /* pointer to CmdParms structure     */
                                        /*                                   */
/**External definitions*******************************************************/
                                        /*                                   */
/**Static definitions ********************************************************/
                                        /*                                   */
/*****************************************************************************/
/* exeinit()                                                                 */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   initialize an EXE file and allocate space for a fake mid.               */
/*                                                                           */
/* Parameters:                                                               */
/*   pdf        pointer to debug file structure.                             */
/*                                                                           */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   rc                                                                      */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   pModuleName != NULL &&                                                  */
/*   pModuleName -> fully qualified file name of the EXE.                    */
/*                                                                           */
/*****************************************************************************/
ULONG exeinit( HMODULE mte,char *pModuleName,OBJTABLEENTRY *pObjectTable )
{                                       /*                                   */
 int           rc;                      /* return code                       */
 MODULE       *mptr;                    /* -> to a module node               */
 MODULE       *lptr;                    /* last pointer to a module          */
 HFILE         FileHandle ;             /* -> to file structure           105*/
 EXEFILE      *pdf;                     /* -> to debug file.                 */
 char          ImageExe[CCHMAXPATH];
 MFILE        *mfile;                   /* -> to MFILE file structure.       */
 char         *fn;

 /****************************************************************************/
 /* If debugging remote, then find the image of the load exe.                */
 /****************************************************************************/
 if( IsEspRemote() )
 {
  char *pFileName;
  /***************************************************************************/
  /* If debugging remote, then find the image of the load exe. If we can't   */
  /* find an image, then use the name that we got from the target machine    */
  /* and default to assembler level debugging.                               */
  /***************************************************************************/
  pFileName = strrchr(pModuleName, '\\') + 1;
  memset(ImageExe,0,sizeof(ImageExe) );
  rc = XSrvFindExe(pFileName,ImageExe,sizeof(ImageExe));
  if( rc == 0)
   pModuleName = ImageExe;
 }
 fn = Talloc(strlen(pModuleName)+1);
 strcpy(fn,pModuleName);

 /****************************************************************************/
 /* - At this point, fn points to the image of the exe. If not               */
 /*   debugging remote, it's the same as the load exe.                       */
 /****************************************************************************/
 rc = opendos(fn,"rb",&FileHandle);
 if( rc != 0 )
  FileHandle = 0;

 /****************************************************************************/
 /* Allocate and build the pdf structure for the EXE.                        */
 /****************************************************************************/
 pdf                 = (EXEFILE *)Talloc(sizeof(EXEFILE));
 mfile               = (MFILE*)Talloc(sizeof(MFILE));
 pdf->DebFilePtr     = mfile;
 pdf->DebFilePtr->fh = FileHandle;
 pdf->DebFilePtr->fn = fn;
 pdf->pid            = pnode->pid;
 pdf->mte            = mte;
 pdf->CodeObjs       = (UINT*)pObjectTable;

 pnode->ExeStruct = pdf;

 /****************************************************************************/
 /* - On ReStart(), convert deferred breakpoints.                            */
 /****************************************************************************/
 ConvertDefBrks(pnode,pdf , FALSE );

/*****************************************************************************/
/* - Get the entry point for this exe.                                       */
/* - We always want an EXE entry point in case we init for asm level debug.  */
/*****************************************************************************/
 pdf->EntryExitPt = xGetExeOrDllEntryOrExitPt( pdf->mte );              /*827*/
 if(pdf->EntryExitPt == 0 )
  return(ERR_EXE_INIT);

/*****************************************************************************/
/* - Get the start of the debug info.                                        */
/* - Come up in assembler if no debug info.                                  */
/*****************************************************************************/
 FileHandle = pdf->DebFilePtr->fh;
 pdf->MidAnchor = NULL;
 pdf->DebugOff = 0;
 if( FileHandle != 0 )
 {
  int NBxxType;
  int ExeType;

  rc = FindDebugStart(FileHandle,
                      &pdf->DebugOff,
                      &NBxxType,
                      &ExeType);

  pdf->ExeFlags.NBxxType = NBxxType;
  pdf->ExeFlags.ExeType  = ExeType;

  if( rc == 1 )
   return(ERR_EXE_INIT);
 }

 if( pdf->DebugOff != 0 )
 {
  /***************************************************************************/
  /* - source level debug.                                                   */
  /***************************************************************************/
  pdf->SrcOrAsm = 0;
  rc=debfileinit(pdf);
  if(rc)
   return(ERR_EXE_INIT);
 }
 else
 {
  /***************************************************************************/
  /* - debug at asm level.                                                   */
  /***************************************************************************/
  pdf->SrcOrAsm = 1;
  asminit(pdf);
 }

 /****************************************************************************/
 /* If we are debugging at asm level and the file was opened, then           */
 /* close it.                                                                */
 /****************************************************************************/
 if( pdf->SrcOrAsm == 1 && FileHandle != 0)
  closedos(FileHandle);

/*****************************************************************************/
/* We are now going to append a fake mid to the end of the module chain.     */
/* ( this code was moved to assure at least one fake node for asm level )    */
/*****************************************************************************/
 for(lptr = (MODULE *)&pdf->MidAnchor,  /* scan to end of mid list           */
     mptr = pdf->MidAnchor;             /*                                   */
     mptr != NULL;                      /*                                   */
     lptr = mptr,                       /* hold this module pointer          */
     mptr = mptr->NextMod               /*                                   */
    ){;}                                /*                                   */
 mptr=(MODULE *)Talloc(sizeof(MODULE)); /* allocate space for fake mid    521*/
 mptr->NextMod = NULL;                  /* last node in the list so gnd it   */
 lptr->NextMod = mptr;                  /* add it to the list                */
 mptr->mid  = FAKEMID;                  /* assume this unique mid            */
 return(0);                             /* assembly level is ok              */
}                                       /* end of exeinit()                  */
