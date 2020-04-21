# include "toyfs.h"

extern USR U;

F *
toy_creat(myfs, tgt, mode)
    FS *myfs;
    char *tgt;
    int mode;
{
    extern F *toy_open();
    extern F *toy_mknod();
    register F *fp;

    mode = IFREG | (mode & IRWXRWXRWX);
    if( (fp = toy_mknod(myfs, tgt, mode, 0)) != 0 )
	return fp;
    if( (fp = toy_open(myfs, tgt)) == 0 )
	return 0;
    toy_itrunc(fp->f_ip);
    return fp;
}

int
toy_itrunc(ip)
    I *ip;
{
    return (*ip->i_fs->fs_ops.fs_itrunc)(ip);
}
