# include "toyfs.h"
# include "bell_toyfs.h"

bell_sync(sp)
    register FS *sp;
{
    register struct bell_toyfs *esp;
    register B *bp;

    esp = (struct bell_toyfs *)sp->fs_filsys;
    for( bp = &sp->fs_bufs; (bp = bp->b_forw) != &sp->fs_bufs; )
    {
	if( bp->b_flags & TOY_DIRTY )
	    toy_bwrite(sp,bp);
    }

    bell_supersync(sp);
}

bell_supersync(sp)
    register FS *sp;
{
    extern B *toy_getblk();
    register B *bp;

    if( (sp->fs_flags & (TOY_DIRTY|TOY_INVAL)) != TOY_DIRTY )
	return;

    sp->fs_flags &= ~TOY_DIRTY;
    if( (bp = toy_getblk(sp,SUPERB)) == 0 )
	return;

    bzero(bp->b_addr,FSBSize);
    *(struct filsys *)bp->b_addr
	    = ((struct bell_toyfs *)sp->fs_filsys)->filsys;
    bp->b_flags |= TOY_DIRTY;
    toy_bwrite(sp,bp);
}
