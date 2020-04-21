# include "toyfs.h"
# include "sys/stat.h"
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
    int ftype;

    esp = (struct bell_toyfs *)ip->i_fs->fs_filsys;
    dip = (struct bell_toyinode *)ip->i_dinode;
    if( ip->i_inlink <= 0 )
    {
	bell_ifree(ip->i_fs, ip->i_number);
	toy_itrunc(ip);
	ip->i_isize = 0;
	ip->i_imode = 0;
    }

    ftype = ip->i_imode & S_IFMT;
    if( ftype == S_IFCHR || ftype == S_IFBLK )
	dip->daddrs[0] = ip->i_irdev;
    ltol3(dip->inode.di_addr,dip->daddrs,AddrsPerInode);

    dip->inode.di_mode = ip->i_imode;
    dip->inode.di_nlink = ip->i_inlink;
    dip->inode.di_uid = ip->i_iuid;
    dip->inode.di_gid = ip->i_igid;
    dip->inode.di_size = ip->i_isize;

    dip->inode.di_atime = ip->i_iatime;
    dip->inode.di_mtime = ip->i_imtime;
    dip->inode.di_ctime = ip->i_ictime;

    if( (bp = toy_bread(esp,FsITOD(&esp->filsys,ip->i_number))) == 0 )
	return -1;
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
bell_itrunc(ip)
    register I *ip;
{
    U.u_errmsg = "bell itrunc not implemented";
}
