/*
**  Sendmail
**  Copyright (c) 1983  Eric P. Allman
**  Berkeley, California
**
**  Copyright (c) 1983 Regents of the University of California.
**  All rights reserved.  The Berkeley software License Agreement
**  specifies the terms and conditions for redistribution.
*/


# include <errno.h>
# include "sendmail.h"

# ifndef DAEMON
# ifndef lint
static char	SccsId[] = "@(#)daemon.c	5.19 (Berkeley) 5/6/86	(w/o daemon mode)";
# endif not lint
# else

# include <netdb.h>
# include <sys/signal.h>
# include <sys/wait.h>
# include <sys/time.h>
# include <sys/resource.h>
#if defined(sgi) || defined(sgi_vax)
#include <strings.h>
#include <fcntl.h>
#undef DBM
#include <ndbm.h>
#endif

# ifndef lint
static char	SccsId[] = "@(#)daemon.c	5.19 (Berkeley) 5/6/86 (with daemon mode)";
# endif not lint

/*
**  DAEMON.C -- routines to use when running as a daemon.
**
**	This entire file is highly dependent on the 4.2 BSD
**	interprocess communication primitives.  No attempt has
**	been made to make this file portable to Version 7,
**	Version 6, MPX files, etc.  If you should try such a
**	thing yourself, I recommend chucking the entire file
**	and starting from scratch.  Basic semantics are:
**
**	getrequests()
**		Opens a port and initiates a connection.
**		Returns in a child.  Must set InChannel and
**		OutChannel appropriately.
**	clrdaemon()
**		Close any open files associated with getting
**		the connection; this is used when running the queue,
**		etc., to avoid having extra file descriptors during
**		the queue run and to avoid confusing the network
**		code (if it cares).
**	makeconnection(host, port, outfile, infile)
**		Make a connection to the named host on the given
**		port.  Set *outfile and *infile to the files
**		appropriate for communication.  Returns zero on
**		success, else an exit status describing the
**		error.
**	maphostname(hbuf, hbufsize)
**		Convert the entry in hbuf into a canonical form.  It
**		may not be larger than hbufsize.
*/
/*
**  GETREQUESTS -- open mail IPC port and get requests.
**
**	Parameters:
**		none.
**
**	Returns:
**		none.
**
**	Side Effects:
**		Waits until some interesting activity occurs.  When
**		it does, a child is created to process it, and the
**		parent waits for completion.  Return from this
**		routine is always in the child.  The file pointers
**		"InChannel" and "OutChannel" should be set to point
**		to the communication channel.
*/

struct sockaddr_in	SendmailAddress;/* internet address of sendmail */

int	DaemonSocket	= -1;		/* fd describing socket */
char	*NetName;			/* name of home (local?) network */

getrequests()
{
	int t;
	register struct servent *sp;
	int on = 1;
	extern reapchild();

	/*
	**  Set up the address for the mailer.
	*/

	sp = getservbyname("smtp", "tcp");
	if (sp == NULL)
	{
		syserr("server \"smtp\" unknown");
		goto severe;
	}
	SendmailAddress.sin_family = AF_INET;
	SendmailAddress.sin_addr.s_addr = INADDR_ANY;
	SendmailAddress.sin_port = sp->s_port;

	/*
	**  Try to actually open the connection.
	*/

# ifdef DEBUG
	if (tTd(15, 1))
		printf("getrequests: port 0x%x\n", SendmailAddress.sin_port);
# endif DEBUG

	/* get a socket for the SMTP connection */
	DaemonSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (DaemonSocket < 0)
	{
		/* probably another daemon already */
		syserr("getrequests: can't create socket");
	  severe:
# ifdef LOG
		if (LogLevel > 0)
			syslog(LOG_ALERT, "cannot get connection");
# endif LOG
		finis();
	}

#ifdef DEBUG
	/* turn on network debugging? */
	if (tTd(15, 15))
		(void) setsockopt(DaemonSocket, SOL_SOCKET, SO_DEBUG, (char *)&on, sizeof on);
#endif DEBUG

	(void) setsockopt(DaemonSocket, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof on);
	(void) setsockopt(DaemonSocket, SOL_SOCKET, SO_KEEPALIVE, (char *)&on, sizeof on);

	if (bind(DaemonSocket, &SendmailAddress, sizeof SendmailAddress) < 0)
	{
		syserr("getrequests: cannot bind");
		(void) close(DaemonSocket);
		goto severe;
	}
	if (listen(DaemonSocket, 10) < 0)
	{
		syserr("getrequests: cannot listen");
		(void) close(DaemonSocket);
		goto severe;
	}

	(void) signal(SIGCHLD, reapchild);

# ifdef DEBUG
	if (tTd(15, 1))
		printf("getrequests: %d\n", DaemonSocket);
# endif DEBUG

	for (;;)
	{
		register int pid;
		auto int lotherend;
		struct sockaddr_in otherend;
		extern int RefuseLA;

		/* see if we are rejecting connections */
		while (getla() > RefuseLA)
			sleep(5);

		/* wait for a connection */
		do
		{
			errno = 0;
			lotherend = sizeof otherend;
			t = accept(DaemonSocket, &otherend, &lotherend);
		} while (t < 0 && errno == EINTR);
		if (t < 0)
		{
			syserr("getrequests: accept");
			sleep(5);
			continue;
		}

		/*
		**  Create a subprocess to process the mail.
		*/

# ifdef DEBUG
		if (tTd(15, 2))
			printf("getrequests: forking (fd = %d)\n", t);
# endif DEBUG

		pid = fork();
		if (pid < 0)
		{
			syserr("daemon: cannot fork");
			sleep(10);
			(void) close(t);
			continue;
		}

		if (pid == 0)
		{
			extern struct hostent *gethostbyaddr();
			register struct hostent *hp;
			char buf[MAXNAME];

			/*
			**  CHILD -- return to caller.
			**	Collect verified idea of sending host.
			**	Verify calling user id if possible here.
			*/

			(void) signal(SIGCHLD, SIG_DFL);

			/* determine host name */
			hp = gethostbyaddr((char *) &otherend.sin_addr, sizeof otherend.sin_addr, AF_INET);
			if (hp != NULL)
			{
				(void) strcpy(buf, hp->h_name);
				if (NetName != NULL && NetName[0] != '\0' &&
				    index(hp->h_name, '.') == NULL)
				{
					(void) strcat(buf, ".");
					(void) strcat(buf, NetName);
				}
			}
			else
			{
				extern char *inet_ntoa();

				/* produce a dotted quad */
				(void) sprintf(buf, "[%s]",
					inet_ntoa(otherend.sin_addr));
			}

			/* should we check for illegal connection here? XXX */

			RealHostName = newstr(buf);

			(void) close(DaemonSocket);
			InChannel = fdopen(t, "r");
			OutChannel = fdopen(dup(t), "w");
# ifdef DEBUG
			if (tTd(15, 2))
				printf("getreq: returning\n");
# endif DEBUG
# ifdef LOG
			if (LogLevel > 11)
				syslog(LOG_DEBUG, "connected, pid=%d", getpid());
# endif LOG
			return;
		}

		/* close the port so that others will hang (for a while) */
		(void) close(t);
	}
	/*NOTREACHED*/
}
/*
**  CLRDAEMON -- reset the daemon connection
**
**	Parameters:
**		none.
**
**	Returns:
**		none.
**
**	Side Effects:
**		releases any resources used by the passive daemon.
*/

clrdaemon()
{
	if (DaemonSocket >= 0)
		(void) close(DaemonSocket);
	DaemonSocket = -1;
}
/*
**  MAKECONNECTION -- make a connection to an SMTP socket on another machine.
**
**	Parameters:
**		host -- the name of the host.
**		port -- the port number to connect to.
**		outfile -- a pointer to a place to put the outfile
**			descriptor.
**		infile -- ditto for infile.
**
**	Returns:
**		An exit code telling whether the connection could be
**			made and if not why not.
**
**	Side Effects:
**		none.
*/

int	h_errno;	/*this will go away when code implemented*/

makeconnection(host, port, outfile, infile)
	char *host;
	u_short port;
	FILE **outfile;
	FILE **infile;
{
	register int s;
	int sav_errno;

	/*
	**  Set up the address for the mailer.
	**	Accept "[a.b.c.d]" syntax for host name.
	*/

	h_errno = 0;
	errno = 0;

	if (host[0] == '[')
	{
		long hid;
		register char *p = index(host, ']');

		if (p != NULL)
		{
			*p = '\0';
			hid = inet_addr(&host[1]);
			*p = ']';
		}
		if (p == NULL || hid == -1)
		{
			usrerr("Invalid numeric domain spec \"%s\"", host);
			return (EX_NOHOST);
		}
		SendmailAddress.sin_addr.s_addr = hid;
	}
	else
	{
		register struct hostent *hp = gethostbyname(host);

		if (hp == NULL)
		{
			if (errno == ETIMEDOUT || h_errno == TRY_AGAIN)
				return (EX_TEMPFAIL);

			/*
			**  XXX Should look for mail forwarder record here
			**  XXX if (h_errno == NO_ADDRESS).
			*/

			return (EX_NOHOST);
		}
		bcopy(hp->h_addr, (char *) &SendmailAddress.sin_addr, hp->h_length);
	}

	/*
	**  Determine the port number.
	*/

	if (port != 0)
		SendmailAddress.sin_port = htons(port);
	else
	{
		register struct servent *sp = getservbyname("smtp", "tcp");

		if (sp == NULL)
		{
			syserr("makeconnection: server \"smtp\" unknown");
			return (EX_OSFILE);
		}
		SendmailAddress.sin_port = sp->s_port;
	}

	/*
	**  Try to actually open the connection.
	*/

# ifdef DEBUG
	if (tTd(16, 1))
		printf("makeconnection (%s)\n", host);
# endif DEBUG

	s = socket(AF_INET, SOCK_STREAM, 0);
	if (s < 0)
	{
		syserr("makeconnection: no socket");
		sav_errno = errno;
		goto failure;
	}

# ifdef DEBUG
	if (tTd(16, 1))
		printf("makeconnection: %d\n", s);

	/* turn on network debugging? */
	if (tTd(16, 14))
	{
		int on = 1;
		(void) setsockopt(DaemonSocket, SOL_SOCKET, SO_DEBUG, (char *)&on, sizeof on);
	}
# endif DEBUG
	(void) fflush(CurEnv->e_xfp);			/* for debugging */
	errno = 0;					/* for debugging */
	SendmailAddress.sin_family = AF_INET;
	if (connect(s, &SendmailAddress, sizeof SendmailAddress) < 0)
	{
		sav_errno = errno;
		(void) close(s);
		/* failure, decide if temporary or not */
	failure:
		switch (sav_errno)
		{
		  case EISCONN:
		  case ETIMEDOUT:
		  case EINPROGRESS:
		  case EALREADY:
		  case EADDRINUSE:
		  case EHOSTDOWN:
		  case ENETDOWN:
		  case ENETRESET:
		  case ENOBUFS:
		  case ECONNREFUSED:
		  case ECONNRESET:
		  case EHOSTUNREACH:
		  case ENETUNREACH:
			/* there are others, I'm sure..... */
			return (EX_TEMPFAIL);

		  case EPERM:
			/* why is this happening? */
			syserr("makeconnection: funny failure, addr=%lx, port=%x",
				SendmailAddress.sin_addr.s_addr, SendmailAddress.sin_port);
			return (EX_TEMPFAIL);

		  default:
			return (EX_UNAVAILABLE);
		}
	}

	/* connection ok, put it into canonical form */
	*outfile = fdopen(s, "w");
#if defined(sgi) || defined(sgi_vax)
	*infile = fdopen(dup(s), "r");
#else
	*infile = fdopen(s, "r");
#endif

	return (EX_OK);
}
/*
**  MYHOSTNAME -- return the name of this host.
**
**	Parameters:
**		hostbuf -- a place to return the name of this host.
**		size -- the size of hostbuf.
**
**	Returns:
**		A list of aliases for this host.
**
**	Side Effects:
**		none.
*/

char **
myhostname(hostbuf, size)
	char hostbuf[];
	int size;
{
	extern struct hostent *gethostbyname();
	struct hostent *hp;

	if (gethostname(hostbuf, size) < 0)
	{
		(void) strcpy(hostbuf, "localhost");
	}
	hp = gethostbyname(hostbuf);
	if (hp != NULL)
	{
		(void) strcpy(hostbuf, hp->h_name);
		return (hp->h_aliases);
	}
	else
		return (NULL);
}
/*
**  MAPHOSTNAME -- turn a hostname into canonical form
**
**	Parameters:
**		hbuf -- a buffer containing a hostname.
**		hbsize -- the size of hbuf.
**
**	Returns:
**		none.
**
**	Side Effects:
**		Looks up the host specified in hbuf.  If it is not
**		the canonical name for that host, replace it with
**		the canonical name.  If the name is unknown, or it
**		is already the canonical name, leave it unchanged.
*/

maphostname(hbuf, hbsize)
	char *hbuf;
	int hbsize;
{
	register struct hostent *hp;
	extern struct hostent *gethostbyname();

	/*
	**  If first character is a bracket, then it is an address
	**  lookup.  Address is copied into a temporary buffer to
	**  strip the brackets and to preserve hbuf if address is
	**  unknown.
	*/

	if (*hbuf == '[')
	{
		extern struct hostent *gethostbyaddr();
		u_long in_addr;
		char ptr[256];
		char *bptr;

		(void) strcpy(ptr, hbuf);
		bptr = index(ptr,']');
		*bptr = '\0';
		in_addr = inet_addr(&ptr[1]);
		hp = gethostbyaddr((char *) &in_addr, sizeof(struct in_addr), AF_INET);
		if (hp == NULL)
			return;
	}
	else
	{
#if defined(sgi) || defined(sgi_vax)
		char tbuf[MAXLINE];

		if (hbsize > 3		/* consult UUCP map if asked */
		    && hbuf[0] == '!'
		    && hbsize <= sizeof(tbuf)) {
			static DBM *db = 0;
			static char fil[MAXLINE];
			register int flen;
			register char *p;
			datum key, res;

			p = index(hbuf+1,'!');
			if (!p)
				p = hbuf+1;
			else
				p++;
			flen = p - hbuf - 2;
			if (NULL != db	/* remember change of file */
			    && (flen != strlen(fil)
				|| 0 != strncmp(hbuf+1,fil,flen))) {
				dbm_close(db);
				db = NULL;
			}
			if (!db		/* initialize the 1st time */
			    && flen < sizeof(fil)) {
				strncpy(fil,hbuf+1,flen);
				fil[flen] = '\0';
				db = dbm_open(fil,O_RDONLY,0);
			}
			strcpy(tbuf,p);
			makelower(tbuf);
			res.dptr = NULL;
			if (NULL != db) {
				p = &tbuf[0];
				for (;;) {
					key.dptr = p;
					key.dsize = strlen(p) + 1;
					res = dbm_fetch(db,key);
					if (res.dptr != NULL
					    || key.dsize < 3)
						break;
					if (*p == '.') {
						p++;
						continue;
					}
					if (!(p = strchr(p,'.')))
						break;
				}
			}
			if (res.dptr != NULL
			    && 0 != strcmp("%s", &res.dptr[0])) {
				if (res.dsize >= 4
				    && !strcmp("!%s",&res.dptr[res.dsize-4])) {
					res.dptr[res.dsize-4]  = '\0';
						 res.dsize -=3;
				}
				if (res.dsize < hbsize)	{
					(void)strcpy(hbuf, res.dptr);
					return;
				}
			}

		} else if (hbsize < sizeof(tbuf)) {
			strcpy(tbuf,hbuf);
			makelower(tbuf);	/* preserve original case */
			hp = gethostbyname(tbuf);
		}
#else
		makelower(hbuf);
		hp = gethostbyname(hbuf);
#endif
	}
	if (hp != NULL)
	{
		int i = strlen(hp->h_name);

		if (i >= hbsize)
			hp->h_name[--i] = '\0';
		(void) strcpy(hbuf, hp->h_name);
	}
}

# else DAEMON
/* code for systems without sophisticated networking */

/*
**  MYHOSTNAME -- stub version for case of no daemon code.
**
**	Can't convert to upper case here because might be a UUCP name.
**
**	Mark, you can change this to be anything you want......
*/

char **
myhostname(hostbuf, size)
	char hostbuf[];
	int size;
{
	register FILE *f;

	hostbuf[0] = '\0';
	f = fopen("/usr/include/whoami", "r");
	if (f != NULL)
	{
		(void) fgets(hostbuf, size, f);
		fixcrlf(hostbuf, TRUE);
		(void) fclose(f);
	}
	return (NULL);
}
/*
**  MAPHOSTNAME -- turn a hostname into canonical form
**
**	Parameters:
**		hbuf -- a buffer containing a hostname.
**		hbsize -- the size of hbuf.
**
**	Returns:
**		none.
**
**	Side Effects:
**		Looks up the host specified in hbuf.  If it is not
**		the canonical name for that host, replace it with
**		the canonical name.  If the name is unknown, or it
**		is already the canonical name, leave it unchanged.
*/

/*ARGSUSED*/
maphostname(hbuf, hbsize)
	char *hbuf;
	int hbsize;
{
	return;
}

#endif DAEMON
