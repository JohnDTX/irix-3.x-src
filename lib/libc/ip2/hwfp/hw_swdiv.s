|	The below are defined in fpk.s:
|		.globl	_f_unpk
|		.globl  _f_pack

|	and the following in dpk.s:
|		.globl  _d_unpk
|		.globl  _d_pack
|		.globl  _d_exte
|		.globl  _d_norm
|		.globl  _d_rcp
|		.globl  _d_nrcp
|		.globl  _d_usel

|	The following are in ipk.s:

|		.globl  _i_pack
|		.globl  _g_nsh
|
		.globl  __sw_f_div
		.globl	__sw_d_div
		.globl  __sw_fr_idiv
		.globl  __sw_dr_idiv
		.globl  __sw_f_idiv
		.globl  __sw_d_idiv

|
|	Floating point exception handling - 
|
|	    When an error is detected in an operation in this module, 
|	a call to the appropriate floating point error handler is made
|	with arguments to indicate the error condition.  This consists
|	of a call to __lraise_fperror or __raise_fperror with the arguments
|	op and type.
|
	.globl	__raise_fperror
	.globl	__lraise_fperror
|
|
|	OPs follow:
|
ADD 	=1
SUB	=2
MUL	=3
DIV	=4
FIX   =5	| precision to integer 
PRECISION =6	| precision change to given precision 
MOD	=7
CMP	=8

|
|	TYPEs
|
INVALID_OP_A	=0x110
INVALID_OP_B2	=0x122
INVALID_OP_C	=0x130
INVALID_OP_D1	=0x141
INVALID_OP_D2	=0x142
INVALID_OP_E1	=0x151
INVALID_OP_E2	=0x152
INVALID_OP_F3	=0x163		| operand not normalized 

INVALID_OP_G	=0x170
INVALID_OP_H	=0x180

DIVZERO		=0x200
OVERFLOW	=0x300

__sw_f_div:
		moveml	a7@(4),#0x0003
_fr_div:         moveml  #0x3c00,a7@-		|modified to load parms 
|                movl    a7@(32),d1		
|                movl    a7@(28),d0
                roll    #0x1,d0
                roll    #0x1,d1
                movb    d0,d5
                eorb    d1,d5
                roll    #0x8,d0
                roll    #0x8,d1
                clrw    d3
                clrw    d4
                movb    d0,d4
                movb    d1,d3
                andw    #0xfffffe00,d0
                andw    #0xfffffe00,d1
                addqb   #0x1,d4
                subqw   #0x1,d4
                bles    _f_div_toperr
                addqb   #0x1,d0
_f_div_backtop:
                addqb   #0x1,d3
                subqw   #0x1,d3
                ble    _f_div_boterr
                addqb   #0x1,d1
_f_div_backbot:
                rorl    #0x2,d1
                rorl    #0x4,d0
                subw    d3,d4
                movw    d1,d3
                swap    d1
                divu    d1,d0
                movw    d0,d2
                mulu    d2,d3
                clrw    d0
                subl    d3,d0
                asrl    #0x2,d0
                divs    d1,d0
                extl    d2
                extl    d0
                swap    d2
                asll    #0x2,d0
                addl    d2,d0
                asll    #0x1,d0
                addw    #0x7f,d4
                bsr     _f_rcp
_f_div_drepk:
                rorl    #0x8,d0
                roxrb   #0x1,d5
                roxrl   #0x1,d0
_f_div_dexit:
|                movl    d0,a7@(32)
                moveml  a7@+,#0x003c
|                movl    a7@+,a7@	|dont pop the stack
|
|	And return the result in d0
|
|		movl	a7@(8),d0
                rts
_f_div_toperr:
                bnes    _f_div_tngu
_f_div_normt:
                subqw   #0x1,d4
                roll    #0x1,d0
                bhis    _f_div_normt
                addqw   #0x1,d4
                bras    _f_div_backtop
|
|	top is infinity or Nan
|
|		registers: d0 - top mant
|			   d1 - bot mant
|			   d2 - ?
|			   d3 - bot exp
|			   d4 - top/final exp
|	
_f_div_tngu:
                cmpb    d3,d4
                bnes    _f_div_tngu1
|
|		both inf or Nan.... find out if either is Nan
|
		tstl	d0			| is top Nan?
		bnes	_f_div_inv_a		| yes
		tstl	d1			| is bot Nan?
		bnes	_f_div_inv_a		| yes
		movl	#INVALID_OP_D2,d1	| inf/inf
		bra	_f_div_goraise
_f_div_tngu1:
|
|	for compatibility, we have to generate divzero if
|	the bottom is zero here.
|
		tstw	d3
		beqs	dvzero
                tstl    d0
                bnes    _f_div_inv_a		| top is Nan
|
|		top is INF.  Check bottom for zero.
|
		tstb	d3
		bnes	_f_div_geninf
		tstl	d1
		bnes	_f_div_geninf
|
|		INF/0
|
dvzero:
		movl	#DIVZERO,d1
		bra	_f_div_goraise
_f_div_inv_a:
                movl	#INVALID_OP_A,d1
		bra	_f_div_goraise		| top is Nan
_f_div_boterr:
                beqs    _f_div_botlow		| bot is zero
|
|		the bot is either inf or nan
|
                tstl    d1
                bnes    _f_div_inv_a		| bot is Nan
|
|		bot is inf. top is not.  result is zero.
|
                clrl    d0
                bras    _f_div_drepk
_f_div_botlow:
                tstl    d1
                beqs    _f_div_bot0
_f_div_normb:
                subqw   #0x1,d3
                roll    #0x1,d1
                bccs    _f_div_normb
                addqw   #0x1,d3
                bra    _f_div_backbot
_f_div_bot0:
|
|		bot and possibly top are zero.
|
		movl	#DIVZERO,d1		| default divide by zero
                tstl    d0			| test top
                bnes    _f_div_goraise		| not zero, go raise exception
		movl	#INVALID_OP_D1,d1	| 0/0
		bra	_f_div_goraise
_f_div_geninf:
                movl    #0xff,d0
                bra    _f_div_drepk
_f_div_dinvop:
                cmpl    d0,d1
                bcss    _f_div_usenan
                tstl    d1
                beqs    _f_div_gennan
                exg     d1,d0
_f_div_usenan:
                lsrl    #0x8,d0
                lsrl    #0x1,d0
                bras    _f_div_bldnan
_f_div_gennan:
                moveq   #0x4,d0
_f_div_bldnan:
                orl     #0x7f800000,d0
                jra    _f_div_dexit
|
|		raise exception on floating point divide
|
_f_div_goraise:
		movl	d1,sp@-
		movl	#DIV,sp@-
		jbsr	__raise_fperror
		addql	#8,sp
		bra	_f_div_dexit		| leave result in d0
|
		.data
		.even
_div_mod_op:	.word 0
		.text

__sw_d_div:		moveml  #0x3f00,a7@-
|                moveml  a7@(36),#0x000f	|again, wrong order of load.
		moveml	a7@(28),#0x000c
		moveml	a7@(36),#0x0003
_dr_dodiv:
		movw	#DIV,_div_mod_op
                movl    d0,d5
                eorl    d2,d5
                clrl    d4
                bsrs    _d_div_extrem
                movw    d6,d5
                subw    d7,d5
                movw    #0x1e,d4
                bsr     _d_div_shsub
                movl    d7,d6
                movw    #0x17,d4
                bsr     _d_div_shsub
                lsll    #0x8,d7
                orl     d3,d2
                beqs    _d_div_dd4
                bset    #0x1,d7
_d_div_dd4:
                lsll    #0x1,d7
                roxll   #0x1,d6
                movl    d6,d2
                movl    d7,d3
                movw    d5,d6
                addw    #0x3ff,d6
                bsr     _d_nrcp
                bra     _d_mod_dmsign
_d_div_unp:
                movl    d0,d7
                andl    #0xfffff,d0
                swap    d7
                lsrw    #0x4,d7
                andw    d4,d7
                bnes    _d_div_unpxit
                tstl    d0
                bnes    _d_div_unp2
                tstl    d1
                beqs    _d_div_unpxit
_d_div_unp2:
                addqw   #0x1,d7
_d_div_unpl:
                subqw   #0x1,d7
                lsll    #0x1,d1
                roxll   #0x1,d0
                btst    #0x14,d0
                beqs    _d_div_unpl
_d_div_unpxit:
                rts
_d_div_extrem:
                movw    #0x7ff,d4
                exg     d2,d0
                exg     d3,d1
                bsrs    _d_div_unp
                exg     d0,d2
                exg     d1,d3
                exg     d7,d6
                beqs    _d_div_topzero
                cmpw    d4,d6
                beqs    _d_div_topbig
                bset    #0x14,d2
_d_div_topzero:
                bsrs    _d_div_unp
                beqs    _d_div_botzero
                cmpw    d4,d7
                beq    _d_div_botbig
                bset    #0x14,d0
                lsll    #0x1,d1
                roxll   #0x1,d0
                rts
|
|
|	d4 - + for div; - for mod.
|	d0/d1 (bottom), d2/d3 (top).
|
_d_div_topbig:
|
|	unpack the top.  If it is zero, we need to produce
|	divzero, rather than illegalop.
|
                bsrs    _d_div_unp
		beqs	_d_div_divzero
|
|	the top is INF or Nan.  Is it Nan?
|
                tstl    d4
                bmis    _d_mod_extrem
                cmpw    d6,d7
                beqs    _d_div_bothextrem
|
|	now find whether the top is NAN
|
                tstl    d2
                bnes    _d_div_topnan
                tstl    d3
                bnes    _d_div_topnan
|
|	the top is INF.
|	we have to check the bottom here for zero.
|
		tstw	d7
		bnes	_d_div_geninf
		orl	d0,d1
                tstl    d1
                bnes    _d_div_geninf
|
|	INF/0
|
		movl	#DIVZERO,d1
		bras	_d_div_goraise
|
|	top is Nan
|	
_d_div_topnan:
		movl	#INVALID_OP_A,d1
		bras	_d_div_goraise
_d_div_botzero:
                tstl    d4
                bmis    _d_mod_botzero
                orl     d2,d3
                bnes    _d_div_divzero
|
|		0/0...
|
		movl	#INVALID_OP_D1,d1
		bra	_d_div_goraise
_d_div_divzero:
|
|		X / 0....
|
		movl	#DIVZERO,d1
		bra	_d_div_goraise
_d_div_geninf:
                movl    #0xffe00000,d2
                bra    _d_div_clrbot
_d_mod_extrem:
|
|	the top is either INF or Nan.
|
                tstl    d2
                bnes    _d_mod_topnan
		tstl	d3
		bnes	_d_mod_topnan
|
|	top is INF
|	
		movl	#INVALID_OP_E2,d1
		bras	_d_div_goraise
|
|	top is Nan
|	
_d_mod_topnan:
		movl	#INVALID_OP_A,d1
		bras	_d_div_goraise
_d_mod_botzero:
		movl	#INVALID_OP_E1,d1
		bras	_d_div_goraise
|
_d_div_bothextrem:
|
|	Both the top and bottom of a divide are extreme.
|
|	If either is Nan, INVALID_OP_A, else INVALID_OP_D2.
|
		orl	d0,d1
		bnes	_d_mod_topnan
		orl	d2,d3
		bnes	_d_mod_topnan
|
|	INF/INF....
|
		movl	#INVALID_OP_D2,d1
		bras 	_d_div_goraise
_d_div_goraise:
		addql	#4,sp
		movl	d1,sp@-
		movw	_div_mod_op,sp@-
		clrw	sp@-
		jbsr	__lraise_fperror
		addql	#8,sp
| 
|	returned.  Result of operation is in d0/d1.
|	as we were called as extreme() (i.e., with a return addr
|	on the stack), we must pop it.
|
		addql	#4,sp
		bra	_d_mod_dmexit1
_d_div_botbig:
|
|	bottom is Nan or INF.
|
                tstl    d0
                beqs    _d_div_botinf
		tstl	d1
		beqs	_d_div_botinf
|
|	bottom is Nan
|	
		movl	#INVALID_OP_A,d1
		bra	_d_div_goraise
_d_div_botinf:
                tstl    d4
                bpls    _d_div_genzero
                addqw   #0x4,a7
                bra     _d_mod_usetop
_d_div_isnan:
                cmpw    d7,d6
                bnes    _d_div_isn2
                cmpl    d0,d2
_d_div_isn2:
                bges    _d_div_isn4
                movw    d7,d6
                movl    d0,d2
_d_div_isn4:
                swap    d2
                lslw    #0x4,d6
                orw     d6,d2
                swap    d2
                lsll    #0x1,d2
                cmpl    #0xffe00000,d2
                bhis    _d_div_gotnan
_d_div_gennan:
                movl    #0xffe00008,d2
                tstl    d4
                bpls    _d_div_gotnan
                addql   #0x2,d2
_d_div_gotnan:
                clrl    d5
                bras    _d_div_clrbot
_d_div_genzero:
                clrl    d2
_d_div_clrbot:
                clrl    d3
_d_div_sign:
                addqw   #0x4,a7
                bra     _d_mod_dmsign
_d_div_shsub:
                clrl    d7
_d_div_shs1:
                addl    d3,d3
                addxl   d2,d2
                cmpl    d0,d2
                dbge    d4,_d_div_shs1
                bset    d4,d7
                subl    d1,d3
                subxl   d0,d2
                dbmi    d4,_d_div_shs1
                bpls    _d_div_shs7
                addl    d1,d3
                addxl   d0,d2
                bclr    d4,d7
                tstw    d4
                dblt    d4,_d_div_shs1
_d_div_shs7:
                rts
_d_mod_usetop:
                addw    #0xb,d6
                bsr     _d_norm
                bsr     _d_rcp
_d_mod_dmsign:
                roxll   #0x1,d5
                roxrl   #0x1,d2
_d_mod_dmexit:
|
|	put the result back in d0/d1.
|
		movl	d2,d0
		movl	d3,d1		
|
|	and restore the rest of the registers and exit
|
_d_mod_dmexit1:
                moveml  a7@+,#0x00fc
|                movl    a7@+,a7@		|dont pop the stack
|                movl    a7@+,a7@
                rts
__sw_d_idiv:
		movl	sp@(4),a0
		movl	sp@(8),d0
		movl	sp@(12),d1
__sw_dr_idiv:
|	enter with the arg in d0, the address in a0.
|
		movl	a0,sp@-		|save a0
		pea	_dr_idiv_rtn
		moveml	#0x3f00,sp@-	|save d2-d7
		movl	a0@+,d2		|get the arg
		movl	a0@,d3
		bra	_dr_dodiv	|do operation
_dr_idiv_rtn:
		movl	sp@+,a0
		movl	d0,a0@+		|and restore the value
		movl	d1,a0@
		rts

__sw_f_idiv:
		movl	sp@(4),a0
		movl	sp@(8),d0
__sw_fr_idiv:
|	enter with the arg in d0, the address in a0.
|	store the value indirect on exit, and return it
|	in d0 as well.
|
|	the wrong arg is in d0, so we have to exchange them.
|
		movl	a0@,d1		|get the arg
		movl	a0,sp@-		|save a0
		exg	d0,d1
		bsr	_fr_div		|do operation
		movl	sp@+,a0
		movl	d0,a0@		|and restore the value
		rts

_d_norm:        tstl    d2
                bnes    _d_norm+0x14
                cmpw    #0x20,d6
                blts    _d_norm+0x16
                subw    #0x20,d6
                exg     d3,d2
                tstl    d2
                beqs    _d_norm+0x24
                bmis    _d_norm+0x22
                lsll    #0x1,d3
                roxll   #0x1,d2
                dbmi    d6,_d_norm+0x16
                dbpl    d6,_d_norm+0x22
                rts
                movw    #0xfffff752,d6
                rts

_d_nrcp:        tstl    d2
                bmis    _d_rcp
                subqw   #0x1,d6
                lsll    #0x1,d3
                roxll   #0x1,d2

_d_rcp: tstw    d6
                bgts    _d_rcp+0x16
                cmpw    #0xffffffcb,d6
                blts    _d_usel+0x26
                negw    d6
                lsrl    #0x1,d2
                roxrl   #0x1,d3
                dbra    d6,_d_rcp+0xc
                clrw    d6
                addl    #0x400,d3
                bccs    _d_rcp+0x28
                addql   #0x1,d2
                bccs    _d_rcp+0x28
                roxrl   #0x1,d2
                roxrl   #0x1,d3
                addqw   #0x1,d6
                cmpw    #0x7ff,d6
                bges    _d_usel+0x2c

_d_usel:        movl    #0xfffff800,d4
                andl    d4,d3
                andl    d2,d4
                eorl    d4,d2
                orl     d2,d3
                movl    d4,d2
                lsll    #0x1,d2
                bcss    _d_usel+0x1c
                cmpw    #0x7ff,d6
                beqs    _d_usel+0x1c
                clrw    d6
                moveq   #0xb,d4
                rorl    d4,d3
                orw     d6,d2
                rorl    d4,d2
                rts
                clrl    d2
                clrl    d3
                rts
                movl    #0xffe00000,d2
                clrl    d3
                rts

_f_rcp: tstl    d0
                bmis    _f_rcp+0x8
                subqw   #0x1,d4
                lsll    #0x1,d0
                tstw    d4
                bgts    _f_rcp+0x1a
                cmpw    #0xffffffe8,d4
                blts    _f_rcp+0x34
                negb    d4
                addqb   #0x1,d4
                lsrl    d4,d0
                clrw    d4
                addl    #0x80,d0
                bccs    _f_rcp+0x26
                roxrl   #0x1,d0
                addqw   #0x1,d4
                cmpw    #0xff,d4
                bges    _f_rcp+0x38
                lsll    #0x1,d0
                scs     d0
                andb    d4,d0
                rts
                clrl    d0
                rts
                movl    #0xff,d0
                rts

