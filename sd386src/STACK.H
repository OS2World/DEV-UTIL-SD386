typedef struct _stack_parms
{
 USHORT CS;
 USHORT SS;
 ULONG  EBP;
 ULONG  ESP;
 UCHAR  SSAtr;
 int    ShowAllFlag;
}STACK_PARMS;

/*****************************************************************************/
/* Parameter interface for call stack dialog function.                       */
/* This was put here for lack of a better place to put it. There is no       */
/* logical reason for it to be here.                                         */
/*****************************************************************************/
typedef struct
{
  AFILE **fpp;
  int    *rc;
} CALLSTACKPARAM;
