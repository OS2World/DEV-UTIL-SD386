        page    ,132
        title   Variable Length String Match Routine
        subttl  Copyright (C) 1988 by IBM (J.E.Christensen)
        name    FINDVSTR

        .386

_TEXT   SEGMENT  DWORD USE32 PUBLIC 'CODE'
_TEXT      ENDS
_DATA   SEGMENT  DWORD USE32 PUBLIC 'DATA'
        align 4
_DATA      ENDS
        ASSUME   CS: FLAT, DS: FLAT, SS: FLAT, ES: FLAT
PUBLIC  FindVstr
_TEXT   SEGMENT  DWORD USE32 PUBLIC 'CODE'
;------------------------------------------------------------------------------;
; name          findvstr -- String matching
;
; synopsis      n = findvstr( sptr, vstrs, nvstrs );
;               int n;          /* index of matching varstr (1..N), or zero */
;               varstr *sptr;   /* variable length string to find (len,chars) */
;               varstr *vstrs;  /* concatenation of varstrs to match against */
;               int nvstrs;     /* # of variable length strings */
;
; description   This function returns the index (1..N) of the matching varstr,
;               or zero if no match is found.
;
; assumptions   Pointers are 32-bits, and a near (32) call was used.
;------------------------------------------------------------------------------;

sptr      =  8
vstrs     = 12
nvstrs    = 16

FindVstr  PROC NEAR
        push    EBP
        mov     EBP,ESP
        push    ESI
        push    EDI
        push    EBX
        push    ECX
        push    EDX

        mov     ECX, [EBP+nvstrs]
        jcxz    fs30                    ;jump if no varstrs to match

;       les     EBX, [EBP+sptr]         ;es:Ebx -> pattern
        mov     EBX, [EBP+sptr]         ;es:Ebx -> pattern          209
        mov     ah,es:[EBX]             ;ah = length of pattern
        inc     EBX                     ;es:EBX -> 1st char of pattern

;       lds     EDX, [EBP+vstrs]        ;ds:EDX -> 1st varstr to match against
        mov     EDX, [EBP+vstrs]        ;ds:EDX -> 1st varstr to match     209
fs10:
        mov     ESI, EDX                ;ds:ESI -> next varstr to match
        lodsb                           ;al = length of varstr to match
        cmp     ah, al                  ;if same length as pattern
        je      fs40                    ;then jump
fs20:
        inc     EDX
        PUSH    EBX
        MOVZX   EBX, AL
        ADD     EDX, EBX
        POP     EBX

        loop    fs10
fs30:
        xchg    EAX, ECX
        pop     EDX
        pop     ECX
        pop     EBX
        pop     EDI
        pop     ESI
        pop     EBP
        ret
fs40:
        push    ECX
        XOR     ECX,ECX
        mov     cl, al
        mov     EDI, EBX
        repe    cmpsb
        pop     ECX
        jne     fs20

        sub     ECX, [EBP+nvstrs]
        neg     ECX                      ;cx = 0 to nvstrs-1
        inc     ECX                      ;cx = 1 to nvstrs
        jmp     fs30
FindVstr endp

_TEXT   ends
        end
