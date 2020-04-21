#
#include "uucp.h"
#include <time.h>
#include <sys/types.h>
#include <signal.h>
#include <dirent.h>
#include "uust.h"
struct us_rsf uu;
DIR *pdf;
delete(no)
{
	char	f[20];
	char	*name;
	register int n;
	int	v;

	v = USR_QUEUE;
	fseek(pdf, 0L, 0);
	while (gnamef(pdf, f)) {
		if(strlen(f) <= 4)
			continue;
		if(f[1] != '.')
			continue;
		name = f + strlen(f) - 4;
		n = atoi(name);
		if (n == no) {
DEBUG(5, "KILL %s\n",f);
			if (f[0] == CMDPRE){
				v = USR_KCOMP;
			}
			if(v == USR_QUEUE)
				v = USR_KINC;
			if(unlink(f) == -1){
				fprintf(stderr,"Can't unlink %s\n",f);
			}
		}
	}
	return(v);
}
int	sigz;
int	getit();
kill(jn)
{
	FILE *fp, *fq;
	register int n;
	int	fnd;
	char *name, file[100];
	int	ret, val;
	char	b[BUFSIZ];
	char	s[200];
	register i;
	int	sqit, shup, sint;

	sigz = 0;
	fnd = 0;
	sigz= 1;


	{
		register int i;
		for(i=0; i<=15; i++) {
			if (ulockf(LCKRSTAT, 15) != FAIL) break;
			sleep(1);
		}
		if (i > 15) {
			fprintf(stderr, "cannot lock %s\n", LCKRSTAT);
			fclose(fq);
			unlink(s);
			return(FAIL);
		}
	}
	shup = (int)signal(SIGHUP, getit);
	sint = (int)signal(SIGINT, getit);
	sqit = (int)signal(SIGQUIT, getit);
	if ((fp=fopen(R_stat, "r")) == NULL) {
		fprintf(stderr, "cannot open %s\n", R_stat);
		rmlock(LCKRSTAT);
		fclose(fq);
		unlink(s);
		sigz = 0;
		signal(SIGHUP, shup);
		signal(SIGINT, sint);
		signal(SIGQUIT, sqit);
		return(FAIL);
	}
	/*
	 * delete the command file from spool dir.
	 * spool directory
	 */
	if ((pdf=opendir(Spool))==NULL) {
		perror(Spool);
		rmlock(LCKRSTAT);
		fclose(fp);
		fclose(fq);
		unlink(s);
		sigz = 0;
		signal(SIGHUP, shup);
		signal(SIGINT, sint);
		signal(SIGQUIT, sqit);
		return(FAIL);
	}
	while(fread(&uu, sizeof(uu), 1 , fp) != NULL) {
	if ((uu.jobn==jn) && ((strcmp(uu.user,User)==SAME)
			    || (getuid()==0)))  {
		if((uu.ustat&(USR_KINC|USR_KCOMP)) == 0){
		DEBUG(5, "Job %d is deleted\n", jn);
		if((uu.ustat&USR_COMP) == 0){
			val = delete(jn);
			if(val == USR_QUEUE){
				val = USR_KINC;
				fprintf(stderr,"Job %hd not found\n",uu.jobn);
			}
			fnd++;
			uu.ustat = val;
			fseek(fq,(long)(-sizeof(uu)), 1);
		}else{
			fprintf(stderr,"job %hd already complete\n",uu.jobn);
		}
		}
	}
	fwrite(&uu, sizeof(uu), 1, fq);
	}
	if(fnd == 0){
		val = delete(jn);
		if(val == USR_QUEUE)
			fprintf(stderr,"Job %hd not found\n",jn);
	}
	closedir(pdf);
	fclose(fq);
	rmlock(LCKRSTAT);
	sigz = 0;
	signal(SIGHUP, shup);
	signal(SIGINT, sint);
	signal(SIGQUIT, sqit);
	return(0);

}
getit()
{
	rmlock(CNULL);
}
