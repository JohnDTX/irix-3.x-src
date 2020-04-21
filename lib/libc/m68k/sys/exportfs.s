| C library -- exportfs

|  int exportfs(argp, dir, ronly);
|	char	*dir;		/* directory mounted on */
|	int	rootid;		/* mount read only flag */
|	int	flags;		/* mount read only flag */

include(../DEFS.m4)

ENTRY(exportfs)
	movl	#81,d0		| sysent index for exportfs
	movl	sp@(4),a0	| fetch argument 1
	movl	sp@(8),d1	| fetch argument 2
	movl	sp@(12),a1	| fetch argument 3
	trap	#0
	jcs	1$
	rts

1$:
	jmp	cerror	
