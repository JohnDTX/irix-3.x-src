; File: jundmod.text - DMOD for Juniper
; Date: 4/24/86

        IDENT   JUNDMOD

        GLOBAL  %D_MOD
;
; %D_MOD(x,y) = x - int(x/y)*y
;

%D_MOD
;
;	gb (sgi) 10/25/85 - mod(x,0) is ILLEGALOP, 
;	not the divzero that the h/w is going to generate...
;	currently, the s/w D_MOD raises divzero here so that
;	we dont have to insert the gobs of code here to check for
;	denormalized values, and negative zero.
;
        move.l  16(sp),$8410.w  ; J_MOVELO x,F1
        move.l  12(sp),$8310.w  ; J_MOVEHI x,F1
        move.l  8(sp),$8400.w   ; J_MOVELO y,F0
        move.l  4(sp),$9D10.w   ; J_DDIV   y,F1
        move.b  d0,$AD11.w      ; J_DtoI   F1
        move.b  d0,$AB11.w      ; J_ItoD   F1
        move.l  8(sp),$8400.w   ; J_MOVELO y,F0
        move.l  4(sp),$9910.w   ; J_DMUL   y,F1
        move.l  16(sp),$8400.w  ; J_MOVELO x,F0
        move.l  12(sp),$9510.w  ; J_FRSUB  x,F1
        rtd     #16
		end

