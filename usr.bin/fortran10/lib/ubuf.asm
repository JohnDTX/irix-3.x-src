; File: ubuf.text
; Date: 08-Jul-85
;

        upper
        ident   idbuf
        
        global  %_flush,%_putch,%_getch,%_wrtit,%_rdit
        
        lower
        global  %%read,%%write,%%lseek
        extern  _write,_lseek,_read
        upper

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
FIBUPAR EQU     602

;
; %%read, %%write - entry points for "C" externals, underscore independent
;

        lower
%%read  jmp     _read
%%write jmp     _write
%%lseek jmp     _lseek
        upper

;
; %_flush - Empty the buffer in the referenced fib
;
; Parameters:  A0.L - Address of the fib 
;
; File should be in bstate BWRING when called.
; Saves all registers.  Sets bstate to BEMPTY.
;

%_flush movem.l d0-d7/a0-a6,-(sp)
        move.b  #BEMPTY,bstate(a0)      ; Set fib state to BEMPTY
        lea     buffer(a0),a1           ; Address of buffer
        move.l  bnxtby(a0),d4           ; Address of nextbyte in buffer
        sub.l   a1,d4                   ; Number of bytes in buffer
        movem.l d4/a5,-(sp)             ; Regs need after _write
        move.l  d4,-(sp)                ; Push byte count for _write
        move.l  a1,-(sp)                ; Push address of buffer
        move.l  fibfd(a0),-(sp)         ; Push file descriptor
        lower
        jsr     _write                  ; Call Idris through wrapper
        upper
        adda.w  #12,sp                  ; Discard parameters of _write
        movem.l (sp)+,d4/a5             ; Regs restore
        cmp.l   d0,d4                   ; Number of bytes written
        beq.s   flushok
        move.w  #3,iores(a5)            ; Ioresult error
flushok movem.l (sp)+,d0-d7/a0-a6
        rts
        
;
; %_putch - Place the generated character into the file buffer
;
; Parameters:  A0.L - Address of the fib 
;              D0.B - Byte to place into the buffer
;
; File should be in bstate BWRING when called.
; Saves all registers.  
;

%_putch move.l  a1,-(sp)                ; Need a scratch a register
        move.l  bnxtby(a0),a1           ; Get address of target for byte
        cmpa.l  blstby(a0),a1           ; Does it point to beyond buffer?
        bne.s   noflush                 ; No need to flush unless full
        jsr     %_flush
        jsr     %_wrtit                 ; Set fib state back to BWRING
        lea     buffer(a0),a1           ; Get address of buffer and target
noflush move.b  d0,(a1)+                ; Stuff character into target
        move.l  a1,bnxtby(a0)           ; Update bnxtby
        move.l  (sp)+,a1                ; Restore a1
        rts
        
;
; %_getch - Get the next input character and stuff it into D0.B
;
; Parameters:  A0.L - Address of the fib 
;
; Returns:     D0.B - Byte read from the file
;
; File should be in bstate BRDING when called.
; Saves all registers.  
;

%_getch move.l  a1,-(sp)                ; Need a scratch a register
        clr.b   FIBEOF(a0)              ; Not eof unless discovered
        move.l  bnxtby(a0),a1           ; Get address of source for byte
        cmpa.l  blstby(a0),a1           ; Does it point beyond blstby?
        ble.s   chinbf                  ; Branch if no need to fill buffer
; Must go to the system for more characters
        movem.l d0-d7/a0-a6,-(sp)       ; Need to be register pure around _read
        movem.l a0/a5,-(sp)             ; Want to restore these early
        move.l  #512,-(sp)              ; _READ char count
        pea     buffer(a0)              ; Pointer to inbuffer
        move.l  fibfd(a0),-(sp)         ; Push file descriptor
        lower
        jsr     _read                   ; Call Idris through wrapper
        upper
        adda.w   #12,sp                 ; Discard parameters to _read
        movem.l (sp)+,a0/a5             ; Restore them
        move.l  d0,d4                   ; Get count of characters read
        bgt.s   inok                    ; Characters read
        beq.s   noterr
        move.w  #13,iores(a5)           ; Read error
        clr.l   d4                      ; Make like no characters read
; End of file encountered, return EOF, EOL, and a blank
; Leave bstate BRDING, but such that EOF will be detected again and again
noterr  move.b  #1,FIBEOF(A0)           ; Set EOF
        move.b  #1,FIBEOL(A0)           ; Set EOL
        movem.l (sp)+,d0-d7/a0-a6       ; Restore registers
        move.l  (sp)+,a1                ; Restore a1
        move.b  #' ',d0                 ; Always blank on EOL
        rts
; Characters actually read, positive count in d4
inok    lea     buffer-1(a0),a1
        adda.l  d4,a1                   ; Computed address of blstby
        move.l  a1,blstby(a0)
        movem.l (sp)+,d0-d7/a0-a6       ; Restore registers
        lea     buffer(a0),a1           ; Computed address of bnxtby
chinbf  move.b  (a1)+,d0                ; Return the character;
        move.l  a1,bnxtby(a0)           ; Update bnxtby
        move.l  (sp)+,a1                ; Restore a1
        rts
        
;
; %_wrtit - Force the file into bstate BWRING
;
; Parameters:  A0.L - Address of the fib 
;
; File may be in any state upon call.
; Saves all registers.  
;

%_wrtit cmpi.b  #BWRING,bstate(a0)      ; Is file already in write state?
        bne.s   chngit                  ; If not, needs to be changed
        rts                             ; File already in write state, return
chngit  cmpi.b  #BRDING,bstate(a0)      ; Is file in reading state?
        bne.s   empty
; File is in reading state, need to seek back to turn it around
        movem.l d0-d7/a0-a6,-(sp)       ; Need to be register pure around _lseek
        move.l  #1,-(sp)                ; Seek sense is relative
        move.l  bnxtby(a0),d4
        sub.l   blstby(a0),d4           ; d4 is negative distance
        subq.l  #1,d4                   ; d4 was still one too big
        move.l  d4,-(sp)                ; Relative position
        move.l  fibfd(a0),-(sp)         ; File descriptor
        lower
        jsr     _lseek
        upper
        adda.w  #12,sp                  ; Discard parameters
        movem.l (sp)+,d0-d7/a0-a6       ; Restore registers
empty   move.b  #BWRING,bstate(a0)      ; Set fib state to BWRING
        pea     buffer(a0)              ; Calculate address of buffer
        move.l  (sp)+,bnxtby(a0)        ; Place proper address into bnxtby
        pea     buffer+512(a0)          ; Calculate address of buffer end
        move.l  (sp)+,blstby(a0)        ; Place it into blstby
        rts
        
;
; %_rdit - Force the file into bstate BRDING
;
; Parameters:  A0.L - Address of the fib 
;
; File may be in any state upon call.
; Saves all registers.  
;

%_rdit  cmpi.b  #BRDING,bstate(a0)      ; Is file already in read state?
        bne.s   rdchng                  ; If not, needs to be changed
        rts                             ; File already in write state, return
rdchng  cmpi.b  #BWRING,bstate(a0)      ; Is file in writing state?
        bne.s   rdempt
; File is in writing state, need to flush buffer before turning
        jsr     %_flush
rdempt  move.b  #BRDING,bstate(a0)      ; Set fib state to BRDING
        pea     buffer(a0)              ; Calculate address of buffer
        move.l  (sp)+,bnxtby(a0)        ; Place proper address into bnxtby
        pea     buffer-1(a0)            ; Calculate proper blstbyt
        move.l  (sp)+,blstby(a0)        ; Set up blstby
        rts
        
        end

