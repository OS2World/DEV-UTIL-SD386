#include "all.h"
char *MemStats(int isw) ;
/*****************************************************************************/
/* Talloc.c                                                               521*/
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
void *Talloc(UINT size)
{
 void *p;

#ifdef __MYMALLOC

//MemStats(0);
 p = (char *)MyMalloc(size,SD386_TEMP_CHAIN);
 MemStats(0);

#else

 p = malloc(size);
 if(p)
  memset(p,0,size);

#endif

 return(p);
}

void *T2alloc(UINT size, int chain)
{
 void *p;

#ifdef __MYMALLOC

// MemStats(0);
 p = (char *)MyMalloc(size, chain);
 MemStats(0);

#else

 p = Talloc(size);

#endif

 return(p);
}

/*****************************************************************************/
/*                                                                           */
/*****************************************************************************/
void *Tfree(void *ptr)
{
#ifdef __MYMALLOC

// MemStats(0);
 MyFree(ptr);
 MemStats(0);

#else

 free(ptr);

#endif
}

/*****************************************************************************/
/* - this function has no counterpart for a compilation that is not using    */
/*   __MYMALLOC. It has been put here to resolve the call to FreeChain       */
/*   in cpydata.c.                                                           */
/*****************************************************************************/
char *FreeChain(int chain)
{
#ifdef __MYMALLOC
 return MyMalloc(0,-chain);
#endif
}

#ifdef __MYMALLOC

char *memnames[]={
    "PERM_CHAIN",
    "TEMP_CHAIN",
    "CPY_CHAIN",
    "BROWSEM_CHAIN",
    "WINDOW_CHAIN"
    ""
    };

typedef struct memheader;
typedef struct memheader *pmemheader;

typedef struct {
        char *prev_ptr;
        char *next_ptr;
        int chain;
        int n;
        char marker[8];
} memheader;

static char *last_ptr = NULL;
char *ptr, *prev, *next;

int   GetChain(char *);

char *MyMalloc(int n, int chain)
{
/*Memory is allocated in chains. A chain can be freed with a single call.*/
/* n>=0                                                     */
/* chain >0 => allocate n bytes to chain and return pointer.*/
/* chain <0 => free -chain.                                 */
/* chain =0 => free (char *) n                              */
/* n<0                                                      */
/* chain >0 => move ptrs from chain -n to chain.            */

    if(n>=0) {
        if (chain > 0) {
           ptr = (char *) malloc(sizeof(memheader)+n+8);
           if(!ptr) {
               fprintf(stderr,"MyMalloc: %d %d\n",n,chain);
               exit(1);
           }
           ((memheader *) ptr)->prev_ptr = last_ptr;
           ((memheader *) ptr)->next_ptr = NULL;
           if (last_ptr) ((memheader *) last_ptr)->next_ptr = ptr;
           ((memheader *) ptr)->chain = chain;
           ((memheader *) ptr)->n = n;
           last_ptr = ptr;
           strncpy(((memheader *)ptr)->marker,"mbegin12",8);
           ptr += sizeof(memheader);
           strncpy(ptr+n,"mend1234",8);
           memset(ptr,'\0',n);
           return ptr;
        }
        else if (chain < 0) {
           chain  = -chain;
           ptr = last_ptr;
           while (ptr) {
              if (((memheader *) ptr)->chain == chain) {
                 next = ((memheader *) ptr)->next_ptr;
                 prev = ((memheader *) ptr)->prev_ptr;
                 if(prev)((memheader *) prev)->next_ptr = next;
                 if(next)((memheader *) next)->prev_ptr = prev;
                 if(ptr == last_ptr) last_ptr = prev;
                 /*Try to force problems if a block is accessed after it is */
                 /*freed by destroying contents.*/
                 memset(ptr,0xAA,((memheader *)ptr)->n+sizeof(memheader)+8);
                 free(ptr);
                 ptr = prev;
              }
              else {
                 ptr = prev = ((memheader *) ptr)->prev_ptr;
              }
           }
           return NULL;
        }
        else {
           char *delete_ptr;
           int idel=0;
           if(!n) return NULL;
           delete_ptr=(char *)(n-sizeof(memheader));
           ptr = last_ptr;
           while (ptr) {
              if (ptr==delete_ptr) {
                 next = ((memheader *) ptr)->next_ptr;
                 prev = ((memheader *) ptr)->prev_ptr;
                 if(prev)((memheader *) prev)->next_ptr = next;
                 if(next)((memheader *) next)->prev_ptr = prev;
                 if(ptr == last_ptr) last_ptr = prev;
                 /*Try to force problems if a block is accessed after it is */
                 /*freed by destroying contents.*/
                 memset(ptr,0xAA,((memheader *)ptr)->n+sizeof(memheader)+8);
                 free(ptr);
                 ptr = prev;
                 idel=1;
              }
              else {
                 ptr = prev = ((memheader *) ptr)->prev_ptr;
              }
           }
           if(!idel) {
               fprintf(stderr,"MyMalloc: deletion not performed.\006\n");
               return  (char *) 1;
           }
        }
    }
    else if(n<0) { /*move everything from -n to chain*/
       n  = -n;
       ptr = last_ptr;
       while (ptr) {
          if (((memheader *) ptr)->chain == n) {
              ((memheader *) ptr)->chain = chain;
          }
          ptr = ((memheader *) ptr)->prev_ptr;
       }
       return NULL;
   }
   return (char *) 2;
    /* Internal memory management error if both conditions fail */
    /* Messages ?? */
}

char *MyStralloc(char *string, int chain) {
    char *ptr=MyMalloc(strlen(string)+1,chain);
    if(ptr) strcpy(ptr,string);
    return ptr;
}

char *MyFree(char *ptr) {
    return (ptr?MyMalloc((int)ptr,0):NULL);
}



int CheckPtr(char *ptr, int mode) {
    int idel=0, ierror=0;
    char *delete_ptr=ptr-sizeof(memheader);

/*Verify that ptr can be found by traversing the chain.*/
    if(mode & 1) {
        ptr = last_ptr;
        while (ptr) {
           if (ptr==delete_ptr) {
              idel=1;
              break;
           }
           else {
              ptr = ((memheader *) ptr)->prev_ptr;
           }
        }
        if(!idel) {
            fprintf(stderr,"CheckPtr: ptr not found.\006\n");
            ierror|=1;
        }
    }
    if(mode & 2) {
        if(strncmp(((memheader *)ptr)->marker,"mbegin12",8)) {
            fprintf(stderr,"CheckPtr: ptr failed heading string check.\006\n");
            ierror|=2;
        }
        if(strncmp(ptr+((memheader *)ptr)->n+sizeof(memheader),"mend1234",8)) {
            fprintf(stderr,"CheckPtr: ptr failed trailing string check.\006\n");
            ierror|=2;
        }
    }
    if(mode & 4) {
       if(*((int *)(ptr-4))!=(((memheader *)ptr)->n)+sizeof(memheader)+8) {
           ierror|=4;
       }
    }
    return ierror;
}

char *MyRealloc(char *ptr, int n, int chain) {
    int oldn;
    char *next, *prev, *new_ptr;
    if(CheckPtr(ptr,4|2|1))
        return (char *) 1;
    if(chain && GetChain(ptr)!=chain)
        return (char *) 2;
    ptr-=sizeof(memheader);
    next = ((memheader *) ptr)->next_ptr;
    prev = ((memheader *) ptr)->prev_ptr;
    oldn=((memheader *)ptr)->n;
    if(oldn>n) {
        memset(ptr+sizeof(memheader)+n+8,0xAA,oldn-n);
    }
    new_ptr=realloc(ptr,n+sizeof(memheader)+8);
    if(!new_ptr) return new_ptr;
    ((memheader *)new_ptr)->n=n;
    strncpy(ptr+sizeof(memheader)+n,"mend1234",8);
    if(oldn<n) {
        memset(new_ptr+sizeof(memheader)+oldn,0x00,n-oldn);
    }
    if(ptr==new_ptr) {
        if(!CheckPtr(new_ptr+sizeof(memheader),4|2|1)) {
            return new_ptr+sizeof(memheader);
        }
        else {
            return NULL;
        }
    }

    /*Fixup  linked list.*/
    ((memheader *) new_ptr)->prev_ptr = prev;
    ((memheader *) new_ptr)->next_ptr = next;
    if(prev) {
        ((memheader *) prev)->next_ptr = new_ptr;
    }
    if(next) {
        ((memheader *) next)->prev_ptr = new_ptr;
    }
    if(ptr == last_ptr) last_ptr = new_ptr;
    if(!CheckPtr(new_ptr+sizeof(memheader),4|2|1)) {
        return new_ptr+sizeof(memheader);
    }
    else {
        return NULL;
    }
}

/*These next two might as well be macros.*/
int GetChain(char *ptr) {
    return((memheader *)(ptr-sizeof(memheader)))->chain;
}

int GetN(char *ptr) {
    return((memheader *)(ptr-sizeof(memheader)))->n;
}

char *MyCalloc(int m, int n, int chain) {
    return MyMalloc(m*n,chain);
}

size_t MaxStorage (void)
  {
    return 10000000;
    /*This doesn't work.*/

#if 0
{
    size_t iMinBytes, iNumBytes, iMaxBytes;
    char *cDummy;

}
    iMaxBytes = iMinBytes = 0;
    iMaxBytes --;
    iNumBytes = iMaxBytes / 2;

    do
      {

        cDummy = (char *)malloc(iNumBytes);
        if (cDummy == NULL)
          {
            iMaxBytes = iNumBytes;
            iNumBytes = (iMaxBytes - iMinBytes) / 2 + iMinBytes;
          }
        else
          {
            free(cDummy);
            iMinBytes = iNumBytes;
            iNumBytes = (iMaxBytes - iMinBytes) / 2 + iMinBytes;
          }

      }
    while (iMinBytes + 1 < iMaxBytes);

    fflush(stdout);
    return iNumBytes-sizeof(memheader);
#endif
  } /* end of MaxStorage */

char *MemStats(int isw) {
    int i, min, max, chain, n;
    struct {
        int num;
        int size;
    } memstats[1000];
    for(i=0;i<1000;++i) {
        memstats[i].num=0;
        memstats[i].size=0;
    }
    max=min=((memheader *)last_ptr)->chain;

    ptr=last_ptr;
    while (ptr) {
       chain=((memheader *)ptr)->chain;
       n=((memheader *)ptr)->n;
       if(min>((memheader *)ptr)->chain) {
           min=((memheader *)ptr)->chain;
       }
       if(max<((memheader *)ptr)->chain) {
           max=((memheader *)ptr)->chain;
       }
       if(CheckPtr(ptr,4|2)) {
           return(ptr);
       }
       ++memstats[chain].num;
       memstats[chain].size+=n;
       ptr = prev = ((memheader *) ptr)->prev_ptr;
   }
   if(isw) return NULL;
   printf("MemStats:%5s %5s       %4s %s\n","Chain","Count","Size","Chain Name");
   for(i=min;i<=max;++i) {
       if(memstats[i].num) {
           printf("MemStats:%5d %5d %10d %s\n",
           i,memstats[i].num,memstats[i].size,memnames[i]);
       }
   }
   return NULL;
}

#endif
