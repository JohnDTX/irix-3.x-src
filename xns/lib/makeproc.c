#include <sys/ioctl.h>
#include <xns/Xnsioctl.h>

/*
 * Make child process by forking twice
 * (so we don't have to wait()).
 */
makeproc()
{
register t;
char buf[8];

	t = fork();
	if (t) {
		wait(buf);
		return(-1);
	}
	t = fork();
	if (t) {
		exit();
	}
}


isolate()
{
register f;

	if (makeproc()<0)
		exit();
	f = xnsfile();
	ioctl(f, NXSETPGRP, 0);
	close(f);
}
