; File: div.text
; Date: 08-Oct-82
        
        IDENT   DIV
        
; COPYRIGHT 1981, 1982 RICHARD E. JAMES III

; THIS ROUTINE PERFORMS A SINGLE PRECISION DIVIDE.

        GLOBAL  %F_DIV
	GLOBAL	__FPERR
	EXTERN	__FPERR

NAN_OPERAND	EQU	$100
OVERFLOW	EQU	$300
ILLEGALOP	EQU 	$120
DIVZERO		EQU	$200
DENORM		EQU	$163

; INPUT:  TWO NUMBERS ON STACK.
; OUTPUT:  QUOTIENT ON STACK.

; INTERNAL REGISTER CONVENTIONS
;       D0      TOP; AB; RQ
;       D1      BOT; C
;       D2           Q 
;       D3      BOTTOM EXP; D
;       D4      TOP/FINAL EXP
;       D5      SIGN

NSAVED  EQU     6*4

        EXTERN  %F_RCP
%F_DIV
;------
        MOVEM.L D0-D5,-(SP)    ;SAVE REGISTERS 
        MOVE.L  NSAVED+8(SP),D0
        MOVE.L  NSAVED+4(SP),D1  ;BOTTOM
; (END OF INTERFACE)
;       D0 = DIVIDEND (TOP)
;       D1 = DIVISOR (BOTTOM)

; DETERMINE SIGN
        ROL.L   #1,D0
        ROL.L   #1,D1
        MOVE.B  D0,D5
        EOR.B   D1,D5          ;SIGN IN BIT 0
; SPLIT OUT EXPONENT
        ROL.L   #8,D0
        ROL.L   #8,D1
        CLR.W   D3
        CLR.W   D4
        MOVE.B  D0,D4          ;EXP OF TOP
        MOVE.B  D1,D3          ;EXP OF BOT
        ANDI.W  #$FE00,D0      ;CLR S,EXP
        ANDI.W  #$FE00,D1      ;CLR S,EXP
; TEST EXPONENTS
        ADDQ.B  #1,D4     
        SUBQ.W  #1,D4          ;TOP
        BLE.S   TOPERR
        ADDQ.B  #1,D0          ;HIDDEN BIT
BACKTOP ADDQ.B  #1,D3     
        SUBQ.W  #1,D3          ;BOT
        BLE     BOTERR
        ADDQ.B  #1,D1          ;HIDDEN BIT
; POSITION MANTISSAS
BACKBOT ROR.L   #2,D1          ;01X...                     ;NORM
        ROR.L   #4,D0          ;0001X...
; COMPUTE TENTATIVE EXPONENT
        SUB.W   D3,D4
; TO COMPUTE AB/CD: 
;   FIRST DO AB/C -> Q, REMAINDER=R
        MOVE.W  D1,D3          ;SAVE D
        SWAP    D1             ;GET C
        DIVU    D1,D0          ;AB/C 29/15->15
        MOVE.W  D0,D2          ;SAVE Q
        MULU    D2,D3          ;Q*D
        CLR.W   D0             ;R IN TOP
        SUB.L   D3,D0          ;R-Q*D = +-31
        ASR.L   #2,D0          ;AVOID OFL
        DIVS    D1,D0          ;MORE QUOTIENT
        EXT.L   D2             ;Q
        EXT.L   D0             ;SECOND QUOT
        SWAP    D2
        ASL.L   #2,D0
        ADD.L   D2,D0          ;30-31 BITS
        ASL.L   #1,D0
; ADJ EXP, ROUND, CHECK EXTREMES, PACK
        ADDI.W  #127,D4
        JSR     %F_RCP
; REPOSITION AND APPEND SIGN
DREPK   ROR.L   #8,D0
        ROXR.B  #1,D5          ;SIGN->X
        ROXR.L  #1,D0          ;INSERT SIGN

;       D0 = QUOTIENT
; EXIT INTERFACE
DEXIT   MOVE.L  D0,NSAVED+2*4(SP)
        MOVEM.L (SP)+,D0-D5
        MOVE.L  (SP)+,(SP)
        RTS

GORAISE
        MOVEM.L D0-D2/A0-A1,-(SP) ;SAVE    
	MOVE.L	d1,-(SP)
	JSR	__FPERR
	ADDQ.L	#4,sp
        MOVEM.L (SP)+,D0-D2/A0-A1    ;RESTORE REGISTERS
;
;	result should be NAN unless DIVZERO (INF)
;
	cmp.l	#DIVZERO,d1
	bge	GENINF
	move.l	#$7f800001,d0
	bra.s	DEXIT

; EXCEPTION HANDLING

TOPERR  BNE.S   TNGU
; TOP IS 0 OR GU, NORMALIZE AND RETURN                     ;NORM
;
;	if d0 is 0, top was zero, just return.
;
	tst.l	d0
	beq	BACKTOP
;
;	raise 'denormalized operand' error
;
        MOVEM.L D0-D2/A0-A1,-(SP) ;SAVE    
	move.l	#DENORM,-(sp)
	jsr	__FPERR
	addq.l	#4,sp
        MOVEM.L (SP)+,D0-D2/A0-A1    ;RESTORE REGISTERS
NORMT   
	SUBQ.W  #1,D4                                      ;NORM
        ROL.L   #1,D0                                      ;NORM
        BHI.S   NORMT          ;LOOP TIL NORM, FALL IF 0   ;NORM
        ADDQ.W  #1,D4                                      ;NORM
        BRA   	BACKTOP        ;0 OR GU

; TOP IS INF OR NAN
TNGU    
;
;	top is infinity or Nan
;
;	registers: d0 - top mant
;		   d1 - bot mant
;		   d2 - ?
;		   d3 - bot exp
;		   d4 - top/final exp
;	
        CMP.B    d3,d4
        BNE.S    TNGU1
;
;	both same and inf or Nan.... find out if either is Nan
;
	TST.L	d0			; is top Nan?
	BNE.S	NAN_OP			; yes
	TST.L	d1			; is bot Nan?
	BNE.S	NAN_OP			; yes
	MOVE.L	#ILLEGALOP,d1		; inf/inf
	BRA	GORAISE
TNGU1
;
;	for compatibility, we have to check that the bottom
;	is not zero here.
;
	tst.w	d3
	beq.s	dvzero
        TST.L    d0
        BNE.S    NAN_OP			; top is nan.
;
;	top is INF.  Check bottom for zero.
;
	tst.b	d3
	bne.s	GENINF			; bot nonzero
	tst.l	d1	
	bne.s	GENINF			; bot nonzero
;
;	INF/0
;
dvzero
	move.l	#DIVZERO,d1
	bra.s	GORAISE
NAN_OP
        MOVE.L	#NAN_OPERAND,d1
	BRA	GORAISE			; top is Nan
BOTERR
       	BEQ.S   BOTLOW			; bot is zero
;
;	the bot is either inf or nan
;
        TST.L    d1
        BNE.S    NAN_OP			; bot is Nan
;
;	bot is inf. top is not.  result is zero.
;
        CLR.L    d0
        BRA    	DREPK
BOTLOW
        TST.L    d1
	BEQ.S	BOTZERO
;
;	raise 'denormalized operand' error
;
        MOVEM.L D0-D2/A0-A1,-(SP) ;SAVE    
	move.l	#DENORM,-(sp)
	jsr	__FPERR
	addq.l	#4,sp
        MOVEM.L (SP)+,D0-D2/A0-A1    ;RESTORE REGISTERS
;
;	normalize the bottom
;
NORMB   SUBQ.W  #1,D3                                      ;NORM
        ROL.L   #1,D1                                      ;NORM
        BCC.S   NORMB          ;LOOP TIL NORMALIZED        ;NORM
        ADDQ.W  #1,D3                                      ;NORM
        BRA   BACKBOT                                    ;NORM
BOTZERO
;
;	bot and possibly top are zero or denormalized.
;
	MOVE.L	#DIVZERO,d1
	BRA	GORAISE

; GENERATE INFINITY FOR ANSWER
GENINF  MOVE.L  #$FF,D0
        BRA   	DREPK

; INVALID OPERAND/OPERATION
;DINVOP  CMP.L   D0,D1
;        BCS.S   USENAN         ;USE LARGER NAN
;        TST.L   D1
;        BEQ.S   GENNAN         ;BOTH ARE INF, GEN A NAN
;        EXG     D1,D0          ;LARGER NAN
;USENAN  LSR.L   #8,D0
;        LSR.L   #1,D0
;        BRA.S   BLDNAN         ;RETURN NAN

;GENNAN  MOVEQ   #4,D0          ;NAN 4
;BLDNAN  ORI.L   #$7F800000,D0
;        BRA.S   DEXIT

        END
