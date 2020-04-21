/* NFSSRC @(#)clnt_raw.c	2.1 86/04/14 */
#ifndef lint
static char sccsid[] = "@(#)clnt_raw.c 1.1 86/02/03 Copyr 1984 Sun Micro";
#endif

/*
 * clnt_raw.c
 *
 * Copyright (C) 1984, Sun Microsystems, Inc.
 *
 * Memory based rpc for simple testing and timing.
 * Interface to create an rpc client and server in the same process.
 * This lets us similate rpc and get round trip overhead, without
 * any interference from the kernal.
 */

#include "types.h"
#include <sys/time.h>
#include <netinet/in.h>
#include "xdr.h"
#include "auth.h"
#include "clnt.h"
#include "rpc_msg.h"

#define NULL ((caddr_t)0)
#define MCALL_MSG_SIZE 24

/*
 * This is the "network" we will be moving stuff over.
 */
char _raw_buf[UDPMSGSIZE];

static char	mashl_callmsg[MCALL_MSG_SIZE];
static u_int	mcnt;

static enum clnt_stat	clntraw_call();
static void		clntraw_abort();
static void		clntraw_geterr();
static bool_t		clntraw_freeres();
static void		clntraw_destroy();

static struct clnt_ops client_ops = {
	clntraw_call,
	clntraw_abort,
	clntraw_geterr,
	clntraw_freeres,
	clntraw_destroy
};

static CLIENT	client_object;
static XDR	xdr_stream;

void	svc_getreq();

/*
 * Create a client handle for memory based rpc.
 */
CLIENT *
clntraw_create(prog, vers)
	u_long prog;
	u_long vers;
{
	struct rpc_msg call_msg;
	XDR *xdrs = &xdr_stream;
	CLIENT	*client = &client_object;

	/*
	 * pre-serialize the staic part of the call msg and stash it away
	 */
	call_msg.rm_direction = CALL;
	call_msg.rm_call.cb_rpcvers = RPC_MSG_VERSION;
	call_msg.rm_call.cb_prog = prog;
	call_msg.rm_call.cb_vers = vers;
	xdrmem_create(xdrs, mashl_callmsg, MCALL_MSG_SIZE, XDR_ENCODE); 
	if (! xdr_callhdr(xdrs, &call_msg)) {
		perror("clnt_raw.c - Fatal header serialization error.");
	}
	mcnt = XDR_GETPOS(xdrs);
	XDR_DESTROY(xdrs);

	/*
	 * Set xdrmem for client/server shared buffer
	 */
	xdrmem_create(xdrs, _raw_buf, UDPMSGSIZE, XDR_FREE);

	/*
	 * create client handle
	 */
	client->cl_ops = &client_ops;
	client->cl_auth = authnone_create();
	return (client);
}

static enum clnt_stat 
clntraw_call(h, proc, xargs, argsp, xresults, resultsp, timeout)
	CLIENT *h;
	u_long proc;
	xdrproc_t xargs;
	caddr_t argsp;
	xdrproc_t xresults;
	caddr_t resultsp;
	struct timeval timeout;
{
	register XDR *xdrs = &xdr_stream;
	struct rpc_msg msg;
	enum clnt_stat status;
	struct rpc_err error;

call_again:
	/*
	 * send request
	 */
	xdrs->x_op = XDR_ENCODE;
	XDR_SETPOS(xdrs, 0);
	((struct rpc_msg *)mashl_callmsg)->rm_xid ++ ;
	if ((! XDR_PUTBYTES(xdrs, mashl_callmsg, mcnt)) ||
	    (! XDR_PUTLONG(xdrs, (long *)&proc)) ||
	    (! AUTH_MARSHALL(h->cl_auth, xdrs)) ||
	    (! (*xargs)(xdrs, argsp))) {
		return (RPC_CANTENCODEARGS);
	}
	(void)XDR_GETPOS(xdrs);  /* called just to cause overhead */

	/*
	 * We have to call server input routine here because this is
	 * all going on in one process. Yuk.
	 */
	svc_getreq(1);

	/*
	 * get results
	 */
	xdrs->x_op = XDR_DECODE;
	XDR_SETPOS(xdrs, 0);
	msg.acpted_rply.ar_verf = _null_auth;
	msg.acpted_rply.ar_results.where = resultsp;
	msg.acpted_rply.ar_results.proc = xresults;
	if (! xdr_replymsg(xdrs, &msg))
		return (RPC_CANTDECODERES);
	_seterr_reply(&msg, &error);
	status = error.re_status;

	if (status == RPC_SUCCESS) {
		if (! AUTH_VALIDATE(h->cl_auth, &msg.acpted_rply.ar_verf)) {
			status = RPC_AUTHERROR;
		}
	}  /* end successful completion */
	else {
		if (AUTH_REFRESH(h->cl_auth))
			goto call_again;
	}  /* end of unsuccessful completion */

	if (status == RPC_SUCCESS) {
		if (! AUTH_VALIDATE(h->cl_auth, &msg.acpted_rply.ar_verf)) {
			status = RPC_AUTHERROR;
		}
		if (msg.acpted_rply.ar_verf.oa_base != NULL) {
			xdrs->x_op = XDR_FREE;
			(void)xdr_opaque_auth(xdrs, &(msg.acpted_rply.ar_verf));
		}
	}

	return (status);
}

static void
clntraw_geterr()
{
}


static bool_t
clntraw_freeres(cl, xdr_res, res_ptr)
	CLIENT *cl;
	xdrproc_t xdr_res;
	caddr_t res_ptr;
{
	register XDR *xdrs = &xdr_stream;

	xdrs->x_op = XDR_FREE;
	return ((*xdr_res)(xdrs, res_ptr));
}

static void
clntraw_abort()
{
}

static void
clntraw_destroy()
{
}
