; File: ufio.text
; Date: 16-Jun-85
;

        upper
        ident   fio
        
        global  %_fwrite,%_fread,%_freadn,%_fseek
        extern  %_putch,%_getch,%_wrtit,%_rdit,%_flush,%_halt
        
        lower
        extern  _write,_lseek
		global  fhalt
        upper

;
; FIB record offsets and scalar definitions
;

IORES   EQU     26

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
BUFFER  EQU     90

;
; %_fwrite - Put a sequence of bytes into a file
;
; Parameters: ST.W - Count of characters to write
;             ST.L - Pointer to buffer containing bytes to write
;             ST.L - Pointer to fib for file to write to
;
; Sets ioresult on write error.
;

%_fwrite 
        move.l  (sp)+,d0                ; Return address
        move.l  (sp)+,a0                ; Get pointer to fib in a0
        move.l  (sp)+,a1                ; Get pointer to user buffer
        move.w  (sp)+,d1                ; Character count
        move.l  d0,-(sp)                ; Restore return address for rts
        move.l  a2,-(sp)                ; Preserve a2
        move.w  #0,IORES(a5)            ; Clear IORESULT before doing I/O
        jsr     %_wrtit                 ; Set file up for writing
; if count < 512 or buffer not empty
; then copy bytes into buffer until either buffer full or count = 0
        cmpi.w  #512,d1                 ; Short output called for?
        blt.s   copyhd                  ; Yup, copy some bytes
        lea     buffer(a0),a2           ; Point to buffer
        cmpa.l  bnxtby(a0),a2           ; Is nxtby pointing to buf start
        bne.s   copyhd                  ; If not, copy some bytes
        bra.s   hddone                  ; Branch if no need for initial copy
copyhd  move.l  bnxtby(a0),a2           ; Point to next hole in buffer
copytp  tst.w   d1                      ; Count = 0?
        beq.s   cpy2dn                  ; If so, don't copy more, quick exit
        cmpa.l  blstby(a0),a2           ; Does it point to beyond buffer?
        beq.s   copydn                  ; If so, don't copy more
        move.b  (a1)+,(a2)+             ; Copy a byte
        subq.w  #1,d1
        bra.s   copytp
copydn  move.l  a2,bnxtby(a0)           ; Update bnxtby after copy
; if buffer full then flushbuffer
hddone  move.l  bnxtby(a0),a2           ; Point to next hole in buffer
        cmpa.l  blstby(a0),a2           ; Does it point to beyond buffer?
        bne.s   nohdfl                  ; If not, no need to flush
        jsr     %_flush                 ; Full buffer, flushit
        jsr     %_wrtit                 ; Set file back up for writing
; Either count = 0 or buffer is empty, and we are free to do direct write
nohdfl  move.w  d1,d2                   ; Need to find count of full blocks
        andi.w  #$fe00,d2               ; Bytes of fullblocks
        beq.s   noblks                  ; Count 0, no fullblocks
        move.l  a1,d0                   ; check that a1 not odd
        andi.w  #1,d0                   ; if so, better to copy into buffer
        bne.s   copyhd                  ; which at least makes alignment ok
        ext.l   d2
        movem.l d1-d7/a0-a6,-(sp)       ; Be pure around _write
        move.l  d2,-(sp)                ; Push byte count for _write
        move.l  a1,-(sp)                ; Push address of buffer
        move.l  fibfd(a0),-(sp)         ; Push file descriptor
        lower
        jsr     _write                  ; Call UNIX through wrapper
        upper
        adda.w  #12,sp                  ; Discard parameters of _write
        movem.l (sp)+,d1-d7/a0-a6       ; Restore regs
        cmp.l   d0,d2                   ; Number of bytes written
        beq.s   wrok
        move.w  #3,iores(a5)
wrok    adda.l  d2,a1                   ; Advance pointer by bytes written
        sub.w   d2,d1                   ; D1 is number of bytes left
; It is still true that either the buffer is empty or the count = 0
; Additionally, the remaining count in d1 is < 512
; Place the remaining bytes into the buffer.  They will fit.
noblks  move.l  bnxtby(a0),a2           ; Point to next hole in buffer
copybt  tst.w   d1                      ; Count = 0?
        beq.s   cpy2dn                  ; If so, don't copy more
        move.b  (a1)+,(a2)+             ; Copy a byte
        subq.w  #1,d1
        bra.s   copybt
cpy2dn  move.l  a2,bnxtby(a0)           ; Update bnxtby after copy
        tst.b   buffed(a0)              ; Is file buffered?
        bne.s   fisbuf                  ; Branch around flush if buffered
        jsr     %_flush
fisbuf  move.l  (sp)+,a2                ; Restore a2
        rts
        
;
; %_fread - Read a byte from a file
;
; Parameters: ST.L - Pointer to buffer to read into
;             ST.L - Pointer to fib for file to read from
;             ST.L - Pointer to 2 byte integer, 0 if no bytes read, 1 otherwise
;
; Sets ioresult on read error. Zero return value in third parameter
; implies end of file.
;

%_fread 
        move.l  (sp)+,d0                ; Return address
        move.l  (sp)+,d1                ; Pointer to result count
        move.l  (sp)+,a0                ; Pointer to fib
        move.l  (sp)+,a1                ; Pointer to buffer
        move.l  d0,-(sp)                ; Return address for rts
        move.l  a3,-(sp)                ; Need a free A register
        move.l  d1,a3                   ; Pointers go in A registers
        move.w  #1,(a3)                 ; Assume read successful
        move.w  #0,IORES(a5)            ; Clear IORESULT before doing I/O
        jsr     %_rdit                  ; Set file up for reading
        jsr     %_getch                 ; Load up D0.B with next char
        move.b  d0,(a1)                 ; Stuff character into buffer
        tst.b   FIBEOF(a0)              ; Error or end of file?
        beq.s   frdone
        clr.w   (a3)                    ; Nothing read
frdone  move.l  (sp)+,a3                ; Restore old a3
        rts
        
;
; %_freadn - Read n bytes from a file
;
; Parameters: ST.L - Pointer to buffer to read into
;             ST.L - Pointer to fib for file to read from
;             ST.L - 4 byte integer, count requested
;
; Sets ioresult on read error.  Return value is count actually read
;

%_freadn
        move.l  (sp)+,d0                ; Return address
        move.l  (sp)+,d1                ; Count requested
        move.l  (sp)+,a0                ; Pointer to fib
        move.l  (sp)+,a1                ; Pointer to buffer
        move.l  d0,-(sp)                ; Return address for rts
        move.l  a2,-(sp)                ; Need a scratch A register
        move.w  #0,IORES(a5)            ; Clear IORESULT before doing I/O
        jsr     %_rdit                  ; Set file up for reading
        moveq.l #-1,d2                  ; Counter of bytes read
frmore  addq.l  #1,d2                   ; Count characters read
        cmp.l   d1,d2                   ; Read enough?
        bge.s   frndone
; INLINE jsr %_getch
        clr.b   FIBEOF(a0)              ; Not eof unless discovered
        move.l  bnxtby(a0),a2           ; Get address of source for byte
        cmpa.l  blstby(a0),a2           ; Does it point beyond blstby?
        ble.s   chinbf                  ; Branch if no need to fill buffer
; Must go to the system for more characters
        jsr     %_getch                 ; Do it the general way
        bra.s   d0set
chinbf  move.b  (a2)+,d0                ; Return the character
        move.l  a2,bnxtby(a0)           ; Update bnxtby
; End of INLINE
d0set   move.b  d0,(a1)+                ; Stuff character into buffer
        tst.b   FIBEOF(a0)              ; Error or end of file?
        beq.s   frmore
frndone move.l  d2,d0                   ; Return count actually read
        move.l  (sp)+,a2                ; Restore scratch register
        rts
        
;
; %_FSEEK - Seek to a position given by a longint
;
; Parameters: ST.L - Address of file
;             ST.L - Byte position to seek
;             ST.L - 0 = SEEK_ABSOLUTE, 1 = SEEK_RELATIVE
;

%_FSEEK
        MOVE.L  (SP)+,A1                ; Get return address
        MOVE.L  (SP)+,D1                ; Seek sense
        MOVE.L  (SP)+,D2                ; Byte position
        MOVE.L  (SP)+,A0                ; File address
        MOVE.L  A1,-(SP)                ; Set up return address for RTS
        CLR.W   IORES(A5)               ; Assume no errors 
        CMPI.B  #BRDING,BSTATE(A0)      ; Reading?
        BNE.S   NOTRD
        JSR     %_WRTIT
        BRA.S   NOSKFL
NOTRD   TST.B   BUFFED(A0)              ; Is the file buffered?
        BEQ.S   NOSKFL                  ; If not buffered, don't flush
        CMPI.B  #BWRING,BSTATE(A0)      ; Is the buffered file set up for writing
        BNE.S   NOSKFL                  ; If not writing, don't flush
        JSR     %_FLUSH                 ; Flush it
NOSKFL  MOVE.B  #BEMPTY,BSTATE(A0)      ; After seek, fib buffer is empty
        MOVEM.L D1-D7/A0-A6,-(SP)
        MOVE.L  D1,-(SP)                ; Push seek sense
        MOVE.L  D2,-(SP)                ; Position
        MOVE.L  FIBFD(A0),-(SP)         ; File descriptor
        LOWER
        JSR     _lseek
        UPPER
        ADDA.W  #12,SP                  ; Discard parameters
        MOVEM.L (SP)+,D1-D7/A0-A6
        TST.L   D0                      ; Successful?
        BGE.S   SKOK
        MOVE.W  #17,IORES(A5)
SKOK    RTS
        
;
; fhalt - Set the halt error flag from FORTRAN
;
; Parameters: ST.L - Pointer to longint error code
;

        LOWER
fhalt   move.l  (sp)+,a1                ; Discard return address
        UPPER
        move.l  (sp)+,a1                ; Pointer to longint error code
        move.l  (a1),d1                 ; Grab error code
        move.w  d1,-(sp)                ; Push it as a word
        jsr     %_halt                  ; Never to return
        
        end

