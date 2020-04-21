/*
 * Copyright (c) 1982 Regents of the University of California
 *
 * static char sccsid[] = "@(#)cerror.s 1.3 9/2/82";
 *
 * static char rcsid[] = "$Header: /d2/3.7/src/usr.bin/dbx/RCS/cerror.s,v 1.1 89/03/27 17:44:19 root Exp $";
 *
 * modified version of cerror
 *
 * The idea is that every time an error occurs in a system call
 * I want a special function "syserr" called.  This function will
 * either print a message and exit or do nothing depending on
 * defaults and use of "onsyserr".
 */

#ifdef sun
.data
.globl	_errno
_errno:
	.long	0
.text
#else /* sgi */
.comm	_errno,4
#endif

.globl	cerror
cerror:
#ifdef vax
	movl	r0,_errno
	calls	$0,_syserr	/* new code */
	mnegl	$1,r0
	ret
#else
	movl	d0,_errno
	jbsr	_syserr		/* new code */
	moveq	#-1,d0
	rts
#endif

.globl	__mycerror		/* clumsy way to get this loaded */

__mycerror:
#ifdef vax
	.word	0
	ret
#else
	rts
#endif
