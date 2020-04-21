# include "toyfs.h"
# include "efs_toyfs.h"

extern USR U;

off_t
efs_writei(ip, base, count, offset)
    register I *ip;
    char *base;
    int count;
    off_t offset;
{
    extern daddr_t efs_bmap_write();
    extern off_t toy_writei();

    return toy_writei(ip, base, count, offset, efs_bmap_write);
}
