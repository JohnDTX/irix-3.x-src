/*
 * ttymodes.c - switch tty modes between cooked and raw
 */

 /* $Source: /d2/3.7/src/xns/lib/RCS/ttymodes.c,v $ */
 /* @(#)$Revision: 1.1 $ */
 /* $Date: 89/03/27 18:41:31 $ */

#include <termio.h>

static	struct	termio old;
static	struct	termio new;
static	short	saved;

rawmode(fd)
	int fd;
{
	if (saved == 0) {
		ioctl(fd, TCGETA, &old);
		saved = 1;
	}
	ioctl(fd, TCGETA, &new);
	new.c_cflag &= ~(CSIZE);
	new.c_cflag |= CS8;
	new.c_iflag &= ~(-1);
	new.c_lflag &= ~(ISIG|ICANON|XCASE|ECHO);
	new.c_oflag &= ~(OPOST);
	new.c_cc[VMIN] = 1;
	new.c_cc[VTIME] = 1;
	ioctl(fd, TCSETA, &new);
}


normalmode(fd)
	int fd;
{
register cd;

	cd = open("/dev/console", 0);
	if (cd<0)
		write(2, "snarfl\n", 7);
	ioctl(cd, TCGETA, &new);
/*
	ioctl(fd, TCGETA, &new);
	new.c_cflag |= CSIZE;
	new.c_cflag &= ~CS8;
	new.c_lflag |= (ISIG|ICANON|XCASE|ECHO);
	new.c_oflag |= OPOST;
*/
	ioctl(fd, TCSETA, &new);
}



restoremode(fd)
	int fd;
{
	ioctl(fd, TCSETA, &old);
}
