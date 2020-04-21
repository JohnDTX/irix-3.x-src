|C startup, transliteration of crt0.s from unix

nfunc	= 1000	|number of functions to monitor
fbsize	= 4006	|number of shorts in mcount buffer + header

	.text
	.globl	_exit,_environ,start,_main,_errno
	.globl	_monitor,__cleanup,cntbase,_etext

start:	
	movl	sp@(4),a0	|ptr to arg0
	clrl	a0@(-4)		|make sure it has a 0 word preceding
	movl	sp,a0		|save ptr to nargs
	subql	#8,sp		|allocate space for two words
	movl	a0@,sp@		|copy down nargs
	addql	#4,a0		|bump ptr past nargs to arg0 ptr
	movl	a0,sp@(4)	|save this as second arg to main

1$:	tstl	a0@+		|look for 0 word marking end of args
	jne	1$		|not there yet
	movl	sp@(4),a1	|get ptr to ptr_to_arg0 ie argv
	cmpl	a1@,a0		|have we gone past *argv
	jlt	2$		|no, a0 must point to valid environ (blo?)
	subql	#4,a0		|else make it point to zero word
2$:	movl	a0,sp@(8)	|this becomes third argument to main
	movl	a0,_environ	|save it in global

|profiling setup
	movl	#_etext,d0	|end of text
	subl	#eprol,d0	|point after this startup
	addl	#7,d0		|round up
	asrl	#3,d0		|divide by 8 = number of shorts in profile
	movl	#nfunc,sp@-	|number of functions to monitor with mcount
	addl	#fbsize,d0	|add number shorts in mcount buf to prof buf
	movl	d0,sp@-		|total number of shorts in buffer
	asll	#1,d0		|number of shorts * sizeof(short) is ...
	movl	d0,sp@-		|number of bytes to allocate
	jsr	_sbrk		|call memory allocator
	addql	#4,sp		|cleanup arg
	cmpl	#-1,d0		|did sbrk fail?
	jeq	3$		|yes, give up and print message
	movl	d0,sp@-		|ptr to core that sbrk returned is bufp
	addl	#12,d0		|leave space for 3 ints
	movl	d0,cntbase	|save resulting ptr as start of counter buffer
	movl	#_etext,sp@-	|high pc
	movl	#eprol,sp@-	|immediately after profile is low pc
	jsr	_monitor	|start profiling
	addl	#20,sp		|pop all 5 args to monitor

	jsr	_main
	addql	#8,sp		|pop argc,argv
	movl	d0,sp@-		|value of main is argument to exit
	jsr	_exit		|cleanup, stop monitoring and go away

3$:	movl	#msgend-msg,sp@-	|nbytes
	movl	#msg,sp@-	|ptr to message
	movl	#2,sp@-		|stderr
	jsr	_write		|tell the user about it

_exit:	jsr	__cleanup	|close files etc
	clrl	sp@-		|0 as first arg to monitor, ignore the rest
	jsr	_monitor	|stop monitoring
	movw	#1,d0		|exit trap
	trap	#0		|never to return
eprol:				|mark end of startup


	.data
msg:	.ascii	"No space for monitor buffer"
	.byte	012		|newline
msgend:

	.even
	.bss
_environ:
	.space	4
_errno:
	.space	4
cntbase:
	.space	4
