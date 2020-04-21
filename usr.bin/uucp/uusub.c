char _Origin_[] = "System V";

/* @(#)uusub.c	1.5 */
#include "uucp.h"
#ifdef UUSUB
#include <time.h>
#include <sys/types.h>
#include "uusub.h"
#define	rid	0		/* user id for super user */
char cno[] = "uusub: cannot open ";

/*
 * command for monitoring uucp subnetwork
 * Probably should be limited to 'root', 'daemon', 'uucp'.
 */
long	timeval;
long	timeod();
char	**Env;
main(argc, argv, envp)
char **argv;
char **envp;
int argc;
{
	FILE *fp, *fq;
	extern char *optarg;
	extern int optind;
	struct ub_l l;
	struct ub_r r;
	short aflag,dflag,rflag,lflag,fflag,cflag,uflag;
	short ua, c;
	char aa[10], da[10], ca[10], s[128];
	char *strcpy();
	time_t time();
	void exit();
	int	fcnt, status;
 
	(void) tzset();		/* SGI: (^*~%!$@%^&. */
	Env = envp;
	strcpy(Progname, "uusub");
	Pchar = 'S';
	aflag=dflag=rflag=lflag=fflag=cflag=uflag=0;
	while ((c=getopt(argc, argv, "x:a:d:c:u:rlf")) != EOF )
		switch(c) {
		case 'x':
			Debug = atoi(optarg);
			break;
		case 'a':
			aflag++;
			strcpy(aa, optarg);
			break;
		case 'd':
			dflag++;
			strcpy(da, optarg);
			break;
		case 'c':
			cflag++;
			strcpy(ca, optarg);
			break;
		case 'u':
			uflag++;
			ua = atoi(optarg);
			break;
		case 'r':
			rflag++;
			break;
		case 'l':
			lflag++;
			break;
		case 'f':
			fflag++;
			break;
		case '?':
			fprintf(stderr,"Usage:uusub -a* -d* -c* -u* -r -l-f\n");
			exit(2);
		}
 
	/*
	 * flush L_sub file
	 */
	if (fflag) {	
		DEBUG(6, "Flush file %s\n", L_sub);

		/*
		 * 2nd argument is irrelevant
		 */
		edit('f',"");	
	}
 
	if (dflag) {	/* delete system "da" from "L_sub" */
		DEBUG(6, "Delete system %s from subnetwork\n",da);
		edit('d',da);
	}
 
	/*
	 * add 'aa' to L_sub file
	 */
	if (aflag) {	
		DEBUG(6, "Add system %s to subnetwork\n",aa);
		if ((fp=fopen(L_sub, "a+")) == NULL) {
			fprintf(stderr, "%s%s\n", cno, L_sub);
			return(FAIL);
		}

		/*
		 * rewind
		 */
		fseek(fp, 0L, 0);	
		while (fread((char *) &l, sizeof(l), 1, fp)!=NULL)
			if (strncmp(l.sys, aa, SYSNSIZE)==SAME) goto out;
		DEBUG(6, "system %s is added\n", aa);

		/*
		 * manufactrue a new entry
		 */
		strcpy(l.sys, aa);	
		time(&l.oktime);
		l.call = l.ok = l.noacu = l.login = l.nack = l.other = 0;
		fwrite((char *) &l,sizeof(l),1,fp);
	  out:  fclose(fp);
		unlink(LCKLSUB);
	}
 
	/*
	 * call system 'ca'
	 */
	if (cflag) {	
		if (strcmp(ca, "all") != SAME)
			xuucico(ca);
		/*
		 * call all system
		 */
		else {
			if ((fp=fopen(L_sub, "r")) == NULL) {
				fprintf(stderr, "%s%s\n", cno, L_sub);
				return(FAIL);
			}
			fcnt = 0;
			while (fread((char *) &l,sizeof(l),1,fp)==1){
				xuucico(l.sys);
				if(fcnt++ >= 5){
					wait(&status);
					fcnt--;
				}
			}
			fclose(fp);
		}
	}
 
	/*
	 * update R_sub file from SYSLOG
	 */
	if (uflag) {	
		update();
	}
	/*
	 * print the R_sub file
	 */
	if (rflag) {
		if ((fp=fopen(R_sub, "r")) == NULL) {
			fprintf(stderr, "%s%s\n", cno, R_sub);
			return(FAIL);
		}
		printf("sysname\tsfile\tsbyte\trfile\trbyte\n");
		while (fread((char *) &r, sizeof(r), 1, fp)!=NULL)
			printf("%s\t%d\t%ld\t%d\t%ld\n",r.sys,r.sf,r.sb,r.rf,r.rb);
		fclose(fp);
	}
 
	/*
	 * print the L_sub fie
	 */
	if (lflag) {	/* print the L_sub file */
		struct tm *tp, *localtime();
 
		if ((fp=fopen(L_sub, "r")) == NULL) {
			fprintf(stderr, "%s%s\n", cno, L_sub);
			return(FAIL);
		}
		printf("sysname\t\#call\t\#ok\tlatest-oktime\t\#noacu\t\#login");
		printf("\t\#nack\t\#other\n");
		while (fread((char *) &l, sizeof(l), 1, fp)!=NULL) {
			tp=localtime(&l.oktime);
			printf("%s\t%d\t%d", l.sys,l.call,l.ok);
			printf("\t(%d/%d-%d:%2.2d)\t", tp->tm_mon+1,
			        tp->tm_mday, tp->tm_hour, tp->tm_min);
			printf("%d\t%d\t%d\t%d\n",l.noacu,l.login,l.nack,l.other);
		}
		fclose(fp);
	}
	return(0);
}
 
/*
 * edit "L_sub" file
 */
edit(flag,arg)	
short flag;
char *arg;
{
	FILE *fq, *fp;
	struct ub_l l;
	char s[64];
 
	DEBUG(6, "enter edit, flag: %d\n",flag);
	sprintf(s, "%s/%s.%.7d",Spool,"lsub",getpid());
	if ((fq=fopen(s, "w")) == NULL) {
		fprintf(stderr, "%s%s\n", cno, s);
		return(FAIL);
	}
	if ((fp=fopen(L_sub, "r")) == NULL) {
		fprintf(stderr, "%s%s\n", cno, L_sub);
		return(FAIL);
	}
	while (fread((char *) &l, sizeof(l), 1, fp)!=NULL)
	 switch(flag) {
	 case 'f':
		l.call = l.ok = l.noacu = l.login = l.nack = l.other = 0;
		fwrite((char *) &l,sizeof(l),1,fq);
		break;
	 case 'd':
		if (strncmp(l.sys, arg, SYSNSIZE)!=SAME) {
			DEBUG(7,"%s is retained\n",l.sys);
			fwrite((char *) &l,sizeof(l),1,fq);
		}
		break;
	 }
	fclose(fp);
	fclose(fq);
	if ((fp=fopen(L_sub, "w")) == NULL) {
		fprintf(stderr, "%s%s\n", cno, L_sub);
		return(FAIL);
	}
	if ((fq=fopen(s, "r")) == NULL) {
		fprintf(stderr, "%s%s\n", cno, s);
		return(FAIL);
	}
	while (fread((char *) &l, sizeof(l), 1, fq)!=NULL) {
		fwrite((char *) &l,sizeof(l),1,fp);
	}
	fclose(fp);
	fclose(fq);
	unlink(LCKLSUB);
	unlink(s);
	unlink("dummy");
	return(0);
}
#else
main(argc, argv)
char **argv;
{	fprintf(stderr, "Uusub is not implemented on this system\n");
}
#endif
update()
{
	FILE *fp, *fq;
	struct ub_l l;
	struct ub_r r;
	char rmt[10], s_r[10];
	char	s[512];
	long bytes, tmv;
	short	ua;
	time_t tick, oldtick, dtick, utick;
	int	sec, min, hr, month, day, yr;
	int	pid, flag;
	struct tm *tp, *localtime();
	register char *p, *q;

	if ((fp=fopen(L_sub, "r")) == NULL) {
		fprintf(stderr, "%s%s\n", cno, L_sub);
		return(FAIL);
	}
	if ((fq=fopen(R_sub, "w")) == NULL) {
		fprintf(stderr, "%s%s\n", cno, R_sub);
		return(FAIL);
	}
	/*
	 * rewind
	 */
	fseek(fq, 0L, 0);

	/*
	 * Flush R_sub file
	 */
	DEBUG(6," Flush file %s\n", R_sub);
	r.sf = r.rf = (long) 0;
	r.sb = r.rb = 0;
	while(fread((char *) &l,sizeof(l),1,fp)==1) {
		DEBUG(6," l.sys: %s\n", l.sys);
		strcpy(r.sys, l.sys);
		fwrite((char *) &r,sizeof(r),1,fq);
	}

	fclose(fp);
	fclose(fq);
	unlink(LCKRSUB);
	if ((fp=fopen(SYSLOG, "r")) == NULL) {
		fprintf(stderr, "%s%s\n", cno, SYSLOG);
		return(FAIL);
	}
	if ((fq=fopen(R_sub,"r+")) == NULL) {
		fprintf(stderr, "%s%s\n", cno, R_sub);
		return(FAIL);
	}
	/*
	 * get current time
	 */
	time(&tick);
	utick = ua*3600L;
	while (fgets(s,sizeof(s),fp)!=NULL) {
		sscanf(s,"%s %*s (%d/%d-%d:%d:%d) ",
			rmt, &month, &day, &hr, &min, &sec); 
		for(p=rmt;*p;p++)
			if(*p == '!')
				*p = '\0';
		flag = 0;
		for(p=s;*p;p++)
			if(*p == '<'){
				flag = 1;
				p++;
				break;
			}else
			if(*p == '>'){
				flag = 2;
				break;
			}
		if(*p)
			p++;
		if(flag)
			sscanf(p, "%ld",&bytes);
		if(fgets(s,sizeof(s),fp) == NULL)
			break;
		if(flag == 0)
			continue;
		DEBUG(9," Rmt: %s ",rmt);
		DEBUG(9," s_r: %s", s_r);
		DEBUG(9," bytes: %ld ",bytes);
		oldtick = timeod(-1, month, day, hr, min, sec);
		oldtick += timezone;
		DEBUG(9," oldtick: %ld\n",oldtick);
		dtick = tick - oldtick;
		if(dtick < utick)
			continue;
		DEBUG(8, " %s inside the time\n",rmt);

		/*
		 * rewind
		 */
		fseek(fq, 0L, 0);
		while (fread((char *) &r,sizeof(r),1,fq)==1){
			if (strncmp(r.sys, rmt, SYSNSIZE)==SAME) {
				if(flag == 1){
					r.sf++;
					r.sb += bytes;
				} else {
					r.rf++;
					r.rb += bytes;
				}
				fseek(fq, - (long) sizeof(r), 1);
				fwrite((char *) &r,sizeof(r),1,fq);
				DEBUG(7,"%s is written\n",r.sys);

				break;	
			}
		}
	}
	fclose(fp);
	fclose(fq);
	unlink(LCKRSUB);
}
closelog()
{
}
int dmsize[12]={31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
#define	dysize(A) (((A)%4)? 365: 366)
/*
 * Convert a time specification to a long value
 */
long	timbuf;
long
gtime(yr, mo, day, hr, min, sec)
{
	register int i;
	long nt;

	tzset();

	if(mo<1 || mo>12)
		return(0L);
	if(day<1 || day>31)
		return(0L);
	if(hr == 24) {
		hr = 0;
		day++;
	}
	if(min<0 || min>59)
		return(0L);
	if (yr<0) {
		(void) time(&nt);
		yr = localtime(&nt)->tm_year;
	}
	if (hr<0 || hr>23)
		return(0L);
	timbuf = 0;
	yr += 1900;
	for(i=1970; i<yr; i++)
		timbuf += dysize(i);
	/* Leap year */
	if (dysize(yr)==366 && mo >= 3)
		timbuf += 1;
	while(--mo)
		timbuf += dmsize[mo-1];
	timbuf += (day-1);
	timbuf *= 24;
	timbuf += hr;
	timbuf *= 60;
	timbuf += min;
	timbuf *= 60;
	timbuf += sec;
	return(timbuf);
}
long
timeod(yr, mo, da, hr, min, sec)
{
	timeval = gtime(yr-1900, mo, da, hr, min, sec);
	return(timeval);
}
