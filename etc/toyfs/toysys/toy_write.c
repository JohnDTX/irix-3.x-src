# include "toyfs.h"
# include "sys/stat.h"

extern USR U;

int
toy_write(fp, buf, len)
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
    newoff = (*fp->f_ip->i_fs->fs_ops.fs_writei)(fp->f_ip,
				buf, len, fp->f_offset);
    if( U.u_errmsg != 0 )
	return -1;
    newoff -= fp->f_offset;
    fp->f_offset += newoff;
    return newoff;
}

off_t
toy_writei(ip, base, count, offset, bmap)
    register I *ip;
    char *base;
    int count;
    off_t offset;
    daddr_t (*bmap)();
{
    extern B *toy_getblk(), *toy_bread();

    int ftype;
    register FS *sp;
    register B *bp;
    register daddr_t bn;
    register int lbn;
    register int brem; int boff;

    ftype = ip->i_imode & S_IFMT;
    if( ftype == S_IFCHR || ftype == S_IFBLK )
    {
	U.u_errmsg = "toy i/o to device file";
	return offset;
    }
    sp = ip->i_fs;

    while( count > 0 )
    {
	lbn = offset >> sp->fs_bshift;
	boff = offset & sp->fs_bmask;
	brem = sp->fs_bsize - boff;
	if( brem > count )
	    brem = count;

	bn = (*bmap)(ip, lbn);

	if( U.u_errmsg != 0 )
	    break;

	if( boff == 0 && brem == sp->fs_bsize )
	    bp = toy_getblk(ip->i_fs, bn);
	else
	    bp = toy_bread(ip->i_fs, bn);
	if( bp == 0 )
	    break;

	bcopy(base, bp->b_addr + boff, brem);
	bp->b_flags |= TOY_DIRTY;
	bp->b_flags &= ~TOY_INVAL;
	toy_brelse(ip->i_fs, bp);

	base += brem;
	offset += brem;
	if( offset >= ip->i_isize )
	{
	    ip->i_flags |= TOY_DIRTY;
	    ip->i_isize = offset;
	}
	count -= brem;
    }

    return offset;
}
