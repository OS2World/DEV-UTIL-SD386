        page    ,132
        title   Copyright (C) 1988 IBM Corp (Written 11/87 by Jim Christensen)
        subttl  Byte Manipulation Routines
        name    BYTES


        .386
_TEXT   SEGMENT  DWORD USE32 PUBLIC 'CODE'
_TEXT      ENDS
_DATA   SEGMENT  DWORD USE32 PUBLIC 'DATA'
_DATA      ENDS
        ASSUME   CS: FLAT, DS: FLAT, SS: FLAT, ES: FLAT
PUBLIC  bindex
PUBLIC  windex
PUBLIC  lindex
_TEXT   SEGMENT  DWORD USE32 PUBLIC 'CODE'
;------------------------------------------------------------------------------;
; name          bindex -- byte index
;
; synopsis      n = bindex( cp, len, c );
;               int n;                  /* 0 to len-1 (index), or len */
;               char *cp;               /* char array to search */
;               int len;                /* # of chars in array (cp) */
;               int c;                  /* char to search for */
;
; description   This function returns the index into the char array cp,
;               of the first occurance of the char specified by c, or
;               len if c does not occur in cp.  If the 0x8000 bit of c
;               is set, then the function returns the index of the first
;               char that is NOT c.
;------------------------------------------------------------------------------;

cp_parm = 8
len_parm = 12
c_parm = 16

bindex  PROC NEAR
        push    ebp
        mov     ebp,esp
        push    edi
        push    ecx

        mov     edi, [ebp+cp_parm]
        mov     ecx, [ebp+len_parm]
        mov     eax, [ebp+c_parm]
        jecxz   bi20

        or      ax, ax
        js      bi30

        repne   scasb
        jne     bi20
bi00:
        xchg    eax, ecx
        sub     eax, [ebp+len_parm]       ;ax = (-1 to -len)
        not     eax                      ;ax = (1 to len) - 1
bi10:
        pop     ecx
        pop     edi
        leave
        ret
bi20:
        mov     eax, [ebp+len_parm]     ;return value for not found
        pop     ecx
        pop     edi
        leave
        ret
bi30:
        repe    scasb
        je      bi20
        jmp     bi00

bindex  endp

;------------------------------------------------------------------------------;
; name          windex -- word index
;
; synopsis      n = windex( ip, ilen, i );
;               int n;                  /* 0 to ilen-1 (index), or len */
;               int *ip;                /* int array to search */
;               int ilen;               /* # of ints in array (ip) */
;               int i;                  /* int to search for */
;
; description   This function returns the index into the int array ip,
;               of the first occurance of the int specified by i, or
;               ilen if i does not occur in ip.
;------------------------------------------------------------------------------;

ip_parm = 8
ilen_parm = 12
i_parm = 16

windex  PROC NEAR
        push    ebp
        mov     ebp,esp
        push    edi
        push    ecx

        mov     edi, [ebp+ip_parm]
        mov     ecx, [ebp+ilen_parm]
        mov     eax, [ebp+i_parm]
        jecxz   wi20

        repne   scasw
        jne     wi20
        xchg    eax, ecx
        sub     eax, [ebp+ilen_parm]      ;eax = (-1 to -ilen)
        not     eax                                 ;eax = (1 to ilen) - 1
wi10:
        pop     ecx
        pop     edi
        leave
        ret
wi20:
        mov     eax, [ebp+ilen_parm]      ;return value for not found
        pop     ecx
        pop     edi
        leave
        ret

windex  endp


;------------------------------------------------------------------------------;
; name          lindex -- long index
;
; synopsis      n = lindex( lptr, len, item );
;               int n;                  /* 0 to len-1 (index), or len */
;               long *lptr;             /* long array to search */
;               int len;                /* # of longs in array (lptr) */
;               long item;              /* long to search for */
;
; description   This function returns the index into the long array lptr,
;               of the first occurance of the long specified by item, or
;               len if item does not occur in lptr.
;------------------------------------------------------------------------------;

lptr_parm = 8
len_parm = 12
item_parm = 16

lindex  PROC NEAR
        push    ebp
        mov     ebp,esp
        push    edi
        push    ecx

        mov     edi, [ebp+lptr_parm]
        mov     ecx, [ebp+len_parm]
        mov     eax, [ebp+item_parm]
        jecxz   li20

        repne   scasd
        jne     li20
        xchg    eax, ecx
        sub     eax, [ebp+len_parm]      ;eax = (-1 to -ilen)
        not     eax                                ;eax = (1 to ilen) - 1
li10:
        pop     ecx
        pop     edi
        leave
        ret
li20:
        mov     eax, [ebp+len_parm]      ;return value for not found
        pop     ecx
        pop     edi
        leave
        ret

lindex  endp


_TEXT   ends
        end
