; File: dvmd.text
; Date: 21-Mar-83

        IDENT   DVMD
        
; COPYRIGHT 1981, 1982, RICHARD E. JAMES III
 
        GLOBAL  %D_DIV
	GLOBAL	__LFPERR
	EXTERN	__LFPERR

NAN_OPERAND	EQU	$100
OVERFLOW	EQU	$300
ILLEGALOP	EQU 	$120
DIVZERO		EQU	$200
DENORM		EQU	$163

; THIS SUBROUTINE PERFORMS DOUBLE PRECISION DIVIDE AND MOD FOR M68000.

; STACK ENTERS WITH RETURN ADDRESS
;   AND TWO LONG OPERANDS.
;       HIGH    LOWER PART OF FIRST OPERAND
;         .     UPPER PART OF FIRST OPERAND
;         .     LOWER PART OF SECOND OPERAND
;         .     UPPER PART OF SECOND OPERAND
;       LOW     RETURN ADDRESS

; RESULT IS LEFT ON TOP OF STACK.

; REGISTER CONVENTIONS:
;       D0/D1   DIVISOR (D0=UPPER)
;       D2/D3   DIVIDEND
;       D4      COUNT
;       D5      SIGN & EXPONENT
;       D6      RESULT EXPONENT
;       D6/D7   QUOTIENT
 
NSAVED  EQU     8*4

        EXTERN  %D_NRCP,%D_NORM,%D_RCP
%D_DIV
;------
; LOAD RETURN ADDR AND OPNDS
        MOVEM.L D0-D7,-(SP)    ;SAVE
        MOVEM.L NSAVED+4(SP),D0-D3
; (END OF ENTRY INTERFACE)
; REGISTERS NOW CONTAIN:
;       D0      DIVISOR, MOST SIGNIFICANT 32 BITS
;       D1      DIVISOR, LEAST SIGNIFICANT 32 BITS
;       D2      DIVIDEND, MOST SIGNIFICANT 32 BITS
;       D3      DIVIDEND, LEAST SIGNIFICANT 32 BITS

; SAVE SIGN OF RESULT
        MOVE.L  D0,D5
        EOR.L   D2,D5          ;SIGN OF RESULT
        CLR.L   D4             ;FLAG FOR DIV
        BSR.S   EXTREM
; COMPUTE RESULTING EXPONENT
        MOVE.W  D6,D5
        SUB.W   D7,D5          ;EXP (AND SIGN)
; DO TOP 30-31 BITS OF DIVIDE (%D_RCP WILL TO POST NORMALIZE)
        MOVE.W  #30,D4         ;COUNT 30..0
        BSR     SHSUB          ;SHIFT AND SUBTRACT LOOP
        MOVE.L  D7,D6          ;TOP OF ANSWER
; DO NEXT 22 BITS OF DIVIDE
        MOVE.W  #23,D4         ;COUNT 23..0 (TOTAL = 54-55 BITS)
        BSR     SHSUB
; PUT TOGETHER ANSWER
        LSL.L   #8,D7          ;LINE UP BOTTOM
        OR.L    D3,D2
        BEQ.S   DD4
        BSET    #1,D7          ;STICKY BIT ON IF REMAINDER <> 0
DD4     LSL.L   #1,D7
        ROXL.L  #1,D6          ;ONE NORM
; NORMALIZE (ONCE), ROUND, CHECK RESULT EXP, PACK
        MOVE.L  D6,D2          ;UPPER
        MOVE.L  D7,D3          ;LOWER
        MOVE.W  D5,D6          ;EXPONENT
        ADDI.W  #1022+1,D6     ;RE-BIAS
        JSR     %D_NRCP        ;NORM ONCE,RND,CK,PACK
        BRA     DMSIGN
        PAGE    ;--------------
; SUBROUTINE FOR UNPACKING ONE OPERAND, AND NORMALIZING A DENORMAL.
; INPUT
;       D0/D1   NUMBER

; OUTPUT
;       D0/D1   MANTISSA
;       D7.W    EXPONENT
;       Z       ON IFF MANTISSA IS ZERO

; UNCHANGED
;       D4      BOTTOM = $07FF

UNP
;------
        MOVE.L  D0,D7          ;START GETTING EXP          ;NORM
        ANDI.L  #$000FFFFF,D0  ;CLEAR OUT SIGN AND EXP     ;NORM
        SWAP    D7                                         ;NORM
        LSR.W   #16-1-11,D7                                ;NORM
        AND.W   D4,D7          ;EXPONENT                   ;NORM
        BNE.S   UNPXIT         ;NORMAL NUMBER              ;NORM
; DENORMALIZED NUMBER OR ZERO:                             ;NORM
        TST.L   D0             ;UPPER                      ;NORM
        BNE.S   UNP2                                       ;NORM
        TST.L   D1             ;LOWER                      ;NORM
        BEQ.S   UNPXIT         ;ZERO                       ;NORM
UNP2    
;
;	value is denormalized
;
        MOVEM.L D0-D2/A0-A1,-(SP) ;SAVE    
	move.l	#DENORM,-(sp)
	jsr	__LFPERR
	addq.l	#4,sp
        MOVEM.L (SP)+,D0-D2/A0-A1    ;RESTORE REGISTERS
	ADDQ.W  #1,D7                                      ;NORM
UNPL    SUBQ.W  #1,D7                                      ;NORM
        LSL.L   #1,D1                                      ;NORM
        ROXL.L  #1,D0          ;NORMALIZE                  ;NORM
        BTST    #20,D0                                     ;NORM
        BEQ.S   UNPL           ;LOOP UNTIL NORMALIZED      ;NORM
UNPXIT  RTS                                                ;NORM

; SUBROUTINE FOR EXTRACTING AND TAKEING CARE OF 0/GU/INF/NAN.

; INPUT
;       D0/D1, D2/D3
;       D4      + FOR DIV; - FOR MOD

; OUTPUT
;       D0/D1 (BOTTOM) AND D2/D3 (TOP) CONVERTED TO
;               000XXXXX/XXXXXXXX
;       D6      EXPONENT OF TOP
;       D7      EXPONENT OF BOTTOM

; UNCHANGED
;       D5      SIGN

EXTREM
;------
        MOVE.W  #$7FF,D4       ;MASK, SIGN.L UNTOUCHED     ;NORM
        EXG     D2,D0                                      ;NORM
        EXG     D3,D1                                      ;NORM
        BSR.S   UNP            ;UNPACK TOP                 ;NORM
        EXG     D0,D2                                      ;NORM
        EXG     D1,D3                                      ;NORM
        EXG     D7,D6                                      ;NORM
        BEQ.S   TOPZERO        ;TOP IS ZERO                ;NORM
        CMP.W   D4,D6
        BEQ.S   TOPBIG         ;TOP IS INF OR NAN
        BSET    #20,D2         ;SET HIDDEN BIT
TOPZERO BSR.S   UNP            ;UNPACK BOTTOM              ;NORM
        BEQ.S   BOTZERO        ;BOTTOM IS 0                ;NORM
        CMP.W   D4,D7
        BEQ   BOTBIG         ;BOTTOM IS INF OR NAN
        BSET    #20,D0
        LSL.L   #1,D1          ;LEFT SHIFT BOTTOM
        ROXL.L  #1,D0
        RTS

; EXCEPTION HANDLING
                                                           ;NORM
; TOP IS INF OR NAN
TOPBIG  
;
;	if UNP returns with Z set, the value was zero...
;	for compatibility, x/0 or mod(x,0) here gives DIVZERO
;
	BSR.S   UNP            ;UNPACK BOTTOM              ;NORM
	beq.s	dvzero
        TST.L   D4                                         ;NORM
        BMI.S   mextrem         ;top is INF or NAN with mod. 
                               ;INF OR NAN / ...
                                                           ;NORM
        CMP.W   D6,D7
        BEQ.S   bothext        ;both the same and either inf or nan
        TST.L   D2             ;TOP
        bne.s   nanop         
	tst.l	d3
	bne.s	nanop
	tst.w	d7		; is bottom zero?
	bne.s	GENINF
	or.l	d0,d1		; check bottom for zero.
	bne.s	GENINF
;
;	INF/0
;
        BRA.S   dvzero         ;raise divide by zero exception for x/0
;
;	top is INF. This is INF unless the bottom is zero.
;
	
        BRA.S   nanop          ;NAN/.. -> illegal op

; BOTTOM IS 0                                              ;NORM
BOTZERO 
;
;	let mod(x,0) raise DIVZERO to match h/w
;
;	TST.L   D4                                         ;NORM
;       BMI.S   illop          ;raise illegalop exception for mod(x,0)
;
;	raise divide by zero exception for x/0 AND 0/0
;
dvzero
	move.l	#DIVZERO,d0
	bra.s	goraise
illop
	move.l	#ILLEGALOP,d0
	bra.s	goraise
nanop
	move.l	#NAN_OPERAND,d0
goraise
        MOVEM.L D0-D2/A0-A1,-(SP) ;SAVE    
	move.l	d0,-(sp)
	jsr	__LFPERR
	addq.l	#4,sp
        MOVEM.L (SP)+,D0-D2/A0-A1    ;RESTORE REGISTERS
;
;	the result is Nan, unless DIVZERO or OVERFLOW
;
	cmp.l	#DIVZERO,d0
	bge.s	GENINF
	clr.l	d3
	move.l	#$7ff00001,d2
;
;	now adjust the stack, due to the bsr to extrem
;
	addq.l	#4,sp
	bra	DMEXIT

; GENERATE INFINITY FOR ANSWER
GENINF  MOVE.L  #$7FF00000*2,D2  ;INFINITY
        BRA.S   CLRBOT         ;= INF

mextrem
;
;	the top is either NAN or INF
;
	tst.l	d2
	bne.s	nanop		; top is Nan
	tst.l	d3
	bne.s	nanop		; top is Nan
;	bra.s	illop 		; mod(INF,x)
;
;	for now, do the same thing as the fpa 
;	(i.e., OVERFLOW)
;
	move.l	#OVERFLOW,d0
	bra.s	goraise
bothext
;
;	both the bottom and top are extreme.
;
	or.l	d0,d1
	bne.s	nanop
	or.l	d2,d3
	bne.s	nanop
;
;	both INF
;
	bra.s	illop

BOTBIG  TST.L   D0
        BNE.S   nanop          ;BOTTOM = NAN, RESULT = NAN
	TST.L	D1
	BNE.S	nanop
        TST.L   D4
        BPL.S   GENZERO        ;... / INF = 0
        ADDQ.W  #4,SP
        BRA     USETOP         ;DMOD(TOP, INF) = TOP

; INVALID OPERAND/OPERATION
;ISNAN   CMP.W   D7,D6
;        BNE.S   ISN2           ;EXPONENTS EQUAL
;        CMP.L   D0,D2
;ISN2    BGE.S   ISN4
;        MOVE.W  D7,D6          ;GET LARGER NUMBER
;        MOVE.L  D0,D2
;ISN4    SWAP    D2
;        LSL.W   #16-11-1,D6
;        OR.W    D6,D2          ;PUT BACK TOGETHER
;        SWAP    D2
;        LSL.L   #1,D2
;        CMPI.L  #$7FF00000*2,D2
;        BHI.S   GOTNAN         ;USE LARGER NAN
;
;GENNAN  MOVE.L  #$7FF00004*2,D2
;        TST.L   D4
;        BPL.S   GOTNAN         ;NAN 4 FOR DIV
;        ADDQ.L  #2,D2          ;NAN 5 FOR MOD
;GOTNAN  CLR.L   D5             ;GENERATE ONLY +NAN
;        BRA.S   CLRBOT

GENZERO CLR.L   D2             ;RESULT = 0

CLRBOT  CLR.L   D3
SIGN    ADDQ.W  #4,SP          ;POP
        BRA     DMSIGN
        PAGE    ;--------------
; THE SHSUB SUBROUTINE DOES A SHIFT-SUBTRACT LOOP.
; THAT IS THE HEART OF DIVIDE AND MOD.
;   THE ALGORITHM IS A SIMPLE SHIFT AND SUBTRACT LOOP, 
;     BUT IT ADDS WHEN IT OVERSHOOTS.
;   WHY NOT USE THE DIVS/DIVU INSTRUCTIONS?  THAT APPROACH IS SLOWER!

; REGISTERS
;       D2/D3   CURRENT DIVIDEND (UPDATED)
;       D0/D1   DIVISOR (UNCHANGED)
;       D4.W    (INPUT) NUMBER OF ITERATIONS -1, AND BIT NUMBER
;       D5/D6   -UNTOUCHED-
;       D7      QUOTIENT BEING DEVELOPED (IGNORED BY MOD)

SHSUB
;------
        CLR.L   D7             ;QUOTIENT
; SHIFT ONCE, SEE IF SUBTRACT NEEDED
SHS1    ADD.L   D3,D3
        ADDX.L  D2,D2          ;(64-BIT LEFT SHIFT)
        CMP.L   D0,D2
        DBGE    D4,SHS1        ;LOOP WHILE DIVIDEND IS SMALL
; TALLY QUOTIENT AND SUBTRACT
        BSET    D4,D7          ;QUOTIENT BIT
        SUB.L   D1,D3
        SUBX.L  D0,D2          ;64-BIT SUBTRACT
        DBMI    D4,SHS1        ;LOOP D4 TIMES
; NOW ONE OF THREE THINGS HAS HAPPENED:
;  1.  COUNT EXHAUSTED AND EXTRA SUBTRACT DONE (FIRST DB HIT COUNT)
;  2.  COUNT EXHAUSTED IN SECOND DB
;  3.  OVERSHOT BECAUSE COMPARE DIDN'T CHECK LOWER PARTS
        BPL.S   SHS7           ;CASE 2
        ADD.L   D1,D3          ;TAKE CARE OF OVERSHOOT
        ADDX.L  D0,D2
        BCLR    D4,D7
        TST.W   D4
        DBLT    D4,SHS1        ;CASE 3
                               ;CASE 1
SHS7    RTS
        PAGE    ;--------------

	    GLOBAL  DMOD

DMOD
        MOVEM.L D0-D7,-(SP)    ;PRESERVE ALL REGS THAT WILL BE USED
        MOVEM.L NSAVED+4(SP),D0-D3  ;OPERANDS
; (END OF ENTRY INTERFACE)
; REGISTERS NOW CONTAIN:
;       D0      SECOND ARGUMENT, MOST SIGNIFICANT 32 BITS
;       D1      SECOND ARGUMENT, LEAST SIGNIFICANT 32 BITS
;       D2      FIRST ARGUMENT, MOST SIGNIFICANT 32 BITS
;       D3      FIRST ARGUMENT, LEAST SIGNIFICANT 32 BITS

        MOVE.L  D2,D5          ;SAVE SIGN OF TOP FOR RESULT
        MOVEQ   #-1,D4         ;MOD FLAG
        BSR     EXTREM         ;UNPACK AND CK END CASES
        MOVE.W  D6,D4          ;TOP EXP
        SUBQ.W  #1,D7
        SUB.W   D7,D4          ;NUMBER OF ITERATIONS = TOP - BOT +1
        BLE.S   USETOP         ;TOP IS THE ANSWER
        LSL.L   #1,D1          ;ADJUST BOT SO THAT BOT>TOP
        ROXL.L  #1,D0
        MOVE.W  D7,D6          ;RESULT USES BOTTOM EXP
        SUBQ.W  #1,D6
        BSR.S   SHSUB          ;DO MAIN WORK
USETOP  ADDI.W  #11,D6
        JSR     %D_NORM        ;NORMALIZE
        JSR     %D_RCP         ;ADJUST EXP, CK EXTREMES, PACK

; COMMON EXIT FOR DIVIDE AND MOD:
; APPEND SIGN
DMSIGN  ROXL.L  #1,D5          ;SIGN->X
        ROXR.L  #1,D2          ;INSERT SIGN
; ANSWER IS NOW IN
;       D2      MOST SIGNIFICANT 32 BITS
;       D3      LEAST SIGNIFICANT 32 BITS
; (EXIT INTERFACE:)
DMEXIT  MOVEM.L D2/D3,NSAVED+3*4(SP)
        MOVEM.L (SP)+,D0-D7
        MOVE.L  (SP)+,(SP)
        MOVE.L  (SP)+,(SP)
        RTS

        END
