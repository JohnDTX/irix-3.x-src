/* @(#)us_sst.c	1.5 */
/* $Source: /d2/3.7/src/usr.bin/uucp/RCS/us_sst.c,v $*/
/* $Revision: 1.1 $*/
/* $Date: 89/03/27 18:30:47 $*/

#include "uucp.h"
#ifdef UUSTAT
#include <sys/types.h>
#include "uust.h"
 
/*
 * searches thru L_stat file using "rmtname"
 * as the key.
 * If the entry is found, then modify the system
 * status as indicated in "flag" and return.
 * return:
 *	0	-> success
 *	FAIL	-> failure
 */
us_sst(flag)
short flag;
{
	register FILE *fp;
	register short i;
	static struct us_ssf s = 0;
	long	ftell();
	long	pos;
	char buf[BUFSIZ];
	long time();
	int	mask;
 
	DEBUG(9, " enter us_sst, status is : %.2d\n", flag);
	mask = umask(0);
	for(i=0; i<=15; i++) {
		if (ulockf(LCKLSTAT, 15) != FAIL)
			break;
		sleep(1);
	}
	if (i > 15) {
		DEBUG(3, "ulockf of %s failed\n", LCKLSTAT);
		umask(mask);
		return(FAIL);
	}
	if ((fp = fopen(L_stat, "r+")) == NULL) {
		DEBUG(3, "fopen of %s failed\n", LCKLSTAT);
		rmlock(LCKLSTAT);
		umask(mask);
		return(FAIL);
	}
	umask(mask);
 
	while(fread(&s, sizeof(s), 1, fp) != NULL){
		DEBUG(9, "s.sysname : %.8s\n", s.sysname);
		if (strncmp(s.sysname, Rmtname, SYSNSIZE) == SAME) {
			pos = ftell(fp);
			fseek(fp,pos - sizeof(s), 0);
			goto out;
		}

	}
 
	strncpy(s.sysname, Rmtname, SYSNSIZE);
out:
	if(flag == US_S_OK)
		s.sucti = time((long *)0);
	s.sti = time((long *)0);
	s.sstat = flag;
	fwrite(&s, sizeof(s), 1, fp);
	fflush(fp);
	fclose(fp);
	rmlock(LCKLSTAT);
	return(0);
}
#endif
