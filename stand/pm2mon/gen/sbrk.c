/*
 * sbrk() --
 * so to fake out C library malloc().
 */
char *sbrk(n)
    int n;
{
    extern char *mbmalloc();

    register char *cp;

    if( (cp = mbmalloc(n)) == 0 )
	return (char *)-1;
    return cp;
}
