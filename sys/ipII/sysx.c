/*
 * SGI system calls
 *
 * $Source: /d2/3.7/src/sys/ipII/RCS/sysx.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:31:02 $
 */

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/proc.h"
#include "../h/buf.h"
#include "../h/inode.h"
#include "../h/reboot.h"
#include "../vm/vm.h"
#include "../h/kversion.h"
#include "../ipII/pte.h"
#include "../ipII/cpureg.h"

#ifdef	GL1
#include "../gl1/shmem.h"
#define	GLVERSION	VERSIONSTRING
#endif
#ifdef	GL2
#undef	imin
#undef	imax
#include <gl2/shmem.h>
#endif
#if defined(KOPT_NOGL) || defined(GL0)
#define	GLVERSION	"no graphics"
#endif

extern	char sgiversion[];

/*
 * Structure defining return values for the various version
 * requests.
 */
char	*versioninfo[] = {
	sgiversion,		/* KVERS_KERNEL */
#ifdef	GL1
	"GL1",			/* KVERS_GLTYPE */
#endif
#ifdef	GL2
	"GL2",
#endif
	GLVERSION,		/* KVERS_GL */
	"ipII",
	"no",			/* KVERS_HAVEFPA */
};

/*
 * tracesys:
 *	- enable/disable event tracing
 */
#ifdef	TRACE
short	traceon;		/* tracing flag */
#endif
tracesys()
{
#ifdef	TRACE
	register struct a {
		int	flag;
	} *uap = (struct a *)u.u_ap;

	if (suser())
		traceon = uap->flag;
#else
	u.u_error = EINVAL;
#endif
}

/*
 * ulimit:
 *	- read/write user limits
 * TODO	- fix #3 to check for more memory than is mappable (not possible now)
 */
ulimit()
{
	register struct a {
		int	cmd;
		long	arg;
	} *uap;
	register n, ts;

	uap = (struct a *)u.u_ap;
	switch(uap->cmd) {
	case 2:
		if (uap->arg > u.u_limit && !suser())
			return;
		u.u_limit = uap->arg;
	case 1:
		u.u_roff = u.u_limit;
		break;

	case 3:
		ts = u.u_tsize;
		n = MAXTDSIZ - u.u_ssize - HIGHPAGES - ts - u.u_procp->p_loadc;
		u.u_roff = (off_t) (u.u_loadaddr + ctob(ts + n) - 1);
		break;
	default:
		u.u_error = EINVAL;
	}
}

/*
 * phys:
 *	- Set up a physical address in user's address space.
 *	- on the ipII, this never works
 */
phys()
{
	u.u_error = EINVAL;
}

/*
 * reboot the system
 */
reboot()
{
	register struct a {
		int	how;
	} *uap = (struct a *)u.u_ap;

	if (!suser())
		return;
	if ((uap->how & RB_NOSYNC) == 0)
		sys_shutdown();
	spl7();
	if (uap->how & RB_HALT) {
		printf("Halting...\n");
		halt();
		/* NOTREACHED */
	}

	doboot();
	/* NOTREACHED */
}

/*
 * getversion:
 *	- system call to get version information from the kernel
 */
getversion()
{
	register struct a {
		u_int	type;
		char	*buf;
	} *uap = (struct a *)u.u_ap;
	register char *cp;

	if (uap->type > KVERS_LAST)
		u.u_error = EINVAL;
	else {
		if ((uap->type == KVERS_HAVEFPA) && havefpa)
			versioninfo[KVERS_HAVEFPA] = "yes";

		cp = versioninfo[uap->type];
		if (copyout(cp, uap->buf, strlen(cp)+1))
			u.u_error = EFAULT;
	}
}

/*
 * System call to allow access to various parts of the multibus i/o or
 * memory space.
 */
sgi_ip2()
{
	register struct a {
		int	how;
	} *uap;

	uap = (struct a *)u.u_ap;
	if (uap->how == 0)
		*MBP_REG &= ~(MBP_HMACC|MBP_LMACC|MBP_HIOACC|MBP_LIOACC);
	else {
		if (uap->how & 1)
			*MBP_REG |= MBP_HIOACC|MBP_LIOACC;
		if (uap->how & 2)
			*MBP_REG |= MBP_HMACC|MBP_LMACC;
	}
}
