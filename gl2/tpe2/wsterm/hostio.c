/**************************************************************************
 *									  *
 * 		 Copyright (C) 1985, Silicon Graphics, Inc.		  *
 *									  *
 *  These coded instructions, statements, and computer programs  contain  *
 *  unpublished  proprietary  information of Silicon Graphics, Inc., and  *
 *  are protected by Federal copyright law.  They  may  not be disclosed  *
 *  to  third  parties  or copied or duplicated in any form, in whole or  *
 *  in part, without the prior written consent of Silicon Graphics, Inc.  *
 *									  *
 **************************************************************************/

#include <errno.h>
#include <fcntl.h>
#include <termio.h>
#include <setjmp.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <xns/Xns.h>
#include <xns/Xnsioctl.h>

#ifdef	OLD
#ifdef TCP
#include <EXOS/netinet/in.h>
#include <EXOS/sys/socket.h>
#endif
#endif	/* OLD */

#ifdef	TCP
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#endif

#include "term.h"
#include "hostio.h"

static unsigned char    rbuf[RWBUFSIZE];
static unsigned char    wbuf[RWBUFSIZE];
static jmp_buf	env;

static int abortconnect();

unsigned wc;
unsigned char  *wp;
int 	 rc;
unsigned char  *rp;

#ifdef TCP
#ifdef	OLD
static struct sockaddr_in  sinme = {
    AF_INET
};
static struct sockaddr_in  sinhim = {
    AF_INET
};
#endif	/* OLD */
#endif


/*
** 	hostconnect - make network connection to hostname
**
*/
int
hostconnect(hostname)
char hostname[];
{
    register int     try, trytcp;
    register int     host = -1;

    if (strcmp(hostname, "serial") == 0) {
	host = open(serialline, O_RDWR);
	if (host < 0)
	    errorm('W', "can't open %s", serialline);
	else
	    hosttype = SERIAL_TYPE;
	if (have488)  {
	    grhost_r = open(I488_RDEV, O_RDONLY);
	    if (grhost_r < 0) {
		errorm('W', "can't open %s", I488_RDEV);
	    	host = -1;
	    }
	    grhost_w = open(I488_WDEV, O_WRONLY);
	    if (grhost_w < 0) {
		errorm('W', "can't open %s", I488_WDEV);
	    	host = -1;
	    }
	}
	else
	    grhost_r = grhost_w = host;
    }
    else if (strcmp(hostname, "ieee488") == 0) {
	/* 
	 * assume that if logins can be done over the IEEE-488, it
	 * is a full-duplex connection, and separate devices for
	 * reading and writing won't be necessary
	 */
	host = open(I488_DEV, O_RDWR);
	if (host < 0)
	    errorm('W', "can't open %s", I488_DEV);
	else
	    hosttype = I488_TYPE;
	grhost_r = grhost_w = host;
    }
    else {

#ifdef TCP
	try = (zflag[3]) ? 1 : 2;	/* if -z 3, only try one */
	trytcp = iflag;
#else
	try = 1;
	trytcp = 0;
#endif
	if (setjmp(env)) {
	    fprintf(stdout,"Interrupted ...\n\r");
	    return -1;
	}
	savesig(SIGINT, abortconnect);
	while (try-- > 0 && host < 0) {
	    if (trytcp) {
#ifdef TCP
		host = tcpconnect(hostname);
		hosttype = TCP_TYPE;
#endif
	    }
	    else {
		host = xnsconnect(hostname, LOGINSOCKET);
		hosttype = XNS_TYPE;
	    }
	    trytcp = !trytcp;
	}
	if (host < 0)
	    errorm('w',"couldn't connect to %s",hostname);
	grhost_r = grhost_w = host;
	restoresig(SIGINT);
    }
    return host;
}

#ifdef TCP
#ifdef	OLD
/*
** 	tcpconnect - attempt an TCP/IP connection
**
*/
extern long rhost();

int
tcpconnect(hostName)
char *hostName;
{
    long    addr;
    int     fd = -1;

    addr = rhost(&hostName);
    if (addr == -1) {
	errorm('w',"TCP/IP: couldn't map host name");
	goto done;
    }
    sinhim.sin_addr.s_addr = addr;
    sinhim.sin_port = IPPORT_TELNET;
    fd = socket(SOCK_STREAM, 0, &sinme, 0);
    if (fd < 0) {
	errorm('W',"TCP/IP: couldn't create a socket for the connection");
	goto done;
    }
    if (connect(fd, &sinhim) != 0) {
 	errorm('W',"TCP/IP: couldn't create a connection ");
 	fd = -1;
 	goto done;
    }
    else
	errno = 0;
done: 
    return(fd);
}

#else	/* OLD */

/*
** 	tcpconnect - attempt an TCP/IP connection
**
*/

int
tcpconnect(hostname)
char *hostname;
{
    long    addr;
    int     fd = -1;
    register struct hostent *host;
    struct servent *sp;
    struct sockaddr_in sin;
    extern struct hostent *gethostbyname();

    sp = getservbyname("telnet", "tcp");
    if (sp == 0) {
	errorm('W', "TCP/IP: couldn't find telnet service in /etc/services");
	errorm('W', "TCP/IP: assuming telnet uses socket 23");
	sin.sin_port = 23;			/* oh well... */
    } else
	sin.sin_port = sp->s_port;
    host = gethostbyname(hostname);
    if (host) {
	sin.sin_family = host->h_addrtype;
	bcopy(host->h_addr, (caddr_t)&sin.sin_addr, host->h_length);
	hostname = host->h_name;
    } else {
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = inet_addr(hostname);
	if (sin.sin_addr.s_addr == -1) {
		errorm('W', "TCP/IP: couldn't map host name");
		goto done;
	}
    }
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
	errorm('W',"TCP/IP: couldn't create a socket for the connection");
	goto done;
    }
    if (connect(fd, (caddr_t)&sin, sizeof (sin)) < 0) {
	extern int errno;

 	errorm('W',"TCP/IP: couldn't create a connection (error %d)", errno);
 	fd = -1;
 	goto done;
    }
    else
	errno = 0;
done: 
    return(fd);
}
#endif	/* OLD */
#endif TCP

/*
**	abortconnect - interrupt routine which will abort a connection
**		       attempt.
**
*/
static
abortconnect()
{
    restoresig(SIGINT);
    longjmp(env, 1);
}


/* 
** 	condline - condition a serial line to host (set to raw mode)
**
*/
condline()
{
    struct termio   t;

    ioctl(host, TCGETA, &t);
    t.c_cflag = speedcode | CS8 | CREAD | CLOCAL | HUPCL;
    t.c_iflag = IGNBRK | IGNPAR;
    if (xflag)
	t.c_iflag |= IXOFF;
    if (yflag)
	t.c_iflag |= IXON;
    if (!fastmode)
	t.c_iflag |= ISTRIP;
    t.c_lflag = 0;
    t.c_oflag = 0;
    t.c_cc[VINTR] = 0;
    t.c_cc[VQUIT] = 0;
    t.c_cc[VERASE] = 0;
    t.c_cc[VKILL] = 0;
    t.c_cc[VMIN] = 1;
    t.c_cc[VTIME] = 0;
    ioctl(host, TCSETAF, &t);
    if (ioctl(host, TCBLKMD, 0) < 0)
	errorm('w',"NOT using blockmode tty driver");

}

/*
**	initcom - initialize communications buffers
**
*/
initcom()
{
    wc = 0;
    wp = wbuf;
    rc = 0;
    rp = rbuf;
    haveconnection = 1;
}

/*
**	closecom - close down communications
**
*/
closecom()
{
    haveconnection = 0;
}

/*
**	fillhostbuffer - read a buffer full from the host
**
*/
fillhostbuffer()
{
    do {
	if (pipeready)
	    recvpipecmd(fromwriter);
	rc = read(infile, (char *)rbuf, RWBUFSIZE);
	if (rc < 0 && errno != EINTR) {
	    errorm('W',"error reading from host");
	    return -1;
	}
    }
    while (rc < 0);
    if (rc == 0) {
	if (escsub4010)
	    return -2;
	closecom();
	return -1;
    }
    rc--;
    rp = rbuf;
    return *rp++;
}

/*
** 	flushhost - flush the graphics output buffer to the host 
**
*/
flushhost()
{
    if (wc == 0)
	return;
    write(grhost_w, (char *)wbuf, wc);
    wp = wbuf;
    wc = 0;
}

/*
** 	sendbreakchar - send a break character down the serial line 
**
*/
sendbreakchar()
{
    if (hosttype == SERIAL_TYPE)
	ioctl(host, TCSBRK, 0);
}
