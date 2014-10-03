        page    ,132
        title   Text Encode/Decode Routines
        subttl  Copyright (C) 1988 by IBM (Written 5/88 by Jim Christensen)
        name    ENCODE

        .386

_TEXT   SEGMENT  DWORD USE32 PUBLIC 'CODE'
_TEXT      ENDS
_DATA   SEGMENT  DWORD USE32 PUBLIC 'DATA'
        align 4
_DATA      ENDS
        ASSUME   CS: FLAT, DS: FLAT, SS: FLAT, ES: FLAT
PUBLIC  Encode
PUBLIC  Decode
_TEXT SEGMENT  DWORD USE32 PUBLIC 'CODE'

TAB             equ 009h
SPACE           equ 020h
LOWZONE         equ 010h
HIGHZONE        equ 0A0h
MAXPERRUN       equ 256-HIGHZONE-1

;------------------------------------------------------------------------------;
; name          Encode --
;
; synopsis      n = Encode( LineOfText, OutputBuffer, LineLength );
;
;               uint n;                 /* # bytes returned in OutputBuffer */
;               char *LineOfText;       /* input line of text */
;               char *OutputBuffer;     /* output buffer */
;               ushort LineLength;      /* # bytes given in LineOfText */
;
; description   This routine
;
; assumptions   The OutputBuffer is large enough to hold the encoded line.
;------------------------------------------------------------------------------;

;       EAX = work,  EBX = work,  ECX = input length,  EDX = current column
;       ds:ESI = input string,  es:EDI = output buffer

LineOfText   = 8
OutputBuffer = 12
LineLength   = 16

Encode  PROC    NEAR
        enter   0,0
        push    ESI
        push    EDI
        push    EBX
        push    ECX
        push    EDX

        mov     ESI, [EBP+LineOfText]   ;ds:ESI -> LineOfText
        mov     EDI, [EBP+OutputBuffer] ;es:EDI -> OutputBuffer
        mov     ECX, [EBP+LineLength]
        xor     EDX, EDX                ;current column
        jmp     short Restart

GotTab0:
        mov     al, SPACE
        mov     EBX, EDX
        sub     EBX, 7                  ;EBX = (dx + 1) - 8
        neg     EBX
        and     EBX, 7                  ;EBX = # of additional spaces
        jz      IgnoreTab0

        dec     ECX                      ;count tab char
        add     EBX, ECX
        dec     EBX
        or      ECX, ECX                 ;set ZF=1 iff ECX=0
        jmp     Rescan

;       Get the next char into AL.  There is no previous char in AH.
Restart:
        jcxz    Fini
        lodsb
        cmp     al, TAB
        je      GotTab0

IgnoreTab0:
        xchg    al, ah
        dec     ECX
        jz      Done

;       Get the next char into AL.  AH contains the previous
;       char, and has not yet been written to the output buffer.
NextByte:
        lodsb                           ;(ah,al) = (prev,next) chars
        cmp     al, TAB
        je      GotTab

IgnoreTab:
        cmp     al, ah
        je      GotRun

        xchg    al, ah                  ;(ah,al) = (next,prev) chars
        cmp     al, HIGHZONE
        jae     EscapeChar

OutputChar:
        stosb
        inc     EDX

        loop    NextByte

Done:
        xchg    al, ah
        cmp     al, HIGHZONE
        jae     EscapeLast
        stosb
Fini:
        xchg    EAX, EDI
        sub     EAX, [EBP+OutputBuffer] ;EAX = # of bytes put into output buffer

        pop     EDX
        pop     ECX
        pop     EBX
        pop     EDI
        pop     ESI
        leave
        ret

EscapeChar:
        mov     byte ptr es:[EDI], HIGHZONE + 1
        inc     EDI
        jmp     OutputChar

EscapeLast:
        xchg    al, ah
        mov     al, HIGHZONE + 1
        stosw
        jmp     Fini

GotTab:
        mov     al, SPACE
        mov     EBX, EDX
        sub     EBX, 6                  ;EBX = (dx + 2) - 8
        neg     EBX
        and     EBX, 7                  ;EBX = # of additional spaces
        jz      IgnoreTab

        dec     ECX                     ;count tab char
        add     EBX, ECX
        cmp     ah, SPACE
        je      Rescan
        dec     EBX

        xchg    al, ah
        cmp     al, HIGHZONE
        jae     EscapePreTab
OutputPreTab:
        stosb
        inc     EDX

        xchg    al, ah
        or      ECX, ECX                ;set ZF=1 iff ECX=0
        jmp     short Rescan

EscapePreTab:
        mov     byte ptr es:[EDI], HIGHZONE + 1
        inc     EDI
        jmp     OutputPreTab

GotRun:
        dec     ECX                      ;count 2nd char of run (ZF=1 iff ECX=0)
        mov     EBX, ECX

;  Here ECX = # bytes togo, EBX-ECX = (# in run) - 2, EDX = column before run
Rescan:
        xchg    ESI, EDI
        repe    scasb                   ;if ECX=0 initially, then ZF=1 required
        xchg    ESI, EDI

        je      EndRun                  ;jump if scan ended on equal comapre
        cmp     al, SPACE
        jne     BackUp
        cmp     byte ptr [ESI-1], TAB
        jne     BackUp

        mov     EAX, EBX
        sub     EAX, ECX  ;EAX = # of chars after 1st two including tab
        add     EAX, EDX  ;EAX = current column - 2 counting tab for 1 space
        sub     EAX, 6
        neg     EAX
        and     EAX, 7                   ;EAX = # of additional spaces
        add     EBX, EAX                 ;backup start of run by that #
        mov     EAX, SPACE + 256*SPACE
        or      ECX, ECX                 ;set ZF=1 iff ECX=0
        jmp     Rescan
BackUp:
        dec     ESI
        inc     ECX
EndRun:
        sub     EBX, ECX               ;EBX = # of matching chars after 1st two
        add     EBX, 2
        add     EDX, EBX
ChkMax:
        cmp     EBX, MAXPERRUN
        jbe     LastRun

        mov     al, HIGHZONE + MAXPERRUN
        stosb
        mov     al, ah
        stosb
        sub     EBX, MAXPERRUN
        jmp     ChkMax
LastRun:
        add     bl, HIGHZONE
        xchg    EAX, EBX
        stosb
        xchg    EAX, EBX
        stosb
        jmp     Restart
Encode  endp


;------------------------------------------------------------------------------;
; name          Decode --
;
; synopsis      n = Decode( EncodedText, OutputBuffer );
;
;               uint n;                 /* # bytes returned in OutputBuffer */
;               char *EncodedText;      /* input encoded line */
;               char *OutputBuffer;     /* output buffer */
;
; description   This routine
;
; assumptions   The decoded text is truncated after 255 characters.
;------------------------------------------------------------------------------;

EncodedText  = 8
OutputBuffer = 12

Decode  PROC    NEAR
        enter   0,0
        push    ESI
        push    EDI
        push    ECX
        push    EDX

        mov     ESI, [EBP+EncodedText]
        mov     EDI, [EBP+OutputBuffer]
        mov     ECX, 255         ;max # bytes for output buffer
dc10:
        lodsb
        cmp     al, LOWZONE
        jb      dc20
        cmp     al, HIGHZONE
        jae     dc30
        stosb
        loop    dc10
dc20:
        xchg    EAX, EDI
        sub     EAX, [EBP+OutputBuffer]  ;EAX=# of bytes put into output buffer

        pop     EDX
        pop     ECX
        pop     EDI
        pop     ESI
        leave
        ret
dc30:
        sub     al, HIGHZONE
        MOVSX   EAX, AL         ; sign extend "HIGHZONE"         107
        sub     ECX, EAX
        jbe     dc40
        xchg    EAX, ECX        ;ECX = repeat count
        xchg    EAX, EDX        ;EDX = residual buffer count
        lodsb                   ;al = repeat char
        rep     stosb
        mov     ECX, EDX
        jmp     dc10
dc40:
        add     EDX, EAX        ;ECX = residual buffer count
        lodsb                   ;al = repeat char
        rep     stosb
        jmp     dc20
Decode  endp

_TEXT   ends
        end
