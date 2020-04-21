#ifndef lint
static	char sccsid[] = "@(#)getnetgrent.c 1.1 86/02/03 Copyr 1985 Sun Micro";
/* @(#)getnetgrent.c	2.1 86/04/11 NFSSRC */
#endif

/* 
 * Copyright (c) 1985 by Sun Microsystems, Inc.
 */

#include <stdio.h>
#include <ctype.h>
#include <rpcsvc/ypclnt.h>

#define MAXGROUPLEN 1024

/* 
 * access members of a netgroup
 */

struct grouplist {		/* also used by pwlib */
	char *gl_machine;
	char *gl_name;
	char *gl_domain;
	struct grouplist *gl_nxt;
};

#ifdef sgi
extern char *malloc();
#else
extern char *malloc(),*alloca();
#endif
extern char *strcpy(),*strncpy();

#ifdef sgi
static char *any();
extern char *index();
#else
static char *any(), *index();
#endif

static struct list {			/* list of names to check for loops */
	char *name;
	struct list *nxt;
};


static struct grouplist *grouplist, *grlist;



static void getgroup();
static void doit();
static char *fill();
static char *match();
static char domain[256];



setnetgrent(grp)
	char *grp;
{
	static char oldgrp[256];
	
	if (strcmp(oldgrp, grp) == 0)
		grlist = grouplist;
	else {
		if (grouplist != NULL)
			endnetgrent();
		getgroup(grp);
		grlist = grouplist;
		(void) strcpy(oldgrp, grp);
	}
}

endnetgrent() {
	struct grouplist *gl;
	
	for (gl = grouplist; gl != NULL; gl = gl->gl_nxt) {
		if (gl->gl_name)
			free(gl->gl_name);
		if (gl->gl_domain)
			free(gl->gl_domain);
		if (gl->gl_machine)
			free(gl->gl_machine);
		free((char *) gl);
	}
	grouplist = NULL;
	grlist = NULL;
}

getnetgrent(machinep, namep, domainp)
	char **machinep, **namep, **domainp;
{
	if (grlist) {
		*machinep = grlist->gl_machine;
		*namep = grlist->gl_name;
		*domainp = grlist->gl_domain;
		grlist = grlist->gl_nxt;
		return (1);
	}
	else
		return (0);
}



static void
getgroup(grp)
	char *grp;
{
	if (getdomainname(domain, sizeof(domain)) < 0) {
		(void) fprintf(stderr, 
		    "getnetgrent: getdomainname system call missing\n");
	    exit(1);
	}
	doit(grp,(struct list *) NULL);
}
		

/*
 * recursive function to find the members of netgroup "group". "list" is
 * the path followed through the netgroups so far, to check for cycles.
 */
static void
doit(group,list)
    char *group;
    struct list *list;
{
    register char *p, *q;
    register struct list *ls;
    char *val;
    struct grouplist *gpls;
 
 
    /*
     * check for non-existing groups
     */
    if ((val = match(group)) == NULL) {
        return;
    }
 
 
    /*
     * check for cycles
     */
    for (ls = list; ls != NULL; ls = ls->nxt) {
        if (strcmp(ls->name, group) == 0) {
            (void) fprintf(stderr,
					"Cycle detected in /etc/netgroup: %s.\n",group);
            return;
        }
    }
 
#ifdef sgi 
    ls = (struct list *) malloc(sizeof(struct list));
#else
    ls = (struct list *) alloca(sizeof(struct list));
#endif
    ls->name = group;
    ls->nxt = list;
    list = ls;
    
    p = val;
    while (p != NULL) {
        while (*p == ' ' || *p == '\t')
            p++;
        if (*p == 0 || *p =='#')
            break;
        if (*p == '(') {
            gpls = (struct grouplist *) malloc(sizeof(struct grouplist));
            p++;
 
            if (!(p = fill(p,&gpls->gl_machine,',')))  {
                goto syntax_error;
            }
            if (!(p = fill(p,&gpls->gl_name,','))) {
                goto syntax_error;
            }
            if (!(p = fill(p,&gpls->gl_domain,')'))) {
                goto syntax_error;
            }
            gpls->gl_nxt = grouplist;
            grouplist = gpls;
        } else {
            q = any(p, " \t\n#");
            if (q && *q == '#')
                break;
            *q = 0;
            doit(p,list);
            *q = ' ';
        }
        p = any(p, " \t");
    }
out:
#ifdef sgi
    (void) free(ls);
#endif
    return;
 
syntax_error:
    (void) fprintf(stderr,"syntax error in /etc/netgroup\n");
    (void) fprintf(stderr,"--- %s\n",val);
    goto out;
}



/*
 * Fill a buffer "target" selectively from buffer "start".
 * "termchar" terminates the information in start, and preceding
 * or trailing white space is ignored. The location just after the
 * terminating character is returned.  
 */
static char *
fill(start,target,termchar)
    char *start;
    char **target;
    char termchar;
{
    register char *p;
    register char *q;
    char *r;
	unsigned size;
	
 
    for (p = start; *p == ' ' || *p == '\t'; p++)
        ;
    r = index(p, termchar);
    if (r == NULL) {
        return(NULL);
    }
    if (p == r) {
		*target = NULL;	
    } else {
        for (q = r-1; *q == ' ' || *q == '\t'; q--)
            ;
		size = q - p + 1;
		*target = malloc(size+1);
		(void) strncpy(*target,p,(int) size);
		(*target)[size] = 0;
	}
    return(r+1);
}



static char *
match(group)
	char *group;
{
	char *val;
	int vallen;
	int err;

	err = yp_match(domain,"netgroup",group,strlen(group),&val,&vallen);
	if (err) {
#ifdef DEBUG
		(void) fprintf(stderr,"yp_match(netgroup,%s) failed: %s\n",group
				,yperr_string(err));
#endif
		return(NULL);
	}
	return(val);
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
