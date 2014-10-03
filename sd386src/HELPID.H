/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   HelpId.h                                                                */
/*                                                                           */
/* Description:                                                              */
/*   list of helpids                                                         */
/*                                                                           */
/* History:                                                                  */
/*                                                                           */
/*   02/08/91 Creation of 32-bit SD86, from 16-bit version.                  */
/*                                                                           */
/*...Release 1.00 (03/03/92)                                                 */
/*...                                                                        */
/*... 03/10/92  602   Srinivas  Hooking up watch points.                     */
/*... 09/28/92  701   Michelle  Add general help, other helps.               */
/*... 10/05/92  701   Michelle  Add window helps.                            */
/*... 10/15/92  701   Michelle  Add keys.                                    */
/*****************************************************************************/

/*
 * Association of mnemonic "key" names with message numbers
 * used to extract online help messages from the OS/2 message
 * file (SD386.TXT in ASCII form, SD386.MSG in binary form).
 */

#define MAXHELPMSG 3000  /* max # of bytes for help message (w/cr and lf) */
#define HELPFILLCHAR "~"

#define  MSGFILE "SD386.MSG"
#define  MAX_SUBSTRINGS        4

/*****************************************************************************/
/* Invocation help id.                                                       */
/*****************************************************************************/
#define  HELP_INVOCATION_SD386        99
#define  HELP_INVOCATION_ESP         100

/*****************************************************************************/
/* Help ids for action bar choices, pulldowns and dialogs.                   */
/*****************************************************************************/

#define  HELP_ABAR_FILE              101
#define  HELP_PULL_GETFUNC           102
#define  HELP_PULL_GETADDR           104
#define  HELP_PULL_GETFILE           106
#define  HELP_PULL_FINDFUNC          108
#define  HELP_PULL_FINDXLINE         109
#define  HELP_PULL_NEXTFILE          110
#define  HELP_PULL_DROPFILE          111
#define  HELP_PULL_BROWSE            112
#define  HELP_PULL_RESTART           114
#define  HELP_PULL_QUIT              115

#define  HELP_ABAR_RUN               116
#define  HELP_PULL_RUN               117
#define  HELP_PULL_SWAP              118
#define  HELP_PULL_NOSWAP            119
#define  HELP_PULL_SSTEP             120
#define  HELP_PULL_STEPINTO          121
#define  HELP_PULL_RUNCURSOR         122
#define  HELP_PULL_RUNNOSWP          123
#define  HELP_PULL_SSTEPNOSWP        124
#define  HELP_PULL_STEPINTONOSWP     125
#define  HELP_PULL_RUNCURSORNOSWP    126
#define  HELP_PULL_SETXLINE          127

#define  HELP_ABAR_BREAKPOINTS       128
#define  HELP_PULL_SETCLRBKP         129
#define  HELP_PULL_SETCONDBKP        130
#define  HELP_PULL_FUNCENTRY         132
#define  HELP_PULL_ADDRESS           134
#define  HELP_PULL_DEFERRED          251
#define  HELP_PULL_ADDRLOAD          253
#define  HELP_PULL_DLLLOAD           254
#define  HELP_PULL_CLRBKPS           136
#define  HELP_PULL_HARDWAREREGS      137
#define  HELP_PULL_DATABREAK         138

#define  HELP_ABAR_SEARCH            139
#define  HELP_PULL_FIND              140
#define  HELP_PULL_REPEATFIND        141

#define  HELP_ABAR_DATA              142
#define  HELP_PULL_SHOWVAR           143
#define  HELP_PULL_SHOWVARPTR        144
#define  HELP_PULL_WATCHVAR          145
#define  HELP_PULL_WATCHVARPTR       146
#define  HELP_PULL_EXPANDVAR         147
#define  HELP_PULL_EDITEXP           148
#define  HELP_PULL_EDITSTORAGE       149
#define  HELP_PULL_FORMATVAR         150
#define  HELP_PULL_REMOVEVAR         151
#define  HELP_PULL_INSERT            152

#define  HELP_ABAR_VIEW              153
#define  HELP_PULL_SOURCE            154
#define  HELP_PULL_ASSEMBLER         155
#define  HELP_PULL_DATA              156
#define  HELP_PULL_DATASHOW          157
#define  HELP_PULL_SHOWAPP           158

#define  HELP_ABAR_SETTINGS          159
#define  HELP_PULL_COLORS            160
#define  HELP_PULL_EXCEPTIONS        161
#define  HELP_PULL_ASSEMBLYSOURCE    162
#define  HELP_PULL_MNEMONICS         163
#define  HELP_PULL_MNEM_AL86         208
#define  HELP_PULL_MNEM_INTEL        209

#define  HELP_ABAR_MISC              164
#define  HELP_PULL_CALLSTACK         165
#define  HELP_PULL_THREADS           166
#define  HELP_PULL_DLLS              167
#define  HELP_PULL_REGS              168
#define  HELP_PULL_EDITREGS          169
#define  HELP_PULL_COPROCREGS        170

#define  HELP_ABAR_HELP              171
#define  HELP_PULL_GENHELP           207
#define  HELP_PULL_KEYS              172

#define  HELP_DLG_GETFUNC            103
#define  HELP_DLG_GETADDR            105
#define  HELP_DLG_GETFILE            107
#define  HELP_DLG_BROWSE             113
#define  HELP_DLG_FIND               174
#define  HELP_DLG_SETCONDBKP         131
#define  HELP_DLG_FUNCENTRY          133
#define  HELP_DLG_ADDRESS            135
#define  HELP_DLG_DEFERRED           252
#define  HELP_DLG_ADDRLOAD           255
#define  HELP_DLG_DLLLOAD            256
#define  HELP_DLG_GETTYPE            192

#define  HELP_DLG_COLORS             173
#define  HELP_DLG_THREADS            175
#define  HELP_DLG_EXCEPTIONS         176
#define  HELP_DLG_WATCH              177
#define  HELP_DLG_CALLSTACK          178
#define  HELP_DLG_EDITREGS           206
#define  HELP_DLG_KEYSHELP           213

#define  HELP_FORMULA                191
#define  HELP_TYPE                   192

#define  HELP_ABAR_ABAR              212

/*****************************************************************************/
/* Help ids for windows.                                                     */
/*****************************************************************************/

#define  HELP_WIN_BROWSE             181
#define  HELP_WIN_DATA               210
#define  HELP_WIN_ASM                211

/* Help ids for format variable object pulldown */

#define  HELP_PULL_ANY               183
#define  HELP_PULL_HEX               184
#define  HELP_PULL_CHAR              185
#define  HELP_PULL_INT               186
#define  HELP_PULL_LONG              187
#define  HELP_PULL_UCHAR             214
#define  HELP_PULL_UINT              215
#define  HELP_PULL_ULONG             216
#define  HELP_PULL_FLOAT             188
#define  HELP_PULL_DOUBLE            189
#define  HELP_PULL_LDOUBLE           217
#define  HELP_PULL_MENU              190

/* Help ids for storage modification messages */

#define  HELP_DATA_CHAR              193
#define  HELP_DATA_UCHAR             194
#define  HELP_DATA_INT               195
#define  HELP_DATA_UINT              196
#define  HELP_DATA_LONG              197
#define  HELP_DATA_ULONG             198
#define  HELP_DATA_FLOAT             199
#define  HELP_DATA_DOUBLE            200
#define  HELP_DATA_LDOUBLE           200
#define  HELP_DATA_SEGMENT           201
#define  HELP_DATA_OFFSET            202
#define  HELP_DATA_HEX               203
#define  HELP_DATA_ASCII             204

#define  HELP_REGS_FLAGS             205
#define  HELP_QUIT_ESP               218
#define  HELP_DLG_GETLINENUMBER      219
#define  HELP_CTRL_BREAK             220

#define  HELP_DLG_PROCESSES          221

#define  HELP_PULL_MSHGET            241
#define  HELP_PULL_MSHPUT            242
#define  HELP_PULL_MSH               243
#define  HELP_PULL_SAVEBKPS          244
#define  HELP_PULL_RESTOREBKPS       245
#define  HELP_PULL_EDITBKPS          246


#define  HELP_BKPTS_SYNTAX           247
#define  HELP_BKPTS_SAVE_MSG         248
#define  HELP_BKPTS_RESTORE_MSG      249
#define  HELP_BKPTS_DEFERRED         250

#define  HELP_PULL_BROWSEMSH         251
#define  HELP_PULL_PROCESSES         257
#define  HELP_PULL_TOGGLEMEMFNCS     258
