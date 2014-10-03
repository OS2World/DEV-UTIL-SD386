        page    ,132
        title   Video Box Routines
        subttl  Copyright (C) 1988 IBM (Written 11/87 by Jim Christensen)
        name    VBOX

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

TBEDGE   = 0CDh
LREDGE   = 0BAh
TLCORNER = 0C9h
TRCORNER = 0BBh
BLCORNER = 0C8h
BRCORNER = 0BCh

        align 4
_DATA   ends
EXTRN   Vio32ShowBuf:near
PUBLIC  Vgetbox
PUBLIC  Vputbox
PUBLIC  Vfmtbox
_TEXT   SEGMENT  DWORD USE32 PUBLIC 'CODE'

;------------------------------------------------------------------------------;
; name          Vgetbox
;
; synopsis      Vgetbox( buffer, row, col, rows, cols );
;               char *buffer;           /* pointer to buffer for data */
;               int row, col,           /* upper left corner of box (0..N) */
;               int rows, cols;         /* size of box (1..N) */
;
; description   This function copies the (attr,char) pairs from the specified
;               rectangle in the video buffer to the buffer provided.
;------------------------------------------------------------------------------;

buffer = 8
row    = 12
col    = 16
rows   = 20
cols   = 24

Vgetbox PROC   NEAR
        push    ebp
        mov     ebp,esp
        push    ds                 ;save flat address
        push    esi
        push    edi
        push    eax
        push    ebx
        push    ecx
        push    edx

        xor     esi, esi           ;clear working reg
        xor     eax, eax           ;clear working reg
        mov     ebx, [ebp+row]
        mov     ecx, [ebp+col]
        mov     edx, [ebp+cols]
        mov     dh,  [ebp+rows]
        call    setABCD
        lds     si, VideoPtr  ;ds:si -> 1st (char,attr) in video ram
        add     si, ax
        mov     edi, [ebp+buffer]   ;es:di -> video save buffer
gb10:
        mov     cl, bl
        rep     movsw
        add     si, dx
        dec     bh
        jnz     gb10

        pop     edx
        pop     ecx
        pop     ebx
        pop     eax
        pop     edi
        pop     esi
        pop     ds
        leave
        ret
Vgetbox endp

;------------------------------------------------------------------------------;
; name          Vputbox
;
; synopsis      Vputbox( buffer, row, col, rows, cols );
;               char *buffer;           /* pointer to buffer for data */
;               int row, col,           /* upper left corner of box (0..N) */
;               int rows, cols;         /* size of box (1..N) */
;
; description   This function copies the (attr,char) pairs from the buffer
;               to the specified rectangle of the video buffer.
;------------------------------------------------------------------------------;

buffer = 8
row    = 12
col    = 16
rows   = 20
cols   = 24

Vputbox PROC   NEAR
        push    ebp
        mov     ebp,esp
        push    esi
        push    edi
        push    eax
        push    ebx
        push    ecx
        push    edx
        push    es                 ;save flat address

        xor     edi, edi           ;clear working reg
        xor     eax, eax           ;clear working reg
        mov     ebx, [ebp+row]
        mov     ecx, [ebp+col]
        mov     edx, [ebp+cols]
        mov     dh,  [ebp+rows]
        call    setABCD
        les     di, VideoPtr      ;es:di -> 1st (char,attr) in video ram
        add     di, ax
        push    ax                ;save starting video buffer offset
        push    di                ;save starting video buffer address
        mov     esi, [ebp+buffer]   ;ds:si -> video save buffer on stack
pb10:
        mov     cl, bl
        rep     movsw
        add     di, dx
        dec     bh
        jnz     pb10

        pop     ax                ;starting video buffer adddress
        sub     di, ax            ;di = # of bytes changed
        pop     ax                ;starting screen buffer offset
        pop     es                 ;restore flat address reg

        push    00                 ;3rd parm to Vio32ShowBuf
        push    edi                ;2nd parm to Vio32ShowBuf
        push    eax                ;1st parm to Vio32ShowBuf
        call    Vio32ShowBuf
        add     esp,0Ch

        pop     edx
        pop     ecx
        pop     ebx
        pop     eax
        pop     edi
        pop     esi
        leave
        ret
Vputbox endp

;------------------------------------------------------------------------------;
; name          Vfmtbox
;
; synopsis      Vfmtbox( buffer, row, col, rows, cols );
;               char *buffer;           /* pointer to buffer for data */
;               int row, col,           /* upper left corner of box (0..N) */
;               int rows, cols;         /* size of box (1..N) */
;
; description   This function formats the specified rectangle of the video ram,
;               and then puts the message contained in the buffer into it.  The
;               format of the message in the buffer is as follows:
;
;               row (byte), col (byte), text (N bytes), zero (byte)
;
;               The buffer contains any number of these sequences, ended by
;               a row of zero.  The row and column are relative to the edges
;               of the box.  A column of zero indicates a centered row.  Note
;               that the first and last row and column are used by the border
;               of the box.
;------------------------------------------------------------------------------;

TVideoAtr       equ byte ptr [ebp-30] ;default physical attribute on stack
VideoOffset     equ word ptr [ebp-32] ;segment offset of start of video buffer

buffer = 8
row    = 12
col    = 16
rows   = 20
cols   = 24

Vfmtbox PROC   NEAR
        push    ebp
        mov     ebp,esp
        push    esi
        push    edi
        push    eax
        push    ebx
        push    ecx
        push    edx
        push    es                 ;save flat address

        xor     eax, eax          ;clear working reg
        mov     ebx, VideoMap     ;es:bx -> Logical to Physical attribute table
        mov     al, VideoAtr      ;al = default logical attribute
        xlat                      ;al = default physical attribute
        push    ax                ;set VideoAtr to default physical attribute

        xor     edi, edi          ;clear working reg
        les     di, VideoPtr      ;es:di -> video buffer
        push    di                ;set VideoOffset to start of logical buffer

        mov     al, ' '
        mov     ebx, [ebp+row]
        mov     ecx, [ebp+col]
        mov     edx, [ebp+cols]
        mov     dh,  [ebp+rows]
        call    filblk

        mov     al, TBEDGE
        mov     bx, [ebp+row]
        mov     cx, [ebp+col]
        mov     dh, 1
        mov     dl, [ebp+cols]
        call    filblk

        mov     al, TBEDGE
        mov     bx, [ebp+row]
        add     bx, [ebp+rows]
        dec     bx
        mov     cx, [ebp+col]
        mov     dh, 1
        mov     dl, [ebp+cols]
        call    filblk

        mov     al, LREDGE
        mov     bx, [ebp+row]
        mov     cx, [ebp+col]
        mov     dh, [ebp+rows]
        mov     dl, 1
        call    filblk

        mov     al, LREDGE
        mov     bx, [ebp+row]
        mov     cx, [ebp+col]
        add     cx, [ebp+cols]
        dec     cx
        mov     dh, [ebp+rows]
        mov     dl, 1
        call    filblk

        mov     al, TLCORNER
        mov     bx, [ebp+row]
        mov     cx, [ebp+col]
        mov     dx, 0101h
        call    filblk

        mov     al, BLCORNER
        mov     bx, [ebp+row]
        add     bx, [ebp+rows]
        dec     bx
        mov     cx, [ebp+col]
        mov     dx, 0101h
        call    filblk

        mov     al, TRCORNER
        mov     bx, [ebp+row]
        mov     cx, [ebp+col]
        add     cx, [ebp+cols]
        dec     cx
        mov     dx, 0101h
        call    filblk

        mov     al, BRCORNER
        mov     bx, [ebp+row]
        add     bx, [ebp+rows]
        dec     bx
        mov     cx, [ebp+col]
        add     cx, [ebp+cols]
        dec     cx
        mov     dx, 0101h
        call    filblk

        mov     esi, [ebp+buffer] ;ds:si -> buffer
fb10:
        lodsw                   ;(ah,al) = relative (col,row) for line
        or      al, al
        jz      fb90            ;jump if end of text
        or      ah, ah
        jnz     fb20            ;jump if starting column specified

        mov     di, si          ;di -> 1st byte of text
        xor     ax, ax
        mov     cx, -1
        push    es
        push    ds
        pop     es              ;es:di -> 1st byte of text
        repne   scasb           ;scan for trailing null
        pop     es
        inc     cx              ;cx = -1 - (# chars + 1) + 1
        inc     cx              ;cx = -(# chars)
        add     cx, [ebp+cols]   ;cx = # of spare columns
        js      fb90            ;jump if text wider than box
        shr     cx, 1

        movzx   cx, cl                   ;cl     = relative col
        movzx   bx, byte ptr [si-2]      ;[si-2] = relative row

fb20:
        add     bx, [ebp+row]   ;bx = absolute row
        add     cx, [ebp+col]   ;cx = absolute col
        mov     ax, VideoCols
        mul     bx              ;ax = screen offset to start of row
        add     ax, cx          ;ax = screen offset to start of msg
        shl     ax, 1           ;ax = video buffer offset to start of msg
        xchg    ax, di          ;es:di -> 1st (char,attr) in video ram
        mov     cx, VideoCols
        mov     ah, TVideoAtr
        jmp     short fb40
fb30:
        stosw
fb40:
        lodsb
        or      al, al
        loopnz  fb30
        jz      fb10
fb90:
        add     sp, 4           ;discard VideoAtr and VideoOffset
        pop     es                 ;restore flat address reg
        push    0               ;3rd parm to Vio16ShowBuf

        mov     ax, [ebp+rows]
        dec     ax
        mov     bx, VideoCols
        mul     bx
        add     ax, [ebp+cols]
        shl     ax, 1
        push    eax              ;2nd parm to Vio16ShowBuf

        mov     ax, VideoCols
        mul     word ptr [ebp+row]
        add     ax, [ebp+col]
        shl     ax, 1           ;ax = starting video buffer offset
        push    eax              ;1st parm to Vio16ShowBuf
        call    Vio32ShowBuf
        add     esp,0Ch

        pop     edx
        pop     ecx
        pop     ebx
        pop     eax
        pop     edi
        pop     esi
        leave
        ret
Vfmtbox endp

;Input  bx = row (0..N)
;       cx = col (0..N)
;       dh = # of rows (1..N)
;       dl = # of cols (1..N)
;
;Output ax = video buffer offset to (row,col)
;       bh = # of rows (1..N)
;       bl = # of cols (1..N)
;       cx = 0
;       dx = (# of cols/screen) - (# of cols used)

setABCD proc    near
        xchg    ax, bx          ;ax = row (0..N)
        push    edx
        mov     bx, VideoCols
        mul     bx
        pop     edx
        add     ax, cx
        shl     ax, 1

        mov     bx, dx          ;(bh,bl) = (# of rows, # of cols)
        xor     dx, dx
        mov     dx, VideoCols
        movzx   cx, bl
        sub     dx, cx
        shl     dx, 1
        xor     cx, cx

        ret
setABCD endp

filblk  proc    near
        push    ax
        call    setABCD
        add     ax, VideoOffset
        xchg    ax, di          ;es:di -> 1st (char,attr) in video ram

        pop     ax
        mov     ah, TVideoAtr    ;display attribute
fl10:
        mov     cl, bl
        rep     stosw

        add     di, dx
        dec     bh
        jnz     fl10

        ret
filblk  endp

_TEXT   ends
        end
