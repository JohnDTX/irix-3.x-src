# include "toyfs.h"
# include "sys/stat.h"
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
    int ftype;

    sp = ip->i_fs;
    esp = (struct efs_toyfs *)sp->fs_filsys;
    dip = (struct efs_toyinode *)ip->i_dinode;

    if( ip->i_inlink <= 0 )
    {
	efs_ifree(ip->i_fs, ip->i_number);
	toy_itrunc(ip);
	ip->i_isize = 0;
	ip->i_imode = 0;
    }

    ftype = ip->i_imode & S_IFMT;
    if( ftype == S_IFCHR || ftype == S_IFBLK )
	dip->inode.di_u.di_dev = ip->i_irdev;

    dip->inode.di_mode = ip->i_imode;
    dip->inode.di_nlink = ip->i_inlink;
    dip->inode.di_uid = ip->i_iuid;
    dip->inode.di_gid = ip->i_igid;
    dip->inode.di_size = ip->i_isize;

    dip->inode.di_atime = ip->i_iatime;
    dip->inode.di_mtime = ip->i_imtime;
    dip->inode.di_ctime = ip->i_ictime;

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
efs_itrunc(ip)
    register I *ip;
{
    register struct efs_toyinode *dip;
    register struct extent *xp;
    register int nx;
    int ftype;

    ip->i_flags |= TOY_DIRTY;

    dip = (struct efs_toyinode *)ip->i_dinode;
    ftype = ip->i_imode&IFMT;
    if( !(ftype == IFREG || ftype == IFDIR
     || ftype == IFLNK || ftype == IFIFO) )
	return 0;

    xp = dip->inode.di_xp+0;
    nx = dip->inode.di_nx;

    if( nx > ExtsPerInode )
    {
	extern B *toy_nbread();

	register B *bp;

	if( (bp = toy_nbread(ip->i_fs,
		(daddr_t)xp->ex_bn, (int)xp->ex_length<<BBShift)) == 0 )
	    return -1;
	xp = (struct extent *)bp->b_addr;
	while( --nx >= 0 )
	{
	    efs_dkfree(ip->i_fs, (daddr_t)xp->ex_bn, (int)xp->ex_length);
	    xp++;
	}
	toy_brelse(ip->i_fs, bp);
	xp = dip->inode.di_xp+0;
	nx = 1;
    }
    while( --nx >= 0 )
    {
	efs_dkfree(ip->i_fs, (daddr_t)xp->ex_bn, (int)xp->ex_length);
	xp++;
    }
    ip->i_isize = 0;
    dip->inode.di_nx = 0;
    bzero((char *)&dip->inode.di_u, sizeof dip->inode.di_u);
    return 0;
}
