# include "toyfs.h"
# include "efs_toyfs.h"

extern USR U;

off_t
efs_readi(ip, base, count, offset)
    register I *ip;
    char *base;
    int count;
    off_t offset;
{
    extern daddr_t efs_bmap_read();
    extern off_t toy_readi();

    return toy_readi(ip, base, count, offset, efs_bmap_read);
}
