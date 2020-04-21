	.data
	.even
_sccsid:
	.long	.L12
	.comm	_tempfile,0x4
	.even
	.globl	_keystr
_keystr:
	.data
	.long	.L32
	.even
	.globl	_multauth
_multauth:
	.long	0x0
	.comm	_oneauth,0x4
	.text
	.globl	_main
_main:
	link	a6,#-.F1
| A1 = 16
	cmpl	#0x1,a6@(0x8)
	jne	.L39
	pea	.L41
	jbsr	_puts
	addql	#4,sp
	pea	.L42
	jbsr	_puts
	addql	#4,sp
	pea	0x1
	jbsr	_exit
	addql	#4,sp
.L39:
	cmpl	#0x2,a6@(0x8)
	jle	.L44
	movl	a6@(0xc),a0
	movl	a0@(0x4),a0
	cmpb	#0x2d,a0@
	jne	.L44
	movl	a6@(0xc),a0
	movl	a0@(0x4),a0
	cmpb	#0x73,a0@(0x1)
	jne	.L44
	movl	a6@(0xc),a0
	movl	a0@(0x4),d0
	addql	#0x2,d0
	movl	d0,_keystr
	movl	d0,sp@-
	jbsr	_eval
	addql	#4,sp
	addql	#0x4,a6@(0xc)
	subql	#0x1,a6@(0x8)
.L44:
	cmpl	#0x11,a6@(0x8)
	jle	.L46
	pea	0x10
	pea	.L48
	pea	__iob+0x1c
	jbsr	_fprintf
	addw	#12,sp
	pea	0x1
	jbsr	_exit
	addql	#4,sp
.L46:
	movl	#0x1,a6@(-72)
	jra	.L51
.L20001:
	pea	.L52
	movl	a6@(-72),d0
	asll	#0x2,d0
	addl	a6@(0xc),d0
	movl	d0,a0
	movl	a0@,sp@-
	jbsr	_fopen
	addql	#8,sp
	lea	a6@(0xffffffc0),a0
	movl	a6@(-72),d1
	subql	#0x1,d1
	asll	#0x2,d1
	addl	d1,a0
	movl	d0,a0@
	jne	.L49
	movl	a6@(-72),d0
	asll	#0x2,d0
	addl	a6@(0xc),d0
	movl	d0,a0
	movl	a0@,sp@-
	jbsr	_error
	addql	#4,sp
.L49:
	addql	#0x1,a6@(-72)
.L51:
	movl	a6@(-72),d0
	cmpl	a6@(0x8),d0
	jlt	.L20001
	movl	#.L55,_tempfile
	movl	_tempfile,sp@-
	jbsr	_mktemp
	addql	#4,sp
	pea	0x1
	pea	0x2
	jbsr	_signal
	addql	#8,sp
	cmpl	#0x1,d0
	jeq	.L56
	pea	_onintr
	pea	0x2
	jbsr	_signal
	addql	#8,sp
.L56:
	pea	.L57
	movl	_tempfile,sp@-
	jbsr	_fopen
	addql	#8,sp
	movl	d0,a6@(-68)
	jne	.L58
	movl	_tempfile,sp@-
	jbsr	_error
	addql	#4,sp
.L58:
	clrl	a6@(-72)
	jra	.L61
.L20003:
	movl	a6@(-72),sp@-
	movl	a6@(-68),sp@-
	lea	a6@(0xffffffc0),a0
	movl	a6@(-72),d0
	asll	#0x2,d0
	addl	d0,a0
	movl	a0@,sp@-
	jbsr	_sortbib
	addw	#12,sp
	addql	#0x1,a6@(-72)
.L61:
	movl	a6@(0x8),d0
	subql	#0x1,d0
	movl	a6@(-72),d1
	cmpl	d0,d1
	jlt	.L20003
	movl	a6@(-68),sp@-
	jbsr	_fclose
	addql	#4,sp
	movl	a6@(-68),sp@-
	pea	a6@(0xffffffc0)
	jbsr	_deliver
	addql	#8,sp
	movl	_tempfile,sp@-
	jbsr	_unlink
	addql	#4,sp
	pea	0x0
	jbsr	_exit
	addql	#4,sp
	unlk	a6
	rts
.F1 = 72
.S1 = 0x0
.M1 = 144		| 12 + 132
	.data
	.even
	.globl	_rsmode
_rsmode:
	.long	0x0
	.text
	.globl	_sortbib
_sortbib:
	link	a6,#-.F2
| A2 = 20
	clrl	a6@(-8)
	clrl	a6@(-20)
	jra	.L68
.L20005:
	tstl	a6@(-20)
	jne	.L70
	movl	#0x1,a6@(-16)
.L70:
	cmpb	#0xa,a6@(-4116)
	jne	.L71
	tstl	_rsmode
	jne	.L72
	movl	#0x1,_rsmode
.L72:
	cmpl	#0x1,_rsmode
	jne	.L71
	movl	#0x1,a6@(-16)
.L71:
	cmpb	#0x2e,a6@(-4116)
	jne	.L74
	cmpb	#0x5b,a6@(-4115)
	jne	.L74
	tstl	_rsmode
	jne	.L75
	movl	#0x2,_rsmode
.L75:
	cmpl	#0x2,_rsmode
	jne	.L74
	movl	#0x1,a6@(-16)
.L74:
	tstl	a6@(-16)
	jeq	.L77
	clrl	a6@(-16)
	movl	a6@(-4),d0
	subl	a6@(-8),d0
	movl	d0,a6@(-12)
	cmpl	#0x8000,d0
	jle	.L78
	movl	a6@(-12),sp@-
	pea	0x8000
	movl	a6@(-20),sp@-
	pea	.L79
	pea	__iob+0x1c
	jbsr	_fprintf
	addw	#20,sp
	pea	0x1
	jbsr	_exit
	addql	#4,sp
.L78:
	movl	a6@(-20),d0
	addql	#0x1,a6@(-20)
	tstl	d0
	jeq	.L80
	pea	a6@(0xffffdfec)
	pea	a6@(0xffffcfec)
	pea	a6@(0xffffbfec)
	pea	a6@(0xffffafec)
	movl	a6@(-12),sp@-
	movl	a6@(-8),sp@-
	movl	a6@(0x10),sp@-
	pea	.L81
	movl	a6@(0xc),sp@-
	jbsr	_fprintf
	addw	#36,sp
	movl	a6@(0xc),a0
	movb	a0@(0xc),d0
	andl	#0x20,d0
	jeq	.L80
	movl	_tempfile,sp@-
	jbsr	_error
	addql	#4,sp
.L80:
	clrb	a6@(-8212)
	movb	a6@(-8212),a6@(-12308)
	movb	a6@(-12308),a6@(-16404)
	movb	a6@(-16404),a6@(-20500)
	clrl	_oneauth
	movl	a6@(-4),a6@(-8)
.L77:
	cmpb	#0x25,a6@(-4116)
	jne	.L68
	pea	a6@(0xffffafec)
	pea	a6@(0xffffefec)
	jbsr	_parse
	addql	#8,sp
.L68:
	movl	a6@(0x8),sp@-
	jbsr	_ftell
	addql	#4,sp
	movl	d0,a6@(-4)
	movl	a6@(0x8),sp@-
	pea	0x1000
	pea	a6@(0xffffefec)
	jbsr	_fgets
	addw	#12,sp
	tstl	d0
	jne	.L20005
	movl	a6@(0x8),sp@-
	jbsr	_ftell
	addql	#4,sp
	movl	d0,a6@(-4)
	subl	a6@(-8),d0
	movl	d0,a6@(-12)
	cmpl	#0x8000,d0
	jle	.L85
	movl	d0,sp@-
	pea	0x8000
	movl	a6@(-20),sp@-
	pea	.L86
	pea	__iob+0x1c
	jbsr	_fprintf
	addw	#20,sp
	pea	0x1
	jbsr	_exit
	addql	#4,sp
.L85:
	cmpb	#0xa,a6@(-4116)
	jeq	.L67
	pea	a6@(0xffffdfec)
	pea	a6@(0xffffcfec)
	pea	a6@(0xffffbfec)
	pea	a6@(0xffffafec)
	movl	a6@(-12),sp@-
	movl	a6@(-8),sp@-
	movl	a6@(0x10),sp@-
	pea	.L88
	movl	a6@(0xc),sp@-
	jbsr	_fprintf
	addw	#36,sp
	movl	a6@(0xc),a0
	movb	a0@(0xc),d0
	andl	#0x20,d0
	jeq	.L67
	movl	_tempfile,sp@-
	jbsr	_error
	addql	#4,sp
.L67:
	unlk	a6
	rts
.F2 = 20500
.S2 = 0x0
.M2 = 168		| 36 + 132
	.globl	_deliver
_deliver:
	linkl	a6,#-.F3
| A3 = 16
	movl	_tempfile,sp@-
	movl	_tempfile,sp@-
	pea	.L92
	movl	a6,d0
	subl	#0x9050,d0
	movl	d0,sp@-
	jbsr	_sprintf
	addw	#16,sp
	movl	a6,d0
	subl	#0x9050,d0
	movl	d0,sp@-
	jbsr	_system
	addql	#4,sp
	cmpl	#0x7f,d0
	jne	.L94
	pea	.L95
	jbsr	_error
	addql	#4,sp
.L94:
	pea	.L96
	movl	_tempfile,sp@-
	jbsr	_fopen
	addql	#8,sp
	movl	d0,a6@(0xc)
	jra	.L97
.L20007:
	movl	a6,d0
	subl	#0x905c,d0
	movl	d0,sp@-
	movl	a6,d0
	subl	#0x9054,d0
	movl	d0,sp@-
	movl	a6,d0
	subl	#0x9058,d0
	movl	d0,sp@-
	pea	.L100
	pea	a6@(0xfffff000)
	jbsr	_sscanf
	addw	#20,sp
	cmpl	#0x3,d0
	jeq	.L101
	pea	.L102
	jbsr	_error
	addql	#4,sp
.L101:
	pea	0x0
	movl	a6,d0
	subl	#0x9054,d0
	movl	d0,a0
	movl	a0@,sp@-
	movl	a6,d0
	subl	#0x9058,d0
	movl	d0,a0
	movl	a0@,d0
	asll	#0x2,d0
	addl	a6@(0x8),d0
	movl	d0,a0
	movl	a0@,sp@-
	jbsr	_fseek
	addw	#12,sp
	cmpl	#0xffffffff,d0
	jne	.L104
	pea	.L105
	jbsr	_error
	addql	#4,sp
.L104:
	movl	a6,d0
	subl	#0x9058,d0
	movl	d0,a0
	movl	a0@,d0
	asll	#0x2,d0
	addl	a6@(0x8),d0
	movl	d0,a0
	movl	a0@,sp@-
	movl	a6,d0
	subl	#0x905c,d0
	movl	d0,a0
	movl	a0@,sp@-
	pea	0x1
	movl	a6,d0
	subl	#0x9000,d0
	movl	d0,sp@-
	jbsr	_fread
	addw	#16,sp
	tstl	d0
	jne	.L107
	pea	.L108
	jbsr	_error
	addql	#4,sp
.L107:
	movl	a6,d0
	subl	#0x9000,d0
	movl	d0,a0
	cmpb	#0xa,a0@
	jeq	.L109
	cmpl	#0x1,_rsmode
	jne	.L109
	subql	#0x1,__iob+0xe
	jlt	.L10000
	movl	__iob+0x12,d0
	addql	#0x1,__iob+0x12
	movl	d0,a0
	movb	#0xa,a0@
	moveq	#0,d0
	movb	a0@,d0
	jra	.L109
.L10000:
	pea	__iob+0xe
	pea	0xa
	jbsr	__flsbuf
	addql	#8,sp
.L109:
	pea	__iob+0xe
	movl	a6,d0
	subl	#0x905c,d0
	movl	d0,a0
	movl	a0@,sp@-
	pea	0x1
	movl	a6,d0
	subl	#0x9000,d0
	movl	d0,sp@-
	jbsr	_fwrite
	addw	#16,sp
	tstl	d0
	jne	.L97
	pea	.L113
	jbsr	_error
	addql	#4,sp
.L97:
	movl	a6@(0xc),sp@-
	pea	0x1000
	pea	a6@(0xfffff000)
	jbsr	_fgets
	addw	#12,sp
	tstl	d0
	jne	.L20007
	unlk	a6
	rts
.F3 = 36956
.S3 = 0x0
.M3 = 152		| 20 + 132
	.globl	_parse
_parse:
	link	a6,#-.F4
| A4 = 16
	clrl	a6@(-8200)
	jra	.L118
.L20009:
	moveq	#0xa,d0
	movl	a6@(-8200),d1
	asll	d0,d1
	lea	a6@(0xffffe000),a0
	addl	d1,a0
	clrb	a0@
	addql	#0x1,a6@(-8200)
.L118:
	cmpl	#0x8,a6@(-8200)
	jlt	.L20009
	pea	a6@(0xfffffc00)
	pea	a6@(0xfffff800)
	pea	a6@(0xfffff400)
	pea	a6@(0xfffff000)
	pea	a6@(0xffffec00)
	pea	a6@(0xffffe800)
	pea	a6@(0xffffe400)
	pea	a6@(0xffffe000)
	pea	.L119
	movl	a6@(0x8),sp@-
	jbsr	_sscanf
	addw	#40,sp
	movl	d0,a6@(-8196)
	clrl	a6@(-8200)
	jra	.L122
.L20013:
	tstl	_oneauth
	jeq	.L125
	tstl	_multauth
	jeq	.L121
.L125:
	tstl	_oneauth
	jeq	.L126
	pea	.L127
	moveq	#0xc,d0
	movl	a6@(-8200),d1
	asll	d0,d1
	addl	a6@(0xc),d1
	movl	d1,sp@-
	jbsr	_strcat
	addql	#8,sp
.L126:
	movl	a6@(-8196),d0
	subql	#0x2,d0
	moveq	#0xa,d1
	asll	d1,d0
	lea	a6@(0xffffe000),a0
	addl	d0,a0
	movl	a0,sp@-
	jbsr	_endcomma
	addql	#4,sp
	tstl	d0
	jne	.L129
	movl	a6@(-8196),d0
	subql	#0x1,d0
	moveq	#0xa,d1
	asll	d1,d0
	lea	a6@(0xffffe000),a0
	addl	d0,a0
	movl	a0,sp@-
	moveq	#0xc,d0
	movl	a6@(-8200),d1
	asll	d0,d1
	addl	a6@(0xc),d1
	movl	d1,sp@-
	jbsr	_strcat
	addql	#8,sp
	jra	.L130
.L129:
	movl	a6@(-8196),d0
	subql	#0x2,d0
	moveq	#0xa,d1
	asll	d1,d0
	lea	a6@(0xffffe000),a0
	addl	d0,a0
	movl	a0,sp@-
	moveq	#0xc,d0
	movl	a6@(-8200),d1
	asll	d0,d1
	addl	a6@(0xc),d1
	movl	d1,sp@-
	jbsr	_strcat
	addql	#8,sp
	subql	#0x1,a6@(-8196)
.L130:
	pea	.L131
	moveq	#0xc,d0
	movl	a6@(-8200),d1
	asll	d0,d1
	addl	a6@(0xc),d1
	movl	d1,sp@-
	jbsr	_strcat
	addql	#8,sp
	movl	#0x1,a6@(-8204)
	jra	.L134
.L20011:
	moveq	#0xa,d0
	movl	a6@(-8204),d1
	asll	d0,d1
	lea	a6@(0xffffe000),a0
	addl	d1,a0
	movl	a0,sp@-
	moveq	#0xc,d0
	movl	a6@(-8200),d1
	asll	d0,d1
	addl	a6@(0xc),d1
	movl	d1,sp@-
	jbsr	_strcat
	addql	#8,sp
	addql	#0x1,a6@(-8204)
.L134:
	movl	a6@(-8196),d0
	subql	#0x1,d0
	movl	a6@(-8204),d1
	cmpl	d0,d1
	jlt	.L20011
	movl	#0x1,_oneauth
	jra	.L120
.L20019:
	cmpb	#0x51,a6@(-8191)
	jne	.L120
	movl	_keystr,d0
	addl	a6@(-8200),d0
	movl	d0,a0
	cmpb	#0x41,a0@
	jne	.L120
	movl	#0x1,a6@(-8204)
.L153:
	movl	a6@(-8204),d0
	cmpl	a6@(-8196),d0
	jge	.L120
	moveq	#0xa,d0
	movl	a6@(-8204),d1
	asll	d0,d1
	lea	a6@(0xffffe000),a0
	addl	d1,a0
	movl	a0,sp@-
	moveq	#0xc,d0
	movl	a6@(-8200),d1
	asll	d0,d1
	addl	a6@(0xc),d1
	movl	d1,sp@-
	jbsr	_strcat
	addql	#8,sp
	addql	#0x1,a6@(-8204)
	jra	.L153
.L20015:
	movl	a6@(-8196),d0
	subql	#0x1,d0
	moveq	#0xa,d1
	asll	d1,d0
	lea	a6@(0xffffe000),a0
	addl	d0,a0
	movl	a0,sp@-
	moveq	#0xc,d0
	movl	a6@(-8200),d1
	asll	d0,d1
	addl	a6@(0xc),d1
	movl	d1,sp@-
	jbsr	_strcat
	addql	#8,sp
	cmpl	#0x2,a6@(-8196)
	jle	.L135
	pea	a6@(0xffffe400)
	moveq	#0xc,d0
	movl	a6@(-8200),d1
	asll	d0,d1
	addl	a6@(0xc),d1
	movl	d1,sp@-
	jbsr	_strcat
	addql	#8,sp
.L135:
.L120:
	addql	#0x1,a6@(-8200)
.L122:
	cmpl	#0x4,a6@(-8200)
	jge	.L114
	movb	a6@(-8191),d0
	movl	_keystr,d1
	addl	a6@(-8200),d1
	movl	d1,a0
	movb	a0@,d1
	cmpb	d1,d0
	jne	.L20019
	cmpb	#0x41,a6@(-8191)
	jeq	.L20013
	cmpb	#0x44,a6@(-8191)
	jeq	.L20015
	cmpb	#0x54,a6@(-8191)
	jeq	.L10002
	cmpb	#0x4a,a6@(-8191)
	jne	.L139
.L10002:
	movl	#0x1,a6@(-8204)
	pea	a6@(0xffffe400)
	jbsr	_article
	addql	#4,sp
	tstl	d0
	jeq	.L144
.L20016:
	addql	#0x1,a6@(-8204)
.L144:
	movl	a6@(-8204),d0
	cmpl	a6@(-8196),d0
	jge	.L135
	moveq	#0xa,d0
	movl	a6@(-8204),d1
	asll	d0,d1
	lea	a6@(0xffffe000),a0
	addl	d1,a0
	movl	a0,sp@-
	moveq	#0xc,d0
	movl	a6@(-8200),d1
	asll	d0,d1
	addl	a6@(0xc),d1
	movl	d1,sp@-
	jbsr	_strcat
	addql	#8,sp
	jra	.L20016
.L139:
	movl	#0x1,a6@(-8204)
.L148:
	movl	a6@(-8204),d0
	cmpl	a6@(-8196),d0
	jge	.L120
	moveq	#0xa,d0
	movl	a6@(-8204),d1
	asll	d0,d1
	lea	a6@(0xffffe000),a0
	addl	d1,a0
	movl	a0,sp@-
	moveq	#0xc,d0
	movl	a6@(-8200),d1
	asll	d0,d1
	addl	a6@(0xc),d1
	movl	d1,sp@-
	jbsr	_strcat
	addql	#8,sp
	addql	#0x1,a6@(-8204)
	jra	.L148
.L121:
.L114:
	unlk	a6
	rts
.F4 = 8204
.S4 = 0x0
.M4 = 172		| 40 + 132
	.globl	_article
_article:
	link	a6,#-.F5
| A5 = 12
	movl	a6@(0x8),sp@-
	pea	.L156
	jbsr	_strcmp
	addql	#8,sp
	tstl	d0
	jne	.L157
.L20020:
	moveq	#0x1,d0
	jra	.L154
.L157:
	movl	a6@(0x8),sp@-
	pea	.L158
	jbsr	_strcmp
	addql	#8,sp
	tstl	d0
	jeq	.L20020
	movl	a6@(0x8),sp@-
	pea	.L160
	jbsr	_strcmp
	addql	#8,sp
	tstl	d0
	jeq	.L20020
	movl	a6@(0x8),sp@-
	pea	.L162
	jbsr	_strcmp
	addql	#8,sp
	tstl	d0
	jeq	.L20020
	movl	a6@(0x8),sp@-
	pea	.L164
	jbsr	_strcmp
	addql	#8,sp
	tstl	d0
	jeq	.L20020
	movl	a6@(0x8),sp@-
	pea	.L166
	jbsr	_strcmp
	addql	#8,sp
	tstl	d0
	jeq	.L20020
	movl	a6@(0x8),sp@-
	pea	.L168
	jbsr	_strcmp
	addql	#8,sp
	tstl	d0
	jeq	.L20020
	movl	a6@(0x8),sp@-
	pea	.L170
	jbsr	_strcmp
	addql	#8,sp
	tstl	d0
	jeq	.L20020
	movl	a6@(0x8),sp@-
	pea	.L172
	jbsr	_strcmp
	addql	#8,sp
	tstl	d0
	jeq	.L20020
	movl	a6@(0x8),sp@-
	pea	.L174
	jbsr	_strcmp
	addql	#8,sp
	tstl	d0
	jeq	.L20020
	moveq	#0,d0
.L154:
	unlk	a6
	rts
.F5 = 0
.S5 = 0x0
.M5 = 140		| 8 + 132
	.globl	_eval
_eval:
	link	a6,#-.F6
| A6 = 12
	clrl	a6@(-4)
	clrl	a6@(-8)
	jra	.L179
.L20022:
	movl	a6@(0x8),d0
	addl	a6@(-4),d0
	movl	d0,a0
	cmpb	#0x2b,a0@
	jne	.L180
	movl	#0x1,_multauth
	addql	#0x1,a6@(-4)
.L180:
	movl	a6@(0x8),d0
	addl	a6@(-4),d0
	movl	d0,a0
	movl	a6@(0x8),d0
	addl	a6@(-8),d0
	movl	d0,a1
	movb	a0@,a1@
	addql	#0x1,a6@(-4)
	addql	#0x1,a6@(-8)
.L179:
	movl	a6@(0x8),d0
	addl	a6@(-4),d0
	movl	d0,a0
	tstb	a0@
	jne	.L20022
	movl	a6@(0x8),d0
	addl	a6@(-8),d0
	movl	d0,a0
	clrb	a0@
	unlk	a6
	rts
.F6 = 8
.S6 = 0x0
.M6 = 132		| 0 + 132
	.globl	_error
_error:
	link	a6,#-.F7
| A7 = 12
	movl	a6@(0x8),sp@-
	jbsr	_perror
	addql	#4,sp
	pea	0x1
	jbsr	_exit
	addql	#4,sp
	unlk	a6
	rts
.F7 = 0
.S7 = 0x0
.M7 = 136		| 4 + 132
	.globl	_onintr
_onintr:
	link	a6,#-.F8
| A8 = 8
	pea	.L184
	pea	__iob+0x1c
	jbsr	_fprintf
	addql	#8,sp
	movl	_tempfile,sp@-
	jbsr	_unlink
	addql	#4,sp
	pea	0x1
	jbsr	_exit
	addql	#4,sp
	unlk	a6
	rts
.F8 = 0
.S8 = 0x0
.M8 = 140		| 8 + 132
	.globl	_endcomma
_endcomma:
	link	a6,#-.F9
| A9 = 12
	movl	a6@(0x8),sp@-
	jbsr	_strlen
	addql	#4,sp
	subql	#0x1,d0
	movl	d0,a6@(-4)
	movl	a6@(0x8),d0
	addl	a6@(-4),d0
	movl	d0,a0
	cmpb	#0x2c,a0@
	jne	.L187
	movl	a6@(0x8),d0
	addl	a6@(-4),d0
	movl	d0,a0
	clrb	a0@
	moveq	#0x1,d0
	jra	.L185
.L187:
	moveq	#0,d0
.L185:
	unlk	a6
	rts
.F9 = 4
.S9 = 0x0
.M9 = 136		| 4 + 132
	.data
.L12:
	.ascii	"@(#)sortbib.c"
	.byte	0x9
	.ascii	"4.1 (Berkeley) 5/6"
	.ascii	"/83"
	.byte	0x0
.L32:
	.ascii	"AD"
	.byte	0x0
.L41:
	.ascii	"Usage:  sortbib [-sKEYS] databas"
	.ascii	"e [...]"
	.byte	0x0
.L42:
	.ascii	""
	.byte	0x9
	.ascii	"-s: sort by fields in KEYS (def"
	.ascii	"ault is AD)"
	.byte	0x0
.L48:
	.ascii	"sortbib: More than %d databases "
	.ascii	"specified"
	.byte	0xa
	.byte	0x0
.L52:
	.ascii	"r"
	.byte	0x0
.L55:
	.ascii	"/tmp/SbibXXXXX"
	.byte	0x0
.L57:
	.ascii	"w"
	.byte	0x0
.L79:
	.ascii	"sortbib: record %d longer than %"
	.ascii	"d (%d)"
	.byte	0xa
	.byte	0x0
.L81:
	.ascii	"%d %D %d : %s %s %s %s"
	.byte	0xa
	.byte	0x0
.L86:
	.ascii	"sortbib: record %d longer than %"
	.ascii	"d (%d)"
	.byte	0xa
	.byte	0x0
.L88:
	.ascii	"%d %D %d : %s %s %s %s"
	.byte	0xa
	.byte	0x0
.L92:
	.ascii	"sort -ft: +1 %s -o %s"
	.byte	0x0
.L95:
	.ascii	"sortbib"
	.byte	0x0
.L96:
	.ascii	"r"
	.byte	0x0
.L100:
	.ascii	"%d %D %d :"
	.byte	0x0
.L102:
	.ascii	"sortbib: sorting error"
	.byte	0x0
.L105:
	.ascii	"sortbib"
	.byte	0x0
.L108:
	.ascii	"sortbib"
	.byte	0x0
.L113:
	.ascii	"sortbib"
	.byte	0x0
.L119:
	.ascii	"%s %s %s %s %s %s %s %s"
	.byte	0x0
.L127:
	.ascii	"~~"
	.byte	0x0
.L131:
	.ascii	" "
	.byte	0x0
.L156:
	.ascii	"The"
	.byte	0x0
.L158:
	.ascii	"A"
	.byte	0x0
.L160:
	.ascii	"An"
	.byte	0x0
.L162:
	.ascii	"Le"
	.byte	0x0
.L164:
	.ascii	"La"
	.byte	0x0
.L166:
	.ascii	"Der"
	.byte	0x0
.L168:
	.ascii	"Die"
	.byte	0x0
.L170:
	.ascii	"Das"
	.byte	0x0
.L172:
	.ascii	"El"
	.byte	0x0
.L174:
	.ascii	"Den"
	.byte	0x0
.L184:
	.ascii	""
	.byte	0xa
	.ascii	"Interrupt"
	.byte	0xa
	.byte	0x0
