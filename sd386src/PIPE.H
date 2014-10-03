/*****************************************************************************/
/* File:                                                                     */
/*   pipe.h                                                               919*/
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Header file for the client/server named pipe communications.            */
/*                                                                           */
/* History:                                                                  */
/*                                                                           */
/*   06/10/94 Created                                                        */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/* By convention, we will designate "Dbg" as the server side of a pipe       */
/* connection. Esp will be treated as the client side of the connection.     */
/*****************************************************************************/
#define DBG_SERVER 0
#define ESP_CLIENT 1

/*****************************************************************************/
/* This structure is copied here from the toolkit and the szName field       */
/* modified. In the toolkit, this field has a length of 1 and here it's      */
/* been modified to handle the maximum length of a pipe name.                */
/*****************************************************************************/
typedef struct _PIPESTATE               /* nmpinf */
{
   USHORT cbOut;                        /* length of outgoing I/O buffer     */
   USHORT cbIn;                         /* length of incoming I/O buffer     */
   BYTE   cbMaxInst;                    /* maximum number of instances       */
   BYTE   cbCurInst;                    /* current number of instances       */
   BYTE   cbName;                       /* length of pipe name               */
   CHAR   szName[CCHMAXPATH];           /* start of name                     */
} PIPESTATE;
