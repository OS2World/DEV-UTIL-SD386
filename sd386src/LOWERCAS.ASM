        page    ,132
        title   Character Translation Routines
        subttl  Copyright (C) 1988 by IBM (Written 5/88 by Jim Christensen)

        .386
_TEXT   SEGMENT  DWORD USE32 PUBLIC 'CODE'
_TEXT      ENDS
_DATA   SEGMENT  DWORD USE32 PUBLIC 'DATA'
_DATA      ENDS
        ASSUME   CS: FLAT, DS: FLAT, SS: FLAT, ES: FLAT
_DATA   segment dword public USE32 'DATA'

;  The translation table for upper case to lower case, for US and world trade.
xlctab  label   byte
   db   00h,01h,02h,03h,04h,05h,06h,07h,08h,09h,0Ah,0Bh,0Ch,0Dh,0Eh,0Fh  ;00-0F
   db   10h,11h,12h,13h,14h,15h,16h,17h,18h,19h,1Ah,1Bh,1Ch,1Dh,1Eh,1Fh  ;10-1F
   db   20h,21h,22h,23h,24h,25h,26h,27h,28h,29h,2Ah,2Bh,2Ch,2Dh,2Eh,2Fh  ;20-2F
   db   30h,31h,32h,33h,34h,35h,36h,37h,38h,39h,3Ah,3Bh,3Ch,3Dh,3Eh,3Fh  ;30-3F
   db   40h,61h,62h,63h,64h,65h,66h,67h,68h,69h,6Ah,6Bh,6Ch,6Dh,6Eh,6Fh  ;40-4F
   db   70h,71h,72h,73h,74h,75h,76h,77h,78h,79h,7Ah,5Bh,5Ch,5Dh,5Eh,5Fh  ;50-5F
   db   60h,61h,62h,63h,64h,65h,66h,67h,68h,69h,6Ah,6Bh,6Ch,6Dh,6Eh,6Fh  ;60-6F
   db   70h,71h,72h,73h,74h,75h,76h,77h,78h,79h,7Ah,7Bh,7Ch,7Dh,7Eh,7Fh  ;70-7F

   db   87h,81h,82h,83h,84h,85h,86h,87h,88h,89h,8Ah,8Bh,8Ch,8Dh,84h,86h  ;80-8F
   db   82h,91h,92h,93h,94h,95h,96h,97h,98h,94h,81h,9Bh,9Ch,9Dh,94h,96h  ;90-9F
   db   160,161,162,163,164,164,166,167,168,169,170,171,172,173,174,175  ;A0-AF
   db   176,177,178,179,180,181,182,183,184,185,186,187,188,189,190,191  ;B0-BF
   db   192,193,194,195,196,197,198,199,200,201,202,203,204,205,206,207  ;C0-CF
   db   208,209,210,211,212,213,214,215,216,217,218,219,220,221,222,223  ;D0-DF
   db   224,225,226,227,228,229,230,231,232,233,234,235,236,237,238,239  ;E0-EF
   db   240,241,242,243,244,245,246,247,248,249,250,251,252,253,254,255  ;F0-FF

        align 4
_DATA   ends
PUBLIC  LowerCase
_TEXT   SEGMENT  DWORD USE32 PUBLIC 'CODE'

;------------------------------------------------------------------------------;
; name          LowerCase -- translate to lower case
;
; synopsis      void LowerCase( inbuf, outbuf );
;
;               char *inbuf;            /* char array to translate */
;               char *outbuf;           /* char array for result */
;
; description   This function translates characters to lower case.
;------------------------------------------------------------------------------;

InBuf  = 8
OutBuf = 12

LowerCase PROC NEAR
        push    ebp
        mov     ebp,esp
        push    eax
        push    ebx
        push    esi
        push    edi

        mov     esi, [ebp+InBuf]
        mov     edi, [ebp+OutBuf]
        mov     ebx, OFFSET FLAT:xlctab
        jmp     short lc20
lc10:
        xlat
        stosb
lc20:
        lodsb
        or      al, al
        jnz     lc10

        pop     edi
        pop     esi
        pop     ebx
        pop     eax
        leave
        ret

LowerCase endp

_TEXT   ends
        end
