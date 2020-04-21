/*
 * Exit 0 if the running kernel supports nfs, non-zero otherwise.
 *
 * $Source: /d2/3.7/src/etc/RCS/havenfs.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 15:37:54 $
 */
#include <signal.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <nfs/nfs.h>

havent_nfs()
{
	exit(-1);
}

main()
{
	fhandle_t fh;

	(void) signal(SIGSYS, havent_nfs);
	if (getfh(0, &fh) < 0) {
		exit(-1);
	}
	exit(0);
}
