/*
 * Get host identifier from system.  Host identifier is a 64 byte tag
 * which uniquely identifies this particular hosts hardware from any
 * other piece of hardware.
 */
int
gethostident(hostident)
	char *hostident;
{
	int fd;

	if ((fd = open("/dev/null", 0)) < 0)
		return (-1);
	if (ioctl(fd, 1, hostident) < 0) {
		close(fd);
		return (-1);
	}
	close(fd);
	return (0);
}
