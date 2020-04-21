; File: jftnmisc.text
; Date: 27-Jun-85

;
; Misc. FORTRAN Stuff - By R. Steven Glanville
;

        ident   JFTNMISC
        
        global  %_IABS,%_IDIM,%_ISIGN,%_IMOD
        global  %_CONJ
        global  %_MAX,%_MIN
        global  %C_CMP
        global  %C_NEG
        global  %_ABS
        
        extern  %I_MOD4
        
;
; function %_IABS(var i: longint): longint;
;
; Integer absolute value
;

%_IABS
        move.l  4(sp),a0
        move.l  (a0),d0
        bpl.s   iabsok
        neg.l   d0
iabsok  rtd     #4
        
;
; function %_IDIM(i,j: longint): longint;
;
; i - j if >= 0, else 0
;

%_IDIM
        move.l  8(sp),d0        ; I
        sub.l   4(sp),d0        ; I - J
        bpl.s   idimok
        clr.l   d0
idimok  rtd     #8
        
;
; function %_ISIGN(i,j: longint): longint;
;
; |i| if j >= 0,  -|i| if j < 0
;

%_ISIGN
        move.l  8(sp),d0        ; I
        bpl.s   noabs
        neg.l   d0
noabs   tst.l   4(sp)           ; |I|
        bpl.s   isignok
        neg.l   d0              ; -|I|
isignok rtd     #8
        
;
; function %_IMOD(i,j: longint): longint;
;
; i - (i/j)*j
;

%_IMOD
        jmp     %I_MOD4
        
;
; function %_CONJ(c: complex): complex;
;

%_CONJ
        movem.l 4(sp),d0/d1
        bchg    #31,d1
        move.l  d1,$8410.w      ; J_MOVELO d1,F1
        move.l  d0,$8310.w      ; J_MOVEHI d0,F1
        rtd     #8
        
;
; %_MAX - FORTRAN max(i,j) function
;
; Parameters: ST.L - First param
;             ST.L - Second param
;
; Returns:    D0.L - Result
;
; Scratches: Only D0.
;

%_MAX
        move.l  4(sp),d0        ; J
        cmp.l   8(sp),d0
        bge.s   maxok
        move.l  8(sp),d0
maxok   rtd     #8
        
;
; %_MIN - FORTRAN min(i,j) function
;
; Parameters: ST.L - First param
;             ST.L - Second param
;
; Returns:    D0.L - Result
;
; Scratches: Only D0.
;

%_MIN
        move.l  4(sp),d0        ; J
        cmp.l   8(sp),d0
        ble.s   minok
        move.l  8(sp),d0
minok   rtd     #8
        
;
; function %_ABS(var r: real): real;
;
; Real absolute value
;

%_ABS
        move.l  (sp)+,a0
        move.l  (sp)+,d0
        bclr    #31,d0
        move.l  d0,$8110.w      ; J_MOVES d0,F1
        jmp     (a0)
        
;
; %C_CMP - Complex compare
;
; Parameters: ST.C - First param
;             ST.C - Second param
;
; Returns: CC - Result
;
; All registers preserved.
;

%C_CMP
        move.l  8(sp),$8100.w   ; J_MOVES B.I,F0
        move.l  16(sp),$B200.w  ; J_FCMP  A.I,F0
        tst.b   $8700.w         ; J_CR
        bne.s   ccmpne
        move.l  4(sp),$8100.w   ; J_MOVES B.R,F0
        move.l  12(sp),$B200.w  ; J_FCMP  A.R,F0
        tst.b   $8700.w         ; J_CR
ccmpne  rtd     #16
        
;
; %C_NEG - Complex negate
;
; Parameters: ST.C - Param
;
; Returns: ST.C - Result
;
; All registers preserved.
;

%C_NEG
        bchg    #7,4(sp)        ; Negate Real part
        bchg    #7,8(sp)        ; Negate Imaginary part
        rts
        
        end

