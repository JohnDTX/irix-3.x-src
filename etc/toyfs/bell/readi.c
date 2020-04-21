# include "toyfs.h"
# include "bell_toyfs.h"

extern USR U;

# ifdef SYSTEM5
# define bcopy(s, t, n)	blt(t, s, n)
# endif SYSTEM5

off_t
bell_readi(ip, base, count, offset)
    register I *ip;
    char *base;
    int count;
    off_t offset;
{
    extern B *toy_bread();

    register B *bp;
    register daddr_t bn;
    register int lbn;
    register int brem; int boff;
    int frem;

    frem = ip->i_isize - offset;
    if( count > frem )
	count = frem;

    while( count > 0 )
    {
	lbn = offset >> FSBShift;
	boff = offset & FSBMask;
	brem = FSBSize - boff;
	if( brem > count )
	    brem = count;

	bn = bell_bmap_read(ip, lbn);

	if( bn < 0 )
	    break;
	if( bn == 0 )
	{
	    bzero(base, brem);
	    break;
	}

	if( (bp = toy_bread(ip->i_fs, bn)) == 0 )
	    break;
	bcopy(bp->b_addr+boff, base, brem);
	toy_brelse(ip->i_fs, bp);

	base += brem;
	offset += brem;
	count -= brem;
    }

    return offset;
}
