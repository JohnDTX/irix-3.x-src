        IDENT   DCMP
;
; COPYRIGHT 1981 RICHARD E. JAMES III

; COMPARE, FOR DOUBLE PRECISION.

	GLOBAL	__LFPERR
	EXTERN	__LFPERR
NAN_OPERAND	EQU	$100
ILLEGALOP	EQU 	$120
DENORM		EQU	$163
DIVZERO		EQU	$200
;
;
; DOUBLE PRECISION FLOATING POINT COMPARE.
;
; INPUT:  2 DOUBLE PRECISION NUMBERS ON STACK
; OUTPUT: CONDITION CODES Z/N/V SET AS
;         IF SIGNED INTEGERS WERE JUST COMPARED.
;         CARRY FLAG SET IFF EITHER OPERAND IS A NAN.
;
; RESTRICTIONS: UNORDERED CASES (E.G.,
;       PROJECTIVE INFINITIES AND NANS)
;       PRODUCE RANDOM RESULTS.
;       A NAN, HOWEVER, DOES COMPARE
;       NOT EQUAL TO ANYTHING.
; REGISTER CONVENTIONS
;       D0/D1   FIRST OPERAND
;       D2/D3   SECOND OPERAND
;       D4      TEMP

NSAVED  EQU     5*4
CODES   EQU     34      ;CONDITION ANSWER

        GLOBAL  %D_CMP
%D_CMP
;------
        MOVEM.L D0-D4,-(SP)  ;SAVE
        MOVEM.L NSAVED+4(SP),D0-D3
; (END OF ENTRY INTERFACE)
; REGISTERS NOW CONTAIN:
;       D0      FIRST ARGUMENT, MOST SIGNIFICANT 32 BITS
;       D1      FIRST ARGUMENT, LEAST SIGNIFICANT 32 BITS
;       D2      SECOND ARGUMENT, MOST SIGNIFICANT 32 BITS
;       D3      SECOND ARGUMENT, LEAST SIGNIFICANT 32 BITS
        MOVE.L  D2,D4
        AND.L   D0,D4          ;CMP SIGNS
        BMI.S   NBOTHMI 
        EXG     D0,D2          ;BOTH MINUS
        EXG     D1,D3
NBOTHMI 
	CMP.L   D2,D0          ;MAIN COMPARE
        BNE.S   GOTCMP         ;GOT THE ANSWER
        MOVE.L  D1,D4
        SUB.L   D3,D4          ;COMPARE LOWERS
        BEQ.S   GOTCMP         ;ENTIRELY EQUAL
        ROXR.L  #1,D4
        ANDI.B  #$0A,CCR       ;CLEAR Z, IN CASE DIFFER BY 1 ULP
GOTCMP  
; *** The old way ***   ANDI.B  #$0E,CCR       ;CLEAR CARRY
; *** The old way ***   MOVE    SR,CODES(SP)
;;; Fixed to not use MOVE SR ... which is PRIVELEDGED in 68010
        beq.s   xEQ
xNE     bmi.s   xNEMI
xNEPL   bvs.s   xNEPLVS
xNEPLVC move.w  #$00,CODES(sp)
        bra.s   goon
xNEPLVS move.w  #$02,CODES(sp)
        bra.s   goon
xNEMI   bvs.s   xNEMIVS
xNEMIVC move.w  #$08,CODES(sp)
        bra.s   goon
xNEMIVS move.w  #$0A,CODES(sp)
        bra.s   goon
xEQ     bmi.s   xEQMI
xEQPL   bvs.s   xEQPLVS
xEQPLVC move.w  #$04,CODES(sp)
        bra.s   goon
xEQPLVS move.w  #$06,CODES(sp)
        bra.s   goon
xEQMI   bvs.s   xEQMIVS
xEQMIVC move.w  #$0C,CODES(sp)
        bra.s   goon
xEQMIVS move.w  #$0E,CODES(sp)
goon
;;; End of fix
        LSL.L   #1,D0
        LSL.L   #1,D2
        CMP.L   D2,D0
        BCC.S   CMP4
        EXG     D0,D2          ;FIND LARGER IN MAGNITUDE
	exg	d1,d3
CMP4    
;
;	check for denormalized operands ...
;
	move.l	d2,d4
	rol.l	#8,d4
	tst.b	d4
	bne.s	cklg
;
;	exponent of smaller is zero...
;
	clr.b	d4
	tst.l	d4
	bne.s	dodenorm
	tst.l	d3
	bne.s	dodenorm
;
;	smaller op was true zero ...
;	check larger.
;
	move.l	d0,d4
	rol.l	#8,d4
	tst.b	d4
	bne.s	cklg
;
;	exponent of larger is zero...
;
	clr.b	d4
	tst.l	d4
	bne.s	dodenorm
	tst.l	d1
	beq.s	CMP6
dodenorm
        MOVEM.L D0-D2/A0-A1,-(SP) ;SAVE    
	MOVE.L	#DENORM,-(SP)	; Nan operand
	JSR	__LFPERR
	ADDQ.L	#4,sp
        MOVEM.L (SP)+,D0-D2/A0-A1    ;RESTORE REGISTERS
	bra.s	CMP8
cklg	
	CMPI.L  #$FFE00000,D0
;        BLS.S   CMP6           ;NO NAN
	bcs.s	CMP6		;Neither NAN nor INF
	bne.s	GENNANOP	;NAN
	tst.l	d1		;is lsl 0?
	bne.s	GENNANOP	;INF
CMP6    OR.L    D1,D0
        OR.L    D2,D0
        OR.L    D3,D0
        BNE.S   CMP8           ;NON-ZERO
        MOVE.W  #$04,CODES(SP) ;-0=0
; (EXIT INTERFACE:)
CMP8    MOVEM.L (SP)+,D0-D4
        MOVE.L  (SP)+,12(SP)
        ADDA.W  #10,SP
        RTR
GENNANOP
        MOVE.W  #$01,CODES(SP) ;C,NZ
        MOVEM.L D0-D2/A0-A1,-(SP) ;SAVE    
	move.l	#ILLEGALOP,-(sp)
	jsr	__LFPERR
	addq.l	#4,sp
	MOVE.L	#NAN_OPERAND,-(SP)	; Nan operand
	JSR	__LFPERR
	ADDQ.L	#4,sp
        MOVEM.L (SP)+,D0-D2/A0-A1    ;RESTORE REGISTERS
	BRA.S	CMP8
        END
