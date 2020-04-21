#ifndef lint
static char sccsid[] = "@(#)svc_raw.c 1.1 85/05/30 Copyr 1984 Sun Micro";
#endif

/*
 * svc_raw.c,   This a toy for simple testing and timing.
 * Interface to create an rpc client and server in the same UNIX process.
 * This lets us similate rpc and get rpc (round trip) overhead, without
 * any interference from the kernal.
 *
 * Copyright (C) 1984, Sun Microsystems, Inc.
 */

#include "types.h"
#include <netinet/in.h>
#include "xdr.h"
#include "auth.h"
#include "clnt.h"
#include "rpc_msg.h"
#include "svc.h"

#define NULL ((caddr_t)0)

/*
 * This is the "network" that we will be moving data over
 */
extern char _raw_buf[UDPMSGSIZE];

static bool_t		svcraw_recv();
static enum xprt_stat 	svcraw_stat();
static bool_t		svcraw_getargs();
static bool_t		svcraw_reply();
static bool_t		svcraw_freeargs();
static void		svcraw_destroy();

static struct xp_ops server_ops = {
	svcraw_recv,
	svcraw_stat,
	svcraw_getargs,
	svcraw_reply,
	svcraw_freeargs,
	svcraw_destroy
};

static SVCXPRT server;
static XDR xdr_stream;
static char verf_body[MAX_AUTH_BYTES];

SVCXPRT *
svcraw_create()
{

	server.xp_sock = 0;
	server.xp_port = 0;
	server.xp_ops = &server_ops;
	server.xp_verf.oa_base = verf_body;
	xdrmem_create(&xdr_stream, _raw_buf, UDPMSGSIZE, XDR_FREE);
	return (&server);
}

static enum xprt_stat
svcraw_stat()
{

	return (XPRT_IDLE);
}

static bool_t
svcraw_recv(xprt, msg)
	SVCXPRT *xprt;
	struct rpc_msg *msg;
{
	register XDR *xdrs = &xdr_stream;

	xdrs->x_op = XDR_DECODE;
	XDR_SETPOS(xdrs, 0);
	if (! xdr_callmsg(xdrs, msg))
	       return (FALSE);
	return (TRUE);
}

static bool_t
svcraw_reply(xprt, msg)
	SVCXPRT *xprt;
	struct rpc_msg *msg;
{
	register XDR *xdrs = &xdr_stream;

	xdrs->x_op = XDR_ENCODE;
	XDR_SETPOS(xdrs, 0);
	if (! xdr_replymsg(xdrs, msg))
	       return (FALSE);
	(void)XDR_GETPOS(xdrs);  /* called just for overhead */
	return (TRUE);
}

static bool_t
svcraw_getargs(xprt, xdr_args, args_ptr)
	SVCXPRT *xprt;
	xdrproc_t xdr_args;
	caddr_t args_ptr;
{

	return ((*xdr_args)(&xdr_stream, args_ptr));
}

static bool_t
svcraw_freeargs(xprt, xdr_args, args_ptr)
	SVCXPRT *xprt;
	xdrproc_t xdr_args;
	caddr_t args_ptr;
{ 
	register XDR *xdrs = &xdr_stream;

	xdrs->x_op = XDR_FREE;
	return ((*xdr_args)(xdrs, args_ptr));
} 

static void
svcraw_destroy()
{
}
