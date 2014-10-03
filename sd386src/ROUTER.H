/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   router.h                                                                */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Include file for router.c                                               */
/*                                                                           */
/* History:                                                                  */
/*                                                                           */
/*   07/11/94 Created                                                        */
/*                                                                           */
/*...                                                                        */
/*... 05/05/94  919   Joe   Add child process support.                       */
/*...                                                                        */
/*****************************************************************************/

/*****************************************************************************/
/* - The connection may be a serial connection or a parallel connection.     */
/*****************************************************************************/
#define SERIAL     1
#define PARALLEL   2

/*****************************************************************************/
/* - connection type definitions.                                            */
/*****************************************************************************/
#define BOUND      1
#define ASYNC      2
#define LOCAL_PIPE 3
#define _NETBIOS   4                    /* uscore used to prevent conflicts. */
#define SOCKET     5

/*****************************************************************************/
/* - client/server definitions                                               */
/*****************************************************************************/
#define _DBG       0                    /* uscore used to prevent conflicts. */
#define _ESP       1                    /* uscore used to prevent conflicts. */

/*****************************************************************************/
/* - this structure contains the definition of the dbg/esp connection.       */
/*****************************************************************************/
typedef struct
{
 int        ConnectType   :3;           /* Either bound, local pipe, async,  */
                                        /* or, netbios.                      */
 int        DbgOrEsp      :1;           /* Which end of the connection.      */
 int        ComPort       :3;           /* async port number.                */
 int        modem         :1;           /* modem is being used.              */
 int        BitRate         ;           /* async bit speed.                  */
 char      *pModemFile      ;           /* file of modem commands.           */
 char      *pLsnName        ;           /* netbios logical adapter name.     */
}CONNECTION;

/*****************************************************************************/
/* - this tells the router to use his default connection handle.             */
/*****************************************************************************/
#define DEFAULT_HANDLE  (LHANDLE)-1
