/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   Boxmenu.h                                                               */
/*                                                                           */
/* Description:                                                              */
/*   structure passed to the boxmenu procedure.                              */
/*                                                                           */
/* History:                                                                  */
/*                                                                           */
/*   02/08/91 Creation of 32-bit SD86, from 16-bit version.                  */
/*                                                                           */
/*...Release 1.00                                                            */
/*...                                                                        */
/*... 02/08/91  100   made changes for 32-bit compilation.                   */
/*...                                                                        */
/*****************************************************************************/

typedef struct {
  ULONG helpid;
  uint  item;
  ulong rowmask;
  ulong selectmask;
} BoxMenuData;

/*****************************************************************************/
/* The mask fields indicate which rows are menu items, and which of those    */
/* rows are currently selectable.  The 2**N bit of the mask corresponds to   */
/* the Nth row of the box.  The 2**0 bit corresponds to the top row.         */
/*****************************************************************************/
