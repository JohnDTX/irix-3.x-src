#include <sys/types.h>
#include <xns/Xns.h>
#include <sys/ioctl.h>
#include <xns/Xnsioctl.h>
#ifndef SYSTEMV
#include <sgtty.h>
#endif  SYSTEMV
#include <signal.h>
#include <pwd.h>
#ifndef NULL
#define	NULL 0
#endif

static struct xns_setup dial;
static char buf[1024];

extern char xnsmsg[];

xcmd(host, command)
char *host, *command;
{
	return(_xcmd(host, command, EXECSOCKET));
}

xsh(host)
char *host;
{
	return(_xcmd(host, "", XSHSOCKET));
}


_xcmd(host,  command, socket)
char *host,  *command;
{
	struct passwd *pwd;
	int cc;
	int x;
	int r;
	register char *p, *q, *user;
	char ourhost[32];

	/*
	 * make network connection to host specified by argv[1].
	 */
	x = xnsfile();
	if (x<0)
		return(x);

	dial.internet.socket = socket;
	strcpy(dial.name, host);

	
	setpwent();
	pwd = (struct passwd *)getpwuid(getuid());
	endpwent();
	user = pwd->pw_name;

	r = ioctl(x, NXCONNECT, &dial);
	if (r<0) {
		sprintf(xnsmsg, "Can't connect to <%s>\n", host);
		return(r);
	}

	for(p=buf, q=user; *q; ) {
		*p++ = *q++;
	}
	*p++= 0;

	gethostname (ourhost, sizeof ourhost);
	for(q=ourhost; *q; ) {
		*p++ = *q++;
	}
	*p++ =0;
	xnswrite(x, buf, p-buf, DST_CMD, 0);
	p = buf;
	for(q=command; *q; ) {
		*p++ = *q++;
	}
	*p++ = 0;
	*p++ = 0;
	*p++ = 0;


	xnswrite(x, buf, p-buf, DST_CMD, 0);

	return(x);
}
