#ifndef lint
static  char sccsid[] = "@(#)ether_addr.c 1.1 86/02/03 Copyr 1985 Sun Micro";
/* @(#)ether_addr.c	2.1 86/04/11 NFSSRC */
#endif

/*
 * Copyright (c) 1985 by Sun Microsystems, Inc.
 *
 * All routines necessary to deal with the file /etc/ethers.  The file
 * contains mappings from 48 bit ethernet addresses to their corresponding
 * hosts name.  The addresses have an ascii representation of the form
 * "x:x:x:x:x:x" where x is a hex number between 0x00 and 0xff;  the
 * bytes are always in network order.
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>

#ifdef sgi
/*
 * In SUN's case, this is in <netinet/if_ether.h>.  In 4.3, the
 * definition was removed and references replaced with "u_char foo[6]".
 * Let's try to avoid putting it in the include file and hope that
 * SUN code that calls this routine doesn't require struct ether_addr.
 * If this assumption is wrong (as it probably is), we will have to
 * bite the bullet.
 */
struct ether_addr {
	unsigned char ether_addr_octet[6];
};
#endif

static int useyp();  /* tells whether we use yp or a local file */
static char domain[256]; /* yp domain name */
static char *filename = "/etc/ethers";

/*
 * Parses a line from /etc/ethers into its components.  The line has the form
 * 8:0:20:1:17:c8	krypton
 * where the first part is a 48 bit ethernet address and the second is
 * the corresponding host's name.
 * Returns zero if successful, non-zero otherwise.
 */
ether_line(s, e, hostname)
	char *s;		/* the string to be parsed */
	struct ether_addr *e;	/* ethernet address struct to be filled in */
	char *hostname;		/* hosts name to be set */
{
	register int i;
	unsigned int t[6];
	
	i = sscanf(s, " %x:%x:%x:%x:%x:%x %s",
	    &t[0], &t[1], &t[2], &t[3], &t[4], &t[5], hostname);
	if (i != 7) {
		return (7 - i);
	}
	for (i = 0; i < 6; i++)
		e->ether_addr_octet[i] = t[i];
	return (0);
}

/*
 * Converts a 48 bit ethernet number to its string representation.
 */
#define EI(i)	(unsigned int)(e->ether_addr_octet[(i)])
char *
ether_ntoa(e)
	struct ether_addr *e;
{
	static char s[18];
	
	s[0] = 0;
	sprintf(s, "%x:%x:%x:%x:%x:%x",
	    EI(0), EI(1), EI(2), EI(3), EI(4), EI(5));
	return (s);
}

/*
 * Converts a ethernet address representation back into its 48 bits.
 */
struct ether_addr *
ether_aton(s)
	char *s;
{
	static struct ether_addr e;
	register int i;
	unsigned int t[6];
	
	i = sscanf(s, " %x:%x:%x:%x:%x:%x",
	    &t[0], &t[1], &t[2], &t[3], &t[4], &t[5]);
	if (i != 6)
	    return ((struct ether_addr *)NULL);
	for (i = 0; i < 6; i++) {
		e.ether_addr_octet[i] = t[i];
	}
	return(&e);
}

/*
 * Given a host's name, this routine returns its 48 bit ethernet address.
 * Returns zero if successful, non-zero otherwise.
 */
ether_hostton(host, e)
	char *host;		/* function input */
	struct ether_addr *e;	/* function output */
{
	char currenthost[256];
	char buf[512];
	char *val = buf;
	int vallen;
	register int reason;
	FILE *f;
	
	if (useyp()) {
		if (reason = yp_match(domain, "ethers.byname", host,
		    strlen(host), &val, &vallen)) {
			return (reason);
		} else {
			return (ether_line(val, e, currenthost));
		}
	} else {
		if ((f = fopen(filename, "r")) == NULL) {
			return (-1);
		}
		reason = -1;
		while (fscanf(f, "%[^\n] ", val) == 1) {
			if ((ether_line(val, e, currenthost) == 0) &&
			    (strcmp(currenthost, host) == 0)) {
				reason = 0;
				break;
			}
		}
		fclose(f);
		return (reason);
	}
}

/*
 * Given a 48 bit ethernet address, this routine return its host name.
 * Returns zero if successful, non-zero otherwise.
 */
ether_ntohost(host, e)
	char *host;		/* function output */
	struct ether_addr *e;	/* function input */
{
	struct ether_addr currente;
	char *a = ether_ntoa(e);
	char buf[512];
	char *val = buf;
	int vallen;
	register int reason;
	FILE *f;
	
	if (useyp()) {
		if (reason = yp_match(domain, "ethers.byaddr", a,
		    strlen(a), &val, &vallen)) {
			return (reason);
		} else {
			return (ether_line(val, &currente, host));
		}
	} else {
		if ((f = fopen(filename, "r")) == NULL) {
			return (-1);
		}
		reason = -1;
		while (fscanf(f, "%[^\n] ", val) == 1) {
			if ((ether_line(val, &currente, host) == 0) &&
			    (bcmp(e, &currente, sizeof(currente)) == 0)) {
				reason = 0;
				break;
			}
		}
		fclose(f);
		return (reason);
	}
}

/*
 * Determines whether or not to use the yellow pages service to do lookups.
 */
static int initted;
static int usingyp;
static int
useyp()
{
	if (!initted) {
		getdomainname(domain, sizeof(domain));
		usingyp = !yp_bind(domain);
		initted = 1;
	}
	return (usingyp);
}
