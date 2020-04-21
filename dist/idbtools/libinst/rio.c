/* rio - remote i/o routines
 *
 * This layer of functions masks the fact that io is occuring by spawning a
 * "dd" command on the other end of a network connection.
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/file.h>

#include <errno.h>
#include <pwd.h>
#include <signal.h>
#include <stdio.h>
#include <netdb.h>

#define Strsize		1024

typedef struct Rio {
	int		ferr;
	fd_set		readfrom;
	fd_set		writeto;
} Rio;

extern struct passwd	*getpwuid ();

static char		myname [Strsize];
static Rio		*rio;
static struct servent	*rshp;

ropen (host, user, name, mode, bsize)
	char		*host;		/* host name */
	char		*user;		/* remote user name */
	char		*name;		/* file name */
	int		mode;		/* 0 for read, 1 for write */
	int		bsize;		/* block size for dd command */
{
	struct passwd	*pw;
	int		f, f2, one;
	char		cmd [1024];

	if (rio == NULL) {
		rio = (Rio *) malloc (getdtablesize () * sizeof (Rio));
		if (rio == NULL) {
			fprintf (stderr, "Can't get memory in ropen.\n");
			return (-1);
		}
	}
	if (*myname == '\0') {
		if ((pw = getpwuid (getuid ())) != NULL) {
			strcpy (myname, pw->pw_name);
		} else {
			strcpy (myname, "root");
		}
	}
	if (rshp == NULL) {
		if ((rshp = getservbyname ("shell", "tcp")) == NULL) {
			return (-1);
		}
	}
	sprintf (cmd, "dd %cf=%s bs=%d", mode == 0 ? 'i' : 'o', name, bsize);
	if ((f = rcmd (&host, rshp->s_port, myname, user, cmd, &f2)) < 0) {
		return (-1);
	}
	one = 1;
	ioctl (f, FIONBIO, &one);
	ioctl (f2, FIONBIO, &one);
	rio [f].ferr = f2;
	FD_ZERO (&rio [f].readfrom);
	FD_ZERO (&rio [f].writeto);
	if (mode == 0) {
		FD_SET (f, &rio [f].readfrom);
	} else {
		FD_SET (f, &rio [f].writeto);
	}
	FD_SET (f2, &rio [f].readfrom);
	return (f);
}

rread (f, buff, n)
	int		f;
	char		*buff;
	int		n;
{
	Rio		*p;
	fd_set		ready;
	int		cc, nr;
	char		ebuff [4096];

	if (n <= 0) return (n);
	p = rio + f;
	nr = 0;
	while (n) {
		bcopy (&p->readfrom, &ready, sizeof (ready));
		if (select (16, &ready, 0, 0, 0) < 0) {
			if (errno != EINTR) {
				perror ("select");
				return (-1);
			}
			else continue;
		} 
		if (FD_ISSET (p->ferr, &ready)) {
			errno = 0;
			cc = read (p->ferr, ebuff, sizeof (ebuff));
			if (cc <= 0) {
				if (errno != EWOULDBLOCK) {
					FD_CLR (p->ferr, &p->readfrom);
				}
			} else {
				if (!ignoremsg (ebuff)) {
					write (2, ebuff, cc);
					return (-1);
				}
			}
		}
		if (FD_ISSET (f, &ready)) {
			errno = 0;
			cc = read (f, buff, n);
			if (cc < 0) {
				if (errno != EWOULDBLOCK) {
					FD_CLR (f, &p->readfrom);
				}
			} else if (cc == 0) {
				break;
			} else {
				nr += cc;
				n -= cc;
				buff += cc;
			}
		}
		if (!FD_ISSET (f, &p->readfrom) &&
		    !FD_ISSET (p->ferr, &p->readfrom)) break;
	}
	return (nr);
}

rwrite (f, buff, n)
	int		f;
	char		*buff;
	int		n;
{
	Rio		*p;
	fd_set		ready, wready;
	int		cc, nw;
	char		ebuff [4096];

	if (n <= 0) return (n);
	p = rio + f;
	nw = 0;
	while (n) {
		bcopy (&p->readfrom, &ready, sizeof (ready));
		bcopy (&p->writeto, &wready, sizeof (wready));
		if (select (16, &ready, &wready, 0, 0) < 0) {
			if (errno != EINTR) {
				perror ("select");
				return (-1);
			}
			else continue;
		} 
		if (FD_ISSET (p->ferr, &ready)) {
			errno = 0;
			cc = read (p->ferr, ebuff, sizeof (ebuff));
			if (cc <= 0) {
				if (errno != EWOULDBLOCK) {
					FD_CLR (p->ferr, &p->readfrom);
				}
			} else {
				if (!ignoremsg (ebuff)) {
					write (2, ebuff, cc);
					return (-1);
				}
			}
		}
		if (FD_ISSET (f, &wready)) {
			errno = 0;
			cc = write (f, buff, n);
			if (cc <= 0) {
				if (errno != EWOULDBLOCK) {
					FD_CLR (f, &p->writeto);
				}
			} else {
				nw += cc;
				n -= cc;
				buff += cc;
			}
		}
		if (!FD_ISSET (f, &p->writeto) &&
		    !FD_ISSET (p->ferr, &p->readfrom)) break;
	}
	return (nw);
}

rclose (f)
	int		f;
{
	char		c, e;

	c = SIGTERM;
	write (rio [f].ferr, &c, 1);
	shutdown (f, 2);
	close (rio [f].ferr);
	return (close (f));
}

ignoremsg (s)
	char		*s;
{
	if (*s < '0' || *s > '9') return (0);
	while (*s >= '0' && *s <= '9' || *s == '+') ++s;
	return (strncmp (s + 1, " records", 8));
}
