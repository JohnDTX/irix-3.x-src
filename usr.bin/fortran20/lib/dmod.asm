        GLOBAL  %D_MOD
; THIS ROUTINE PERFORMS A DOUBLE DMOD.
; It is Pascal callable, in the sense that the result
;   is left in D0/D1.
		EXTERN DMOD
;   DEFINITION:
;       DMOD(TOP,BOT) = RSLT ,  SUCH THAT:
;       N * BOT + RSLT = TOP  FOR SOME INTEGER.
;       ABS(RSLT) < ABS(BOT)
;       RSLT HAS SAME SIGN AS TOP (OR IS ZERO).
;       DMOD(TOP,BOT) = DMOD(TOP,ABS(BOT))
;       DMOD(-TOP,BOT) = -DMOD(TOP,BOT)

; INPUT:  Two Numbers
; OUTPUT: Remainder in D0/D1.

; REGISTER CONVENTIONS
;       D0/D1   BOT
;       D2/D3   TOP
;       D4      FLAG AND COUNT
;       D5      SIGN OF TOP AND RESULT
;       D6      EXPONENT
;       D7      EXPONENT

%D_MOD
;------
        move.l  16(sp),-(sp)
        move.l  16(sp),-(sp)
        move.l  16(sp),-(sp)
        move.l  16(sp),-(sp)
        jsr   DMOD
        movem.l (sp)+,d0/d1
        move.l  (sp)+,12(sp)
        adda.w  #12,sp
        rts

		end
