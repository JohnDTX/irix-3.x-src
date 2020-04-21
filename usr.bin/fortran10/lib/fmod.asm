
COMREG  equ     $1040
STCREG  equ     $1042
DATREG  equ     $1044

ADDCMD  equ     $1001
SUBCMD  equ     $1007
MULCMD  equ     $100b
DIVCMD  equ     $1013
MODCMD  equ     $1030
MINCMD  equ     $101e
MAXCMD  equ     $101f
DADDCMD equ     $1002
DSUBCMD equ     $1008
DMULCMD equ     $100c
DDIVCMD equ     $1014
FLTCMD  equ     $1024
TNCCMD  equ     $1027
DFLTCMD equ     $1044
DTNCCMD equ     $1045
F2DCMD  equ     $1042

;

		global	%F_MOD
%F_MOD
        move.l  (sp)+,a0
        move.w  #DIVCMD,COMREG.W
        move.l  4(sp),DATREG.W
        move.l  (sp),DATREG.W
mod_lp  tst.w   STCREG.W
        bge.s   mod_lp
		move.l	DATREG.W,d0
		move.w	#TNCCMD,COMREG.W
		move.l	d0,DATREG.W
		move.l	DATREG.W,d0
		move.w	#FLTCMD,COMREG.W
		move.l	d0,DATREG.W
		move.l	DATREG.W,d0
		move.w	#MULCMD, COMREG.W
		move.l	d0,DATREG.W
		move.l	(sp),DATREG.W
		move.l	DATREG.W,d0
		move.w	#SUBCMD,COMREG.W
		move.l	4(sp),DATREG.W
		move.l	d0,DATREG.W
        addq.w  #8,(sp)
        move.l  DATREG.W,d0
        jmp     (a0)
        rts
		end
