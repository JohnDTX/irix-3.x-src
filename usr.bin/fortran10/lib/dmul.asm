; File: dmul.text
; Date: 08-Oct-82
        
        IDENT   DMUL
        
; COPYRIGHT 1981, 1982, RICHARD E. JAMES III
 
; THIS MODULE PERFORMS DOUBLE PRECISION
; MULTIPLY FOR THE M68000.

; STACK ENTERS WITH RETURN ADDRESS
;   AND TWO LONG OPERANDS.
;       HIGH    LOWER PART OF FIRST OPERAND
;         .     UPPER PART OF FIRST OPERAND
;         .     LOWER PART OF SECOND OPERAND
;         .     UPPER PART OF SECOND OPERAND
;       LOW     RETURN ADDRESS

; RESULT IS LEFT ON TOP OF STACK.

; REGISTER CONVENTIONS:
;       D0-D3   OPERANDS OR PIECES OF RESULT
;       D5      EXPONENT OF LARGER
;       D5      TEMP FOR MULTIPLYING
;       D6      SIGN & EXPONENT
;       D7      EXPONENT OF SMALLER
;       D7      ZERO
 
NSAVED  EQU     8*4

        GLOBAL  %D_MUL
        EXTERN  %D_EXTE,%D_NRCP,%D_USEL
	GLOBAL	__LFPERR
	EXTERN  __LFPERR
NAN_OPERAND	EQU	$100
OVERFLOW	EQU	$300
ILLEGALOP	EQU 	$120
DENORM		EQU	$163
DIVZERO		EQU	$200

%D_MUL
;------
; LOAD RETURN ADDR AND OPNDS
        MOVEM.L D0-D7,-(SP)    ;SAVE
        MOVEM.L NSAVED+4(SP),D0-D3
; (END OF ENTRY INTERFACE)
; REGISTERS NOW CONTAIN:
;       D0      SECOND ARGUMENT, MOST SIGNIFICANT 32 BITS
;       D1      SECOND ARGUMENT, LEAST SIGNIFICANT 32 BITS
;       D2      FIRST ARGUMENT, MOST SIGNIFICANT 32 BITS
;       D3      FIRST ARGUMENT, LEAST SIGNIFICANT 32 BITS

; SAVE SIGN OF RESULT
        MOVE.L  D0,D5
        EOR.L   D2,D5          ;SIGN OF RESULT
        ASL.L   #1,D0          ;TOSS SIGN
        ASL.L   #1,D2   ;EEMMMMM0
;
;	gb - if we have Nan*Inf and msl(Nan) == msl(INF),
;	     the INF will be selected as the larger here, 
;	     and will not be checked at nanop.  Thus, if
;	     msl(a) == msl(b), we swap if lsl(a) == 0.
;	     
; ORDER OPERANDS (AT LEAST EXPONENTS)
;
;	this will place the larger op in d2/d3 UNLESS one
;	of the ops was zero, in which case this op is in d2/d3.
;
;
;
        CMP.L   D2,D0
;        BLS.S   ESWAP
        bcs.s	ESWAP
;
	move.l	d2,d6
	rol.l	#8,d6
	tst.b	d6
	beq.s	GOSWAP
	tst.l	d3
	bne.s	ESWAP
GOSWAP
        EXG     D0,D2          ;D2/D3=LARGER
        EXG     D1,D3
; EXTRACT AND CHECK EXPONENTS
ESWAP   JSR     %D_EXTE
        MOVE.W  D6,D5          ;LARGER EXP
        MOVE.L  D5,D6
        ADD.W   D7,D6          ;RESULT EXP (AND SIGN)
        CMP.W   #$7FF,D5       ;CK LARGER
        BEQ     BADOP          ;INF OR NAN
;
;	if d2/d3 == 0, a simple cmp.l and bls.s
;	will not find that the swap should occur, so 
;	we need to check for d2 having an exp of zero
;	specially.
;
	tst.w	d5
	bne.s	CONT
;
;	d2/d3 may be denormalized, check it....
;
	tst.l	d2
	bne.s	onedenorm
	tst.l	d2
	beq.s	CONT
onedenorm
;
;	denormalized number - generate exception...
;
        MOVEM.L D0-D2/A0-A1,-(SP) ;SAVE    
	move.l	#DENORM,-(sp)
	jsr	__LFPERR
	addq.l	#4,sp
        MOVEM.L (SP)+,D0-D2/A0-A1    ;RESTORE REGISTERS
NORMU1   SUBQ.W  #1,D5                                      ;NORM
        LSL.L   #1,D3          ;ADJ DENORMALIZED NUMBER
        ROXL.L  #1,D2
        BPL.S   NORMU1          ;NORM A DENORM              ;NORM
        ADDQ.W  #1,D5                                      ;NORM
CONT
        TST.W   D7             ;CK SMALLER
        BEQ     UFL            ;0 OR GU
; SET HIDDEN BITS
        BSET    #31,D0
BACK    BSET    #31,D2
; SPLIT MANTISSAS INTO 4 PIECES
        MOVEM.L D0-D3,-(SP)    ;STORE 8 WORDS
        MOVEM.W (SP),D0-D3     ;RELOAD ONE OPERAND, SPREAD OUT
        MOVE.W  4*2(SP),D5     ;0
        MULU    D5,D0          ;00
        MULU    D5,D1          ;   01
        MULU    D5,D2          ;      02
        MULU    D5,D3          ;         03
        CLR.L   D7             ;USED FOR ADDX INSTRUCTON

        MOVE.W  2*2(SP),D5
        MULU    5*2(SP),D5
        ADD.L   D5,D3          ;         12
        ADDX.L  D7,D1

        MOVE.W  1*2(SP),D5 
        MULU    6*2(SP),D5 
        ADD.L   D5,D3          ;         21
        ADDX.L  D7,D1

        MOVE.W     (SP),D5 
        MULU    7*2(SP),D5 
        ADD.L   D5,D3          ;         30
        ADDX.L  D7,D1

        MOVE.W  1*2(SP),D5
        MULU    5*2(SP),D5
        ADD.L   D5,D2          ;      11
        ADDX.L  D7,D0

        MOVE.W     (SP),D5 
        MULU    6*2(SP),D5 
        ADD.L   D5,D2          ;      20
        ADDX.L  D7,D0

        SWAP    D0
        MOVE.W     (SP),D5 
        MULU    5*2(SP),D5 
        ADD.L   D5,D1          ;   10
        ADDX.W  D7,D0
        SWAP    D0
        ADDA.W  #4*4,SP
; ADD TOGETHER
                               ;01--  IN D0
                               ;-12-  IN D1
                               ;--23  IN D2
                               ;---34 IN D3
        MOVE.W  D1,D3
        SWAP    D3             ;--23
        CLR.W   D1
        SWAP    D1             ;-1--
        ADD.L   D2,D3          ;--23
        ADDX.L  D7,D0
        ADD.L   D1,D0          ;01--
; PUT TOGETHER
        MOVE.L  D0,D2
        SUBI.W  #1022,D6       ;TOSS XTRA BIAS
	CMP.W	#$7FF,d6
	BLT.S	GOBUILD
        MOVEM.L D0-D2/A0-A1,-(SP) ;SAVE    
	MOVE.L	#OVERFLOW,-(sp)
	JSR	__LFPERR
	ADDQ.L	#4,sp
        MOVEM.L (SP)+,D0-D2/A0-A1    ;RESTORE REGISTERS
;
;	result is INF
;
	clr.l	d3
	move.l	#$7ff00000*2,d2
	bra.s 	MSIGN
GOBUILD
        JSR     %D_NRCP        ;NORM,RND,CK,PACK
MSIGN   ROXL.L  #1,D6
        ROXR.L  #1,D2          ;APPEND SIGN
; PUT ANSWER ON STACK AND EXIT

; ANSWER IS NOW IN
;       D2      MOST SIGNIFICANT 32 BITS
;       D3      LEAST SIGNIFICANT 32 BITS
; (EXIT INTERFACE:)
MEXIT   MOVEM.L D2/D3,NSAVED+3*4(SP)
        MOVEM.L (SP)+,D0-D7
        MOVE.L  (SP)+,(SP)     ;RETURN ADDR
        MOVE.L  (SP)+,(SP)
        RTS


; EXCEPTION HANDLING
BADOP
;
;	larger op is either INF or Nan.
;
;	check for Nan - true iff d2/d3 are zero.
;
	tst.l	d2
	bne.s	nanop
	tst.l	d3
	beq.s	ckinfz
nanop
;
;	Nan as operand.... signal it.
;
	MOVE.L	#NAN_OPERAND,d1
	BRA.S	GORAISE
;
;	check for inf * 0
;
ckinfz
	MOVE.L  D2,D5          ;LARGER MANTISSA, IF IT IS NAN, USE IT
        OR.W    D7,D5          ;SMALLER EXP
        OR.L    D0,D5
        OR.L    D1,D5          ;SMALLER VALUE
	BNE.S	REALOFL		
INFZERO
;
;		INF * 0 
;
;        TST.L   d0
;        BNE.S   REALOFL		; inf * (x!=0), result=inf
;	TST.L	d1
;        BNE.S   REALOFL		; inf * (x!=0), result=inf
;
	MOVE.L	#ILLEGALOP,d1
GORAISE
        MOVEM.L D0-D2/A0-A1,-(SP) ;SAVE    
	MOVE.L	d1,-(SP)
	JSR	__LFPERR
	ADDQ.L	#4,sp
        MOVEM.L (SP)+,D0-D2/A0-A1    ;RESTORE REGISTERS
; 
;	result is Nan
;
	clr.l	d3
	move.l	#$7ff00001,d2
	BRA.S	MEXIT
;
;	True overflow.....
;
REALOFL     
	MOVE.L	#$7ff00000,d2
	CLR.L	d3
	BRA.S	MEXIT

UFL     MOVE.L  D0,D7
        OR.L    D1,D7          ;MANTISSA OF SMALLER
        BEQ.S   SIGNED0        ;0 * NUMBER
;
;	denormalized number - generate exception...
;
        MOVEM.L D0-D2/A0-A1,-(SP) ;SAVE    
	move.l	#DENORM,-(sp)
	jsr	__LFPERR
	addq.l	#4,sp
        MOVEM.L (SP)+,D0-D2/A0-A1    ;RESTORE REGISTERS
NORMU   SUBQ.W  #1,D6                                      ;NORM
        LSL.L   #1,D1          ;ADJ DENORMALIZED NUMBER
        ROXL.L  #1,D0
        BPL.S   NORMU          ;NORM A DENORM              ;NORM
        ADDQ.W  #1,D6                                      ;NORM
        BRA     BACK                                       ;NORM
; (IF BOTH ARE DENORMALIZED, ANSWER WILL BE ZERO ANYWAY)   ;NORM

;GENNAN  MOVE.L  #$7FF00002,D2
;        BRA.S   MEXIT

SIGNED0 CLR.L   D2
        CLR.L   D3
        BRA   MSIGN

        END
