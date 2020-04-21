# include "toyfs.h"
# include "bell_toyfs.h"
# include "sys/dir.h"


extern USR U;

TOYDIR *
bell_readdir(iobp)
    register TOYIOB *iobp;
{
    static TOYDIR d1;
    register struct direct *dep;

    if( iobp->nleft <= 0 )
    {
	if( (iobp->nleft = toy_read(iobp->fd, iobp->base, FSBSize)) <= 0 )
	    return 0;
	iobp->ptr = iobp->base;
    }

    d1.d_offset = iobp->fd->f_offset-iobp->nleft;

    dep = (struct direct *)iobp->ptr;
    iobp->nleft -= sizeof (struct direct);
    iobp->ptr += sizeof (struct direct);

    d1.d_len = DIRSIZ;
    d1.d_name = dep->d_name;
    d1.d_ino = dep->d_ino;
    return &d1;
}
