static char Sccsid[]="@(#)any.c	3.1";
/*
	If any character of `s' is `c', return 1
	else return 0.
*/

any(c,s)
register char c, *s;
{
	while (*s)
		if (*s++ == c)
			return(1);
	return(0);
}
