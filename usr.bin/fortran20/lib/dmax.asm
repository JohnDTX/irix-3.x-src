        IDENT   DMAX
;
; COPYRIGHT 1981 RICHARD E. JAMES III

; MAX, MIN, DIM  FOR DOUBLE PRECISION.

        GLOBAL  %D_MAX,%D_MIN,%D_DIM
        EXTERN  %D_SUB,%D_CMP
; MAX AND MIN RETURN ONE OF THEIR ARGUMENTS.
;   IF EITHER ARGUMENT IS A NAN, THEN IT IS RETURNED.
;   (MAX RETURNS THE LAST NAN; MIN RETURNS THE FIRST.)

;
; Parameters: ST.Q - First arg
;             ST.Q - Second arg
;
; Returns: D0/D1
;
; Scratches: Only D0,D1.
;

; DIM(X,Y)=MAX(0,X-Y)
;   PASCAL CALLABLE.

%D_DIM
;------
        move.l  16(sp),-(sp)
        move.l  16(sp),-(sp)
        move.l  16(sp),-(sp)
        move.l  16(sp),-(sp)
        jsr     %D_SUB
        move.l  (sp)+,8(sp)
        move.l  (sp)+,8(sp)
        clr.l   12(sp)
        clr.l   16(sp)
;      (FALL INTO MAX)

; FOR MAX AND MIN:
; INPUT:  2 DOUBLE PRECISION NUMBERS ON STACK
; OUTPUT: 1 DOUBLE PRECISION NUMBER ON STACK

%D_MAX
;------
        MOVE.L  16(SP),-(SP)  ;COPY ARG
        MOVE.L  16(SP),-(SP)  ;COPY ARG
        MOVE.L  16(SP),-(SP)  ;COPY ARG
        MOVE.L  16(SP),-(SP)  ;COPY ARG
        jsr   %D_CMP
        BCC.S   MXOK    ;NO NANS
; ONE WAS A NAN OR UNORDERED INF:
        BCLR    #7,12(SP)      ;REMOVE SIGN OF FIRST
        CMPI.L  #$7FF00001,12(SP)
MXOK    BLT.S   MOTHR          ;USE OTHER ARGUMENT
        movem.l 12(sp),d0/d1
        move.l  (sp),16(sp)
        adda.w  #16,sp
        rts

%D_MIN
;------
        MOVE.L  16(SP),-(SP)  ;COPY ARG
        MOVE.L  16(SP),-(SP)  ;COPY ARG
        MOVE.L  16(SP),-(SP)  ;COPY ARG
        MOVE.L  16(SP),-(SP)  ;COPY ARG
        jsr   %D_CMP
        BCC.S   MNOK    ;NO NANS
; ONE WAS A NAN OR UNORDERED INF:
        BCLR    #7,4(SP)     ;REMOVE SIGN
        CMPI.L  #$7FF00000,4(SP)
MNOK    BLE.S   MN9
MOTHR   MOVE.L  4(SP),12(SP) ;OTHER ARG
        MOVE.L  8(SP),16(SP) ;OTHER ARG
MN9     movem.l 12(sp),d0/d1
        move.l  (sp),16(sp)
        adda.w  #16,sp
        rts

        END
