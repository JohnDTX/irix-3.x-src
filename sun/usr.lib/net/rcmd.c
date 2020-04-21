#ifndef lint
static	char sccsid[] = "@(#)rcmd.c 1.1 85/05/30 SMI"; /* from UCB 4.8 83/03/19 */
#endif

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>

#include <netdb.h>
#include <errno.h>

extern	errno;
#ifdef sgi
char	*index();
#else
char	*index(), *sprintf();
#endif

rcmd(ahost, rport, locuser, remuser, cmd, fd2p)
	char **ahost;
	int rport;
	char *locuser, *remuser, *cmd;
	int *fd2p;
{
	int s, timo = 1;
	struct sockaddr_in sin, sin2, from;
	char c;
	int lport = IPPORT_RESERVED - 1;
	struct hostent *hp;

	hp = gethostbyname(*ahost);
	if (hp == 0) {
		fprintf(stderr, "%s: unknown host\n", *ahost);
		return (-1);
	}
	*ahost = hp->h_name;
retry:
	s = rresvport(&lport);
	if (s < 0)
		return (-1);
	sin.sin_family = hp->h_addrtype;
	bcopy(hp->h_addr, (caddr_t)&sin.sin_addr, hp->h_length);
	sin.sin_port = rport;
	if (connect(s, (caddr_t)&sin, sizeof (sin), 0) < 0) {
		if (errno == EADDRINUSE) {
			close(s);
			lport--;
			goto retry;
		}
		if (errno == ECONNREFUSED && timo <= 16) {
			(void) close(s);
			sleep(timo);
			timo *= 2;
			goto retry;
		}
		perror(hp->h_name);
		return (-1);
	}
	lport--;
	if (fd2p == 0) {
		write(s, "", 1);
		lport = 0;
	} else {
		char num[8];
		int s2 = rresvport(&lport), s3;

		if (s2 < 0) {
			(void) close(s);
			return (-1);
		}
		listen(s2, 1);
		(void) sprintf(num, "%d", lport);
		if (write(s, num, strlen(num)+1) != strlen(num)+1) {
			perror("write: setting up stderr");
			(void) close(s2);
			goto bad;
		}
		{ int len = sizeof (from);
		  s3 = accept(s2, &from, &len, 0);
		  close(s2);
		  if (s3 < 0) {
			perror("accept");
			lport = 0;
			goto bad;
		  }
		}
		*fd2p = s3;
		from.sin_port = ntohs((u_short)from.sin_port);
		if (from.sin_family != AF_INET ||
		    from.sin_port >= IPPORT_RESERVED) {
			fprintf(stderr,
			    "socket: protocol failure in circuit setup.\n");
			goto bad2;
		}
	}
	(void) write(s, locuser, strlen(locuser)+1);
	(void) write(s, remuser, strlen(remuser)+1);
	(void) write(s, cmd, strlen(cmd)+1);
	if (read(s, &c, 1) != 1) {
		perror(*ahost);
		goto bad2;
	}
	if (c != 0) {
		while (read(s, &c, 1) == 1) {
			(void) write(2, &c, 1);
			if (c == '\n')
				break;
		}
		goto bad2;
	}
	return (s);
bad2:
	if (lport)
		(void) close(*fd2p);
bad:
	(void) close(s);
	return (-1);
}

rresvport(alport)
	int *alport;
{
	struct sockaddr_in sin;
	int s;

	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = 0;
	s = socket(AF_INET, SOCK_STREAM, 0, 0);
	if (s < 0)
		return (-1);
	for (;;) {
		sin.sin_port = htons((u_short)*alport);
		if (bind(s, (caddr_t)&sin, sizeof (sin), 0) >= 0)
			return (s);
		if (errno != EADDRINUSE && errno != EADDRNOTAVAIL) {
			perror("socket");
#ifdef sgi
			(void) close(s);
#endif
			return (-1);
		}
		(*alport)--;
		if (*alport == IPPORT_RESERVED/2) {
			fprintf(stderr, "socket: All ports in use\n");
#ifdef sgi
			(void) close(s);
#endif
			return (-1);
		}
	}
}

ruserok(rhost, superuser, ruser, luser)
	char *rhost;
	int superuser;
	char *ruser, *luser;
{
	FILE *hostf;
	char ahost[32];
	int first = 1;
	char domain[256];

	if (getdomainname(domain, sizeof(domain)) < 0) {
		fprintf(stderr, "rcmd: getdomainname system call missing\n");
		exit(1);
	}
	hostf = superuser ? (FILE *)0 : fopen("/etc/hosts.equiv", "r");
again:
	if (hostf) {
		char ahost[32];
		int hostmatch, usermatch;

#ifdef sgi
		hostmatch = usermatch = 0;
#endif
		while (fgets(ahost, sizeof (ahost), hostf)) {
			char *user;

			if ((user = index(ahost, '\n')) != 0)
				*user++ = '\0';
			if ((user = index(ahost, ' ')) != 0)
				*user++ = '\0';
			if (ahost[0] == '+' && ahost[1] == 0)
				hostmatch = 1;
			else if (ahost[0] == '+' && ahost[1] == '@') {
				hostmatch = innetgr(ahost + 2, rhost,
				    ruser, domain);
			} else if (ahost[0] == '-' && ahost[1] == '@') {
				if (innetgr(ahost + 2, rhost, ruser, domain))
					break;
			}
			else if (ahost[0] == '-') {
				if (!strcmp(rhost, ahost+1))
					break;
			}
			else
				hostmatch = !strcmp(rhost, ahost);

			if (user) {
				if (user[0] == '+' && user[1] == 0)
					usermatch = 1;
				else if (user[0] == '+' && user[1] == '@')
					usermatch = innetgr(user+2, rhost,
					    ruser, domain);
				else if (user[0] == '-' && user[1] == '@') {
					if (innetgr(user+2, rhost,
					    ruser, domain))
						break;
				}
				else if (user[0] == '-') {
					if (!strcmp(user+1, ruser))
						break;
				}
				else
					usermatch = !strcmp(user, ruser);
			}
			else
				usermatch = !strcmp(ruser, luser);
			if (hostmatch && usermatch) {
				fclose(hostf);
				return (0);
			}
		}
		fclose(hostf);
	}
	if (first == 1) {
#ifdef sgi
		/*
		 * Imported from BSD4.3 version of ruserok
		 */
#include <sys/param.h>
#include <sys/stat.h>
#include <pwd.h>
		struct stat sbuf;
		struct passwd *pwd;
		char pbuf[MAXPATHLEN];

		first = 0;
		if ((pwd = getpwnam(luser)) == NULL)
			return(-1);
		(void)strcpy(pbuf, pwd->pw_dir);
		(void)strcat(pbuf, "/.rhosts");
		if ((hostf = fopen(pbuf, "r")) == NULL)
			return(-1);
		(void)fstat(fileno(hostf), &sbuf);
		if (sbuf.st_uid && sbuf.st_uid != pwd->pw_uid) {
			fclose(hostf);
			return(-1);
		}
#else
		first = 0;
		hostf = fopen(".rhosts", "r");
#endif
		goto again;
	}
	return (-1);
}
