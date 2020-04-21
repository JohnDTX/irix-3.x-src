# include "toyfs.h"
# include "efs_toyfs.h"

# include "sys/stat.h"

# include "stdio.h"

/*
 2074 -rw-rw-r--  1  165.0        1756 16:03, 10jun85
 */
int
efs_idump(ip, level, F)
    register I *ip;
    int level;
    FILE *F;
{
    int ftype;

    if( ip == 0 )
	return -1;

    ftype = ip->i_imode&S_IFMT;

    bell_cidump(ip, F);
    if( level <= 1 )
    {
	fprintf(F, " %13s", cdate(&ip->i_imtime));
	return;
    }

    bell_didump(&ip->i_iatime, F);

    if( ftype == S_IFREG || ftype == S_IFDIR || ftype == S_IFLNK )
	efs_dumpu(ip, level, F);

    return 0;
}

efs_dumpu(ip, level, F)
    register I *ip;
    int level;
    FILE *F;
{
    register struct extent *xp;
    register struct efs_toyinode *dip;
    register int nx;

    dip = (struct efs_toyinode *)ip->i_dinode;
    xp = dip->inode.di_xp + 0;
    nx = dip->inode.di_nx;

    fprintf(F, " %dx:", nx);

    if( nx <= ExtsPerInode )
    {
	efs_dumpexts(xp, nx, F);
    }
    else
    {
	extern B *toy_nbread();
	register B *bp;
	int tnx;

	for( tnx = nx; nx > 0; xp++ , tnx -= nx )
	{
	fprintf(F, " *[%ld+%d: %ld]",
		(long)xp->ex_offset, xp->ex_length, xp->ex_bn);
	nx = xp->ex_length<<ExtsPerBBShift;
	if( nx > tnx )
	    nx = tnx;
	if( level <= 2 )
	    continue;

	if( (bp = toy_nbread(ip->i_fs,
		(daddr_t)xp->ex_bn, (int)xp->ex_length<<BBShift)) == 0 )
	{
	    fprintf(F, " [read error]");
	    return;
	}
	efs_dumpexts((struct extent *)bp->b_addr, nx, F);
	toy_brelse(ip->i_fs, bp);
	}
    }
}

efs_dumpexts(xp, nx, F)
    register struct extent *xp;
    register int nx;
    FILE *F;
{
    while( --nx >= 0 )
    {
	fprintf(F, " [%ld+%d: %ld]",
		(long)xp->ex_offset, xp->ex_length, xp->ex_bn);
	xp++;
    }
}
