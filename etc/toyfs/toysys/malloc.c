# include "toyfs.h"

extern char *malloc();

extern USR U;

char *
toy_malloc(size)
    char *size;
{
    register char *cp;

    if( (cp = malloc(size)) == 0 )
	U.u_errmsg = "OUT OF CORE";

    return cp;
}
