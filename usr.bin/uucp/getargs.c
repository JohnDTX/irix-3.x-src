/* @(#)getargs.c	1.4 */
#include <stdio.h>
#include "uucp.h"


/*
 * generate a vector of pointers (arps) to the
 * substrings in string "s".
 * Each substring is separated by blanks and/or tabs.
 *	s	-> string to analyze
 *	arps	-> array of pointers
 * returns:
 *	i	-> # of subfields
 * Bug:
 * Should pass # of elements in arps in case s
 * is garbled from file.
 */
getargs(s, arps)
register char *s, *arps[];
{
	register int i;

	i = 0;
	while (1) {
		arps[i] = NULL;
		while (*s == ' ' || *s == '\t')
			*s++ = '\0';
		if (*s == '\n')
			*s = '\0';
		if (*s == '\0')
			break;
		arps[i++] = s++;
		while (*s != '\0' && *s != ' '
			&& *s != '\t' && *s != '\n')
				s++;
	}
	return(i);
}
