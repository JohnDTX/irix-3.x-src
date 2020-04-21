        IDENT   ADDSUB
; COPYRIGHT 1981, RICHARD E. JAMES III
;
;*********************************************************************
;
;	updated 10/4/85 to do exception processing for FORTRAN/Pascal
;	using the C exception processing system.  -- G.Boyd
;
;*********************************************************************
;
;
;
; THIS MODULE HANDLES SINGLE PRECISION ADD AND SUBTRACT FOR THE M68000.

        GLOBAL  %F_ADD
        GLOBAL  %F_SUB
	GLOBAL	__FPERR
	EXTERN	__FPERR

; STACK:
;   ENTRY CONDITION:
;       TWO OPERANDS ON STACK
;   EXIT CONDITION: 
;       RESULT ON STACK
;
; INTERNAL REGISTER CONVENTIONS.
;       D0      OPND1    
;       D1      OPND2/SUM
;       D2      SIGN OF OPND1
;       D3      SIGN OF OPND2
;       D4      EXP OF OPND1
;       D5      EXP OF OPND2
;       D6      DIFF OF EXPS
;       D7      RESERVED FOR STICKY

NSAVED  EQU     7*4            ;7 REGS SAVED + one long for optype

NAN_OPERAND	EQU	$100
OVERFLOW	EQU	$300
ILLEGALOP	EQU 	$120
DENORM		EQU	$163
DIVZERO		EQU	$200

;
;	local storage for the current operation-in-progress
;

%F_SUB
;------
        BCHG    #7,4(SP)
%F_ADD
;------
; SAVE REGS AND LOAD OPNDS
        MOVEM.L D0-D6,-(SP) ;SAVE    
        MOVEM.L NSAVED+4(SP),D0/D1
; (END OF INTERFACE)
;       D0 = SECOND OPERAND (WITH SIGN FLIPPED FOR SUB)
;       D1 = FIRST OPERAND
; EXTRACT SIGNS
        ASL.L   #1,D0          ;SIGN -> C
        SCS     D2
        ASL.L   #1,D1          ;SIGN -> C
        SCS     D3
; REORDER
        CMP.L   D1,D0
        BLS.S   ADD2           ;GET LARGER
        EXG     D0,D1
        EXG     D2,D3
; EXTRACT EXPONENTS
ADD2    ROL.L   #8,D1
        ROL.L   #8,D0
        CLR.L   D5
        MOVE.B  D1,D5          ;LARGER EXP
        CLR.W   D4
        MOVE.B  D0,D4          ;SMALLER EXP
        BNE.S   NUNFL          ;NOT 0 OR DENORMALIZED
; SMALLER IS 0 OR DENORMALIZED     
        TST.L   D0             ;IF SMALLER=0
;
;	NOTE! 0 +/- Nan is not caught by the exception handling routine. - gb
;
        BEQ   	USEL           ;USE LARGER (SIGN OF 0-0 UNPREDICTABLE)
;
;	denormalized number as input -- raise exception.
;
        MOVEM.L D0-D2/A0-A1,-(SP) ;SAVE    
	move.l	#DENORM,-(sp)
	jsr	__FPERR
	addq.l	#4,sp
        MOVEM.L (SP)+,D0-D2/A0-A1    ;RESTORE REGISTERS
        TST.B   D5             ;LARGER EXP
        BNE.S   GU1            ;NOT GU
; BOTH ARE DENORMALIZED
        CMP.B   D3,D2          ;COMPARE SIGNS
        BNE.S   GUSUB
        ADD.L   D0,D1          ;ADD
        ADDX.B  D1,D1          ;INCR EXP TO 1 IF OVERFLOW
        BRA.S   ASBUILD
GUSUB   SUB.L   D0,D1          ;SUBTRACT
        BRA.S   ASBUILD
; NEITHER IS DENORMALIZED
NUNFL   MOVE.B  #1,D0          ;CLR EXP AND
        ROR.L   #1,D0
GU1     MOVE.B  #1,D1          ;ADD HIDDEN BIT
        ROR.L   #1,D1
GU2     CMPI.B  #$FF,D5
        BEQ.S   OVFL           ;INF/NAN
; ALIGN SMALLER
        MOVE.W  D5,D6
        SUB.W   D4,D6          ;DIFFERENCE OF EXPS
SHL     CMPI.W  #8,D6
        BLT.S   SHX            ;EXIT WHEN <8 *S
        SUBQ.W  #8,D6
        LSR.L   #8,D0          ;ALIGN
        BNE.S   SHL            ;LOOP
SHX     LSR.L   D6,D0          ;FINISH ALIGN
        CMP.B   D3,D2          ;CMP SIGNS
        BNE.S   DIFF           ;DECIDE WHETHER TO ADD OR SUBTRACT

; ADD THEM
        ADD.L   D0,D1          ;SUM
        BCC.S   ENDAS          ;NO C, OK
        ROXR.L  #1,D1          ;PULL IN OVERFLOW *S
        ADDQ.W  #1,D5
        CMP.W   #$FF,D5
        BLT.S   ENDAS          ;NO OFL
        BRA   REALOFL

; SUBTRACT THEM
DIFF    SUB.L   D0,D1
        BMI.S   ENDAS          ;IF NORMORMALIZED
        BEQ   CANCEL         ;RESULT=0
NORM    ASL.L   #1,D1          ;NORMALIZE
        DBMI    D5,NORM        ;DEC EXP
        SUBQ.W  #1,D5
        BGT.S   ENDAS          ;NOT GU
        BEQ.S   NORM2
        CLR.W   D5
        LSR.L   #1,D1          ;GRAD UND
NORM2   LSR.L   #1,D1

ENDAS
; ROUND (NOT FULLY STANDARD)
        ADDI.L  #$80,D1        ;ROUND
        BCC.S   ROFL           ;ROUND DID NOT CAUSE MANTISSA TO OFL
        ROXR.L  #1,D1
        ADDQ.W  #1,D5
        CMPI.W  #$FF,D5
        BEQ.S   REALOFL         ;ROUND CAUSED EXP TO OVERFLOW
; REBUILD ANSWER
ROFL    LSL.L   #1,D1          ;TOSS HIDDEN
USEL    MOVE.B  D5,D1          ;INSERT EXP
ASBUILD ROR.L   #8,D1
        ROXR.B  #1,D3
        ROXR.L  #1,D1          ;APPLY SIGN
;       D1 = ANSWER
; (EXIT INTERFACE:)
ASEXIT  MOVE.L  D1,NSAVED+2*4(SP)      ;PUT RESULT ON STACK
        MOVEM.L (SP)+,D0-D6    ;RESTORE REGISTERS
        MOVE.L  (SP)+,(SP)     ;RET ADDR
        RTS

; EXCEPTION CASES

; LARGER EXPONENT = 255:
OVFL    LSL.L   #1,D1
        TST.L   D1             ;LARGER MANT
	BEQ.S	OVFL1
;
;	larger is NAN - generate the exception.
;
	MOVE.L	#NAN_OPERAND,d0
	BRA.S	_F_ADD_GOSIGNAL
;        BNE.S   USEL           ;LARGER=NAN
OVFL1
        CMP.B   D4,D5          ;EXPS
        BNE.S   USEL           ;LARGER=INF
; AFFINE MODE ASSUMED IN THIS IMPLEMENTATION
        CMP.B   D3,D2          ;SIGNS
        BEQ.S   USEL           ;INF+INF=INF
;
;	in affine mode,
;	the addition or subtraction of unlike infinities are illegal.
;		- generate the exception
;
	MOVE.L	#ILLEGALOP,D0
_F_ADD_GOSIGNAL
;
;	call _raise_fperror:
;		_raise_fperror(operation,errorcode)
;
        MOVEM.L D0-D2/A0-A1,-(SP) ;SAVE    
	MOVE.L	D0,-(SP)	;errorcode
	JSR	__FPERR		; never returns
	ADDQ.L	#4,SP
        MOVEM.L (SP)+,D0-D2/A0-A1    ;RESTORE REGISTERS
	cmp.l	#DIVZERO,d0
	bge	GENINF
;
;	result is Nan unless OVERFLOW
;
GENNAN  MOVE.L  #$7F800001,D1     
        BRA.S   ASEXIT 
 
; COMPLETE CANCELLATION
CANCEL  CLR.L   D1             ;RESULT=0
; (NEED MINUS 0 FOR (-0)+(-0), ROUND TO -INF)
        BRA.S   ASEXIT
; RESULT OVERFLOWS:
;
;	generate the overflow exception.
;
REALOFL  
	MOVE.L	#OVERFLOW,D0
	BRA.S	_F_ADD_GOSIGNAL
GENINF
	MOVE.L  #$FF,D1        ;MAX EXP
        BRA.S   ASBUILD

        END
