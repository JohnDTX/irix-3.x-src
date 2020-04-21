# include "toyfs.h"

extern USR U;

F *
toy_mknod(myfs, tgt, mode, addr)
    FS *myfs;
    char *tgt;
    int mode;
    int addr;
{
    extern I *toy_makei();
    extern F *toy_openi();

    register F *fp;
    register I *ip;

    if( (ip = toy_makei(myfs, tgt, mode, addr)) == 0 )
	return 0;
    fp = toy_openi(ip);
    toy_iput(ip);
    return fp;
}

I *
toy_makei(myfs, tgt, mode, addr)
    FS *myfs;
    char *tgt;
    int mode;
    int addr;
{
    TOYDIR d1;

    extern I *toy_namei(), *toy_imake();

    register I *ip;

    U.u_errmsg = 0;

    if( (ip = toy_namei(myfs, tgt, 1)) != 0 )
    {
	;;;; /* for now, return error.  should truncate */
	U.u_errmsg = "can't creat existing file";
	return 0;
    }
    if( !U.u_lastcomp )
    {
	U.u_errmsg = "No such directory";
	return 0;
    }

    U.u_errmsg = 0;

    if( (ip = toy_imake(myfs, mode, addr)) == 0 )
	return 0;
    
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
    return ip;
}
