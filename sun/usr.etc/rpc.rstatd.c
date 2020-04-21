#ifndef lint
/* @(#)rpc.rstatd.c	2.2 86/05/15 NFSSRC */ 
static  char sccsid[] = "@(#)rpc.rstatd.c 1.1 86/02/05 Copyr 1984 Sun Micro";
#endif

/*
 * Copyright (c) 1984 by Sun Microsystems, Inc.
 */

/* 
 * rstat demon:  called from inet
 *
 */

#ifdef sgi
#define	vax	1
#endif
#include <signal.h>
#include <stdio.h>
#include <rpc/rpc.h>
#include <sys/socket.h>
#include <nlist.h>
#include <sys/dk.h>
#include <sys/errno.h>
#include <sys/vmmeter.h>
#include <net/if.h>
#include <sys/time.h>
#include <rpcsvc/rstat.h>

struct nlist nl[] = {
#define	X_CPTIME	0
	{ "_cp_time" },
#define	X_SUM		1
	{ "_sum" },
#define	X_IFNET		2
	{ "_ifnet" },
#define	X_DKXFER	3
	{ "_dk_xfer" },
#define	X_BOOTTIME	4
	{ "_boottime" },
#define	X_AVENRUN	5
	{ "_avenrun" },
#define X_HZ		6
	{ "_hz" },
	"",
};
int kmem;
int firstifnet, numintfs;	/* chain of ethernet interfaces */
int stats_service();

int sincelastreq = 0;		/* number of alarms since last request */
#define CLOSEDOWN 20		/* how long to wait before exiting */

union {
    struct stats s1;
    struct statsswtch s2;
    struct statstime s3;
} stats;

int updatestat();
extern int errno;

#ifndef FSCALE
#define FSCALE (1 << 8)
#endif

main(argc, argv)
	char **argv;
{
	SVCXPRT *transp;
	struct sockaddr_in addr;
	int len = sizeof(struct sockaddr_in);
	int readfds, port, readfdstmp;


#ifdef DEBUG
	{
	int s;
	struct sockaddr_in addr;
	int len = sizeof(struct sockaddr_in);
	
	if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
		perror("inet: socket");
		return -1;
	}
	if (bind(s, &addr, sizeof(addr)) < 0) {
		perror("bind");
		return -1;
	}
	if (getsockname(s, &addr, &len) != 0) {
		perror("inet: getsockname");
		(void)close(s);
		return -1;
	}
	pmap_unset(RSTATPROG, RSTATVERS_ORIG);
	pmap_set(RSTATPROG, RSTATVERS_ORIG, IPPROTO_UDP,ntohs(addr.sin_port));
	pmap_unset(RSTATPROG, RSTATVERS_SWTCH);
	pmap_set(RSTATPROG, RSTATVERS_SWTCH,IPPROTO_UDP,ntohs(addr.sin_port));
#ifndef sgi
	pmap_unset(RSTATPROG, RSTATVERS_TIME);
	pmap_set(RSTATPROG, RSTATVERS_TIME,IPPROTO_UDP,ntohs(addr.sin_port));
#endif
	if (dup2(s, 0) < 0) {
		perror("dup2");
		exit(1);
	}
	}
#endif	
	if (getsockname(0, &addr, &len) != 0) {
		perror("rstat: getsockname");
		exit(1);
	}
	if ((transp = svcudp_bufcreate(0, RPCSMALLMSGSIZE, RPCSMALLMSGSIZE))
	    == NULL) {
		fprintf("svc_rpc_udp_create: error\n");
		exit(1);
	}
	if (!svc_register(transp,RSTATPROG,RSTATVERS_ORIG,stats_service, 0)) {
		fprintf(stderr, "svc_rpc_register: error\n");
		exit(1);
	}
	if (!svc_register(transp,RSTATPROG,RSTATVERS_SWTCH,stats_service,0)) {
		fprintf(stderr, "svc_rpc_register: error\n");
		exit(1);
	}
#ifndef sgi
	if (!svc_register(transp,RSTATPROG,RSTATVERS_TIME,stats_service,0)) {
		fprintf(stderr, "svc_rpc_register: error\n");
		exit(1);
	}
#endif
	setup();
	updatestat();
	alarm(1);
	signal(SIGALRM, updatestat);
	svc_run();
	fprintf(stderr, "svc_run should never return\n");
}

static int
stats_service(reqp, transp)
	 struct svc_req  *reqp;
	 SVCXPRT  *transp;
{
	int have;
	
#ifdef DEBUG
	fprintf(stderr, "entering stats_service\n");
#endif
	switch (reqp->rq_proc) {
		case RSTATPROC_STATS:
			sincelastreq = 0;
			if (reqp->rq_vers == RSTATVERS_ORIG) {
				if (svc_sendreply(transp, xdr_stats,
				    &stats.s1, TRUE) == FALSE) {
					fprintf(stderr,
					    "err: svc_rpc_send_results");
					exit(1);
				}
				return;
			}
			if (reqp->rq_vers == RSTATVERS_SWTCH) {
				if (svc_sendreply(transp, xdr_statsswtch,
				    &stats.s2, TRUE) == FALSE) {
					fprintf(stderr,
					    "err: svc_rpc_send_results");
					exit(1);
				    }
				return;
			}
#ifndef sgi
			if (reqp->rq_vers == RSTATVERS_TIME) {
				if (svc_sendreply(transp, xdr_statstime,
				    &stats.s3, TRUE) == FALSE) {
					fprintf(stderr,
					    "err: svc_rpc_send_results");
					exit(1);
				    }
				return;
			}
#endif
		case RSTATPROC_HAVEDISK:
			have = havedisk();
			if (svc_sendreply(transp,xdr_long, &have, TRUE) == 0){
			    fprintf(stderr, "err: svc_sendreply");
			    exit(1);
			}
			return;
		case 0:
			if (svc_sendreply(transp, xdr_void, 0, TRUE)
			    == FALSE) {
				fprintf(stderr, "err: svc_rpc_send_results");
				exit(1);
			    }
			return;
		default: 
			svcerr_noproc(transp);
			return;
		}
}

updatestat()
{
	int off, i, hz;
	struct vmmeter sum;
	struct ifnet ifnet;
	double avrun[3];
	struct timeval tm, btm;
	
#ifdef DEBUG
	fprintf(stderr, "entering updatestat\n");
#endif
	if (sincelastreq >= CLOSEDOWN) {
#ifdef DEBUG
	fprintf(stderr, "about to closedown\n");
#endif
		exit(0);
	}
	sincelastreq++;
	if (lseek(kmem, (long)nl[X_HZ].n_value, 0) == -1) {
		fprintf(stderr, "can't seek in kmem\n");
		exit(1);
	}
	if (read(kmem, &hz, sizeof hz) != sizeof hz) {
		fprintf(stderr, "can't read hz from kmem\n");
		exit(1);
	}
	if (lseek(kmem, (long)nl[X_CPTIME].n_value, 0) == -1) {
		fprintf(stderr, "can't seek in kmem\n");
		exit(1);
	}
 	if (read(kmem, stats.s1.cp_time, sizeof (stats.s1.cp_time))
	    != sizeof (stats.s1.cp_time)) {
		fprintf(stderr, "can't read cp_time from kmem\n");
		exit(1);
	}
	if (lseek(kmem, (long)nl[X_AVENRUN].n_value, 0) ==-1) {
		fprintf(stderr, "can't seek in kmem\n");
		exit(1);
	}
#ifdef vax
 	if (read(kmem, avrun, sizeof (avrun)) != sizeof (avrun)) {
		fprintf(stderr, "can't read avenrun from kmem\n");
		exit(1);
	}
	stats.s2.avenrun[0] = avrun[0] * FSCALE;
	stats.s2.avenrun[1] = avrun[1] * FSCALE;
	stats.s2.avenrun[2] = avrun[2] * FSCALE;
#else vax
#ifdef sun
 	if (read(kmem, stats.s2.avenrun, sizeof (stats.s2.avenrun))
	    != sizeof (stats.s2.avenrun)) {
		fprintf(stderr, "can't read avenrun from kmem\n");
		exit(1);
	}
#else sun
	put machine dependent code here
#endif sun
#endif vax

	if (lseek(kmem, (long)nl[X_BOOTTIME].n_value, 0) == -1) {
		fprintf(stderr, "can't seek in kmem\n");
		exit(1);
	}
 	if (read(kmem, &btm, sizeof (stats.s2.boottime))
	    != sizeof (stats.s2.boottime)) {
		fprintf(stderr, "can't read boottime from kmem\n");
		exit(1);
	}
	stats.s2.boottime = btm;


#ifdef DEBUG
	fprintf(stderr, "%d %d %d %d\n", stats.s1.cp_time[0],
	    stats.s1.cp_time[1], stats.s1.cp_time[2], stats.s1.cp_time[3]);
#endif

	if (lseek(kmem, (long)nl[X_SUM].n_value, 0) ==-1) {
		fprintf(stderr, "can't seek in kmem\n");
		exit(1);
	}
 	if (read(kmem, &sum, sizeof sum) != sizeof sum) {
		fprintf(stderr, "can't read sum from kmem\n");
		exit(1);
	}
	stats.s1.v_pgpgin = sum.v_pgpgin;
	stats.s1.v_pgpgout = sum.v_pgpgout;
	stats.s1.v_pswpin = sum.v_pswpin;
	stats.s1.v_pswpout = sum.v_pswpout;
	stats.s1.v_intr = sum.v_intr;
	gettimeofday(&tm);
	stats.s1.v_intr -= hz*(tm.tv_sec - btm.tv_sec) +
	    hz*(tm.tv_usec - btm.tv_usec)/1000000;
	stats.s2.v_swtch = sum.v_swtch;

	if (lseek(kmem, (long)nl[X_DKXFER].n_value, 0) == -1) {
		fprintf(stderr, "can't seek in kmem\n");
		exit(1);
	}
 	if (read(kmem, stats.s1.dk_xfer, sizeof (stats.s1.dk_xfer))
	    != sizeof (stats.s1.dk_xfer)) {
		fprintf(stderr, "can't read dk_xfer from kmem\n");
		exit(1);
	}

	stats.s1.if_ipackets = 0;
	stats.s1.if_opackets = 0;
	stats.s1.if_ierrors = 0;
	stats.s1.if_oerrors = 0;
	stats.s1.if_collisions = 0;
	for (off = firstifnet, i = 0; off && i < numintfs; i++) {
		if (lseek(kmem, off, 0) == -1) {
			fprintf(stderr, "can't seek in kmem\n");
			exit(1);
		}
		if (read(kmem, &ifnet, sizeof ifnet) != sizeof ifnet) {
			fprintf(stderr, "can't read ifnet from kmem\n");
			exit(1);
		}
		stats.s1.if_ipackets += ifnet.if_ipackets;
		stats.s1.if_opackets += ifnet.if_opackets;
		stats.s1.if_ierrors += ifnet.if_ierrors;
		stats.s1.if_oerrors += ifnet.if_oerrors;
		stats.s1.if_collisions += ifnet.if_collisions;
		off = (int) ifnet.if_next;
	}
	gettimeofday(&stats.s3.curtime, 0);
	alarm(1);
}

static 
setup()
{
	struct ifnet ifnet;
	int off, *ip;
	
	nlist("/vmunix", nl);
	if (nl[0].n_value == 0) {
		fprintf (stderr, "Variables missing from namelist\n");
		exit (1);
	}
	if ((kmem = open("/dev/kmem", 0)) < 0) {
		fprintf(stderr, "can't open kmem\n");
		exit(1);
	}

	off = nl[X_IFNET].n_value;
	if (lseek(kmem, off, 0) == -1) {
		fprintf(stderr, "can't seek in kmem\n");
		exit(1);
	}
	if (read(kmem, &firstifnet, sizeof(int)) != sizeof (int)) {
		fprintf(stderr, "can't read firstifnet from kmem\n");
		exit(1);
	}
	numintfs = 0;
	for (off = firstifnet; off;) {
		if (lseek(kmem, off, 0) == -1) {
			fprintf(stderr, "can't seek in kmem\n");
			exit(1);
		}
		if (read(kmem, &ifnet, sizeof ifnet) != sizeof ifnet) {
			fprintf(stderr, "can't read ifnet from kmem\n");
			exit(1);
		}
		numintfs++;
		off = (int) ifnet.if_next;
	}
}

/* 
 * returns true if have a disk
 */
static
havedisk()
{
	int i, cnt;
	long  xfer[DK_NDRIVE];

	nlist("/vmunix", nl);
	if (nl[X_DKXFER].n_value == 0) {
		fprintf (stderr, "Variables missing from namelist\n");
		exit (1);
	}
	if ((kmem = open("/dev/kmem", 0)) < 0) {
		fprintf(stderr, "can't open kmem\n");
		exit(1);
	}
	if (lseek(kmem, (long)nl[X_DKXFER].n_value, 0) == -1) {
		fprintf(stderr, "can't seek in kmem\n");
		exit(1);
	}
	if (read(kmem, xfer, sizeof xfer)!= sizeof xfer) {
		fprintf(stderr, "can't read kmem\n");
		exit(1);
	}
	cnt = 0;
	for (i=0; i < DK_NDRIVE; i++)
		cnt += xfer[i];
	return (cnt != 0);
}
