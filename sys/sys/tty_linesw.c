/*
 * $Source: /d2/3.7/src/sys/sys/RCS/tty_linesw.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:35:42 $
 */
#include "../h/types.h"
#include "../h/conf.h"

/*
 * Line Discipline Switch Table
 */
extern	nulldev();
extern	ttopen(), ttclose(), ttread(), ttwrite(), ttioctl(), ttin(), ttout();

/* order:	open close read write ioctl rxint txint modemint */

struct	linesw linesw[] = {
	{ ttopen, ttclose, ttread, ttwrite,
	  ttioctl, ttin, ttout, nulldev, },			/* 0 */
};

/* number on entries in linesw */
short	linecnt = sizeof(linesw) / sizeof(linesw[0]);
