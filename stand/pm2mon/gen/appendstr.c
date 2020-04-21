/*
 * appendstr() --
 * append strings (vector terminated by 0)
 * to tgt, but not more than n bytes.
 */
char *
appendstr(tgt,n,a)
    register char *tgt;
    int n;
    char *a;
{
    extern char *strlcpy();

    register char **ap,*zp;

    zp = tgt+n;

    ap = &a;
    while( *ap != 0 )
	tgt = strlcpy(tgt,zp,*ap++);
    return tgt;
}
