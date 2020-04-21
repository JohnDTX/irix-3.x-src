# include "toyfs.h"
# include "bell_toyfs.h"

extern USR U;

off_t
bell_writei(ip, base, count, offset)
    register I *ip;
    char *base;
    int count;
    off_t offset;
{
    extern daddr_t bell_bmap_write();
    extern off_t toy_writei();

    return toy_writei(ip, base, count, offset, bell_bmap_write);
}
