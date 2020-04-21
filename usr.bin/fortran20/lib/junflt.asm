; File: junflt.text - A combination of CMP, FPK, IPK and MOD
; Date: 27-Jun-85

        IDENT   JUN_FLT

        GLOBAL  %F_MAX,%F_MIN,%F_DIM
;	GLOBAL	__FPERR
;	EXTERN	__FPERR

NAN_OPERAND	EQU	$100
OVERFLOW	EQU	$300
ILLEGALOP	EQU 	$120

;
; Parameters: ST.S - First arg
;             ST.S - Second arg
;
; Returns: F1.S - The result
;
; Scratches: Only F1.
;

; DIM(X,Y) == MAX(X-Y,0)

%F_DIM
        move.l  8(sp),$8110.w   ; J_MOVES x,F1
        move.l  4(sp),$8F10.w   ; J_FSUB  y,F1
        move.l  $8110.w,8(sp)   ; J_MOVES F1,x
        clr.l   4(sp)           ; y := 0
;      (FALL INTO MAX)

%F_MAX
        move.l  8(sp),$8110.w   ; J_MOVES x,F1
        move.l  4(sp),$B210.w   ; J_FCMP  y,F1
        tst.b   $8700.w         ; J_CR
        bgt.s   maxrts
        move.l  4(sp),$8110.w   ; J_MOVES y,F1
maxrts  rtd     #8

%F_MIN
        move.l  4(sp),$8110.w   ; J_MOVES y,F1
        move.l  8(sp),$B210.w   ; J_FCMP  x,F1
        tst.b   $8700.w         ; J_CR
        blt.s   minrts
        move.l  8(sp),$8110.w   ; J_MOVES x,F1
minrts  rtd     #8
        
;
; %F_MOD(x,y) = x - int(x/y)*y
;

;%F_MOD
;
;	gb (sgi) 10/22/85 - have to check for mod(INF,y)
;
;	move.l	8(sp),d0
;	bclr	#31,d0
;	cmp.l	#$7f800000,d0
;	bne.s	gomod
;
;	illegal operation - mod(INF,x)
;
;	move.l	#ILLEGALOP,-(sp)
;	jsr	__FPERR
;	addq.l	#4,sp
;	move.l	d0,$8110.w	; move result to F1
;	rtd 	#8
;gomod
;        move.l  8(sp),$8110.w   ; J_MOVES x,F1
;        move.l  4(sp),$9B10.w   ; J_FDIV  y,F1
;        move.b  d0,$A911.w      ; J_FtoI  F1
;        move.b  d0,$A711.w      ; J_ItoF  F1
;        move.l  4(sp),$9710.w   ; J_FMUL  y,F1
;        move.l  8(sp),$9310.w   ; J_FRSUB x,F1
;        rtd     #8
        end
; File: fpk.text
; Date: 21-Mar-83
        
        IDENT   FPK
        
; COPYRIGHT 1981, 1982 RICHARD E. JAMES III

; THIS MODULE CONTAINS SEVERAL SINGLE
; PRECISION UTILITIES FOR FORTRAN INTRINSICS.

; FORMAT OF UNPACKED RECORD:
;     RECORD
;       D0/D1   +0/+4   MANTISSA: ARRAY[2]OF INT; (UPPER/LOWER)
;       D2.W    +8      EXPONENT: -32768..32767;
;       D3.W    +10     SIGN:     (IN SIGN BIT);
;       D3.B    +11     TYPE:     1..5               
;     END;

        PAGE    ;----------------
        GLOBAL  %PF_UNPK,%F_UNPK
        GLOBAL  %U_STOR
	GLOBAL	__UPERR
	EXTERN	__UPERR

NAN_OPERAND	EQU	$100
OVERFLOW	EQU	$300
ILLEGALOP	EQU 	$120

; SPLIT A SINGLE PREC. NUMBER INTO ITS SIGN, EXPONENT, AND MANTISSA.
 
; INPUT:
;       FLOATING VALUE
;       ADDR OF RECORD
; REGISTERS: A0,D0,D1 DESTROYED.

%PF_UNPK
;------
;;;;;;; Begin RSG changes to preserve D3-D7/A2-A3 09-Feb-83 ;;;;;;;
        MOVE.L  8(SP),D1       ;NUMBER
        move.l  (sp)+,a0       ; Pop return address         ;;RSG;;
        move.l  (sp)+,d0       ; Pop result address         ;;RSG;;
        move.l  a0,(sp)        ; Set up final return        ;;RSG;;
        movem.l d3-d7/a2-a3,-(sp) ;                         ;;RSG;;
        subq.w  #4,sp          ; Dummy param for %U_STOR    ;;RSG;;
        move.l  d0,-(sp)       ; Param for %U_STOR          ;;RSG;;
        BSR.S   %F_UNPK
        bsr.s   %U_STOR        ; BSR instead of fall through;;RSG;;
        movem.l (sp)+,d3-d7/a2-a3 ;                         ;;RSG;;
        rts                                                 ;;RSG;;
;;;;;;; End** RSG changes to preserve D3-D7/A2-A3 09-Feb-83 ;;;;;;;

%U_STOR                        ;JUMPED TO
;------
        MOVE.L  4(SP),A0       ;RECORD ADDR
        SWAP    D2
        MOVE.W  D3,D2
        MOVEM.L D0-D2,(A0)     ;STORE RECORD
        MOVE.L  (SP)+,A0       ;RETURN ADDR
        ADDQ.L  #8,SP
        JMP     (A0)           ;RETURN

%F_UNPK
;------
        MOVE.L  D1,D3
        SWAP    D3             ;SIGN IN BIT 15
        LSL.L   #1,D1          ;TOSS SIGN
        ROL.L   #8,D1
        CLR.W   D2
        MOVE.B  D1,D2          ;EXPONENT
        BNE.S   UNP2           ;NOT 0/GU        
        MOVE.B  #1,D3          ;--ZERO   
        TST.L   D1                     
        BEQ.S   UNP8                   
        MOVE.B  #2,D3          ;--DENORMALIZED     
        BRA.S   UNP8
UNP2    CMP.B   #255,D2        ;INF/NAN       
        BNE.S   UNP3           ;PLAIN
        MOVE.W  #$6000,D2       
        CLR.B   D1             ;ERASE EXP
        MOVE.B  #4,D3          ;--INF
        TST.L   D1                     
        BEQ.S   UNP9                   
        MOVE.B  #5,D3          ;--NAN    
        BRA.S   UNP9                   
UNP3    MOVE.B  #1,D1          ;HIDDEN BIT
        SUBQ.W  #1,D2                  
        MOVE.B  #3,D3          ;--NUM    
UNP8    SUBI.W  #126+23,D2
UNP9    ROR.L   #1,D1
        LSR.L   #8,D1
        CLR.L   D0             ;UPPER
        RTS
        PAGE    ;---------------
        GLOBAL  %PF_PACK,%F_PACK
        GLOBAL  %U_LOAD

; RECONSTRUCT A SINGLE PRECISION NUMBER FROM A RECORD CONTAINING
; ITS PIECES.

; INPUT  (%PF_... ENTRIES)
;       ADDRESS OF WHERE TO PUT ANSWER
;       ADDRESS OF RECORD
; REGISTERS: A0, D0-D3 DESTROYED
; USAGE:
;       D0      UPPER/ANSWER           
;       D1      LOWER                  
;       D2      RESERVED *S
;       D3      SIGN AND TYPE
;       D4      EXPONENT               

; ERROR CONDITIONS
;   IF THE NUMBER IS TOO BIG FOR 
;   SINGLE PRECISION, INFINTY IS GENERATED.

%U_LOAD
;------
        MOVE.L  2*4(SP),A0
        MOVEM.L (A0),D0-D2
        MOVE.W  D2,D3          ;SIGN, TYPE
        SWAP    D2             ;EXPONENT
        MOVE.L  3*4(SP),A0     ;ADDR FOR RESULT
        RTS

%PF_PACK
;------
;;;;;;; Begin RSG changes to preserve D3-D7/A2-A3 09-Feb-83 ;;;;;;;
        movem.l (sp)+,d0/d1/d2 ; Pop params                 ;;RSG;;
        movem.l d3-d7/a2-a3,-(sp) ;                         ;;RSG;;
        movem.l d0/d1/d2,-(sp) ; Push params                ;;RSG;;
        BSR.S   %U_LOAD
        BSR.S   %F_PACK
        MOVE.L  D1,(A0)        ;STORE RESULT
        MOVE.L  (SP)+,A0
        ADDQ.L  #8,SP
        movem.l (sp)+,d3-d7/a2-a3 ;                         ;;RSG;;
        JMP     (A0)           ;RETURN
;;;;;;; End** RSG changes to preserve D3-D7/A2-A3 09-Feb-83 ;;;;;;;

%F_PACK
;------
        MOVE.W  D2,D4          ;EXP
        CMPI.B  #4,D3          ;TYPE : INF/NAN
        BLT.S   SPK0                   
        OR.L    D0,D1          ;NAN CODE, OR 0 FOR INF     ;PKNAN
        ORI.L   #$7F800000,D1  ;EXP FOR NAN/INF            ;PKNAN
        LSL.L   #1,D1                                      ;PKNAN
        BRA.S   SPKSGN         ;NAN OR INF    
SPK0
;       CLR.B   D2             ;FOR STICKY
        TST.L   D0
        BEQ.S   SPK2
; SHIFT FROM UPPER INTO LOWER
SPK1    
;       OR.B    D1,D2          ;STICKY
        MOVE.B  D0,D1
        ADDQ.W  #8,D4          ;ADJ EXP
        ROR.L   #8,D1
        LSR.L   #8,D0
        BNE.S   SPK1           ;LOOP TIL TOP=0
SPK2    MOVE.L  D1,D0
        BEQ.S   SPKSGN 
; FIND TOP BIT
        BMI.S   SPK5           ;IF ALREADY NRM
SPK4    SUBQ.W  #1,D4          ;ADJ EXP       
        LSL.L   #1,D0          ;NORMALIZE     
        BPL.S   SPK4
SPK5    ADDI.W  #126+23+9,D4        
        BSR.S   %F_RCP                 
;
;	check the result of F_RCP for overflow
;
	cmp.b	#$FF,d0
	bne.s 	RESOK
	move.l	#OVERFLOW,-(sp)
	jsr	__UPERR
	addq.l	#4,sp
	move.l	#$FF,d0
RESOK
        ROR.L   #8,D0
        MOVE.L  D0,D1
SPKSGN  LSL.W   #1,D3
        ROXR.L  #1,D1          ;APPEND SIGN   
        RTS


        GLOBAL  %F_SIGN

; INPUT:
;       SPACE FOR RETURN VALUE
;       ARG TO HAVE SIGN APPLIED TO 
;       ARG WITH SIGN
;       RETURN ADDR

%F_SIGN
;------
        ROXL    8(SP)          ;KILL SIGN IN ARG1
        ROXL    4(SP)          ;GET SIGN OF SECOND
        ROXR    8(SP)          ;PUT SIGN ON
        move.l  8(sp),$8110.w  ;RETURN VALUE if Juniper F1
        MOVE.L  (SP)+,(SP)     ;RETURN ADDR
        MOVE.L  (SP)+,(SP)     ;RETURN ADDR
        RTS
        PAGE    ;-----------
        GLOBAL  %F_RCP                 
; ROUND, CHECK FOR OVER/UNDERFLOW, AND PACK IN THE EXPONENT.

; INPUT:
;       D0      MANTISSA (- IF NORM)  *S
;       D4      BIASED EXPONENT        
; OUTPUT:
;       D0      MOST OF NUMBER, NO SIGN OR HIDDEN BIT,
;               EXP IS IN BOTTOM.      

%F_RCP
;------
        TST.L   D0
        BMI.S   RCP1
; DO EXTRA NORMALIZE (FOR MUL/DIV)
        SUBQ.W  #1,D4
        LSL.L   #1,D0          ;DO ONE NORMALIZE
RCP1
        TST.W   D4
        BGT.S   RCP2
; UNDERFLOW
        CMPI.W  #-24,D4                
        BLT.S   SIGNED0                
        NEG.B   D4             
        ADDQ.B  #1,D4                  
        LSR.L   D4,D0          ;DENORMALIZE   
        CLR.W   D4             ;EXP=0
RCP2
        ADDI.L  #$80,D0        ;CRUDE ROUND   
        BCC.S   RCP4                   
; ROUND OVERFLOWED
        ROXR.L  #1,D0
        ADDQ.W  #1,D4          ;ADJ EXP       
RCP4
        CMPI.W  #$FF,D4                
        BGE.S   RCPBIG                 
        LSL.L   #1,D0          ;TOSS HIDDEN   
        SCS     D0             ;NO HIDDEN IMPLIES 0/DENORM ;NORM
        AND.B   D4,D0          ;                           ;NORM
        RTS                            
                                                           ;NORM
SIGNED0 CLR.L   D0                     
        RTS                            

RCPBIG  MOVE.L  #$FF,D0        ;INFINITY      
        RTS                            
 
        END
; File: ipk.text
; Date: 23-Aug-83

        ident   ipk
; COPYRIGHT 1981, RICHARD E. JAMES III

; THIS MODULE PERFORMS VARIOUS TYPE COERCIONS FOR FORTRAN.
; ALSO INCLUDED ARE THE ROUNDING AND TRUNCATING OPERATIONS.

; THE PATTERN OF THE ENTRY POINTS FOR TYPE COERCION IS
;       %X_2_Y
; WHERE
;       X IN [I,F] IS THE SOURCE TYPE
;       Y IN [I,F] IS THE DESTINATION TYPE
;       I AND F FOR INTEGER AND SINGLE, RESPECTIVELY.

; THE CALLING SEQUENCE IS
;       ENTER WITH STACK CONTAINING
;               ARGUMENT (4 BYTES)
;               RETURN ADDRESS
;       EXIT WITH STACK CONTAINING
;               RESULT (4 BYTES)
;       ALL REGISTERS ARE PRESERVED.

; THE LOGIC FOR EACH IS
;       SAVE REGISTERS.
;       LOAD ARGUMENT.
;       UNPACK INTO INTERNAL FORM.
;       REPACK INTO DESTINATION FORM.
;       STORE RESULT ON STACK.
;       RESTORE REGISTERS AND RETURN.

NSAVED  EQU     5*4            ;SAVE D0-D4

        GLOBAL  %I_2_F,%F_2_I

        EXTERN  %F_PACK,%F_UNPK
        EXTERN  %U_LOAD,%U_STOR
	GLOBAL	__UPERR
	EXTERN	__UPERR

NAN_OPERAND	EQU	$100
OVERFLOW	EQU	$300
ILLEGALOP	EQU 	$120

%I_2_F                         ; FLOAT, REAL
;------
        MOVEM.L D0-D4,-(SP)
        MOVE.L  NSAVED+4(SP),D1
        BSR.S   %I_UNPK
        JSR     %F_PACK
        MOVE.L  D1,NSAVED+4(SP)
        MOVEM.L (SP)+,D0-D4
        RTS
%F_2_I                         ; IFIX, INT
;------
        MOVEM.L D0-D4,-(SP)
        MOVE.L  NSAVED+4(SP),D1
        JSR     %F_UNPK
        BSR.S   %I_PACK
        MOVE.L  D1,NSAVED+4(SP)
        MOVEM.L (SP)+,D0-D4
        RTS
        PAGE    ;--------------
        GLOBAL  %I_UNPK,%I_PACK
;
; UTILITIES FOR CONVERTING BETWEEN INTERNAL UNPACKED FORM AND INTEGER.

; REGISTERS
;       D1      INTEGER
;          VS.
;       D0/D1   NUMBER (64-BIT INTEGER)
;       D2.W    EXPONENT
;       D3.W    SIGN IN SIGN BIT
;       D3.B    KIND (3)

%I_UNPK
;------
        CLR.W   D2             ;EXPONENT
        MOVEQ   #3,D3          ;SIGN=+, KIND=NUMBER
        TST.L   D1             ;LOWER
        BPL.S   IUPOS
        NEG.L   D1             ;(LARGEST NEG NUM WILL BE OK!)
        BSET    #15,D3         ;NEGATIVE
IUPOS   CLR.L   D0             ;UPPER
        RTS

%I_PACK
;------
        CMPI.B  #4,D3
        BGE.S   BIGI           ;INF OR NAN
        BSR.S   GNSH           ;SHIFT BY THE EXPONENT
        OR.W    D2,D0
        TST.L   D0
        BNE.S   BIGI           ;>= 2**32 OR GNSH COUNDN'T DO IT ALL
        TST.L   D1             ;LOWER
        BMI.S   BIGI           ;>= 2**31
        TST.W   D3             ;SIGN
        BPL.S   IPZ
        NEG.L   D1             ;CHANGE SIGN (2'S COMPL)
IPZ     RTS

BIGI    
;
;	generate an exception for the overflow
;
	move.l	#OVERFLOW,-(sp)
	jsr	__UPERR
	addq.l	#4,sp
	move.l	#$80000000,d1
        RTS
        PAGE    ;--------------
        GLOBAL  %PG_NSH
; NOMINALLY SHIFT THE MANTISSA BY THE AMOUNT IN THE EXPONENT.
; IF RESULT WILL NOT FIT IN 64 BITS, THE EXPONENT IS LEFT NON-ZERO,
;     GARBAGE IN UPPER/LOWER.

; INPUT:
;       ADDR OF RECORD
; REGISTERS: A0, D0-D4 DESTROYED

%PG_NSH
;------
        JSR     %U_LOAD
        BSR.S   GNSH
        JMP     %U_STOR

; INPUT AND OUTPUT: D0-D3  UNPACKED RECORD.
; OUTPUT: D2.L=0  IF SHIFT COULD BE DONE.

GNSH
;------
        MOVEQ   #32,D4
        TST.W   D2             ;EXPONENT
        BMI.S   SHR            ;RIGHT SHIFT
; LEFT SHIFT
LL      CMP.W   D4,D2
        BLT.S   SIMPL          ;SIMPLE LEFT
        TST.L   D0
        BNE.S   NSHOFL         ;TOO BIG
        SUB.W   D4,D2
        EXG     D1,D0          ;SHIFT 32
        BRA.S   LL     
SIMPL   ROL.L   D2,D0
        ROL.L   D2,D1
        MOVEQ   #1,D4
        ASL.L   D2,D4
        SUBQ.L  #1,D4          ;FORM MASK
        MOVE.L  D0,D2
        AND.L   D4,D2          ;OFL PART
        BNE.S   NSHOFL
        AND.L   D1,D4          ;BETWEEN PARTS
        EOR.L   D4,D1          ;CLEAR BOTTOM
        OR.L    D4,D0          ;INTO TOP
        BRA.S   NSHXIT         ;EXIT

; SHIFT RIGHT
SHR     NEG.W   D2             ;-EXP
LR      CMP.W   D4,D2
        BLT.S   SIMPR
        MOVE.L  D0,D1          ;SHIFT 32
        CLR.L   D0
        SUB.W   D4,D2
        BRA.S   LR
SIMPR   MOVEQ   #1,D4
        ASL.L   D2,D4
        SUBQ.L  #1,D4          ;FORM MASK
        NOT.L   D4
        AND.L   D4,D1          ;LOWER, 0
        AND.L   D0,D4          ;UPPER, 0
        EOR.L   D4,D0          ;0, MID
        OR.L    D0,D1          ;LOWER, MID
        ROR.L   D2,D4          ;0, UPPER
        ROR.L   D2,D1          ;MID, LOWER
        MOVE.L  D4,D0          ;UPPER
NSHXIT  CLR.L   D2
NSHOFL
        RTS                    ;RETURN FROM %G_NSH
        PAGE    ;--------------
        GLOBAL  %I_NINT,%F_ANINT
        GLOBAL  %G_NINT

; ROUTINES TO FIND THE NEAREST INTEGER.
; GENERAL DESIGN:
;       LOAD ARGUMENT
;       UNPACK INTO INTERNAL FORM
;       IF EXPONENT IS POSITIVE THEN
;               SHIFT TO FIND BIT AFTER DECIMAL POINT
;               ROUND UP IF 1
;       REPACK
;       STORE RESULT ON STACK AND RETURN

; THE CALLING SEQUENCE IS
;       ENTER WITH STACK CONTAINING
;               ROOM FOR ANSWER (4 BYTES)
;               ARGUMENT (4 BYTES)
;               RETURN ADDRESS
;       EXIT WITH STACK CONTAINING
;               RESULT (4 BYTES)
;       REGISTERS D0-D4 ARE DESTROYED.

%G_NINT                        ; (INTERNAL ROUTINE)
;------
        TST.W   D2             ;EXPONENT
        BPL.S   NORND          ;DO NOT NEED TO ADD .5
        ADDQ.W  #1,D2
        BSR.S   GNSH           ;SHIFT TO GET XXXXX.X
        ASR.L   #1,D0          ;DIV 2
        ROXR.L  #1,D1
        ADDX.L  D2,D1          ;ADDS ONE IF BIT SHOVED OFF
        ADDX.L  D2,D0          ;NOTE: GNSH LEFT D2.L=0
NORND   RTS

%I_NINT                        ; NINT ( REAL ) -> INTEGER
;------
        movem.l d1-d4,-(sp)
        move.l  20(sp),d1
        jsr     %F_UNPK
        bsr.s   %G_NINT
        bsr     %I_PACK
        move.l  d1,d0
        movem.l (sp)+,d1-d4
        move.l  (sp)+,(sp)
        rts
%F_ANINT                       ; ANINT ( REAL ) -> REAL
;------
        move.l  4(sp),d1
        movem.l d3-d4,-(sp)
        jsr     %F_UNPK
        bsr.s   %G_NINT
        bra.s   AIXIT
        page    ;--------------
        GLOBAL  %F_AINT
        GLOBAL  %G_INT         ;FOR INTERNAL USE ONLY
; TRUNCATION IS LIKE ROUNDING WITHOUT THE ROUND.

%G_INT
;------
        TST.W   D2             ;EXPONENT
        BMI     GNSH 
        RTS

%F_AINT                        ; AINT ( REAL ) -> REAL
;------
        move.l  4(sp),d1
        movem.l d3-d4,-(sp)
        jsr     %F_UNPK
        bsr.s   %G_INT

AIXIT   jsr     %F_PACK
        movem.l (sp)+,d3-d4
        move.l  d1,$8110.w
        move.l  (sp)+,(sp)
        rts

        end

