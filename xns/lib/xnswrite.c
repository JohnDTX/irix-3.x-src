#include <sys/ioctl.h>
#include <xns/Xnsioctl.h>

xnswrite(f, s, cc, dtype, control)
char *s;
{
struct xnsio io;

	io.addr = s;
	io.count = cc;
	io.dtype = dtype;
	io.control = control;
	ioctl(f, NXWRITE, &io);
	return(io.count);
}
