/* NFSSRC @(#)auth_none.c	2.1 86/04/14 */
#ifndef lint
static char sccsid[] = "@(#)auth_none.c 1.1 86/02/03 Copyr 1984 Sun Micro";
#endif

/*
 * auth_none.c
 * Creates a client authentication handle for passing "null" 
 * credentials and verifiers to remote systems. 
 * 
 * Copyright (C) 1984, Sun Microsystems, Inc. 
 */

#include "types.h"	/* <> */
#include "xdr.h"	/* <> */
#include "auth.h"	/* <> */
#define NULL ((caddr_t)0)
#define MAX_MARSHEL_SIZE 20

/*
 * Authenticator operations routines
 */
static void	authnone_verf();
static void	authnone_destroy();
static bool_t	authnone_marshal();
static bool_t	authnone_validate();
static bool_t	authnone_refresh();

static struct auth_ops ops = {
	authnone_verf,
	authnone_marshal,
	authnone_validate,
	authnone_refresh,
	authnone_destroy
};

static AUTH	no_client;
static char	marshalled_client[MAX_MARSHEL_SIZE];
static u_int	mcnt;

AUTH *
authnone_create()
{
	XDR xdr_stream;
	register XDR *xdrs;

	if (! mcnt) {
		no_client.ah_cred = no_client.ah_verf = _null_auth;
		no_client.ah_ops = &ops;
		xdrs = &xdr_stream;
		xdrmem_create(xdrs, marshalled_client, (u_int)MAX_MARSHEL_SIZE,
		    XDR_ENCODE);
		(void)xdr_opaque_auth(xdrs, &no_client.ah_cred);
		(void)xdr_opaque_auth(xdrs, &no_client.ah_verf);
		mcnt = XDR_GETPOS(xdrs);
		XDR_DESTROY(xdrs);
	}
	return (&no_client);
}

/*ARGSUSED*/
static bool_t
authnone_marshal(client, xdrs)
	AUTH *client;
	XDR *xdrs;
{

	return ((*xdrs->x_ops->x_putbytes)(xdrs, marshalled_client, mcnt));
}

static void 
authnone_verf()
{
}

static bool_t
authnone_validate()
{

	return (TRUE);
}

static bool_t
authnone_refresh()
{

	return (FALSE);
}

static void
authnone_destroy()
{
}
