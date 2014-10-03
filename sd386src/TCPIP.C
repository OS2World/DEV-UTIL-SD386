/*****************************************************************************/
/* File:                                                                     */
/*   tcpip.c                                                                 */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*  tcpip management functions.                                              */
/*                                                                           */
/* History:                                                                  */
/*                                                                           */
/*   05/01/96 Created.                                                       */
/*                                                                           */
/*****************************************************************************/

#include "all.h"

#define  OS2
 #undef   T_NULL                        /* resolve conflict                  */
 #undef   T_PTR                         /* resolve conflict                  */
 #include <types.h>
 #include <netinet\in.h>
 #include <arpa\nameser.h>
 #include <resolv.h>
 #include <sys\socket.h>
 #include <netdb.h>
#undef   OS2

/*****************************************************************************/
/* - tcpip dlls are loaded only if the user tries to make a tcpip            */
/*   connection.                                                             */
/* - functions are called by pointer.                                        */
/* - following are the prototypes currently used.                            */
/*****************************************************************************/
#define htons_(x)   ((* bswap_)(x))
#define ntohs_(x)   ((* bswap_)(x))
#define htonl_(x)   ((* lswap_)(x))
#define ntohl_(x)   ((* lswap_)(x))

static int ( * _System sock_init_)( void );
static void( * _System psock_errno_)( char * );
static int ( * _System socket_)( int, int, int );
static int ( * _System connect_)( int, struct sockaddr *, int );
static int ( * _System bind_)( int, struct sockaddr *, int );
static int ( * _System gethostid_)(void);
static int ( * _System listen_)( int, int);
static int ( * _System accept_)( int, struct sockaddr *, int * );
static int ( * _System sock_errno_)( void );
static int ( * _System send_)( int, char *, int, int );
static int ( * _System recv_)( int, char *, int, int );
static int ( * _System soclose_)( int );
static int ( * _System setsockopt_)( int, int, int, char *, int );

static struct hostent * ( * _System gethostbyname_)( char * );
static struct servent * ( * _System getservbyname_)( char *, char * );
static struct hostent * ( * _System gethostbyaddr_)( char *, int, int );
static unsigned short   ( * _System bswap_)(unsigned short);
static unsigned long    ( * _System lswap_)(unsigned long);
static char *           ( * _System inet_ntoa_)(struct in_addr);

/*****************************************************************************/
/* - this is the socket decriptor bound to the machine address on the        */
/*   server side of the connection.                                          */
/*****************************************************************************/
static int BaseSocketDescriptor;

int  SockGetDescriptor(void){return(BaseSocketDescriptor);}
void SockSetDescriptor(int s){BaseSocketDescriptor=s;}

/*****************************************************************************/
/* SockInit()                                                                */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Manage the sd386/esp connections.                                       */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   pConnection    -> to the structure defining the remote connection.      */
/*   pLsnHandle     where to stuff the session handle for the caller.        */
/*   p_MoreRcInfo   ->holder of additional tcpip error return codes(rc).     */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   rc            system/tcpip return code.                                 */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   SD386 is the client and esp is the server.                              */
/*                                                                           */
/*****************************************************************************/
APIRET SockInit( CONNECTION *pConnection,
                 LHANDLE    *pLsnHandle,
                 int        *p_MoreRcInfo )
{
 APIRET  rc;
 char    buf[CCHMAXPATH];
 ULONG   somte = 0;
 ULONG   tcpmte = 0;
 int     SocketDescriptor;
 int     NewSocketDescriptor;
 int     DbgOrEsp;

 /****************************************************************************/
 /* - load the dlls and the function entry points.                           */
 /****************************************************************************/
 rc = DosLoadModule( buf, sizeof(buf), "SO32DLL", &somte );
 if(rc)
 {
  *p_MoreRcInfo = CANT_LOAD_TCPIP_DLL;
  return(rc);
 }

 rc = DosLoadModule( buf, sizeof(buf), "TCP32DLL", &tcpmte );
 if(rc)
 {
  *p_MoreRcInfo = CANT_LOAD_TCPIP_DLL;
  return(rc);
 }

 if( (rc = DosQueryProcAddr(somte,  0, "SOCK_INIT"    , (PFN*)&sock_init_  ))||
     (rc = DosQueryProcAddr(somte,  0, "PSOCK_ERRNO"  , (PFN*)&psock_errno_))||
     (rc = DosQueryProcAddr(somte,  0, "SOCK_ERRNO"   , (PFN*)&sock_errno_ ))||
     (rc = DosQueryProcAddr(somte,  0, "SOCKET"       , (PFN*)&socket_     ))||
     (rc = DosQueryProcAddr(somte,  0, "CONNECT"      , (PFN*)&connect_    ))||
     (rc = DosQueryProcAddr(somte,  0, "GETHOSTID"    , (PFN*)&gethostid_  ))||
     (rc = DosQueryProcAddr(somte,  0, "BIND"         , (PFN*)&bind_       ))||
     (rc = DosQueryProcAddr(somte,  0, "LISTEN"       , (PFN*)&listen_     ))||
     (rc = DosQueryProcAddr(somte,  0, "ACCEPT"       , (PFN*)&accept_     ))||
     (rc = DosQueryProcAddr(somte,  0, "SEND"         , (PFN*)&send_       ))||
     (rc = DosQueryProcAddr(somte,  0, "RECV"         , (PFN*)&recv_       ))||
     (rc = DosQueryProcAddr(somte,  0, "SOCLOSE"      , (PFN*)&soclose_    ))||
     (rc = DosQueryProcAddr(somte,  0, "SETSOCKOPT"   , (PFN*)&setsockopt_ ))||
     (rc = DosQueryProcAddr(tcpmte, 0, "BSWAP"        , (PFN*)&bswap_      ))||
     (rc = DosQueryProcAddr(tcpmte, 0, "LSWAP"        , (PFN*)&lswap_      ))||
     (rc = DosQueryProcAddr(tcpmte, 0, "INET_NTOA"    , (PFN*)&inet_ntoa_  ))||
     (rc = DosQueryProcAddr(tcpmte, 0, "GETHOSTBYNAME", (PFN*)&gethostbyname_))||
     (rc = DosQueryProcAddr(tcpmte, 0, "GETHOSTBYADDR", (PFN*)&gethostbyaddr_))||
     (rc = DosQueryProcAddr(tcpmte, 0, "GETSERVBYNAME", (PFN*)&getservbyname_))
   )
  return(rc);

 /****************************************************************************/
 /* - check for a running tcpip.                                             */
 /****************************************************************************/
 rc = (* sock_init_)();
 if( rc != 0 )
 {
  (* psock_errno_)("\nsock_init()");
  *p_MoreRcInfo = TCPIP_NOT_RUNNING;
   return(1);
 }

 /****************************************************************************/
 /* - Esp is the server.                                                     */
 /* - Dbg(SD386) is the client.                                              */
 /****************************************************************************/
 DbgOrEsp = pConnection->DbgOrEsp;

 rc = 0;
 if( DbgOrEsp == _DBG )
 {
  /***************************************************************************/
  /* - on the client side grab a socket descriptor and connect to the        */
  /*   foreign machine name.                                                 */
  /***************************************************************************/
  SocketDescriptor = SockSocket();
  if( SocketDescriptor < 0 )
  {
   *p_MoreRcInfo = TCPIP_ERROR;
   return(1);
  }
  rc = SockConnect(SocketDescriptor, pConnection->pLsnName );
  NewSocketDescriptor  = SocketDescriptor;
  BaseSocketDescriptor = 0;
 }
 else /* esp/server */
 {
  /***************************************************************************/
  /* - on the server side grab a socket descriptor, bind the port to         */
  /*   to the machine address, and signal readiness for connections.         */
  /***************************************************************************/
  if( IsParent() )
  {
   BaseSocketDescriptor = SocketDescriptor = SockSocket();
   if( SocketDescriptor < 0 )
   {
    *p_MoreRcInfo = TCPIP_ERROR;
    return(1);
   }
  }
  else
   SocketDescriptor = BaseSocketDescriptor;

  if( IsParent() )
  {
   rc = SockBind(SocketDescriptor);
   if(rc == 0 )
   {
    rc = SockListen(SocketDescriptor);
   }
  }

  /***************************************************************************/
  /* - now accept a connection from a client.                                */
  /***************************************************************************/
  if( rc == 0 )
  {
   NewSocketDescriptor = SockAccept(SocketDescriptor);
   if(NewSocketDescriptor < 0 )
    rc = TCPIP_ERROR;
  }
 }

 if( rc )
 {
  *p_MoreRcInfo = rc;
   rc = 1;
 }
 else
  *pLsnHandle = NewSocketDescriptor;

 return(rc);
}

/*****************************************************************************/
/* SockGetAnotherSocket()                                                    */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   This function is only used for parallel connections to set up           */
/*   connection between the esp and dbg queues.                              */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   pConnection    -> to the structure defining the remote connection.      */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   SocketDescriptor                                                        */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
int SockGetAnotherSocket( CONNECTION *pConnection )
{
 int     SocketDescriptor;
 int     DbgOrEsp;

 /****************************************************************************/
 /* - Esp is the server.                                                     */
 /* - Dbg(SD386) is the client.                                              */
 /****************************************************************************/
 DbgOrEsp = pConnection->DbgOrEsp;

 if( DbgOrEsp == _DBG )
 {
  SocketDescriptor = SockSocket();
  SockConnect(SocketDescriptor, pConnection->pLsnName );
 }
 else /* esp/server */
 {
  SocketDescriptor = SockAccept(BaseSocketDescriptor);
 }
 return(SocketDescriptor);
}

/*****************************************************************************/
/* SockSocket()                                                              */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Get a stream socket.                                                    */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   SocketDescriptor                                                        */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
int SockSocket( void )
{
 int SocketDescriptor;
 /****************************************************************************/
 /*  Get a stream socket for accepting connections.                          */
 /*                                                                          */
 /*   SocketDescriptor = socket( int domain, int type, int protocol )        */
 /*                                                                          */
 /*   domain   = AF_INET is the domain supported by OS/2.                    */
 /*   type     = SOCK_STREAM for reliable xmission at TCP layer.             */
 /*   protocol = 0 for default protocol associated with AF_INET/SOCK_STREAM  */
 /*                                                                          */
 /****************************************************************************/
 SocketDescriptor = (* socket_)(AF_INET, SOCK_STREAM, 0);
 if( SocketDescriptor < 0)
 {
  (* psock_errno_)("\nSocket()");
 }
 return(SocketDescriptor);
}

/*****************************************************************************/
/* SockConnect()                                                             */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Connect to a server.                                                    */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   SocketDescriptor                                                        */
/*   ip_name           this is a foreign name such as "elvis".               */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   rc                                                                      */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
int SockConnect( int SocketDescriptor, char *ip_name )
{
 int port;
 int rc;

 struct in_addr      IP_address;
 struct sockaddr_in  ForeignAddress;
 struct servent      ServEnt;
 struct hostent      ForeignHost;
 struct sockaddr    *pForeignAddress;
 struct servent     *pServEnt;
 struct hostent     *pForeignHost;

 pForeignHost    = &ForeignHost;
 pForeignAddress = (struct sockaddr *)&ForeignAddress;
 pServEnt        = &ServEnt;

 /****************************************************************************/
 /* - get the name and address of this machine.                              */
 /****************************************************************************/
 memset(&ForeignHost, 0, sizeof(ForeignHost));

 pForeignHost = gethostbyname_(ip_name);
 if( pForeignHost == NULL )
  return(TCPIP_NO_HOST_NAME);

 /****************************************************************************/
 /* - get the sockets port assigned to sd386.                                */
 /****************************************************************************/
 pServEnt = getservbyname_( "sd386", NULL );
 if( pServEnt == NULL )
  return(TCPIP_NO_SERVICES_PORT);

 /****************************************************************************/
 /* - define/print the foreign address we're connecting to.                  */
 /****************************************************************************/
 IP_address.s_addr = *((unsigned long *)pForeignHost->h_addr);
 port              = ntohs_(pServEnt->s_port);

 if( IsVerbose() ) {printf("\nforeign addr = %s", inet_ntoa_(IP_address));}
 if( IsVerbose() ) {printf("\nforeign name = %s", pForeignHost->h_name ); }
 if( IsVerbose() ) {printf("\nforeign port = %d", port );fflush(0);       }

 /****************************************************************************/
 /* - make a connection.                                                     */
 /* - this is done by the client only.                                       */
 /* - if the server is not ready to accept connections, then sleep for       */
 /*   a second and try again.                                                */
 /****************************************************************************/
 rc = -1;
 while( rc != 0 )
 {
  memset(&ForeignAddress, 0, sizeof(ForeignAddress));
  ForeignAddress.sin_family      = AF_INET;
  ForeignAddress.sin_port        = htons_(port);
  ForeignAddress.sin_addr.s_addr = IP_address.s_addr;
  rc = (* connect_)(SocketDescriptor, pForeignAddress, sizeof(ForeignAddress));
  if( rc )
  {
   (* psock_errno_)("\nconnect()");
   rc = (* sock_errno_)();
   if( rc == SOCECONNREFUSED )
    return(TCPIP_ESP_NOT_STARTED);
   else
    return(TCPIP_ERROR);
  }
 }
 return(0);
}

/*****************************************************************************/
/* SockBind()                                                                */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Bind a socket to a machine address. (Server only )                      */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   SocketDescriptor                                                        */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   rc                                                                      */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
int SockBind( int SocketDescriptor )
{
 int rc;
 int port;

 struct in_addr      IP_address;
 struct servent      ServEnt;
 struct sockaddr_in  ServerAddress;
 struct sockaddr    *pServerAddress;
 struct servent     *pServEnt;
 struct hostent      HostEnt;
 struct hostent     *pHostEnt = &HostEnt;

 pServerAddress = (struct sockaddr *)&ServerAddress;
 pServEnt       = &ServEnt;

 /****************************************************************************/
 /* - get the name and address of this machine.                              */
 /****************************************************************************/
 IP_address.s_addr = htonl_(gethostid_());

 pHostEnt = (* gethostbyaddr_)((char*)&IP_address, sizeof(IP_address), AF_INET);

 printf("\nmachine addr = %s", inet_ntoa_(IP_address));
 printf("\nmachine name = %s", pHostEnt->h_name);

 /****************************************************************************/
 /* - get the sockets port assigned to sd386.                                */
 /****************************************************************************/
 pServEnt = getservbyname_( "sd386", NULL );
 if( pServEnt == NULL )
  return(TCPIP_NO_SERVICES_PORT);

 port = ntohs_(pServEnt->s_port);

 printf("\nsockets port = %d", port );fflush(0);
 fflush(0);

 memset(pServerAddress, 0, sizeof(ServerAddress) );

 ServerAddress.sin_family      = AF_INET;
 ServerAddress.sin_port        = htons_(port);
 ServerAddress.sin_addr.s_addr = INADDR_ANY;

 rc = (* bind_)( SocketDescriptor, pServerAddress, sizeof(ServerAddress) );
 if( rc < 0)
 {
  (* psock_errno_)("\nbind()");
  return(TCPIP_ERROR);
 }
 return(rc);
}

/*****************************************************************************/
/* SockListen()                                                              */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Signal readiness for accepting connections on the server side of        */
/*   the connection.                                                         */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   SocketDescriptor                                                        */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   rc                                                                      */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
int SockListen( int SocketDescriptor )
{
 int rc;
 /****************************************************************************/
 /* - signal readiness for accepting connections.                            */
 /****************************************************************************/
 rc = (* listen_)(SocketDescriptor, 1);
 if(rc)
 {
  (* psock_errno_)("\nlisten()");
 }
 return(rc);
}

/*****************************************************************************/
/* SockAccept()                                                              */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Accept connections on the server side.                                  */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   SocketDescriptor                                                        */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   rc                                                                      */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
int SockAccept( int SocketDescriptor )
{
 int SocketNameLen;
 int NewSocketDescriptor;

 struct sockaddr_in  ForeignAddress;
 struct in_addr      IP_address;
 struct hostent      HostEnt;
 struct hostent     *pHostEnt        = &HostEnt;
 struct sockaddr    *pForeignAddress = (struct sockaddr *)&ForeignAddress;

 /****************************************************************************/
 /* - accept a connection from a foreign host.                               */
 /****************************************************************************/
 memset(&ForeignAddress, 0, sizeof(ForeignAddress) );
 SocketNameLen = sizeof(ForeignAddress);
 NewSocketDescriptor = (* accept_)(SocketDescriptor,
                                   pForeignAddress,
                                   &SocketNameLen);
 if( NewSocketDescriptor == -1 )
 {
  (* psock_errno_)("\naccept()");
  return(-1);
 }

 /****************************************************************************/
 /* - get the name and address of this machine.                              */
 /****************************************************************************/
 IP_address = ForeignAddress.sin_addr;
 pHostEnt   = (* gethostbyaddr_)((char*)&IP_address, sizeof(IP_address), AF_INET);

 printf("\nconnection accepted from %s", pHostEnt->h_name );

 return(NewSocketDescriptor);
}

/*****************************************************************************/
/* SockSend()                                                                */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Send bytes.                                                             */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   SocketDescriptor                                                        */
/*   pBuf             -> our transmit buffer.                                */
/*   Len              number of bytes to transmit.                           */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
void SockSend( int SocketDescriptor, PVOID pBuf, ULONG Len )
{
 int rc;

 if( Len == 0 )
  return;

again:
 rc = (* send_)(SocketDescriptor, pBuf, Len, 0);
 if( rc < 0 )
 {
  rc = (* sock_errno_)();
  if( rc == SOCEINTR )
   goto again;

  /***************************************************************************/
  /* - We will get this return code when the socket has been closed.         */
  /* - Just wait for the operating system to kill this thread.               */
  /***************************************************************************/
  if( (rc == SOCENOTSOCK) /* || rc == any other error */ )
  {
   for(;;){ DosSleep(60000); }
  }
 }
}

/*****************************************************************************/
/* SockRecv()                                                                */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Recv bytes.                                                             */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   SocketDescriptor                                                        */
/*   pBuf             -> our transmit buffer.                                */
/*   Len              number of bytes to transmit.                           */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
void SockRecv( int SocketDescriptor, PVOID pBuf, ULONG Len )
{
 int   Bytes;
 int   rc;
 char *cp;

 if( Len == 0 )
  return;

 cp    = (char *)pBuf;

 while( Len > 0 )
 {
  Bytes  = (* recv_)( SocketDescriptor, cp, Len, 0 );
  if( Bytes < 0 )
  {
   rc = (* sock_errno_)();
   if( rc == SOCEINTR )
    continue;

   /**************************************************************************/
   /* - We will get this return code when the socket has been closed.        */
   /**************************************************************************/
   if( (rc == SOCENOTSOCK) /* || rc == any other error */ )
   {
    for(;;){ DosSleep(60000); }
   }
  }
  cp    += Bytes;
  Len   -= Bytes;
 }
 return;
}

/*****************************************************************************/
/* SockClose()                                                               */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Close a socket.                                                         */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   SocketDescriptor                                                        */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
void SockClose( int SocketDescriptor )
{
 int rc;

 if( SocketDescriptor == 0 )
 {
  if( BaseSocketDescriptor == 0 )
   return;
  SocketDescriptor = BaseSocketDescriptor;
  (* setsockopt_)( SocketDescriptor, SOL_SOCKET, SO_REUSEADDR, NULL, 0 );
 }

 rc = (* soclose_)(SocketDescriptor);
 if(rc < 0 )
 {
  (* psock_errno_)("\nclose()");
 }
 if( IsVerbose() ) {printf("\nSocket close rc=%d", rc);}
 if( IsVerbose() ) {printf("  Socket Descriptor=%d", SocketDescriptor);fflush(0);}
}
