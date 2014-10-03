/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   go.h                                                                    */
/*                                                                           */
/* Description:                                                              */
/*   Definations for interface with the GO procedure                         */
/*                                                                           */
/* History:                                                                  */
/*                                                                           */
/*   02/08/91 Creation of 32-bit SD86, from 16-bit version.                  */
/*                                                                           */
/*...Release 1.00                                                            */
/*...                                                                        */
/*... 02/08/91  100   made changes for 32-bit compilation.                   */
/*... 10/09/91  246   Srinivas  proper message if not able resolve exp.      */
/*...                                                                        */
/*...Release 1.00 (03/03/92)                                                 */
/*...                                                                        */
/*... 03/10/92  602   Srinivas  Hooking up watch points.                     */
/*****************************************************************************/

#define GODATA  6                       /* Go until data break condition true*/
#define GOTID   7                       /* switch focus to new another thread*/

#define OVERCALL 0                      /*                                827*/
#define INTOCALL 1                      /*                                827*/

#define LOAD_MODULES      0x0001        /* handle dll loads.              827*/
#define RUNNING_DEFERRED  0x0002        /* return on dll notify.          827*/

#define TRAP_SS         1               /* Single step interrupt             */
#define TRAP_NMI        2               /* Non-maskable interrupt            */
#define TRAP_BPT        3               /* Breakpoint interrupt (3)          */
#define TRAP_ASYNC      4               /* Breakpoint interrupt (3)          */
#define TRAP_FP        16               /* Floating point exception          */
#define TRAP_DATA      92               /* Data Breakpoint condition true    */
#define TRAP_ENTR      93               /* 1st procedure entry               */
#define TRAP_SIG       94               /* Signal waiting for proces         */
#define TRAP_KEYB      95               /* Interrupt from keyboard           */
#define TRAP_ENDT      96               /* Thread has ended                  */
#define TRAP_CALL      97               /* Fault on missing procedure        */
#define TRAP_NEW       98               /* Application starting              */
#define TRAP_EXIT      99               /* Application exit                  */
#define TRAP_GP        100              /* General protection exception      */
#define TRAP_IDIV      101              /* Interrupt Divide overflow         */
#define TRAP_IOVR      102              /* Interrupt Overflow                */
#define TRAP_IBND      103              /* Interrupt Bound                   */
#define TRAP_IOPC      104              /* Interrupt Invalid opcode          */
#define TRAP_IPXA      105              /* Intrpt Processor extenon not avail*/
#define TRAP_IPXE      106              /* Intrpt Processor extension error  */
#define TRAP_ISYS      107              /* Interrupted System Call           */
#define TRAP_ABEND     108              /* Ptrace abnormal return code       */
#define TRAP_INTERLOCK 109              /* Resource interlock.            901*/
#define TRAP_SS_ERROR  110              /* Resource interlock.            901*/
#define TRAP_STMT      999              /* Should never happen...            */
#define TRAP_PROC      111              /* Error trying to start thread      */
#define TRAP_INIT      200              /* initialization has been done   236*/
#define TRAP_EXP       0xFFFFFFFF       /* exception resolving abnormal   246*/

#define TRAP_ADDR_LOAD 112              /* Address load break.               */
#define TRAP_DLL_LOAD  113              /* Dll load break.                   */
                                        /* return code.                   246*/
#define TRAP_WATCH 250                  /* watch point hit                602*/

/* WARNING -- The following MUST match the definitions in SYMTAB.H */

#define STUBTYPEMASK 0x7

#define PRIVSTUB 0                      /* PrivStub                          */
#define PS2EXIT 10                      /* # bytes from start to exiting JMP */

#define CODESTUB 1                      /* CodeStub                          */
#define FAULTSTUB 0x10                  /* Code for module not loaded        */
#define DEBUGSTUB 0x40
