# include "toyfs.h"

extern USR U;

I *
toy_iget(sp, inum)
    FS *sp;
    ino_t inum;
{
    extern I *toy_rawiget();

    register I *ip;

    if( (ip = toy_rawiget(sp, inum)) != 0 && ip->i_imode == 0 )
    {
	U.u_errmsg = "Unallocated inode";
	toy_iput(ip);
	return 0;
    }
    return ip;
}

int
toy_iput(ip)
    register I *ip;
{
    register FS *sp;

    if( ip == 0 )
	return -1;

    if( --ip->i_count > 0 )
	return 0;

    if( ip->i_flags & TOY_DIRTY )
	toy_iwrite(ip);

    ip->i_count = 0;

    sp = ip->i_fs;

    if( sp->fs_ninodes < sp->fs_maxinodes )
    {
	/*ip->i_number = 0 */;
	return 0;
    }

    sp->fs_ninodes--;
    ip->i_forw->i_back = ip->i_back;
    ip->i_back->i_forw = ip->i_forw;

    free((char *)ip);
    return 0;
}

I *
toy_rawiget(sp, inum)
    register FS *sp;
    ino_t inum;
{
    register I *ip, *fip;

    U.u_errmsg = 0;

    fip = &sp->fs_inodes;
    for( ip = fip; (ip = ip->i_forw) != fip; )
	if( ip->i_number == inum )
	{
	    ip->i_forw->i_back = ip->i_back;
	    ip->i_back->i_forw = ip->i_forw;

	    ip->i_forw = &sp->fs_inodes;
	    ip->i_back = sp->fs_inodes.i_back;
	    ip->i_forw->i_back = ip->i_back->i_forw = ip;
	    ip->i_count++;
	    return ip;
	}

    return (*sp->fs_ops.fs_iread)(sp, inum);
}

I *
toy_newi(sp, inum, inosize)
    FS *sp;
    ino_t inum;
    int inosize;
{
    extern char *toy_malloc();

    register I *ip;

    if( (ip = (I *)toy_malloc(sizeof *ip + inosize)) == 0 )
	return 0;
    ip->i_dinode = (char *)(ip+1);
    ip->i_flags = TOY_INVAL;
    ip->i_fs = sp;
    ip->i_count = 0;
    ip->i_forw = sp->fs_inodes.i_forw;
    ip->i_back = &sp->fs_inodes;
    ip->i_forw->i_back = ip->i_back->i_forw = ip;
    ip->i_number = inum;
    sp->fs_ninodes++;
    return ip;
}

toy_iinit(sp)
    register FS *sp;
{
    register I *ip;

    ip = &sp->fs_inodes;
    ip->i_forw = ip->i_back = ip;
    sp->fs_ninodes = 0;
    sp->fs_maxinodes = 20;
}

toy_ikill(sp)
    register FS *sp;
{
    register I *ip, *fip, *nip;

    fip = &sp->fs_inodes;
    for( nip = fip->i_forw; (ip = nip) != fip; )
    {
	nip = ip->i_forw;
	free((char *)ip);
    }
    sp->fs_ninodes = 0;
}
