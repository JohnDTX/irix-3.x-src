#include <xns/Xnsioctl.h>
#include "zmsg.h"
#include <stdio.h>

zrecv(f, buf, size, dowait)
char *buf;
{
register cc;
register ZMSG p;
struct xnsio io;

	p = (ZMSG)buf;
	if (dowait) {
		cc = read(f, buf, size);
	} else {
		io.addr = buf;
		io.count = size;
		io.dtype = 0;
		cc = ioctl(f, NXREAD, &io);
		cc = io.count;
		if (cc==0)
			return(0);
	}

	if (cc <= sizeof (struct zmsg))
		return(-1);

	if (p->version != ZVERSION) {
		fprintf(stderr, "z library version error\n");
		fprintf(stderr, "receiving version %d, we have version %d\n",
			p->version, ZVERSION);
		return(-1);
	}

	return(cc - sizeof (struct zmsg));
}
