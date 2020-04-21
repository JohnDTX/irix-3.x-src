	.data
	.even
BusErrorReturnAddress:
	.space	4
	.even
BerrVector:
	.space	4
	.even
vectorbase:
	.space	4
	.text
nullfunction:
	link	a6,#-.F1
	moveml	#.S1,a6@(-.F1)
| A1 = 8
	.globl	BusErrorHandler
BusErrorHandler:
	movw	sp@,sp@(50)
	addl	#52,sp
	movl	BusErrorReturnAddress,sp@
	addl	#4,sp
	movw	#0,sp@
	subql	#6,sp
	rte
	bra	.L15
.L15:	unlk	a6
	rts
.F1 = 0
.S1 = 0
| M1 = 132
	.data
	.text
	.globl	saferead
saferead:
	link	a6,#-.F2
	moveml	#.S2,a6@(-.F2)
| A2 = 12
	movw	#0x1,a6@(-8)
	movl	#0x8,BerrVector
	movl	BerrVector,a0
	movl	a0@,a6@(-6)
	movl	BerrVector,a0
	movl	#BusErrorHandler,a0@
	movl	#ReadNotThere,BusErrorReturnAddress
	movl	a6@(0x8),a0
	movw	a0@,a6@(-2)
	movl	BerrVector,a0
	movl	a6@(-6),a0@
	tstw	a6@(-8)
	beq	.L20
	movw	a6@(-2),d0
	extl	d0
	andl	#0xffff,d0
	bra	.L19
.L20:
	.globl	ReadNotThere
ReadNotThere:
	movl	BerrVector,a0
	movl	a6@(-6),a0@
	moveq	#-1,d0
	bra	.L19
	bra	.L19
.L19:	unlk	a6
	rts
.F2 = 8
.S2 = 0
| M2 = 132
	.data
	.text
	.globl	safewrite
safewrite:
	link	a6,#-.F3
	moveml	#.S3,a6@(-.F3)
| A3 = 16
	movw	#0x1,a6@(-6)
	movl	#0x8,BerrVector
	movl	BerrVector,a0
	movl	a0@,a6@(-4)
	movl	BerrVector,a0
	movl	#BusErrorHandler,a0@
	movl	#WriteNotThere,BusErrorReturnAddress
	movl	a6@(0x8),a0
	movw	a6@(0xe),a0@
	movl	BerrVector,a0
	movl	a6@(-4),a0@
	tstw	a6@(-6)
	beq	.L24
	moveq	#0x1,d0
	bra	.L23
.L24:
	.globl	WriteNotThere
WriteNotThere:
	movl	BerrVector,a0
	movl	a6@(-4),a0@
	clrl	d0
	bra	.L23
	bra	.L23
.L23:	unlk	a6
	rts
.F3 = 8
.S3 = 0
| M3 = 132
	.data
