#ifndef lint
static  char sccsid[] = "@(#)gethostent.c 1.1 86/02/03  Copyr 1984 Sun Micro";
/* @(#)gethostent.c	2.1 86/04/11 NFSSRC */
#endif

/* 
 * Copyright (c) 1985 by Sun Microsystems, Inc.
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <rpcsvc/ypclnt.h>

/*
 * Internet version.
 */
#define	MAXALIASES	35
#define	MAXADDRSIZE	14

static char domain[256];
static int stayopen;
static char *current = NULL;	/* current entry, analogous to hostf */
static int currentlen;
static struct hostent *interpret();
struct hostent *gethostent();
char *inet_ntoa();
static char *any();
static char HOSTDB[] = "/etc/hosts";
static FILE *hostf = NULL;
static int usingyellow;		/* are yellow pages up? */
static void yellowup();

#ifdef sgi
int	h_errno;	/* BSD 4.3 error return for gethostby{name,addr} */
#endif

struct hostent *
gethostbyaddr(addr, len, type)
	char *addr;
	register int len, type;
{
	register struct hostent *p;
	int reason;
	char *adrstr, *val;
	int vallen;

	sethostent(0);
	if (!usingyellow) {
		while (p = gethostent()) {
			if (p->h_addrtype != type || p->h_length != len)
				continue;
			if (bcmp(p->h_addr, addr, len) == 0)
				break;
		}
	}
	else {
		adrstr = inet_ntoa(*(int *)addr);
		if (reason = yp_match(domain, "hosts.byaddr",
		    adrstr, strlen(adrstr), &val, &vallen)) {
#ifdef DEBUG
			fprintf(stderr, "reason yp_first failed is %d\n",
			    reason);
#endif
			p = NULL;
		    }
		else {
			p = interpret(val, vallen);
			free(val);
		}
	}
	endhostent();
#ifdef sgi
	/*
	 * Set h_errno for 4.3 compatibility
	 */
	if (p == NULL)
		h_errno = HOST_NOT_FOUND;
#endif
	return (p);
}

struct hostent *
gethostbyname(name)
	register char *name;
{
	register struct hostent *p;
	register char **cp;
	int reason;
	char *val;
	int vallen;

	sethostent(0);
	if (!usingyellow) {
		while (p = gethostent()) {
			if (strcmp(p->h_name, name) == 0)
				break;
			for (cp = p->h_aliases; *cp != 0; cp++)
				if (strcmp(*cp, name) == 0)
					goto found;
		}
	}
	else {
		if (reason = yp_match(domain, "hosts.byname",
		    name, strlen(name), &val, &vallen)) {
#ifdef DEBUG
			fprintf(stderr, "reason yp_first failed is %d\n",
			    reason);
#endif
			p = NULL;
		    }
		else {
			p = interpret(val, vallen);
			free(val);
		}
	}
found:
	endhostent();
#ifdef sgi
	/*
	 * Set h_errno for 4.3 compatibility
	 */
	if (p == NULL)
		h_errno = HOST_NOT_FOUND;
#endif
	return (p);
}

sethostent(f)
	int f;
{
	if (getdomainname(domain, sizeof(domain)) < 0) {
		fprintf(stderr, 
		    "sethostent: getdomainname system call missing\n");
		exit(1);
	}
	if (hostf == NULL)
		hostf = fopen(HOSTDB, "r");
	else
		rewind(hostf);
	if (current)
		free(current);
	current = NULL;
	stayopen |= f;
	yellowup(1);	/* recompute whether yellow pages are up */
}

endhostent()
{
	if (current && !stayopen) {
		free(current);
		current = NULL;
	}
	if (hostf && !stayopen) {
		fclose(hostf);
		hostf = NULL;
	}
}

struct hostent *
gethostent()
{
	struct hostent *hp;
	int reason;
	char *key, *val;
	int keylen, vallen;
	static char line1[BUFSIZ+1];

	yellowup(0);
	if (!usingyellow) {
		if (hostf == NULL && (hostf = fopen(HOSTDB, "r")) == NULL)
			return (NULL);
	        if (fgets(line1, BUFSIZ, hostf) == NULL)
			return (NULL);
		return interpret(line1, strlen(line1));
	}
	if (current == NULL) {
		if (reason =  yp_first(domain, "hosts.byaddr",
		    &key, &keylen, &val, &vallen)) {
#ifdef DEBUG
			fprintf(stderr, "reason yp_first failed is %d\n",
			    reason);
#endif
			return NULL;
		    }
	}
	else {
		if (reason = yp_next(domain, "hosts.byaddr",
		    current, currentlen, &key, &keylen, &val, &vallen)) {
#ifdef DEBUG
			fprintf(stderr, "reason yp_next failed is %d\n",
			    reason);
#endif
			return NULL;
		}
	}
	if (current)
		free(current);
	current = key;
	currentlen = keylen;
	hp = interpret(val, vallen);
	free(val);
	return (hp);
}

static struct hostent *
interpret(val, len)
{
	static char *host_aliases[MAXALIASES];
#ifdef sgi
	static char *hostaddrlist[MAXALIASES];
#endif
	static char hostaddr[MAXADDRSIZE];
	static struct hostent host;
	static char line[BUFSIZ+1];
	char *p;
	register char *cp, **q;

	strncpy(line, val, len);
	p = line;
	line[len] = '\n';
	if (*p == '#')
		return (gethostent());
	cp = any(p, "#\n");
	if (cp == NULL)
		return (gethostent());
	*cp = '\0';
	cp = any(p, " \t");
	if (cp == NULL)
		return (gethostent());
	*cp++ = '\0';
	/* THIS STUFF IS INTERNET SPECIFIC */
#ifdef sgi
	/*
	 * This change is required by a change in the declaration
	 * of struct hostent between BSD 4.2 and BSD 4.3.  SUN's
	 * implementation is based on 4.2; ours is 4.3.
	 */
	host.h_addr_list = hostaddrlist;
	host.h_addr_list[0] = hostaddr;
	host.h_addr_list[1] = NULL;
	*((u_long *)host.h_addr_list[0]) = inet_addr(p);
#else
	host.h_addr = hostaddr;
	*((u_long *)host.h_addr) = inet_addr(p);
#endif
	host.h_length = sizeof (u_long);
	host.h_addrtype = AF_INET;
	while (*cp == ' ' || *cp == '\t')
		cp++;
	host.h_name = cp;
	q = host.h_aliases = host_aliases;
	cp = any(cp, " \t");
	if (cp != NULL) 
		*cp++ = '\0';
	while (cp && *cp) {
		if (*cp == ' ' || *cp == '\t') {
			cp++;
			continue;
		}
		if (q < &host_aliases[MAXALIASES - 1])
			*q++ = cp;
		cp = any(cp, " \t");
		if (cp != NULL)
			*cp++ = '\0';
	}
	*q = NULL;
	return (&host);
}

static char *
any(cp, match)
	register char *cp;
	char *match;
{
	register char *mp, c;

	while (c = *cp) {
		for (mp = match; *mp; mp++)
			if (*mp == c)
				return (cp);
		cp++;
	}
	return ((char *)0);
}

/* 
 * check to see if yellow pages are up, and store that fact in usingyellow.
 * The check is performed once at startup and thereafter if flag is set
 */
static void
yellowup(flag)
{
	static int firsttime = 1;
	char *key, *val;
	int keylen, vallen;
#ifdef sgi
	int so;
#endif

	if (firsttime || flag) {
		firsttime = 0;
		if (domain[0] == 0) {
			if (getdomainname(domain, sizeof(domain)) < 0) {
				fprintf(stderr, 
			    "gethostent: getdomainname system call missing\n");
				exit(1);
			}
		}
#ifdef sgi
		/*
		 * Make sure we are running on a kernel supporting TCP/IP
		 * before calling yp_bind to see if the Yellow Pages
		 * are present.  The socket call will fail on XNS only
		 * kernels.
		 *
		 * This is a bit of a kludge, but it is easier than
		 * straightening out the low level YP routines that
		 * simply print out a message and 'exit(1)' when
		 * socket() fails.
		 */
		if ((so = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
			usingyellow = 0;
			return;
		}
		(void) close(so);
#endif
		usingyellow = !yp_bind(domain);
	}	
}
