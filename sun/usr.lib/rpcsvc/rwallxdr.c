#ifndef lint
/* @(#)rwallxdr.c	2.1 86/04/14 NFSSRC */
static  char sccsid[] = "@(#)rwallxdr.c 1.1 86/02/05 Copyr 1985 Sun Micro";
#endif

/* 
 * Copyright (c) 1985 by Sun Microsystems, Inc.
 */

#include <stdio.h>
#include <rpc/rpc.h>
#include <rpcsvc/rwall.h>

rwall(host, msg)
	char *host;
	char *msg;
{
	return (callrpc(host, WALLPROG, WALLVERS, WALLPROC_WALL,
	    xdr_wrapstring, &msg,  xdr_void, NULL));
}
