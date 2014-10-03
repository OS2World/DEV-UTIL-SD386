/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   kbd.h                                                                   */
/*                                                                           */
/* Description:                                                              */
/*   definitions for the keyboard.                                           */
/*                                                                           */
/* History:                                                                  */
/*                                                                           */
/*   02/08/91 Creation of 32-bit SD86, from 16-bit version.                  */
/*                                                                           */
/*...Release 1.01 (04/03/92)                                                 */
/*...                                                                        */
/*... 05/08/92  701   Srinivas  Cua Interface.                               */
/**Includes*******************************************************************/

#define UP         0x4800               /* up arrow                          */
#define C_UP       0x8D00               /* Ctrl up arrow                     */
#define DOWN       0x5000               /* down arrow                        */
#define C_DOWN     0x9100               /* Ctrl down arrow                   */

#define LEFT       0x4B00               /* left arrow                        */
#define C_LEFT     0x4B00               /* Ctrl left arrow                   */
#define RIGHT      0x4D00               /* right arrow                       */
#define C_RIGHT    0x4D00               /* Ctrl right arrow                  */
#define BACKSPACE  0x0E08               /* backspace (rubout)                */

#define TAB        0x0F09               /* tab                               */
#define S_TAB      0x0F00               /* Shift tab                         */

#define HOME       0x4700               /* home                              */
#define C_HOME     0x7700               /* Ctrl home                         */
#define END        0x4F00               /* end                               */
#define C_END      0x7500               /* Ctrl end                          */
#define PGUP       0x4900               /* page up                           */
#define C_PGUP     0x8400               /* Ctrl page up                      */
#define PGDN       0x5100               /* page down                         */
#define C_PGDN     0x7600               /* Ctrl page down                    */

#define SPACEBAR   0x3920               /* spacebar                          */
#define C_SPACEBAR 0xCC20               /* spacebar (Ctrl shft gnd by GetKey)*/
#define A_SPACEBAR 0xAA20               /* spacebar (Alt shft gend by GetKey)*/
#define S_SPACEBAR 0xBB20               /* spacebar (Shift + spacebar)    808*/
#define ENTER      0x1C0D               /* enter                             */
#define C_ENTER    0x1C0A               /* Ctrl enter                        */
#define A_ENTER    0x1C00               /* Alt enter                         */
#define S_ENTER    0x1C0D               /* Shift enter                    808*/

#define INS        0x5200               /* insert                            */
#define C_INS      0x9200               /* Ctrl insert                       */
#define S_INS      0x5201               /* shift insert                      */
#define A_INS      0xA200               /* Alt   insert                   808*/
#define DEL        0x5300               /* delete                            */
#define C_DEL      0x9300               /* Ctrl delete                       */
#define S_DEL      0x9301               /* shift delete                      */
#define A_DEL      0xA300               /* Alt   delete                   808*/
#define ESC        0x011B               /* escape                            */

#define PADPLUS    0x4E2B               /* numeric keypad plus               */
#define PADMINUS   0x4A2D               /* numeric keypad minus              */
#define PADENTER   0x000D               /* numeric keypad entry              */

#define F1         0x3B00               /* PF1                               */
#define F2         0x3C00               /* PF2                               */
#define F3         0x3D00               /* PF3                               */
#define F4         0x3E00               /* PF4                               */
#define F5         0x3F00               /* PF5                               */
#define F6         0x4000               /* PF6                               */
#define F7         0x4100               /* PF7                               */
#define F8         0x4200               /* PF8                               */
#define F9         0x4300               /* PF9                               */
#define F10        0x4400               /* PF10                              */
#define F11        0x8500               /* PF11                              */
#define F12        0x8600               /* PF12                              */

#define C_F1       0x5E00               /* Ctrl PF1                          */
#define C_F2       0x5F00               /* Ctrl PF2                          */
#define C_F3       0x6000               /* Ctrl PF3                          */
#define C_F4       0x6100               /* Ctrl PF4                          */
#define C_F5       0x6200               /* Ctrl PF5                          */
#define C_F6       0x6300               /* Ctrl PF6                          */
#define C_F7       0x6400               /* Ctrl PF7                          */
#define C_F8       0x6500               /* Ctrl PF8                          */
#define C_F9       0x6600               /* Ctrl PF9                          */
#define C_F10      0x6700               /* Ctrl PF10                         */
#define C_F11      0x8900               /* Ctrl PF11                         */
#define C_F12      0x8A00               /* Ctrl PF12                         */

#define A_F1       0x6800               /* Alt PF1                           */
#define A_F2       0x6900               /* Alt PF2                           */
#define A_F3       0x6A00               /* Alt PF3                           */
#define A_F4       0x6B00               /* Alt PF4                           */
#define A_F5       0x6C00               /* Alt PF5                           */
#define A_F6       0x6D00               /* Alt PF6                           */
#define A_F7       0x6E00               /* Alt PF7                           */
#define A_F8       0x6F00               /* Alt PF8                           */
#define A_F9       0x7000               /* Alt PF9                           */
#define A_F10      0x7100               /* Alt PF10                          */
#define A_F11      0x8B00               /* Alt PF11                          */
#define A_F12      0x8C00               /* Alt PF12                          */

#define A_F        0x2100               /* Alt F                             */
#define A_R        0x1300               /* Alt R                             */
#define A_B        0x3000               /* Alt B                             */
#define A_E        0x1200               /* Alt E                             */
#define A_S        0x1F00               /* Alt S                             */
#define A_D        0x2000               /* Alt D                             */
#define A_W        0x1100               /* Alt W                             */
#define A_V        0x2F00               /* Alt V                             */
#define A_O        0x1800               /* Alt O                             */
#define A_M        0x3200               /* Alt M                             */
#define A_H        0x2300               /* Alt H                             */

#define S_F1       0x5400               /* Shift PF1                         */
#define S_F2       0x5500               /* Shift PF2                         */
#define S_F3       0x5600               /* Shift PF3                         */
#define S_F4       0x5700               /* Shift PF4                         */
#define S_F5       0x5800               /* Shift PF5                         */
#define S_F6       0x5900               /* Shift PF6                         */
#define S_F7       0x5A00               /* Shift PF7                         */
#define S_F8       0x5B00               /* Shift PF8                         */
#define S_F9       0x5C00               /* Shift PF9                         */
#define S_F10      0x5D00               /* Shift PF10                        */
#define S_F11      0x8700               /* Shift PF11                        */
#define S_F12      0x8800               /* Shift PF12                        */

#define SLASH      0x352f               /* Slash (/)                         */
#define key_R      0x1352               /* R                                 */
#define key_F      0x2146               /* F                                 */
#define key_f      0x2166               /* f                                 */
#define key_r      0x1372               /* r                                 */
#define key_D      0x2044               /* D                                 */
#define key_d      0x2064               /* d                                 */
#define key_S      0x1F53               /* S                                 */
#define key_s      0x1F73               /* s                                 */
#define key_T      0x1454               /* T                                 */
#define key_t      0x1474               /* t                                 */
#define key_H      0x2348               /* H                                 */
#define key_h      0x2368               /* h                                 */
#define key_A      0x1E41               /* A                                 */
#define key_a      0x1E61               /* a                                 */
#define key_N      0x314E               /* N                                 */
#define key_n      0x316E               /* n                                 */
#define key_y      0x1579               /* y                                 */
#define key_Y      0x1559               /* Y                                 */
#define key_0      0x0b30               /* 0                                 */
#define key_1      0x0231               /* 1                                 */
#define key_2      0x0332               /* 2                                 */
#define key_3      0x0433               /* 3                                 */
#define key_4      0x0534               /* 4                                 */
#define key_5      0x0635               /* 5                                 */
#define key_6      0x0736               /* 6                                 */
#define key_7      0x0837               /* 7                                 */
#define key_8      0x0938               /* 8                                 */
#define key_9      0x0a39               /* 9                                 */

#define MOUSECLICK 0xFFFF               /* indicates a mouse click out side  */
                                        /* the current context.           701*/
#define MOUSEPICK  0xFFFE               /* indicates a mouse click on a sel  */
                                        /* ectable item in the window.    701*/
#define SCROLLBAR  0xFFFD               /* indicates a mouse click on scroll */
                                        /* bar.                           701*/
#define SCROLLUP   0xFFFC               /* indicates a mouse click on scroll */
                                        /* bar up arrow.                  701*/
#define SCROLLDOWN 0xFFFB               /* indicates a mouse click on scroll */
                                        /* bar down arrow.                701*/
#define BGNEXT     0xFFFA               /* indicates a mouse click on back   */
                                        /* ground next color button       701*/
#define FGNEXT     0xFFF9               /* indicates a mouse click on fore   */
                                        /* ground next color button       701*/
#define TYNEXT     0xFFF8               /* indicates a mouse click on Type   */
                                        /* nxt button in watchpoints win  701*/
#define SPNEXT     0xFFF7               /* indicates a mouse click on Scope  */
                                        /* nxt button in watchpoints win  701*/
#define SZNEXT     0xFFF6               /* indicates a mouse click on Size   */
                                        /* nxt button in watchpoints win  701*/
#define STNEXT     0xFFF4               /* indicates a mouse click on Status */
                                        /* nxt button in watchpoints win  701*/

#define DATAKEY    0x00FF               /* AUTOEXIT data key (ret by keystr) */
#define HEXKEYFLD  3                    /* hex field type for keystr         */
#define BINKEYFLD  9                    /* bin field type for keystr         */

/*****************************************************************************/
/*                         CURSOR CONTROL                                    */
/*****************************************************************************/
typedef struct _csr{
  UCHAR col;
  UCHAR row;
  UINT  mode;
} CSR;

#define CSR_NORMAL 0
#define CSR_INSERT 1
#define CSR_HIDDEN 2
