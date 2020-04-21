#ifndef lint
/* @(#)rarpd.c	2.1 86/04/16 NFSSRC */
static  char sccsid[] = "@(#)rarpd.c 1.1 86/02/05 Copyr 1985 Sun Micro";
#endif

/*
 * rarpd.c  Reverse-ARP server.
 * Copyright (c) 1985 by Sun Microsystems, Inc.
 */

#include <stdio.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <sys/time.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <net/nit.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>
#include <netinet/ip_var.h>
#include <netinet/udp.h>
#include <netinet/udp_var.h>
#include <netdb.h>

#define BUFSIZE 10000
char *malloc();

static int
	received, 	/* total packets read */
	bad,		/* packets that did not look like rarp packets */
	unknown,	/* unknown ether -> ip address mapping */
	processed,	/* answer known and sent */
	delayed,	/* answer was delayed before sending */
	weird;		/* unexpected, yet valid */
	/* ignored = received - (bad + unknown + processed) */
	
static int if_fd;
static struct ether_addr my_etheraddr, *eap;
static u_char my_ipaddr[4];  /* in network order */
static char *cmdname;
static int debug;

extern int errno;

/*
 * When the rarp packet is a request (arp_op = REVARP_REQUEST = 3), then
 *	arp_xsha is the ethernet address of the sender of the request;
 *	arp_xspa is undefined;
 *	arp_xtha is the 'target' hardware address, i.e. the sender's address,
 *	arp_xtpa is undefined.
 * The rarp reply (arp_op = REVARP_REPLY = 4) looks like:
 *	arp_xsha is the responder's (our) ethernet address;
 *	arp_xspa is the responder's (our) IP address;
 *	arp_xtha is identical to the request packet;
 *	arp_xtpa is the request's desired IP address.
 */
struct rarp_request {
	struct	ether_header rr_eheader;
	struct ether_arp rr_arpheader;
	u_short rr_len;  /* not part of the protocol; fill in at "read" time */
};

main(argc, argv)
	int argc;
	char **argv;
{
	register int i;
	struct ifreq ifr;
	int delay_fd[2], d_fd;
	char *device, *buf, *cause;
	register struct rarp_request *rqst;
	struct rarp_request ans;
	char host[256];
	register struct hostent *hp;
	struct timeval now, then;
	struct timezone zone;

	cmdname = argv[0];
	while (argc > 1 && argv[1][0] == '-' && argv[1][2] == '\0') {
		switch (argv[1][1]) {
		case 'd':
			debug++;
			break;
		default:
			usage();
		}
		argc--, argv++;
	}
	if (argc != 3)
		usage();
	/*
	 * Using the hostname, get the associated IP address.
	 */
	device = argv[2];  /* really host name */
	if (((hp = gethostbyname(device)) == (struct hostent *) NULL) ||
	    (hp->h_length != sizeof (my_ipaddr))) {
		fprintf(stderr, "%s: cannot find host entry for %s\n",
		    argv[0], device);
		exit(2);
	}
	bcopy(hp->h_addr, my_ipaddr, sizeof (my_ipaddr));
	/*
	 * Open the ether device and pull out the ether address.
	 */
	device = argv[1];
        if_fd = rarp_open(device, htons((u_short)ETHERPUP_REVARPTYPE));
        if (if_fd < 0)
                exit(5);
	strncpy(ifr.ifr_name, device, sizeof ifr.ifr_name);
        if (ioctl (if_fd, SIOCGIFADDR, (caddr_t) &ifr) < 0) {
		fprintf(stderr, "%s: (ioctl) cannot find ether entry for %s\n",
                    argv[0], device);
	    	exit(3);
        }
	eap = (struct ether_addr *) &ifr.ifr_addr.sa_data[0];
	my_etheraddr = *eap;

	if (!debug) {
		/*
		 * Background
		 */
		while (if_fd < 3) {
			if_fd = dup(if_fd);
		}
		switch (fork()) {
		
		case -1:
			fprintf(stderr, "%s: fork failure\n", argv[0]);
			exit(6);
			break;
		
		case 0:
			break;
		
		default:
			exit(0);
			break;
		}
		for (i = 0; i < 32; i++) {
			if (i != if_fd) {
				close(i);
			}
		}
		(void) open("/", O_RDONLY, 0);
		(void) dup2(0, 1);
		(void) dup2(0, 2);
		/*
		 * Detach terminal
		 */
		if ((i = open("/dev/tty", O_RDWR, 0)) >= 0) {
			ioctl(i, TIOCNOTTY, 0);
			close(i);
		}
	}
	/*
	 * Fork off a delayed responder which uses a pipe.
	 */
	 if (pipe(delay_fd)) {
	 	perror("rarpd: pipe");
	 	exit(7);
	}

	 switch (fork()) {
	 
	 case -1:
	 	perror("rarpd: delayed fork");
	 	exit(8);
	 	break;
	 
	 case 0:
	 	/* child reads the pipe and sends responses */
	 	close(delay_fd[1]);  /* no writing */
	 	d_fd = delay_fd[0];
	 	buf = host;
	 	i = sizeof (ans) - sizeof (ans.rr_len);
	 	for (;;) {
	 		if ((read(d_fd, &then, sizeof (then)) != sizeof (then))
	 		    || (read(d_fd, buf, i) != i)) {
	 			perror("delayed rarpd: read");
	 			exit(9);
	 		}
	 		received++;
	 		/*
	 		 * It is our job to never send a (delayed) reply
	 		 * in the same second of time that it was created;
	 		 * rather wait up to three seconds before responding.
	 		 */
	 		gettimeofday(&now, &zone);
	 		if (now.tv_sec == then.tv_sec) {
	 			sleep(3);
	 			delayed++;
	 		}
	 		if (rarp_write(if_fd, buf, i) != i) {
	 			perror("delayed rarpd: rarp_write");
	 			exit(10);
	 			break;  /* eliminate warning msgs */
	 		};
	 	}
	 	/*NOTREACHED*/
	 	exit(0);
	 	break;
	 
	 default:
	 	/* parent does most processing and maybe writes to the child */
	 	close(delay_fd[0]);  /* no reading */
	 	d_fd = delay_fd[1];
	 	break;
	 }
	/*
	 * read RARP packets and respond to them.
	 *
	 * This server adapts to heavy loads by ignoring requests;
	 * this is accomplished by processing only the last packet
	 * of a multi-packet read call.
	 *
	 * This server delays answering requests if it cannot
	 * "tftp" boot the requestor.
	 */
	buf = malloc(BUFSIZE);
	for (;;) {
		char *bp, *last;
		struct nit_hdr *nh;
		int datalen, cnt;

		if ((i = read(if_fd, buf, BUFSIZE)) < 0) {
			perror("rarpd: read");
			exit(11);
		}
		if (debug > 1)
			fprintf(stderr, "DEBUG: read read %d\n", i);
		/*
		 * find the last request in the requests' buffer.
		 */
		cnt = 0;
		last = 0;
		for (bp = buf; bp < buf+i;
		    bp += ((sizeof(*nh)+datalen+sizeof(int)-1)
				& ~(sizeof (int)-1))) {
			nh = (struct nit_hdr *)bp;
			if (nh->nh_state != NIT_CATCH)
				datalen = 0;
			else {
				cnt++;
				last = bp;
				datalen = nh->nh_datalen;
			}
		}
		/*
		 * Consistency check: verify that the total read length
		 * matches the cumulative length obtained by using the
		 * individual lengths contained in the nit headers.  This
		 * check must take header alignment into account.
		 */
		if (last == 0 || ((u_int)(last+sizeof(*nh)+datalen+sizeof(int)-1)
					& ~(sizeof(int)-1)) !=
		    ((u_int)(buf+i+sizeof(int)-1) & ~(sizeof(int)-1))) {
			fprintf(stderr, "%s: badly aligned read.\n", argv[0]);
			exit(12);
		}

		received += cnt;
		if (debug > 1)
			fprintf(stderr, "DEBUG: received %d packets\n", cnt);
		nh = (struct nit_hdr *)last;
		ans = *(struct rarp_request *)(last+sizeof(*nh));
		ans.rr_len = nh->nh_wirelen;
		/* 
		 * Sanity checks ... set i to sizeof an rarp packet
		 */
		i = sizeof (ans) - sizeof (ans.rr_len);
		cause = 0;

		if (ans.rr_len < i)
			cause="rr_len";
		else if (ans.rr_eheader.ether_type !=
		    (u_short)ETHERPUP_REVARPTYPE)
			cause="type";
		else if (ans.rr_arpheader.arp_hrd != htons(ARPHRD_ETHER))
			cause="hrd";
		else if (ans.rr_arpheader.arp_pro != ETHERPUP_IPTYPE)
			cause="pro";
		else if (ans.rr_arpheader.arp_hln != 6)
			cause="hln";
		else if (ans.rr_arpheader.arp_pln != 4)
			cause="pln";
		if (cause) {
			if (debug)
				fprintf(stderr,
				    "DEBUG: sanity check failed; cause: %s\n",
				    cause);
			continue;
		}
		switch (ans.rr_arpheader.arp_op) {
		
		case REVARP_REQUEST:
			if (debug > 1)
				fprintf(stderr, "DEBUG: REVARP_REQUEST\n");
			break;
		
		case ARPOP_REQUEST:
			if (debug > 1)
				fprintf(stderr, "DEBUG: ARPOP_REQUEST\n");
			arp_request(&ans);
			continue;
		
		default:
			if (debug)
				fprintf(stderr,
				    "DEBUG: INVALID 0x%xd\n",
				    ans.rr_arpheader.arp_op);
			bad++;
			continue;
		}
		/*
		 * Check for weird (although valid) requests
		 */
		if (bcmp(ans.rr_arpheader.arp_xsha,
		    ans.rr_arpheader.arp_xtha, 6)  |
		    bcmp(ans.rr_arpheader.arp_xsha,
		    &ans.rr_eheader.ether_shost, 6)) {
			if (debug)
				fprintf(stderr, "DEBUG: WEIRD\n");
			weird++;
		}
		/*
		 * process the RARP request.
		 */
		ans.rr_eheader.ether_dhost =
		    *(struct ether_addr *)ans.rr_arpheader.arp_xsha;
		ans.rr_arpheader.arp_op = REVARP_REPLY;
		bcopy(&my_etheraddr, ans.rr_arpheader.arp_xsha,
		    sizeof (ans.rr_arpheader.arp_xsha));
		bcopy(my_ipaddr, ans.rr_arpheader.arp_xspa,
		    sizeof (ans.rr_arpheader.arp_xspa));
		if ((ether_ntohost(host, ans.rr_arpheader.arp_xtha) != 0) ||
		    ((hp = gethostbyname(host)) == (struct hostent *)NULL) ||
		    (hp->h_length != sizeof (ans.rr_arpheader.arp_xtpa))) {
			if (debug)
			    fprintf(stderr, "DEBUG: UNKNOWN\n");
			unknown++;
			continue;
		}
		bcopy(hp->h_addr, ans.rr_arpheader.arp_xtpa,
		    sizeof (ans.rr_arpheader.arp_xtpa));
		/*
		 * Add the requestor's ARP entry in anticipation of
		 * further conversation.
		 */
		 add_arp(ans.rr_arpheader.arp_xtpa,
		     &ans.rr_eheader.ether_dhost);
		/*
		 * send the answer.
		 * It is delayed if we cannot tftp boot the requestor
		 */
		sprintf(host, "/tftpboot/%08X",
		    ntohl(*(u_long *)ans.rr_arpheader.arp_xtpa));
		if ((!debug) && stat(host, buf)) {
			gettimeofday(&now, &zone);
			if ((write(d_fd, &now, sizeof (now)) != sizeof (now))
			    || (write(d_fd, &ans, i) != i)) {
				perror("rarpd: delayed write");
				exit(13);
			}
			delayed++;
		} else if (rarp_write(if_fd, &ans, i) != i) {
			perror("rarpd: rarp_write");
			exit(14);
		}
		processed++;
	}
}

/*
 * down loads regular ARP entries to the kernel.
 * NB: Only down loads if an entry does not already exist.
 */
static
add_arp(ipap, eap)
	char *ipap;  /* IP address pointer */
	struct ether_addr *eap;
{
	struct arpreq ar;
	struct sockaddr_in *sin;
	int s;
	
	/*
	 * Common part of query or set
	 */
	bzero((caddr_t)&ar, sizeof (ar));
	ar.arp_pa.sa_family = AF_INET;
	sin = (struct sockaddr_in *)&ar.arp_pa;
	sin->sin_addr = *(struct in_addr *)ipap;
	s = socket(AF_INET, SOCK_DGRAM, 0);
	/*
	 * If one exits, return
	 */
	if ((ioctl(s, SIOCGARP, (caddr_t)&ar) == 0) || (errno != ENXIO)) {
		close(s);
		return;
	}
	/*
	 * Set the entry
	 */
	bcopy(eap, ar.arp_ha.sa_data, sizeof (*eap));
	ar.arp_flags = 0;
        (void) ioctl(s, SIOCSARP, (caddr_t)&ar);
        close(s);
}

/*
 * The RARP spec says we must be able to process ARP requests, even through
 * the packet type is RARP.  Let's hope this feature is not heavily used.
 */
static
arp_request(a)
	struct rarp_request *a;
{

	if (!bcmp(my_ipaddr, a->rr_arpheader.arp_xtpa, sizeof (my_ipaddr))) {
		return;
	}
	a->rr_eheader.ether_dhost =
	    *(struct ether_addr *)a->rr_arpheader.arp_xsha;
	a->rr_arpheader.arp_op = ARPOP_REPLY;
	bcopy(a->rr_arpheader.arp_xsha, a->rr_arpheader.arp_xtha,
	    sizeof (a->rr_arpheader.arp_xsha));
	bcopy(a->rr_arpheader.arp_xspa, a->rr_arpheader.arp_xtpa,
	    sizeof (a->rr_arpheader.arp_xspa));
	bcopy(&my_etheraddr, a->rr_arpheader.arp_xsha,
	    sizeof (a->rr_arpheader.arp_xsha));
	bcopy(my_ipaddr, a->rr_arpheader.arp_xspa,
	    sizeof (a->rr_arpheader.arp_xspa));
	add_arp(a->rr_arpheader.arp_xtpa, &a->rr_eheader.ether_dhost);
	(void) rarp_write(if_fd, a, sizeof (a) - sizeof(a->rr_len));
}

usage()
{
	fprintf(stderr, "Usage: %s [-d] device hostname\n", cmdname);
	exit(1);
}
static int
rarp_open(device, type)
	char *device;
	u_short type;
{
	int if_fd;
	struct sockaddr_nit snit;
	struct nit_ioc nioc;
	
	if_fd = socket(AF_NIT, SOCK_RAW, NITPROTO_RAW);
	if (if_fd < 0) {
		perror("nit socket");
		return (-1);
	}

	snit.snit_family = AF_NIT;
	strncpy(snit.snit_ifname, device, NITIFSIZ);
	if (bind(if_fd, &snit, sizeof(snit)) != 0) {
		perror(device);
		return (-2);
	}

	bzero(&nioc, sizeof(nioc));
	nioc.nioc_bufspace = NITBUFSIZ;
	nioc.nioc_chunksize = NITBUFSIZ;
	nioc.nioc_typetomatch = type;
	nioc.nioc_snaplen = 32767;
	nioc.nioc_flags = NF_TIMEOUT;
	nioc.nioc_timeout.tv_usec = 200;
	if (ioctl(if_fd, SIOCSNIT, &nioc) != 0) {
		perror("nit ioctl");
		return (-3);
	}
	return (if_fd);
}

static int
rarp_write(fd, buf, len)
	int fd, len;
	char *buf;
{
	struct sockaddr sa;
	int offset = sizeof(sa.sa_data);
	int result;

	sa.sa_family = AF_UNSPEC;
	bcopy(buf, sa.sa_data, offset);
	result = sendto(fd, buf+offset, len-offset, 0, &sa, sizeof(sa));
	return (result+offset);
}

/*
 * Read and copy the last packet into <pkt>,
 * stuffing the wirelength at the end.
 */
static int
rarp_last(fd, pkt, plen)
	int fd;
	char *pkt;
	int plen;
{
	register char *bp, *last;
	register struct nit_hdr *nh;
	register int len, datalen;
	char buf[NITBUFSIZ];

	if ((len = read(fd, buf, sizeof(buf))) < 0) {
		perror("rarpd: read");
		return (-1);
	}
	/*
	 * find the last request in the requests' buffer.
	 */
	last = 0;
	for (bp = buf; bp < buf+len; bp += sizeof(*nh)+datalen) {
		nh = (struct nit_hdr *)bp;
		if (nh->nh_state != NIT_CATCH)
			datalen = 0;
		else {
			last = bp;
			datalen = nh->nh_datalen;
		}
	}
	if (last == 0) {
		fprintf(stderr, "rarp_last: no packets returned\n");
		return (-2);
	}
	if (last+sizeof(*nh)+datalen != buf+len) {
		fprintf(stderr, "rarp_last: truncated packet\n");
		return (-3);
	}

#define MIN(s, t)	((s)<(t)?(s):(t))
	datalen = MIN(datalen, plen - sizeof (u_short));
	nh = (struct nit_hdr *)last;
	bcopy(last+sizeof(*nh), pkt, datalen);
	((u_short *)(pkt+plen))[-1] = nh->nh_wirelen;

	return (datalen + sizeof (u_short));
}
