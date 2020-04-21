| C library -- stat

| int status = stat(pathname, &statbuf);

| #include <sys/types.h>
| #include <sys/stat.h>
| char *pathname;
| struct stat statbuf;

include(../DEFS.m4)

ENTRY(stat)
	moveq	#40,d0
	movl	sp@(4),a0	| fetch argument
	movl	sp@(8),d1	| fetch argument
	trap	#0
	jcs	1$
	moveq	#0,d0
	rts

1$:	jmp	cerror
