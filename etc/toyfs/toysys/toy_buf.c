# include "toyfs.h"

extern USR U;

extern B *toy_getblk(), *toy_ngetblk();
extern B *toy_bread(), *toy_nbread();

B *
toy_getblk(sp, blkno)
    FS *sp;
    daddr_t blkno;
{
    return toy_ngetblk(sp, blkno, sp->fs_bsize);
}

B *
toy_ngetblk(sp, blkno, bcount)
    register FS *sp;
    daddr_t blkno;
    int bcount;
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
	if( bp->b_count <= 0 && bp->b_bcount == bcount )
	    ebp = bp;

	if( bp->b_blkno == blkno && bp->b_bcount == bcount )
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
     && (bp = (B *)toy_malloc(sizeof *bp + bcount)) != 0 )
    {
	bp->b_addr = (char *)(bp + 1);
	sp->fs_nbufs++;
	bp->b_flags = TOY_INVAL;
	bp->b_blkno = blkno;
	bp->b_forw = bp->b_back = bp;
	bp->b_bcount = bcount;
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
    if( bp->b_flags & TOY_DIRTY )
	toy_bwrite(sp, bp);

    if( --bp->b_count > 0 )
	return;

    /*
     * if too many bufs, free this one.
     * otherwise it stays in the cache.
     */
    if( sp->fs_nbufs > sp->fs_maxbufs )
    {
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
    FS *sp;
    daddr_t blkno;
{
    return toy_nbread(sp, blkno, sp->fs_bsize);
}

B *
toy_nbread(sp, blkno, bcount)
    register FS *sp;
    daddr_t blkno;
    int bcount;
{
    extern off_t lseek();

    register B *bp;

    bp = toy_ngetblk(sp, blkno, bcount);
    if( bp->b_flags & TOY_INVAL )
    {
	if( lseek(sp->fs_fd, (off_t)blkno<<sp->fs_bshift, 0) < 0
	 || read(sp->fs_fd, bp->b_addr, bp->b_bcount) != bp->b_bcount )
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
     || write(sp->fs_fd, bp->b_addr, bp->b_bcount) != bp->b_bcount )
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
    sp->fs_ichunk.b_flags = TOY_INVAL;
    bp = &sp->fs_ichunk;
    bp->b_addr = 0;
    bp->b_flags = TOY_INVAL;
    toy_ichunkinit(sp, 1);
}

toy_ichunkinit(sp, ichunksize)
    register FS *sp;
    int ichunksize;
{
    register B *bp;

    sp->fs_ichunksize = ichunksize;
    if( sp->fs_flags & TOY_RONLY )
    {
	bp = &sp->fs_ichunk;
	if( bp->b_addr != 0 )
	    free(bp->b_addr);
	bp->b_addr = 0;
	bp->b_flags = TOY_INVAL;
	bp->b_bcount = ichunksize<<sp->fs_bshift;
	if( bp->b_bcount != 0 )
	    bp->b_addr = toy_malloc(bp->b_bcount);
    }
}

B *
toy_ichunkread(sp, ibn)
    register FS *sp;
    daddr_t ibn;
{
    extern off_t lseek();

    extern B *toy_bread();

    register B *bp;

    if( !(sp->fs_flags&TOY_RONLY && (bp = &sp->fs_ichunk)->b_addr != 0) )
    {
	return toy_bread(sp, ibn);
    }

    if( bp->b_flags & TOY_INVAL
     || !(bp->b_blkno <= ibn
     && ibn < bp->b_blkno + sp->fs_ichunksize) )
    {
	bp->b_flags |= TOY_INVAL;
	bp->b_resid = 0;
	while( lseek(sp->fs_fd, (off_t)ibn<<sp->fs_bshift, 0) < 0
	 || read(sp->fs_fd, bp->b_addr, bp->b_bcount-bp->b_resid) < 0 )
	{
	    bp->b_resid += sp->fs_bsize;
	    if( bp->b_bcount-bp->b_resid <= 0 )
	    {
		U.u_errmsg = "ICHUNK READ ERROR";
		return 0;
	    }
	}
	bp->b_blkno = ibn;
	bp->b_flags &= ~TOY_INVAL;
    }

    return bp;
}

toy_ichunkrelse(sp, bp)
    register FS *sp;
    register B *bp;
{
    if( !(bp == &sp->fs_ichunk) )
    {
	toy_brelse(sp, bp);
	return;
    }
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
    bp = &sp->fs_ichunk;
    if( bp->b_addr != 0 )
	free(bp->b_addr);
    bp->b_addr = 0;
    bp->b_flags = TOY_INVAL;
}
