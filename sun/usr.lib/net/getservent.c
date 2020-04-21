#ifndef lint
static  char sccsid[] = "@(#)getservent.c 1.1 86/02/03  Copyr 1984 Sun Micro";
/* @(#)getservent.c	2.1 86/04/11 NFSSRC */
#endif

/* 
 * Copyright (c) 1984 by Sun Microsystems, Inc.
 */

/* 
 * unlike gethost, getpw, etc, this doesn't implement getservbyxxx
 * directly
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
static char *current = NULL;	/* current entry, analogous to servf */
static int currentlen;
static struct servent *interpret();
struct servent *getservent();
char *inet_ntoa();
static char *any();
static char SERVDB[] = "/etc/services";
static FILE *servf = NULL;
static int usingyellow;		/* are yellow pages up? */

struct servent *
getservbyport(port, proto)
	int port;
	char *proto;
{
	register struct servent *p;

	setservent(0);
	while (p = getservent()) {
		if (p->s_port != port)
			continue;
		if (proto == 0 || strcmp(p->s_proto, proto) == 0)
			break;
	}
	endservent();
	return (p);
}

struct servent *
getservbyname(name, proto)
	register char *name, *proto;
{
	register struct servent *p;
	register char **cp;

	setservent(0);
	while (p = getservent()) {
		if (strcmp(name, p->s_name) == 0)
			goto gotname;
		for (cp = p->s_aliases; *cp; cp++)
			if (strcmp(name, *cp) == 0)
				goto gotname;
		continue;
gotname:
		if (proto == 0 || strcmp(p->s_proto, proto) == 0)
			break;
	}
	endservent();
	return (p);
}

setservent(f)
	int f;
{
	if (getdomainname(domain, sizeof(domain)) < 0) {
		fprintf(stderr, 
		    "setservent: getdomainname system call missing\n");
		exit(1);
	}
	if (servf == NULL)
		servf = fopen(SERVDB, "r");
	else
		rewind(servf);
	if (current)
		free(current);
	current = NULL;
	stayopen |= f;
	yellowup(1);	/* recompute whether yellow pages are up */
}

endservent()
{
	if (current && !stayopen) {
		free(current);
		current = NULL;
	}
	if (servf && !stayopen) {
		fclose(servf);
		servf = NULL;
	}
}

struct servent *
getservent()
{
	int reason;
	char *key, *val;
	int keylen, vallen;
	static char line1[BUFSIZ+1];
	struct servent *sp;

	yellowup(0);
	if (!usingyellow) {
		if (servf == NULL && (servf = fopen(SERVDB, "r")) == NULL)
			return (NULL);
	        if (fgets(line1, BUFSIZ, servf) == NULL)
			return (NULL);
		return interpret(line1, strlen(line1));
	}
	if (current == NULL) {
		if (reason =  yp_first(domain, "services.byname",
		    &key, &keylen, &val, &vallen)) {
#ifdef DEBUG
			fprintf(stderr, "reason yp_first failed is %d\n",
			    reason);
#endif
			return NULL;
		    }
	}
	else {
		if (reason = yp_next(domain, "services.byname",
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
	sp = interpret(val, vallen);
	return (sp);
}

static struct servent *
interpret(val, len)
{
	static char *serv_aliases[MAXALIASES];
	static struct servent serv;
	static char line[BUFSIZ+1];
	char *p;
	register char *cp, **q;

	strncpy(line, val, len);
	p = line;
	line[len] = '\n';
	if (*p == '#')
		return (getservent());
	cp = any(p, "#\n");
	if (cp == NULL)
		return (getservent());
	*cp = '\0';
	serv.s_name = p;
	p = any(p, " \t");
	if (p == NULL)
		return (getservent());
	*p++ = '\0';
	while (*p == ' ' || *p == '\t')
		p++;
	cp = any(p, ",/");
	if (cp == NULL)
		return (getservent());
	*cp++ = '\0';
	serv.s_port = htons((u_short)atoi(p));
	serv.s_proto = cp;
	q = serv.s_aliases = serv_aliases;
	cp = any(cp, " \t");
	if (cp != NULL)
		*cp++ = '\0';
	while (cp && *cp) {
		if (*cp == ' ' || *cp == '\t') {
			cp++;
			continue;
		}
		if (q < &serv_aliases[MAXALIASES - 1])
			*q++ = cp;
		cp = any(cp, " \t");
		if (cp != NULL)
			*cp++ = '\0';
	}
	*q = NULL;
	return (&serv);
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
static
yellowup(flag)
{
	static int firsttime = 1;
	char *key, *val;
	int keylen, vallen;

	if (firsttime || flag) {
		firsttime = 0;
		if (domain[0] == 0) {
			if (getdomainname(domain, sizeof(domain)) < 0) {
				fprintf(stderr, 
			    "getservent: getdomainname system call missing\n");
				exit(1);
			}
		}
		usingyellow = !yp_bind(domain);
	}	
}
