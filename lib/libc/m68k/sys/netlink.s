| C library -- netlink

| status = netlink(text,linkname,netdev)

include(../DEFS.m4)

ENTRY(netlink)
	moveq	#81,d0
	movl	sp@(4),a0
	movl	sp@(8),d1
	movl	sp@(12),a1
	trap	#0
	jcs	1$
	moveq	#0, d0
	rts

1$:	jmp	cerror	
