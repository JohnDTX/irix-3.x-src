# include "toyfs.h"
# include "efs_toyfs.h"

# define DEBUG toy_debug

extern USR U;


extern daddr_t efs_noshove_map();

int efs_maxext = MaxExtLength;	/* patchable maximum extent length */

/*
 * efs_bmap_write() --
 * map a logical block number within a file
 * to a physical block number on its volume.
 * this may involve (1) allocating a new block
 * (or (2) re-using an already-mapped block),
 * which may involve (3) allocating a new extent
 * (or (4) extending an existing extent),
 * which may involve (5) creating or (6) growing
 * an indirect extent (or (7) just tweaking it).
 */
daddr_t
efs_bmap_write(ip, lbn)
    I *ip;
    register int lbn;
{
    extern daddr_t efs_dkalloc();

    register struct efs_toyinode *dip;
    register struct extent *xp;
    register int nx;
    int wx;
    daddr_t bn;
    struct extent x;

    dip = (struct efs_toyinode *)ip->i_dinode;
    xp = dip->inode.di_xp+0;
    nx = dip->inode.di_nx;

    x.ex_offset = lbn;
    x.ex_length = 1;
    x.ex_magic = 0;

    if( nx <= ExtsPerInode )
    {
	if( (bn = efs_noshove_map(ip->i_fs,
		xp, nx, lbn, &wx)) > 0 )
	    return bn;

	/*
	 * need a new extent.
	 * if there is room in the inode, stick it right in.
	 * otherwise, transition to indirect extent.
	 * in either case, we need another data block.
	 */
	if( (bn = efs_dkalloc(ip, 1)) == 0 )
	    return -1;

	x.ex_bn = bn;
	if( nx < ExtsPerInode )
	{
	    efs_shove(xp, nx, wx, &x);
	    dip->inode.di_nx++;
	}
	else
	{
	    if( efs_tshove(ip, wx, &x) < 0 )
		return -1;
	}
    }
    else
    {
	extern B *toy_nbread();
	register B *bp;

	if( (bp = toy_nbread(ip->i_fs,
		(daddr_t)xp->ex_bn, (int)xp->ex_length<<BBShift)) == 0 )
	    return -1;

	if( (bn = efs_noshove_map(ip->i_fs,
		(struct extent *)bp->b_addr, nx, lbn, &wx)) > 0 )
	{
	    bp->b_flags |= TOY_DIRTY;
	    toy_brelse(ip->i_fs, bp);
	    return bn;
	}

	/*
	 * allocate the needed data block.
	 */
	if( (bn = efs_dkalloc(ip, 1)) == 0 )
	{
	    toy_brelse(ip->i_fs, bp);
	    return -1;
	}

	x.ex_bn = bn;
	if( nx*sizeof (struct extent) < bp->b_bcount )
	{
	    efs_shove((struct extent *)bp->b_addr, nx, wx, &x);
	    bp->b_flags |= TOY_DIRTY;
	    toy_brelse(ip->i_fs, bp);

	    dip->inode.di_nx++;
	}
	else
	{
	    toy_brelse(ip->i_fs, bp);
	    if( efs_iexpand(ip, wx, &x) < 0 )
		return -1;
	}
    }

    ip->i_flags |= TOY_DIRTY;

    return bn;
}

/*
 * efs_noshove_map() --
 * try to map the block without shoving anything.
 * if this works, just return the physical block number.
 * otherwise, return 0 and pass back the number of the
 * new extent that will be needed (from efs_findext()).
 */
daddr_t
efs_noshove_map(sp, xp, nx, lbn, _wx)
    register FS *sp;
    register struct extent *xp;
    int nx;
    daddr_t lbn;
    int *(_wx);
{
    register struct extent *wxp;
    int exoff;
    int wx;

    wx = efs_findext(xp, nx, lbn, &exoff);
    wxp = xp + wx;

    *_wx = wx;

    /*
     * if the block lies within an existing
     * extent, just rewrite.
     */
    if( wx < nx && exoff >= 0 )
	return wxp->ex_bn + exoff;

    wxp--;
    /*
     * If there is at least one extent already, check
     * the previous extent to see if it can be extended.
     * ie, if the the block number falls at the end of
     * the previous extent, and the extent is not already
     * of maximum size, and the next physical block after
     * the extent is available.
     */
    if( nx > 0 && wx > 0
     && wxp->ex_offset + wxp->ex_length == lbn
     && wxp->ex_length < efs_maxext )
    {
	daddr_t xb;

	xb = wxp->ex_bn + wxp->ex_length;

	if( efs_adj_dkalloc(sp, xb) == 0 )
	{
	    register int kluge;

	    /* wxp->ex_length++; */
	    kluge = wxp->ex_length + 1;
	    wxp->ex_length = kluge;
	    return xb;
	}
    }

    return -1;
}

/*
 * efs_findext() --
 * find the extent for a given logical block.
 * return the extent index and pass back the
 * offset from that extent.  if the block
 * falls within a holey region, the index is
 * that of the extent which it will occupy
 * after expansion.  if the block is past
 * the end, the index is nx.
 */
int
efs_findext(xp, nx, lbn, _exoff)
    register struct extent *xp;
    int nx;
    register int lbn;
    int (*_exoff);
{
    register int i;
    register int exoff;

    for( i = 0; i < nx; i++ )
    {
	exoff = lbn - xp->ex_offset;
	if( exoff < (int)xp->ex_length )
	{
	    *_exoff = exoff;
	    break;
	}
	xp++;
    }

    return i;
}

/*
 * efs_tshove() --
 * transition from direct to indirect inode.
 */
int
efs_tshove(ip, wx, wxp)
    register I *ip;
    int wx;
    struct extent *wxp;
{
    extern daddr_t efs_dkalloc();
    extern B *toy_getblk();

    register B *bp;
    register struct efs_toyinode *dip;
    struct extent x;
    daddr_t ibn;

    dip = (struct efs_toyinode *)ip->i_dinode;

    x.ex_length = 1;
    x.ex_magic = 0;
    x.ex_offset = 0;
    if( (x.ex_bn = efs_dkalloc(ip, 1)) == 0 )
	return -1;
    if( (bp = toy_getblk(ip->i_fs, (daddr_t)x.ex_bn)) == 0 )
	return -1;

    /* bzero((char *)bp->b_addr, BBSize); */
    bcopy((char *)&dip->inode.di_u, bp->b_addr, sizeof dip->inode.di_u);
    bzero((char *)&dip->inode.di_u, sizeof dip->inode.di_u);

    efs_shove((struct extent *)bp->b_addr, (int)dip->inode.di_nx, wx, wxp);
    *dip->inode.di_xp = x;
    dip->inode.di_nx++;
    ip->i_flags |= TOY_DIRTY;

    bp->b_flags |= TOY_DIRTY;
    toy_brelse(ip->i_fs, bp);

    return 0;
}

/*
 * efs_iexpand() --
 * expand an indirect extent!
 */
int
efs_iexpand(ip, wx, wxp)
    register I *ip;
    int wx;
    struct extent *wxp;
{
    extern B *toy_nbread(), *toy_ngetblk();
    extern daddr_t efs_dkalloc();

    register B *bp, *wbp;
    register struct extent *xp;
    register struct efs_toyinode *dip;
    struct extent x;
    register int nx;

    dip = (struct efs_toyinode *)ip->i_dinode;
    xp = dip->inode.di_xp+0;
    nx = dip->inode.di_nx;
    
    if( (bp = toy_nbread(ip->i_fs,
	    (daddr_t)xp->ex_bn, (int)xp->ex_length<<BBShift)) == 0 )
	return -1;
    x.ex_magic = 0;
    x.ex_offset = 0;
    x.ex_length = dip->inode.di_xp[0].ex_length + 1;

    if( (x.ex_bn = efs_dkalloc(ip, (int)x.ex_length)) == 0 )
	return -1;
    if( (wbp = toy_ngetblk(ip->i_fs,
	    (daddr_t)x.ex_bn, (int)x.ex_length<<BBShift)) == 0 )
    {
	toy_brelse(ip->i_fs, bp);
	return -1;
    }

    /* bzero((char *)wbp->b_addr + wbp->b_bcount - BBSize, BBSize); */
    bcopy((char *)bp->b_addr, (char *)wbp->b_addr, nx*sizeof (struct extent));
    toy_brelse(ip->i_fs, bp);
    efs_shove((struct extent *)wbp->b_addr, wx, nx, wxp);
    wbp->b_flags |= TOY_DIRTY;
    toy_brelse(ip->i_fs, wbp);

    efs_dkfree(ip->i_fs, (daddr_t)xp->ex_bn, (int)xp->ex_length);
    *xp = x;
    dip->inode.di_nx++;
    ip->i_flags |= TOY_DIRTY;
    return 0;
}

efs_shove(xp, nx, wx, wxp)
    register struct extent *xp;
    int nx, wx;
    struct extent *wxp;
{
    register int nxa;

    xp += nx;
    nxa = nx - wx;

    /* shove everything past the new extent */
    while( --nxa >= 0 )
    {
	xp[0] = xp[-1];
	xp--;
    }

    /* tweak new extent */
    *xp = *wxp;
}
