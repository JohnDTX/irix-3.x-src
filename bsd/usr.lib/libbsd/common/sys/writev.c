/*
 * BSD compatible writev
 */
#include <sys/types.h>
#include <sys/uio.h>

int
writev(fd, iov, niov)
	int fd;
	struct iovec *iov;
	int niov;
{
	char buf[16384];
	register struct iovec *iv;
	register int len;
	register int nb;
	register int r;
	register int i;
	register char *cp;

	/*
	 * Try to write data out atomically by copying it to a single
	 * buffer.  If we can't, then write the data out in as few
	 * pieces as possible.
	 */
	nb = 0;
	len = 0;
	iv = iov;
	cp = buf;
	for (i = niov; --i >= 0; iv++) {
top:
		if (len + iv->iov_len > sizeof(buf)) {
			if (len) {
				r = write(fd, buf, len);
				if (r < 0)
					return (r);
				nb += r;
				len = 0;
				goto top;
			} else {
				/*
				 * Can't fit this iov in the buffer.
				 * Just write it out...
				 */
				r = write(fd, iv->iov_base, iv->iov_len);
				if (r < 0)
					return (r);
				nb += r;
				continue;
			}
		}
		bcopy(iv->iov_base, cp, iv->iov_len);
		len += iv->iov_len;
		cp += iv->iov_len;
	}
	if (len) {
		r = write(fd, buf, len);
		if (r < 0)
			return (r);
		nb += r;
	}
	return (nb);
}
