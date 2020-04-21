# include "toyfs.h"

extern USR U;

I *
toy_link(myfs, src, tgt)
    FS *myfs;
    char *src, *tgt;
{
    extern I *toy_namei(), *toy_imake(), *toy_iget();
    extern TOYIOB *toy_opendir();

    register I *ip; I *jp;

    TOYDIR d1;

    if( (ip = toy_namei(myfs, src, 1)) == 0 )
	return 0;

    if( (jp = toy_namei(myfs, tgt, 1)) != 0 )
    {
	toy_iput(ip);
	toy_iput(jp);
	U.u_errmsg = "Already exists";
	return 0;
    }
    if( !U.u_lastcomp )
    {
	U.u_errmsg = "No such directory";
	return 0;
    }
    U.u_errmsg = 0;

    d1.d_name = U.u_compbuf;
    d1.d_len = strlen(U.u_compbuf)+1;
    d1.d_ino = ip->i_number;
    d1.d_offset = U.u_slotoff;
    if( toy_direnter(myfs, U.u_pino, &d1) < 0 )
    {
	toy_iput(ip);
	return 0;
    }
    ip->i_inlink++;
    ip->i_flags |= TOY_DIRTY;
    toy_iput(ip);

    return ip;
}

int
toy_direnter(sp, pino, dep)
    FS *sp;
    ino_t pino;
    register TOYDIR *dep;
{
    extern I *toy_iget();
    extern TOYIOB *toy_opendir();

    int rv;
    register I *ip;
    register TOYIOB *iobp;

    if( (ip = toy_iget(sp, pino)) == 0 )
	return -1;

    iobp = toy_opendir(ip);
    toy_iput(ip);
    if( iobp == 0 )
	return -1;

    rv = toy_writedir(iobp, dep);

    toy_closedir(iobp);
    return rv;
}
