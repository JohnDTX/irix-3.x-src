; File: guinit.text
; Date: 26-Sep-84
;

        IDENT   IDINIT
        
        GLOBAL  %P830701,%_END,%_TERM,%_VERS

;
; Fib for IDRIS 
;
; fib = record
;        fwindow: pbytes;                                (* 0 *)
;        FEOLN: Boolean;                                 (* 4 *)
;        FEOF: Boolean;                                  (* 5 *)
;        FTEXT: Boolean;                                 (* 6 *)
;        fstate: (FTVALID, FIEMPTY, FIVALID, FTEMPTY);   (* 7 *)
;        frecsize: integer;                              (* 8 *)
;        fnextblock: integer;                            (* 10 *)
;        fd: longint;                                    (* 12 *)
;        buffed: Boolean;                                (* 16 *)
;        bstate: (BEMPTY,BRDING,BWRING);                 (* 17 *)
;        bnxtby: pbytes;                                 (* 18 *)
;        blstby: pbytes;                                 (* 22 *)
;        filnam: pac64; {Zero Terminated}                (* 26 *)
;        buffer: array[0..511] of byte                   (* 90 *)
;      end;
;
; The fib is followed by the window at offset 602
;

;
; FIB record offsets and scalar definitions
;

STDIN   EQU     0
STDOUT  EQU     1
STDERR  EQU     2

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

;
; %P830701 - The initial routine when executing under IDRIS
;
; Upon entry, IDRIS argc and argv are on the stack.  %P830701
; must create fibs for input, output, and stderr and translate IDRIS
; format argc and argv into the format expected by Pascal;
;
; Upon exit from %P830701 the stack will be:
;
; |-----------|
; |  Pascal   |
; | arg area  |                       1868
; |-----------|
; | Breakaddr |                       1864
; |Stderr FIB |                       1252
; | Input FIB |                        640
; |Output FIB |                         28
; |  ^Heap    |                         24
; | Ioresult  |  Integer                22
; | ^ stderr  |                         18
; |   Argc    |  Pascal format          16
; |   Argv    |  Pascal format          12
; | ^ output  |                         8
; |  ^ input  |                         4
; | Ret. Addr | <-- SP                  0
; |-----------|
;

%P830701
; Set D0 to number of bytes needed for argc/argv copy
        MOVE.L  8(SP),D1        ; System argc
        MOVE.L  12(SP),A0       ; System argv
        MOVE.L  D1,D0           ; D0 is count of bytes needed
        ASL.L   #2,D0           ; Need room for 4*argc for argv array
NXTARG  SUBQ.L  #1,D1           ; Next argument
        BLT.S   CNTDNE
        MOVE.L  (A0)+,A1        ; Point to next argument
CMORE   ADDQ.L  #1,D0           ; Count character in argument
        TST.B   (A1)+           ; Found end of string?
        BNE.S   CMORE           ; Zero byte terminates
        ADDQ.L  #1,D0
        ANDI.L  #$FFFFFFFE,D0   ; Round up odd D0
        BRA.S   NXTARG          ; More arguments?
; D0 has necessary count of bytes needed for argc/argv copy
; Now set D2 to argc and A1 to argv for later use.
CNTDNE  MOVE.L  8(SP),D2        ; System argc
        MOVE.L  12(SP),A1       ; System argv
; Adjust stack to new positions, preserving D0, D2, A1
        MOVE.L  D7,D4           ; Save commonsize in non clobbered reg
        MOVE.L  (SP)+,D3        ; Pop return address for %P830701
        MOVE.L  (SP)+,A0        ; Pop return address to IDRIS
        SUBA.L  D0,SP           ; Room for argc/argv args
        SUBA.L  #1864,SP        ; Room for 3 fibs, 6 ptrs, 2 ints
        MOVE.L  SP,D0           ; Force SP to divisible by 4
        ANDI.L  #$FFFFFFFC,D0   ; Maybe noop, maybe subtract 2
        MOVE.L  D0,SP
        MOVE.L  A0,-(SP)        ; Restore return address to IDRIS
; The stack is now set up exactly as it will be upon exit
; Use D2 and A1 as set above
        MOVE.W  D2,16(SP)       ; Initialize Pascal argc
        LEA     1868(SP),A2     ; Pascal Argv
        MOVE.L  A2,12(SP)       ; Initialize Pascal argv
        MOVE.L  A2,A3           ; Calculate loc of Pascal string args
        ASL.L   #2,D2           ; Need room for 4*argc for argv array
        ADDA.L  D2,A3           ; A3 is the loc of Pascal string args
BLOOP   MOVE.L  A3,D0           ; Need a scratch register
        ANDI.L  #1,D0           ; Check the last bit, see if A3 is odd
        BEQ.S   A3EVEN          ; Bit off, A3 is even
        ADDQ.L  #1,A3           ; Can't have an odd target
A3EVEN  TST.L   D2              ; Done with all the arguments?
        BLE.S   BDONE           ; Branch on count <= 0
        SUBQ.L  #4,D2           ; 4 at a crack since we multiplied above
        MOVE.L  A3,(A2)+        ; Set next Pascal argv
        MOVE.L  (A1),A0         ; Get pointer to source string
        CLR.L   D0              ; Length counter
BMORE   TST.B   (A0)+           ; Found end of string?
        BEQ.S   NOMORE          ; Zero byte terminates
        ADDQ.L  #1,D0           ; Counter
        BRA.S   BMORE
NOMORE  MOVE.B  D0,(A3)+        ; Stuff length into target string
        MOVE.L  (A1)+,A0        ; Pointer to start of src string
BMORE2  TST.L   D0              ; More characters left
        BEQ.S   NOMORE2         ; Ran out?
        SUBQ.L  #1,D0           ; Decrement counter
        MOVE.B  (A0)+,(A3)+     ; Copy a character to dst string
        BRA.S   BMORE2
NOMORE2 BRA.S   BLOOP
; Argc and Argv converted
BDONE   CLR.W   22(SP)          ; Clear ioresult
; Initialize pointer to input
        LEA     640(SP),A1      ; Get address of input
        MOVE.L  A1,4(SP)        ; Put address of input into place
; Initialize input fib
        LEA     FIBUPAR(A1),A2  ; Compute address of window
        MOVE.L  A2,(A1)         ; Initialize fwindow
        CLR.B   FIBEOL(A1)
        CLR.B   FIBEOF(A1)
        MOVE.B  #1,FIBTEXT(A1)
        MOVE.B  #F_IEMPTY,FIBSTAT(A1)
        MOVE.W  #1,FIBRSIZ(A1)
        CLR.W   FIBNXTB(A1)
        MOVE.L  #STDIN,FIBFD(A1); FD STDIN is standard input
        MOVE.B  #1,BUFFED(A1)   ; Input can always be buffered
        MOVE.B  #BEMPTY,BSTATE(A1) ;Buffering state is BEMPTY
        CLR.B   FILNAM(A1)      ; No name for close purge
; Initialize pointer to output
        LEA     28(SP),A1       ; Get address of output
        MOVE.L  A1,8(SP)        ; Put address of output into place
; Initialize output fib
        LEA     FIBUPAR(A1),A2  ; Compute address of window
        MOVE.L  A2,(A1)         ; Initialize fwindow
        CLR.B   FIBEOL(A1)
        CLR.B   FIBEOF(A1)
        MOVE.B  #1,FIBTEXT(A1)
        MOVE.B  #F_IVALID,FIBSTAT(A1)
        MOVE.W  #1,FIBRSIZ(A1)
        CLR.W   FIBNXTB(A1)
        MOVE.L  #STDOUT,FIBFD(A1) ; FD STDOUT is standard output
        CLR.B   BUFFED(A1)      ; Standard output is not buffered
        MOVE.B  #BEMPTY,BSTATE(A1) ;Buffering state is BEMPTY
        CLR.B   FILNAM(A1)      ; No name for close purge
; Initialize pointer to stderr
        LEA     1252(SP),A1     ; Get address of stderr
        MOVE.L  A1,18(SP)       ; Put address of stderr into place
; Initialize stderr fib
        LEA     FIBUPAR(A1),A2  ; Compute address of window
        MOVE.L  A2,(A1)         ; Initialize fwindow
        CLR.B   FIBEOL(A1)
        CLR.B   FIBEOF(A1)
        MOVE.B  #1,FIBTEXT(A1)
        MOVE.B  #F_IVALID,FIBSTAT(A1)
        MOVE.W  #1,FIBRSIZ(A1)
        CLR.W   FIBNXTB(A1)
        MOVE.L  #STDERR,FIBFD(A1) ; FD STDERR is stderr
        CLR.B   BUFFED(A1)      ; Standard stderr is not buffered
        MOVE.B  #BEMPTY,BSTATE(A1) ;Buffering state is BEMPTY
        CLR.B   FILNAM(A1)      ; No name for close purge
        MOVE.L  D3,A3           ; Get return address in A register
        MOVE.L  D4,D7           ; Common size
        EXT.L   D7              ; Need extended to long for now
        JMP     (A3)
;
; Once the main program has done a:
;
;       LINK    #xxx,A5
;
; the stack will be:
;
; |-----------|
; | Breakaddr |                  +1868
; |Stderr FIB |                  +1256
; | Input FIB |                   +644
; |Output FIB |                    +32
; |  ^Heap    |                    +28
; | Ioresult  |  Integer           +26
; | ^ stderr  |                    +22
; |   Argc    |  Pascal format     +20
; |   Argv    |  Pascal format     +16
; | ^ Output  |                    +12
; |  ^ Input  |                     +8
; | Ret. Addr |                     +4
; |  Old A5   | <-- A5
; |-----------|
; |    ...    |
; |  Globals  |
; |    ...    |
; |-----------| <-- SP
;

;
; %_TERM - Next to last routine
;

%_TERM
        RTS

;
; %_END - The Final Routine
;

%_END
        CLR.L   D0              ; Return no error from program
        RTS

;
; The resulting stack is:
;
; |-----------|
; |   Argc    |
; |   Argv    |
; | ^ output  |
; |  ^ input  |
; | Ret. Addr | <-- SP
; |-----------|
;
        
;
; %_VERS - Return Operating System Version Number
;

%_VERS
        MOVE.W   #2,D0          ; 2 = UNISOFT
        RTS
        
        END

