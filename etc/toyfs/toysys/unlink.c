# include "toyfs.h"

extern USR U;

int
toy_unlink(myfs, name)
    FS *myfs;
    char *name;
{
    extern I *toy_namei();

    register I *ip;

    TOYDIR d1;

    if( (ip = toy_namei(myfs, name, 1)) == 0 )
	return -1;

    d1.d_offset = U.u_slotoff;
    d1.d_ino = 0;
    d1.d_name = U.u_compbuf;
    d1.d_len = strlen(U.u_compbuf)+1;

    if( toy_direnter(myfs, U.u_pino, &d1) < 0 )
    {
	toy_iput(ip);
	return -1;
    }
    if( ip->i_inlink > 0 )
	ip->i_inlink--;
    ip->i_flags |= TOY_DIRTY;

    toy_iput(ip);
    return 0;
}
