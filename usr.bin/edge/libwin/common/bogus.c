
    /*
     * version of bcopy which handles a destination overlapping the source
     */
bcopy(from, to, length)
    char *from, *to;
    long length;
{
    if ((from < to) && (to < from+length)) {
	    /* copy from tail, to prevent clobbering source */
	while (length--)
	    *(to+length) = *(from+length);
    } else {
	memcpy(to, from, length);
    }
}


gl_getcharinfo(w, h, d)
short	*w;
short	*h;
short	*d;
{
	*w = 9;
	*h = 14;
	*w = 2;
}


bzero(p, nbytes)
char	*p;
int	nbytes;
{
	char	*q;

	for (q = p; q < (char *) ((int) p + nbytes); q++) {
		*q = '\0';
	}
}
