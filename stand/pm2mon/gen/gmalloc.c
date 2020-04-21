char *
gmalloc(size)
    unsigned size;
{
    extern char *mbmalloc();

    register char *s;

    if( (s = mbmalloc(size)) == 0 )
	printf("? Out of core\n");
    return s;
}
