# include "toyfs.h"
# include "efs_toyfs.h"


extern daddr_t efs_mapext();
extern B *toy_bread();


daddr_t
efs_bmap_read(ip, lbn)
    I *ip;
    int lbn;
{
    FS *sp;
    register struct efs_dinode *dip;
    register int nx;
    register daddr_t rv;

    sp = ip->i_fs;
    dip = (struct efs_dinode *)ip->i_dinode;
    nx = dip->di_nx;

    /*
     * Check direct extents.
     */
    if( nx <= ExtsPerInode )
    {
	rv = efs_mapext(dip->di_x, nx, lbn);
	return rv;
    }
    else
    {
	register B *bp;
	int nxblk;

	/*
	 * Check the indirect extent, 
	 * one block at a time.
	 */
	nxblk = dip->di_x[0].ex_bn;
	while( nx > 0 )
	{
	    if( (bp = toy_bread(sp, nxblk)) == 0 )
		return 0;
	    
	    rv = efs_mapext((struct extent *)bp->b_addr
		    , nx>ExtsPerBB?ExtsPerBB:nx, lbn);
	    toy_brelse(sp, bp);
	    if( rv >= 0 )
		return rv;

	    nxblk++;
	    nx -= ExtsPerBB;
	}

	return -1;
    }
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
	    return xp->ex_bn + exoff;

	xp++;
    }

    return -1;
}
