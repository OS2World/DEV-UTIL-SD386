        page    ,132
        title   Assorted Assembler Routines
        subttl  Copyright (C) 1988 by IBM (Written 11/87 by Jim Christensen)
        name    MISC

        .386

        extrn   KBD16FLUSHBUFFER:FAR      ;routine flushes key stroke buffer
        extrn   KBD16CHARIN:FAR

_TEXT   SEGMENT  DWORD USE32 PUBLIC 'CODE'
_TEXT      ENDS
_DATA   SEGMENT  DWORD USE32 PUBLIC 'DATA'
        align 4
_DATA      ENDS
        ASSUME   CS: FLAT, DS: FLAT, SS: FLAT, ES: FLAT
PUBLIC  utoa
_TEXT   SEGMENT  DWORD USE32 PUBLIC 'CODE'
Int3    PROC    NEAR
        int     3
        ret 0
Int3    endp

if 0
;------------------------------------------------------------------------------;
; name          akeycod - addresses our stubbed in key code for user's screen
;                         group.
;
; synopsis      p = akeycod( &codlen );
;
;               uchar *p;               /* ->address of code to stub in      */
;               uchar codlen;           /* length of code to stub in         */
;
; description   This routine sets up instructions that will get a key stroke
;               in the user's screen group putting that key stroke in his data
;               area.  We will stub this code in at his current ExecCSIP.
;------------------------------------------------------------------------------;

Func    akeycod addrclen, 4
        push    bp                      ;save bp register
        mov     bp, sp                  ;bp address stack
        sub     sp, 4                   ;allocate work area
        jmp     skpstb                  ;do not execute the stub code!

stub:   push    0                       ;for keyboard handle 0
        call    KBD16FLUSHBUFFER        ;flush key stroke buffer (handle 0)
        push    ds                      ;push ds:0 == where key is to be stored
        push    0                       ;  in user's data area
        push    0                       ;0=>wait until a char is available
        push    0                       ;for keyboard handle 0
        call    KBD16CHARIN             ;get a keystroke (handle 0)
        int     3                       ;gets us back to SD86 from app scr grp
stublen EQU     $-stub                  ;length of stub code

skpstb: push    di                      ;protect di register
        les     di, [bp+addrclen]       ;es:di->code length variable (uchar)
        mov     byte ptr es:[di], stublen ;store length of code in codlen
        pop     di                      ;restore di register

        lea     ax, stub                ;offset of code stub
        mov     dx, cs                  ;segment of code stub (p == dx:ax)
        add     sp, 4                   ;get rid of work area
        pop     bp                      ;restore bp register
eFunc   akeycod ret                     ;return to caller
endif

;------------------------------------------------------------------------------;
; name          utoa -- Unsigned to ASCII
;
; synopsis      n = utoa( num, buf );
;
;               uint n;                 /* # of bytes in ASCII result */
;               uint num;               /* unsigned number to be formatted */
;               uchar *buf;             /* buffer for ASCII string */
;
; description   This routine converts a binary, unsigned number into an ASCII
;               string.  The length of the ASCII string is returned.
;------------------------------------------------------------------------------;

num    = 8
buf    = 12

utoa    PROC    NEAR
        push    EBP
        mov     EBP,ESP
        push    EDI
        push    EBX
        push    ECX
        push    EDX

        sub     ESP, 12                  ;allocate work area
        mov     EDI,ESP
        push    ss
        pop     es                       ;es:edi -> work area
        mov     ECX, 10
        mov     EAX, [EBP+num]
ut10:
        xor     EDX, EDX
        div     ECX
        xchg    EAX, EDX
        add     al, '0'
        stosb
        xchg    EAX, EDX
        or      EAX, EAX
        jnz     ut10

        mov     EBX, EDI
        mov     ECX, EDI
        sub     ECX, ESP                 ;ECX = # of chars in string
        push    ECX
;       les     EDI, [EBP+buf]           ;es:EDI -> buf
        mov     EDI, [EBP+buf]           ;es:EDI -> buf
ut20:
        dec     EBX
        mov     al, ss:[EBX]
        stosb
        loop    ut20

        pop     EAX
        add     ESP, 12
        pop     EDX
        pop     ECX
        pop     EBX
        pop     EDI
        pop     EBP
        ret
utoa    endp

_TEXT   ends
        end
