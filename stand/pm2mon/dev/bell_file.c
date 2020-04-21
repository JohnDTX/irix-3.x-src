#
# include "pmII.h"

# include "sys/types.h"
# include "dklabel.h"

# include "bell_fs.h"
# include "bell_inode.h"
# include "bell_ino.h"
# include "bell_dir.h"
# include "safs.h"



# undef  DEBUG do_debug
# include "dprintf.h"
# define MIN(a, b)	((a)<(b)?(a):(b))

# define itod(n)	(INODEBLOCK+(unsigned)((n)-FIRSTINO)/NinodesPerBlock)
# define itoo(n)	((unsigned)((n)-FIRSTINO)%NinodesPerBlock)


int
bell_probefs(readfunc, _fstype)
    int (*readfunc)();
    int *(_fstype);
{
    union
    {
	char c[DBLOCK];
	struct filsys f;
    }   x;

    if( (*readfunc)(SUPERBLOCK, x.c, DBLOCK) )
	return 0;

    if( x.f.s_magic != FsMAGIC )
	return 0;

    if( x.f.s_type == Fs2b )
    {
	*_fstype = Fs2b;
	FsDblockShift = 1;
    }
    else
    {
	*_fstype = Fs1b;
	FsDblockShift = 0;
    }

    FsBlockSize = DBLOCK<<FsDblockShift;
    FsBlockShift = FsDblockShift+DBSHIFT;
    FsBlockOffMask = ~(~0<<FsBlockShift);

    NaddrsPerBlock = FsBlockSize / sizeof (daddr_t);
    NinodesPerBlock = FsBlockSize / sizeof (struct dinode);

    fs_bread = readfunc;
    { extern struct fino *bell_iget(); fs_iget = bell_iget; }
    { extern struct gendir *bell_dread(); fs_dread = bell_dread; }
    { extern int bell_readi(); fs_readi = bell_readi; }

    kprint(FsBlockSize);
    printf(" FS");
    return 1;
}


PROMSTATIC
	struct fino igetino;		/* private for bell_iget */
	char *bell_ibuf;

struct fino *
bell_iget(inum)
    register long inum;
{
    extern char *gmalloc();

    register struct dinode *dip;
    register char *cp, *tp;
    register i;
    register struct fino *ip;

    flushblk();

    if( bell_ibuf == 0 )
    if( (bell_ibuf = gmalloc(FsBlockSize)) == 0 )
	return 0;

dprintf((" bell_iget(%d)", inum));
dprintf((" *read(%d, $%x, %d)", FsbToDb(itod(inum)), bell_ibuf, FsBlockSize));
    if( (*fs_bread)(FsbToDb(itod(inum)), bell_ibuf, FsBlockSize) )
    {
	printf("? Can't read inum %d\n", inum);
	return 0;
    }

    dip = (struct dinode *)bell_ibuf + itoo(inum);

    /* convert dinode to ino */
    ip = &igetino;
    ip->flag = dip->di_mode;
    ip->inum = inum;
    ip->isize = dip->di_size;

    ip->off = 0;
dprintf((" mode 0%o, size %d", ip->flag, ip->isize));

    cp = dip->di_addr; tp = (char *)ip->addrs;
    for( i = NADDR; --i >= 0; )
    {
	*tp++ = 0;
	*tp++ = *cp++;
	*tp++ = *cp++;
	*tp++ = *cp++;
    }

    return ip;
}

int
bell_readi(ip, cp)
    struct fino *ip;
    char *cp;
{
    extern daddr_t bell_bmap();

    return freadi(ip, bell_bmap, cp);
}

PROMSTATIC
	struct gendir gendir1;

struct gendir *
bell_dread(dp)
    register struct dio *dp;
{
    register struct direct *ep;

    register int rsize;

    if( dp->dircnt < sizeof (struct direct) )
    {
	dp->dirptr = (struct direct *)dp->dirbase;
	if( (rsize = (*fs_readi)(dp->dirfile, (char *)dp->dirptr)) <= 0 )
	    return 0;
	dp->dircnt = rsize;
    }

    dp->dircnt -= sizeof (struct direct);
    ep = dp->dirptr++;

    gendir1.d_ino = ep->d_ino;
    gendir1.d_name = ep->d_name;
    gendir1.d_len = DIRSIZ;
    return &gendir1;
}

daddr_t
bell_bmap(ip, n)
    register struct fino *ip;
    daddr_t n;
{
    extern char *getblk();

    int iboff[NIADDRS];
    register int idlev;

    idlev = NIADDRS;

    if( n < 0 )
	return 0;

    /*file blocks 0...MAXDIRECT-1 are direct blocks*/
    if( n < NDIRECT )
	return ip->addrs[n];

    /*
     * addresses MAXDIRECT...NADDR-1
     * are single...multiple indirect blocks.
     * the first step is to determine
     * how many levels of indirection, and
     * at what logical offsets within the
     * indirect blocks.
     */
    n -= NDIRECT;
    for( ;; )
    {
	if( --idlev < 0 )
	    return 0;
	iboff[idlev] = n%NaddrsPerBlock;
	n /= NaddrsPerBlock;
	if( --n < 0 )
	    break;
    }

    /* fetch through indirect blocks */
    n = ip->addrs[NADDR - idlev - 1];
    while( idlev < NIADDRS )
    {
	n = ((daddr_t *)getblk(n, idlev))[ iboff[idlev] ];
	idlev++;
    }

    return n;
}
