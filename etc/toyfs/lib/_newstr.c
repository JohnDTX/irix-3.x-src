#
    extern char *malloc();
/*
 * newstr() --
 * returns a new (malloc()ed) copy of
 * string s, or 0 if no space.
 */
char *newstr(src)
    char *src;
{
    register char *s,*m,*t;

    s = src;
    while( *s++ != 000 )
	;
    if( (m = malloc(s - src)) == 0 )
	return 0;
    s = src;
    t = m;
    while( (*t++ = *s++) != 000 )
	;
    return m;
}
