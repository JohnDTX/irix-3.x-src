# include "Qdevices.h"
# include "Qglobals.h"
# include "remprom.h"
# include "common.h"

# include "ctype.h"
# include "Xns.h"

# undef DEBUG do_debug
# include "dprintf.h"



# define BASTARD_XX
# ifdef BASTARD_XX
char client[] = "prom";
char username[] = "guest";



xx_main(argc,argv)
    int argc; char **argv;
{
    extern char z[];

    char *host,*cmd;
    int sockno;

    if( breaked() )
	return;

    SET_USER_PROGRAM;

    argc--; argv++;

    host = z;
    if( --argc >= 0 )
	host = *argv++;

    if( argc <= 0 )
    {
	cmd = z;
	argc = 1; argv = &cmd;
	sockno = XSHSOCKET;
    }
    else
    {
	sockno = EXECSOCKET;
    }

    if( net_opentty(host,sockno,argc,argv) < 0 )
    {
	printf("? connect failed\n");
    }
    else
    {
	xtermulate(sockno==EXECSOCKET);
	net_closetty();
    }

    CLEAR_USER_PROGRAM;
}

char *
shcat(tgt,len,argc,argv)
    register char *tgt; int len;
    int argc; char **argv;
{
    extern char *strlcpy();

    register char *lp;

    tgt--;
    lp = tgt+len;
    while( --argc >= 0 )
    {
	if( tgt < lp )
	    tgt++;
	tgt = strlcpy(tgt,lp,*argv++);
	*tgt = ' ';
    }
    *tgt = 000;
    return tgt;
}



# define KEYESC	0x1E	/*^^*/
xtermulate(raw)
    int raw;
{
    register int r;
    register int onechar;

    for( ;; )
    {
	if( ready(KEYIN) )
	{
	    if( (onechar = getchar()) == KEYESC )
	    {
		if( (onechar = getchar()) == 'q' )
		{
		    /*
		    net_eof();
		     */
		    newline();
		    break;
		}
		net_putchar(KEYESC);
	    }
	    if(raw) if( onechar == '\r' ) onechar = '\n';
	    net_putchar(onechar);
	}
	onechar = net_getchar();
	if( onechar == READTIMEOUT )
	    continue;
	if( onechar < 0 )
	    break;

	putchar(onechar);
	if(raw) if( onechar == '\n' ) putchar('\r');
    }
}


struct
{
    unsigned char *ptr;
    int cnt;
}   X;

int
net_opentty(host,socket,argc,argv)
    char *host;
    int socket;
    int argc; char **argv;
{
    extern char *strlcpy();
    extern char *shcat();

    extern int net_oldpri;

    char cmdbuf[512];
    register char *tgt,*last;

    net_oldpri = spl1();

    if( openhost(host,socket) < 0 )
    {
	splx(net_oldpri);
	return -1;
    }

    tgt = cmdbuf;
    last = tgt+sizeof cmdbuf-4;

    tgt = strlcpy(tgt,last,username);
    *tgt++ = 000;
    last++;

    tgt = strlcpy(tgt,last,client);
    *tgt++ = 000;
    last++;

    if( TypedWriteSeq(DST_CMD,cmdbuf,tgt-cmdbuf) < 0 )
    {
	net_closetty();
	return -1;
    }

    tgt = shcat(cmdbuf,sizeof cmdbuf-4,argc,argv);
    *tgt++ = 000;
    *tgt++ = 000;
    *tgt++ = 000;
    if( TypedWriteSeq(DST_CMD,cmdbuf,tgt-cmdbuf) < 0 )
    {
	net_closetty();
	return -1;
    }

dprintf((" opentty"));
    return 0;
}

int
openhost(host,socket)
    char *host;
    int socket;
{
    Xaddr bootaddr; char *info;
    char confbuf[512];

    bootstr(confbuf,sizeof confbuf,host,"/");
    confbuf[0] = SERV_HOSTNAME;

    if( findhost(confbuf,&bootaddr.host,&info) < 0 )
	return -1;

    bootaddr.socket = socket;
    if( !OpenSeq(IDSTARTSOCKET+1,bootaddr) )
	return -1;

    return 0;
}

/*
net_eof()
{
    short junk;

    TypedWriteSeq(DST_EOF,&junk,0);
}
 */

net_closetty()
{
    CloseSeq();
    splx(net_oldpri);
dprintf((" closetty"));
}

int
net_getchar()
{
extern short urgkeyin;

    extern char *seqdataptr;

    if( X.cnt <= 0 )
    {
dprintf((" ((RS"));
urgkeyin = 1;
	X.cnt = ReadSeq();
urgkeyin = 0;
dprintf((" RS %d))",X.cnt));
	if( X.cnt == READTIMEOUT )
	    return READTIMEOUT;
	if( X.cnt <= 0 )
	    return -1;
	X.ptr = (unsigned char *)seqdataptr;
SendSystemPacket(0,DST_SYSTEM);/*ack it now*/
    }

    --X.cnt;
    return *X.ptr++;
}

int
net_putchar(c)
    int c;
{
    char junk;
    junk = c;
    TypedWriteSeq(DST_DATA,&junk,1);
}

# else  BASTARD_XX
xx_main() {}
# endif BASTARD_XX
