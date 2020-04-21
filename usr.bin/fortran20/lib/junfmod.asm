; File: junfmod.text - FMOD function for juniper

        IDENT   JUNFMOD

        GLOBAL  %F_MOD
;
; %F_MOD(x,y) = x - int(x/y)*y
;

%F_MOD
;
;	gb (sgi) 10/22/85 - have to check for mod(INF,y)
;
;	move.l	8(sp),d0
;	bclr	#31,d0
;	cmp.l	#$7f800000,d0
;	bne.s	gomod
;
;	illegal operation - mod(INF,x)
;
;	move.l	#ILLEGALOP,-(sp)
;	jsr	__FPERR
;	addq.l	#4,sp
;	move.l	d0,$8110.w	; move result to F1
;	rtd 	#8
;gomod
        move.l  8(sp),$8110.w   ; J_MOVES x,F1
        move.l  4(sp),$9B10.w   ; J_FDIV  y,F1
        move.b  d0,$A911.w      ; J_FtoI  F1
        move.b  d0,$A711.w      ; J_ItoF  F1
        move.l  4(sp),$9710.w   ; J_FMUL  y,F1
        move.l  8(sp),$9310.w   ; J_FRSUB x,F1
        rtd     #8
        end

