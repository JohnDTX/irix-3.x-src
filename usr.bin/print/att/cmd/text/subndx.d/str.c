/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

int
string (token, max)

char	*token;
int	max  ;		/*max. no. of non null characters allowed in token*/

{	
	char	*s;
	int	i;

	s = token;
	for (i = 0; i <= max; i++)
		if (*s++ == '\0')
			return (1);

	return (0);                   /*if no null found, return false*/
}


