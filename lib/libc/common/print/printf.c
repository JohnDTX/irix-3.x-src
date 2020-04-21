/*	@(#)printf.c	1.4	*/
/*LINTLIBRARY*/
#include <stdio.h>
#include <varargs.h>

extern int _doprnt();

/*VARARGS1*/
int
printf(format, va_alist)
char *format;
va_dcl
{
	register int count;
	va_list ap;

	va_start(ap);
	count = _doprnt(format, ap, stdout);
	va_end(ap);
	return(ferror(stdout)? EOF: count);
}
