# include "toyfs.h"

extern USR U;

int
toy_close(fp)
    register F *fp;
{
    if( fp == 0 )
	return -1;
    if( fp->f_ip != 0 )
	toy_iput(fp->f_ip);
    free((char *)fp);
    return 0;
}
