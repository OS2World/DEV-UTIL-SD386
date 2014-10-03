/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   cuadata.h                                                               */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*  Static structures for all pulldowns,popups and actionbar.                */
/*                                                                           */
/*...Release 1.01 (04/03/92)                                                 */
/*...                                                                        */
/*... 05/08/92  701   Srinivas  Cua Interface.                               */
/*****************************************************************************/

/*****************************************************************************/
/* Data for getfunc popup.                                                   */
/*****************************************************************************/
static uchar  Popt_GetFunc[]  = " Search For Function ";
static uchar  Popp_GetFunc[]  = "Enter the Function name:";
static BUTTON Popbt_GetFunc[] = { {15,27,5,"Enter",ENTER},
                                  {15,37,6,"Cancel",ESC},
                                  {15,48,4,"Help",F1} };
POPUPSHELL Pop_GetFunc = { 8,16,9,46,3,1,Popt_GetFunc,Popp_GetFunc,
                           &Popbt_GetFunc[0]};

/*****************************************************************************/
/* Data for browse popup.                                                    */
/*****************************************************************************/
static uchar  Popt_Browse[]  = " Browse a File ";
static uchar  Popp_Browse[]  = "Enter the File name:";
static BUTTON Popbt_Browse[] = { {15,27,5,"Enter",ENTER},
                                 {15,37,6,"Cancel",ESC},
                                 {15,48,4,"Help",F1} };
POPUPSHELL Pop_Browse = { 8,16,9,46,3,1,Popt_Browse,Popp_Browse,
                           &Popbt_Browse[0]};

/*****************************************************************************/
/* Data for getfile popup.                                                   */
/*****************************************************************************/
static uchar  Popt_GetFile[]  = " Search For a File ";
static uchar  Popp_GetFile[]  = "Enter the File name:";
static BUTTON Popbt_GetFile[] = { {15,27,5,"Enter",ENTER},
                                  {15,37,6,"Cancel",ESC},
                                  {15,48,4,"Help",F1} };
POPUPSHELL Pop_GetFile = { 8,16,9,46,3,1,Popt_GetFile,Popp_GetFile,
                           &Popbt_GetFile[0]};

/*****************************************************************************/
/* Data for find string popup.                                               */
/*****************************************************************************/
static uchar  Popt_FindStr[]  = " Search For a String ";
static uchar  Popp_FindStr[]  = "Enter the String:";
static BUTTON Popbt_FindStr[] = { {15,27,5,"Enter",ENTER},
                                  {15,37,6,"Cancel",ESC},
                                  {15,48,4,"Help",F1} };
POPUPSHELL Pop_FindStr = { 8,16,9,46,3,1,Popt_FindStr,Popp_FindStr,
                           &Popbt_FindStr[0]};

/*****************************************************************************/
/* Data for conditional break points popup.                                  */
/*****************************************************************************/
static uchar  Popt_CondBkp[]  = " Conditional Breakpoints ";
static uchar  Popp_CondBkp[]  = "Enter the Break Condition:";
static BUTTON Popbt_CondBkp[] = { {15,27,5,"Enter",ENTER},
                                  {15,37,6,"Cancel",ESC},
                                  {15,48,4,"Help",F1} };
POPUPSHELL Pop_CondBkp = { 8,16,9,46,3,1,Popt_CondBkp,Popp_CondBkp,
                           &Popbt_CondBkp[0]};

/*****************************************************************************/
/* Data for name or address break points popup.                              */
/*****************************************************************************/
static uchar  Popt_Bkpt[]  = " Breakpoints ";
static uchar  Popp_Bkpt[]  = "Enter the Name Or Address:";
static BUTTON Popbt_Bkpt[] = { {15,27,5,"Enter",ENTER},
                               {15,37,6,"Cancel",ESC},
                               {15,48,4,"Help",F1} };
POPUPSHELL Pop_Bkpt = { 8,16,9,46,3,1,Popt_Bkpt,Popp_Bkpt,
                           &Popbt_Bkpt[0]};

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
