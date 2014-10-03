/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   netbios.c                                                               */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*  Netbios management functions.                                            */
/*                                                                           */
/* History:                                                                  */
/*                                                                           */
/*   07/15/94 Created.                                                       */
/*                                                                           */
/*****************************************************************************/

#include "all.h"

/*****************************************************************************/
/* NetBiosInit()                                                             */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Set up netbios session with a remote probe.                             */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   pConnection    -> to the structure defining the remote connection.      */
/*   pLsnHandle     where to stuff the session handle for the caller.        */
/*   pNB_MoreRcInfo ->holder of additional netbios info.                     */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   rc            a netbios return code.                                    */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   The following session names are agreed upon by the dbg and the esp:     */
/*                                                                           */
/*    esp?          -> name on the esp side of the connection.               */
/*    sd3?          -> name of the dbg side of the connection.               */
/*                                                                           */
/*   where ? is a 13 character name extracted from the /n invocation         */
/*           option.                                                         */
/*                                                                           */
/*****************************************************************************/
static  unsigned ( * _Far16 _Pascal NetBios)(char *) = NULL;

APIRET  NetBiosInit( CONNECTION *pConnection,
                        LHANDLE *pLsnHandle,
                            int *pNB_MoreRcInfo )
{
 APIRET  rc;
 BYTE    sess;
 BYTE    ncbs;
 BYTE    names;
 BYTE    RequestOrRelease;
 char    LsnDbgName[MAX_LSN_NAME];         /* max length allowed by netbios. */
 char    LsnEspName[MAX_LSN_NAME];
 int     DbgOrEsp;
 char    buf[CCHMAXPATH];
 ULONG   mte = 0;

 /****************************************************************************/
 /* - Load the Netbios entry point.                                          */
 /* - We will assume that if we can load it that we will be able to get      */
 /*   the procedure address.                                                 */
 /****************************************************************************/
 if( NetBios == NULL )
 {
  rc = DosLoadModule( buf, sizeof(buf), "ACSNETB", &mte );
  if(rc)
  {
   *pNB_MoreRcInfo = CANT_LOAD_NETB_DLL;
   return(rc);
  }

  DosQueryProcAddr(mte, 0, "NETBIOS",(PFN*)&NetBios);
 }

 DbgOrEsp = pConnection->DbgOrEsp;
 /****************************************************************************/
 /* - grab some resources from the netbios pool.                             */
 /****************************************************************************/
 if( IsParent() == TRUE )
 {
  sess  = 2;
  ncbs  = 3;
  names = 1;
 }
 else
 {
  sess  = 1;
  ncbs  = 2;
  names = 1; /* <-- need a 1 here but I don't know why...*/
             /* we will not be adding a name to the adapter table. */
 }

 RequestOrRelease = REQUEST;

 rc = NetBios_Reset( sess, ncbs, names, RequestOrRelease, pNB_MoreRcInfo );
 if( rc ) return(rc);

 /****************************************************************************/
 /* - now build the logical session names for the dbg/esp ends of the        */
 /*   connection. This connection assumes that the dbg and esp ends          */
 /*   of the connection have the same logical session name as defined        */
 /*   by the user. We're using an agreed upon convention.                    */
 /****************************************************************************/
 BuildLsnName(LsnDbgName,LSN_DBG,pConnection);
 BuildLsnName(LsnEspName,LSN_ESP,pConnection);

 /****************************************************************************/
 /* - tell the name to the adapter if first instance of the connection.      */
 /****************************************************************************/
 if( IsParent() == TRUE )
 {
  if( DbgOrEsp == _DBG )
   rc = NetBios_AddName( LsnDbgName );
  else
   rc = NetBios_AddName( LsnEspName );

  if(rc) return(rc);
 }

 /****************************************************************************/
 /* - now, make/receive the call to/from the remote session.                 */
 /****************************************************************************/
 rc = NetBios_Call_Answer( DbgOrEsp,LsnDbgName,LsnEspName,pLsnHandle);
 return(rc);
}

/*****************************************************************************/
/* NetBios_Reset()                                                           */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Requests/Releases resources from the netbios pool.                      */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   sess                  number of sessions.                               */
/*   ncbs                  number of network control blocks(NCBs).           */
/*   names                 number of unique name identifiers.                */
/*   RequestOrRelease      0 = request 1 = release.                          */
/*   pInadequateResources  ->holder for which resource is inadequate.        */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   rc  = NCB.Reset.ncb_retcode. This is the return code from the           */
/*         netbios call.                                                     */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*  This is currently hard coded for the primary adapter.                    */
/*                                                                           */
/*****************************************************************************/
APIRET NetBios_Reset( BYTE  sess,
                      BYTE  ncbs,
                      BYTE  names,
                      BYTE  RequestOrRelease,
                      int  *pInadequateResources)
{
 NCB_RESET NCB_Reset;

 memset(&NCB_Reset, 0, sizeof(NCB_Reset) );

 NCB_Reset.ncb_command  = NB_RESET_WAIT;
 NCB_Reset.ncb_lana_num = PRIMARY_ADAPTER;
 NCB_Reset.ncb_lsn      = RequestOrRelease;
 NCB_Reset.req_sessions = sess;
 NCB_Reset.req_commands = ncbs;
 NCB_Reset.req_names    = names;

 (* NetBios)( (char *)&NCB_Reset );

 *pInadequateResources = 0;

 if( NCB_Reset.ncb_retcode == NB_INADEQUATE_RESOURCES )
 {
  if( NCB_Reset.act_sessions != sess )
  {
   *pInadequateResources = INADEQUATE_SESSIONS;
  }
  else if( NCB_Reset.act_commands != ncbs )
  {
   *pInadequateResources = INADEQUATE_COMMANDS;
  }
  else if( NCB_Reset.act_names != names )
  {
   *pInadequateResources = INADEQUATE_NAMES;
  }
 }
 return(NCB_Reset.ncb_retcode);
}

/*****************************************************************************/
/* NetBios_AddName()                                                         */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Add a unique logical name that will identify this adapter to the        */
/*   the network.                                                            */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   pLocalName    -> to the unique name identifying the adapter.            */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   rc  = ncb.ncb_retcode. This is the return code from the                 */
/*         netbios call.                                                     */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
APIRET NetBios_AddName( char *pLocalName )
{
 NCB       ncb;

 memset(&ncb, 0, sizeof(ncb) );

 ncb.ncb_command  = NB_ADD_NAME_WAIT;
 ncb.ncb_lana_num = PRIMARY_ADAPTER;

 strncpy(ncb.ncb_name,pLocalName,16);

 (* NetBios)( (char *)&ncb );

 return(ncb.ncb_retcode);
}

/*****************************************************************************/
/* NetBios_Call_Answer()                                                     */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Make a call to get a netbios connection to an esp. Return the           */
/*   logical session number back to the caller.                              */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   DbgOrEsp      whether we are connecting from esp or dbg.                */
/*   pDbgName      -> to the unique name identifying the Dbg session.        */
/*   pEspName      -> to the unique name identifying the Esp session.        */
/*   pLsn          -> to the unique session number returned by netbios.      */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   rc  = NCB.Reset.ncb_retcode. This is the return code from the           */
/*         netbios call.                                                     */
/*                                                                           */
/*   ncb.ncb_lsn = logical session number that we get back from netbios.     */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
APIRET NetBios_Call_Answer(int      DbgOrEsp,
                           char    *pDbgName,
                           char    *pEspName,
                           LHANDLE *pLsn)
{
 NCB    ncb;

 memset(&ncb, 0, sizeof(ncb) );

 ncb.ncb_lana_num = PRIMARY_ADAPTER;
 ncb.ncb_rto      = 0;                  /* infinite receive timeout.         */
 ncb.ncb_sto      = 0;                  /* infinite send timeout.            */

 /****************************************************************************/
 /* - make/listen for the call                                               */
 /* - give the handle back to the caller.                                    */
 /****************************************************************************/
 ncb.ncb_command  = NB_CALL_WAIT;
 strncpy(ncb.ncb_name ,pDbgName, 16);
 strncpy(ncb.ncb_callname, pEspName, 16);
 if( DbgOrEsp == _ESP )
 {
  ncb.ncb_command  = NB_LISTEN_WAIT;
  strncpy(ncb.ncb_name, pEspName, 16);
  strncpy(ncb.ncb_callname, pDbgName, 16);
 }

 /****************************************************************************/
 /* - try to make the connection a number of times and then give up.         */
 /****************************************************************************/
 {
  int retries = 5;

  ncb.ncb_retcode = NB_SES_OPEN_REJECTED;
  while( (ncb.ncb_retcode == NB_SES_OPEN_REJECTED) && (retries > 0) )
  {
   ( * NetBios)( (char *)&ncb );
   retries--;
  }
 }

 *pLsn = ncb.ncb_lsn;

 return(ncb.ncb_retcode);
}
/*****************************************************************************/
/* NetBiosOriginateSession()                                                 */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   This function is only used for parallel connections to set up           */
/*   connection between the esp and dbg queues. The resources have           */
/*   already been allocated by parent dbg/esp.                               */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   pConnection    -> to the structure defining the remote connection.      */
/*   pLsnHandle     where to stuff the session handle for the caller.        */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   rc            a netbios return code.                                    */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   The following session names are agreed upon by the dbg and the esp:     */
/*                                                                           */
/*    esp?          -> name on the esp side of the connection.               */
/*    sd3?          -> name of the dbg side of the connection.               */
/*                                                                           */
/*   where ? is a 13 character name extracted from the /n invocation         */
/*           option.                                                         */
/*                                                                           */
/*****************************************************************************/
APIRET  NetBiosOriginateSession( CONNECTION *pConnection , LHANDLE *pLsnHandle)
{
 APIRET  rc;
 char    LsnDbgName[MAX_LSN_NAME];         /* max length allowed by netbios. */
 char    LsnEspName[MAX_LSN_NAME];
 int     DbgOrEsp;

 DbgOrEsp = pConnection->DbgOrEsp;

 /****************************************************************************/
 /* - now build the logical session names for the dbg/esp ends of the        */
 /*   connection.  This connection assumes that the dbg and esp ends of      */
 /*   the connection have the same logical session name as defined by the    */
 /*   user.  We're using an agreed upon convention.                          */
 /****************************************************************************/
 BuildLsnName(LsnDbgName,LSN_DBG,pConnection);
 BuildLsnName(LsnEspName,LSN_ESP,pConnection);

 /****************************************************************************/
 /* - now, make/receive the call to/from the remote session.                 */
 /****************************************************************************/
 rc = NetBios_Call_Answer( DbgOrEsp,LsnDbgName,LsnEspName,pLsnHandle);
 return(rc);
}

/*****************************************************************************/
/* - a utility routine to build the logical session names.                   */
/*****************************************************************************/
void BuildLsnName( char *LsnName, char *header, CONNECTION *pConnection )
{
 char *cp;
 int   len;

 /****************************************************************************/
 /* - clear the caller's buffer.                                             */
 /* - put in the header.                                                     */
 /* - concatenate the user specified logical session name.                   */
 /****************************************************************************/
 memset( LsnName,' ',MAX_LSN_NAME );
 strcpy(LsnName, header);

 cp  = LsnName + strlen(LsnName);
 len = MAX_LSN_NAME - strlen(LsnName);
 if( strlen(pConnection->pLsnName) > len )
  memcpy(cp, pConnection->pLsnName, len );
 else
  memcpy(cp, pConnection->pLsnName, strlen(pConnection->pLsnName) );
 return;
}

/*****************************************************************************/
/* NetBios_Send()                                                            */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Send a netbios message buffer.                                          */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   pBuf       -> our transmit buffer.                                      */
/*   Len        number of bytes to transmit.                                 */
/*   LsnHandle  handle for the connection.                                   */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
void NetBios_Send( LHANDLE LsnHandle, PVOID pBuf, ULONG Len)
{
 NCB ncb;

 memset(&ncb, 0, sizeof(ncb) );

 ncb.ncb_command        = NB_SEND_WAIT;
 ncb.ncb_lana_num       = PRIMARY_ADAPTER;
 ncb.ncb_lsn            = LsnHandle;
 ncb.ncb_buffer_address = pBuf;
 ncb.ncb_length         = (USHORT)Len;

 ( * NetBios)( (char *)&ncb );

 if( ncb.ncb_retcode != 0 )
 {
  if( ncb.ncb_retcode != NB_SESSION_CLOSED )
  {
   if( IsVerbose()){printf("\nNetbios send error rc=%d",ncb.ncb_retcode);fflush(0);}
  }
 }
}

/*****************************************************************************/
/* NetBios_Recv()                                                            */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Receive a netbios message buffer.                                       */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   pBuf       -> our transmit buffer.                                      */
/*   Len        number of bytes to transmit.                                 */
/*   LsnHandle  handle for the connection.                                   */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
void NetBios_Recv( LHANDLE LsnHandle, PVOID pBuf, ULONG Len)
{
 NCB ncb;

 memset(&ncb, 0, sizeof(ncb) );

 ncb.ncb_command        = NB_RECEIVE_WAIT;
 ncb.ncb_lana_num       = PRIMARY_ADAPTER;
 ncb.ncb_lsn            = LsnHandle;
 ncb.ncb_buffer_address = pBuf;
 ncb.ncb_length         = (USHORT)Len;

 (* NetBios)( (char *)&ncb );

 if( ncb.ncb_retcode != 0 )
 {
  if( ncb.ncb_retcode != NB_SESSION_CLOSED )
  {
   if( IsVerbose() ){printf("\nNetbios receive error rc=%d",ncb.ncb_retcode);fflush(0);}
  }
 }
}

void NetBiosClose( LHANDLE LsnHandle )
{
 NCB    ncb;

 /****************************************************************************/
 /* - We might get a return code of NB_SESSION_CLOSED from this call if      */
 /*   the other end of the connection has already hung up. This is ok.       */
 /****************************************************************************/
 memset(&ncb, 0, sizeof(ncb) );

 ncb.ncb_command        = NB_HANG_UP_WAIT;
 ncb.ncb_lana_num       = PRIMARY_ADAPTER;
 ncb.ncb_lsn            = LsnHandle;

 (* NetBios)( (char *)&ncb );

 if( IsVerbose() ) {printf("\nNetbios close rc=%d",ncb.ncb_retcode);fflush(0);}
}
