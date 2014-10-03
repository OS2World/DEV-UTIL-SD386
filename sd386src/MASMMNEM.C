 
/***-START-OF-SPECIFICATIONS-****************************************
***                                                               ***
***        CP/386 Experimental Operating System                   ***
***                                                               ***
***        Task Level Debugger                                    ***
***                                                               ***
***        COPYRIGHT   I B M   CORPORATION  1983, 1984, 1985      ***
***                                   1986, 1987, 1988, 1989      ***
***                                                               ***
***        LICENSED MATERIAL  -  PROGRAM PROPERTY OF I B M        ***
***                                                               ***
***        REFER TO COPYRIGHT INSTRUCTIONS: FORM G120-2083        ***
***                                                               ***
*********************************************************************
*                                                                   *
*                                                                   *
*   MODULE-NAME = MASMMNEM                                          *
*                                                                   *
*   DESCRIPTIVE-NAME = MASM mnemonics for the dis-assembler.  This  *
*                      is included into TLDUNPIK.C.                 *
*                                                                   *
*                      Note that the mnemonic tables are have the   *
*                      format of a data table for the routine       *
*                      "lookup", so it can be used for the          *
*                      assembler.                                   *
*                                                                   *
*                                                                   *
*   STATUS = Version 3 Release 2                                    *
*                                                                   *
*                                                                   *
*   CHANGE-ACTIVITY = translated from PLS/286 March 1989 by         *
*                     D. C. Toll, IBM Research, Yorktown Heights    *
*                                                                   *
*   05/02/89,       = extra orders for the 486 added.               *
*   D. C. Toll                                                      *
*                                                                   *
*                                                                   *
****-END-OF-SPECIFICATIONS-*****************************************/
 
 
 
 
/*************************************************************************
*                                                                        *
*   The following table holds the MASM mnemonics:  these are accessed    *
*   by mnemonic number, not by instruction value.                        *
*                                                                        *
*   Note that these vectors all have names starting with "masm".         *
*                                                                        *
*************************************************************************/
 
 
CHAR masmmnem[] = {
        3,3,'A','A','A',
        3,3,'A','A','D',                 /*   2  */
        3,3,'A','A','M',
        3,3,'A','A','S',                 /*   4  */
        3,3,'A','D','C',
        3,3,'A','D','D',                 /*   6  */
        3,3,'A','N','D',
        4,4,'A','R','P','L',             /*   8  */
        5,5,'B','O','U','N','D',
        3,3,'B','S','F',                 /*  10  */
        3,3,'B','S','R',
        2,2,'B','T',                     /*  12  */
        3,3,'B','T','C',
        3,3,'B','T','R',                 /*  14  */
        3,3,'B','T','S',
        4,4,'C','A','L','L',             /*  16  */
        3,3,'C','B','W',
        3,3,'C','D','Q',                 /*  18  */
        3,3,'C','L','C',
        3,3,'C','L','D',                 /*  20  */
        3,3,'C','L','I',
        4,4,'C','L','T','S',             /*  22  */
        3,3,'C','M','C',
        3,3,'C','M','P',                 /*  24  */
        5,5,'C','M','P','S','B',
        5,5,'C','M','P','S','D',         /*  26  */
        5,5,'C','M','P','S','W',
        3,3,'C','W','D',                 /*  28  */
        4,4,'C','W','D','E',
        3,3,'D','A','A',                 /*  30  */
        3,3,'D','A','S',
        3,3,'D','E','C',                 /*  32  */
        3,3,'D','I','V',
        5,5,'E','N','T','E','R',         /*  34  */
        3,3,'E','S','C',
        3,3,'H','L','T',                 /*  36  */
        4,4,'I','D','I','V',
        4,4,'I','M','U','L',             /*  38  */
        2,2,'I','N',
        3,3,'I','N','C',                 /*  40  */
        4,4,'I','N','S','B',
        4,4,'I','N','S','D',             /*  42  */
        4,4,'I','N','S','W',
        3,3,'I','N','T',                 /*  44  */
        4,4,'I','N','T','O',
        4,4,'I','R','E','T',             /*  46  */
        5,5,'I','R','E','T','D',
        2,2,'J','A',                     /*  48  */
        2,2,'J','B',
        4,4,'J','C','X','Z',             /*  50  */
        5,5,'J','E','C','X','Z',
        2,2,'J','G',                     /*  52  */
        2,2,'J','L',
        3,3,'J','M','P',                 /*  54  */
        3,3,'J','N','A',
        3,3,'J','N','B',                 /*  56  */
        3,3,'J','N','G',
        3,3,'J','N','L',                 /*  58  */
        3,3,'J','N','O',
        3,3,'J','N','S',                 /*  60  */
        3,3,'J','N','Z',
        2,2,'J','O',                     /*  62  */
        3,3,'J','P','E',
        3,3,'J','P','O',                 /*  64  */
        2,2,'J','S',
        2,2,'J','Z',                     /*  66  */
        4,4,'L','A','H','F',
        3,3,'L','A','R',                 /*  68  */
        3,3,'L','D','S',
        3,3,'L','E','A',                 /*  70  */
        5,5,'L','E','A','V','E',
        3,3,'L','E','S',                 /*  72  */
        3,3,'L','F','S',
        4,4,'L','G','D','T',             /*  74  */
        3,3,'L','G','S',
        4,4,'L','I','D','T',             /*  76  */
        4,4,'L','L','D','T',
        4,4,'L','M','S','W',             /*  78  */
        4,4,'L','O','C','K',
        5,5,'L','O','D','S','B',         /*  80  */
        5,5,'L','O','D','S','D',
        5,5,'L','O','D','S','W',         /*  82  */
        4,4,'L','O','O','P',
        6,6,'L','O','O','P','N','Z',     /*  84  */
        5,5,'L','O','O','P','Z',
        3,3,'L','S','L',                 /*  86  */
        3,3,'L','S','S',
        3,3,'L','T','R',                 /*  88  */
        3,3,'M','O','V',
        5,5,'M','O','V','S','B',         /*  90  */
        5,5,'M','O','V','S','D',
        5,5,'M','O','V','S','W',         /*  92  */
        5,5,'M','O','V','S','X',
        5,5,'M','O','V','Z','X',         /*  94  */
        3,3,'M','U','L',
        3,3,'N','E','G',                 /*  96  */
        3,3,'N','O','P',
        3,3,'N','O','T',                 /*  98  */
        2,2,'O','R',
        3,3,'O','U','T',                 /* 100  */
        5,5,'O','U','T','S','B',
        5,5,'O','U','T','S','D',         /* 102  */
        5,5,'O','U','T','S','W',
        3,3,'P','O','P',                 /* 104  */
        4,4,'P','O','P','A',
        5,5,'P','O','P','A','D',         /* 106  */
        4,4,'P','O','P','F',
        5,5,'P','O','P','F','D',         /* 108  */
        4,4,'P','U','S','H',
        5,5,'P','U','S','H','A',         /* 110  */
        6,6,'P','U','S','H','A','D',
        5,5,'P','U','S','H','F',         /* 112  */
        6,6,'P','U','S','H','F','D',
        5,5,'P','U','S','H','I',         /* 114  */
        3,3,'R','C','L',
        3,3,'R','C','R',                 /* 116  */
        3,3,'R','E','P',
        5,5,'R','E','P','N','Z',         /* 118  */
        4,4,'R','E','P','Z',
        3,3,'R','E','T',                 /* 120  */
        3,3,'R','O','L',
        3,3,'R','O','R',                 /* 122  */
        4,4,'S','A','H','F',
        3,3,'S','A','L',                 /* 124  */
        3,3,'S','A','R',
        3,3,'S','B','B',                 /* 126  */
        5,5,'S','C','A','S','B',
        5,5,'S','C','A','S','D',         /* 128  */
        5,5,'S','C','A','S','W',
        3,3,'S','E','G',                 /* 130  */
        4,4,'S','E','T','A',
        5,5,'S','E','T','A','E',         /* 132  */
        4,4,'S','E','T','B',
        5,5,'S','E','T','B','E',         /* 134  */
        4,4,'S','E','T','G',
        5,5,'S','E','T','G','E',         /* 136  */
        4,4,'S','E','T','L',
        5,5,'S','E','T','L','E',         /* 138  */
        5,5,'S','E','T','N','O',
        5,5,'S','E','T','N','S',         /* 140  */
        5,5,'S','E','T','N','Z',
        5,5,'S','E','T','P','E',         /* 142  */
        5,5,'S','E','T','P','O',
        4,4,'S','E','T','O',             /* 144  */
        4,4,'S','E','T','S',
        4,4,'S','E','T','Z',             /* 146  */
        4,4,'S','G','D','T',
        3,3,'S','H','L',                 /* 148  */
        4,4,'S','H','L','D',
        3,3,'S','H','R',                 /* 150  */
        4,4,'S','H','R','D',
        4,4,'S','I','D','T',             /* 152  */
        4,4,'S','L','D','T',
        4,4,'S','M','S','W',             /* 154  */
        3,3,'S','T','C',
        3,3,'S','T','D',                 /* 156  */
        3,3,'S','T','I',
        5,5,'S','T','O','S','B',         /* 158  */
        5,5,'S','T','O','S','D',
        5,5,'S','T','O','S','W',         /* 160  */
        3,3,'S','T','R',
        3,3,'S','U','B',                 /* 162  */
        4,4,'T','E','S','T',
        4,4,'W','A','I','T',             /* 164  */
        4,4,'V','E','R','R',
        4,4,'V','E','R','W',             /* 166  */
        4,4,'X','C','H','G',
        4,4,'X','L','A','T',             /* 168  */
        3,3,'X','O','R',
        4,4,'?','?','?','?',             /* 170  */
        5,5,'B','S','W','A','P',
        7,7,'C','M','P','X','C','H','G', /* 172  */
        4,4,'I','N','V','D',
        6,6,'I','N','V','L','P','G',     /* 174  */
        6,6,'W','B','I','N','V','D',
        4,4,'X','A','D','D',             /* 176  */
        0                   };           /* the terminator  */
 
 
   /*  the following table holds the mnemonics for the 80387
       numerics processor.                                           */
 
CHAR masmm387[] = {
        4,4,'F','A','B','S',
        4,4,'F','A','D','D',                /*   2  */
        5,5,'F','A','D','D','P',
        4,4,'F','B','L','D',                /*   4  */
        5,5,'F','B','S','T','P',
        4,4,'F','C','H','S',                /*   6  */
        5,5,'F','C','L','E','X',
        4,4,'F','C','O','M',                /*   8  */
        5,5,'F','C','O','M','P',
        6,6,'F','C','O','M','P','P',        /*  10  */
        7,7,'F','D','E','C','S','T','P',
        5,5,'F','D','I','S','I',            /*  12  */
        4,4,'F','D','I','V',
        5,5,'F','D','I','V','P',            /*  14  */
        5,5,'F','D','I','V','R',
        6,6,'F','D','I','V','R','P',        /*  16  */
        4,4,'F','E','N','I',
        5,5,'F','F','R','E','E',            /*  18  */
        5,5,'F','I','A','D','D',
        5,5,'F','I','C','O','M',            /*  20  */
        6,6,'F','I','C','O','M','P',
        5,5,'F','I','D','I','V',            /*  22  */
        6,6,'F','I','D','I','V','R',
        4,4,'F','I','L','D',                /*  24  */
        5,5,'F','I','M','U','L',
        7,7,'F','I','N','C','S','T','P',    /*  26  */
        5,5,'F','I','N','I','T',
        4,4,'F','I','S','T',                /*  28  */
        5,5,'F','I','S','T','P',
        5,5,'F','I','S','U','B',            /*  30  */
        6,6,'F','I','S','U','B','R',
        3,3,'F','L','D',                    /*  32  */
        5,5,'F','L','D','C','W',
        6,6,'F','L','D','E','N','V',        /*  34  */
        6,6,'F','L','D','L','G','2',
        6,6,'F','L','D','L','N','2',        /*  36  */
        6,6,'F','L','D','L','2','E',
        6,6,'F','L','D','L','2','T',        /*  38  */
        5,5,'F','L','D','P','I',
        4,4,'F','L','D','Z',                /*  40  */
        4,4,'F','L','D','1',
        4,4,'F','M','U','L',                /*  42  */
        5,5,'F','M','U','L','P',
        6,6,'F','N','C','L','E','X',        /*  44  */
        6,6,'F','N','D','I','S','I',
        5,5,'F','N','E','N','I',            /*  46  */
        6,6,'F','N','I','N','I','T',
        4,4,'F','N','O','P',                /*  48  */
        6,6,'F','N','S','A','V','E',
        6,6,'F','N','S','T','C','W',        /*  50  */
        7,7,'F','N','S','T','E','N','V',
        6,6,'F','N','S','T','S','W',        /*  52  */
        6,6,'F','P','A','T','A','N',
        5,5,'F','P','R','E','M',            /*  54  */
        5,5,'F','P','T','A','N',
        7,7,'F','R','N','D','I','N','T',    /*  56  */
        6,6,'F','R','S','T','O','R',
        5,5,'F','S','A','V','E',            /*  58  */
        6,6,'F','S','C','A','L','E',
        6,6,'F','S','E','T','P','M',        /*  60  */
        5,5,'F','S','Q','R','T',
        3,3,'F','S','T',                    /*  62  */
        5,5,'F','S','T','C','W',
        6,6,'F','S','T','E','N','V',        /*  64  */
        4,4,'F','S','T','P',
        5,5,'F','S','T','S','W',            /*  66  */
        4,4,'F','S','U','B',
        5,5,'F','S','U','B','P',            /*  68  */
        5,5,'F','S','U','B','R',
        6,6,'F','S','U','B','R','P',        /*  70  */
        4,4,'F','T','S','T',
        5,5,'F','W','A','I','T',            /*  72  */
        4,4,'F','X','A','M',
        4,4,'F','X','C','H',                /*  74  */
        7,7,'F','X','T','R','A','C','T',
        5,5,'F','Y','L','2','X',            /*  76  */
        7,7,'F','Y','L','2','X','P','1',
        5,5,'F','2','X','M','1',            /*  78  */
        4,4,'?','?','?','?',
        6,6,'F','F','R','E','E','P',        /*  80  */
        4,4,'F','C','O','S',
        6,6,'F','P','R','E','M','1',        /*  82  */
        4,4,'F','S','I','N',
        7,7,'F','S','I','N','C','O','S',    /*  84  */
        5,5,'F','U','C','O','M',
        6,6,'F','U','C','O','M','P',        /*  86  */
        7,7,'F','U','C','O','M','P','P',
        0                              };   /*  the terminator  */
 
 
 
   /*  mnemonic numbers for each instruction - a value of "odd" in
       this table implies a special case of some sort, and the
       mnemonic is not to be printed automatically without looking
       further                                                      */
 
 
#define odd 0xFFFF                      /*  mnemonic number to cause
                                            special action to be taken  */
 
USHORT masmmnem_16[256] = {
         6, 6, 6, 6, 6, 6, 109, 104,                  /*   0  */
         99, 99, 99, 99, 99, 99, 109, odd,            /*   8  */
         5, 5, 5, 5, 5, 5, 109, 104,                  /*  10  */
         126, 126, 126, 126, 126, 126, 109, 104,      /*  18  */
         7, 7, 7, 7, 7, 7, odd, 30,                   /*  20  */
         162, 162, 162, 162, 162, 162, odd, 31,       /*  28  */
         169, 169, 169, 169, 169, 169, odd, 1,        /*  30  */
         24, 24, 24, 24, 24, 24, odd, 4,              /*  38  */
         40, 40, 40, 40, 40, 40, 40, 40,              /*  40  */
         32, 32, 32, 32, 32, 32, 32, 32,              /*  48  */
         109, 109, 109, 109, 109, 109, 109, 109,      /*  50  */
         104, 104, 104, 104, 104, 104, 104, 104,      /*  58  */
         110, 105, 9, 8, odd, odd, odd, odd,          /*  60  */
         109, 38, 109, 38, 41, 43, 101, 103,          /*  68  */
         62, 59, 49, 56, 66, 61, 55, 48,              /*  70  */
         65, 60, 63, 64, 53, 58, 57, 52,              /*  78  */
         odd, odd, odd, odd, 163, 163, 167, 167,      /*  80  */
         89, 89, 89, 89, odd, 70, odd, odd,           /*  88  */
         97, 167, 167, 167, 167, 167, 167, 167,       /*  90  */
         17, 28, 16, 164, 112, 107, 123, 67,          /*  98  */
         89, 89, 89, 89, 90, 92, 25, 27,              /*  A0  */
         163, 163, 158, 160, 80, 82, 127, 129,        /*  A8  */
         89, 89, 89, 89, 89, 89, 89, 89,              /*  B0  */
         89, 89, 89, 89, 89, 89, 89, 89,              /*  B8  */
         odd, odd, 120, 120, 72, 69, odd, odd,        /*  C0  */
         34, 71, 120, 120, 44, 44, 45, 46,            /*  C8  */
         odd, odd, odd, odd, odd, odd, odd, 168,      /*  D0  */
         odd, odd, odd, odd, odd, odd, odd, odd,      /*  D8  */
         84, 85, 83, 50, 39, 39, 100, 100,            /*  E0  */
         16, 54, 54, 54, 39, 39, 100, 100,            /*  E8  */
         79, odd, odd, odd, 36, 23, odd, odd,         /*  F0  */
         19, 155, 21, 157, 20, 156, odd, odd  };      /*  F8  */
 
USHORT masmmnem_32[256] = {
         6, 6, 6, 6, 6, 6, 109, 104,                  /*   0  */
         99, 99, 99, 99, 99, 99, 109, odd,            /*   8  */
         5, 5, 5, 5, 5, 5, 109, 104,                  /*  10  */
         126, 126, 126, 126, 126, 126, 109, 104,      /*  18  */
         7, 7, 7, 7, 7, 7, odd, 30,                   /*  20  */
         162, 162, 162, 162, 162, 162, odd, 31,       /*  28  */
         169, 169, 169, 169, 169, 169, odd, 1,        /*  30  */
         24, 24, 24, 24, 24, 24, odd, 4,              /*  38  */
         40, 40, 40, 40, 40, 40, 40, 40,              /*  40  */
         32, 32, 32, 32, 32, 32, 32, 32,              /*  48  */
         109, 109, 109, 109, 109, 109, 109, 109,      /*  50  */
         104, 104, 104, 104, 104, 104, 104, 104,      /*  58  */
         111, 106, 9, 8, odd, odd, odd, odd,          /*  60  */
         109, 38, 109, 38, 41, 42, 101, 102,          /*  68  */
         62, 59, 49, 56, 66, 61, 55, 48,              /*  70  */
         65, 60, 63, 64, 53, 58, 57, 52,              /*  78  */
         odd, odd, odd, odd, 163, 163, 167, 167,      /*  80  */
         89, 89, 89, 89, odd, 70, odd, odd,           /*  88  */
         97, 167, 167, 167, 167, 167, 167, 167,       /*  90  */
         29, 18, 16, 164, 113, 108, 123, 67,          /*  98  */
         89, 89, 89, 89, 90, 91, 25, 26,              /*  A0  */
         163, 163, 158, 159, 80, 81, 127, 128,        /*  A8  */
         89, 89, 89, 89, 89, 89, 89, 89,              /*  B0  */
         89, 89, 89, 89, 89, 89, 89, 89,              /*  B8  */
         odd, odd, 120, 120, 72, 69, odd, odd,        /*  C0  */
         34, 71, 120, 120, 44, 44, 45, 47,            /*  C8  */
         odd, odd, odd, odd, odd, odd, odd, 168,      /*  D0  */
         odd, odd, odd, odd, odd, odd, odd, odd,      /*  D8  */
         84, 85, 83, 51, 39, 39, 100, 100,            /*  E0  */
         16, 54, 54, 54, 39, 39, 100, 100,            /*  E8  */
         79, odd, odd, odd, 36, 23, odd, odd,         /*  F0  */
         19, 155, 21, 157, 20, 156, odd, odd  };      /*  F8  */
 
 
   /*  mnemonic numbers for 80 and 81 instructions, for each value
       of the "reg" field of the following byte.                    */
 
USHORT masmmnem8081[24] = {
         6, 99, 5, 126, 7, 162, 169, 24,
         6, 99, 5, 126, 7, 162, 169, 24,
         6, 99, 5, 126, 7, 162, 169, 24  };
 
 
   /*  mnemonic numbers for D4, D5 orders  */
 
USHORT masmmnemAA[2] = { 3, 2 };
 
 
   /*  mnemonic numbers for F6, F7 orders, for reg = 2 to 7,
       and for FE, FF orders, for reg = 0 and reg = 1.        */
 
USHORT masmmnemF6F7[8] = {
         40, 32, 98, 96, 95, 38, 33, 37  };
 
 
   /*  mnemonic numbers for FF orders, reg = 2 to reg = 6  */
 
USHORT masmmnemFF[5] = { 16, 16, 54, 54, 109 };
 
 
 
   /*  mnemonic numbers for shift instructions  */
 
USHORT masmshiftmnem[24] = {
         121, 121, 121,    /*  ROL  */
         122, 122, 122,    /*  ROR  */
         115, 115, 115,    /*  RCL  */
         116, 116, 116,    /*  RCR  */
         148, 148, 148,    /*  SHL  */
         150, 150, 150,    /*  SHR  */
         170, 170, 170,    /*  not used  */
         125, 125, 125 };  /*  SAR  */
 
 
   /*  mnemonic numbers for REP prefix orders  */
 
USHORT masmrepmnem[3] = { 117, 118, 119 };
 
 
   /*  mnemonic numbers for 0F orders  */
 
USHORT masmmnem0F[256] = {
       odd, odd,  68,  86, odd, odd, 22,  odd,    /*  00  */
       173, 175, odd, odd, odd, odd, odd, odd,    /*  08  */
       odd, odd, odd, odd, odd, odd, odd, odd,    /*  10  */
       odd, odd, odd, odd, odd, odd, odd, odd,    /*  18  */
       odd, odd, odd, odd, odd, odd, odd, odd,    /*  20  */
       odd, odd, odd, odd, odd, odd, odd, odd,    /*  28  */
       odd, odd, odd, odd, odd, odd, odd, odd,    /*  30  */
       odd, odd, odd, odd, odd, odd, odd, odd,    /*  38  */
       odd, odd, odd, odd, odd, odd, odd, odd,    /*  40  */
       odd, odd, odd, odd, odd, odd, odd, odd,    /*  48  */
       odd, odd, odd, odd, odd, odd, odd, odd,    /*  50  */
       odd, odd, odd, odd, odd, odd, odd, odd,    /*  58  */
       odd, odd, odd, odd, odd, odd, odd, odd,    /*  60  */
       odd, odd, odd, odd, odd, odd, odd, odd,    /*  68  */
       odd, odd, odd, odd, odd, odd, odd, odd,    /*  70  */
       odd, odd, odd, odd, odd, odd, odd, odd,    /*  78  */
        62,  59,  49,  56,  66,  61,  55,  48,    /*  80  */
        65,  60,  63,  64,  53,  58,  57,  52,    /*  88  */
       144, 139, 133, 132, 146, 141, 134, 131,    /*  90  */
       145, 140, 142, 143, 137, 136, 138, 135,    /*  98  */
       109, 104, odd,  12, 149, 149, 172, 172,    /*  A0  */
       109, 104, odd,  15, 151, 151, odd,  38,    /*  A8  */
       odd, odd,  87,  14,  73,  75,  94,  94,    /*  B0  */
       odd, odd, odd,  13,  10,  11,  93,  93,    /*  B8  */
       176, 176, odd, odd, odd, odd, odd, odd,    /*  C0  */
       odd, odd, odd, odd, odd, odd, odd, odd,    /*  C8  */
       odd, odd, odd, odd, odd, odd, odd, odd,    /*  D0  */
       odd, odd, odd, odd, odd, odd, odd, odd,    /*  D8  */
       odd, odd, odd, odd, odd, odd, odd, odd,    /*  E0  */
       odd, odd, odd, odd, odd, odd, odd, odd,    /*  E8  */
       odd, odd, odd, odd, odd, odd, odd, odd,    /*  F0  */
       odd, odd, odd, odd, odd, odd, odd, odd };  /*  F8  */
 
 
   /*  mnemonic numbers for 0F 00 orders  */
 
USHORT masmmnem0F00[6] = {
         153, 161, 77, 88, 165, 166 };
 
 
   /*  mnemonic numbers for 0F BA orders  */
 
USHORT masmmnem0FBA[4] = { 12, 15, 14, 13 };
 
 
   /*  mnemonic numbers for 0F 01 orders  */
 
USHORT masmmnem0F01[8] = {
         147, 152, 74, 76, 154, 170, 78, 174 };
 
 
   /*  the following table is the mnemonic table for memory access
       387 instructions                                              */
 
USHORT masmm387mem[64] = {
     2, 42,  8,  9, 67, 69, 13, 15,     /*   0  (D8)  */
    32, 79, 62, 65, 34, 33, 64, 63,     /*   8  (D9)  */
    19, 25, 20, 21, 30, 31, 22, 23,     /*  10  (DA)  */
    24, 79, 28, 29, 79, 32, 79, 65,     /*  18  (DB)  */
     2, 42,  8,  9, 67, 69, 13, 15,     /*  20  (DC)  */
    32, 79, 62, 65, 57, 79, 58, 66,     /*  28  (DD)  */
    19, 25, 20, 21, 30, 31, 22, 23,     /*  30  (DE)  */
    24, 79, 28, 29,  4, 24,  5, 29  };  /*  38  (DF)  */
 
 
   /*  the following table is the mnemonic table for 387
       instructions with a reg field                                 */
 
USHORT masmm387reg[64] = {
     2, 42,   8,   9,  67,  69,  13,  15,    /*   0  (D8)  */
    32, 74, odd,  65, odd, odd, odd, odd,    /*   8  (D9)  */
    79, 79,  79,  79,  79,  79,  79,  79,    /*  10  (DA)  */
    79, 79,  79,  79,  79,  79,  79,  79,    /*  18  (DB)  */
     2, 42,   8,   9,  67,  69,  13,  15,    /*  20  (DC)  */
    18, 74,  62,  65,  85,  86,  79,  79,    /*  28  (DD)  */
     3, 43,   9, odd,  70,  68,  14,  16,    /*  30  (DE)  */
    80, 74,  65,  65, odd,  79,  79,  79 };  /*  38  (DF)  */
 
 
   /*  the following table is the mnemonic table for 387
       orders D9, mod = 3, regf = 4-7                                */
 
USHORT masmmnemD94[32] = {
     6,  1, odd, odd, 71, 73, odd, odd,
    41, 38,  37,  39, 35, 36,  40, odd,
    78, 76,  55,  53, 75, 82,  11,  26,
    54, 77,  61,  84, 56, 59,  83,  81  };
 
 
   /*  the following table is the mnemonic table for 387
       orders DB, mod = 3, regf = 4                                  */
 
USHORT masmmnemDB4[8] = {
    17, 12, 7, 27, 60, odd, odd, odd  };
 
 
 
 /********************************************************************
 *                                                                   *
 *    The following are structures to enable use to choose either    *
 *    16-bit or 32-bit dis-assembly when generating ASM/86 format    *
 *    instructions                                                   *
 *                                                                   *
 ********************************************************************/
 
 
struct {
  CHAR *p1[2];
  USHORT *p2[16];
  USHORT w1[19];
  CHAR m;
} MASM_16   = {
                masmmnem,
                masmm387,
                masmmnem_16,
                masmmnem8081,
                masmmnemAA,
                0,                          /*  C6C7 not used for MASM  */
                masmmnemF6F7,
                masmmnemFF,
                masmshiftmnem,
                masmrepmnem,
                masmmnem0F00,
                masmmnem0F01,
                masmm387mem,
                masmm387reg,
                masmmnemD94,
                masmmnemDB4,
                masmmnem0F,
                masmmnem0FBA,
                35,                       /*  index of mnemonic ESCAPE  */
                104,                         /*  index of mnemonic POP  */
                104,                    /*  index of mnemonic POP word  */
                104,                   /*  index of mnemonic POP dword  */
                130,                       /*  index of mnemonic SEGOV  */
                89,                            /*  index of mnemonic L  */
                89,                           /*  index of mnemonic ST  */
                170,              /*  index of mnemonic ???? (illegal)  */
                163,                          /*  index of mnemonic TI  */
                163,                         /*  index of mnemonic TIB  */
                163,                         /*  index of mnemonic TIW  */
                109,                        /*  index of mnemonic PUSH  */
                171,                       /*  index of mnemonic BSWAP  */
                1,                      /*  index of 387 mnemonic FABS  */
                48,                     /*  index of 387 mnemonic FNOP  */
                10,                   /*  index of 387 mnemonic FCOMPP  */
                87,                  /*  index of 387 mnemonic FUCOMPP  */
                66,                    /*  index of 387 mnemonic FSTSW  */
                79,           /*  index of 387 mnemonic ???? (illegal)  */
                1     };           /*  set to 1 if this is MASM format  */
 
 
struct {
  CHAR *p1[2];
  USHORT *p2[16];
  USHORT w1[19];
  CHAR m;
} MASM_32   = {
                masmmnem,
                masmm387,
                masmmnem_32,
                masmmnem8081,
                masmmnemAA,
                0,                          /*  C6C7 not used for MASM  */
                masmmnemF6F7,
                masmmnemFF,
                masmshiftmnem,
                masmrepmnem,
                masmmnem0F00,
                masmmnem0F01,
                masmm387mem,
                masmm387reg,
                masmmnemD94,
                masmmnemDB4,
                masmmnem0F,
                masmmnem0FBA,
                35,                       /*  index of mnemonic ESCAPE  */
                104,                         /*  index of mnemonic POP  */
                104,                    /*  index of mnemonic POP word  */
                104,                   /*  index of mnemonic POP dword  */
                130,                       /*  index of mnemonic SEGOV  */
                89,                            /*  index of mnemonic L  */
                89,                           /*  index of mnemonic ST  */
                170,              /*  index of mnemonic ???? (illegal)  */
                163,                          /*  index of mnemonic TI  */
                163,                         /*  index of mnemonic TIB  */
                163,                         /*  index of mnemonic TIW  */
                109,                        /*  index of mnemonic PUSH  */
                171,                       /*  index of mnemonic BSWAP  */
                1,                      /*  index of 387 mnemonic FABS  */
                48,                     /*  index of 387 mnemonic FNOP  */
                10,                   /*  index of 387 mnemonic FCOMPP  */
                87,                  /*  index of 387 mnemonic FUCOMPP  */
                66,                    /*  index of 387 mnemonic FSTSW  */
                79,           /*  index of 387 mnemonic ???? (illegal)  */
                1     };           /*  set to 1 if this is MASM format  */
