char _Origin_[] = "System V";
/* @(#)uustat.c	1.8 */
static	char	*Sccsid = "@(#)$Header: /d2/3.7/src/usr.bin/uucp/RCS/uustat.c,v 1.1 89/03/27 18:31:03 root Exp $";
/*
 * $Log:	uustat.c,v $
 * Revision 1.1  89/03/27  18:31:03  root
 * Initial check-in for 3.7
 * 
 * Revision 1.2  86/07/25  17:01:38  paulm
 * Use opendir/readdir to access directories.
 * 
 * Revision 1.1  86/06/09  11:41:28  paulm
 * Initial revision
 * 
 * Revision 1.6  85/02/07  22:16:53  bob
 * Fixed bug introduced in last rev: this should be 6; made a defined constant
 * to avoid future confusion.
 * 
 * Revision 1.5  85/02/07  21:35:33  bob
 * Fixed 8 char sys name bugs
 * 
 */

#include "uucp.h"
#ifdef UUSTAT
#include <time.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/stat.h>
#include <dirent.h>
#include "uust.h"
#define	rid	0		/* user id for super user */
#define	SUFSIZE	6		/* chars for suffix */
 
/*
 * system status text
 */
char *us_stext[] = {
	"CONVERSATION SUCCEEDED",
	"BAD SYSTEM",
	"WRONG TIME TO CALL",
	"SYSTEM LOCKED",
	"NO DEVICE AVAILABLE",
	"DIAL FAILED",
	"LOGIN FAILED",
	"HANDSHAKE FAILED",
	"STARTUP FAILED",
	"CONVERSATION IN PROGRESS",
	"CONVERSATION FAILED",
	"CALL SUCCEEDED"
	};
/*
 * request status text
 */  
char *us_rtext[] = {
	"STATUS UNKNOWN: SYSTEM ERROR",
	"COPY FAIL",
	"LOCAL ACCESS TO FILE DENIED",
	"REMOTE ACCESS TO FILE DENIED",
	"A BAD UUCP COMMAND GENERATED",
	"REMOTE CAN'T CREATE TEMP FILE",
	"CAN'T COPY TO REMOTE DIRECTORY",
	"CAN'T COPY TO LOCAL DIRECTORY - FILE LEFT IN PUBDIR/USER/FILE",
	"LOCAL CAN'T CREATE TEMP FILE",
	"CAN'T EXECUTE UUCP",
	"COPY (PARTIALLY) SUCCEEDED",
	"COPY FINISHED, JOB DELETED",
	"JOB IS QUEUED",
	"JOB KILLED (COMPLETE)",
	"JOB KILLED (INCOMPLETE)"
	};
 
short vflag = 0;
int	octal;
 
/*
 * uustat --- A command that provides uucp status.
 */
#define O_COMP		01
#define O_JOB		02
#define O_MACH		04
#define O_REJ		010
#define O_KILL		020
#define O_USER		040
#define O_SYS		0100
#define O_OLD		0200
#define O_YNG		0400
#define O_ALL		01000
#define O_OCTAL		02000
#define O_QUEUE		04000
#define O_GROUP	(O_SYS|O_USER|O_OLD|O_YNG)
#define O_LMACH	010000
int	type;
int	uid;
short	ja;
short ca, ka, oa, ya;
char sa[SYSNSIZE+1], ua[SYSNSIZE+1], ma[SYSNSIZE+1];
char s[128], buf[BUFSIZ];
struct us_rsf u;
struct us_ssf ss;
DIR *pdirf;
int	catch();
int	set;
char	**Env;
long	ftell();
int	nsys;
main(argc, argv, envp)
char **argv;
char **envp;
int argc;
{
	register int c;
	register FILE *fp, *fq;
	extern char *optarg;
	extern int optind;
	int	ret;
	struct stat sb;
	long cnt, pos;
	void exit(), perror();
	char *strncpy();
	int	del	= 0;
 
	(void) tzset();		/* SGI: (^*~%!$@%^&. */
	Env = envp;
	uid = getuid();
	strcpy(Progname, "uustat");
	Pchar = 'S';
	while ((c=getopt(argc, argv, "x:c:j:O:s:k:u:r:o:y:m:M:vq")) != EOF ) 
		switch(c) {
		case 'x':
			Debug = atoi(optarg);
			break;
		case 'c':
			if(type&(O_MACH|O_KILL|O_JOB|O_REJ|O_GROUP))
				goto error;
			type |= O_COMP;
			ca = atoi(optarg);
			break;
		/*
		 * print status of job
		 */
		case 'j':
			if(type&(O_MACH|O_KILL|O_COMP|O_REJ|O_GROUP))
				goto error;
			type |= O_JOB;
			if (strcmp(optarg, "all") == SAME){ ja = -1;
				type |= O_ALL;
			}else	ja = atoi(optarg);
			break;
		case 'q':
			if(type&(O_JOB|O_GROUP|O_KILL|O_COMP|O_REJ))
				goto error;
			type |= O_QUEUE;
			break;
		/*
		 * print machine status
		 */
		case 'M':
		case 'm':
			if(type&(O_JOB|O_GROUP|O_KILL|O_COMP|O_REJ))
				goto error;
			type |= O_MACH;
			if(c == 'M')
				type |= O_LMACH;
			strncpy(ma, optarg, SYSNSIZE);
			ma[SYSNSIZE] = '\0';
			break;
		/*
		 * rejuvenate job
		 */
		case 'r':
			if(type&(O_MACH|O_KILL|O_JOB|O_COMP|O_OLD|O_YNG|O_USER))
				goto error;
			type |= O_REJ;
			if(strcmp(optarg, "all") == SAME)
				if(getuid() != 0){
					fprintf(stderr, "All for superuser only\n");
					exit(1);
				}else
					type |= O_ALL;
			ja = atoi(optarg);
			break;
		/*
		 * kill job
		 */
		case 'k':
			if(type&(O_JOB|O_MACH|O_COMP|O_REJ))
				goto error;
			type |= O_KILL;
			ka = atoi(optarg);
			break;
		/*
		 * print status of jobs belonging to user
		 */
		case 'u':
			if(type&(O_JOB|O_MACH|O_KILL|O_COMP|O_REJ))
				goto error;
			type |= O_USER;
			strncpy(ua, optarg, SYSNSIZE+1);
			ua[SYSNSIZE] = '\0';
			break;
		case 'o':
			if(type&(O_JOB|O_MACH|O_KILL|O_COMP|O_REJ))
				goto error;
			type |= O_OLD;
			oa = atoi(optarg);
			break;
		case 'y':
			if(type&(O_JOB|O_MACH|O_KILL|O_COMP|O_REJ))
				goto error;
			type |= O_YNG;
			ya = atoi(optarg);
			break;
		case 's':
			if(type&(O_JOB|O_MACH|O_KILL|O_COMP|O_REJ))
				goto error;
			type |= O_SYS;
			strncpy(sa, optarg, SYSNSIZE);
			sa[SYSNSIZE] = '\0';
			break;
		case 'v':
			vflag++;
			break;
		case 'O':
			if(type&(O_MACH|O_GROUP|O_KILL|O_COMP|O_REJ))
				goto error;
			type |= O_OCTAL;
			if (strcmp(optarg, "all") == SAME) ja = -1;
			else	ja = atoi(optarg);
			break;
		case '?':
		error:
			 fprintf(stderr, "Usage: uustat [-j* -v]");
			 fprintf(stderr, " [-m*] [-k*] [-c*] [-v]\n");
			 fprintf(stderr, "\t\t[-r* -u* -s* -o* -y* -v]\n");
			 exit(2);
		}
 
	/*
	 * user: the current user name
 	 * remove entries in R_stat older than ca hours
	 * used only by "uucp" or "root"
	 */
	guinfo(getuid(), User, s);	
	if (type&O_REJ) {
		rejuv();
		exit(0);
	}
 
	if (type&O_COMP) {
 
		if ((strcmp(User,"uucp")!=SAME)&&(getuid()!=rid)) {
			fprintf(stderr,"Only uucp or root is allowed ");
			fprintf(stderr,"to use '-c' option\n");
			exit(1);
		}
		/*
		 * Create temp file to compress R_stat
		 */
		ret = chdir(Spool);
		ASSERT(ret == 0, "CANNOT CHDIR TO - ", Spool, ret);
		if(ret != 0) {
			DEBUG(1, "No Spool - %s\n", Spool);
			exit(0);
		}
		DEBUG(5, "enter clean mode, ca: %d\n", ca);
		sprintf(s, "%s/%s.%.7d",Spool,"rstat",getpid());
		DEBUG(5, "temp file: %s\n", s);
		if ((fq=fopen(s, "w+")) == NULL) {
			fprintf(stderr, "cannot open temp file\n");
			return(FAIL);
		}
		chmod(s, 0666);

		/*
		 * Create lock file to control access
		 */
		signal(SIGHUP, catch);
		signal(SIGINT, catch);
		signal(SIGQUIT, catch);
		set++;
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
		if ((fp=fopen(R_stat, "r")) == NULL) {
			fprintf(stderr, "cannot open %s\n", R_stat);
			fclose(fq);
			unlink(s);
			rmlock(LCKRSTAT);
			return(FAIL);
		}
		while(fread(&u, sizeof(u), 1, fp) != NULL) {
			if (older(u.stime, ca))	continue;
			else 
				fwrite(&u, sizeof(u), 1, fq);
		}
		fclose(fp);
		fclose(fq);
		if (xmv(s, R_stat) == FAIL)
			fprintf(stderr, "mv fails in uustat: %s\n", "-c");
		rmlock(LCKRSTAT);
		return(0);
	}
 
	if (type&O_MACH) {		/* print machine status */
		if ((fp=fopen(L_stat, "r")) == NULL) {
			fprintf(stderr, "cannot open %s\n", L_stat);
			return(FAIL);
		}
		if (strcmp(ma,"all") == SAME) {
			while(fread(&ss, sizeof(ss), 1, fp) != NULL){
				sout(&ss);
			}
			fclose(fp);
			return(0);
		}
		while(fread(&ss, sizeof(ss), 1, fp) != NULL){
			if (strncmp(ma, ss.sysname, SYSNSIZE) == SAME) {
				sout(&ss);
				fclose(fp);
				return(0);
			}
		}
		fprintf(stderr, "system %s or its status unknown\n", ma);
		fclose(fp);
		return(1);
	}
 
	if(type&O_QUEUE){
		queue();
		exit(0);
	}
	/*
	 * Kill the job 'ka' and remove the Command file
	 * from spool directory. 
	 * Remove the entry from R_stat file and compact
	 * the hole. 
	 */
	if (type&O_KILL) {
		ret = chdir(Spool);
		ASSERT(ret == 0, "CANNOT CHDIR TO - ", Spool, ret);
		if(ret != 0) {
			DEBUG(1, "No Spool - %s\n", Spool);
			exit(0);
		}
		signal(SIGHUP, catch);
		signal(SIGINT, catch);
		signal(SIGQUIT, catch);
		kill(ka);
		exit(0);
	}
 
	if ((fp=fopen(R_stat, "r+")) == NULL) {
		fprintf(stderr, "fopen of %s failed\n",R_stat);
		return(FAIL);
	}
	if(stat(R_stat, &sb) == -1){
		fprintf(stderr, "Can't stat %s\n", R_stat);
		exit(1);
	}
	cnt = (sb.st_size + sizeof(u) - 1)/sizeof(u);
	pos = sb.st_size;
	if(cnt){
	fseek(fp, pos-sizeof(u), 0);
DEBUG(5,"cnt %d\n",cnt);
DEBUG(5,"tell %ld\n",ftell(fp));
	c = 0;
	while(cnt){
		c++;
		if(fread(&u, sizeof(u), 1, fp) == NULL)
			break;
		fseek(fp, pos-(long)(sizeof(u)) - (long)(c*sizeof(u)), 0);
		cnt--;
		DEBUG(5, "user: %s ", u.user);
		DEBUG(5, " User: %s\n", User);
		DEBUG(5, "tell %ld\n", ftell(fp));
		DEBUG(5, "tell %ld\n", pos);
		DEBUG(5, "tell %ld\n", pos);

		/*
		 * print request status of job# 'ja'
		 */
		if(type&O_JOB) {
			if ((ja==-1) || (ja==u.jobn))	jout(&u);
		}

		else if (type&O_GROUP) {
			if ((((type&O_USER) == 0) || (strcmp(u.user,ua) == SAME )) &&
			    (((type&O_SYS) == 0) || (strncmp(u.rmt, sa, SYSNSIZE) == SAME )) &&
			    (((type&O_OLD) == 0) || older(u.qtime, oa)) &&
			    (((type&O_YNG) == 0) || !older(u.qtime, ya)) )	jout(&u);
		}
 
		/* no option is given to "uustat" command,
		 * print the status of all jobs issued by
		 * the current user	
		 */
		else if (strcmp(u.user, User) == SAME)
			jout(&u);
	}
	}
	fclose(fp);
	return(0);
}
catch()
{
	rmlock(CNULL);
}
 
/*
 * print a record of us_ssf in l_stat file
 */

sout(s)
struct us_ssf *s;
{
	register struct tm *tp;
	struct tm *localtime();
 
	tp = localtime(&s->sti);
	printf("%.7s\t%2.2d/%2.2d-%2.2d:%2.2d",s->sysname,tp->tm_mon+1,
		tp->tm_mday, tp->tm_hour, tp->tm_min);
	if((type&O_LMACH) && s->sucti){
		tp = localtime(&s->sucti);
		printf("\t%2.2d/%2.2d-%2.2d:%2.2d",tp->tm_mon+1,
			tp->tm_mday, tp->tm_hour, tp->tm_min);
	}
	printf("\t%s\n",us_stext[s->sstat]);
	return(0);
}
 
/*
 * print one line of job status in "u"
 */
jout(u)
struct us_rsf *u;
{
	register i, j;
	register struct tm *tp;
	struct tm *localtime();
	int	ov;
 
	tp = localtime(&u->qtime);
	printf("%-4.4d %.7s %.7s",u->jobn,u->user,u->rmt);
	printf(" %2.2d/%2.2d-%2.2d:%2.2d", tp->tm_mon+1, tp->tm_mday,
		tp->tm_hour, tp->tm_min);

	/*
	 * status time
	 */
	tp = localtime(&u->stime);	
	printf(" %2.2d/%2.2d-%2.2d:%2.2d", tp->tm_mon+1, tp->tm_mday,
		tp->tm_hour, tp->tm_min);
	if(type&O_OCTAL){
		printf(" %.6o\n",u->ustat);
		return(0);
	}
	if(u->ustat){
		ov = 0;
		if(u->ustat == (USR_COMP|USR_COK))
			printf(" %s",us_rtext[11]);
		else
		for (j=1, i=u->ustat; i>0; j++, i=i>>1) {
			if (i&01){ 
				if(ov > 1 && ((ov-1)%3 == 0))
					printf("\n");
				printf(" %s",us_rtext[j]);
				if(ov++ == 0)
					printf("\n");
			}
		}
	}
	if(ov != 1)
		printf("\n");
	return(0);
}
 
 

/*
 *	rt 	-> request time
 *	t	-> hours
 * return:
 *	1	-> if rt older than current time by t hours or more.
 */
older(rt,t)
short t;
time_t rt;
{
	time_t ct, time();		/* current time */
	time(&ct);
	return ((ct-rt) > (t*3600L));
}

cleanup(code) {
	exit(code);
}
rejuv()
{
	register int n;
	char *name, file[100];
	long	tm;
	char	snam[10];
	register i;
	int	ret;
	int	flg;

	/*
	 * for spool directory
	 */
	flg = 0;
	ret = chdir(Spool);
	ASSERT(ret == 0, "CANNOT CHDIR TO - ", Spool, ret);
	if(ret != 0) {
		DEBUG(1, "No Spool - %s\n", Spool);
		exit(0);
	}
	/*
	 * delete the command file from spool dir.
	 * spool directory
	 */
	if ((pdirf = opendir(Spool)) == NULL) {
		perror(Spool);
		return(FAIL);
	}
	/*
	 * get next file name from 
	 * directory 'pdirf'
	 */
	time(&tm);
	while (gnamef(pdirf, file) == TRUE) {
		if((n = strlen(file)) <= 4)
			continue;
		name = file + n - 4;
		n = atoi(name);
		if(uid == rid && (type&O_SYS)){
			for(i=0;i<14;i++){
				if(file[i] == '\0')
					break;
				if(file[i] == '.'){
					strncpy(snam, file[i+1], 5);
					if(strncmp(snam, sa, SYSNSIZE) == SAME)
						goto rej;
				}
			}
			continue;
		}
		if ((type&O_ALL) || (n == ja)) {
rej:
DEBUG(5, "Update %s\n", file);
			flg++;
			if(utime(file, 0) == -1)
				fprintf(stderr,"Can't modify time for %s\n",file);
		}
	}
	if(flg == 0)
		fprintf(stderr,"Can't find job\n");
	closedir(pdirf);
	return(0);
}
struct q_dir{
	char	q_sys[8];
	time_t	q_otime;
	time_t	q_ltime;
	int	q_num;
	int	q_cnum;
	time_t	q_lck;
};
struct q_dir *qp;
struct q_dir *lookup();
#define NSYS	100
queue()
{
	FILE *pdf;
	char	f[15];
	char	ff[15];
	register i;
	int	ret;
	int	compar();
	struct stat sb;
	register struct q_dir *q;
	long	time();
	char	*p;
	char	*ctime();
	int	len;

	clear(f, sizeof(f));
	if((qp = (struct q_dir *)calloc(sizeof(struct q_dir), NSYS)) == NULL){
		fprintf(stderr, "Can't allocate storage for queue\n");
		exit(1);
	}
	clear(qp,sizeof(struct q_dir)*NSYS);
	ret = chdir(Spool);
	ASSERT(ret == 0, "CANNOT CHDIR TO - ", Spool, ret);
	if(ret != 0) {
		DEBUG(1, "No Spool - %s\n", Spool);
		exit(0);
	}

	if ((pdf=fopen(Spool,"r"))==NULL) {
		perror(Spool);
		return(FAIL);
	}
	while(gnamef(pdf, f)){
		if(f[1] == '.'){
			strncpy(ff, f, sizeof(f));
			len = strlen(f);
			if(len > SUFSIZE){
				f[len-SUFSIZE] = '\0';
			}
			if(f[0] == 'o')
				continue;
			if((q = lookup(&f[2])) == NULL)
				continue;
			if(f[0] == CMDPRE){
				q->q_cnum++;
			}else
				q->q_num++;
			if(stat(ff, &sb) == -1)
				continue;
			if(q->q_otime == 0L){
				q->q_ltime = q->q_otime = sb.st_mtime;
			}
			if(sb.st_mtime < q->q_otime){
				q->q_otime = sb.st_mtime;
			}
			if(sb.st_mtime > q->q_ltime){
				q->q_ltime = sb.st_mtime;
			}
			continue;
		}
		if(strncmp(f, "LCK..", 5) == SAME){
			if((q = lookup(&f[5])) == NULL)
				continue;
			if(stat(f, &sb) == -1)
				continue;
			q->q_lck = sb.st_mtime;
		}
	}
	qsort(qp, nsys, sizeof(struct q_dir), compar);
	fprintf(stdout,"System     # jobs #file          earliest            latest                lock\n");
	q = qp;
	for(i=0;i<NSYS;i++,q++){
		if(q->q_sys[0] == '\0')
			break;
		fprintf(stdout,"%-*.*s     %4d  %4d %.2s%.16s",
			SYSNSIZE,
			SYSNSIZE,
			q->q_sys,q->q_cnum,q->q_num,
			(q->q_otime &&
			(q->q_otime+(24*3600L) <= time((long *)0)))?"**":"  ",
			q->q_otime?(p = ctime(&q->q_otime) + 4)
			:"                    ");
		fprintf(stdout,"  %.16s ",
			(q->q_ltime&&(q->q_otime != q->q_ltime))?(p = ctime(&q->q_ltime) + 4)
			:"                     ");
		fprintf(stdout," %.2s",
			(q->q_lck &&
			(q->q_lck + 3600L <= time((long *)0)))?"**":"  ");
		fprintf(stdout,"%.16s\n",
			q->q_lck?(p = ctime(&q->q_lck) + 4):"");
	}
}
clear(p,c)
register char *p;
register int c;
{
	register int i;

	for(i=0;i<c;i++)
		*p++ = 0;
}
compar(p, q)
struct q_dir *q, *p;
{

	if(q->q_cnum < p->q_cnum)
		return(-1);
	return(1);
}
struct q_dir *
lookup(sys)
register char	*sys;
{
	register i;
	register struct q_dir *q;

	q = qp;
	for(i=0;i<NSYS;i++,q++){
		if(q->q_sys[0] == '\0')
			break;
		if(strncmp(sys, q->q_sys, SYSNSIZE) == SAME)
			return(q);
	}
	if(i >= NSYS)
		return(NULL);
	strcpy(q->q_sys, sys);
	q->q_otime = 0L;
	q->q_num = 0;
	q->q_cnum = 0;
	q->q_lck = 0L;
	nsys++;
	return(q);
}
 
#else
main(argc, argv)
char **argv;
{
	fprintf(stderr,"Uustat command does not exist on your system\n");
}
#endif
