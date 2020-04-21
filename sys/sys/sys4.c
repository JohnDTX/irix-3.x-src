/*
 * Everything in this file is a routine implementing a system call.
 *
 * $Source: /d2/3.7/src/sys/sys/RCS/sys4.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:35:38 $
 */
#include "../h/param.h"
#include "../h/systm.h"
#include "../h/user.h"
#include "../h/buf.h"
#include "../h/file.h"
#include "../h/fstyp.h"
#include "../h/inode.h"
#include "../h/mount.h"
#include "../h/nami.h"
#include "../h/proc.h"
#include "../h/utsname.h"
#include "../h/printf.h"

gtime()
{
	u.u_rtime = time;
}

stime()
{
	register struct a {
		time_t	time;
	} *uap;

	uap = (struct a *)u.u_ap;
	if (suser()) {
		extern long time_usec;
		extern int time_ticks, timedelta;
		register int s;
		register struct mount *rootmp;

		s = splmax();
		time = uap->time;
		timedelta = 0;
		time_ticks = 0;
		time_usec = 0;
		rtclockset();
		splx(s);
		rootmp = rootdir->i_mntdev;
		FS_UPDATETIME(rootmp);
	}
}

setuid()
{
	register unsigned uid;
	register struct a {
		int	uid;
	} *uap;

	uap = (struct a *)u.u_ap;
	uid = uap->uid;
	if (uid >= MAXUID) {
		u.u_error = EINVAL;
		return;
	}
	if (uid && (uid == u.u_ruid || uid == u.u_procp->p_suid))
		u.u_uid = uid;
	else if (suser()) {
		u.u_uid = uid;
		u.u_procp->p_uid = uid;
		u.u_procp->p_suid = uid;
		u.u_ruid = uid;
	}
}

getuid()
{
	u.u_rval1 = u.u_ruid;
	u.u_rval2 = u.u_uid;
}

setgid()
{
	register unsigned gid;
	register struct a {
		int	gid;
	} *uap;

	uap = (struct a *)u.u_ap;
	gid = uap->gid;
	if (gid >= MAXUID) {
		u.u_error = EINVAL;
		return;
	}
	if (u.u_rgid == gid || suser()) {
		u.u_gid = gid;
		u.u_rgid = gid;
	}
}

getgid()
{
	u.u_rval1 = u.u_rgid;
	u.u_rval2 = u.u_gid;
}

getpid()
{
	u.u_rval1 = u.u_procp->p_pid;
	u.u_rval2 = u.u_procp->p_ppid;
}

setpgrp()
{
	register struct proc *p = u.u_procp;
	register struct a {
		int	flag;
	} *uap;

	uap = (struct a *)u.u_ap;
	if (uap->flag) {
		if (p->p_pgrp != p->p_pid) {
			u.u_ttyp = NULL;
 			u.u_ttyip = NULL;
		}
		p->p_pgrp = p->p_pid;
	}
	u.u_rval1 = p->p_pgrp;
}

sync()
{
	update();
}

nice()
{
	register n;
	register struct a {
		int	niceness;
	} *uap;

	uap = (struct a *)u.u_ap;
	n = uap->niceness;
	if ((n < 0 || n > 2*NZERO) && !suser())
		n = 0;
	n += u.u_procp->p_nice;
	if (n >= 2*NZERO)
		n = 2*NZERO -1;
	if (n < 0)
		n = 0;
	u.u_procp->p_nice = n;
	u.u_rval1 = n - NZERO;
}

/*
 * Unlink system call.
 */
unlink()
{
	struct a {
		char	*fname;
	};
	struct argnamei nmarg;

	nmarg.cmd = NI_DEL;
	(void) namei(USERPATH, &nmarg, DONTFOLLOW);
}

chdir()
{
	chdirec(&u.u_cdir);
}

chroot()
{
	if (!suser())
		return;
	chdirec(&u.u_rdir);
}

chdirec(ipp)
register struct inode **ipp;
{
	register struct inode *ip;
	struct a {
		char	*fname;
	};
	struct argnotify noarg;
	register flag;

	ip = namei(USERPATH, NI_LOOKUP, FOLLOWLINK);
	if (ip == NULL)
		return;
	if (ip->i_ftype != IFDIR) {
		u.u_error = ENOTDIR;
		goto bad;
	}
	if (FS_ACCESS(ip, IEXEC))
		goto bad;
	flag = (ipp == &u.u_rdir) ? NO_CHROOT : NO_CHDIR;
	if (fsinfo[ip->i_fstyp].fs_notify & flag) {
		noarg.cmd = flag;
		FS_NOTIFY(ip, &noarg);
	}
	if (u.u_error)
		goto bad;
	iunlock(ip);
	if (*ipp) {
		iunuse(*ipp);
	}
	*ipp = ip;
	return;

bad:
	iput(ip);
}

chmod()
{
	register struct inode *ip;
	register struct a {
		char	*fname;
		int	fmode;
	} *uap;
	struct argnamei nmarg;

	uap = (struct a *)u.u_ap;
	if ((ip = owner()) == NULL)
		return;
	nmarg.cmd = NI_CHMOD;
	nmarg.mode = uap->fmode;
	if (! FS_SETATTR(ip, &nmarg)) {
		iput(ip);
		return;
	}
	ip->i_flag |= ICHG;
	iput(ip);
}

chown()
{
	register struct inode *ip;
	register struct a {
		char	*fname;
		int	uid;
		int	gid;
	} *uap;
	struct argnamei nmarg;

	uap = (struct a *)u.u_ap;
	if ((ip = owner()) == NULL)
		return;
	nmarg.cmd = NI_CHOWN;
	nmarg.uid = (ushort)uap->uid;
	nmarg.gid = (ushort)uap->gid;
	if (! FS_SETATTR(ip, &nmarg)) {
		iput(ip);
		return;
	}
	ip->i_flag |= ICHG;
	iput(ip);
}

ssig()
{
	register a;
	register struct proc *p;
	struct a {
		int	signo;
		int	fun;
	} *uap;

	uap = (struct a *)u.u_ap;
	a = uap->signo;
	if (a <= 0 || a > NSIG || a == SIGKILL) {
		u.u_error = EINVAL;
		return;
	}
	u.u_rval1 = u.u_signal[a-1];
	u.u_signal[a-1] = uap->fun;
	(void) spl6();
	u.u_procp->p_sig &= ~(1L<<(a-1));
	(void) spl0();
	if (a == SIGCLD) {
		a = u.u_procp->p_pid;
		for (p = &proc[1]; p < procNPROC; p++) {
			if ((a == p->p_ppid) && (p->p_stat == SZOMB))
				psignal(u.u_procp, SIGCLD);
		}
	}
}

kill()
{
	register struct proc *p, *q;
	register arg;
	register struct a {
		int	pid;
		int	signo;
	} *uap;
	int f;

	uap = (struct a *)u.u_ap;
	if (uap->signo < 0 || uap->signo > NSIG) {
		u.u_error = EINVAL;
		return;
	}
	/* Prevent proc 1 (init) from being SIGKILLed */
	if (uap->signo == SIGKILL && uap->pid == 1) {
		u.u_error = EINVAL;
		return;
	}
	f = 0;
	arg = uap->pid;
	if (arg > 0)
		p = &proc[1];
	else
		p = &proc[2];
	q = u.u_procp;
	if (arg == 0 && q->p_pgrp == 0) {
		u.u_error = ESRCH;
		return;
	}
	for(; p < procNPROC; p++) {
		if (p->p_stat == NULL)
			continue;
		if (arg > 0 && p->p_pid != arg)
			continue;
		if (arg == 0 && p->p_pgrp != q->p_pgrp)
			continue;
		if (arg < -1 && p->p_pgrp != -arg)
			continue;
		if (! (u.u_uid == 0 ||
			u.u_uid == p->p_uid ||
			u.u_ruid == p->p_uid ||
			u.u_uid == p->p_suid ||
			u.u_ruid == p->p_suid ))
			if (arg > 0) {
				u.u_error = EPERM;
				return;
			} else
				continue;
		f++;
		if (uap->signo)
			psignal(p, uap->signo);
		if (arg > 0)
			break;
	}
	if (f == 0)
		u.u_error = ESRCH;
}

times()
{
	register struct a {
		time_t	(*times)[4];
	} *uap;
	time_t loctime[4];

	uap = (struct a *)u.u_ap;
#if notdef
	if (hz==60) {
		if (copyout((caddr_t)&u.u_utime, (caddr_t)uap->times,
			    sizeof(*uap->times)))
			u.u_error = EFAULT;
		spl7();
		u.u_rtime = lbolt;
		spl0();
	} else {
		loctime[0] = u.u_utime * 60 / hz;
		loctime[1] = u.u_stime * 60 / hz;
		loctime[2] = u.u_cutime * 60 / hz;
		loctime[3] = u.u_cstime * 60 / hz;
		if (copyout((caddr_t)&loctime[0], (caddr_t)uap->times,
			    sizeof(loctime)))
			u.u_error = EFAULT;
		spl7();
		u.u_rtime = lbolt*60/hz;
		spl0();
	}
#else
#if HZ == 60
	if (copyout((caddr_t)&u.u_utime, (caddr_t)uap->times,
		    sizeof(*uap->times)))
		u.u_error = EFAULT;
	u.u_rtime = lbolt;
#else
	loctime[0] = (u.u_utime * 60) / HZ;
	loctime[1] = (u.u_stime * 60) / HZ;
	loctime[2] = (u.u_cutime * 60) / HZ;
	loctime[3] = (u.u_cstime * 60) / HZ;
	if (copyout((caddr_t)&loctime[0], (caddr_t)uap->times,
		    sizeof(loctime)))
		u.u_error = EFAULT;
	u.u_rtime = (lbolt*60)/HZ;
#endif
#endif
}

profil()
{
	register struct a {
		short	*bufbase;
		unsigned bufsize;
		unsigned pcoffset;
		unsigned pcscale;
	} *uap;

	uap = (struct a *)u.u_ap;
	u.u_prof.pr_base = uap->bufbase;
	u.u_prof.pr_size = uap->bufsize;
	u.u_prof.pr_off = uap->pcoffset;
	u.u_prof.pr_scale = uap->pcscale;
}

/*
 * alarm clock signal
 */
alarm()
{
	register struct proc *p;
	register c;
	register struct a {
		int	deltat;
	} *uap;

	uap = (struct a *)u.u_ap;
	p = u.u_procp;
	c = p->p_clktim;
	p->p_clktim = uap->deltat;
	u.u_rval1 = c;
}

/*
 * indefinite wait.
 * no one should wakeup(&u)
 */
pause()
{
	for(;;)
		(void) sleep((caddr_t)&u, PSLEP);
}

/*
 * mode mask for creation of files
 */
umask()
{
	register struct a {
		int	mask;
	} *uap;
	register t;

	uap = (struct a *)u.u_ap;
	t = u.u_cmask;
	u.u_cmask = uap->mask & 0777;
	u.u_rval1 = t;
}

/*
 * Set IUPD and IACC times on file.
 */
utime()
{
	register struct a {
		char	*fname;
		time_t	*tptr;
	} *uap;
	register struct inode *ip;
	time_t tv[2];

	uap = (struct a *)u.u_ap;
	if (uap->tptr != NULL) {
		if (copyin((caddr_t)uap->tptr, (caddr_t)tv, sizeof(tv))) {
			u.u_error = EFAULT;
			return;
		}
	} else {
		tv[0] = time;
		tv[1] = time;
	}
	ip = namei(USERPATH, NI_LOOKUP, FOLLOWLINK);
	if (ip == NULL)
		return;
	if (u.u_uid != ip->i_uid && u.u_uid != 0) {
		if (uap->tptr != NULL)
			u.u_error = EPERM;
		else
			(void) FS_ACCESS(ip, IWRITE);
	}
	if (!u.u_error) {
		ip->i_flag |= IACC|IUPD|ICHG;
		FS_IUPDAT(ip, &tv[0], &tv[1]);
	}
	iput(ip);
}

gethostname()
{
	register struct a {
		char	*hostname;
		int	len;
	} *uap = (struct a *)u.u_ap;
	register u_int len;

	len = uap->len;
	if (len > hostnamelen + 1)
		len = hostnamelen + 1;
	if (copyout((caddr_t)hostname, (caddr_t)uap->hostname, (int)len))
		u.u_error = EFAULT;
}

sethostname()
{
	register struct a {
		char	*hostname;
		u_int	len;
	} *uap = (struct a *)u.u_ap;

	if (!suser())
		return;
	if (uap->len > MAXHOSTNAMELEN) {
		u.u_error = EINVAL;
		return;
	}
	hostnamelen = uap->len;
	if (copyin((caddr_t)uap->hostname, hostname, (int)uap->len))
		u.u_error = EFAULT;
	else {
		hostname[hostnamelen] = 0;
		strncpy(utsname.sysname, hostname, sizeof(utsname.sysname)-1);
		strncpy(utsname.nodename, hostname, sizeof(utsname.nodename)-1);
		utsname.sysname[sizeof(utsname.sysname)-1] = 0;
		utsname.nodename[sizeof(utsname.nodename)-1] = 0;
	}
}

getdomainname()
{
	register struct a {
		char	*domainname;
		int	len;
	} *uap = (struct a *)u.u_ap;
	register u_int len;

	len = uap->len;
	if (len > domainnamelen + 1)
		len = domainnamelen + 1;
	if (copyout((caddr_t)domainname,(caddr_t)uap->domainname,len))
		u.u_error = EFAULT;
}

setdomainname()
{
	register struct a {
		char	*domainname;
		u_int	len;
	} *uap = (struct a *)u.u_ap;

	if (!suser())
		return;
	if (uap->len > MAXHOSTNAMELEN) {
		u.u_error = EINVAL;
		return;
	}
	domainnamelen = uap->len;
	if (copyin((caddr_t)uap->domainname, domainname, uap->len))
		u.u_error = EFAULT;
	domainname[domainnamelen] = 0;
}

/*
 * sys_shutdown:
 *	- shut the system down by:
 *		- killing all running processes
 *		- unmounting all the filesystems
 *		- syncing out the buffer cache
 */
char	shuttingdown;

sys_shutdown()
{
	register struct proc *p;
	register struct mount *mp;
	register int iter;
	int sentsig;

	/*
	 * Start shutdown procedures.  First set shuttingdown so that fork
	 * and exec will fail.
	 */
	shuttingdown++;
	setConsole(CONSOLE_NOT_ON_PTY);
	resetConsole();

	/*
	 * Next, we scan through the process table and give SIGKILL's
	 * to anything that's alive.
	 */
	for (iter = 0; iter < 10; iter++) {
		sentsig = 0;
		for (p = proc; p < procNPROC; p++) {
			/*
			 * Skip the current process if the slot is unused, or
			 * its a system process, or its the running process.
			 */
			if ((p->p_stat == 0) || (p->p_flag & SSYS) ||
			    (p == u.u_procp))
				continue;
			switch (p->p_stat) {
			  case SZOMB:		/* ignore zombies */
			  case SWAIT:		/* don't have these */
			  case SIDL:		/* can be ignored */
			  case SSTOP:		/* can be ignored */
				continue;
			  case SSLEEP:
				/*
				 * If process was sleeping, assure that it
				 * will be destroyed by the signal below.
				 * This is guaranteed to work, even if the
				 * process was in the kernel doing something
				 * tricky.  It will not guarantee a clean
				 * kernel state, but since we are rebooting
				 * this isn't very important.
				 */
				p->p_pri = PZERO + 1;
				/* FALL THROUGH */
			  case SRUN:
				psignal(p, SIGKILL);
				sentsig++;
				break;
			}
		}
		if (!sentsig)
			break;
		delay(hz >> 1);
	}
	if (sentsig) {
		update();
		bdwait();
	}

	/*
	 * Update filesystem state.  Close all open files, then
	 * update the time in the superblock, then unmount all
	 * filesystems (including root).
	 */
	closeall();
	mp = rootdir->i_mntdev;
	FS_UPDATETIME(mp);
	umountall();

	/*
	 * Force everything (including root bufs) out because umount may
	 * fail for non-root filesystems, and will always fail for root.
	 */
	update();
	bdwait();
}

sginap()
{
	register struct a {
		long	ticks;
	} *uap;
	int s;
	int id;
	extern int wakeup();
	
	uap = (struct a *)u.u_ap;
	s = spl6();
	if (uap->ticks) {
		id = timeout(wakeup, &u.u_procp->p_time, uap->ticks);
		if (sleep(&u.u_procp->p_time, PSLEP|PCATCH)) {
			/*
			 * Somebody wants to interrupt us...allow it.
			 */
			untimeout_id(id);
		}
	} else {
		/*
		 * Zero ticks means a voluntary reschedule
		 */
		setrq(u.u_procp);
		swtch();
	}
	splx(s);
}
