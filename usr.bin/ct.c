char _Origin_[] = "System V";

/* @(#)ct.c	1.7 */
/*
 *	ct [-h] [-v] [-w n] [-s speed] telno ...
 *
 *	dials the given telephone number, waits for the
 *	modem to answer, and initiates a login process.
 *
 */

#include <errno.h>
#include <fcntl.h>
#include <pwd.h>
#include <signal.h>
#include <stdio.h>
#include <sys/termio.h>
#include <time.h>
#include <sys/types.h>
#include <sys/dir.h>
#include <sys/stat.h>
#include <utmp.h>

#define ROOT	0
#define SYS	3
#define TTYMOD	0666
#define DEV	"/dev/"
#define LCK	"/usr/spool/uucp/LCK.."
#define LEGAL	"0123456789-*:#;e<w=f"

char	log[] = "/usr/adm/ctlog",
	getty[] = "/etc/getty",
	acu[sizeof DEV + DIRSIZ - 5] = DEV,
	tty[sizeof DEV + DIRSIZ - 5] = DEV,
	lock[sizeof LCK + DIRSIZ - 5] = LCK,
	devtab[] = "/usr/lib/uucp/L-devices";

extern int optind;
extern char *optarg;
int	pid,
	status,
	verbose;
char	*num,
	*WTMP = WTMP_FILE,
	*wspeed = "300";
FILE	*fdl,
	*Ldevices;

int	disconnect();
char	*strcpy(),
	*strncpy(),
	*strchr();
long	time();
void	exit(),
	rewind();

unsigned	alarm(),
		sleep();

struct	passwd	*getpwuid();
struct	termio	termio;

time_t	Log_on,
	Log_elpsd;

main(argc, argv)
	char		*argv[];
{
	register int	c, dn = 0;
	register char	*dp, *aptr;
	char		tbuf[32];
	int		dl, count,
			hangup = 1,
			max = -1;

	(void) signal(SIGHUP, SIG_IGN);
	(void) signal(SIGQUIT, SIG_IGN);

	if ((Ldevices = fopen(devtab, "r")) == NULL) {
		(void) fprintf(stderr, "ct: can't open %s\n", devtab);
		exit(1);
	}

	while ((c = getopt(argc, argv, "hvw:s:")) != EOF)
		switch (c) {
		case 'h':
			hangup = 0;
			break;

		case 'v':
			verbose = 1;
			break;

		case 'w':
			max = atoi(optarg);
			break;

		case 's':
			wspeed = optarg;
			break;

		case '?':
			dn++;
			break;
		}

	if (dn || optind == argc) {
		(void) fputs("Usage: ct [-h] [-v] [-w n] [-s speed] telno ...\n", stderr);
		exit(1);
	}

	dn = 0;
	for (count = argc - 1; count >= optind; --count) {
		num = argv[count];
		if (strlen(num) >= sizeof tbuf - 1) {
			(void) fprintf(stderr, "ct: phone number too long -- %s\n", num);
			++dn;
		}
		if (strspn(num, LEGAL) < strlen(num)) {
			(void) fprintf(stderr, "ct: bad phone number -- %s\n", num);
			++dn;
		}
	}
	if (dn)
		exit(1);

	for (count = 0; ; ) {
		dn = gdev();
		if (count == 0) {
			if (dn >= 0)
				(void) fprintf(stderr, "Allocated dialer at %s baud\n", wspeed);
			else {
				(void) fprintf(stderr, "%d busy dialer", -dn);
				if (dn < -1)
					(void) fputc('s', stderr);
				(void) fprintf(stderr, " at %s baud\n", wspeed);
				if (max <= 0) {
					if (!isatty(fileno(stdin)))
						max = 0;
					if (max < 0) {
						(void) fputs("Wait for dialer? ", stderr);
						if ((c = getchar()) == EOF || tolower(c) != 'y')
							exit(1);
						while (c != '\n')
							c = getchar();
						(void) fputs("Time, in minutes? ", stderr);
						(void) scanf("%d", &max);
						while (getchar() != '\n');
					}
					if (max <= 0)
						exit(1);
				} else
					(void) fputs("Waiting for dialer\n", stderr);
			}
			if (!isatty(fileno(stdin)))
				hangup = 0;
			if (hangup) {
				(void) fputs("Confirm hang-up? ", stderr);
				if ((c = getchar()) == EOF || tolower(c) != 'y')
					if (dn >= 0)
						error();
					else
						exit(1);
				while (c != '\n')
					c = getchar();
				if (isatty(fileno(stdout)))
					verbose = 0;
				(void) ioctl(0, TCSETAW, &termio);
				(void) sleep(5);
			}
			(void) close(2);
			(void) dup(1);
		}
		if (dn >= 0)
			break;
		if (verbose && count) {
			(void) fputs("Dialer", stderr);
			if (dn == -1)
				(void) fputs(" is", stderr);
			else
				(void) fputs("s are", stderr);
			(void) fprintf(stderr, " busy (%d minute", count);
			if (count > 1)
				(void) fputc('s', stderr);
			(void) fputs(")\n", stderr);
		}
		if (count++ >= max) {
			if (verbose)
				(void) fputs("*** TIMEOUT ***\n", stderr);
			exit(1);
		}
		(void) sleep(60);
	}
	if (count && verbose)
		(void) fputs("Allocated dialer\n", stderr);
	if (verbose)
		(void) fprintf(stderr, "acu=\"%s\" tty=\"%s\"\n", acu, tty);
	
	(void) close(Ldevices);

	if ((dl = open(tty, O_RDWR | O_NDELAY)) < 0) {
		if (!hangup || verbose)
			(void) fprintf(stderr, "ct: can't open %s\n", tty);
		error();
	}

	for (count = optind; count < argc; count++) {
		if ((aptr = strchr(strcpy(tbuf, num), '\0'))[-1] != '-') {
			*aptr++ = '-';
			*aptr = '\0';
		}
		if (verbose)
			(void) fprintf(stderr, "Dialing %s\n", tbuf);
		if (write(dn, num, (unsigned) strlen(num)) >= 0)
			break;
		if (!hangup || verbose)
			(void) fprintf(stderr, "ct: write error on %s\n", acu);
		(void) close(dn);
		if ((dn = open(acu, O_WRONLY)) < 0) {
			if (!hangup || verbose)
				(void) fprintf(stderr, "ct: reopen error on %s\n", acu);
			error();
		}
		num = argv[count + 1];
	}
	(void) close(dn);
	if (count == argc)
		error();

	fdl = fopen(tty, "r+");
	(void) close(dl);

	if (verbose)
		(void) fputs("Connected\n", stderr);

	(void) signal(SIGINT, disconnect);
	(void) signal(SIGTERM, disconnect);
	(void) signal(SIGALRM, disconnect);

	(void) sleep(2);
	(void) close(0);

	Log_on = time((long *) 0);

	for (;;) {
		if ((pid = fork()) == 0) {
			startat();
			(void) setpgrp();
			(void) execl(getty, "getty", "-h", "-t60", &tty[5], wspeed, 0);
		}

		if (pid <= 0) {
			if (pid < 0 && (!hangup || verbose))
				(void) fputs("ct: can't fork for getty\n", stderr);
			error();
		}

		while (wait(&status) != pid);

		if ((status & 0xff00) < 0) {
			if (!hangup || verbose)
				(void) fputs("ct: can't exec getty\n", stderr);
			error();
		}

		if (fdl != NULL) {
			rewind(fdl);
			(void) fputs("\nReconnect? ", fdl);

			rewind(fdl);
			(void) alarm(20);
			c = getc(fdl);
			(void) alarm(0);

			if (c == EOF || tolower(c) == 'n')
				disconnect();
			while (c != '\n')
				c = getc(fdl);
		}
	}
}

disconnect()
{
	register int	pid,
			hrs, mins, secs;
	register char	*aptr;
	extern char	*ctime(),
			*getenv();

	(void) signal(SIGINT, SIG_IGN);
	(void) signal(SIGTERM, SIG_IGN);

	Log_elpsd = time((long *) 0) - Log_on;

	(void) unlink(lock);

	(void) ioctl(fileno(fdl), TCSETA, &termio);
	(void) fclose(fdl);

	if (verbose)
		(void) fputs("Disconnected\n", stderr);

	stopat(num);

	if (chown(tty, ROOT, SYS) < 0 || chmod(tty, TTYMOD) < 0) {
		if (verbose)
			(void) fprintf(stderr, "Can't chown/chmod on %s\n", tty);
		exit(1);
	}
	exit(0);
}

error()
{
	(void) unlink(lock);
	exit(-1);
}

gdev()
{
	register char	*lspeed;
	register int	dn;
	char		Lbuf[80],
			*getarg();
	int		lck, pid,
			exists = 0;

	(void) fseek(Ldevices, (long) 0, 0);

	while (fgets(Lbuf, sizeof Lbuf, Ldevices) != NULL) {
		if (strchr("# \t\n", Lbuf[0]) != NULL)
			continue;
		if (strcmp(getarg(Lbuf), "ACU"))
			continue;
		(void) strncpy(&tty[5], getarg((char *) 0), sizeof tty - 6);
		(void) strncpy(&acu[5], getarg((char *) 0), sizeof acu - 6);
		lspeed = getarg((char *) 0);
		if (strcmp(wspeed, lspeed) != 0)
			continue;
		exists++;
		(void) strcpy(&lock[21], &tty[5]);
		if ((lck = open(lock, O_WRONLY | O_CREAT | O_EXCL, 0444)) >= 0
		    && (dn = open(acu, O_WRONLY)) >= 0) {
			(void) signal(SIGINT, error);
			(void) signal(SIGTERM, error);
			pid = getpid();
			(void) write(lck, (char *) &pid, sizeof (pid));
			(void) close(lck);
			return dn;
		}
	}
	if (exists)
		return -exists;
	(void) fprintf(stderr, "No %s baud dialers on this system\n", wspeed);
	error();
/*NOTREACHED*/
}

char *
getarg(p)
	register char	*p;
{
	register char	*q;
	static char	*savepoint;

	if (p == (char *) 0)
		p = savepoint;

	while (*p == ' ' || *p == '\t')
		p++;
	q = p;
	while (*p != ' ' && *p != '\t' && *p != '\0' && *p != '\n')
		p++;
	*p = '\0';
	savepoint = ++p;
	return q;
}

/*
 * Create an entry in utmp file if one does not already exist.
 */
startat()
{
	struct utmp utmpbuf ;
	register struct utmp *u,*oldu ;
	struct utmp *getutid() ;
	extern char *WTMP ;
	FILE *fp ;
/*	Set up the prototype for the utmp structure we want to write.	*/

	u = &utmpbuf ;
	zero(&u->ut_user[0],sizeof(u->ut_user)) ;
	zero(&u->ut_line[0],sizeof(u->ut_line)) ;

/*	Fill in the various fields of the utmp structure.		*/

	u->ut_id[0] = tty[8] ;
	u->ut_id[1] = tty[9] ;
	u->ut_id[2] = '\0' ;
	u->ut_id[3] = '\0' ;
	u->ut_pid = getpid() ;

	u->ut_exit.e_termination = 0 ;
	u->ut_exit.e_exit = 0 ;
	u->ut_type = INIT_PROCESS ;
	time(&u->ut_time) ;
	setutent() ;	/* Start at beginning of utmp file. */

/*	For INIT_PROCESSes put in the name of the program in the	*/
/*	"ut_user" field.						*/

	    strncpy(&u->ut_user[0],"getty",sizeof(u->ut_user)) ;
            strncpy(&u->ut_line[0],(tty+5),sizeof(u->ut_line));

/*	Write out the updated entry to utmp file.			*/
	pututline(u) ;

/*	Now attempt to add to the end of the wtmp file.  Do not create	*/
/*	if it doesn't already exist.  **  Note  ** This is the reason	*/
/*	"r+" is used instead of "a+".  "r+" won't create a file, while	*/
/*	"a+" will.							*/

	if ((fp = fopen(WTMP,"r+")) != NULL)
	  {
	    fseek(fp,0L,2) ;	/* Seek to end of file */
	    fwrite(u,sizeof(*u),1,fp) ;
	    fclose(fp) ;
        }
	endutent() ;
}

/*
 * Change utmp file entry to "dead".
 * Make entry in ct log.
 */
stopat(num)
char *num;
{
	register long stopt;
	struct utmp utmpbuf ;
	register struct utmp *u,*oldu ;
	struct utmp *getutid() ;
	extern char *WTMP ;
	FILE *fp ;

	stopt = time((long *)0);

/*	Set up the prototype for the utmp structure we want to write.	*/

	u = &utmpbuf ;
	zero(&u->ut_user[0],sizeof(u->ut_user)) ;
	zero(&u->ut_line[0],sizeof(u->ut_line)) ;

/*	Fill in the various fields of the utmp structure.		*/

	u->ut_id[0] = tty[8] ;
	u->ut_id[1] = tty[9] ;
	u->ut_id[2] = '\0' ;
	u->ut_id[3] = '\0' ;
	u->ut_pid = pid ;
        strncpy(&u->ut_line[0],(tty+5),sizeof(u->ut_line));
	u->ut_type = USER_PROCESS ;

/*	Find the old entry in the utmp file with the user name and	*/
/*	copy it back.							*/

	if (u = getutid(u))
	  {
	    utmpbuf = *u ;
	    u = &utmpbuf ;
	  }

	u->ut_exit.e_termination = status & 0xff ;
	u->ut_exit.e_exit = (status >> 8) & 0xff ;
	u->ut_type = DEAD_PROCESS ;
	time(&u->ut_time) ;

/*	Write out the updated entry to utmp file.			*/

	pututline(u) ;

/*	Now attempt to add to the end of the wtmp file.  Do not create	*/
/*	if it doesn't already exist.  **  Note  ** This is the reason	*/
/*	"r+" is used instead of "a+".  "r+" won't create a file, while	*/
/*	"a+" will.							*/

	if ((fp = fopen(WTMP,"r+")) != NULL)
	  {
	    fseek(fp,0L,2) ;	/* Seek to end of file */
	    fwrite(u,sizeof(*u),1,fp) ;
	    fclose(fp) ;
        }
	endutent() ;

/*	Do the log accounting 					*/

	if (exists(log) && (fp = fopen (log, "a")) != NULL)
	{
		char		*aptr;
		int		hrs, mins, secs;
		extern char	*ctime (),
				*getenv ();

		*getenv ("TZ") = '\0';
		(aptr = ctime (&Log_on))[16] = '\0';
		hrs = Log_elpsd / 3600;
		mins = (Log_elpsd %= 3600) / 60;
		secs = Log_elpsd % 60;
		(void) fprintf (fp, "%-8s ", getpwuid (getuid ())->pw_name);
		(void) fprintf (fp, "(%4s)  %s ", wspeed, aptr);
		if (hrs)
			(void) fprintf (fp, "%2d:%.2d", hrs, mins);
		else
			(void) fprintf (fp, "   %2d", mins);
		(void) fprintf (fp, ":%.2d  %s\n", secs, num);
		(void) fclose (fp);
	}
}
exists(file)
char *file;
{
	struct stat statb;
	extern errno;

	if (stat(file, &statb) == -1 && errno == ENOENT)
		return(0);
	return(1);
}

trunc(str, chr)
register char *str;
register char chr;
{
	for (;*str;str++) {
		if (*str == chr) {
			*str = '\0';
			break;
		}
	}
}

zero(adr,size)
register char *adr ;
register int size ;
  {
        while (size--) *adr++ = '\0' ;
  }

