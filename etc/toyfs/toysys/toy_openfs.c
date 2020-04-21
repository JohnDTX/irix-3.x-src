# include "toyfs.h"

extern USR U;

FS *
toy_openfs(name, wflag)
    char *name;
    int wflag;
{
    extern FS *toy_ropenfs(), *toy_wopenfs();

    if( !wflag )
	return toy_ropenfs(name);
    else
	return toy_wopenfs(name);
}
