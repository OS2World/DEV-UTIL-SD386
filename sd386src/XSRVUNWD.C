/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   xsrvunwd.c                                                              */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Start the debuggee.                                                     */
/*                                                                           */
/* History:                                                                  */
/*                                                                           */
/*   07/01/93 Created.                                                       */
/*                                                                           */
/*...                                                                        */
/*... 06/04/93  827   Joe       Add remote debug support.                    */
/*...                                                                        */
/*****************************************************************************/
#include "all.h"

#define ROLLBACK        300
#define NEARRET         1
#define FARRET          0
#define ATR32           0xD0            /* stack frame is 32-bit          107*/
#define ATR16           0x00            /* stack frame 1s 16-bit          107*/
#define NUM_FRAME_BYTES 12              /* bumped to 12.                  706*/
/*****************************************************************************/
/* SetActFrames()                                                            */
/*                                                                           */
/* Description:                                                              */
/*  1. Establishes the following arrays for the current state of the user's  */
/*     stack:                                                                */
/*                                                                           */
/*      ActFaddrs[]   the CS:IP or EIP values for the stack frames        107*/
/*      ActCSAtrs[]   0 => 16-bit stack frame  1 => 32-bit stack frame    107*/
/*      ActFlines[][] the mid/lno values for the CS:IP/EIP return values  107*/
/*      ActScopes[]   pointers to SSProc records for the frame CS:IP/EIP vals*/
/*      ActFrames[]   offsets in the stack of the stack frames ( EBP values )*/
/*                                                                           */
/* Parameters:                                                               */
/*   none                                                                    */
/*                                                                           */
/* Return:                                                                   */
/*   none                                                                    */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*    This routine has to guarantee the above arrays are valid. "Valid"      */
/*    implies, for example, that ActFaddrs contains valid addresses          */
/*    in the application code.                                               */
/*                                                                           */
/*****************************************************************************/
ULONG XSrvGetCallStack(STACK_PARMS *pparms,
                       UCHAR       **ppActCSAtrs,
                       UINT        **ppActFrames,
                       UINT        **ppActFaddrs )

{                                       /*                                   */
 int      n;                            /* stack frame index                 */
 uint     limit;                        /* top of stack not including Exec...*/
 uint     frame;                        /* stack frame offset                */
 ushort   SS;                           /* SS selector value                 */
 ushort   CS;                           /* CS part of CS:IP on stack      107*/
 ushort   IP;                           /* IP part of CS:IP on stack      107*/
 uint     EIP;                          /* EIP on stack                   107*/
 uint    *fptr;                         /* pointer to stack frame            */
 uint     lframe[NUM_FRAME_BYTES/2];    /* local copy of stack frame      107*/
 uint     bytesread;                    /* bytes read from user addr space521*/
 int      ret;                          /* Far or Near return flag.          */
 ushort   ReturnCS;                     /* stack frame return CS          107*/
 ULONG    FlatCSIP;                     /* frame CS:IP as a flat addr     107*/
 uchar    SSAtr;                        /* current frame's 16/32-bit stat 107*/
 ushort   BP;                           /* BP part of stack frame         107*/
 uint     PrevFrame;                    /* previous frame location.       706*/

 USHORT   AppPTB_CS;
 USHORT   AppPTB_SS;
 ULONG    AppPTB_EBP;
 ULONG    AppPTB_ESP;
 UCHAR    AppPTB_SSAtr;
 int      ShowAllFlag;
 int      ShowNamedFlag = FALSE;
 UINT     NActFrames;
 UCHAR    ActCSAtrs[MAXAFRAMES];        /* 0=>16-bit frame 1=>32-bit         */
 UINT     ActFrames[MAXAFRAMES];        /* value of frame pointer (bp)       */
 UINT     ActFaddrs[MAXAFRAMES];        /* value of return "EIP" on stack    */
 UCHAR   *pActCSAtrs = NULL;
 UINT    *pActFrames = NULL;
 UINT    *pActFaddrs = NULL;
 BOOL     Initial_EBP_Ok;

 /****************************************************************************/
 /* - pull off the unwind values.                                            */
 /****************************************************************************/
 AppPTB_CS    = pparms->CS;
 AppPTB_SS    = pparms->SS;
 AppPTB_EBP   = pparms->EBP;
 AppPTB_ESP   = pparms->ESP;
 AppPTB_SSAtr = pparms->SSAtr;
 ShowAllFlag  = pparms->ShowAllFlag;

 /****************************************************************************/
 /* - Sometimes we might stop within optimized code or assembler code        */
 /*   where the EBP may have been used for a general purpose register.       */
 /*   When this happens, the call stack unwinding can't get started.  So,    */
 /*   we run a verification check on the initial EBP and if it fails, we     */
 /*   try to scan the stack looking for a valid link in the EBP chain.       */
 /****************************************************************************/
 Initial_EBP_Ok = FALSE;
 if( IsValid_EBP(AppPTB_EBP) == TRUE )
  Initial_EBP_Ok = TRUE;

 if( (AppPTB_SSAtr == ATR32) && (Initial_EBP_Ok == FALSE) )
 /****************************************************************************/
 /*                                                                          */
 /* - If the initial EBP is invalid, then start scanning the stack looking   */
 /*   for a valid link in the EBP chain defined like so:                     */
 /*                                                                          */
 /*                ----------------         ----------------                 */
 /*  Initial EBP->| Next EBP       |------>| xxxxxxxxxxxxxxx|-->             */
 /*                ----------------         ----------------                 */
 /*               | 32 bit eip ret |       | 32 bit eip ret |                */
 /*                ----------------         ----------------                 */
 /*               | 32 bit eip ret |       | 32 bit eip ret |                */
 /*                ----------------         ----------------                 */
 /*                                                                          */
 /* - Note that a 32 bit eip return in either of the stack addresses above   */
 /*   EBP is considered valid.                                               */
 /*                                                                          */
 /****************************************************************************/
 {
  #define BIT32_STACK_WIDTH    4
  #define MAX_STACK_ITERATIONS 100

  BOOL   Next_EBP_Ok;

  ULONG  StackAddress;
  ULONG  NextStackAddress;
  int    i;
  UCHAR *pNext_EBP_Buffer;

  Initial_EBP_Ok = FALSE;
  Next_EBP_Ok    = FALSE;
  StackAddress   = AppPTB_ESP;

  for( i = 1; i <= MAX_STACK_ITERATIONS ; i++ )
  {
   /**************************************************************************/
   /* - verify the first stack frame.                                        */
   /**************************************************************************/
   if( IsValid_EBP( StackAddress ) == TRUE )
    Initial_EBP_Ok = TRUE;

   /**************************************************************************/
   /* - verify the next stack frame.                                         */
   /**************************************************************************/
   Next_EBP_Ok = FALSE;
   if( Initial_EBP_Ok == TRUE )
   {
    pNext_EBP_Buffer = Getnbytes( StackAddress, BIT32_STACK_WIDTH, &bytesread);
    NextStackAddress = *(ULONG *)pNext_EBP_Buffer;

    Next_EBP_Ok      = FALSE;

    if( IsValid_EBP( NextStackAddress ) == TRUE )
    {
     Next_EBP_Ok = TRUE;
     break;
    }
   }

   StackAddress += BIT32_STACK_WIDTH;
  }
  /***************************************************************************/
  /* - if we were successful, then update the EBP with its starting value.   */
  /***************************************************************************/
  if( Next_EBP_Ok == TRUE )
   AppPTB_EBP = StackAddress;
 }

 /****************************************************************************/
 /* - Now, we're ready to start the unwind algorithm.                        */
 /* - AppPTB_EBP will be the value from the DosDebug registers, or           */
 /*   it will be the seed value determined from the above seeding algorithm  */
 /*   for a 32 bit stack.                                                    */
 /****************************************************************************/
 ReturnCS=AppPTB_CS;                    /* initialize stack return CS     107*/
 BP=(ushort)AppPTB_EBP;                 /* saves a union!                 107*/
 if( (AppPTB_EBP && AppPTB_SSAtr) ||    /* if there is a stack frame ...  107*/
     (BP         && !AppPTB_SSAtr))
  {
   limit = frame = AppPTB_EBP;          /* assume 32-bit current stack    107*/
   if ( !AppPTB_SSAtr )                 /* 16-bit current stack ?         107*/
    limit = frame = (uint)Sys_SelOff2Flat(AppPTB_SS,BP);

   SSAtr = AppPTB_SSAtr;                /* init stack frame 16/32-bit stat107*/
                                        /* begin stack frame scan            */
   for( n=0 ;                           /* initialeze the frame index        */
        n < MAXAFRAMES ;                /* proceed to oldest frame or max    */
        ++n                             /* bump frame index                  */
      )                                 /*                                   */
    {                                   /* current ss:bp+2->0th cs:ip        */
                                        /*         (SS:EBP+4->0th EIP)    107*/
     /************************************************************************/
     /*                                                                      */
     /*  1. Get the return address from the stack. It may be a near          */
     /*     return( IP ) or a far return( CS:IP ) on the 16-bit stack frame. */
     /*     It may be a flat address return (EIP) on the 32-bit stack frame. */
     /*     It may be a thunk return!                                        */
     /*                                                                      */
     /************************************************************************/
     fptr = (uint *)Getnbytes(frame,    /* get BP/IP/CS, BP/IP, | EBP/EIP 827*/
                              NUM_FRAME_BYTES,
                              &bytesread);
     if ( bytesread != NUM_FRAME_BYTES )/* did we get something back ?    521*/
      break;                            /* if not then forget it          107*/
     memcpy( lframe, fptr, NUM_FRAME_BYTES ); /* make local copy of frame 107*/
     ActCSAtrs[n]=SSAtr;                /* assume new frame same as old   107*/
     if ( SSAtr )                       /* curr frame 32-bit ?            107*/
     {                                                                  /*706*/
      #define MAXITER 2                                                 /*706*/
      int iter;                                                         /*706*/
      int foundit = FALSE;                                              /*706*/
                                                                        /*706*/
      /*******************************************************************706*/
      /* - When C-set2 makes a 32-16 call, then the 32 bit function       706*/
      /*   containing the call has a call in the prolog to check if       706*/
      /*   there is enough stack space to execute the 16 bit function.    706*/
      /*   If there isn't, then the stack gets moved.                     706*/
      /*******************************************************************706*/
      for( iter=1;!foundit && iter <= MAXITER; iter++)                  /*706*/
      {                                                                 /*706*/
                                                                        /*706*/
       EIP = *(lframe+iter);             /* define EIP                  /*706*/
       if ( !IsNearReturn( EIP, (ushort)1 ) )                           /*827*/
        continue;                                                       /*706*/
       foundit = TRUE;                                                  /*706*/
      }                                                                 /*706*/
                                                                        /*706*/
      if( foundit == FALSE )                                            /*706*/
      {                                                                 /*706*/
       int StackSwitch = TRUE;                                          /*706*/
/*------------------------------------------------------------------------706*/
       if( (ShowAllFlag == TRUE) || (ShowNamedFlag == TRUE) )           /*706*/
       {                                                                /*706*/
        /*********************************************************************/
        /*                                                                706*/
        /* - Before we switch to a 16 bit stack let's first check to      706*/
        /*   see if the chain continues but the eip is just bad. This can 706*/
        /*   happen in the case of an exception handler. A typical example706*/
        /*   might look like so:                                          706*/
        /*                                                                706*/
        /* 0x22AC4->0X22AF4->0X22B4C->0X22C64->0X22DDC->0X22E1C->0X22E38->0  */
        /*          1A02C5EA 1A02C28C 1A02C0FA 0000000  010062   104B2    706*/
        /*                                     |                          706*/
        /*                                      - bad eip                 706*/
        /*                                                                706*/
        /*****************************************************************706*/
        {                                                               /*706*/
         uint  nframe[2];                                               /*706*/
         uint *p;                                                       /*706*/
                                                                        /*706*/
         p = (uint *)Getnbytes(*lframe,8,&bytesread );                  /*827*/
         if( bytesread < 8 )                                            /*706*/
          goto skipit;                                                  /*706*/
         memcpy(nframe,p,8);                                            /*706*/
         EIP = *(nframe + 1);                                           /*706*/
         if ( IsNearReturn( EIP, (ushort)1 ) )                          /*827*/
         {                                                              /*706*/
          foundit = TRUE;                                               /*706*/
          StackSwitch = FALSE;                                          /*706*/
          memcpy(lframe,nframe,8);                                      /*706*/
         }                                                              /*706*/
        }                                                               /*706*/
                                                                        /*706*/
skipit:                                                                 /*706*/
        if( foundit == FALSE )                                          /*706*/
        {                                                               /*706*/
         /********************************************************************/
         /* Well, let's try one final brute force technique.              706*/
         /*                                                               706*/
         /********************************************************************/
         {                                                              /*706*/
          #define STACK_WORDS 50                                        /*706*/
          ushort *ptr;                                                  /*706*/
          ushort  lclstack[STACK_WORDS];                                /*706*/
          int     nbytes;                                               /*706*/
          ushort *lclframe;                                             /*706*/
                                                                        /*706*/
          /*******************************************************************/
          /* now get a chunk of the stack and make it local.              706*/
          /*******************************************************************/
/*827*/   ptr =(ushort *)Getnbytes(frame,STACK_WORDS*sizeof(short),&bytesread);
          if ( bytesread < 6 )                                          /*706*/
           break;    /* break out of the big for{} */                   /*706*/
          memcpy( lclstack, ptr , bytesread );                          /*706*/
          lclframe = lclstack;                                          /*706*/
                                                                        /*706*/
          /*******************************************************************/
          /* Scoot down through the stack until we find a CS:IP that      706*/
          /* was generated by a far call. This will be our "prospective"  706*/
          /* frame.                                                       706*/
          /*******************************************************************/
          SS = Flat2Sel(frame);                                         /*706*/
          for(nbytes = 6; (nbytes <= bytesread); lclframe++,nbytes += 2)/*706*/
          {                                                             /*706*/
           IP = *(lclframe+1);                                          /*706*/
           CS = *(lclframe+2);                                          /*706*/
           FlatCSIP=Sys_SelOff2Flat( CS ,IP );                         /*706*/
                                                                        /*706*/
           if ( IsFarReturn(FlatCSIP ) )                                /*827*/
           {                                                            /*706*/
            ushort  NextIP;                                             /*706*/
            ushort  NextCS;                                             /*706*/
            uint    NextFlatCSIP;                                       /*706*/
            uint    numbytes;                                           /*706*/
            ushort nextlclframe[3];                                     /*706*/
                                                                        /*706*/
            /*****************************************************************/
            /* Now, let's take our "prospective" frame and see if it      706*/
            /* is linked to another frame. If it is, then we continue     706*/
            /* unwinding from our "prospective" frame.                    706*/
            /*****************************************************************/
            BP = *lclframe;                                             /*706*/
/*827*/     ptr = (ushort *)Getnbytes((uint)Sys_SelOff2Flat(SS,BP),6,&numbytes);
            if( numbytes != 6 )                                         /*706*/
             continue;                                                  /*706*/
            memcpy(nextlclframe,ptr,6);                                 /*706*/
                                                                        /*706*/
            NextIP = *(nextlclframe+1);                                 /*706*/
            NextCS = *(nextlclframe+2);                                 /*706*/
            NextFlatCSIP=Sys_SelOff2Flat( NextCS ,NextIP );            /*706*/
                                                                        /*706*/
            if (  IsFarReturn(NextFlatCSIP ) )                          /*827*/
            {                                                           /*706*/
             frame = frame + (uint)( (lclframe-lclstack)*sizeof(short));/*706*/
             memcpy( lframe, lclframe, 6 );                             /*706*/
             break;                                                     /*706*/
            }                                                           /*706*/
           }  /* end of test for look ahead frame.                */    /*706*/
          }   /* end of loop to scan stack looking for next frame.*/    /*706*/
         }    /* end of code block for generic 16-32 thunk.       */    /*706*/
        }     /* end of test for generic 16-32 thunk.             */    /*706*/
       }      /* end of ShowAllFlag tests.                        */    /*706*/
/*------------------------------------------------------------------------706*/
       /**********************************************************************/
       /* - When we get here, we will just switch the stack to 16 bit if  706*/
       /*   the ShowAllFlag was not set.                                  706*/
       /* - If the ShowAllFlag was set then the two additional tests will 706*/
       /*   have set up the stack frame for continuing the unwind.        706*/
       /* - If the ShowAllFlag was set and neither test found a frame,    706*/
       /*   then we fall into default processing as if the tests had      706*/
       /*   not been run.                                                 706*/
       /******************************************************************706*/
       if( StackSwitch == TRUE )                                        /*706*/
       {                                                                /*706*/
        ActCSAtrs[n]=ATR16;                                             /*706*/
        SSAtr=ATR16;                                                    /*706*/
        goto stack16;                                                   /*706*/
       }                                                                /*706*/
      }                                                                 /*706*/

      ActFaddrs[n] = EIP;               /* stuff EIP value into ActFaddrs 107*/
      frame = *lframe;                  /* define new frame               107*/
     }                                  /* end curr frame 32-bit          107*/
     else                               /* curr frame is 16-bit           107*/
     {
stack16:
      SS = Flat2Sel(frame);             /* stack selector for 16-bit      107*/
      BP = (ushort)*lframe;             /* define BP from stack           107*/
      IP = *((ushort*)lframe+1);        /* define IP from stack           107*/
      CS = *((ushort*)lframe+2);        /* define CS from stack           107*/
      PrevFrame = frame;                /* hold on to previos frame loc.  706*/
      frame = (uint)Sys_SelOff2Flat( SS , BP ); /* define new frame          107*/

      ret = NEARRET;                    /* assume near return on stack.      */
      FlatCSIP=Sys_SelOff2Flat( CS ,IP ); /* trial return flat addr607*/
      if (                              /* addr valid and                 827*/
           IsFarReturn( FlatCSIP ) )    /* this a 16-bit far return ?     827*/
       ret = FARRET;

      /***********************************************************************/
      /* At this point, we have determined the return type of the stack      */
      /* frame address.  Now, if the return is near, then this frame was     */
      /* run in the CS of a previous frame with a far return.  By            */
      /* previous, I mean a frame that we have already unwound.  If it's a   */
      /* far return, then we update the CS that subsequent nodes were run in.*/
      /*                                                                     */
      /***********************************************************************/
      if( ret == NEARRET )
      {
       /**********************************************************************/
       /* Test the return address to see if it was generated by a 16 bit     */
       /* near call.                                                         */
       /**********************************************************************/
       CS=ReturnCS;                                                     /*706*/
       FlatCSIP=Sys_SelOff2Flat(CS,IP);                              /*706*/
       if( IsNearReturn(FlatCSIP,(ushort)0) )                           /*827*/
        goto NEAR_16_BIT;                                               /*706*/
                                                                        /*706*/
       /**********************************************************************/
       /* Let's look for a CL386 thunk.                                      */
       /**********************************************************************/
       FlatCSIP=*(lframe+1);                                            /*706*/
       if ( IsNearReturn(FlatCSIP,(ushort)1) )                          /*827*/
        goto THUNK;                                                     /*706*/
                                                                        /*706*/
       /**********************************************************************/
       /* Okay... so we'll look for a Cset thunk.                         706*/
       /**********************************************************************/
       FlatCSIP = CsetThunk(&frame,lframe);                             /*706*/
       if ( IsNearReturn(FlatCSIP,(ushort)1) )                          /*827*/
        goto THUNK;                                                     /*706*/
                                                                        /*706*/
       /**********************************************************************/
       /* Hmmm...  well, this could be a PL/X86 thunk.                       */
       /* The frame will be EIP/IP/CS.  Let's try!                           */
       /**********************************************************************/
       IP = *((ushort*)lframe+2);                                       /*706*/
       CS = *((ushort*)lframe+3);                                       /*706*/

       /**********************************************************************/
       /* - test bits 2 and 3 of the CS to verify that it's a valid          */
       /*   LDT ring 2/3 selector.                                           */
       /**********************************************************************/
       if( ( (CS & 6) == 6 )  &&
           ( FlatCSIP=Sys_SelOff2Flat(CS,IP) ) &&                    /*706*/
           IsFarReturn(FlatCSIP)                                        /*827*/
         )                                                              /*827*/
        goto THUNK;                                                     /*706*/
                                                                        /*706*/
/*------------------------------------------------------------------------706*/
       if( ShowAllFlag == TRUE )                                        /*706*/
       {                                                                /*706*/
       /**********************************************************************/
       /* Well, let's try one final brute force technique.                706*/
       /*                                                                 706*/
       /**********************************************************************/
        #define STACK_DWORDS 50                                         /*706*/
        uint *ptr;                                                      /*706*/
        uint  lclstack[STACK_DWORDS];                                   /*706*/
        int   nbytes;                                                   /*706*/
        uint *lclframe;                                                 /*706*/
                                                                        /*706*/
        /*********************************************************************/
        /* go back to the previous frame and align on a dword boundary.   706*/
        /*********************************************************************/
        frame = PrevFrame;                                              /*706*/
        frame += 3;                                                     /*706*/
        frame &= 0xfffffffc;                                            /*706*/
                                                                        /*706*/
        /*********************************************************************/
        /* now get a chunk of the stack and make it local.                706*/
        /*********************************************************************/
/*827*/ ptr = (uint *)Getnbytes(frame, STACK_DWORDS*sizeof(int),&bytesread);
        if ( bytesread < 8 )                                            /*706*/
         break;                                                         /*706*/
        memcpy( lclstack, ptr , bytesread );                            /*706*/
        lclframe = lclstack;                                            /*706*/
                                                                        /*706*/
        /*********************************************************************/
        /* Scoot down through the stack until we get to an EIP that was   706*/
        /* generated by a near call. This will be our "prospective" frame.706*/
        /*********************************************************************/
        for(nbytes = 8; (nbytes <= bytesread); lclframe++, nbytes += 4) /*706*/
        {                                                               /*706*/
         FlatCSIP = *(lclframe+1);                                      /*706*/
         if ( IsNearReturn(FlatCSIP,(ushort)1) )                        /*827*/
         {                                                              /*706*/
          uint  nextcsip;                                               /*706*/
          uint  nextlclframe[2];                                        /*706*/
          uint  numbytes;                                               /*706*/
                                                                        /*706*/
          /*******************************************************************/
          /* Now, let's take our "prospective" frame and see if it really 706*/
          /* is linked to another frame. If it is, then we continue       706*/
          /* unwinding from our "prospective" frame.                      706*/
          /*******************************************************************/
          ptr = (uint *)Getnbytes( *lclframe, 8, &numbytes);            /*827*/
          if( numbytes != 8 )                                           /*706*/
           continue;                                                    /*706*/
          memcpy(nextlclframe,ptr,8);                                   /*706*/
                                                                        /*706*/
          nextcsip  = *(nextlclframe + 1 );                             /*706*/
          if ( IsNearReturn(nextcsip,(ushort)1) )                       /*827*/
          {                                                             /*706*/
           frame = frame + (lclframe - lclstack)*sizeof(int);           /*706*/
           memcpy( lframe, lclframe, 8 );                               /*706*/
           goto THUNK;                                                  /*706*/
          }                                                             /*706*/
         }                                                              /*706*/
        }                                                               /*706*/
       } /* end of ShowAllFlag test    */                               /*706*/
/*------------------------------------------------------------------------706*/
       /**********************************************************************/
       /* you have run out of luck at this point so we just quit.         706*/
       /**********************************************************************/
       break;                                                           /*706*/

THUNK:
       ActCSAtrs[n]=ATR32;            /* next frame is 32-bit           107*/
       SSAtr=ATR32;                   /* set for next loop iteration    107*/
       frame=*lframe;                 /* define new 32-bit frame        107*/

      }
      else /* if(ret == FARRET) */      /* stack return is far               */
       ReturnCS=CS;                     /* update the return CS              */

NEAR_16_BIT:                                                            /*706*/
      ActFaddrs[n] = FlatCSIP;          /* stuff flat CS:IP into ActFaddrs107*/
     }                                  /* end curr frame is 16-bit       107*/

     if( frame < limit )                /* if frame is outside frame space   */
      break;                            /*                                   */
     /************************************************************************/
     /* In the following statements FindScope() returns a pointer to the     */
     /* SSProc record in the symbols area for an EIP return value in the     */
     /* ActFaddrs array.  As a side effect, the mid and lno associated with  */
     /* this EIP are put into the ActFlines array.  The offset ( EBP ) into  */
     /* the stack of the stack frame is stored in ActFrames.                 */
     /************************************************************************/

#if 0
     ActScopes[n] = FindScope( ActFaddrs[n] , /* ->EIP value to scope     107*/
                         &ActFlines[n][0] );  /* where to stuff mid/lno   224*/
#endif
     ActFrames[n] = frame;              /* stack offset of this frame        */

     /************************************************************************/
     /* At this point, if we're only interested in seeing stack frames    706*/
     /* for which we have named functions, then if the stack frame has    706*/
     /* no scope, then we simply respin the loop on this value of n.      706*/
     /* This can be time consuming especially for large PM apps.          706*/
     /************************************************************************/
#if 0
     if( (ShowNamedFlag == TRUE) && (ActScopes[n] == 0) )               /*706*/
      n--;                                                              /*706*/
#endif
    }                                   /* end of stack frame scan           */
  }                                     /*                                   */
 else                                   /* if AppPTB_EBP was null         107*/
  n = 0 ;                               /* then there are no active frames   */
                                        /*                                   */
 /****************************************************************************/
 /* Save the number of active frames and define  a pointer to the SSProc     */
 /* record for the currently executing function.                             */
 /****************************************************************************/
 NActFrames = n;                        /* save the number of active frames  */

 if( NActFrames )
 {
  pActCSAtrs = Talloc(NActFrames*sizeof(ActCSAtrs[0]));
  pActFrames = Talloc(NActFrames*sizeof(ActFrames[0]));
  pActFaddrs = Talloc(NActFrames*sizeof(ActFaddrs[0]));
  memcpy(pActCSAtrs,ActCSAtrs,NActFrames*sizeof(ActCSAtrs[0]));
  memcpy(pActFrames,ActFrames,NActFrames*sizeof(ActFrames[0]));
  memcpy(pActFaddrs,ActFaddrs,NActFrames*sizeof(ActFaddrs[0]));

  *ppActCSAtrs = pActCSAtrs;
  *ppActFrames = pActFrames;
  *ppActFaddrs = pActFaddrs;
 }
 return(NActFrames);

}

/*****************************************************************************/
/* IsFarReturn()                                                             */
/*                                                                           */
/* Description:                                                              */
/*   Verifies that a stack return address was generated by a far call.       */
/*                                                                           */
/* Parameters:                                                               */
/*   addr        input - flat address of instruction.                     107*/
/*   pdf         input - EXE/DLL containing the instruction.                 */
/*                                                                           */
/* Return:                                                                   */
/*   TRUE or FALSE.                                                          */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   pdf is valid.                                                           */
/*                                                                           */
/*****************************************************************************/
int IsFarReturn(UINT addr)              /* ->current instruction             */
                                        /* EXE/DLL containing addr.          */
{                                       /*                                   */
 DTIF    packet;                        /* disassembly info packet.          */
 uint    BaseLoadAddrOfObject;
/*****************************************************************************/
/*                                                                        706*/
/* 1. Find out the base load address of the object containing this        706*/
/*    address.                                                            706*/
/* 2. Roll back the address one instruction.                              706*/
/* 3. Disassemble the instruction and test for far return.                706*/
/*                                                                        706*/
/*****************************************************************************/
 if( ( BaseLoadAddrOfObject = GetBaseAddr(addr) ) == 0 )                /*706*/
  return(FALSE);                                                        /*706*/
 addr = _rollback ( addr, -1, BaseLoadAddrOfObject );                   /*706*/
 memset(&packet,0,sizeof(DTIF));        /* bzero->memset.               /*706*/
 packet.Flags.Use32bit=0;               /* we are in USE16 segment      /*706*/
 _GetInstrPacket( addr, (DTIF*)&packet );                                /*706*/
 if(packet.retType == CALLFARIND ||     /* far call indirect.           /*706*/
    packet.retType == CALLFARDIR ||     /* far call direct.             /*706*/
    packet.retType == RETFAR            /* far return. ( PM dispatch).  /*706*/
   )                                                                    /*706*/
  return( TRUE );                                                       /*706*/
 return( FALSE );                                                       /*706*/
}                                       /* end IsFarReturn().           /*706*/

/*****************************************************************************/
/* IsNearReturn()                                                            */
/*                                                                           */
/* Description:                                                              */
/*   Verifies that a stack return address was generated by a near 16-/32-bit */
/*   caller!                                                              107*/
/*                                                                           */
/* Parameters:                                                               */
/*   addr        input - flat address of instruction.                        */
/*   pdf         input - EXE/DLL containing the instruction.                 */
/*   Im32Bit     input - tells us this is a USE32 segment or not.            */
/*                                                                           */
/* Return:                                                                   */
/*   TRUE or FALSE.                                                          */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   pdf is valid.                                                           */
/*                                                                           */
/*****************************************************************************/
int IsNearReturn(UINT addr,USHORT Im32Bit)
                                        /* ->current instruction             */
                                        /* EXE/DLL containing addr.          */
                                        /* 1=>USE32  0=>USE16                */
{                                       /*                                   */
 DTIF    packet;                        /* disassembly info packet.          */
 ULONG   BaseAddr;
/*****************************************************************************/
/*                                                                           */
/* 1. Find out the base address and length of the module that contains       */
/*    the address.                                                           */
/* 2. Roll back the address one instruction.                                 */
/* 3. Disassemble the instruction and test for far return.                   */
/*                                                                           */
/*****************************************************************************/

 if( (BaseAddr = GetBaseAddr(addr)) == 0)
  return(FALSE);
 addr = _rollback ( addr, -1, BaseAddr);
 memset(&packet,0,sizeof(DTIF));        /* bzero->memset.                    */
 packet.Flags.Use32bit=0;               /* assume USE16 segment              */
 if ( Im32Bit )
   packet.Flags.Use32bit=1;             /* USE32 segment                     */
 _GetInstrPacket( addr, (DTIF*)&packet );
 if(packet.retType == CALLIPDISP  ||    /* near call IP+DISP.                */
    packet.retType == CALLNEARIND ||    /* near call indirect.               */
    packet.retType == CALLREGNEAR       /* near call direct.                 */
   )
  return( TRUE );
 return( FALSE );
}                                       /* end IsNearReturn().               */

/*****************************************************************************/
/* _rollback                                                                  */
/*                                                                           */
/* Description:                                                              */
/*   adjusts an address backwards in the instruction stream.                 */
/*                                                                           */
/* Parameters:                                                               */
/*   iap        ->current instruction (where we are sitting in showA window).*/
/*   deltai     number of instructions to adjust iap by.                     */
/*   fbyte      offset of first byte in current module.                      */
/*   lbyte      offset of last byte in current module.                       */
/*                                                                           */
/* Return:                                                                   */
/*   p          iap modified by deltai worth of instructions (when possible).*/
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   deltai is negative.                                                     */
/*                                                                           */
/*****************************************************************************/
UINT _rollback(UINT iap,int deltai,UINT fbyte )
                                        /* ->current instruction             */
                                        /* # of instsrs to adj for disasm    */
                                        /* offset of first byte in curr mid  */
{                                       /* begin instdelta                   */
  int   lentab[ROLLBACK];               /* retained instruction lengths      */
  int   i;                              /* index into lentab                 */
  uint  trialiap;                       /* experimental ptr to scroll     107*/
  UCHAR *streamptr;                     /* ->read in instruction stream   107*/
  UCHAR type;                           /* indicates 16- or 32-bit code   107*/
  uint  read;                           /* number of bytes read in by DBGetCS*/
  UCHAR bitness;

/*****************************************************************************/
/*                                                                           */
/* Scrolling assembly instructions backward is tricky.    The idea is to     */
/* start disassembling (for instruction length only) at a point well behind  */
/* where you currently are and keep track of these lengths.  At some point,  */
/* this stream of disassembly will (about 99.999% of the time!) meet back at */
/* the current instruction.  You can then back track thru an array of lengths*/
/* to figure out the proper address to scroll back to.                       */
/*                                                                           */
/*****************************************************************************/
  if ( deltai < 0 &&                    /* want to delta backward and        */
       iap != fbyte )                   /*   we can ?                     107*/
  {                                     /* begin delta backward              */
    bitness = _GetBitness( iap );          /* set type for InstLengthGlob    107*/
    type = (bitness==BIT16)?USE16:USE32;
    trialiap = fbyte;                   /* assume scroll back to mid start107*/
    if ( fbyte + ROLLBACK <  iap )      /* just need rollback amount ?    107*/
      trialiap = iap - ROLLBACK;        /* scroll back shorter amt        107*/

    streamptr=Getnbytes(trialiap,       /* read in all bytes up thru addr 107*/
                        iap-trialiap,
                        &read);

    i = 0;                              /* initialize index into lentab      */
    while( trialiap < iap )             /* still need disasm lengths ?    107*/
    {                                   /* disasm forward till we converge   */
      lentab[i] = _InstLengthGlob( streamptr, type ); /* gimme inst len !  107*/
      trialiap += lentab[i];            /* bump to next instr address     107*/
      streamptr += lentab[i++];         /* bump to next instr & next entry107*/

    }                                   /* end disasm frwd till we converge  */

    if ( trialiap == iap )              /* did we converge ?              107*/
      for(
           i--;                         /* back up to last instr entry       */
           i >= 0;                      /* make sure we still have entries   */
           i--                          /* back up another entry             */
         )
      {                                 /* add up all instr lengths for delta*/
        iap -= lentab[i];               /* back up by this entry's length 107*/
        if ( !( ++deltai ) )            /* done scrolling back ?             */
          break;                        /* finished adjusting iap            */
      }                                 /* end add up all instr lens for delt*/
  }                                     /* end delta backward                */

  return( iap );                        /* give back new deltad address      */
}                                       /* end instdelta                     */


/*************************************************************************107*/
/* InstLengthGlob()                                                       107*/
/*                                                                        107*/
/* Description:                                                           107*/
/*   Gets the length of an assembler instruction where the instruction    107*/
/*   stream is already in our memory (no more DBGets!).                   107*/
/*                                                                        107*/
/* Parameters:                                                            107*/
/*   inststreamptr  input - points to instruction stream.                 107*/
/*   type           input - 0=>USE16, 1=>USE32.                           107*/
/*                                                                        107*/
/* Return:                                                                107*/
/*                  length of instruction.                                107*/
/*                                                                        107*/
/* Assumptions:                                                           107*/
/*                                                                        107*/
/*    instruction stream already read in via DBGet!                       107*/
/*************************************************************************107*/
UCHAR _InstLengthGlob( UCHAR* inststreamptr, UCHAR type )
{
 DTIF     InstrPacket;
 int      PacketSize;
 UCHAR    InstrLen;                     /* instruction length             534*/
 UCHAR    hexbuffer[HEXBUFFSIZE];       /* where disassembler puts hex.   108*/
 char     mnebuffer[MNEMBUFFSIZE];      /* where disassembler puts mne.   108*/
 UCHAR    textbuffer[TEXTBUFFSIZE];     /* where disassembler puts text.  108*/

 memset(&InstrPacket,0, sizeof(InstrPacket));
 memset(hexbuffer   ,0, sizeof(hexbuffer));
 memset(mnebuffer   ,0, sizeof(mnebuffer));
 memset(textbuffer  ,0, sizeof(textbuffer));

 InstrPacket.InstPtr=inststreamptr;     /* ->read in hex from user app       */
 InstrPacket.InstEIP=0xffffffff;        /* EIP value for this instr->        */
 InstrPacket.Flags.Use32bit=type;       /* based upon address type           */
 InstrPacket.Flags.MASMbit=1;           /* 1 for masm disassembly.           */
 InstrPacket.Flags.N387bit=1;           /* not a 80x87 processor instr       */
 InstrPacket.Flags.Unused1=0;           /* make zero due to possible futur   */
 /****************************************************************************/
 /* We don't really care about these buffers at this time. We put them       */
 /* in to satisfy the disassembler.                                          */
 /****************************************************************************/
 InstrPacket.HexBuffer=hexbuffer;       /* hexbuffer will have instr strea   */
 InstrPacket.MneBuffer=mnebuffer;       /* -> disassembled mnemonic.         */
 InstrPacket.TextBuffer=textbuffer;     /* for disasembly text               */
 DisAsm( &InstrPacket );                /* disassemble current instruction   */
 InstrLen = InstrPacket.retInstLen;     /*                                534*/
 if(InstrPacket.retType == REPETC )     /* instr is of repeat form ?      534*/
 {                                      /*                                534*/
   InstrPacket.HexBuffer=hexbuffer;     /* hexbuffer will have instr strea534*/
   InstrPacket.MneBuffer=mnebuffer;     /* -> disassembled mnemonic.      534*/
   InstrPacket.TextBuffer=textbuffer;   /* for disasembly text            534*/
   DisAsm( &InstrPacket );              /* disassemble current instruction534*/
   InstrLen += InstrPacket.retInstLen;  /*                                534*/
 }                                      /*                                534*/
 return(InstrLen);                      /* caller gets instruction length 534*/
}                                       /* end GetInstrPacket.               */

/*****************************************************************************/
/* CsetThunk()                                                            706*/
/*                                                                        706*/
/* Description:                                                           706*/
/*   Find the EIP and the EBP of the caller in a 32-16 thunk.             706*/
/*                                                                        706*/
/*            ------------                                                706*/
/*    BP---->|    BP      |---  16 bit pointer to top of thunk save area  706*/
/*            ------------    |                                           706*/
/*           |            |   |                                           706*/
/*            ------------    |                                           706*/
/*           |            |                                              706*/
/*            -------------------------  <----32 bit save area. 38H bytes 706*/
/*      ---0 |REGSAVE_2_ESP            |                                  706*/
/*     |      -------------------------                                   706*/
/*     |   4 |REGSAVE_SS               |                                  706*/
/*     |      -------------------------                                   706*/
/*     |   8 |                         |                                  706*/
/*     |      -------------------------                                   706*/
/*     |   C |                         |                                  706*/
/*     |      -------------------------                                   706*/
/*     |   10|                         |                                  706*/
/*     |      -------------------------                                   706*/
/*     |   14|                         |                                  706*/
/*     |      -------------------------                                   706*/
/*     |   18|REGSAVE_2_GS|REGSAVE_2_FS|                                  706*/
/*     |      -------------------------                                   706*/
/*     |   1C|REGSAVE_2_DS|REGSAVE_2_ES|                                  706*/
/*     |      -------------------------                                   706*/
/*     |   20|REGSAVE_2_ESI            |                                  706*/
/*     |      -------------------------                                   706*/
/*     |   24|REGSAVE_2_EDI            |                                  706*/
/*     |      -------------------------                                   706*/
/*     |   28|REGSAVE_2_EBX            |                                  706*/
/*     |      -------------------------                                   706*/
/*     |   2C|REGSAVE_2_EBP            | <---this is the EBP of the       706*/
/*     |      -------------------------      caller's frame.              706*/
/*      -->30|REGSAVE_2_EIP            | <---this is the return address   706*/
/*            -------------------------      into the caller.             706*/
/*         34|REGSAVE_1_ESP            |                                  706*/
/*            -------------------------                                   706*/
/*           |                         |                                  706*/
/*           |                         |                                  706*/
/*           | caller's auto           |                                  706*/
/*           |                         |                                  706*/
/*  caller's |                         |                                  706*/
/*   EBP----> -------------------------                                   706*/
/*           |      EBP                |                                  706*/
/*            -------------------------                                   706*/
/*           |      ESP                | <-- this is for a stack adjustmnt706*/
/*            -------------------------      within the prolog of a 32 bit706*/
/*           |      EIP                |     function that contains a     706*/
/*            -------------------------       thunk to a 16 bit function. 706*/
/*           |                         |                                  706*/
/*                                                                        706*/
/*                                                                        706*/
/* Parameters:                                                            706*/
/*   *pframe          input - ->to the variable( called "frame" actually) 706*/
/*                              that holds the current frame location.    706*/
/*                                                                        706*/
/* Return:                                                                706*/
/*   CallerRetAddr      - the flat return eip into the caller.            706*/
/*                                                                        706*/
/* Assumptions:                                                           706*/
/*                                                                        706*/
/*************************************************************************706*/
#define BYTESTOREAD 8                                                   /*706*/
#define OFFSET_OF_REGSAVE_2_GS   0x18                                   /*706*/
#define OFFSET_OF_REGSAVE_2_EBP  0x2C                                   /*706*/
uint CsetThunk(uint *pframe , uint *lframe )                            /*706*/
{                                                                       /*706*/
 uint  BytesObtained;                                                   /*706*/
 uint *PtrToFrame;                                                      /*706*/
 uint  frame;                                                           /*706*/
 uint  CallerRetAddr;                                                   /*706*/
                                                                        /*706*/
 /************************************************************************706*/
 /* add +30 to get to the offset of the "real" frame.                     706*/
 /************************************************************************706*/
 frame = *pframe + OFFSET_OF_REGSAVE_2_EBP - OFFSET_OF_REGSAVE_2_GS;    /*706*/
                                                                        /*706*/
 /************************************************************************706*/
 /* Get 8 bytes of from the users stack. EBP/EIP.                         706*/
 /************************************************************************706*/
                                                                        /*706*/
 PtrToFrame = (uint *)Getnbytes(frame, BYTESTOREAD, &BytesObtained);    /*827*/
                                                                        /*706*/
 /************************************************************************706*/
 /* Return the EIP unless we couldn't read from the stack.                706*/
 /************************************************************************706*/
 if ( BytesObtained != BYTESTOREAD )                                    /*706*/
  return(0);                                                            /*706*/
                                                                        /*706*/
 /************************************************************************706*/
 /* - update the lframe for the caller.                                   706*/
 /* - update the frame location in the caller.                            706*/
 /* - return the caller's EIP.                                            706*/
 /************************************************************************706*/
 memcpy( lframe, PtrToFrame, BYTESTOREAD );                             /*706*/
 *pframe = *PtrToFrame;                                                 /*706*/
 CallerRetAddr = *(PtrToFrame+1);                                       /*706*/
                                                                        /*706*/
 return( CallerRetAddr);                                                /*706*/
}                                       /* end CsetThunk().             /*706*/

/*****************************************************************************/
/* IsValid_EBP()                                                             */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Checks an EBP to see if it meets the requirements of being a valid      */
/*   link in the chain of EBPs.                                              */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   ebp            EBP that we're testing.                                  */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   EBP_Ok    TRUE ==>valid.                                                */
/*             FALSE==>invalid.                                              */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/

#define BIT32_STACK_WIDTH   4
#define EIP_ITERATION_CHECK 2
#define USE32_SEGMENT       1

BOOL IsValid_EBP( ULONG ebp )
{
 ULONG  StackAddress;
 BOOL   EBP_Ok;
 int    iterations;
 UCHAR *pEIP_Buffer;
 ULONG  ulEIP;
 UINT   BytesRead;

 StackAddress = ebp + BIT32_STACK_WIDTH;
 EBP_Ok       = FALSE;

 for( iterations = 1; iterations <= EIP_ITERATION_CHECK; iterations++ )
 {
  pEIP_Buffer = Getnbytes( StackAddress, BIT32_STACK_WIDTH, &BytesRead);
  ulEIP       = *(ULONG *)pEIP_Buffer;

  if ( IsNearReturn( ulEIP, USE32_SEGMENT ) == TRUE )
  {
   EBP_Ok = TRUE;
   break;
  }
  StackAddress += BIT32_STACK_WIDTH;
 }
 return( EBP_Ok );
}
