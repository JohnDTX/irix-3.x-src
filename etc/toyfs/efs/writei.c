# include "toyfs.h"
# include "efs_toyfs.h"

extern USR U;

# ifdef SYSTEM5
# define bcopy(s, t, n)	blt(t, s, n)
# endif SYSTEM5

off_t
efs_writei(ip, base, count, offset)
    register I *ip;
    char *base;
    int count;
    off_t offset;
{
    extern B *toy_getblk(), *toy_bread();

    register B *bp;
    register daddr_t bn;
    register int lbn;
    register struct efs_toyinode *dip;
    register int brem; int boff;

    dip = (struct efs_toyinode *)ip->i_dinode;

    while( count > 0 )
    {
	lbn = offset >> BBShift;
	boff = offset & BBMask;
	brem = BBSize - boff;
	if( brem > count )
	    brem = count;

	bn = efs_bmap_write(ip, lbn);

	if( U.u_errmsg != 0 )
	    break;

	if( boff == 0 && brem == BBSize )
	    bp = toy_getblk(ip->i_fs, bn);
	else
	    bp = toy_bread(ip->i_fs, bn);
	if( bp == 0 )
	    break;

	bcopy(base, bp->b_addr + boff, brem);
	bp->b_flags |= TOY_DIRTY;
	bp->b_flags &= ~TOY_INVAL;
	toy_brelse(ip->i_fs, bp);

	base += brem;
	offset += brem;
	if( offset >= ip->i_isize )
	{
	    ip->i_flags |= TOY_DIRTY;
	    ip->i_isize = offset;
	}
	count -= brem;
    }

    return offset;
}
