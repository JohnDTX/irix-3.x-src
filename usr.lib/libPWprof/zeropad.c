static char Sccsid[]="@(#)zeropad.c	3.1";
/*
	Replace initial blanks with '0's in `str'.
	Return `str'.
*/

char *zeropad(str)
char *str;
{
	register char *s;

	for (s=str; *s == ' '; s++)
		*s = '0';
	return(str);
}
