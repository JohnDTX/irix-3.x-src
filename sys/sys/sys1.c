/*
 * $Source: /d2/3.7/src/sys/sys/RCS/sys1.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:35:35 $
 */
#include "../h/param.h"
#include "../h/systm.h"
#include "../h/user.h"
#include "../h/acct.h"
#include "../h/buf.h"
#include "../h/file.h"
#include "../h/fstyp.h"
#include "../h/inode.h"
#include "../h/map.h"
#include "../h/nami.h"
#include "../h/proc.h"
#include "../h/seg.h"
#include "../h/text.h"
#include "../h/sysinfo.h"
#include "../vm/vm.h"
#include "machine/reg.h"
#include "machine/pte.h"
#include "machine/psr.h"

/*
 * exec system call, with and without environments.
 */
struct execa {
	char	*fname;
	char	**argp;
	char	**envp;
};

exec()
{
	((struct execa *)u.u_ap)->envp = NULL;
	exece();
}

exece()
{
	register char *cp;
	register struct buf *bp;
	register struct execa *uap;
	register int nc;
	register int ap, na;
	register int ne, ucp;
	int indir, uid, gid;
	char *sharg;
	struct inode *ip;
	swblk_t bno;
	char cfname[DIRSIZ + 1];
	char cfarg[SHSIZE];
	int hitend;
	int amount;
	int leftover;
	union {
		struct exdata	Ux_A;
		char		ux_shell[ SHSIZE ];
	} u_exdata, save;
	extern char shuttingdown;

	/*
	 * If system is going down, don't allow exec's
	 */
	if (shuttingdown) {
		u.u_error = EBUSY;
		return;
	}

	sysinfo.sysexec++;
	ip = namei(USERPATH, NI_LOOKUP, FOLLOWLINK);
	if (ip == NULL)
		return;
	bno = 0;
	bp = 0;
	indir = 0;
	uid = u.u_uid;
	gid = u.u_gid;
	if (FS_ACCESS(ip, ISUID) == 0)
		uid = ip->i_uid;
	if (FS_ACCESS(ip, ISGID) == 0)
		gid = ip->i_gid;

  again:
	if (FS_ACCESS(ip, IOBJEXEC)
	    || (u.u_procp->p_flag&STRC) && FS_ACCESS(ip, IREAD))
		goto bad;

	/*
	 * Read in first few bytes of file for segment sizes, ux_mag:
	 *	407 = plain executable
	 *	410 = RO text
	 *	413 = demand paged RO text
	 * Also an ASCII line beginning with #! is
	 * the file name of a ``shell'' and arguments may be prepended
	 * to the argument list if given here.
	 *
	 * SHELL NAMES ARE LIMITED IN LENGTH.
	 *
	 * ONLY ONE ARGUMENT MAY BE PASSED TO THE SHELL FROM
	 * THE ASCII LINE.
	 */
	u_exdata.ux_shell[0] = 0;		/* for zero length files */
	u.u_base = (caddr_t)&u_exdata;
	u.u_count = sizeof(u_exdata);
	u.u_offset = 0;
	u.u_segflg = 1;
	FS_READI(ip);
	if (u.u_error)
		goto bad;
#ifndef lint
	if (u.u_count > sizeof(u_exdata) - sizeof(u_exdata.Ux_A) &&
	    u_exdata.ux_shell[0] != '#') {
		u.u_error = ENOEXEC;
		goto bad;
	}
#endif
	switch (u_exdata.ux_mag) {

	case 0407:
		u_exdata.ux_dsize += u_exdata.ux_tsize;
		u_exdata.ux_tsize = 0;
		break;

	case 0413:
		/*
		 * Make sure text and data sizes are page aligned so that the
		 * fill on demand from inode code doesn't have to be as smart
		 */
		if ((u_exdata.ux_tsize & (NBPG - 1)) ||
		    (u_exdata.ux_dsize & (NBPG - 1))) {
			u.u_error = ENOEXEC;
			goto bad;
		}
		/* FALL THROUGH */
	case 0410:
		if (u_exdata.ux_tsize == 0) {
			u.u_error = ENOEXEC;
			goto bad;
		}
		break;

	default:
		if (u_exdata.ux_shell[0] != '#' ||
		    u_exdata.ux_shell[1] != '!' ||
		    indir) {
			u.u_error = ENOEXEC;
			goto bad;
		}
		cp = &u_exdata.ux_shell[2];		/* skip "#!" */
		while (cp < &u_exdata.ux_shell[SHSIZE]) {
			if (*cp == '\t')
				*cp = ' ';
			else if (*cp == '\n') {
				*cp = '\0';
				break;
			}
			cp++;
		}
		if (*cp != '\0') {
			u.u_error = ENOEXEC;
			goto bad;
		}
		cp = &u_exdata.ux_shell[2];
		while (*cp == ' ')
			cp++;
		u.u_dirp = cp;
		while (*cp && *cp != ' ')
			cp++;
		sharg = NULL;
		if (*cp) {
			*cp++ = '\0';
			while (*cp == ' ')
				cp++;
			if (*cp) {
				bcopy((caddr_t)cp, (caddr_t)cfarg, SHSIZE);
				sharg = cfarg;
			}
		}
		bcopy((caddr_t)u.u_dent.d_name, (caddr_t)cfname, DIRSIZ);
		cfname[DIRSIZ] = 0;
		indir = 1;
		iput(ip);
		ip = namei(SYSPATH, NI_LOOKUP, FOLLOWLINK);
		if (ip == NULL)
			return;
		goto again;
	}

	/* make sure entry point is reasonable (click aligned too) */
	if ((u_exdata.ux_entloc < USRTEXT) ||
	    (u_exdata.ux_entloc > 0x2000) ||
	    (u_exdata.ux_entloc & (ctob(1) - 1))) {
		u.u_error = ENOEXEC;
		goto bad;
	}

	/* if using fpa, make sure we have one */
	if (!havefpa && (u_exdata.ux_entloc == 0x2000)) {
		u.u_error = ENOFPA;
		goto bad;
	}

	/*
	 * Collect arguments on "file" in swap space.
	 */
	na = 0;
	ne = 0;
	nc = 0;
	uap = (struct execa *)u.u_ap;
	if ((bno = rmalloc(argmap, (long)ctod((int)btoc(NCARGS)))) == 0) {
		swkill(u.u_procp, "exece");
		goto bad;
	}

	if (uap->argp) for (;;) {
		ap = NULL;
		if (indir && (na == 1 || na == 2 && sharg))
			ap = (int)uap->fname;
		else if (uap->argp) {
			ap = fuword((caddr_t)uap->argp);
			uap->argp++;
		}
		if (ap==NULL && uap->envp) {
			uap->argp = NULL;
			if ((ap = fuword((caddr_t)uap->envp)) == NULL)
				break;
			uap->envp++;
			ne++;
		}
		if (ap == NULL)
			break;
		na++;
		if (ap == -1)
			u.u_error = EFAULT;

	    /* copy users string into the buffer */
		hitend = 0;
		do {
			if (nc >= NCARGS-1) {
				u.u_error = E2BIG;
oops:
				if (bp)
					brelse(bp);
				bp = 0;
				goto badarg;
			}
			leftover = FsBSIZE(swapdev) - (nc & FsBMASK(swapdev));
			if ((nc & FsBMASK(swapdev)) == 0) {
			    if (bp)
				bdwrite(bp);
			    /* XXX change this to just use vcache directly */
			    bp = getblk(swapdev,
					FsLTOP(swapdev,
					       bno + (nc>>FsBSHIFT(swapdev))),
					BTOBB(FsBSIZE(swapdev)));
			    cp = bp->b_un.b_addr;
			}
			if (indir && na == 2 && sharg != NULL) {
				if ((amount = fsstring(sharg, cp,
							      leftover)) < 0) {
					hitend = 1;
					amount = -amount;
				} else
					sharg += amount;
			} else {
				if ((amount = fustring((caddr_t)ap, cp,
						       leftover)) < 0) {
					hitend = 1;
					amount = -amount;
				} else
					ap += amount;
			}
			if ((amount == 0) || u.u_error)
				goto oops;
			cp += amount;
			nc += amount;
		} while (!hitend);
	}

	save.Ux_A = u.u_exdata.Ux_A;
	u.u_exdata.Ux_A = u_exdata.Ux_A;

	if (bp)
		bdwrite(bp);
	bp = 0;
	nc = (nc + NBPW-1) & ~(NBPW-1);
	if (indir)
		bcopy((caddr_t)cfname, (caddr_t)u.u_dent.d_name, DIRSIZ);
	getxfile(ip, nc + (na+4)*NBPW, uid, gid);
	if (u.u_error) {
		u.u_exdata.Ux_A = save.Ux_A;
badarg:
/* XXX i/o cancel code was here... */
		goto bad;
	}

	/*
	 * copy back arglist
	 */
	ucp = USRSTACK - nc - NBPW;
	ap = ucp - na*NBPW - 3*NBPW;
	u.u_ar0[SP] = ap;
	(void) suword((caddr_t)ap, na-ne);
	nc = 0;
	for (;;) {
		ap += NBPW;
		if (na==ne) {
			(void) suword((caddr_t)ap, 0);
			ap += NBPW;
		}
		if (--na < 0)
			break;
		(void) suword((caddr_t)ap, ucp);

	    /* copy back one string */
		hitend = 0;
		do {
			leftover = FsBSIZE(swapdev) - (nc & FsBMASK(swapdev));
			if ((nc & FsBMASK(swapdev)) == 0) {
			    if (bp)
				brelse(bp);
			    bp = bread(swapdev,
				       FsLTOP(swapdev,
					      bno + (nc>>FsBSHIFT(swapdev))),
				       BTOBB(FsBSIZE(swapdev)));
			    bp->b_flags |= B_AGE;	/* throw away */
			    bp->b_flags &= ~B_DELWRI;	/* cancel io */
			    cp = bp->b_un.b_addr;
			}
			if ((amount = sustring(cp, (caddr_t)ucp,
						   leftover)) < 0) {
				hitend = 1;
				amount = -amount;
			}
			ucp += amount;
			cp += amount;
			nc += amount;
		} while (!hitend);
	}
	(void) suword((caddr_t)ap, 0);
	setregs();
bad:
	if (bp)
		brelse(bp);
	if (bno)
		rmfree(argmap, (long)ctod((int) btoc(NCARGS)), bno);
	iput(ip);
}

/*
 * Read in and set up memory for executed file.
 */
getxfile(ip, nargc, uid, gid)
	register struct inode *ip;
	int nargc, uid, gid;
{
	register size_t ts, ds, ss;
	register struct proc *p;
	long oldloadaddr;
	long hadshmem;

	if (u.u_exdata.ux_tsize!=0 && (ip->i_flag&ITEXT)==0 &&
	    ip->i_count!=1) {
		register struct file *fp;

		for (fp = file; fp < fileNFILE; fp++) {
			if ((fp->f_count > 0) &&
			    (fp->f_inode == ip) &&
			    (fp->f_flag & FWRITE)) {
				u.u_error = ETXTBSY;
				return;
			}
		}
	}

	/*
	 * Compute text and data sizes and make sure not too large.
	 */
	ts = btoc(u.u_exdata.ux_tsize);
	ds = btoc(u.u_exdata.ux_dsize+u.u_exdata.ux_bsize);
	if (nargc)
		ss = btoc(nargc);
	else
		ss = 1;		/* make sure its > 0 (for vtopte to work) */
	p = u.u_procp;
	oldloadaddr = u.u_loadaddr;
	u.u_loadaddr = u.u_exdata.ux_entloc;
	p->p_loadc = btop(u.u_loadaddr);
	hadshmem = p->p_flag & SSHMEM;
	p->p_flag &= ~SSHMEM;
	if (chksize((unsigned)ts, (unsigned)ds, (unsigned)ss)) {
		u.u_loadaddr = oldloadaddr;
		p->p_loadc = btoc(u.u_loadaddr);
		p->p_flag |= hadshmem;
		return;
	}
	p->p_flag |= hadshmem;

	/*
	 * Make sure enough space to start process.
	 */
	bzero((caddr_t)&u.u_cdmap, sizeof(struct dmap));
	bzero((caddr_t)&u.u_csmap, sizeof(struct dmap));
	if (swpexpand(ds, ss, &u.u_cdmap, &u.u_csmap) == NULL) {
		u.u_loadaddr = oldloadaddr;
		p->p_loadc = btoc(u.u_loadaddr);
		return;
	}

	/*
	 * At this point, committed to the new image!
	 * Release virtual memory resources of old process, and
	 * initialize the virtual memory of the new process.
	 */
	shmexec();
	cxput(p, 0);
	u.u_loadaddr = oldloadaddr;
	p->p_loadc = btoc(u.u_loadaddr);
	vrelvm();
	u.u_loadaddr = u.u_exdata.ux_entloc;
	p->p_loadc = btoc(u.u_loadaddr);
	bcopy((caddr_t)&u.u_cdmap, (caddr_t)&u.u_dmap, sizeof(u.u_dmap));
	bcopy((caddr_t)&u.u_csmap, (caddr_t)&u.u_smap, sizeof(u.u_smap));

	u.u_procp->p_flag &= ~(SSEQL|SUANOM);
	vgetvm(ts, ds, ss);
	if (ts == 0) {
		/*
		 * Program is impure (0407). Read it in now.
		 * XXX we can probably demand load this too
		 */
		u.u_base = (caddr_t)ctob(dptov(p, 0));
		u.u_count = (int)u.u_exdata.ux_dsize;
		u.u_offset = (int)(sizeof(u.u_exdata) + u.u_exdata.ux_tsize);
		u.u_segflg = 0;
		FS_READI(ip);
	}
	xalloc(ip);
	if (u.u_error)
		swkill(u.u_procp, "i/o error mapping pages");

	/*
	 * set SUID/SGID protections, if no tracing
	 */
	if ((p->p_flag&STRC)==0) {
		u.u_uid = uid;
		p->p_uid = uid;
		u.u_gid = gid;
	} else
		psignal(p, SIGTRAP);
	u.u_tsize = ts;
	u.u_dsize = ds;
	u.u_ssize = ss;
	u.u_prof.pr_scale = 0;

	/* setup floating point if sky board present and user is using it */
	if (havefpa && (p->p_loadc == 2)) {
		u.u_pcb.pcb_fpinuse = 1;
		fpinit();
		fpsave();
		u.u_pcb.pcb_fpsaved = 1;
	}
}

/*
 * exit system call:
 * pass back caller's arg
 */
rexit()
{
	register struct a {
		int	rval;
	} *uap;

	uap = (struct a *)u.u_ap;
	exit((uap->rval & 0377) << 8);
}

/*
 * Release resources.
 * Enter zombie state.
 * Wake up parent and init processes,
 * and dispose of children.
 */
exit(rv)
{
	register int i;
	register struct proc *p, *q;
	extern char shuttingdown;

	p = u.u_procp;
	if ((p->p_pid == 1) && !shuttingdown)
		panic("init died!");
	p->p_flag &= ~(STRC);
	p->p_flag |= SWEXIT;
	p->p_clktim = 0;
	p->p_cpticks = 0;
	p->p_pctcpu = 0;
	for (i=0; i<NSIG; i++)
		u.u_signal[i] = 1;

#include "sgigsc.h"
#if NSGIGSC > 0
	sgCleanup();			/* clean up sgigsc stuff */
#endif

	/*
	 * Get rid of context first. If vrelvm() has to sleep (on p_poip,
	 * for instance), someone may come along and try to steal our
	 * context, catching us with our pants down...
	 */
	cxput(p, 0);
	/*
	 * NOTE: We can now use the xp_ fields of struct proc...
	 */

	if ((p->p_pid == p->p_pgrp) && (u.u_ttyp != NULL) &&
	    (*u.u_ttyp == p->p_pgrp)) {
		*u.u_ttyp = 0;
		signal(p->p_pgrp, SIGHUP);
	}
	p->p_pgrp = 0;
	for (i=0; i<NOFILE; i++) {
		if (u.u_ofile[(short)i] != NULL)
			closef(u.u_ofile[(short)i]);
	}
	iunuse(u.u_cdir);
	if (u.u_rdir) {
		iunuse(u.u_rdir);
	}
	exitmisc(p);			/* machine dependent exit stuff */
	semexit();
	shmexit();
	acct(rv);

	/*
	 * Release virtual memory (text included)
	 */
	vrelvm();
#ifdef	NOTDEF
	/*
	 * These are now done safely in swtch()
	 */
	vrelpt(u.u_procp);
	vrelu(u.u_procp, 0);
#endif

	p->p_stat = SZOMB;
	p->xp_xstat = rv;
	p->xp_utime = u.u_cutime + u.u_utime;
	p->xp_stime = u.u_cstime + u.u_stime;
	for (q = &proc[1]; q < procNPROC; q++) {
		if (p->p_pid == q->p_ppid) {
			q->p_ppid = 1;
			if (q->p_stat == SZOMB)
				psignal(&proc[1], SIGCLD);
			if (q->p_stat == SSTOP)
				setrun(q);
		} else
		if (p->p_ppid == q->p_pid)
			psignal(q, SIGCLD);
		if (p->p_pid == q->p_pgrp)
			q->p_pgrp = 0;
	}

	swtch();
	/* NOTREACHED */
	/* no deposit, no return */
}

/*
 * Wait system call.
 * Search for a terminated (zombie) child,
 * finally lay it to rest, and collect its status.
 * Look also for stopped (traced) children,
 * and pass back status from them.
 */
wait()
{
	register struct proc *p;
	register short f;
	register short pid;

	pid = u.u_procp->p_pid;
loop:
	f = 0;
	for (p = &proc[1]; p < procNPROC; p++) {
		if (p->p_ppid == pid) {
			f++;
			if (p->p_stat == SZOMB) {
				freeproc(p, 1);
				return;
			}
			if (p->p_stat == SSTOP) {
				if ((p->p_flag&SWTED) == 0) {
					p->p_flag |= SWTED;
					u.u_rval1 = p->p_pid;
					u.u_rval2 = (fsig(p)<<8) | 0177;
					return;
				}
				continue;
			}
		}
	}
	if (f) {
		(void) sleep((caddr_t)u.u_procp, PWAIT);
		goto loop;
	}
	u.u_error = ECHILD;
}

/*
 * Remove zombie children from the process table.
 */
freeproc(p, flag)
register struct proc *p;
{
	if (flag) {
		register n;

		n = u.u_procp->p_cpu + p->p_cpu;
		if (n > 80)
			n = 80;
		u.u_procp->p_cpu = n;
		u.u_rval1 = p->p_pid;
		u.u_rval2 = p->xp_xstat;
	}
	u.u_cutime += p->xp_utime;
	u.u_cstime += p->xp_stime;
	p->p_stat = NULL;
	p->p_pid = 0;
	p->p_ppid = 0;
	p->p_sig = 0L;
	p->p_flag = 0;
	p->p_wchan = 0;
}

sbreak()
{
	struct a {
		char	*nsiz;
	};
	register int n, d;

	/*
	 * set n to new data size
	 * set d to new-old
	 */

	n = btoc(((struct a *)u.u_ap)->nsiz) - btoc(u.u_loadaddr) -
		ctos(u.u_tsize) * stoc(1);
	if (n < 0)
		n = 0;					/* XXX */
	d = n - u.u_dsize;
/* XXX */
#ifdef	notdef
	if (ctob(u.u_dsize+d) > u.u_rlimit[RLIMIT_DATA].rlim_cur) {
		u.u_error = ENOMEM;
		return;
	}
#endif
	if (chksize((u_int)u.u_tsize, (u_int)u.u_dsize+d, (u_int)u.u_ssize))
		return;
	if (swpexpand(u.u_dsize+d, u.u_ssize, &u.u_dmap, &u.u_smap)==0)
		return;
	expand(d, 0, 0);
}

/*
 * grow the stack to include the SP
 * true return if successful.
 */
grow(sp)
	long sp;
{
	register int si;

	if (sp >= USRSTACK-ctob(u.u_ssize))
		return (0);
	si = btoc(USRSTACK-sp) - u.u_ssize + SINCR;
	if (chksize((u_int)u.u_tsize, (u_int)u.u_dsize, (u_int)u.u_ssize+si))
		return (0);
	if (swpexpand(u.u_dsize, u.u_ssize+si, &u.u_dmap, &u.u_smap)==0)
		return (0);
	
	expand(si, 1, 0);
	return (1);
}

/* XXX */
/* All of the below came from kern_fork.c from 4.2; and should probably
   be left that way... */

/*
 * fork system call.
 */
fork()
{
	register short err, ppid;
	extern char shuttingdown;

	/*
	 * If system is going down, don't allow exec's
	 */
	if (shuttingdown) {
		u.u_error = EBUSY;
		goto out;
	}

	sysinfo.sysfork++;
	bzero((caddr_t)&u.u_cdmap, sizeof(struct dmap));
	bzero((caddr_t)&u.u_csmap, sizeof(struct dmap));
	if (swpexpand(u.u_dsize, u.u_ssize, &u.u_cdmap, &u.u_csmap) == 0) {
		u.u_error = ENOMEM;
		goto out;
	}

	ppid = u.u_procp->p_pid;		/* save parents pid */
	err = newproc();
	if (err < 0) {
		syserr.procovf++;
		u.u_error = EAGAIN;
		(void) vsexpand(0, &u.u_cdmap, 1);
		(void) vsexpand(0, &u.u_csmap, 1);
	} else
	if (err > 0) {				/* child is running now */
		u.u_rval1 = ppid;
		u.u_rval2 = 1;
		u.u_ior = 0;
		u.u_iow = 0;
		u.u_ioch = 0;
		u.u_cstime = 0;
		u.u_stime = 0;
		u.u_cutime = 0;
		u.u_utime = 0;
		u.u_start = time;
		u.u_acflag = AFORK;
		return;
	}

out:
    /* parent runs this code */
	u.u_rval2 = 0;
	u.u_ar0[PC] += 2;			/* XXX get rid of me */
}

/* XXX get rid of these two things */
short	mpid;

/*
 * Create a new process-- the internal version of
 * sys fork.
 * It returns 1 in the new process, 0 in the old.
 */
newproc()
{
	register struct proc *cp;
	register struct proc *pp;
	register int n;
	register struct file *fp;

retry:
    /* pick a pid to use */
	mpid++;
	if (mpid >= 30000) {
		mpid = 0;
		goto retry;
	}
	n = 0;
	cp = NULL;
	for (pp = proc; pp < procNPROC; pp++) {
		if (pp->p_stat == NULL) {
			if (cp == NULL)
				cp = pp;
		} else {
			if ((pp->p_pid == mpid) || (pp->p_pgrp == mpid))
				goto retry;
			if (pp->p_uid == u.u_uid)
				n++;
		}
	}

	/*
	 * Disallow if
	 *  No processes at all;
	 *  not su and too many procs owned; or
	 *  not su and would take last slot.
	 */
	if ((cp == NULL) ||
	    ((u.u_uid != 0) && ((cp == procNPROC - 1) || (n > MAXUPRC)) ) )
		return -1;

	forkstat.cntfork++;
	forkstat.sizfork += pp->p_dsize + pp->p_ssize;
	/*
	 * Make a proc table entry for the new process.
	 */
	pp = u.u_procp;
	cp->p_flag = SLOAD | SLOSTCX;
	cp->p_stat = SIDL;
	cp->p_pri = PUSER + pp->p_nice - NZERO;
	cp->p_time = 0;
	cp->p_cpu = 0;
	cp->p_nice = pp->p_nice;
	cp->p_slptime = 0;
	cp->p_uid = pp->p_uid;
	cp->p_suid = pp->p_suid;
	/* cp->p_pctcpu has to be zero because of exit() */
	/* cp->p_cpticks has to be zero because of exit() */
	cp->p_pgrp = pp->p_pgrp;
	cp->p_pid = mpid;
	cp->p_ppid = pp->p_pid;
	/* cp->p_addr is setup in procdup via vgetpt() */
	/* cp->p_poip has to be zero because of vrelvm() */
	cp->p_szpt = pp->p_szpt;
	cp->p_tsize = pp->p_tsize;
	cp->p_dsize = pp->p_dsize;
	cp->p_ssize = pp->p_ssize;
	cp->p_rssize = 0;
	cp->p_maxrss = pp->p_maxrss;
	/* cp->p_swrss is not setup; state is not SSWAP */
	/* cp->p_swaddr is not setup; state is not SSWAP */
	/* cp->p_p0br is setup in procdup via vgetpt() */
	/* cp->p_xlink is setup in procdup via xlink() */
	cp->p_sig = pp->p_sig;
	cp->p_wchan = 0;
	cp->p_textp = pp->p_textp;
	/* cp->p_link is setup in setrq() below */
	/* cp->p_clktim has to be zero because of exit() */
	cp->p_ndx = cp - proc;
	cp->p_loadc = pp->p_loadc;
	cp->p_grhandle = 0;

	/*
	 * Increase reference counts on shared objects.
	 */
	for (n = 0; n < NOFILE; n++) {
		fp = u.u_ofile[n];
		if (fp == NULL)
			continue;
		fp->f_count++;
	}
	u.u_cdir->i_count++;
	if (u.u_rdir)
		u.u_rdir->i_count++;

	shmfork(cp, pp);

	/*
	 * When the resume is executed for the new process,
	 * here's where it will resume.
	 */
	if (save(u.u_ssave)) {
		u.u_lock = 0;
		u.u_procp->p_flag |= SPTECHG;
		return 1;
	}

	/*
	 * Partially simulate the environment
	 * of the new process so that when it is actually
	 * created (by copying) it will look right.
	 * This begins the section where we must prevent the parent
	 * from being swapped.
	 */
	pp->p_flag |= SKEEP;
	(void) procdup(cp);

	/*
	 * Make child runnable and add to run queue.
	 */
	(void) spl6();
	cp->p_stat = SRUN;
	setrq(cp);
	(void) spl0();

	/*
	 * Cause child to take a non-local goto as soon as it runs.
	 */
	cp->p_flag |= SSWAP;

	/*
	 * Now can be swapped.
	 */
	pp->p_flag &= ~SKEEP;

	/*
	 * 0 return means parent.
	 */
	u.u_rval1 = cp->p_pid;		/* return childs pid to parent */
	return (0);
}
