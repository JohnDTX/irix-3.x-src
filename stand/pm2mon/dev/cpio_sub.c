#
/*
 * cpio_sub.c --
 * cpio package (currently used by tapeboot).
 *
 *	int cpio_init(bread,blksize)
 *	    int (*bread)();
 *	    int blksize;
 *	{
 *		initialize the cpio package
 *		to read units of blksize from (*bread)
 *		(which is understood to take an int byte
 *		count and a buffer address).
 *		return 0 (success) or -1 (failure).
 *	}
 *
 *	void cpio_close()
 *	{
 *		perform cleanup
 *	}
 *
 *	int cpio_scan(file,matcher)
 *	    char *file;
 *	    int (*matcher)();
 *	{
 *		loop over cpio headers,
 *		calling matcher with the header and file.
 *		return 0 if matcher returns true on any header,
 *		otherwise cleanup and return -1.
 *	}
 *
 *	int cpio_read(buf,len)
 *	    char *buf;
 *	    int len;
 *	{
 *		read up to len bytes, into buf.
 *		return the actual count.
 *	}
 */
# include "sys/types.h"
# include "pmIImacros.h"

# undef  DEBUG do_debug
# include "dprintf.h"

# include "cpiohdr.h"

# define longword(a,b)	((unsigned short)(a)<<16|(unsigned short)(b))
# define cpio_size(hp)	longword((hp)->cpiofilesize[0],(hp)->cpiofilesize[1])
# define cpio_round(x)	(((x)+1)&~01)

char cpiotrailer[] = "TRAILER!!!";

# define BLKFAC	4


struct
{
    char *ptr;		/* buffered data remaining */
    long cnt;		/* amt of buffered data remaining */
    char *base;		/* buffered data area */
    long basesize;	/* size of data area */

    off_t tapoff;
    int (*bread)();
    short blksize;

    char found;
}   C;


int
cpio_init(bread,blocksize)
    int (*bread)();
    int blocksize;
{
    extern char *gmalloc();

    C.tapoff = 0;
    C.blksize = blocksize;
dprintf(("cpio init 0x%x %d\n",bread,blocksize));
    C.basesize = C.blksize*BLKFAC;
    C.bread = bread;
    C.found = 0;
    if( C.base == 0 )
    {
	if( (C.base = gmalloc(C.basesize)) == 0 )
	    return -1;
    }
    C.ptr = C.base; C.cnt = 0;

    return 0;
}


int
cpio_scan(file,matcher)
    char *file;
    int (*matcher)();
{
    struct cpiohdr h;
    off_t filesize;

dprintf((" cpio scan"));
    for( ;; )
    {
	if( cpio_gethead(&h,&filesize) < 0 )
	    break;

	if( filesize == 0 && strcmp(h.cpioname,cpiotrailer) == 0 )
	    break;

	if( (*matcher)(&h,file,filesize) )
	    return 0;

	if( cpio_fwdseek(cpio_round(filesize)) < 0 )
	    break;
    }

    if( !C.found )
    {
	printf("\"%s\" not found\n",file);
	return -1;
    }

    cpio_close();
    return -1;
}

cpio_close()
{
}

int
cpio_read(buf,len)
    char *buf;
    register int len;
{
    register int x;

    x = len;

    x -= x % C.blksize;
    if( x <= 0 )
    {
	if( cpio_fwdseek(0) < 0 )
	    return -1;
    }
    if( C.cnt > 0 )
    {
	x = C.cnt<len ? C.cnt : len;
	if( cpio_fread(buf,x) < 0 )
	    return -1;
	return x;
    }

    if( cpio_fread(buf,x) < 0 )
	return -1;
    return x;
}


/*
 * cpio_fwdseek() --
 * read until past the desired new (relative) position.
 */
int
cpio_fwdseek(x)
    off_t x;
{
dprintf((" fwdseek(%d)",x));
    x += C.tapoff - C.cnt;

    while( C.tapoff <= x )
    {
	if( cpio_tread(C.base,C.basesize) < 0 )
	    return -1;
    }

    C.cnt = C.tapoff - x;
    C.ptr = C.base + (C.basesize - C.cnt);
dprintf((" /%d\n",C.cnt));
}

/*
 * cpio_gethead() --
 * read and partially verify one cpio header.
 * pass back the file size (not including header).
 */
int
cpio_gethead(hp,_filesize)
    register struct cpiohdr *hp;
    off_t *_filesize;
{
    if( cpio_fread((char *)hp,sizeof *hp - sizeof hp->cpioname) < 0
     || hp->cpiomagic != CPIOMAGIC
     || !(0 < hp->cpionamesize && hp->cpionamesize <= sizeof hp->cpioname)

     || cpio_fread(hp->cpioname,cpio_round(hp->cpionamesize)) < 0
     || hp->cpioname[hp->cpionamesize-1] != 000 )
    {
	printf("? Bad cpio header\n");
	return -1;
    }

    *_filesize = cpio_size(hp);
dprintf((" gethead %d <%s> %d\n",hp->cpionamesize,hp->cpioname,*_filesize));

    return 0;
}

/*
 * cpio_fread() --
 * buffered read of len bytes to buf.
 */
int
cpio_fread(tgt,len)
    register char *tgt;
    register int len;
{
    register int xcnt;

dprintf((" fread(%d)\n",len));
    while( len > 0 )
    {
	if( C.cnt <= 0 )
	{
	    if( cpio_fwdseek(0) < 0 )
		return -1;
	}
	xcnt = C.cnt<len ? C.cnt : len;
	bcopy(C.ptr,tgt,xcnt);

	C.ptr += xcnt; C.cnt -= xcnt;

	tgt += xcnt; len -= xcnt;
    }

    return 0;
}


/* matcher subroutines */
int
cpio_fullmatch(hp,str)
    struct cpiohdr *hp;
    char *str;
{
    if( strcmp(hp->cpioname,str) == 0 )
    {
	C.found++;
	return 1;
    }

    return 0;
}

int
cpio_partmatch(hp,str,size)
    register struct cpiohdr *hp;
    register char *str;
    off_t size;
{
    register char passed;
    register char *hstr;

    hstr = hp->cpioname;

    passed = 0;
    if( *str == 000 )
	passed++;

    while( *str != 000 && *hstr == *str )
	hstr++ , str++;

    if( str[0] == 000 )
    {
	while( *hstr == '/' )
	    hstr++;
	if( hstr[0] == 000 || hstr[0] == '/' && hstr[1] != '.'
	 || hstr > hp->cpioname && hstr[-1] == '/' )
	    passed++;
    }

    if( passed )
    {
	C.found = 1;
	printf("%10d %s\n",size,hp->cpioname);
    }

    return 0;
}

int
cpio_tread(base,size)
    char *base;
    int size;
{
    register int x;

    C.cnt = 0;
dprintf((" x %d base 0x%x",x,base));
    if( (*C.bread)(size,base) )
	return -1;
    C.tapoff += size;
    return 0;
}
