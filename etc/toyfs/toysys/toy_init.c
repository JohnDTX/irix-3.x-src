# include "toyfs.h"

short toy_debug = 0;

USR U;

int
toy_init()
{
    register B *bp;

    U.u_errmsg = 0;
    U.u_inited++;
    umask(U.u_umask = umask(0));
    time(&U.u_time);
    U.u_curino = ROOTINO;
}
