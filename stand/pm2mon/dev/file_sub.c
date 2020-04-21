#
/*
 * file_sub.c --
 * unix file system package (currently used by disk boot).
 *
 *	void closef()
 *	{
 *		perform cleanup.
 *	}
 *
 *	int openf(file)
 *	     char *file;
 *	{
 *		set up to read file.
 *		return 0 (success) or -1 (failure).
 *	}
 *
 *	int readf(buf)
 *	    char *buf;
 *	{
 *		read one block from the file to buf.
 *		return the actual count.
 *	}
 */
# include "pmII.h"

# include "sys/types.h"
# include "dklabel.h"

# include "sys/inode.h"
# include "sys/ino.h"
# include "sys/dir.h"
# include "safs.h"

# ifndef IFLNK
# define IFLNK		0120000
# endif  IFLNK
# ifndef MAXLNKDEPTH
# define MAXLNKDEPTH	7
# endif  MAXLNKDEPTH
# define MAXNAME	256



# undef DEBUG do_debug
# include "dprintf.h"
# define MIN(a,b)	((a)<(b)?(a):(b))

# define PROMSTATIC




daddr_t iblocknums[MAXINDIRECT+1];
char *iblockp[MAXINDIRECT+1];
struct fino *fs_inode;


# define closei(x)	iput(x)
# define openi(x)	(*fs_iget)(x)


int
openf(path)
    char *path;
{
    extern struct fino *namei();

    if( (fs_inode = namei(ROOTINO,path)) == 0 )
	return -1;
    return 0;
}

closef()
{
    if( fs_inode != 0 )
	closei(fs_inode);
    fs_inode = 0;
}

int
readf(buf)
    char *buf;
{
    return (*fs_readi)(fs_inode,buf);
}

iput(ip)
    register struct fino *ip;
{
    ip->flag = -1;
}

PROMSTATIC
	struct dio dio;
struct fino *
namei(curino,path)
    register long curino;
    char *path;
{
    register char *comp,*ap,*cp;
    register struct fino *ip;
    long pino;

    register int complen,ecomplen;
    register struct gendir *gp;

    char slbuf[MAXNAME];
    short nloop;
    char *endpath;

    nloop = 0;
    comp = path;
    endpath = comp+strlen(comp);

top:
    if( *comp == '/' )
    {
	curino = ROOTINO;
	while( *comp == '/' )
	    comp++;
    }

    if( (ip = openi(curino)) == 0 )
	return 0;

    while( *comp != 000 )
    {
	pino = curino;

	/* break off next component */
	cp = comp;
	while( *cp != 000 && *cp != '/' )
	    cp++;

	complen = cp-comp;

	/* get next pathname component */
	if( dopen(ip,&dio) < 0 )
	{
	    iput(ip);
	    return 0;
	}

	curino = 0;

	/* search the directory */
	while( (gp = (*fs_dread)(&dio)) != 0 )
	if( gp->d_ino != 0 )
	{
	    ecomplen = MIN(gp->d_len,complen);

	    /*
	     * if the entry name is not null-terminated,
	     * it is presumed to match anything beyond
	     * its length.
	     */
	    if( strncmp(gp->d_name,comp,ecomplen) == 0
	     && (ecomplen >= gp->d_len || gp->d_name[ecomplen] == 000) )
	    {
		curino = gp->d_ino;
		break;
	    }
	}

	dclose(&dio);

	if( curino == 0 )
	{
	    printf("\"%s\" not found\n",path);
	    return 0;
	}

	comp = cp;

	while( *comp == '/' )
	    comp++;

	if( (ip = openi(curino)) == 0 )
	    return 0;

	if( (ip->flag&IFMT) == IFLNK )
	{
dprintf((" sl:%d",ip->isize));
	    /*
	     * found a symbolic link.
	     * copy trailing name to start of expansion space.
	     */
	    if( endpath-comp >= MAXNAME )
		goto too_long;

	    strcpy(slbuf,comp);
	    comp = slbuf;
	    endpath = comp + strlen(comp);

	    /*
	     * don't get carried away with symbolic links.
	     */
	    if( ++nloop > MAXLNKDEPTH )
	    {
		printf("too many symlinks (max %d)\n",MAXLNKDEPTH);
		goto put_out;
	    }

	    cp = endpath + ip->isize + 1;
	    if( cp >= slbuf + sizeof slbuf )
		goto too_long;

	    /*
	     * read in symbolic link text.
	     */
	    (*fs_readi)(ip,dio.dirbase);

	    /*
	     * prepend it to remainder of name
	     * (first copy trailing name forward,
	     * then copy in symbolic link text).
	     */
	    ap = endpath;
	    endpath = cp;
	    *cp = 000;
	    while( ap > comp )
		*--cp = *--ap;
	    *--cp = '/';
	    comp = slbuf;
	    if( ip->isize > 0 )
	    {
dprintf((" [%s]",dio.dirbase));
		bcopy(dio.dirbase,slbuf,ip->isize);
	    }
	    else
	    {
		comp++;
	    }
	    closei(ip);

	    /*
	     * start over again.
	     */
	    curino = pino;
	    goto top;
	}
    }

    return ip;

too_long:
    printf("name too long (max %d)\n",MAXNAME);
put_out:
    closei(ip);
out:
    return 0;
}

int
dopen(ip,dp)
    register struct fino *ip;
    register struct dio *dp;
{
    extern char *gmalloc();

    if( (ip->flag&IFMT) != IFDIR )
    {
dprintf((" dopen([0%o %d])",ip->flag,ip->isize));
	printf("Is not a directory\n");
	return -1;
    }

    if( dp->dirbase == 0 )
    if( (dp->dirbase = gmalloc(FsBlockSize)) == 0 )
	return -1;

    dp->dirfile = ip;
    ip->off = 0;
    dp->dircnt = 0;
    return 0;
}

dclose(dp)
    register struct dio *dp;
{
    iput(dp->dirfile);
    dp->dircnt = 0;
}

int
freadi(ip, bmap, cp)
    struct fino *ip;
    daddr_t (*bmap)();
    char *cp;
{
    register int rsize;
    register daddr_t bn, lbn;

    rsize = FsBlockSize;

    if( ip->off+rsize > ip->isize )
    {
	rsize = ip->isize - ip->off;

dprintf(("trunc %d", rsize));
	if( rsize <= 0 )
	    return 0;
    }

# define lblkno(x)	((x)>>FsBlockShift)
    lbn = lblkno(ip->off);
    bn = (*bmap)(ip, lbn);
dprintf((" bmap %d", bn));
    ip->off += rsize;

    if( bn == 0 )
    {
dprintf((" hole %d", lbn));
	bzero(cp, rsize);
	return rsize;
    }

dprintf((" *read(%d, $%x, %d)", FsbToDb(bn), cp, FsBlockSize));
    (*fs_bread)(FsbToDb(bn), cp, FsBlockSize);
    return rsize;
}

/*
 * getblk() --
 * try to keep indirect blocks around.
 */
char *
getblk(n,level)
    daddr_t n;
    register int level;
{
    extern char *gmalloc();

    register char *bp;

    if( (bp = iblockp[level]) == 0 )
    {
	bp = iblockp[level] = gmalloc(MAXFSBSIZE);
	if( bp == 0 )
	    return 0;
    }

    if( iblocknums[level] == n )
	return bp;

    if( n == 0 )
    {
	bzero(bp,FsBlockSize);
	return bp;
    }

    (*fs_bread)(FsbToDb(n),bp,FsBlockSize);
    iblocknums[level] = n;
    return bp;
}

flushblk()
{
    register int level;

    for( level = MAXINDIRECT+1; --level >= 0; )
	iblocknums[level] = -1;
}


# define FILESPERLINE	4
fs_list()
{
    register struct gendir *gp;
    register int i;

    if( dopen(fs_inode,&dio) < 0 )
	return;

    i = 0;

    while( (gp = (*fs_dread)(&dio)) != 0 )
    if( gp->d_ino != 0 )
    if( gp->d_name[0] != '.' )
    {
	char x[DIRSIZ+1];

	strncpy(x,gp->d_name,DIRSIZ);
	x[DIRSIZ] = 000;
	printf("%-18s",x);

	i++;
	if( i >= FILESPERLINE )
	{
	    i = 0;
	    newline();
	}
    }

    if( i != 0 )
	newline();

    dclose(&dio);
}

kprint(n)
    unsigned n;
{
    n /= 1024;

    if( n <= 0 )
	printf("half-K");
    else
	printf("%dK",n);
}
