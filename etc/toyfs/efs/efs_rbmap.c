# include "toyfs.h"
# include "efs_toyfs.h"


extern daddr_t efs_mapext();
extern daddr_t efs_ixmapext();


daddr_t
efs_bmap_read(ip, lbn)
    I *ip;
    int lbn;
{
    register struct efs_toyinode *dip;
    register struct extent *xp;
    register int nx;

    dip = (struct efs_toyinode *)ip->i_dinode;
    xp = dip->inode.di_xp+0;
    nx = dip->inode.di_nx;

    if( nx <= ExtsPerInode )
	return efs_mapext(xp, nx, lbn);
    else
	return efs_ixmapext(ip, xp, nx, lbn);
}

daddr_t
efs_ixmapext(ip, xp, nx, lbn)
    register I *ip;
    register struct extent *xp;
    int nx;
    daddr_t lbn;
{
    extern B *toy_nbread();

    register B *bp;
    register int bnx;
    register daddr_t bn;

    while( nx > 0 )
    {
	bnx = xp->ex_length<<ExtsPerBBShift;
	if( bnx > nx )
	    bnx = nx;

	if( (bp = toy_nbread(ip->i_fs,
		(daddr_t)xp->ex_bn, (int)xp->ex_length<<BBShift)) == 0 )
	    return -1;
	bn = efs_mapext((struct extent *)bp->b_addr, bnx, lbn);
	toy_brelse(ip->i_fs, bp);

	if( bn >= 0 )
	    return bn;

	xp++;
	nx -= bnx;
    }

    return -1;
}

/*
 * given a list of extents,
 * map a logical block number to a physical block number.
 * return the physical block number; 0 for hole; -1 for EOF.
 */
daddr_t
efs_mapext(xp, nx, lbn)
    register struct extent *xp;
    register int nx;
    register int lbn;
{
    register int exoff;

    while( --nx >= 0 )
    {
	exoff = lbn - (daddr_t)xp->ex_offset;

	if( exoff < 0 )
	    return 0;
	if( exoff < (int)xp->ex_length )
	    return (daddr_t)xp->ex_bn + exoff;

	xp++;
    }

    return -1;
}
