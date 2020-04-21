/*	@(#)ecvt.c	2.5.1.1	*/
/*	3.0 SID #	1.2	*/
/*LINTLIBRARY*/
/*
 *	ecvt converts to decimal
 *	the number of digits is specified by ndigit
 *	decpt is set to the position of the decimal point
 *	sign is set to 0 for positive, 1 for negative
 *
  *	For the IEEE version, there are two forms of these
  *	conversion routines, one for single and one for double
  *	precision.  The default is the single precision routine
  *	(i.e., the %f spec gets the single version).  The double
  *	precision version has routines of the same name prefixed
  *	by '_d_'. %F or %Lf would get the double precision version.
  *	The long float version is in _d_ecvt.c.
  *		(GB) (SGI) 6-14-83.
 */
#define SW_DIV 1
#include <sys/signal.h>
#include <fperr.h>
#include <nan.h>
#include <values.h>
#define	NMAX	((DSIGNIF * 3 + 19)/10) /* restrict max precision */
#define	NDIG	80
extern char *_d_cvt();

char *
_d_ecvt(value, ndigit, decpt, sign)
long float	value;
int	ndigit, *decpt, *sign;
{
	return (_d_cvt(value, ndigit, decpt, sign, 0));
}

char *
_d_fcvt(value, ndigit, decpt, sign)
long float	value;
int	ndigit, *decpt, *sign;
{
	return (_d_cvt(value, ndigit, decpt, sign, 1));
}

static char buf[NDIG];

static char *
_d_cvt(value, ndigit, decpt, sign, f_flag)
#ifndef SW_DIV
register 
#endif
long float value;
int	ndigit, *sign, f_flag;
register int	*decpt;
{
	register char *p = &buf[0], *p_last = &buf[ndigit];
#ifdef SW_DIV
	long float _sw_d_idiv(),_sw_d_div();
#endif

	/* raise exception on Not-a-Number or infinity (3b or SGI only) */
	KILLNaN(value); 
	if (*sign = (value < 0.0))
		value = -value;
	buf[0] = '\0';
	*decpt = 0;
	if (value != 0.0) { /* rescale to range [1.0, 10.0) */
		/* in binary for speed and to minimize error build-up */
		/* even for the IEEE standard with its high exponents,
		   it's probably better for speed to just loop on them */
		static struct s { long float p10; int n; } s[] = {
			1e32,	32,
			1e16,	16,
			1e8,	8,
			1e4,	4,
			1e2,	2,
			1e1,	1,
		};
		register struct s *sp = s;

		++*decpt;
		if (value >= 2.0 * MAXPOWTWO) /* can't be precisely integral */
			do {
				for ( ; value >= sp->p10; *decpt += sp->n)
#ifdef SW_DIV
					_sw_d_idiv(&value,sp->p10);
#else
					value /= sp->p10;
#endif
			} while (sp++->n > 1);
		else if (value >= 10.0) { /* convert integer part separately */
#ifndef SW_DIV
			register 
#endif
			long float pow10 = 10.0, powtemp;

			while ((powtemp = 10.0 * pow10) <= value)
				pow10 = powtemp;
#ifdef SW_DIV
			for ( ; ; _sw_d_idiv(&pow10,(long float)10.0)) {
				register int digit = (int)_sw_d_div(value,pow10);
#else
			for ( ; ; pow10 /= 10.0) {
				register int digit = value/pow10;
#endif
				*p++ = digit + '0';
				value -= digit * pow10;
				++*decpt;
				if (pow10 <= 10.0)
					break;
			}
		} else if (value < 1.0)
			do {
				for ( ; value * sp->p10 < 10.0; *decpt -= sp->n)
					value *= sp->p10;
			} while (sp++->n > 1);
	}
	if (f_flag)
		p_last += *decpt;
	if (p_last >= buf) {
		if (p_last > &buf[NDIG - 2])
			p_last = &buf[NDIG - 2];
		for ( ; ; ++p) {
			if (value == 0 || p >= &buf[NMAX])
				*p = '0';
			else {
				register int intx; /* intx in [0, 9] */
				*p = (intx = (int)value) + '0';
				value = 10.0 * (value - (long float)intx);
			}
			if (p >= p_last) {
				p = p_last;
				break;
			}
		}
		if (*p >= '5') /* check rounding in last place + 1 */
			do {
				if (p == buf) { /* rollover from 99999... */
					buf[0] = '1'; /* later digits are 0 */
					++*decpt;
					if (f_flag)
						++p_last;
					break;
				}
				*p = '0';
			} while (++*--p > '9'); /* propagate carries left */
		*p_last = '\0';
	}
	return (buf);
}
