        page    64,120
        title   Video Interface Routine
        subttl  Copyright (C) 1988 by IBM (J.E.Christensen)
        name    PUTUP

        .386
_TEXT   SEGMENT  DWORD USE32 PUBLIC 'CODE'
_TEXT      ENDS
_DATA   SEGMENT  DWORD USE32 PUBLIC 'DATA'
_DATA      ENDS
        ASSUME   CS: FLAT, DS: FLAT, SS: FLAT, ES: FLAT
_DATA   SEGMENT  DWORD USE32 PUBLIC 'DATA'
        extrn    VideoPtr:dword  ;Pointer to logical video buffer
        extrn    VideoMap:dword  ;Pointer to logical to physical attr vector
        extrn    BoundPtr:dword  ;Pointer to screen bounds.                 400
        extrn    VideoCols:word  ;# of columns per screen row
        align 4
_DATA      ENDS
EXTRN   Vio32ShowBuf:near
PUBLIC  putup
PUBLIC  putxb
_TEXT   SEGMENT  DWORD USE32 PUBLIC 'CODE'

LOWZONE         equ 010h
HIGHZONE        equ 0A0h

;------------------------------------------------------------------------------;
; name          putup -- Format Screen
;
; synopsis      putup( buffer, ptrs, top, nlines, column, attrs );
;
;               char   *buffer;   /* buffer of ASCIIZ strings             */
;               ushort ptrs[];    /* vector of buffer offsets             */
;               uint   top;       /* 1st line of screen to use (0..N)     */
;               uint   nlines;    /* # of lines to show                   */
;               uint   column;    /* # of chars to skip each line         */
;               uchar  attrs[];   /* vector of line logical attributes    */
;
; description   This routine formats the display screen with text.
;
;*...Release 1.00 (Pre-release 107 11/13/91)                                 *;
;*...                                                                        *;
;*... 11/13/91  400   Srinivas  Vertical Register Display.                   *;
;*...                                                                        *;
;-----------------------------------------------------------------------------;

buffer = 8
ptrs   = 12
top    = 16
nlines = 20
column = 24
attrs  = 28

putup   PROC NEAR
        push    ebp
        mov     ebp,esp
        push    esi
        push    edi
        push    edx
        push    ecx
        push    ebx
        push    eax
        pushfd
        push    es                 ;save flat address
        cld

                                        ;fix it up.
pu10:
        xor     edi, edi           ;clear working reg
        xor     eax, eax           ;clear working reg
        xor     edx, edx           ;clear working reg
        xor     ecx, ecx           ;clear working reg
        les     di,  VideoPtr
        mov     ax,  VideoCols     ;ax = # of cols per screen
        mul     byte ptr [ebp+top]
        shl     ax,  1
        add     di,  ax             ;es:di -> starting video (attr,char)
        push    ax                 ;save starting video buffer offset
        push    di                 ;save starting video buffer address
pu20:
        mov     esi, [ebp+ptrs]           ;ds:si -> offset vector
        lodsw                           ;ax = next element of offset vector
        mov     [ebp+ptrs], esi         ;update pointer
        xchg    eax,  edx                  ;dx = buffer offset to next line

        mov     esi, [ebp+attrs]          ;ds:si -> attribute vector
        lodsb                           ;al = next element of attribute vector
        mov     [ebp+attrs], esi         ;update pointer

        mov     ebx, BoundPtr         ;pointer to screen bounds           400
        add     ebx, [ebp+top]        ;position to correct line           400
        mov     cl, byte ptr [ebx]    ;cx = # of cols to be painted       400
        push    cx                    ;save the # of cols                 400

        mov     ebx, VideoMap         ;ds:bx -> logical to physical attr map
        xlat                            ;change al from logical to physical attr
        mov     ah,  al                  ;ah = harware video attribute

        mov     esi, [ebp+buffer]         ;ds:si -> start of text buffer
        add     esi, edx                  ;ds:si -> start of next line

        mov     bx, [ebp+column]         ;bx = starting test column

        cmp     cx,0                    ;if number of cols to write is 0  701
        jz      skipit                  ;then don't bother to call.

        call    putone                  ;put one line up on the display
skipit:
        xor     ebx, ebx                ;clear working reg                400
        pop     bx                      ;restore the # of cols            400
        mov     ax,  VideoCols          ;ax = # of cols per screen        400
        sub     ax,bx                   ;calculate the diff in bytes      400
        shl     ax,1                    ;double the number                400
        add     di,ax                   ;adjust the di                    400
        inc     byte ptr [ebp+top]      ;increment the top line           400

        dec     byte ptr [ebp+nlines]              ;if more lines to go
        jnz     pu20                    ;then loop on next line

        pop     ax                      ;starting video buffer adddress
        sub     di, ax                  ;di = # of bytes changed
        pop     ax                      ;starting screen buffer offset
        pop     es                 ;restore flat address reg

        push    00                 ;3rd parm to Vio32ShowBuf
        push    edi                ;2nd parm to Vio32ShowBuf
        push    eax                ;1st parm to Vio32ShowBuf
        call    Vio32ShowBuf
        add     esp,0Ch

        popfd
        pop     eax
        pop     ebx
        pop     ecx
        pop     edx
        pop     edi
        pop     esi
        leave
        ret
putup   endp

;Input  ds:si -> start of text line
;       es:di -> start of video line
;       ah =  hardware video attribute
;       bx =  # of text chars to skip
;       cx =  video line length

putone          proc near
        xor     dx, dx                  ;current text column
        jmp     short PREcmp
PRErep:
        sub     al, HIGHZONE
        add     dl, al
        adc     dh, 00
        lodsb
        cmp     dx, bx
        jb      PREnxt

        sub     dx, bx
        sub     cx, dx
        jbe     PUTful
        xchg    cx, dx
        rep     stosw
        xchg    cx, dx
        jmp     short PUT1st
PREnxt:
        lodsb
        cmp     al, LOWZONE
        jb      PUTend
        cmp     al, HIGHZONE
        jae     PRErep
        inc     dx
PREcmp:
        cmp     dx, bx
        jb      PREnxt
PUT1st:
        xor     dx, dx
PUTnxt:
        lodsb
        cmp     al, LOWZONE
        jb      PUTend
        cmp     al, HIGHZONE
        jae     PUTrep
        stosw
        loop    PUTnxt
        ret
PUTend:
        mov     al, ' '
        rep     stosw
        ret
PUTrep:
        sub     al, HIGHZONE
        mov     dl, al
        lodsb
        sub     cx, dx
        jbe     PUTful

        xchg    cx, dx
        rep     stosw
        xchg    cx, dx
        jmp     PUTnxt
PUTful:
        add     cx, dx
        rep     stosw
        ret
putone          endp

;------------------------------------------------------------------------------;
; name          putxb -- Translate Screen Blanks
;
; synopsis      putxb( line, repchr );
;
;               uint line;      /* line number to xlate (0..N)          */
;               char repchr;    /* replacement character for blanks     */
;
; description   This routine translates all blanks on the specified line
;               on the screen to another character.
;
;*...Release 1.00 (Pre-release 107 11/13/91)                                 *;
;*...                                                                        *;
;*... 11/13/91  400   Srinivas  Vertical Register Display.                   *;
;*...                                                                        *;
;------------------------------------------------------------------------------;

line   = 8
repchr = 12

putxb   PROC NEAR
        push    ebp
        mov     ebp,esp
        push    edi
        push    eax
        push    ebx
        push    ecx
        push    ds                 ;save flat address
        push    es                 ;save flat address

        xor     edi, edi           ;clear working reg
        xor     eax, eax           ;clear working reg
        xor     ecx, ecx           ;clear working reg
        xor     ebx, ebx           ;clear working reg                     400
        mov     cx, VideoCols
        mov     ax, [ebp+line]
        mul     cl
        shl     ax, 1

        mov     ebx, BoundPtr         ;pointer to screen bounds           400
        add     ebx, [ebp+line]       ;position to correct line           400
        mov     cl, byte ptr [ebx]    ;cx = # of cols to be painted       400
        push    cx                    ;save the # of cols                 400

        push    ax                      ;save starting video buffer offset
        lds     di, VideoPtr
        add     di, ax
        mov     ax, ds
        mov     es, ax
        mov     ax, [ebp+repchr]
        jmp     short xb20
xb10:
        inc     di                      ;skip char
        inc     di                      ;skip attr
xb20:
        cmp     byte ptr [di], 20h      ;if not a blank char
        loopne  xb10                    ;then loop on next char
        jcxz    xb40                    ;jump if last char on line
xb30:
        stosb                           ;xlate blank to repchr
        inc     di                      ;skip attr
        cmp     byte ptr [di], 20h
        loope   xb30
        jcxz    xb40                    ;jump if last char on line
        jmp     xb10
xb40:
        jne     xb50                    ;jump if last char on line not blank
        stosb
xb50:
        pop     ax                      ;starting video buffer adddress
        pop     cx                      ;# of cols per video line
        shl     cx, 1
        pop     es                 ;restore flat address reg
        pop     ds                 ;restore flat address reg

        push    00                 ;3rd parm to Vio32ShowBuf
        push    ecx                ;2nd parm to Vio32ShowBuf
        push    eax                ;1st parm to Vio32ShowBuf
        call    Vio32ShowBuf
        add     esp,0Ch

        pop     ecx
        pop     ebx
        pop     eax
        pop     edi
        leave
        ret
putxb   endp

_TEXT   ends
        end

