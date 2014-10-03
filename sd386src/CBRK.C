/*****************************************************************************/
/* File:                                             IBM INTERNAL USE ONLY   */
/*   cbrk.c                                                                  */
/*                                                                           */
/* Description:                                                              */
/*  complex and data breakpoint handling                                     */
/*                                                                           */
/*                                                                           */
/* History:                                                                  */
/*                                                                           */
/*   02/08/91 Creation of 32-bit SD86, from 16-bit version.                  */
/*                                                                           */
/*                                                                           */
/*...16->32 port.                                                            */
/*...                                                                        */
/*... 02/08/91  101   Joe       port to 32 bit.                              */
/*... 02/08/91  112   Joe       port to 32 bit.                              */
/*                                                                           */
/*...Release 1.00 (Pre-release 1)                                            */
/*...                                                                        */
/*... 07/09/91  206   srinivas  Hooking up of complex break points.          */
/*...                                                                        */
/*...Release 1.00 (03/03/92)                                                 */
/*...                                                                        */
/*... 03/12/92  604   Srinivas  Conditional break points failing on signed   */
/*...                           variables.                                   */
/*****************************************************************************/
#include "all.h"

extern BRKCOND      DataBreak;
extern uchar        IfDataBreak;
extern uchar        IsAddrExpr;         /* Set by ParseExpr                  */
extern uint         ExprAddr;           /* Set by ParseExpr               101*/
extern uint         ExprMid;            /* Set by ParseExpr                  */
extern uint         ExprTid;            /* Set by ParseExpr                  */
extern PtraceBuffer AppPTB;

EvalCbrk(BRKCOND *p)
{
    union
    {
     char  _8;
     short _16;                         /*                                206*/
     long  _32;
    }data;

    char  *charptr;
    short   *wordptr;                   /*                                206*/
    long  *longptr;
    int    bytesread;

    uint target = p->opaddr;            /*                                206*/

    if( TestBit(target,STACKADDRBIT) )  /*                                206*/
    {
      if( TestBit(target,STACKADDRSIGN) == FALSE )                      /*206*/
         ResetBit( target, STACKADDRBIT );                              /*206*/
      target += AppPTB.EBP;
      if( AppPTB.SSAtr == 0 )                                           /*206*/
         target = Data_SelOff2Flat( AppPTB.SS, LoFlat(target) );
    }                                   /*                                206*/

    switch( p->opsize ){

      case 2:

        data._32 = 0;
        wordptr = (short *)DBGet(target, 2, (uint *)&bytesread );       /*206*/
        if ( wordptr )
        {
         data._16 = *wordptr;
         break;
        }
        goto failed;

      case -2:

        wordptr = (short *)DBGet(target, 2, (uint *)&bytesread );       /*206*/
        if ( wordptr )
        {
         data._16 = *wordptr;
         data._32 = data._16;
         break;
        }
        goto failed;

      case 4:
      case -4:
        longptr = ( long *)DBGet(target, 4, (uint *)&bytesread );
        if ( longptr )
        {
         data._32 = *longptr;
         break;
        }
        goto failed;

      case 1:

        data._32 = 0;
        charptr = ( char *)DBGet(target, 1, (uint *)&bytesread );
        if ( charptr )
        {
         data._8 = *charptr;
         break;
        }
        goto failed;

      case -1:

        charptr = ( char *)DBGet(target, 1, (uint *)&bytesread );
        if ( charptr )
        {
         data._8 = *charptr;
         data._32 = data._8;
         break;
        }
        goto failed;

    }
    switch( p->relation ){
      case COND_NE:  return( data._32 != p->constant );
      case COND_EQ:  return( data._32 == p->constant );
      case COND_GT:  return( data._32 >  p->constant );
      case COND_GE:  return( data._32 >= p->constant );
      case COND_LT:  return( data._32 <  p->constant );
      case COND_LE:  return( data._32 <= p->constant );
      case COND_GTU: return( (ulong)data._32 >  (ulong)p->constant );
      case COND_GEU: return( (ulong)data._32 >= (ulong)p->constant );
      case COND_LTU: return( (ulong)data._32 <  (ulong)p->constant );
      case COND_LEU: return( (ulong)data._32 <= (ulong)p->constant );
    }
    failed:
      return( TRUE );

}

#define NRELOPS 7
static  twoc relops[ NRELOPS ] = {
  '>' + 256*' ',   /*0  >   */
  '>' + 256*'=',   /*1  >=  */
  '<' + 256*' ',   /*2  <   */
  '<' + 256*'=',   /*3  <=  */
  '!' + 256*'=',   /*4  !=  */
  '=' + 256*'=',   /*5  ==  */
  '=' + 256*' '    /*6  =   */
};
#define RELOP_NE 4

static  scondtab[ NRELOPS ] = {
  COND_GT, COND_GE, COND_LT, COND_LE, COND_NE, COND_EQ, COND_EQ
};/*  0        1        2        3        4        5        6  */

static  ucondtab[ NRELOPS ] = {
  COND_GTU, COND_GEU, COND_LTU, COND_LEU, COND_NE, COND_EQ, COND_EQ
};/*  0         1         2         3         4        5        6  */

#define NOKTYPES 6
static  ushort oktypes[ NOKTYPES ] = {  /*                                101*/
  TYPE_CHAR,  /*0*/                     /*                                101*/
  TYPE_SHORT, /*1*/                     /*                                101*/
  TYPE_LONG,  /*2*/                     /*                                101*/
  TYPE_UCHAR, /*3*/                     /*                                101*/
  TYPE_USHORT,/*4*/                     /*                                101*/
  TYPE_ULONG  /*5*/                     /*                                101*/
};
static  uchar oktypesigned[ NOKTYPES ] = {
  TRUE, TRUE, TRUE, FALSE, FALSE, FALSE
};/*0     1     2     3      4      5  */

static  uchar term2msg[] = "Constant required after ==";
#define T2MOP 24          /*0123456789012345678901234*/
static  uchar relopmsg[] =
"Syntax: var op const   (op is one of ==,!=,>=,<=,>,<";


    uchar *
ParseCbrk(BRKCOND *p,uint mid,uint lno, int sfi)
{
    uchar *cp;                          /* was register.                  112*/
    uint opx, n, IsSigned;

 ULONG typeno;

    if( *(cp = p->pCondition) ){
        if( !(cp = ParseExpr(cp,0x10,mid,lno, sfi)) || !IsAddrExpr )
            return( "Incorrect variable name" );
        if( (ExprAddr >> REGADDCHECKPOS) == REGISTERTYPEADDR )
            return( "Can't use register variable" );
        p->opaddr = ExprAddr;

        if( *cp ){
            if( (opx = windex(relops, NRELOPS, *(twoc*)cp)) == NRELOPS )
                return( relopmsg );
        }else
            opx = RELOP_NE;

        /* Set the type of comparison based on the variable type and relop */
        typeno = HandleUserDefs( mid, ExprTid );
        if( typeno  ){
            switch( QtypeGroup(ExprMid, typeno ) ){
              case TG_SCALAR:
                if( (n = windex(oktypes, NOKTYPES, typeno )) == NOKTYPES )
                    goto caseBadType;
                IsSigned = oktypesigned[n];  break;
              case TG_POINTER:
                IsSigned = FALSE;  break;
              default:
              caseBadType:
                return( "Unsupported variable type" );
            }
            p->opsize = (signed char)QtypeSize(ExprMid, typeno );       /*604*/

        }else
            p->opsize = (signed char)sizeof(uint);                      /*604*/

        if( IsSigned ){
            p->opsize = (signed char)((signed char)0 - p->opsize);      /*604*/
            p->relation = (uchar)scondtab[opx];
        }else{
            p->relation = (uchar)ucondtab[opx];
        }

        /* Set the constant to compare the variable against */
        if( *cp )
        {
         extern long CmplxBkptVal;
         cp = ParseExpr(cp+2,0,0,0, sfi);
         if( !cp )
         {
          *(twoc*)(term2msg+T2MOP) = relops[opx];
          return( term2msg );
         }
         p->constant = CmplxBkptVal;
        }else
        {
         return( "This feature not implemented yet!" );
        }
    }

    return( NULL );
}
