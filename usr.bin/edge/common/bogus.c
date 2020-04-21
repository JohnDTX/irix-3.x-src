
#ifdef mips


/*
 * $Source: /d2/3.7/src/usr.bin/edge/common/RCS/bogus.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:45:46 $
 */


gl_getcharinfo(w, h, d)
short	*w;
short	*h;
short	*d;
{
	*w = 9;
	*h = 14;
	*d = 2;
}

gl_findwidth()
{
	return(9);

}

gl_finddescender()
{

	return(2);
}


ringbell(){}



bzero(p, nbytes)
char	*p;
int	nbytes;
{
	char	*q;

	for (q = p; q < (char *) ((int) p + nbytes); q++) {
		*q = '\0';
	}
}
#endif
