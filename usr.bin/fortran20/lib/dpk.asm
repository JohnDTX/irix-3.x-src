; File: dpk.text
; Date: 09-Jan-85

; New %F_2_D at the end on 09-Jan-85

        IDENT   DPK
        
; COPYRIGHT 1981, 1982 RICHARD E. JAMES III

; THIS MODULE CONTAINS DOUBLE PRECISION ROUNDING, CONVERSION, AND
;   TRUNCATING OPERATIONS, PLUS OTHER UTILITIES FOR FORTRAN INTRINSICS.

; TYPE COERCION
;   THE PATTERN OF THE ENTRY POINTS IS
;       %X_2_Y
;   WHERE
;       X IN [I,F,D] IS THE SOURCE TYPE
;       Y IN [I,F,D] IS THE DESTINATION TYPE
;       I, F, D FOR INTEGER, SINGLE, DOUBLE RESPECTIVELY.

; THE CALLING SEQUENCE IS
;       ENTER WITH STACK CONTAINING
;               ARGUMENT (4 OR 8 BYTES)
;               RETURN ADDRESS
;       EXIT WITH STACK CONTAINING
;               RESULT
;       ALL REGISTERS ARE PRESERVED.

; THE LOGIC FOR EACH IS
;       SAVE REGISTERS.
;       LOAD ARGUMENT.
;       UNPACK INTO INTERNAL FORM.
;       REPACK INTO DESTINATION FORM.
;       STORE RESULT ON STACK.
;       RESTORE REGISTERS AND RETURN.

NSAVED  EQU     5*4            ;SAVE D0-D4

        GLOBAL  %I_2_D,%D_2_I,%D_2_F
;       GLOBAL  %F_2_D

        EXTERN  %I_PACK,%I_UNPK
        EXTERN  %F_PACK,%F_UNPK
        EXTERN  %G_NINT,%G_INT

%I_2_D                         ; DBLE ( INTEGER )
;------
        MOVE.L  (SP),-(SP)
        MOVE.L  8(SP),4(SP)    ;MAKE EXTRA ROOM ON STACK
        MOVEM.L D0-D4,-(SP)
        MOVE.L  NSAVED+4(SP),D1
        JSR     %I_UNPK
;---;        BRA.S   XIT_2_D
;---;%F_2_D                         ; DOUBLE ( REAL )
;---;;------
;---;        MOVE.L  (SP),-(SP)
;---;        MOVE.L  8(SP),4(SP)
;---;        MOVEM.L D0-D4,-(SP)
;---;        MOVE.L  NSAVED+4(SP),D1
;---;        JSR     %F_UNPK
XIT_2_D BSR     %D_PACK
        MOVEM.L D0-D1,NSAVED+4(SP)
        MOVEM.L (SP)+,D0-D4
        RTS
%D_2_I                         ; INT ( DOUBLE )
;------
        MOVEM.L D0-D4,-(SP)
        MOVEM.L NSAVED+4(SP),D0-D1
        BSR     %D_UNPK
        JSR     %I_PACK
        BRA.S   D_2_XIT
%D_2_F                         ; REAL ( DOUBLE )
;------
        MOVEM.L D0-D4,-(SP)
        MOVEM.L NSAVED+4(SP),D0-D1
        BSR     %D_UNPK
        JSR     %F_PACK
D_2_XIT MOVE.L  D1,NSAVED+2*4(SP)
        MOVEM.L (SP)+,D0-D4
        MOVE.L  (SP)+,(SP)     ;ADJUST STACK
        RTS
        PAGE    ;--------------
        GLOBAL  %I_IDNINT,%D_DNINT,%D_DINT

; ROUTINES TO FIND THE NEAREST INTEGER (ROUNDED/TRUNCATED).

; THE CALLING SEQUENCE IS
;       ENTER WITH STACK CONTAINING
;               ROOM FOR ANSWER (4 OR 8 BYTES)
;               ARGUMENT (4 OR 8 BYTES)
;               RETURN ADDRESS
;       EXIT WITH STACK CONTAINING
;               RESULT (4 OR 8 BYTES)
;       REGISTERS D0-D4 ARE DESTROYED.

; GENERAL DESIGN:
;       LOAD ARGUMENT
;       UNPACK INTO INTERNAL FORM
;       IF EXPONENT IS POSITIVE THEN
;               SHIFT TO FIND BIT AFTER DECIMAL POINT
;               ROUND UP IF 1
;       REPACK

%I_IDNINT                      ; IDNINT ( DOUBLE ) -> INTEGER
;------
        movem.l d1-d6,-(sp)
        movem.l 28(sp),d0/d1
        bsr.s   %D_UNPK
        jsr     %G_NINT
        jsr     %I_PACK
        move.l  d1,d0
        movem.l (sp)+,d1-d6
        move.l  (sp)+,(sp)
        move.l  (sp)+,(sp)
        rts
%D_DNINT                       ; DNINT ( DOUBLE ) -> DOUBLE
;------
        movem.l 4(sp),d0/d1
        movem.l d2-d6,-(sp);
        bsr.s   %D_UNPK
        jsr     %G_NINT
        bra.s   DIXIT
%D_DINT                        ; DINT ( DOUBLE ) -> DOUBLE
;------
        movem.l 4(sp),d0/d1
        movem.l d2-d6,-(sp)
        bsr.s   %D_UNPK
        jsr     %G_INT
DIXIT   bsr     %D_PACK
        movem.l (sp)+,d2-d6
                                ; Result is in D0/D1
        move.l  (sp)+,(sp)
        move.l  (sp)+,(sp)
        rts
        PAGE    ;---------------------
        GLOBAL  %PD_UNPK,%D_UNPK
        EXTERN  %U_STOR
; SPLIT A DOUBLE PRECISION NUMBER INTO ITS
;   SIGN, EXPONENT, AND MANTISSA.
                              
; INPUT:
;       FLOATING VALUE
;       ADDR OF RECORD
; REGISTERS: A0,D0-D4 DESTROYED.

; FORMAT OF UNPACKED RECORD:
;       RECORD
;       D0/D1   MANTISSA: ARRAY[2] OF LONGINT;
;       D2.W    EXPONENT: -32768..32767;
;       D3.W    SIGN:     (BIT 15)
;       D3.B    TYPE:     1..5               
;       END;

%PD_UNPK
;------
;;;;;;; Begin RSG changes to preserve D3-D7/A2-A3 09-Feb-83 ;;;;;;;
        MOVE.L  8(SP),A0       ;ADDR OF NUMBER
        move.l  (sp)+,a1       ; Pop return address         ;;RSG;;
        move.l  (sp)+,d2       ; Pop result address         ;;RSG;;
        move.l  a1,(sp)        ; Set up final return        ;;RSG;;
        movem.l d3-d7/a2-a3,-(sp) ;                         ;;RSG;;
        subq.w  #4,sp          ; Dummy param for %U_STOR    ;;RSG;;
        move.l  d2,-(sp)       ; Param for %U_STOR          ;;RSG;;
        MOVEM.L (A0),D0/D1     ;NUMBER
        BSR.S   %D_UNPK
     ;;;JMP     %U_STOR                                     ;;RSG;;
        jsr     %U_STOR        ; JSR instead of JMP         ;;RSG;;
        movem.l (sp)+,d3-d7/a2-a3 ;                         ;;RSG;;
        rts                                                 ;;RSG;;
;;;;;;; End** RSG changes to preserve D3-D7/A2-A3 09-Feb-83 ;;;;;;;

; INPUT:  D0/D1  NUMBER
; OUTPUT: D0-D3  UNPACKED RECORD
; REGISTERS DESTROYED:  D0-D4

%D_UNPK
;------
        MOVE.L  #$FFF00000,D2  ;MASK FOR SIGN AND EXPONENT
        MOVE.L  D0,D3
        SWAP    D3             ;SIGN
        AND.L   D0,D2          ;EXTRACT EXPONENT
        EOR.L   D2,D0          ;TOP OF MANTISSA CLEARED OUT
        MOVE.L  D1,D4
        OR.L    D0,D4          ;NON-ZERO IFF MANTISSA NON-ZERO
        LSL.L   #1,D2          ;TOSS SIGN
        BNE.S   UNP2           ;NOT 0/GU
        MOVE.B  #1,D3          ;--ZERO   
        TST.L   D4                     
        BEQ.S   UNP8                   
        MOVE.B  #2,D3          ;--GU     
        BRA.S   UNP8
UNP2    SWAP    D2
        LSR.W   #16-11,D2      ;EXP TO BOTTOM OF REG
        CMP.W   #$7FF,D2       ;INF/NAN       
        BNE.S   UNP3           ;PLAIN
        MOVE.W  #$6000,D2
        MOVE.B  #4,D3          ;--INF    
        TST.L   D4                     
        BEQ.S   UNP9                   
        MOVE.B  #5,D3          ;--NAN    
        BRA.S   UNP9                   
UNP3    BSET    #20,D0         ;HIDDEN BIT
        SUBQ.W  #1,D2                  
        MOVE.B  #3,D3          ;--NUM    
UNP8    SUBI.W  #1022+52,D2
UNP9    RTS
        PAGE    ;-----------------------
        GLOBAL  %PD_PACK,%D_PACK
        EXTERN  %U_LOAD

; RECONSTRUCT A DOUBLE PRECISION NUMBER FROM A RECORD CONTAINING
;   ITS PIECES.

; INPUT
;       ADDRESS OF WHERE TO PUT ANSWER
;       ADDRESS OF RECORD
; REGISTERS: A0, D0-D4 DESTROYED
; USAGE:
;       D2      UPPER
;       D3      LOWER                  
;       D6      EXPONENT               

; ERROR CONDITIONS
;   IF THE NUMBER IS TOO BIG FOR DOUBLE PRECISION,
;     INFINTY IS GENERATED.

%PD_PACK
;------
;;;;;;; Begin RSG changes to preserve D3-D7/A2-A3 09-Feb-83 ;;;;;;;
        movem.l (sp)+,d0/d1/d2 ; Pop parameters             ;;RSG;;
        movem.l d3-d7/a2-a3,-(sp) ;                         ;;RSG;;
        movem.l d0/d1/d2,-(sp) ; Push parameters            ;;RSG;;
        JSR     %U_LOAD
        BSR.S   %D_PACK
        MOVEM.L D0/D1,(A0)     ;STORE RESULT
        move.l  (sp)+,a0       ; Pop return address         ;;RSG;;
        addq.w  #8,sp                                       ;;RSG;;
        movem.l (sp)+,d3-d7/a2-a3 ;                         ;;RSG;;
        jmp     (a0)                                        ;;RSG;;
;;;;;;; End** RSG changes to preserve D3-D7/A2-A3 09-Feb-83 ;;;;;;;

; INPUT:  D0-D3  UNPACKED RECORD
; OUTPUT: D0/D1  NUMBER
; REGISTERS DESTROYED:  D0-D4

%D_PACK
;------                                             
        CMPI.B  #4,D3          ;TYPE         
        BLT.S   DPK0                   
        OR.L    D1,D0                                      ;PKNAN
        LSL.L   #1,D0                                      ;PKNAN
        ORI.L   #$FFE00000,D0
        BRA.S   DPKSGN         ;NAN OR INF    
DPK0
        ADDI.W  #1022+52+12,D2
        EXG     D0,D2          ;UPPER : EXP
        EXG     D0,D6          ;NOW D2=UPPER, D6=EXP, D6 SAVED IN D0
        EXG     D1,D3          ;LOWER : SIGN/TYPE
        BSR.S   %D_NORM        ;NORMALIZE
        BSR     %D_RCP                 
        MOVE.L  D0,D6          ;RESTORE D6
        MOVE.L  D2,D0
        EXG     D3,D1
DPKSGN  LSL.W   #1,D3          ;SIGN
        ROXR.L  #1,D0          ;APPEND SIGN   
        RTS
        PAGE    ;---------------
        GLOBAL  %D_EXTE
; EXTRACT EXPONENTS FROM 2 DOUBLE PRECISION NUMBERS.

; INPUT
;       D0/D1   ONE OPERAND
;       D2/D3   OTHER OPERAND

; OUTPUT
;       D0/D1   MANTISSA, WAITING FOR HIDDEN BIT TO BE TURNED ON
;       D2/D3   OTHER MANTISSA
;       D6      EXPONENT CORRESPONDING TO D2/D3
;       D7      EXPONENT CORRESPONDING TO D0/D1

; DESTROYS D4

%D_EXTE
;------
        MOVEQ   #11,D4         ;SIZE OF EXPONENT
; TRANSFORM EACH FROM          ; EU0 ML  IN D2/D3 (AND D0/D1)
        ROL.L   D4,D0
        ROL.L   D4,D2          ; U0E
        ROL.L   D4,D1
        ROL.L   D4,D3          ;     LM
        MOVE.L  #$7FF,D6       ;EXPONENT-SIZED MASK
        MOVE.L  D6,D7
        AND.L   D2,D6          ; 00E     EXPONENT
        EOR.L   D6,D2          ; U00
        MOVE.L  D7,D4
        AND.L   D3,D4          ; 00M     CARRY
        EOR.L   D4,D3          ;     L0
        LSR.L   #1,D2          ; 0U0
        OR.L    D4,D2          ; 0UM
; END OF TRANSFORM OF LARGER   ; 0UM L0  IN D2/D3, PLUS EXP IN D6
        MOVE.L  D7,D4
        AND.L   D0,D7          ; 00E
        EOR.L   D7,D0          ; U00
        AND.L   D1,D4          ; 00M
        EOR.L   D4,D1          ;     L0
        LSR.L   #1,D0          ; 0U0
        OR.L    D4,D0          ; 0UM
; END OF TRANSFORM OF SMALLER  ; 0UM L0  IN D0/D1, EXP IN D7
        RTS
        PAGE    ;--------------
        GLOBAL  %D_NORM
; NORMALIZE A DOUBLE PRECISION NUMBER.

; INPUT
;       D2/D3   MANTISSA
;       D6      EXPONENT

%D_NORM
;------ 
        TST.L   D2
        BNE.S   NORM1          ;UPPER IS NON-ZERO          ;GUBUG
        CMPI.W  #32,D6
        BLT.S   NORM           ;ABOUT TO BE DENORMALIZED
        SUBI.W  #32,D6
        EXG     D3,D2          ;SHIFT 32
        TST.L   D2
        BEQ.S   CANCELN        ;IF RESULT = 0
NORM1   BMI.S   NORMXIT        ;IF ALREADY NORMALIZED      ;GUBUG
NORM    LSL.L   #1,D3          ;NORMALIZE
        ROXL.L  #1,D2
        DBMI    D6,NORM        ;LOOP UNTIL NORMALIZED
        DBPL    D6,NORMXIT     ;MAKE SURE D6 DECREMENTED   ;GUBUG
NORMXIT RTS

CANCELN MOVE.W  #-2222,D6      ;EXP=0 FOR ZERO
        RTS
        PAGE    ;----------------------
        GLOBAL  %D_NRCP,%D_RCP,%D_USEL  
; ROUND, CHECK FOR OVER/UNDERFLOW, AND PACK IN THE EXPONENT.

; %D_NRCP DOES ONE NORMALIZE AND THEN CALLS %D_RCP.
; %D_RCP ROUNDS THE DOUBLE VALUE AND PACKS THE EXPONENT IN,
;   CATCHING INFINITY, ZERO, AND DENORMALIZED NUMBERS.
; %D_USEL PUTS TOGETHER THE LARGER ARGUMENT.

; INPUT:
;       D2/D3   MANTISSA (- IF NORM)   
;       D6      BIASED EXPONENT        
;       (NEED SIGN, STICKY)            
; OUTPUT:
;       D2/D3   MOST OF NUMBER,        
;               NO SIGN OR HIDDEN BIT,
;               WAITING TO SHIFT SIGN IN.
; OTHER:
;       D4      LOST
;       D5      UNCHANGED

%D_NRCP 
;------
        TST.L   D2
        BMI.S   %D_RCP         ;ALREADY NORMALIZED
        SUBQ.W  #1,D6
        LSL.L   #1,D3          ;DO EXTRA NORMALIZE (FOR MUL/DIV)
        ROXL.L  #1,D2

%D_RCP
;------
        TST.W   D6
        BGT.S   DRCPOK
; EXPONENT IS NEG; DENORMALIZE BEFORE ROUNDING
        CMP.W   #-53,D6
        BLT.S   SIGNED0        ;GO ALL THE WAY TO ZERO
        NEG.W   D6
DENL    LSR.L   #1,D2          ;DENORMALIZE
        ROXR.L  #1,D3
        DBRA    D6,DENL        ;DECR EXPONENT AND LOOP
        CLR.W   D6
; ROUND
DRCPOK  ADDI.L  #$400,D3       ;CRUDE ROUND             
        BCC.S   DRCP1          ;RND DIDNT OFL
        ADDQ.L  #1,D2          ;CARRY
        BCC.S   DRCP1
        ROXR.L  #1,D2
        ROXR.L  #1,D3
        ADDQ.W  #1,D6
DRCP1   CMPI.W  #$7FF,D6
        BGE.S   DRCPBIG        ;EXP TOO BIG
%D_USEL
;------
; REBUILD ANSWER         D2/D3 = HUM LX  H=HIDDEN, U=UPPER, M=MIDDLE
                               ;          L=LOWER, X=TRASH
USEL    MOVE.L  #$FFFFF800,D4  ;         MASK
        AND.L   D4,D3          ;     L0
        AND.L   D2,D4          ; HU0
        EOR.L   D4,D2          ;     0M  PART TO MOVE
        OR.L    D2,D3          ;     LM
        MOVE.L  D4,D2          ; HU0
        LSL.L   #1,D2          ; U00     TOSS HIDDEN BIT (IF ANY)
        BCS.S   DRA            ;         HIDDEN            ;NORM
        CMP.W   #$7FF,D6                                   ;NORM
        BEQ.S   DRA            ;         INF/NAN           ;NORM
        CLR.W   D6             ;         ASSUME 0 IF NO HID;NORM
DRA     MOVEQ   #11,D4
        ROR.L   D4,D3          ;     ML
        OR.W    D6,D2          ; U0E     PUT IN EXPONENT
        ROR.L   D4,D2          ; EU0
        RTS                            
                                                           ;NORM
SIGNED0 CLR.L   D2                     
        CLR.L   D3
        RTS                            

DRCPBIG MOVE.L  #$FFE00000,D2  ;INFINITY      
        CLR.L   D3
        RTS                            
        PAGE    ;-------------------
        GLOBAL  %D_SIGN

; TRANSFER THE SIGN OF THE SECOND ARGUMENT TO THE FIRST.
; PASCAL CALLABLE.
;
; Returns: D0,D1 = Result
;
; Scratches: Only D0,D1.
;

%D_SIGN
;------
        ROXL    12(SP)         ;KILL SIGN IN ARG1
        ROXL    4(SP)          ;GET SIGN OF SECOND
        ROXR    12(SP)         ;PUT SIGN ON
        movem.l 12(sp),d0/d1   ; Return value
        move.l  (sp)+,12(sp)
        adda.w  #12,sp
        rts

        END
; My convert single to double
; Date 09-Jan-85

        ident   F2D
        
        global  %F_2_D

;
; %F_2_D - Convert single precision to double
;
; Parameters: ST.L - Single arg
;
; Returns:    ST.Q - Double result
;
; All registers are preserved.
;

%F_2_D
        move.l  (sp),-(sp)
        movem.l d0/d1,-(sp)
        clr.l   d1
        move.l  16(sp),d0
        andi.l  #$7fffffff,d0
        beq.s   setsign
        cmpi.l  #$7f800000,d0
        blt.s   notbig
        move.l  d0,d1
        andi.l  #$007fffff,d1
        move.l  #$7ff00000,d0
        bra.s   setsign
notbig  cmpi.l  #$007fffff,d0
        bgt.s   notsmal
        moveq   #24,d1
normlop subq.w  #1,d1
        lsl.l   #1,d0
        btst    #23,d0
        beq.s   normlop
        bclr    #23,d0
        lsl.w   #7,d1
        swap    d1
        or.l    d1,d0
        clr.l   d1
        move.w  d0,d1
        andi.b  #7,d1
        ror.l   #3,d1
        lsr.l   #3,d0
        addi.l  #$36900000,d0
        bra.s   setsign
notsmal subi.l  #$3f800000,d0
        move.b  d0,d1
        andi.b  #7,d1
        ror.l   #3,d1
        asr.l   #3,d0
        addi.l  #$3ff00000,d0
setsign btst    #7,16(sp)
        beq.s   signOK
        bset    #31,d0
signOK  move.l  d0,12(sp)
        move.l  d1,16(sp)
f2drts  movem.l (sp)+,d0/d1
        rts
        
        end
        
                                                                                                                                         