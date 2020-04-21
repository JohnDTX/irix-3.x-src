; File: ftnchar.text
; Date: 11-Mar-85

        ident   ftnchar
        
        global  %FC_ASS,%FC_TMP,%FC_SUB,%FC_SUB1,%FC_FIN
        global  %FC_CMP,%_ICHAR,%_CHAR,%_LEN,%_INDEX
        extern  %_FERROR

;
; %FC_ASS - Assign some characters to a FORTRAN character variable
;
; Parameters: ST.L - Address to put next character
;             ST.L - Number of bytes left unasigned
;             ST.L - Address of characters to copy
;             ST.L - Number of characters to copy
;
; Result:     ST.L - Address of next free byte in destination
;             ST.L - Number of bytes left unassigned
;
; This routine clobbers: D0,D1,D2,A0,A1
;

%FC_ASS
        move.l  (sp)+,d2        ; Pop return address
        move.l  (sp)+,d1        ; Length of source
        move.l  (sp)+,a1        ; Address of source
        move.l  (sp)+,d0        ; Room in destination
        move.l  (sp)+,a0        ; Address of destination
        cmp.l   d0,d1           ; Will this overflow dest?
        ble.s   as_room         ; No. Do it all.
        move.l  d0,d1           ; Yes. Only copy what fits
as_room sub.l   d1,d0           ; D0 = Final room left
        bra.s   as_test         ; D1 = count to copy
as_loop move.b  (a1)+,(a0)+     ; Copy the next byte
as_test subq.l  #1,d1           ; Any more to copy?
        bpl.s   as_loop         ; Yes.
        move.l  a0,-(sp)        ; No. Push results
        move.l  d0,-(sp)
        move.l  d2,a0
        jmp     (a0)
        
;
; %FC_TMP - Assign some characters to a FORTRAN character temporary
;           This routine assumes there is adaquate room to copy
;
; Parameters: ST.L - Address of first character of temporary
;             ST.L - Number of bytes used in temporary
;             ST.L - Address of characters to copy
;             ST.L - Number of characters to copy. Must be > 0.
;
; Result:     ST.L - Address of first character of temporary
;             ST.L - Number of bytes used in temporary
;
; This routine clobbers: D0,D1,D2,A0,A1
;

%FC_TMP
        move.l  (sp)+,d2        ; Pop return address
        move.l  (sp)+,d1        ; Length of source
        move.l  (sp)+,a1        ; Address of source
        move.l  (sp)+,d0        ; Length of destination
        move.l  (sp),a0         ; Address of temporary
        adda.l  d0,a0           ; First free hole
        add.l   d1,d0           ; D0 = Final size
        move.l  d0,-(sp)        ; Push final length
        subq.l  #1,d1           ; Get ready for DBRA
tm_loop move.b  (a1)+,(a0)+     ; Copy the next byte
        dbra    d1,tm_loop      ; Any more to copy?
        move.l  d2,a0           ; No. Return
        jmp     (a0)
        
;
; %FC_SUB - Compute and verify a FORTRAN character substring
;           For substrings as: VVV(EEE:EEE) or VVV(:EEE)
;
; Parameters: ST.L - Address of main string
;             ST.L - Size of main string
;             ST.W - Beginning substring position
;             ST.W - Ending substring position
;
; Result:     ST.L - Address of substring
;             ST.L - Size of substring
;
; This routine preserves all registers.
;

%FC_SUB
        movem.l d0-d2/a0-a1,-(sp)
        move.l  20(sp),a1       ; Pop return address
        move.w  24(sp),d2       ; End of substring
        ext.l   d2
			; is end > 0?
	bgt.s	end_ok		; yep.
	move.l	#1,d2		; nope.  move one to it.
end_ok
        move.w  26(sp),d1       ; Start of substring
        ext.l   d1
        bgt.s   start_ok          
		; min <= 0
		move.l	#1,d1			; set it to one.
start_ok
        move.l  28(sp),d0       ; Length of main string
        move.l  32(sp),a0       ; Address of string
;        cmp.l   d0,d2           ; Is max too big?
;        ble.s   maxok           ; Nope.
;	GB - 9/23/86. allow any max that the user specifies.
;		move.l	d0,d2			; (GB) make max = length
;
maxok
        sub.l   d1,d2           ; No. Is (length-1) > 0?
        bge.s   minltmax          ; Yes.
		move.l	28(sp),d1
		move.l	#0,d2			; (GB) set it to zero.
minltmax
        addq.l  #1,d2           ; No. Set D2 to size of substring
        lea     -1(a0,d1.l),a0  ; Compute substring address
        move.l  a0,32(sp)       ; and store it
        move.l  d2,28(sp)       ; Store substring length
        move.l  a1,24(sp)       ; Store return address
        movem.l (sp)+,d0-d2/a0-a1
        addq.w  #4,sp
        rts
;as_err  
;		d0 = length of parent string
;		d2 = (size of substring) - 1 
;		d1 = index to start of substring
;		a0 = address of string
;
;		These routines are called if:			and the result is:
;
;			the end index is too large.			the end index is set to max.
;			the length of the substr is <=0 	the length is set to one.
;			
;		move.l  a1,-(sp)        ; Pass return address for ..
;        move.w  #707,-(sp)      ; .. FORTRAN run time error #707
;        jsr     %_FERROR
		

;
; %FC_SUB1 - Compute and verify a FORTRAN character substring
;            Upper limit on substring is taken as string size
;            For substrings as: VVV(EEE:) or VVV(:)
;
; Parameters: ST.L - Address of main string
;             ST.L - Size of main string
;             ST.W - Beginning substring position
;
; Result:     ST.L - Address of substring
;             ST.L - Size of substring
;
; This routine preserves all registers.
;

%FC_SUB1
        movem.l d0-d1/a0-a1,-(sp)
        move.l  16(sp),a1       ; Pop return address
        move.w  20(sp),d1       ; Start of substring
        ext.l   d1
        bgt.s   start1_ok          
		; min <= 0
		move.l	#1,d1			; set it to one.
start1_ok
        move.l  22(sp),d0       ; Length of main string
        move.l  26(sp),a0       ; Address of string
        sub.l   d1,d0           ; No. Is min > length?
        bge.s   min1ltmax          ; Yes.
        	move.l  22(sp),d1       ; Length of main string
		move.l	#0,d0			; (GB) set it to zero.
min1ltmax
        addq.l  #1,d0           ; No. Set D0 to size of substring
        lea     -1(a0,d1.l),a0  ; Compute substring address
        move.l  a0,26(sp)       ; and store it
        move.l  d0,22(sp)       ; Store substring length
        move.l  a1,18(sp)       ; Store return address
        movem.l (sp)+,d0-d1/a0-a1
        addq.w  #2,sp
        rts
        
;
; %FC_FIN - Finish character assign and blank fill
;
; Parameters: ST.L - Address to put next character
;             ST.L - Number of bytes left unasigned
;             ST.L - Address of characters to copy
;             ST.L - Number of characters to copy
;
; This routine clobbers: D0,D1,D2,A0,A1
;

%FC_FIN
        move.l  (sp)+,d2        ; Pop return address
        move.l  (sp)+,d1        ; Length of source
        move.l  (sp)+,a1        ; Address of source
        move.l  (sp)+,d0        ; Room in destination
        move.l  (sp)+,a0        ; Address of destination
        cmp.l   d0,d1           ; Will this overflow dest?
        ble.s   fi_room         ; No. Do it all.
        move.l  d0,d1           ; Yes. Only copy what fits
fi_room sub.l   d1,d0           ; D0 = Final room left
        bra.s   fi_test         ; D1 = count to copy
fi_loop move.b  (a1)+,(a0)+     ; Copy the next byte
fi_test subq.l  #1,d1           ; Any more to copy?
        bpl.s   fi_loop         ; Yes.
                                ; No. D0 = count, A0 = address
        subq.l  #1,d0           ; Get ready for DBRA
        blt.s   fi_done         ; Done if count <= zero
        moveq   #' ',d1         ; Get a fast blank
fi_blnk move.b  d1,(a0)+        ; Blank the next byte
        dbra    d0,fi_blnk      ; More to fill?
fi_done move.l  d2,a0
        jmp     (a0)
        
;
; %FC_CMP - Compare two FORTRAN character values
;
; Parameters: ST.L - Address of left value
;             ST.L - Size of left value
;             ST.L - Address of rignt value
;             ST.L - Size of right value
;
; Assumes: The length of both args is > 0
;
; Result:     CC   - Result of comparison
;
; This routine preserves all registers.
;

%FC_CMP
        movem.l d0-d1/a0-a2,-(sp)
        lea     24(sp),a2       ; (A2) - arguments
        move.l  (a2)+,d1        ;  D1  = right length
        move.l  (a2)+,a1        ; (A1) = right value
        move.l  (a2)+,d0        ;  D0  = left length
        move.l  (a2)+,a0        ; (A0) = left value
        move.l  20(sp),-(a2)    ; Set up return address
        cmp.l   d0,d1           ; See which is longer
        beq.s   cm_eq           ; Neither.
        blt.s   cm_ll           ; Left arg is
;
; Right arg is longer
;
        sub.l   d0,d1           ; D1 = extra size of right arg
        subq.l  #1,d0           ; Set up DBNE instruction
cm_rlp  cmpm.b  (a1)+,(a0)+     ; Are they equal?
        dbne    d0,cm_rlp       ; Yes. So far.
        bne.s   cm_done         ; If not equal, we're done.
        moveq   #' ',d0         ; Get a fast blank
        subq.l  #1,d1           ; Set up DBNE instruction
cm_rlp2 cmp.b   (a1)+,d0        ; See if left all blanks
        dbne    d1,cm_rlp2      ;
        bra.s   cm_done         ;
;
; Left arg is longer
;
cm_ll   sub.l   d1,d0           ; D0 = extra size of left arg
        subq.l  #1,d1           ; Set up DBNE instruction
cm_llp  cmpm.b  (a1)+,(a0)+     ; Are they equal?
        dbne    d1,cm_llp       ; Yes. So far.
        bne.s   cm_done         ; If not equal, we're done.
        subq.l  #1,d0           ; Set up DBNE instruction
cm_llp2 cmpi.b  #' ',(a0)+      ; See if right all blanks
        dbne    d0,cm_llp2      ;
        bra.s   cm_done         ;
;
; Neither arg is longer
;
cm_eq   subq.l  #1,d0           ; Set up DBNE instruction
cm_elp  cmpm.b  (a1)+,(a0)+     ; Are they equal?
        dbne    d0,cm_elp       ; Yes. So far.
cm_done movem.l (sp)+,d0-d1/a0-a2
        adda    #16,sp
        rts
        
;
; %_ICHAR - Convert character into integer
;
; Parameters: ST.L - Address of character string
;             ST.L - Length of character string
;
; Returns:    D0.L - Integer value of character
;
; Scratches: Only D0.
;

%_ICHAR
        move.l  a0,-(sp)
        clr.l   d0              ; Place to put result
        move.l  12(sp),a0       ; (A0) = string
        move.b  (a0),d0         ; Fetch first character
        move.l  4(sp),12(sp)    ; Set up return address
        move.l  (sp)+,a0
        addq.w  #8,sp
        rts
        
;
; %_CHAR - Convert integer into character
;
; Parameters: ST.L - Address to put character
;             ST.B - Integer
;
; Returns:    ST.L - Address of character
;             ST.L - Length (=1)
;
; All registers are preserved.
;

%_CHAR
        subq.w  #2,sp
        move.l  2(sp),(sp)
        move.l  a0,-(sp)
        move.l  12(sp),a0       ; Get address of temp
        move.b  10(sp),(a0)     ; Put character there
        move.l  #1,8(sp)        ; Fill in string length
        move.l  (sp)+,a0
        rts
        
;
; %_LEN - Return the length of a string
;
; Parameters: ST.L - Address of character string
;             ST.L - Length of character string
;
; Returns:    D0.L - Integer length
;
; Scratches: Only D0.
;

%_LEN
        move.l  4(sp),d0        ; D0 = length
        move.l  (sp),8(sp)      ; Set up return address
        addq.l  #8,sp
        rts
        
;
; %_INDEX - Return the position of a substring in a string
;
; Parameters: ST.L - Address of main string    sp+32
;             ST.L - Length of main string     sp+28
;             ST.L - Address of substring      sp+24
;             ST.L - Length of substring       sp+20
;
; Returns:    D0.L - Integer position
;
; Scratches: Only D0.
;

%_INDEX
        movem.l d1-d2/a1-a2,-(sp)
        clr.l   d0              ; D0 = position of match
        bra.s   in_test         ;
in_loop addq.l  #1,d0           ; D0 = new starting position
        move.l  32(sp),a1       ; A1 = place to search main string
        move.l  24(sp),a2       ; A2 = Address of substring
        addq.l  #1,32(sp)       ; Bump main string address
        subq.l  #1,28(sp)       ; and main string length
        subq.l  #1,d2           ;
in_lop2 cmpm.b  (a1)+,(a2)+     ; Are characters the same?
        dbne    d2,in_lop2      ;
        beq.s   in_done         ; If equal, return answer
in_test move.l  28(sp),d1       ; D1 = Length of main string
        move.l  20(sp),d2       ; D2 = Length of substring
        cmp.l   d2,d1           ; Is there room for a match?
        bge.s   in_loop         ; Yes. Try next position
        clr.l   d0              ; Set result to 0 = No-Match
in_done move.l  16(sp),32(sp)   ; Set up return address
        movem.l (sp)+,d1-d2/a1-a2
        adda.w  #16,sp
        rts
        
        end

