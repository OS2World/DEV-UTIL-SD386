#include "all.h"

#define ENDOFCMDS 0

/*****************************************************************************/
/* - verbose handling for commands sent to the probe.                     917*/
/*****************************************************************************/
typedef struct
{
 int   msgnum;
 char *msg;
}ESP_CMD;

static ESP_CMD  CmdMsg[] =
{
 FINDEXE               , "FindEexe"               ,
 STARTUSER             , "StartUser"              ,
 GOINIT                , "GoInit"                 ,
 DEFBRK                , "DefBrk"                 ,
 UNDBRK                , "UndBrk"                 ,
 PUTINBRK              , "PutInBrk"               ,
 PULLOUTBRK            , "PullOutBrk"             ,
 INSERTALLBRK          , "InsertAllBrk"           ,
 REMOVEALLBRK          , "RemoveAllBrk"           ,
 DOSDEBUG              , "DosDebug"               ,
 GETTHREADINFO         , "GetThreadInfo"          ,
 FREEZETHREAD          , "FreezeThread"           ,
 THAWTHREAD            , "ThawThread"             ,
 GETCALLSTACK          , "GetCallStack"           ,
 GOSTEP                , "GoStep"                 ,
 GETEXEORDLLENTRY      , "GetExeOrDllEntry"       ,
 NORMALQUIT            , "NormalQuit"             ,
 SETEXECADDR           , "SetExecAddr"            ,
 GOFAST                , "GoFast"                 ,
 DEFWPS                , "DefWps"                 ,
 PUTINWPS              , "PutInWps"               ,
 PULLOUTWPS            , "PullOutWps"             ,
 GETDATABYTES          , "GetDataBytes"           ,
 GETMEMBLKS            , "GetMemBlks"             ,
 SETXCPTNOTIFY         , "SetXcptNotify"          ,
 SETEXECTHREAD         , "SetExecThread"          ,
 WRITEREGS             , "WriteRegs"              ,
 GETCOREGS             , "GetCoregs"              ,
 TERMINATEESP          , "TerminateEsp"           ,
 SETESPRUNOPTS         , "SetEspRunOpts"          ,
 CTRL_BREAK            , "CtrlBreak"              ,
 NEW_PROCESS           , "NewProcess"             ,
 START_ESP_QUE         , "StartEspQue"            ,
 START_QUE_LISTEN      , "StartQueListen"         ,
 CONNECT_DBG           , "ConnectDbg"             ,
 CONNECT_ESP           , "ConnectEsp"             ,
 SERIAL_POLL           , "SerialPoll"             ,
 CONNECT_NOTIFY        , "ConnectNotify"          ,
 GOENTRY               , "GoEntry"                ,
 KILL_LISTEN_THREAD    , "KillListenThread"       ,
 SELECT_SESSION        , "SelectSession"          ,
 ENDOFCMDS             , ""
};

/*****************************************************************************/
/* - print the command message.                                              */
/*****************************************************************************/
void PrintCmdMessage( int cmd )
{
 int              i;
 ESP_CMD         *pmsg;

 for( i = 0 , pmsg = CmdMsg; pmsg[i].msgnum != ENDOFCMDS ; i++ )
 {
  if( ( cmd == pmsg[i].msgnum ) )
  {
   if( pmsg[i].msgnum == SERIAL_POLL )
    printf("%c",'_');
   else
   {
    char  fmt[10];
    sprintf(fmt,"\n%c-%d%c", '%', strlen(pmsg[i].msg),'s' );
    printf(fmt, pmsg[i].msg );
   }
   break;
  }
  fflush(0);
 }
}

#ifdef __ESP__
/*---------------------------------------------------------------------------*/
/*****************************************************************************/
/* - verbose handling for the esp queue messages.                            */
/*****************************************************************************/
typedef struct
{
 int   msgnum;
 char *msg;
}ESPQ_MSG;


static ESPQ_MSG  EspMsg[] =
{
 ESP_QMSG_END_SESSION    , "Esp_Qmsg_End_Session"    ,
 ESP_QMSG_NEW_SESSION    , "Esp_Qmsg_New_Session"    ,
 ESP_QMSG_CTRL_BREAK     , "Esp_Qmsg_Ctrl_Break"     ,
 ESP_QMSG_DISCONNECT     , "Esp_Qmsg_Disconnect"     ,
 ESP_QMSG_CONNECT_REQUEST, "Esp_Qmsg_Connect_Request",
 ESP_QMSG_QUE_TERM       , "Esp_Qmsg_Que_Term"       ,
 ESP_QMSG_ERROR          , "Esp_Qmsg_Error"          ,
 ESP_QMSG_EMPTY          , "Esp_Qmsg_Empty"          ,
 ESP_QMSG_NEW_PROCESS    , "Esp_Qmsg_New_Process"    ,
 ESP_QMSG_OPEN_CONNECT   , "Esp_Qmsg_OpenConnect"    ,
 ESP_QMSG_SELECT_SESSION , "Esp_Qmsg_Select_Session" ,
 ESP_QMSG_SELECT_ESP     , "Esp_Qmsg_Select_Esp"     ,
 ESP_QMSG_PARENT_TERM    , "Esp_Qmsg_Parent_Term"    ,
 ESP_QMSG_CHILD_TERM     , "Esp_Qmsg_Child_Term"     ,
  -1 , ""
};

void PrintQueMessage( void *p,void *q)
{
 PREQUESTDATA     pqr = (PREQUESTDATA)p;
 int              i;
 ESPQ_MSG          *pmsg;

 for( i = 0 , pmsg = EspMsg; pmsg[i].msgnum != -1 ; i++ )
 {
  if( (pqr->ulData == pmsg[i].msgnum ) )
  {
   printf("\n%-21s",  pmsg[i].msg );fflush(0);
   break;
  }
 }
}
/*---------------------------------------------------------------------------*/
#endif

#ifdef __DBG__
/*---------------------------------------------------------------------------*/
/*****************************************************************************/
/* - verbose handling for the dbg queue messages.                            */
/*****************************************************************************/
typedef struct
{
 int   msgnum;
 char *msg;
}DBGQ_MSG;


static DBGQ_MSG  DbgMsg[] =
{
 DBG_QMSG_CTRL_BREAK        , "DbgQmsgCtrlBreak"       ,
 DBG_QMSG_OPEN_CONNECT      , "DbgQmsgOpenConnect"     ,
 DBG_QMSG_QUE_TERM          , "DbgQmsgQueTerm"         ,
 DBG_QMSG_NEW_PROCESS       , "DbgQmsgNewProcess"      ,
 DBG_QMSG_DISCONNECT        , "DbgQmsgDisConnect"      ,
 DBG_QMSG_CONNECT_ESP       , "DbgQmsgConnectEsp"      ,
 DBG_QMSG_SELECT_SESSION    , "DbgQmsgSelectSession"   ,
 DBG_QMSG_ERROR             , "DbgQmsgError"           ,
 DBG_QMSG_KILL_LISTEN       , "DbgQmsgKillListen"      ,
 DBG_QMSG_PARENT_TERM       , "DbgQmsgParentTerm"      ,
 DBG_QMSG_CHILD_TERM        , "DbgQmsgChildTerm"       ,
 DBG_QMSG_SELECT_PARENT_ESP , "DbgQmsgSelectParentEsp" ,
  -1 , ""
};

void PrintDbgQueMessage( void *p)
{
 PREQUESTDATA     pqr = (PREQUESTDATA)p;
 int              i;
 DBGQ_MSG          *pmsg;

 for( i = 0 , pmsg = DbgMsg; pmsg[i].msgnum != -1 ; i++ )
 {
  if( (pqr->ulData == pmsg[i].msgnum ) )
  {
   printf("\n%-21s",  pmsg[i].msg );fflush(0);
   break;
  }
 }
}
/*---------------------------------------------------------------------------*/
#endif
