# undef SNEAKYX
/*
 * xcp  (rcp hacked for use with xns protocols)
 */
/* if S_IFLNK defined, compile with clstat.o */

#include <sys/types.h>
#include <sys/stat.h>
#	define filetype(s)	((s)->st_mode&S_IFMT)
#	define protection(s)	((s)->st_mode&07777)
#include <sys/ioctl.h>
#include <xns/Xns.h>
#include <xns/Xnsioctl.h>
#	define MAXPKTSIZE	1024
#	define XCPMAXNAME	1024
#	define XCPMAXUSER	20
#	define XCPMAXHOST	20
#include <signal.h>
#include <errno.h>
	extern int errno;

#include <stdio.h>
#include <ctype.h>


#ifdef	SYSTEM5
# define bcopy(s,t,n)	blt(t,s,n)
#endif

# ifdef S_IFLNK
extern int clstat();
# define c_stat(f,p)	(!Lflag?clstat(f,p):stat(f,p))
# else S_IFLNK
# define clstat(f,p)	stat(f,p)
# define c_stat(f,p)	stat(f,p)
# endif S_IFLNK

# define DEBUG xcpdebug
# ifndef _DEBUG_
/* macros for debugging */
# define _DEBUG_

# ifdef DEBUG
#   define dprintf(x)	(DEBUG?printf x:0)
#   define dinterp(x)	(DEBUG?x:0)
#   define IFDEBUG(x)	x
#   define ASSERT(c) if(!(c))_assert("c",__FILE__,__LINE__)
extern short DEBUG;
# else  DEBUG
#   define dprintf(x)
#   define dinterp(x)
#   define IFDEBUG(x)
#   define ASSERT(c)
# endif DEBUG

# endif _DEBUG_
short xcpdebug = 0;


# define vprintf(x)	(vflag?printf x:0)

extern char *basename(), *strlcpy(), *skipnum(), *agetpw();
extern int lostconn();


typedef struct
{
	char space[XCPMAXHOST+1+XCPMAXUSER+1];
	char *host, *user, *file;
}	Netname;


# define X_FILE		'C'
# define X_SYMLINK	'L'
# define X_DIRECTORY	'D'
# define X_END		'E'
# define X_NOERROR	'\0'
# define X_ERROR	'\1'
# define X_FATALERROR	'\2'


short _super = 0;

char iamaserver = 0;		/* when called as server */
char dflag = 0;			/* target must be a dir */
char Lflag = 0;			/* follow symlinks (source only) */
char rflag = 0;			/* dir recursion allowed */
char fflag = 0;			/* called as a source server */
char tflag = 0;			/* called as a sink server */
char vflag = 0;			/* verbose */
char Tflag = 0;			/* use test version of xcp */
char *targetfile;

char protoflags[20];
char *client = "xcp";
char *sourcer = "xcp";
char *sinker = "xcp";
char *dbg_sourcer = "xcp.test -f";
char *dbg_sinker = "xcp.test -t";
char *remotedebugout = "/tmp/xcp.dbg";


short myumask;
int myuserid;

int	remotefd = -1;
int	Errcnt = 0;

char gotEOF; off_t EOFdatum;


char *progname = "xcp";
char *usage = "usage:  xcp [-Lrv] source1 [ source2 ... ] target";

main(argc, argv)
	int argc; char **argv;
{
	register char *ap;
	char argbotch;
	
	umask(myumask = umask(0));
	myuserid = getuid();

	argbotch = 0;
	argc--; argv++;
	while (argc > 0 && *(ap = *argv) == '-') {
		ap++;
		argc--; argv++;

		while( *ap != 000 )
		switch(*ap++) {

		case 'H':
			Lflag = 0;
			break;
		case 'L':
			Lflag++;
			break;
		case 'v':
			vflag++;
			break;
		case 'r':
			rflag++;
			break;
		case 'd':
			dflag++;
			break;
		case 'f':
			fflag++;
			break;
		case 't':
			tflag++;
			break;
		case 'D':
			xcpdebug++;
			break;
		case 'T':
			Tflag++;
			sinker = dbg_sinker;
			sourcer = dbg_sourcer;
			break;
		default:
			argbotch++;
			errwarn("Unknown flag %c", ap[-1]);
			argbotch++;
			break;
		}
	}

	if (fflag && tflag)
		errexit("Incompatible flags [-ft]");

	if (argbotch)
		errexit(usage);

	if (fflag) {
		iamaserver++;
# ifdef DEBUG
		if (xcpdebug) freopen(remotedebugout, "w", stdout);
# endif DEBUG
		ServerSourceXcp(argc, argv);
	}
	else
	if (tflag) {
		iamaserver++;
# ifdef DEBUG
		if (xcpdebug) freopen(remotedebugout, "w", stdout);
# endif DEBUG
		ServerSinkXcp(argc, argv);
	}
	else {
		sprintf(protoflags, "%s%s%s%s%s",
# ifdef DEBUG
			xcpdebug>1?" -D":
# endif DEBUG
			"",
			Tflag>1?" -T":"",
			dflag?" -d":"", Lflag?" -L":"", rflag?" -r":"");

		ClientXcp(argc, argv);
	}

	exit(Errcnt);
}

ServerSourceXcp(argc, argv)
	int argc; char **argv;
{
dprintf((" xcp source server\n"));
	remotefd = 2;
	(void) get_response();
	if (argc < 1)
		errexit("arg cnt");
	while (--argc >= 0)
		source1(*argv++);
}

ServerSinkXcp(argc, argv)
	int argc; char **argv;
{
dprintf((" xcp sink server\n"));
	remotefd = 2;
	if (argc < 1)
		errexit("arg cnt");
	if (--argc > 0)
		errexit("ambiguous target??");
	sink1(*argv++);
}

ClientXcp(argc, argv)
	int argc; char **argv;
{
	Netname Target, Source;
	char *sourcefile, *targetfile;

dprintf((" xcp client\n"));
	if (argc < 2)
		errexit(usage);

	targetfile = argv[--argc];
	if (argc > 1)
		dflag++;

	remotefd = -1;
	signal(SIGPIPE, lostconn);

	splitname(targetfile, &Target);
	if (*Target.file == 000)
		Target.file = ".";

	if (Target.host == 000)
		if (dflag)
			verifydir(Target.file);

	while (--argc >= 0) {
		sourcefile = *argv++;

		splitname(sourcefile, &Source);
		if (*Source.file == 000)
			Source.file = ".";

		NetCopy(&Source, &Target);
	}
}

NetCopy(sp, tp)
	register Netname *sp, *tp;
{
	if (sp->host != 0)
		if (tp->host != 0)
			RRcopy(sp, tp);
		else
			RLcopy(sp, tp);
	else
		if (tp->host != 0)
			LRcopy(sp, tp);
		else
			LLcopy(sp, tp);
}

RRcopy(sp, tp)
	register Netname *sp, *tp;
{
	runsystem("xx %s %s %s %s '%s:%s'", sp->host,
		client, protoflags, sp->file, tp->host, tp->file);
}

LRcopy(sp, tp)
	register Netname *sp, *tp;
{
	/* may be able to reuse remote fd */
	if (remotefd < 0) {
		remotefd = runxcmd(tp->host, tp->user,
			"%s %s -t %s", sinker, protoflags, tp->file);
		if (remotefd < 0)
			return;
		if (get_response() < 0) {
			close(remotefd);
			remotefd = -1;
			return;
		}
	}
	source1(sp->file);
}

LLcopy(sp, tp)
	register Netname *sp, *tp;
{
	/* hack - we don't have cp -r; also cp follows symlinks */
	{
		extern char *getcwd();

		static char *mydirectory;
		char hostbuf[XCPMAXHOST];
		char filebuf[XCPMAXNAME];

		gethostname(hostbuf, sizeof hostbuf);
		tp->host = hostbuf;
		if (*tp->file != '/') {
			if (mydirectory == 0) {
				getcwd(filebuf, sizeof filebuf);
				mydirectory = filebuf;
				if (mydirectory == 0) {
					errwarn("I'm lost!");
					return;
				}
			}
			if (catname(filebuf, sizeof filebuf,
					mydirectory, tp->file) < 0) {
				errwarn("name too long %s", filebuf);
				return;
			}
			tp->file = filebuf;
		}
		LRcopy(sp, tp);
		return;
	}
}

RLcopy(sp, tp)
	register Netname *sp, *tp;
{
	/* can't reuse remote fd in this case */
	remotefd = runxcmd(sp->host, sp->user,
		"%s %s -f %s", sourcer, protoflags, sp->file);
	if (remotefd < 0)
		return;
	sink1(tp->file);
	close(remotefd);
	remotefd = -1;
}

/*
 * data source, ie read files
 */
source1(name)
	char *name;
{
	struct stat stb;

dprintf((" source1(%s)\n",name));
	if (c_stat(name, &stb) < 0) {
		errwarn("%s inaccessible", name);
		return;
	}

	vprintf(("< %s", name));
dprintf(("\n"));

	switch(filetype(&stb)) {

# ifdef S_IFLNK
	case S_IFLNK:
		slsource(name, &stb);
		break;
# endif S_IFLNK

 	case S_IFREG:
		plainsource(name, &stb);
		break;

	case S_IFDIR:
		if (rflag) {
			vprintf(("/\n"));
			dirsource(name, &stb);
			return;
		}
		/* fall through */
	default:
		errwarn("%s is not a plain file", name);
		return;
	}

	vprintf(("\n"));
}

plainsource(name, sp)
	char *name;
	register struct stat *sp;
{
	char buf[BUFSIZ];
	char sizerr;
	register int amt;
	long total, nleft;
	register int f;

dprintf((" plainsource(%s)\n",name));
	if ((f = open(name, 0)) < 0)
		return scerrwarn("can't read %s", name);

	if (remotefile(X_FILE, protection(sp),
			sp->st_size, basename(name)) < 0) {
		(void) close(f);
		return;
	}

	sizerr = 0;
	total = 0;
	nleft = sp->st_size;

	while (nleft > 0) {
		amt = nleft > BUFSIZ ? BUFSIZ : nleft;
		if (!sizerr && read(f, buf, amt) != amt)
			sizerr++;
dprintf((" >>"));
		(void) WRITE(remotefd, buf, amt);
		total += amt;
		nleft -= amt;
	}

	(void) close(f);
	if (!sizerr)
		good_ack();
	else
		errwarn("%s changed size", name);
	(void) get_response();
}

#include <dirent.h>

dirsource(name, sp)
	char *name;
	struct stat *sp;
{
	register struct dirent *dp;
	char buf[XCPMAXNAME];
	int namelen;
	DIR *d;

dprintf((" dirsource(%s)\n",name));
	d = opendir(name);
	if (d == 0) {
		scerrwarn("can't read dir %s", name);
		return;
	}

	if (remotefile(X_DIRECTORY,
			protection(sp), 0L, basename(name)) < 0) {
		closedir(d);
		return;
	}

	namelen = strlen(name);

	while ((dp = readdir(d)) != 0) {
		if (dp->d_ino == 0)
			continue;
		if (strcmp(dp->d_name, ".") == 0
		 || strcmp(dp->d_name, "..") == 0)
			continue;
		if (catname(buf, sizeof buf, name, dp->d_name) < 0) {
			errwarn("name too long %.100s", buf);
			continue;
		}
		source1(buf);
	}

	closedir(d);
	sprintf(buf, "%c\n", X_END);
	netwrite(buf);
	(void) get_response();
}

get_response()
{
	register int resp, c;
	register char *cp;
	char rbuf[MAXPKTSIZE];

	cp = rbuf;

dprintf((" -"));
	if ((c = netgetc()) < 0) {
		lostconn();
		return;
	}

	resp = c;
	switch (resp) {

	case X_NOERROR:
		return (0);

	default:
		*cp++ = resp;
		/* fall through */

	case X_ERROR:
	case X_FATALERROR:
		Errcnt++;
		netgetline(cp, sizeof rbuf-1);
		localwrite(rbuf);
		if (resp == X_FATALERROR)
			exit(1);
		return (-1);
	}
}


/*
 * data sink, ie create files.
 */
sink1(targ)
	char *targ;
{
	char cmdbuf[MAXPKTSIZE];
	char first;
	struct stat stb; char targisdir;

dprintf((" sink1(%s)\n",targ));
	if (dflag)
		verifydir(targ);
	good_ack();
	targisdir = stat(targ, &stb) == 0 && filetype(&stb) == S_IFDIR;

	first = 1;

	for (;;) {
		if (netgetline(cmdbuf, sizeof cmdbuf) < 0)
			return;

dprintf((" cmd %x\n",cmdbuf[0]));
		switch (cmdbuf[0]) {

		case X_ERROR:
		case X_FATALERROR:
			Errcnt++;
			localwrite(cmdbuf+1);
			if (cmdbuf[0] == X_FATALERROR)
				exit(1);
			break;

		case X_END:
			good_ack();
			return;

		case X_FILE:
		case X_SYMLINK:
		case X_DIRECTORY:
			if (filedirsink(targ, targisdir, cmdbuf) < 0) {
				lostconn();
				return;
			}
			break;

		default:
			/*
			 * Check for the case "xcp remote:foo\* local:bar".
			 * In this case, the line "No match." can be returned
			 * by the shell before the xcp command on the remote
			 * is executed so the error message convention is
			 * not followed.
			 */
			if (first)
				errwarn(cmdbuf);
			else
				errwarn("protocol error: control record %s",
					cmdbuf);
			lostconn();
			return;
		}

		first = 0;
	}
}

int
filedirsink(targ, targisdir, cmdstring)
	char *targ;
	char targisdir;
	char *cmdstring;
{
	register int rv;
	char nambuf[XCPMAXNAME];
	char existed; struct stat stb;
	char cmd; short mode; long size; char *fname;

dprintf((" filedirsink(%s)\n",targ));
	if( getfilerecord(cmdstring, &cmd, &mode, &size, &fname) < 0 )
		return errwarn("protocol error: %s", cmdstring);

	if (targisdir) {
		if (catname(nambuf, sizeof nambuf, targ, fname) < 0)
		    return errwarn("name too long %.100s", nambuf);
		targ = nambuf;
	}

	existed = clstat(targ, &stb) == 0;

	switch(cmd) {

	case X_DIRECTORY:
		rv = dirsink(targ);
		break;

	case X_FILE:
		rv = plainsink(targ, size);
		break;

	case X_SYMLINK:
		rv = slsink(targ, size, mode);
		existed = 1;	/* don't chmod! */
		break;

	default:
		rv = -1;
		break;
	}

	if (rv >= 0 && !existed) {
dprintf((" chmod(%s,0%o)\n",targ, mode & ~myumask));
		(void) chmod(targ, mode & ~myumask);
	}

	return rv;
}

int
plainsink(targ, size)
	char *targ;
	long size;
{
	extern int write();

	register int of;
	int wrerror;

dprintf((" plainsink(%s,%ld)\n",targ,size));
	if ((of = creat(targ, 0)) < 0)
		return scerrwarn("can't creat %s", targ);
	good_ack();

	vprintf(("> %s", targ));

	wrerror = sinkloop(of, size, write);

	vprintf(("\n"));

	(void) close(of);
	(void) get_response();

	if (wrerror)
		return scerrwarn("write error");
	good_ack();
	return 0;
}

int
sinkloop(f, size, writer)
	int f;
	long size;
	int (*writer)();
{
	char buf[BUFSIZ];
	long total; register long nleft;
	char wrerror;

	wrerror = 0;
	gotEOF = 0;
	total = 0;
	nleft = size;

	while (nleft > 0) {
		register int ramt, amt;

		amt = nleft > BUFSIZ ? BUFSIZ : nleft;

		ramt = netread(remotefd, buf, amt);

		if (ramt <= 0) {
			if (gotEOF) {
				if (total == EOFdatum) {
					size = total;
					break;
				}
				return errwarn("Size %d should be %d??",
					total, EOFdatum);
			 }
			return errwarn("Size %d premature EOF??", total);
		}

		if( !wrerror && (*writer)(f, buf, ramt) != ramt )
			wrerror--;

		total += ramt;
		nleft -= ramt;
	}

	if (wrerror)
		return scerrwarn("write error");
	return 0;
}

int
dirsink(dirname)
	char *dirname;
{
	struct stat stb;

dprintf((" dirsink(%s)\n",dirname));
	if (stat(dirname, &stb) == 0) {
		if (filetype(&stb) != S_IFDIR)
			return errno = ENOTDIR , -1;
	}
	else {
		if (runsystem("mkdir %s", dirname) < 0)
			return -1;
	}

	return sink1(dirname);
}

good_ack()
{
dprintf((" +"));
	write(remotefd, "", 1);
}


WRITE(f, s, cc)
{
	write(f, s, cc);
}

runxcmd(host, user, args)
	char *host;
	char *user;
	struct { int x[8]; } args;
{
	static char *myusername = 0;
	char buf[MAXPKTSIZE];
	register int x;

	sprintf(buf, args);
	if (user == 0) {
		if (myusername == 0)
			myusername = agetpw(myuserid);
		user = myusername;
	}
dprintf((" runxcmd %s.%s \"%s\"\n",host,user,buf));
# ifdef SNEAKYX
	x = sneakyxcmd(host, user, buf);
# else  SNEAKYX
	x = xcmd(host, buf);
# endif SNEAKYX
	if (x<0)
		errwarn("can't run on %s", host);
	return(x);
}

int
netgetline(buf, len)
	register char *buf;
	int len;
{
	register int c;

	len --;

	for (;;) {
	    if ((c = netgetc()) < 0)
		    return -1;

	    if (c == '\n')
		    break;
	    if (len > 0) {
		    len --;
		    *buf++ = c;
	    }
	}

	*buf++ = 0;
	return 0;
}

int
netgetc()
{
	auto unsigned char ch;
	if (netread(remotefd, &ch, 1) <= 0)
		return -1;
	return ch;
}

int
netread(fd, addr, count)
	int fd;
	char *addr;
	int count;
{
	struct bufio {
		char *_ptr;
		int _cnt;
		char _buf[MAXPKTSIZE];
		char _pad;
	};
	static struct bufio b1;

	if (count == 0) {
		b1._cnt = 0;
		return 0;
	}
	if (b1._cnt <= 0) {
	    static char pkttype;

		b1._ptr = b1._buf;
dprintf((" <<"));
		b1._cnt = xnsread(fd, b1._buf, MAXPKTSIZE, &pkttype, 0);
	    if (b1._cnt <= 0)
		    return b1._cnt;
	    if (pkttype == DST_EOF) {
		    b1._buf[b1._cnt] = 000;
		    b1._cnt = 0;
dprintf((" EOF \"%s\"\n",b1._buf));
		    skipnum(b1._buf, 10, &EOFdatum);
		    gotEOF++;
		    return 0;
	    }
	}

	if (count > b1._cnt)
		count = b1._cnt;

	bcopy(b1._ptr, addr, count);
	b1._ptr += count;
	b1._cnt -= count;

	return count;
}


scerrexit(a)
	struct { int x[6]; } a;
{
	c_error(1, &a);
	exit(1);
}

errexit(a)
	struct { int x[5]; } a;
{
	c_error(0, &a);
	exit(1);
}

int
scerrwarn(a)
	struct { int x[6]; } a;
{
	c_error(1, &a);
	return -1;
}

int
errwarn(a)
	struct { int x[5]; } a;
{
	c_error(0, &a);
	return -1;
}

lostconn()
{
	if (remotefd < 0)
		return;
	close(remotefd);
	remotefd = -1;
	errwarn("lost connection");
}

/*VARARGS*/
c_error(scflag, args)
	int scflag;
	struct { int x[6]; } *args;
{
	extern char *sys_errlist[];
	extern int sys_nerr;
	extern int errno;

	char myhost[XCPMAXHOST];
	char message[MAXPKTSIZE];
	register char *localmessage, *zp;
	register int xerrno;

	/* <HOST> PROGNAME: {ARGS} \n */
	xerrno = errno;

	Errcnt++;
	gethostname(myhost, sizeof myhost);

	localmessage = message;
	sprintf(localmessage, "%c<%s> ", X_ERROR, myhost);
	localmessage += strlen(localmessage);

	zp = localmessage;
	sprintf(zp, "%s: ",progname);
	zp += strlen(zp);

	if (scflag) {
		if ((unsigned)xerrno < sys_nerr)
			sprintf(zp, "%s -- ", sys_errlist[xerrno]);
		else
			sprintf(zp, "Error %d -- ", xerrno);
		zp += strlen(zp);
	}

	sprintf(zp, *args);
	zp += strlen(zp);

	*zp++ = '\n';
	*zp = 000;

	netwrite(message);
	localwrite(localmessage);

	errno = xerrno;
}

int
mkdir(name)
	char *name;
{
	if (runsystem("mkdir %s", name) < 0)
		return -1;
	return 0;
}

verifydir(cp)
	char *cp;
{
	struct stat stb;

	errno = ENOTDIR;
	if (stat(cp, &stb) == 0 && filetype(&stb) == S_IFDIR)
		return 0;
	scerrexit("%s should be a dir", cp);
}

okname(user)
	char *user;
{ 
	register char *cp = user;
	register int c;

	while ((c = *cp++) != 000)
	if (!(isascii(c)
	 && (isalpha(c) || isdigit(c) || c != '_' || c != '-'))) {
		errwarn("xcp: invalid user name %s", user);
		return 0;
	}

	return 1;
}

int
runsystem(args)
	struct { int x[8]; } args;
{
	char buf[MAXPKTSIZE];

	sprintf(buf, args);
dprintf((" runsystem(%s)\n",buf));
	return system(buf);
}

netwrite(string)
    char *string;
{
dprintf((" >>"));
	if (remotefd >= 0)
		WRITE(remotefd, string, strlen(string));
}

localwrite(string)
	register char *string;
{
	register int len;

	if (!iamaserver) {
		len = strlen(string);
		WRITE(2, string, len);
		if (len <= 0 || string[len-1] != '\n')
			WRITE(2, "\n", 1);
	}
}

int
remotefile(cmd, mode, size, name)
	char cmd; short mode; long size; char *name;
{
	char buf[1+4 +1+ 20 +1+ XCPMAXNAME];

	sprintf(buf,"%c%04o %ld %s\n",cmd,mode,size,name);
dprintf((" remotefile %s",buf));
	netwrite(buf);
	return get_response();
}

int
getfilerecord(cmdstring, _cmd, _mode, _size, _name)
	char *cmdstring;
	char *(_cmd); short *(_mode); long *(_size); char **(_name);
{
	long ljunk;
	register char *cp;

dprintf((" filerecord %s",cmdstring));
	cp = cmdstring;
	*_cmd = *cp++;
	if (*_cmd == 000)
		return -1;

	while (isspace(*cp))
		cp++;
	cp = skipnum(cp, 010, &ljunk);
	*_mode = ljunk;

	while (isspace(*cp))
		cp++;
	cp = skipnum(cp, 10, _size);

	while (isspace(*cp))
		cp++;
	*_name = cp;
	while (*cp != 000)
		if(*cp++ == '\n')
			*--cp = 000;
	return 1;
}

int
splitname(name, np)
	char *name;
	register Netname *np;
{
	register char *cp;

	np->host = np->user = 0;
	np->file = name;

	cp = name;
	for (;;) {
		if (*cp == '/' || *cp == 000)
			return 0;
		if (*cp == ':') {
			*cp = 000;
			np->host = np->space;
			strncpy(np->host, name, sizeof np->space-1);
			*cp++ = ':';
			np->file = cp;
			break;
		}
		cp++;
	}

	cp = np->host;
	for (;;) {
		if (*cp == 000)
			break;
		if (*cp == '.') {
			*cp++ = 000;
			np->user = cp;
			break;
		}
		cp++;
	}

	if (np->user != 0 && !okname(np->user))
		return -1;

	if (!_super)
	if (np->user != 0)
		return errwarn(".USER feature not implemented");
	return 0;
}

int
catname(buf, size, dir, comp)
	char *buf;
	int size;
	char *dir;
	char *comp;
{
	register char *zp;
	register char *tp;

	tp = buf;
	zp = tp+size-1;

	tp = strlcpy(tp, zp, dir);
	if (tp > buf && tp[-1] != '/')
		zp++ , *tp++ = '/';
	tp = strlcpy(tp, zp, comp);
	if (*tp == 000)
		return 0;
	return -1;
}

# ifdef S_IFLNK
slsource(name, sp)
	char *name;
	register struct stat *sp;
{
	char buf[BUFSIZ];
	char sizerr;
	register int amt;
	long total, nleft;
	register int f;

dprintf((" slsource(%s)\n",name));
	if ((f = sl_open(name, 0)) < 0)
		return scerrwarn("can't open link %s", name);

	if (remotefile(X_SYMLINK, protection(sp),
			sp->st_size, basename(name)) < 0) {
		(void) sl_close(f);
		return;
	}

	sizerr = 0;
	total = 0;
	nleft = sp->st_size;

	while (nleft > 0) {
		amt = nleft > BUFSIZ ? BUFSIZ : nleft;
		if (!sizerr && sl_read(f, buf, amt) != amt)
			sizerr++;
dprintf((" >>"));
		(void) WRITE(remotefd, buf, amt);
		total += amt;
		nleft -= amt;
	}

	(void) sl_close(f);
	if (!sizerr)
		good_ack();
	else
		errwarn("%s changed size", name);
	(void) get_response();
}

int
slsink(targ, size)
	char *targ;
	long size;
{
	extern int sl_write();

	register int of;
	int wrerror;

dprintf((" slsink(%s,%ld)\n",targ,size));
	if ((of = sl_creat(targ, 0)) < 0)
		return scerrwarn("can't creat link %s", targ);
	good_ack();

	vprintf(("> %s", targ));

	wrerror = sinkloop(of, size, sl_write);

	vprintf(("\n"));

	(void) sl_close(of);
	(void) get_response();

	if (wrerror)
		return scerrwarn("write error");
	good_ack();
	return 0;
}
# endif S_IFLNK

char *
basename(name)
    register char *name;
{
    register char *bp;

    bp = name;

    while( *name != 000 )
	if( *name++ == '/' )
	{
	    while( *name == '/' )
		name++;
	    if( *name == 000 )
		break;
	    bp = name;
	}

    return bp;
}

char *
strlcpy(tp,zp,sp)
    register char *tp,*zp,*sp;
{
    while( tp < zp )
    {
	if( (*tp = *sp) == 000 )
	    break;
	tp++ , sp++;
    }
    return tp;
}

char *
skipnum(sp,defradix,ip)
    register char *sp;
    int defradix;
    long *ip;
{
    register long radix,number;
    register unsigned digit;
    register int negflag;

    /*determine sign and radix*/
    number = 0;
    if( defradix == 0 )
	defradix = 0x10;
    radix = defradix;
    if( (negflag = *sp == '-') || *sp == '+' )
	sp++;			/*skip over optional sign*/
    if( *sp == '0' )
    {				/*leading 0 ==> octal, decimal or hex*/
	radix = defradix;
	sp++;
	if( *sp == 'x' || *sp == 'X' )
	{			/*leading 0x ==> hex*/
	    radix = 0x10;
	    sp++;
	}
	if( *sp == 'o' || *sp == 'O' )
	{
	    radix = 010;
	    sp++;
	}
	if( *sp == 't' || *sp == 'T' )
	{
	    radix = 10;
	    sp++;
	}
    }

    for( ;; )
    {				/*build number a digit at a time*/
	digit = *sp;
	if( (digit += 0-'0') <= ('9'-'0') )
	    digit += 0;		/*digit in '0'..'9'*/
	else
	if( (digit += '0'-'a') <= ('f'-'a') )
	    digit += 0xa;	/*digit in 'a'..'f'*/
	else
	if( (digit += 'a'-'A') <= ('F'-'A') )
	    digit += 0xA;	/*digit in 'A'..'F'*/
	else
	    break;		/*not a digit*/
	if( digit >= radix )
	    break;		/*illegal digit*/
	number = number*radix + digit;
	sp++;
    }

    /*now sp points past the last legal digit*/
    *ip = negflag?-number:number;
    return sp;
}

# include "pwd.h"
extern struct passwd *getpwuid();
char *
agetpw(uid)
    int uid;
{
    static char bertha[XCPMAXUSER];
    register struct passwd *pp;

    setpwent();
    if ((pp = getpwuid(uid)) == 0)
	errexit("Uid %d not in /etc/passwd", uid);
    strcpy(bertha,pp->pw_name);
    endpwent();
    return bertha;
}
