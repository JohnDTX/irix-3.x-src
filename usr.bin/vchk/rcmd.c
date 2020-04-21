#ifndef lint
static char sccsid[] = "@(#)rcmd.c	4.1 82/04/02";
#endif

#include <stdio.h>
#include <sys/types.h>
#include <sys/misc.h>
#include <sys/socket.h>
#include <sys/in.h>
#include <errno.h>


#define	swab(x)		((((x) >> 8) | ((x) << 8)) & 0xffff)

extern	errno;
char	*index(), *sprintf();
int	rcmdoptions;

rcmd(ahost, rport, locuser, remuser, cmd, fd2p)
	char **ahost;
	int rport;
	char *locuser, *remuser, *cmd;
	int *fd2p;
{
	int s, timo = 1;
	long addr;
	struct sockaddr_in sin, sin2, from;
	char c;
	short port;

	addr = rhost(ahost);
	if (addr == -1) {
		fprintf(stderr, "%s: unknown host\n", *ahost);
		return (-1);
	}
retry:
	s = rresvport(rcmdoptions|SO_KEEPALIVE);
	if (s < 0)
		return (-1);
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = addr;
	sin.sin_port = rport;
	sin.sin_port = htons(sin.sin_port);
	/*
	fprintf(stderr,"rcmd: connecting on port %d\n", sin.sin_port);
	*/
	if (connect(s, &sin) < 0) {
		if (errno == ECONNREFUSED && timo <= 16) {
			(void) close(s);
			sleep(timo);
			timo *= 2;
			goto retry;
		}
		/*
		fprintf (stderr,"rcmd, cant connect\n");
		*/
		perror(*ahost);
		return (-1);
	}
	/*
	fprintf(stderr,"rcmd: connected\n");
	*/
	if (fd2p == 0) {
		write(s, "", 1);
		port = 0;
	} else {
		char num[8];
		int s2 = rresvport(rcmdoptions|SO_ACCEPTCONN);
		extern errno;

		if (s2 < 0) {
			(void) close(s);
			return (-1);
		}
		if ((socketaddr(s2, &sin2)) < 0) {
			perror("rcmd");
			fprintf (stderr, "error=%d\n", errno);
			exit(1);
		}
		port = sin2.sin_port;
	/*
	fprintf(stderr,"rcmd: port for fd2 = %d\n", port);
	*/
		port = htons((u_short)port);
	/*
	fprintf(stderr,"rcmd: port for fd2 = %d\n", port);
	*/
		(void) sprintf(num, "%d", port);
		(void) write(s, num, strlen(num)+1);
	/*
	fprintf(stderr,"rcmd: wrote %x chars (%s) to net\n", strlen(num)+1, num);
	*/
	/*
	fprintf(stderr,"rcmd: about to accept\n");
	*/
		if (accept(s2, &from) < 0) {
			perror("accept");
			goto bad;
		}
	/*
	fprintf(stderr,"rcmd: accepted\n");
	*/
		from.sin_port = ntohs(from.sin_port);
		if (from.sin_family != AF_INET ||
		    from.sin_port >= IPPORT_RESERVED) {
			fprintf(stderr,
			    "socket: protocol failure in circuit setup.\n");
			fprintf(stderr, "family=%d, port=%d\n",
			    from.sin_family, from.sin_port);
			goto bad;
		}
		*fd2p = s2;
	}
	/*
	fprintf(stderr,"rcmd: writing to the net \n");
	*/
	(void) write(s, locuser, strlen(locuser)+1);
	(void) write(s, remuser, strlen(remuser)+1);
	(void) write(s, cmd, strlen(cmd)+1);
	{ int foo;
		if ((foo=read(s, &c, 1)) != 1) {
			printf ("rcmd, tried to read 1, got %x\n", foo);
			perror(*ahost);
			goto bad;
		}
	}
	if (c != 0) {
		while (read(s, &c, 1) == 1) {
			(void) write(2, &c, 1);
			if (c == '\n')
				break;
		}
		goto bad;
	}
	printf(stderr,"returning from rcmd\n");
	return (s);
bad:
	if (port)
		(void) close(*fd2p);
	(void) close(s);
	return (-1);
}

rresvport(options)
	int options;
{
	struct sockaddr_in sin;
	static short lport = IPPORT_RESERVED - 1;
	int s;

	for (;;) {
		sin.sin_family = AF_INET;
		sin.sin_port = lport;
		sin.sin_addr.s_addr = 0;
		sin.sin_port = htons(sin.sin_port);
		s = socket(SOCK_STREAM, 0, &sin, options);
		if (s >= 0)
			return (s);
		if (errno != EADDRINUSE && errno != EADDRNOTAVAIL) {
			perror("socket");
			return (-1);
		}
		lport--;
		if (lport == IPPORT_RESERVED/2) {
			fprintf(stderr, "socket: All ports in use\n");
			return (-1);
		}
	}
}

#ifdef notdef
ruserok(rhost, ruser, luser)
	char *rhost, *ruser, *luser;
{
	FILE *hostf;
	char ahost[32];
	int first = 1;

	hostf = fopen("/etc/hosts.equiv", "r");
again:
	if (hostf) {
		while (fgets(ahost, sizeof (ahost), hostf)) {
			char *user;
			if (index(ahost, '\n'))
				*index(ahost, '\n') = 0;
			user = index(ahost, ' ');
			if (user)
				*user++ = 0;
			if (!strcmp(rhost, ahost) &&
			    !strcmp(ruser, user ? user : luser))
				goto ok;
		}
		(void) fclose(hostf);
	}
	if (first == 1) {
		first = 0;
		hostf = fopen(".rhosts", "r");
		goto again;
	}
	return (-1);
ok:
	(void) fclose(hostf);
	return (0);
}
#else
ruserok(x,y,z)
{
	return 1;
}
#endif
