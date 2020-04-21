/*
 * SGI Broadcast boot/name server.
 */

#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <xns/Xns.h>
#include <sys/ioctl.h>
#include <xns/Xnsioctl.h>

int net;			/* net file descriptor */
char myaddr[6];			/* local physical address */
short etype;			/* ether type */
char bootaddr[6];		/* boot address delivered in reply */
char buf[2048];			/* input buffer */
char *vec[10];			/* parsed args from network */
char *hostname;			/* name of host */
char *bootdir;			/* default boot directory */
char path[256];			/* place to build pathnames */
short maxpath;			/* space remaining in path after bootdir */
short debugflag;

#define	EQ(a,b)		(strcmp(a,b)==0)

#ifdef BSD4.2
ignore()
{
}
struct sigvec ignvec ={ ignore, (int)SIG_IGN, NULL};
#endif

main(argc, argv)
	int argc;
	char **argv;
{
	register int i, c;
	char *procname;

	procname = argv[0];
	if (argc < 3)
		goto usage;
	/*
	 * Check for options
	 */
	for (;;) {
		argv++;
		argc--;
		if (argv[0][0] == '-') {
			switch (argv[0][1]) {
			  case 'd':
				debugflag++;
				continue;
			  default:
				goto usage;
			}
		}
		break;
	}
	if (argc != 2)
		goto usage;

	/* take last two args as hostname and bootdir */
	hostname = argv[0];
	bootdir = argv[1];
	strcat(bootdir, "/");
	maxpath = (sizeof path) - strlen(bootdir) - 1;
	setup();
	listen();

usage:
	fprintf(stderr, "usage: %s [-d] hostname bootdir\n", procname);
	exit(-1);
}

setup()
{
	int x;
	int timeout();

	if (makeproc()<0)
		exit(0);
	/*
	 * open network file.
	 */
	net = xnsfile();
	if (net<0) {
		conserr("sgbounce: can't open net file\n", 0);
		exit(1);
	}
	/*
	 * find out what our physical net address is.
	 * Condition the driver for raw mode and for receiving
	 * SG_BOUNCE type packets.
	 */
	ioctl(net, NXSETPGRP, 0);
	x = ioctl(net, NXPHYSADDR, myaddr);
	x = ioctl(net, NXIORAW, 0);
	x = ioctl(net, NXIOBOUNCE, 0);

#ifdef SYSTEMV
	etype = 0x8016;
#else
	etype = 0x1680;
#endif

#ifdef BSD4.2
	sigvec(SIGHUP, &ignvec, NULL);
	sigvec(SIGINT, &ignvec, NULL);
	sigvec(SIGTERM, &ignvec, NULL);
#else
	signal(SIGHUP, SIG_IGN);
	signal(SIGINT, SIG_IGN);
	signal(SIGTERM, SIG_IGN);
#endif
}

listen()
{
	register cc, nargs, f;
	register char *p;
	int bcast();

	/*
	 *	read the net, reject non-broadcast packets,
	 *	use received src address as outgoing dst address,
	 *	use myaddr as outgoing src address,
	 *	return the broadcast message only if getfile
	 *	returns a positive value.
	 */
	while (1) {
		cc = read(net, buf, sizeof buf);
		if (debugflag)
			printf("[%d]\n", cc);
		if (cc<=0) {
			continue;
		}
		if (debugflag)
			prw(buf, 14);
		if (isbcast(buf)==0)
			continue;
		if((nargs=scan(&buf[16], cc-16))<=0)
			continue;

		if ((f=getfile(buf[16], nargs))<=0)
			continue;
		buf[16] = f;
		bcopy(&buf[6], buf, 6);
		bcopy(myaddr, &buf[6], 6);
		bcopy(&etype, &buf[12], 2);
		if (debugflag)
			prw(buf, 14);
		p = &buf[18];
		strcpy(p, hostname);
		strcat(p, ":");
		strcat(p, path);
		cc = write(net, buf, 128);
		if (debugflag)
			printf("sent %d\n", cc);
	}
}

getfile(cmd, nargs)
	int cmd;
	int nargs;
{
	int f;

	switch(cmd) {
	  case SERV_BOOTIRIS:
		vec[2] = "iris.boot";
		vec[1] = "*";
		/* FALLTHROUGH */
	  case SERV_SENDFILE:
		if (!EQ(vec[1], hostname) && !EQ(vec[1],"*"))
			return(0);
		if (nargs!=3)
			return(0);

		/*
		 * first try default boot directory
		 * (skip this if vec[2] is an absolute path)
		 */
		if (vec[2][0] != '/') {
			strcpy(path, bootdir);
			strncat(path, vec[2], maxpath);
			f = open(path, 0);
			if (f>=0) {
				close(f);
				return(SERV_REPLY);
			}
		}
		f = open(vec[2], 0);
		if (f>=0) {
			close(f);
			strcpy(path, vec[2]);
			return(SERV_REPLY);
		}
		if (EQ("*", vec[1]))
			return(-1);
		return(SERV_NOFILE);
	  case SERV_HOSTNAME:
		if (strcmp(hostname, vec[1])==0)
			return(SERV_IDENT);
	  default: 
		return(-1);
	}
}

/*
 * check for broadcast address.
 */
isbcast(s)
	register unsigned char *s;
{
	register unsigned char x = 0xff;
	register int i;

	for(i=0; i<6; i++)
		if (*s++ != x)
			return(0);
	return(1);
}

scan(s, len)
	register char *s;
	int len;
{
	register c, i;
	register char *end;

	i = 0;
	if (len<=0)
		return(i);

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
	return(i);
}
