#ifndef lint
static  char sccsid[] = "@(#)getgrent.c 1.1 86/02/03  Copyr 1984 Sun Micro";
#endif

/* 
 * Copyright (c) 1984 by Sun Microsystems, Inc.
 * @(#)getgrent.c	2.2 86/04/14 NFSSRC
 */

#include <stdio.h>
#include <grp.h>
#include <rpcsvc/ypclnt.h>
#define MAXINT 0x7fffffff;
#define	MAXGRP	200

static char domain[256];
static struct group NULLGP = {NULL, NULL, 0, NULL};
static char GROUP[] = "/etc/group";
static FILE *grf = NULL;	/* pointer into /etc/group */
static char *yp;		/* pointer into yellow pages */
static int yplen;
static char *oldyp = NULL;	
static int oldyplen;
static struct group group;
static char *gr_mem[MAXGRP];
static struct list {
    char *name;
    struct list *nxt
} *minuslist;			/* list of - items */
static struct group *interpret();
static struct group *interpretwithsave();
static struct group *save();
static struct group *getnamefromyellow();
static struct group *getgidfromyellow();

struct group *
getgrgid(gid)
register gid;
{
	struct group *gp;
	char line[BUFSIZ+1];

	setgrent();
	if (!grf)
		return NULL;
	while (fgets(line, BUFSIZ, grf) != NULL) {
		gp = interpret(line, strlen(line));
		if (matchgid(line, &gp, gid)) {
			endgrent();
			return gp;
		}
	}
	endgrent();
	return NULL;
}

struct group *
getgrnam(name)
register char *name;
{
	struct group *gp;
	char line[BUFSIZ+1];

	setgrent();
	if (!grf)
		return NULL;
	while (fgets(line, BUFSIZ, grf) != NULL) {
		gp = interpret(line, strlen(line));
		if (matchname(line, &gp, name)) {
			endgrent();
			return gp;
		}
	}
	endgrent();
	return NULL;
}

setgrent()
{
	if (getdomainname(domain, sizeof(domain)) < 0) {
		fprintf(stderr, 
		    "setgrent: getdomainname system call missing\n");
		exit(1);
	}
	if(!grf)
		grf = fopen(GROUP, "r");
	else
		rewind(grf);
	if (yp)
		free(yp);
	yp = NULL;
	freeminuslist();
}

endgrent()
{
	if(grf) {
		fclose(grf);
		grf = NULL;
	}
	if (yp)
		free(yp);
	yp = NULL;
	freeminuslist();
}

static char *
grskip(p,c)
	register char *p;
	register c;
{
	while(*p && *p != c && *p != '\n') ++p;
	if(*p) *p++ = 0;
	return(p);
}

struct group *
getgrent()
{
	char line1[BUFSIZ+1];
	register char *p, **q;
	static struct group *savegp, *gp;

	if (domain[0] == 0 && getdomainname(domain, sizeof(domain)) < 0) {
		fprintf(stderr, 
		    "getgrent: getdomainname system call missing\n");
		exit(1);
	}
	if(!grf && !(grf = fopen(GROUP, "r")))
		return(NULL);
  again:
	if (yp) {
		gp = interpretwithsave(yp, yplen, savegp);
		free(yp);
		getnextfromyellow();
		if (onminuslist(gp))
			goto again;
		else
			return gp;
	}
	else if (!(p = fgets(line1, BUFSIZ, grf)))
		return(NULL);
	gp = interpret(line1, strlen(line1));
		switch(line1[0]) {
			case '+':
				if (strcmp(gp->gr_name, "+") == 0) {
					getfirstfromyellow();
					savegp = save(gp);
					goto again;
				}
				savegp = save(gp);
				gp = getnamefromyellow(gp->gr_name+1, savegp);
				if (gp == NULL)
					goto again;
				else if (onminuslist(gp))
					goto again;
				else
					return gp;
				break;
			case '-':
				addtominuslist(gp->gr_name+1);
				goto again;
				break;
			default:
				if (onminuslist(gp))
					goto again;
				return gp;
				break;
		}
}

static struct group *
interpret(val, len)
	char *val;
{
	register char *p, **q;
	static struct group gp;
	static char line[BUFSIZ+1];

	strncpy(line, val, len);
	p = line;
	line[len] = '\n';
	line[len+1] = 0;

	gp.gr_name = p;
	gp.gr_passwd = p = grskip(p,':');
	gp.gr_gid = atoi(p = grskip(p,':'));
	gp.gr_mem = gr_mem;
	p = grskip(p,':');
	grskip(p,'\n');
	q = gr_mem;
	while(*p){
		if (q < &gr_mem[MAXGRP-1])
			*q++ = p;
		p = grskip(p,',');
	}
	*q = NULL;
	return(&gp);
}

static
freeminuslist() {
	struct list *ls;
	
	for (ls = minuslist; ls != NULL; ls = ls->nxt) {
		free(ls->name);
		free(ls);
	}
	minuslist = NULL;
}

static struct group *
interpretwithsave(val, len, savegp)
	char *val;
	struct group *savegp;
{
	struct group *gp;
	
	gp = interpret(val, len);
	if (savegp->gr_passwd && *savegp->gr_passwd)
		gp->gr_passwd =  savegp->gr_passwd;
	if (savegp->gr_mem && *savegp->gr_mem)
		gp->gr_mem = savegp->gr_mem;
	return gp;
}

static
onminuslist(gp)
	struct group *gp;
{
	struct list *ls;
	register char *nm;
	
	nm = gp->gr_name;
	for (ls = minuslist; ls != NULL; ls = ls->nxt)
		if (strcmp(ls->name, nm) == 0)
			return 1;
	return 0;
}

static
getnextfromyellow()
{
	int reason;
	char *key;
	int keylen;
	
	if (reason = yp_next(domain, "group.byname",
	    oldyp, oldyplen, &key, &keylen,
	    &yp, &yplen)) {
#ifdef DEBUG
fprintf(stderr, "reason yp_next failed is %d\n", reason);
#endif
		yp = NULL;
	}
	if (oldyp)
		free(oldyp);
	oldyp = key;
	oldyplen = keylen;
}

static
getfirstfromyellow()
{
	int reason;
	char *key;
	int keylen;
	
	if (reason =  yp_first(domain, "group.byname",
	    &key, &keylen, &yp, &yplen)) {
#ifdef DEBUG
fprintf(stderr, "reason yp_first failed is %d\n", reason);
#endif
		yp = NULL;
	}
	if (oldyp)
		free(oldyp);
	oldyp = key;
	oldyplen = keylen;
}

static struct group *
getnamefromyellow(name, savegp)
	char *name;
	struct group *savegp;
{
	struct group *gp;
	int reason;
	char *val;
	int vallen;
	
	if (reason = yp_match(domain, "group.byname",
	    name, strlen(name), &val, &vallen)) {
#ifdef DEBUG
fprintf(stderr, "reason yp_next failed is %d\n", reason);
#endif
		return NULL;
	}
	else {
		gp = interpret(val, vallen);
		free(val);
		if (savegp->gr_passwd && *savegp->gr_passwd)
			gp->gr_passwd =  savegp->gr_passwd;
		if (savegp->gr_mem && *savegp->gr_mem)
			gp->gr_mem = savegp->gr_mem;
		return gp;
	}
}

static
addtominuslist(name)
	char *name;
{
	struct list *ls;
	char *buf;
	
	ls = (struct list *)malloc(sizeof(struct list));
	buf = (char *)malloc(strlen(name) + 1);
	strcpy(buf, name);
	ls->name = buf;
	ls->nxt = minuslist;
	minuslist = ls;
}

/* 
 * save away psswd, gr_mem fields, which are the only
 * ones which can be specified in a local + entry to override the
 * value in the yellow pages
 */
static struct group *
save(gp)
	struct group *gp;
{
	static struct group *sv;
	char *gr_mem[MAXGRP];	
	char **av, **q;
	int lnth;
	
	/*
	* free up stuff from last time around
	*/
	if (sv) {
		for (av = sv->gr_mem; *av != NULL; av++) {
			if (q >= &gr_mem[MAXGRP-1])
				break;
			free(*q);
			q++;
		}
		free(sv->gr_passwd);
		free(sv->gr_mem);
		free(sv);
	}
	sv = (struct group *)malloc(sizeof(struct group));
	sv->gr_passwd = (char *)malloc(strlen(gp->gr_passwd) + 1);
	strcpy(sv->gr_passwd, gp->gr_passwd);

	q = gr_mem;
	for (av = gp->gr_mem; *av != NULL; av++) {
		if (q >= &gr_mem[MAXGRP-1])
			break;
		*q = (char *)malloc(strlen(*av) + 1);
		strcpy(*q, *av);
		q++;
	}
	*q = 0;
	lnth = (sizeof (char *)) * (q - gr_mem + 1);
	sv->gr_mem = (char **)malloc(lnth);
	bcopy(gr_mem, sv->gr_mem, lnth);
	return sv;
}

static
matchname(line1, gpp, name)
	char line1[];
	struct group **gpp;
	char *name;
{
	struct group *savegp;
	struct group *gp = *gpp;

	switch(line1[0]) {
		case '+':
			if (strcmp(gp->gr_name, "+") == 0) {
				savegp = save(gp);
				gp = getnamefromyellow(name, savegp);
				if (gp) {
					*gpp = gp;
					return 1;
				}
				else
					return 0;
			}
			if (strcmp(gp->gr_name+1, name) == 0) {
				savegp = save(gp);
				gp = getnamefromyellow(gp->gr_name+1, savegp);
				if (gp) {
					*gpp = gp;
					return 1;
				}
				else
					return 0;
			}
			break;
		case '-':
			if (strcmp(gp->gr_name+1, name) == 0) {
				*gpp = NULL;
				return 1;
			}
			break;
		default:
			if (strcmp(gp->gr_name, name) == 0)
				return 1;
	}
	return 0;
}

static
matchgid(line1, gpp, gid)
	char line1[];
	struct group **gpp;
{
	struct group *savegp;
	struct group *gp = *gpp;

	switch(line1[0]) {
		case '+':
			if (strcmp(gp->gr_name, "+") == 0) {
				savegp = save(gp);
				gp = getgidfromyellow(gid, savegp);
				if (gp) {
					*gpp = gp;
					return 1;
				}
				else
					return 0;
			}
			savegp = save(gp);
			gp = getnamefromyellow(gp->gr_name+1, savegp);
			if (gp && gp->gr_gid == gid) {
				*gpp = gp;
				return 1;
			}
			else
				return 0;
			break;
		case '-':
			if (gid == gidof(gp->gr_name+1)) {
				*gpp = NULL;
				return 1;
			}
			break;
		default:
			if (gp->gr_gid == gid)
				return 1;
	}
	return 0;
}

static
gidof(name)
	char *name;
{
	struct group *gp;
	
	gp = getnamefromyellow(name, &NULLGP);
	if (gp)
		return gp->gr_gid;
	else
		return MAXINT;
}

static struct group *
getgidfromyellow(gid, savegp)
	int gid;
	struct group *savegp;
{
	struct group *gp;
	int reason;
	char *val;
	int vallen;
	char gidstr[20];
	
	sprintf(gidstr, "%d", gid);
	if (reason = yp_match(domain, "group.bygid",
	    gidstr, strlen(gidstr), &val, &vallen)) {
#ifdef DEBUG
fprintf(stderr, "reason yp_next failed is %d\n", reason);
#endif
		return NULL;
	}
	else {
		gp = interpret(val, vallen);
		free(val);
		if (savegp->gr_passwd && *savegp->gr_passwd)
			gp->gr_passwd =  savegp->gr_passwd;
		if (savegp->gr_mem && *savegp->gr_mem)
			gp->gr_mem = savegp->gr_mem;
		return gp;
	}
}
