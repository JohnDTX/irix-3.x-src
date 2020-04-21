/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifndef lint
char copyright[] =
"@(#) Copyright (c) 1983 Regents of the University of California.\n\
 All rights reserved.\n";
#endif not lint

#ifndef lint
static char sccsid[] = "@(#)main.c	5.7 (Berkeley) 4/20/86";
#endif not lint

/*
 * Routing Table Management Daemon
 */
#include "defs.h"
#include <sys/ioctl.h>
#include <sys/time.h>

#include <net/if.h>

#include <errno.h>
#include <signal.h>
#include <syslog.h>

int	supplier = -1;		/* process should supply updates */
int	gateway = 0;		/* 1 if we are a gateway to parts beyond */

struct	rip *msg = (struct rip *)packet;
int	hup();

#ifdef sgi
long time_ck = 0;		/* check for lost timer */
#endif

main(argc, argv)
	int argc;
	char *argv[];
{
	int cc;
	struct sockaddr from;
	u_char retry;
	
	argv0 = argv;
	openlog("routed", LOG_PID | LOG_ODELAY, LOG_DAEMON);
	setlogmask(LOG_UPTO(LOG_WARNING));
	sp = getservbyname("router", "udp");
	if (sp == NULL) {
		fprintf(stderr, "routed: router/udp: unknown service\n");
		exit(1);
	}
	addr.sin_family = AF_INET;
	addr.sin_port = sp->s_port;
	s = getsocket(AF_INET, SOCK_DGRAM, &addr);
	if (s < 0)
		exit(1);
	argv++, argc--;
	while (argc > 0 && **argv == '-') {
		if (strcmp(*argv, "-s") == 0) {
			supplier = 1;
			argv++, argc--;
			continue;
		}
		if (strcmp(*argv, "-q") == 0) {
			supplier = 0;
			argv++, argc--;
			continue;
		}
		if (strcmp(*argv, "-t") == 0) {
			tracepackets++;
			setlogmask(LOG_UPTO(LOG_DEBUG));
			argv++, argc--;
			continue;
		}
		if (strcmp(*argv, "-d") == 0) {
			setlogmask(LOG_UPTO(LOG_DEBUG));
			argv++, argc--;
			continue;
		}
		if (strcmp(*argv, "-g") == 0) {
			gateway = 1;
			argv++, argc--;
			continue;
		}
		fprintf(stderr,
			"usage: routed [ -s ] [ -q ] [ -t ] [ -g ]\n");
		exit(1);
	}
#ifndef DEBUG
	if (!tracepackets) {
		int t;

		if (fork())
			exit(0);
		for (t = 0; t < 20; t++)
			if (t != s)
				(void) close(t);
		(void) open("/", 0);
		(void) dup2(0, 1);
		(void) dup2(0, 2);
		t = open("/dev/tty", 2);
		if (t >= 0) {
			ioctl(t, TIOCNOTTY, (char *)0);
			(void) close(t);
		}
	}
#endif
	/*
	 * Any extra argument is considered
	 * a tracing log file.
	 */
	if (argc > 0)
		traceon(*argv);
	/*
	 * Collect an initial view of the world by
	 * checking the interface configuration and the gateway kludge
	 * file.  Then, send a request packet on all
	 * directly connected networks to find out what
	 * everyone else thinks.
	 */
	rtinit();
	gwkludge();
	ifinit();
	if (gateway > 0)
		rtdefault();
	if (supplier < 0)
		supplier = 0;
	msg->rip_cmd = RIPCMD_REQUEST;
	msg->rip_vers = RIPVERSION;
	msg->rip_nets[0].rip_dst.sa_family = AF_UNSPEC;
	msg->rip_nets[0].rip_metric = HOPCNT_INFINITY;
	msg->rip_nets[0].rip_dst.sa_family = htons(AF_UNSPEC);
	msg->rip_nets[0].rip_metric = htonl(HOPCNT_INFINITY);
	toall(sendmsg);
	signal(SIGALRM, timer);
	signal(SIGHUP, hup);
	signal(SIGTERM, hup);
	timer();

	for (;;) {
		int ibits;
		register int n;
#ifdef sgi
		register long ntime;
		/* we tend to loose timer interrupts, so compensate */
		ntime = time((long*)0);
		if (ntime > time_ck + TIMER_RATE*2) {
			syslog(LOG_ERR,
			       "lost timer: alarm()=%d signal()=%x",
			       alarm(0), signal(SIGALRM,timer));
			timer();
		}
#endif
		ibits = 1 << s;
		n = select(20, &ibits, 0, 0, 0);
		if (n < 0)
			continue;
		if (ibits & (1 << s))
			process(s);
		/* handle ICMP redirects */
	}
}

#ifdef sgi
/* block timer signals for a while
 */
extern int had_alarm, block_alarms;
#endif

process(fd)
	int fd;
{
	struct sockaddr from;
	int fromlen = sizeof (from), cc, omask;

	cc = recvfrom(fd, packet, sizeof (packet), 0, &from, &fromlen);
	if (cc <= 0) {
		if (cc < 0 && errno != EINTR)
			perror("recvfrom");
		return;
	}
	if (fromlen != sizeof (struct sockaddr_in))
		return;
#ifdef sgi
	block_alarms = 1;
#else
	omask = sigblock(sigmask(SIGALRM));
#endif
	rip_input(&from, cc);
#ifdef sgi
	block_alarms = 0;
	if (had_alarm) timer();
#else
	sigsetmask(omask);
#endif
}

getsocket(domain, type, sin)
	int domain, type;
	struct sockaddr_in *sin;
{
	int s, on = 1;

	if ((s = socket(domain, type, 0)) < 0) {
		perror("socket");
		syslog(LOG_ERR, "socket: %m");
		return (-1);
	}
	if (setsockopt(s, SOL_SOCKET, SO_BROADCAST, &on, sizeof (on)) < 0) {
		syslog(LOG_ERR, "setsockopt SO_BROADCAST: %m");
		close(s);
		return (-1);
	}
	if (bind(s, sin, sizeof (*sin), 0) < 0) {
		perror("bind");
		syslog(LOG_ERR, "bind: %m");
		close(s);
		return (-1);
	}
	return (s);
}
