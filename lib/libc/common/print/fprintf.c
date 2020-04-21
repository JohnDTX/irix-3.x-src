/*	@(#)fprintf.c	1.4	*/
/*LINTLIBRARY*/
#include <stdio.h>
#include <varargs.h>

extern int _doprnt();

/*VARARGS2*/
int
fprintf(iop, format, va_alist)
FILE *iop;
char *format;
va_dcl
{
	register int count;
	va_list ap;

	va_start(ap);
	count = _doprnt(format, ap, iop);
	va_end(ap);
	return(ferror(iop)? EOF: count);
}
