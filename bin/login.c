#ifndef	lint
/*	@(#)login.c	1.3	*/
char _Origin_[] = "System V";
/* $Source: /d2/3.7/src/bin/RCS/login.c,v $ */
static	char	Sccsid[] = "@(#)$Revision: 1.1 $";
/* $Date: 89/03/27 14:50:39 $ */
#endif

/*
 * login [ name ] [ environment args ]
 *
 *	Conditional assemblies:
 *
 *	PASSREQ	causes password to be required
 *	NO_MAIL	causes the MAIL environment variable not to be set
 *	NOSHELL does not put non-standard shell names into environment
 *	CONSOLE, if defined, only allows root logins on the device
 *		specified by CONSOLE.  CONSOLE MUST NOT be defined as
 *		either "/dev/syscon" or "/dev/systty"!!
 *	MAXTRYS is the number of attempts permitted.  0 is "no limit".
 *	MAXTIME is the number of seconds before giving up.  0 is "no limit".
 */

/*
 * enhanced at SGI to put TZ (time-zone) into environment
 * this is especially useful to UUCP
 *
 * enhanced at SGI to put USER (bsd4.2 version of LOGNAME) into environment
 */

#include <sys/types.h>
#include <utmp.h>
#include <signal.h>
#include <pwd.h>
#include <sys/param.h>
#include <stdio.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <termio.h>

#ifndef	MAXTRYS
#	define	MAXTRYS	10	/* default */
#endif
#ifndef	MAXTIME
#	define	MAXTIME	60	/* default */
#endif

#define	TRUE		1
#define	FALSE		0
#define	FAILURE		(-1)

#define SCPYN(a, b)	strncpy(a, b, sizeof(a))
#define DIAL_FILE	"/etc/dialups"
#define DPASS_FILE	"/etc/d_passwd"
#define SHELL		"/bin/sh"
#define	PATH		"PATH=:/bin:/usr/bin"
#define	ROOTPATH	"PATH=:/bin:/usr/bin:/etc"
#define SUBLOGIN	"<!sublogin>"

#define	ROOTUID		0

#define	MAXARGS		63
#define	MAXENV		1024
#define MAXLINE		256
#ifndef	TZFILE
#define	TZFILE		"/etc/TZ"
#endif

struct	passwd nouser = {"", "nope", 0, 0, "" ,"" ,"" ,"" ,""};
struct	utmp utmp;
#define NMAX 64
char	u_name[NMAX];
char	minusnam[16]	= "-";
char	shell[64]	= { "SHELL="	};
char	home[64]	= { "HOME="	};
char	logname[30]	= { "LOGNAME="	};
char	user[30]	= { "USER="	};
char	tz[12]		= { "TZ="	};
char	*tp;
char	term[64];
char	env_term[5+64]	= { "TERM="	};
char	env_remotehost[12+MAXHOSTNAMELEN] = { "REMOTEHOST=" };
char	env_remotename[12+NMAX] = { "REMOTEUSER=" };

#ifndef	NO_MAIL
char	mail[30] = { "MAIL=/usr/mail/" };
#endif

char	*envinit[10+MAXARGS] = {
		home,PATH,logname,0,0, 0,0
	};
int	basicenv;
int	tzfd;		/* file descriptor for TZFILE */
char	envblk[MAXENV];
struct	passwd *pwd;

char	*crypt();
char	*getpass();
char	*getenv();
long	lseek();
extern	char **environ;

#define	WEEK	(24L * 7 * 60 * 60) /* 1 week in seconds */
time_t	when;
time_t	maxweeks;
time_t	minweeks;
time_t	now;
long	a64l(), time();


int	rflag = 0;
int	hflag = 0;
int	usererr = -1;
char	remotehost[MAXHOSTNAMELEN+1];
char	rusername[NMAX+1], lusername[NMAX+1];
char	rpassword[NMAX+1];


main(argc, argv ,renvp)
char **argv,**renvp;
{
	register char *namep;
	int j,k,l_index,length;
	char *ttyn,*ttyntail;
	int nopassword;
	register int i;
	register struct utmp *u;
	struct utmp *getutent(), *pututline();
	FILE *fp;
	char **envp,*ptr,*endptr;
	int sublogin;
	extern char **getargs();
	extern char *terminal();
#if	MAXTRYS > 0
	int	trys = 0;
#endif

/*	Set flag to disable the pid check if you find that you are	*/
/*	a subsystem login.						*/

	sublogin = FALSE;
	if (*renvp && strcmp(*renvp,SUBLOGIN) == 0)
		sublogin = TRUE;

	umask(0);
	alarm( MAXTIME );
	signal(SIGQUIT, 1);
	signal(SIGINT, 1);
	nice(0);
	ttyn = terminal(0);

	if (ttyn==0)
		ttyn = "/dev/tty??";
	if (ttyntail = strrchr(ttyn,'/')) ttyntail++;
	else ttyntail = ttyn;

	/* -r is used by rlogind to cause the autologin protocol;
	 * -h is used by other servers to pass the name of the
	 *	remote host to login.
	 * -t is used to set the terminal type
	 */
	while (argc > 1) {
		if (strcmp(argv[1], "-r") == 0 && argc > 2) {
			if (rflag || hflag) {
				printf("Only one of -r and -h allowed\n");
				exit(1);
			}
			rflag = 1;
			usererr = doremotelogin(argv[2]);
			doremoteterm(term);
			argc -= 2;
			argv += 2;
			continue;
		}
		if (strcmp(argv[1], "-h") == 0 && argc > 2 && getuid() == 0) {
			if (rflag || hflag) {
				printf("Only one of -r and -h allowed\n");
				exit(1);
			}
			hflag = 1;
			strncpy(remotehost, argv[2], sizeof(remotehost)-1);
			strncpy(rusername, "UNKNOWN", sizeof(rusername)-1);
			argc -= 2;
			argv += 2;
			continue;
		}
		if (strcmp(argv[1], "-t") == 0 && argc > 2 && getuid() == 0) {
			strncpy(term, argv[2], sizeof(term)-1);
			argc -= 2;
			argv += 2;
			continue;
		}

		SCPYN(utmp.ut_user, argv[1]);
		SCPYN(u_name, argv[1]);
		envp = &argv[2];
		goto first;
	}
	if (rflag)
		goto first;
loop:
	if (!usererr)			/* no extra tries for remote login */
		exit(1);		/* that is what 4.3 does, so... */
	u_name[0] = utmp.ut_user[0] = '\0';
	rflag = 0;			/* forget remote name after failure */
first:
#if	MAXTRYS > 0
	if( ++trys > MAXTRYS ) {
		sleep( MAXTIME );
		exit( 1 );
	}
#endif
	if (!rflag) {
		while (utmp.ut_user[0] == '\0') {
			printf("login: ");
			if ((envp = getargs()) != (char**)NULL) {
				SCPYN(utmp.ut_user,*envp);
				SCPYN(u_name, *envp++);
			}
		}
		setpwent();
		if ((pwd = getpwnam(u_name)) == NULL)
			pwd = &nouser;
		endpwent();
	}
	if (strncmp("pri=", pwd->pw_gecos, 4) == 0) {
		int	mflg, pri;

		pri = 0;
		mflg = 0;
		i = 4;
		if (pwd->pw_gecos[i] == '-') {
			mflg++;
			i++;
		}
		while(pwd->pw_gecos[i] >= '0' && pwd->pw_gecos[i] <= '9')
			pri = (pri * 10) + pwd->pw_gecos[i++] - '0';
		if (mflg)
			pri = -pri;
		nice(pri);
	}
	nopassword = 1;
	if (usererr == -1 && *pwd->pw_passwd != '\0') {
		nopassword = 0;
		if(gpass("Password:", pwd->pw_passwd))
			goto loop;
	}

	/*
	 * get dialup password, if necessary
	 */
	if(dialpass(ttyn))
		goto loop;

	if(chdir(pwd->pw_dir) < 0) {
		printf("Unable to change directory to \"%s\"\n",pwd->pw_dir);
		if(chdir("/") < 0) {
			printf("Unable to change directory to \"/\"\n");
			goto loop;
		}
		printf("You are in \"/\" instead.\n");
	}
	time(&utmp.ut_time);
	utmp.ut_pid = getpid();

/*	Find the entry for this pid (or line if we are a sublogin) in	*/
/*	the utmp file.							*/

	while ((u = getutent()) != NULL) {
		if ((u->ut_type == INIT_PROCESS || u->ut_type ==
		    LOGIN_PROCESS || u->ut_type == USER_PROCESS)
		    && ( (sublogin && strncmp(u->ut_line,ttyntail,
		    sizeof(u->ut_line)) == 0) || u->ut_pid == utmp.ut_pid) ) {

/*	Copy in the name of the tty minus the "/dev/", the id, and set	*/
/*	the type of entry to USER_PROCESS.				*/

			SCPYN(utmp.ut_line,ttyntail);
			utmp.ut_id[0] = u->ut_id[0];
			utmp.ut_id[1] = u->ut_id[1];
			utmp.ut_id[2] = u->ut_id[2];
			utmp.ut_id[3] = u->ut_id[3];
			utmp.ut_type = USER_PROCESS;

/*	Return the new updated utmp file entry.				*/

			pututline(&utmp);
			break;
		}
	}
	endutent();		/* Close utmp file */

	if (u == (struct utmp *)NULL){
		printf("No utmp entry.  You must exec \"login\" from\
 the lowest level \"sh\".\n");
		exit(1);
	}

/*	Now attempt to write out this entry to the wtmp file if we	*/
/*	were successful in getting it from the utmp file and the	*/
/*	wtmp file exists.						*/

	if (u != NULL && (fp = fopen(WTMP_FILE,"r+")) != NULL) {
		fseek(fp,0L,2);		/* Seek to end of file. */
		fwrite(&utmp,sizeof(utmp),1,fp);
		fclose(fp);
	}
	chown(ttyn, pwd->pw_uid, pwd->pw_gid);
	chmod(ttyn, 0622);

/*	If the shell field starts with a '*', do a chroot to the home	*/
/*	directory and perform a new login.				*/

	if(*pwd->pw_shell == '*') {
		if(chroot(pwd->pw_dir) < 0) {
			printf("No Root Directory\n");
			goto loop;
		}

/*	Set the environment flag <!sublogin> so that the next login	*/
/*	knows that it is a sublogin.					*/

		envinit[0] = SUBLOGIN;
		envinit[1] = (char*)NULL;
		printf("Subsystem root: %s\n",pwd->pw_dir);
		execle("/bin/login", "login", 0,&envinit[0]);
		execle("/etc/login", "login", 0,&envinit[0]);
		printf("No /bin/login or /etc/login on root\n");
		goto loop;
	}

	if (setgid(pwd->pw_gid) == FAILURE) {
		printf("Bad group id.\n");
		exit(1);
	}
	if (setuid(pwd->pw_uid) == FAILURE) {
		printf("Bad user id.\n");
		exit(1);
	}

/*	Set up the basic environment for the exec.  This includes	*/
/*	HOME, PATH, LOGNAME, SHELL, and MAIL, and TZ, USER, and TERM.	*/

	strcat(home, pwd->pw_dir);
	strcat(logname, pwd->pw_name);
	strcat(user, pwd->pw_name);	/* SGI enhancement for 4.2 conv */
	if(pwd->pw_uid == ROOTUID) {
#ifdef CONSOLE
		if(strcmp(ttyn, CONSOLE)) {
			printf("Not on system console\n");
			exit(10);
		}
#endif
		envinit[1] = ROOTPATH;
	}
	if (*pwd->pw_shell == '\0') {
		pwd->pw_shell = SHELL;
		strcat(shell, pwd->pw_shell);
	}
#ifndef NOSHELL
	else {
		envinit[3] = shell;
		strcat(shell, pwd->pw_shell);
	}
#endif

/*	Find the name of the shell.					*/

	if ((namep = strrchr(pwd->pw_shell, '/')) == NULL)
		namep = pwd->pw_shell;
	else
		namep++;

/*	Generate the name of the shell with a '-' sign in front of it.	*/
/*	This causes .profile processing when a shell is exec'ed.	*/

	strcat(minusnam, namep);

#ifndef	NO_MAIL
	if (envinit[3] == (char*)NULL) envinit[3] = mail;
	else envinit[4] = mail;
	strcat(mail,pwd->pw_name);
#endif

/*	Find the end of the basic environment.				*/

	for (basicenv=3; envinit[basicenv];basicenv++);

				/* add in time-zone - SGI */
	tp = getenv("TZ");
	if (tp > 0)
		strncpy(tz+3,tp,sizeof(tz)-3);
	else if ((tzfd=open(TZFILE,0)) >= 0) {
		read(tzfd,tz+3,sizeof(tz)-3);
		tp = strchr(tz,'\n');
		if (tp)
			*tp = '\0';
		close(tzfd);
	} else {
		strcpy(tz+3,"PST8PDT");
	}
	envinit[basicenv++] = tz;	/* allow for TZ */
	envinit[basicenv++] = user;	/* allow for USER */
	if (0 != term[0])
		envinit[basicenv++] = strcat(env_term,term);
	else if (0 != (tp=getenv("TERM")))
		envinit[basicenv++] = strncat(env_term,tp,sizeof(term));
	if (remotehost[0] != '\0')	/* install remote host name */
		envinit[basicenv++] = strcat(env_remotehost,remotehost);
	else if (0 != (tp=getenv("REMOTEHOST")))
		envinit[basicenv++] = strncat(env_remotehost,tp,MAXHOSTNAMELEN);
	if (rusername[0] != '\0')	/* and remote user name */
		envinit[basicenv++] = strcat(env_remotename,rusername);
	else if (0 != (tp=getenv("REMOTEUSER")))
		envinit[basicenv++] = strncat(env_remotename,tp,NMAX);

/*	Add in all the environment variables picked up from the		*/
/*	argument list to "login" or from the user response to the	*/
/*	"login" request.						*/

	if (envp) for (j=0,k=0,l_index=0,ptr= &envblk[0];
		       *envp && j < MAXARGS-1
		       ;j++,envp++) {

/*	Scan each string provided.  If it doesn't have the format	*/
/*	xxx=yyy, then add the string "Ln=" to the beginning.		*/

		if ((endptr = strchr(*envp,'=')) == (char*)NULL) {
			envinit[basicenv+k] = ptr;
			sprintf(ptr,"L%d=%s",l_index,*envp);

/*	Advance "ptr" to the beginning of the next argument.		*/

			while (*ptr++);
			k++;
			l_index++;
		} else {

/*	Check to see whether this string replaces any previously	*/
/*	defined string.							*/

			for (i=0,length= endptr+1-*envp; i < basicenv+k;i++) {
				if (strncmp(*envp,envinit[i],length) == 0) {

/*	If the variable to be changed is "SHELL=" or "PATH=", don't	*/
/*	allow the substitution.						*/

					if (strncmp(*envp,"SHELL=",6) != 0
					    && strncmp(*envp,"PATH=",5) != 0)
						envinit[i] = *envp;
					break;
				}
			}

/*	If it doesn't, place it at the end of environment array.	*/

			if (i == basicenv+k) {
				envinit[basicenv+k] = *envp;
				k++;
			}
		}
	}

/*	Switch to the new environment.					*/

	environ = envinit;
	alarm(0);
#ifdef PASSREQ
	if (nopassword && !rflag) {
		printf("You don't have a password.  Choose one.\n");
		printf("passwd %s\n", u_name);
		execl("/bin/passwd", "passwd", u_name, 0);
		printf("Can not execute /bin/passwd\n");
		exit(1);
	}
#endif

/* is the age of the password to be checked? */

	if (*pwd->pw_age != NULL) {
		/* retrieve (a) week of previous change;
			(b) maximum number of valid weeks	*/
		when = (long) a64l (pwd->pw_age);
		/* max, min and weeks since last change are packed radix 64 */
		maxweeks = when & 077;
		minweeks = (when >> 6) & 077;
		when >>= 12;
		now  = time(0)/WEEK;
		if (when > now || (now > when + maxweeks) && (maxweeks >= minweeks)) {
			printf ("Your password has expired. Choose\
 a new one\n");
			execl("/bin/passwd", "passwd", u_name, 0);
			exit(9);
		}
	}

	signal(SIGQUIT, 0);
	signal(SIGINT, 0);
	execlp(pwd->pw_shell, minusnam, 0);
	printf("No shell\n");
	exit(1);
}

dialpass(ttyn)
char *ttyn;
{
	register FILE *fp;
	char defpass[30];
	char line[80];
	register char *p1, *p2;

	if((fp=fopen(DIAL_FILE, "r")) == NULL)
		return(0);
	while((p1 = fgets(line, sizeof(line), fp)) != NULL) {
		while(*p1 != '\n' && *p1 != ' ' && *p1 != '\t')
			p1++;
		*p1 = '\0';
		if(strcmp(line, ttyn) == 0)
			break;
	}
	fclose(fp);
	if(p1 == NULL || (fp = fopen(DPASS_FILE, "r")) == NULL)
		return(0);
	while((p1 = fgets(line, sizeof(line)-1, fp)) != NULL) {
		while(*p1 && *p1 != ':')
			p1++;
		*p1++ = '\0';
		p2 = p1;
		while(*p1 && *p1 != ':')
			p1++;
		*p1 = '\0';
		if(strcmp(pwd->pw_shell, line) == 0) {
			fclose(fp);
			if (*p2 == '\0')
				return(0);
			return(gpass("Dialup Password:", p2));
		}
		if(strcmp(SHELL, line) == 0)
			SCPYN(defpass, p2);
	}
	fclose(fp);
	return(gpass("Dialup Password:", defpass));
}

gpass(prmt, pswd)
char *prmt, *pswd;
{
	register char *p1;

	p1 = crypt(getpass(prmt), pswd);
	if(strcmp(p1, pswd)) {
		printf("Login incorrect\n");
		return(1);
	}
	return(0);
}

#define	WHITESPACE	0
#define	ARGUMENT	1

char **getargs()
{
	static char envbuf[MAXLINE];
	static char *args[MAXARGS];
	register char *ptr,**answer;
	register int c;
	int state;
	extern int quotec();

	for (ptr= &envbuf[0]; ptr < &envbuf[sizeof(envbuf)];) *ptr++ = '\0';
	for (answer= &args[0]; answer < &args[MAXARGS];)
		*answer++ = (char *)NULL;
	for (ptr= &envbuf[0],answer= &args[0],state = WHITESPACE;
	     (c = getc(stdin)) != EOF;) {

		switch (c) {
		case '\n' :
			if (ptr == &envbuf[0]) return((char **)NULL);
			else return(&args[0]);
		case ' ' :
		case '\t' :
			if (state == ARGUMENT) {
				*ptr++ = '\0';
				state = WHITESPACE;
			}
			break;
		case '\\' :
			c = quotec();
		default :
			if (state == WHITESPACE) {
				*answer++ = ptr;
				state = ARGUMENT;
			}
			*ptr++ = c;
		}

/*	If the buffer is full, force the next character to be read to	*/
/*	be a <newline>.							*/

		if (ptr == &envbuf[sizeof(envbuf)-1]) {
			ungetc('\n',stdin);
			putc('\n',stdout);
		}
	}

/*	If we left loop because an EOF was received, exit immediately.	*/

	exit(0);
}

int quotec()
{
	register int c,i,num;

	switch(c = getc(stdin)) {
	case 'n' :
		c = '\n';
		break;
	case 'r' :
		c = '\r';
		break;
	case 'v' :
		c = 013;
		break;
	case 'b' :
		c = '\b';
		break;
	case 't' :
		c = '\t';
		break;
	case 'f' :
		c = '\f';
		break;
	case '0' :
	case '1' :
	case '2' :
	case '3' :
	case '4' :
	case '5' :
	case '6' :
	case '7' :
		for (num=0,i=0; i < 3;i++) {
			num = num * 8 + (c - '0');
			if ((c = getc(stdin)) < '0' || c > '7')
				break;
		}
		ungetc(c,stdin);
		c = num & 0377;
		break;
	default :
		break;
	}
	return (c);
}

/*
 * terminal(f): return "/dev/ttyXX" which the the name of the
 * tty belonging to file f.  This routine is the same as ttyname()
 * except that it rejects /dev/syscon and /dev/systty, which are
 * links to other device names.
 *
 * This program works in two passes: the first pass tries to
 * find the device by matching device and inode numbers; if
 * that doesn't work, it tries a second time, this time doing a
 * stat on every file in /dev and trying to match device numbers
 * only. If that fails too, NULL is returned.
 */

char *
terminal(f)
int	f;
{
	struct stat fsb, tsb;
	struct dirent *dp;
	DIR *dirf;
	register int pass1;
	static char rbuf[32], dev[]="/dev/";

	if(isatty(f) == 0)
		return(NULL);
	if(fstat(f, &fsb) < 0)
		return(NULL);
	if((fsb.st_mode & S_IFMT) != S_IFCHR)
		return(NULL);
	if((dirf = opendir(dev)) == NULL)
		return(NULL);
	pass1 = 1;
	do {
		while((dp = readdir(dirf)) != NULL) {
			if(dp->d_ino == 0)
				continue;
			if(pass1 && dp->d_ino != fsb.st_ino)
				continue;
			if (strncmp(&dp->d_name[0],"syscon",DIRSIZ) == 0 ||
				strncmp(&dp->d_name[0],"systty",DIRSIZ) == 0)
				continue;
			(void) strcpy(rbuf, dev);
			(void) strcat(rbuf, dp->d_name);
			if(stat(rbuf, &tsb) < 0)
				continue;
			if(tsb.st_rdev == fsb.st_rdev &&
				(tsb.st_mode&S_IFMT) == S_IFCHR &&
				(!pass1 || tsb.st_ino == fsb.st_ino)) {
				(void) closedir(dirf);
				return(rbuf);
			}
		}
		(void) rewinddir(dirf);
	} while(pass1--);
	(void) closedir(dirf);
	return(NULL);
}


doremotelogin(host)
	char *host;
{
	strncpy(remotehost, host, sizeof(remotehost)-1);
	getstr(rusername, sizeof (rusername), "remuser");
	getstr(lusername, sizeof (lusername), "locuser");
	getstr(term, sizeof(term), "Terminal type");
	if (getuid()) {
		pwd = &nouser;
		return(-1);
	}
	pwd = getpwnam(lusername);
	if (pwd == NULL) {
		pwd = &nouser;
		return(-1);
	}
	SCPYN(utmp.ut_user, lusername);
	SCPYN(u_name, lusername);
	return(ruserok(host, (pwd->pw_uid == 0), rusername, lusername));
}

char	*speeds[] =
    { "0", "50", "75", "110", "134", "150", "200", "300",
      "600", "1200", "1800", "2400", "4800", "9600", "19200", "38400" };
#define	NSPEEDS	(sizeof (speeds) / sizeof (speeds[0]))

doremoteterm(term)
	char *term;
{
	register char *cp, **cpp;
	struct termio tp;
	char *speed;

	(void)ioctl(0, TCGETA, &tp);
	cp = strchr(term, '/');
	if (cp) {
		*cp++ = '\0';
		speed = cp;
		cp = index(speed, '/');
		if (cp)
			*cp++ = '\0';
		for (cpp = speeds; cpp < &speeds[NSPEEDS]; cpp++)
			if (strcmp(*cpp, speed) == 0) {
				tp.c_cflag &= ~CBAUD;
				tp.c_cflag |= (cpp-speeds);
				break;
		}
	}

	tp.c_iflag |= (BRKINT|IGNPAR|ISTRIP|ICRNL|IXON);
	tp.c_oflag |= (OPOST|ONLCR);
	tp.c_lflag |= (ISIG|ICANON|ECHO|ECHOE|ECHOK);
	tp.c_cc[VEOL] = '\n';
	tp.c_cc[VEOF] = CEOF;
	(void)ioctl(0, TCSETA, &tp);
}

getstr(buf, cnt, err)
	char *buf;
	int cnt;
	char *err;
{
	char c;

	do {
		if (read(0, &c, 1) != 1)
			exit(1);
		if (--cnt < 0) {
			printf("%s too long\r\n", err);
			exit(1);
		}
		*buf++ = c;
	} while (c != 0);
}



/* why is this stuff here? It is not present in the 5.3 version of login
 */
#ifdef notdef
/*	@(#)getut.c	1.1	*/

/*	Routines to read and write the /etc/utmp file.			*/
/*									*/

#define	MAXFILE	79	/* Maximum pathname length for "utmp" file */

#ifdef	DEBUG
#undef	UTMP_FILE
#define	UTMP_FILE "utmp"
#endif

static int fd = -1;	/* File descriptor for the utmp file. */
static char utmpfile[MAXFILE+1] = UTMP_FILE;	/* Name of the current
						 * "utmp" like file.
						 */
static long loc_utmp;	/* Where in "utmp" the current "ubuf" was
			 * found.
			 */
static struct utmp ubuf;	/* Copy of last entry read in. */


/* "getutent" gets the next entry in the utmp file.
 */

struct utmp *getutent()
{
	extern int fd;
	extern char utmpfile[];
	extern struct utmp ubuf;
	extern long loc_utmp,lseek();
	extern int errno;
	register char *u;
	register int i;
	struct stat stbuf;

/* If the "utmp" file is not open, attempt to open it for
 * reading.  If there is no file, attempt to create one.  If
 * both attempts fail, return NULL.  If the file exists, but
 * isn't readable and writeable, do not attempt to create.
 */

	if (fd < 0) {

/* Make sure file is a multiple of 'utmp' entries long */
		if (stat(utmpfile,&stbuf) == 0) {
			if((stbuf.st_size % sizeof(struct utmp)) != 0) {
				unlink(utmpfile);
			}
		}
		if ((fd = open(utmpfile, O_RDWR|O_CREAT, 0644)) < 0) {

/* If the open failed for permissions, try opening it only for
 * reading.  All "pututline()" later will fail the writes.
 */
			if (errno == EACCES
			    && (fd = open(utmpfile, O_RDONLY)) < 0)
				return(NULL);
		}
	}

/* Try to read in the next entry from the utmp file.  */
	if (read(fd,&ubuf,sizeof(ubuf)) != sizeof(ubuf)) {

/* Make sure ubuf is zeroed. */
		for (i=0,u=(char *)(&ubuf); i<sizeof(ubuf); i++) *u++ = '\0';
		loc_utmp = 0;
		return(NULL);
	}

/* Save the location in the file where this entry was found. */
	loc_utmp = lseek(fd,0L,1) - (long)(sizeof(struct utmp));
	return(&ubuf);
}

/*	"getutid" finds the specified entry in the utmp file.  If	*/
/*	it can't find it, it returns NULL.				*/

struct utmp *getutid(entry)
register struct utmp *entry;
{
	extern struct utmp ubuf;
	struct utmp *getutent();
	register short type;

/* Start looking for entry.  Look in our current buffer before */
/* reading in new entries. */
	do {

/* If there is no entry in "ubuf", skip to the read. */
		if (ubuf.ut_type != EMPTY) {
			switch(entry->ut_type) {

/* Do not look for an entry if the user sent us an EMPTY entry. */
			case EMPTY:
				return(NULL);

/* For RUN_LVL, BOOT_TIME, OLD_TIME, and NEW_TIME entries, only */
/* the types have to match.  If they do, return the address of */
/* internal buffer. */
			case RUN_LVL:
			case BOOT_TIME:
			case OLD_TIME:
			case NEW_TIME:
				if (entry->ut_type == ubuf.ut_type) return(&ubuf);
				break;

/* For INIT_PROCESS, LOGIN_PROCESS, USER_PROCESS, and DEAD_PROCESS */
/* the type of the entry in "ubuf", must be one of the above and */
/* id's must match. */
			case INIT_PROCESS:
			case LOGIN_PROCESS:
			case USER_PROCESS:
			case DEAD_PROCESS:
				if (((type = ubuf.ut_type) == INIT_PROCESS
					|| type == LOGIN_PROCESS
					|| type == USER_PROCESS
					|| type == DEAD_PROCESS)
				    && ubuf.ut_id[0] == entry->ut_id[0]
				    && ubuf.ut_id[1] == entry->ut_id[1]
				    && ubuf.ut_id[2] == entry->ut_id[2]
				    && ubuf.ut_id[3] == entry->ut_id[3])
					return(&ubuf);
				break;

/* Do not search for illegal types of entry. */
			default:
				return(NULL);
			}
		}
	} while (getutent() != NULL);

/* Return NULL since the proper entry wasn't found. */
	return(NULL);
}

/* "getutline" searches the "utmp" file for a LOGIN_PROCESS or
 * USER_PROCESS with the same "line" as the specified "entry".
 */

struct utmp *getutline(entry)
register struct utmp *entry;
{
	extern struct utmp ubuf,*getutent();
	register struct utmp *cur;

/* Start by using the entry currently incore.  This prevents */
/* doing reads that aren't necessary. */
	cur = &ubuf;
	do {
/* If the current entry is the one we are interested in, return */
/* a pointer to it. */
		if (cur->ut_type != EMPTY && (cur->ut_type == LOGIN_PROCESS
		    || cur->ut_type == USER_PROCESS) && strncmp(&entry->ut_line[0],
		    &cur->ut_line[0],sizeof(cur->ut_line)) == 0) return(cur);
	} while ((cur = getutent()) != NULL);

/* Since entry wasn't found, return NULL. */
	return(NULL);
}

/*	"pututline" writes the structure sent into the utmp file.	*/
/*	If there is already an entry with the same id, then it is	*/
/*	overwritten, otherwise a new entry is made at the end of the	*/
/*	utmp file.							*/

struct utmp *pututline(entry)
struct utmp *entry;
{
	int fc;
	struct utmp *answer;
	extern long time();
	extern struct utmp ubuf;
	extern long loc_utmp,lseek();
	extern struct utmp *getutid();
	extern int fd,errno;
	struct utmp tmpbuf;

/* Copy the user supplied entry into our temporary buffer to */
/* avoid the possibility that the user is actually passing us */
/* the address of "ubuf". */
	tmpbuf = *entry;
	getutent();
	if (fd < 0) {
#ifdef	ERRDEBUG
		gdebug("pututline: Unable to create utmp file.\n");
#endif
		return((struct utmp *)NULL);
	}
/* Make sure file is writable */
	if ((fc=fcntl(fd, F_GETFL, NULL)) == -1
	    || (fc & O_RDWR) != O_RDWR) {
		return((struct utmp *)NULL);
	}

/* Find the proper entry in the utmp file.  Start at the current */
/* location.  If it isn't found from here to the end of the */
/* file, then reset to the beginning of the file and try again. */
/* If it still isn't found, then write a new entry at the end of */
/* the file.  (Making sure the location is an integral number of */
/* utmp structures into the file incase the file is scribbled.) */

	if (getutid(&tmpbuf) == NULL) {
#ifdef	ERRDEBUG
		gdebug("First getutid() failed.  fd: %d",fd);
#endif
		setutent();
		if (getutid(&tmpbuf) == NULL) {
#ifdef	ERRDEBUG
			loc_utmp = lseek(fd, 0L, 1);
			gdebug("Second getutid() failed.  fd: %d loc_utmp: %ld\n",fd,loc_utmp);
#endif
			fcntl(fd, F_SETFL, fc | O_APPEND);
		} else {
			lseek(fd, -(long)sizeof(struct utmp), 1);
		}
	} else {
		lseek(fd, -(long)sizeof(struct utmp), 1);
	}

/* Write out the user supplied structure.  If the write fails, */
/* then the user probably doesn't have permission to write the */
/* utmp file. */
	if (write(fd,&tmpbuf,sizeof(tmpbuf)) != sizeof(tmpbuf)) {
#ifdef	ERRDEBUG
		gdebug("pututline failed: write-%d\n",errno);
#endif
		answer = (struct utmp *)NULL;
	} else {
/* Copy the user structure into ubuf so that it will be up to */
/* date in the future. */
		ubuf = tmpbuf;
		answer = &ubuf;

#ifdef	ERRDEBUG
		gdebug("id: %c%c loc: %x\n",ubuf.ut_id[0],ubuf.ut_id[1],
		    ubuf.ut_id[2],ubuf.ut_id[3],loc_utmp);
#endif
	}
	fcntl(fd, F_SETFL, fc);
	return(answer);
}

/*	"setutent" just resets the utmp file back to the beginning.	*/

void
setutent()
{
	register char *ptr;
	register int i;
	extern int fd;
	extern struct utmp ubuf;
	extern long loc_utmp;

	if (fd != -1) lseek(fd,0L,0);

/* Zero the stored copy of the last entry read, since we are */
/* resetting to the beginning of the file. */

	for (i=0,ptr=(char*)&ubuf; i < sizeof(ubuf);i++) *ptr++ = '\0';
	loc_utmp = 0L;
}

/*	"endutent" closes the utmp file.				*/

void
endutent()
{
	extern int fd;
	extern long loc_utmp;
	extern struct utmp ubuf;
	register char *ptr;
	register int i;

	if (fd != -1) close(fd);
	fd = -1;
	loc_utmp = 0;
	for (i=0,ptr= (char *)(&ubuf); i < sizeof(ubuf);i++) *ptr++ = '\0';
}

/*	"utmpname" allows the user to read a file other than the	*/
/*	normal "utmp" file.						*/

utmpname(newfile)
char *newfile;
{
	extern char utmpfile[];

/* Determine if the new filename will fit.  If not, return 0. */
	if (strlen(newfile) > MAXFILE) return (0);

/* Otherwise copy in the new file name. */
	else strcpy(&utmpfile[0],newfile);

/* Make sure everything is reset to the beginning state. */
	endutent();
	return(1);
}

#ifdef	ERRDEBUG

gdebug(format,arg1,arg2,arg3,arg4,arg5,arg6)
char *format;
int arg1,arg2,arg3,arg4,arg5,arg6;
{
	register FILE *fp;
	register int errnum;
	extern int errno;

	if ((fp = fopen("/etc/dbg.getut","a+")) == NULL) return;
	fprintf(fp,format,arg1,arg2,arg3,arg4,arg5,arg6);
	fclose(fp);
}
#endif
#endif
