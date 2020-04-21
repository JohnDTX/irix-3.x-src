; File: cmp.text
; Date: 01-Nov-83

        IDENT   CMP_

;
; COPYRIGHT 1981 RICHARD E. JAMES III
;
; COMPARE FOR SINGLE PRECISION ON MC68000.

        GLOBAL  %F_CMP
	GLOBAL	__FPERR
	EXTERN	__FPERR
NAN_OPERAND	EQU	$100
ILLEGALOP	EQU 	$120
DENORM		EQU	$163
DIVZERO		EQU	$200

; SINGLE PRECISION FLOATING POINT
;   COMPARE.
;
; INPUT:  2 FLOATING POINT NUMBERS
;         ON STACK
; OUTPUT: CONDITION CODES Z/N/V SET AS IF SIGNED INTEGERS WERE
;           JUST COMPARED.
;         CARRY FLAG SET IFF EITHER OPERAND IS A NAN.
;
; RESTRICTIONS: UNORDERED CASES (E.G., PROJECTIVE INFINITIES AND NANS)
;         PRODUCE RANDOM RESULTS.
;       A NAN, HOWEVER, DOES COMPARE NOT EQUAL TO ANYTHING.
;
; REGISTER CONVENTIONS
;       D0      FIRST OPERAND
;       D1      SECOND OPERAND

NSAVED  EQU     3*4            ;NUMBER OF BYTES FOR SAVED REGS
CODES   EQU     18             ;WHERE TO PUT CONDITION CODE ON STACK

%F_CMP
;------
        MOVEM.L D0-D2,-(SP)    ;SAVE REGISTERS
        MOVEM.L NSAVED+4(SP),D0/D1
; (END OF INTERFACE)

        MOVE.L  D1,D2
        AND.L   D0,D2          ;CMP SIGNS
        BMI.S   NBOTHMI 
        EXG     D0,D1          ;BOTH MINUS
NBOTHMI CMP.L   D1,D0          ;MAIN COMPARE
; *** The old way ***   ANDI.B  #$0E,CCR       ;CLEAR C
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
        LSL.L   #1,D1
        CMP.L   D1,D0
        BCC.S   CMP4
        EXG     D0,D1          ;FIND LARGER
CMP4    
;
;	check the smaller for a denormalized operand...
;
	move.l	d1,d2
	rol.l	#8,d2
	tst.b	d2
	bne.s	cklg
;
;	exponent of smaller is zero ...
;
	clr.b	d2
	tst.l	d2
	bne.s	gendenorm
;
;	the smaller is true zero.  Check the larger for denorm.
;
	move.l	d0,d2
	rol.l	#8,d2
	tst.b	d2
	bne.s	cklg
;
;	exponent of both are zero ...
;
	clr.b	d2
	tst.l	d2
	beq.s	cklg
gendenorm
        MOVEM.L D0-D2/A0-A1,-(SP) ;SAVE    
	move.l	#DENORM,-(sp)
	jsr	__FPERR
	addq.l	#4,sp
        MOVEM.L (SP)+,D0-D2/A0-A1    ;RESTORE REGISTERS
	bra.s	CMP8
cklg
	CMPI.L  #$FF000000,D0
        BLS.S   CMP6 
;
; NAN -- first raise ILLEGALOP exception, then NANOP exception.
;
        MOVE.W  #$01,CODES(SP) ;C,NZ
        MOVEM.L D0-D2/A0-A1,-(SP) ;SAVE    
	move.l	#ILLEGALOP,-(sp)
	jsr	__FPERR
	addq.l	#4,sp
	move.l	#NAN_OPERAND,-(sp)
	jsr	__FPERR
	addq.l	#4,sp
        MOVEM.L (SP)+,D0-D2/A0-A1    ;RESTORE REGISTERS
CMP6    TST.L   D0
        BNE.S   CMP8
        MOVE.W  #$04,CODES(SP) ;-0=0

;   RESULT IS THE CONDITION CODE IN CODES(SP)
; (EXIT INTERFACE:)
CMP8    MOVEM.L (SP)+,D0-D2
        MOVE.L  (SP)+,4(SP)
        ADDQ.L  #2,SP
        RTR

        END
