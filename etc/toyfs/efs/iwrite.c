# include "toyfs.h"
# include "efs_toyfs.h"

extern USR U;


int
efs_iwrite(ip)
    I *ip;
{
    extern B *toy_bread();

    register B *bp;
    register FS *sp;
    register struct efs_toyfs *esp;
    struct efs_toyinode *dip;

    sp = ip->i_fs;
    esp = (struct efs_toyfs *)sp->fs_filsys;
    dip = (struct efs_toyinode *)ip->i_dinode;

    if( ip->i_inlink <= 0 )
    {
	efs_trunc(ip);
	ip->i_imode = 0;
	efs_ifree(ip->i_fs, ip->i_number);
    }

    dip->inode.di_mode = ip->i_imode;
    dip->inode.di_nlink = ip->i_inlink;
    dip->inode.di_uid = ip->i_iuid;
    dip->inode.di_gid = ip->i_igid;
    dip->inode.di_size = ip->i_isize;

    if( (bp = toy_bread(sp, ITOD(&esp->filsys, ip->i_number))) == 0 )
	return -1;
    ((struct efs_dinode *)bp->b_addr)[ ITOO(&esp->filsys, ip->i_number) ]
							    = dip->inode;
    if( toy_bwrite(sp, bp) < 0 )
    {
	toy_brelse(sp, bp);
	return -1;
    }
    toy_brelse(sp, bp);

    ip->i_flags &= ~TOY_DIRTY;
    return 0;
}

int
efs_trunc(ip)
    register I *ip;
{
    extern daddr_t efs_bmap_read();

    register struct efs_toyinode *dip;
    register daddr_t bn;
    int ftype;
    int nblks;

    ip->i_flags |= TOY_DIRTY;

    dip = (struct efs_toyinode *)ip->i_dinode;
    ftype = ip->i_imode&IFMT;
    if( !(ftype == IFREG || ftype == IFDIR
     || ftype == IFLNK || ftype == IFIFO) )
	return 0;

    nblks = ip->i_isize+BBSize-1>>BBShift;
    while( --nblks >= 0 )
	if( (bn = efs_bmap_read(ip, nblks)) > 0 )
	    efs_dkfree(ip->i_fs, bn);

    ip->i_isize = 0;
    dip->inode.di_nx = 0;
    bzero((char *)&dip->inode.di_u, sizeof dip->inode.di_u);
    return 0;
}
