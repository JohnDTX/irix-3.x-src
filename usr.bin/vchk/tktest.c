/*
 * (C) 1983 UniSoft Corp. of Berkeley CA
 *
 * UniPlus Source Code. This program is proprietary
 * with Unisoft Corporation and is not to be reproduced
 * or used in any manner except as authorized in
 * writing by Unisoft.
 */

#include <stdio.h>
main (c, l)
	char *l[];
{
	char *sp;
	char buf[1000];

	sp = buf;
	sp += cps(sp,"/usr/bin/take");
	while (*++l) {
		*sp++ = ' ';
		sp += cps(sp,*l);
	}
	fprintf(stderr,"%s\n",buf);
	fprintf(stderr,"returned %d\n",exec(buf));
}
