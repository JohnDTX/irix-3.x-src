#ifndef lint
static char sccsid[] = "@(#)clnt_simple.c 1.1 85/05/30 Copyr 1984 Sun Micro";
#endif

/* 
 * clnt_simple.c
 * Simplified front end to rpc.
 *
 * Copyright (C) 1984, Sun Microsystems, Inc.
 */

#include <stdio.h>
#include <rpc/rpc.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netdb.h>

callrpc(host, prognum, versnum, procnum, inproc, in, outproc, out)
	char *host;
	xdrproc_t inproc, outproc;
	char *in, *out;
{
	struct sockaddr_in server_addr;
	enum clnt_stat clnt_stat;
	struct hostent *hp;
	struct timeval timeout, tottimeout;

	static CLIENT *client;
	static int socket = RPC_ANYSOCK;
	static int oldprognum, oldversnum, valid;
	static char oldhost[256];

	if (valid && oldprognum == prognum && oldversnum == versnum
		&& strcmp(oldhost, host) == 0) {
		/* reuse old client */		
	}
	else {
		close(socket);
		socket = RPC_ANYSOCK;
		if (client) {
			clnt_destroy(client);
			client = NULL;
		}
		if ((hp = gethostbyname(host)) == NULL)
			return ((int) RPC_UNKNOWNHOST);
		timeout.tv_usec = 0;
		timeout.tv_sec = 10;
		bcopy(hp->h_addr, &server_addr.sin_addr, hp->h_length);
		server_addr.sin_family = AF_INET;
		server_addr.sin_port =  0;
		if ((client = clntudp_create(&server_addr, prognum,
		    versnum, timeout, &socket)) == NULL)
			return ((int) rpc_createerr.cf_stat);
		valid = 1;
		oldprognum = prognum;
		oldversnum = versnum;
		strcpy(oldhost, host);
	}
	tottimeout.tv_sec = 25;
	tottimeout.tv_usec = 0;
	clnt_stat = clnt_call(client, procnum, inproc, in,
	    outproc, out, tottimeout);
	/* 
	 * if call failed, empty cache
	 */
	if (clnt_stat != RPC_SUCCESS)
		valid = 0;
	return ((int) clnt_stat);
}
