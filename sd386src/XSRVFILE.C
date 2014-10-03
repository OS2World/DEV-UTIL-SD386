/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   xsrvfile.c                                                           827*/
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*  some x-server file access functions.                                     */
/*                                                                           */
/* History:                                                                  */
/*                                                                           */
/*   06/23/93 Created.                                                       */
/*                                                                           */
/*...                                                                        */
/*... 06/04/93  827   Joe       Add remote debug support.                    */
/*...                                                                        */
/*****************************************************************************/
#include "all.h"

/*****************************************************************************/
/* Talloc                                                                 521*/
/*                                                                           */
/* Description:                                                              */
/*   allocate some memory and clear the area.                                */
/*                                                                           */
/* Parameters:                                                               */
/*   size       size of space required.                                      */
/*                                                                           */
/* Return:                                                                   */
/*   void *                                                                  */
/*                                                                           */
/*****************************************************************************/
  void *
Talloc(UINT size)
{
 void *p = malloc(size);                /* allocate the storage.             */
 if(p)                                  /* clear the storage.                */
  memset(p,0,size);                     /*                                   */
 return(p);                             /* return pointer to storage.        */
}

void *Tfree(void *ptr)
{
 free(ptr);
}
