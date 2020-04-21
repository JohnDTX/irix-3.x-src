# include "toyfs.h"
# include "bell_toyfs.h"

extern USR U;


daddr_t
bell_bmap_write(ip, lbn)
    I *ip;
    register daddr_t lbn;
{
    extern B *toy_bread(), *toy_getblk();

    int iboff[IndirectAddrsPerInode];

    register struct bell_toyinode *dip;
    register int idlev;
    register B *bp;
    register int pbn;

    dip = (struct bell_toyinode *)ip->i_dinode;
    idlev = IndirectAddrsPerInode;
    U.u_niblocks = 0;
    U.u_iblocks[0] = 0;

    if( lbn < 0 )
    {
	U.u_errmsg = "EOF";
	return 0;
    }

    /*file blocks 0...DirectAddrsPerInode are direct blocks*/
    if( lbn < DirectAddrsPerInode )
    {
	pbn = dip->daddrs[lbn];
	if( pbn == 0 )
	{
	    pbn = bell_dkalloc(ip);
	    if( U.u_errmsg != 0 )
		return 0;
	    dip->daddrs[lbn] = pbn;
	    if( bell_iwrite(ip) < 0 )
		return 0;
	}
	U.u_iblocks[0] = pbn;
	return pbn;
    }

    /*
     *addresses NADDR-NINADR...NADDR-1
     *are single...multiple indirect blocks.
     *the first step is to determine
     *how many levels of indirection, and
     *at what logical offsets within the
     *indirect blocks.
     */
    lbn -= DirectAddrsPerInode;
    for( ;; )
    {
	if( --idlev < 0 )
	{
	    U.u_errmsg = "EOF";
	    return 0;
	}
	iboff[idlev] = lbn&AddrsPerFSBMask;
	lbn >>= AddrsPerFSBShift;
	if( --lbn < 0 )
	    break;
    }

    /*fetch through indirect blocks*/
    lbn = AddrsPerInode-idlev-1;
    pbn = dip->daddrs[lbn];
    if( pbn == 0 )
    {
	pbn = bell_dkalloc(ip);
	if( U.u_errmsg != 0 )
	    return 0;
	dip->daddrs[lbn] = pbn;
	if( bell_iwrite(ip) < 0 )
	    return 0;
    }
    while( idlev < IndirectAddrsPerInode )
    {
	U.u_niblocks++;

	U.u_iblocks[IndirectAddrsPerInode-idlev] = pbn;
	bp = toy_bread(ip->i_fs, pbn);
	if( bp == 0 )
	    return 0;

	lbn = iboff[idlev++];
	pbn = ((daddr_t *)bp->b_addr)[ lbn ];
	if( pbn == 0 )
	{
	    pbn = bell_dkalloc(ip);
	    if( U.u_errmsg != 0 )
		return 0;
	    ((daddr_t *)bp->b_addr)[ lbn ] = pbn;
	    bp->b_flags |= TOY_DIRTY;
	    toy_bwrite(ip->i_fs, bp);
	    toy_brelse(ip->i_fs, bp);
	}
	else
	{
	    toy_brelse(ip->i_fs, bp);
	}
    }

    U.u_iblocks[0] = pbn;
    return pbn;
}
