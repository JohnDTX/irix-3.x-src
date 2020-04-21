/*	@(#)fwrite.c	3.4	*/
/*LINTLIBRARY*/
/*
 * This version writes directly to the buffer rather than looping on putc.
 * Ptr args aren't checked for NULL because the program would be a
 * catastrophic mess anyway.  Better to abort than just to return NULL.
 */
#include <stdio.h>
#include "stdiom.h"

#define MIN(x, y)	(x < y ? x : y)

extern char *memcpy();

int
fwrite(ptr, size, count, iop)
char *ptr;
register int size, count;
register FILE *iop;
{
	register unsigned nleft;
	register int n;
	register unsigned char *cptr, *bufend;

	if (size <= 0 || count <= 0 || _WRTCHK(iop))
		return (0);

	bufend = _bufend(iop);

	for (nleft = count * size; ; ptr += n) {
		while ((n = bufend - (cptr = iop->_ptr)) <= 0)  /* full buf */
			if (_xflsbuf(iop) == EOF)
				return (count - (nleft + size - 1)/size);
		n = MIN(nleft, n);
		(void) memcpy((char *) cptr, ptr, n);
		iop->_cnt -= n;
		iop->_ptr += n;
		_BUFSYNC(iop);
		if ((nleft -= n) == 0)  { /* done; flush if "unbuffered" or */
			if (iop->_flag & (_IONBF | _IOLBF)) /* line-buffered */
				(void) _xflsbuf(iop);
			return (count);
		}
	}
}
