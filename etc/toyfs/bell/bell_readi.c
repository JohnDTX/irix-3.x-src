# include "toyfs.h"
# include "bell_toyfs.h"

extern USR U;

off_t
bell_readi(ip, base, count, offset)
    register I *ip;
    char *base;
    int count;
    off_t offset;
{
    extern daddr_t bell_bmap_read();
    extern off_t toy_readi();

    return toy_readi(ip, base, count, offset, bell_bmap_read);
}
