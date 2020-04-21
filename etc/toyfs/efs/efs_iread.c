# include "toyfs.h"
# include "efs_toyfs.h"

extern USR U;

I *
efs_iread(sp, inum)
    FS *sp;
    ino_t inum;
{
    extern I *toy_newi();
    extern B *toy_ichunkread();

    register I *ip;
    register B *bp;
    register struct efs_toyinode *dip;
    register struct efs_toyfs *esp;
    daddr_t ibn;
    int iboff;

    esp = (struct efs_toyfs *)sp->fs_filsys;
    ibn = ITOD(&esp->filsys, inum);
    iboff = ITOO(&esp->filsys, inum);
    if( !(esp->filsys.fs_firstcg <= ibn && ibn < esp->filsys.fs_size) )
    {
	U.u_errmsg = "Invalid inumber";
	return 0;
    }

    if( (ip = toy_newi(sp, inum, sizeof (struct efs_toyinode))) == 0 )
	return 0;

    if( (bp = toy_ichunkread(sp, ibn)) == 0 )
    {
	toy_iput(ip);
	return 0;
    }

    iboff += (ibn-bp->b_blkno) << InodesPerBBShift;

    dip = (struct efs_toyinode *)ip->i_dinode;
    dip->inode = ((struct efs_dinode *)bp->b_addr)[ iboff ];

    toy_ichunkrelse(sp, bp);

    ip->i_irdev = dip->inode.di_u.di_dev;

    ip->i_imode = dip->inode.di_mode;
    ip->i_inlink = dip->inode.di_nlink;
    ip->i_iuid = dip->inode.di_uid;
    ip->i_igid = dip->inode.di_gid;
    ip->i_isize = dip->inode.di_size;

    ip->i_iatime = dip->inode.di_atime;
    ip->i_imtime = dip->inode.di_mtime;
    ip->i_ictime = dip->inode.di_ctime;

    ip->i_count = 1;
    ip->i_flags &= ~TOY_INVAL;
    return ip;
}
