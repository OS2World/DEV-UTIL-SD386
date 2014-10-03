/*****************************************************************************/
/* - Netbios supports a primary and an alternae adapter. For now, we         */
/*   will only support the primary adapter.                                  */
/*****************************************************************************/
#define  PRIMARY_ADAPTER   0

/*****************************************************************************/
/* - some defines specifing whether we want to request resources from        */
/*   or release resources to the netbios pool.                               */
/*****************************************************************************/
#define  REQUEST           0
#define  RELEASE           1

/*****************************************************************************/
/* - some defines specifying whether we want to make a call to another       */
/*   machine or answer a call from another machine.                          */
/*****************************************************************************/
#define  NETBIOS_CALL      0
#define  NETBIOS_ANSWER    1

/*****************************************************************************/
/* - defines for some likely netbios errors.                                 */
/*****************************************************************************/
#define INADEQUATE_SESSIONS  1
#define INADEQUATE_COMMANDS  2
#define INADEQUATE_NAMES     3
#define CANT_LOAD_NETB_DLL   4

/*****************************************************************************/
/* - logical name headers for the different ends of a connection. These      */
/*   will be appended by a user defined name taken from the /n option.       */
/*****************************************************************************/
#define MAX_LSN_NAME 16
#define LSN_RES_NAME 3                  /* num chars reserved by debugger.   */

#define LSN_DBG    "Sd3"
#define LSN_ESP    "Esp"

typedef struct network_control_block  NCB;
typedef struct ncb_reset              NCB_RESET;
typedef struct ncb_status_information NCB_STATUS_INFO;
