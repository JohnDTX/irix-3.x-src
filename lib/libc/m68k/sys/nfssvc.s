| C library -- nfssvc

| int sock;	/* fd of open UDP socket */
|
| error = nfssvc(sock);

include(../DEFS.m4)

ENTRY(nfssvc)
	moveq	#95,d0		| sysent index
	movl	sp@(4),a0	| fetch argument
	trap	#0
	jcs	1$
	moveq	#0,d0
	rts

1$:	jmp	cerror
