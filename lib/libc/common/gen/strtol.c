/*	@(#)strtol.c	1.2	*/
/*LINTLIBRARY*/
#include <ctype.h>
#define DIGIT(x) (isdigit(x)? ((x)-'0'): (10+tolower(x)-'a'))
#define MBASE 36

long
strtol(str, ptr, base)
register char *str;
char **ptr;
register int base;
{
	register long val;
	register int xx, sign;

	val = 0L;
	sign = 1;
	if(base < 0 || base > MBASE)
		goto OUT;
	while(isspace(*str))
		++str;
	if(*str == '-') {
		++str;
		sign = -1;
	} else if(*str == '+')
		++str;
	if(base == 0) {
		if(*str == '0') {
			++str;
			if(*str == 'x' || *str == 'X') {
				++str;
				base = 16;
			} else
				base = 8;
		} else
			base = 10;
	} else if(base == 16)
		if(str[0] == '0' && (str[1] == 'x' || str[1] == 'X'))
			str += 2;
	/*
	 * for any base > 10, the digits incrementally following
	 *	9 are assumed to be "abc...z" or "ABC...Z"
	 */
	while(isalnum(*str) && (xx=DIGIT(*str)) < base) {
		/* accumulate neg avoids surprises near maxint */
		val = base*val - xx;
		++str;
	}
OUT:
	if(ptr != (char**)0)
		*ptr = str;
	return(sign*(-val));
}
