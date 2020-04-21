# include "toyfs.h"
# include "efs_toyfs.h"

extern USR U;

int
efs_writedir(iobp, dep)
    TOYIOB *iobp;
    TOYDIR *dep;
{
    return bell_writedir(iobp, dep);
}
