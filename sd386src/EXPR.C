/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   expr.c                                                                  */
/*                                                                           */
/* Description:                                                              */
/*   Expression handling routines.                                           */
/*                                                                           */
/*                                                                           */
/* History:                                                                  */
/*                                                                           */
/*   02/08/91 Creation of 32-bit SD86, from 16-bit version.                  */
/*                                                                           */
/*...16->32 port.                                                            */
/*...                                                                        */
/*... 02/08/91  101   Joe       port to 32 bit.                              */
/*... 02/08/91  110   Srinivas  port to 32 bit.                              */
/*... 02/08/91  112   Joe       port to 32 bit.                              */
/*                                                                           */
/*...Release 1.00 (Pre-release 1)                                            */
/*...                                                                        */
/*... 07/09/91  205   srinivas  Hooking up of register variables.            */
/*... 07/18/91  211   Christina need to call findsvar when there is a func   */
/*...                           above main & there is a global variable      */
/*... 07/26/91  219   srinivas  handling near pointers.                      */
/*... 08/05/91  221   srinivas  Hooking up registers and constants in data   */
/*                              window.                                      */
/*... 09/25/91  240   Srinivas  recursive PLX based variables.               */
/*... 09/25/91  243   Srinivas  ISADDREXPR global flag problems.             */
/*...                                                                        */
/*...Release 1.00 (Pre-release 1.05 10/10/91)                                */
/*...                                                                        */
/*... 10/16/91  303   Srinivas  Trap if register constants are entered in    */
/*                              data window when you are in assembly view    */
/*                    --------> Fix Removed taken care by fix No: 533.       */
/*... 11/07/91  312   Srinivas  In data window SS:ESP+a gives wrong results. */
/*...                                                                        */
/*...Release 1.00 (Pre-release 1.08 12/05/91)                                */
/*...                                                                        */
/*... 12/20/91  501   Srinivas  If flat address is specifed in data window   */
/*...                           some times it gives bogus results.           */
/*... 02/10/92  513   Srinivas  Allow array indexs in datawindow to be       */
/*...                           variables.                                   */
/*... 02/12/92  521   Srinivas  Port to C-Set/2.                             */
/*...                                                                        */
/*...Release 1.00 (03/03/92)                                                 */
/*...                                                                        */
/*... 03/20/92  607   Srinivas  CRMA fixes.                                  */
/*...                                                                        */
/*...Release 1.02 (10/22/92)                                                 */
/*...                                                                        */
/*... 01/26/93  809   Selwyn    HLL Level 2 support.                         */
/*... 03/03/93  813   Joe       Revised types handling for HL03.             */
/*... 09/16/93  901   Joe       Add code to handle resource interlock error. */
/*... 10/05/93  904   Joe       Fix for unable to display x[i] in the data   */
/*...                           window.                                      */
/*                                                                           */
/**Includes*******************************************************************/

#include "all.h"                        /* SD86 include files                */
static UINT ParseMshExpr=0;

/**Defines *******************************************************************/

#define NREGS16 16                      /* # of registers in 16 bit world 221*/
#define NREGS32 10                      /* # of registers in 32 bit world 221*/
                                        /* Parse States                      */
#define OP0  0x00                       /* initial -- no operands            */
#define OP0P 0x01                       /* got operand(s) and plus           */
#define OP0M 0x02                       /* got operand(s) and minus          */
#define OP0T 0x04                       /* got operand(s) and times          */

#define OP1  0x10                       /* got one operand                   */
#define OP1P 0x11                       /* got one operand and plus          */
#define OP1M 0x12                       /* got one operand and minus         */
#define OP1T 0x14                       /* got one operand and times         */
#define OP1C 0x18                       /* got one operand and colon         */

#define OP2  0x20                       /* got two operands                  */
#define OP2P 0x21                       /* got two operands and plus         */
#define OP2M 0x22                       /* got two operands and minus        */
#define OP2T 0x24                       /* got two operands and times        */

#define OP3  0x40                       /* got two operands and '.'or '->'   */

#define OP4  0x80                       /* mask for OP4L and OP4R            */
#define OP4L 0x81                       /* got two operands and '['          */
#define OP4R 0x82                       /* got two operands and '[number' ???*/
                                        /*                      'number]'    */
#define DONTCARE 0                      /*                                513*/
#define NTYPES   8                      /*                                513*/
/**External declararions******************************************************/

extern PtraceBuffer AppPTB;             /* Application Ptrace buffer.        */
extern uint         DataMid;            /* Set by DBData -- Module ID.       */
extern uint         DataTid;            /* Set by DBData -- Type ID.         */
extern SSProc      *Qproc;              /* Set by Qsymbol() to the SSProc 809*/
                                        /*                  and findsvar.    */
extern USHORT       dtab[];             /*                                513*/

UCHAR  BadExprMsg[] = "Incorrect expression";
UINT   ExprMid;                         /* Set by ParseExpr.                 */
UINT   ExprTid;                         /* Set by ParseExpr and findlvar     */
UCHAR *ParseError;                      /* Set by ParseExpr if error message */
UINT   ExprAddr;                        /* Set by ParseExpr.                 */
UINT   ExprLno;                         /* Set by ParseExpr.                 */
SCOPE  ExprScope;                       /* Set by ParseExpr and findlvar.    */
long   CmplxBkptVal;                    /*                                   */
UCHAR  IfUscores = TRUE;                /* TRUE if syms may have uscore prefx*/
UCHAR  IsAddrExpr;                      /* Set by ParseExpr if address       */
                                        /* yeilding expression.              */


/**Static definitions ********************************************************/

static  UCHAR ErrMsgBuf[40];            /* local message buffer.             */
static  UCHAR RegNames32[] = "eax ecx edx ebx esp ebp esi edi efl eip ";
static  UCHAR RegNames16[] = "axcxdxbxspbpsidiflipcsdsesfsgsss";

/*****************************************************************************/
/* ParseExpr()                                                               */
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*  Expression Evaluation Routine                                            */
/*                                                                           */
/*  cp = ParseExpr( expr, context, optmid, optline )                         */
/*                                                                           */
/*     uint Context;                                                         */
/*          Context & 0x0F => 0 = normal, 1 = constant, 2 = no names         */
/*          Context & 0x10 => "optmid" and "optline" parms are specified     */
/*          Context & 0x20 => do not use local names in the expression       */
/*                                                                           */
/*  expr =    &token<op>token<op>token<op>...........                        */
/*         **..                                                              */
/*                                                                           */
/*            token=hex or decimal constant, or a                            */
/*                  register, or a                                           */
/*                  symbol.                                                  */
/*                                                                           */
/*  Evaluation proceeds L->R.                                                */
/*  Expressions using the ":" operator have a default base of hex.           */
/*                                                                           */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   cp       input - pointer to the text expression.                        */
/*   context  input - see above.                                             */
/*   optmid   input - optional module id.                                    */
/*   optline  input - optional line.                                         */
/*   optsfi   input - optional source file index.                            */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   sym      Pointer into expression where we encountered a parsing error.  */
/*            If this points to a NULL, then parsing proceeded ok.           */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   none.                                                                   */
/*                                                                           */
/*****************************************************************************/
uchar * ParseExpr( uchar *cp,
                   uint context,
                   uint optmid,
                   uint optline,
                   int  optsfi  )
                                        /* pointer to text string            */
                                        /* see above                         */
                                        /* only if context & 0x10. module id.*/
                                        /* only if context & 0x10. src line#.*/
{                                       /*                                   */
 uint   mid;                            /* a module id.                      */
 ulong  bigans;                         /* a long representation of a const. */
 uint   ans;                            /* a uint representation of a const. */
 uint   c;                              /* the current character in the expr.*/
 uint   state;                          /* initial state.                    */
 uint   base;                           /* hex or decimal base.              */
 uint   exprbase;                       /* working base for an expression.   */
 uint   progline;                       /* the program source line with var. */
 uint   SymRegIndex;                    /* the register index.               */
 uint   deref;                          /* flag for "*" operator.            */
 uint   addrof;                         /* flag for "&" operator.            */
 uint   fldoff;                         /* offset of variable in structure.  */
 uchar *sym;                            /* -> to token in the expr string.   */
 uint   symlen;                         /* length of token in string.        */
 uint   symloc;                         /* -> to a symbol location.       112*/
 uint   seg;                            /* the selector of symloc.           */
 uint   off;                            /* the offset of symloc.             */
 uchar  buffer[MAXNAMELEN];             /* buffer for a symbol name.      813*/
 uint   NoLocalNames;                   /* flag for using/not using locals.  */
 uint   temp;                           /*                                221*/
 uchar  Regname[4];                     /* temp array to hold reg name    221*/
 uint   IsSymRegName;                   /* flag to know if a sym is a reg 221*/
 uint   TempExprTid;                    /* temp var to hold exprtid       219*/
 DEBFILE     *pdf;                                                      /*901*/
 int          progsfi;
 UCHAR       *cppsym;
 BOOL         ThisProcessing = FALSE;

startallover:
 off   = 0;
 state = OP0;
/*****************************************************************************/
/* First, we initialize some global values.                                  */
/*                                                                           */
/*****************************************************************************/
 ExprAddr   = NULL;                     /*                                112*/
 ExprScope  = NULL;                     /*                                   */
 ExprMid    = 0;                        /*                                   */
 ExprTid    = 0;                        /*                                   */
 ExprLno    = 0;                        /*                                   */
 IsAddrExpr = TRUE;                     /*                                243*/
 ParseError = NULL;                     /*                                   */
                                        /*                                   */
 deref = addrof = base = seg = 0;       /*                                   */
 IsSymRegName = 0;                      /*                                221*/
 exprbase = 10;                         /*                                   */
                                        /*                                   */
/*****************************************************************************/
/* If we are parsing from the executing context, then we will get the mid    */
/* and source line from the currently executing afile and ExecLine. Else     */
/* we use the passed values.                                                 */
/*                                                                           */
/*****************************************************************************/
#ifdef MSH
 if(cp[0]=='!' && ParseMshExpr ) {
    int iret;
    ParseMshExpr = 0; /*Control recursion.*/
    iret=mshsd386command(cp+1);
    /*iret = 1: expression evaluates to non-zero; */
    /*       0: expression evaluates to zero;     */
    /*      -1: syntax error.                     */
    /*      -2: semantic error.                   */
    /*      -3: exit                              */
    /*       8: REXX semantic error               */
    /*      10: REXX return code zero             */
    /*      11: REXX return code non-zero         */
    return (uchar *)(10+iret);
 }
 ParseMshExpr = 0; /*Control recursion.*/
#endif

 if( context & 0x10 )                   /* use optmid and optline if proper  */
 {                                      /* context.                          */
  mid = optmid;                         /*                                   */
  progline = optline;                   /*                                   */
  progsfi  = optsfi;                    /*                                   */
 }                                      /*                                   */
 else                                   /* else                              */
 {                                      /*                                   */
  AFILE *execfp;

  execfp = GetExecfp();
  mid = (execfp ? execfp->mid : 0);     /* get mid from the executing afile. */
  progline = GetExecLno();              /* use executing line                */
 }                                      /*                                   */
                                        /*                                   */
 NoLocalNames = ((context & 0x20) ?     /* set a flag if we are excluding    */
                  TRUE : FALSE);        /* local names.                      */
                                        /*                                   */
 context &= 0x0F;                       /* precipitate normal=0,1=constant,  */
                                        /* or 2=no names context.            */
                                        /*                                   */
/*****************************************************************************/
/* At this point, we are in the OP0 state and will begin to parse off and    */
/* process tokens. We will stay in the token parsing loop until all tokens   */
/* are parsed.                                                               */
/*                                                                           */
/*****************************************************************************/

 for( ;; )                              /*  begin token parsing.             */
 {                                      /*  one loop iteration per token.    */
/*****************************************************************************/
/* At this point we want to establish the start of a token or the end of     */
/* parsing. When c==NULL, then parsing is complete. At the completion of the */
/* loop c will be the first token char and cp will point to the character    */
/* following c.                                                              */
/*                                                                           */
/*****************************************************************************/
  for( ;; ++cp )                        /* scan string skipping spaces.      */
  {                                     /*                                   */
   sym = cp;                            /* save the beginning of the token.  */
   if( (c = *cp) > 0x20 )               /* if we hit a non-space then we     */
   {                                    /*                                   */
    ++cp;                               /* are at the start of a token and   */
    break;                              /* can exit the loop.                */
   }                                    /*                                   */
   if( !c )                             /* if we hit end-of-string then      */
    goto fini;                          /* go home.                          */
  }                                     /*                                   */
/*****************************************************************************/
/* At this point, we handle tokens that are constants.  The first thing we   */
/* want to do is establish the base.  If we have the c hex constant          */
/* designation "0x...", then the base is hex.  Otherwise we use decimal.     */
/*                                                                           */
/*****************************************************************************/
 ParseNumber:                           /* handle token is a number.         */
                                        /*                                   */
  if( c >= '0' && c <= '9' )            /* if we have a decimal digit, or    */
  {                                     /*                                   */
   if( c == '0' && ((*cp == 'x') ||     /* if the second digit is x or    501*/
                    (*cp == 'X')))      /* if the second digit is X, then 501*/
   {                                    /*                                   */
    base = 16;                          /* we assume hex base.               */
    ++cp;                               /* bump cp past the "x" part of token*/
   }else                                /*                                   */
    base = exprbase;                    /* use default base = decimal        */

/*****************************************************************************/
/* We now execute a loop to evaluate the constant.                           */
/*                                                                           */
/* bigans = bigans*base + c(current digit value).                            */
/*                                                                           */
/* Upper case hex digits are folded to lower case.                           */
/*                                                                           */
/* Loop exits at "break" statement when we encounter a non-digit.            */
/*                                                                           */
/* Also, we might begin evaluating with base=10 for a number that looks like */
/* 19FC. If we do, then we change the base to hex and re-evaluate.           */
/*                                                                           */
/*****************************************************************************/
                                        /*                                   */
 ReparseNumber:                         /*                                   */
                                        /*                                   */
   for( bigans = c - '0' ;; ++cp )      /* begin loop to evaluate constant.  */
   {                                    /*                                   */
    c = *cp - '0';                      /* get value of next digit.          */
    if( c > 9 )                         /* if this is a hex digit a-f, then  */
    {                                   /*                                   */
     c = *cp | 0x20;                    /* convert c to its lower case value.*/
     c = c - 'a' + 10;                  /* calc "real" value of it.          */
     if( c>=10 && c<=15 )               /* if value is in hex a-f range, then*/
     {                                  /*                                   */
      if( base == 10 )                  /* if we are in a decimal base,      */
      {                                 /*                                   */
       base = 16;                       /* change the base to hex ,          */
       c = '0';                         /* force a leading zero and          */
       cp = sym;                        /* backup to start of symbol and     */
       goto ReparseNumber;              /* restart parsing with right base.  */
      }                                 /*                                   */
     }else                              /* c is not a valid decimal or hex   */
      break;                            /* digit, so we have hit the end of  */
                                        /* the token.                        */
    }                                   /*                                   */
    bigans = bigans * base + c;         /* update the value.                 */
   }                                    /* end loop to evaluate constant     */
                                        /*                                   */
/*****************************************************************************/
/* At this point, we can be in many parsing states. If we are in the OP0     */
/* state, then we are working on a simple constant or a seg:off constant.    */
/*                                                                           */
/*****************************************************************************/
   if( state == OP0 )                   /*                                   */
    seg = ( uint )(bigans >> 16);       /* anticipate possible seg:off form. */
   ans = ( uint )bigans;                /* hold 16 bit result.               */
   goto GotOperand;                     /* go to next state and next token.  */
  }                                     /*                                   */
                                        /*                                   */
/*****************************************************************************/
/* If we know by definition, context==1, that this token is a constant, then */
/* getting to this point is a nono.                                          */
/*                                                                           */
/*****************************************************************************/
  if( context == 1 )                    /* if constant by definition, then   */
  {                                     /*                                   */
   ParseError="Constant required";      /* it's bye bye time.                */
   goto bummer;                         /*                                   */
  }                                     /*                                   */
/*****************************************************************************/
/* The following code parses a valid c identifier.                           */
/*                                                                           */
/*****************************************************************************/
  if( IsOKchar( ( uchar )c) )           /* If the leading character is valid,*/
   while( IsOKchar(*cp) )               /* then parse to the end of the      */
   {                                    /* token.                            */
    if(*cp == 0x15 )                    /* maintain italian relations and    */
     *cp = '@';                         /* convert  to @.                   */
    ++cp;                               /*                                   */
   };                                   /*                                   */
                                        /*                                   */

/*****************************************************************************/
/* At this point, if we have an operator or a register, we will fall into    */
/* one of these two cases of a switch statement. Otherwise, we will fall     */
/* through.                                                                  */
/*                                                                           */
/* symlen==1 => operator. Valid operators are + - * [ ] -> . & :             */
/* symlen==2 => register.                                                    */
/*                                                                           */
/*****************************************************************************/
  symlen = cp - sym;                    /*                                521*/
  switch( symlen )                      /* begin switch on symlen.           */
  {
    case 1:
      /***********************************************************************/
      /* This is the case of an operator, symlen==1. Single character        */
      /* symbol names will fall through.                                     */
      /***********************************************************************/

      switch( c )                       /* begin switch on operator          */
      {                                 /*                                   */
       case '+':                        /*                                   */
         /********************************************************************/
         /* The + operator takes states:                                     */
         /*                                                                  */
         /* OP0->OP0P                                                        */
         /* OP1->OP1P                                                        */
         /* OP2->OP2P                                                        */
         /*                                                                  */
         /* All other entry states are invalid.                              */
         /*                                                                  */
         /********************************************************************/
         if( (state & 0x0F) == 0 )
         {
          state |= OP0P;
          goto NextSymbol;
         }

         ParseError="Invalid use of \"+\"";
         goto bummer;

       case '-':
         /********************************************************************/
         /* We may have a -> operator at this point. We are working          */
         /* on an expression of the form "ptr->something". The "var"         */
         /* part will have been parsed off and seg:off=&ptr. The             */
         /* following code will resolve the "ptr" part.                      */
         /*                                                                  */
         /* This is only valid from the OP2 state.                           */
         /*                                                                  */
         /********************************************************************/
         if( (state == OP2) &&
             (*cp == '>')
           )
         {
          TempExprTid = ExprTid;                                        /*219*/
          /*******************************************************************/
          /* save the ExprTid in a temp var TempExprTid and pass TempExpr 219*/
          /* Tid to QderefType to dereference the type no. This is being  219*/
          /* done since exprtid is being used in DerefPointer to determine219*/
          /* type of pointer.                                             219*/
          /*******************************************************************/
          if( !QderefType(ExprMid, (USHORT *)&TempExprTid) )            /*219*/
          {
           ParseError = "Invalid type for \"->\"";
           goto bummer;
          }

          symloc=DerefPointer( off,ExprMid );                    /*240*//*112*/
          if( symloc == NULL )          /*                                205*/
          {
           if( ThisProcessing == FALSE )
           {
            ParseError = "Invalid pointer for \"->\"";
            goto bummer;
           }
           else
           {
            context = 0x20;
            cp += 1;
            goto startallover;
           }
          }

          ExprTid = TempExprTid;        /* refresh exprtid to correct val 219*/

/*        seg = SelOf(symloc);     */   /* selector of deref'd pointer.      */
          off = OffOf(symloc);          /* offset of deref'd pointer.        */
          cp += 1;                      /* bump one for ">".                 */
          state = OP3;                  /* move to OP3 state.                */
          goto NextSymbol;              /* now go get the next token.        */
         }                              /*                                   */

         /********************************************************************/
         /* The - operator takes states:                                     */
         /*                                                                  */
         /* OP0->OP0M                                                        */
         /* OP1->OP1M                                                        */
         /* OP2->OP2M                                                        */
         /*                                                                  */
         /* All other entry states are invalid.                              */
         /*                                                                  */
         /********************************************************************/
         if( (state & 0x0F) == 0 )
         {
          state |= OP0M;
          goto NextSymbol;
         }

         ParseError = "Invalid use of \"-\"";
         goto bummer;

       case '*':
         /********************************************************************/
         /* The * treated as "contents of" is valid in the OP0 state.        */
         /*                                                                  */
         /********************************************************************/
         if( state == OP0 &&
             !addrof
           )
          {
           deref += 1;
           goto NextSymbol;
          }

         /********************************************************************/
         /* The * operator as times takes states                             */
         /*                                                                  */
         /* OP0->OP0T                                                        */
         /* OP1->OP1T                                                        */
         /* OP2->OP2T                                                        */
         /*                                                                  */
         /* All other entry states are invalid.                              */
         /*                                                                  */
         /********************************************************************/

         if( (state & 0x0F) == 0 )
         {
          state |= OP0T;
          goto NextSymbol;
         }

         ParseError = "Invalid use of \"*\"";
         goto bummer;


       case '&':
         /********************************************************************/
         /* The & operator will set the addrof flag in the OP0 state if it   */
         /* has not already been set, or deref has not been set by a previous*/
         /* "*" interpreted as "contents of."                                */
         /*                                                                  */
         /********************************************************************/
         if( state == OP0 &&
             !(addrof|deref)
           )
         {
          addrof = 1;
          goto NextSymbol;
         }

         ParseError = "Invalid use of \"&\"";
         goto bummer;

       case ':':
         /********************************************************************/
         /* At this point, we are working on an expression of the form       */
         /* selector:offset.  We get here after parsing off a selector value */
         /* in the OP0 state.  By default, expressions of this form are      */
         /* considered hex base.  If the operand before the colon was parsed */
         /* as decimal, then we will convert to hex.                         */
         /*                                                                  */
         /********************************************************************/
         if( state == OP1 )
         {
          state = OP1C;
          ExprTid = 0;                 /* reset the expression type.      243*/
          seg = off;
          exprbase = 16;

          if( base == 10 )
          {
           for( c = ans = 0;
                seg;
                seg /= 10, c += 4
              )
            ans |= ((seg % 10) << c);
           seg = ans;
          }
          goto NextSymbol;
         }

         ParseError = "Invalid use of \":\"";
         goto bummer;

       case '.':
         /********************************************************************/
         /* The "." operator takes us to the OP3 state. The OP3 state        */
         /* handles expressions of the form:                                 */
         /*                                                                  */
         /*   ptr->var                                                       */
         /*   struct.member                                                  */
         /*                                                                  */
         /********************************************************************/
         if( state == OP2
           )
         {
          state = OP3;
          goto NextSymbol;
         }

         ParseError = "Invalid use of \".\"";
         goto bummer;

       case '[':
         /********************************************************************/
         /* The "[" operator takes us to the OP4L state.  The OP4L state     */
         /* tells us that we are parsing the index of an array.              */
         /*                                                                  */
         /********************************************************************/
         if( state == OP2 )
         {
          state = OP4L;
          goto NextSymbol;
         }

         ParseError = "Invalid use of \"[\"";
         goto bummer;

       case ']':
         /********************************************************************/
         /* The "]" operator takes us to the OP2 state.  You can get to the  */
         /* OP4R state only from the OP4L state.                             */
         /*                                                                  */
         /********************************************************************/
         if( state == OP4R )
         {
          state = OP2;
          goto NextSymbol;
         }

         ParseError = "Invalid use of \"]\"";
         goto bummer;
      }
      break;                            /* end of switch on operator.        */

    case 2:

      /***********************************************************************/
      /* The identifier that we parsed off may be a variable or a reg     221*/
      /* name. There are NREGS16 registers with indices in the range      221*/
      /* 0-NREGS16-1. We first aassume SymRegIndex out of range.          221*/
      /***********************************************************************/
      SymRegIndex = NREGS16;

      /***********************************************************************/
      /* This is the case of a possible register. 2 character symbol         */
      /* names will not have a valid register index and will fall thru.      */
      /***********************************************************************/
      SymRegIndex = windex(RegNames16, NREGS16, *(twoc *)sym);
      IsSymRegName = (SymRegIndex < NREGS16) ? TRUE : FALSE;
                                        /* set flag to know the sym is reg221*/
      break;

    case 3:

      /***********************************************************************/
      /* The identifier that we parsed off may be a variable or a reg     221*/
      /* name. There are NREGS32 registers with indices in the range      221*/
      /* 0-NREGS32-1. We first aassume SymRegIndex out of range.          221*/
      /*                                                                     */
      /* Copy the register name from sym into a temp array with a blank   221*/
      /* character added as a pading.                                     221*/
      /***********************************************************************/
      SymRegIndex = NREGS32;                                            /*221*/
      memcpy(Regname,sym,3);                                            /*221*/
      Regname[3]  = ' ';                                                /*221*/

      /***********************************************************************/
      /* This is the case of a possible register. 2 character symbol         */
      /* names will not have a valid register index and will fall thru.      */
      /***********************************************************************/
      SymRegIndex = lindex(RegNames32, NREGS32, *(fourc *)Regname);
      IsSymRegName = (SymRegIndex < NREGS32) ? TRUE : FALSE;
                                        /* set flag to know the sym is reg221*/
      break;
  }                                     /* end of switch on symlen           */

/*****************************************************************************/
/* At this point, we may have an expression of the form ABxx:offset. The     */
/* "ABxx" would have been parsed as a symbol name. It may also be a register */
/* name. In this case, we will treat the symbol "ABxx" as a hex number, back */
/* up and reparse i.e., we will start all over.                              */
/*                                                                           */
/*****************************************************************************/
  if( (state == OP1C || *cp == ':') &&  /*                                   */
       !IsSymRegName                    /*                                   */
    )                                   /*                                   */
  {                                     /*                                   */
   c = '0';                             /* force a leading zero              */
   cp = sym;                            /* backup to start of symbol         */
   goto ParseNumber;                    /*                                   */
  }                                     /* parse as a number                 */
                                        /*                                   */
/*****************************************************************************/
/* At this time if we are in OP4* state it indicates we have a expression 513*/
/* of type ....[xxx]. We then call ParseArrayIndex with the pointer to    513*/
/* the variable name which is index. This function returns the value of   513*/
/* the index. If it is not successful report error and return.            513*/
/*****************************************************************************/
  if (state & OP4)                                                      /*513*/
  {                                                                     /*513*/
    uint  IndexValue;                                                   /*513*/

    buffer[ symlen+2 ] = 0;               /* changed 1 to a 2.           813*/
    *(USHORT*)buffer = symlen;            /*                             813*/
    memcpy( buffer+2 ,sym, symlen);       /*                             813*/
    if (ParseArrayIndex(buffer+2, mid, progline, progsfi, &IndexValue))
    {                                                                   /*513*/
      ans = IndexValue;                                                 /*513*/
      goto GotOperand;                                                  /*513*/
    }                                                                   /*513*/
    else                                                                /*513*/
    {                                                                   /*513*/
      ParseError = BadExprMsg;                                          /*513*/
      goto bummer;                                                      /*513*/
    }                                                                   /*513*/
  }                                                                     /*513*/

  if(context == 2)
  {
   ParseError = BadExprMsg;
   goto bummer;
  }

/*****************************************************************************/
/* At this time, we have a token that is a potential symbol variable.        */
/*                                                                           */
/*****************************************************************************/
  buffer[ symlen+2 ] = 0;               /* changed 1 to a 2.              813*/
  *(USHORT*)buffer = symlen;            /*                                813*/
  memcpy( buffer+2 ,sym, symlen);       /*                                813*/

/*****************************************************************************/
/* We now look for the symbol in the symbolic info. First, as a local        */
/* variable, and then as a static variable. We move to the OP2 state from    */
/* this state.                                                               */
/*                                                                           */
/*****************************************************************************/
  if( (state == OP0) && ( mid != 0 ) )  /* add mid != 0 check.               */
  {                                     /*                                   */
   if( !NoLocalNames )                  /* if local names are ok, then       */
   {                                    /*                                   */
    BOOL this;

    this   = FALSE;
    symloc = findlvar(mid, buffer, progline, progsfi, &this);
    if( this == TRUE )
    {
     ThisProcessing = TRUE;
     cppsym         = Talloc( strlen(buffer+2) + 3 );
     cppsym[0]      = '-' ;
     cppsym[1]      = '>' ;

     strcpy( cppsym + 2, buffer + 2 );

     cp      = cppsym;
     off     = symloc;
     ExprLno = progline;
     ExprMid = mid;
     state   = OP2;
     goto NextSymbol;
    }

    if( symloc != NULL  )               /*  if we found a local symbol, then */
     ExprLno = progline;                /*  establish the source line context*/
 /* these lines ARE needed! see caseg.c  211 */
    else                                /*                                   */
     symloc = findsvar(mid, buffer);    /*  else look for a static symbol.   */
                                        /*                                   */
    if( symloc != NULL )                /*  if we found a symbol, then    110*/
    {                                   /*                                   */
/*   seg = SelOf(symloc);   */          /*  establish its selector and    112*/
     off = symloc;                      /*  offset location.              112*/
     ExprMid = mid;                     /*  establish its mid.               */
     state = OP2;                       /*  move to OP2 state.               */
     ExprLno = progline;                /*  establish the srcline context 513*/
     goto NextSymbol;                   /*  go get the next token.           */
    }                                   /*                                   */
   }                                    /*                                   */
                                        /*                                   */
/*****************************************************************************/
/* We were not successful in our search for the symbol in local or static    */
/* data. So, we will look in the public area.                                */
/*                                                                           */
/*****************************************************************************/
   pdf=FindExeOrDllWithAddr(GetExecAddr());                             /*901*/

   symloc= DBPub(buffer+2,pdf);                                         /*901*/
   if( (symloc == NULL)  && IfUscores )
   {
    buffer[1] = '_';
    symloc = DBPub(buffer+1,pdf);                                       /*901*/
   }                                    /*                                   */
                                        /*                                   */
   if( symloc != NULL )                 /* if we finally found the symbol,110*/
   {                                    /* then establish                    */
/*  seg = SelOf(symloc);    */          /* the selector of its location,     */
    off = symloc;                       /* the offset of its location,       */
    ExprMid = DataMid;                  /* the expression module id, and     */
    ExprTid = DataTid;                  /* the expression type id.           */
    state = OP2;                        /* go to the 2 operand state.        */
    ExprLno = progline;                 /*  establish the srcline context 513*/
    goto NextSymbol;                    /* go get the next token.            */
   }                                    /*                                   */
  }                                     /* end of find the symbol.           */

/*****************************************************************************/
/* At this point, we have a pointer to the symbol location, symloc. We also  */
/* have an ExprMid and an ExprTid. If we are in the OP3 state, we have       */
/* 2 operands and a "." or a "->". In this case, we need to adjust off for   */
/* the location of the field within the structure. We leave in state OP2.    */
/*                                                                           */
/*****************************************************************************/
  if( state == OP3 )
  {
   if( QstructField(ExprMid, buffer, &fldoff, &ExprTid) )
   {
    off += fldoff;
    state = OP2;
    goto NextSymbol;
   }
/*****************************************************************************/
/* At this point, we may have a typedef record for a PL/X variable.          */
/* A call to findlvar() will establish ExprTid for the PL/X expression of    */
/* the form ptr->sbase, where sbase is a based variable.                     */
/*****************************************************************************/
   {                                    /* open {}.                          */
    uint   typeno;                      /* type number.                      */
    Trec  *tp;                          /* -> base rec for parent dfile no   */
    SSRec *p;                           /*                                809*/
    if((p = Qsymbol(mid, progline, progsfi, buffer)) &&
       (p->RecType == SSUSERDEF )       /*                            809 112*/
      )                                 /*                                   */
     {                                  /*                                   */
      typeno = ((SSUserDef *)p)->TypeIndex;/*define type of based var.809 112*/
      tp = QbasetypeRec(mid, typeno);   /* get -> to the type record.        */

      if( tp && (tp->RecType == T_PTR ) )                               /*813*/
       typeno = ((TD_POINTER *)tp)->TypeIndex;                          /*813*/

      ExprTid = typeno;                 /* establish the expression type.    */
      ExprMid = mid;                    /* establish mid of typedef.         */
      ExprScope = Qproc;                /* establish the expression scope.   */
      state = OP2;                      /*                                   */
      goto NextSymbol;                  /*                                   */
     }                                  /*                                   */
   }                                    /* close {}                          */
  }                                     /* end of "if state = OP3"           */
/*****************************************************************************/
/* We get to here with a register name so we get a value from the DosDebug   */
/* buffer.                                                                221*/
/* The DosDebug structure looks some thing like this.                     221*/
/*                                                                        221*/
/* {                                                                      221*/
/*  .....                                                                 221*/
/*  ulong  EAX       |                                                    221*/
/*  ulong  ECX       |                                                    221*/
/*  ulong  EDX       |  for these registers the windex and lindex funcs   221*/
/*  ulong  EBX       |  return a SymRegIndex in the range of 0 to 9       221*/
/*  ulong  ESP       |                                                    221*/
/*  ulong  EBP       |  if it falls here the processing is simple         221*/
/*  ulong  ESI       |  take the address of AppPTB.EAX cast it to a uint  221*/
/*  ulong  EDI       |  added the SymRegIndex to it and then take the     221*/
/*  ulong  EFlags    |  contents of the new address.                      221*/
/*  ulong  EIP       |                                                    221*/
/*  ...                                                                   221*/
/*  ushort CS        |                                                    221*/
/*  ulong  Dslim     |  You fall here only to grab the addr of selectors  221*/
/*  ulong  DsBase    |  In this cases the SymRegIndex will be greater     221*/
/*  uchar  DsAcc     |  than 9.                                           221*/
/*  uchar  DsAtr     |  In this cases take the address of AppPTB.CS as    221*/
/*  ushort DS        |  base.                                             221*/
/*  ulong  Eslim     |  Calculate the offset difference between the CS    221*/
/*  ulong  EsBase    |  field and the selector field for which we have    221*/
/*  uchar  EsAcc     |  determine the address of. Here in the below code  221*/
/*  uchar  EsAtr     |  12 is a magic figure which is the size of offset  221*/
/*  ushort ES        |  between each selector fields in DosDebug Buffer.  221*/
/*                                                                        221*/
/*************************************************************************221*/
  if( IsSymRegName )                                                    /*221*/
  {                                                                     /*221*/
   if (state != OP1C)                   /* one operand and a ":"          243*/
     ExprTid = VALUE_CONSTANT;          /* expression type is CONSTANT    243*/
   if ( SymRegIndex < 10 )                                              /*221*/
     ans = *( (uint *)&(AppPTB.EAX) + SymRegIndex );                    /*221*/
   else                                                                 /*221*/
   {                                                                    /*221*/
                                                                        /*221*/
     temp = (uint) &(AppPTB.CS);                                        /*221*/
     temp += ((SymRegIndex - 10) * 12 );                                /*221*/
     ans = *( (ushort *)temp );                                         /*221*/
   }                                                                    /*221*/
   IsSymRegName = 0;                    /* reset the register name flag   312*/
   goto GotOperand;
  }
/*****************************************************************************/
/* We are done.                                                              */
/*****************************************************************************/

  goto fini;

 GotOperand:

/*****************************************************************************/
/* At this point, we are going to go through a state transition.             */
/*                                                                           */
/*****************************************************************************/
  switch( state )                       /*                                   */
  {                                     /*                                   */
   case OP0:                            /* initial state.                    */
       off = ans;                       /*                                   */
       CmplxBkptVal = off;              /*                                   */
       state = OP1;                     /*                                   */
       break;                           /*                                   */
                                        /*                                   */
   case OP0M:                           /* an initial negative number.       */
       off = -(int)ans;                 /*                                521*/
       CmplxBkptVal = (long)((int)off); /*                                   */
       state = OP1;                     /*                                   */
       break;                           /*                                   */
                                        /*                                   */
   case OP1C:                           /* one operand and a ":"             */
       off = ans;                       /*                                   */
       state = OP2;                     /*                                   */
       break;                           /*                                   */
                                        /*                                   */
   case OP1P:                           /* one operand and a "+".            */
   case OP2P:                           /* two operands and a "+".           */
       off += ans;                      /*                                   */
       goto ResetOp;                    /*                                   */
                                        /*                                   */
   case OP1M:                           /* one operand and a "-".            */
   case OP2M:                           /* two operands and a "-".           */
       off -= ans;                      /*                                   */
       goto ResetOp;                    /*                                   */
                                        /*                                   */
   case OP1T:                           /* one operand and a "-".            */
   case OP2T:                           /* two operands and a "-".           */
       off *= ans;                      /*                                   */
                                        /*                                   */
   ResetOp:                             /*                                   */
       /**********************************************************************/
       /* Reset +,-, and * states to OP0,OP1, or OP2 states.                 */
       /*                                                                    */
       /**********************************************************************/
       state &= 0xF0;                   /*                                   */
       break;                           /*                                   */
                                        /*                                   */
   case OP4L:                           /* two operands and a "[".           */
       if( QarrayItem(ExprMid,          /*                                   */
                      ans,              /*                                   */
                      &off,             /*                                   */
                      &ExprTid)         /*                                   */
         )                              /*                                   */
       {                                /*                                   */
           state = OP4R;                /*                                   */
           break;                       /*                                   */
       }                                /*                                   */
                                        /*                                   */
       ParseError = "Invalid type for \"[\"";
       goto bummer;                     /*                                   */
                                        /*                                   */
   default:                             /* invalid states.                   */
       ParseError = BadExprMsg;         /*                                   */
       goto bummer;                     /*                                   */
  }                                     /*                                   */
                                        /*                                   */
                                        /*                                   */
  NextSymbol:;                          /*                                   */
 }                                      /* end of token parsing loop.        */
/*****************************************************************************/
/* All tokens are parsed. We now establish the base symloc for "*" references*/
/*****************************************************************************/
                                        /*                                   */
 fini:                                  /*                                   */
 if( deref )                            /* now resolve "contents of"         */
 {                                      /*                                   */
  if( state != OP2 )                    /* must be in OP2 state.             */
  {                                     /*                                   */
   ParseError="Invalid operand for \"*\"";/*                                 */
   goto bummer;                         /*                                   */
  }                                     /*                                   */
                                        /*                                   */
  while( deref-- )                      /* loop while pointer is not deref'd.*/
  {                                     /*                                   */
   /**************************************************************************/
   /* dereference the pointer and convert to flat if need be.             112*/
   /**************************************************************************/
   symloc = DerefPointer( off,ExprMid );                                /*240*/
   if( symloc != NULL)                  /*                                205*/
   {                                    /*                                   */
/*  seg = SelOf(symloc);   */           /*                                112*/
    off = symloc;                       /*                                112*/
    if( !QderefType(ExprMid, (USHORT*)&ExprTid) )/*                          */
        ExprTid = 0;                    /*                                   */
   }                                    /*                                   */
   else                                 /*                                   */
   {                                    /*                                   */
    ParseError = "Invalid pointer for \"*\"";/*                              */
    goto bummer;                        /*                                   */
   }                                    /*                                   */
  }                                     /*                                   */
 }                                      /*                                   */
/*****************************************************************************/
/* We can return from the OP1 state or the OP2 state.                        */
/*                                                                           */
/*****************************************************************************/
                                        /*                                   */
 switch( state )                        /*                                   */
 {                                      /*                                   */
  case OP2:                             /*                                   */
    if( addrof )                        /* if "&" flag is set, then          */
    {                                   /*                                   */
     ExprTid = ADDRESS_CONSTANT;        /* expression type is CONSTANT    243*/
     IsAddrExpr = FALSE;                /*                                243*/
    }                                   /*                                   */
  NormalReturn:                         /*                                   */
    if( TestBit(seg,02) )               /* if its a GDT then it must be   221*/
      ExprAddr = Data_SelOff2Flat((ushort)seg,                              /*827*/
                 (ushort)off);          /* simple flat address else       607*/
    else                                /* convert into flat addr.        221*/
      ExprAddr = off;                   /*                                221*/
    return( sym );                      /* return -> to vestigial expression.*/
                                        /*                                   */
  case OP1:                             /*                                   */
    if( !addrof )                       /*                                   */
    {                                   /* its a flat address the segment 501*/
      seg = 0;                          /* does not make any sense.       501*/
      goto NormalReturn;                /*                                501*/
    }                                   /*                                501*/
 }                                      /*                                   */

 if(*sym && (ThisProcessing == TRUE) )
 {
  /***************************************************************************/
  /* We will come here if we've tried to find a class variable and           */
  /* failed. Go back and try again, this time only looking for globals.      */
  /***************************************************************************/
  cp             = sym;
  context        = 0x20;
  ThisProcessing = FALSE;
  goto startallover;
 }
/*****************************************************************************/
/* At this point, if there is a vestigial expression, then something has gone*/
/* awry. If there isn't a vestigial expression, just being here is an error. */
/*****************************************************************************/
 if( *sym )                             /* did we finish parsing?            */
 {                                      /* nope, we need to build a message. */
  uint n;                               /* was register.                  112*/
  static uchar emsg[] =                 /* This is the base message. We'll   */
               "\"%.*s\" is incorrect"; /* add to it.                        */
                                        /*                                   */
  n=strlen(sym);                        /* n = length of expression vestige. */
  if( (n) > (sizeof(ErrMsgBuf) -        /* if n is too big to fit in the     */
             sizeof(emsg)               /* front of the buffer, then chop it */
            )                           /* off.                              */
    )                                   /*                                   */
  {                                     /*                                   */
   n=(sizeof(ErrMsgBuf) - sizeof(emsg));/*                                   */
  }                                     /*                                   */
                                        /*                                   */
  ParseError=ErrMsgBuf;                 /*                                   */
  sprintf(ParseError,emsg,n,sym);       /* format the message.            521*/
 }                                      /*                                   */
 else                                   /*                                   */
  ParseError = BadExprMsg;              /* just a bad expression.            */
                                        /*                                   */
bummer:                                 /*                                   */
 return( NULL );                        /*                                   */
}

 uint
GetAbsAddr(uint addr,uint sfx,uint *absptr)
{

    if( TestBit(addr,STACKADDRBIT) )                                    /*205*/
    {                                                                   /*205*/
      addr = StackBPRelToAddr( addr , sfx );                            /*205*/
      if( addr == NULL )                                                /*205*/
            return( FALSE );                                            /*205*/
    }else if( (addr >> REGADDCHECKPOS) == REGISTERTYPEADDR ){           /*205*/
        return( FALSE );
    }
    *absptr = addr;
    return( TRUE );
}

/*****************************************************************************/
/* ParseArrayIndex()                                                      513*/
/*                                                                           */
/* Description:                                                              */
/*                                                                           */
/*  Gets the value of a index variable passed.                               */
/*                                                                           */
/* Parameters:                                                               */
/*                                                                           */
/*   cp         input - pointer to the index variable name.                  */
/*   optmid     input - module id.                                           */
/*   optline    input - line no.                                             */
/*   IndexValue input - -> index value.                                      */
/*                                                                           */
/* Return:                                                                   */
/*                                                                           */
/*   TRUE     able to find the index variable value.                         */
/*   FALSE    not able to find index variable value.                         */
/*                                                                           */
/* Assumptions:                                                              */
/*                                                                           */
/*   none.                                                                   */
/*                                                                           */
/*****************************************************************************/
uint ParseArrayIndex(uchar *cp,
                     uint optmid,
                     uint optline,
                     int  optsfi,
                     uint *IndexValue)
{
    uchar  *xp;
    uint    sfx;
    uchar  *dp;
    uint    read;
    uint    n;
    uint    Save_ExprAddr;
    SCOPE   Save_ExprScope;
    uint    Save_ExprMid;
    uint    Save_ExprLno;
    uint    Save_ExprTid;
    uchar   Save_IsAddrExpr;
    uchar  *Save_ParseError;
    uint    Return_Value = FALSE;

    /*************************************************************************/
    /* save the global values set by parseexpr since they may be lost by  513*/
    /* the next call to parseexpr.                                        513*/
    /*************************************************************************/
    Save_ExprAddr   = ExprAddr;
    Save_ExprScope  = ExprScope;
    Save_ExprMid    = ExprMid;
    Save_ExprLno    = ExprLno;
    Save_ExprTid    = ExprTid;
    Save_IsAddrExpr = IsAddrExpr;
    Save_ParseError = ParseError;

    /*************************************************************************/
    /* call parseexpr to get the expression address and other details     513*/
    /*************************************************************************/
    xp = ParseExpr(cp, 0x10, optmid, optline, optsfi);
    if( !xp || *xp )
       goto Error;

    /*************************************************************************/
    /* get the stack frame index and check if its not active or scope is  513*/
    /* not correct return failure.                                        513*/
    /*************************************************************************/
    sfx=StackFrameIndex(ExprScope);
    if( ExprScope && !sfx )
       goto Error;

    /*************************************************************************/
    /* find the type no of the index variable. If it is not one of the    513*/
    /* following return a error. (int,uint,char,uchar,long,ulong)         513*/
    /*************************************************************************/
    n = windex(dtab, NTYPES, ExprTid);
    if (n > (NTYPES-3))
       goto Error;

    /*************************************************************************/
    /* - read the contents of the index variable.                         513*/
    /* - depending on the type cast the index value to int.               513*/
    /*************************************************************************/
    if( !(dp = GetAppData(ExprAddr,QtypeSize(DONTCARE,ExprTid), &read, sfx)) )
       goto Error;

    switch( ExprTid )
    {
        case TYPE_CHAR:
          *IndexValue = *(char*)dp;
          break;

        case TYPE_UCHAR:
          *IndexValue = *(uchar*)dp;
          break;

        case TYPE_SHORT:
          *IndexValue = *(short*)dp;
          break;

        case TYPE_USHORT:
          *IndexValue = *(ushort*)dp;
          break;

        case TYPE_LONG:
          *IndexValue = *(long*)dp;
          break;

        case TYPE_ULONG:
          *IndexValue = *(ulong*)dp;
          break;

        default:
          goto Error;
    }

    Return_Value = TRUE;

    /*************************************************************************/
    /* restore the saved variables and return.                            513*/
    /*************************************************************************/
    Error:
      ExprAddr   = Save_ExprAddr;
      ExprScope  = Save_ExprScope;
      ExprMid    = Save_ExprMid;
      ExprLno    = Save_ExprLno;
      ExprTid    = Save_ExprTid;
      IsAddrExpr = Save_IsAddrExpr;
      ParseError = Save_ParseError;
      return(Return_Value);
}
