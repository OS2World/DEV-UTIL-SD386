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
_DATA   SEGMENT  DWORD USE32 PUBLIC 'DATA'
        extrn    VideoPtr:dword  ;Pointer to logical video buffer
        extrn    VideoMap:dword  ;Pointer to logical to physical attr vector
        extrn    VideoCols:word  ;# of columns per screen row
        extrn    VideoAtr:byte   ;default logical video attribute
        align 4
_DATA      ENDS
EXTRN   Vio32ShowBuf:near
PUBLIC  putrcx                  ; Fix No 400
_TEXT   SEGMENT  DWORD USE32 PUBLIC 'CODE'

;------------------------------------------------------------------------------;
; name          putrc -- Display at Row,Col
;
; synopsis      putrcx( row, col, string );
;
;               uint row;       /* row (0..N)                   */
;               uint col;       /* column (0..N)                */
;               uchar *string;  /* string of attributes and chars  */
;
; description   This routine puts a string at the specified row and column.
;               Bytes in the string are interpreted depending on their value:
;
;               0x00            End of string
;               0x01 .. 0x7F    Literal character
;               0x80 .. 0xFE    Logical display attribute (0x80 bit is turned
;                               off).  The value 0x80 is a special case, and
;                               uses the existing attributes on the screen.
;               0xFF            Repeat count byte follows this byte (0..254).
;                               The byte following the repeat count may be a
;                               a literal character or display attribute (the
;                               characters on the screen are not changed).
;                               A repeat count of 255 is a special case, and
;                               copies the following character to the display
;                               regardless of it's value (literal escape).
;
;              Note :: The filter through which putrcx gets called can't
;                      handle multiple rows at a time. So putrcx should
;                      be called seperately for displaying each row.
;
;* History:                                                                  *;
;*                                                                           *;
;*   02/08/91 Creation of 32-bit SD86, from 16-bit version.                  *;
;*                                                                           *;
;*...Release 1.00 (Pre-release 107 11/13/91)                                 *;
;*...                                                                        *;
;*... 11/13/91  400   Srinivas  Vertical Register Display.                   *;
;*...                                                                        *;
;-----------------------------------------------------------------------------;

EOT = 0
TAB = 9
ATR = 80h
MAXLATR = 27                       ;max value for logical attribute

row    = 8
col    = 12
string = 16

putrcx  PROC NEAR                 ; Fix No 400
        push    ebp
        mov     ebp,esp
        push    esi
        push    edi
        push    eax
        push    ebx
        push    ecx
        pushfd
        push    es                 ;save flat address
        cld

        xor     edi, edi           ;clear working reg
        xor     eax, eax           ;clear working reg
        xor     ecx, ecx           ;clear working reg
        les     di,  VideoPtr
        mov     ax,  VideoCols     ;ax = # of cols per screen
        mul     byte ptr [ebp+row]
        add     ax,  [ebp+col]
        shl     ax, 1
        add     di, ax             ;es:di -> starting video (attr,char)
        push    ax                 ;save starting video buffer offset
        push    di                 ;save starting video buffer address

        mov     al,  VideoAtr      ;al = default video attribute
        mov     ebx, VideoMap      ;ds:bx -> logical to physical attr map
        xlat                       ;change al from logical to physical attr
        mov     ah, al             ;ah = default hardware video attribute

        mov     esi, [ebp+string]  ;ds:si -> string
        jmp     short rc30
rc10:
        or      al, ATR            ;assume graphics char
rc20:
        stosw
rc30:
        lodsb
        test    al, ATR
        jnz     rc40
        cmp     al, EOT
        jne     rc20
rcxt:
        pop     ax                 ;starting video buffer adddress
        sub     di, ax             ;di = # of bytes changed
        pop     ax                 ;starting screen buffer offset
        pop     es                 ;restore flat address reg

        push    00                 ;3rd parm to Vio16ShowBuf
        push    edi                ;2nd parm to Vio16ShowBuf
        push    eax                ;1st parm to Vio16ShowBuf
        call    Vio32ShowBuf

        add     esp,0Ch

        popfd
        pop     ecx
        pop     ebx
        pop     eax
        pop     edi
        pop     esi
        leave
        ret
rc40:
        cmp     al, 0FFh
        je      rc50               ;jump if repeat count
        and     al, not ATR
        jz      rc45               ;jump if zero (special case)
        cmp     al, MAXLATR
        ja      rc10               ;jump if too big for logical attr

        mov     ebx, VideoMap      ;ds:bx -> logical to physical attr map
        xlat                       ;change al from logical to physical attr
        mov     ah, al             ;ah = hardware video attribute
        jmp     rc30
rc45:
        lodsb
        test    al, ATR
        jnz     rc40
        cmp     al, EOT
        je      rcxt
        stosb                      ;change data char only
        inc     di                 ;leave attribute unchanged
        jmp     rc45
rc50:
        lodsb                      ;al = repeat count
        cmp     al, 0FFh           ;if literal escape
        je      rc80               ;then jump (special case)
        mov     cl, al
        xor     ch, ch
        lodsb                      ;char to repeat
        test    al, ATR
        jnz     rc60
        rep     stosw
        jmp     rc30
rc55:
        or      al, ATR            ;assume graphics char
        rep     stosw
        jmp     rc30
rc60:
        and     al, not ATR
        cmp     al, MAXLATR
        ja      rc55               ;jump if too big for logical attr

        mov     ebx, VideoMap      ;ds:bx -> logical to physical attr map
        xlat                       ;change al from logical to physical attr
        jcxz    lpend
rc70:
        inc     di                 ;leave data char unchanged
        stosb                      ;change attribute only
        loop    rc70
lpend:
        jmp     rc30               ;loop on next char (default attr same)
rc80:
        lodsb                      ;al = literal character
        jmp     rc20

putrcx  endp                       ; Fix No 400

_TEXT   ends
        end
