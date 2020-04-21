|	This is the MIT assembler version of
|	the syntactics floating point library.  All comments
|	were put in by G.Boyd with reference to the Motorola
|	assembler version.

|	The following functions have been modified to return 
|	their results in registers d0/d1. In addition, those
|	functions with a star were SVS Pascal-callable, and
|	were modified 
|		a. to NOT expect extra room on the stack 
|		   on the return. 
|		b. to NOT pop the stack by the mysterious 
|		   longwords that were written in, as this is 
|		   unneeded in C.
|	All two-argument functions were
|	modified to expect the last parameter pushed first (C),
|	rather than the reverse.
|
|
|	   C called:  (done)
|		_[f,d]_[add,sub], 
|		_[f,d]_neg,
|		_[f,d]_[div,mul], 
|		_[f,d]_2_[d,f],
|		_i_2_[d,f],
|		_[d,f]_2_i,
|		_[d,f]_mod,	*
|	     (needed)
|		_[f,d]_[max,min],
|	    
|	_[f,d]_cmp were modified to expect the correct order on the stack.
|

|	The following functions have been modified in the way
|	that they return their VAR record to pascal. This was
|	necessitated by a different calling convention:
|
|
|		_pf_pack, _pf_unpk
|
|	IN ADDITION: the above two routines have been modified
|	to save and restore the registers they clobber. (bug #56).
|

|	The global declarations are broken parially by
|	original filename below.  They are all defined
|	in this module.
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
|	Additional entry points follow:
|
		.globl  _f_sub
		.globl  _f_add
|		.globl  _i_sup
|		.globl  _i_sdn
|		.globl  _f_sup
|		.globl  _f_sdn
|		.globl  _i_dup
|		.globl  _i_ddn
|		.globl  _d_ddn
|		.globl  _f_dim
|		.globl  _f_max
|		.globl  _f_min
		.globl  _f_cmp
		.globl  _d_sub
		.globl  _d_add
|		.globl  _d_dim
|		.globl  _d_max
|		.globl  _d_min
		.globl  _d_cmp
		.globl  _f_div
		.globl	__sw_f_div
		.globl  _d_mul
		.globl	_i_2_d
|
|	The following are user-callable entries for
|	float/integer conversions:
|
		.globl  _float		| same as _i_2_f
		.globl  __lfloat	| same as _i_2_d
		.globl  _fix		| same as _f_2_i
		.globl  __lfix		| same as _d_2_i
		.globl 	_fneg		| same as _f_neg
		.globl  __lfneg		| same as _d_neg

		.globl	_f_2_d
		.globl	_d_2_i
		.globl 	_d_2_f
		.globl	_i_2_f
		.globl  _f_2_i
		.globl	_d_div
		.globl	__sw_d_div
|
|	[f,d]_mod were removed after the Juniper release.  They have never
|	been supported or documented and will not be.  (GB) 7/19/85.
|	The code was left in, as parts of it are common for other routines.
|
| 		.globl	_d_mod
|		.globl	_f_mod

		.globl	_f_mul	
		.globl  _pf_unpk
		.globl  _pf_pack
|
|	And, the following were added by SGI:
|
		.globl 	_f_neg
		.globl  _d_neg
|
|
|	GB (1-6-83). The compiler was modified to pass single
|	precision arguments in registers whenever possible.  
|	
|	    The following were added to expect the call
|
|		    	func(d0,d1)
|
|		_fr_add,_fr_sub,_fr_mul,_fr_div,_fr_cmp
|
		.globl	_fr_add
		.globl	_fr_sub
		.globl	_fr_mul
		.globl	_fr_div
		.globl	_fr_cmp
|
|	    The following were changed to expect the call
|
|			func(d0)
|
|		_ir_2_f,_fr_2_i,_fr_neg,_fr_2_d,ir_2_d
|
		.globl	_ir_2_f
		.globl	_fr_2_i
		.globl	_fr_neg
		.globl	_fr_2_d
		.globl	_ir_2_d
|
|	The following indirect routines were added to expect the
|	address of the result in a0 and the argument in d0:
|
|		_fr_iadd,_fr_isub,_fr_imul,_fr_idiv
|
		.globl	_fr_iadd
		.globl  _fr_isub
		.globl	_fr_imul
		.globl  _fr_idiv
		.globl	__sw_f_idiv
|
|	The following were added to expect the argument in d0,d1:
|
|		_dr_2_f,_dr_2_i,_dr_neg
|
		.globl	_dr_2_f
		.globl	_dr_neg
		.globl	_dr_2_i
|
|	The following were added to expect the address of the result in
|	a0 and the argument in d0,d1:
|
|		_dr_iadd,_dr_isub,_dr_imul,_dr_idiv
|
		.globl	_dr_iadd
		.globl	_dr_isub
		.globl  _dr_imul
		.globl  _dr_idiv
		.globl	__sw_d_idiv

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

|
|
|	The neg routines were added by GB (SGI)
|
_d_neg:
__lfneg:
		movl	a7@(8),d1	|move least signif word to d1  
_f_neg:
_fneg:
		movl	sp@(4),d0	|move msl to d0
_dr_neg:
_fr_neg:
		bchg	#31,d0		|and invert the bit.
                rts

|
|	f_sub: enter wanting to subtract d0-d1
|
		.data
_f_add_optype:	.word	ADD
		.text
|
_f_add:
		moveml	a7@(4),#0x0003
		bra	_fr_add
_f_sub:
		moveml	a7@(4),#0x0003
_fr_sub:
		bchg	#31,d1		|change d1 sign and add
		movw	#SUB,_f_add_optype
		bras	_fr_add_enter
_fr_add:         
		movw	#ADD,_f_add_optype
_fr_add_enter:
		moveml  #0x3e00,a7@-	|save regs d2-d6
                asll    #0x1,d0
                scs     d2
                asll    #0x1,d1
                scs     d3
                cmpl    d1,d0
                blss    _f_add_add2
                exg     d0,d1
                exg     d2,d3
_f_add_add2:
                roll    #0x8,d1
                roll    #0x8,d0
                clrl    d5
                movb    d1,d5
                clrw    d4
                movb    d0,d4
                bnes    _f_add_nunfl
|
|	the last branch was taken if the exponent is non-zero (the number is 
|	neither zero nor denormalized). (smaller number).
|
                tstl    d0
                beq	_f_add_usel
|
|	the smaller is denormalized. Raise 'denormalized number' exception...
|
		movl	d1,sp@-		| save d1
		movl	#INVALID_OP_F3,sp@-
		movw	_f_add_optype,sp@-
		clrw	sp@-
		jbsr	__raise_fperror
		addql	#8,sp
		movl	sp@+,d1		|restore d1
		tstb	d5		| test exp of larger.
		bnes	_f_add_usel	| use the larger
|
|	denorm/0 +/- denorm/0 --> generate zero
|
		clrl	d1
		bra	_f_add_asexit
|                tstb    d5
|                bnes    _f_add_gu1
|                cmpb    d3,d2
|                bnes    _f_add_gusub
|                addl    d0,d1
|                addxb   d1,d1
|                bras	_f_add_asbuild
_f_add_gusub:
                subl    d0,d1
                bras	_f_add_asbuild
|
|	at this point, neither is denormalized
|
_f_add_nunfl:
                movb    #0x1,d0
                rorl    #0x1,d0
_f_add_gu1:
                movb    #0x1,d1
                rorl    #0x1,d1
                cmpb    #0xff,d5
                beqs    _f_add_ovfl
                movw    d5,d6
                subw    d4,d6
_f_add_shl:
                cmpw    #0x8,d6
                blts    _f_add_shx
                subqw   #0x8,d6
                lsrl    #0x8,d0
                bnes    _f_add_shl
_f_add_shx:
                lsrl    d6,d0
                cmpb    d3,d2
                bnes    _f_add_diff
                addl    d0,d1
                bccs    _f_add_endas
                roxrl   #0x1,d1
                addqw   #0x1,d5
                cmpw    #0xff,d5
                blts    _f_add_endas
                bra     _f_add_geninf
_f_add_diff:
                subl    d0,d1
                bmis    _f_add_endas
                beq    _f_add_cancel
_f_add_norm:
                asll    #0x1,d1
                dbmi    d5,_f_add_norm
                subqw   #0x1,d5
                bgts    _f_add_endas
                beqs    _f_add_norm2
                clrw    d5
                lsrl    #0x1,d1
_f_add_norm2:
                lsrl    #0x1,d1
_f_add_endas:
                addl    #0x80,d1
                bccs    _f_add_rofl
                roxrl   #0x1,d1
                addqw   #0x1,d5
                cmpw    #0xff,d5
                beq     _f_add_geninf
_f_add_rofl:
                lsll    #0x1,d1
_f_add_usel:
                movb    d5,d1
_f_add_asbuild:
                rorl    #0x8,d1
                roxrb   #0x1,d3
                roxrl   #0x1,d1
_f_add_asexit:
                movl    d1,d0			|result placed in correct reg
                moveml  a7@+,#0x007c
		movw	#ADD,_f_add_optype
		rts
|
_f_add_ovfl:
                lsll    #0x1,d1
                tstl    d1
		beqs	_f_add_ovfl1
                movl    #INVALID_OP_A,d0	|larger is NAN - invalid operand
		bra	_f_add_gosignal
_f_add_ovfl1:
                cmpb    d4,d5
		bnes	_f_add_usel		|larger is inf -valid
                cmpb    d3,d2
		beqs	_f_add_usel		|like sign - go ahead
|
|	in affine mode, 
|	unlike infinities are illegal
|
                movl	#INVALID_OP_B2,d0       
		bra	_f_add_gosignal
|
|	addition/subtraction single precision overflow.
|
_f_add_gosignal:
		movl	d0,sp@-
		movw	_f_add_optype,sp@-
		clrw	sp@-
		jbsr	__raise_fperror
		addql	#8,sp
		movl	d0,d1
		bra	_f_add_asexit
_f_add_cancel:
                clrl    d1
                bras    _f_add_asexit
_f_add_geninf:
		movl	#OVERFLOW,d0
                bras    _f_add_gosignal

|
|		floating compare of d0 to d1 (fcmp(d0,d1))
|
|
_f_cmp:
		moveml	a7@(4),#0x0003
_fr_cmp:
|	        movw	#0,sp@-		| move sr placeholder to stack
|		movl	d2,sp@-		| dont clobber d2
|		exg	d0,d1
_f_cmp_check0:
		btst	#31,d0
		beqs	_f_cmp_d0pos
		cmpl	#0x80000000,d0
		bnes	_f_cmp_d0not0
		clrl	d0
		bras	_f_cmp_check1
_f_cmp_d0not0:
		cmpl	#0xff800001,d0
		bcss	_f_cmp_check1
		bras	_f_cmp_d0Nan
_f_cmp_d0pos:
		cmpl	#0x7f800001,d0
		bges	_f_cmp_d0Nan

_f_cmp_check1:
		btst	#31,d1
		beqs	_f_cmp_d1pos
		cmpl	#0x80000000,d1
		bnes	_f_cmp_d1not0
		clrl	d1
		bras	_f_cmp_cmp
_f_cmp_d1not0:
		cmpl	#0xff800001,d1
		bccs	_f_cmp_d1Nan
		btst	#31,d0
		beqs	_f_cmp_cmp
		exg	d0,d1
		bras	_f_cmp_cmp
_f_cmp_d1pos:
		cmpl	#0x7f800001,d1
		bges	_f_cmp_d1Nan

_f_cmp_cmp:
                cmpl    d1,d0
                andb    #0xe,cc
		rts
|		movw	sr,d2
|                lsll    #0x1,d0
|                lsll    #0x1,d1
|                cmpl    d1,d0
|                bccs    _f_cmp_cmp4
|                exg     d0,d1
|_f_cmp_cmp4:
|                cmpl    #0xff000000,d0
|                blss    _f_cmp_cmp6
||
||		the larger number is a Nan. 
|
_f_cmp_d1Nan:
		clrl	sp@-		|stack marker. good number was in d0.
		movl	d0,d1		|d1 now has good number
		bras	_f_cmp_Nan
_f_cmp_d0Nan:
		movl	#1,sp@-
_f_cmp_Nan:
		movl	d1,sp@-		|save good number
_f_cmp_goraise:
		movl	#INVALID_OP_A,sp@-	| Nan operand
		movl	#CMP,sp@-
		jbsr	__raise_fperror
		addql	#8,sp
|
|	result is compared to the other operand.
|
		movl	sp@+,d1
		tstl	sp@+		|if zero, must exchange
		bnes	_f_cmp_check1
		exg	d0,d1
		bras	_f_cmp_cmp

|_f_cmp_cmp6:
|                tstl    d0
|                bnes    _f_cmp_cmp8
|		moveq	#4,d2
|_f_cmp_cmp8:
|		movw	d2,d0
|		movl	a7@+,d2
|
||	now place the result in the ccr
|
|		movw	d0,cc
|                rts
|
		.data
_d_add_optype:	.word	ADD
		.text

|_d_sub:         bchg    #0x7,a7@(4)		|wrong order
_d_sub:         bchg    #0x7,a7@(12)		
		movw	#SUB,_d_add_optype
		bras	_d_add_enter

_d_add:
|		moveml  #0xff00,a7@-
|                moveml  a7@(36),#0x000f
		movw	#ADD,_d_add_optype
_d_add_enter:
		moveml	#0x3f00,a7@-
		moveml	a7@(28),#0x000f
|
_dr_doadd:
		asll    #0x1,d0
                scs     d4
                asll    #0x1,d2
                scs     d5
                cmpl    d2,d0
                blss    _d_add_dadd1
                exg     d0,d2
                exg     d1,d3
                exg     d4,d5
_d_add_dadd1:
		extw    d5
                eorb    d4,d5
                bsr     _d_exte
                tstw    d7
                bnes    _d_add_nunfl
                movl    d0,d4
                orl     d1,d4
                beq     _d_add_useln
                lsll    #0x1,d1
                roxll   #0x1,d0
                tstw    d6
                bnes    _d_add_gu1
                lsll    #0x1,d3
                roxll   #0x1,d2
                bras    _d_add_shz
_d_add_nunfl:
                bset    #0x1f,d0
_d_add_gu1:
                cmpw    #0x7ff,d6
                beqs    _d_add_ovfl
                bset    #0x1f,d2
                subw    d6,d7
                negw    d7
_d_add_shl:
                subqw   #0x8,d7
                blts    _d_add_shx
                movb    d0,d1
                rorl    #0x8,d1
                lsrl    #0x8,d0
                bnes    _d_add_shl
                tstl    d1
                bnes    _d_add_shl
                bras    _d_add_shz
_d_add_shx:
                addqw   #0x7,d7
                bmis    _d_add_shz
_d_add_shy:
                lsrl    #0x1,d0
                roxrl   #0x1,d1
                dbra    d7,_d_add_shy
_d_add_shz:
                tstb    d5
                bmis    _d_add_diff
                addl    d1,d3
                addxl   d0,d2
                bccs    _d_add_endas
                roxrl   #0x1,d2
                roxrl   #0x1,d3
                addqw   #0x1,d6
                cmpw    #0x7ff,d6
                blts    _d_add_endas
                bras    _d_add_geninf
_d_add_diff:
                subl    d1,d3
                subxl   d0,d2
                bccs    _d_add_diff2
                notw    d5
                negl    d3
                negxl   d2
_d_add_diff2:
                bsr     _d_norm
_d_add_endas:
                bsr     _d_rcp
_d_add_assgn:
                lslw    #0x1,d5
                roxrl   #0x1,d2
_d_add_asexit:
|                moveml  #0x000c,a7@(44)		|result placed on stack
|                moveml  a7@+,#0x00ff
		movl	d2,d0
		movl	d3,d1
_d_add_asexit1:
		moveml	a7@+,#0x00fc
|                movl    a7@+,a7@		|dont pop the stack
|                movl    a7@+,a7@
|		movl	a7@(12),d0		|put result in d0/d1 (GB)
|		movl	a7@(16),d1
                rts
|
|	after this point, all offsets from _d_add are incorrect.
|	replace them.
|
_d_add_useln:
                bset    #0x1f,d2
_d_add_usel:
                bsr     _d_usel
                bras    _d_add_assgn
_d_add_ovfl:
                movl    d2,d4
                orl     d3,d4
                beqs    _d_add_infop
|
|	larger is NAN.
|
                movl    #INVALID_OP_A,d0	|larger is NAN - invalid operand
		bra	_d_add_gosignal
|
|	larger is INF.
|
_d_add_infop:
                cmpw    d6,d7
                tstb    d5
                bpls    _d_add_usel
|
|	in affine mode, 
|	unlike infinities are illegal
|
                movl	#INVALID_OP_B2,d0       
		bra	_d_add_gosignal
_d_add_gosignal:
		movl	d0,sp@-
		movw	_d_add_optype,sp@-
		clrw	sp@-
		jbsr	__lraise_fperror
		addql	#8,sp
| 
|	returned.  Result of operation is in d0/d1.
|
		bra	_d_add_asexit1
|
_d_add_geninf:
		movl	#OVERFLOW,d0
		bras	_d_add_gosignal

|_d_dim:         movl    a7@+,a7@(16)
|                bsr     _d_sub
|                movl    a7@(8),a7@-
|                clrl    a7@(12)
|                clrl    a7@(16)
|
|_d_max:         movl    a7@(16),a7@-
|                movl    a7@(16),a7@-
|                movl    a7@(16),a7@-
|                movl    a7@(16),a7@-
|                bsrs    _d_cmp
|                bccs    _d_max+0x22
|                bclr    #0x7,a7@(12)
|                cmpl    #0x7ff00001,a7@(12)
|                blts    _d_min+0x24
|                movl    a7@+,a7@
|                movl    a7@+,a7@
|                rts
|
|_d_min:         movl    a7@(16),a7@-
|                movl    a7@(16),a7@-
|                movl    a7@(16),a7@-
|                movl    a7@(16),a7@-
|                bsrs    _d_cmp
|                bccs    _d_min+0x22
|                bclr    #0x7,a7@(4)
|                cmpl    #0x7ff00000,a7@(4)
|                bles    _d_min+0x30
|                movl    a7@(4),a7@(12)
|                movl    a7@(8),a7@(16)
|                movl    a7@+,a7@
|                movl    a7@+,a7@
|                rts

_d_cmp:         moveml  #0xf800,a7@-
|                moveml  a7@(24),#0x000f	|wrong order of load.
		moveml	a7@(24),#0x000f
|		moveml	a7@(32),#0x0003
_d_cmp_redo:
                movl    d2,d4
                andl    d0,d4
                bmis    _d_cmp_bothmi
		cmpl	#0x80000000,d0
		bnes	_d_cmp_check1
		tstl	d1
		bnes	_d_cmp_nbothmi
		clrl	d0
		bras	_d_cmp_nbothmi
_d_cmp_check1:
		cmpl	#0x80000000,d2
		bnes	_d_cmp_nbothmi
		tstl	d3
		bnes	_d_cmp_nbothmi
		clrl	d2
		bras	_d_cmp_nbothmi
_d_cmp_bothmi:
                exg     d0,d2
                exg     d1,d3
_d_cmp_nbothmi:
|
|	GB (SGI) add code to check for Nan as input....
|
		movl	d0,d4
		bclr	#31,d4
		cmpl	#0x7ff00000,d4
		blt	_d_cmp_chkn2
		bgt	_d_cmp_d0Nan
		tstl	d1
		bne	_d_cmp_d0Nan
_d_cmp_chkn2:
		movl	d2,d4
		bclr	#31,d4
		cmpl	#0x7ff00000,d4
		blt	_d_cmp_docomp
		bgt	_d_cmp_d2Nan
		tstl	d3
		bne	_d_cmp_d2Nan
_d_cmp_docomp:
                cmpl    d2,d0
                bnes    _d_cmp_gotcmp
                movl    d1,d4
                subl    d3,d4
                beqs    _d_cmp_gotcmp
                roxrl   #0x1,d4
                andb    #0xa,cc
_d_cmp_gotcmp:
                andb    #0xe,cc
|
|		just return the values and unwind the stack
|
                moveml  a7@+,#0x001f
		rts
_d_cmp_d2Nan:
		clrl	sp@-		|stack marker. good number was in d0.
		movl	d0,d2		|d2 now has good number
		movl	d1,d3
		bras	_d_cmp_goraise
_d_cmp_d0Nan:
		movl	#1,sp@-
_d_cmp_goraise:
		movl	#INVALID_OP_A,sp@-	| Nan operand
		movl	#CMP,sp@-
		jbsr	__lraise_fperror
		addql	#8,sp
|
|	result is compared to the other operand.
|
		tstl	sp@+		|if zero, must exchange
		bne	_d_cmp_redo
		exg	d0,d2
		exg	d1,d3
		bra	_d_cmp_redo
|                movw    sr,a7@(34)
|                lsll    #0x1,d0
|                lsll    #0x1,d2
|                cmpl    d2,d0
|                bccs    _d_cmp_cmp4
|                exg     d0,d2
|_d_cmp_cmp4:
|                cmpl    #0xffe00000,d0
|                blss    _d_cmp_cmp6
|                movw    #0x1,a7@(34)
|                bras    _d_cmp_cmp8
|_d_cmp_cmp6:
|                orl     d1,d0
|                orl     d2,d0
|                orl     d3,d0
|                bnes    _d_cmp_cmp8
|                movw    #0x4,a7@(34)
|_d_cmp_cmp8:
|                moveml  a7@+,#0x001f
|                movl    a7@+,a7@(12)		|dont pop the stack
|                addw    #0xa,a7
|		movw	a7@(14),a7@-		|move the cc to the t.o.s.
|                rtr

_f_div:
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
|
_d_mul:
|		moveml  #0xff00,a7@-		|mul is commutative.
|               moveml  a7@(36),#0x000f		|parm order doesn't matter.
		moveml	#0x3f00,a7@-
		moveml	a7@(28),#0x000f
_dr_domul:
                movl    d0,d5
                eorl    d2,d5
                asll    #0x1,d0
                asll    #0x1,d2
|
|	gb - if we have Nan*Inf and msl(Nan) == msl(INF),
|	     the INF will be selected as the larger here, 
|	     and will not be checked at nanop.  Thus, if
|	     msl(a) == msl(b), we swap if lsl(a) == 0.
|	     This seems inappropriate, but it guards this
|	     special case.
|	     
| ORDER OPERANDS (AT LEAST EXPONENTS)
|       CMP.L   D2,D0
|;        BLS.S   ESWAP
|       bcs.s	ESWAP
|	tst.l	d3
|	bne.s	ESWAP
|GOSWAP
                cmpl    d2,d0
|                blss    _d_mul_eswap
		bcss	_d_mul_eswap
		tstl	d3
		bnes	_d_mul_eswap
                exg     d0,d2
                exg     d1,d3
_d_mul_eswap:
                bsr     _d_exte
                movw    d6,d5
                movl    d5,d6
                addw    d7,d6
                cmpw    #0x7ff,d5
                beq     _d_mul_badop
                tstw    d7
                beq     _d_mul_ufl
                bset    #0x1f,d0
_d_mul_back:
                bset    #0x1f,d2
                moveml  #0xf000,a7@-
                movemw  a7@,#0x000f
                movw    a7@(8),d5
                mulu    d5,d0
                mulu    d5,d1
                mulu    d5,d2
                mulu    d5,d3
                clrl    d7
                movw    a7@(4),d5
                mulu    a7@(10),d5
                addl    d5,d3
                addxl   d7,d1
                movw    a7@(2),d5
                mulu    a7@(12),d5
                addl    d5,d3
                addxl   d7,d1
                movw    a7@,d5
                mulu    a7@(14),d5
                addl    d5,d3
                addxl   d7,d1
                movw    a7@(2),d5
                mulu    a7@(10),d5
                addl    d5,d2
                addxl   d7,d0
                movw    a7@,d5
                mulu    a7@(12),d5
                addl    d5,d2
                addxl   d7,d0
                swap    d0
                movw    a7@,d5
                mulu    a7@(10),d5
                addl    d5,d1
                addxw   d7,d0
                swap    d0
                addw    #0x10,a7
                movw    d1,d3
                swap    d3
                clrw    d1
                swap    d1
                addl    d2,d3
                addxl   d7,d0
                addl    d1,d0
                movl    d0,d2
                subw    #0x3fe,d6
|
|	check result for overflow.
|
		cmpw	#0x7FF,d6
		blts	_d_mul_gobuild
		movl	#OVERFLOW,d1
		bra	_d_mul_goraise
_d_mul_gobuild:
                bsr     _d_nrcp
_d_mul_msign:
                roxll   #0x1,d6
                roxrl   #0x1,d2
|
|	here, if the result is denormalized, UNDERFLOW_A should be
|	signalled.
|
_d_mul_mexit:
|                moveml  #0x000c,a7@(44)
|                moveml  a7@+,#0x00ff
		movl	d2,d0
		movl	d3,d1
_d_mul_mexit1:
		moveml	a7@+,#0x00fc	|restore regs
|                movl    a7@+,a7@	|dont pop the stack
|                movl    a7@+,a7@
|
|	And move the result back into d0/d1
|
|		movl	a7@(12),d0
|		movl	a7@(16),d1
                rts
_d_mul_badop:
|
|	larger operand was extreme value: either NaN or INF.
|
|	we need to do the following:
|
|	    if the larger mantissa was not zero, we have a Nan as operand.
|		signal it.
|
		tstl	d2
		bnes	_d_mul_nanop
		tstl	d3
		beqs	_d_mul_ckinfz
|
|	Nan as operand.... signal it.
|
_d_mul_nanop:
		movl	#INVALID_OP_A,d1
		bra	_d_mul_goraise
|
|	last, raise an illegal operation error for INF*0.
|
_d_mul_ckinfz:
		movl  	d2,d5          	|LARGER MANTISSA, IF IT IS NAN, USE IT
        	orw    	d7,d5		|smaller exp
        	orl    	d0,d5
        	orl    	d1,d5		|smaller value
		bnes  	_d_mul_realofl		
_d_mul_infzero:
|
|	INF * 0...
|
		movl	#INVALID_OP_C,d1
		bra	_d_mul_goraise
_d_mul_goraise:
		movl	d1,sp@-
		movw	#MUL,sp@-
		clrw	sp@-
		jbsr	__lraise_fperror
		addql	#8,sp
| 
|	returned.  Result of operation is in d0/d1.
|
		bra	_d_mul_mexit1
|
|	True overflow.....
|
_d_mul_realofl:
                movw    #0x7ff,d6
                bsr     _d_usel
                bras    _d_mul_msign
_d_mul_ufl:
                movl    d0,d7
                orl     d1,d7
                beqs    _d_mul_signed0
_d_mul_normu:
                subqw   #0x1,d6
                lsll    #0x1,d1
                roxll   #0x1,d0
                bpls    _d_mul_normu
                addqw   #0x1,d6
                bra     _d_mul_back
_d_mul_gennan:
                movl    #0x7ff00002,d2
                bras    _d_mul_mexit
_d_mul_signed0:
                clrl    d2
                clrl    d3
                bra    _d_mul_msign

__lfloat:
_i_2_d:
|	        movl    a7@,a7@-
|                movl    a7@(8),a7@(4)
                movl	a7@(4),d0
_ir_2_d:
		movl	d0,d1
		moveml  #0x3e00,a7@-
|                movl    a7@(24),d1
                bsr     _i_unpk
                bras    _i_d_xit

_f_2_d:
		movl	a7@(4),d0
_fr_2_d:         
|		movl    a7@,a7@-
|               movl    a7@(8),a7@(4)
                moveml  #0x3e00,a7@-
|               movl    a7@(24),d1
		movl	d0,d1
                bsr     _f_unpk			| regs seem ok (GB)
_i_d_xit:
                bsr     _d_pack
|               moveml  #0x0003,a7@(24)		|not enuf room to move
						|fn result to stack.
                moveml  a7@+,#0x007c
|
|	function result already in d0/d1, but there is an extra
|	longword below the return address we must reclaim.
|
|		movl	a7@(4),d0
|		movl	a7@(8),d1
|		movl	a7@+,a7@
                rts

_d_2_i:
__lfix:
                moveml  a7@(4),#0x0003
_dr_2_i:
	        moveml  #0x3800,a7@-
                bsrs    _d_unpk
                bsr     _i_pack
                bras    _f_d_xit

_d_2_f:	
		moveml	a7@(4),#0x0003
_dr_2_f:
		moveml  #0x3800,a7@-
                bsrs    _d_unpk
                bsr     _f_pack		| regs saved -ok no changes (GB)
_f_d_xit:
                movl    d1,d0
                moveml  a7@+,#0x001c
|                movl    a7@+,a7@	|dont pop the stack
|
|	move the function result to d0
|
|		movl	a7@(8),d0
                rts

|_i_idnin:       moveml  a7@(4),#0x0003
|                bsrs    _d_unpk
|                bsr     _g_nint
|                bsr     _i_pack
|                movl    d1,a7@(12)
|                movl    a7@+,a7@
|                movl    a7@+,a7@
|                rts
|
|_d_dnint:       moveml  a7@(4),#0x0003
|                bsrs    _d_unpk
|                bsr     _g_nint
|                bras    _d_dint+0xc
|
|_d_dint:        moveml  a7@(4),#0x0003
|                bsrs    _d_unpk
|                bsr     _g_int
|                bsrs    _d_pack
|                moveml  #0x0003,a7@(12)
||dont want the stack popped here,either.
|                movl    a7@+,a7@
|                movl    a7@+,a7@
|                rts

|_pd_unpk:       movl    a7@(8),a0
|		 moveml	 #0x1c,_pf_regs		(GB)
|                moveml  a0@,#0x0003
|                bsrs    _d_unpk
|
|	GB - #56 u_stor now expects registers d2-d4 to have been
|		 saved in the data location _pf_regs.
|
|                bra     _u_stor

_d_unpk:        movl    #0xfff00000,d2
                movl    d0,d3
                swap    d3
                andl    d0,d2
                eorl    d2,d0
                movl    d1,d4
                orl     d0,d4
                lsll    #0x1,d2
                bnes    _d_unpk+0x24
                movb    #0x1,d3
                tstl    d4
                beqs    _d_unpk+0x4a
                movb    #0x2,d3
                bras    _d_unpk+0x4a
                swap    d2
                lsrw    #0x5,d2
                cmpw    #0x7ff,d2
                bnes    _d_unpk+0x40
                movw    #0x6000,d2
                movb    #0x4,d3
                tstl    d4
                beqs    _d_unpk+0x4e
                movb    #0x5,d3
                bras    _d_unpk+0x4e
                bset    #0x14,d0
                subqw   #0x1,d2
                movb    #0x3,d3
                subw    #0x432,d2
                rts

|_pd_pack:       moveml	#0x1c,_pf_regs		| GB - dont clobber d2-d4
|		 bsr     _u_load
|                bsrs    _d_pack
|                moveml  #0x0003,a0@
|		 moveml	_pf_regs,#0x1c		| restore them
|                rts

_d_pack:        cmpb    #0x4,d3
                blts    _d_pack+0x12
                orl     d1,d0
                lsll    #0x1,d0
                orl     #0xffe00000,d0
                bras    _d_pack+0x28
                addw    #0x43e,d2
                exg     d0,d2
                exg     d0,d6
                exg     d1,d3
                bsrs    _d_norm
                bsr     _d_rcp
                movl    d0,d6
                movl    d2,d0
                exg     d3,d1
                lslw    #0x1,d3
                roxrl   #0x1,d0
                rts

_d_exte:        moveq   #0xb,d4
                roll    d4,d0
                roll    d4,d2
                roll    d4,d1
                roll    d4,d3
                movl    #0x7ff,d6
                movl    d6,d7
                andl    d2,d6
                eorl    d6,d2
                movl    d7,d4
                andl    d3,d4
                eorl    d4,d3
                lsrl    #0x1,d2
                orl     d4,d2
                movl    d7,d4
                andl    d0,d7
                eorl    d7,d0
                andl    d1,d4
                eorl    d4,d1
                lsrl    #0x1,d0
                orl     d4,d0
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

|_d_sign:        roxll   a7@(12)
|                roxll   a7@(4)
|                roxrl   a7@(12)
|                movl    a7@(16),a7@(24)
|                movl    a7@(12),a7@(20)
|                movl    a7@+,a7@(12)
|                addw    #0xc,a7
|                rts
|
		.data
		.even
_div_mod_op:	.word 0
		.text

_d_div:		
__sw_d_div:		
		moveml  #0x3f00,a7@-
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

_d_mod:
|	All this junk is unnecessary for us.....
| 	 	 movl    a7@+,a7@(16)
|                bsrs    _d_mod_dmod
|                movl    a7@(4),a7@(12)
|                movl    a7@(8),a7@(4)
|                movl    a7@+,a7@(4)
|                rts
_d_mod_dmod:
                moveml  #0x3f00,a7@-		
|               moveml  a7@(36),#0x000f		|wrong order parm load again
		moveml 	a7@(28),#0x000c
		moveml	a7@(36),#0x0003
|
|	We altered the stack offsets on entry from what they
|	used to be.  Use the new stack offsets.
|
|		moveml 	a7@(36),#0x000c
|		moveml	a7@(44),#0x0003

		movw	#MOD,_div_mod_op
                movl    d2,d5
                moveq   #0xffffffff,d4
                bsr     _d_div_extrem
                movw    d6,d4
                subqw   #0x1,d7
                subw    d7,d4
                bles    _d_mod_usetop
                lsll    #0x1,d1
                roxll   #0x1,d0
                movw    d7,d6
                subqw   #0x1,d6
                bsrs    _d_div_shsub
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
|
|	_pf_unpk and _pf_pack are Pascal-callable.  Again, the 
|	inverse order of loading parameters on the stack is used
|	by SVS Pascal, so they have been changed.  Otherwise,
|	everything appears OK.
|

|_pf_unpk:       movl    a7@(8),d1
_pf_unpk:       movl    a7@(4),d1	|the value to unpack is the first arg
		moveml	#0x001C,_pf_regs | save d2-d4
                bsrs    _f_unpk
|
|	GB - bug #56 11/3/83
|		_u_stor now restores registers d2-d4 from 
|		the data location _pf_regs.
|
|
		bras	_u_stor
	
		.data
		.even
_pf_regs:	.space	16		| space for four registers
		.text

|_u_stor:        movl    a7@(4),a0
_u_stor:        movl    a7@(8),a0	|the address to unpack it to is second
                swap    d2
                movw    d3,d2
                moveml  #0x0007,a0@
|                movl    a7@+,a0	|simple return works since we
|                addql   #0x8,a7	|dont pop the stack!
|                jmp     a0@
		moveml	_pf_regs,#0x001C | restore d2-d4 GB #56
		rts

_f_unpk:        movl    d1,d3
                swap    d3
                lsll    #0x1,d1
                roll    #0x8,d1
                clrw    d2
                movb    d1,d2
                bnes    _f_unpk+0x1c
                movb    #0x1,d3
                tstl    d1
                beqs    _f_unpk+0x40
                movb    #0x2,d3
                bras    _f_unpk+0x40
                cmpb    #0xff,d2
                bnes    _f_unpk+0x36
                movw    #0x6000,d2
                clrb    d1
                movb    #0x4,d3
                tstl    d1
                beqs    _f_unpk+0x44
                movb    #0x5,d3
                bras    _f_unpk+0x44
                movb    #0x1,d1
                subqw   #0x1,d2
                movb    #0x3,d3
                subw    #0x95,d2
                rorl    #0x1,d1
                lsrl    #0x8,d1
                clrl    d0
                rts

|_u_load:        movl    a7@(8),a0
_u_load:        movl    a7@(12),a0		|the address of the unpacked
						|record is the second arg.
						|note two return addrs on stk
                moveml  a0@,#0x0007
                movw    d2,d3
                swap    d2
|	         movl    a7@(12),a0
                movl    a7@(8),a0		|addr of return value is 
						|first (2 ret addrs)
                rts
|
_pf_pack:	moveml	#0x1c,_pf_regs	|GB (#56) dont clobber regs.
	        bsrs    _u_load
                bsrs    _f_pack
                movl    d1,a0@
		moveml	_pf_regs,#0x1c	|GB (#56)
|                movl    a7@+,a0
|                addql   #0x8,a7	|dont pop the stack!
|                jmp     a0@
		rts
|
_f_pack:        movw    d2,d4
                cmpb    #0x4,d3
                blts    _f_pack_spk0
                orl     d0,d1
                orl     #0x7f800000,d1
                lsll    #0x1,d1
                bras    _f_pack_spksgn
_f_pack_spk0:
                tstl    d0
                beqs    _f_pack_spk2
_f_pack_spk1:
                movb    d0,d1
                addqw   #0x8,d4
                rorl    #0x8,d1
                lsrl    #0x8,d0
                bnes    _f_pack_spk1
_f_pack_spk2:
                movl    d1,d0
                beqs    _f_pack_spksgn
                bmis    _f_pack_spk5
_f_pack_spk4:
                subqw   #0x1,d4
                lsll    #0x1,d0
                bpls    _f_pack_spk4
_f_pack_spk5:
                addw    #0x9e,d4
                bsrs    _f_rcp
|
|		check the result of rcp for overflow
|
		cmpb	#0xff,d0		| is it inf/nan?
		beq	_f_pack_goraise
                rorl    #0x8,d0
                movl    d0,d1
_f_pack_spksgn:
                lslw    #0x1,d3
                roxrl   #0x1,d1
                rts
|
|		overflow!
|
_f_pack_goraise:
		movl	#OVERFLOW,sp@-
		movl	#PRECISION,sp@-
		jbsr	__raise_fperror
		addql	#8,sp
		movl	d0,d1		| result in correct register
		rts

|_f_sign:        roxll   a7@(8)
|                roxll   a7@(4)
|                roxrl   a7@(8)
|                movl    a7@(8),a7@(12)
|                movl    a7@+,a7@
|                movl    a7@+,a7@
|                rts

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

_float:
_i_2_f:
		movl	a7@(4),d0
_ir_2_f:
		moveml  #0x3800,a7@-
                movl    d0,d1
                bsrs    _i_unpk
                bsr     _f_pack			| no changes (regs saved) GB
                movl    d1,d0
                moveml  a7@+,#0x001c
|
|	result in d0.
|	
|		movl	a7@(4),d0
                rts

_fix:
_f_2_i:
		movl	a7@(4),d0
_fr_2_i:
		moveml  #0x3800,a7@-
                movl    d0,d1
                bsr     _f_unpk			| regs seem ok
                bsrs    _i_pack
                movl    d1,d0
                moveml  a7@+,#0x001c
|
|	result in d0.
|	
|		movl	a7@(4),d0
                rts

_i_unpk:        clrw    d2
                moveq   #0x3,d3
                tstl    d1
                bpls    _i_unpk+0xe
                negl    d1
                bset    #0xf,d3
                clrl    d0
                rts

_i_pack:        cmpb    #0x4,d3
                bges    _i_pack_bigi
                bsrs    _g_nsh
                orw     d2,d0
                tstl    d0
                bnes    _i_pack_bigi
                tstl    d1
                bmis    _i_pack_bigi
                tstw    d3
                bpls    _i_pack_ipz
                negl    d1
_i_pack_ipz:
                rts
_i_pack_bigi:
		movl	#OVERFLOW,sp@-
		movl	#FIX,sp@-
		jbsr	__raise_fperror
		addql	#8,sp
                movl    d0,d1
                rts
|
|	GB 11/3/83 (#56) - save d2-d4 in pf_regs.  _u_stor restores them.
|
|_pg_nsh:	 moveml  #0x001C,_pf_regs | save d2-d4 (GB)
|		 bsr     _u_load
|                bsrs    _g_nsh
|                bra     _u_stor

_g_nsh: moveq   #0x20,d4
                tstw    d2
                bmis    _g_nsh+0x2c
                cmpw    d4,d2
                blts    _g_nsh+0x14
                tstl    d0
                bnes    _g_nsh+0x52
                subw    d4,d2
                exg     d1,d0
                bras    _g_nsh+0x6
                roll    d2,d0
                roll    d2,d1
                moveq   #0x1,d4
                asll    d2,d4
                subql   #0x1,d4
                movl    d0,d2
                andl    d4,d2
                bnes    _g_nsh+0x52
                andl    d1,d4
                eorl    d4,d1
                orl     d4,d0
                bras    _g_nsh+0x50
                negw    d2
                cmpw    d4,d2
                blts    _g_nsh+0x3a
                movl    d0,d1
                clrl    d0
                subw    d4,d2
                bras    _g_nsh+0x2e
                moveq   #0x1,d4
                asll    d2,d4
                subql   #0x1,d4
                notl    d4
                andl    d4,d1
                andl    d0,d4
                eorl    d4,d0
                orl     d0,d1
                rorl    d2,d4
                rorl    d2,d1
                movl    d4,d0
                clrl    d2
                rts

|_g_nint:        tstw    d2
|                bpls    _g_nint+0x10
|                addqw   #0x1,d2
|                bsrs    _g_nsh
|                asrl    #0x1,d0
|                roxrl   #0x1,d1
|                addxl   d2,d1
|                addxl   d2,d0
|                rts
|
|_i_nint:        movl    a7@(4),d1
|                bsr     _f_unpk		| note regs prob (GB)
|                bsrs    _g_nint
|                bsr     _i_pack
|                movl    d1,a7@(8)
|                movl    a7@+,a7@
|                rts
|
|_f_anint:       movl    a7@(4),d1
|                bsr     _f_unpk		| ditto
|                bsrs    _g_nint
|                bras    _f_aint+0xa
|
|_g_int: tstw    d2
|                bmi     _g_nsh
|                rts

|_f_aint:        movl    a7@(4),d1
|                bsr     _f_unpk		| (see below)
|                bsrs    _g_int
|                bsr     _f_pack		| note - need to save regs(GB)
						| if this is ever uncommented
|                movl    d1,a7@(8)
|                movl    a7@+,a7@
|                rts
|
|
|	The calling sequence for _f_mod has been altered so that
|	the dummy slot for the return value is not on the t.o.s.
|	Also, d0 is not saved/restored, as the function result 
|	happens to be in d0 on exit.
|

_f_mod:
|	 	moveml  #0xdc00,a7@-
	 	moveml  #0x5e00,a7@-	|dont save d0.save d6 instead
|                movl    a7@(28),d0	|wrong order of parm load. changed
|                movl    a7@(24),d1
                movl    a7@(28),d1
                movl    a7@(24),d0

                lsll    #0x1,d0
                scs     d5
                lsll    #0x1,d1
                roll    #0x8,d0
                roll    #0x8,d1
                clrw    d3
                clrw    d4
                movb    d0,d3
                movb    d1,d4
                addqb   #0x1,d3
                subqw   #0x1,d3
                bles    _f_mod_toperr
                movb    #0x1,d0
_f_mod_backtop:
                addqb   #0x1,d4
                subqw   #0x1,d4
                bles    _f_mod_boterr
                movb    #0x1,d1
_f_mod_backbot:
                rorl    #0x3,d0
                rorl    #0x2,d1
                subw    d4,d3
                blts    _f_mod_mod7
_f_mod_lsub:
                addl    d0,d0
                subl    d1,d0
                dbmi    d3,_f_mod_lsub
                dbpl    d3,_f_mod_ladd
                bras    _f_mod_loopend
_f_mod_ladd:
                addl    d0,d0
                addl    d1,d0
                dbpl    d3,_f_mod_ladd
                dbmi    d3,_f_mod_lsub
_f_mod_loopend:
                bpls    _f_mod_signok
                addl    d1,d0
_f_mod_signok:
                asll    #0x1,d0
                bmis    _f_mod_mnorm
                beqs    _f_mod_mrpk
_f_mod_mnorml:
                asll    #0x1,d0
                dbmi    d4,_f_mod_mnorml
                dbpl    d4,_f_mod_mnorm
                bras    _f_mod_mnorm
_f_mod_mod7:
                addw    d3,d4
                asll    #0x2,d0
_f_mod_mnorm:
                bsr     _f_rcp
_f_mod_mrpk:
                rorl    #0x8,d0
                roxrb   #0x1,d5
                roxrl   #0x1,d0
|
|	Here, at the exit interface, the result is in 
|	the correct register.  Rather than monkeying
|	around, we will simply make this instruction
|	clobber one of the arguments and we have NOT saved
|	d0, nor will we restore it.
_f_mod_mexit:
|                movl    d0,a7@(32)
                movl    d0,a7@(28)
                moveml  a7@+,#0x007a	|d0 not saved/restored.d6 instead.
|                movl    a7@+,a7@	|dont pop the stack
|                movl    a7@+,a7@	|
                rts
_f_mod_toperr:
                bnes    _f_mod_modnan
                roll    #0x1,d0
                bras    _f_mod_backtop
_f_mod_boterr:
                bnes    _f_mod_botbig
_f_mod_normb:
                subqw   #0x1,d4
                roll    #0x1,d1
                bhis    _f_mod_normb
|
|	if equal, the bot was zero
|
                bnes    _f_mod_botok
		movl	#INVALID_OP_E1,d1
		bras	_f_mod_goraise
_f_mod_botok:
                addqw   #0x1,d4
                bras    _f_mod_backbot
_f_mod_botbig:
|
|	if d1 == 0xff, bot is inf. This is ok.  It will return the
|	result top.
|
                cmpl    #0xff,d1
                beqs    _f_mod_mrpk
|	
|	bot is Nan.
|
		movl	#INVALID_OP_A,d1
		bras	_f_mod_goraise
_f_mod_modnan:
|	
|	top is either INF or Nan.  Decide
|
		movl	#INVALID_OP_A,d1	|assume Nan
		cmpl	#0x000000ff,d0
		bnes	_f_mod_goraise
		movl	#INVALID_OP_E2,d1
_f_mod_goraise:
		movl	d1,sp@-			|type
		movl	#MOD,sp@-		|op
		jbsr	__raise_fperror
		addql	#8,sp
		bras	_f_mod_mexit
|                rorl    #0x8,d0
|                rorl    #0x8,d1
|                cmpl    d0,d1
|                bcss    _f_mod_bigtop
|                exg     d0,d1
|_f_mod_bigtop:
|                lsrl    #0x1,d0
|                cmpl    #0x7f800000,d0
|                bgts    _f_mod_mexit
|_f_mod_gennan:
|                movl    #0x7f800005,d0
|                bras    _f_mod_mexit

|
|	_f_mul expects arguments in registers (_f_mul(d0,d1))
|
_f_mul:
		moveml	a7@(4),#0x0003
_fr_mul: 	
		moveml  #0x3c00,a7@-		|mul is commutative. no order
                movl    d0,d5
                eorl    d1,d5
                asll    #0x1,d0
                asll    #0x1,d1
                cmpl    d1,d0
                blss    _f_mul_eswap
                exg     d0,d1
_f_mul_eswap:
                roll    #0x8,d0
                roll    #0x8,d1
                clrw    d4
                movb    d0,d4
                clrw    d3
                movb    d1,d3
                addw    d3,d4
                cmpb    #0xff,d1
                beqs    _f_mul_badop
                tstb    d0
                jeq	_f_mul_ufl
                movb    #0x1,d0
                rorl    #0x1,d0
_f_mul_back:
                movb    #0x1,d1
                rorl    #0x1,d1
                movw    d0,d2
                movw    d1,d3
                swap    d0
                swap    d1
                mulu    d0,d3
                mulu    d1,d2
                mulu    d1,d0
                addl    d2,d3
                addxb   d3,d3
                andw    #0x1,d3
                swap    d3
                addl    d3,d0
                subw    #0x7e,d4
|
|	check the result for overflow
|
		cmpw	#0xff,d4
		blts	_f_mul_gobuild
		movl	#OVERFLOW,d1
		bras	_f_mul_goraise
_f_mul_gobuild:
                bsr     _f_rcp
_f_mul_mbuild:
                rorl    #0x8,d0
                roxll   #0x1,d5
                roxrl   #0x1,d0
|
|	here, UNDERFLOW_A must be signaled if 
|	the result is denormalized.  Check d0 for this.
|	This is not currently done for efficiency reasons.
|
|		movl	d0,d1
|		andl	#0x7f800000,d1
|		bnes	_f_mul_mexit
|		tstl	d0
|		beqs	_f_mul_mexit
|
|	UNDERFLOW_A signal.
|
|		movl	#UNDERFLOW_A,d1
|		bras	_f_mul_goraise
_f_mul_mexit:
                moveml  a7@+,#0x003c
|
|	move the result to d0 for the return
|
                rts
|
|		_f_mul exception processing.
|	
|		registers:
|			d0 - smaller operand in form MMMMMMEE
|			d1 - larger operand in form MMMMMMEE
|
_f_mul_badop:
|
|		at this point, we only know that overflow was
|		GENERATED, not that the operands were invalid.
|		check first that the larger exp was 0xff
|
		cmpb	#0xff,d1		|is larger exp Nan or Inf?
		beq	_f_mul_ofl2		|yes...invalid operand
		movl	#OVERFLOW,d1
		bra	_f_mul_goraise
_f_mul_ofl2:
                clrb    d1
                tstl    d1			| is larger Nan?
                beqs    _f_mul_ofl1		| 
| 		
|		larger is Nan
|
		movl	#INVALID_OP_A,d1
		bra	_f_mul_goraise
_f_mul_ofl1:
|
|		larger is infinity....  INF * 0
|
                tstl    d0
                bnes    _f_mul_mni		| inf * x!=0, result=inf
		movl	#INVALID_OP_C,d1
_f_mul_goraise:
		movl	d1,sp@-
		movl	#MUL,sp@-
		jbsr	__raise_fperror
		addql	#8,sp
		jra	_f_mul_mexit		| leave result in d0
|
|
_f_mul_mni:
                movb    #0xff,d1
                movl    d1,d0
                bras    _f_mul_mbuild
_f_mul_ufl:
                tstl    d0
                beqs    _f_mul_mbuild
                jmi    _f_mul_back
_f_mul_normden:
                subqw   #0x1,d4
                lsll    #0x1,d0
                bpls    _f_mul_normden
                jra    _f_mul_back
_f_mul_gennan:
                movl    #0x7f800002,d0
                bras    _f_mul_mexit
_f_mul_signed0:
                clrl    d0
                bras    _f_mul_mbuild
|
|
|	indirect  routines added!!!
|
|

_fr_iadd:
|	enter with the arg in d0, the address in a0.
|	store the value indirect on exit, and return it
|	in d0 as well.
|
|	the wrong argument is in d0, but this op is commutative!
|
		movl	a0@,d1		|get the arg
		movl	a0,sp@-		|save a0
		bsr	_fr_add		|do operation
		movl	sp@+,a0
		movl	d0,a0@		|and restore the value
		rts
_fr_isub:
|	enter with the arg in d0, the address in a0.
|	store the value indirect on exit, and return it
|	in d0 as well.
|
|	the wrong arg is in d0, so we have to exchange them.
|
		movl	a0@,d1		|get the arg
		movl	a0,sp@-		|save a0
		exg	d0,d1
		bsr	_fr_sub		|do operation
		movl	sp@+,a0
		movl	d0,a0@		|and restore the value
		rts
_fr_imul:
|	enter with the arg in d0, the address in a0.
|	store the value indirect on exit, and return it
|	in d0 as well.
|
|	the wrong argument is in d0, but this op is commutative!
|
		movl	a0@,d1		|get the arg
		movl	a0,sp@-		|save a0
		bsr	_fr_mul		|do operation
		movl	sp@+,a0
		movl	d0,a0@		|and restore the value
		rts
__sw_f_idiv:
		movl	sp@(4),a0
		movl	sp@(8),d0
_fr_idiv:
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

|
|	double precision versions of the indirect float routines...
|
|
_dr_iadd:
|	enter with the arg in d0, the address in a0.
|
		movl	a0,sp@-		|save a0
		pea	_dr_iadd_rtn	|put return address on stack	
		moveml	#0x3f00,sp@-	|save d2-d7
		movl	a0@+,d2		|get the arg
		movl	a0@,d3
		bra	_dr_doadd	|do operation
_dr_iadd_rtn:
		movl	sp@+,a0
		movl	d0,a0@+		|and restore the value
		movl	d1,a0@
		rts
_dr_isub:
|	enter with the arg in d0, the address in a0.
|
		movl	a0,sp@-		|save a0
		pea	_dr_isub_rtn	|put return address on stack
		moveml	#0x3f00,sp@-	|save d2-d7
		movl	d0,d2
		movl	d1,d3
		bchg	#31,d2
		movl	a0@+,d0		|get the arg
		movl	a0@,d1
		bra	_dr_doadd	|do operation
_dr_isub_rtn:
		movl	sp@+,a0
		movl	d0,a0@+		|and restore the value
		movl	d1,a0@
		rts
_dr_imul:
|	enter with the arg in d0, the address in a0.
|
		movl	a0,sp@-		|save a0
		pea	_dr_imul_rtn
		moveml	#0x3f00,sp@-	|save d2-d7
		movl	a0@+,d2		|get the arg
		movl	a0@,d3
		bra	_dr_domul	|do operation
_dr_imul_rtn:
		movl	sp@+,a0
		movl	d0,a0@+		|and restore the value
		movl	d1,a0@
		rts
__sw_d_idiv:
		movl	sp@(4),a0
		movl	sp@(8),d0
		movl	sp@(12),d1
_dr_idiv:
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
