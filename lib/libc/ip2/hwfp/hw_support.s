|
|	support.s - Those portions of the old float.s which
|		    are necessary to support the single precision
|		    versions of the Syntactics floating point library
|		    once it is using the Juniper fpa.  These include
|		    the routines to pack and unpack a floating point
|		    number (pf_pack, pf_unpk).
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
INVALID_OP_G	=0x170
INVALID_OP_H	=0x180

DIVZERO		=0x200
OVERFLOW	=0x300

	.globl	_pf_unpk,_pf_pack
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
                bnes    _f_unpk1c
                movb    #0x1,d3
                tstl    d1
                beqs    _f_unpk40
                movb    #0x2,d3
                bras    _f_unpk40
_f_unpk1c:
                cmpb    #0xff,d2
                bnes    _f_unpk36
                movw    #0x6000,d2
                clrb    d1
                movb    #0x4,d3
                tstl    d1
                beqs    _f_unpk44
                movb    #0x5,d3
                bras    _f_unpk44
_f_unpk36:
                movb    #0x1,d1
                subqw   #0x1,d2
                movb    #0x3,d3
_f_unpk40:
                subw    #0x95,d2
_f_unpk44:
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
                bmis    _f_rcp8
                subqw   #0x1,d4
                lsll    #0x1,d0
_f_rcp8:
                tstw    d4
                bgts    _f_rcp1a
                cmpw    #0xffffffe8,d4
                blts    _f_rcp34
                negb    d4
                addqb   #0x1,d4
                lsrl    d4,d0
                clrw    d4
_f_rcp1a:
                addl    #0x80,d0
                bccs    _f_rcp26
                roxrl   #0x1,d0
                addqw   #0x1,d4
_f_rcp26:
                cmpw    #0xff,d4
                bges    _f_rcp38
                lsll    #0x1,d0
                scs     d0
                andb    d4,d0
                rts
_f_rcp34:
                clrl    d0
                rts
_f_rcp38:
                movl    #0xff,d0
                rts

