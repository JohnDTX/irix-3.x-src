#
/*
 * xib.c --
 * exerciser for ib driver.
 *
 */
# include "stdio.h"
char *progname = 0;

# include "sys/types.h"
# include "signal.h"

# define max(a,b)	((a)>(b)?(a):(b))
# define min(a,b)	((a)<(b)?(a):(b))

extern char *malloc();

# define BYTEMASK 0377

# define MAXFUNK	40
struct funk
{
    int (*ffunk)();
    int farg;
};
struct funk funks[MAXFUNK];
int nafunc = 0;


int MAXTRIAL = 8*1024;
char *ibfile = "/dev/ib01";

int FD;
char *junkbuf;

int trialno = 0;
time_t starttime,endtime;
long rcount = 0;
long xcount = 0;
int junksize = 4096;

char rflg = 0;
char bflg = 0;
char vflg = 0;

char *usage = "usage:  xib [-bflmrt ...]";
main(rgc,rgv)
    int rgc;
    char **rgv;
{
    extern int xmit(),recv();

    register char *ap;
    auto int num;
    register int iii;
    register struct funk *fp;

    rgc--; rgv++;
    while( rgc > 0 && *(ap = *rgv) == '-' )
    {
	rgc--; rgv++; ap++;
	while( *ap != 000 )
	switch(*ap++)
	{
	case 'b':
	    bflg++;
	    break;
	case 'f':
	    if( --rgc < 0 )
		errexit("missing -f {ibfile}");
	    ibfile = *rgv++;
	    break;
	case 'i':
	    if( --rgc < 0 )
		errexit("missing -i {id}");
	    progname = *rgv++;
	    break;
	case 'r':
	    rflg++;
	    break;
	case 'v':
	    vflg++;
	    break;
	case 'l':
	case 't':
	    if( --rgc < 0 )
		errexit("missing -%c {size}",ap[-1]);
	    if( cnum(*rgv,&num) <= 0
	     || num < 0 )
		errexit("illegal -%c {size}, %s",ap[-1],*rgv);
	    if( num > junksize )
		junksize = num;
	    rgv++;
	    if( nafunc >= MAXFUNK )
		errexit("too many [-lt] args (max %d)"
			,MAXFUNK);
	    fp = funks + nafunc++;
	    fp->ffunk = (ap[-1]=='l') == (!rflg)
		    ?recv :xmit;
	    fp->farg = num;
	    break;
	case 'm':
	    if( --rgc < 0 )
		errexit("missing -%c {maxtrial}",ap[-1]);
	    if( cnum(*rgv,&num) <= 0 )
		errexit("illegal -%c {maxtrial}",ap[-1]);
	    MAXTRIAL = num;
	    break;
	default:
	    errexit(usage);
	    break;
	}
    }

    if( rgc != 0 )
	errexit(usage);

    if( (junkbuf = malloc(junksize)) == 0 )
	errexit("-lt size too big");

    if( (FD = open(ibfile,2)) < 0 )
	scerrexit("can't open %s",ibfile);
    time(&starttime);

    csignal(SIGHUP);
    csignal(SIGINT);
    csignal(SIGQUIT);
    csignal(SIGTERM);

    for( trialno = 0; trialno < MAXTRIAL; trialno++ )
    {
	fp = funks;
	for( iii = nafunc; --iii >= 0; )
	{
	    (*fp->ffunk)(FD,fp->farg);
	    fp++;
	}
    }

    time(&endtime);
    errwarn("succeeded %d trials after ~%d secs; %d in, %d out"
	    ,trialno,(int)(endtime-starttime)
	    ,rcount,xcount);
    exit(0);
}

xmit(fd,len)
    int len;
{
    extern char ranjunk();

    register char *bp;
    register int cnt;

    if( vflg )
	errwarn("%5d xmit%d",trialno,len);
    if( !bflg )
    {
	cnt = len;
	bp = junkbuf;
	while( --cnt >= 0 )
	    *bp++ = ranjunk();
    }
    if( (cnt = write(fd,junkbuf,len)) != len )
	scerrexit("write(...%d) returned %d trial#%d"
		,len,cnt,trialno);
    xcount += len;
}

recv(fd,len)
    int len;
{
    extern char ranjunk();

    register char *bp;
    register int cnt;

    if( (cnt = read(fd,junkbuf,junksize)) != len )
	scerrexit("read(...%d) returned %d shouldbe %d trial#%d"
		,junksize,cnt,len,trialno);
    if( !bflg )
    {
	bp = junkbuf;
	while( --cnt >= 0 )
	    if( *bp++ != ranjunk() )
	    {
		errwarn("data loss trial#%d"
			,trialno);
		printdiff(bp-1 - junkbuf);
		exit(0);
	    }
    }
    if( vflg )
	errwarn("%5d recv%d",trialno,len);
    rcount += len;
}

short value = 0;
char ranjunk()
{
    return (char) value++;
}

cleanup()
{
    time(&endtime);
    errexit("INTR trial#%d after ~%d secs; %d in, %d out"
	    ,trialno,(int)(endtime-starttime)
	    ,rcount,xcount);
}

printdiff(n)
    int n;
{
    char ranbuf[20];
    int iii;
    register char *cp;
    int b,m;

    b = max(0,n-7);
    m = min(b+16,junksize)-b;
    cp = ranbuf;
    value -= n-b;
    value--;
    for( iii = m; --iii >= 0; )
	*cp++ = ranjunk();
    printf("at %d, expected:\n",n);
    prdata(ranbuf,m,stderr);
    prdata(junkbuf+b,m,stderr);
}

csignal(signo)
    int signo;
{
    register int osig;
    extern int cleanup();

    if( (osig = (int)signal(signo,SIG_IGN)) == (int)SIG_DFL )
	signal(signo,cleanup);
    else
	signal(signo,osig);
}
