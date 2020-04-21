#
/*
 * memboot.c --
 * boot from memory (presumably prom).
 * called via the boot switch.
 */
# define PROMSTATIC

PROMSTATIC	long memoff;
# define DEFMEMBOOTADDR 0xFA0000

int
mem_open(ext,file)
    char *ext,*file;
{
    memoff = DEFMEMBOOTADDR;
    if( memstring(ext,file,&memoff) < 0 )
	return -1;
    printf("(membooting: 0x%x)\n",memoff);
    return 0;
}

mem_close()
{
}

int
mem_read(_ptr,len)
    char (**_ptr);
    int len;
{
    *_ptr = (char *)memoff; memoff += len;
    return len;
}

int
memstring(ext,file,_ptr)
    register char *ext;
    char *file;
    long *_ptr;
{
if(*ext==000)ext=file;

    if( *ext == 000 || strcmp(ext,"defaultboot") == 0 )
	return 0;
    else
    if( !isnum(ext,_ptr) )
    {
	illegalnum(ext);
	return -1;
    }

    return 0;
}
