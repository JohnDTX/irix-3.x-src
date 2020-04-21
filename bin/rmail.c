/*
 *  RMAIL -- UUCP mail server.
 *
 *	This program reads the >From ... remote from ... lines that
 *	UUCP is so fond of and turns them into something reasonable.
 *	It calls sendmail giving it a -f option built from these
 *	lines.
 */

#include <stdio.h>
#include <sysexits.h>
#include <syslog.h>
#include <pwd.h>
#include <sys/types.h>
#include <utmp.h>

extern char* getenv();
extern char* getlogin();
extern struct passwd* getpwuid();

typedef char	bool;
#define TRUE	1
#define FALSE	0

extern FILE	*popen();
extern char	*strchr();
extern char	*strrchr();

bool	Debug = FALSE;

#define MAILER	"/usr/lib/sendmail"

#define BUFLEN 512
char lbuf[BUFLEN];			/* one line of the message */
char from[BUFLEN] = "";			/* accumulated path of sender */
char ufrom[BUFLEN] = "/dev/null";	/* user on remote system */
char sys_buf[BUFLEN];
char cmd[2000];

main(argc, argv)
	char **argv;
{
	FILE *out;			/* output to sendmail */
	register char *uf;		/* ptr into ufrom */
	register int i;

	/* find out who we are in case there is no 'from' line
	 *	This is because some SGI versions of RCS (ab)use rmail
	 *	to send the lock-breaking notification.
	 */
	uf = getenv("LOGNAME");
	if (!uf || !strlen(uf))
		uf = getlogin();
	if (!uf || !strlen(uf))
	    uf = getpwuid(geteuid())->pw_name;

#ifdef DEBUG
	if (argc > 1 && strcmp(argv[1], "-T") == 0) {
		Debug = TRUE;
		argc--;
		argv++;
	}
#endif DEBUG
	openlog("rmail", LOG_PID | LOG_NOWAIT, LOG_DAEMON);

	if (argc < 2) {
		syslog(LOG_ERR, "argc=%d",argc);
		exit(EX_USAGE);
	}

	for (;;) {
		register char *cp;
		register char *sys;

		if (!fgets(lbuf, sizeof(lbuf), stdin))
			exit(1);	/* quit if file is empty */

		if (strncmp(lbuf, "From ", 5) != 0
		    && strncmp(lbuf, ">From ", 6) != 0)
			break;
		(void)sscanf(lbuf, "%*s %s", ufrom);
		uf = ufrom;
		cp = lbuf;
		for (;;) {
			cp = strchr(cp+1, 'r');
			if (cp == NULL) {
				register char *p = strrchr(uf, '!');

				if (p != NULL) {
					*p = '\0';
					sys = uf;
					uf = p + 1;
				} else if (NULL != (p = strrchr(uf, '@'))) {
					*p = '\0';
					sys = p+1;
					/* take only the end of the route */
					p = strrchr(uf, ':');
					if (NULL != p)
						uf = p+1;
				} else {
					sys = "somewhere";
				}
				break;
			}

			if (1 == sscanf(cp, "remote from %s", sys=sys_buf))
				break;
		}

		if ((strlen(from) + strlen(sys)) <= sizeof(from)) {
			(void)strcat(from, sys);
			(void)strcat(from, "!");
		}
#ifdef DEBUG
		if (Debug)
			syslog(LOG_DEBUG,"ufrom='%s' sys='%s' from='%s'",
			       uf, sys, from);
#endif
	}
	(void)strcat(from, uf);

	(void)sprintf(cmd, "%s -ee -f%s -i", MAILER, from);
	while (*++argv != NULL) {
		(void)strcat(cmd, " '");
		if (**argv == '(')
			(void)strncat(cmd, *argv+1, strlen(*argv)-2);
		else
			(void)strcat(cmd, *argv);
		(void)strcat(cmd, "'");
	}
#ifdef DEBUG
	if (Debug) {
		syslog("cmd='%s' lbuf='%s'", cmd, lbuf);
	}
#endif
	out = popen(cmd, "w");
	fputs(lbuf, out);
	while (fgets(lbuf, sizeof lbuf, stdin))
		fputs(lbuf, out);
	i = pclose(out);
	sync();				/* make things good at this end */
	if ((i & 0377) != 0)
	{
		syslog(LOG_ERR, "pclose: status=0%o", i);
		exit(EX_OSERR);
	}

	exit(0);
}
