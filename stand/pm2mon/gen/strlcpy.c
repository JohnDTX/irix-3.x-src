/*
 * strlcpy() --
 * copy up to N bytes from src to tgt;
 * return a pointer to the last tgt byte.
 */
char *strlcpy(tgt,last,src)
    register char *tgt,*last;
    register char *src;
{
    register char *last;

    for( ; tgt != last; tgt++ )
	if( (*tgt = *src++) == 000 )
	    break;

    return tgt;
}
