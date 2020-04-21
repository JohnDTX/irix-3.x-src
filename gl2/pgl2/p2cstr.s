|
|
|
|
|	p2cstr - transform a pascal-type string into a C
|			 string.  A pascal string consists of
|			 a length byte followed by the string.
|			 A C string is simply null-terminated.
|	
|		NOTE: It is assumed that the string is only 
|			  needed for temporary operations, and
|			  a static buffer is assumed sufficient.
|
|
	.data
buffer:
	.space	256
	.text
	.globl	G_P2CSTR

G_P2CSTR:
|
|	sp@(4) is the address of the Pascal string.
|
	movl	sp@(4),a0		|get source address
	movl	#buffer,a1		|destination
	clrl	d0				|count
	movb	a0@+,d0			|get count
	jra		2$
1$: movb	a0@+,a1@+		|loop
2$:	dbra	d0,1$
	clrb	a1@				|null terminate
	movl	#buffer,d0		|return value
	rts


