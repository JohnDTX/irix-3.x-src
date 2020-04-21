# include "sys/types.h"
# include "ctype.h"

/*
Tue Jun 11 12:26:17 1985 -> 12:26,11jun85
 123456789 123456789 1234567
 */
char *
cdate(clkp)
    time_t *clkp;
{
    extern char *ctime();
    static char bertha[64];
    register char *ap,*cp,*bp;

    bp = bertha;
    cp = ctime(clkp);

    ap = cp+11;
    *bp++ = *ap++; *bp++ = *ap++;
    *bp++ = *ap++; *bp++ = *ap++;
    *bp++ = *ap++; *bp++ = ',';

    ap = cp+8;
    *bp++ = *ap++;
    if( ap[-1] == ' ' )
	bp--;
    *bp++ = *ap++;

    ap = cp+4;
    *bp++ = tolower(*ap++); *bp++ = *ap++;
    *bp++ = *ap++;

    ap = cp+22;
    *bp++ = *ap++; *bp++ = *ap++;

    *bp = 000;
    return bertha;
}
