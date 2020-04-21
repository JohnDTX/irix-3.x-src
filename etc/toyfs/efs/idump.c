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
    register struct efs_dinode *dip;
    int ftype;

    if( ip == 0 )
	return -1;

    dip = &((struct efs_toyinode *)ip->i_dinode)->inode;
    ftype = ip->i_imode&S_IFMT;

    bell_cidump(ip, F);
    if( level <= 1 )
    {
	fprintf(F, " %13s", cdate(&dip->di_mtime));
	return;
    }

    bell_didump(&dip->di_atime, F);

    if( ftype == S_IFREG || ftype == S_IFDIR || ftype == S_IFLNK )
	efs_dumpexts(dip->di_x, dip->di_nx, F);

    return 0;
}

efs_dumpexts(xp, n, F)
    register struct extent *xp;
    register int n;
    FILE *F;
{
    while( --n >= 0 )
    {
	fprintf(F," [%ld+%d: %ld]",
		(long)xp->ex_offset, xp->ex_length, xp->ex_bn);
	xp++;
    }
}
