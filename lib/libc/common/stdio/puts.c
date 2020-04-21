/*	@(#)puts.c	3.3	*/
/*LINTLIBRARY*/
/*
 * This version writes directly to the buffer rather than looping on putc.
 * Ptr args aren't checked for NULL because the program would be a
 * catastrophic mess anyway.  Better to abort than just to return NULL.
 */
#include <stdio.h>
#include "stdiom.h"

extern char *memccpy();

int
puts(ptr)
char *ptr;
{
	char *p;
	register int ndone = 0, n;
	register unsigned char *cptr, *bufend;
	register FILE *fo = stdout;

	if (_WRTCHK(fo))
		return (0);

	bufend = _bufend(fo);

	for ( ; ; ptr += n) {
		while ((n = bufend - (cptr = fo->_ptr)) <= 0) /* full buf */
			if (_xflsbuf(fo) == EOF)
				return(EOF);
		if ((p = memccpy((char *) cptr, ptr, '\0', n)) != NULL)
			n = p - (char *) cptr;
		fo->_cnt -= n;
		fo->_ptr += n;
		_BUFSYNC(fo);
		ndone += n;
		if (p != NULL) {
			fo->_ptr[-1] = '\n'; /* overwrite '\0' with '\n' */
			if (fo->_flag & (_IONBF | _IOLBF)) /* flush line */
				if (_xflsbuf(fo) == EOF)
					return(EOF);
			return(ndone);
		}
	}
}
