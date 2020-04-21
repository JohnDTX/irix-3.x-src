/* NFSSRC @(#)yppasswd.c	2.1 86/04/17 */
#ifndef lint
static  char sccsid[] = "@(#)yppasswd.c 1.1 86/02/05 Copyr 1985 Sun Micro";
#endif

/*
 * Copyright (c) 1985 by Sun Microsystems, Inc.
 */

#include <stdio.h>
#include <signal.h>
#include <pwd.h>
#include <rpc/rpc.h>
#include <rpcsvc/yppasswd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <errno.h>

char *index();
struct yppasswd *getyppw();

main(argc, argv)	
	char **argv;
{
	int ans, port, ok;
	char domain[256];
	char *master;
	struct yppasswd *yppasswd;

	if (getdomainname(domain, sizeof(domain)) < 0) {
		fprintf(stderr, "can't get domainname\n");
		exit(1);
	}
	if (yp_master(domain, "passwd.byname", &master) != 0) {
		fprintf(stderr, "can't get master for passwd file\n");
		exit(1);
	}
	port = getrpcport(master, YPPASSWDPROG, YPPASSWDPROC_UPDATE,
		IPPROTO_UDP);
	if (port == 0) {
		fprintf(stderr, "%s is not running yppasswd daemon\n",master);
		exit(1);
	}
	if (port >= IPPORT_RESERVED) {
		fprintf(stderr,
		    "yppasswd daemon is not running on privileged port\n");
		exit(1);
	}
	yppasswd = getyppw(argc, argv);
	ans = callrpc(master, YPPASSWDPROG, YPPASSWDVERS,
	    YPPASSWDPROC_UPDATE, xdr_yppasswd, yppasswd, xdr_int, &ok);
	if (ans != 0) {
		clnt_perrno(ans);
		fprintf(stderr, "\n");
		fprintf(stderr, "couldn't change passwd\n");
	}
	else if (ok != 0)
		fprintf(stderr, "couldn't change passwd\n");
	else
		printf("yellow pages passwd changed on %s\n", master);
	free(master);
}

struct	passwd *pwd;
#ifndef sgi
struct	passwd *getpwent();
int	endpwent();
#endif
char	*strcpy();
char	*crypt();
char	*getpass();
char	*getlogin();
char	*pw;
char	pwbuf[10];
char	pwbuf1[10];
char	hostname[256];
extern	int errno;

struct yppasswd *
getyppw(argc, argv)
	char *argv[];
{
	char *p;
	int i;
	char saltc[2];
	long salt;
	int u;
	int insist;
	int ok, flags;
	int c, pwlen;
	char *uname;
	static struct yppasswd yppasswd;

	insist = 0;
	uname = NULL;
	if (argc > 1)
		uname = argv[1];
	if (uname == NULL) {
		if ((uname = getlogin()) == NULL) {
			fprintf(stderr, "you don't have a login name\n");
			exit(1);
		}
		gethostname(hostname, sizeof(hostname));
		printf("Changing yp password for %s\n", uname);
	}

	while (((pwd = getpwent()) != NULL) && strcmp(pwd->pw_name, uname))
		;
	u = getuid();
	if (pwd == NULL) {
		printf("Not in passwd file.\n");
		exit(1);
	}
	if (u != 0 && u != pwd->pw_uid) {
		printf("Permission denied.\n");
		exit(1);
	}
	endpwent();
	/* getpass actually returns up to 50 chars */
	strncpy(pwbuf1, getpass("Old yp password:"), sizeof(pwbuf1));
	pwbuf1[sizeof(pwbuf1)-1] = 0;
tryagain:
	strcpy(pwbuf, getpass("New password:"));
	pwlen = strlen(pwbuf);
	if (pwlen == 0) {
		printf("Password unchanged.\n");
		exit(1);
	}
	/*
	 * Insure password is of reasonable length and
	 * composition.  If we really wanted to make things
	 * sticky, we could check the dictionary for common
	 * words, but then things would really be slow.
	 */
	ok = 0;
	flags = 0;
	p = pwbuf;
	while (c = *p++) {
		if (c >= 'a' && c <= 'z')
			flags |= 2;
		else if (c >= 'A' && c <= 'Z')
			flags |= 4;
		else if (c >= '0' && c <= '9')
			flags |= 1;
		else
			flags |= 8;
	}
	if (flags >= 7 && pwlen >= 4)
		ok = 1;
	if ((flags == 2 || flags == 4) && pwlen >= 6)
		ok = 1;
	if ((flags == 3 || flags == 5 || flags == 6) && pwlen >= 5)
		ok = 1;
	if (!ok && insist < 2) {
		printf("Please use %s.\n", flags == 1 ?
			"at least one non-numeric character" :
			"a longer password");
		insist++;
		goto tryagain;
	}
	if (strcmp(pwbuf, getpass("Retype new password:")) != 0) {
		printf("Mismatch - password unchanged.\n");
		exit(1);
	}
	time(&salt);
	salt = 9 * getpid();
	saltc[0] = salt & 077;
	saltc[1] = (salt>>6) & 077;
	for (i = 0; i < 2; i++) {
		c = saltc[i] + '.';
		if (c > '9')
			c += 7;
		if (c > 'Z')
			c += 6;
		saltc[i] = c;
	}
	pw = crypt(pwbuf, saltc);
	yppasswd.oldpass = pwbuf1;
	pwd->pw_passwd = pw;
	yppasswd.newpw = *pwd;
	return (&yppasswd);
}
