# define SYSTEM5
# include "cib.defs"
/*
 * tib.c --
 * print kernel trace buffer.
 */
# include "../gpib/ib_trc.h"

# include "stdio.h"
# include "nlist.h"


char *progname = "tib";


struct trace _trcbuf[NTCELLS];
struct trace *_trc;

struct nlist uvars[] = 
{
# define K_TRCBUF	((caddr_t)uvars[0].n_value)
	{"__trcbuf"},
# define K_TRC		((caddr_t)uvars[1].n_value)
	{"__trc"},
#
	{0}
};

char *unic = "/vmunix";
char *memf = "/dev/kmem";
char lflg = 0;

char *usage = "usage:  trc [-kln ...]";

main(rgc,rgv)
    int rgc;
    char **rgv;
{
    register char *ap;

    if( --rgc < 0 )
	errexit("arg cnt");
    progname = *rgv++;
    while( rgc > 0 && *(ap = *rgv) == '-' )
    {
	rgc--; rgv++; ap++;
	while( *ap != 000 )
	switch(*ap++)
	{
	case 'k':
	    if( --rgc < 0 )
		errexit("missing -k {mem}");
	    memf = *rgv++;
	    break;
	case 'l':
	    lflg++;
	    break;
	case 'n':
	    if( --rgc < 0 )
		errexit("missing -n {unix}");
	    unic = *rgv++;
	    break;
	default:
	    errexit(usage);
	    break;
	}
    }
    if( rgc != 0 )
	errexit(usage);

    if( gnlist(unic,uvars) < 0 )
	errexit("bad nlist %s",unic);

    dumptrc();

    exit(0);
}


dumptrc()
{
    register struct trace *tp;
    register int iii;

    kread(K_TRCBUF,_trcbuf,sizeof _trcbuf);
    kread(K_TRC,&_trc,sizeof _trc);
    if( !( K_TRCBUF<=(caddr_t)_trc
     && (caddr_t)_trc<K_TRCBUF+NTCELLS*sizeof (struct trace) ) )
	errexit("_trc == 0x%x, out of range",_trc);
    tp = (struct trace*)(
	    (char*)_trcbuf + ((caddr_t)_trc - K_TRCBUF) );
    for( iii = NTCELLS; --iii >= 0; )
    {
	if( tp->when != 0 )
	    dump1(tp);
	tp++;
	if( tp == _trcbuf+NTCELLS )
	    tp = _trcbuf;
    }
}

dump1(tp)
    register struct trace *tp;
{
    extern char *kstring();

    register char *s;
    register int iii;
    register int *ap;

    if( lflg )
	printf("%5d",tp->when);

    setkstring();
    ap = tp->args+0;
    s = kstring((caddr_t)*ap);
    *ap++ = (int)s;
    iii = 1;
    while( *s != 000 )
    if( *s++ == '%' )
    {
	switch(*s++)
	{
	case 's':
	    if( iii >= NTARGS )
	    {
		errwarn("too many %%s in \"%s\"",tp->args[0]);
		iii--;
	    }
	    *ap = (int)kstring((caddr_t)*ap);
	    break;
	case '%':
	    ap--;
	    iii--;
	    break;
	default:
	    break;
	}
	ap++;
	iii++;
    }

    ap = tp->args+0;
    printf(ap[0],ap[1],ap[2],ap[3]);
    fflush(stdout);

    if( lflg )
	printf("\n");
}

int
kread(addr,buf,len)
    caddr_t addr;
    char *buf;
    int len;
{
    static int memfd = -1;
    register int cnt;

    if( memfd < 0 )
    {
	if( (memfd = open(memf,0)) < 0 )
	    scerrexit("can't open %s",memf);
    }
    lseek(memfd,(off_t)addr,0);
    if( (cnt = read(memfd,buf,len)) != len )
	scerrexit("mem read err @ 0x%x",addr);
    return cnt;
}

# define NKSTRING 1024
char ksbuf[NKSTRING+1];
char *kslim = ksbuf+NKSTRING;
char *kspt = ksbuf+0;

setkstring()
{
    kspt = ksbuf+0;
    kslim = ksbuf+NKSTRING;
}

char *kstring(x)
    caddr_t x;
{
    register char *s,*z;
    kseek(x);
    s = z = kspt;
    while( (*z = kgetc()) != 000 )
    {
	if( z >= kslim )
	{
	    errwarn("kstrings too big");
	    return s;
	}
	z++;
    }
    *z++ = 000;
    kspt = z;
    return s;
}

# define MEMBUF 64
struct mio
{
    char mbed[MEMBUF];
    caddr_t moff;
    int mused;
};
struct mio m;

kseek(x)
    caddr_t x;
{
    m.mused = (int)x%MEMBUF;
    m.moff = x - m.mused;
    kread(m.moff,m.mbed,sizeof m.mbed);
}

kgetc()
{
    if( m.mused >= MEMBUF )
    {
	m.moff += MEMBUF;
	kseek(m.moff);
    }
    return m.mbed[m.mused++]&BYTEMASK;
}

int gnlist(unic,vars)
    char *unic;
    struct nlist *vars;
{
    register struct nlist *np;
    char botch;

    nlist(unic,vars);

    botch = 0;
    for( np = vars; np->n_name != 0 && *np->n_name != 000; np++ )
	if( np->n_type == -1 || np->n_value == 0 )
	{
	    botch--;
	    errwarn("kvar %s not found",np->n_name);
	}
    return botch;
}
