/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
	str_rev, given a string, returns a string which is the reverse.
	For example, given "word", "drow" is returned.
*/

#define DEBUG_STRR 0

strn_rev (str, rstr, n)

char	*str, *rstr;
int	n;					/*length of initial substring*/

{	
	int	i, j;

	j = 0;
	for (i = n - 1; i >= 0; i--) {
		*(rstr + j) = *(str + i);
		j++;
	}
	*(rstr + j) = '\0';
	if (DEBUG_STRR)
		printf ("\n strr.c: str: %s rstr: %s", str, rstr);
}


