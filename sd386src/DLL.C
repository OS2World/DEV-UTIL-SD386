/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   dll.c                                                                   */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*  DLL handling functions.                                                  */
/*                                                                           */
/*                                                                           */
/* History:                                                                  */
/*                                                                           */
/*   02/08/91 Creation of 32-bit SD86, from 16-bit version.                  */
/*                                                                           */
/*...16->32 port.                                                            */
/*...                                                                        */
/*... 05/14/91  105   Christina port to 32 bit.                              */
/*... 02/08/91  107   Dave      port to 32 bit.                              */
/*... 02/08/91  110   Srinivas  port to 32 bit.                              */
/*... 02/08/91  116   Joe       port to 32 bit.                              */
/*...                                                                        */
/*...Release 1.00 (Pre-release 105 10/10/91)                                 */
/*...                                                                        */
/*... 10/23/91  306   Srinivas  Handling multiple dll loads and dll frees.   */
/*...                                                                        */
/*...Release 1.08 (Pre-release 108 )       )                                 */
/*...                                                                        */
/*... 02/04/92  510   Srinivas  Not able to step into func at source level.  */
/*...                           Too many modules in exe                      */
/*... 02/12/92  521   Srinivas  Port to C-Set/2.                             */
/*... 01/26/93  809   Selwyn    HLL Level 2 support.                         */
/*...                                                                        */
/*...Release 1.04 (04/30/93)                                                 */
/*...                                                                        */
/*... 05/04/93  822   Joe       Add mte table handling.                      */
/*...                                                                        */
/**Includes*******************************************************************/

#include "all.h"                        /* SD86 include files                */

/**External definitions*******************************************************/

extern PROCESS_NODE *pnode;             /* -> to process list.               */
extern char         *msg[];             /* message array.                    */
extern CmdParms      cmd;               /* pointer to CmdParms structure     */

/*****************************************************************************/
/* dllinit()                                                                 */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   initialize a DLL.                                                       */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   mte          - mte handle of the DLL.                                   */
/*   pModuleName  - -> to dll name.                                          */
/*   pObjectTable - -> to the object table for this dll.                     */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   pnode -> to at least one node.                                          */
/*   pModuleName -> to a fully qualified dll name.                        822*/
/*                                                                           */
/*****************************************************************************/
APIRET dllinit( HMODULE mte , char *pModuleName, OBJTABLEENTRY *pObjectTable )
{                                       /*                                   */
 DEBFILE      *pdf;                     /* -> to debug file structure        */
 HFILE         FileHandle;              /* ddl file FILE structure        105*/
 MODULE       *mptr;                    /* -> to a module node               */
 uint          rc=0;                    /* return code                       */
 uint          midbias;                 /* bias for the module id            */
 DEBFILE      *pdfx;                    /* -> to debug file structure        */
 char          ImageExe[CCHMAXPATH];                                    /*827*/
 char         *pFileName;                                               /*827*/

/*****************************************************************************/
/* Scan to see of the dll is already loaded. If so, then return.             */
/*****************************************************************************/
 pdfx = pnode->ExeStruct->next;
 for(; pdfx ; pdfx=pdfx->next)
  if ( pdfx->mte == mte )
     return (0);

 if( IsEspRemote() )
 {
  /***************************************************************************/
  /* If debugging remote, then find the image of the load dll. If we can't   */
  /* find an image, then use the name that we got from the target machine.   */
  /* For system dlls, this should be no problem. If, however, we can't       */
  /* find an image that the user DOES intend to debug, then he will default  */
  /* to assembler level debugging.                                           */
  /***************************************************************************/
  pFileName = strrchr(pModuleName, '\\') + 1;
  memset(ImageExe,0,sizeof(ImageExe) );
  rc = XSrvFindExe(pFileName,ImageExe,sizeof(ImageExe));
  if( rc == 0)
   pModuleName = ImageExe;
 }

/*****************************************************************************/
/*                                                                           */
/* We have to find a range to assign the the mids to.                        */
/* To find a mid range, we scan the current EXE/DLL list and get             */
/* the next range of available values for this DLL. The first node that      */
/* we encounter that has source info, a MidAnchor != null, will be the       */
/* last DLL initialized and will contain the highest range used so far.      */
/* The midbias will be used later after debfileinit() to adjust the          */
/* base range, 1 to n, defined by debfileinit().                             */
/*                                                                           */
/* NOTE:                                                                     */
/* We could have assigned numbers continuously from one DLL to the next      */
/* but primarily for convenience, we chose to associate a range with         */
/* each DLL. This should not be restrictive since a mid range of 1000        */
/* accomodates an application with 1000 linked modules.                      */
/*                                                                           */
/*****************************************************************************/
 midbias = 0;                           /*                                   */
 pdfx = pnode->ExeStruct->next;         /* scan the ring of pdf nodes.       */
 for(; pdfx ; pdfx=pdfx->next)          /*                                   */
 {                                      /*                                   */
  if(pdfx->MidAnchor)                   /* stop at first node with debug     */
  {                                     /* info. This will have the last     */
   midbias = pdfx->MidAnchor->mid - 1;  /* range assigned.                   */
   break;                               /*                                   */
  }                                     /*                                   */
 }                                      /*                                   */
 midbias += 1000;                       /* define the mid adjustment.     510*/
 if(midbias == 131000)                  /*                                510*/
  return(ERR_DLL_TOO_MANY);             /*                                   */
/*****************************************************************************/
/* - allocate space for the DEBFILE structure.                            822*/
/* - allocate space for the DEBFILE part of the MFILE structure.          822*/
/* - allocate space for module name and copy it out of the module load    822*/
/*   table. (The module load table will eventually be freed.)             822*/
/*****************************************************************************/
 pdf=(DEBFILE *)Talloc(sizeof(DEBFILE));                                /*822*/
 pdf->DebFilePtr=(MFILE*)Talloc(sizeof(MFILE));                         /*822*/
 {                                                                      /*822*/
  char *ptemp;                                                          /*822*/
  int   len;                                                            /*822*/
                                                                        /*822*/
  len = strlen( pModuleName );                                          /*822*/
  ptemp = Talloc( len + 1 );                                            /*822*/
                                                                        /*822*/
  strcpy( ptemp , pModuleName );                                        /*822*/
  pModuleName = ptemp;                                                  /*822*/
 }                                                                      /*822*/

/*****************************************************************************/
/* - Open the dll for read access.                                           */
/*****************************************************************************/
 FileHandle = 0;
 rc = opendos(pModuleName,"rb",&FileHandle);
 if( rc != 0 )
 {
  FileHandle = 0;
 }
/*****************************************************************************/
/* Now, we can start building the node.                                      */
/*****************************************************************************/
 pdf->DebFilePtr->fn=pModuleName;       /* add the name to the structure  822*/
 pdf->DebFilePtr->fh=FileHandle;        /* store the debug file handle    105*/
 pdf->mte=mte;                          /* add the mte handle                */
 pdf->SrcOrAsm=1;                       /* assume asm    level debug         */
 pdf->MidAnchor = NULL;                 /* initialize the module chain anchor*/
 pdf->pid = pnode->pid;                 /* we will need the pid.             */
 pdf->CodeObjs = (UINT*)pObjectTable;                                   /*822*/

/*****************************************************************************/
/* Now, we will get the debug format and offset of the debug info in the     */
/* file. We will handle the return from FindDebugStart() as follows:         */
/*                                                                           */
/*                                                                           */
/* rc  |  DebugOff    | Action.                                              */
/* ----|--------------|-------------------------------------                 */
/* 0   |   0          | Normal asm level. File not developed with /CO.       */
/* 0   |   long value | Source level. File developed with /CO.               */
/* 1   |   0          | Error in debug info. Default to asm level.           */
/*                                                                           */
/* If we have source level info but have problems initializing in            */
/* debfileinit(), then we will drop back to the asm level.                   */
/*                                                                           */
/*****************************************************************************/
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
   return(ERR_BAD_DEBUG_INFO);
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
 /* - adjust mid range.                                                      */
 /****************************************************************************/
 for(
     mptr = pdf->MidAnchor;
     mptr != NULL;
     mptr = mptr->NextMod
    )
  mptr->mid += midbias;

 /****************************************************************************/
 /* - get the entry point if dll has debug info.                             */
 /*   This if for the /i option in case the user wants to debug an           */
 /*   InitTerm routine.                                                      */
 /****************************************************************************/
 if( pdf->DebugOff != 0 )
  pdf->EntryExitPt = xGetExeOrDllEntryOrExitPt( pdf->mte );

 /****************************************************************************/
 /* - add the node to the front of the chain.                                */
 /****************************************************************************/
 pdf->next=pnode->ExeStruct->next;
 pnode->ExeStruct->next = pdf;

/*****************************************************************************/
/* At this point the file is initialized for assembler level or source       */
/* level debug.                                                              */
/*****************************************************************************/
 rc = ConvertDefBrks(pnode,pdf , TRUE);
 if( pdf->SrcOrAsm == 1 && FileHandle != 0)
 {
  closedos(FileHandle);
  pdf->DebFilePtr->fh=0;
 }

 return(rc);
}
