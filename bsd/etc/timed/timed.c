/*
 * Copyright (c) 1985 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifndef lint
char copyright[] =
"@(#) Copyright (c) 1985 Regents of the University of California.\n\
 All rights reserved.\n";
#endif not lint

#ifndef lint
static char sccsid[] = "@(#)timed.c	2.10 (Berkeley) 6/2/86";
#endif not lint

#include "globals.h"
#define TSPTYPES
#include <protocols/timed.h>
#include <net/if.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <setjmp.h>

int id;
int trace;
int sock, sock_raw;
int status = 0;
int backoff;
int slvcount;				/* no. of slaves controlled by master */
int machup;
u_short sequence;			/* sequence number */
long delay1;
long delay2;
long random();
char hostname[MAXHOSTNAMELEN];
struct host hp[NHOSTS];
char tracefile[] = "/usr/adm/timed.log";
FILE *fd;
jmp_buf jmpenv;
struct netinfo *nettab = NULL;
int nslavenets;		/* Number of networks were I could be a slave */
int nmasternets;	/* Number of networks were I could be a master */
int nignorednets;	/* Number of ignored networks */
int nnets;		/* Number of networks I am connected to */
struct netinfo *slavenet;
struct netinfo *firstslavenet();
int Mflag;
int justquit = 0;

struct nets {
	char *name;
	long net;
	struct nets *next;
} *nets = (struct nets *)0;

#if defined(sgi) || defined(sgi_vax)
/* hosts that we trust */
struct goodhost {
	struct sockaddr_in host;
	struct goodhost *next;
} *goodhosts = 0;
#endif

/*
 * The timedaemons synchronize the clocks of hosts in a local area network.
 * One daemon runs as master, all the others as slaves. The master
 * performs the task of computing clock differences and sends correction
 * values to the slaves. 
 * Slaves start an election to choose a new master when the latter disappears 
 * because of a machine crash, network partition, or when killed.
 * A resolution protocol is used to kill all but one of the masters
 * that happen to exist in segments of a partitioned network when the 
 * network partition is fixed.
 *
 * Authors: Riccardo Gusella & Stefano Zatti
 */

main(argc, argv)
int argc;
char **argv;
{
	int on;
	int ret;
	long seed;
	int nflag, iflag;
	struct timeval time;
	struct servent *srvp;
	long casual();
	char *date();
	int n;
	int flag;
	char buf[BUFSIZ];
	struct ifconf ifc;
	struct ifreq ifreq, *ifr;
	register struct netinfo *ntp;
	struct netinfo *ntip;
	struct netinfo *savefromnet;
	struct sockaddr_in server;
	u_short port;
	uid_t getuid();

#ifdef lint
	ntip = NULL;
#endif

	Mflag = 0;
	on = 1;
	backoff = 1;
	trace = OFF;
	nflag = OFF;
	iflag = OFF;
	openlog("timed", LOG_CONS|LOG_PID, LOG_DAEMON);

	if (getuid() != 0) {
		fprintf(stderr, "Timed: not superuser\n");
		exit(1);
	}

	while (--argc > 0 && **++argv == '-') {
		(*argv)++;
		do {
			switch (**argv) {

			case 'M':
				Mflag = 1; 
				break;
			case 't':
				trace = ON; 
				break;
			case 'n':
#ifdef sgi
				if (argc < 2)
					break;
#endif
				argc--, argv++;
				if (iflag) {
					fprintf(stderr,
				    "timed: -i and -n make no sense together\n");
				} else {
					nflag = ON;
					addnetname(*argv);
				}
				while (*(++(*argv)+1)) ;
				break;
			case 'i':
#ifdef sgi
				if (argc < 2)
					break;
#endif
				argc--, argv++;
				if (nflag) {
					fprintf(stderr,
				    "timed: -i and -n make no sense together\n");
				} else {
					iflag = ON;
					addnetname(*argv);
				}
				while (*(++(*argv)+1)) ;
				break;
#if defined(sgi) || defined(sgi_vax)
			case 'F':
				add_good_host("localhost");
				if (argc < 2)
					break;
				do {
					argc--, argv++;
					add_good_host(*argv);
				} while (argc >= 2 && **(argv+1) != '-');
				while (*(++(*argv)+1)) ;
				break;
#endif
			default:
				fprintf(stderr, "timed: -%c: unknown option\n", 
							**argv);
#if defined(sgi) || defined(sgi_vax)
				exit(1);
#endif
				break;
			}
		} while (*++(*argv));
	}
#if defined(sgi) || defined(sgi_vax)
	if (argc > 0) {
		fprintf(stderr, "timed: [-tM] [-i net | -n net] [-F [host1...]]\n");
		exit(1);
	}
#endif

#ifndef DEBUG
	if (fork())
		exit(0);
	{ int s;
	  for (s = getdtablesize(); s >= 0; --s)
		(void) close(s);
	  (void) open("/dev/null", 0);
	  (void) dup2(0, 1);
	  (void) dup2(0, 2);
	  s = open("/dev/tty", 2);
	  if (s >= 0) {
		(void) ioctl(s, TIOCNOTTY, (char *)0);
		(void) close(s);
	  }
	}
#endif

	if (trace == ON) {
		fd = fopen(tracefile, "w");
#ifndef sgi
		setlinebuf(fd);
#endif
		fprintf(fd, "Tracing started on: %s\n\n", 
					date());
	}

	srvp = getservbyname("timed", "udp");
	if (srvp == 0) {
		syslog(LOG_CRIT, "unknown service 'timed/udp'");
		exit(1);
	}
	port = srvp->s_port;
	server.sin_port = srvp->s_port;
	server.sin_family = AF_INET;
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0) {
		syslog(LOG_ERR, "socket: %m");
		exit(1);
	}
	if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (char *)&on, 
							sizeof(on)) < 0) {
		syslog(LOG_ERR, "setsockopt: %m");
		exit(1);
	}
	if (bind(sock, &server, sizeof(server))) {
		if (errno == EADDRINUSE)
		        syslog(LOG_ERR, "server already running");
		else
		        syslog(LOG_ERR, "bind: %m");
		exit(1);
	}

	/* choose a unique seed for random number generation */
	(void)gettimeofday(&time, (struct timezone *)0);
	seed = time.tv_sec + time.tv_usec;
	srandom(seed);

	sequence = random();     /* initial seq number */

#ifndef sgi
	/* rounds kernel variable time to multiple of 5 ms. */
	time.tv_sec = 0;
	time.tv_usec = -((time.tv_usec/1000) % 5) * 1000;
	(void)adjtime(&time, (struct timeval *)0);
#endif

	id = getpid();

	if (gethostname(hostname, sizeof(hostname) - 1) < 0) {
		syslog(LOG_ERR, "gethostname: %m");
		exit(1);
	}
	hp[0].name = hostname;

	if (nflag || iflag) {
		struct netent *getnetent();
		struct netent *n;
		struct nets *np;
		for ( np = nets ; np ; np = np->next) {
			n = getnetbyname(np->name);
			if (n == NULL) {
				syslog(LOG_ERR, "getnetbyname: unknown net %s",
					np->name);
				exit(1);
			}
			np->net = n->n_net;
		}
	}
	ifc.ifc_len = sizeof(buf);
	ifc.ifc_buf = buf;
	if (ioctl(sock, SIOCGIFCONF, (char *)&ifc) < 0) {
		syslog(LOG_ERR, "get interface configuration: %m");
		exit(1);
	}
	n = ifc.ifc_len/sizeof(struct ifreq);
	ntp = NULL;
	for (ifr = ifc.ifc_req; n > 0; n--, ifr++) {
		if (ifr->ifr_addr.sa_family != AF_INET)
			continue;
		ifreq = *ifr;
		if (ntp == NULL)
			ntp = (struct netinfo *)malloc(sizeof(struct netinfo));
		ntp->my_addr = 
			((struct sockaddr_in *)&ifreq.ifr_addr)->sin_addr;
#if defined(sgi) || defined(sgi_vax)
		if (goodhosts != 0)		/* trust ourself */
			add_good_host(inet_ntoa(ntp->my_addr));
#endif
		if (ioctl(sock, SIOCGIFFLAGS, 
					(char *)&ifreq) < 0) {
			syslog(LOG_ERR, "get interface flags: %m");
			continue;
		}
		if ((ifreq.ifr_flags & IFF_UP) == 0 ||
			((ifreq.ifr_flags & IFF_BROADCAST) == 0 &&
			(ifreq.ifr_flags & IFF_POINTOPOINT) == 0)) {
			continue;
		}
		if (ifreq.ifr_flags & IFF_BROADCAST)
			flag = 1;
		else
			flag = 0;
		if (ioctl(sock, SIOCGIFNETMASK, 
					(char *)&ifreq) < 0) {
			syslog(LOG_ERR, "get netmask: %m");
			continue;
		}
		ntp->mask = ((struct sockaddr_in *)
			&ifreq.ifr_addr)->sin_addr.s_addr;
		if (flag) {
			if (ioctl(sock, SIOCGIFBRDADDR, 
						(char *)&ifreq) < 0) {
				syslog(LOG_ERR, "get broadaddr: %m");
				continue;
			}
			ntp->dest_addr = *(struct sockaddr_in *)&ifreq.ifr_broadaddr;
		} else {
			if (ioctl(sock, SIOCGIFDSTADDR, 
						(char *)&ifreq) < 0) {
				syslog(LOG_ERR, "get destaddr: %m");
				continue;
			}
			ntp->dest_addr = *(struct sockaddr_in *)&ifreq.ifr_dstaddr;
		}
		ntp->dest_addr.sin_port = port;
		if (nflag || iflag) {
			u_long addr, mask;
			struct nets *n;

			addr = ntohl(ntp->dest_addr.sin_addr.s_addr);
			mask = ntohl(ntp->mask);
			while ((mask & 1) == 0) {
				addr >>= 1;
				mask >>= 1;
			}
			for (n = nets ; n ; n = n->next)
				if (addr == n->net)
					break;
			if (nflag && !n || iflag && n)
				continue;
		}
		ntp->net = ntp->mask & ntp->dest_addr.sin_addr.s_addr;
		ntp->next = NULL;
		if (nettab == NULL) {
			nettab = ntp;
		} else {
			ntip->next = ntp;
		}
		ntip = ntp;
		ntp = NULL;
	}
	if (ntp)
		(void) free((char *)ntp);
	if (nettab == NULL) {
		syslog(LOG_ERR, "No network usable");
		exit(1);
	}

	for (ntp = nettab; ntp != NULL; ntp = ntp->next)
		lookformaster(ntp);
	setstatus();
	/*
	 * Take care of some basic initialization.
	 */
	/* us. delay to be used in response to broadcast */
	delay1 = casual((long)10000, 200000);	

	/* election timer delay in secs. */
	delay2 = casual((long)MINTOUT, (long)MAXTOUT);

	if (Mflag) {
		/* open raw socket used to measure time differences */
		sock_raw = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP); 
		if (sock_raw < 0)  {
			syslog(LOG_ERR, "opening raw socket: %m");
			exit (1);
		}

		/*
		 * number (increased by 1) of slaves controlled by master: 
		 * used in master.c, candidate.c, networkdelta.c, and 
		 * correct.c 
		 */
		slvcount = 1;
		ret = setjmp(jmpenv);

		switch (ret) {

		case 0: 
			makeslave(firstslavenet());
			setstatus();
			break;
		case 1: 
			/* Just lost our master */
			setstatus();
			slavenet->status = election(slavenet);
			checkignorednets();
			setstatus();
			if (slavenet->status == MASTER)
				makeslave(firstslavenet());
#if defined(sgi) || defined(sgi_vax)
/* This is yet another botch in the protocol.  election() often finds
 *	other slaves that accept us as master before the real master
 *	gets around to telling us to shut up.  We must not pass time 
 *	adjustments to these slaves.  That is because the real master 
 *	will also be sending them adjustments.
 */
			else {
				rmnetmachs(slavenet);
				makeslave(slavenet);
			}
#else
			else
				makeslave(slavenet);
#endif
			setstatus();
			break;
		case 2:
			/* Just been told to quit */
			fromnet->status = SLAVE;
			setstatus();
#if defined(sgi) || defined(sgi_vax)
/* ignore networks with bogus masters, at least for a while. */
			if (!good_host(&from.sin_addr.s_addr))
				savefromnet = slavenet;
			else
#endif
			savefromnet = fromnet;
			rmnetmachs(fromnet);
			checkignorednets();
			if (slavenet)
				makeslave(slavenet);
			else
				makeslave(savefromnet);
			setstatus();
			justquit = 1;
			break;
			
		default:
			/* this should not happen */
			syslog(LOG_ERR, "Attempt to enter invalid state");
			break;
		}
			
		if (status == MASTER) 
			master();
		else 
			slave();
	} else {
		/* if Mflag is not set timedaemon is forced to act as a slave */
		status = SLAVE;
		if (setjmp(jmpenv)) {
			setstatus();
			checkignorednets();
		}
		makeslave(firstslavenet());
		for (ntp = nettab; ntp != NULL; ntp = ntp->next)
			if (ntp->status == MASTER)
				ntp->status = IGNORE;
		setstatus();
		slave();
	}
}

/*
 * Try to become master over ignored nets..
 */
checkignorednets()
{
	register struct netinfo *ntp;
	for (ntp = nettab; ntp != NULL; ntp = ntp->next)
		if (ntp->status == IGNORE)
			lookformaster(ntp);
}

#if defined(sgi) || defined(sgi_vax)
/* suppress an upstart, untrust worthy, self-appointed master
 */
suppress(addr,name,net)
struct sockaddr_in *addr;
char *name;
struct netinfo *net;
{
	struct tsp msg, *acksend(), *readmsg();
	static struct timeval wait;

	if (trace)
		fprintf(fd, "suppress: squashing %s\n", name);
	while (0 != readmsg(TSP_ANY, (char *)ANYADDR, &wait, net)) {
		if (trace)
			fprintf(fd, "suppress:\t\tdiscarded\n");
	}
	syslog(LOG_NOTICE, "suppressing false master %s", name);
	msg.tsp_type = TSP_QUIT;
	(void)strcpy(msg.tsp_name, hostname);
	(void) acksend(&msg, addr, name, TSP_ACK, (struct netinfo *)NULL);
}

#endif
lookformaster(ntp)
	register struct netinfo *ntp;
{
	struct tsp resp, conflict, *answer, *readmsg(), *acksend();
	struct timeval time;
	char mastername[MAXHOSTNAMELEN];
	struct sockaddr_in masteraddr;

	ntp->status = SLAVE;
	/* look for master */
	resp.tsp_type = TSP_MASTERREQ;
	(void)strcpy(resp.tsp_name, hostname);
	answer = acksend(&resp, &ntp->dest_addr, (char *)ANYADDR, 
	    TSP_MASTERACK, ntp);
#if defined(sgi) || defined(sgi_vax)
	if (answer != NULL && !good_host(&from.sin_addr.s_addr)) {
		suppress(&from, answer->tsp_name, ntp);
		answer = NULL;
	}
#endif
	if (answer == NULL) {
		/*
		 * Various conditions can cause conflict: race between
		 * two just started timedaemons when no master is
		 * present, or timedaemon started during an election.
		 * Conservative approach is taken: give up and became a
		 * slave postponing election of a master until first
		 * timer expires.
		 */
		time.tv_sec = time.tv_usec = 0;
		answer = readmsg(TSP_MASTERREQ, (char *)ANYADDR,
		    &time, ntp);
		if (answer != NULL) {
			ntp->status = SLAVE;
			return;
		}

		time.tv_sec = time.tv_usec = 0;
		answer = readmsg(TSP_MASTERUP, (char *)ANYADDR,
		    &time, ntp);
		if (answer != NULL) {
			ntp->status = SLAVE;
			return;
		}

		time.tv_sec = time.tv_usec = 0;
		answer = readmsg(TSP_ELECTION, (char *)ANYADDR,
		    &time, ntp);
		if (answer != NULL) {
			ntp->status = SLAVE;
			return;
		}
		ntp->status = MASTER;
	} else {
		(void)strcpy(mastername, answer->tsp_name);
		masteraddr = from;

		/*
		 * If network has been partitioned, there might be other
		 * masters; tell the one we have just acknowledged that 
		 * it has to gain control over the others. 
		 */
		time.tv_sec = 0;
		time.tv_usec = 300000;
		answer = readmsg(TSP_MASTERACK, (char *)ANYADDR, &time,
		    ntp);
		/*
		 * checking also not to send CONFLICT to ack'ed master
		 * due to duplicated MASTERACKs
		 */
		if (answer != NULL && 
		    strcmp(answer->tsp_name, mastername) != 0) {
			conflict.tsp_type = TSP_CONFLICT;
			(void)strcpy(conflict.tsp_name, hostname);
			if (acksend(&conflict, &masteraddr, mastername,
			    TSP_ACK, (struct netinfo *)NULL) == NULL) {
				syslog(LOG_ERR, 
				    "error on sending TSP_CONFLICT");
#if defined(sgi) || defined(sgi_vax)
				if (trace)
					fprintf(fd,
					    "lookformaster: sendto errno %d\n",
						errno);
#else
				exit(1);
#endif
			}
		}
	}
}
/*
 * based on the current network configuration, set the status, and count
 * networks;
 */
setstatus()
{
	register struct netinfo *ntp;

	status = 0;
	nmasternets = nslavenets = nnets = nignorednets = 0;
	if (trace)
		fprintf(fd, "Net status:\n");
	for (ntp = nettab; ntp != NULL; ntp = ntp->next) {
		switch ((int)ntp->status) {
		  case MASTER:
			nmasternets++;
			break;
		  case SLAVE:
			nslavenets++;
			break;
		  case IGNORE:
			nignorednets++;
			break;
		}
		if (trace) {
			fprintf(fd, "\t%-16s", inet_ntoa(ntp->net));
			switch ((int)ntp->status) {
			  case MASTER:
				fprintf(fd, "MASTER\n");
				break;
			  case SLAVE:
				fprintf(fd, "SLAVE\n");
				break;
			  case IGNORE:
				fprintf(fd, "IGNORE\n");
				break;
			  default:
				fprintf(fd, "invalid state %d\n");
				break;
			}
		}
		nnets++;
		status |= ntp->status;
	}
	status &= ~IGNORE;
	if (trace)
		fprintf(fd,
		      "\tnets = %d, masters = %d, slaves = %d, ignored = %d\n",
		      nnets, nmasternets, nslavenets, nignorednets);
}

makeslave(net)
	struct netinfo *net;
{
	register struct netinfo *ntp;

	for (ntp = nettab; ntp != NULL; ntp = ntp->next)
		if (ntp->status == SLAVE && ntp != net)
			ntp->status = IGNORE;
	slavenet = net;
}
	
struct netinfo *
firstslavenet()
{
	register struct netinfo *ntp;

	for (ntp = nettab; ntp != NULL; ntp = ntp->next)
		if (ntp->status == SLAVE)
			return (ntp);
	return ((struct netinfo *)0);
}

/*
 * `casual' returns a random number in the range [inf, sup]
 */

long
casual(inf, sup)
long inf;
long sup;
{
	float value;

	value = (float)(random() & 0x7fffffff) / 0x7fffffff;
	return(inf + (sup - inf) * value);
}

char *
date()
{
	char    *ctime();
	struct	timeval tv;

	(void)gettimeofday(&tv, (struct timezone *)0);
	return (ctime(&tv.tv_sec));
}

addnetname(name)
	char *name;
{
	register struct nets **netlist = &nets;

	while (*netlist)
		netlist = &((*netlist)->next);
	*netlist = (struct nets *)malloc(sizeof **netlist);
	if (*netlist == (struct nets *)0) {
		syslog(LOG_ERR, "malloc failed");
		exit(1);
	}
	bzero((char *)*netlist, sizeof(**netlist));
	(*netlist)->name = name;
}
#if defined(sgi) || defined(sgi_vax)

/* note a host as trustworthy */
add_good_host(name)
char* name;
{
	register struct goodhost *ghp;
	register struct hostent *hp;

	ghp = (struct goodhost*)malloc(sizeof(*ghp));
	if (!ghp) {
		syslog(LOG_ERR, "malloc failed");
		exit(1);
	}

	ghp->host.sin_family = AF_INET;
	ghp->host.sin_addr.s_addr = inet_addr(name);
	if (ghp->host.sin_addr.s_addr == -1) {
		hp = gethostbyname(name);
		if (hp) {
			ghp->host.sin_family = hp->h_addrtype;
			bcopy(hp->h_addr, (caddr_t)&ghp->host.sin_addr,
			      hp->h_length);
		} else {
			printf("timed: unknown host %s\n", name);
			exit(1);
		}
	}

	ghp->next = goodhosts;
	goodhosts = ghp;
}


/* see if a machine is trustworthy */
int				/* 1=trust this guy to change our date */
good_host(hp)
struct in_addr *hp;
{
	register struct goodhost *ghp = goodhosts;

	if (!ghp)			/* trust everyone if no one named */
		return 1;

	do {
		if (hp->s_addr == ghp->host.sin_addr.s_addr)
			return 1;	/* found him, so say so */
	} while (0 != (ghp = ghp->next));

	return 0;			/* did not find him */
}

#endif
