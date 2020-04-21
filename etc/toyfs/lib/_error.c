# include <stdio.h>


/* # arguments passed to printf */
# define NERRARGS	6


/* exports */
extern int errwarn(), scerrwarn(), errexit(), scerrexit();
extern char *progname;
extern int warned;


/* imports */
extern int fprintf(), fflush(), _exit();
extern int errno, sys_nerr;
extern char *sys_errlist[];


/* handy globals */
char *progname;				/* name of program */
int warned;				/* flag set if warning issued */


int
errwarn(a)
    struct { int x[NERRARGS]; } a;
{
    warned = -1;
    if( progname != 0 )
	fprintf(stderr, "%s: ", progname);
    fprintf(stderr, a);
    fprintf(stderr, "\n");
    fflush(stderr);
    return -1;
}

int
scerrwarn(a)
    struct { int x[NERRARGS]; } a;
{
    register int xerrno;

    xerrno = errno;

    warned = -1;
    if( progname != 0 )
	fprintf(stderr, "%s: ", progname);
    if( (unsigned)xerrno < sys_nerr )
	fprintf(stderr, "%s -- ", sys_errlist[xerrno]);
    else
	fprintf(stderr, "Error %d -- ", xerrno);
    fprintf(stderr, a);
    fprintf(stderr, "\n");
    fflush(stderr);

    errno = xerrno;
    return -1;
}


errexit(a)
    struct { int x[NERRARGS]; } a;
{
    if( progname != 0 )
	fprintf(stderr, "%s: ", progname);
    fprintf(stderr, a);
    fprintf(stderr, "\n");
    fflush(stderr);
    _exit(-1);
}

scerrexit(a)
    struct { int x[NERRARGS]; } a;
{
    register int xerrno;

    xerrno = errno;
    if( progname != 0 )
	fprintf(stderr, "%s: ", progname);
    if( (unsigned)xerrno < sys_nerr )
	fprintf(stderr, "%s -- ", sys_errlist[xerrno]);
    else
	fprintf(stderr, "Error %d -- ", xerrno);
    fprintf(stderr, a);
    fprintf(stderr, "\n");
    fflush(stderr);
    errno = xerrno;

    _exit(-1);
}
