/* porting utilities
 *
 * "$Header: /d2/3.7/src/cypress/RCS/sgi_utils.c,v 1.1 89/03/27 15:03:42 root Exp $"
 */


#include <sys/termio.h>
#include <sys/stropts.h>

/*
 * convert an ioctl() kernel call into a streams ioctl()
 */
tty_ioctl(fd, cmd, argp)
int fd;					/* do it to this file descriptor */
int cmd;				/* do this */
char *argp;				/* arguements for ioctl(2) */
{
    struct strioctl buf;

    buf.ic_cmd = cmd;
    buf.ic_timout = -1;
    buf.ic_len = (cmd >> 16) & IOCPARM_MASK;
    return ioctl(fd, I_STR, &buf);
}
