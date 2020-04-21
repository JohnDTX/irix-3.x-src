# include "toyfs.h"

extern USR U;

extern I *toy_copen();

extern F *toy_open();

F *
toy_open(myfs, name)
    FS *myfs;
    char *name;
{
    extern F *toy_openi();

    register I *ip;
    register F *fp;

    if( (ip = toy_copen(myfs, name)) == 0 )
	return 0;
    fp = toy_openi(ip);
    toy_iput(ip);
    return fp;
}

I *
toy_copen(myfs, name)
    FS *myfs;
    char *name;
{
    extern I *toy_namei();
    register I *ip;

    ip = toy_namei(myfs, name, 1);
    return ip;
}
