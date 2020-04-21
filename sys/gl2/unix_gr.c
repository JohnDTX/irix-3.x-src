#include "../h/param.h"
#include "../h/systm.h"
#include "../h/user.h"
#include "../h/proc.h"
#include "../h/tty.h"
#include "../h/buf.h"
#include "../vm/vm.h"
#include "../h/cmap.h"
#include "machine/cx.h"
#include "machine/pte.h"
#include "machine/cpureg.h"

#include "debug.h"

#undef	imin
#undef	imax

#include <gl2/shmem.h>
#include <gl2/gr.h>
#include <gl2/gltypes.h>

struct	shmem *gl_kernelshmemptr = (struct shmem *)SHMEM_VBASE;

#ifdef pmII
#define	shmem_addr(p, offset) \
	(&((p)->p_p0br[(p)->p_szpt * NPTEPG - 4 + offset]))
#endif

#ifdef juniper
#define	shmem_addr(p, offset) \
	(&((p)->p_p0br[(p)->p_szpt * NPTEPG - 2 + offset]))
#endif

/*
 * gr_coredump:
 *	- core dump the current process (try to, anyway)
 *	- then kill it
 */
gr_coredump()
{
	int status;

	gr_unlockmem(u.u_procp, u.u_procp->p_lockaddr, u.u_procp->p_lockcount);
	status = 1;
	if (core())
		status += 0200;
	exit(status);
}

/*
 * gr_freepages:
 *	- return the number of free virtual pages left in the users address
 *	  space
 *	- this doesn't take into account the swap limit
 */
gr_freepages()
{
	return (MAXTDSIZ - u.u_tsize - u.u_dsize - u.u_ssize -
		HIGHPAGES - u.u_procp->p_loadc);
}

/*
 * gr_txport:
 *	- return textport # of current process, if any
 */
gr_txport()
{
	extern short win_dev;
	extern short havegrconsole;

	/* ICK */
	if ((major(u.u_ttyd) == win_dev) ||
	    ((major(u.u_ttyd) == 0/*console_dev*/) && havegrconsole))
		return (minor(u.u_ttyd));
	return (-1);
}

/*
 * gr_getshmempa:
 *	- this is used by sureg
 *	- should just have a macro in vmmac.h for pages beyond the user
 *	  stack
 */
gr_getshmempa(p, offset)
	register struct proc *p;
	int offset;
{
	return (shmem_addr(p, offset)->pg_pfnum);
}

/* HACK HACK HACK */
gr_shmempte()
{
	struct pte apte;
	getpte(SHMEM_VBASE, KCX, &apte);
	return (*(long *)&apte);
}

static	char console_kb_stash[10];
static	short console_kb_chars, console_kb_spot;

/* ARGSUSED */
gr_softintr(tn, c, isbreak)
	short tn, isbreak;
	register char c;
{
	if (isbreak)
		c = 0x80;
	console_kb_stash[console_kb_chars++] = c;
}

grgetchar()
{
	char c;
	extern int (*kbd_intr)();
	extern int wn_softintr();

	kbd_intr = gr_softintr;
	for (;;) {
		if (console_kb_chars) {
			c = console_kb_stash[console_kb_spot++];
			console_kb_chars--;
			kbd_intr = wn_softintr;
			return (c);
		}
		console_kb_spot = 0;
		c = duxgetchar(0, 0L);
		kb_translate(c);
	}
}

/*
 * gr_lockmem:
 *	- lock down feedback pages
 */
gr_lockmem(uservaddr, count)
	caddr_t uservaddr;
	long count;
{
	extern struct pte *iolock();

	if (iolock(uservaddr, count) == NULL)
		return (1);
	u.u_procp->p_lockaddr = uservaddr;
	u.u_procp->p_lockcount = count;
	u.u_procp->p_flag |= SFEED;
	return (0);
}

/*
 * gr_unlockmem:
 *	- unlock the user feedback pages
 */
gr_unlockmem(p, uservaddr, count)
	struct proc *p;
	caddr_t uservaddr;
	long count;
{
	if (p->p_flag & SFEED) {
		p->p_flag &= ~SFEED;
		if ((p->p_lockaddr != uservaddr) || (p->p_lockcount != count))
			panic("gr_unlockmem");
		iounlock(vtopte(p, btop(uservaddr)), (unsigned)uservaddr,
			       count, B_READ);
	}
}

/*
 * grsys:
 *	- handle graphics commands from the kernel
 */
grsys()
{
	register struct a {
		int	cmd;
		long	addr;
	} *uap = (struct a *)u.u_ap;
    
	u.u_rval1 = gr_ioctl(uap->cmd, uap->addr);
	if (gl_ioerror) {
		u.u_error = gl_ioerror;
		u.u_rval1 = -1;
	}
}

/*
 *	gr_getoshandle -
 *		return a long which points to the process descriptor
 *		of the current process.
 */
gr_getoshandle()
{
	return ((long)u.u_procp);
}

/*
 *	gr_getgrhandle -
 *		return a long which points to the window struct (grhandle)
 *		of the specified process (oshandle).  the return value will
 *		be zero if the specified process is not a graphics process.
 */
struct inputchan *
gr_getgrhandle(p)
	struct proc *p;
{
	if (p == 0)
		return ((struct inputchan *)0);
	return ((struct inputchan *) (p->p_grhandle));
}

/*
 *	gr_setgrhandle -
 *		make the process discriptor of the specified process
 *		(oshandle) point to the named window struct (grhandle).
 */
gr_setgrhandle(p, grhandle)
	register struct proc *p;
	long grhandle;
{
	if (p == NULL)			/* during boot up, this happens */
		return;

	if (!grhandle && (p->p_flag & SGR)) {
		p->p_flag &= ~SGR;
		memfree(shmem_addr(p, 0), 1, 1);
	}
	p->p_grhandle = grhandle;
}

/*
 * gr_setshmem:
 *	- set the shmem context to a new process
 *	- return the previous shmem context
 */
long
gr_setshmem(p)
	register struct proc *p;
{
	register struct pte *pte;
	struct pte apte, x;
	register int s;
	extern short shmem_pa;		/* kernels shmem */

	s = spl6();
	getpte(SHMEM_VBASE, KCX, &apte);
	if (p && p->p_pid)
		pte = shmem_addr(p, 0);
	else {
		initpte(&x, shmem_pa, PG_V | PG_KW);
/*		*(long *)&x = shmem_pa | PG_AS_OBRAM | PG_KW | PG_V; */
		pte = &x;
	}
	setpte(SHMEM_VBASE, KCX, pte);
	splx(s);
	return (*(long *)&apte);
}

/*
 * gr_restoreshmem:
 *	- go back to the context before gr_setshmem was called.
 */
gr_restoreshmem(apte)
	long apte;
{
	register int s;

	s = spl6();
	setpte(SHMEM_VBASE, KCX, (struct pte *)&apte);
	splx(s);
}

/* ARGSUSED */
gr_sleep(p, addr)
	struct proc *p;
	long addr;
{
	sleep((caddr_t)addr, PSLEP);
}

/* ARGSUSED */
gr_wakeup(p, addr)
	struct proc *p;
	long addr;
{
	wakeup((caddr_t)addr);
}

gr_systype()
{
	return (GRSYS_UNIX);
}

/*
 * gr_getshmem:
 *	- allocate shared memory for the current user
 *	- return kernel virtual address of the area
 */
struct shmem *
gr_getshmem()
{
	register struct proc *p = u.u_procp;
	register struct pte *pte;

	if (p->p_flag & SGR)
		panic("duplicate gr_getshmem");
	cxput(p, 1);
	(void) vmemall(shmem_addr(p, 0), 1, p, CSYS);
	p->p_flag |= SGR;
	pte = shmem_addr(p, 0);
	clearseg(pte->pg_pfnum);
/*
	*(long *)pte &= ~PG_PROT;
	*(long *)pte |= PG_UWKW | PG_AS_OBRAM | PG_V;
*/
	initpte(pte, pte->pg_pfnum, PG_V | PG_UWKW);
	return ((struct shmem *)SHMEM_VBASE);
}

gr_kill(p)
	register struct proc *p;
{
	if (p->p_flag & SGR) {
		p->p_flag &= ~SGR;
		memfree(shmem_addr(p, 0), 1, 1);
		psignal(p, SIGKILL);
	}
}

kb_issoft()
{
	return (1);	/* UNIX always has a soft keyboard? */
}

/*
 * gr_os_stopproc:
 *	- stop the named process from using the pipe
 */
/* ARGSUSED */
gr_os_stopproc(p)
	struct proc *p;
{
	runrun++;
}

#include "tcp.h"
gr_signalqueue(p)
	struct proc *p;
{
#if NTCP > 0
	selwakeup(p, 0);
#endif
}

#include "sgigsc.h"
#if NSGIGSC == 0
gr_queuenotempty(p)
	struct proc *p;
{
	if (!gr_isqempty(p->p_grhandle))
		return 1;
	return 0;
}

sgqenter() { }
#endif
