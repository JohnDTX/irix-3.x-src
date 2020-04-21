# include "toyfs.h"
# include "efs_toyfs.h"

# define DEBUG toy_debug
# include "dprintf.h"

extern USR U;


extern daddr_t efs_dkalloc();

daddr_t
efs_bmap_write(ip, lbn)
    I *ip;
    register int lbn;
{
    struct efs_toyfs *esp;
    register struct efs_dinode *dip;
    register struct extent *xp;
    daddr_t bn;
    register int nx, wx;
    register int nxblk;

    esp = (struct efs_toyfs *)ip->i_fs->fs_filsys;
    dip = (struct efs_dinode *)ip->i_dinode;

    nx = dip->di_nx;

    /*
     * Case1, using direct extents.
     */
    if( nx <= ExtsPerInode )
    {
	auto int exoff;

	wx = efs_findext(dip->di_x, nx, lbn, &exoff);
	xp = dip->di_x+wx;

	/*
	 * if the block lies within an existing
	 * extent, just rewrite.
	 */
	if( wx < nx && exoff >= 0 )
	    return xp->ex_bn + exoff;

	/*
	 * If there is at least one extent already, check
	 * the previous extent to see if it can be extended.
	 * ie, if the the block number falls at the end of
	 * the previous extent, and the extent is not already
	 * of maximum size, and the next physical block after
	 * the extent is available.
	 */
	if( nx > 0 && wx > 0
	 && (xp-1)->ex_offset + (xp-1)->ex_length == lbn
	 && (xp-1)->ex_length < MaxExtLength-1 )
	{
	    register char *b;
	    daddr_t xb;

	    b = esp->bbitmap;
	    xb = (xp-1)->ex_bn + (xp-1)->ex_length;

	    if( GETBIT(b, xb) )
	    {
		register int kluge;

		esp->filsys.fs_tfree--;
		ip->i_fs->fs_flags |= TOY_DIRTY;
		CLRBIT(b, xb);
		/* (xp-1)->ex_length++; */
		kluge = (xp-1)->ex_length + 1;
		(xp-1)->ex_length = kluge;
		return xb;
	    }
	}

	/*
	 * need a new extent.
	 */
	if( efs_newext(esp, ip, wx) < 0 )
	    return -1;

	xp->ex_offset = lbn;
	xp->ex_length = 0;
	if( (bn = efs_dkalloc(ip)) == 0 )
	    return -1;
	xp->ex_bn = bn;
	xp->ex_length = 1;
	return bn;
    }

    if( (bn = efs_bmap_read(ip, lbn)) > 0 )
	return bn;

    U.u_errmsg = "Indirect extents NOT IMPLEMENTED";
    return -1;
}

/*
 * find the extent for a given logical block.
 * if the block falls within a holey region,
 * return the extent which the block will occupy.
 * return the extent index.
 */
int
efs_findext(xp, n, lbn, _exoff)
    register struct extent *xp;
    int n;
    register int lbn;
    int (*_exoff);
{
    register int i;
    register int exoff;

    for( i = 0; i < n; i++ )
    {
	exoff = lbn - xp->ex_offset;
	if( exoff < (int)xp->ex_length )
	{
	    *_exoff = exoff;
	    break;
	}
	xp++;
    }

dprintf((" findext(%d) %d",n,i));
    return i;
}

int
efs_newext(sp, ip, wx)
    struct efs_toyfs *sp;
    I *ip;
    int wx;
{
    register struct efs_toyinode *dip;
    register struct extent *xp;
    register int nxblk;
    register int i;
    int nxa;

dprintf((" newext(%d)",wx));
    dip = (struct efs_toyinode *)ip->i_dinode;
    nxa = dip->inode.di_nx - wx;

    /*
     * Is there room in the inode?
     */
    if( dip->inode.di_nx < ExtsPerInode )
    {
	xp = dip->inode.di_x+dip->inode.di_nx;
	/* shove everything past the new extent */
	while( --nxa >= 0 )
	{
	    xp[0] = xp[-1];
	    xp--;
	}
	xp->ex_length = 0;
	xp->ex_bn = 0;
	dip->inode.di_nx++;
	ip->i_flags |= TOY_DIRTY;
	return 0;
    }

    U.u_errmsg = "Indirect extents NOT IMPLEMENTED";
    return -1;
}
