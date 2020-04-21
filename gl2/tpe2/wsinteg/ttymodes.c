/*
 * ttymodes.c - switch tty modes between cooked and raw
 *
 * Same as routines in libxns.a, except TCSETAF is used instead of
 * TCSETA to prevent garbled output, and test for saved in restoremode().
 */

#include <termio.h>

static struct termio    old;
static struct termio    new;
static short    saved;

rawmode(fd)
int     fd;
{
    if (saved == 0) {
	ioctl(fd, TCGETA, &old);
	saved = 1;
    }
    ioctl(fd, TCGETA, &new);
    new.c_cflag &= ~(CSIZE);
    new.c_cflag |= CS8;
    new.c_iflag &= ~(-1);
    new.c_lflag &= ~(ISIG | ICANON | XCASE | ECHO);
    new.c_oflag &= ~(OPOST);
    new.c_cc[VMIN] = 1;
    new.c_cc[VTIME] = 1;
    ioctl(fd, TCSETAF, &new);
}


restoremode(fd)
int     fd;
{
    if (saved)
	ioctl(fd, TCSETAF, &old);
}
