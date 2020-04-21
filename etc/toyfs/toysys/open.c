# include "toyfs.h"

extern USR U;

I *
toy_open(myfs, name)
    FS *myfs;
    char *name;
{
    extern I *toy_namei();
    register I *ip;

    ip = toy_namei(myfs, name, 1);
    return ip;
}
