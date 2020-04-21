/*
 * strcmpfold.c:
 *
 * Case-folded
 * Compare strings:  s1>s2: >0  s1==s2: 0  s1<s2: <0
 *
 * HISTORY:
 * 8 November 1983	Jeffrey Mogul	Stanford
 *	- Created (using strcmp.c as a base)
 */

#include <ctype.h>

#define	canon(c)	(isupper(c) ? tolower(c) : (c) )

strcmpfold(s1, s2)
register char *s1, *s2;
{

	while (canon(*s1) == canon(*s2))
		if (*s1++=='\0')
			return(0);
		else
			s2++;
	return(canon(*s1) - canon(*s2));
}

