# include "toyfs.h"

extern USR U;

B *
toy_getblk(sp, blkno)
    register FS *sp;
    daddr_t blkno;
{
    extern char *toy_malloc();

    register B *bp, *fbp; B *ebp;

    /*
     * find buf if in core, and move it to head of list.
     * also find slot in case not in core.
     */
    ebp = 0;

    fbp = &sp->fs_bufs;
    for( bp = fbp; (bp = bp->b_forw) != fbp; )
    {
	if( bp->b_count <= 0 )
	    ebp = bp;

	if( bp->b_blkno == blkno )
	{
	    bp->b_back->b_forw = bp->b_forw;
	    bp->b_forw->b_back = bp->b_back;
	    bp->b_forw = fbp->b_forw;
	    bp->b_back = fbp;
	    bp->b_forw->b_back = bp->b_back->b_forw = bp;
	    bp->b_count++;
	    return bp;
	}
    }

    /*
     * not in core.  possibly make another buf.
     * or possibly use the slot found above.
     */
    if( (sp->fs_nbufs < sp->fs_maxbufs || ebp == 0)
     && (bp = (B *)toy_malloc(sizeof *bp + sp->fs_bsize)) != 0 )
    {
	bp->b_addr = (char *)(bp + 1);
	sp->fs_nbufs++;
	bp->b_flags = TOY_INVAL;
	bp->b_blkno = blkno;
	bp->b_forw = bp->b_back = bp;
    }
    else
    if( (bp = ebp) == 0 )
    {
	return 0;
    }

    /*
     * write out the old contents.
     */
    if( bp->b_flags & TOY_DIRTY )
	toy_bwrite(sp, bp);

    bp->b_flags = TOY_INVAL;
    bp->b_blkno = blkno;

    /*
     * move to the head of the buf list.
     */
    bp->b_back->b_forw = bp->b_forw;
    bp->b_forw->b_back = bp->b_back;
    bp->b_forw = fbp->b_forw;
    bp->b_back = fbp;
    bp->b_forw->b_back = bp->b_back->b_forw = bp;
    bp->b_count = 1;
    return bp;
}

toy_brelse(sp, bp)
    register FS *sp;
    register B *bp;
{
    if( --bp->b_count > 0 )
	return;

    /*
     * if too many bufs, free this one.
     */
    if( sp->fs_nbufs > sp->fs_maxbufs )
    {
	if( bp->b_flags & TOY_DIRTY )
	    toy_bwrite(sp, bp);

	bp->b_forw->b_back = bp->b_back;
	bp->b_back->b_forw = bp->b_forw;
	sp->fs_nbufs--;
	/* free(bp->b_addr); */
	free(bp);
	return;
    }
}

B *
toy_bread(sp, blkno)
    register FS *sp;
    daddr_t blkno;
{
    extern off_t lseek();

    register B *bp;

    bp = toy_getblk(sp, blkno);
    if( bp->b_flags & TOY_INVAL )
    {
	if( lseek(sp->fs_fd, (off_t)blkno<<sp->fs_bshift, 0) < 0
	 || read(sp->fs_fd, bp->b_addr, sp->fs_bsize) < 0 )
	{
	    U.u_errmsg = "READ ERROR";
	    return 0;
	}
	bp->b_flags &= ~TOY_INVAL;
    }
    return bp;
}

int
toy_bwrite(sp, bp)
    register FS *sp;
    register B *bp;
{
    extern off_t lseek();

    if( lseek(sp->fs_fd, (off_t)bp->b_blkno<<sp->fs_bshift, 0) < 0
     || write(sp->fs_fd, bp->b_addr, sp->fs_bsize) != sp->fs_bsize )
    {
	U.u_errmsg = "WRITE ERROR";
	return -1;
    }

    bp->b_flags &= ~TOY_DIRTY;
    return 0;
}

toy_binit(sp)
    register FS *sp;
{
    register B *bp;

    bp = &sp->fs_bufs;
    bp->b_forw = bp->b_back = bp;
    sp->fs_nbufs = 0;
    sp->fs_maxbufs = 10;
}

toy_bkill(sp)
    register FS *sp;
{
    register B *bp, *fbp, *nbp;

    fbp = &sp->fs_bufs;
    for( nbp = fbp->b_forw; (bp = nbp) != fbp; )
    {
	nbp = bp->b_forw;
	free((char *)bp);
    }
    sp->fs_nbufs = 0;
}
