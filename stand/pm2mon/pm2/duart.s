	.data
	.comm	_dad,0x0
	.even
	.globl	_dad
_dad:
	.long	0xfc4000
	.long	0xfc4010
	.long	0xfc6000
	.long	0xfc6010
	.globl	_opcr
_opcr:
	.word	62708
	.word	0
	.text
	.globl	_dinit
_dinit:
	link	a6,#-.F1
	movl	a5,sp@-
	movl	d7,sp@-
	movl	a6@(8),d7
| A1 = 12
	andl	#0xfffffffe,d7
	movl	d7,d0
	asll	#0x2,d0
	addl	#_dad,d0
	movl	d0,a0
	movl	a0@,a5
	movl	d7,d0
	addl	#_opcr,d0
	movl	d0,a0
	movb	a0@,a5@(0x18)
	movb	#0xeb,a5@(0x8)
	clrb	a5@(0xa)
	movl	a5,sp@-
	jbsr	_duart_port_dfl
	addql	#4,sp
	movl	d7,d0
	addql	#0x1,d0
	asll	#0x2,d0
	addl	#_dad,d0
	movl	d0,a0
	movl	a0@,sp@-
	jbsr	_duart_port_dfl
	addql	#4,sp
	movl	sp@+,d7
	movl	sp@+,a5
	unlk	a6
	rts
.F1 = 8
.S1 = 0x2080
.M1 = 136		| 4 + 132
	.globl	_duart_port_dfl
_duart_port_dfl:
	link	a6,#-.F2
	movl	a5,sp@-
	movl	a6@(8),a5
| A2 = 12
	movb	#0x93,a5@
	movb	#0x7,a5@
	movb	#0x10,a5@(0x4)
	movb	#0x20,a5@(0x4)
	movb	#0x30,a5@(0x4)
	movb	#0x40,a5@(0x4)
	movb	#0xbb,a5@(0x2)
	movb	#0x5,a5@(0x4)
	movl	sp@+,a5
	unlk	a6
	rts
.F2 = 4
.S2 = 0x2000
.M2 = 132		| 0 + 132
	.data
	.globl	_du_speedbits
_du_speedbits:
	.word	17493
	.word	26248
	.word	39355
	.word	52292
	.even
	.globl	_du_speeds
_du_speeds:
	.word	300
	.word	600
	.word	1200
	.word	2400
	.word	4800
	.word	9600
	.word	19200
	.word	0
	.text
	.globl	_setbaud
_setbaud:
	link	a6,#-.F3
	movl	d7,sp@-
| A3 = 16
	moveq	#0,d7
	jra	.L23
.L20001:
	movl	d7,d0
	asll	#0x1,d0
	addl	#_du_speeds,d0
	movl	d0,a0
	movw	a0@,d0
	extl	d0
	cmpl	a6@(0xc),d0
	jeq	.L22
	addql	#0x1,d7
.L23:
	movl	d7,d0
	asll	#0x1,d0
	addl	#_du_speeds,d0
	movl	d0,a0
	tstw	a0@
	jne	.L20001
.L22:
	movl	d7,d0
	addl	#_du_speedbits,d0
	movl	d0,a0
	movl	a6@(0x8),d0
	asll	#0x2,d0
	addl	#_dad,d0
	movl	d0,a1
	movl	a1@,a1
	movb	a0@,a1@(0x2)
	movl	sp@+,d7
	unlk	a6
	rts
.F3 = 4
.S3 = 0x80
.M3 = 132		| 0 + 132
	.globl	_modem
_modem:
	link	a6,#-.F4
	movl	a5,sp@-
| A4 = 16
	movw	a6@(0x8+2),d0
	andl	#0x2,d0
	asll	#0x2,d0
	addl	#_dad,d0
	movl	d0,a0
	movl	a0@,a5
	movl	a6@(0x8),d0
	addl	#_opcr,d0
	movl	d0,a0
	movb	a0@,a5@(0x18)
	tstl	a6@(0xc)
	jeq	.L27
	movw	a6@(0x8+2),d0
	andl	#0x1,d0
	moveq	#0x11,d1
	asll	d0,d1
	movb	d1,a5@(0x1a)
	jra	.L26
.L27:
	movw	a6@(0x8+2),d0
	andl	#0x1,d0
	moveq	#0x11,d1
	asll	d0,d1
	movb	d1,a5@(0x1c)
.L26:
	movl	sp@+,a5
	unlk	a6
	rts
.F4 = 4
.S4 = 0x2000
.M4 = 132		| 0 + 132
	.data
