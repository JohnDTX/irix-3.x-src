# include "toyfs.h"

extern USR U;

off_t
toy_tell(fp)
    F *fp;
{
    U.u_errmsg = 0;
    return fp->f_offset;
}
    
off_t
toy_seek(fp, offset)
    F *fp;
    long offset;
{
    U.u_errmsg = 0;

    fp->f_offset = offset;
    return offset;
}
