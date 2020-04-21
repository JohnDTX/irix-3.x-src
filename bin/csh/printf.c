/*	@(#)printf.c	2.1	SCCS id keyword	*/
/* Copyright (c) 1980 Regents of the University of California */
/*
 * Hacked "printf" which prints through putchar.
 * DONT USE WITH STDIO!
 */

#ifdef SVR3
#include <varargs.h>
printf(fmt, va_alist)
char *fmt;
va_dcl
{
	_doprnt(fmt, &va_alist, 0);
}
#else
printf(fmt, args)
char *fmt;
{
	_doprnt(fmt, &args, 0);
}
#endif

_strout(string, count, adjust, foo, fillch)
register char *string;
register int count;
int adjust;
register struct { int a[6]; } *foo;
{

	if (foo != 0)
		abort();
	while (adjust < 0) {
		if (*string=='-' && fillch=='0') {
			putchar(*string++);
			count--;
		}
		putchar(fillch);
		adjust++;
	}
	while (--count>=0)
		putchar(*string++);
	while (adjust) {
		putchar(fillch);
		adjust--;
	}
}
