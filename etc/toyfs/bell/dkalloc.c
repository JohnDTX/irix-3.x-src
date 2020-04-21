# include "toyfs.h"
# include "bell_toyfs.h"

# define repeat		do
# define until(x)	while(!(x))

extern USR U;


daddr_t
bell_dkalloc(ip)
    I *ip;
{
    extern B *toy_getblk();

    extern B *toy_bread();

    register struct bell_toyfs *esp;
    register daddr_t bno;
    register B *bp;

    esp = (struct bell_toyfs *)ip->i_fs->fs_filsys;

    repeat
    {
	if( esp->filsys.s_nfree <= 0
	 || (bno = esp->filsys.s_free[--esp->filsys.s_nfree]) == 0 )
	{
	    U.u_errmsg = "No space on file system";
	    return 0;
	}
    }
    until( !bell_badblock(&esp->filsys, bno) );

    if( esp->filsys.s_nfree <= 0 )
    {
	if( (bp = toy_bread(ip->i_fs, bno)) == 0 )
	    return 0;
	esp->filsys.s_nfree = ((struct fblk *)bp->b_addr)->df_nfree;
	bcopy((char *)((struct fblk *)bp->b_addr)->df_free
		, (char *)esp->filsys.s_free, sizeof esp->filsys.s_free);
	toy_brelse(ip->i_fs, bp);

	esp->filsys.s_fmod = 1;
	ip->i_fs->fs_flags |= TOY_DIRTY;
	bell_supersync(ip->i_fs);
    }

    if( !(0 < esp->filsys.s_nfree && esp->filsys.s_nfree < NICFREE) )
    {
	U.u_errmsg = "Bad free count";
	return 0;
    }

    if( (bp = toy_getblk(ip->i_fs, bno)) == 0 )
	return 0;
    bzero(bp->b_addr, FSBSize);
    toy_brelse(ip->i_fs, bp);

    if( esp->filsys.s_tfree > 0 )
	esp->filsys.s_tfree--;

    ip->i_fs->fs_flags |= TOY_DIRTY;
    esp->filsys.s_fmod = 1;
    return bno;
}

/*
 * place the specified disk block back on the free list of the
 * specified device.
 */
bell_dkfree(sp, bno)
    register FS *sp;
    daddr_t bno;
{
    extern B *toy_getblk();

    register struct bell_toyfs *esp;
    register B *bp;

    esp->filsys.s_fmod = 1;
    if( bell_badblock(&esp->filsys, bno) )
	return;
    if( esp->filsys.s_nfree <= 0 )
    {
	esp->filsys.s_nfree = 1;
	esp->filsys.s_free[0] = 0;
    }
    if( esp->filsys.s_nfree >= NICFREE )
    {
	if( (bp = toy_getblk(sp, bno)) == 0 )
	    return;
	((struct fblk *)bp->b_addr)->df_nfree = esp->filsys.s_nfree;
	bcopy((char *)esp->filsys.s_free
		, (char *)((struct fblk *)bp->b_addr)->df_free
		, sizeof esp->filsys.s_free);
	esp->filsys.s_nfree = 0;
	bp->b_flags |= TOY_DIRTY;
	toy_bwrite(sp, bp);
	toy_brelse(sp, bp);
    }
    sp->fs_flags |= TOY_DIRTY;

    esp->filsys.s_free[esp->filsys.s_nfree++] = bno;
    esp->filsys.s_tfree++;
    esp->filsys.s_fmod = 1;
}

/*
 * Check that a block number is in the range between the I list
 * and the size of the device.
 * This is used mainly to check that a
 * garbage file system has not been mounted.
 *
 * bad block on dev x/y -- not in range
 */
int
bell_badblock(sp, bn)
    register struct filsys *sp;
    daddr_t bn;
{
    if( bn < sp->s_isize || bn >= sp->s_fsize)
    {
	U.u_errmsg = "Bad block on filsystem";
	return 1;
    }
    return 0;
}
