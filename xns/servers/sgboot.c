/*
 * SG Boot Server.
 * Original version waits for a boot-socket connection and
 * transfers a file.
 * This version is started by /etc/xnsd when a connection
 * is made to the boot-socket, if the "-x" flag is given.
 */
#include <stdio.h>
#include <sys/types.h>
#include <xns/Xns.h>
#include <xns/Xnsioctl.h>
#include <signal.h>
#include <termio.h>
struct termio ttvec;


char bigbuf[17*1024];
char *bootbuf;
int bootsize = 8*1024;
char buf[2048];
short size;
short file;
char *vec[10];			/* parsed args from network */
char *hostname;			/* name of host */
char *bootdir;			/* default boot directory */
char *procname;			/* argv[0] */
char path[256];			/* place to build pathnames */
short maxpath;			/* max pathname */
short socket;			/* incoming socket number */
int wtime;			/* timeout period for io */
short xflag;                    /* we're being run by xnsd */
short debugflag;		/* causes stuff to print on console */

#define	EQ(a,b)		(strcmp(a,b)==0)


main(argc, argv)
char **argv;
{
	register i, j;
	register c;
	int timeout();

	procname = argv[0];

	if (argc < 3) {
usage:
		fprintf(stderr, "usage: %s [-d] [-x] hostname bootdir\n",
				procname);
		exit(-1);
	}

	for (i = 1; i < argc - 2; i++) {
		if (argv[i][0] == '-') {
			c = argv[i][1];
			switch(c) {
			case 'x':
				xflag = 1;
				break;
			case 'd':
				debugflag = 1;
				break;
			default:
				goto usage;
			}
		} else
			goto usage;
	}

	/* take last two args as hostname and bootdir */
	hostname = argv[argc - 2];
	bootdir = argv[argc - 1];

	strcat(bootdir, "/");
	maxpath = sizeof path - strlen(bootdir) - 1;

	bootbuf = (char *)(((int)bigbuf + 1023) & ~0x1ff);

	setvec();
	if (xflag)
		listen();
	else {
		if (makeproc("", "sgboot")<0)
			exit();
		while (1)
			listen();
	}
}

listen()
{
	int net, cc, nargs, cmd;

	if (xflag)
		net = 0;
	else {
		/*
		 * Set up and wait for boot connection.
		 */
		if ((net=xnsfile())<0) {
			sleep(60);
			return;
		}
		ioctl(net, NXBLOCKIO, 0);
		ioctl(net, NXSETPGRP, 0);
		socket = BOOTSOCKET;
		ioctl(net, NXSOCKET, &socket);
		cc = ioctl(net, NXSOCKWAIT, 0);
		if (cc<0)
			goto out;
	}
	if (debugflag)
		conserr("sgboot connection\n", 0);

	/*
	 * Do timed-out read for config message.
	 */
	cc = tread(net, buf, sizeof buf, 5);
	if (debugflag)
		conserr("sgboot tread returns %d\n", cc);

	if (cc>0) {
		buf[cc] = 0;
		if (debugflag)
			conserr("%s\n", buf);
		if((nargs=scan(buf, cc))<=0) {
			close(net);
			return;
		}
		cmd = vec[0][0];
		file = getfile(cmd, nargs, net);

		if (file<0) {
			close(net);
			return;
		}
		/*
		 * timed transfer of the file.
		 */
		if (cmd==SERV_WRITEFILE) {
			writefile(net, file);
		} else
		while ((cc=read(file, bootbuf, bootsize)) > 0) {
			cc = tsend(net, bootbuf, cc, wtime, DST_OLDDATA);
			if (cc<0) {
				break;
			}
		}
	}

out:
	close(net);
	close(file);
}


writefile(from, to)
{
	struct xnsio io;
	int ret;

	while (1) {
		io.addr = bootbuf;
		io.count = bootsize;
		io.dtype = 0;

		ret = ioctl(from, NXREAD, &io);

		if (ret!=0 || io.count<=0)
			break;
		write(file, bootbuf, io.count);
		write(1, bootbuf, io.count);
	}
}


/*
 * Return file descriptor for transfer, -1 for no transfer.
 * Command syntax:
 *	    vec[0]	 vec[1]	       vec[2]	      vec[3]
 *	SERV_SENDFILE : <hostname> : filename
 */
getfile(cmd, nargs, net)
{
register f, g, t;
struct xnsio io;

	if (debugflag) {
		conserr("sgboot cmd %c ", cmd);
		for(f=0; f<nargs; f++)
			conserr("%s\n", vec[f]);
	}
	switch(cmd) {
	case SERV_BOOTIRIS:
		vec[1] = "*";
		vec[2] = "iris.boot";
	case SERV_SENDFILE:
		wtime = 10;
		if (!EQ(vec[1], hostname) && !EQ(vec[1],"*"))
			return -1;
		if (nargs!=3)
			return -1;
		if (vec[2][0] != '/') {
			strcpy(path, bootdir);
			strncat(path, vec[2], maxpath);
			f = open(path, 0);
			if (f>=0)
				return f;
		}
		return(open(vec[2], 0));
	case SERV_READFILE:
		if (!EQ(vec[1], hostname))
			return -1;
		if ((f = open(vec[2], 0)) < 0) {
			io.addr = buf;
			io.count = error(buf);
			io.dtype = DST_FERROR;
			ioctl(net, NXWRITE, &io);
			return f;
		}
		t = makeproc(vec[3], "readfile");
		if (t)
			return -1;
		wtime = 0;
		return f;
	case SERV_WRITEFILE:
		if (!EQ(vec[1], hostname))
			return -1;
		if ((f = creat(vec[2], 0666)) < 0) {
			io.addr = buf;
			io.count = error(buf);
			io.dtype = DST_FERROR;
			ioctl(net, NXWRITE, &io);
			return f;
		}
		t = makeproc(vec[3], "writefile");
		if (t)
			return -1;
		wtime = 0;
		return f;

	default: 
		return -1;
	}
}


/*
 * Parse colon-separated tokens of string s.
 * vec[i] points to the ith token.
 * return number of tokens found.
 */
scan(s, len)
register char *s;
{
	register c, i;
	register char *end;

	i = 0;
	if (len<=0)
		return i;

	end = &s[len+1];
	vec[0] = s;

	for(; s<end; s++) {
		c = *s;
		if (c!=':' && c!='\0' && c!='\r' && c!='\n')
			continue;
		*s++ = 0;
		vec[++i] = s;
		if (c=='\0'||c=='\r'||c=='\n')
			break;
	}
	return i;
}


/*
 * Make child process by forking twice
 * (so we don't have to wait()).
 */
makeproc(u, name)
char *u;
char *name;
{
	register t;
	register char *s;

	t = fork();
	if (t)
		return -1;
	t = fork();
	if (t)
		exit();
	for(s=procname; *s; s++) {
		if (*name)
			*s = *name++; else
			*s = 0;
	}
}


error(s)
register char *s;
{
	extern errno, sys_nerr;
	extern char *sys_errlist[];
	register char *e;

	e = (errno < sys_nerr) ? sys_errlist[errno] : "Unknown error";
	s[0] = (char)errno;
	s[1] = 0;
	strcat(&s[1], e);
	return strlen(s);
}
