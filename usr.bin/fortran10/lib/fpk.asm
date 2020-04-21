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
        move.l  8(sp),d0       ;RETURN VALUE
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
