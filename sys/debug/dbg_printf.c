/*
 * Kernel debugger trace printf.
 */
#ifdef OS_DEBUG
#include "../h/printf.h"
#include "../h/types.h"

/*
 * dbg_printf:
 *	- var args hook for tracing, etc.
 */
/*VARARGS2*/
dbgprintf(flag, fmt, x1)
	int flag;
	char *fmt;
	int x1;
{
	if (dbgtest(flag)) {
		doprnt(con_putchar, fmt, &x1);
	}
}



#define TRACE_BUFSIZ 500
char print_buf[TRACE_BUFSIZ];
static char *stashputc_addr = print_buf;
static int pbuf_remaining = TRACE_BUFSIZ;
static int pbuf_ovfl = 0;
static int pbuf_inuse = 0;

extern short kswitch;

/*
 * trace_printf  -  print to console
 *		(or force printing to serial port if kswitch is TRUE).
 *	If console is graphics, print to a stash buffer if the current
 *	interrupt priority level is high.
 */
/*VARARGS1*/
trace_printf(fmt, x1)
  u_int x1;
{
	register int s, slev;
	extern dbgstash();

	if (con_putchar == duputchar || kswitch) {
		doprnt(duputchar, fmt, &x1);
		return;
	}
	s = spl7();
	slev = (s & 0x0700) >> 8;
	if (slev >= 3) {
		/* save chars in stash buffer if interrupt priority >= 3 */
		splx(s);
		dbgstash(fmt, &x1);
		return;
	}
	pbuf_inuse++;
	splx(s);

	if (print_buf[0]) {
		printf("%s", print_buf);
		s = spl7();
		stashputc_addr = print_buf;
		*stashputc_addr = 0;
		pbuf_remaining = TRACE_BUFSIZ;
	}
	pbuf_inuse = 0;
	if (pbuf_ovfl) {
		pbuf_ovfl = 0;
		splx(s);
		printf("\n - print buf ovfl -\n");
	} else
		splx(s);

	doprnt(con_putchar, fmt, &x1);
}

/*VARARGS1*/
static
dbgstash(fmt, x1)
  u_int x1;
{
	int s;
	extern stashputc();

	if ((pbuf_remaining > 0) && (pbuf_inuse == 0)) {
		s = spl6();
		doprnt(stashputc, fmt, x1);
		*stashputc_addr = 0;
		splx(s);
	} else
		pbuf_ovfl++;
}

int
stashputc(c)
  register char c;
{
	if (--pbuf_remaining > 0)
		*stashputc_addr++ = c;
}
#endif OS_DEBUG
