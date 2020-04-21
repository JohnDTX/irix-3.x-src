# ifdef BOOT_BKY

NOT DEBUGGED
/*
 * boot routines for bky file system.
 * NOT DEBUGGED
 */
# include "bky_types.h"
# include "bky_param.h"
# include "bky_inode.h"
# include "bky_fs.h"
# include "bky_dir.h"

# include "pmIImacros.h"
# define DBSHIFT	9
# include "safs.h"

# define PROMSTATIC


# define DEBUG do_debug
# include "dprintf.h"



PROMSTATIC
	struct fs *bky_Filsys;
	char *bky_ibuf;

PROMSTATIC
	struct fino igetino;
struct fino *
bky_iget(inum)
    long inum;
{
    extern char *gmalloc();

    register struct fino *ip;
    register struct dinode *dp;
    register struct fs *fp;
    register daddr_t bno;

    fp = bky_Filsys;

    if( bky_ibuf == 0 )
    if( (bky_ibuf = gmalloc(fp->fs_bsize)) == 0 )
	return 0;

    bno = fsbtodb(fp, itod(fp, inum));
    (*fs_bread)(bno, bky_ibuf, fp->fs_bsize);
    dp = (struct dinode *)bky_ibuf + itoo(fp, inum);

    ip = &igetino;
    ip->flag = dp->di_mode;
    ip->inum = inum;
    ip->isize = dp->di_size;
    ip->off = 0;

    bcopy(dp->di_db, ip->addrs, sizeof dp->di_db+sizeof dp->di_ib);

    return ip;
}

static daddr_t
bky_bmap(ip, bn)
    register struct fino *ip;
    daddr_t bn;
{
    int i, j, sh;
    daddr_t nb, *bap;

    if( bn < 0 )
    {
	printf("bn %d<0\n", bn);
	return 0;
    }

    /*
     * blocks 0..NDADDR are direct blocks
     */
    if( bn < NDADDR )
	return ip->addrs[bn];

    /*
     * addresses NIADDR have single and double indirect blocks.
     * the first step is to determine how many levels of indirection.
     */
    sh = 1;
    bn -= NDADDR;
    for (j = NIADDR; j > 0; j--)
    {
	sh *= NINDIR(bky_Filsys);
	if (bn < sh)
		break;
	bn -= sh;
    }
    if (j == 0)
    {
	printf("bn ovf %d\n", bn);
	return ((daddr_t)0);
    }

    /*
     * fetch the first indirect block address from the inode
     */
    nb = ip->addrs[NDADDR+NIADDR - j];
    if (nb == 0)
    {
	printf("bn void %d\n", bn);
	return ((daddr_t)0);
    }

    /*
     * fetch through the indirect blocks
     */
    for (; j <= NIADDR; j++)
    {
	bap = (daddr_t *)getblk(nb, j);
	sh /= NINDIR(bky_Filsys);
	i = (bn / sh) % NINDIR(bky_Filsys);
	nb = bap[i];
	if(nb == 0)
	{
	    printf("bn void %d\n", bn);
	    return ((daddr_t)0);
	}
    }

    return (nb);
}

PROMSTATIC
	struct gendir gendir1;
/*
 * get next entry in a directory.
 */
struct gendir *
bky_dread(dp)
    register struct dio *dp;
{
    register struct direct *ep;
    register struct gendir *gp;
    register int rsize;

    if( dp->dircnt <= 0 )
    {
	dp->dirptr = (struct direct *)dp->dirbase;
	if( (rsize = bky_readi(dp->dirfile, (char *)dp->dirptr)) <= 0 )
	    return 0;
	dp->dircnt = rsize;
    }

    ep = dp->dirptr;
    dp->dirptr = (struct direct *)( (char *)ep + ep->d_reclen );

    gp = &gendir1;
    gp->d_ino = ep->d_ino;
    gp->d_name = ep->d_name;
    gp->d_len = ep->d_namlen+1;

    return gp;
}

int
bky_readi(ip, cp)
    register struct fino *ip;
    char *cp;
{
    register struct fs *fs;
    register int rsize;
    register daddr_t bn, lbn;

    fs = bky_Filsys;

    rsize = FsBlockSize;

    if( ip->off+rsize >= ip->isize )
    {
	rsize = ip->isize - ip->off;

dprintf((" trunc %d", rsize));
	if( rsize <= 0 )
	    return 0;
    }

    lbn = lblkno(fs, ip->off);
    bn = bky_bmap(ip, lbn);
    ip->off += rsize;

    rsize = blksize(fs, ip, lbn);

    if( bn == 0 )
    {
dprintf((" hole %d", lbn));
	bzero(cp, rsize);
	return rsize;
    }

    (*fs_bread)(fsbtodb(fs, bn), cp, rsize);
    return rsize;
}


PROMSTATIC
	struct fs *bky_Filsys;

int
bky_probefs(readfunc, _fstype)
    int (*readfunc)();
    int *(_fstype);
{
    extern char *gmalloc();

    register struct fs *fp;

    if( bky_Filsys == 0 )
    if( (bky_Filsys = (struct fs *)gmalloc(SBSIZE)) == 0 )
	return 0;

    fp = bky_Filsys;

    if( (*readfunc)(SBLOCK, (char *)fp, SBSIZE) )
	return 0;

    if( fp->fs_magic != FS_MAGIC )
	return 0;

    *_fstype = fp->fs_bsize>>DBSHIFT;

    FsBlockShift = fp->fs_fsbtodb;
    FsBlockSize = fp->fs_bsize;

    printf("BKY ");
    kprint(fp->fs_bsize);
    printf("/");
    kprint(fp->fs_fsize);
    printf(" FS");

    return 1;
}

# else  BOOT_BKY

int
bky_probefs()
{
    return 0;
}

# endif BOOT_BKY
