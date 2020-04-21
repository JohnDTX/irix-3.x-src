char _Origin_[] = "System V";

/* @(#)uuclean.c	1.3 */
#include "uucp.h"
#include <signal.h>
#include <pwd.h>
#include <sys/types.h>
#include <time.h>
#include <sys/stat.h>
#include <dirent.h>



/*
 * search through the spool directory (Spool) and delete
 * all files with a requested prefix which are older than
 * (nomtime) secounds. If the -m option is set, the program 
 * will try to send mail to the usid of the file.
 * options:
 *	m	-> send mail for deleted file
 *	d	-> directory to clean
 *	n	-> time to age files before delete (in hours)
 *	p	-> prefix for search
 *	x	-> turn on debug outputs
 * exit:
 *	0	-> normal return
 *	1	-> can not read directory
 */
#define DPREFIX "U"
#define NOMTIME 72	/* hours to age files before deletion */

char *strcpy();
int	wrnf;

#define MAXPRE 25
char	Sys[MAXPRE][NAMESIZE];
int	Nsys = 0;
char	*Logfile;
char	*Mailfile;
char	*mf;
char	*xf;
int	onemf;
int	jjn;
char	**Env;
main(argc, argv, envp)
char *argv[];
char	**envp;
{
	register DIR *pdirf;
	struct stat stbuf;
	int	mflg=0,pflg=0;
	int	ret;
	char	msg[BUFSIZ];
	extern int onintr();
	extern int errno;
	time_t nomtime, ptime, time();
	void exit();
	char file[NAMESIZE];

	(void) tzset();		/* SGI: (^*~%!$@%^&. */
	Env = envp;
	strcpy(Progname, "uuclean");
	nstat.t_qtime = time((long *)0);
	Pchar = 'K';
	nomtime = NOMTIME * 3600L;

	while (argc>1 && argv[1][0] == '-') {
		switch (argv[1][1]) {
		case 'd':
			Spool = &argv[1][2];
			break;
		case 'm':
			mflg = 1;
			if(argv[1][2]){
				onemf++;
				Mailfile = &argv[1][2];
			}
			break;
		case 'n':
			nomtime = atoi(&argv[1][2]) * 3600L;
			break;

		/*
		 * Specify prefix(es) -pstring
		 * 	string		-> prefix
		 * 	""		-> null forces uuclean to operate on all 		 *			-> files
		 */
		case 'p':
			if(strcmp(&argv[1][2], "all") == SAME)
				pflg = 2;
			else{
				pflg = 1;
				stpre(&argv[1][2]);
			}
			break;
		case 's':
			stsys(&argv[1][2]);
			break;
		case 'x':
			Debug = atoi(&argv[1][2]);
			if (Debug <= 0)
				Debug = 1;
			break;
		case 'w':
			wrnf++;
			if(argv[1][2])
				Logfile = &argv[1][2];
			break;
		default:
			printf("unknown flag %s\n", argv[1]); break;
		}
		--argc;  argv++;
	}

	if(pflg == 0){
		pflg++;
		stpre("LCK");
		stpre("C");
		stpre("X");
		stpre("T");
		stpre("TM");
		stpre("D");
		stpre("STST");
		stpre("LTMP");
	}
	if ((pdirf = opendir(Spool)) == NULL) {
		printf("%s directory unreadable\n", Spool);
		exit(1);
	}
	DEBUG(4, "DEBUG# %s\n", "START");
	ret = chdir(Spool);
	ASSERT(ret == 0, "UUCLEAN:CANNOT CHDIR TO SPOOL - ", Spool, errno);


	/*
	 * Set LOGNAME so mail comes from uucp
	 */
	ret = guinfo(geteuid(), User, msg);
	setuucp(User);
	time(&ptime);
	while (gnamef(pdirf, file)) {
		if (Nsys && chksys(file) == FALSE)
			continue;
		if(pflg == 1)
		if (chkpre(file) == FALSE) 
			continue;
		if (stat(file, &stbuf) == -1) {
			DEBUG(4, "stat on %s failed\n", file);
			continue;
		}
		if ((stbuf.st_mode & S_IFMT) == S_IFDIR)
			continue;
		if ((ptime - stbuf.st_mtime) < nomtime)
			continue;
		if(wrnf){
			wrnent(file, "", 0, stbuf.st_mtime);
			continue;
		}
		if ((file[0] == CMDPRE) && (file[1] == '.')){
			jjn = atoi(&file[strlen(file) - 4]);
			notfyuser(file);
			kill(jjn);
		}
		DEBUG(4, "unlink file %s\n", file);
		if(unlink(file) == -1)
			continue;
		wrnent(file, "", 1, stbuf.st_mtime);
		if (mflg) 
			sdmail(file, (int) stbuf.st_uid);
	}

	closedir(pdirf);
	exit(0);
}


char Pre[MAXPRE][NAMESIZE];
int Npre = 0;

/*
 * check for prefix
 *	file	-> file name to check
 *
 *	return codes:
 *		FALSE	->  not prefix
 *		TRUE	->  is prefix
 */
chkpre(file)
register char *file;
{
	register int i;

	for (i = 0; i < Npre; i++) {
		if (prefix(Pre[i], file))
			return(TRUE);
	}
	return(FALSE);
}
chksys(file)
register char *file;
{
	register int i;

	if(Nsys == 0)
		return(TRUE);
	for (i = 0; i < Nsys; i++) {
		if (substr(Sys[i], file) == TRUE)
			return(TRUE);
	}
	return(FALSE);
}
substr(p, q)
register char *p,*q;
{
	register n;

	n = strlen(p);
	for(;*q;q++){
		if(*p == *q){
			if(strncmp(p,q,n) == SAME)
				return(TRUE);
		}
	}
	return(FALSE);
}

/*
 * store prefix
 *	p	-> prefix
 *
 *	return codes:  none
 */
stpre(p)
char	*p;
{
	if (Npre < MAXPRE)
		strncat(Pre[Npre++], p, MAXBASENAME);
	return;
}
/*
 * store system name
 *	p	-> system
 *
 *	return codes:  none
 */
stsys(p)
char	*p;
{
	if (Nsys < MAXPRE)
		strncat(Sys[Nsys++], p, MAXBASENAME);
	return;
}


/*
 * notfiy requestor of deleted requres
 * return
 *	none
 */
notfyuser(file)
register char *file;
{
	register FILE *fp;
	register int numrq;
	char frqst[100], lrqst[100];
	char msg[BUFSIZ];
	char *args[10];
	char *strcat();
	struct stat sb;
	int	statop;

	xf = file;
	if ((fp = fopen(file, "r")) == NULL)
		return;
	if(stat(file, &sb) != -1)
		nstat.t_qtime = sb.st_mtime;
	if (fgets(frqst, sizeof(frqst), fp) == NULL) {
		fclose(fp);
		return;
	}
	numrq = 1;
	while (fgets(lrqst, sizeof(lrqst), fp))
		numrq++;
	fclose(fp);
	sprintf(msg,
		"uuclean: File %s delete. \nCould not contact remote. \n%d requests deleted.\n",
		 file, numrq);
	if (numrq == 1) {
		strcat(msg, "REQUEST: ");
		strcat(msg, frqst);
	} else {
		strcat(msg, "FIRST REQUEST: ");
		strcat(msg, frqst);
		strcat(msg, "\nLAST REQUEST: ");
		strcat(msg, lrqst);
	}
	getargs(frqst, args);
	/*
	 * Notify user via mail
	 */
	statop = strchr(args[4], 'o') != NULL;
	if(onemf){
		mf = Mailfile;
		stmesg(args[3], msg, "");
	}
	if(statop){
		if(args[0][0] == 'S'){
			mf = args[7];
		}else
		if(args[0][0] == 'R'){
			mf = args[5];
		}
		stmesg(args[3], msg, "");
		return;
	}
	mailst(args[3], msg, "");
	return;
}
stmesg(u, f, m)
char	*u, *f, *m;
{
	struct tm *tp;
	FILE	*Cf;
	extern struct tm *localtime();
	time_t	time();
	time_t	clock;
	long	td,th,tm,ts;

	if((Cf = fopen(mf, "a+")) == NULL){
		return;
	}
	chmod(mf, 0666);
DEBUG(4,"STM %d\n",Cf);
	fprintf(Cf, "uucp job %.4s ", xf + strlen(xf)-4);
	time(&clock);
	tp = localtime(&clock);
	fprintf(Cf, "(%d/%d-%d:%2.2d:%2.2d) ", tp->tm_mon + 1,
		tp->tm_mday, tp->tm_hour, tp->tm_min, tp->tm_sec);
	td = clock - nstat.t_qtime;
	ts = td%60;
	td /= 60;
	tm = td%60;
	td /= 60;
	th = td;
	fprintf(Cf, "(%ld:%ld:%ld) ", th, tm, ts);
	fprintf(Cf, "%s %s", f, m);
	fclose(Cf);
}


/*
 * determine the owner of the file (file,
 * create a message string and call "mailst"
 * to send the cleanup message.
 * This is only implemented for local system
 * mail at this time.
 */
sdmail(file, uid)
register char *file;
{
	static struct passwd *pwd;
	struct passwd *getpwuid();
	char mstr[40];

	sprintf(mstr, "uuclean deleted file %s\n", file);
	if (pwd->pw_uid == uid) {
		mailst(pwd->pw_name, mstr, "");
	return(0);
	}

	setpwent();
	if ((pwd = getpwuid(uid)) != NULL) {
		mailst(pwd->pw_name, mstr, "");
	}
	return(0);
}

cleanup(code)
{

	exit(code);
}

/*
 * Make del entry
 *	file	-> ptr to file string
 *	status	-> ptr to status string
 * Returns:
 *	none
 */
wrnent(file, status, f, tv)
register char *file, *status;
time_t tv;
{
	register struct tm *tp;
	extern struct tm *localtime();
	time_t clock;
	char	*lfile;
	static int pid = 0;
	static FILE *Df = 0, *Wf = 0;

	if (pid == 0){
		pid = getpid();
	}
	if(f == 1)
		if(Df == NULL){
			if((Df = fopen(LOGDEL, "a+")) == NULL){
				return;
			}
		}
	if(Wf == NULL){
		if(Logfile != NULL){
			if((Wf = fopen(Logfile, "w")) == NULL){
				return;
			}
		}else
			Wf = stdout;
	}
	time(&clock);
	tp = localtime(&clock);
	if(wrnf)
		stamp(Wf, tp, status, file, clock, tv);
	if(f)
		stamp(Df, tp, status, file, clock, tv);
	return;
}
stamp(fd, tp, status, file, c, tv)
FILE	*fd;
struct tm *tp;
char *status, *file;
time_t tv, c;
{
	int	sec, hr, day, min;
	time_t td;

	fprintf(fd, "(%d/%d-%d:%2.2d:%2.2d) ", tp->tm_mon + 1,
		tp->tm_mday, tp->tm_hour, tp->tm_min, tp->tm_sec);
	td = c - tv;
	if(td < 0)
		td = -td;
	day = td/(3600L*24L);
	td %= 3600L*24L;
	hr = td/3600;
	td %= 3600;
	min = td/60;
	sec = td%60;
	fprintf(fd, "(%2.2d:%2.2d:%2.2d:%2.2d) ", day, hr, min, sec);
	fprintf(fd, "%s (%s)\n", status, file);
	fflush(fd);
}
