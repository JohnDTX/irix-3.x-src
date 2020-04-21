/*
 * $Source: /d2/3.7/src/sys/sys/RCS/main.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:35:24 $
 */

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/user.h"
#include "../h/proc.h"
#include "../h/utsname.h"
#include "../vm/vm.h"
#include "../h/inode.h"
#include "../com/com_pncc.h"
#include "../h/buf.h"
#include "../h/mount.h"
#include "../h/cmap.h"
#include "machine/pte.h"
#include "machine/cpureg.h"

#include "xns.h"

/*
 * Initialization code.
 * Called from cold start routine as
 * soon as a stack and segmentation
 * have been established.
 * Functions:
 *	clear and free user core
 *	turn on clock
 *	hand craft 0th process
 *	call all initialization routines
 *	fork - process 0 to schedule
 *	     - process 1 execute bootstrap (/etc/init)
 *	     - process 2 for pageout process
 *	System will abort to proms if either process 1 or process 2 die
 */


extern	short icode[];		/* user init code */
extern	int szicode;		/* init code size */
extern	char version[];		/* version string */
extern	char release[];		/* utsname release string */
extern	char utsversion[];	/* utsname version string */

int	*nofault;
struct	inode *rootdir;
int	cmask, cdlimit;

/* have to define these somewhere */
time_t	boottime;
struct	proc *proc, *procNPROC;
char	hostname[MAXHOSTNAMELEN+1];
short	hostnamelen;
char	domainname[MAXHOSTNAMELEN+1];
short	domainnamelen;

#ifdef	OS_DEBUG
short	kdebug = 1;
#else
short	kdebug = 0;
#endif

main()
{
	register struct proc *p;

	printf("%s\n", version);
	printf("(C) Copyright 1986 - Silicon Graphics Inc.\n");
#ifdef	PM2
	strcpy(utsname.machine, "m68010");
#endif
#ifdef	IP2
	strcpy(utsname.machine, "m68020");
#endif
	strcpy(utsname.release, release);
	strcpy(utsname.version, utsversion);
	startup();

	/* set up scheduler process */
	p = &proc[0];
	p->p_p0br = u.u_pcb.pcb_p0br;
	p->p_szpt = 1;
	p->p_addr = uaddr(p);
	p->p_stat = SRUN;
	p->p_flag = SLOAD|SSYS|SLOSTCX;
	p->p_nice = NZERO;
	p->p_avgrss = 0;
	p->p_loadc = btoc(USRTEXT);
	u.u_procp = p;
	u.u_cmask = cmask;
	u.u_limit = cdlimit;
	u.u_loadaddr = USRTEXT;
	u.u_stack[0] = 0xa5a5a5a5;

	/* init the world */
	clkstart();
	cxinit();
	seminit();
	msginit();
#if NXNS > 0
	init_clist();
#endif
	init_bio();
	init_file();
	init_inodes();
	pncc_init();	/* init the pathname component cache */

	flckinit();	/* initialize file locks */
	strinit();	/* initialize streams */
	fsinit();	/* init filesystems via the switch */

	/* now we can configure the devices */
	spl0();
	configure();
	swfree();

	/*
	 * Set the scan rate and other parameters of the paging subsystem.
	 */
	setupclock();

	/* start timer events running */
	schedcpu();
	schedpaging();

#ifdef	INET
	mbinit();
#endif
#include "loop.h"
#if NLOOP > 0
	loattach();
#endif
#include "cypress.h"
#if NCYPRESS > 0
	if_cyinit();
#endif
#ifdef	INET
{
	int s;

	/*
	 * Block reception of incoming packets
	 * until protocols have been initialized.
	 */
	s = splimp();
	ifinit();
	domaininit();
	splx(s);
}
#endif

	/*
	 * mount root; setup the system rootdir as well as attach the
	 * scheduler's (mother of all processes) current directory to root.
	 */
	if (mountdev(rootdev, 0, (struct inode *)NULL))
		panic("iinit");
	rootdir = iget(mount, ROOTINO);
	rootdir->i_flag &= ~ILOCKED;
	u.u_cdir = iget(mount, ROOTINO);
	u.u_cdir->i_flag &= ~ILOCKED;
	u.u_rdir = NULL;
	sbgettime();
#ifdef HAVERTC
	todclkset();
#endif
	boottime = time;

	u.u_rdir = NULL;
	u.u_start = time;
	bzero((caddr_t)&u.u_dmap, sizeof(struct dmap));
	bzero((caddr_t)&u.u_smap, sizeof(struct dmap));

	/*
	 * make init process and
	 * enter scheduling loop
	 */
	proc[0].p_szpt = 1;
	if (newproc()) {
		expand((int)btoc(szicode), 0, 0);
		(void) swpexpand(u.u_dsize, 0, &u.u_dmap, &u.u_smap);
		(void) copyout((caddr_t)icode, (caddr_t)u.u_loadaddr, szicode);
		/*
		 * Return goes to loc. 0 of user init
		 * code just copied out.
		 */
		return;
	}

	/*
	 * make page-out daemon (process 2)
	 * the daemon has ctopt(nswbuf*KLMAX) pages of page
	 * table so that it can map dirty pages into
	 * its address space during asychronous pushes.
	 */
	proc[0].p_szpt = ctopt(nswbuf*KLMAX + UPAGES);
	if (newproc()) {
		proc[2].p_flag |= SLOAD|SSYS;
		proc[2].p_dsize = u.u_dsize = nswbuf*KLMAX;
		proc[2].p_flag &= ~(SPTECHG | SLOSTCX);
		pageout();
		/*NOTREACHED*/
	}

	/*
	 * Enter scheduling loop
	 */
	proc[0].p_szpt = 1;
	sched();
}
