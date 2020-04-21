/*
* $Source: /d2/3.7/src/stand/lib/dev/RCS/in_ntoa.c,v $
* $Revision: 1.1 $
* $Date: 89/03/27 17:14:32 $
*/

#include "types.h"

/*
 * Note:  this buffer must NOT be declared 'static' (as it is
 * in the normal libbsd.a version) or else it winds up as DATA
 * instead of BSS.  Needless to say that doesn't work very well
 * in PROM.
 *
 * Thus the funny name:
 */
char _inet_ntoa_buf[18];

/*
 * Convert network-format internet address
 * to base 256 d.d.d.d representation.
 */
char *
inet_ntoa(in)
	unsigned long in;
{
	register char *p;

	p = (char *)&in;
#define	UC(b)	(((int)b)&0xff)
	sprintf(_inet_ntoa_buf, "%d.%d.%d.%d",
		UC(p[0]), UC(p[1]), UC(p[2]), UC(p[3]));
	return (_inet_ntoa_buf);
}
