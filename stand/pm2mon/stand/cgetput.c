/* cooked getchar - putchar */

/*
 * no-echo getchar (now same as getchar)
 */
int
negetchar()
{
    return getchar();
}

/*
 * echo getchar
 */
int
egetchar()
{
    register int c;
    c = getchar();
    putchar(c);
    return c;
}

/*
 * cooked getchar.
 */
int
cgetchar()
{
    register int c;

    if( (c = egetchar()) == '\r' )
    {
	c = '\n';
	putchar(c);
    }
    return c;
}

/*
 * cooked putchar.
 */
cputchar(c)
    register int c;
{
    putchar(c);
    if( c == '\n' )
	putchar('\r');
}
