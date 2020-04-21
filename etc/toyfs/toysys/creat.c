# include "toyfs.h"

extern USR U;

F *
toy_creat(myfs, tgt, mode)
    FS *myfs;
    char *tgt;
    int mode;
{
    extern F *toy_mknod();

    return toy_mknod(myfs, tgt, IFREG|(mode & IRWXRWXRWX), 0);
}
