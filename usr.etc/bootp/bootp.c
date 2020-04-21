#ifndef lint
static char sccsid[] = "@(#)bootp.c	1.1 (Stanford) 1/22/86";
#endif

/*
 * BOOTP (bootstrap protocol) server daemon.
 *
 * Answers BOOTP request packets from booting client machines.
 * See [SRI-NIC]<RFC>RFC951.TXT for a description of the protocol.
 */

/*
 * history
 * 01/22/86	Croft	created.
 *
 * 12/15/86	ptm	Modified for SGI use.  Changes are isolated
 *			with "ifdef sgi".
 *
 * 02/11/87	ptm	Fixed some problems in forwarding code. Removed
 *			"ifdef sgi" to improve readability.  Use the
 *			RCS history if you need to see how it used
 *			to look.
 */


#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/file.h>

#include <net/if.h>
#include <netinet/in.h>
#define	iaddr_t struct in_addr
#include "bootp.h"

#include <signal.h>
#include <stdio.h>
#include <strings.h>
#include <errno.h>
#include <ctype.h>
#include <netdb.h>
#include <setjmp.h>
#include <syslog.h>
extern char *inet_ntoa();

int	debug;
extern	int errno;
struct	sockaddr_in sin = { AF_INET };
int	s;		/* socket fd */
u_char	buf[1024];	/* receive packet buffer */
long	time(), tloc;	/* time of day */
#define	IFMAX	10	/* maximum number of interfaces supported */
struct	ifreq ifreq[IFMAX]; /* holds interface configuration */
struct	ifconf ifconf;	/* int. config. ioctl block (points to ifreq) */
struct	arpreq arpreq;	/* arp request ioctl block */

/*
 * Globals below are associated with the bootp database file (bootptab).
 */

char	*bootptab = "/usr/etc/bootptab";
FILE	*fp;
int	f;
char	line[256];	/* line buffer for reading bootptab */
char	*linep;		/* pointer to 'line' */
int	linenum;	/* current line number in bootptab */
char	homedir[64];	/* bootfile homedirectory */
char	defaultboot[64]; /* default file to boot */

#define	MHOSTS	512	/* max number of 'hosts' structs */

struct hosts {
	char	host[31];	/* host name (and suffix) */
	u_char	htype;		/* hardware type */
	u_char	haddr[6];	/* hardware address */
	iaddr_t	iaddr;		/* internet address */
	char	bootfile[32];	/* default boot file name */
} hosts[MHOSTS];

int	nhosts;		/* current number of hosts */
long	modtime;	/* last modification time of bootptab */

#define	MYNAMES_MAX	32
char	*mynames[MYNAMES_MAX];	/* save names of executing host */
iaddr_t	myhostaddr;	/* save (main) internet address of executing host */
int	forwarding;	/* flag that controls cross network forwarding */

/*
 * List of netmasks and corresponding network numbers for all
 * attached interfaces that are configured for Internet.
 */
struct	netaddr {
	u_long	netmask;	/* network mask (not shifted) */
	u_long	net;		/* address masked by netmask */
	iaddr_t	myaddr;		/* this hosts full ip address on that net */
} nets[IFMAX];

/*
 * For sgi, bootp is a single threaded server invoked
 * by the inet master daemon (/usr/etc/inetd).  Once
 * a bootp has been started, it will handle all subsequent
 * bootp requests until it has been idle for TIMEOUT
 * seconds, at which point the bootp process terminates
 * and inetd will start listening to the IPPORT_BOOTPS
 * socket again.
 */
#define	TIMEOUT	300	/* number of idle seconds to wait */

int timed_out = 0;

/*
 * SIGALRM trap handler
 */
timeout(sig)
{
	/* set flag for mainline */
	timed_out = 1;
}



main(argc, argv)
	char *argv[];
{
	register struct bootp *bp;
	register int n;
	struct	sockaddr_in from;
	int	fromlen;
	register struct hostent *hp;
	register struct ifreq *ifrp;
	register struct netaddr *np;
	struct ifreq ifnetmask;
	int ifcount;
	int len;

	for (argc--, argv++ ; argc > 0 ; argc--, argv++) {
		if (argv[0][0] == '-') {
			switch (argv[0][1]) {
			case 'd':
				debug++;
				break;
			
			case 'f':
				/* enable cross network forwarding */
				forwarding++;
				break;
			}
		}
	}

	time(&tloc);
	openlog("bootp", LOG_PID|LOG_CONS, LOG_DAEMON);
	if (debug) syslog(LOG_DEBUG, "starting at %s", ctime(&tloc));

	/*
	 * It is not necessary to create a socket for input.
	 * The inet master daemon passes the required socket
	 * to this process as fd 0.  Bootp takes control of
	 * the socket for a while and gives it back if it ever
	 * remains idle for the timeout limit.
	 */
	s = 0;
	ifconf.ifc_len = sizeof ifreq;
	ifconf.ifc_req = ifreq;
	if (ioctl(s, SIOCGIFCONF, (caddr_t)&ifconf) < 0
	    || ifconf.ifc_len <= 0) {

		syslog(LOG_ERR, "'get interface config' ioctl failed (%m)");
		exit(1);
	}

	/*
	 * Count the number of interfaces that are ifconfiged for Internet,
	 * not counting the loopback interface.  It turns out that the
	 * forwarding code in the ip layer in the kernel is only enabled
	 * when there are at least two interfaces that speak Internet
	 * protocols.  The BOOTP forwarding logic for cross-gateway
	 * booting must only be invoked when the BOOTP server is running
	 * on a system that can forward at the ip layer.
	 *
	 * While counting, also get the netmasks and the corresponding
	 * network numbers.
	 */
#define SINPTR(p)	((struct sockaddr_in *)(p))
#define SINADDR(p)	(SINPTR(p)->sin_addr.s_addr)
	bzero(nets, sizeof nets);
	np = nets;
	ifcount = 0;
	len = ifconf.ifc_len;
	for (ifrp = &ifreq[0]; len > 0; len -= sizeof (struct ifreq), ifrp++) {
		if (SINPTR(&ifrp->ifr_addr)->sin_family == AF_INET &&
		    strncmp(ifrp->ifr_name, "lo", 2)) {

			ifcount++;
			strncpy(ifnetmask.ifr_name, ifrp->ifr_name,
				sizeof(ifnetmask.ifr_name));
			if (ioctl(s, SIOCGIFNETMASK, (caddr_t)&ifnetmask) < 0) {
				syslog(LOG_ERR, "'get interface netmask' ioctl failed (%m)");
				exit(1);
			}
			np->netmask = SINADDR(&ifnetmask.ifr_addr);
			np->myaddr = SINPTR(&ifrp->ifr_addr)->sin_addr;
			np->net = np->netmask & np->myaddr.s_addr;
			np++;
		}
	}
#undef SINPTR
#undef SINADDR
	if (ifcount < 2 && forwarding) {
		if (debug) 
			syslog(LOG_DEBUG,
			       "less than two interfaces, -f flag ignored");
		forwarding = 0;
	}
	if (debug) syslog(LOG_DEBUG, "%d interfaces, forwarding is %s\n",
		ifcount, forwarding ? "ENABLED" : "DISABLED");
	len = sizeof (sin);
	if (getsockname(s, &sin, &len) < 0) {
		syslog(LOG_ERR, "getsockname fails (%m)");
		exit(1);
	}
	/*
	 * Save the name and IP address of the executing host
	 */
	makenamelist();

	/*
	 * Set an alarm to wake up bootp after timeout
	 */
	(void) signal(SIGALRM, timeout);
	(void) alarm(TIMEOUT);

	for (;;) {
		if (timed_out) {
			if (debug) syslog(LOG_DEBUG, "timed out");
			exit(0);
		}

		fromlen = sizeof (from);
		n = recvfrom(s, buf, sizeof buf, 0, (caddr_t)&from, &fromlen);
		if (n <= 0)
			continue;

		/* reset the timeout clock */
		(void) alarm(TIMEOUT);

		bp = (struct bootp *)buf;
		if (n < sizeof *bp)
			continue;

		readtab();	/* (re)read bootptab */

		switch (bp->bp_op) {
		case BOOTREQUEST:
			request();
			break;

		case BOOTREPLY:
			reply();
			break;
		}
	}

}

/*
 * Process BOOTREQUEST packet.
 *
 * <SGI CHANGES>
 *
 * Our version does implement the hostname processing
 * specified in RFC 951.  If the client specifies a hostname, then
 * only that hostname will respond.  We do one additional thing
 * that the RFC does not specify:  if the server name is not specified,
 * then we fill it in, so that the client knows the name as well
 * as the IP address of the server who responded.
 *
 * Our version also implements the forwarding algorithm specified
 * in RFC 951, but not used in the Stanford version of this code.
 * If a request is received that specifies a host other than this
 * one, check the network address of that host and forward the
 * request to him if he lives on a different wire than the one
 * from which the request was received.
 *
 * Another change from the RFC and the original version of the
 * code is that the BOOTP server will respond to the client even
 * if the client is not listed in the BOOTP configuration file,
 * provided that the client already knows his IP address.  The
 * reason for this change is to supply a bit more ease of use.
 * If there is a file out there that you want to boot, it seems
 * a shame to have to edit bootptab on the server before
 * you can boot the lousy file.  All the more so because if you
 * have already configured the IP address of your system, then
 * the bootptab information is redundant.  (This will make the
 * transition from XNS network boot to IP network boot a little
 * less painful).
 *
 * <END SGI CHANGES>
 */
request()
{
	register struct bootp *rq = (struct bootp *)buf;
	struct bootp rp;
	char path[128], file[128];
	iaddr_t fromaddr;
	register struct hosts *hp;
	register struct hostent *hostentp;
	register n;

	rp = *rq;	/* copy request into reply */
	rp.bp_op = BOOTREPLY;
	
	if (rq->bp_yiaddr.s_addr != 0 && rq->bp_giaddr.s_addr != 0) { 
		/*
		 * yiaddr has already been filled in by forwarding bootp
		 */
		hp = (struct hosts *) 0;
	} else if (rq->bp_ciaddr.s_addr == 0) {
		/*
		 * client doesnt know his IP address, 
		 * search by hardware address.
		 */
		for (hp = &hosts[0], n = 0 ; n < nhosts ; n++,hp++)
			if (rq->bp_htype == hp->htype
			   && bcmp(rq->bp_chaddr, hp->haddr, 6) == 0)
				break;
		if (n == nhosts) {
			/*
			 * The requestor isn't listed in bootptab.
			 */
			hp = (struct hosts *) 0;
			/*
			 * Try Reverse ARP using /etc/ethers or YP before
			 * giving up.
			 */
			if (!reverse_arp(rq->bp_chaddr, &rp.bp_yiaddr)) {
				/*
				 * Don't trash the request at this point,
				 * since it may be for another server with
				 * better tables than we have.
				 */
				if (debug)
				    syslog(LOG_DEBUG, "%s %s",
					"no Internet address for",
					ether_ntoa(rq->bp_chaddr));
				/* Play it safe */
				rp.bp_yiaddr.s_addr = 0;
			}
		} else
			rp.bp_yiaddr = hp->iaddr;
	} else {
		/* search by IP address */
		for (hp = &hosts[0], n = 0 ; n < nhosts ; n++,hp++)
			if (rq->bp_ciaddr.s_addr == hp->iaddr.s_addr)
				break;
		if (n == nhosts) {
			/*
			 * The requestor knows his Internet address already,
			 * but he isn't listed in bootptab.  Try to satisfy
			 * the request anyway.
			 */
			hp = (struct hosts *) 0;
		}
	}

	fromaddr = rq->bp_ciaddr.s_addr ? rq->bp_ciaddr : rp.bp_yiaddr;
	
	/*
	 * Check whether the requestor specified a particular server.
	 * If not, fill in the name of the current host.  If a
	 * particular server was requested, then don't answer the
	 * request unless the name matches our name or one of our
	 * aliases.
	 */
	if (rq->bp_sname[0] == 0)
		strncpy(rp.bp_sname, mynames[0], sizeof(rp.bp_sname));
	else if (!matchhost(rq->bp_sname)) {
		iaddr_t destaddr;

		/*
		 * Not for us.
		 */
		if (!forwarding)
		    return;
		/*
		 * Look up the host by name and decide whether
		 * we should forward the message to him.
		 */
		if ((hostentp = gethostbyname(rq->bp_sname)) == 0) {
		    syslog(LOG_INFO, "request for unknown host %s from %s",
			rq->bp_sname,
			hp ? hp->host : inet_ntoa(rq->bp_ciaddr.s_addr));
		    return;
		}
		destaddr.s_addr = *(u_long *)(hostentp->h_addr_list[0]);
		/*
		 * If the other server is on a different cable from the
		 * requestor, then forward the request.  If on the same
		 * wire, there is no point in forwarding.  Note that in
		 * the case that we don't yet know the IP address of the
		 * client, there is no way to tell whether the client and
		 * server are actually on the same wire.  In that case
		 * we forward regardless.  It's redundant, but there's
		 * no way to get around it.
		 */
		if (!samewire(&fromaddr, &destaddr)) {
		    /*
		     * If we were able to compute the client's Internet
		     * address, pass that information along in case the
		     * other server doesn't have the info.
		     */
		    rq->bp_yiaddr = rp.bp_yiaddr;
		    forward(rq, &fromaddr, &destaddr);
		}
		return;
	}
	/*
	 * If we get here and the 'from' address is still zero, that
	 * means we don't recognize the client and we can't pass the buck.
	 * So we have to give up.
	 */
	if (fromaddr.s_addr == 0)
		return;
	syslog(LOG_INFO, "request from %s for '%s'",
		hp ? hp->host : inet_ntoa(rq->bp_ciaddr.s_addr), rq->bp_file);

	if (rq->bp_file[0] == 0) {
		/* client didnt specify file */
		if (hp == (struct hosts *)0 || hp->bootfile[0] == 0)
			strcpy(file, defaultboot);
		else
			strcpy(file, hp->bootfile);
	} else {
		/* client did specify file */
		strcpy(file, rq->bp_file);
	}
	if (file[0] == '/')	/* if absolute pathname */
		strcpy(path, file);
	else {			/* else look in boot directory */
		strcpy(path, homedir);
		strcat(path, "/");
		strcat(path, file);
	}
	/* try first to find the file with a ".host" suffix */
	n = strlen(path);
	if (hp != (struct hosts *)0 && hp->host[0] != 0) {
		strcat(path, ".");
		strcat(path, hp->host);
	}
	if (access(path, R_OK) < 0) {
		path[n] = 0;	/* try it without the suffix */
		if (access(path, R_OK) < 0) {
			/*
			 * We don't have the file.  Don't respond unless
			 * the client asked for us by name, in case some
			 * other server does have the file.  If he asked
			 * for us by name, send him back a null pathname
			 * so that he knows we don't have his boot file.
			 */
			if (rq->bp_sname[0] == 0)
				return;		/* didnt ask for us */
			syslog(LOG_ERR, "boot file %s missing", path);
			path[0] = '\0';
		}
	}
	syslog(LOG_INFO, "replyfile %s", (path[0] != '\0') ? path : "<NULL>");
	strcpy(rp.bp_file, path);
	sendreply(&rp, 0);
}


/*
 * Process BOOTREPLY packet (something is using us as a gateway).
 */
reply()
{
	struct bootp *bp = (struct bootp *)buf;
	iaddr_t dst, gate;

	dst = bp->bp_yiaddr.s_addr ? bp->bp_yiaddr : bp->bp_ciaddr;

	if (debug)
		syslog(LOG_DEBUG, "forwarding BOOTP reply from %s to %s",
			bp->bp_sname, inet_ntoa(dst.s_addr));

	/*
	 * Try to compute a better giaddr in case we didn't know the
	 * client's IP address when we forwarded the original request.
	 * The remote server would not be returning the request unless
	 * it contained the client's Internet address, so this time
	 * we should get the right answer.
	 */
	if (bestaddr(&dst, &gate))
		bp->bp_giaddr = gate;
	else
		syslog(LOG_ERR, "can't find net for %s", inet_ntoa(dst.s_addr));

	sendreply(bp, 1);
}

/*
 * Forward a BOOTREQUEST packet to another server.
 */
forward(bp, from, dest)
	register struct bootp *bp;
	register iaddr_t *from;
	register iaddr_t *dest;
{
	struct sockaddr_in to;

	/*
	 * Note that the hop count field in the BOOTP request is not
	 * used in this style of forwarding.  The message is forwarded
	 * only once in this way, although it may go through many
	 * gateways to get to the destination.
	 */
	if (bp->bp_giaddr.s_addr) {
		/*
		 * This is a protocol error.  Be tolerant about it,
		 * but print a warning message.
		 */
		syslog(LOG_ERR, "forward request with gateway address already set (%s)", inet_ntoa(bp->bp_giaddr.s_addr));
	}

	/*
	 * Attempt to select the appropriate address to use as our
	 * gateway address.  Because of the way ARP works, the Internet
	 * address we give the client for referring to us must be
	 * the IP address of the interface that is on the same
	 * wire as the client.  This is because the client will
	 * need to do an ARP to get our Ethernet address in order
	 * to send TFTP packets through us to the real server.
	 *
	 * If we don't know the client's IP address yet, then we
	 * can't tell which wire the packet came in on, so we will
	 * probably guess wrong here.  If the remote server forwards the
	 * message through this process on the return trip (which
	 * the current SGI server code will do), then we will get another
	 * shot at 'bestaddr' on the return trip and that time it
	 * had better be right.  Other BOOTP servers (not SGI) may
	 * send the reply packet directly to the client using
	 * normal IP routing.
	 */
	(void) bestaddr(from, &bp->bp_giaddr);
	
	to.sin_family = htons(AF_INET);
	to.sin_port = htons(IPPORT_BOOTPS);
	to.sin_addr.s_addr = dest->s_addr;	/* already in network order */

	if (debug)
		syslog(LOG_DEBUG, "forwarding BOOTP request to %s (%s)",
			bp->bp_sname, inet_ntoa(dest->s_addr));

	if (sendto(s, (caddr_t)bp, sizeof *bp, 0, &to, sizeof to) < 0)
		syslog(LOG_ERR, "forwarding failed (%m)");
}

/*
 * Send a reply packet to the client.  'forward' flag is set if we are
 * not the originator of this reply packet.
 */
sendreply(bp, forward)
	register struct bootp *bp;
{
	iaddr_t dst;
	struct sockaddr_in to;

	to = sin;
	to.sin_port = htons(IPPORT_BOOTPC);
	/*
	 * If the client IP address is specified, use that
	 * else if gateway IP address is specified, use that
	 * else make a temporary arp cache entry for the client's NEW 
	 * IP/hardware address and use that.
	 */
	if (bp->bp_ciaddr.s_addr) {
		dst = bp->bp_ciaddr;
		if (debug) syslog(LOG_DEBUG, "reply ciaddr %x", dst.s_addr);

	} else if (bp->bp_giaddr.s_addr && forward == 0) {
		dst = bp->bp_giaddr;
		to.sin_port = htons(IPPORT_BOOTPS);
		if (debug) syslog(LOG_DEBUG, "reply giaddr %x", dst.s_addr);

	} else {
		dst = bp->bp_yiaddr;
		if (debug) syslog(LOG_DEBUG, "reply yiaddr %x", dst.s_addr);
		setarp(&dst, bp->bp_chaddr, bp->bp_hlen);
	}

	if (forward == 0) {
		/*
		 * If we are originating this reply, we
		 * need to find our own interface address to
		 * put in the bp_siaddr field of the reply.
		 * If this server is multi-homed, pick the
		 * 'best' interface (the one on the same net
		 * as the client).
		 */
		int gotmatch = 0;

		gotmatch = bestaddr(&dst, &bp->bp_siaddr);
		if (bp->bp_giaddr.s_addr == 0) {
			if (gotmatch == 0) {
				syslog(LOG_ERR, "missing gateway address");
				return;
			}
			bp->bp_giaddr.s_addr = bp->bp_siaddr.s_addr;
		}
	}
	to.sin_addr = dst;
	if (sendto(s, (caddr_t)bp, sizeof *bp, 0, &to, sizeof to) < 0)
		syslog(LOG_ERR, "send failed (%m)");
}


/*
 * Setup the arp cache so that IP address 'ia' will be temporarily
 * bound to hardware address 'ha' of length 'len'.
 */
setarp(ia, ha, len)
	iaddr_t *ia;
	u_char *ha;
{
	struct sockaddr_in *si;

	arpreq.arp_pa.sa_family = AF_INET;
	si = (struct sockaddr_in *)&arpreq.arp_pa;
	si->sin_addr = *ia;
	bcopy(ha, arpreq.arp_ha.sa_data, len);
	if (ioctl(s, SIOCSARP, (caddr_t)&arpreq) < 0)
		syslog(LOG_ERR, "set arp ioctl failed");
}


/*
 * Read bootptab database file.  Avoid rereading the file if the
 * write date hasnt changed since the last time we read it.
 */
readtab()
{
	struct stat st;
	register char *cp;
	int v;
	register i;
	char temp[64], tempcpy[64];
	register struct hosts *hp;
	int skiptopercent;

	if (fp == 0) {
		if ((fp = fopen(bootptab, "r")) == NULL) {
			syslog(LOG_ERR, "can't open %s", bootptab);
			exit(1);
		}
	}
	fstat(fileno(fp), &st);
	if (st.st_mtime == modtime && st.st_nlink)
		return;	/* hasnt been modified or deleted yet */
	fclose(fp);
	if ((fp = fopen(bootptab, "r")) == NULL) {
		syslog(LOG_ERR, "can't open %s", bootptab);
		exit(1);
	}
	fstat(fileno(fp), &st);
	if (debug)
		syslog(LOG_DEBUG, "(re)reading %s", bootptab);
	modtime = st.st_mtime;
	homedir[0] = defaultboot[0] = 0;
	nhosts = 0;
	hp = &hosts[0];
	linenum = 0;
	skiptopercent = 1;

	/*
	 * read and parse each line in the file.
	 */
	for (;;) {
		if (fgets(line, sizeof line, fp) == NULL)
			break;	/* done */
		if ((i = strlen(line)))
			line[i-1] = 0;	/* remove trailing newline */
		linep = line;
		linenum++;
		if (line[0] == '#' || line[0] == 0 || line[0] == ' ')
			continue;	/* skip comment lines */
		/* fill in fixed leading fields */
		if (homedir[0] == 0) {
			getfield(homedir, sizeof homedir);
			continue;
		}
		if (defaultboot[0] == 0) {
			getfield(defaultboot, sizeof defaultboot);
			continue;
		}
		if (skiptopercent) {	/* allow for future leading fields */
			if (line[0] != '%')
				continue;
			skiptopercent = 0;
			continue;
		}
		/* fill in host table */
		getfield(hp->host, sizeof hp->host);
		getfield(temp, sizeof temp);
		sscanf(temp, "%d", &v);
		hp->htype = v;
		getfield(temp, sizeof temp);
		strcpy(tempcpy, temp);
		cp = tempcpy;
		/* parse hardware address */
		for (i = 0 ; i < sizeof hp->haddr ; i++) {
			char *cpold;
			char c;
			cpold = cp;
			while (*cp != '.' && *cp != ':' && *cp != 0)
				cp++;
			c = *cp;	/* save original terminator */
			*cp = 0;
			cp++;
			if (sscanf(cpold, "%x", &v) != 1)
				goto badhex;
			hp->haddr[i] = v;
			if (c == 0)
				break;
		}
		if (hp->htype == 1 && i != 5) {
	badhex:		syslog(LOG_ERR, "bad hex address: %s at line %d of bootptab",
				temp, linenum);
			continue;
		}
		getfield(temp, sizeof temp);
		i = inet_addr(temp);
		if (i == -1) {
			register struct hostent *hep;
			hep = gethostbyname(temp);
			if (hep != 0 && hep->h_addrtype == AF_INET)
				i = *(int *)(hep->h_addr_list[0]);
		}
		if (i == -1 || i == 0) {
			syslog(LOG_ERR, "bad internet address: %s at line %d of bootptab",
				temp, linenum);
			continue;
		}
		hp->iaddr.s_addr = i;
		getfield(hp->bootfile, sizeof hp->bootfile);
		if (++nhosts >= MHOSTS) {
			syslog(LOG_ERR, "'hosts' table length exceeded");
			exit(1);
		}
		hp++;
	}
}


/*
 * Get next field from 'line' buffer into 'str'.  'linep' is the 
 * pointer to current position.
 */
getfield(str, len)
	char *str;
{
	register char *cp = str;

	for ( ; *linep && (*linep == ' ' || *linep == '\t') ; linep++)
		;	/* skip spaces/tabs */
	if (*linep == 0) {
		*cp = 0;
		return;
	}
	len--;	/* save a spot for a null */
	for ( ; *linep && *linep != ' ' & *linep != '\t' ; linep++) {
		*cp++ = *linep;
		if (--len <= 0) {
			*cp = 0;
			syslog(LOG_ERR, "string truncated: %s, on line %d of bootptab",
				str, linenum);
			return;
		}
	}
	*cp = 0;
}

/*
 * Perform Reverse ARP lookup using /etc/ether and /etc/hosts (or
 * their Yellow Pages equivalents)
 *
 * Returns 1 on success, 0 on failure.
 */
reverse_arp(eap, iap)
char *eap;		/* Ethernet address to lookup */
iaddr_t *iap;		/* filled in if successful */
{
	register struct hostent *hp;
	char host[512];
	int ret;

	/*
	 * Call routine to access /etc/ethers or its Yellow Pages equivalent
	 */
	if ((ret = ether_ntohost(host, eap)))
		return (0);
	
	/*
	 * Now access the hostname database (/etc/hosts or Yellow Pages).
	 */
	if ((hp = gethostbyname(host)) == 0) {
		syslog(LOG_ERR, "gethostbyname(%s) fails (%m)", host);
		return (0);
	}

	/*
	 * Return primary Internet address
	 */
	iap->s_addr = *(u_long *)(hp->h_addr_list[0]);
	return (1);
}

/*
 * Add a string to the array of names of this host
 */
addtolist(sp)
char *sp;
{
	register char **namepp;
	register int index;

	/*
	 * Check for duplicates and find end of list
	 */
	index = 0;
	namepp = mynames;
	while (*namepp) {
		if (!strcmp(*namepp, sp))
			return;
		namepp++, index++;
	}

	if (index >= MYNAMES_MAX - 1) {
		syslog(LOG_ERR, "too many host names, %s ignored", sp);
		return;
	}

	if ((*namepp = (char *) malloc(strlen(sp) + 1)) == (char *)0) {
		syslog(LOG_ERR, "can't allocate memory");
		return;
	}

	strcpy(*namepp, sp);
}

/*
 * Build a list of all the names and aliases by which
 * the current host is known on all of its interfaces.
 */
makenamelist()
{
	register struct hostent *hp;
	register char **namepp;
	char name[64];
	struct ifreq *ifrp;
	int len;

	bzero(mynames, sizeof mynames);

	/*
	 * Get name of host as told to the kernel and look that
	 * up in the hostname database.
	 */
	gethostname(name, sizeof(name));
	if ((hp = gethostbyname(name)) == 0) {
		syslog(LOG_ERR, "gethostbyname(%s) fails (%m)", name);
		exit(1);
	}
	addtolist(name);

	/*
	 * Remember primary Internet address
	 */
	myhostaddr.s_addr = *(u_long *)(hp->h_addr_list[0]);

	/*
	 * Go through all the interfaces and look up all the Internet
	 * addresses in the host database.
	 */
#define SINPTR(p)	((struct sockaddr_in *)(p))
#define SINADDR(p)	(SINPTR(p)->sin_addr.s_addr)
	len = ifconf.ifc_len;
	for (ifrp = &ifreq[0]; len > 0; len -= sizeof (struct ifreq), ifrp++) {
		if (SINPTR(&ifrp->ifr_addr)->sin_family == AF_INET) {
			hp = gethostbyaddr(
				&SINADDR(&ifrp->ifr_addr),
				sizeof (struct in_addr),
				AF_INET);
			if (hp == (struct hostent *)0) {
				syslog(LOG_ERR, "gethostbyaddr(%x) fails (%m)",
				    SINADDR(&ifrp->ifr_addr));
				continue;
			}
			/*
			 * Add the host name and all aliases from that entry
			 */
			addtolist(hp->h_name);
			for (namepp = hp->h_aliases; *namepp; namepp++)
				addtolist(*namepp);
		}
	}
#undef SINPTR
#undef SINADDR
}

/*
 * Check the passed name against the current host's names.
 *
 * Return value
 *	TRUE if match
 *	FALSE if no match
 */
matchhost(name)
register char *name;
{
	register char **namepp;

	for (namepp = mynames; *namepp; namepp++)
		if (!strcmp(*namepp, name))
			return 1;

	return 0;
}

/*
 * Select the address of the interface on this server that is
 * on the same net as the 'from' address.
 *
 * Return value
 *	TRUE if match
 *	FALSE if no match
 */
bestaddr(from, answ)
iaddr_t *from;
iaddr_t *answ;		/* filled in always */
{
	register struct netaddr *np;
	int match = 0;

	if (from->s_addr == 0) {
		answ->s_addr = myhostaddr.s_addr;
	} else {
		/*
		 * Search the table of nets to which the server is connected
		 * for the net containing the source address.
		 */
		for (np = nets; np->netmask != 0; np++)
			if ((from->s_addr & np->netmask) == np->net) {
				answ->s_addr = np->myaddr.s_addr;
				match = 1;
				break;
			}

		/*
		 * If no match in table, default to our 'primary' address
		 */
		if (np->netmask == 0)
			answ->s_addr = myhostaddr.s_addr;
	}
	return match;
}

/*
 * Check whether two passed IP addresses are on the same wire.
 * The first argument is the IP address of the requestor, so
 * it must be on one of the wires to which the server is
 * attached.
 *
 * Return value
 *	TRUE if on same wire
 *	FALSE if not on same wire
 */
samewire(src, dest)
register iaddr_t *src, *dest;
{
	register struct netaddr *np;

	/*
	 * It may well happen that src is zero, since the datagram
	 * comes from a PROM that may not yet know its own IP address.
	 * In that case the socket layer doesnt know what to fill in
	 * as the from address.  In that case, we have to assume that
	 * the source and dest are on different wires.  This means
	 * we will be doing forwarding sometimes when it isnt really
	 * necessary.
	 */
	if (src->s_addr == 0 || dest->s_addr == 0)
		return 0;

	/*
	 * In order to take subnetworking into account, one must
	 * use the netmask to tell how much of the IP address
	 * actually corresponds to the real network number.
	 *
	 * Search the table of nets to which the server is connected
	 * for the net containing the source address.
	 */
	for (np = nets; np->netmask != 0; np++)
		if ((src->s_addr & np->netmask) == np->net)
			return ((dest->s_addr & np->netmask) == np->net);

	syslog(LOG_ERR, "can't find source net for address %x\n", src->s_addr);
	return 0;
}
