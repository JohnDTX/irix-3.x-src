/* @(#)us_rrs.c	1.6 */
#include "uucp.h"
#ifdef UUSTAT
#include <sys/types.h>
#include "uust.h"
 
/*
 * We get the job number from a command file "cfile".
 * using the jobn as the key to search thru "R_stat"
 * file and modify the corresponding status as indicated
 * in "stat".	"Stat" is defined in "uust.h".
 * return:
 *	0	-> success
 *	FAIL	-> failure
 */
long	ftell();
us_rrs(cfilel,stat)
char *cfilel;
short stat;
{
	FILE	*fp;
	register short i;
	struct us_rsf u;
	char cfile[20], *strrchr(), *lxp, *name, buf[BUFSIZ];
	char *strcpy();
	long	pos;
	long time();
	short n;
	int	mask;
 
	/*
	 * strip path info
	 */
	strcpy(cfile, (lxp=strrchr(cfilel, '/'))?lxp+1:cfilel);	
	DEBUG(9, "\nenter us_rrs, cfile: %s", cfile);
	DEBUG(9, "  request status: %o\n", stat);
	
	/*
	 * extract the last 4 digits
	 * convert to digits
	 */
	name = cfile + strlen(cfile) - 4;  
	n = atoi(name);		
	mask = umask(0);
	for(i=0; i<=15; i++) {
		if (ulockf(LCKRSTAT, 15) != FAIL) 
			break;
		sleep(1);
	}
	if (i > 15) {
		DEBUG(3, "ulockf of %s failed\n", LCKRSTAT);
		umask(mask);
		return(FAIL);
	}
	if ((fp = fopen(R_stat, "r+")) == NULL) {
		DEBUG(3, "fopen of %s failed\n", R_stat);
		rmlock(LCKRSTAT);
		umask(mask);
		return(FAIL);
	}
	umask(mask);
	while(fread(&u, sizeof(u), 1, fp) != NULL){
		if (u.jobn == n) {
			DEBUG(6, " jobn : %d\n", u.jobn);

			pos = ftell(fp);
			u.ustat = stat;
			fseek(fp, pos-(long)sizeof(u), 0);
			fwrite(&u, sizeof(u), 1, fp);
			break;
		}

	}
	fflush(fp);
	fclose(fp);
	rmlock(LCKRSTAT);
	return(FAIL);
}
#endif
