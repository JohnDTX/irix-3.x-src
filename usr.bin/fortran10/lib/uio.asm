; File: uio.text
; Date: 28-Nov-83
;
        
        UPPER
        IDENT   MERIO
        
        GLOBAL  %_RESET,%_REWRT,%_EOF,%_EOLN
        GLOBAL  %_BLKRD,%_BLKWR,%_UREAD,%_UWRIT
        GLOBAL  %_GET,%_PUT,%_UPARR,%_SEEK,%_UCLR,%_UBUSY

        EXTERN  %_PANIC,%_LGET,%_FLUSH,%_RDIT,%_WRTIT,%_PUTCH,%_HALTF
        EXTERN  %I_MUL4
        
        LOWER
        EXTERN  _lseek,_creat,_open,_close,_write,_read
        UPPER
;
; FIB record offsets and scalar definitions
;

F_TVALID EQU    0               ; ISO file, F^ defined
F_IEMPTY EQU    1               ; Interactive, F^ undefined
F_IVALID EQU    2               ; Interactive, F^ defined
F_TEMPTY EQU    3               ; ISO file, F^ undefined

BEMPTY  EQU     0               ; Buffer empty
BRDING  EQU     1               ; Buffer set up for reading
BWRING  EQU     2               ; Buffer set up for writing

FIBEOL  EQU     4
FIBEOF  EQU     5
FIBTEXT EQU     6
FIBSTAT EQU     7
FIBRSIZ EQU     8
FIBNXTB EQU     10
FIBFD   EQU     12
BUFFED  EQU     16
BSTATE  EQU     17
BNXTBY  EQU     18
BLSTBY  EQU     22
FILNAM  EQU     26
BUFFER  EQU     90
FIBUPAR EQU     602

IORES   EQU     26

;
;
; Convert string arguments to Idris format from UCSD format.
;      UCSD  format :    Nstring
;      Idris format :    string0
; This routine will move the data to -100(A6) and insert null.
;

UCID
    MOVEM.L D0/A2-A3,-(SP)
    LEA     -100(A6),A2     ; Pointer to the target string
    MOVE.B  (A3)+,D0        ; Get the length.
    BEQ.S   ID9             ; If zero, null string, ignore.
ID1
    MOVE.B  (A3)+,(A2)+     ; Shuffle the string over. (must be > 0)
    SUBQ.B  #1,D0           ; Repeat for entire string.
    BNE.S   ID1
    
ID9 CLR.B   (A2)            ; Place the null.
    MOVEM.L (SP)+,D0/A2-A3
    RTS

;
; %_REWRT - Rewrite a file
;
; Parameters: ST.L - File Address
;             ST.L - Address of Name String
;             ST.W - Kind: -2=text, -1=file, 0=inter, >0=bytes/record
;

%_REWRT
        MOVEQ   #1,D1           ; 1 = Rewrite
        BRA.S   RSETWRT

;
; %_RESET - Reset a file
;
; Parameters: ST.L - File Address
;             ST.L - Address of Name String
;             ST.W - Kind: -2=text, -1=file, 0=inter, >0=bytes/record
;             ST.W - Buffed: 0=notbuffered, 1=buffered
;

%_RESET
        CLR.W   D1              ; 0 = Reset
RSETWRT MOVEM.L D3-D5/A2-A3,-(SP)
        MOVE.W  24(SP),D4       ; Buffed Flag
        MOVE.W  26(SP),D2       ; File Kind
        MOVE.L  28(SP),A3       ; Address of string
        MOVE.L  32(SP),A1       ; File address
        LINK    A6,#-100        ; Get space for Idris file name
; Init FIB
        MOVE.B  #1,FIBEOL(A1)
        MOVE.B  #1,FIBEOF(A1)
        CLR.B   FIBTEXT(A1)
        MOVE.B  #F_TEMPTY,FIBSTAT(A1)
        CLR.W   FIBNXTB(A1)
        LEA     FIBUPAR(A1),A0  ; Compute address of window
        MOVE.L  A0,(A1)         ; Initialize fwindow
        MOVE.B  D4,BUFFED(A1)   ; Buffered as passed to Reset/Rewrite
        MOVE.B  #BEMPTY,BSTATE(A1) ; Buffering state is empty
        CLR.B   FILNAM(A1)      ; No name unless set below
        CMP.W   #-2,D2
        BEQ.S   RRTEXT
        TST.W   D2
        BEQ.S   RRTEXT
; Recbytes tag is neither -2 nor 0, therefor it is either
; -1, meaning file or positive, meaning that is recsize
        BLT.S   FILE
        MOVE.W  D2,FIBRSIZ(A1)  ; recsize in D2
        BRA.S   INITDN
FILE    CLR.L   (A1)            ; File;, set fwindow to nil
        CLR.W   FIBRSIZ(A1)     ; Set record size to zero
        BRA.S   INITDN
RRTEXT  CLR.B   1(A0)           ; D2 is 0 or -2, fwindow^[1] := 0
        MOVE.W  #1,FIBRSIZ(A1)
        MOVE.B  #1,FIBTEXT(A1)
        TST.W   D2
        BNE.S   INITDN
        MOVE.B  #F_IEMPTY,FIBSTAT(A1)
INITDN  CLR.W   IORES(A5)       ; Assume no errors 
        JSR     UCID            ; Convert A3 string to UNIX format
        CMPI.B  #63,(A3)        ; Is length <= 63
        BGT.S   TOOLNG
        LEA     -100(A6),A0     ; Source address
        LEA     FILNAM(A1),A2   ; Target address
MOVMOR  MOVE.B  (A0)+,(A2)+     ; Move next byte
        BNE.S   MOVMOR          ; Loop until zero character
TOOLNG  TST.W   D1
        BEQ.S   OPENIT
; Creat the file, close it since can't creat then read, then reopen
        MOVEM.L D1-D7/A0-A6,-(SP)
        MOVE.L  #420,-(SP)      ; UNIX creat mode
        PEA     -100(A6)        ; Pointer to Unix fname
        LOWER
        JSR     _creat          ; Create it
        UPPER
        ADDQ.W  #8,SP           ; Discard parameters
        MOVE.L  D0,-(SP)        ; Push file descriptor
        LOWER
        JSR     _close          ; Close it
        UPPER
        ADDQ.W  #4,SP           ; Discard parameters of _close
        MOVEM.L (SP)+,D1-D7/A0-A6
OPENIT  MOVEM.L D1-D7/A0-A6,-(SP)
        MOVE.L  #2,-(SP)        ; UNIX update mode
        PEA     -100(A6)        ; Pointer to Unix fname
        LOWER
        JSR     _open           ; Open it
        UPPER
        ADDQ.W  #8,SP           ; Discard parameters
        MOVEM.L (SP)+,D1-D7/A0-A6
        TST.L   D0              ; Peek at result of open
        BGE.S   MD2OK           ; If mode 2 ok, don't fool around
        MOVEM.L D1-D7/A0-A6,-(SP)
        CLR.L   -(SP)           ; UNIX read only mode 0
        PEA     -100(A6)        ; Pointer to Unix fname
        LOWER
        JSR     _open           ; Try to open read only
        UPPER
        ADDQ.W  #8,SP           ; Discard parameters
        MOVEM.L (SP)+,D1-D7/A0-A6
MD2OK   MOVE.L  D0,FIBFD(A1)    ; Check result from open or create
        BLT.S   OPERR
        CLR.B   FIBEOL(A1)
        CLR.B   FIBEOF(A1)
        TST.W   FIBRSIZ(A1)
        BGT.S   MV_DONE
        CMPI.B  #F_TEMPTY,FIBSTAT(A1)
        BNE.S   MV_NISO                 ; If state = TEMPTY
        MOVE.B  #F_TVALID,FIBSTAT(A1)   ; then state := TVALID
MV_NISO CMPI.B  #F_IEMPTY,FIBSTAT(A1)
        BNE.S   MV_DONE                 ; If state = IEMPTY
        MOVE.B  #F_IVALID,FIBSTAT(A1)   ; then state := IVALID
MV_DONE UNLK    A6
        MOVEM.L (SP)+,D3-D5/A2-A3
        MOVE.L  (SP)+,A0                ; Return address
        ADDA.W  #12,SP                  ; Discard parameters
        JMP     (A0)                    ; Return
OPERR   MOVE.W  #10,IORES(A5)   ; File not found error
        BRA.S   MV_DONE

;
; %_EOF - End of file predicate
;
; Parameters:  ST.L - File address
;
; Returns:     D0.B - Boolean Result
;
; This routine preserves all registers.
;

%_EOF
        MOVE.L  A0,-(SP)        ; Save A0
        MOVE.L  8(SP),A0        ; Get file address
        CMPI.B  #F_TEMPTY,FIBSTAT(A0)
        BNE.S   EOF_OK          ; F^ must be valid ...
        JSR     %_RDIT          ; Have to be in read state to do LGET
        JSR     %_LGET          ; ... if file is TEXT
EOF_OK  MOVE.B  FIBEOF(A0),D0   ; Pass answer to caller
        MOVE.L  (SP)+,A0        ; Restore A0
        MOVE.L  (SP)+,(SP)      ; Position return address
        RTS

;
; %_EOLN - End of line predicate
;
; Parameters:  ST.L - File address
;
; Returns:     D0.B - Boolean Result
;
; This routine preserves all registers.
;

%_EOLN
        MOVE.L  A0,-(SP)        ; Save A0
        MOVE.L  8(SP),A0        ; Get file address
        CMPI.B  #F_TEMPTY,FIBSTAT(A0)
        BNE.S   E_NOGET         ; F^ must be valid ...
        JSR     %_RDIT          ; Have to be in read state to do LGET
        JSR     %_LGET          ; ... if file is TEXT
E_NOGET MOVE.B  FIBEOL(A0),D0   ; Pass answer to caller
        MOVE.L  (SP)+,A0        ; Restore A0
        MOVE.L  (SP)+,(SP)      ; Position return address
        RTS

;
; %_BLKRD - Blockread
;
; Parameters: ST.L - File Address
;             ST.L - Buffer address
;             ST.W - # Blocks to read
;             ST.W - Block Number, -1 = Sequential
;
; Returns:    D0.W - # Blocks actually read
;

%_BLKRD
        MOVEQ   #1,D2           ; 1 = blockread
        BRA.S   B_RD_WR

;
; %_BLKWR - Blockwrite
;
; Parameters: ST.L - File Address
;             ST.L - Buffer address
;             ST.W - # Blocks to write
;             ST.W - Block Number, -1 = Sequential
;
; Returns:    D0.W - # Blocks actually written
;

%_BLKWR
        CLR.W   D2              ; 0 = blockwrite
B_RD_WR MOVEM.L D3-D5/A2-A3,-(SP)
        MOVE.W  24(SP),D1       ; Block Number
        MOVE.W  26(SP),D4       ; # Blocks
        MOVE.L  28(SP),D3       ; Buffer Address
        MOVE.L  32(SP),A3       ; File Address
        CLR.W   IORES(A5)       ; Assume no errors 
        TST.W   D4              ; Negative number of blocks?
        BLT     NOBRW
        TST.W   D1              ; Negative block number?
        BGE.S   NOTSEQ
        MOVE.W  FIBNXTB(A3),D1  ; Sequential, set to FIBNXTB
NOTSEQ  MULS    #512,D1         ; Get byte position for transfer
; Seek to position for transfer
        MOVEM.L D1-D7/A0-A6,-(SP)
        CLR.L   -(SP)           ; Seek sense is absolute
        MOVE.L  D1,-(SP)        ; Position
        MOVE.L  FIBFD(A3),-(SP) ; File descriptor
        LOWER
        JSR     _lseek
        UPPER
        ADDA.W  #12,SP          ; Discard parameters
        MOVEM.L (SP)+,D1-D7/A0-A6
        TST.L   D0              ; Seek successful?
        BGE.S   BSKOK
        MOVE.W  #17,IORES(A5)
        BRA.S   NOBRW
BSKOK   MOVEM.L D1-D7/A0-A6,-(SP)
        MULS    #512,D4         ; Number of characters to transfer
        MOVE.L  D4,-(SP)        ; _write char count
        MOVE.L  D3,-(SP)        ; Pointer to buffer
        MOVE.L  FIBFD(A3),-(SP) ; Push file descriptor
        MOVE.L  D3,A1           ; Need pointer to buf in A reg for read
        TST.W   D2              ; Read or write?
        BNE.S   READIT
        LOWER
        JSR     _write          ; Call Idris through wrapper
        UPPER
        BRA.S   RWDN
READIT  TST.L   D4              ; Is the read > zero length?
        BLE.S   CLEARD          ; If not, don't clear anything
CLRLP   CLR.L   (A1)+           ; Clear 4 bytes of buffer
        SUBQ.L  #4,D4           ; Decrement count of bytes to clear
        BGT.S   CLRLP           ; More?, if so loop
CLEARD                          ; Want label in upper case
        LOWER
        JSR     _read
        UPPER
RWDN    ADDA.W  #12,SP          ; Discard parameters
        MOVEM.L (SP)+,D1-D7/A0-A6
        MOVE.L  D0,D4           ; Get character count
        BGE.S   NOBRWE
        MOVE.W  #3,IORES(A5)
        BRA.S   NOBRW
NOBRWE  ADD.L   #511,D4         ; Round up
        DIVS    #512,D4         ; Compute number of blocks read
        MOVE.W  D4,D0           ; Return number of blocks read
        ADD.W   FIBNXTB(A3),D4  ; Compute new next block
        MOVE.W  D4,FIBNXTB(A3)  ; Put it back into fib
        BRA.S   BRWDNE
NOBRW   CLR.W   D0              ; No blocks read or written
BRWDNE  MOVEM.L (SP)+,D3-D5/A2-A3
        MOVE.L  (SP)+,A0                ; Return address
        ADDA.W  #12,SP                  ; Discard parameters
        JMP     (A0)                    ; Return
        
;
; %_GET - Get
;
; %_PUT - Put
;
; Parameters: ST.L - File Address
;

%_GET
        MOVE.L  (SP)+,A1        ; Get return address
        MOVE.L  (SP)+,A0        ; Parameter to LGET in A0
        JSR     %_RDIT          ; Have to be in read state to do LGET
        JSR     %_LGET          ; Perform the get
        JMP     (A1)
        
%_PUT
        MOVE.L  (SP)+,A1                ; Get return address
        MOVE.L  (SP)+,A0                ; File Descriptor
        MOVE.L  A1,-(SP)                ; Set up return address for RTS
        MOVE.W  #0,IORES(A5)            ; Assume no errors
        JSR     %_WRTIT                 ; Set file up for writing
        MOVEA.L (A0),A1                 ; A1 points to the window
        MOVE.W  FIBRSIZ(A0),D1          ; Get hold of record size
PUTLP   TST.W   D1                      ; More bytes to put?
        BLE.S   PUTDN
        MOVE.B  (A1)+,D0                ; Place byte for putch
        JSR     %_PUTCH                 ; Put it into file
        SUBQ.W  #1,D1                   ; Loop counter
        BRA.S   PUTLP
PUTDN   TST.B   BUFFED(A0)      ; Is file buffered?
        BNE.S   FISBUF          ; Branch around flush if buffered
        JSR     %_FLUSH
FISBUF  RTS

;
; %_UPARR - Computes the address of F^
;
; Parameters: ST.L - Address of file
;
; Returns:    A0.L - Address of F^
;
; Scratches only A0
;

%_UPARR
        MOVE.L  4(SP),A0        ; Get address of file
        CMPI.B  #F_IVALID,FIBSTAT(A0) ; Does state = FIVALID?
        BEQ.S   UNOGET          ; Yes. No need to call GET
        CMPI.B  #F_TVALID,FIBSTAT(A0) ; Does state = FTVALID?
        BEQ.S   UNOGET          ; Yes. No need to call GET
        JSR     %_RDIT          ; Have to be in read state to do LGET
        JSR     %_LGET          ;
UNOGET  MOVE.L  (A0),A0         ; Get address of f^
        MOVE.L  (SP)+,(SP)
        RTS
        
;
; %_SEEK - Seek
;
; Parameters: ST.L - Address of file
;             ST.L - Record number to seek
;

%_SEEK
        MOVE.L  (SP)+,A1                ; Get return address
        MOVE.L  (SP)+,D1                ; Record number
        MOVE.L  (SP)+,A0                ; File address
        MOVE.L  A1,-(SP)                ; Set up return address for RTS
        CLR.W   IORES(A5)               ; Assume no errors 
        TST.B   BUFFED(A0)              ; Is the file buffered?
        BEQ.S   NOSKFL                  ; If not buffered, don't flush
        CMPI.B  #BWRING,BSTATE(A0)      ; Is the buffered file set up for writing
        BNE.S   NOSKFL                  ; If not writing, don't flush
        JSR     %_FLUSH                 ; Flush it
NOSKFL  MOVE.B  #BEMPTY,BSTATE(A0)      ; After seek, fib buffer is empty
        MOVE.L  D1,-(SP)                ; Multiply record number
        MOVE.W  FIBRSIZ(A0),D1          ; Record size, need it long
        EXT.L   D1
        MOVE.L  D1,-(SP)                ; By record size
        JSR     %I_MUL4                 ; Get result on stack
        MOVE.L  (SP)+,D1                ; True seek position now in D1
        BLT.S   SKBAD
        MOVEM.L D1-D7/A0-A6,-(SP)
        CLR.L   -(SP)                   ; Seek sense is absolute
        MOVE.L  D1,-(SP)                ; Position
        MOVE.L  FIBFD(A0),-(SP)         ; File descriptor
        LOWER
        JSR     _lseek
        UPPER
        ADDA.W  #12,SP                  ; Discard parameters
        MOVEM.L (SP)+,D1-D7/A0-A6
        TST.L   D0                      ; Successful?
        BGE.S   SKOK
SKBAD   MOVE.W  #17,IORES(A5)
SKOK    RTS
        
;
; %_UREAD - Unitread
;
; %_UWRIT - Unitwrite
;
; Parameters: ST.W - Unit number
;             ST.L - Buffer address
;             ST.L - # Bytes to read
;             ST.L - Block Number, 0 = Sequential
;             ST.W - Mode
;

;
; %_UCLR - Unit Clear
;
; Parameters: ST.W - Unit to clear
;

;
; %_UBUSY - Unit Busy
;
; Parameters: ST.W - Unit to check
;
; Returns:    ST.B - Boolean Result
;

%_UREAD
%_UWRIT
%_UCLR
%_UBUSY
        LEA     MESS,A0         ;
        MOVE.W  #24,D0          ; D0 = Message length
        JSR     %_PANIC
        JSR     %_HALTF
        
MESS    DATA.B  'No unit I/O under UNIX ',$0a
        
        END

;
; Isolate %_CLOSE and %_IORES so that null FORTRAN program is 2k smaller
;
        
        IDENT MERIO2
        
		GLOBAL  %_TRUNC
        GLOBAL  %_CLOSE,%_IORES
        EXTERN  %_FLUSH
		EXTERN  %_FTRUNC
        LOWER
        EXTERN  _close,_unlink
		EXTERN  _lseek
        UPPER

;
; FIB record offsets and scalar definitions
;

BEMPTY  EQU     0               ; Buffer empty
BRDING  EQU     1               ; Buffer set up for reading
BWRING  EQU     2               ; Buffer set up for writing

FIBEOL  EQU     4
FIBEOF  EQU     5
FIBTEXT EQU     6
FIBSTAT EQU     7
FIBRSIZ EQU     8
FIBNXTB EQU     10
FIBFD   EQU     12
BUFFED  EQU     16
BSTATE  EQU     17
BNXTBY  EQU     18
BLSTBY  EQU     22
FILNAM  EQU     26
BUFFER  EQU     90
FIBUPAR EQU     602

IORES   EQU     26

;
; %_CLOSE - Close a file
;
; Parameters: ST.L - File Address
;             ST.W - Mode: 0=Normal, 1=Lock, 2=Purge
;

%_CLOSE MOVE.L  (SP)+,A1        ; Pop return address
        MOVE.W  (SP)+,D2        ; Pop Close Mode
        MOVE.L  (SP)+,A0        ; Pop file address
        MOVE.L  A1,-(SP)        ; Push return address so can RTS
        CLR.W   IORES(A5)       ; Assume no errors 
        TST.B   BUFFED(A0)      ; Is the file buffered?
        BEQ.S   NOFLUSH         ; If not buffered, don't flush
        CMPI.B  #BWRING,BSTATE(A0) ; Is the buffered file set up for writing
        BNE.S   NOFLUSH         ; If not writing, don't flush
        JSR     %_FLUSH         ; Flush it
NOFLUSH MOVE.B  #1,FIBEOL(A0)
        MOVE.B  #1,FIBEOF(A0)
        MOVEM.L D1-D7/A0-A6,-(SP)
        MOVE.L  FIBFD(A0),-(SP) ; Push file descriptor
        LOWER
        JSR     _close          ; Close it
        UPPER
        ADDQ.W  #4,SP           ; Discard parameters
        MOVEM.L (SP)+,D1-D7/A0-A6
        TST.L   D0              ; Succeed?
        BGE.S   CLDNE
        MOVE.W  #3,IORES(A5)    ; Close error 
        RTS
CLDNE   CMPI.W  #2,D2           ; Purge
        BNE.S   CLKEEP
        TST.B   FILNAM(A0)      ; Is there a file name there?
        BEQ.S   CLKEEP          ; If not, don't try and unlink it
        MOVEM.L D1-D7/A0-A6,-(SP)
        PEA     FILNAM(A0)      ; Push file name
        LOWER
        JSR     _unlink         ; Try and purge it
        UPPER
        ADDQ.W  #4,SP           ; Discard parameter
        MOVEM.L (SP)+,D1-D7/A0-A6
CLKEEP  RTS

;
; %_TRUNC - Trucate a file
;
; Parameters: ST.L - File Address
;

%_TRUNC MOVE.L  (SP)+,A1        ; Pop return address
        MOVE.L  (SP)+,A0        ; Pop file address
        MOVE.L  A1,-(SP)        ; Push return address so can RTS
        CLR.W   IORES(A5)       ; Assume no errors 
        TST.B   BUFFED(A0)      ; Is the file buffered?
        BEQ.S   DONTFL         ; If not buffered, don't flush
        CMPI.B  #BWRING,BSTATE(A0) ; Is the buffered file set up for writing
        BNE.S   DONTFL        ; If not writing, don't flush
        JSR     %_FLUSH         ; Flush it
;
; the following lines were in the %_CLOSE version of this.  I dont
; believe that we want them here.
;
;DONTFL MOVE.B  #1,FIBEOL(A0)
;        MOVE.B  #1,FIBEOF(A0)
;        MOVEM.L D1-D7/A0-A6,-(SP)
DONTFL MOVEM.L D1-D7/A0-A6,-(SP)
; file is flushed.  do an lseek(fd,0,1) to get length.
		MOVE.L  A0,-(SP)		; save copy of a0
		MOVE.L  #1,-(SP)
		MOVE.L  #0,-(SP)
        MOVE.L  FIBFD(A0),-(SP) ; Push file descriptor
        LOWER
        JSR     _lseek          ; Close it
        UPPER
        ADDA.W  #12,SP           ; Discard parameters
		MOVE.L 	(SP)+,A0
; return value is file length.  Push it for ftruncate.
		MOVE.L	D0,-(SP)
        MOVE.L  FIBFD(A0),-(SP) ; Push file descriptor
        JSR     %_FTRUNC          ; Close it
        ADDQ.W  #8,SP           ; Discard parameters
        MOVEM.L (SP)+,D1-D7/A0-A6
        TST.L   D0              ; Succeed?
        BGE.S   TRDNE
        MOVE.W  #3,IORES(A5)    ; Close error 
TRDNE   RTS
        
;
; %_IORES - Ioresult
;
; Parameters: None.
;
; Returns:    D0.W - Ioresult
;
; Scratches only D0
;

%_IORES
        MOVE.W  IORES(A5),D0    ; Fetch IORESULT
        RTS

        END

