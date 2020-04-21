# include "toyfs.h"
# include "bell_toyfs.h"

extern USR U;


int
bell_iwrite(ip)
    I *ip;
{
    extern B *toy_bread();

    register B *bp;
    register struct bell_toyinode *dip;
    register struct bell_toyfs *esp;

    esp = (struct bell_toyfs *)ip->i_fs->fs_filsys;
    dip = (struct bell_toyinode *)ip->i_dinode;
    dip->inode.di_mode = ip->i_imode;
    dip->inode.di_nlink = ip->i_inlink;
    dip->inode.di_uid = ip->i_iuid;
    dip->inode.di_gid = ip->i_igid;
    dip->inode.di_size = ip->i_isize;

    if( (bp = toy_bread(esp,FsITOD(&esp->filsys,ip->i_number))) == 0 )
	return -1;
    ltol3(dip->inode.di_addr,dip->daddrs,AddrsPerInode);
    ((struct dinode *)bp->b_addr)[ FsITOO(&esp->filsys,ip->i_number) ]
	    = dip->inode;
    if( toy_bwrite(ip->i_fs,bp) < 0 )
    {
	toy_brelse(&esp->filsys,bp);
	return -1;
    }

    toy_brelse(&esp->filsys,bp);
    return 0;
}
