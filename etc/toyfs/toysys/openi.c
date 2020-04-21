# include "toyfs.h"

extern USR U;

extern char *toy_malloc();

F *
toy_openi(ip)
    register I *ip;
{
    register F *fp;

    U.u_errmsg = 0;

    if( (fp = (F *)toy_malloc(sizeof (F))) == 0 )
	return 0;

    ip->i_count++;
    fp->f_offset = 0;
    fp->f_ip = ip;
    return fp;
}
