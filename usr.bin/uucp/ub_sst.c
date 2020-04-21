/* @(#)ub_sst.c	1.4 */
#include "uucp.h"
#ifdef UUSUB
#include <sys/types.h>
#include "uusub.h"
 
/*
 * searches thru L_sub file using "rmtname"
 * as the key.
 * If the entry is found, then modify the connection
 * status as indicated in "flag" and return.
 * return:
 *	0	-> success
 *	FAIL	-> failure
 */
ub_sst(flag)
register short flag;
{
	register FILE *fp;
	register int i;
	struct ub_l l;
	time_t time();
 
	DEBUG(9, " enter ub_sst, status is : %d\n", flag);
	DEBUG(9,"Rmtname: %s\n", Rmtname);
	for(i=0; i<=15; i++) {
		if (ulockf(LCKLSUB, 15) != FAIL) 
		break; 
		sleep(1);
	}
	if (i > 15) {
		DEBUG(3, "ulockf of %s failed\n", LCKLSUB);
		return(FAIL);
	}
	if ((fp = fopen(L_sub, "r+")) == NULL) {
		DEBUG(3, "fopen of %s failed\n", L_sub);
		rmlock(LCKLSUB);
		return(FAIL);
	}
 
	while (fread((char *) &l, sizeof(l), 1, fp) == 1)
		if (strncmp(l.sys, Rmtname, SYSNSIZE) == SAME) {
		  switch(flag) {
			case ub_ok:	
				l.ok++;
				time(&l.oktime);
				break;
			case ub_noacu:	
				l.noacu++;
				break;
			case ub_login:	
				l.login++;
				break;
			case ub_nack:	
				l.nack++;
				break;
			default:	
				l.other++;
				break;
		  }

			l.call++;
			DEBUG(6, "in ub_sst name=Rmtname: %s\n", l.sys);
			fseek(fp, -(long)sizeof(l), 1);
			fwrite((char *) &l, sizeof(l), 1, fp);

			/*
			 * go to exit
			 */
			break;		
		}

	/*
	 * exit point
	 */
	fclose(fp);		
	rmlock(LCKLSUB);
	return(0);
}
#endif
