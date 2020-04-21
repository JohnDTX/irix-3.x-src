# include "toyfs.h"
# include "efs_toyfs.h"

extern USR U;

extern long efs_nextfree();

# define DEBUG efs_debug
short efs_debug = 0;


efs_dkfree(sp, bn, l)
    register FS *sp;
    register daddr_t bn;
    int l;
{
    register struct efs_toyfs *esp;

    sp->fs_flags |= TOY_DIRTY;
    esp = (struct efs_toyfs *)sp->fs_filsys;
    esp->filsys.fs_tfree += l;
    while( --l >= 0 )
    {
	SETBIT(esp->bbitmap, bn);
	bn++;
    }
}

int
efs_adj_dkalloc(sp, bn)
    register FS *sp;
    register daddr_t bn;
{
    extern B *toy_getblk();

    register struct efs_toyfs *esp;
    register char *b;
    register B *bp;

    esp = (struct efs_toyfs *)sp->fs_filsys;
    b = esp->bbitmap;
    if( GETBIT(b, bn) )
    {
	CLRBIT(b, bn);
	if( (bp = toy_getblk(sp, bn)) == 0 )
	    return -1;
	bzero(bp->b_addr, bp->b_bcount);
	bp->b_flags |= TOY_DIRTY;
	toy_brelse(sp, bp);
	esp->filsys.fs_tfree --;
	sp->fs_flags |= TOY_DIRTY;
	return 0;
    }
    return -1;
}

daddr_t
efs_dkalloc(ip, l)
    register I *ip;
    int l;
{
    extern B *toy_ngetblk();

    register B *bp;
    register struct efs_toyfs *esp;
    register int i;
    register daddr_t bn;

    ip->i_fs->fs_flags |= TOY_DIRTY;
    esp = (struct efs_toyfs *)ip->i_fs->fs_filsys;

    /* for now, just return the next free */
    if( esp->filsys.fs_tfree <= 0 )
    {
	U.u_errmsg = "Out of free dk space";
	return 0;
    }

    for( i = 2; --i >= 0; )
    {
	if( !(esp->filsys.fs_firstcg <= esp->brotor
	 && esp->brotor < esp->nbbitmap) )
	    esp->brotor = esp->filsys.fs_firstcg;
	bn = efs_nextfree(esp->bbitmap, esp->nbbitmap, (long)esp->brotor, l);
	if( bn > 0 )
	{
	    esp->filsys.fs_tfree -= l;
	    break;
	}
	if( esp->brotor <= esp->filsys.fs_firstcg )
	{
	    U.u_errmsg = "Out of free dk space";
	    break;
	}
    }

    if( (bp = toy_ngetblk(ip->i_fs, bn, l<<BBShift)) == 0 )
	return 0;
    bzero(bp->b_addr, bp->b_bcount);
    bp->b_flags |= TOY_DIRTY;
    toy_brelse(ip->i_fs, bp);

    esp->brotor = bn;
    return bn;
}

/*
 * efs_nextfree() --
 * look for a free run of length l in bitmap b,
 * starting at fb (ending at nb).
 */
long
efs_nextfree(b, nb, fb, l)
    register char *b;
    long nb;
    register long fb;
    int l;
{
    register long xb;

    xb = 0;
    l--;

    /*
     * xb is the first block current run.
     */
    while( ++fb < nb )
    {
	if( GETBIT(b, fb) )
	{
	    if( xb == 0 )
		xb = fb;
	    CLRBIT(b, fb);
	    if( fb - xb >= l )
		return xb;
	}
	else
	{
	    if( xb != 0 )
	    {
		while( xb < fb )
		{
		    SETBIT(b, xb);
		    xb++;
		}
	    }
	    xb = 0;
	}
    }

    if( xb != 0 )
    {
	while( xb < fb )
	{
	    SETBIT(b, xb);
	    xb++;
	}
    }

    return 0;
}
