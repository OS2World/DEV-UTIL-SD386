/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   wps.h                                                                   */
/*                                                                           */
/* Description:                                                              */
/*   Watch point window stuff.                                               */
/*                                                                           */
/*...Release 1.00 (03/03/92)                                                 */
/*...                                                                        */
/*... 03/10/92  602   Srinivas  Hooking up watch points.                     */
/*****************************************************************************/

#define  PROMPTLEN   255                /* expression field length           */
#define  ADDRESSLEN  8                  /* address field length              */
#define  SCOPELEN    6                  /* scope field length                */
#define  TYPELEN     9                  /* type field length                 */
#define  STATUSLEN   6                  /* status field length               */

typedef struct _DEBUG_REGISTER {
  UINT    Address;
  UCHAR   Aligned;
  UCHAR   Size;
  UCHAR   Scope;
  UCHAR   Type;
  UCHAR   Status;
  UINT    Wpindex;
  UCHAR   Prompt[PROMPTLEN + 1];
} DEBUG_REGISTER;

typedef struct _WP_REGISTER {
  UINT    Address;
  UCHAR   Size;
  UCHAR   Scope;
  UCHAR   Type;
  UCHAR   Status;
  UINT    Wpindex;
  INT     IsSet;
} WP_REGISTER;

#define  NODEBUGREGS 4

#define  WPS_LOCAL   0
#define  WPS_GLOBAL  1

#define  READWRITE  0
#define  WRITE      1
#define  EXECUTE    2

#define  DISABLED 0
#define  ENABLED  1
#define  ARMED    2
