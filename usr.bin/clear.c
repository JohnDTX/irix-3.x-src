char _Origin_[] = "UC Berkeley";

#
	char	*sccsid = "@(#)clear.c	2.3";

/* load me with -ltermlib */
/* #include <retrofit.h> on version 6 */
/*
 * clear - clear the screen
 */

#include <stdio.h>
#include <termio.h>

char	*getenv();
char	*tgetstr();
char	PC;
short	ospeed;
#undef	putchar
int	putchar();

main()
{
	char *cp = getenv("TERM");
	char clbuf[20];
	char pcbuf[20];
	char *clbp = clbuf;
	char *pcbp = pcbuf;
	char *clear;
	char buf[1024];
	char *pc;
	struct termio tty;

	ioctl(1, TCGETA, &tty);
	ospeed = tty.c_cflag & CBAUD;
	if (cp == (char *) 0)
		exit(1);
	if (tgetent(buf, cp) != 1)
		exit(1);
	pc = tgetstr("pc", &pcbp);
	if (pc)
		PC = *pc;
	clear = tgetstr("cl", &clbp);
	if (clear)
		tputs(clear, tgetnum("li"), putchar);
	exit (clear != (char *) 0);
}
