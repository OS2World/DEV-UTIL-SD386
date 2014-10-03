/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   keystr.h                                                                */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Flag definitions etc. for GetString().                                  */
/*                                                                           */
/* History:                                                                  */
/*                                                                           */
/*   12/10/93 Created                                                        */
/*                                                                           */
/*...                                                                        */
/*... 12/10/93  910   Joe   Clear fields on cursor sensitive prompting.      */
/*...                                                                        */
/*****************************************************************************/

#define AUTOEXIT 0x0001      /* exit field on left or right or data char */
#define HEXONLY  0x0002      /* only data chars 0..9, A..F, and a..f */
#define BINONLY  0x0008      /* only data chars 0 or 1                       */
#define CLEAR1ST 0x0004      /* clear the field on first character entry  910*/
                             /* when doing cursor sensitive prompting.    910*/
