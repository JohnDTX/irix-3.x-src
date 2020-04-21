/*	@(#)putw.c	1.3	*/
/*	3.0 SID #	1.2	*/
/*LINTLIBRARY*/
/*
 * The intent here is to provide a means to make the order of
 * bytes in an io-stream correspond to the order of the bytes
 * in the memory while doing the io a `word' at a time.
 */
#include <stdio.h>

int
putw(w, stream)
int w;
register FILE *stream;
{
	register char *s = (char *)&w;
	register int i = sizeof(int);

	while (--i >= 0)
		(void) putc(*s++, stream);
	return (ferror(stream));
}
