# include "toyfs.h"
# include "efs_toyfs.h"

extern USR U;

# define repeat		do
# define until(x)	while(!(x))

I *
efs_imake(sp, mode, addr)
    register FS *sp;
    int mode;
    int addr;
{
    extern ino_t efs_ialloc();
    extern I *toy_rawiget();

    register I *ip;
    register struct efs_toyinode *dip;
    ino_t inum;

    repeat
    {
	if( (inum = efs_ialloc(sp)) == 0 )
	    return 0;
    }
    until( (ip = toy_rawiget(sp, inum)) != 0 && ip->i_imode == 0 );
	
    ip->i_flags |= TOY_DIRTY;
    dip = (struct efs_toyinode *)ip->i_dinode;
    bzero((char *)&dip->inode, sizeof dip->inode);

    ip->i_imode = mode;
    ip->i_irdev = addr;
    ip->i_iatime = ip->i_imtime = ip->i_ictime = U.u_time;
    return ip;
}
