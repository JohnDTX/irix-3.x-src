/*
 * joinspec() --
 * put together a prefix, extension, and file,
 * to make a (well-formed) boot string.
 */
joinspec(prefix,ext,file,tgt,n)
    char *prefix,*ext,*file;
    register char *tgt;
    int n;
{
    extern char *strlcpy();
    register char *last;

    last = tgt+n-3;

    tgt = strlcpy(tgt,last,prefix);

    last++;
    *tgt++ = '.';
    tgt = strlcpy(tgt,last,ext);

    last++;
    *tgt++ = ':';
    tgt = strlcpy(tgt,last,file);

    *tgt = 000;
}
