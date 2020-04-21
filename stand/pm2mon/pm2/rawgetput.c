#include "duart.h"



nwputcraw(c, n)
register c, n;
{
	register duart *dp;

	dp = dad[n];
	if((dp->d_sr & SRTR) == 0) return(-1);
	dp->d_thr = c;		/* Output char */

	return(0);
}

putcraw(c, n)
register c, n;
{
	register duart *dp;

	dp = dad[n];
	while((dp->d_sr & SRTR) == 0) ;
	dp->d_thr = c;		/* Output char */

	return(c);
}

/*
 * Getc routines
 */
getcraw(n)
register n;
{
	register duart *dp;
	register c;

	dp = dad[n];
	while(((c = dp->d_sr) & SRRR) == 0) ;
	if (c & SRRB) {
		c = dp->d_rhr;
		return(-1);
	}
	return dp->d_rhr;
}

nwgetcraw(n)
register n;
{
	if((dad[n]->d_sr & SRRR) == 0)
		return NOCHAR;
	return getcraw(n);
}

flush(port) {

    /* flush stray input from port port */
    while (nwgetcraw(port) != NOCHAR) ;

}
