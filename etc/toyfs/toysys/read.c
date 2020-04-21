# include "toyfs.h"

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
