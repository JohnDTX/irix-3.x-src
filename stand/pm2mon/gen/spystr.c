# include "ctype.h"

char spybuf[20];
char *spystr(s,n)
    register char *s;
    int n;
{
    register char *bp;
    register int ccc;

    if( n > sizeof spybuf - 1 )
	n = sizeof spybuf - 1;
    bp = spybuf;
    while( --n >= 0 )
	*bp++ = isprint(ccc = toascii(*s++))?ccc:'.';
    *bp++ = 000;
    return spybuf;
}
