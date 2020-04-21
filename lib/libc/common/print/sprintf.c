/*	@(#)sprintf.c	1.5	*/
/*LINTLIBRARY*/
#include <stdio.h>
#include <varargs.h>
#include <values.h>

extern int _doprnt();

/*VARARGS2*/
int
sprintf(string, format, va_alist)
char *string, *format;
va_dcl
{
	register int count;
	FILE siop;
	va_list ap;
	register FILE *sp;

	sp = &siop;
	sp->_cnt = MAXINT;
	sp->_base = sp->_ptr = (unsigned char *)string;
	sp->_flag = _IOWRT;
	sp->_file = _NFILE;
	va_start(ap);
	count = _doprnt(format, ap, sp);
	va_end(ap);
	*sp->_ptr = '\0'; /* plant terminating null character */
	return(count);
}
