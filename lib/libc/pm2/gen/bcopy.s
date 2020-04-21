|
| Fast 68010 block copy:
|	void bcopy(from, to, count);
|	char *from;
|	char *to;
|	int count;
|
| Because the 68010 has restrictions on addressing boundaries, we can't do
| long or word fetches on odd boundaries.  Accordingly, this code is real
| picky about boundaries, and tries to get aligned asap.  This explains
| most of the complexity below.
|
| One other note.  The 680[01]0 chips have a bug in the moveml instruction
| which causes an extra read cycle after the final move.  We get around
| this bug by not using a moveml instruction to move the last long of data.
|
| Written by: Kipp Hickman, 1986
|

include(../DEFS.m4)

ENTRY(bcopy)
	movl	sp@(4), a0	| a0 := from
	movl	sp@(8), a1	| a1 := to
	movl	sp@(12), d0	| d0 := count
	jeq	goaway		| count is zero, all done
|
| Check and see if pointers overlap in a bad way.  If so, do copy backwards.
| We try to do copies in the forwards direction, because that's the fastest
| on the 68010.  A forwards copy won't work if the from region overlaps the
| to region in such a way that the start of the from region is less than
| the start of the to region, AND if the end of the from region is greater
| than the start of the to region.
|
	cmpl	a0, a1		| See if from < to
	jeq	goaway		| Buffers exactly overlap - all done!
	jlt	forwards	| to < from; go forwards
	addl	d0, a0		| a0 := from + count
	cmpl	a0, a1		| See if to >= from + count
	jge	goforwards	|   Yes - copy forwards (the faster way)
	movl	sp@(4), a0	| Restore a0
	jra	backwards

goforwards:
	movl	sp@(4), a0	| Restore a0
|
| Forwards copy.  First, see if we can get addresses aligned on a short
| boundary.
|
forwards:
	moveml	#0x7F3E, sp@-	| Save all but a0, a1, a7, and d0
	movl	a0, d2		| Use d2&d3 as temporaries
	andb	#1, d2
	movl	a1, d3
	andb	#1, d3
	cmpb	d2, d3		| Compare lower bit of a0 & a1
	jne	forw_slow	|   Low bits differ - copy slowly
	tstb	d3		| Is address odd?
	jeq	forw_copy240
	movb	a0@+,a1@+	| Advance to even address
	subql	#1, d0
|
| Addresses are short aligned.  Lets boogie!
|
| Copy in 240 byte chunks
|
forw_copy240:
	cmpl	#240, d0		| 240 bytes left?
	jle	forw_copy144		| Nope...
	moveml	a0@+, #0x7CFE		| copy   0- 47
	moveml	#0x7CFE, a1@
	moveml	a0@+, #0x7CFE		| copy  48- 95
	moveml	#0x7CFE, a1@(48)
	moveml	a0@+, #0x7CFE		| copy  96-143
	moveml	#0x7CFE, a1@(96)
	moveml	a0@+, #0x7CFE		| copy 144-191
	moveml	#0x7CFE, a1@(144)
	moveml	a0@+, #0x7CFE		| copy 192-239
	moveml	#0x7CFE, a1@(192)
	addw	#240, a1		| advance destination
	subl	#240, d0		| Reduce count by 240
	jne	forw_copy240
	jra	byebye
|
| Copy in 144 byte chunks
|
forw_copy144:
	cmpl	#144, d0		| 144 bytes left?
	jle	forw_copy48		| Nope...
	moveml	a0@+, #0x7CFE		| copy   0- 47
	moveml	#0x7CFE, a1@
	moveml	a0@+, #0x7CFE		| copy  48- 95
	moveml	#0x7CFE, a1@(48)
	moveml	a0@+, #0x7CFE		| copy  96-143
	moveml	#0x7CFE, a1@(96)
	addw	#144, a1		| advance destination
	subl	#144, d0		| Reduce count by 144
	jne	forw_copy144
	jra	byebye
|
| Copy in 48 byte chunks
|
forw_copy48:
	cmpl	#48, d0			| 48 bytes left?
	jle	forw_copyrest		| Nope...
	moveml	a0@+, #0x7CFE		| copy   0- 47
	moveml	#0x7CFE, a1@
	addw	#48, a1			| advance destination
	subl	#48, d0			| Reduce count by 48
	jne	forw_copy48
	jra	byebye
|
| Copy remaining data
|
forw_copyrest:
	cmpl	#4, d0			| a long left?
	jlt	forw_slow		| Nope...
	movl	a0@+, a1@+		| copy a long
	subql	#4, d0
	jne	forw_copyrest
|
| Copy remaining bytes
|
| Slow (forwards) byte at a time copy
|
forw_slow:
	tstl	d0
	jeq	byebye
	movb	a0@+, a1@+
	subql	#1, d0
	jra	forw_slow
byebye:
	moveml	sp@+, #0x7CFE	| Restore all but d0, a7, a1, and a0
goaway:
	rts
|
| Backwards copy
|
backwards:
	moveml	#0x7F3E, sp@-	| Save all but a0, a1, a7, and d0
	addl	d0, a0		| Advance to end
	addl	d0, a1
|
| See if we can get addresses aligned on a short
| boundary.
|
	movl	a0, d2		| Clobber d2&d3
	andb	#1, d2
	movl	a1, d3
	andb	#1, d3
	cmpb	d2, d3		| Compare lower two bits of a0 & a1
	jne	back_slow	|   Low bits differ - copy slowly
	tstb	d3		| Is address odd?
	jeq	back_copy240
	movb	a0@-,a1@-	| Retreat to even address
	subql	#1, d0
|
| Addresses are short aligned.  Lets boogie!
|
back_copy240:
	cmpl	#240, d0		| 240 bytes left?
	jle	back_copy144		| Nope...
	moveml	a0@(-48), #0x7CFE	| copy  47-  0
	moveml	#0x7F3E, a1@-
	moveml	a0@(-96), #0x7CFE	| copy  95- 48
	moveml	#0x7F3E, a1@-
	moveml	a0@(-144), #0x7CFE	| copy 143- 96
	moveml	#0x7F3E, a1@-
	moveml	a0@(-192), #0x7CFE	| copy 191-144
	moveml	#0x7F3E, a1@-
	moveml	a0@(-240), #0x7CFE	| copy 239-192
	moveml	#0x7F3E, a1@-
	subw	#240, a0		| Retreat pointer
	subl	#240, d0		| Decrease count
	jne	back_copy240
	jra	byebye
|
| Copy 144 bytes
|
back_copy144:
	cmpl	#144, d0		| 144 bytes left?
	jle	back_copy48		| Nope...
	moveml	a0@(-48), #0x7CFE	| copy  47-  0
	moveml	#0x7F3E, a1@-
	moveml	a0@(-96), #0x7CFE	| copy  95- 48
	moveml	#0x7F3E, a1@-
	moveml	a0@(-144), #0x7CFE	| copy 143- 96
	moveml	#0x7F3E, a1@-
	subw	#144, a0		| Retreat pointer
	subl	#144, d0		| Decrease count
	jne	back_copy144
	jra	byebye
|
| Copy 48 bytes
|
back_copy48:
	cmpl	#48, d0			| 48 bytes left?
	jle	back_copyrest		| Nope...
	moveml	a0@(-48), #0x7CFE	| copy  47-  0
	moveml	#0x7F3E, a1@-
	subw	#48, a0			| Retreat pointer
	subl	#48, d0			| Decrease count
	jne	back_copy48
	jra	byebye
|
| Copy remaining data
|
back_copyrest:
	cmpl	#4, d0			| a long left?
	jlt	back_slow		| Nope...
	movl	a0@-, a1@-		| copy a long
	subql	#4, d0
	jne	back_copyrest
|
| Slow (backwards), byte at a time, copy
|
back_slow:
	tstl	d0
	jeq	byebye
	movb	a0@-, a1@-
	subql	#1, d0
	jra	back_slow
