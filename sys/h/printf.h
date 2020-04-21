/*
 * Defines for the kernel fprintf
 *
 * $Source: /d2/3.7/src/sys/h/RCS/printf.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:29:51 $
 */
#ifdef	KERNEL

/* serial */
extern	int duputchar(), dugetchar(), duconsreset();

/* kernel window */
extern	int grputchar(), grgetchar(), grconsreset();

/* psuedo-tty */
extern	int ptyPutChar(), ptyConsoleReset();

/* pointers to current console routines */
extern	int (*con_putchar)(), (*con_getchar)();

extern	short havegrconsole;		/* non-zero if graphics console */
extern	char consoleOnPTY;		/* non-zero if console on pty */

#if 0
/* flags to console putchar routines */
#define	CO_BEGIN	-1		/* start output */
#define	CO_END		-2		/* end output (flush) */
#define	CO_RESET	-3		/* reset the console */
#endif

/* where console i/o is going */
#define	CONSOLE_ON_SERIAL	0
#define	CONSOLE_ON_WIN		1
#define	CONSOLE_ON_PTY		2
#define	CONSOLE_NOT_ON_PTY	3

#endif	KERNEL
