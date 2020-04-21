# include "toyfs.h"

extern USR U;


int
toy_sync(sp)
    register FS *sp;
{
    register int (*syncer)();

    U.u_errmsg = 0;
    toy_isync(sp);
    toy_bsync(sp);
    if( (syncer = sp->fs_ops.fs_sync) == 0 )
	return -1;
    return (*sp->fs_ops.fs_sync)(sp);
}

toy_isync(sp)
    register FS *sp;
{
    register I *ip, *fip;

    fip = &sp->fs_inodes;
    for( ip = fip; (ip = ip->i_forw) != fip; )
	if( ip->i_flags & TOY_DIRTY )
	    toy_iwrite(ip);
}

toy_bsync(sp)
    register FS *sp;
{
    register B *bp, *fbp;

    fbp = &sp->fs_bufs;
    for( bp = fbp; (bp = bp->b_forw) != fbp; )
	if( bp->b_flags & TOY_DIRTY )
	    toy_bwrite(sp, bp);
}
