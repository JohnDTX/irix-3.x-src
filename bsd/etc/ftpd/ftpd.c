/*
 * Copyright (c) 1985 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifndef lint
char copyright[] =
"@(#) Copyright (c) 1985 Regents of the University of California.\n\
 All rights reserved.\n";
#endif not lint

#ifndef lint
static char sccsid[] = "@(#)ftpd.c	5.7 (Berkeley) 5/28/86";
#endif not lint

/*
 * FTP server.
 */
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <sys/wait.h>

#include <netinet/in.h>

#include <arpa/ftp.h>
#include <arpa/inet.h>
#include <arpa/telnet.h>

#include <stdio.h>
#include <signal.h>
#include <pwd.h>
#include <setjmp.h>
#include <netdb.h>
#include <errno.h>
#include <strings.h>
#include <syslog.h>

/*
 * File containing login names
 * NOT to be used on this machine.
 * Commonly used to disallow uucp.
 */
#define	FTPUSERS	"/etc/ftpusers"

extern	int errno;
extern	char *sys_errlist[];
extern	char *crypt();
extern	char version[];
extern	char *home;		/* pointer to home directory for glob */
extern	FILE *popen(), *fopen(), *freopen();
extern	int  pclose(), fclose();
extern	char *getline();
extern	char cbuf[];

struct	sockaddr_in ctrl_addr;
struct	sockaddr_in data_source;
struct	sockaddr_in data_dest;
struct	sockaddr_in his_addr;

int	data;
jmp_buf	errcatch, urgcatch;
int	logged_in;
struct	passwd *pw;
int	debug;
int	timeout = 900;    /* timeout after 15 minutes of inactivity */
int	logging;
int	guest;
#ifndef sgi
int	wtmp;
#endif
int	type;
int	form;
int	stru;			/* avoid C keyword */
int	mode;
int	usedefault = 1;		/* for data transfers */
int	pdata;			/* for passive mode */
int	unique;
int	transflag;
char	tmpline[7];
char	hostname[32];
char	remotehost[32];

/*
 * Timeout intervals for retrying connections
 * to hosts that don't accept PORT cmds.  This
 * is a kludge, but given the problems with TCP...
 */
#define	SWAITMAX	90	/* wait at most 90 seconds */
#define	SWAITINT	5	/* interval between retries */

int	swaitmax = SWAITMAX;
int	swaitint = SWAITINT;

int	lostconn();
int	myoob();
FILE	*getdatasock(), *dataconn();

main(argc, argv)
	int argc;
	char *argv[];
{
	int addrlen, on = 1;
	long pgid;
	char *cp;

	addrlen = sizeof (his_addr);
	if (getpeername(0, &his_addr, &addrlen) < 0) {
		syslog(LOG_ERR, "getpeername (%s): %m",argv[0]);
		exit(1);
	}
	addrlen = sizeof (ctrl_addr);
	if (getsockname(0, (char *) &ctrl_addr, &addrlen) < 0) {
		syslog(LOG_ERR, "getsockname (%s): %m",argv[0]);
		exit(1);
	}
	data_source.sin_port = htons(ntohs(ctrl_addr.sin_port) - 1);
	debug = 0;
	openlog("ftpd", LOG_PID, LOG_DAEMON);
	argc--, argv++;
	while (argc > 0 && *argv[0] == '-') {
		for (cp = &argv[0][1]; *cp; cp++) switch (*cp) {

		case 'v':
			debug = 1;
			break;

		case 'd':
			debug = 1;
			break;

		case 'l':
			logging = 1;
			break;

		case 't':
			timeout = atoi(++cp);
			goto nextopt;
			break;

		default:
			fprintf(stderr, "ftpd: Unknown flag -%c ignored.\n",
			     *cp);
			break;
		}
nextopt:
		argc--, argv++;
	}
	(void) signal(SIGPIPE, lostconn);
#ifdef sgi
	(void) signal(SIGCHLD, SIG_DFL);
#else
	(void) signal(SIGCHLD, SIG_IGN);
#endif
	if (signal(SIGURG, myoob) < 0) {
		syslog(LOG_ERR, "signal: %m");
	}
	/* handle urgent data inline */
#ifdef SO_OOBINLINE
	if (setsockopt(0, SOL_SOCKET, SO_OOBINLINE, (char *)&on, sizeof(on)) < 0) {
		syslog(LOG_ERR, "setsockopt: %m");
	}
#endif SO_OOBINLINE
#ifdef sgi
	pgid = getpgrp();
#else
	pgid = getpid();
#endif
	if (ioctl(fileno(stdin), SIOCSPGRP, (char *) &pgid) < 0) {
		syslog(LOG_ERR, "ioctl: %m");
	}
	dolog(&his_addr);
	/* do telnet option negotiation here */
	/*
	 * Set up default state
	 */
	logged_in = 0;
	data = -1;
	type = TYPE_A;
	form = FORM_N;
	stru = STRU_F;
	mode = MODE_S;
	tmpline[0] = '\0';
	(void) gethostname(hostname, sizeof (hostname));
	reply(220, "%s FTP server (%s) ready.",
		hostname, version);
	for (;;) {
		(void) setjmp(errcatch);
		(void) yyparse();
	}
}



#ifdef sgi
char *need_chroot = NULL;
int need_uid = 0;
int need_gid = 0;

/* fork to do a command, since here in AT&T System V land we cannot
 *	get back to root after doing a setuid()
 *
 * This also returns errno=0 or an error #
 */
static int				/* return result of fork() */
do_fork()
{
	register int pid;

	fflush(stdout);			/* prevent standard I/O confusion */
	fflush(stderr);			/* with the parent */

	pid = fork();
	if (!pid) {
		if (need_chroot
		    && chroot(need_chroot) < 0)
			_exit(errno);

		if (setgid(need_gid) >= 0
		    && setuid(need_uid) >= 0) {
			errno = 0;
		} else {
			_exit(errno);
		}
	}
	return pid;
}

/* clean up after do_fork() and leave something good in errno
 */
static int
fin_fork(pid)
register int pid;
{
	int result;
	register int wres;
	
 	if (pid < 0)
		return -1;

	if (pid != wait(&result))
		return -1;

	errno = 0;
	if (!result)
		return 0;

	if (!(result & 0xff))
		errno = (result>>8);
	else
		errno = result & 0xff;
	return -1;
}
#endif


lostconn()
{

	if (debug)
		syslog(LOG_DEBUG, "lost connection");
	dologout(-1);
}

pass(passwd)
	char *passwd;
{
	char *xpasswd, *savestr();
	static struct passwd save;
#ifdef sgi
	register int pid;
#endif

	if (logged_in || pw == NULL) {
		reply(503, "Login with USER first.");
		return;
	}
	if (!guest) {		/* "ftp" is only account allowed no password */
		xpasswd = crypt(passwd, pw->pw_passwd);
		/* The strcmp does not catch null passwords! */
		if (*pw->pw_passwd == '\0' || strcmp(xpasswd, pw->pw_passwd)) {
			reply(530, "Login incorrect.");
			pw = NULL;
			return;
		}
	}
#ifdef sgi
	if (chdir(pw->pw_dir)) {
		reply(550, "User %s: can't change directory to %s.",
			pw->pw_name, pw->pw_dir);
		goto bad;
	}
	need_gid = pw->pw_gid;
	need_uid = pw->pw_uid;
	need_chroot = (guest ? pw->pw_dir : NULL);
	pid = do_fork();
	if (!pid)
		_exit(0);
	if (fin_fork(pid) < 0) {
		reply(530, "Login incorrect.");
		goto bad;
	}
#else
	setegid(pw->pw_gid);
	initgroups(pw->pw_name, pw->pw_gid);
	if (chdir(pw->pw_dir)) {
		reply(530, "User %s: can't change directory to %s.",
			pw->pw_name, pw->pw_dir);
		goto bad;
	}

	/* grab wtmp before chroot */
	wtmp = open("/usr/adm/wtmp", O_WRONLY|O_APPEND);
	if (guest && chroot(pw->pw_dir) < 0) {
		reply(550, "Can't set guest privileges.");
		if (wtmp >= 0) {
			(void) close(wtmp);
			wtmp = -1;
		}
		goto bad;
	}
#endif
	if (!guest)
		reply(230, "User %s logged in.", pw->pw_name);
	else
		reply(230, "Guest login ok, access restrictions apply.");
	logged_in = 1;
	dologin(pw);
#ifndef sgi
	seteuid(pw->pw_uid);
#endif
	/*
	 * Save everything so globbing doesn't
	 * clobber the fields.
	 */
	save = *pw;
	save.pw_name = savestr(pw->pw_name);
	save.pw_passwd = savestr(pw->pw_passwd);
	save.pw_comment = savestr(pw->pw_comment);
	save.pw_gecos = savestr(pw->pw_gecos);
	save.pw_dir = savestr(pw->pw_dir);
	save.pw_shell = savestr(pw->pw_shell);
	pw = &save;
	home = pw->pw_dir;		/* home dir for globbing */
	return;
bad:
#ifdef sgi
	need_uid = -1;
	need_gid = -1;
	need_chroot = "";
#else
	seteuid(0);
#endif
	pw = NULL;
}

char *
savestr(s)
	char *s;
{
	char *malloc();
	char *new = malloc((unsigned) strlen(s) + 1);
	
	if (new != NULL)
		(void) strcpy(new, s);
	return (new);
}

retrieve(cmd, name)
	char *cmd, *name;
{
	FILE *fin, *dout;
	struct stat st;
	int (*closefunc)(), tmp;

	if (cmd == 0) {
#ifdef notdef
		/* no remote command execution -- it's a security hole */
		if (*name == '|')
			fin = popen(name + 1, "r"), closefunc = pclose;
		else
#endif
#ifdef sgi
		register int pid = do_fork();	/* open it as the real user */
		if (!pid) {
			(void)access(name, 04);
			_exit(errno);
		}
		if (fin_fork(pid) < 0)
			fin = NULL;
		else
#endif
			fin = fopen(name, "r"), closefunc = fclose;
	} else {
		char line[BUFSIZ];

		(void) sprintf(line, cmd, name), name = line;
		fin = popen(line, "r"), closefunc = pclose;
	}
	if (fin == NULL) {
		if (errno != 0)
			reply(550, "%s: %s.", name, sys_errlist[errno]);
		return;
	}
	st.st_size = 0;
	if (cmd == 0 &&
	    (stat(name, &st) < 0 || (st.st_mode&S_IFMT) != S_IFREG)) {
		reply(550, "%s: not a plain file.", name);
		goto done;
	}
	dout = dataconn(name, st.st_size, "w");
	if (dout == NULL)
		goto done;
	if ((tmp = send_data(fin, dout)) > 0 || ferror(dout) > 0) {
		reply(550, "%s: %s.", name, sys_errlist[errno]);
	}
	else if (tmp == 0) {
		reply(226, "Transfer complete.");
	}
	(void) fclose(dout);
	data = -1;
	pdata = -1;
done:
	(*closefunc)(fin);
}

store(name, mode)
	char *name, *mode;
{
	FILE *fout, *din;
	int (*closefunc)(), dochown = 0, tmp;
	char *gunique(), *local;
#ifdef notdef
	/* no remote command execution -- it's a security hole */
	if (name[0] == '|')
		fout = popen(&name[1], "w"), closefunc = pclose;
	else
#endif
	{
		struct stat st;
#ifdef sgi
		register int pid;
#endif

		local = name;
		if (stat(name, &st) < 0) {
			dochown++;
		}
		else if (unique) {
			if ((local = gunique(name)) == NULL) {
				return;
			}
			dochown++;
		}
#ifdef sgi
		pid = do_fork();	/* create it as real user */
		if (!pid) {
			(void)fopen(local, mode);
			_exit(errno);
		}
		if (fin_fork(pid) < 0)
			fout = NULL;
		else
#endif
		fout = fopen(local, mode), closefunc = fclose;
	}
	if (fout == NULL) {
		reply(553, "%s: %s.", local, sys_errlist[errno]);
		return;
	}
	din = dataconn(local, (off_t)-1, "r");
	if (din == NULL)
		goto done;
	if ((tmp = receive_data(din, fout)) > 0 || ferror(fout) > 0) {
		reply(552, "%s: %s.", local, sys_errlist[errno]);
	}
	else if (tmp == 0 && !unique) {
		reply(226, "Transfer complete.");
	}
	else if (tmp == 0 && unique) {
		reply(226, "Transfer complete (unique file name:%s).", local);
	}
	(void) fclose(din);
	data = -1;
	pdata = -1;
done:
#ifndef sgi
	if (dochown)
		(void) chown(local, pw->pw_uid, -1);
#endif
	(*closefunc)(fout);
}

FILE *
getdatasock(mode)
	char *mode;
{
	int s, on = 1;

	if (data >= 0)
		return (fdopen(data, mode));
	s = socket(AF_INET, SOCK_STREAM, 0);
	if (s < 0)
		return (NULL);
#ifndef sgi
	seteuid(0);
#endif
	if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *) &on, sizeof (on)) < 0)
		goto bad;
	/* anchor socket to avoid multi-homing problems */
	data_source.sin_family = AF_INET;
	data_source.sin_addr = ctrl_addr.sin_addr;
	if (bind(s, &data_source, sizeof (data_source)) < 0)
		goto bad;
#ifndef sgi
	seteuid(pw->pw_uid);
#endif
	return (fdopen(s, mode));
bad:
#ifndef sgi
	seteuid(pw->pw_uid);
#endif
	(void) close(s);
	return (NULL);
}

FILE *
dataconn(name, size, mode)
	char *name;
	off_t size;
	char *mode;
{
	char sizebuf[32];
	FILE *file;
	int retry = 0;

	if (size >= 0)
		(void) sprintf (sizebuf, " (%ld bytes)", size);
	else
		(void) strcpy(sizebuf, "");
	if (pdata > 0) {
		struct sockaddr_in from;
		int s, fromlen = sizeof(from);

		s = accept(pdata, &from, &fromlen);
		if (s < 0) {
			reply(425, "Can't open data connection.");
			(void) close(pdata);
			pdata = -1;
			return(NULL);
		}
		(void) close(pdata);
		pdata = s;
#ifdef sgi
		reply(150, "Opening data connection for %s (%s,%d)%s.",
#else
		reply(150, "Openning data connection for %s (%s,%d)%s.",
#endif
		     name, inet_ntoa(from.sin_addr),
		     ntohs(from.sin_port), sizebuf);
		return(fdopen(pdata, mode));
	}
	if (data >= 0) {
		reply(125, "Using existing data connection for %s%s.",
		    name, sizebuf);
		usedefault = 1;
		return (fdopen(data, mode));
	}
	if (usedefault)
		data_dest = his_addr;
	usedefault = 1;
	file = getdatasock(mode);
	if (file == NULL) {
		reply(425, "Can't create data socket (%s,%d): %s.",
		    inet_ntoa(data_source.sin_addr),
		    ntohs(data_source.sin_port),
		    sys_errlist[errno]);
		return (NULL);
	}
	data = fileno(file);
	while (connect(data, &data_dest, sizeof (data_dest)) < 0) {
		if (errno == EADDRINUSE && retry < swaitmax) {
			sleep((unsigned) swaitint);
			retry += swaitint;
			continue;
		}
		reply(425, "Can't build data connection: %s.",
		    sys_errlist[errno]);
		(void) fclose(file);
		data = -1;
		return (NULL);
	}
	reply(150, "Opening data connection for %s (%s,%d)%s.",
	    name, inet_ntoa(data_dest.sin_addr),
	    ntohs(data_dest.sin_port), sizebuf);
	return (file);
}

/*
 * Tranfer the contents of "instr" to
 * "outstr" peer using the appropriate
 * encapulation of the date subject
 * to Mode, Structure, and Type.
 *
 * NB: Form isn't handled.
 */
send_data(instr, outstr)
	FILE *instr, *outstr;
{
	register int c;
	int netfd, filefd, cnt;
	char buf[BUFSIZ];

	transflag++;
	if (setjmp(urgcatch)) {
		transflag = 0;
		return(-1);
	}
	switch (type) {

	case TYPE_A:
		while ((c = getc(instr)) != EOF) {
			if (c == '\n') {
				if (ferror (outstr)) {
					transflag = 0;
					return (1);
				}
				(void) putc('\r', outstr);
			}
			(void) putc(c, outstr);
		/*	if (c == '\r')			*/
		/*		putc ('\0', outstr);	*/
		}
		transflag = 0;
		if (ferror (instr) || ferror (outstr)) {
			return (1);
		}
		return (0);
		
	case TYPE_I:
	case TYPE_L:
		netfd = fileno(outstr);
		filefd = fileno(instr);

		while ((cnt = read(filefd, buf, sizeof (buf))) > 0) {
			if (write(netfd, buf, cnt) < 0) {
				transflag = 0;
				return (1);
			}
		}
		transflag = 0;
		return (cnt < 0);
	}
	reply(550, "Unimplemented TYPE %d in send_data", type);
	transflag = 0;
	return (-1);
}

/*
 * Transfer data from peer to
 * "outstr" using the appropriate
 * encapulation of the data subject
 * to Mode, Structure, and Type.
 *
 * N.B.: Form isn't handled.
 */
receive_data(instr, outstr)
	FILE *instr, *outstr;
{
	register int c;
	int cnt;
	char buf[BUFSIZ];


	transflag++;
	if (setjmp(urgcatch)) {
		transflag = 0;
		return(-1);
	}
	switch (type) {

	case TYPE_I:
	case TYPE_L:
		while ((cnt = read(fileno(instr), buf, sizeof buf)) > 0) {
			if (write(fileno(outstr), buf, cnt) < 0) {
				transflag = 0;
				return (1);
			}
		}
		transflag = 0;
		return (cnt < 0);

	case TYPE_E:
		reply(553, "TYPE E not implemented.");
		transflag = 0;
		return (-1);

	case TYPE_A:
		while ((c = getc(instr)) != EOF) {
			while (c == '\r') {
				if (ferror (outstr)) {
					transflag = 0;
					return (1);
				}
				if ((c = getc(instr)) != '\n')
					(void) putc ('\r', outstr);
			/*	if (c == '\0')			*/
			/*		continue;		*/
			}
			(void) putc (c, outstr);
		}
		transflag = 0;
		if (ferror (instr) || ferror (outstr))
			return (1);
		return (0);
	}
	transflag = 0;
	fatal("Unknown type in receive_data.");
	/*NOTREACHED*/
}

fatal(s)
	char *s;
{
	reply(451, "Error in server: %s\n", s);
	reply(221, "Closing connection due to server error.");
	dologout(0);
}

/*VARARGS2*/
#if defined(sgi) && defined(SVR3)
reply(n, s, args,p1,p2,p3,p4)
#else
reply(n, s, args)
#endif
	int n;
	char *s;
{

	printf("%d ", n);
#if defined(sgi) && defined(SVR3)
	printf(s, args,p1,p2,p3,p4);
#else
	_doprnt(s, &args, stdout);
#endif
	printf("\r\n");
	(void) fflush(stdout);
	if (debug) {
		syslog(LOG_DEBUG, "<--- %d ", n);
#if defined(sgi) && defined(SVR3)
		syslog(LOG_DEBUG, s, args,p1,p2,p3,p4);
#else
		syslog(LOG_DEBUG, s, &args);
#endif
	}
}

/*VARARGS2*/
#if defined(sgi) && defined(SVR3)
lreply(n, s, args,p1,p2,p3,p4)
#else
lreply(n, s, args)
#endif
	int n;
	char *s;
{
	printf("%d-", n);
#if defined(sgi) && defined(SVR3)
	printf(s, args,p1,p2,p3,p4);
#else
	_doprnt(s, &args, stdout);
#endif
	printf("\r\n");
	(void) fflush(stdout);
	if (debug) {
		syslog(LOG_DEBUG, "<--- %d- ", n);
#if defined(sgi) && defined(SVR3)
		syslog(LOG_DEBUG, s, args,p1,p2,p3,p4);
#else
		syslog(LOG_DEBUG, s, &args);
#endif
	}
}

ack(s)
	char *s;
{
	reply(250, "%s command successful.", s);
}

nack(s)
	char *s;
{
	reply(502, "%s command not implemented.", s);
}

yyerror(s)
	char *s;
{
	char *cp;

	cp = index(cbuf,'\n');
	*cp = '\0';
	reply(500, "'%s': command not understood.",cbuf);
}

delete(name)
	char *name;
{
	struct stat st;

#ifdef sgi
	register int pid;
	pid = do_fork();
	if (!pid) {
		if (stat(name, &st) < 0)
			_exit(errno);
		if ((st.st_mode&S_IFMT) == S_IFDIR)
			(void)rmdir(name);
		else
			(void)unlink(name);
		_exit(errno);
	}
	if (fin_fork(pid) < 0) {
		reply(550, "%s: %s.", name, sys_errlist[errno]);
		return;
	}
#else
	if (stat(name, &st) < 0) {
		reply(550, "%s: %s.", name, sys_errlist[errno]);
		return;
	}
	if ((st.st_mode&S_IFMT) == S_IFDIR) {
		if (rmdir(name) < 0) {
			reply(550, "%s: %s.", name, sys_errlist[errno]);
			return;
		}
		goto done;
	}
	if (unlink(name) < 0) {
		reply(550, "%s: %s.", name, sys_errlist[errno]);
		return;
	}
done:
#endif
	ack("DELE");
}

cwd(path)
	char *path;
{

#ifdef sgi
	register int pid = do_fork();	/* try 1st it as the real user */
	if (!pid) {			/* since root can do anything */
		(void)chdir(path);
		_exit(errno);
	}
	if (fin_fork(pid) < 0) {
		reply(550, "%s: %s.", path, sys_errlist[errno]);
		return;
	}
#endif
	if (chdir(path) < 0) {
		reply(550, "%s: %s.", path, sys_errlist[errno]);
		return;
	}
	ack("CWD");
}

makedir(name)
	char *name;
{
	struct stat st;
#ifdef sgi
	register int pid = do_fork();
	if (!pid) {
		(void)mkdir(name, 0777);
		_exit(errno);
	}
	if (fin_fork(pid) < 0) {
		reply(550, "%s: %s.", name, sys_errlist[errno]);
		return;
	}
#else
	int dochown = stat(name, &st) < 0;
	
	if (mkdir(name, 0777) < 0) {
		reply(550, "%s: %s.", name, sys_errlist[errno]);
		return;
	}
	if (dochown)
		(void) chown(name, pw->pw_uid, -1);
#endif
	reply(257, "MKD command successful.");
}

removedir(name)
	char *name;
{

#ifdef sgi
	register int pid;
	pid = do_fork();
	if (!pid) {
		(void)rmdir(name);
		_exit(errno);
	}
	if (fin_fork(pid) < 0) {
		reply(550, "%s: %s.", name, sys_errlist[errno]);
		return;
	}
#else
	if (rmdir(name) < 0) {
		reply(550, "%s: %s.", name, sys_errlist[errno]);
		return;
	}
#endif
	ack("RMD");
}

pwd()
{
	char path[MAXPATHLEN + 1];

	if (getwd(path) == NULL) {
		reply(550, "%s.", path);
		return;
	}
	reply(257, "\"%s\" is current directory.", path);
}

char *
renamefrom(name)
	char *name;
{
	struct stat st;

#ifdef sgi
	register int pid;
	pid = do_fork();
	if (!pid) {
		(void)stat(name, &st);
		_exit(errno);
	}
	if (fin_fork(pid) < 0) {
		reply(550, "%s: %s.", name, sys_errlist[errno]);
		return ((char *)0);
	}
#else
	if (stat(name, &st) < 0) {
		reply(550, "%s: %s.", name, sys_errlist[errno]);
		return ((char *)0);
	}
#endif
	reply(350, "File exists, ready for destination name");
	return (name);
}

renamecmd(from, to)
	char *from, *to;
{

#ifdef sgi
	register int pid;
	pid = do_fork();
	if (!pid) {
		(void)rename(from, to);
		_exit(errno);
	}
	if (fin_fork(pid) < 0) {
		reply(550, "rename: %s.", sys_errlist[errno]);
		return;
	}
#else
	if (rename(from, to) < 0) {
		reply(550, "rename: %s.", sys_errlist[errno]);
		return;
	}
#endif
	ack("RNTO");
}

dolog(sin)
	struct sockaddr_in *sin;
{
	struct hostent *hp = gethostbyaddr(&sin->sin_addr,
		sizeof (struct in_addr), AF_INET);
	time_t t;
	extern char *ctime();

	if (hp) {
		(void) strncpy(remotehost, hp->h_name, sizeof (remotehost));
		endhostent();
	} else
		(void) strncpy(remotehost, inet_ntoa(sin->sin_addr),
		    sizeof (remotehost));
	if (!logging)
		return;
	t = time((time_t *) 0);
	syslog(LOG_INFO,"FTPD: connection from %s at %s", remotehost, ctime(&t));
}

#include <utmp.h>

#define	SCPYN(a, b)	(void) strncpy(a, b, sizeof (a))
struct	utmp utmp;

/*
 * Record login in wtmp file.
 */
#ifdef sgi
#ifdef mips
extern struct utmp *getutid();
#endif
static char* id = "ftpA";
static int line_num = 0;
static char line_nc[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
#define MAX_LINE_NUM (sizeof(line_nc)-1)
#define LINE_ID id[3]

dologin(pw)
	struct passwd *pw;
{
	register FILE *fp;
	register struct utmp *ufound;

	while (line_num <= MAX_LINE_NUM) {	/* try until we get a slot */
		LINE_ID = line_nc[line_num];
		SCPYN(utmp.ut_line, id);
		SCPYN(utmp.ut_id, id);
		SCPYN(utmp.ut_name, pw->pw_name);
		utmp.ut_pid = getpid();
		utmp.ut_type = USER_PROCESS;
		utmp.ut_time = time(0);
		setutent();
		ufound = getutid(&utmp);
		if (!ufound || USER_PROCESS != ufound->ut_type) {
			(void)pututline(&utmp);
			continue;	/* check that it is not stolen */
		}
		if (ufound != NULL && ufound->ut_pid == getpid()) {
			endutent();
			if ((fp = fopen("/etc/wtmp","r+")) != NULL) {
				fseek(fp,0L,2);
				fwrite((char*)&utmp,sizeof(utmp),1,fp);
				fclose(fp);
			}
			break;
		}
		line_num++;
	}
	endutent();
}
#else
dologin(pw)
	struct passwd *pw;
{
	char line[32];

	if (wtmp >= 0) {
		/* hack, but must be unique and no tty line */
		(void) sprintf(line, "ftp%d", getpid());
		SCPYN(utmp.ut_line, line);
		SCPYN(utmp.ut_name, pw->pw_name);
		SCPYN(utmp.ut_host, remotehost);
		utmp.ut_time = (long) time((time_t *) 0);
		(void) write(wtmp, (char *)&utmp, sizeof (utmp));
		if (!guest) {		/* anon must hang on */
			(void) close(wtmp);
			wtmp = -1;
		}
	}
}
#endif

/*
 * Record logout in wtmp file
 * and exit with supplied status.
 */
dologout(status)
	int status;
{

	if (logged_in) {
#ifdef sgi
		register FILE *fp;

		if (line_num < MAX_LINE_NUM) {
			SCPYN(utmp.ut_line, id);
			SCPYN(utmp.ut_id, id);
			SCPYN(utmp.ut_name, pw->pw_name);
			utmp.ut_pid = getpid();
			utmp.ut_type = DEAD_PROCESS;
			utmp.ut_time = time(0);
			setutent();
			(void)pututline(&utmp);
			endutent();
			if ((fp = fopen("/etc/wtmp","r+")) != NULL) {
				utmp.ut_user[0] = '\0';
				fseek(fp,0L,2);
				fwrite((char*)&utmp,sizeof(utmp),1,fp);
				fclose(fp);
			}
		}
#else
		(void) seteuid(0);
		if (wtmp < 0)
			wtmp = open("/usr/adm/wtmp", O_WRONLY|O_APPEND);
		if (wtmp >= 0) {
			SCPYN(utmp.ut_name, "");
			SCPYN(utmp.ut_host, "");
			utmp.ut_time = (long) time((time_t *) 0);
			(void) write(wtmp, (char *)&utmp, sizeof (utmp));
			(void) close(wtmp);
		}
#endif
	}
	/* beware of flushing buffers after a SIGPIPE */
	_exit(status);
}

/*
 * Special version of popen which avoids
 * call to shell.  This insures noone may 
 * create a pipe to a hidden program as a side
 * effect of a list or dir command.
 */
#define	tst(a,b)	(*mode == 'r'? (b) : (a))
#define	RDR	0
#define	WTR	1
#ifdef sgi
static	int popen_pid[NOFILE];
#else
static	int popen_pid[5];
#endif

static char *
nextarg(cpp)
	char *cpp;
{
	register char *cp = cpp;

	if (cp == 0)
		return (cp);
	while (*cp && *cp != ' ' && *cp != '\t')
		cp++;
	if (*cp == ' ' || *cp == '\t') {
		*cp++ = '\0';
		while (*cp == ' ' || *cp == '\t')
			cp++;
	}
	if (cp == cpp)
		return ((char *)0);
	return (cp);
}

FILE *
popen(cmd, mode)
	char *cmd, *mode;
{
	int p[2], ac, gac;
	register myside, hisside, pid;
	char *av[20], *gav[512];
	register char *cp;

	if (pipe(p) < 0)
		return (NULL);
	cp = cmd, ac = 0;
	/* break up string into pieces */
	do {
		av[ac++] = cp;
		cp = nextarg(cp);
	} while (cp && *cp && ac < 20);
	av[ac] = (char *)0;
	gav[0] = av[0];
	/* glob each piece */
	for (gac = ac = 1; av[ac] != NULL; ac++) {
		char **pop;
		extern char **glob(), **copyblk();

		pop = glob(av[ac]);
		if (pop == (char **)NULL) {	/* globbing failed */
			char *vv[2];

			vv[0] = av[ac];
			vv[1] = 0;
			pop = copyblk(vv);
		}
		av[ac] = (char *)pop;		/* save to free later */
		while (*pop && gac < 512)
			gav[gac++] = *pop++;
	}
	gav[gac] = (char *)0;
	myside = tst(p[WTR], p[RDR]);
	hisside = tst(p[RDR], p[WTR]);
	if ((pid = fork()) == 0) {
		/* myside and hisside reverse roles in child */
		(void) close(myside);
		(void) dup2(hisside, tst(0, 1));
		(void) close(hisside);
#ifdef sgi
		if (need_chroot		/* chroot, setuid, setgid */
		    && chroot(need_chroot) < 0)
			_exit(1);
		if (setgid(need_gid) < 0
		    || setuid(need_uid) < 0)
			_exit(1);
#endif
		execv(gav[0], gav);
		_exit(1);
	}
	for (ac = 1; av[ac] != NULL; ac++)
		blkfree((char **)av[ac]);
	if (pid == -1)
		return (NULL);
	popen_pid[myside] = pid;
	(void) close(hisside);
	return (fdopen(myside, mode));
}

pclose(ptr)
	FILE *ptr;
{
#if defined(sgi) && defined(mips)
	register int f, r;
	register void (*hstat)(), (*istat)(), (*qstat)();
#else
	register f, r, (*hstat)(), (*istat)(), (*qstat)();
#endif
	int status;

	f = fileno(ptr);
	(void) fclose(ptr);
	istat = signal(SIGINT, SIG_IGN);
	qstat = signal(SIGQUIT, SIG_IGN);
	hstat = signal(SIGHUP, SIG_IGN);
	while ((r = wait(&status)) != popen_pid[f] && r != -1)
		;
	if (r == -1)
		status = -1;
	(void) signal(SIGINT, istat);
	(void) signal(SIGQUIT, qstat);
	(void) signal(SIGHUP, hstat);
	return (status);
}

/*
 * Check user requesting login priviledges.
 * Disallow anyone who does not have a standard
 * shell returned by getusershell() (/etc/shells).
 * Disallow anyone mentioned in the file FTPUSERS
 * to allow people such as uucp to be avoided.
 */
checkuser(name)
	register char *name;
{
	register char *cp;
#ifdef sgi
	char line[BUFSIZ], *index();
#else
	char line[BUFSIZ], *index(), *getusershell();
#endif
	FILE *fd;
#ifndef sgi
	struct passwd *pw;
#endif
	int found = 0;

#ifndef sgi
	pw = getpwnam(name);
	if (pw == NULL)
		return (0);
	while ((cp = getusershell()) != NULL)
		if (strcmp(cp, pw->pw_shell) == 0)
			break;
	endpwent();
	endusershell();
	if (cp == NULL)
		return (0);
#endif
	fd = fopen(FTPUSERS, "r");
	if (fd == NULL)
		return (1);
	while (fgets(line, sizeof (line), fd) != NULL) {
		cp = index(line, '\n');
		if (cp)
			*cp = '\0';
		if (strcmp(line, name) == 0) {
			found++;
			break;
		}
	}
	(void) fclose(fd);
	return (!found);
}

myoob()
{
	char *cp;

#ifdef sgi
	(void)signal(SIGURG, myoob);
#endif
	/* only process if transfer occurring */
	if (!transflag) {
		return;
	}
	cp = tmpline;
	if (getline(cp, 7, stdin) == NULL) {
		reply(221, "You could at least say goodby.");
		dologout(0);
	}
	upper(cp);
	if (strcmp(cp, "ABOR\r\n"))
		return;
	tmpline[0] = '\0';
	reply(426,"Transfer aborted. Data connection closed.");
	reply(226,"Abort successful");
	longjmp(urgcatch, 1);
}

/*
 * Note: The 530 reply codes could be 4xx codes, except nothing is
 * given in the state tables except 421 which implies an exit.  (RFC959)
 */
passive()
{
	int len;
	struct sockaddr_in tmp;
	register char *p, *a;

	pdata = socket(AF_INET, SOCK_STREAM, 0);
	if (pdata < 0) {
		reply(530, "Can't open passive connection");
		return;
	}
	tmp = ctrl_addr;
	tmp.sin_port = 0;
#ifndef sgi
	seteuid(0);
#endif
	if (bind(pdata, (struct sockaddr *) &tmp, sizeof(tmp)) < 0) {
#ifndef sgi
		seteuid(pw->pw_uid);
#endif
		(void) close(pdata);
		pdata = -1;
		reply(530, "Can't open passive connection");
		return;
	}
#ifndef sgi
	seteuid(pw->pw_uid);
#endif
	len = sizeof(tmp);
	if (getsockname(pdata, (char *) &tmp, &len) < 0) {
		(void) close(pdata);
		pdata = -1;
		reply(530, "Can't open passive connection");
		return;
	}
	if (listen(pdata, 1) < 0) {
		(void) close(pdata);
		pdata = -1;
		reply(530, "Can't open passive connection");
		return;
	}
	a = (char *) &tmp.sin_addr;
	p = (char *) &tmp.sin_port;

#define UC(b) (((int) b) & 0xff)

	reply(227, "Entering Passive Mode (%d,%d,%d,%d,%d,%d)", UC(a[0]),
		UC(a[1]), UC(a[2]), UC(a[3]), UC(p[0]), UC(p[1]));
}

char *
gunique(local)
	char *local;
{
	static char new[MAXPATHLEN];
	char *cp = rindex(local, '/');
	int d, count=0;
	char ext = '1';

	if (cp) {
		*cp = '\0';
	}
	d = access(cp ? local : ".", 2);
	if (cp) {
		*cp = '/';
	}
	if (d < 0) {
		syslog(LOG_ERR, "%s: %m", local);
		return((char *) 0);
	}
	(void) strcpy(new, local);
	cp = new + strlen(new);
	*cp++ = '.';
	while (!d) {
		if (++count == 100) {
			reply(452, "Unique file name not cannot be created.");
			return((char *) 0);
		}
		*cp++ = ext;
		*cp = '\0';
		if (ext == '9') {
			ext = '0';
		}
		else {
			ext++;
		}
		if ((d = access(new, 0)) < 0) {
			break;
		}
		if (ext != '0') {
			cp--;
		}
		else if (*(cp - 2) == '.') {
			*(cp - 1) = '1';
		}
		else {
			*(cp - 2) = *(cp - 2) + 1;
			cp--;
		}
	}
	return(new);
}
