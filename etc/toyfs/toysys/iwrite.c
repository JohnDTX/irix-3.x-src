# include "toyfs.h"

extern USR U;

int
toy_iwrite(ip)
    register I *ip;
{
    return (*ip->i_fs->fs_ops.fs_iwrite)(ip);
}
