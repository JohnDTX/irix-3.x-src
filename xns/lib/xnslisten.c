#include <sys/ioctl.h>
#include <xns/Xnsioctl.h>

xnslisten(x)
{
int net, cc;
short socket;

	socket = x;
	if ((net=xnsfile())<0) {
		return(-1);
	}
	ioctl(net, NXBLOCKIO, 0);
	ioctl(net, NXIOFAST, 0);
	ioctl(net, NXSETSOCKWAIT, &socket);

	return(net);
}
