# include "toyfs.h"

extern USR U;

static char ronly[] = "File system mounted read-only";

int
toy_nullsync()
{
    U.u_errmsg = ronly;
    return -1;
}

I *
toy_nullimake()
{
    U.u_errmsg = ronly;
    return 0;
}

int
toy_nulliwrite()
{
    U.u_errmsg = ronly;
    return 0;
}

int
toy_nullitrunc()
{
    U.u_errmsg = ronly;
    return -1;
}

off_t
toy_nullwritei()
{
    U.u_errmsg = ronly;
    return -1;
}

int
toy_nullwritedir()
{
    U.u_errmsg = ronly;
    return -1;
}
