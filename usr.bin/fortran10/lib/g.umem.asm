; File: gumem.text
; Date: 02-Jun-84

        IDENT   IDMEM
        
        GLOBAL  %_NEW,%_NEW4
        EXTERN  _malloc
        EXTERN  %_PANIC,%_HALTF,%_PRLOC
        
FREEMEM EQU     28              ; FREEMEM(A5) is pointer to heap top
BRKADDR EQU   1868              ; BRKADDR(A5) is break address

;
; %_NEW - Allocate memory in the heap
;
; Parameters:  ST.W - Number of bytes needed
;
;
; %_NEW4 - Allocate memory in the heap
;
; Parameters:  ST.L - Number of bytes needed
;
; Scratches D0,D1,D2,A0,A1
;

%_NEW
        MOVE.L  (SP)+,A1        ; Return address
        MOVE.W  (SP)+,D0        ; Number of bytes
        EXT.L   D0              ; Make it a long
        MOVE.L  D0,-(SP)        ; Push it for call on %_NEW4
        MOVE.L  A1,-(SP)        ; Replace return address, fall through to %_NEW4

%_NEW4
        MOVE.L  4(SP),D0        ; Number of bytes
        MOVE.L  (SP)+,(SP)      ; Position return address
        MOVE.L  D0,D2           ; Need a copy to destroy
        ANDI.L  #1,D2           ; Is it odd
        BEQ.S   EVEN
        ADDQ.L  #1,D0           ; Round odd up
EVEN    MOVE.L  D0,-(SP)
        JSR     _malloc
        ADDQ.W  #4,SP
        RTS
        END
        
        IDENT   IDMEM2
        
        GLOBAL  %_MEMAV,%_DISP
        GLOBAL  %_DISP4
        EXTERN  _free
        
;
; %_MEMAV - Memory Available in the heap
;
; Parameters:  None.
;
; Results:     D0.L - #bytes available
;
; Scratches: Only D0
;

%_MEMAV
        MOVE.L  #30000,D0       ; Any large number is an ok return
        RTS

;
; %_DISP - Dispose
;
; Parameters:  ST.L - Address of pointer
;              ST.W - Number of bytes to free
;

%_DISP
        MOVE.L  6(SP),A0        ; Address of pointer
        MOVE.L  (A0),-(SP)      ; Push pointer
        JSR     _free
        ADDQ.L  #4,SP           ; Discard parameter
        MOVE.L  (SP)+,A1        ; Return address
        MOVE.W  (SP)+,D0        ; Number of bytes
        MOVE.L  (SP)+,A0        ; Address of pointer
        CLR.L   (A0)            ; Set the pointer to NIL
        JMP     (A1)
        
;
; %_DISP4 - Dispose
;
; Parameters:  ST.L - Address of pointer
;              ST.L - Number of bytes to free
;

%_DISP4
        MOVE.L  8(SP),A0        ; Address of pointer
        MOVE.L  (A0),-(SP)      ; Push pointer
        JSR     _free
        ADDQ.L  #4,SP           ; Discard parameter
        MOVE.L  (SP)+,A1        ; Return address
        MOVE.L  (SP)+,D0        ; Number of bytes
        MOVE.L  (SP)+,A0        ; Address of pointer
        CLR.L   (A0)            ; Set the pointer to NIL
        JMP     (A1)
        
        END

