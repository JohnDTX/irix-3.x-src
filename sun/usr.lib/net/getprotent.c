#ifndef lint
static  char sccsid[] = "@(#)getprotoent.c 1.1 86/02/03  Copyr 1984 Sun Micro";
/* @(#)getprotoent.c	2.1 86/04/11 NFSSRC */
#endif

/* 
 * Copyright (c) 1984 by Sun Microsystems, Inc.
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
static char *current = NULL;	/* current entry, analogous to protof */
static int currentlen;
static struct protoent *interpret();
struct protoent *getprotoent();
char *inet_ntoa();
static char *any();
static char PROTODB[] = "/etc/protocols";
static FILE *protof = NULL;
static int usingyellow;		/* are yellow pages up? */

struct protoent *
getprotobynumber(proto)
{
	register struct protoent *p;
	int reason;
	char adrstr[12], *val;
	int vallen;

	setprotoent(0);
	if (!usingyellow) {
		while (p = getprotoent()) {
			if (p->p_proto == proto)
				break;
		}
	}
	else {
		sprintf(adrstr, "%d", proto);
		if (reason = yp_match(domain, "protocols.bynumber",
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
	endprotoent();
	return (p);
}

struct protoent *
getprotobyname(name)
	register char *name;
{
	register struct protoent *p;
	register char **cp;
	int reason;
	char *val;
	int vallen;

	setprotoent(0);
	if (!usingyellow) {
		while (p = getprotoent()) {
			if (strcmp(p->p_name, name) == 0)
				break;
			for (cp = p->p_aliases; *cp != 0; cp++)
				if (strcmp(*cp, name) == 0)
					goto found;
		}
	}
	else {
		if (reason = yp_match(domain, "protocols.byname",
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
	endprotoent();
	return (p);
}

setprotoent(f)
	int f;
{
	if (getdomainname(domain, sizeof(domain)) < 0) {
		fprintf(stderr, 
		    "setprotoent: getdomainname system call missing\n");
		exit(1);
	}
	if (protof == NULL)
		protof = fopen(PROTODB, "r");
	else
		rewind(protof);
	if (current)
		free(current);
	current = NULL;
	stayopen |= f;
	yellowup(1);	/* recompute whether yellow pages are up */
}

endprotoent()
{
	if (current && !stayopen) {
		free(current);
		current = NULL;
	}
	if (protof && !stayopen) {
		fclose(protof);
		protof = NULL;
	}
}

struct protoent *
getprotoent()
{
	int reason;
	char *key, *val;
	int keylen, vallen;
	static char line1[BUFSIZ+1];
	struct protoent *pp;

	yellowup(0);
	if (!usingyellow) {
		if (protof == NULL && (protof = fopen(PROTODB, "r")) == NULL)
			return (NULL);
	        if (fgets(line1, BUFSIZ, protof) == NULL)
			return (NULL);
		return interpret(line1, strlen(line1));
	}
	if (current == NULL) {
		if (reason =  yp_first(domain, "protocols.bynumber",
		    &key, &keylen, &val, &vallen)) {
#ifdef DEBUG
			fprintf(stderr, "reason yp_first failed is %d\n",
			    reason);
#endif
			return NULL;
		    }
	}
	else {
		if (reason = yp_next(domain, "protocols.bynumber",
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
	pp = interpret(val, vallen);
	free(val);
	return (pp);
}

static struct protoent *
interpret(val, len)
{
	static char *proto_aliases[MAXALIASES];
	static struct protoent proto;
	static char line[BUFSIZ+1];
	char *p;
	register char *cp, **q;

	strncpy(line, val, len);
	p = line;
	line[len] = '\n';
	if (*p == '#')
		return (getprotoent());
	cp = any(p, "#\n");
	if (cp == NULL)
		return (getprotoent());
	*cp = '\0';
	proto.p_name = p;
	cp = any(p, " \t");
	if (cp == NULL)
		return (getprotoent());
	*cp++ = '\0';
	while (*cp == ' ' || *cp == '\t')
		cp++;
	p = any(cp, " \t");
	if (p != NULL)
		*p++ = '\0';
	proto.p_proto = atoi(cp);
	q = proto.p_aliases = proto_aliases;
	if (p != NULL) {
		cp = p;
		while (cp && *cp) {
			if (*cp == ' ' || *cp == '\t') {
				cp++;
				continue;
			}
			if (q < &proto_aliases[MAXALIASES - 1])
				*q++ = cp;
			cp = any(cp, " \t");
			if (cp != NULL)
				*cp++ = '\0';
		}
	}
	*q = NULL;
	return (&proto);
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
			   "getprotoent: getdomainname system call missing\n");
				exit(1);
			}
		}
		usingyellow = !yp_bind(domain);
	}	
}
