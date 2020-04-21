; File: sky.math.text
; Date: 18-Sep-83

        ident   SKY_MATH

        global  %_SKY
        global  %F_ADD,%F_SUB,%F_MUL,%F_DIV
        global  %D_ADD,%D_SUB,%D_MUL,%D_DIV
        global  %F_2_D,%D_2_F,%F_2_I,%I_2_F,%D_2_I,%I_2_D
        global  %F_CMP,%D_CMP
        global  %F_MOD,%F_MIN,%F_MAX,%F_DIM
        global  %_SQRT,%_SIN,%_COS,%_ATAN,%_EXP,%_LN
        
        extern  %_PANIC,%_HALTF,_open
        
;
; Some SKY card EQU's:
;

COMREG  equ     $1040
STCREG  equ     $1042
DATREG  equ     $1044

ADDCMD  equ     $1001
SUBCMD  equ     $1007
MULCMD  equ     $100b
DIVCMD  equ     $1013
MODCMD  equ     $1030
MINCMD  equ     $101e
MAXCMD  equ     $101f
DADDCMD equ     $1002
DSUBCMD equ     $1008
DMULCMD equ     $100c
DDIVCMD equ     $1014
FLTCMD  equ     $1024
TNCCMD  equ     $1027
DFLTCMD equ     $1044
DTNCCMD equ     $1045
F2DCMD  equ     $1042
D2FCMD  equ     $1043
CMPCMD  equ     $105d
DCMPCMD equ     $105e
SQRTCMD equ     $102f
SINCMD  equ     $1029
COSCMD  equ     $1028
ATANCMD equ     $102b
EXPCMD  equ     $102c
LNCMD   equ     $102d

;
; %_SKY - Set up SKY card.
;

%_SKY   rts

;
; %F_ADD - Single precision floating point add
;
; %F_SUB - Single precision floating point subtract
;
; %F_MUL - Single precision floating point multiply
;
; %F_DIV - Single precision floating point divide
;
; Parameters: ST.L - First arg
;             ST.L - Second arg
;
; Returns:    ST.L - Answer
;
; All registers are preserved.
;

%F_ADD
        move.w  #ADDCMD,COMREG.W
        move.l  8(sp),DATREG.W
        move.l  4(sp),DATREG.W
        move.l  (sp)+,(sp)
        move.l  DATREG.W,4(sp)
        rts

%F_SUB
        move.w  #SUBCMD,COMREG.W
        move.l  8(sp),DATREG.W
        move.l  4(sp),DATREG.W
        move.l  (sp)+,(sp)
        move.l  DATREG.W,4(sp)
        rts

%F_MUL
        move.w  #MULCMD,COMREG.W
        move.l  8(sp),DATREG.W
        move.l  4(sp),DATREG.W
        move.l  (sp)+,(sp)
        move.l  DATREG.W,4(sp)
        rts

%F_DIV
        move.w  #DIVCMD,COMREG.W
        move.l  8(sp),DATREG.W
        move.l  4(sp),DATREG.W
        move.l  (sp)+,(sp)
div_lp  tst.w   STCREG.W
        bge.s   div_lp
        move.l  DATREG.W,4(sp)
        rts
        page
;
; %D_ADD - Double precision floating point add
;
; %D_SUB - Double precision floating point subtract
;
; %D_MUL - Double precision floating point multiply
;
; %D_DIV - Double precision floating point divide
;
; Parameters: ST.Q - First arg
;             ST.Q - Second arg
;
; Returns:    ST.Q - Answer
;
; All registers are preserved.
;

%D_ADD
        move.w  #DADDCMD,COMREG.W
        bra.s   do_dble
%D_SUB
        move.w  #DSUBCMD,COMREG.W
        bra.s   do_dble
%D_MUL
        move.w  #DMULCMD,COMREG.W
        bra.s   do_dble
%D_DIV
        move.w  #DDIVCMD,COMREG.W
do_dble move.l  12(sp),DATREG.W
        move.l  16(sp),DATREG.W
        move.l  4(sp),DATREG.W
        move.l  8(sp),DATREG.W
        move.l  (sp)+,4(sp)
        addq.w  #4,sp
dad_lop tst.w   STCREG.W
        bge.s   dad_lop
        move.l  DATREG.W,4(sp)
        move.l  DATREG.W,8(sp)
        rts
        page
;
; %I_2_F - Float the top of stack
;
; Parameters: ST.L - Arg1
;
; Returns: ST.L - Floating point representation
;

%I_2_F
        move.w  #FLTCMD,COMREG.W
        move.l  4(sp),DATREG.W
        move.l  DATREG.W,4(sp)
        rts

;
; %F_2_I - Truncate Floating Point to Integer
;
; Parameters: ST.L - Arg1
;
; Returns: ST.L - Integer value
;

%F_2_I
        move.w  #TNCCMD,COMREG.W
        move.l  4(sp),DATREG.W
        move.l  DATREG.W,4(sp)
        rts

;
; %I_2_D - Double Float the top of stack
;
; Parameters: ST.L - Arg1
;
; Returns: ST.Q - Double representation
;

%I_2_D
        move.w  #DFLTCMD,COMREG.W
        move.l  4(sp),DATREG.W
        move.l  (sp),-(sp)
        move.l  DATREG.W,4(sp)
        move.l  DATREG.W,8(sp)
        rts

;
; %D_2_I - Double Trunc the top of stack
;
; Parameters: ST.Q - Arg1
;
; Returns: ST.L - Integer representation
;

%D_2_I
        move.w  #DTNCCMD,COMREG.W
        move.l  4(sp),DATREG.W
        move.l  8(sp),DATREG.W
        move.l  (sp)+,(sp)
        move.l  DATREG.W,4(sp)
        rts

;
; %F_2_D - Single to Double Convert the top of stack
;
; Parameters: ST.L - Arg1
;
; Returns: ST.Q - Double representation
;

%F_2_D
        move.w  #F2DCMD,COMREG.W
        move.l  4(sp),DATREG.W
        move.l  (sp),-(sp)
        move.l  DATREG.W,4(sp)
        move.l  DATREG.W,8(sp)
        rts

;
; %D_2_F - Double to Single the top of stack
;
; Parameters: ST.Q - Arg1
;
; Returns: ST.L - Single representation
;

%D_2_F
        move.w  #D2FCMD,COMREG.W
        move.l  4(sp),DATREG.W
        move.l  8(sp),DATREG.W
        move.l  (sp)+,(sp)
        move.l  DATREG.W,4(sp)
        rts
        page
;
; %F_CMP - Floating point compare
;
; Parameters: ST.L - First arg
;             ST.L - Second arg
;
; Returns: CC = Result of compare
;

%F_CMP
        move.w  #CMPCMD,COMREG.W
        move.l  8(sp),DATREG.W
        move.l  4(sp),DATREG.W
        move.l  (sp)+,(sp)
        move.l  (sp)+,(sp)
        move.w  DATREG.W,CCR
        rts
        
;
; %D_CMP - Double precision floating point compare
;
; Parameters: ST.Q - First arg
;             ST.Q - Second arg
;
; Returns: CC = Result of compare
;

%D_CMP
        move.w  #DCMPCMD,COMREG.W
        move.l  12(sp),DATREG.W
        move.l  16(sp),DATREG.W
        move.l  4(sp),DATREG.W
        move.l  8(sp),DATREG.W
        move.l  (sp),16(sp)
        adda.w  #16,sp
        move.w  DATREG.W,CCR
        rts
        page
;
; %F_MAX - Floating point maximum
;
; %F_MIN - Floating point minimum
;
; %F_DIM - Floating point DIM
;
; %F_MOD - Floating point modulus
;
; Parameters: ST.L - Arg1
;             ST.L - Arg2
;
; Returns: D0.L - Result
;
; Scratches: D0,A0.
;

%F_MIN
        move.l  (sp)+,a0
        move.w  #MINCMD,COMREG.W
        move.l  (sp)+,DATREG.W
        move.l  (sp)+,DATREG.W
        move.l  DATREG.W,d0
        jmp     (a0)

%F_MAX
        move.l  (sp)+,a0
        move.w  #MAXCMD,COMREG.W
        move.l  (sp)+,DATREG.W
        move.l  (sp)+,DATREG.W
        move.l  DATREG.W,d0
        jmp     (a0)

%F_DIM
        move.l  (sp)+,a0
        move.w  #SUBCMD,COMREG.W
        move.l  4(sp),DATREG.W
        move.l  (sp)+,DATREG.W
        addq.w  #4,sp
        move.l  DATREG.W,d0
        bge.s   dim_ok
        clr.l   d0
dim_ok  jmp     (a0)

%F_MOD
        move.l  (sp)+,a0
        move.w  #DIVCMD,COMREG.W
        move.l  4(sp),DATREG.W
        move.l  (sp),DATREG.W
mod_lp  tst.w   STCREG.W
        bge.s   mod_lp
		move.l	DATREG.W,d0
		move.w	#TNCCMD,COMREG.W
		move.l	d0,DATREG.W
		move.l	DATREG.W,d0
		move.w	#FLTCMD,COMREG.W
		move.l	d0,DATREG.W
		move.l	DATREG.W,d0
		move.w	#MULCMD, COMREG.W
		move.l	d0,DATREG.W
		move.l	(sp),DATREG.W
		move.l	DATREG.W,d0
		move.w	#SUBCMD,COMREG.W
		move.l	4(sp),DATREG.W
		move.l	d0,DATREG.W
        addq.w  #8,sp
        move.l  DATREG.W,d0
        jmp     (a0)
		page

;%F_MOD
;        move.l  (sp)+,a0
;        move.w  #MODCMD,COMREG.W
;        move.l  4(sp),DATREG.W
;        move.l  (sp)+,DATREG.W
;        addq.w  #4,sp
;mod_lp  tst.w   STCREG.W
;        bge.s   mod_lp
;        move.l  DATREG.W,d0
;        jmp     (a0)
;        page
;
; %_SQRT - Square root
;
; %_SIN - Sine
;
; %_COS - Cosine
;
; %_ATAN - Arc Tangent
;
; %_EXP - Exponential
;
; %_LN - Logarithm
;
; Parameters: ST.L - Arg
;
; Returns: D0.L - Answer
;

%_SQRT
        move.w  #SQRTCMD,COMREG.W
        bra.s   dotrans
%_SIN
        move.w  #SINCMD,COMREG.W
        bra.s   dotrans
%_COS
        move.w  #COSCMD,COMREG.W
        bra.s   dotrans
%_ATAN
        move.w  #ATANCMD,COMREG.W
        bra.s   dotrans
%_EXP
        move.w  #EXPCMD,COMREG.W
        bra.s   dotrans
%_LN
        move.w  #LNCMD,COMREG.W
dotrans move.l  (sp)+,a0
        move.l  (sp)+,DATREG.W
translp tst.w   STCREG.W
        bge.s   translp
        move.l  DATREG.W,d0
        jmp     (a0)

        end

