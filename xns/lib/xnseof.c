#include <sys/types.h>
#include <sys/ioctl.h>
#include <xns/Xns.h>
#include <xns/Xnsioctl.h>

xnseof(f)
{
struct xnsio io;

	io.addr = 0;
	io.count = 0;
	io.dtype = DST_END;
	io.control = 0;
	ioctl(f, NXWRITE, &io);
}
