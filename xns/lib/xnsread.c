#include <sys/ioctl.h>
#include <xns/Xnsioctl.h>

xnsread(f, s, cc, dtype, control)
char *s, *dtype, *control;
{
struct xnsio io;

	io.addr = s;
	io.count = cc;
	ioctl(f, NXREAD, &io);
	if (dtype)
		*dtype = io.dtype;
	if (control)
		*control = io.control;
	return(io.count);
}
