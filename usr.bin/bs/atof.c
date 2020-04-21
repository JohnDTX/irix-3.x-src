/*	@(#)atof.c	1.3	*/
/*	@(#)atof.c	1.3	*/
/*	3.0 SID #	1.2	*/
/*LINTLIBRARY*/
/*
 *	C library - ascii to floating
 */

#include <ctype.h>
#include <values.h>

extern double ldexp();

char *Atof;		/* ADDED for bs only */

double
atof(p)
register char *p;
{
	register int c, eexp, exp, neg, negexp, bexp;
	double fl, flexp, exp5;

	Atof = p;	/* ADDED for bs only */
	neg = 1;
	while(isspace(*p))
		++p;
	if(*p == '-') {
		++p;
		neg = -1;
	}
	else if(*p == '+')
		++p;

	exp = 0;
	fl = 0;
	while((c = *p++), isdigit(c))
		if(fl < 2.0 * MAXPOWTWO)
			fl = 10*fl + (c-'0');
		else
			exp++;

	if(c == '.')
		while((c = *p++), isdigit(c))
			if(fl < 2.0 * MAXPOWTWO) {
				fl = 10*fl + (c-'0');
				exp--;
			}

	negexp = 1;
	eexp = 0;
	if((c == 'E') || (c == 'e')) {
		if((c = *p++) == '+')
			;
		else if(c == '-')
			negexp = -1;
		     else
			--p;

		while((c = *p++), isdigit(c))
			eexp = 10*eexp + (c-'0');

		if(negexp < 0)
			eexp = -eexp;
		exp = exp + eexp;
	}

	negexp = 1;
	if(exp < 0) {
		negexp = -1;
		exp = -exp;
	}

	/*
	 * The following computation is done in two stages,
	 * first accumulating powers of 5, then jamming powers of 2,
	 * to avoid underflow in situations like the following (for
	 * the DEC representation): 1.2345678901234567890e-37,
	 * where exp would be about (-37 + -18) = -55, and the
	 * value 10^(-55) can't be represented, but 5^(-55) can
	 * be represented, and then 2^(-55) jammed via ldexp().
	 */
	flexp = 1;
	exp5 = 5;
	bexp = exp;
	while(1) {
		if(exp&01)
			flexp *= exp5;
		exp >>= 1;
		if(exp == 0)
			break;
		exp5 *= exp5;
	}
	if(negexp < 0)
		fl /= flexp;
	else
		fl *= flexp;
	fl = ldexp(fl, negexp*bexp);
	if(neg < 0)
		fl = -fl;
	Atof = p - 1;	/* ADDED for bs only */
	return(fl);
}
