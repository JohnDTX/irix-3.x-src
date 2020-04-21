/*
**			     Network boot utilities
**
**			    Paul Haeberli - July 1983
**
*/
/* hi paul - there is a good reason for this!! */

typedef	unsigned char	u_char;
typedef	unsigned short	u_short;
typedef	unsigned int	u_int;
typedef	unsigned long	u_long;

# include "Qdevices.h"
# include "Qglobals.h"
# include "Xns.h"
# include "common.h"

# undef DEBUG do_debug
# include "dprintf.h"


int net_oldpri;
short net_bcastid;
long net_bctimeout;
char net_cmd;

int
net_open(ext,file)
    char *ext,*file;
{
    char str[3 + sizeof _commdat->bootstr];

    bootstr(str,sizeof str,ext,file);

    net_oldpri = spl1();

    if( net_start(str) < 0 )
    {
	splx(net_oldpri);
	return -1;
    }

    return 0;
}

net_close()
{
    CloseSeq();
    /* ResetExcelan();	/* GB - dont leave interrupt pending */
    if( net_oldpri != 0 )
	splx(net_oldpri);
}

int
net_read(_ptr,len)
    char (**_ptr);
    int len;
{
    extern char *seqdataptr;

    register int nbytes;

    while( (nbytes = ReadSeq()) <= 0 ) {
	if( nbytes == -1 )
	    break;
dprintf(("ReadSeq ret: %d\n",nbytes));
    }
dprintf(("net_read ret: %d\n",nbytes));

    *_ptr = seqdataptr;
    return nbytes;
}



/*
 * net_start() --
 * find boot server and set up a connection.
 * for use later.
 */
int
net_start( config )
    char *config;
{
    Xaddr bootaddr; char *info;
    register int conflen;

    if( findhost(config,&bootaddr.host,&info) < 0 )
	return -1;

    if( *info == SERV_NOFILE )
    {
	printf("File not found\n");
	return -1;
    }

    printf("netbooting: %s\n",info+2);

    bootaddr.socket = BOOTSOCKET;
    if( !OpenSeq(BOOTSOCKET,bootaddr) )
    {
	printf("? Failure opening boot connection\n");
	return -1;
    }
    conflen = strlen(config)+3;
    if( WriteSeq(config,conflen) != conflen )
    {
	printf("? Failure sending boot request\n");
	CloseSeq();
	return -1;
    }

    return 0;
}

int
findhost(config,_host,_info)
    char *config;
    Xhost (*_host);
    char (**_info);
{
    register int ntries;

    setbcast();

    for( ntries = 8; --ntries >= 0; )
    {
	if( bcast(config) < 0 )
	    return -1;
	if( getreply(_host,_info) >= 0 )
	    return 0;
    }

    printf("No boot server responding\n");
    return -1;
}

Xhost BcastAddress = { 0xFFFF, 0xFFFF, 0xFFFF };

setbcast()
{
    net_bcastid = Qupcount;
    /*net_bctimeout = Qtime+20;*/
}

int
bcast(str)
    char *str;
{
    extern char nxpresent;

    Xserverreq serverreq;
    register Xserverreq *Req = &serverreq;

dprintf((" bcast(%s) id=%d",str,Qupcount));

    if( !InitSeq() )
    {
	printf("? InitSeq failed\n");
	return -1;
    }

    if( !nxpresent )
    {
	printf("No ethernet!\n");
	return -1;
    }

    Req->etherheader.dst = BcastAddress;
    Req->exchid = net_bcastid;
    net_cmd = str[0];
    strncpy(Req->infostr,str,INFOSTRLEN);
    WriteEnet(SG_BOUNCE,Req,sizeof(Xserverreq));

    EnetReadTimeout(1000);	/*1 sec*/
    net_bctimeout = Qtime+2;	/*~1 sec*/

    return 0;
}

int
getreply(_host,_info)
    Xhost *_host;
    char **_info;
{
    extern Xhost MyHostAddress;
    extern char *readdatabuf;
    register int nbytes;

    for( ;; )
    {
	register Xserverreq *Reply = (Xserverreq *)readdatabuf;

	if( Qtime >= net_bctimeout )
	    return -1;

	if( (nbytes = ReadEnet(SG_BOUNCE)) == READTIMEOUT )
	    return -1;

dprintf((" time%d/%d",Qtime,net_bctimeout));
	if( HostSame(&MyHostAddress,&Reply->etherheader.dst)
	 && Reply->exchid == net_bcastid )
	if( (net_cmd == SERV_SENDFILE || net_cmd == SERV_BOOTIRIS)
		&& (Reply->infostr[0] == SERV_NOFILE
			|| Reply->infostr[0] == SERV_REPLY)
	 || net_cmd == SERV_HOSTNAME
		&& Reply->infostr[0] == SERV_IDENT )
	{
dprintf((" us"));
	    *_host = Reply->etherheader.src;
	    *_info = Reply->infostr;
	    return 0;
	}
    }
}
