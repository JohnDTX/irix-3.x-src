/*
 * 4.2bsd readv emulator.
 *
 * $Source: /d2/3.7/src/bsd/usr.lib/libbsd/common/sys/RCS/readv.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 15:01:38 $
 */
#include <sys/types.h>
#include <sys/uio.h>

/*
 * Read from "fd", scattering the data through the supplied iov's
 */
int
readv(fd, iov, niov)
	register int fd;
	register struct iovec *iov;
	register int niov;
{
	register int nb;
	register int totalByteCount;

	totalByteCount = 0;
	for (; --niov >= 0; iov++) {
		nb = read(fd, iov->iov_base, iov->iov_len);
		if (nb < 0) {
			/* error */
			return (nb);
		}
		totalByteCount += nb;
		if (nb != iov->iov_len) {
			/* truncated read.  stop now */
			break;
		}
	}
	return (totalByteCount);
}
