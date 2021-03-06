; File: pasdpwr10.text
; Date: 16-Feb-83

        ident   pasdpwr10
        
        global  %_DPWR10
        
        extern  %D_MUL

;
; %_DPWR10 - Double Power of Ten: returns 10Dx, 0 <= x <= 308
;
; Parameters: ST.W - Power
;
; Returns:    D0.L/D1.L - Result
;
; Scratches: Only D0,D1
;

%_DPWR10
        move.l  (sp)+,d1
        move.w  (sp)+,d0
        move.l  d1,-(sp)
        move.l  a0,-(sp)
        tst.w   d0
        blt.s   dpowzer         ; Arg < 0 means return 0
        cmpi.w  #308,d0         ; Is it > 308?
        ble.s   dpowok          ; No. Its OK
        move.l  #$7ff00000,d0   ; Yes. Return inf.
        clr.l   d1
        bra.s   dpowrts
dpowzer clr.l   d0
        clr.l   d1
        bra.s   dpowrts
dpowok  move.w  d0,d1
        andi.w  #15,d1          ; Find 10Dxx where
        lea     dtab1+8,a0      ; xx = lower 4 bits
        lsl.w   #3,d1
        adda.w  d1,a0
        move.l  -(a0),-(sp)     ; Push 10Dxx
        move.l  -(a0),-(sp)
        lsr.w   #4,d0           ; Find 10Dyy where
        lsl.w   #3,d0           ; yy = upper bits
        beq.s   dpowdon         ; No need if yy = 0
        lea     dtab16+8,a0
        adda.w  d0,a0
        move.l  -(a0),-(sp)     ; Push 10Dyy
        move.l  -(a0),-(sp)
        jsr     %D_MUL          ; Compute 10Dxx+yy
dpowdon move.l  (sp)+,d0        ; Store result
        move.l  (sp)+,d1
dpowrts move.l  (sp)+,a0
        rts
        page
;
; power of ten tables...double precision version
;

dtab1   data.l  $3FF00000,$00000000  ; 1.0D+00
        data.l  $40240000,$00000000  ; 1.0D+01
        data.l  $40590000,$00000000  ; 1.0D+02
        data.l  $408F4000,$00000000  ; 1.0D+03
        data.l  $40C38800,$00000000  ; 1.0D+04
        data.l  $40F86A00,$00000000  ; 1.0D+05
        data.l  $412E8480,$00000000  ; 1.0D+06
        data.l  $416312D0,$00000000  ; 1.0D+07
        data.l  $4197D784,$00000000  ; 1.0D+08
        data.l  $41CDCD65,$00000000  ; 1.0D+09
        data.l  $4202A05F,$20000000  ; 1.0D+10
        data.l  $42374876,$E8000000  ; 1.0D+11
        data.l  $426D1A94,$A2000000  ; 1.0D+12
        data.l  $42A2309C,$E5400000  ; 1.0D+13
        data.l  $42D6BCC4,$1E900000  ; 1.0D+14
        data.l  $430C6BF5,$26340000  ; 1.0D+15
        data.l  $4341C379,$37E08000  ; 1.0D+16
        
dtab16  data.l  $3FF00000,$00000000  ; 1.0D+00
        data.l  $4341C379,$37E08000  ; 1.0D+16
        data.l  $4693B8B5,$B5056E17  ; 1.0D+32
        data.l  $49E5E531,$A0A1C873  ; 1.0D+48
        data.l  $4D384F03,$E93FF9F5  ; 1.0D+64
        data.l  $508AFCEF,$51F0FB5F  ; 1.0D+80
        data.l  $53DDF675,$62D8B363  ; 1.0D+96
        data.l  $5730A1F5,$B8132466  ; 1.0+112
        data.l  $5A827748,$F9301D32  ; 1.0+128
        data.l  $5DD48057,$38B51A75  ; 1.0+144
        data.l  $6126C2D4,$256FFCC3  ; 1.0+160
        data.l  $64794514,$5230B378  ; 1.0+176
        data.l  $67CC0E1E,$F1A724EB  ; 1.0+192
        data.l  $6B1F25C1,$86A6F04C  ; 1.0+208
        data.l  $6E714A52,$DFFC6799  ; 1.0+224
        data.l  $71C33234,$DE7AD7E3  ; 1.0+240
        data.l  $75154FDD,$7F73BF3C  ; 1.0+256
        data.l  $7867A93A,$2954F3B8  ; 1.0+272
        data.l  $7BBA44DF,$832B8D46  ; 1.0+288
        data.l  $7F0D2A1B,$E4048F90  ; 1.0+304
        
        end
                                                                                                                                                                                                                                                                                                                                                         