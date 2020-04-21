/*
 * $Source: /d2/3.7/src/sys/sys/RCS/sig.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:35:31 $
 */
#include "../h/param.h"
#include "../h/systm.h"
#include "../h/user.h"
#include "../h/buf.h"
#include "../h/file.h"
#include "../h/fstyp.h"
#include "../h/inode.h"
#include "../h/nami.h"
#include "../h/proc.h"
#include "../h/seg.h"
#include "../h/text.h"
#include "../vm/vm.h"
#include "machine/psr.h"
#include "machine/reg.h"
#include "machine/cpureg.h"

/*
 * Priority for tracing
 */
#define	IPCPRI	PZERO

/*
 * Tracing variables.
 * Used to pass trace command from
 * parent to child being traced.
 * This data base cannot be
 * shared and is locked
 * per user.
 */
struct ipctrace
{
	int	ip_data;
	int	ip_lock;
	int	ip_req;
	int	*ip_addr;
} ipc;

/*
 * Send the specified signal to
 * all processes with 'pgrp' as
 * process group.
 * Called by tty.c for quits and
 * interrupts.
 */
signal(pgrp, sig)
register pgrp;
{
	register struct proc *p;

	if (pgrp == 0)
		return;
	for(p = &proc[1]; p < procNPROC; p++)
		if (p->p_pgrp == pgrp)
			psignal(p, sig);
}

/*
 * Send the specified signal to
 * the specified process.
 */
psignal(p, sig)
register struct proc *p;
register sig;
{
	int s;

	sig--;
	if (sig < 0 || sig >= NSIG)
		return;
	s = spl6();
	p->p_sig |= 1L<<sig;
	splx(s);
	if (p->p_stat == SSLEEP && p->p_pri > PZERO) {
		if (p->p_pri > PUSER)
			p->p_pri = PUSER;
		setrun(p);
	}
}

/*
 * Returns true if the current
 * process has a signal to process.
 * This is asked at least once
 * each time a process enters the
 * system.
 * A signal does not do anything
 * directly to a process; it sets
 * a flag that asks the process to
 * do something to itself.
 */
issig()
{
	register n;
	register struct proc *p, *q;
	int s;

	p = u.u_procp;
	while(p->p_sig) {
		n = fsig(p);
		if (n == SIGWINCH) {
			if ((u.u_signal[SIGWINCH-1] != 0) &&
			    ((u.u_signal[SIGWINCH-1]&01) == 0)) {
				/* signal is being caught */
				return (n);
			} else {
				/* ignored, or default */
			}
		} else
		if (n == SIGCLD) {
			if (u.u_signal[SIGCLD-1]&01) {
				for (q = &proc[1]; q < procNPROC; q++)
					if (p->p_pid == q->p_ppid &&
					 q->p_stat == SZOMB)
						freeproc(q, 0);
			} else if (u.u_signal[SIGCLD-1])
				return(n);
		} else if (n == SIGPWR) {
			if (u.u_signal[SIGPWR-1] && (u.u_signal[SIGPWR-1]&1)==0)
				return(n);
		} else if ((u.u_signal[n-1]&1) == 0 || (p->p_flag&STRC))
			return(n);
		s = spl6();
		p->p_sig &= ~(1L<<(n-1));
		splx(s);
	}
	return(0);
}

/*
 * Check if this process got a fatal signal while sleeping.
 */
int
isfatalsig(p) 
	register struct proc *p;
{
	register int s;

	s = spl6();
	if ((p->p_flag & STRC) == 0) {
		register int n;
		register unsigned long sig;
		
		for (n = 1, sig = p->p_sig; sig != 0; n++, sig >>= 1) {
			if ((sig & 1) == 0) {
				continue;
			}
			switch (n) {
			  case SIGCLD:	/* non-fatal signals */
			  case SIGPWR:
			  case SIGWINCH:
				continue;
			}
			if (u.u_signal[n-1] == (int)SIG_DFL) {
				splx(s);
				return 1;
			}
		}
	}
	splx(s);
	return 0;
}

/*
 * Enter the tracing STOP state.
 * In this state, the parent is
 * informed and the process is able to
 * receive commands from the parent.
 */
stop()
{
	register struct proc *pp, *cp;

loop:
	cp = u.u_procp;
	if (cp->p_ppid != 1)
	for (pp = &proc[0]; pp < procNPROC; pp++)
		if (pp->p_pid == cp->p_ppid) {
			wakeup((caddr_t)pp);
			cp->p_stat = SSTOP;
			swtch();
			if ((cp->p_flag&STRC)==0 || procxmt())
				return;
			goto loop;
		}
	exit(fsig(u.u_procp));
}

/*
 * Perform the action specified by
 * the current signal.
 * The usual sequence is:
 *	if (issig())
 *		psig();
 */
psig()
{
	register n, p;
	register struct proc *rp;
	int s;

	rp = u.u_procp;
	if (u.u_pcb.pcb_fpinuse && u.u_pcb.pcb_fpsaved==0) {
		fpsave();
		u.u_pcb.pcb_fpsaved = 1;
	}
	if (rp->p_flag&STRC)
		stop();
	n = fsig(rp);
	if (n==0)
		return;
	s = spl6();
	rp->p_sig &= ~(1L<<(n-1));
	splx(s);
	if ((p=u.u_signal[n-1]) != 0) {
		if (p & 1)
			return;
		u.u_error = 0;
		if (n != SIGILL && n != SIGTRAP)
			u.u_signal[n-1] = 0;
		sendsig((caddr_t)p, n);
		return;
	}
	switch(n) {
	case SIGWINCH:
		/* default behaviour is to ignore this signal */
		return;
	case SIGQUIT:
	case SIGILL:
	case SIGTRAP:
	case SIGIOT:
	case SIGEMT:
	case SIGFPE:
	case SIGBUS:
	case SIGSEGV:
	case SIGSYS:
		if (core())
			n += 0200;
	}
	exit(n);
}

/*
 * find the signal in bit-position
 * representation in p_sig.
 */
fsig(p)
struct proc *p;
{
	register short i;
	register long n;

	n = p->p_sig;
	i = NSIG - 1;
	do {
		if (n & 1L)
			return(NSIG - i);
		n >>= 1;
	} while (--i != -1);
	return(0);
}

/*
 * Create a core image on the file "core"
 * If you are looking for protection glitches,
 * there are probably a wealth of them here
 * when this occurs to a suid command.
 *
 * It writes UPAGES block of the
 * user.h area followed by the entire
 * data+stack segments.
 */
core()
{
	struct argnamei nmarg;
	register struct inode *ip;

	if (u.u_uid != u.u_ruid)
		return(0);

	u.u_error = 0;
	u.u_dirp = "core";
	nmarg.cmd = NI_CREAT;
	nmarg.mode = ((IREAD|IWRITE)>>6)|((IREAD|IWRITE)>>3)|(IREAD|IWRITE);
	nmarg.ftype = 0;
	ip = namei(SYSPATH, &nmarg, FOLLOWLINK);
	if (u.u_error)
		return (0);

	if (!FS_ACCESS(ip, IWRITE) && ip->i_ftype == IFREG) {
		FS_OPENI(ip, (nmarg.rcode == FSN_FOUND) ?
		    FTRUNC|FWRITE : FCREAT|FWRITE);
		if (!u.u_error) {
			FS_ITRUNC(ip);
			u.u_limit = (daddr_t)CDLIMIT;
			u.u_fmode = FWRITE;
			u.u_base = (caddr_t)&u;
			u.u_count = ctob(UPAGES);
			u.u_offset = 0;
			u.u_segflg = 1;
			FS_WRITEI(ip);
			if (u.u_error == 0) {
				u.u_base = (caddr_t)ctob(dptov(u.u_procp, 0));
				u.u_count = ctob(u.u_dsize);
				u.u_offset = ctob(UPAGES);
				u.u_segflg = 0;
				FS_WRITEI(ip);
			}
			if (u.u_error == 0) {
				u.u_base = (caddr_t)
				    ctob(sptov(u.u_procp, u.u_ssize - 1));
				u.u_count = ctob(u.u_ssize);
				u.u_offset = ctob(UPAGES+u.u_dsize);
				u.u_segflg = 0;
				FS_WRITEI(ip);
			}
			FS_CLOSEI(ip, FWRITE, 1, (off_t) 0);
		}
	} else
		u.u_error = EACCES;
	iput(ip);
	return (u.u_error == 0);
}

/*
 * sys-trace system call.
 */
ptrace()
{
	register struct ipctrace *ipcp;
	register struct proc *p;
	register struct a {
		int	req;
		int	pid;
		int	*addr;
		int	data;
	} *uap;

	uap = (struct a *)u.u_ap;
	if (uap->req <= 0) {
		u.u_procp->p_flag |= STRC;
		return;
	}
	for (p=proc; p < procNPROC; p++) 
		if (p->p_stat==SSTOP
		 && p->p_pid==uap->pid
		 && p->p_ppid==u.u_procp->p_pid)
			goto found;
	u.u_error = ESRCH;
	return;

found:
	ipcp = &ipc;
	while (ipcp->ip_lock)
		(void) sleep((caddr_t)ipcp, IPCPRI);
	ipcp->ip_lock = p->p_pid;
	ipcp->ip_data = uap->data;
	ipcp->ip_addr = uap->addr;
	ipcp->ip_req = uap->req;
	p->p_flag &= ~SWTED;
	setrun(p);
	while (ipcp->ip_req > 0) {
		(void) sleep((caddr_t)ipcp, IPCPRI);
	}
	u.u_rval1 = ipcp->ip_data;
	if (ipcp->ip_req < 0)
		u.u_error = EIO;
	ipcp->ip_lock = 0;
	wakeup((caddr_t)ipcp);
}

/*
 * Code that the child process
 * executes to implement the command
 * of the parent process in tracing.
 */
#define	PHYSOFF(p, o) \
	((char *)(p)+(int)(o))
procxmt()
{
	register struct ipctrace *ipcp;
	register int i;
	register *p;
	register struct text *xp;
	register int rv;

	ipcp = &ipc;
	if (ipcp->ip_lock != u.u_procp->p_pid)
		return 0;

	rv = 0;
	i = ipcp->ip_req;
	ipcp->ip_req = 0;

	switch (i) {

	/* read user I/D */
	case 1:
	case 2:
		ipcp->ip_data = fuword((caddr_t)ipcp->ip_addr);
		break;

	/* read u */
	case 3:
		i = (int)ipcp->ip_addr;
		if (i<0 || i >= ctob(UPAGES) || (i & 1))
			goto error;
		ipcp->ip_data = *(int *)PHYSOFF(&u, i);
		break;

	/* write user I */
	/* Must set up to allow writing */
	case 4:
		/*
		 * If text, must assure exclusive use
		 */
		if (xp = u.u_procp->p_textp) {
			if (xp->x_count!=1)
				goto error;
			xp->x_iptr->i_flag &= ~ITEXT;
		}
		if ((i = suiword((caddr_t)ipcp->ip_addr, ipcp->ip_data)) < 0) {
			if (chgprot((caddr_t)ipcp->ip_addr, RW) &&
			    chgprot((caddr_t)ipcp->ip_addr+(sizeof(int)-1), RW))
				i = suiword((caddr_t)ipcp->ip_addr,
					    ipcp->ip_data);
			(void) chgprot((caddr_t)ipcp->ip_addr, RO);
			(void) chgprot((caddr_t)ipcp->ip_addr+(sizeof(int)-1),
				       RO);
		}
		if (i<0)
			goto error;
		if (xp)
			xp->x_flag |= XWRIT;
		break;

	/* write user D */
	case 5:
		if (suword((caddr_t)ipcp->ip_addr, 0) < 0)
			goto error;
		(void) suword((caddr_t)ipcp->ip_addr, ipcp->ip_data);
		break;

	/* write u */
	case 6:
		i = (int)ipcp->ip_addr;
		p = (int *)PHYSOFF(&u, i);
		for (i=0; i<17; i++)
			if (p == &u.u_ar0[regloc[i]])
				goto ok;
		if (p == &u.u_ar0[RPS]) {
			/* assure user space and priority 0 */
			ipcp->ip_data &= ~0x2700;
			goto ok;
		}
		goto error;

	ok:
		*p = ipcp->ip_data;
		break;

	/* set signal and continue */
	/* one version causes a trace-trap */
	case 9:
		u.u_ar0[RPS] |= PS_T;
	case 7:
		if ((int)ipcp->ip_addr != 1)
			u.u_ar0[PC] = (int)ipcp->ip_addr;
		(void) spl6();
		u.u_procp->p_sig = 0L;
		(void) spl0();
		if (ipcp->ip_data)
			psignal(u.u_procp, ipcp->ip_data);
		rv = 1;
		break;

	/* force exit */
	case 8:
		wakeup((caddr_t)ipcp);
		exit(fsig(u.u_procp));

	/* read u registers */
	case 10:
		if ((i = (int)ipcp->ip_addr) < 0 || i > 17)
			goto error;
		if (i == 17)
			ipcp->ip_data = u.u_ar0[regloc[17]] & 0xFFFF;
		else
			ipcp->ip_data = u.u_ar0[regloc[i]];
		break;

	/* write u registers */
	case 11:
		if ((i = (int)ipcp->ip_addr) < 0 || i > 17)
			goto error;
		if (i == 17) {
			ipcp->ip_data &= ~0x2700;	/* user only */
			u.u_ar0[regloc[17]] =
				(u.u_ar0[regloc[17]] & ~0xFFFF) |
				(ipcp->ip_data & 0xFFFF);
		} else
			u.u_ar0[regloc[i]] = ipcp->ip_data;
		break;
		
	default:
	error:
		ipcp->ip_req = -1;
	}

	u.u_procp->p_slptime = 0;
	wakeup((caddr_t)ipcp);
	return rv;
}
