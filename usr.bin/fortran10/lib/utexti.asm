; File: utexti.text
; Date: 14-Jul-83

        UPPER
        IDENT   IDTEXTI
        
        GLOBAL  %R_C,%R_I,%R_STR,%R_PAOC,%R_LN,%_LGET
        EXTERN  %_EOLN,%_GETCH,%_RDIT
        
;
; FIB record offsets and scalar definitions
;

F_TVALID EQU    0               ; ISO file, F^ defined
F_IEMPTY EQU    1               ; Interactive, F^ undefined
F_IVALID EQU    2               ; Interactive, F^ defined
F_TEMPTY EQU    3               ; ISO file, F^ undefined

FIBEOL  EQU     4
FIBEOF  EQU     5
FIBTEXT EQU     6
FIBSTAT EQU     7
FIBRSIZ EQU     8
FIBNXTB EQU     10
FIBFD   EQU     12
FIBUPAR EQU     602

IORES   EQU     26

;
; %_LGET - Perform a get on a file
;
; Parameters: A0.L - FIB address
;
; All registers preserved
;

%_LGET  MOVEM.L D0-D7/A0-A6,-(SP)
        CLR.W   D5                      ; D5 is character counter
LGETTP  CMP.W   FIBRSIZ(A0),D5          ; frecsize :: ctr
        BGE.S   LOOPBT
        MOVEA.L (A0),A3                 ; A3 points to the window
        ADDA.W  D5,A3                   ; A3 points to next win byte
        JSR     %_GETCH                 ; Load up D0.B with next char
        MOVE.B  D0,(A3)                 ; Stuff character into window
        TST.B   FIBEOF(A0)              ; Error or end of file?
        BNE.S   LGDONE
        ADDQ.W  #1,D5                   ; Loop counter
        BRA.S   LGETTP
LOOPBT  TST.B   FIBTEXT(A0)             ; Is it file of char?
        BEQ.S   LGDONE
        CLR.B   FIBEOL(A0)              ; Clear EOL
        MOVEA.L (A0),A3                 ; A3 points to the window
        CMPI.B  #10,(A3)                ; Was last char read <lf>
        BNE.S   LGDONE
        MOVE.B  #' ',(A3)               ; Coerce <lf> to blank
        MOVE.B  #1,FIBEOL(A0)           ; Set EOL
LGDONE  CMPI.B  #F_TEMPTY,FIBSTAT(A0)
        BNE.S   LG_NISO                 ; If state = TEMPTY
        MOVE.B  #F_TVALID,FIBSTAT(A0)   ; then state := TVALID
LG_NISO CMPI.B  #F_IEMPTY,FIBSTAT(A0)
        BNE.S   LG_STDN                 ; If state = IEMPTY
        MOVE.B  #F_IVALID,FIBSTAT(A0)   ; then state := IVALID
LG_STDN MOVEM.L (SP)+,D0-D7/A0-A6
        RTS
        
;
; MKVALID - Fetch a character if needed
;
; Paratemers: A0.L - File address
;
; All registers are preserved.
;

MKVALID CMPI.B  #F_IVALID,FIBSTAT(A0)
        BEQ.S   MV_DONE         ; If state <> IVALID or ...
        CMPI.B  #F_TVALID,FIBSTAT(A0)
        BEQ.S   MV_DONE         ; ... state <> TVALID then
        JSR     %_LGET          ; Do a get(f)
MV_DONE RTS

;
; MKEMPTY - Set fstate to TEMPTY or IEMPTY
;
; Parameters: A0.L - FIB Address
;
; All registers are preserved
;

MKEMPTY CMPI.B  #F_TVALID,FIBSTAT(A0)
        BNE.S   ME_NISO                 ; If state = TVALID
        MOVE.B  #F_TEMPTY,FIBSTAT(A0)   ; then state := TEMPTY
ME_NISO CMPI.B  #F_IVALID,FIBSTAT(A0)
        BNE.S   ME_DONE                 ; If state = IVALID
        MOVE.B  #F_IEMPTY,FIBSTAT(A0)   ; then state := IEMPTY
ME_DONE RTS

;
; %R_C - Read a character
;
; Parameters: ST.L - File Address
;
; Returns:    D0.W - The character read
;
; Scratches: Only D0
;

%R_C
        MOVE.L  A0,-(SP)
        MOVE.L  8(SP),A0        ; A0 = file address
        MOVE.W  #0,IORES(A5)    ; Clear IORESULT before doing I/O
        JSR     %_RDIT          ; Set file up for reading
        BSR.S   MKVALID
        CLR.W   D0
        MOVE.B  FIBUPAR(A0),D0
        BSR.S   MKEMPTY
        MOVE.L  (SP)+,A0
        MOVE.L  (SP)+,(SP)
        RTS

;
; %R_LN - READLN
;
; Parameters:  ST.L - Address of input file
;

%R_LN
        MOVE.L  4(SP),A0        ; A0 = fib address
        MOVE.L  (SP)+,(SP)      ; Set up return address
        MOVE.W  #0,IORES(A5)    ; Clear IORESULT before doing I/O
        JSR     %_RDIT          ; Set file up for reading
        CMPI.B  #F_TEMPTY,FIBSTAT(A0) ; ISO files must ...
        BNE.S   LN_TEST         ; ... have a valid window
LN_LOOP JSR     %_LGET          ; Get a character.
LN_TEST TST.B   FIBEOL(A0)      ; Was it an EOL?
        BEQ.S   LN_LOOP         ; No. Get another.
        CMPI.B  #F_TVALID,FIBSTAT(A0) ; Yes. IOS FILE?
        BEQ.S   LN_ISO          ; Yes.
        CLR.B   FIBEOL(A0)      ; No. Clear EOL flag
        MOVE.B  #F_IEMPTY,FIBSTAT(A0) ; Set state to IEMPTY
        RTS                     ; and return.
LN_ISO  MOVE.B  #F_TEMPTY,FIBSTAT(A0) ; Set state to TEMPTY
        RTS                     ; and return.

;
; %R_PAOC - Read Packed Array of Character
;
; Parameters: ST.L - File Address
;             ST.L - Array Address
;             ST.W - Size of array in bytes
;

%R_PAOC
        movem.l d3-d4,-(sp)
        move.w  12(sp),d2       ; Size of array
        move.w  d2,d3           ; Save size
        move.l  14(sp),a1       ; Address of PAOC
        move.l  18(sp),a0       ; Address of File
        move.w  #0,IORES(a5)    ; Clear IORESULT before doing I/O
        jsr     %_rdit          ; Set file up for reading
        move.l  a0,-(sp)
        jsr     %_EOLN
        tst.b   d0              ; Check for EOL
        bne.s   rp_fill         ; PAOCs never eat an EOL
        jsr     mkvalid         ; Make sure there's a character
        bra.s   rp_beg          ; Start reading
rp_loop bsr     %_lget          ; Do a GET(F)
rp_beg  move.l  a0,-(sp)        ; Check for EOL
        jsr     %_EOLN
        tst.b   d0
        bne.s   rp_fill
        move.b  FIBUPAR(a0),d0  ; Fetch the character
        tst.b   FIBEOF(a0)      ; Check for EOF
        bne.s   rp_fill
        move.b  d0,(a1)+
        subq.w  #1,d2
        bgt.s   rp_loop
        jsr     mkempty         ; Set state to EMPTY
        bra.s   rp_rts
rp_lop2 move.b  #' ',(a1)+
rp_fill subq.w  #1,d2
        bge.s   rp_lop2
rp_rts  movem.l (sp)+,d3-d4
        move.l  (sp)+,a0
        adda.w  #10,sp
        jmp     (a0)

;
; %R_STR - Read String
;
; Parameters: ST.L - File Address
;             ST.L - String Address
;             ST.W - Max size of string
;
; Clobbers: D0,D2,A0,A1
;

%R_STR
        movem.l d4/a2,-(sp)
        move.w  12(sp),d2        ; Size of string
        move.l  14(sp),a1       ; Address of string
        move.l  18(sp),a0       ; Address of file
        move.l  a1,a2           ; Save pointer to length byte
        clr.b   (a1)+
        move.w  #0,IORES(a5)    ; Clear IORESULT before doing I/O
        jsr     %_RDIT          ; Set file up for reading
        move.l  a0,-(sp)        ; Check for EOL
        jsr     %_EOLN
        tst.b   d0
        bne.s   rs_done         ; Strings never eat an EOL
        bsr     mkvalid         ; Make sure there's a character
        bra.s   rs_beg          ; Start reading
rs_loop bsr     %_lget          ; Do a get(f)
rs_beg  move.l  a0,-(sp)        ; Check for EOL
        jsr     %_EOLN
        tst.b   d0
        bne.s   rs_done         ; Strings never eat an EOL
        tst.b   FIBEOF(a0)      ; Check for EOF
        bne.s   rs_done         ; Strings never eat an EOF
        move.b  FIBUPAR(a0),d0  ; Fetch the character
        move.b  d0,(a1)+
        addq.b  #1,(a2)
        subq.w  #1,d2
        bgt.s   rs_loop
        bsr     mkempty         ; Set to TEMPTY or IEMPTY
rs_done movem.l (sp)+,d4/a2
        move.l  (sp)+,a0
        adda.w  #10,sp
        jmp     (a0)

;
; %R_I - Read Integer
;
; Parameters: ST.L - File Address
;
; Returns:    ST.L - The integer read
;

%R_I
        MOVEM.L D1-D7/A0-A6,-(SP)
        MOVE.L  60(SP),A0       ; File Address
        MOVE.W  #0,IORES(A5)    ; Clear IORESULT before doing I/O
        JSR     %_RDIT          ; Set file up for reading
        CLR.L   D2              ; Value
        CLR.W   D6              ; D6 = #chars
        CLR.B   D7              ; Sign
        BSR     MKVALID         ; Make sure there is a character
        BRA.S   RI_BEG          ;
RI_LOPB JSR     %_LGET          ; Do a GET(F)
RI_BEG  TST.B   FIBEOF(A0)      ; Is EOF set?
        BNE.S   RI_DONE         ; Yes. Don't loop forever.
        MOVE.B  FIBUPAR(A0),D0  ; Fetch the character
        CMPI.B  #' ',D0         ; Is this a blank?
        BEQ.S   RI_LOPB         ; Yes. Keep skipping.
        CMPI.B  #'+',D0         ; No. Is it a sign?
        BEQ.S   RI_PLUS
        CMPI.B  #'-',D0
        BNE.S   RI_NUM          ; Not signed.
        SUBQ.B  #1,D7
RI_PLUS ADDQ.W  #1,D6
RI_LOOP JSR     %_LGET
        TST.B   FIBEOF(A0)      ; Is EOF set?
        BNE.S   RI_DONE         ; Yes. Don't loop forever.
        MOVE.B  FIBUPAR(A0),D0  ; Get the character
RI_NUM  ADDQ.W  #1,D6
        SUBI.B  #'0',D0
        BLT.S   RI_TEST
        CMPI.B  #9,D0
        BGT.S   RI_TEST
        EXT.W   D0
        EXT.L   D0
        MOVE.L  D2,D3
        MULU    #10,D2
        SWAP    D3
        MULU    #10,D3
        ASL.L   #8,D3
        ASL.L   #8,D3
        ADD.L   D3,D2
        ADD.L   D0,D2
        BRA.S   RI_LOOP
RI_TEST TST.B   D7
        BEQ.S   RI_DONE
        NEG.L   D2
RI_DONE MOVE.L  D2,D0
        MOVEM.L (SP)+,D1-D7/A0-A6
        MOVE.L  (SP)+,(SP)
        RTS
        
        END

                                                                                                                                                                                                                                                                    