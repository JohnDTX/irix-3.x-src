#ifndef lint
/* @(#)rpc.rwalld.c	2.1 86/04/17 NFSSRC */ 
static  char sccsid[] = "@(#)rpc.rwalld.c 1.1 86/02/05 Copyr 1984 Sun Micro";
#endif

/*
 * Copyright (c) 1984 by Sun Microsystems, Inc.
 */

#include <rpcsvc/rwall.h>
#include <rpc/rpc.h>
#include <stdio.h>
#include <netdb.h>
#include <sys/socket.h>

int splat();

main()
{
	register SVCXPRT *transp;
	struct sockaddr_in addr;
	int len = sizeof(struct sockaddr_in);
	
	if (getsockname(0, &addr, &len) != 0) {
		perror("rstat: getsockname");
		exit(1);
	}
	if ((transp = svcudp_create(0)) == NULL) {
		fprintf(stderr, "svc_rpc_udp_create: error\n");
		exit(1);
	}
	if (!svc_register(transp, WALLPROG, WALLVERS, splat, 0)) {
		fprintf(stderr, "svc_rpc_register: error\n");
		exit(1);
	}
	svc_run();
	fprintf(stderr, "Error: svc_run shouldn't have returned\n");
}

splat(rqstp, transp)
	register struct svc_req *rqstp;
	register SVCXPRT *transp;
{
	FILE *fp, *popen();
	char *msg;
        struct sockaddr_in addr;	
	struct hostent *hp;
	char buf[256];

	switch (rqstp->rq_proc) {
		case 0:
			if (svc_sendreply(transp, xdr_void, 0)  == FALSE) {
				fprintf(stderr, "err: rusersd");
				exit(1);
			    }
			exit(0);
		case WALLPROC_WALL:
			if (!svc_getargs(transp, xdr_wrapstring, &msg)) {
			    	svcerr_decode(transp);
				exit(1);
			}
			if (svc_sendreply(transp, xdr_void, 0)  == FALSE) {
				fprintf(stderr, "err: rusersd");
				exit(1);
			}
			if (fork() == 0) {/* fork off child to do it */
#ifdef sgi
				fp = popen("/etc/wall", "w");
#else
				fp = popen("/bin/wall", "w");
#endif
				fprintf(fp, "%s", msg);
				pclose(fp);
				exit(0);
			}
			exit(0);
		default: 
			svcerr_noproc(transp);
			exit(0);
	}
}
