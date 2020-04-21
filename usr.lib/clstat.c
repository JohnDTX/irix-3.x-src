# define STATIC static

/*
 * clstat() --
 * compatible lstat.
 * same as stat on systems not implementing lstat.
 */
# include "sys/types.h"
# include "sys/stat.h"
# include "signal.h"
# include "errno.h"
	extern int errno;

# ifdef S_IFLNK
extern int lstat();
# endif S_IFLNK
extern int stat();

static int (*statter)();

static
catchsys(signo)
    int signo;
{
    if( signo == SIGSYS )
	statter = stat;
}

int
clstat(file,_stat)
    char *file;
    struct stat *(_stat);
{
# ifdef S_IFLNK
    if( statter == 0 )
    {
	register int osig;
	struct stat stat1;

	osig = (int)signal(SIGSYS,catchsys);
	statter = lstat;
	lstat("/",&stat1);
	signal(SIGSYS,osig);
    }

    return (*statter)(file,_stat);
# else  S_IFLNK
    return stat(file,_stat);
# endif S_IFLNK
}



# ifdef SYSTEM5
# define bcopy(s,t,n)	blt(t,s,n)
# endif SYSTEM5
# define NSLBUFS	4

/*
 * routines for dealing with symbolic links.
 * several symbolic links can be open at once.
 */
typedef struct
{
	char *name;		/* opening arguments */
	int openflag,netflag;
	struct stat stb;

	char *buf;		/* holding area for link contents */
	int bufsize,bufused;
	int roff,woff;
} SLBUF;

STATIC
SLBUF slbufs[NSLBUFS];


int
sl_open(name,openflag,netflag)
    char *name;
    int openflag,netflag;
{
    extern char *malloc();

    register SLBUF *L;
    register int sfd;
    register int rv;

# ifdef S_IFLNK
    for( L = slbufs+NSLBUFS , sfd = NSLBUFS; --L , --sfd >= 0; )
	if( L->name == 0 )
	    break;
    if( sfd <= 0 )
	return -1;
    L->name = malloc(strlen(name)+1);
    strcpy(L->name,name);

    L->openflag = openflag;
    rv = clstat(name,&L->stb);
    L->netflag = netflag;
    L->roff = L->woff = 0;
    L->bufused = 0;

    switch(openflag)
    {
    case 0:
	if( rv < 0 || (L->stb.st_mode&S_IFMT) != S_IFLNK )
	    break;
	return sfd;

    case 1:
	if( rv == 0 )
	    break;
	return sfd;

    default:
	break;
    }

    L->name = 0;
# endif S_IFLNK

    return -1;
}

int
sl_creat(name,mode)
    char *name;
    int mode;
{
    return sl_open(name,1,mode);
}

int
sl_close(sfd)
    int sfd;
{
    register SLBUF *L;
    register int rv;

    if( (unsigned)sfd >= NSLBUFS
     || (L = slbufs+sfd)->name == 0 )
	return -1;

    rv = 0;
    if( L->openflag == 1 )
	rv = sl_netsymlink(L->buf,L->name,L->netflag);

    free(L->name);
    L->name = 0;
    return rv;
}

int
sl_write(sfd,src,len)
    int sfd;
    char *src;
    int len;
{
    register SLBUF *L;

    if( (unsigned)sfd >= NSLBUFS
     || (L = slbufs+sfd)->name == 0 )
	return -1;

    if( sl_extend(L,L->woff+len+1) < 0 )
	return -1;

    bcopy(src,L->buf+L->woff,len);
    L->woff += len;
    L->buf[L->woff] = 000;
    return len;
}

int
sl_read(sfd,tgt,len)
    int sfd;
    char *tgt;
    int len;
{
    register SLBUF *L;
    register int resid;

# ifdef S_IFLNK
    if( (unsigned)sfd >= NSLBUFS
     || (L = slbufs+sfd)->name == 0 )
	return -1;

    if( (resid = L->bufused - L->roff) <= len )
    {
	if( L->roff >= L->stb.st_size )
	    return 0;

	resid = L->roff + len;

	if( sl_extend(L,resid+1) < 0 )
	    return -1;
	if( (L->bufused = readlink(L->name,L->buf,resid)) < 0 )
	    return -1;
	L->buf[L->bufused] = 000;

	if( (resid = L->bufused - L->roff) <= 0 )
	    return 0;
    }

    if( resid > len )
	resid = len;
    bcopy(L->buf+L->roff,tgt,resid);
    L->roff += resid;

    return resid;
# else  S_IFLNK
    return -1;
# endif S_IFLNK
}

int
sl_extend(L,newsize)
    register SLBUF *L;
    int newsize;
{
    extern char *malloc();

    register char *oldbuf;

    newsize += 20;
    if( L->bufsize >= newsize )
	return 0;
    oldbuf = L->buf;
    if( (L->buf = malloc(newsize)) == 0 )
    {
	L->buf = oldbuf;
	return -1;
    }
    if( oldbuf != 0 )
    {
	bcopy(oldbuf,L->buf,L->bufsize);
	free(oldbuf);
    }
    L->bufsize = newsize;
    return 0;
}

int
sl_netsymlink(a,b,n)
    char *a,*b;
    int n;
{
# ifdef S_IFLNK
# ifdef S_INLNK
    return (n&S_INLNK) != 0 ? netlink(a,b,n&S_ILNIX) : symlink(a,b);
# else  S_INLNK
    return symlink(a,b);
# endif S_INLNK
# else  S_IFLNK
    return -1;
# endif S_IFLNK
}

char *
sl_getlink(name)
    char *name;
{
    char buf[512];
    register SLBUF *L;
    register char *s;
    register int sl_got;

    if( (sl_got = sl_open(name,0)) < 0 )
	return 0;
    while( sl_read(sl_got,buf,sizeof buf) > 0 )
	;
    L = slbufs+sl_got;
    s = L->buf;

    L->name = 0;
    L->buf = 0;
    L->bufsize = 0;
    return s;
}
