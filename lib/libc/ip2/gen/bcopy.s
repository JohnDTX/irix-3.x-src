|
| Block copy for 68020:
|	void bcopy(from, to, count);
|	char *from, *to;
|	int count;
|
| If source and destination overlap, it does the correct thing.
|
| Written by: Kipp Hickman, 1986
|
| $Source: /d2/3.7/src/lib/libc/ip2/gen/RCS/bcopy.s,v $
| $Revision: 1.1 $
| $Date: 89/03/27 16:16:25 $
|

include(../DEFS.m4)

ENTRY(bcopy)
	movl	sp@(4), a0	| a0 := from
	movl	sp@(8), a1	| a1 := to
	movl	sp@(12), d0	| d0 := count
	jeq	back_byebye	|   If count is zero, all done!
|
| See if source and destination overlap
|
	cmpl	a0, a1		| Is "to" >= "from"
	jeq	back_byebye	|   to == from; don't do anything!
	jgt	backwards	|   Yes - do copy the fast way
	addl	d0, a1		| Point a1 at end
	cmpl	a0, a1		| Is "to+count"  <= "from"
	jle	gobackwards	|   Yes - do copy the fast way
|
| If we get here then:
|	(to <= from) &&
|	(to + count > from)
| Which means that the source and destination overlap in such a way that
| the fast backwards copy won't work.  Do the copy the slower forwards way.
|
	movl	sp@(8), a1	| Restore a1
	jra	forwards	| *SIGH* do copy the slow way

gobackwards:
	movl	sp@(8), a1	| Restore a1
|
| Normal (fastest) backwards version of copy code.
|
backwards:
	addl	d0, a0		| a0 := end of from region
	addl	d0, a1		| a1 := end of to region
|
| Try to get addresses aligned on a long boundary.  If alignment of low
| two bits do not agree, then we can never get the pointers aligned
| the same, so there is no helping here.  In this case, punt and do
| the copy slowly.
|
	movl	a0, d0		| Clobber d0
	andb	#3, d0
	movl	a1, d1
	andb	#3, d1
	cmpb	d0, d1		| Compare lower two bits of a0 & a1
	jne	back_copy	|   Low bits differ - copy slowly
|
| Pointers are aligned on the same boundary.  Try to get them long
| aligned.  If low bit is set, then pointers are odd.  Make them even.
|
	movl	sp@(12), d0	| d0 := count
	btst	#0, d1		| See if pointers are odd
	jeq	back_shortalign

back_bytealign:
	movb	a0@-, a1@-	| Copy the byte
	subql	#1, d0		| Decrement count
	jeq	back_byebye	| All done...
|
| Now pointers are even.  Try to get them long aligned.
|
back_shortalign:
	btst	#1, d1		| See if pointers are long aligned?
	jeq	back_copy256	|   YES - copy!
	cmpl	#2, d0		| If less than two bytes remain,
	jlt	back_copybytes	|   just copy bytes...
	movw	a0@-, a1@-	| Copy a short
	subql	#2, d0		| Adjust count
	jeq	back_byebye
	jra	back_copy256
|
| Start copying.  The 68020 can do long/word fetches on odd boundaries.
| If the addresses are not aligned nicely, then the copy will work
| but it will go slowly.  The code above has attempted to align the
| pointers, when it can.
|
back_copy:
	movl	sp@(12), d0	| d0 := count
back_copy256:
	cmpl	#256, d0	| See if more than 256 bytes need to be copied
	jlt	back_copy128	| nope, copy remainder

	movl	a0@-, a1@-	| 000-015
	movl	a0@-, a1@-
	movl	a0@-, a1@-
	movl	a0@-, a1@-
	movl	a0@-, a1@-	| 016-031
	movl	a0@-, a1@-
	movl	a0@-, a1@-
	movl	a0@-, a1@-
	movl	a0@-, a1@-	| 032-047
	movl	a0@-, a1@-
	movl	a0@-, a1@-
	movl	a0@-, a1@-
	movl	a0@-, a1@-	| 048-063
	movl	a0@-, a1@-
	movl	a0@-, a1@-
	movl	a0@-, a1@-
	movl	a0@-, a1@-	| 064-079
	movl	a0@-, a1@-
	movl	a0@-, a1@-
	movl	a0@-, a1@-
	movl	a0@-, a1@-	| 080-095
	movl	a0@-, a1@-
	movl	a0@-, a1@-
	movl	a0@-, a1@-
	movl	a0@-, a1@-	| 096-111
	movl	a0@-, a1@-
	movl	a0@-, a1@-
	movl	a0@-, a1@-
	movl	a0@-, a1@-	| 112-127
	movl	a0@-, a1@-
	movl	a0@-, a1@-
	movl	a0@-, a1@-
	movl	a0@-, a1@-	| 128-143
	movl	a0@-, a1@-
	movl	a0@-, a1@-
	movl	a0@-, a1@-
	movl	a0@-, a1@-	| 144-159
	movl	a0@-, a1@-
	movl	a0@-, a1@-
	movl	a0@-, a1@-
	movl	a0@-, a1@-	| 160-175
	movl	a0@-, a1@-
	movl	a0@-, a1@-
	movl	a0@-, a1@-
	movl	a0@-, a1@-	| 176-191
	movl	a0@-, a1@-
	movl	a0@-, a1@-
	movl	a0@-, a1@-
	movl	a0@-, a1@-	| 192-207
	movl	a0@-, a1@-
	movl	a0@-, a1@-
	movl	a0@-, a1@-
	movl	a0@-, a1@-	| 208-223
	movl	a0@-, a1@-
	movl	a0@-, a1@-
	movl	a0@-, a1@-
	movl	a0@-, a1@-	| 224-239
	movl	a0@-, a1@-
	movl	a0@-, a1@-
	movl	a0@-, a1@-
	movl	a0@-, a1@-	| 240-255
	movl	a0@-, a1@-
	movl	a0@-, a1@-
	movl	a0@-, a1@-
	subl	#256, d0
	jne	back_copy256
	rts

back_copy128:
	cmpl	#128, d0	| See if more than 128 bytes need to be copied
	jlt	back_copy64	| nope, copy remainder

	movl	a0@-, a1@-	| 000-015
	movl	a0@-, a1@-
	movl	a0@-, a1@-
	movl	a0@-, a1@-
	movl	a0@-, a1@-	| 016-031
	movl	a0@-, a1@-
	movl	a0@-, a1@-
	movl	a0@-, a1@-
	movl	a0@-, a1@-	| 032-047
	movl	a0@-, a1@-
	movl	a0@-, a1@-
	movl	a0@-, a1@-
	movl	a0@-, a1@-	| 048-063
	movl	a0@-, a1@-
	movl	a0@-, a1@-
	movl	a0@-, a1@-
	movl	a0@-, a1@-	| 064-079
	movl	a0@-, a1@-
	movl	a0@-, a1@-
	movl	a0@-, a1@-
	movl	a0@-, a1@-	| 080-095
	movl	a0@-, a1@-
	movl	a0@-, a1@-
	movl	a0@-, a1@-
	movl	a0@-, a1@-	| 096-111
	movl	a0@-, a1@-
	movl	a0@-, a1@-
	movl	a0@-, a1@-
	movl	a0@-, a1@-	| 112-127
	movl	a0@-, a1@-
	movl	a0@-, a1@-
	movl	a0@-, a1@-
	subl	#128, d0
	jne	back_copy128
	rts

back_copy64:
	cmpl	#64, d0		| See if more than 64 bytes need to be copied
	jlt	back_copy32		| nope, copy remainder

	movl	a0@-, a1@-	| 000-015
	movl	a0@-, a1@-
	movl	a0@-, a1@-
	movl	a0@-, a1@-
	movl	a0@-, a1@-	| 016-031
	movl	a0@-, a1@-
	movl	a0@-, a1@-
	movl	a0@-, a1@-
	movl	a0@-, a1@-	| 032-047
	movl	a0@-, a1@-
	movl	a0@-, a1@-
	movl	a0@-, a1@-
	movl	a0@-, a1@-	| 048-063
	movl	a0@-, a1@-
	movl	a0@-, a1@-
	movl	a0@-, a1@-
	subl	#64, d0
	jne	back_copy64
	rts

back_copy32:
	cmpl	#32, d0		| See if more than 32 bytes need to be copied
	jlt	back_copyrest	| nope, copy remainder of longs

	movl	a0@-, a1@-	| 000-015
	movl	a0@-, a1@-
	movl	a0@-, a1@-
	movl	a0@-, a1@-
	movl	a0@-, a1@-	| 016-031
	movl	a0@-, a1@-
	movl	a0@-, a1@-
	movl	a0@-, a1@-
	subl	#32, d0
	jne	back_copy32
	rts

back_copyrest:
	cmpl	#4, d0		| See if more than 4 bytes need to be copied
	jlt	back_copyfinal	| nope, copy remainder of shorts
	movl	a0@-, a1@-	| Copy a long
	subql	#4, d0
	jne	back_copyrest

back_copyfinal:
	cmpl	#2, d0		| See if more than 2 bytes need to be copied
	jlt	back_copybytes	| nope, copy final byte
	movw	a0@-, a1@-	| Copy a short
	subql	#2, d0
	jeq	back_byebye

back_copybytes:
	tstl	d0
	jeq	back_byebye
|
| Copy a bunch of bytes
|
back_copybuzz:
	movb	a0@-, a1@-
	subql	#1, d0
	jne	back_copybytes

back_byebye:
	rts
|
| Forwards copy version of the copy...
|
| Try to get addresses aligned on a long boundary.  If alignment of low
| two bits do not agree, then we can never get the pointers aligned
| the same, so there is no helping here.  In this case, punt and do
| the copy slowly.
|
forwards:
	movl	a0, d0		| Clobber d0
	andb	#3, d0
	movl	a1, d1
	andb	#3, d1
	cmpb	d0, d1		| Compare lower two bits of a0 & a1
	jne	forw_copy	|   Low bits differ - copy slowly
|
| Pointers are aligned on the same boundary.  Try to get them long
| aligned.  If low bit is set, then pointers are odd.  Make them even.
|
	movl	sp@(12), d0	| d0 := count
	btst	#0, d1		| See if pointers are odd
	jeq	forw_shortalign

forw_bytealign:
	movb	a0@+, a1@+	| Copy the byte
	subql	#1, d0		| Decrement count
	jeq	forw_byebye	| All done...
|
| Now pointers are even.  Try to get them long aligned.
|
forw_shortalign:
	btst	#1, d1		| See if pointers are long aligned?
	jeq	forw_copy256	|   YES - copy!
	cmpl	#2, d0		| If less than two bytes remain,
	jlt	forw_copybytes	|   just copy bytes...
	movw	a0@+, a1@+	| Copy a short
	subql	#2, d0		| Adjust count
	jeq	forw_byebye
	jra	forw_copy256
|
| Start copying.  The 68020 can do long/word fetches on odd boundaries.
| If the addresses are not aligned nicely, then the copy will work
| but it will go slowly.  The code above has attempted to align the
| pointers, when it can.
|
forw_copy:
	movl	sp@(12), d0	| d0 := count
forw_copy256:
	cmpl	#256, d0	| See if more than 256 bytes need to be copied
	jlt	forw_copy128	| nope, copy remainder

	movl	a0@+, a1@+	| 000-015
	movl	a0@+, a1@+
	movl	a0@+, a1@+
	movl	a0@+, a1@+
	movl	a0@+, a1@+	| 016-031
	movl	a0@+, a1@+
	movl	a0@+, a1@+
	movl	a0@+, a1@+
	movl	a0@+, a1@+	| 032-047
	movl	a0@+, a1@+
	movl	a0@+, a1@+
	movl	a0@+, a1@+
	movl	a0@+, a1@+	| 048-063
	movl	a0@+, a1@+
	movl	a0@+, a1@+
	movl	a0@+, a1@+
	movl	a0@+, a1@+	| 064-079
	movl	a0@+, a1@+
	movl	a0@+, a1@+
	movl	a0@+, a1@+
	movl	a0@+, a1@+	| 080-095
	movl	a0@+, a1@+
	movl	a0@+, a1@+
	movl	a0@+, a1@+
	movl	a0@+, a1@+	| 096-111
	movl	a0@+, a1@+
	movl	a0@+, a1@+
	movl	a0@+, a1@+
	movl	a0@+, a1@+	| 112-127
	movl	a0@+, a1@+
	movl	a0@+, a1@+
	movl	a0@+, a1@+
	movl	a0@+, a1@+	| 128-143
	movl	a0@+, a1@+
	movl	a0@+, a1@+
	movl	a0@+, a1@+
	movl	a0@+, a1@+	| 144-159
	movl	a0@+, a1@+
	movl	a0@+, a1@+
	movl	a0@+, a1@+
	movl	a0@+, a1@+	| 160-175
	movl	a0@+, a1@+
	movl	a0@+, a1@+
	movl	a0@+, a1@+
	movl	a0@+, a1@+	| 176-191
	movl	a0@+, a1@+
	movl	a0@+, a1@+
	movl	a0@+, a1@+
	movl	a0@+, a1@+	| 192-207
	movl	a0@+, a1@+
	movl	a0@+, a1@+
	movl	a0@+, a1@+
	movl	a0@+, a1@+	| 208-223
	movl	a0@+, a1@+
	movl	a0@+, a1@+
	movl	a0@+, a1@+
	movl	a0@+, a1@+	| 224-239
	movl	a0@+, a1@+
	movl	a0@+, a1@+
	movl	a0@+, a1@+
	movl	a0@+, a1@+	| 240-255
	movl	a0@+, a1@+
	movl	a0@+, a1@+
	movl	a0@+, a1@+
	subl	#256, d0
	jne	forw_copy256	| copy some more...
	rts

forw_copy128:
	cmpl	#128, d0	| See if more than 128 bytes need to be copied
	jlt	forw_copy64	| nope, copy remainder

	movl	a0@+, a1@+	| 000-015
	movl	a0@+, a1@+
	movl	a0@+, a1@+
	movl	a0@+, a1@+
	movl	a0@+, a1@+	| 016-031
	movl	a0@+, a1@+
	movl	a0@+, a1@+
	movl	a0@+, a1@+
	movl	a0@+, a1@+	| 032-047
	movl	a0@+, a1@+
	movl	a0@+, a1@+
	movl	a0@+, a1@+
	movl	a0@+, a1@+	| 048-063
	movl	a0@+, a1@+
	movl	a0@+, a1@+
	movl	a0@+, a1@+
	movl	a0@+, a1@+	| 064-079
	movl	a0@+, a1@+
	movl	a0@+, a1@+
	movl	a0@+, a1@+
	movl	a0@+, a1@+	| 080-095
	movl	a0@+, a1@+
	movl	a0@+, a1@+
	movl	a0@+, a1@+
	movl	a0@+, a1@+	| 096-111
	movl	a0@+, a1@+
	movl	a0@+, a1@+
	movl	a0@+, a1@+
	movl	a0@+, a1@+	| 112-127
	movl	a0@+, a1@+
	movl	a0@+, a1@+
	movl	a0@+, a1@+
	subl	#128, d0
	jne	forw_copy128
	rts

forw_copy64:
	cmpl	#64, d0		| See if more than 64 bytes need to be copied
	jlt	forw_copy32	| nope, copy remainder

	movl	a0@+, a1@+	| 000-015
	movl	a0@+, a1@+
	movl	a0@+, a1@+
	movl	a0@+, a1@+
	movl	a0@+, a1@+	| 016-031
	movl	a0@+, a1@+
	movl	a0@+, a1@+
	movl	a0@+, a1@+
	movl	a0@+, a1@+	| 032-047
	movl	a0@+, a1@+
	movl	a0@+, a1@+
	movl	a0@+, a1@+
	movl	a0@+, a1@+	| 048-063
	movl	a0@+, a1@+
	movl	a0@+, a1@+
	movl	a0@+, a1@+
	subl	#64, d0
	jne	forw_copy64
	rts

forw_copy32:
	cmpl	#32, d0		| See if more than 32 bytes need to be copied
	jlt	forw_copyrest	| nope, copy remainder of longs

	movl	a0@+, a1@+	| 000-015
	movl	a0@+, a1@+
	movl	a0@+, a1@+
	movl	a0@+, a1@+
	movl	a0@+, a1@+	| 016-031
	movl	a0@+, a1@+
	movl	a0@+, a1@+
	movl	a0@+, a1@+
	subl	#32, d0
	jne	forw_copy32
	rts

forw_copyrest:
	cmpl	#4, d0		| See if more than 4 bytes need to be copied
	jlt	forw_copyfinal	| nope, copy remainder of shorts
	movl	a0@+, a1@+	| Copy a long
	subql	#4, d0
	jne	forw_copyrest

forw_copyfinal:
	cmpl	#2, d0		| See if more than 2 bytes need to be copied
	jlt	forw_copybytes	| nope, copy final byte
	movw	a0@+, a1@+	| Copy a short
	subql	#2, d0
	jeq	forw_byebye

forw_copybytes:
	tstl	d0
	jeq	forw_byebye
|
| Copy a bunch of bytes
|
forw_copybuzz:
	movb	a0@+, a1@+
	subql	#1, d0
	jne	forw_copybytes

forw_byebye:
	rts
