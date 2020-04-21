# include "toyfs.h"
# include "efs_toyfs.h"

TOYDIR *
efs_readdir(iobp)
    TOYIOB *iobp;
{
    extern TOYDIR *bell_readdir();

    return bell_readdir(iobp);
}
