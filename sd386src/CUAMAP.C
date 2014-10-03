/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   Cuamap.c                                                                */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   All the Cua Mapping functions.                                          */
/*                                                                           */
/*...Release 1.01 (04/03/92)                                                 */
/*...                                                                        */
/*... 05/08/92  701   Srinivas  Cua Interface.                               */
/*... 01/12/93  808   Selwyn    Profile fixes/improvements.                  */
/*****************************************************************************/
#include "all.h"
uchar ScrollShade1[] =  {SHADEDARK,0};
uchar ScrollShade2[] =  {Attrib(vaMenuCsr),' ',0};
uchar hilite[]       =  {RepCnt(1),Attrib(vaMenuCsr),0};
uchar hiatt[]        =  {RepCnt(1),Attrib(vaMenuSel),0};
uchar badhilite[]    =  {RepCnt(1),Attrib(vaBadSel),0};
uchar normal[]       =  {RepCnt(1),Attrib(vaMenuBar),0};
uchar badnormal[]    =  {RepCnt(1),Attrib(vaBadAct),0};
uchar Shadow[]       =  {RepCnt(1),Attrib(vaShadow),0};
uchar ClearField[]   =  {RepCnt(1),' ',0};
uchar InScrollMode   =  FALSE;

extern CmdParms cmd;
extern UINT     MenuRow;

/*****************************************************************************/
/* GetFuncsFromEvents()                                                      */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*  Get i/o events from the user and find the function he wants to exec.     */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   none                                                                    */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   FuncCode   code of the function to be executed.                         */
/*                                                                           */
/*****************************************************************************/
#define RETURN 1

int GetFuncsFromEvents( int EscKeyFlag , void * pParmBlk )
{
 int       FuncCode;
 EVENT    *pEvent;
 int       Choice;
 static    uint      WaitTime = SEM_INDEFINITE_WAIT;
 int       ( * FuncCodeGetter)(int , void *);

 FuncCode = 0;
 FuncCodeGetter = CuaMenuBar;
 for(;;)
 {
   /**************************************************************************/
   /* Get an event from the "getter" of function codes.                      */
   /*  - Based on the event and the interface in use, decide which           */
   /*    "getter" to call.                                                   */
   /*  - initialize a choice for the getter.                                 */
   /**************************************************************************/
   pEvent = GetEvent( WaitTime );
   switch( pEvent->Type )
   {
    case TYPE_MOUSE_EVENT:

     WaitTime = SEM_INDEFINITE_WAIT;
     switch( pEvent->Value )
     {
       case EVENT_BUTTON_2_DOWN:
         if( MouseEventInActionBar( &Choice ) )
         {
           beep();
           continue;
         }
         else
           return( RIGHTMOUSECLICK );

       case EVENT_BUTTON_1_DOWN:
       {
         if( pEvent->Row == MenuRow )
         {
           switch( pEvent->Col )
           {
             case SHRINKUPARROWCOL:
             case EXPANDDNARROWCOL:

              if( pEvent->FakeEvent == TRUE )
                WaitTime = SCROLL_REGULAR_WAIT;
              else
                WaitTime = SCROLL_INITIAL_WAIT;

              if( pEvent->Col == SHRINKUPARROWCOL )
                return( SHRINKSTORAGE );

              if( pEvent->Col == EXPANDDNARROWCOL )
                return( EXPANDSTORAGE );

              break;

             default:
              break;
           }
         }
       }                                /* intentional fall - through        */

       goto case_EVENT_BUTTON_1_DOWN_MOVE;
case_EVENT_BUTTON_1_DOWN_MOVE:

       case EVENT_BUTTON_1_DOWN_MOVE:
         if( MouseEventInActionBar( &Choice ) )
         {
           if( pEvent->Value == EVENT_BUTTON_1_DOWN )
             break;
           else
             continue;
         }
         else
         if( pEvent->Value == EVENT_BUTTON_1_DOWN )
           return( LEFTMOUSECLICK );
         continue;

       default:
         continue;
     }
     break;

    case TYPE_KBD_EVENT:

     WaitTime = SEM_INDEFINITE_WAIT;
     Choice = pEvent->Value;
     switch( Choice )
     {
       case ESC:
        if( EscKeyFlag == RETURN )
         return( ESCAPE );
        else
        {
          if( KeyInActionBarExpressKeys( &Choice ) )
            break;
          FuncCodeGetter = (int (*)(int,void*))Convert;
        }
       break;

       case F10:
        Choice = -1;
        break;

       case  key_0:                  /* 0  0x0b30                         */
       case  key_1:                  /* 1  0x0231                         */
       case  key_2:                  /* 2  0x0332                         */
       case  key_3:                  /* 3  0x0433                         */
       case  key_4:                  /* 4  0x0534                         */
       case  key_5:                  /* 5  0x0635                         */
       case  key_6:                  /* 6  0x0736                         */
       case  key_7:                  /* 7  0x0837                         */
       case  key_8:                  /* 8  0x0938                         */
       case  key_9:                  /* 9  0x0a39                         */
        return(Choice);


       default:
        if( KeyInActionBarExpressKeys( &Choice ) )
          break;

        FuncCodeGetter = (int (*)(int,void*))Convert;
/*Supporting a command line interface using MSH.*/
/*MSH   commandLine.nparms=0; */
        break;
     }
     break;

#ifdef MSH
  case TYPE_MSH_EVENT:
/*Supporting a command line interface using MSH.*/
     commandLine.nparms=0;
     FuncCodeGetter = (int (*)(int,void*))MshSD386CommandProcessor;
     break;
#endif
   }
   FuncCode = FuncCodeGetter( Choice , pParmBlk);

   /********************************************************************* 808*/
   /* If the function code returned is ExpressBar (ESC), we have to display  */
   /* the action bar with the first pulldown.                                */
   /**************************************************************************/
   if( FuncCode == EXPRESSBAR )                                         /*808*/
   {                                                                    /*808*/
     Choice = 1;                                                        /*808*/
     FuncCodeGetter = CuaMenuBar;                                       /*808*/
     FuncCode = FuncCodeGetter( Choice , pParmBlk);                     /*808*/
/*Supporting a command line interface using MSH.*/
/*MSH   commandLine.nparms=0; */
/*MSH   commandLine.nparms=0; */
   }                                                                    /*808*/
/* SD386Log(FuncCode,NULL); */
   return( FuncCode );
 }
}
