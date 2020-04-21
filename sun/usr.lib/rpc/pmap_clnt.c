/* NFSSRC @(#)pmap_clnt.c	2.2 86/04/14 */
#ifndef lint
static char sccsid[] = "@(#)pmap_clnt.c 1.1 86/02/03 Copyr 1984 Sun Micro";
#endif

/*
 * pmap_clnt.c
 * Client interface to pmap rpc service.
 *
 * Copyright (C) 1984, Sun Microsystems, Inc.
 */

#include "types.h"
#include <netinet/in.h>
#include "xdr.h"
#include "auth.h"
#include "clnt.h"
#include "rpc_msg.h"
#include "pmap_prot.h" 
#include "pmap_clnt.h"
#include <sys/socket.h>
#include <sys/time.h>
#include <stdio.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#define NAMELEN 255

static struct timeval timeout = { 5, 0 };
static struct timeval tottimeout = { 60, 0 };
static struct sockaddr_in myaddress;

void clnt_perror();


/*
 * Set a mapping between program,version and port.
 * Calls the pmap service remotely to do the mapping.
 */
bool_t
pmap_set(program, version, protocol, port)
	u_long program;
	u_long version;
	u_long protocol;
	u_short port;
{
	struct sockaddr_in myaddress;
	int socket = -1;
	register CLIENT *client;
	struct pmap parms;
	bool_t rslt;

	get_myaddress(&myaddress);
	client = clntudp_bufcreate(&myaddress, PMAPPROG, PMAPVERS,
	    timeout, &socket, RPCSMALLMSGSIZE, RPCSMALLMSGSIZE);
	if (client == (CLIENT *)NULL)
		return (FALSE);
	parms.pm_prog = program;
	parms.pm_vers = version;
	parms.pm_prot = protocol;
	parms.pm_port = port;
	if (CLNT_CALL(client, PMAPPROC_SET, xdr_pmap, &parms, xdr_bool, &rslt,
	    tottimeout) != RPC_SUCCESS) {
		clnt_perror(client, "Cannot register service");
		return (FALSE);
	}
	CLNT_DESTROY(client);
	(void)close(socket);
	return (rslt);
}

/*
 * Remove the mapping between program,version and port.
 * Calls the pmap service remotely to do the un-mapping.
 */
bool_t
pmap_unset(program, version)
	u_long program;
	u_long version;
{
	struct sockaddr_in myaddress;
	int socket = -1;
	register CLIENT *client;
	struct pmap parms;
	bool_t rslt;

	get_myaddress(&myaddress);
	client = clntudp_bufcreate(&myaddress, PMAPPROG, PMAPVERS,
	    timeout, &socket, RPCSMALLMSGSIZE, RPCSMALLMSGSIZE);
	if (client == (CLIENT *)NULL)
		return (FALSE);
	parms.pm_prog = program;
	parms.pm_vers = version;
	parms.pm_port = parms.pm_prot = 0;
	CLNT_CALL(client, PMAPPROC_UNSET, xdr_pmap, &parms, xdr_bool, &rslt,
	    tottimeout);
	CLNT_DESTROY(client);
	(void)close(socket);
	return (rslt);
}

/* 
 * don't use gethostbyname, which would invoke yellow pages
 */
get_myaddress(addr)
	struct sockaddr_in *addr;
{
	int s;
	char buf[BUFSIZ];
	struct ifconf ifc;
	struct ifreq ifreq, *ifr;
	int len;
#ifdef sgi
	int gotnetifaddr = 0;
	long hostid, gethostid();
	
	hostid = gethostid();
	if (hostid != 0) {
		bzero((char*)addr, sizeof(*addr));
		addr->sin_family = AF_INET;
		addr->sin_addr.s_addr = hostid;
		addr->sin_port = htons(PMAPPORT);
		return;
	}
#endif
	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("get_myaddress: socket");
		exit(1);
	}
	ifc.ifc_len = sizeof (buf);
	ifc.ifc_buf = buf;
	if (ioctl(s, SIOCGIFCONF, (char *)&ifc) < 0) {
		perror("get_myaddress: ioctl (get interface configuration)");
		exit(1);
	}
	ifr = ifc.ifc_req;
	for (len = ifc.ifc_len; len; len -= sizeof ifreq) {
		ifreq = *ifr;
		if (ioctl(s, SIOCGIFFLAGS, (char *)&ifreq) < 0) {
			perror("get_myaddress: ioctl");
			exit(1);
		}
		if ((ifreq.ifr_flags & IFF_UP) &&
		    ifr->ifr_addr.sa_family == AF_INET) {
#ifdef sgi
			if ((ifreq.ifr_flags & IFF_LOOPBACK) == 0) {
				gotnetifaddr = 1;
			} else if (gotnetifaddr) {
				break;
			}
#endif
			*addr = *((struct sockaddr_in *)&ifr->ifr_addr);
			addr->sin_port = htons(PMAPPORT);
#ifdef sgi
			if (!gotnetifaddr) {
				continue;
			}
#endif
			break;
		}
		ifr++;
	}
	close(s);
}
