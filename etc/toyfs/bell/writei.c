# include "toyfs.h"
# include "bell_toyfs.h"

extern USR U;

# ifdef SYSTEM5
# define bcopy(s, t, n)	blt(t, s, n)
# endif SYSTEM5

off_t
bell_writei(ip, base, count, offset)
    register I *ip;
    char *base;
    int count;
    off_t offset;
{
    extern B *toy_bread(), *toy_getblk();

    register B *bp;
    register daddr_t bn;
    register int lbn;
    register int brem; int boff;
    register struct bell_toyinode *dip;
    int frem;

    dip = (struct bell_toyinode *)ip->i_dinode;

    while( count > 0 )
    {
	lbn = offset >> FSBShift;
	boff = offset & FSBMask;
	brem = FSBSize - boff;
	if( brem > count )
	    brem = count;

	bn = bell_bmap_write(ip, lbn);

	if( U.u_errmsg != 0 )
	    break;

	if( boff == 0 && brem == FSBSize )
	    bp = toy_bread(ip->i_fs, bn);
	else
	    bp = toy_getblk(ip->i_fs, bn);
	if( bp == 0 )
	    break;

	bcopy(base, bp->b_addr+boff, brem);
	bp->b_flags |= TOY_DIRTY;
	bp->b_flags &= ~TOY_INVAL;
	toy_brelse(ip->i_fs, bp);

	base += brem;
	offset += brem;
	if( offset >= ip->i_isize )
	    ip->i_isize = offset;
	count -= brem;
    }

    return offset;
}
