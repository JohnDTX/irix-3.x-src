#include <xns/Xnsioctl.h>
#include "zmsg.h"
#include <stdio.h>

static unsigned char addr[6], slot[6];



zinit(msg, name, discardmode, debugflag)
register struct zmsg *msg;
char *name;
{
register x;
register val;

	
	/*
	 * get network file descriptor.
	 * is hardwired for now.
	 */
	x = open("/dev/ttyn7", 2);
	if (x<0) {
		if (debugflag)
		fprintf(stderr, "can't open /dev/ttyn7\n"); fflush(stderr);
		return(-1);
	}


	/*
	 * construct multicast address from name.
	 */
	addr[0] = 0xff;
	bzero(&addr[1], 5);
	strncpy(&addr[1], name, 5);
	msg->etype = 0x8014;
	msg->version = ZVERSION;


	/*
	 * set up address slot on ethernet board.
	 */
	ioctl(x, NXPUTSLOT, addr);
	ioctl(x, NXGETSLOT, slot);
	if (bcmp(slot, addr)) {
		if (debugflag)
		fprintf(stderr, "cannot init address slot\n"); fflush(stderr);
		return(-2);
	}


	/*
	 * enable address slot for reception.
	 */
	val = ioctl(x, NXSETRCV, (discardmode)?10:1);
	if (val) {
		if (debugflag) {
			fprintf(stderr, "cannot enable address slot\n");
			fflush(stderr);
		} else
			return(-1);
	}



	/*
	 * construct packet header.
	 */
	bcopy(slot, msg->dst, 6);
	ioctl(x, NXPHYSADDR, msg->src);
	msg->etype = 0x8014;




	/*
	 * enable raw output.
	 */
	ioctl(x, NXIORAW, 0);


	return(x);
}
