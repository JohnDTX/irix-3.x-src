/* @(#)imsg.c	1.3 */
/* $Source: /d2/3.7/src/usr.bin/uucp/RCS/imsg.c,v $*/
/* $Revision: 1.1 $*/
/* $Date: 89/03/27 18:30:29 $*/

#include "uucp.h"

#define	read	Read
#define	write	Write

char Msync[2] = "\020";

/*
 * read message routine used before a
 * protocol is agreed upon.
 *	msg	-> address of input buffer
 *	fn	-> input file descriptor 
 * returns:
 *	EOF	-> no more messages
 *	0	-> message returned
 */
imsg(msg, fn)
register char *msg;
register int fn;
{
	register char c;
	short fndsync;
	int ret;
	char *bmsg;
	static char c1;

	fndsync = 0;
	bmsg = msg;
	DEBUG(7, "imsg %s>", "");
	while ((ret = read(fn, &c1, sizeof(c1))) == sizeof(c1)){
		c = c1;
		c1 &= 0177;
		DEBUG(7, (c1>=' '||c1=='\n') ? "%c" : "\\%03o ", c1);
		if (c == Msync[0]) {
			msg = bmsg;
			fndsync = 1;
		}
		else if (c == '\0' || c == '\n') {
			if (fndsync) {
				*msg = '\0';
				return(0);
			}
		} else {
			if (fndsync)
				*msg++ = c;
		}

	}
	*msg = '\0';
	return(EOF);
}

/*
 * initial write message routine -
 * used before a protocol is agreed upon.
 *	type	-> message type
 *	msg	-> message body address
 *	fn	-> file descriptor
 * return: 
 *	always 0
 */
omsg(type, msg, fn)
register char *msg;
register char type;
int fn;
{
	register char *c;
	char buf[BUFSIZ];

	c = buf;
	*c++ = Msync[0];
	*c++ = type;
	while (*msg)
		*c++ = *msg++;
	*c++ = '\0';
	write(fn, buf, (unsigned) strlen(buf) + 1);
	return(0);
}
