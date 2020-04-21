#include <sys/types.h>
#include <xns/Xns.h>
#include <sys/ioctl.h>
#include <xns/Xnsioctl.h>

static struct xns_setup dial;
extern char xnsmsg[];

xnsconnect(host, socket)
char *host;
{
register net, error;

	net = xnsfile();
	if (net<0) {
		return(net);
	}

	strcpy(dial.name, host);
	dial.internet.socket = socket;

	error = ioctl(net, NXCONNECT, &dial);
	if (error<0) {
		sprintf(xnsmsg, "Can't connect to <%s>\n", host);
		close(net);
		net = error;
	}

	return(net);
}
