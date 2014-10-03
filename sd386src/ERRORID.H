/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   errorid.h                                                               */
/*                                                                           */
/* Description:                                                              */
/*   list of error ids.                                                      */
/*                                                                           */
/* History:                                                                  */
/*                                                                           */
/*   10/18/93 Created.                                                       */
/*                                                                           */
/*...                                                                        */
/*****************************************************************************/

#define  ERR_BAD_INVOCATION_OPTION     500
#define  ERR_NO_EXE                    501
#define  ERR_COM_PORT                  502
#define  ERR_CANT_OPEN_MODEM           503
#define  ERR_ESP_NOT_READY             504
#define  ERR_CANT_GET_MODEM_ATTENTION  505
#define  ERR_CONNECT_TIMEOUT           506

/*****************************************************************************/
/* User Profile errors                                                       */
/*****************************************************************************/
#define  ERR_USER_PROFILE              507
#define  ERR_PROFILE_READERROR         508
#define  ERR_PROFILE_VERSION           509
#define  ERR_PROFILE_KEY_DEFS          510
#define  ERR_PROFILE_INVALID_FUNCTION  511
#define  ERR_PROFILE_INVALID_ATTRIBUTE 512
#define  ERR_PROFILE_BG_COLOR          513
#define  ERR_PROFILE_FG_COLOR          514
#define  ERR_PROFILE_EXCEPTION_TYPE    515
#define  ERR_PROFILE_EXCEPTION         516
#define  ERR_PROFILE_INVALID_KEY       517
#define  ERR_PROFILE_NESTED_COMMENT    518
#define  ERR_PROFILE_GENERAL           519

/*****************************************************************************/
/* Other errors                                                              */
/*****************************************************************************/
#define  ERR_FILE_READ                 520
#define  ERR_FILE_CANT_FIND_EXE        521
#define  ERR_FILE_CHECK_DLL        /* removed */
#define  ERR_DOSSTARTSESSION           523
#define  ERR_DBG_INIT                  524
#define  ERR_EXE_INIT                  525
#define  ERR_EXE_OPEN              /* removed */
#define  ERR_EXE_NOIMAGE           /* removed */
#define  ERR_EXE_ENTRY                 528
#define  ERR_DLL_TOO_MANY              529
#define  ERR_BAD_DEBUG_INFO            530
#define  ERR_VIEW_CANT_BUILD           531
#define  ERR_COM_PORT_INVALID          532
#define  ERR_COM_PORT_OPEN             533
#define  ERR_COM_PORT_PARMS            534
#define  ERR_DATA_INVALID_STRUCT       535
#define  ERR_CANT_REGISTER_HANDLER     536
#define  ERR_CANT_START_QUE            537
#define  ERR_TRAP_INTERLOCK            538
#define  ERR_RESPONSE_FILE             539
#define  ERR_BAD_CONNECT               540
#define  ERR_ESP_QUE_ERROR             541
#define  ERR_DBG_QUE_ERROR             ERR_ESP_QUE_ERROR
#define  ERR_NB_INADEQUATE_RESOURCES   542
#define  ERR_NB_CANT_LOAD_DLL          543
#define  ERR_PM_APP_TYPE               544
#define  ERR_AT_COMMAND                545
#define  ERR_PHONE_NUMBER              546
#define  ERR_DOSDEBUG_INIT             547
#define  ERR_BKPT_FILE_OPEN            548
#define  ERR_BKPT_FILE_COMMENT         549
#define  ERR_BKPT_FILE_SYNTAX_BRACE    550
#define  ERR_BKPT_FILE_SYNTAX_DLLEXE   551
#define  ERR_BKPT_FILE_SYNTAX_FUNC     552
#define  ERR_BKPT_FILE_SYNTAX_LNO      553
#define  ERR_BKPT_FILE_SYNTAX_NOFUNC   554
#define  ERR_BKPT_FILE_SYNTAX_NOFILE   555
#define  ERR_BKPT_FILE_SYNTAX_COND     556
#define  ERR_BKPT_FILE_ENV_SD386BRK    557
#define  ERR_BKPT_FILE_WRITE           558
#define  ERR_BKPT_FILE_ENV_EDITOR      559
#define  ERR_BKPT_FILE_EDITOR_OPEN     560
#define  ERR_BKPT_FILE_EDITOR_START    561
#define  ERR_BKPT_FILE_EDITOR_PMTYPE   562
#define  ERR_BKPT_DEFN_DLL             563
#define  ERR_BKPT_DEFN_FUNCTION        564
#define  ERR_BKPT_DEFN_SYNTAX          565
#define  ERR_BKPT_DEFN_ADDRESS         566
#define  ERR_BKPT_DEFN_LINENO          571
#define  ERR_MSH_LOAD_MODULE           567
#define  ERR_MSH_PROC_ADDR             568
#define  ERR_MSH_LOOP_FILE             569
#define  ERR_MSH_LOOP_START            570
#define  ERR_TCPIP_CANT_LOAD_DLL       572
#define  ERR_TCPIP_NOT_RUNNING         573
#define  ERR_TCPIP_ERROR               574
#define  ERR_TCPIP_NO_SERVICES_PORT    575
#define  ERR_TCPIP_NO_HOST_NAME        576
#define  ERR_TCPIP_ESP_NOT_STARTED     577

