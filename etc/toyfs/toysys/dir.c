# include "toyfs.h"

extern USR U;

TOYIOB *
toy_opendir(ip)
    I *ip;
{
    extern TOYIOB *toy_fopen();

    U.u_errmsg = 0;

    if( (ip->i_imode&IFMT) != IFDIR )
    {
	U.u_errmsg = "Not a directory";
	return 0;
    }

    return toy_fopen(ip);
}

int
toy_closedir(iobp)
    TOYIOB *iobp;
{
    return toy_fclose(iobp);
}

TOYDIR *
toy_readdir(iobp)
    TOYIOB *iobp;
{
    U.u_errmsg = 0;

    return (*iobp->fd->f_ip->i_fs->fs_ops.fs_readdir)(iobp);
}

int
toy_writedir(iobp, dep)
    TOYIOB *iobp;
    TOYDIR *dep;
{
    U.u_errmsg = 0;

    return (*iobp->fd->f_ip->i_fs->fs_ops.fs_writedir)(iobp, dep);
}
