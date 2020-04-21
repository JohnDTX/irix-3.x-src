char _Origin_[] = "System V";

static	char sccsid[] = "@(#)passwd.c	1.2";
static	char	*Sccsid = "@(#)$Header: /d2/3.7/src/bin/RCS/passwd.c,v 1.1 89/03/27 14:50:52 root Exp $";
/*
# $Log:	passwd.c,v $
 * Revision 1.1  89/03/27  14:50:52  root
 * Initial check-in for 3.7
 * 
 * Revision 1.4  86/09/10  13:31:48  paulm
 * Print hostname in "changing passwd" message to emphasize the
 * difference between passwd and yppasswd.
 * 
 * Also distinguish between username not in /etc/passwd and
 * permission denied in error reporting.
 * 
 * Revision 1.3  84/11/12  17:46:11  bob
 * 1. Fixed to cope with account names longer than 8 chars (bug).
 * 2. Eliminated race conditions during update.
 * 
 */
/*
 * Enter a password in the password file.
 * This program should be suid with the owner
 * having write permission on /etc
 */

#include <stdio.h>
#include <signal.h>
#include <pwd.h>
#include <sys/types.h>

char	passwd[] = "/etc/passwd";
char	opasswd[] = "/etc/opasswd";
char	temp[]	 = "/etc/ptmp";
struct	passwd *pwd, *getpwent();
void	endpwent();
char	*crypt();
char	*getpass();
char	*pw;
char	pwbuf[10];
char	opwbuf[10];
char	buf[10];
char	hostname[256];

time_t	when;
time_t	now;
time_t	maxweeks;
time_t	minweeks;
long	a64l();
char	*l64a();
long	time();
int	count; /* count verifications */

#define WEEK (24L * 7 * 60 * 60) /* seconds per week */
#define MINLENGTH 3  /* for passwords */

main (argc, argv)
	int argc;
	char *argv[];
{
	char *p;
	int i;
	char saltc[2];
	long salt;
	int u;
	int insist;
	int ok, flags;
	int c;
	int pwlen;
	FILE *tf;
	char *uname, *getlogin();

	setgid(0);	/* needed for proper group ownership of new passwd */
	insist = 0;
	count = 0;
	if (argc < 2) {
		if ((uname = getlogin()) == NULL) {
			fprintf (stderr, "Usage: passwd user\n");
			goto bex;
		}
		gethostname(hostname, sizeof(hostname));
		printf("Changing password for %s on %s\n", uname, hostname);
	} else
		uname = argv[1];

	while ((pwd = getpwent()) != NULL && strncmp (pwd->pw_name, uname, L_cuserid-1) != 0)
		;
	u = getuid();
	if (pwd == NULL) {
		fprintf (stderr, "Not in passwd file.\n");
		goto bex;
	}
	if (u != 0 && u != pwd->pw_uid) {
		fprintf (stderr, "Permission denied.\n");
		goto bex;
	}
	endpwent();
	if (pwd->pw_passwd[0] && u != 0) {
		strcpy (opwbuf, getpass ("Old password:"));
		pw = crypt (opwbuf, pwd->pw_passwd);
		if (strcmp (pw, pwd->pw_passwd) != 0) {
			fprintf (stderr, "Sorry.\n");
			goto bex;
		}
	} else
		opwbuf[0] = '\0';
	if (*pwd->pw_age != NULL) {
		/* password age checking applies */
		when = (long) a64l (pwd->pw_age);
		/* max, min and week of last change are encoded in radix 64 */
		maxweeks = when & 077;
		minweeks = (when >> 6) & 077;
		when >>= 12;
		now  = time ((long *) 0)/WEEK;
		if (when <= now) {
			if (u != 0 && (now < when + minweeks)) {
				fprintf (stderr, "Sorry: < %ld weeks since the last change\n", minweeks);
				goto bex;
			}
			if (minweeks > maxweeks && u != 0) {
				fprintf (stderr, "You may not change this password.\n");
				goto bex;
			}
		}
	}
tryagn:
	strcpy (pwbuf, getpass ("New password:"));
	pwlen = strlen (pwbuf);
	if (u != 0 && (pwlen <= MINLENGTH || strcmp (pwbuf, opwbuf) == 0)) {
		if (pwlen <= MINLENGTH) fprintf (stderr, "Too short. ");
		fprintf (stderr, "Password unchanged.\n");
		goto bex;
	}
	ok = 0;
	flags = 0;
	p = pwbuf;
	while (c = *p++){
		if (c>='a' && c<='z') flags |= 2;
		else if (c>='A' && c<='Z') flags |= 4;
		else if (c>='0' && c<='9') flags |= 1;
		else flags |= 8;
	}
	if (flags >= 7 && pwlen >= 4) ok = 1;
	if ((flags == 2 || flags == 4) && pwlen >= 6) ok = 1;
	if ((flags == 3 || flags == 5 || flags == 6) && pwlen >= 5) ok = 1;

	if (ok == 0 && insist < 3) {
		if (flags==1)
			fprintf (stderr, "Please use at least one non-numeric character.\n");
		else
			fprintf (stderr, "Please use a longer password.\n");
		insist++;
		goto tryagn;
	}
	strcpy (buf, getpass ("Re-enter new password:"));
	if (strcmp (buf, pwbuf)) {
		if (++count > 2) {
			fprintf (stderr, "Too many tries; try again later.\n");
			goto bex;
		} else
			fprintf (stderr, "They don't match; try again.\n");
		goto tryagn;
	}
	time (&salt);
	salt += getpid();

	saltc[0] = salt & 077;
	saltc[1] = (salt >> 6) & 077;
	for (i=0; i<2; i++) {
		c = saltc[i] + '.';
		if (c>'9') c += 7;
		if (c>'Z') c += 6;
		saltc[i] = c;
	}
	pw = crypt (pwbuf, saltc);
	signal (SIGHUP, SIG_IGN);
	signal (SIGINT, SIG_IGN);
	signal (SIGQUIT, SIG_IGN);

	umask (0133);	/* sure don't want to be writable by others */

			/* do access to retain compatibility with old progs */
	if (access (temp, 0) >= 0) {
		fprintf (stderr, "Temporary file busy; try again later.\n");
		goto bex;
	}

		/*
		 * We start the process of
		 *	rm /etc/opasswd
		 *	mv /etc/passwd /etc/opasswd
		 * here to use the link() operation as a mutex semaphore
		 * to lock the passwd file.
		 */

	if (unlink (opasswd) < 0 && access (opasswd, 0) == 0) {
		fprintf (stderr, "cannot unlink %s\n", opasswd);
		goto bex;
	}
	if (link (passwd, opasswd) < 0) {
		fprintf (stderr, "Temporary file busy; try again later.\n");
		goto bex;
	}

	if ((tf = fopen (temp, "w")) == NULL) {
		fprintf (stderr, "Cannot create temporary file\n");
		goto bex;
	}

/*
 *	copy passwd to temp, replacing matching lines
 *	with new password.
 */

	while ((pwd = getpwent()) != NULL) {
		if (strncmp (pwd->pw_name, uname, L_cuserid-1) == 0) {
			u = getuid();
			if (u != 0 && u != pwd->pw_uid) {
				fprintf (stderr, "Permission denied.\n");
				goto out;
			}
			pwd->pw_passwd = pw;
			if (*pwd->pw_age != NULL) {
				if (maxweeks == 0) 
					*pwd->pw_age = '\0';
				else {
					when = maxweeks + (minweeks << 6) + (now << 12);
					pwd->pw_age = l64a (when);
				}
			}
		}
		putpwent (pwd, tf);
	}
	endpwent ();
	fclose (tf);

/*
 *	Finish renaming temp file back to passwd file.
 */

	if (unlink (passwd) < 0) {
		fprintf (stderr, "cannot unlink %s\n", passwd);
		goto out;
	}

	if (link (temp, passwd) < 0) {
		fprintf (stderr, "cannot link %s to %s\n", temp, passwd);
		if (link (opasswd, passwd) < 0) {
			fprintf (stderr, "cannot recover %s\n", passwd);
			goto bex;
		}
		goto out;
	}

	if (unlink (temp) < 0) {
		fprintf (stderr, "cannot unlink %s\n", temp);
		goto out;
	}

	exit (0);

out:
	unlink (temp);

bex:
	exit (1);
}
