# define BOOT_EFS
# define EFS_CACSIZE	8192

# ifdef BOOT_EFS

#
# include "pmII.h"

# include "sys/types.h"
# include "dklabel.h"

# define BBShift	9
# define BBSize	(1<<BBShift)
# define BBMask	(BBSize-1)
# include "efs_fs.h"
# include "efs_inode.h"
# include "bell_dir.h"
# include "safs.h"
# define ExtsPerBB	(BBSize / sizeof (struct extent))
# define ExtsPerDinode	EFS_DIRECTEXTENTS
# define InodesPerBB	(BBSize / sizeof (struct efs_dinode))



# undef  DEBUG do_debug
# include "dprintf.h"

PROMSTATIC
	long Firstcg;
	long Cgfsize;
	short Ipcg;
	char *efs_ibuf;

# ifdef EFS_CACSIZE
PROMSTATIC
	int (*efs_bread)();
struct cac
{
    char *bp;		/* ptr to cached data */
    int nbytes;		/* number of bytes cached */
    daddr_t bn;		/* disk block number first cached */
} efscac;
extern int efs_cacread();
				/*FsBlockShift-FsDblockShift*/
# define SectorsToBytes(x)	((x)<<(BBShift-0))
# endif EFS_CACSIZE

int
efs_probefs(readfunc, _fstype)
    int (*readfunc)();
    int *(_fstype);
{
    union
    {
	char c[BBSize];
	struct efs f;
    }   x;

    if( (*readfunc)(SUPERBLOCK, x.c, DBLOCK) )
	return 0;

    if( x.f.fs_magic != EFS_MAGIC )
	return 0;

    FsDblockShift = 0;
    FsBlockSize = BBSize;
    FsBlockShift = BBShift;
    FsBlockOffMask = BBSize-1;

# ifdef EFS_CACSIZE
    efs_bread = readfunc;
    fs_bread = efs_cacread;
# else  EFS_CACSIZE
    fs_bread = readfunc;
# endif EFS_CACSIZE
    Firstcg = x.f.fs_firstcg;
    Cgfsize = x.f.fs_cgfsize;
    Ipcg = x.f.fs_cgisize * InodesPerBB;
    { extern struct fino *efs_iget(); fs_iget = efs_iget; }
    { extern struct gendir *bell_dread(); fs_dread = bell_dread; }
    { extern int efs_readi(); fs_readi = efs_readi; }

    printf("EFS");
    return 1;
}


PROMSTATIC
	struct fino igetino;		/* private for efs_iget */
struct fino *
efs_iget(inum)
    register long inum;
{
    extern char *gmalloc();

    register struct efs_dinode *dip;
    register i;
    register struct fino *ip;

    flushblk();
# ifdef EFS_CACSIZE
    efscac.nbytes = 0;
# endif EFS_CACSIZE

    if( efs_ibuf == 0 )
    if( (efs_ibuf = gmalloc(BBSize)) == 0 )
	return 0;

dprintf((" efs_iget(%d)", inum));
dprintf((" *read(%d, $%x, %d)", FsbToDb(ITOD(0, inum)), efs_ibuf, BBSize));
    if( (*fs_bread)(FsbToDb(ITOD(0, inum)), efs_ibuf, BBSize) )
    {
	printf("? Can't read inum %d\n", inum);
	return 0;
    }

    dip = (struct efs_dinode *)efs_ibuf + ITOO(0, inum);

    /* convert dinode to ino */
    ip = &igetino;
    ip->flag = dip->di_mode;
    ip->inum = inum;
    ip->isize = dip->di_size;

    ip->off = 0;
dprintf((" mode 0%o, size %d", ip->flag, ip->isize));

    ip->naddrs = dip->di_numextents;
    bcopy((char *)&dip->di_u, (char *)ip->addrs, sizeof dip->di_u);

    return ip;
}

int
efs_readi(ip, cp)
    struct fino *ip;
    char *cp;
{
    extern daddr_t efs_bmap();

    return freadi(ip, efs_bmap, cp);
}


daddr_t
efs_bmap(ip, lbn)
    register struct fino *ip;
    daddr_t lbn;
{
    extern daddr_t efs_mapext(), efs_ixmapext();

    if( ip->naddrs <= EFS_DIRECTEXTENTS )
	return efs_mapext((struct extent *)ip->addrs, ip->naddrs, lbn);
    else
	return efs_ixmapext((struct extent *)ip->addrs, ip->naddrs, lbn);
}

daddr_t
efs_ixmapext(xp, nx, lbn)
    register struct extent *xp;
    int nx;
    daddr_t lbn;
{
    extern daddr_t efs_imapext();
    register int bnx, inx;
    register daddr_t rv;

    for( inx = ExtsPerDinode; nx > 0 && --inx >= 0; )
    {
	if( (bnx = xp->ex_length*ExtsPerBB) > nx )
	    bnx = nx;
	if( (rv = efs_imapext(xp, bnx, lbn)) >= 0 )
	    return rv;

	xp++;
	nx -= bnx;
    }

    return -1;
}

daddr_t
efs_imapext(xp, nx, lbn)
    register struct extent *xp;
    int nx;
    daddr_t lbn;
{
    extern char *getblk();
    extern daddr_t efs_mapext();

    register char *bp;
    register daddr_t nxblk, rv;

    /*
     * Check the indirect extent, 
     * one block at a time.
     */
    nxblk = xp->ex_bn;
    while( nx > 0 )
    {
	if( (bp = getblk(FsbToDb(nxblk), 1)) == 0 )
	    return 0;
	
	if( (rv = efs_mapext((struct extent *)bp
		, nx>ExtsPerBB?ExtsPerBB:nx, lbn)) >= 0 )
	    return rv;

	nxblk++;
	nx -= ExtsPerBB;
    }

    return -1;
}

/*
 * loop over extents.
 * return 0 for hole.
 * return -1 for EOF.
 */
daddr_t
efs_mapext(xp, n, lbn)
    register struct extent *xp;
    register int n;
    register int lbn;
{
    register int exoff;

    while( --n >= 0 )
    {
	exoff = lbn - xp->ex_offset;

	if( exoff < 0 )
	    return 0;
	if( exoff < (int)xp->ex_length )
	{
# ifdef EFS_CACSIZE
	    efs_encac((daddr_t)FsbToDb(xp->ex_bn+exoff),
		    FsbToBytes(xp->ex_length-exoff));
# endif EFS_CACSIZE
	    return xp->ex_bn + exoff;
	}

	xp++;
    }

    return -1;
}

# ifdef EFS_CACSIZE

efs_encac(bn, nbytes)
    daddr_t bn;
    int nbytes;
{
    extern char *efs_cack();
    extern char *gmalloc();
    register char *bp;

    if( efs_cack(bn, 1) != 0 )
	return;

dprintf((" encac(%d,%d)\n",bn,nbytes));
    if( (bp = efscac.bp) == 0 )
    {
	if( (bp = gmalloc(EFS_CACSIZE)) == 0)
	    return 0;
	efscac.bp = bp;
    }
    if( nbytes > EFS_CACSIZE )
	nbytes = EFS_CACSIZE;
    (*efs_bread)(bn, bp, nbytes);
    efscac.nbytes = nbytes;
    efscac.bn = bn;
}

int
efs_cacread(bn, cp, n)
    daddr_t bn;
    char *cp;
    int n;
{
    extern char *efs_cack();
    register char *bp;

    if( (bp = efs_cack(bn, n)) != 0 )
    {
dprintf((" hit(%d,%d)",bn,n));
	bcopy(bp, cp, n);
	return 0;
    }
dprintf((" miss(%d,%d)",bn,n));
    return (*efs_bread)(bn, cp, n);
}

char *
efs_cack(bn, nbytes)
    register daddr_t bn;
    int nbytes;
{
    register long byteoff;

    if( bn < efscac.bn )
	return 0;
    bn -= efscac.bn;
    byteoff = SectorsToBytes(bn);
    if( byteoff + nbytes <= efscac.nbytes )
	return efscac.bp + byteoff;
    return 0;
}
# endif EFS_CACSIZE

# else  BOOT_EFS

int
efs_probe()
{
    return 0;
}

# endif BOOT_EFS
