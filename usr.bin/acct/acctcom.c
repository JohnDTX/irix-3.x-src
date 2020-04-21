char _Origin_[] = "System V";
/*	@(#)acctcom.c	1.6 of 3/31/82	*/
static char sccsid[]="@(#) acctcom.c: 1.6";
/* $Source: /d2/3.7/src/usr.bin/acct/RCS/acctcom.c,v $ */
static	char	*Sccsid = "@(#)$Revision: 1.1 $";
/* $Date: 89/03/27 17:42:10 $ */
/*
	acctcom [-bhkmrtv] [-l name] [-u uid] [-s time] [-e time] 
		[-g gid] [-n pattern] [-C cpuvalue] [-H hogvalue]
		[-O sysvalue] [file...]
 *	usually used to read /usr/adm/pacct
 *	reads file... (acct.h format), printing selected records
 *	reads files in order given, but may read individual files
 *	forward or backward; reads std. input if no files given
 *	and std. input is not a tty.
 *	If std. input is a tty reads /usr/adm/pacct.
 *	-b	read files backward (for lastcom effect)
 *	-d mo/dy all time args following occur on given
		month and day rather than last 24 hours
 *	-g	group	print only records with specified gid
 *	-h	show "hog factor (cpu/elapsed)
 *	-i	show I/O
 *	-k	show kcore-minutes
 *	-m	show mean core size (the default)
 *	-r	show cpu factor (user/(sys+user))
 *	-t	show separate sys and user cpu times
 *	-v	verbose (turns off initial 2-line heading)
 *	-l name	print only records with specified linename (/dev/linename)
 *	-u user	print only records with specified uid (or name)
	 if name given is ? prints processes without login
	 if name is # prints proceses using super-user privilege
 *	-s time	print only records of processes active on or after time
 *	-e time	print only records of processes active on or before time
 *	if interrupted, shows line it was looking at
 *	-C prints processes exceeding specified CPU time (sec.)
 *	-H print processes with hog factors exceeding cut-off.
 *	-I print processes transferring more characters than cut-off
 *	-O print proceses with system time exceeding specified value (sec.)
 *	-o file  copy chosen records in binary to file instead of printing stdout
 */

#include <sys/types.h>
#include "acctdef.h"
#include <grp.h>
#include <stdio.h>
#include <sys/acct.h>
#include <pwd.h>
#include <sys/stat.h>

#define DEBUG	0  /* use 1 instead of 0 while debugging; */
#define MYKIND(flag)   ((flag & ACCTF) == 0)
#define SU(flag)   ((flag & ASU) == ASU)
#define PACCT "/usr/adm/pacct"
#define MEANSIZE	01
#define KCOREMIN	02
#define HOGFACTOR	04
#define	SEPTIME		010
#define	CPUFACTOR	020
#define IORW		040
#define	ARRSZ		16    /* record size 32 bytes; total bytes in array
			         = 512 */

struct	acct ab;
struct acct actbuff[ARRSZ]; /* used for reading ARRSZ records at a time; */
int bufx, bufcnt;	  /* bufx = index into actbuff for next item to read;
			  ** bufcnt = no. of useful structs in actbuff;
			  */
long	elapsed, sys, user, cpu, mem, io, rw;
char	command_name[16];
time_t	end_time;
int	backward;
int	flag_field,average,quiet;
double	cpucut,syscut,hogcut,iocut;
double	realtot,cputot,usertot,systot,kcoretot,iotot,rwtot;
long	cmdcount;
int	option;
int	verbose = 1;
dev_t	linedev	= -1;
uid_t	uidval;
uid_t	gidval; /*should create typedef for group*/
int	uidflag,gidflag;
int	noid; /*user doesn't have login on this machine*/
int	su_user;
time_t	tstrt_b, tstrt_a, tend_b, tend_a;
char	strt_b[10], strt_a[10], end_b[10], end_a[10];
int 	flg_eb, flg_ea, flg_sb, flg_sa;
int	nfiles;
int	sig2;
char	obuf[BUFSIZ];
char	*cname = NULL; /* command name pattern to match*/

long	ftell();
char	*ctime();
long	time();
char	*devtolin();
long	convtime();
uid_t	namtouid();
char	*uidtonam();
struct group *getgrnam(),*grp_ptr;
dev_t	lintodev();
struct	tm *localtime();
long	tmsecs();
int	catch2();
long	expand();
int	isdevnull();
char	*strcpy(), *strcat();
char	ofile[40];
int	fileout = 0;
FILE	*ostrm, *fopen();

int stiflg =0;
main(argc, argv)
char **argv;
{

	setbuf(stdout,obuf);
	if (signal(2, 1) != 1)
		signal(2, catch2);
	time(&tend_b);
	tstrt_b = tend_b;
	tstrt_a = tend_b - SECSINDAY;
	tend_a = tend_b - SECSINDAY;

	while (--argc > 0) {
		if (**++argv == '-')
			switch(*++*argv) {
			case 'C':
				if(--argc>0) {
					sscanf(*++argv,"%f",&cpucut);
				};
				continue;
			case 'O':
				if(--argc>0) {
					sscanf(*++argv,"%f",&syscut);
				};
				continue;
			case 'H':
				if(--argc) {
					sscanf(*++argv,"%f",&hogcut);
				};
				continue;
			case 'I':
				if(--argc) {
					sscanf(*++argv,"%f",&iocut);
				};
				continue;
			case 'a':
				average++;
				continue;
			case 'b':
				backward++;
				continue;
			case 'g':
				if(--argc > 0) {
					if(sscanf(*++argv,"%d",&gidval)!=1)
						if((grp_ptr=getgrnam(*argv))==0)
							fprintf(stderr,"No such group\n");
						else
							gidval=grp_ptr->gr_gid;
					gidflag++;
				};
				continue;
			case 'h':
				option |= HOGFACTOR;
				continue;
			case 'i':
				option |= IORW;
				continue;
			case 'k':
				option |= KCOREMIN;
				continue;
			case 'm':
				option |= MEANSIZE;
				continue;
			case 'n':
				if(--argc>0)
					cname=(char *)cmset(*++argv);
				continue;
			case 't':
				option |= SEPTIME;
				continue;
			case 'r':
				option |= CPUFACTOR;
				continue;
			case 'v':
				verbose=0;
				continue;
			case 'l':
				if (--argc > 0)
					linedev = lintodev(*++argv);
				continue;
			case 'u':
				if (--argc > 0) {
					++argv;
					if(**argv == '?') {
						noid++;
						continue;
					};
					if(**argv == '#') {
						su_user++;
						continue;
					};
					uidflag++;
					if (sscanf(*argv, "%d", &uidval) != 1)
						uidval = namtouid(*argv);
					continue;
				}
			case 'd':
				if (--argc > 0) {
					convday(*++argv);
					tstrt_b = convtime("24:");
					tstrt_a = convtime("00:");
					tend_b = convtime("24:");
					tend_a = convtime("00:");
				}
				continue;
			case 'q':
				quiet++;
				verbose=0;
				average++;
				continue;
			case 's':
				if (--argc > 0) {
					flg_ea++;
					strcpy(end_a, *++argv);
				}
				continue;
			case 'S':
				if (--argc > 0) {
					flg_sa++;
					strcpy(strt_a, *++argv);
				}
				continue;
			case 'f':
				flag_field++;
				continue;
			case 'e':
				if (--argc > 0) {
					flg_sb++;
					strcpy(strt_b, *++argv);
				}
				continue;
			case 'E':
				if (--argc > 0) {
					flg_eb++;
					strcpy(end_b, *++argv);
				}
				continue;
			case 'o':
				if(--argc > 0) {
					strcpy(ofile, *++argv);
					fileout++;
					ostrm = fopen(ofile, "w");
				}
				continue;
			default:
				error("Unknown option in acctcom ...");
			}
		else {
			dofile(*argv);
			nfiles++;
		}
	}
	if(nfiles==0) {  
		if(isatty(0))
			dofile(PACCT);
		else if(isdevnull())
			dofile(PACCT);
		else {
			stiflg = 1;
			backward = 0;
			dofile(NULL);
		}
	};
	doexit(0);
}

dofile(fname)
char *fname;
{
	long rfirst, rlast;
	long rfind();

	if(flg_sb)	tstrt_b = convtime(strt_b);
	if(flg_sa)	tstrt_a = convtime(strt_a);
	if(flg_eb)	tend_b = convtime(end_b);
	if(flg_ea)	tend_a = convtime(end_a);

	if (fname != NULL)
		if (freopen(fname, "r", stdin) == NULL) {
			fprintf(stderr,  "acctcom: cannot open %s\n", fname);
			return;
		}
	if (DEBUG) {
		printf("tend_a = %s",ctime(&tend_a));
		printf("tend_b = %s",ctime(&tend_b));
		printf("tstrt_a = %s",ctime(&tstrt_a));
		printf("tstrt_b = %s",ctime(&tstrt_b));
	}
	if (tend_a > tend_b || tstrt_a > tstrt_b || tstrt_a > tend_b)
		error("Inconsistent time specifications ...");
		/* do not process any file; */

	/* print selected times in s, S, e, E once for each file; */
	if (flg_sa) printf("START AFT: %s", ctime(&tstrt_a));
	if (flg_sb) printf("START BEF: %s", ctime(&tstrt_b));
	if (flg_ea) printf("END AFTER: %s", ctime(&tend_a));
	if (flg_eb) printf("END BEFOR: %s", ctime(&tend_b));
	tend_a = (tend_a > tstrt_a) ? tend_a : tstrt_a;
	/*
	printf("tend_a = %s",ctime(&tend_a));
	printf("tend_b = %s",ctime(&tend_b));
	printf("tstrt_a = %s",ctime(&tstrt_a));
	printf("tstrt_b = %s",ctime(&tstrt_b));
	*/
	if (stiflg == 0) {
		if (backward) {
			if (DEBUG) printf("Finding last record:\n");
			rlast = rfind(tend_b,-1);
			fseek(stdin, (long)(ARRSZ+rlast+1)*sizeof (struct acct), 0);
			bufx = -1;
		} else  {
			if (DEBUG) printf("Finding first record:\n");
			rfirst = rfind(tend_a,1);
			bufx = bufcnt = 0;
			fseek(stdin,rfirst*sizeof (struct acct),0);
		}
	}
	/* printf("bufx = %d\n",bufx); */

	while (aread(&ab)) {
		if (!MYKIND(ab.ac_flag))
			continue;
		if(su_user && !SU(ab.ac_flag))
			continue;
		elapsed = expand(ab.ac_etime);
		sys = expand(ab.ac_stime);
		user = expand(ab.ac_utime);
		cpu = sys + user;
		if(cpu == 0)
			cpu = 1;
		mem = expand(ab.ac_mem);
		substr(ab.ac_comm,command_name,0,8);
		end_time = ab.ac_btime + SECS(elapsed); /* d + 30); */
		io=expand(ab.ac_io);
		rw=expand(ab.ac_rw);
		if(cpucut != 0 && cpucut >=  SECS(cpu))
			continue;
		if(syscut != 0 && syscut >=  SECS(sys))
			continue;
		if (linedev != -1 && ab.ac_tty != linedev)
			continue;
		if (uidflag  && ab.ac_uid != uidval)
			continue;
		if(gidflag && ab.ac_gid != gidval)
			continue;
		if (end_time < tend_a) {
			if (backward) return;
			else continue; /* this is executed only once perhaps; */
		}
		if (end_time > tend_b) {
			if (backward) continue; /* this is executed only once perhaps; */
			else return;
		}
		if ((ab.ac_btime > tstrt_b) || (ab.ac_btime < tstrt_a))
			continue;
		if(cname && !cmatch(ab.ac_comm,cname))
			continue;
		if(iocut && iocut > io)
			continue;
		if(noid && uidtonam(ab.ac_uid)[0] != '?')
			continue;
		if ((verbose) && (fileout == 0)) {
			printhd();
			verbose = 0;
		}
		if(elapsed == 0)
			elapsed++;
		if(hogcut !=0 && hogcut >= (double)cpu/(double)elapsed)
			continue;
		if(fileout)
			fwrite(&ab, sizeof(ab), 1, ostrm);
		else
			println();
		if(average) {
			cmdcount++;
			realtot += (double)elapsed;
			usertot += (double)user;
			systot +=  (double)sys;
			kcoretot += (double)mem;
			iotot += (double)io;
			rwtot += (double)rw;
		};
	}
}

catch2()
{
	sig2++;
}


aread(aba)
struct acct *aba;
{
	long nrcrd;

	if (sig2) {
		printf("\n");
		println();
		doexit(2);
	}
	/*
	if (backward) {
		if (ftell(stdin) < 2*sizeof(*aba))
			return(0);
		fseek(stdin, (long)-2*sizeof(*aba), 1);
		fread(aba, sizeof(*aba), 1, stdin);
		return(1);
	} else
		return(fread(aba, sizeof(*aba), 1, stdin) == 1);
	*/

	if (backward) {
		if (bufx==-1) { /* fill acctcbuff; */
			if ((nrcrd = ftell(stdin)/sizeof (struct acct) - ARRSZ) <= 0)
				return(0);
			else {
				bufcnt = (nrcrd < ARRSZ) ? nrcrd : ARRSZ;
				fseek(stdin, (long)-(bufcnt+ARRSZ)*sizeof (struct acct), 1);
				fread(actbuff,bufcnt*sizeof (struct acct),1,stdin);
				bufx = bufcnt-1;
				/* printf("bufx_backward = %d\n",bufx); */
			}
		}
		*aba = actbuff[bufx--];
	} else {
		if (bufx == bufcnt) { /* fill actbuff; */
			if ((bufcnt = fread(actbuff,sizeof (struct acct),ARRSZ,stdin)) <= 0)
				return(0);
			/* printf("bufcnt = %d\n",bufcnt); */
			bufx = 0;
		}
		*aba = actbuff[bufx++];
	}
	return(1);
}

printhd()
{
	printf("COMMAND                      START    END          REAL");
	ps("CPU");
	if (option & SEPTIME)
		ps("(SECS)");
	if (option & IORW){
		ps("CHARS");
		ps("BLOCKS");
	}
	if (option & CPUFACTOR)
		ps("CPU");
	if (option & HOGFACTOR)
		ps("HOG");
	if (!option || (option & MEANSIZE))
		ps("MEAN");
	if (option & KCOREMIN)
		ps("KCORE");
	printf("\n");
	printf("NAME         USER   TTYNAME  TIME     TIME       (SECS)");
	if (option & SEPTIME) {
		ps("SYS");
		ps("USER");
	} else
		ps("(SECS)");
	if (option & IORW) {
		ps("TRNSFD");
		ps("READ");
	}
	if (option & CPUFACTOR)
		ps("FACTOR");
	if (option & HOGFACTOR)
		ps("FACTOR");
	if (!option || (option & MEANSIZE))
		ps("SIZE(K)");
	if (option & KCOREMIN)
		ps("MIN");
	if(flag_field)
		printf("  F STAT");
	printf("\n");
	fflush(stdout);
}

ps(s)
char	*s;
{
	printf("%8.8s", s);
}

println()
{

	extern char *lookname();
	char name[32];

	if(quiet)
		return;
	if(!SU(ab.ac_flag))
		strcpy(name,command_name);
	else {
		strcpy(name,"#");
		strcat(name,command_name);
	}
	printf("%-9.9s", name);
	strcpy(name,lookname(ab.ac_uid,ab.ac_tty,ab.ac_btime));
	if(*name != '?')
		printf("  %-8.8s", name);
	else
		printf("  %-8d",ab.ac_uid);
	printf(" %-8.8s",ab.ac_tty != -1? devtolin(ab.ac_tty):"?");
	printf("%.9s", &ctime(&ab.ac_btime)[10]);
	printf("%.9s ", &ctime(&end_time)[10]);
	pf((double)SECS(elapsed));
	if (option & SEPTIME) {
		pf((double)sys / HZ);
		pf((double)user / HZ);
	} else
		pf((double)cpu / HZ);
	if (option & IORW)
		printf("%8ld%8ld",io,rw);
	if (option & CPUFACTOR)
		pf((double)user / cpu);
	if (option & HOGFACTOR)
		pf((double)cpu / elapsed);
	if (!option || (option & MEANSIZE))
		pf(KCORE(mem / cpu));
	if (option & KCOREMIN)
		pf(MINT(KCORE(mem)));
	if(flag_field)
		printf("  %1o %3o", (int)ab.ac_flag, (unsigned)ab.ac_stat);
	printf("\n");
	fflush(stdout);
}

char *
lookname(uid,tty,start)
uid_t	uid;
dev_t	tty;
time_t	start;
{
	return(uidtonam(uid));
}

pf(v)
double	v;
{
	printf("%8.2f", v);
}

/*
 * return uid of name, -1 if not found
 */
uid_t
namtouid(name)
char *name;
{
	struct passwd *getpwnam();
	register struct passwd *pp;

	setpwent();
	if ((pp = getpwnam(name)) == NULL)
		error("acctcom: unknown login-name ....");
		/*
		return((uid_t)-1);
		*/
	return((uid_t)pp->pw_uid);
}
#include <time.h>
long	daydiff;


/*
 * convday computes number of seconds to be subtracted
 * because of -d mon/day argument
 */
convday(str)
register char *str;
{
	struct tm *cur;
	long tcur;
	int mday, mon;
	register i;

	if (sscanf(str, "%d/%d", &mon, &mday) != 2) {
		fprintf(stderr, "acctcom: can't scan -d %s\n", str);
		exit(1);
	}
	mon--;
	time(&tcur);
	daydiff = tcur;
	for (i = 0; i < 100; i++) {
		cur = localtime(&tcur);
		if (mday == cur->tm_mday && mon == cur->tm_mon) {
			daydiff -= tcur;
			return;
		}
		tcur -= SECSINDAY;
	}
	daydiff = 0;
	fprintf(stderr, "acctcom: bad -d %s\n", str);
	exit(1);
}

/*
 * convtime converts time arg to internal value
 * arg has form hr:min:sec, min or sec are assumed to be 0 if omitted
 * times assumed to be within last 24 hours, unless -d mo/dy given before
 */
long
convtime(str)
char *str;
{
	struct tm *cur, arg;
	long tcur;

	arg.tm_min = 0;
	arg.tm_sec = 0;
	if (sscanf(str, "%d:%d:%d", &arg.tm_hour, &arg.tm_min,
		&arg.tm_sec) < 1) {
		fprintf(stderr, "acctcom: bad time: %s\n", str);
		exit(1);
	}
	time(&tcur);
	cur = localtime(&tcur);
	if (tmless(&arg, cur))
		tcur -= (daydiff+tmsecs(&arg, cur));
	else {
		tcur -= SECSINDAY-tmsecs(cur, &arg);
		if (daydiff)
			tcur -= daydiff-SECSINDAY;
	}
	return(tcur);
}
cmatch(comm, cstr)
register char	*comm, *cstr;
{

	char	xcomm[9];
	register  i;

	for(i=0;i<8;i++){
		if(comm[i]==' '||comm[i]=='\0')
			break;
		xcomm[i] = comm[i];
	}
	xcomm[i] = '\0';

	return(regex(cstr,xcomm));
}

cmset(pattern)
register char	*pattern;
{

	if((pattern=(char *)regcmp(pattern,0))==NULL){
		error("pattern syntax");
	}

	return((unsigned)pattern);
}

doexit(status)
{
	if(!average)
		exit(status);
	if(cmdcount != 0) {
		printf("\cmds=%ld ",cmdcount);
		printf("Real=%-6.2f ",SECS(realtot)/cmdcount);
		cputot = systot + usertot;
		printf("CPU=%-6.2f ",SECS(cputot)/cmdcount);
		printf("USER=%-6.2f ",SECS(usertot)/cmdcount);
		printf("SYS=%-6.2f ",SECS(systot)/cmdcount);
		printf("CHAR=%-8.2f ",iotot/cmdcount);
		printf("BLK=%-8.2f ",rwtot/cmdcount);
		printf("USR/TOT=%-4.2f ",usertot/cputot);
		printf("HOG=%-4.2f ",cputot/realtot);
		printf("\n");
	}
	else
		printf("\nNo commands matched\n");
	exit(status);
}
isdevnull()
{
	struct stat	filearg;
	struct stat	devnull;

	if(fstat(0,&filearg) == -1) {
		fprintf(stderr,"acctcom: cannot stat stdin\n");
		return(NULL);
	}
	if(stat("/dev/null",&devnull) == -1) {
		fprintf(stderr,"acctcom: cannot stat /dev/null\n");
		return(NULL);
	}

	if(filearg.st_rdev == devnull.st_rdev) return(1);
	else return(NULL);
}

error(s)
char *s;
{
	fprintf(stderr,"%s\n",s);
	exit(1);
}

/*
** If fl = -1, rfind() returns the first record number whose endtime
** is greater than xtime or the last record with endtime = xtime.
** For fl = 1, it returns the record number of the last record
** whose endtime is less than x time or the first record with
** endtime = xtime.
*/
long
rfind(xtime,fl)
time_t xtime;
int fl;
{
	long first, last, midp;

	/*
	** A binary search is made to obtain bounds for the records
	** which includes all records with endtime between tend_a and tend_b.
	*/
	first = 0L;
	if (DEBUG) swprnt("first",first);
	fseek(stdin,0L,0);
	fread(&ab,sizeof (struct acct),1,stdin);
	end_time = ab.ac_btime + SECS(expand(ab.ac_etime));
	if (end_time >= xtime && fl == 1) return(first);
	if (end_time > xtime && fl == -1) return(first);
	if (fseek(stdin,-(long)sizeof (struct acct),2) == 0) last = ftell(stdin)/sizeof (struct acct);
	if (DEBUG) swprnt("last",last);
	fseek(stdin,last*sizeof (struct acct),0);
	fread(&ab,sizeof (struct acct),1,stdin);
	end_time = ab.ac_btime + SECS(expand(ab.ac_etime));
	if (end_time < xtime && fl == 1) return(last);
	if (end_time <= xtime && fl == -1) return(last);
	while (last-first > 1) {
		midp = (first+last)/2;
		fseek(stdin,midp*sizeof (struct acct),0);
		fread(&ab,sizeof (struct acct),1,stdin);
		end_time = ab.ac_btime + SECS(expand(ab.ac_etime));
		if (end_time > xtime) {
			last = midp;
			if (DEBUG) swprnt("last",last);
		}
		else if (end_time < xtime) {
			first = midp;
			if (DEBUG) swprnt("first",first);
		}
		else if (fl==1) {
			last = midp;
			if (DEBUG) swprnt("last",last);
		}
		else {
			first = midp;
			if (DEBUG) swprnt("first",first);
		}
	}
	if (fl==1) last = first;
		if (DEBUG) prntnb(last);
		return(last);
}

swrite(num)
long num;
{
	fseek(stdin,num*sizeof (struct acct),0);
	fread(&ab,sizeof (struct acct),1,stdin);
		elapsed = expand(ab.ac_etime);
		if(elapsed == 0)
			elapsed++;
		sys = expand(ab.ac_stime);
		user = expand(ab.ac_utime);
		cpu = sys + user;
		if(cpu == 0)
			cpu = 1;
		mem = expand(ab.ac_mem);
		io=expand(ab.ac_io);
		rw=expand(ab.ac_rw);
	substr(ab.ac_comm,command_name,0,8);
	end_time = ab.ac_btime + SECS(expand(ab.ac_etime));
	printf("Record num = %ld\n",num);
	println();
}

prntnb(item)
long item;
{
	long bl, bu, max; /* bl, bu are seek positions around item; */
	long i;

	bl = (item < 5) ? item : item-5;
	fseek(stdin,0L,2);
	max = ftell(stdin)/sizeof (struct acct) - 1;
	bu = (max < item+5) ? max : item+5;

	fseek(stdin,bl*sizeof (struct acct),0);
	printf("\nNeighbourhood (%ld,%ld,%ld):\n",bl,item,bu);
	for (i=bl; i<=bu; i++) {
		swrite(i);
	}
}

swprnt(str,item)
char *str;
long item;
{
	printf("%s:\n",str);
	swrite(item);
}
