/* @(#)us_crs.c	1.5 */
/* $Source: /d2/3.7/src/usr.bin/uucp/RCS/us_crs.c,v $*/
/* $Revision: 1.1 $*/
/* $Date: 89/03/27 18:30:45 $*/

#include "uucp.h"
#ifdef UUSTAT
#include <sys/types.h>
#include "uust.h"
 
/*
 * Whenever a command file (i.e. C.*****) file is spooled by uucp,
 * creates an entry in the beginning of "R_stat" file. 
 * Future expansion: An R_stat entry may be created by, e.g.
 * uux, rmail, stock, or any command using uucp.
 * return:
 *	0	-> success
 *	FAIL	-> failure
 */
us_crs(cfile)
char cfile[NAMESIZE];
{
	register FILE *fq;
	register short i;
	char *name, *s, buf[BUFSIZ];
	struct us_rsf u;
	long time();
	void free();
	int	mask;
 
	DEBUG(6, "Enter us_crs, cfile: %s\n", cfile);
	clear(&u,sizeof(u));
	mask = umask(0);
	if ((fq = fopen(R_stat, "a+")) == NULL) {
		DEBUG(3, "fopen of %s failed\n", s);
		umask(mask);
		return(FAIL);
	}

	umask(mask);
	/*
	 * manufacture a new entery
	 */
	name = cfile + strlen(cfile) - 4;
	u.jobn = atoi(name);
	u.qtime = u.stime = time((long *) 0);
	u.ustat = USR_QUEUE;
	strncpy(u.user, User, SYSNSIZE);
	strncpy(u.rmt, Rmtname, SYSNSIZE);
	fwrite(&u, sizeof(u), 1, fq);
	fflush(fq);
	fclose(fq);
	return(0);
}
clear(p, c)
register char *p;
register int c;
{
	register i;

	for(i=0;i<c;i++)
		*p++ = 0;
}
#endif
