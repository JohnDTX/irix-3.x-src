| C library -- nfsmount

|  int nfsmount(argp, dir, ronly);
|	struct nfs_args *argp;		/* network f/s arguments */
|	char		*dir;		/* directory to mount on */
|	int		ronly;		/* mount read only flag */

include(../DEFS.m4)

ENTRY(nfsmount)
	movl	#94,d0		| sysent index for nfsmount
	movl	sp@(4),a0	| fetch argument 1
	movl	sp@(8),d1	| fetch argument 2
	movl	sp@(12),a1	| fetch argument 3
	trap	#0
	jcs	1$
	rts

1$:
	jmp	cerror	
