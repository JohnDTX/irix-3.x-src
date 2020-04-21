; File: mul.text
; Date: 08-Oct-82
        
        IDENT   MUL
        
; COPYRIGHT 1981, 1982, RICHARD E. JAMES III
 
; THIS MODULE PERFORMS SINGLE PRECISION MULTIPLY FOR THE M68000.

        GLOBAL  %F_MUL

; STACK ENTERS WITH RETURN ADDRESS AND TWO LONG OPERANDS.

; RESULT IS LEFT ON TOP OF STACK.

; REGISTER CONVENTIONS:
;       D0      OPND1/UPPER1
;       D1      OPND2/UPPER2
;       D2      9/LOWER1
;       D3      LOWER2
;       D4      EXP 
;       D5      SIGN 

	GLOBAL	__FPERR
	EXTERN  __FPERR
NAN_OPERAND	EQU	$100
OVERFLOW	EQU	$300
ILLEGALOP	EQU 	$120
DENORM		EQU	$163
DIVZERO		EQU	$200
 
NSAVED  EQU     6*4

        EXTERN  %F_RCP
%F_MUL
;------
; LOAD RETURN ADDR AND OPNDS
        MOVEM.L D0-D5,-(SP)    ;SAVE
        MOVEM.L NSAVED+4(SP),D0/D1
; (END OF INTERFACE)
;       D0,D1:  OPERANDS

; SAVE SIGN OF RESULT
        MOVE.L  D0,D5
        EOR.L   D1,D5
        ASL.L   #1,D0          ;TOSS SIGN
        ASL.L   #1,D1          ;EEMMMMM0
        CMP.L   D1,D0
; ORDER OPERANDS
        BLS.S   ESWAP
        EXG     D0,D1          ;D1=LARGER
; EXTRACT AND CHECK EXPONENTS
ESWAP   ROL.L   #8,D0
        ROL.L   #8,D1          ;MMMMM0EE
        CLR.W   D4
        MOVE.B  D0,D4
        CLR.W   D3
        MOVE.B  D1,D3
        ADD.W   D3,D4          ;RSLT EXP
        CMP.B   #$FF,D1
        BEQ.S   BADOP            ;INF OR NAN
        TST.B   D0
        BEQ   UFL            ;0 OR GU (DENORMALIZED)
; CLEAR EXP; SET HIDDEN BIT
        MOVE.B  #1,D0
        ROR.L   #1,D0
BACK    MOVE.B  #1,D1
        ROR.L   #1,D1
; SPLIT MANTISSAS INTO 2 PIECES
        MOVE.W  D0,D2          ;LOWER
        MOVE.W  D1,D3
        SWAP    D0             ;UPPER
        SWAP    D1
; MULTIPLY THE PIECES
        MULU    D0,D3          ;U1*L2
        MULU    D1,D2          ;U2*L1
        MULU    D1,D0          ;U2*U1
; ADD TOGETHER
        ADD.L   D2,D3          ;MIDDLE PRODS
        ADDX.B  D3,D3          ;CARRY TO BIT 0
        ANDI.W  #1,D3          ;TOSS REST
        SWAP    D3             ;QUICK ROT 16
        ADD.L   D3,D0          ;(CANT OFL)
        SUBI.W  #126,D4        ;TOSS XTRA BIAS
	cmp.w	#$ff,d4
	bge.s	OFL
        JSR     %F_RCP         ;RND,CK,PACK

; BUILD ANSWER
MBUILD  ROR.L   #8,D0
        ROXL.L  #1,D5
        ROXR.L  #1,D0          ;APPEND SIGN

;       D0 = ANSWER
; (EXIT INTERFACE:)
MEXIT   MOVE.L  D0,NSAVED+2*4(SP)
        MOVEM.L (SP)+,D0-D5    ;RESTORE REGISTERS
        MOVE.L  (SP)+,(SP)     ;RETURN ADDRESS
        RTS

; EXCEPTION HANDLING
 
OFL
;
;	overflow was GENERATED.
;
	MOVE.L	#OVERFLOW,d1
	BRA.S	GORAISE
BADOP
;
;	larger input was INF or Nan
;
        CLR.B    d1
        TST.L    d1		; is larger Nan?
        BEQ.S    OFL1		; no, larger is INF.
; 	
;	larger is Nan
;
	MOVE.L	#NAN_OPERAND,d1
	BRA.S	GORAISE	
OFL1
;
;	larger is infinity....  check for INF * 0
;
        TST.L    d0
        BNE.S    MNI		; inf * (x!=0), result=inf
	MOVE.L	#ILLEGALOP,d1
GORAISE
        MOVEM.L D0-D2/A0-A1,-(SP) ;SAVE    
	MOVE.L	d1,-(SP)
	JSR	__FPERR
	ADDQ.L	#4,sp
        MOVEM.L (SP)+,D0-D2/A0-A1    ;RESTORE REGISTERS
;
;	result is NAN unless DIVZERO or OVERFLOW
;
	move.l	#$FF,d0
	cmp.l	#DIVZERO,d1
	bge	MBUILD
	move.l	#$7f800001,d0
	bra	MEXIT

MNI
        MOVE.B  #$ff,d1
        MOVE.L  d1,d0
        BRA.S   MBUILD

UFL     TST.L   D0             ;MANTISSA OF SMALLER
        BEQ.S   MBUILD         ;0*
; NORMALIZING MODE IS EMBODIED IN THE NEXT FEW LINES:      ;NORM
        BMI   BACK                                       ;NORM
;
;	have to raise 'denormalized operand' exception
;
        MOVEM.L D0-D2/A0-A1,-(SP) ;SAVE    
	move.l	#DENORM,-(sp)
	jsr	__FPERR
	addq.l	#4,sp
        MOVEM.L (SP)+,D0-D2/A0-A1    ;RESTORE REGISTERS
NORMDEN SUBQ.W  #1,D4          ;ADJ EXP                    ;NORM
        LSL.L   #1,D0          ;NORMALIZE DENORMAL NUMBER  ;NORM
        BPL.S   NORMDEN                                    ;NORM
        BRA     BACK                                       ;NORM
GENNAN  MOVE.L  #$7F800002,D0  ;NAN 2
        BRA   MEXIT
SIGNED0 CLR.L   D0 
        BRA   MBUILD

        END
