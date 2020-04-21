# include "toyfs.h"

extern USR U;

int
toy_closefs(sp)
    register FS *sp;
{
    U.u_errmsg = 0;
    if( sp == 0 )
	return -1;
    return (*sp->fs_ops.fs_umount)(sp);
}
