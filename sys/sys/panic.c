/*
 * Scaled down version of C Library printf:
 *	- only %s %u %d (==%u) %o %x %D are recognized
 *	- used to print diagnostic information directly on console tty.
 *	  Since it is not interrupt driven, all system activities are
 *	  pretty much suspended
 *
 * $Source: /d2/3.7/src/sys/sys/RCS/panic.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:35:27 $
 */

#include "debug.h"
#include "xns.h"

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/tty.h"
#include "../h/user.h"
#include "../h/conf.h"
#include "../h/printf.h"
#include "../h/inode.h"
#include "../streams/stream.h"
#include "../streams/strcomp.h"

short	panicing;

/*
 * panic:
 *	 - panic is called on unresolvable fatal errors
 */
panic(s)
	char *s;
{
#if NDEBUG > 0
	extern short kswitch;
#endif

	(void) spl7();
#if NDEBUG > 0
	if (kswitch)
		setConsole(CONSOLE_ON_SERIAL);
	else
#endif
	setConsole(CONSOLE_NOT_ON_PTY);

	if (panicing) {
		printf("panic recursion: %s\n", s);
		halt();
	} else {
		resetConsole();
		panicing = 1;
		printf("panic: %s\n", s);
#if NDEBUG > 0
		debug( "PANIC" );
#endif
		update();
	}
	halt();
}

/*
 * Write a character to a stream.  Return 0 on failure, 1 on success.
 */
int
streamPutChar(ip, c)
	struct inode *ip;
	char c;
{
	register struct stdata *stp;
	register mblk_t *mp;

	stp = ip->i_sptr;
	ASSERT(stp);

	mp = allocb(1, BPRI_HI);
	if (mp) {		/* ignore operation if no buffer */
		*mp->b_wptr++ = c;
		putnext(stp->sd_wrq, mp);
		if (qready())
			runqueues();
		return 1;
	}
	return 0;
}

#if NXNS > 0
static
uputchar(c)
	char c;
{
	struct tty *tp;

	tp = (struct tty *)((char*)u.u_ttyp
			    - (long)(&((struct tty *)0)->t_pgrp));
	ttuwrite(tp, c);
}
#else
static
uputchar(c)
	char c;
{
	if (u.u_ttyip)
		(void) streamPutChar(u.u_ttyip, c);
}
#endif

/*
 * uprintf:
 *	- print out to the current user and the console
 */
/*VARARGS1 PRINTFLIKE1*/
uprintf(fmt, args)
	char *fmt;
	int args;
{
	int s;
	extern short console_dev;

	s = spl7();
	doprnt(con_putchar, fmt, &args);
	splx(s);
	/*
	 * Print out to current users terminal, if the user is not
	 * using the console.
	 */
	if (u.u_ttyp
	    && major(u.u_ttyd) != console_dev)
		doprnt(uputchar, fmt, &args);
}

/*
 * prdev prints a warning message.
 * dev is a block special device argument.
 */
prdev(str, dev)
	char *str;
	dev_t dev;
{
	register dev_t maj;

	maj = bmajor(dev);
	if (maj >= bdevcnt)
		uprintf("%s on bad dev 0x%04x\n", str, dev);
	else
		(*bdevsw[maj].d_print)(dev, str);
}
