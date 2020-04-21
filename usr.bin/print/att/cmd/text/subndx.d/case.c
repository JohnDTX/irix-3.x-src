/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
	ulcase searches the input string for any upper case letters and
	changes them to lower case. It returns a 1 if a change is made,
	and 0 if none.
*/


ulcase (string)
char	*string;
{
	int	chg = 0;
	int	lcase = 'a' - 'A';
	char	*ch ;

	ch = string;					/*get first character*/
	while (*ch != '\0') {
		if (*ch >= 'A' && *ch <= 'Z')		/*if upper case */ {
			*ch += lcase;			/*change to lower case*/
			chg = 1;
		}
		ch++;					/*get next character in string*/
	}
	return (chg);
}


