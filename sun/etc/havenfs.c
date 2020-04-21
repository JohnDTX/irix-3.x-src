/*
 * Exit 0 if the running kernel supports nfs, non-zero otherwise.
 *
 * $Source: /d2/3.7/src/sun/etc/RCS/havenfs.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:21:16 $
 */
#include <signal.h>
#include <sys/types.h>
#include <sys/errno.h>
#if defined mips
# include <sys/fs/nfs.h>
#else
# include <nfs/nfs.h>
#endif

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
