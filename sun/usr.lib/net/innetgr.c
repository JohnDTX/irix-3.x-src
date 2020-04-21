#ifndef lint
static	char sccsid[] = "@(#)innetgr.c 1.1 86/02/03 Copyr 1985 Sun Micro";
/* @(#)innetgr.c	2.1 86/04/11 NFSSRC */
#endif

/* 
 * Copyright (c) 1985 by Sun Microsystems, Inc.
 */

#include <stdio.h>
#include <ctype.h>
#include <rpcsvc/ypclnt.h>

/* 
 * innetgr: test whether I'm in /etc/netgroup
 * 
 */

extern char *malloc();

#ifdef sgi
static char *any(), *strcpy();
extern char *index();
#else
static char *any(), *index(), *strcpy();
#endif
static char *name, *machine, *domain;
static char thisdomain[256];
static char *list[200];		/* can nest recursively this deep */
static char **listp;		/* pointer into list */

innetgr(grp, mach, nm, dom)
	char *grp;
	char *mach;
	char *nm;
	char *dom;
{
	int res;

	if (getdomainname(thisdomain, sizeof(thisdomain)) < 0) {
		(void) fprintf(stderr, 
		    "innetgr: getdomainname system call missing\r\n");
	    exit(1);
	}
	listp = list;
	machine = mach;
	name = nm;
	domain = dom;
	if (domain) {
		if (name && !machine) {
			if (lookup("netgroup.byuser",grp,name,domain,&res)) {
				return(res);
			}
		} else if (machine && !name) {
			if (lookup("netgroup.byhost",grp,machine,domain,&res)) {
				return(res);
			}
		}
	}
	return doit(grp);
}
	

	

/* 
 * calls itself recursively
 */
static
doit(group)
	char *group;
{
	char *key, *val;
	int vallen,keylen;
	char *r;
	int match;
	register char *p, *q;
	register char **lp;
	int err;
	
	*listp++ = group;
	if (listp > list + sizeof(list)) {
		(void) fprintf(stderr, "innetgr: recursive overflow\r\n");
		listp--;
		return (0);
	}
	key = group;
	keylen = strlen(group);
	err = yp_match(thisdomain, "netgroup", key, keylen, &val, &vallen);
	if (err) {
#ifdef DEBUG
		if (err == YPERR_KEY)
			(void) fprintf(stderr,
			    "innetgr: no such netgroup as %s\n", group);
		else
			(void) fprintf(stderr, "innetgr: yp_match, %s\n",yperr_string(err));
#endif
		listp--;
		return(0);
	}
	/* 
	 * check for recursive loops
	 */
	for (lp = list; lp < listp-1; lp++)
		if (strcmp(*lp, group) == 0) {
			(void) fprintf(stderr,
			    "innetgr: netgroup %s called recursively\r\n",
			    group);
			listp--;
			return(0);
		}
	
	p = val;
	p[vallen] = 0;
	while (p != NULL) {
		match = 0;
		while (*p == ' ' || *p == '\t')
			p++;
		if (*p == 0 || *p == '#')
			break;
		if (*p == '(') {
			p++;
			while (*p == ' ' || *p == '\t')
				p++;
			r = q = index(p, ',');
			if (q == NULL) {
				(void) fprintf(stderr,
				    "innetgr: syntax error in /etc/netgroup\r\n");
				listp--;
				return(0);
			}
			if (p == q || machine == NULL)
				match++;
			else {
				while (*(q-1) == ' ' || *(q-1) == '\t')
					q--;
				if (strncmp(machine, p, q-p) == 0)
					match++;
			}
			p = r+1;

			while (*p == ' ' || *p == '\t')
				p++;
			r = q = index(p, ',');
			if (q == NULL) {
				(void) fprintf(stderr,
				    "innetgr: syntax error in /etc/netgroup\r\n");
				listp--;
				return(0);
			}
			if (p == q || name == NULL)
				match++;
			else {
				while (*(q-1) == ' ' || *(q-1) == '\t')
					q--;
				if (strncmp(name, p, q-p) == 0)
					match++;
			}
			p = r+1;

			while (*p == ' ' || *p == '\t')
				p++;
			r = q = index(p, ')');
			if (q == NULL) {
				(void) fprintf(stderr,
				    "innetgr: syntax error in /etc/netgroup\r\n");
				listp--;
				return(0);
			}
			if (p == q || domain == NULL)
				match++;
			else {
				while (*(q-1) == ' ' || *(q-1) == '\t')
					q--;
				if (strncmp(domain, p, q-p) == 0)
					match++;
			}
			p = r+1;
			if (match == 3) {
				free(val);
				listp--;
				return 1;
			}
		}
		else {
			q = any(p, " \t\n#");
			if (q && *q == '#')
				break;
			if (q)
				*q = 0;
			if (doit(p)) {
				free(val);
				listp--;
				return 1;
			}
			if (q)
				*q = ' ';
		}
		p = any(p, " \t");
	}
	free(val);
	listp--;
	return 0;
}

/* 
 * scans cp, looking for a match with any character
 * in match.  Returns pointer to place in cp that matched
 * (or NULL if no match)
 */
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
 * return 1 if "what" is in the comma-separated, newline-terminated "list"
 */
static
inlist(what,list)
	char *what;
	char *list;
{
#	define TERMINATOR(c)    (c == ',' || c == '\n')

	register char *p;
	int len;
         
	len = strlen(what);     
	p = list;
	do {             
		if (strncmp(what,p,len) == 0 && TERMINATOR(p[len])) {
			return(1);
		}
		while (!TERMINATOR(*p)) {
			p++;
		}
		p++;
	} while (*p);
	return(0);
}




/*
 * Lookup a host or user name in a yp map.  Set result to 1 if group in the 
 * lookup-up list of groups. Return 1 if the map was found.
 */
static
lookup(map,group,name,domain,res)
	char *map;
	char *group;
	char *name;
	char *domain;
	int *res;
{
	int err;
	char *val;
	int vallen;
	char key[256];
	char *wild = "*";
	int i;

	for (i = 0; i < 4; i++) {
		switch (i) {
		case 0: makekey(key,name,domain); break;
		case 1: makekey(key,wild,domain); break;	
		case 2: makekey(key,name,wild); break;
		case 3: makekey(key,wild,wild); break;	
		}
		err  = yp_match(thisdomain,map,key,strlen(key),&val,&vallen); 
		if (!err) {
			*res = inlist(group,val);
			free(val);
			if (*res) {
				return(1);
			}
		} else {
#ifdef DEBUG
			(void) fprintf(stderr,
				"yp_match(%s,%s) failed: %s.\n",map,key,yperr_string(err));
#endif
			if (err != YPERR_KEY)  {
				return(0);
			}
		}
	}
	*res = 0;
	return(1);
}



/*
 * Generate a key for a netgroup.byXXXX yp map
 */
static
makekey(key,name,domain)
	register char *key;
	register char *name;
	register char *domain;
{
	while (*key++ = *name++)
		;
	*(key-1) = '.';
	while (*key++ = *domain++)
		;
}	
