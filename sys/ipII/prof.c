/*
 * $Source: /d2/3.7/src/sys/ipII/RCS/prof.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:30:57 $
 */
/*
 * Kernel profiling support
 *
 * To profile some activity of the kernel, the user issues a kprof(1) system
 * call to enable profiling.  Once the testing is completed, the user
 * issues a kprof(0) to stop profiling.  After that, the user can read the
 * buffer out of the kernel's virtual memory using nlist() and /dev/kmem...
 *
 * Turning profiling on when it is already on, turns profiling off, clears
 * out the time counters and procedure count buffer, then turns profiling
 * back on.
 *
 * Written by: Kipp Hickman
 */

#include "prof.h"
#if NPROF > 0

#include "../h/param.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/kprof.h"

#include "../ipII/pte.h"

struct	mcount *mcountbuf = mcbuf;
long	profticks;

/*
 * kprof:
 *	- system call to enable/disable kernel profiling
 */
kprof()
{
	struct a {
		int	state;
	};

	if (!suser())
		return;
	if (((struct a *)u.u_ap)->state) {
		if (profiling) {
			profiling = 0;
			clearprof();
		}
		profiling = 1;
	} else
		profiling = 0;
}

/*
 * kprofgather:
 *	- system call to gather up profile data
 */
kprofgather()
{
	struct a {
		caddr_t	countbuf;
		long	countbufsize;
		caddr_t	timebuf;
		long	timebufsize;
	} *uap = (struct a *)u.u_ap;

	if ((uap->countbufsize < sizeof(mcbuf)) ||
	    (uap->timebufsize < PROFSIZE))
		u.u_error = E2BIG;
	else {
		if (copyout((caddr_t)mcbuf, uap->countbuf, sizeof(mcbuf)) ||
		    copyout((caddr_t)profbuf, uap->timebuf, PROFSIZE))
			u.u_error = EFAULT;
	}
}

/*
 * clearprof:
 *	- clear out profiling information
 */
clearprof()
{
	register int s;
	register struct mcount *mp;

	s = spl7();
	mp = mcbuf;
	for (mp = mcbuf; mp < mcountbuf; mp++)
		mp->mc_count = 0;
	bzero(profbuf, PROFSIZE);
	splx(s);
}

#endif
/*
 * Kernel profiling support
 *
 * To profile some activity of the kernel, the user issues a kprof(1) system
 * call to enable profiling.  Once the testing is completed, the user
 * issues a kprof(0) to stop profiling.  After that, the user can read the
 * buffer out of the kernel's virtual memory using nlist() and /dev/kmem...
 *
 * Turning profiling on when it is already on, turns profiling off, clears
 * out the time counters and procedure count buffer, then turns profiling
 * back on.
 *
 * Written by: Kipp Hickman
 */

#include "prof.h"
#if NPROF > 0

#include "../h/param.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/kprof.h"

#include "../machine/pte.h"

struct	mcount *mcountbuf = mcbuf;
long	profticks;

/*
 * kprof:
 *	- system call to enable/disable kernel profiling
 */
kprof()
{
	struct a {
		int	state;
	};

	if (!suser())
		return;
	if (((struct a *)u.u_ap)->state) {
		if (profiling) {
			profiling = 0;
			clearprof();
		}
		profiling = 1;
	} else
		profiling = 0;
}

/*
 * kprofgather:
 *	- system call to gather up profile data
 */
kprofgather()
{
	struct a {
		caddr_t	countbuf;
		long	countbufsize;
		caddr_t	timebuf;
		long	timebufsize;
	} *uap = (struct a *)u.u_ap;

	if ((uap->countbufsize < sizeof(mcbuf)) ||
	    (uap->timebufsize < PROFSIZE))
		u.u_error = E2BIG;
	else {
		if (bigcopyout((caddr_t)mcbuf, uap->countbuf, sizeof(mcbuf)))
			u.u_error = EFAULT;
		else
		if (bigcopyout((caddr_t)profbuf, uap->timebuf, PROFSIZE))
			u.u_error = EFAULT;
	}
}

/*
 * bigcopyout:
 *	- perform a larger than pagesize copyout
 */
bigcopyout(from, to, count)
	register caddr_t from;
	register caddr_t to;
	register int count;
{
	register int amount;

	while (count) {
		amount = MIN(NBPG, count);
		if (copyout(from, to, amount))
			return -1;
		count -= amount;
		from += NBPG;
		to += NBPG;
	}
	return 0;
}

/*
 * clearprof:
 *	- clear out profiling information
 */
clearprof()
{
	register int s;
	register struct mcount *mp;

	s = spl7();
	mp = mcbuf;
	for (mp = mcbuf; mp < mcountbuf; mp++)
		mp->mc_count = 0;
	bzero(profbuf, PROFSIZE);
	splx(s);
}

#endif
