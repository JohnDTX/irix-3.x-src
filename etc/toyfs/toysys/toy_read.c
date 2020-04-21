# include "toyfs.h"
# include "sys/stat.h"

extern USR U;

int
toy_read(fp, buf, len)
    register F *fp;
    char *buf;
    int len;
{
    register off_t newoff;

    U.u_errmsg = 0;

    if( fp == 0 )
    {
	U.u_errmsg = "Invalid fd";
	return -1;
    }

    newoff = (*fp->f_ip->i_fs->fs_ops.fs_readi)(fp->f_ip,
				buf, len, fp->f_offset);
    if( U.u_errmsg != 0 )
	return -1;
    newoff -= fp->f_offset;
    fp->f_offset += newoff;
    return newoff;
}

off_t
toy_readi(ip, base, count, offset, bmap)
    register I *ip;
    char *base;
    int count;
    off_t offset;
    daddr_t (*bmap)();
{
    extern B *toy_bread();

    int ftype;
    register FS *sp;
    register B *bp;
    register daddr_t bn;
    register int lbn;
    register int brem; int boff;
    int frem;

    ftype = ip->i_imode & S_IFMT;
    if( ftype == S_IFCHR || ftype == S_IFBLK )
    {
	U.u_errmsg = "toy i/o from device file";
	return offset;
    }
    sp = ip->i_fs;

    frem = ip->i_isize - offset;
    if( count > frem )
	count = frem;

    while( count > 0 )
    {
	lbn = offset >> sp->fs_bshift;
	boff = offset & sp->fs_bmask;
	brem = sp->fs_bsize - boff;
	if( brem > count )
	    brem = count;

	bn = (*bmap)(ip, lbn);

	if( bn < 0 )
	    break;
	if( bn == 0 )
	{
	    bzero(base, brem);
	}
	else
	{
	    if( (bp = toy_bread(ip->i_fs, bn)) == 0 )
		break;
	    bcopy(bp->b_addr+boff, base, brem);
	    toy_brelse(ip->i_fs, bp);
	}

	base += brem;
	offset += brem;
	count -= brem;
    }

    return offset;
}
