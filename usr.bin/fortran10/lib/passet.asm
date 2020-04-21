; File: passet.text
; Date: 07-Mar-83
        
        IDENT   PASSET
        
        GLOBAL  %_UNION,%_DIFF,%_RDIFF,%_INTER
        GLOBAL  %_SING,%_RANGE
        GLOBAL  %_ADJ,%_ADJSZ,%_SETGE,%_SETLE,%_SETEQ,%_SETNE
        
;
; The format of a set on the stack is:
;
;  +---------+
;  | 15 - 0  |
;  +---------+
;  | 31 - 16 |
;  +---------+
;  |   ...   |
;  +---------+
;  | Last Wd |
;  +---------+
;  | # Bytes |
;  +---------+
;

;
; %_INTER - Set intersection
;
; Parameters: ST.S - First set
;             ST.S - Second set
;
; Returns:    ST.S - Result set
;
; This procedure saves all registers
;

%_INTER
        CLR.L   -(SP)           ; Need room if both sets []
        MOVEM.L D0-D2/A0-A1,-(SP)
        LEA     28(SP),A0       ; Address of second set
        MOVE.W  (A0)+,D0        ; Size of second set
        ADDA.W  D0,A0           ; Address of top of set 2 +
        MOVE.L  A0,A1           ; Address of first set
        MOVE.W  (A1)+,D1        ; Size of first set
        ADDA.W  D1,A1           ; Address of top of set 1 +
        CMP.W   D0,D1
        BGE.S   I_GOON
        MOVE.W  D1,D0
I_GOON  MOVE.W  D0,D1           ; D0 and D1 = smallest size set
        LSR.W   #1,D0           ; D0 = number words
        BRA.S   I_TEST
I_LOOP  MOVE.W  -(A0),D2
        AND.W   D2,-(A1)
I_TEST  SUBQ.W  #1,D0
        BPL.S   I_LOOP
        MOVE.W  D1,-(A1)        ; Store set size
        MOVE.L  24(SP),A0       ; Set up return address
        MOVE.L  A0,-(A1)        ; Can't do MOVE.L 24(SP),-(A0)
        MOVE.L  A1,20(SP)       ; Set up new SP value
        MOVEM.L (SP)+,D0-D2/A0-A1
        MOVE.L  (SP)+,SP        ; Pop new SP value
        RTS

;
; %_SING - Singleton set
;
; Parameters: ST.W - Singleton value
;
; Result:     ST.S - Resulting set
;
; This routine saves all registers
;

%_SING
        MOVEM.L D0-D4/A0-A2,-(SP)
        LEA     36(SP),A0       ; address of singleton value
        MOVE.W  (A0),D0         ; D0 = singleton value
        CLR.W   (A0)            ; Default 1st word of result to zero
        MOVE.W  D0,D1
        LSR.W   #4,D1           ; D1 = # Words of leading zeroes
        MOVE.W  D1,D2
        ADD.W   #1,D2           ; D2 = # Words to insert into stack
        MOVE.W  D2,D3
        LSL.W   #1,D3           ; D3 = # Bytes to insert into stack
        MOVE.L  SP,A1           ; Current top of stack
        SUBA.W  D3,SP
        MOVE.L  SP,A2           ; New top of stack
        MOVE.W  #18,D4          ; # Words to copy in D4
S_LOOP1 MOVE.W  (A1)+,(A2)+     ; Copy each remaining word
        SUB.W   #1,D4
        BNE.S   S_LOOP1
ZEROSTK MOVE.L  A2,A0
S_LOOP2 CLR.W   (A2)+           ; Default each word of result to zero
        SUB.W   #1,D2
        BNE.S   S_LOOP2
        MOVE.W  D3,(A0)+        ; Copy #bytes in last word of set
        CLR.W   D2
        ANDI.W  #$0F,D0         ; Mask off 4 bits
        BSET    D0,D2           ; Set the bit
        MOVE.W  D2,(A0)         ; Copy word with bit set into stack
        MOVEM.L (SP)+,D0-D4/A0-A2
        RTS
        
;
; %_UNION - Set union
;
; Parameters: ST.S - First set
;             ST.S - Second set
;
; Returns:    ST.S - Result set
;
; This procedure saves all registers
;

%_UNION
        CLR.L   -(SP)
        MOVEM.L D0-D3/A0-A1,-(SP)
        LEA     32(SP),A0       ; Address of second set
        MOVE.W  (A0)+,D0        ; Size of second set
        ADDA.W  D0,A0           ; Address of top of set 2 +
        MOVE.L  A0,A1           ; Address of first set
        MOVE.W  (A1)+,D1        ; Size of first set
        ADDA.W  D1,A1           ; Address of top of set 1 +
        MOVE.W  D0,D3
        CMP.W   D0,D1
        BGE.S   U_GOON
        MOVE.W  D1,D3           ; D3 = smallest set size
U_GOON  LSR.W   #1,D3           ; D3 = # Words
        BRA.S   U_TEST
U_LOOP  MOVE.W  -(A0),D2
        OR.W    D2,-(A1)
U_TEST  SUBQ.W  #1,D3
        BPL.S   U_LOOP
        SUB.W   D1,D0           ; Compare set sizes
        BEQ.S   U_DONE          ; Finish up if same size
        BLT.S   U_A_BIG
        ADD.W   D0,D1           ; D1 = Biggest Size
        LSR.W   #1,D0           ; D0 = # Words bigger
UB_LOOP MOVE.W  -(A0),-(A1)
        SUBQ.W  #1,D0
        BGT.S   UB_LOOP
        BRA.S   U_DONE
U_A_BIG ADDA.W  D0,A1           ; First set is bigger
U_DONE  MOVE.W  D1,-(A1)        ; Store set size
        MOVE.L  28(SP),A0       ; Set up return address
        MOVE.L  A0,-(A1)
        MOVE.L  A1,24(SP)       ; Set up new SP value
        MOVEM.L (SP)+,D0-D3/A0-A1
        MOVE.L  (SP)+,SP        ; Pop new SP value
        RTS

;
; %_DIFF - Set difference
;
; Parameters: ST.S - First set
;             ST.S - Second set
;
; Returns:    ST.S - Result set
;
; This procedure saves all registers
;

%_DIFF
        CLR.L   -(SP)
        MOVEM.L D0-D3/A0-A1,-(SP)
        LEA     32(SP),A0       ; Address of second set
        MOVE.W  (A0)+,D0        ; Size of second set
        ADDA.W  D0,A0           ; Address of top of set 2 +
        MOVE.L  A0,A1           ; Address of first set
        MOVE.W  (A1)+,D1        ; Size of first set
        ADDA.W  D1,A1           ; Address of top of set 1 +
        MOVE.W  D0,D3
        CMP.W   D0,D1
        BGE.S   D_GOON
        MOVE.W  D1,D3           ; D3 = smallest set size
D_GOON  LSR.W   #1,D3           ; D3 = # Words
        BRA.S   D_TEST
D_LOOP  MOVE.W  -(A0),D2
        NOT.W   D2
        AND.W   D2,-(A1)
D_TEST  SUBQ.W  #1,D3
        BPL.S   D_LOOP
        SUB.W   D1,D0           ; Compare set sizes
        BGT.S   D_DONE          ; Finished if A <= B in size
        ADDA.W  D0,A1           ; First set is bigger
D_DONE  MOVE.W  D1,-(A1)        ; Store set size
        MOVE.L  28(SP),A0       ; Set up return address
        MOVE.L  A0,-(A1)
        MOVE.L  A1,24(SP)       ; Set up new SP value
        MOVEM.L (SP)+,D0-D3/A0-A1
        MOVE.L  (SP)+,SP        ; Pop new SP value
        RTS

;
; %_RDIFF - Reverse set difference
;
; Parameters: ST.S - First set
;             ST.S - Second set
;
; Returns:    ST.S - Result set
;
; This procedure saves all registers
;

%_RDIFF
        CLR.L   -(SP)
        MOVEM.L D0-D3/A0-A1,-(SP)
        LEA     32(SP),A0       ; Address of second set
        MOVE.W  (A0)+,D0        ; Size of second set
        ADDA.W  D0,A0           ; Address of top of set 2 +
        MOVE.L  A0,A1           ; Address of first set
        MOVE.W  (A1)+,D1        ; Size of first set
        ADDA.W  D1,A1           ; Address of top of set 1 +
        MOVE.W  D0,D3
        CMP.W   D0,D1
        BGE.S   V_GOON
        MOVE.W  D1,D3           ; D3 = smallest set size
V_GOON  LSR.W   #1,D3           ; D3 = # Words
        BRA.S   V_TEST
V_LOOP  NOT.W   -(A1)
        MOVE.W  -(A0),D2
        AND.W   D2,(A1)
V_TEST  SUBQ.W  #1,D3
        BPL.S   V_LOOP
        SUB.W   D1,D0           ; Compare set sizes
        BLE.S   V_DONE          ; Finished if A <= B in size
        ADD.W   D0,D1           ; D1 = Biggest Size
        LSR.W   #1,D0           ; D0 = # Words bigger
VB_LOOP MOVE.W  -(A0),-(A1)
        SUBQ.W  #1,D0
        BGT.S   VB_LOOP
V_DONE  MOVE.W  D1,-(A1)        ; Store set size
        MOVE.L  28(SP),A0       ; Set up return address
        MOVE.L  A0,-(A1)
        MOVE.L  A1,24(SP)       ; Set up new SP value
        MOVEM.L (SP)+,D0-D3/A0-A1
        MOVE.L  (SP)+,SP        ; Pop new SP value
        RTS

;
; %_RANGE - Set range
;
; Parameters: ST.W - Minimum value
;             ST.W - Maximum value
;
; Result:     ST.S - Resulting set
;
; This procedure saves all registers.
;

%_RANGE
        MOVEM.L D0-D4/A0-A2,-(SP)
        MOVE.W  36(SP),D1       ; D1 = max value
        MOVE.W  38(SP),D0       ; D0 = min value
        CLR.L   36(SP)          ; Zero the parameters
        LEA     40(SP),A0       ; A0 = address of set + 1
        TST.W   D0              ; if Min < 0 then Min := 0
        BPL.S   MINPOS
        CLR.W   D0
MINPOS  CMP.W   D0,D1           ; Compare Min to Max
        BLT.S   NULLSET
        MOVE.W  D1,D2
        LSR.W   #4,D2           ; D2 = Size in words - 1 of final set
        MOVE.W  D2,D3
        LSL.W   #1,D3           ; D3 = Size in bytes - 2 of final set
        BEQ.S   NOEXPAN
        MOVE.L  SP,A1           ; Current top of stack
        SUBA.W  D3,SP
        MOVE.L  SP,A2           ; New top of stack
        MOVE.W  #18,D4          ; # Words to copy in D4
R_LOOP1 MOVE.W  (A1)+,(A2)+     ; Copy each remaining word
        SUB.W   #1,D4
        BNE.S   R_LOOP1
R_LOOP3 CLR.W   (A2)+           ; Default each word of result to zero
        SUB.W   #1,D2
        BNE.S   R_LOOP3
NOEXPAN ADD.W   #2,D3           ; D3 = Size in bytes
        MOVE.W  D3,36(SP)       ; Copy #bytes in last word of set
R_LOOP2 MOVE.W  D0,D2
        MOVE.W  D2,D3
        LSR.W   #4,D2
        LSL.W   #1,D2
        NEG.W   D2
        ANDI.W  #$0F,D3
        CLR.W   D4
        BSET    D3,D4           ; Build the set one bit at a time
        OR.W    D4,-2(A0,D2)
        ADDQ.W  #1,D0
        CMP.W   D0,D1
        BGE.S   R_LOOP2
        MOVEM.L (SP)+,D0-D4/A0-A2
        RTS
NULLSET MOVE.L  32(SP),A0       ; Move return address up one word
        MOVE.L  A0,34(SP)
        MOVEM.L (SP)+,D0-D4/A0-A2
        TST.W   (SP)+           ; Delete extra word
        RTS
        
;
; %_ADJ - Set adjust
;
; Parameters: ST.S - Set
;             ST.W - Desired size in bytes
;
; Returns:    ST.S' - Adjusted set without size word
;
; This routine preserves all registers
;

%_ADJ
        SUBA.W  #256,SP         ; Allow for worst-case set expansion
        MOVEM.L D0-D1/A0-A1,-(SP)
        MOVE.W  276(SP),D0      ; Desired size
        MOVE.W  278(SP),D1      ; Current size
        LEA     280(SP),A0      ; Points to end of set
        MOVE.L  272(SP),A1      ; Return address
        SUB.W   D0,D1
        BGE.S   A_OK
A_LOOP  CLR.W   -(A0)
        ADDQ.W  #2,D1
        BMI.S   A_LOOP
        BRA.S   A_DONE
A_OK    ADDA.W  D1,A0
A_DONE  MOVE.L  A1,-(A0)        ; Set up return address
        MOVE.L  A0,16(SP)       ; Set up new SP
        MOVEM.L (SP)+,D0-D1/A0-A1
        MOVE.L  (SP)+,SP
        RTS

;
; %_ADJSZ - Set adjust, leave size on stack
;
; Parameters: ST.S - Set
;             ST.W - Desired size in bytes
;
; Returns:    ST.S - Adjusted set with size word
;
; This routine preserves all registers
;

%_ADJSZ
        SUBA.W  #256,SP         ; Allow for worst-case set expansion
        MOVEM.L D0-D1/A0-A1,-(SP)
        MOVE.W  276(SP),D0      ; Desired size
        MOVE.W  278(SP),D1      ; Current size
        LEA     280(SP),A0      ; Points to end of set
        MOVE.L  272(SP),A1      ; Return address
        SUB.W   D0,D1
        BGE.S   A_OKS
A_LOOPS CLR.W   -(A0)
        ADDQ.W  #2,D1
        BMI.S   A_LOOPS
        BRA.S   A_DONES
A_OKS   ADDA.W  D1,A0
A_DONES MOVE.W  D0,-(A0)        ; Set up return set length
        MOVE.L  A1,-(A0)        ; Set up return address
        MOVE.L  A0,16(SP)       ; Set up new SP
        MOVEM.L (SP)+,D0-D1/A0-A1
        MOVE.L  (SP)+,SP
        RTS

;
; %_SETNE - Set inequality test
;
; Parameters: ST.S - First set
;             ST.S - Second set
;
; Returns:    D0.B - Result
;
; Scratches: Only D0
;

%_SETNE
        CLR.L   -(SP)
        MOVEM.L D1-D3/A0-A2,-(SP)
        MOVEQ   #1,D3           ; Result if <>
        BRA.S   EQ_NE

;
; %_SETEQ - Set equality test
;
; Parameters: ST.S - First set
;             ST.S - Second set
;
; Returns:    D0.B - Result
;
; Scratches: Only D0
;

%_SETEQ
        CLR.L   -(SP)
        MOVEM.L D1-D3/A0-A2,-(SP)
        CLR.W   D3              ; Result if <>
EQ_NE   LEA     32(SP),A0       ; Address of second set
        MOVE.W  (A0)+,D0        ; Size of second set
        ADDA.W  D0,A0           ; Address of top of set 2 +
        MOVE.L  A0,A1           ; Address of first set
        MOVE.W  (A1)+,D1        ; Size of first set
        ADDA.W  D1,A1           ; Address of top of set 1 +
        MOVE.L  A1,A2           ; Save address for result
        MOVE.W  D0,D2
        CMP.W   D0,D1
        BGE.S   EQ_GOON
        MOVE.W  D1,D2           ; D2 = smallest set size
EQ_GOON SUBA.W  D2,A0
        SUBA.W  D2,A1
        LSR.W   #1,D2           ; D2 = # Words
        BRA.S   EQ_TEST
EQ_LOOP CMPM.W  (A0)+,(A1)+
        BNE.S   EQ_FALS
EQ_TEST SUBQ.W  #1,D2
        BPL.S   EQ_LOOP
        SUB.W   D0,D1           ; Compare set sizes
        BEQ.S   EQ_TRUE         ; Finish up if same size
        BMI.S   E_TOP_S
        SUBA.W  D0,A1           ; Set A1 to address of first
                                ; word in extra stuff + 2
        LSR.W   #1,D1           ; D1 = # Words bigger
EQLOOP2 TST.W   -(A1)
        BNE.S   EQ_FALS
        SUBQ.W  #1,D1
        BGT.S   EQLOOP2
        BRA.S   EQ_TRUE
E_TOP_S SUBA.W  D0,A0           ; Top set is smaller
        SUBA.W  D1,A0
        ASR.W   #1,D1
EQLOOP3 TST.W   -(A0)
        BNE.S   EQ_FALS
        ADDQ.W  #1,D1
        BMI.S   EQLOOP3
EQ_TRUE EORI.W  #1,D3
EQ_FALS MOVE.B  D3,D0           ; Get result
        MOVE.L  28(SP),-(A2)    ; Set up return address
        MOVE.L  A2,24(SP)       ; Set up new SP value
        MOVEM.L (SP)+,D1-D3/A0-A2
        MOVE.L  (SP)+,SP        ; Pop new SP value
        RTS

;
; %_SETGE - Set inclusion test
;
; Parameters: ST.S - First set
;             ST.S - Second set
;
; Returns:    D0.B - Result
;
; Scratches: Only D0
;

%_SETGE
        CLR.L   -(SP)
        MOVEM.L D1-D3/A0-A2,-(SP)
        LEA     32(SP),A1       ; Address of second set
        MOVE.W  (A1)+,D1        ; Size of second set
        ADDA.W  D1,A1           ; Address of top of set 2 +
        MOVE.L  A1,A0           ; Address of first set
        MOVE.W  (A0)+,D0        ; Size of first set
        ADDA.W  D0,A0           ; Address of top of set 1 +
        MOVE.L  A0,A2           ; Save address for result
        BRA.S   LE_GE           ; Compute (A1) <= (A0)
        
;
; %_SETLE - Set inclusion test
;
; Parameters: ST.S - First set
;             ST.S - Second set
;
; Returns:    D0.B - Result
;
; Scratches: Only D0
;

%_SETLE
        CLR.L   -(SP)
        MOVEM.L D1-D3/A0-A2,-(SP)
        LEA     32(SP),A0       ; Address of second set
        MOVE.W  (A0)+,D0        ; Size of second set
        ADDA.W  D0,A0           ; Address of top of set 2 +
        MOVE.L  A0,A1           ; Address of first set
        MOVE.W  (A1)+,D1        ; Size of first set
        ADDA.W  D1,A1           ; Address of top of set 1 +
        MOVE.L  A1,A2           ; Save address for result
LE_GE   MOVE.W  D0,D2
        CMP.W   D0,D1
        BGE.S   LE_GOON
        MOVE.W  D1,D2           ; D2 = smallest set size
LE_GOON LSR.W   #1,D2           ; D2 = # Words
        BRA.S   LE_TEST
LE_LOOP MOVE.W  -(A0),D3
        NOT.W   D3
        AND.W   -(A1),D3
        BNE.S   LE_FALS
LE_TEST SUBQ.W  #1,D2
        BPL.S   LE_LOOP
        SUB.W   D0,D1           ; Compare set sizes
        BEQ.S   LE_TRUE         ; Finish up if same size
        BMI.S   LE_TRUE         ; True if (A1) shorter
        LSR.W   #1,D1           ; D1 = # Words bigger
LELOOP2 TST.W   -(A1)
        BNE.S   LE_FALS
        SUBQ.W  #1,D1
        BPL.S   LELOOP2
LE_TRUE MOVEQ   #1,D3
        BRA.S   LE_DONE
LE_FALS CLR.W   D3
LE_DONE MOVE.B  D3,D0           ; Set up result
        MOVE.L  28(SP),-(A2)    ; Set up return address
        MOVE.L  A2,24(SP)       ; Set up new SP value
        MOVEM.L (SP)+,D1-D3/A0-A2
        MOVE.L  (SP)+,SP        ; Pop new SP value
        RTS

        END

                                                                                                                                                                                                                                                                                                                                                                                                                                                            