	.data
	.globl	__Origin_
__Origin_:
	.word	21369
	.word	29556
	.word	25965
	.word	8278
	.word	0
_ID:
	.word	16424
	.word	9001
	.word	26223
	.word	29300
	.word	30062
	.word	25902
	.word	25353
	.word	12590
	.word	12544
	.comm	_line,0x1f4
	.comm	_bline,0x1f4
	.text
	.globl	_main
_main:
	link	a6,#-.F1
	movl	a5,sp@-
	movl	d2,sp@-
| A1 = 8
	pea	.L35
	pea	.L34
	jbsr	_fopen
	addql	#8,sp
	movl	d0,a6@(-12)
	tstl	d0
	jne	.L36
	pea	.L38
	jbsr	_printf
	addql	#4,sp
	pea	0x1
	jbsr	_exit
	addql	#4,sp
.L36:
	pea	a6@(0xfffffff8)
	jbsr	_time
	addql	#4,sp
	jbsr	_getpid
	moveq	#0x10,d1
	movl	a6@(-8),d2
	asrl	d1,d2
	addl	d2,d0
	addl	a6@(-8),d0
	movl	d0,sp@-
	jbsr	_srand
	addql	#4,sp
	.data
	.even
.L43:
	.long   0x3f800000
	.text
	movl	.L43,a6@(-4)
	jra	.L46
.L20001:
	.data
	.even
.L50:
	.long   0x46fffe00
	.text
	movl	a6@(-4),d1
	movl	.L50,d0
	jbsr	_fr_div
	movl	d0,d1
	moveml	#0x4000,sp@-
	moveml	#0x4000,sp@-
	jbsr	_rand
	moveml	sp@+,#0x2
	jbsr	_ir_2_f
	
	
	jbsr	_fr_cmp
	jge	.L49
	pea	_line
	pea	_bline
	jbsr	_strcpy
	addql	#8,sp
.L49:
	.data
	.even
.L52:
	.long   0x3f800000
	.text
	lea	a6@(0xfffffffc),a0
	movl	.L52,d0
	jbsr	_fr_iadd
.L46:
	movl	a6@(-12),sp@-
	pea	0x1f4
	pea	_line
	jbsr	_fgets
	addw	#12,sp
	movl	d0,a5
	cmpl	#0x0,a5
	jne	.L20001
	pea	__iob+0xe
	pea	_bline
	jbsr	_fputs
	addql	#8,sp
	moveq	#0,d0
	movl	sp@+,d2
	movl	sp@+,a5
	unlk	a6
	rts
.F1 = 20
.S1 = 0x2004
.M1 = 144		| 12 + 132
| end
	.globl	fltused
	.data
.L34:
	.ascii	"/usr/games/lib/fortunes"
	.byte	0x0
.L35:
	.ascii	"r"
	.byte	0x0
.L38:
	.ascii	"Memory fault -- core dumped"
	.byte	0xa
	.byte	0x0
