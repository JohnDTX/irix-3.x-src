; File: mod.text
; Date: 21-Mar-83
        
        IDENT   MOD
        
; COPYRIGHT 1981, 1982 RICHARD E. JAMES III

; THIS ROUTINE PERFORMS A SINGLE AMOD.
;   DEFINITION:
;       AMOD(TOP,BOT) = RSLT ,  SUCH THAT:
;       N * BOT + RSLT = TOP  FOR SOME INTEGER.
;       ABS(RSLT) < ABS(BOT)
;       RSLT HAS SAME SIGN AS TOP (OR IS ZERO).
;       AMOD(TOP,BOT) = AMOD(TOP,ABS(BOT))
;       AMOD(-TOP,BOT) = -AMOD(TOP,BOT)

; INPUT:
;       SPACE ON STACK FOR RESULT,
;       TWO NUMBERS ON STACK.
; OUTPUT:  Remainder in D0.

; REGISTER CONVENTIONS
;       D0      TOP
;       D1      BOT
;       D3      EXPONENT OF TOP; ITERATION COUNT
;       D4      BOTTOM/FINAL EXPONENT
;       D5      SIGN OF TOP AND RSLT

NSAVED  EQU     4*4

	GLOBAL	__FPERR
	EXTERN	__FPERR

DIVZERO		EQU	$200
NAN_OPERAND	EQU	$100
OVERFLOW	EQU	$300
ILLEGALOP	EQU 	$120
DENORM		EQU	$163
        GLOBAL  %F_MOD
        EXTERN  %F_RCP
%F_MOD
;------
        MOVEM.L D1/D3/D4/D5,-(SP)
        MOVE.L  NSAVED+2*4(SP),D0  ;TOP
        MOVE.L  NSAVED+1*4(SP),D1  ;BOTTOM
; (END OF INTERFACE)
; SAVE SIGN OF TOP; DELETE SIGN OF BOTTOM
        LSL.L   #1,D0
        SCS     D5             ;SAVE SIGN OF TOP
        LSL.L   #1,D1          ;TOSS SIGN
; SPLIT OUT EXPONENTS
        ROL.L   #8,D0
        ROL.L   #8,D1
        CLR.W   D3
        CLR.W   D4
        MOVE.B  D0,D3          ;EXP OF TOP
        MOVE.B  D1,D4          ;EXP OF BOT
; TEST EXPONENTS
        ADDQ.B  #1,D3     
        SUBQ.W  #1,D3          ;TOP
        BLE.S   TOPERR
        MOVE.B  #1,D0          ;HIDDEN BIT (AND CLEAR EXP FIELD)
BACKTOP ADDQ.B  #1,D4     
        SUBQ.W  #1,D4          ;BOT
        BLE.S   BOTERR
        MOVE.B  #1,D1          ;HIDDEN BIT
; POSITION MANTISSAS
BAKBOT  ROR.L   #1+2,D0        ; 001X...                   ;NORM
        ROR.L   #1+1,D1        ; 01X...
; DECIDE HOW MANY ITERATIONS TO DO
        SUB.W   D4,D3
        BLT.S   MOD7           ;TOP IS THE ANSWER
; HERE IS THE GUTS OF MOD:
;   THE ALGORITHM IS A SIMPLE SHIFT AND SUBTRACT LOOP, 
;     BUT IT ADDS WHEN IT OVERSHOOTS.
;   WHY NOT USE THE DIVS/DIVU INSTRUCTIONS?  THE OVERHEAD FOR
;     THAT APPROACH IS SO LARGE THAT IT IS NOT FASTER UNTIL
;     THE NUMBER IF ITERATIONS IS GREATER THAN ABOUT 70!
;     (NORMALLY AMOD WILL BE CALLED WITH ARGUMENT EXPONENTS
;     DIFFERING BY LESS THAN THE MANTISSA SIZE OF 24.)
LSUB    ADD.L   D0,D0          ;LEFT SHIFT TOP
        SUB.L   D1,D0
        DBMI    D3,LSUB
        DBPL    D3,LADD        ;OVERSHOT
        BRA.S   LOOPEND
LADD    ADD.L   D0,D0
        ADD.L   D1,D0          ;COMPENSATE FOR OVERSHOOT
        DBPL    D3,LADD
        DBMI    D3,LSUB
; ADJUST SO THAT SIGN IS POSITIVE
LOOPEND BPL.S   SIGNOK
        ADD.L   D1,D0          ;FIX UP IF WRONG SIGN
; NORMALIZE
SIGNOK  ASL.L   #1,D0
        BMI.S   MNORM
        BEQ.S   MRPK           ;IF ZERO RESULT
MNORML  ASL.L   #1,D0
        DBMI    D4,MNORML      ;LOOP UNTIL NORMALIZED
        DBPL    D4,MNORM       ;DO LAST DEC OF EXPONENT
        BRA.S   MNORM          ;DENORMALIZED
; USE TOP AS ANSWER
MOD7    ADD.W   D3,D4          ;GET BACK EXPONENT OF TOP
        ASL.L   #2,D0
MNORM
; ADJ EXP, ROUND, CHECK EXTREMES, PACK
        JSR     %F_RCP
; REPOSITION AND APPEND SIGN
MRPK    ROR.L   #8,D0
        ROXR.B  #1,D5          ;SIGN->X
        ROXR.L  #1,D0          ;INSERT SIGN
; EXIT INTERFACE
MEXIT   movem.l (sp)+,d1/d3/d4/d5
        move.l  (sp)+,(sp)
        move.l  (sp)+,(sp)
        rts

; EXCEPTION HANDLING

TOPERR  BNE.S   MODNAN         ;MOD(NAN OR INF, ...) = NAN
        ROL.L   #1,D0
        BEQ.S   BACKTOP        ;MOD(0 OR GU, ...): GO DO IT
;
;	top is denormalized...
;
        MOVEM.L D0-D2/A0-A1,-(SP) ;SAVE    
	move.l	#DENORM,-(sp)
	JSR	__FPERR
	addq.l	#4,sp
        MOVEM.L (SP)+,D0-D2/A0-A1    ;RESTORE REGISTERS
	bra.s	BACKTOP

BOTERR  BNE.S   BOTBIG         ;MOD(..., INF/NAN)          ;NORM
NORMB   SUBQ.W  #1,D4                                      ;NORM
        ROL.L   #1,D1                                      ;NORM
        BHI.S   NORMB          ;LOOP UNTIL NORM, NOT 0     ;NORM
        BEQ.S   DODIVZERO      ;bot is zero
        MOVEM.L D0-D2/A0-A1,-(SP) ;SAVE    
	MOVE.L	#DENORM,-(sp)
	JSR	__FPERR
	addq.l	#4,sp
        MOVEM.L (SP)+,D0-D2/A0-A1    ;RESTORE REGISTERS
        ADDQ.W  #1,D4                                      ;NORM
        BRA   	BAKBOT                                     ;NORM
DODIVZERO
;
;	MOD(..., 0) = NAN          ;NORM
;
	MOVE.L	#DIVZERO,d1
	BRA.S	GORAISE

BOTOK
        ADDQ.W  #1,D4                                      ;NORM
        BRA   BAKBOT                                     ;NORM
BOTBIG
        CMP.L   #$000000FF,D1  ; BOT is INF
        BEQ.S   MRPK           ;MOD(TOP, INF) = TOP
;
;	Bot is Nan
;
	MOVE.L	#NAN_OPERAND,d1
	BRA.S	GORAISE
                               
;
; 	top is either INF or Nan.  Decide
;
MODNAN  
	MOVE.L	#NAN_OPERAND,d1
	AND.L	#$ffffff00,d0
	CMP.L	#$000000ff,d0
	bne.s	GORAISE
;	
;	for now, emulate the fpa and raise DIVZERO instead...
;
;	move.l	#ILLEGALOP,d1
	move.l	#OVERFLOW,d1
GORAISE
        MOVEM.L D0-D2/A0-A1,-(SP) ;SAVE    
	move.l	d1,-(sp)
	jsr	__FPERR
	addq.l	#4,sp
        MOVEM.L (SP)+,D0-D2/A0-A1    ;RESTORE REGISTERS
;
;	result is NAN unless DIVZERO or OVERFL
;	
	move.l	#$FF,d0
	cmp.l	#DIVZERO,d1
	bge	MRPK
	move.l	#$7f800001,d0
	bra	MEXIT

;BIGTOP  LSR.L   #1,D0          ;PLUS SIGN
;        CMP.L   #$7F800000,D0
;        BGT.S   MEXIT          ;RETURN LARGER NAN
;                               ;NEITHER WAS NAN, SO MAKE ONE
;
;GENNAN  MOVE.L  #$7F800005,D0
;        BRA.S   MEXIT

;-------------------------------------------------------------
;
;	old code follows:
;
;TOPERR  BNE.S   MODNAN         ;MOD(NAN OR INF, ...) = NAN
;        ROL.L   #1,D0
;        BRA.S   BACKTOP        ;MOD(0 OR GU, ...): GO DO IT
;
;BOTERR  BNE.S   BOTBIG         ;MOD(..., INF/NAN)          ;NORM
;NORMB   SUBQ.W  #1,D4                                      ;NORM
;        ROL.L   #1,D1                                      ;NORM
;        BHI.S   NORMB          ;LOOP UNTIL NORM, NOT 0     ;NORM
;        BEQ.S   GENNAN         ;MOD(..., 0) = NAN          ;NORM
;        ADDQ.W  #1,D4                                      ;NORM
;        BRA.S   BAKBOT                                     ;NORM
;BOTBIG
;        CMP.L   #$000000FF,D1  ;INF
;        BEQ.S   MRPK           ;MOD(TOP, INF) = TOP
;                               ;MOD(..., NAN) = NAN
;
;; IF EITHER IS NAN, RETURN LARGER; ELSE CREATE NAN (INVALID OPERATION)
;MODNAN  
;	ROR.L   #8,D0
;        ROR.L   #8,D1
;        CMP.L   D0,D1
;        BCS.S   BIGTOP         ;BLO
;        EXG     D0,D1          ;GET LARGER
;BIGTOP  LSR.L   #1,D0          ;PLUS SIGN
;        CMP.L   #$7F800000,D0
;        BGT.S   MEXIT          ;RETURN LARGER NAN
;                               ;NEITHER WAS NAN, SO MAKE ONE
;
;GENNAN  MOVE.L  #$7F800005,D0
;        BRA.S   MEXIT
;
        END
