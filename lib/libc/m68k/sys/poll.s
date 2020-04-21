| C library -- poll

| fds = poll(fdp, nfds, timeout)
| int fds;
| struct pollfd *fdp;
| unsigned long nfds;
| long timeout;
|

include(../DEFS.m4)

ENTRY(poll)
	moveq	#86,d0
	movl	sp@(4),a0	| fdp
	movl	sp@(8),d1	| nfds
	movl	sp@(12),a1	| timeout
	trap	#0
	jcs	1$
	rts

1$:	jmp	cerror	
