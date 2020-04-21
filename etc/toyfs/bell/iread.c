# include "toyfs.h"
# include "bell_toyfs.h"

extern USR U;

I *
bell_iread(sp, inum)
    register FS *sp;
    ino_t inum;
{
    extern I *toy_newi();
    extern B *toy_bread();

    register I *ip;
    register B *bp;
    register struct bell_toyinode *dip;
    register struct bell_toyfs *bsp;
    daddr_t ibn;

    bsp = (struct bell_toyfs *)sp->fs_filsys;
    ibn = FsITOD(sp->fs_dev, inum);
    if( !(FsITOD(sp->fs_dev, ROOTINO) <= ibn && ibn < bsp->filsys.s_isize) )
    {
	U.u_errmsg = "Invalid inumber";
	return 0;
    }

    if( (ip = toy_newi(sp, inum, sizeof (struct bell_toyinode))) == 0 )
	return 0;

    bp = toy_bread(sp, ibn);

    if( bp == 0 )
    {
	toy_iput(ip);
	return 0;
    }

    dip = (struct bell_toyinode *)ip->i_dinode;
    dip->inode = ((struct dinode *)bp->b_addr)[ FsITOO(sp->fs_dev, inum) ];
    toy_brelse(sp, bp);
    ip->i_imode = dip->inode.di_mode;
    ip->i_inlink = dip->inode.di_nlink;
    ip->i_iuid = dip->inode.di_uid;
    ip->i_igid = dip->inode.di_gid;
    ip->i_isize = dip->inode.di_size;
    l3tol(dip->daddrs, dip->inode.di_addr, AddrsPerInode);
    ip->i_irdev = dip->daddrs[0];

    ip->i_count = 1;
    ip->i_flags &= ~TOY_INVAL;
    return ip;
}
