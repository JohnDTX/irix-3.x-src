# include "toyfs.h"

extern USR U;

I *
toy_imake(sp, mode, addr)
    FS *sp;
    register int mode;
    int addr;
{
    U.u_errmsg = 0;

    return (*sp->fs_ops.fs_imake)(sp, mode & ~U.u_umask);
}
