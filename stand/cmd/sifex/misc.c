/*
**	misc.c		- Copyright (C), JCS Computer Services 1983
**			- Author: chase
**			- Date: January 1985
**			- Any use, copying or alteration is prohibited
**			- and morally inexcusable,
**			- unless authorized in writing by JCS.
**	$Source: /d2/3.7/src/stand/cmd/sifex/RCS/misc.c,v $
**	$Revision: 1.1 $
**	$Date: 89/03/27 17:13:17 $
*/

#include "types.h"
#include <sys/dklabel.h>
#include "disk.h"
#include "streg.h"
#include "fex.h"

rsq(msg)
char *msg;
{
	isinited[dunit] = 0;		/* Force reset on next operation */
	if(msg) printf("%s: ", msg);
    for(;;) {
	printf("Retry/Skip/Quit/Verbose? ");
	switch(gch()) {
	case 'r':	printf("Retry\n"); return RETRY;
	case 's':	printf("Skip\n"); return SKIP;
	case 'q':	printf("Quit\n"); return QUIT;
	case 'v':	printf("Verbose %s\n", (verbose = !verbose)?
				"on": "off"); continue;
	default:	printf("Answer r/s/q/v please\n");
	}
    }
}

uleq(a, b)
register char *a, *b;
{
	while(*a || *b)
		if((*a++|040) != (*b++|040))
			return 0;
	return 1;
}

bfill(cp, val)
register char *cp;
register val;
{
	register i, j = secsize[dunit];

	for(i = 0; i < j; i++)
		*cp++ = (val==-1) ? i  : val;
}

getnum(base, min, max)
{
	register c;
	register nc = 0;
	char buf[16];

	if(base == 16) printf("0x");
	else if(base == 8) printf("0");
	buf[0] = 0;
	while((c = gch()) != '\n') {
		if(c == '\b') {
			if(nc) {
				printf("\b \b");
				nc--;
				buf[nc] = 0;
			}
			continue;
		}
		if(c == 'U'-'@') {
			while(nc) {
				printf("\b \b");
				nc--;
			}
			buf[nc] = 0;
			continue;
		}

		switch(c) {
		case 'a': case 'b': case 'c':
		case 'd': case 'e': case 'f':
			if(base < 16) {
				printf("\007");
				continue;
			}
			break;
		case '9': case '8':
			if(base < 10) {
				printf("\007");
				continue;
			}
			break;
		case '0': case '1': case '2': case '3':
		case '4': case '5': case '6': case '7':
			break;
		default:
			printf("\007");
			continue;
		}
		printf("%c", c);
		buf[nc++] = c;
		buf[nc] = 0;
		if(atoi(buf, base) > max) {
			printf(">>>Max:");
			pn(max, base);
			return -1;
		}
	}
	if(nc && atoi(buf, base) < min) {
		printf("<<<Min:");
		pn(min, base);
		return -1;
	}
	if(nc) return atoi(buf, base);
	return -1;
}

cvt(c)
register c;
{
	if(c > '9')
		return c - 'a' + 10;
	return c - '0';
}

atoi(str, base)
register char *str;
{
	register n = 0;

	while(*str) {
		n *= base;
		n += cvt(*str++);
	}
	return n;
}

pn(n, base)
{
	printf(base==16?"0x%x" : base==8?"0%o" : "%d", n);
}

char *
getline()
{
	static char buf[128];
	register c, cn=0;

	buf[0] = 0;
	while((c = gch()) != '\n') switch(c) {
	case '\b':
		if(cn) {
			printf("\b \b");
			buf[--cn] = 0;
		}
		break;
	case 'U'-'@':
		while(cn) {
			printf("\b \b");
			cn--;
		}
		buf[0] = 0;
		break;
	case 'W'-'@':
		while(cn && buf[cn-1] == ' ') {
			printf("\b");
			buf[--cn] = 0;
		}
		while(cn && buf[cn-1] != ' ') {
			printf("\b \b");
			buf[--cn] = 0;
		}
		break;
	default:
		buf[cn++] = c;
		buf[cn] = 0;
		printf("%c", c);
	}
	return buf;
}

/*
** NOT DEFINED STUFF
*/
dskintr()
{
}

rdwrsq(msg)
	char *msg;
{
	isinited[dunit] = 0;		/* Force reset on next operation */
	if(msg) printf("%s: ", msg);
    for(;;) {
	printf("re-Read/re-Write/Skip/Quit/Verbose (r/w/s/q/v)? ");
	switch(gch()) {
	case 'r':	printf("re-Read\n"); return 0;
	case 'w':	printf("re-Write\n"); return 1;
	case 's':	printf("Skip\n"); return 2;
	case 'q':	printf("Quit\n"); return 3;
	case 'v':	printf("Verbose %s\n", (verbose = !verbose)?
				"on": "off"); continue;
	default:	printf("Answer r/w/s/q/v please\n");
	}
    }
}
