
        IDENT   FMAX

;
; COPYRIGHT 1981 RICHARD E. JAMES III
;
; MAX, MIN, AND DIM FOR SINGLE PRECISION ON MC68000.

        GLOBAL  %F_MAX,%F_MIN,%F_DIM

        EXTERN  %F_SUB,%F_CMP

; MAX AND MIN RETURN ONE OF THEIR ARGUMENTS.
;   IF EITHER ARGUMENT IS A NAN, THEN IT IS RETURNED.
;   (MAX RETURNS THE LAST NAN; MIN RETURNS THE FIRST.)

;
; Parameters: ST.L - First arg
;             ST.L - Second arg
;
; Returns: D0.L - The result
;
; Scratches: Only D0.
;

; DIM(X,Y)=MAX(X-Y,0)
;   PASCAL CALLABLE.

%F_DIM
;------
        move.l  8(sp),-(sp)
        move.l  8(sp),-(sp)
        jsr     %F_SUB
        move.l  (sp)+,8(sp)
        clr.l   4(sp)
;      (FALL INTO MAX)

%F_MAX
;------
        MOVE.L  8(SP),-(SP)    ;COPY ARG
        MOVE.L  8(SP),-(SP)    ;COPY ARG
        jsr   %F_CMP
        BCC.S   MXOK           ;NO NANS
; ONE WAS A NAN OR UNORDERED INF:
        BCLR    #7,8(SP)       ;REMOVE SIGN OF FIRST
        CMPI.L  #$7F800001,8(SP)
MXOK    BGE.S   MX9
        MOVE.L  4(SP),8(SP)    ;OTHER ARG
MX9     move.l  8(sp),d0
        move.l  (sp)+,(sp)
        MOVE.L  (SP)+,(SP)
        RTS

%F_MIN
;------
        MOVE.L  8(SP),-(SP)    ;COPY ARG
        MOVE.L  8(SP),-(SP)    ;COPY ARG
        jsr   %F_CMP
        BCC.S   MNOK           ;NO NANS
; ONE WAS A NAN OR UNORDERED INF:
        BCLR    #7,4(SP)       ;REMOVE SIGN
        CMPI.L  #$7F800000,4(SP)
MNOK    BLE.S   MN9
        MOVE.L  4(SP),8(SP)    ;OTHER ARG
MN9     move.l  8(sp),d0
        move.l  (sp)+,(sp)
        MOVE.L  (SP)+,(SP)
        RTS

        END
