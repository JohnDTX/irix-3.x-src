/* @(#)sysacct.c	1.2 */
#include <sys/types.h>

/*
 * output accounting info
 */
sysacct(bytes, time)
time_t time;
long bytes;
{

	/*
	 * shuts lint up
	 */
	if (time == bytes)
	return;
}
