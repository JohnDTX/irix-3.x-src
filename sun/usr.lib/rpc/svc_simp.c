/* NFSSRC @(#)svc_simple.c	2.1 86/04/14 */
#ifndef lint
static char sccsid[] = "@(#)svc_simple.c 1.1 86/02/03 Copyr 1984 Sun Micro";
#endif

/* 
 * svc_simple.c
 * Simplified front end to rpc.
 *
 * Copyright (C) 1984, Sun Microsystems, Inc.
 */

#include <stdio.h>
#include <rpc/rpc.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netdb.h>

static struct proglst {
	char *(*p_progname)();
	int  p_prognum;
	int  p_procnum;
	xdrproc_t p_inproc, p_outproc;
	struct proglst *p_nxt;
} *proglst;
int universal();
static SVCXPRT *transp;
static madetransp;
struct proglst *pl;

registerrpc(prognum, versnum, procnum, progname, inproc, outproc)
	char *(*progname)();
	xdrproc_t inproc, outproc;
{
	
	if (procnum == NULLPROC) {
		fprintf(stderr,
		    "can't reassign procedure number %d\n", NULLPROC);
		return (-1);
	}
	if (!madetransp) {
		madetransp = 1;
		transp = svcudp_create(RPC_ANYSOCK);
		if (transp == NULL) {
			fprintf(stderr, "couldn't create an rpc server\n");
			return (-1);
		}
	}
	pmap_unset(prognum, versnum);
	if (!svc_register(transp, prognum, versnum, universal, IPPROTO_UDP)) {
	    	fprintf(stderr, "couldn't register prog %d vers %d\n",
		    prognum, versnum);
		return (-1);
	}
	pl = (struct proglst *)malloc(sizeof(struct proglst));
	if (pl == NULL) {
		fprintf(stderr, "registerrpc: out of memory\n");
		return (-1);
	}
	pl->p_progname = progname;
	pl->p_prognum = prognum;
	pl->p_procnum = procnum;
	pl->p_inproc = inproc;
	pl->p_outproc = outproc;
	pl->p_nxt = proglst;
	proglst = pl;
	return (0);
}

static
universal(rqstp, transp)
	struct svc_req *rqstp;
	SVCXPRT *transp;
{
	int prog, proc, i;
	char *outdata;
	char xdrbuf[UDPMSGSIZE];
	struct proglst *pl;

	/* 
	 * enforce "procnum 0 is echo" convention
	 */
	if (rqstp->rq_proc == NULLPROC) {
		if (svc_sendreply(transp, xdr_void, 0) == FALSE) {
			fprintf(stderr, "xxx\n");
			exit(1);
		}
		return;
	}
	prog = rqstp->rq_prog;
	proc = rqstp->rq_proc;
	for (pl = proglst; pl != NULL; pl = pl->p_nxt)
		if (pl->p_prognum == prog && pl->p_procnum == proc) {
			/* decode arguments into a CLEAN buffer */
			bzero(xdrbuf, sizeof(xdrbuf)); /* required ! */
			if (!svc_getargs(transp, pl->p_inproc, xdrbuf)) {
				svcerr_decode(transp);
				return;
			}
			outdata = (*(pl->p_progname))(xdrbuf);
			if (outdata == NULL && pl->p_outproc != xdr_void)
				/* there was an error */
				return;
			if (!svc_sendreply(transp, pl->p_outproc, outdata)) {
				fprintf(stderr,
				    "trouble replying to prog %d\n",
				    pl->p_prognum);
				exit(1);
			/* free the decoded arguments */
			(void)svc_freeargs(transp, pl->p_inproc, xdrbuf);
			}
			return;
		}
	fprintf(stderr, "never registered prog %d\n", prog);
	exit(1);
}

