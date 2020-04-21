        IDENT   DADDSUB
; COPYRIGHT 1981, RICHARD E. JAMES III
;
; THIS MODULE HANDLES DOUBLE PRECISION
;   ADD AND SUBTRACT FOR THE M68000.
;
; STACK:
;   ENTRY CONDITION:
;       TWO OPERANDS (16 BYTES) ON STACK
;   EXIT CONDITION: 
;       RESULT (8 BYTES) ON STACK
;
;       REGISTER CONVENTIONS.
;       D0/D1   SMALLER OPERAND (D0=MOST SIGNIFICANT)
;       D2/D3   LARGER OPERAND
;       D4      11 OR MASK OF 11 BITS
;       D5      SIGNS: SIGN OF .W =SIGN OF ANSWER
;                      SIGN OF .B =COMPARISON OF SIGNS
;       D6      EXPONENT OF LARGER
;       D7      EXPONENT OF SMALLER

NSAVED  EQU     8*4            ;8 REGS SAVED
                              
        GLOBAL  %D_ADD      
        GLOBAL  %D_SUB
        GLOBAL  __LFPERR
        EXTERN  %D_EXTE,%D_NORM,%D_RCP,%D_USEL,__LFPERR
NAN_OPERAND	EQU	$100
OVERFLOW	EQU	$300
ILLEGALOP	EQU 	$120
DENORM		EQU	$163
DIVZERO		EQU	$200

%D_SUB
;------
        BCHG    #7,4(SP)
%D_ADD
;------
; SAVE REGS AND LOAD OPNDS
        MOVEM.L D0-D7,-(SP)    ;SAVE    
        MOVEM.L NSAVED+4(SP),D0-D3
; (END OF ENTRY INTERFACE)
; REGISTERS NOW CONTAIN:
;       D0      SECOND ARGUMENT, MOST SIGNIFICANT 32 BITS
;       D1      SECOND ARGUMENT, LEAST SIGNIFICANT 32 BITS
;       D2      FIRST ARGUMENT, MOST SIGNIFICANT 32 BITS
;       D3      FIRST ARGUMENT, LEAST SIGNIFICANT 32 BITS

; EXTRACT SIGNS
        ASL.L   #1,D0          ;SIGN -> C
        SCS     D4           
        ASL.L   #1,D2          ;SIGN -> C
        SCS     D5
; REORDER
        CMP.L   D2,D0    
        BLS.S   DADD1          ;GET LARGER INTO D2/D3
        EXG     D0,D2
        EXG     D1,D3 
        EXG     D4,D5
DADD1   EXT.W   D5             ;SIGN OF LARGER
        EOR.B   D4,D5          ;COMPARISON OF SIGNS
; EXTRACT EXPONENTS
        JSR     %D_EXTE        ;LARGER -> D2/D3,D6; SMALLER -> D0/D1,D7
        TST.W   D7
        BNE.S   NUNFL          ;NOT ZERO OR DENORMALIZED
; SMALLER IS 0 OR DENORMALIZED     
        MOVE.L  D0,D4
        OR.L    D1,D4
        BEQ     USELN          ;IF SMALLER = 0, USE LARGER
;
;	we have at least one denormalized number -
;	raise the error ....
;
        MOVEM.L D0-D2/A0-A1,-(SP) ;SAVE    
	move.l	#DENORM,-(sp)
	jsr	__LFPERR
	addq.l	#4,sp
        MOVEM.L (SP)+,D0-D2/A0-A1    ;RESTORE REGISTERS
; (SIGN OF 0-0 UNPREDICTABLE)
        LSL.L   #1,D1
        ROXL.L  #1,D0
        TST.W   D6             ;LARGER EXP
        BNE.S   GU1            ;NOT GU
        LSL.L   #1,D3
        ROXL.L  #1,D2
        BRA.S   SHZ            ;BOTH GU, NO HIDDEN OR ALIGN NEEDED
NUNFL   BSET    #31,D0         ;ADD HIDDEN BIT
GU1     CMPI.W  #$7FF,D6
        BEQ.S   OVFL           ;INF/NAN
        BSET    #31,D2
; ALIGN SMALLER
        SUB.W   D6,D7          
        NEG.W   D7             ;D7 = DIFF OF EXPS
SHL     SUBQ.W  #8,D7   
        BLT.S   SHX            ;EXIT WHEN <8
        MOVE.B  D0,D1          ;SHIFT 8 BITS DOWN
        ROR.L   #8,D1
        LSR.L   #8,D0
        BNE.S   SHL            ;LOOP
        TST.L   D1
        BNE.S   SHL            ;LOOP WHILE NOT 0
        BRA.S   SHZ            ;ALL SIGNIFICANCE SHIFTED OFF
SHX     ADDQ.W  #7,D7
        BMI.S   SHZ
SHY     LSR.L   #1,D0          ;SHIFT ONE BIT AT A TIME
        ROXR.L  #1,D1
        DBRA    D7,SHY         ;FINAL PART OF ALIGNMENT
SHZ
; DECIDE WHETHER TO ADD OR SUBTRACT
        TST.B   D5             ;CMP SIGNS
        BMI.S   DIFF
; ADD THEM
        ADD.L   D1,D3          ;SUM
        ADDX.L  D0,D2
        BCC.S   ENDAS          ;NO C, OK
        ROXR.L  #1,D2          ;PULL OFL IN
        ROXR.L  #1,D3
        ADDQ.W  #1,D6     
        CMP.W   #$7FF,D6
        BLT.S   ENDAS          ;NO OFL
	MOVE.L	#OVERFLOW,D0
	BRA.S	GOSIGNAL
; SUBTRACT THEM
DIFF    SUB.L   D1,D3          ;SUBTRACT LOWERS
        SUBX.L  D0,D2          ;SUBTRACT UPPERS
        BCC.S   DIFF2
;  CANCELLED DOWN INTO 2ND WORD BUT GOT WRONG SIGN:
        NOT.W   D5             ;FLIP RESULT SIGN
        NEG.L   D3
        NEGX.L  D2             ;NEGATE VALUE
DIFF2   JSR     %D_NORM
; REJOIN, ROUND                             
ENDAS   JSR     %D_RCP         ;ROUND, CHECK, AND PACK
ASSGN   LSL.W   #1,D5          ;GET SIGN
        ROXR.L  #1,D2          ; SEU     PUT IN SIGN
; EXIT
; ANSWER IS NOW IN
;       D2      MOST SIGNIFICANT 32 BITS
;       D3      LEAST SIGNIFICANT 32 BITS
; (EXIT INTERFACE:)
ASEXIT  MOVEM.L D2/D3,NSAVED+3*4(SP)  ;RET
        MOVEM.L (SP)+,D0-D7
        MOVE.L  (SP)+,(SP)     ;RET ADDR
        MOVE.L  (SP)+,(SP)
        RTS


; EXCEPTION CASES

USELN   BSET    #31,D2         ;HIDDEN BIT, IN CASE LARGER IS NUMBER
USEL    JSR     %D_USEL        ;USE THE LARGER
        BRA.S   ASSGN

; LARGER EXPONENT = 1023:
OVFL    MOVE.L  D2,D4          ;LARGER MANTISSA
        OR.L    D3,D4
        BEQ.S   NONAN
	MOVE.L	#NAN_OPERAND,d0 ;LARGER=NAN, GENERATE EXCEPTION
	BRA.S	GOSIGNAL
NONAN
        CMP.W   D6,D7          ;EXPS
        BNE.S   USEL           ;LARGER=INF AND SMALLER=NUMBER
;(NEED NAN FOR PROJECTIVE, ...)
        TST.B   D5             ;COMPARISON OF SIGNS
        BPL.S   USEL           ;INF+INF=INF
                               ;INF-INF=NAN
;In affine mode, addition or subtraction of infinities of unlike sign
;is illegal.
	MOVE.L	#ILLEGALOP,D0
GOSIGNAL
        MOVEM.L D0-D2/A0-A1,-(SP) ;SAVE    
	MOVE.L	D0,-(SP)
	JSR	__LFPERR
	ADDQ.L	#4,sp
        MOVEM.L (SP)+,D0-D2/A0-A1    ;RESTORE REGISTERS
	CMP.L	#DIVZERO,D0
	BGE.S	GENINF
	CLR.L	d3
	MOVE.L	#$7ff00001,d2
	BRA.S	ASEXIT
	
; RESULT OVERFLOWS:
GENINF  MOVE.L  #$FFE00000,D2
        CLR.L   D3
        BRA.S   ASSGN

 
	END
