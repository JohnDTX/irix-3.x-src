/*
 * Indirect console driver
 *
 * $Source: /d2/3.7/src/sys/ipII/RCS/console.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:30:38 $
 */
#include "../h/param.h"
#include "../h/user.h"
#include "../h/inode.h"
#include "../h/conf.h"
#include "../h/printf.h"
#include "../h/setjmp.h"
#include "../streams/strcomp.h"
#include "sgigsc.h"

extern	short win_dev, duart_dev, console_dev;
extern	int *nofault;
extern	int gr_reset();

int	(*con_putchar)() = duputchar;
int	(*con_getchar)() = dugetchar;
int	(*con_reset)() = 0;
char	consoleOnPTY;

#ifndef	KOPT_NOGL
short	consduart = 0;
short	havegrconsole;
#endif

/*
 * Reset the console
 */
void
resetConsole()
{
	if (con_reset) {
		(*con_reset)();
	}
}

/*
 * Set the device that the console is using.
 * Once we have switched consoles, reset the new one.
 */
void
setConsole(where)
	int where;
{
	struct cdevsw *from;
	static struct cdevsw *lastFrom;

	consoleOnPTY = 0;
#ifdef	KOPT_NOGL
	where = CONSOLE_ON_SERIAL;
#else
	if (where == CONSOLE_NOT_ON_PTY) {
		if (consduart || !havegrconsole)
			where = CONSOLE_ON_SERIAL;
		else
			where = CONSOLE_ON_WIN;
		lastFrom = 0;			/* force reset */
	}
#endif
	switch (where) {
	  case CONSOLE_ON_SERIAL:
		from = &cdevsw[duart_dev];
		con_putchar = duputchar;
		con_getchar = dugetchar;
		con_reset = 0;
		break;
#ifndef	KOPT_NOGL
	  case CONSOLE_ON_WIN:
		from = &cdevsw[win_dev];
		con_putchar = grputchar;
		con_getchar = grgetchar;
		con_reset = gr_reset;
		break;
#if NSGIGSC > 0
	  case CONSOLE_ON_PTY:
		from = &cdevsw[win_dev];
		con_putchar = ptyPutChar;
		con_getchar = 0;		/* force crash if used */
		con_reset = 0;
		consoleOnPTY = 1;
		break;
#endif
#endif
	}
	bcopy(from, &cdevsw[console_dev], sizeof(struct cdevsw));
}

/*
 * Initialize the console.  Choose which device the console will
 * be using.
 */
con_init()
{
	/* force console to serial device */
	setConsole(CONSOLE_ON_SERIAL);
#ifndef	KOPT_NOGL
	{
		int *saved_jb;
		jmp_buf jb;

		/*
		 * Try to reset the graphics subsystem.  If that succeeds,
		 * and we haven't been patched to use the serial console,
		 * switch the console to the graphics device.
		 */
		saved_jb = nofault;
		if (setjmp(jb) == 0) {
			nofault = jb;
			gr_init();
			havegrconsole = 1;
			if (consduart == 0)
				setConsole(CONSOLE_ON_WIN);
		}
		nofault = saved_jb;
	}
#endif
	initvectors();
}
