/*	@(#)getgrent.c	1.2	*/
/*	3.0 SID #	1.2	*/
/*LINTLIBRARY*/
#include <stdio.h>
#include <grp.h>

#define	CL	':'
#define	CM	','
#define	NL	'\n'
#define	MAXGRP	100

extern int atoi(), fclose();
extern char *fgets();
extern FILE *fopen();
extern void rewind();

static char GROUP[] = "/etc/group";
static FILE *grf = NULL;
static char line[BUFSIZ+1];
static struct group grp;
static char *gr_mem[MAXGRP];

void
setgrent()
{
	if(!grf)
		grf = fopen(GROUP, "r");
	else
		rewind(grf);
}

void
endgrent()
{
	if(grf) {
		(void) fclose(grf);
		grf = NULL;
	}
}

static char *
grskip(p,c)
register char *p;
register c;
{
	while(*p && *p != c)
		++p;
	if(*p)
	 	*p++ = 0;
	return(p);
}

struct group *
getgrent()
{
	register char *p, **q;

	if(!grf && !(grf = fopen(GROUP, "r")))
		return(NULL);
	if(!(p = fgets(line, BUFSIZ, grf)))
		return(NULL);
	grp.gr_name = p;
	grp.gr_passwd = p = grskip(p, CL);
	grp.gr_gid = atoi(p = grskip(p, CL));
	grp.gr_mem = gr_mem;
	p = grskip(p, CL);
	(void) grskip(p, NL);
	q = gr_mem;
	while(*p) {
		*q++ = p;
		p = grskip(p, CM);
	}
	*q = NULL;
	return(&grp);
}
