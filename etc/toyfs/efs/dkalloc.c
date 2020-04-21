# include "toyfs.h"
# include "efs_toyfs.h"

extern USR U;

extern long efs_nextfree();


efs_dkfree(sp, bn)
    register FS *sp;
    register daddr_t bn;
{
    register struct efs_toyfs *esp;

    sp->fs_flags |= TOY_DIRTY;
    esp = (struct efs_toyfs *)sp->fs_filsys;
    esp->filsys.fs_tfree ++;
    SETBIT(esp->bbitmap, bn);
}

daddr_t
efs_dkalloc(ip)
    register I *ip;
{
    extern B *toy_getblk();

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
	bn = efs_nextfree(esp->bbitmap, esp->nbbitmap, (long)esp->brotor);
	if( bn > 0 )
	{
	    esp->filsys.fs_tfree--;
	    break;
	}
	if( esp->brotor <= esp->filsys.fs_firstcg )
	{
	    U.u_errmsg = "Out of free dk space";
	    break;
	}
    }

    if( (bp = toy_getblk(ip->i_fs, bn)) == 0 )
	return 0;
    bzero(bp->b_addr, BBSize);
    toy_brelse(ip->i_fs, bp);

    esp->brotor = bn;
    return bn;
}

long
efs_nextfree(b, nb, n)
    register char *b;
    long nb;
    register long n;
{

    while( ++n < nb )
	if( GETBIT(b, n) )
	{
	    CLRBIT(b, n);
	    return n;
	}
    return 0;
}
