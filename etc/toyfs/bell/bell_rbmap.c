# include "toyfs.h"
# include "bell_toyfs.h"

extern USR U;


daddr_t
bell_bmap_read(ip,lbn)
    I *ip;
    register daddr_t lbn;
{
    extern B *toy_bread();

    int iboff[IndirectAddrsPerInode];

    register struct bell_toyinode *dip;
    register int idlev;
    register B *bp;

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
	U.u_iblocks[0] = lbn = dip->daddrs[lbn];
	return lbn;
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
    lbn = dip->daddrs[NADDR-idlev-1];
    while( idlev < IndirectAddrsPerInode )
    {
	U.u_niblocks++;
	U.u_iblocks[IndirectAddrsPerInode-idlev] = lbn;
	bp = toy_bread(ip->i_fs,lbn);
	if( bp == 0 )
	    return 0;
	lbn = ((daddr_t *)bp->b_addr)[ iboff[idlev++] ];
	toy_brelse(ip->i_fs,bp);
    }

    U.u_iblocks[0] = lbn;
    return lbn;
}
