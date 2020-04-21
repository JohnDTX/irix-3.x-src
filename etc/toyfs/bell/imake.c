# include "toyfs.h"
# include "bell_toyfs.h"

extern USR U;

I *
bell_imake(sp, mode)
    register FS *sp;
    int mode;
{
    U.u_errmsg = "imake not implemented";
    return 0;
}
