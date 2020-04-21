# include "toyfs.h"

# include "stdio.h"

extern USR U;

char *progname;

toy_errwarn(a)
    struct { int x[6]; } a;
{
    if( progname != 0 )
	fprintf(stderr, "%s: ", progname);
    if( U.u_errmsg != 0 )
	fprintf(stderr, "%s -- ", U.u_errmsg);
    fprintf(stderr, a);
    fprintf(stderr, "\n");
    fflush(stderr);
}

toy_errexit(a)
    struct { int x[6]; } a;
{
    toy_errwarn(a);
    exit(-1);
}
