# include "toyfs.h"
# include "stdio.h"

extern USR U;

int
toy_idump(ip,level,F)
    I *ip;
    int level;
    FILE *F;
{
    U.u_errmsg = 0;
    return (*ip->i_fs->fs_ops.fs_idump)(ip,level,F);
}
