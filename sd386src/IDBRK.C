/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   idbrk.c                                                                 */
/*                                                                           */
/* Description:                                                              */
/*   immediate and deferred breakpoint handling.                             */
/*                                                                           */
/*                                                                           */
/* History:                                                                  */
/*                                                                           */
/*   02/08/91 Creation of 32-bit SD86, from 16-bit version.                  */
/*                                                                           */
/*...16->32 port.                                                            */
/*...                                                                        */
/*... 02/08/91  115   Srinivas  port to 32 bit.                              */
/*                                                                           */
/*...Release 1.00 (Pre-release 1)                                            */
/*...                                                                        */
/*... 07/09/91  204   srinivas  Hooking up of deferred breakpoints.          */
/*                                                                           */
/*...Release 1.00 (Pre-release 108 12/05/91)                                 */
/*...                                                                        */
/*... 02/10/92  515   Srinivas  Multiple hits of a deferred break points     */
/*                              (same func names).                           */
/*... 02/12/92  521   Srinivas  Port to C-Set/2.                             */
/*...                                                                        */
/*...Release 1.00 (03/03/92)                                                 */
/*...                                                                        */
/*... 03/20/92  607   Srinivas  CRMA fixes.                                  */
/*... 10/05/92  707   Selwyn    Function entry/address breaks not getting    */
/*...                           marked.                                      */
/*... 12/07/92  805   Selwyn    Function entry breakpoint not being shown.   */
/**Includes*******************************************************************/

#include "all.h"                        /* SD86 include files                */
#include "diabkpt.h"

/**Externs********************************************************************/

extern CmdParms      cmd;
extern uchar         hilite[];
extern uchar         normal[];

/*****************************************************************************/
/* ParseNameAddr()                                                           */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Parse name or address.                                                  */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   addr       output - -> location to stuff address into.                  */
/*   msg        output - -> location where to stuff name or error message.   */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   rc         return code. 0 => ok. 1 => expression error.                 */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
#define OP0  0x00                       /* initial -- no operands            */
#define OP1  0x10                       /* got one operand                   */
#define OP1C 0x18                       /* got one operand and colon         */
#define OP2  0x20                       /* got two operands                  */
#define OPE  0x30                       /* got an error.                     */
#define OPN  0x40                       /* got a name.                       */
                                        /*                                   */
 uint                                   /*                                   */
ParseNameAddr(char *expr, ULONG *addr,uchar *emsg)
{                                       /*                                   */
 uchar *cp;                             /* a char pointer.                   */
 ulong  bigans;                         /* a holder for a long value.        */
 uint   ans;                            /* a holder for an int value.        */
 uint   c;                              /* the current character in the expr.*/
 uint   base;                           /* hex or decimal base.              */
 uchar *sym;                            /* -> to token in expr.              */
 uint   symlen;                         /* length of token in expr.          */
 uint   seg;                            /* selector of addr.                 */
 uint   off;                            /* offset of addr.                   */
 uint   state = OP0;                    /* state.                            */
                                        /*                                   */
 cp = expr;                             /*                                   */
 base = 16;                             /*                                   */
 for( ;; )                              /*  begin token parsing.             */
 {                                      /*  one loop iteration per token.    */
  /***************************************************************************/
  /* At this point, we have an address that should be in sel:off form or a   */
  /* function name. If we have an address, then the first character will     */
  /* be a number and we will proceed to parse off a number. If the first     */
  /* char is an alpha, then we proceed parse off a valid c/plx identifier.   */
  /*                                                                         */
  /* A proper address input consists of three tokens. We return here         */
  /* each time we want to parse one off. A function name consists of only    */
  /* one token. So, we're done when parsing when we hit the null at the      */
  /* end of the string returned from the expr.                               */
  /*                                                                         */
  /* Initially, we're in the OP0 state at the beginning of a token. We may   */
  /* return here in other states.                                            */
  /*                                                                         */
  /***************************************************************************/
  while( (c = *(sym=cp++) ) == ' ' ){;} /*                                   */
  if( c==0 )                            /*                                   */
   break;                               /* DONE! we've hit the end of string.*/
                                        /*                                   */
 ParseNumber:                           /* handle token is a number.         */
                                        /*                                   */
  if( c >= '0' && c <= '9' )            /* fall in if{} if token starts as   */
  {                                     /* a number.                         */
   if( c == '0' && *cp == 'x' )         /* adjust if 0x form of input.       */
    ++cp;                               /*                                   */
                                        /*                                   */
   for( bigans = c - '0' ;; ++cp )      /* begin loop to evaluate constant.  */
   {                                    /*                                   */
    c = *cp - '0';                      /* get value of next digit.          */
    if( c > 9 )                         /* check potential a-f.              */
    {                                   /*                                   */
     c = *cp | 0x20;                    /* convert c to its lower case value.*/
     c = c - 'a' + 10;                  /* calc "real" value of it.          */
     if( c < 10 || c > 15 )             /* if not a-f then we're done with   */
      break;                            /* this number.                      */
    }                                   /*                                   */
    bigans = bigans * base + c;         /* update the value.                 */
   }                                    /*                                   */
   /**************************************************************************/
   /* At this point, we can be in many parsing states. If we are in the OP0  */
   /* state, then we are working on a simple constant or a seg:off constant. */
   /*                                                                        */
   /**************************************************************************/
   if( state == OP0 )                   /*                                   */
    seg = ( uint )(bigans >> 16);       /* anticipate possible seg:off form. */
   ans = ( uint )bigans;                /* hold 16 bit result.               */
   switch( state )                      /*                                   */
   {                                    /*                                   */
    case OP0:                           /* initial state.                    */
     off = ans;                         /*                                   */
     state = OP1;                       /*                                   */
     break;                             /*                                   */
                                        /*                                   */
    case OP1C:                          /* one operand and a ":"             */
     off = ans;                         /*                                   */
     state = OP2;                       /*                                   */
     break;                             /*                                   */
                                        /*                                   */
    default:                            /* invalid states.                   */
     state = OPE;                       /*                                   */
     break;                             /* go process exit.                  */
   }                                    /*                                   */
   if(state == OPE )                    /* if we have an error then abort    */
    break;                              /* the parsing loop.                 */
   continue; /* with NextSymbol */      /* go to next symboland next token.  */
  }                                     /*                                   */

  /***************************************************************************/
  /* The following code parses a valid c or pl/x identifier.                 */
  /***************************************************************************/
  if( IsOKchar( ( uchar )c) )           /* If the leading character is valid,*/
   while( IsOKchar(*cp) )               /* then parse to the end of the      */
   {                                    /* token.                            */
    if(*cp == 0x15 )                    /* maintain italian relations and    */
     *cp = '@';                         /* convert  to @.                   */
    ++cp;                               /*                                   */
   };                                   /*                                   */

  /***************************************************************************/
  /* The followinig code handles operators. In our case, a ':'.              */
  /* sym points to a token that looks like ":xxxx" and cp would point to     */
  /* the first x. c=':' or some invalid operator.                            */
  /*                                                                         */
  /***************************************************************************/
  symlen=cp-sym;                        /* calc symbol length.               */
  if( symlen==1 )                       /*                                   */
  {                                     /*                                   */
   if( c==':' )                         /*                                   */
   {                                    /*                                   */
    if( state == OP1 )                  /*                                   */
    {                                   /*                                   */
     state = OP1C;                      /* advance to OP1C state.            */
     seg = off;                         /* we now have our seg part of       */
     continue; /* with NextSymbol */    /* an address. We'll go on           */
    }                                   /* and get the offset.               */
    state = OPE;                        /* bad news.                         */
    break;                              /*                                   */
   }                                    /*                                   */
   if( !IsOKchar( ( uchar )c) )         /* Is this is not a valid            */
   {                                    /* identifier, then go to the error  */
    state = OPE;                        /* state.                            */
    break;                              /*                                   */
   }                                    /*                                   */
  }                                     /*                                   */
  /***************************************************************************/
  /* At this point, we may have an expression of the form ABxx:ABxx. In the  */
  /* case of the first operand, if *cp = ':', then we say "oops...I need to  */
  /* force this to be parsed as a number". So, we insert a leading 0 and     */
  /* continue. If we get here and the second operand has been parsed off as  */
  /* a symbol, then we trap this by being in the OP1C state.                 */
  /*                                                                         */
  /***************************************************************************/
  if( (state == OP1C || *cp == ':') )   /*                                   */
  {                                     /*                                   */
   c = '0';                             /* force a leading zero              */
   cp = sym;                            /* backup to start of symbol         */
   goto ParseNumber;                    /*                                   */
  }                                     /* parse as a number                 */

  /***************************************************************************/
  /* We have a token that should be a function name.  If we find the name, we*/
  /* will proceed to the OP2 state. If we don't we will break out of the     */
  /* parsing loop and return with . If we don't we return with incorrect sym.*/
  /***************************************************************************/
  if( state == OP0 )                    /*                                   */
  {                                     /*                                   */
   state = OPN;                         /* go to the got name state.         */
   continue; /* with NextSymbol */      /* go get the next token.            */
  }                                     /*                                   */
  break;                                /* break out of the loop, we're      */
                                        /* confused.                         */
 }                                      /*                                   */
 /****************************************************************************/
 /* come here after leaving the token parsing loop.                          */
 /*                                                                          */
 /* state          *sym   rc   addr   emsg                                   */
 /* --------------|------|----|------|-------|------------------------------ */
 /*  OPN          | yes  | 0  | x    |name   |normal name.                   */
 /*  OP2          | no   | 0  | yes  |0      |normal return with address.    */
 /*  OP1,OP2,OP1C | yes  | 1  | x    |vestige|expression error.              */
 /*  OP1,OP2,OP1C | no   | 1  | x    |expr   |expression error.              */
 /*  OPE          | x    | x  | x    |x      |expression error.              */
 /*                                                                          */
 /****************************************************************************/
 *addr= NULL;                           /* assume null address.              */
 if(*sym)                               /* if after all of this and the      */
  state = OPE;                          /* expression has not been consumed  */
                                        /* by the parser, then state = OPE.  */
 switch( state )                        /*                                   */
 {                                      /*                                   */
  case OPN:                             /* found a name.                     */
    strcpy(emsg,expr);                  /* stuff it in a string.             */
    return(0);                          /* return success.                   */
  case OP1:                             /* one operand state.             204*/
   if( state == OP1 && *sym == 0 )      /* if its a simple case of flat   204*/
   {                                    /* address.                       204*/
     *addr = off;                       /* return sucess.                 204*/
     return(0);                         /*                                204*/
   }                                    /*                                204*/
   goto caseop2;
caseop2:

  case OP2:                             /* sucessfully parsed an address.    */
   if( state == OP2 && *sym == 0 )      /*                                   */
   {                                    /*                                   */
    if( TestBit(seg,02) )               /* if its a GDT then it must be   204*/
    {
       *addr = Data_SelOff2Flat((ushort)seg,
                 (ushort)off);          /* simple flat address else       607*/
       if( *addr == NULL )                                              /*827*/
       {                                                                /*827*/
        strcpy(emsg,"Address not loaded.");                             /*827*/
        return(1);                                                      /*827*/
       }
    }
    else                                /* convert into flat address and  204*/
       *addr = off;                     /* assign                         204*/
                                        /*                                204*/
    return(0);                          /*                                   */
   }                                    /*                                   */
   goto caseop0;
caseop0:
  case OP0:                             /* initial state.(null expr)         */
  case OP1C:                            /* one operand state.                */
  case OPE:                             /* one operand and a ":"             */
   if( *sym )                           /* the rest are errors.              */
    strcpy( emsg,sym);                  /*                                   */
   else                                 /*                                   */
    strcpy( emsg,expr);                 /*                                   */
   strcat( emsg," is incorrect");       /*                                   */
   return(1);                           /*                                   */
 }                                      /*                                   */
 return(0);
}                                       /*                                   */
/*****************************************************************************/
/* SetIDBrk()                                                                */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Set an immediate or deferred breakpoints by name or address.            */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   expr      ->   the raw expression.                                      */
/*   DorI           immediate or deferred.                                   */
/*   DefineType     the type of the breakpoint...see brk.h.                  */
/*   ppmsg     ->-> to an error message.                                     */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   rc                                                                      */
/*                                                                           */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*****************************************************************************/
extern UCHAR         IfUscores;         /* TRUE if syms may have uscore prefx*/
extern UCHAR         VideoAtr;
extern AFILE        *allfps;
extern PROCESS_NODE *pnode;

static UCHAR  buffer[ PROMAX + 2];

APIRET SetIDBrk( UCHAR *expr, int DorI, int DefineType, char **ppmsg)
{
 uint     rc;
 BRK     *pbrk;
 ULONG    mid;
 ULONG    lno;
 char    *cp;
 char    *cpp;
 char    *cpq;
 int      len;
 AFILE   *fp;
 BOOL     IsDotQualified;

 ULONG    addr      = NULL;
 char    *pdllname  = NULL;
 char    *pfuncname = NULL;
 DEBFILE *pdf       = NULL;

 /****************************************************************************/
 /* - First, do some syntax checking.                                        */
 /****************************************************************************/
 switch( DefineType )
 {
  case BP_FUNC_NAME:
  {
   /**************************************************************************/
   /* - handle [dllname.]funcname.                                           */
   /* - strip off any path info that may have been included.                 */
   /**************************************************************************/
   cpq            = strrchr(expr, '\\');
   cp = (cpq)?cpq+1:expr;

   IsDotQualified = strchr(expr, '.')?TRUE:FALSE;

   if( IsDotQualified == TRUE )
   {
    cpp = strchr(cp, '.' );

    if( cp == cpp )
    ;
    else
    {
     len      = cpp - cp + 1;
     pdllname = Talloc(len);

     strncpy(pdllname, cp, len - 1 );
    }
    cp = cpp + 1;
   }
   len = strlen(cp);
   if( len == 0 )
   {
    /**************************************************************************/
    /* - there MUST be a function name.                                       */
    /**************************************************************************/
    Error( ERR_BKPT_DEFN_SYNTAX, FALSE, 1, expr );
    return(1);
   }
   pfuncname = Talloc(len + 1);
   strcpy(pfuncname, cp);
  }
  break;

  case BP_DLL_LOAD:
  {
   buffer[0] = ' ';
   rc = ParseNameAddr(expr, &addr, &buffer[1]);
   if( rc != 0 )
   {
    Error( ERR_BKPT_DEFN_SYNTAX, FALSE, 1, expr );
    return(1);
   }
   pdllname = Talloc(strlen(expr) + 1);
   strcpy(pdllname, expr );
  }
  break;

  case BP_ADDR:
  case BP_LOAD_ADDR:
  {
   /**************************************************************************/
   /* - handle an address entry.                                             */
   /**************************************************************************/
   buffer[0] = ' ';
   rc = ParseNameAddr(expr, &addr, &buffer[1]);
   if( (rc != 0) || (addr == NULL) )
   {
    Error( ERR_BKPT_DEFN_SYNTAX, FALSE, 1, expr );
    return(1);
   }
  }
  break;
 }

 /****************************************************************************/
 /* - At this point, we have:                                                */
 /*                                                                          */
 /*   - a [dllname.]funcname                                                 */
 /*   - or, an address                                                       */
 /*                                                                          */
 /****************************************************************************/
 if( DorI == BP_IMMEDIATE )
 {
  /***************************************************************************/
  /* - if we have an immediate breakpoint and the user has specified a dll,  */
  /*   then we need to check to verify that the dll is loaded.               */
  /***************************************************************************/
  if( pdllname )
  {
   pdf = findpdf( pdllname );
   if( pdf == NULL )
   {
    Error( ERR_BKPT_DEFN_DLL, FALSE, 1, pdllname );
    Tfree(pdllname);
    return(1);
   }
  }

  switch( DefineType )
  {
   case BP_FUNC_NAME:
   {
    /*************************************************************************/
    /* - convert the function name to an address.                            */
    /*************************************************************************/
    strcpy( buffer + 1, pfuncname );

    if( pdf != NULL )
    {
     /************************************************************************/
     /* - come here when a dll name was specified.                           */
     /************************************************************************/
     addr   = DBPub(buffer+1,pdf);
     if( (addr == NULL) && IfUscores )
     {
      buffer[0] = '_';
      addr= DBPub(buffer,pdf);
     }
     if( addr == NULL )
     {
      Error( ERR_BKPT_DEFN_FUNCTION, FALSE, 2, pfuncname, pdllname );
      Tfree(pfuncname);
      Tfree(pdllname);
      return(1);
     }
    }
    else
    {
     /************************************************************************/
     /* - come here when a dll name was not specified.                       */
     /************************************************************************/
     pdf  = pnode->ExeStruct;
     addr = NULL;
     for(; pdf ; pdf=pdf->next)
     {
      addr   = DBPub(buffer+1,pdf);
      if( (addr == NULL) && IfUscores )
      {
       buffer[0] = '_';
       addr= DBPub(buffer,pdf);
      }
      if( addr != NULL) break;
     }

     if(addr == NULL)
     {
      if( DoYouWantToDefer() == TRUE )
       DorI = BP_DEFR;
      else
       return(0);
     }
    }
   }
   break;

   case BP_ADDR:
   /**************************************************************************/
   /* - check if the address is loaded.                                      */
   /**************************************************************************/
   {
    pdf = FindExeOrDllWithAddr(addr);
    if( pdf == NULL )
    {
     Error( ERR_BKPT_DEFN_ADDRESS, FALSE, 1, expr );
     return(1);
    }
   }
   break;

  }
 }

 /****************************************************************************/
 /* - now define the breakpoint.                                             */
 /****************************************************************************/
 if( DorI == BP_IMMEDIATE )
 {
  LNOTAB *pLnoTabEntry;
  int     sfi;

  mid = DBMapInstAddr(addr, &pLnoTabEntry, pdf);
  sfi = lno = 0;
  if( pLnoTabEntry )
  {
   lno = pLnoTabEntry->lno;
   sfi = pLnoTabEntry->sfi;
  }
  pbrk                   = DefBrk(addr, TRUE);
  pbrk->mid              = mid;
  pbrk->lno              = lno;
  pbrk->sfi              = sfi;
  pbrk->flag.DorI        = BP_IMMEDIATE;
  pbrk->flag.DefineType  = DefineType;
  pbrk->flag.ActionType  = BRK_SIMP;
  pbrk->dllname          = pdllname;
  pbrk->funcname         = pfuncname;

  /***************************************************************************/
  /* - scan the views and mark this breakpoint if necessary.                 */
  /***************************************************************************/
  for( fp = allfps; mid && fp; fp = fp->next )
    if( fp->mid == pbrk->mid )
      MarkLineBRKs( fp );
 }
 else /* DorI == BP_DEFR */
 {
  /***************************************************************************/
  /* - define deferred breakpoints.                                          */
  /***************************************************************************/
  pbrk                  = DefBrk( NULL, FALSE);
  pbrk->flag.DorI       = BP_DEFR;
  pbrk->flag.DefineType = DefineType;
  pbrk->flag.ActionType = BRK_SIMP;

  switch( DefineType )
  {
   case BP_FUNC_NAME:
   {
    pbrk->dllname  = pdllname;
    pbrk->funcname = pfuncname;
   }
   break;

   case BP_LOAD_ADDR:
   {
    /*************************************************************************/
    /* - add the load address.                                               */
    /*************************************************************************/
    pbrk->brkat = addr;
   }
   break;

   case BP_DLL_LOAD:
   {
    /*************************************************************************/
    /* - attach the dll name.                                                */
    /*************************************************************************/
    pbrk->dllname = pdllname;
   }
   break;
  }

  /***************************************************************************/
  /* - a little post-processing on the breakpoint definition.                */
  /* - put the dll name in lower case.                                       */
  /***************************************************************************/
  if( pbrk && pbrk->dllname )
   strlwr(pbrk->dllname);
 }
 return(0);
}

/*****************************************************************************/
/* ConvertDefBrks()                                                          */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*   Scan bp ring and convert deferred to immediate bps.                     */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   pdf      input  - DEBFILE pointer.                                      */
/*            Insert - flag for breakpoint insertion.                        */
/*                     TRUE  - insert. Used with DLL loads.                  */
/*                   - FALSE - do not insert. Used with EXE loads.           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*  pdf is not null.                                                         */
/*                                                                           */
/*****************************************************************************/
extern uchar        IfUscores;          /* TRUE if syms may have uscore prefx*/

APIRET ConvertDefBrks( PROCESS_NODE *pnode, DEBFILE *pdf, int Insert)
{
 BRK     *pbrk;
 ULONG    mid;
 ULONG    lno;
 uint     addr;
 uchar    sbuffer[256];
 ULONG    junk;
 char    *cp;
 char    *cpp;
 AFILE   *fp;
 char     dllname[13];
 int      len;
 APIRET   ReturnType = 0;


 pbrk = pnode->allbrks;
 for(;pbrk; pbrk=pbrk->next )
 {
  if( pbrk->flag.DorI == BP_IMMEDIATE )
   continue;

  /***************************************************************************/
  /* - massage the dll name for our use.                                     */
  /***************************************************************************/
  cp  = strrchr( pdf->DebFilePtr->fn, '\\' );
  cpp = strchr(  cp, '.');
  len = cpp - cp - 1;
  memset( dllname, 0, sizeof(dllname) );
  strncpy( dllname, cp+1, len);

  /***************************************************************************/
  /* - now handle the breakpoint.                                            */
  /***************************************************************************/
  switch( pbrk->flag.DefineType )
  {
   case BP_LOAD_ADDR:
   {
    DEBFILE *pdfx;

    if(pbrk->flag.Reported == 1 )
    ;
    else
    {
     pdfx = FindExeOrDllWithAddr(pbrk->brkat);
     if( pdfx )
     {
      pbrk->flag.Reported = 1;
      pbrk->mte           = pdf->mte;
      ReturnType = TRAP_ADDR_LOAD;
     }
    }
   }
   break;

   case BP_DLL_LOAD:
   {
    if( pbrk->dllname &&
        (stricmp(dllname, pbrk->dllname) == 0)
      )
    {
     pbrk->flag.Reported    = 1;
     ReturnType = TRAP_DLL_LOAD;
    }
   }
   break;

   case BP_SRC_LNO:
   {
    if( pbrk->dllname == NULL )
    ;
    else
    {
     /************************************************************************/
     /* - if this breakpoint is defined for this dll and the srcname         */
     /*   and lno can be mapped to an address then convert the breakpoint    */
     /*   and define it to esp.                                              */
     /************************************************************************/
     if( stricmp(dllname, pbrk->dllname) == 0 )
     {
      int sfi;

      mid  = MapSourceFileToMidSfi( pbrk->srcname, &sfi, pdf );

      if( (mid != 0 ) &&
          ( (addr = DBMapLno(mid, pbrk->lno, sfi, &junk, pdf) ) != NULL)
        )
      {
       pbrk->brkat            = addr;
       pbrk->mid              = mid;
       pbrk->sfi              = sfi;
       pbrk->flag.DorI        = BP_IMMEDIATE;
       pbrk->flag.WasDeferred = TRUE;
       pbrk->mte              = pdf->mte;
       xDefBrk( pbrk->brkat );
      }
     }
    }
   }
   break;

   case BP_FUNC_NAME:
   {
    /*************************************************************************/
    /* - if the breakpoint definition specifies a dll, then we check         */
    /*   for a dll name match between the breakpoint and the pdf being       */
    /*   loaded.                                                             */
    /*************************************************************************/
    if( pbrk->dllname &&
        (stricmp(dllname, pbrk->dllname) != 0)
      )
     continue;

    /*************************************************************************/
    /* - now check for the existence of the function name in this dll.       */
    /*************************************************************************/
    strcpy(sbuffer+1, pbrk->funcname);

    addr = DBPub(sbuffer+1, pdf);

    if( (addr == NULL) && IfUscores )
    {
     sbuffer[0] = '_';
     addr= DBPub(sbuffer,pdf);
    }

    /*************************************************************************/
    /* - if there is no breakpoint in this dll by this function name,        */
    /*   then go on to the next breakpoint.                                  */
    /* - else, convert to an immediate breakpoint.                           */
    /*************************************************************************/
    if( addr == NULL )
     continue;

    {
     int     sfi;
     LNOTAB *pLnoTabEntry;

     mid = DBMapInstAddr(addr, &pLnoTabEntry, pdf);
     lno = sfi = 0;
     if( pLnoTabEntry )
     {
      lno = pLnoTabEntry->lno;
      sfi = pLnoTabEntry->sfi;
     }

     pbrk->brkat            = addr;
     pbrk->mid              = mid;
     pbrk->lno              = lno;
     pbrk->sfi              = sfi;
     pbrk->flag.DorI        = BP_IMMEDIATE;
     pbrk->flag.WasDeferred = TRUE;
     pbrk->mte              = pdf->mte;
    }

    /*************************************************************************/
    /* - Now, we have created an immediate break for this dll. If a dll      */
    /*   name was not specified, then we need to spin off another deferred   */
    /*   breakpoint to catch any more dlls with this same function name.     */
    /*************************************************************************/
    if( pbrk->dllname == NULL )
    {
     BRK *p;

     p                  = DefBrk( NULL, FALSE);
     p->flag.DorI       = BP_DEFR;
     p->flag.DefineType = BP_FUNC_NAME;
     p->flag.ActionType = BRK_SIMP;
     p->funcname        = Talloc(strlen(pbrk->funcname) + 1 );

     strcpy(p->funcname, pbrk->funcname);
    }

    /*************************************************************************/
    /* - now that we've setup another deferred break, add the dll name to    */
    /*   the breakpoint just converted.                                      */
    /*************************************************************************/
    pbrk->dllname = Talloc( strlen(dllname) + 1);

    strncpy( pbrk->dllname, dllname, len);
    strlwr(pbrk->dllname);

    /*************************************************************************/
    /* - define the breakpoint to the probe.                                 */
    /*************************************************************************/
    xDefBrk( pbrk->brkat );
   }
   break;

   case BP_RESTART:
   {
    /*************************************************************************/
    /* - restart breakpoints get converted to address breakpoints.           */
    /*************************************************************************/
    if( stricmp(dllname, pbrk->dllname) == 0 )
    {
     /************************************************************************/
     /* pbrk->brkat contains only the offset part of the breakpoint          */
     /* address. We have to add the base address of the object               */
     /* containing the breakpoint.                                           */
     /************************************************************************/
     pbrk->brkat           += GetLoadAddr(pdf->mte, pbrk->objnum);
     pbrk->flag.DorI        = BP_IMMEDIATE;
     pbrk->flag.DefineType  = BP_ADDR;

     xDefBrk( pbrk->brkat );
    }
   }
   break;

   default:
    break;
  }

  /***************************************************************************/
  /* - scan the views and mark this breakpoint if necessary.                 */
  /***************************************************************************/
  for( fp = allfps; mid && fp; fp = fp->next )
    if( fp->mid == pbrk->mid )
      MarkLineBRKs( fp );
 }
 return(ReturnType);
}                                       /* end ConvertDefBrks().             */
/*****************************************************************************/
/* SaveBrks()                                                                */
/*                                                                           */
/* Description:                                                              */
/*   Save breakpoints across restart.                                        */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*  pnode      input -> to the process node.                                 */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*  An address in a bp node will always have an associated exe/dll node.     */
/*                                                                           */
/*****************************************************************************/
void SaveBrks( PROCESS_NODE *pnode )
{
 BRK     *pbrk;
 BRK     *pnext;
 DEBFILE *pdf;
 int      len;
 uint     addr;
 ushort   objnum;
 uint     loadaddr = 0 ;

 /****************************************************************************/
 /* - get rid of file breakpoints since they will be resoted from file.      */
 /****************************************************************************/
 pbrk = pnode->allbrks;
 for( ; pbrk; )
 {
  pnext = pbrk->next;
  if( pbrk->flag.File == TRUE )
  {
   UndBrk( pbrk->brkat, TRUE );
  }
  pbrk = pnext;
 }

 pbrk = pnode->allbrks;
 for( ; pbrk; pbrk=pbrk->next )
 {
  /***************************************************************************/
  /* - reset a load address breakpoint back to unreported.                   */
  /***************************************************************************/
  if( pbrk->flag.DefineType == BP_LOAD_ADDR )
  {
   pbrk->flag.Reported = 0;
   pbrk->mte           = 0;
  }

  /***************************************************************************/
  /* - If the breakpoint is deferred, then leave it that way.                */
  /***************************************************************************/
  if( pbrk->flag.DorI == BP_DEFR )
   continue;

  /***************************************************************************/
  /* - If the breakpoint is a load address/dll load then reset it will       */
  /*   simply retain its deferred status. Actually, you'll never get to      */
  /*   this code since it was handled above.                                 */
  /***************************************************************************/
  else if( (pbrk->flag.DefineType == BP_DLL_LOAD) ||
           (pbrk->flag.DefineType == BP_LOAD_ADDR)
         )
  ;

  /***************************************************************************/
  /* - if the debuggee did not DosFreeModule() a dll in which a deferred     */
  /*   breakpoint was hit, then there will be an immediate breakpoint        */
  /*   defined. we need to set this breakpoint back to its deferred          */
  /*   state.                                                                */
  /***************************************************************************/
  else if( pbrk->flag.WasDeferred == TRUE )
  {
   pbrk->flag.DorI        = BP_DEFR;
   pbrk->flag.WasDeferred = FALSE;
   pbrk->brkat            = 0;
   pbrk->mid              = 0;
   pbrk->lno              = 0;
   pbrk->sfi              = 0;
   pbrk->mte              = 0;
  }
  else
  {
   /**************************************************************************/
   /*  - At this point we have an immediate breakpoint that we want to defer */
   /*    for restart.                                                        */
   /**************************************************************************/
   addr = pbrk->brkat;
   pdf  = FindExeOrDllWithAddr(addr);

   if( pbrk->dllname == NULL )
   {
    /*************************************************************************/
    /* - if the dllname is not defined, then define it.                      */
    /*************************************************************************/
    char *cp;
    char *cpp;

    cp            = strrchr( pdf->DebFilePtr->fn, '\\' );
    cpp           = strchr(  cp, '.');
    len           = cpp - cp - 1;
    pbrk->dllname = Talloc(len + 1);

    strncpy( pbrk->dllname, cp+1, len);
    strlwr(pbrk->dllname);
   }

   /**************************************************************************/
   /* Now, we want to convert the selector back to a segment number to be    */
   /* reconverted at load time. We do this beacuse we don't know if the      */
   /* segment number will be loaded into the same selector on restart.       */
   /**************************************************************************/
   objnum = MapAddrtoObjnum(pdf, pbrk->brkat, &loadaddr);
   if (objnum != 0 )
   {
     pbrk->objnum           = objnum;
     pbrk->brkat           -= loadaddr;
     pbrk->flag.DorI        = BP_DEFR;
     pbrk->flag.DefineType  = BP_RESTART;
   }
  }
 }
}

/*****************************************************************************/
/* - put a message into the dialog window.                                   */
/*****************************************************************************/
void  DisplayBkPntChoice( DIALOGSHELL *shell, DIALOGCHOICE *ptr )
{
 char  *pMsgBuf;
 char  *pLine;
 int    i;

 pMsgBuf = GetHelpMsg(HELP_BKPTS_DEFERRED, NULL, 0 );
 pLine   = strtok( pMsgBuf, "\n");
 i       = 0;
 do
 {
  {
   char *cp;
   char *cpend;

   cpend = pLine + strlen(pLine);

   for( cp = pLine; cp <= cpend; cp++ )
   {
    if( *cp == '\r' )
    {
     *cp = ' ';
    }
   }
  }

  putrc( shell->row + shell->SkipLines + i, shell->col + 2, pLine );

  pLine = strtok(NULL, "\n");
  i++;
 }
 while( pLine != NULL );
}                                                                       /*701*/

/*****************************************************************************/
/* - ask the user about deferring the breakpoint.                            */
/*****************************************************************************/
BOOL DoYouWantToDefer( void )
{
 int   i;
 UINT  key;
 CHAR *cp;

 DosBeep( 1000/*Hz*/, 3/*Millisecs*/ );
 VideoAtr = vaHelp;

 GetScrAccess();

 cp = GetHelpMsg(HELP_BKPTS_DEFERRED, NULL, 0 );

 i  = 0;
 while( *cp++ == '\n') i++;

 for( i = 0; *cp ; cp++ )
  if( *cp == '\n' )
   i++;

 Dia_BkPnt.length = i + 5;

 DisplayDialog( &Dia_BkPnt, FALSE );
 key = ProcessYesNoBox( &Dia_BkPnt, &Dia_BkPnt_Choices );
 RemoveDialog( &Dia_BkPnt );
 SetScrAccess();

 if( (key == key_y) || (key == key_Y) || (key == ENTER) )
  return( TRUE );

 return(FALSE);
}
