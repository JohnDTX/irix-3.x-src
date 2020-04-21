/*
 * $Source: /d2/3.7/src/sys/pmII/RCS/debug.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:33:34 $
 */
#include "../h/param.h"
#include "../h/systm.h"
#include "../h/proc.h"
#include "../h/user.h"
#include "../h/text.h"
#include "../h/map.h"
#include "../h/cmap.h"
#include "../vm/vm.h"
#include "../h/setjmp.h"
#include "../h/printf.h"
#include "../h/tty.h"
#include "../h/buf.h"
#include "../h/conf.h"
#include "../h/fs.h"
#include "../h/inode.h"
#include "../h/mount.h"
#include "../com/pipe_inode.h"
#include "../pmII/cpureg.h"
#include "../pmII/reg.h"
#include "../pmII/pte.h"
#include "../pmII/trap.h"
#include "../pmII/cx.h"
#include "../multibus/mbvar.h"
#include "nx.h"

#include	"efs.h"
#if NEFS > 0
#	include	"../efs/efs_inode.h"
#	include "../efs/efs_sb.h"

extern short	efs_fstyp;
#endif

#include	"nfs.h"
#if NNFS > 0
#	include	"../nfs/nfs.h"
#	include	"../nfs/rnode.h"

extern short	nfs_fstyp;
#endif

#undef	DUMPC
#undef	DUMPT
#define	DUMPI
#define	DUMPN
#undef	TRACE

#ifdef	notdef
/*
 * crashdump:
 *	- dump the kernel virtual memory out to disk
 * XXX  build up a dump struct containing sizing info
 * XXX	first page dumped should contain struct
 * XXX	second-N pages should contain pagemap/protmap
 * XXX	N+1-M should contain kernel memory
 * XXX	M+1-K should contain all of physical memory
 * XXX  move this somewhere else
 */

dev_t	dumpdev = 0;
daddr_t	dumplo	= 500;

crashdump()
{
	register daddr_t bn;
	register long kva, mbva;
	short page, prot;
	int e;

	if (dumpdev == 0) {
		printf("dumpdev == 0\n");
		return;
	}
	bn = dumplo;
	printf("Dumping kernel at %d on dev 0x%04x\n", bn, dumpdev);
	for (kva = KERN_VBASE; kva < KERN_VLIMIT; kva += NBPG) {
		page = btop(kva ^ (KCX << 16));
		prot = *(((u_short *)PROTBASE) + page);
		if (((prot & PR_ASMASK) != PR_ASOBRAM) ||
		    !(((prot & PR_PROTMASK) == PR_KR) ||
		      ((prot & PR_PROTMASK) == PR_KW)))
			break;
		mbva = mbmapkget(kva, NBPG);
		e = (*bdevsw[major(dumpdev)].d_dump)(dumpdev, bn, mbva, NBPG);
		if (e) {
			printf("dump error (%d), giving up\n", e);
			break;
		}
		mbmapkput(mbva, NBPG);
		bn += (NBPG / DEV_BSIZE);
		printf(".");
	}
	printf("\nLast kva=%x bn=%d\n", kva, bn);
}
#endif

/*
 * Kernel debugger...
 *	- allows simple manipulation of the system at the time of a crash
 *	  which will hopefully aid diagnosis of failure
 *
 * Written by: Kipp Hickman
 */

#define	DUMP_KERNEL	0
#define	DUMP_PHYSICAL	1

extern	int *nofault;

/*
 * this stack area is used to run on during debugging
 * the size is well known in locore.s
 */
long	savessp;			/* saved system stack pointer */
struct	pte upte;			/* original udot pte */

kern_debug(msg)
	char *msg;
{
	register char ch;
	char warning_count;
	int fault_type;
	extern short kdebug;
	extern short havegrconsole;
	jmp_buf jb;
	int *save_nofault;

	if (!kdebug) {
		printf("debugger disabled\n");
		return;
	}
	if (kswitch)
		setConsole(CONSOLE_ON_SERIAL);

	/*
	 * Make sure message is printable.
	 */
	printf("\nKernel Debugger:");
	save_nofault = nofault;
	if (setjmp(jb) == 0) {
		nofault = jb;
		if (msg)
			printf(" %s", msg);
	}
	nofault = save_nofault;

	getpte(UDOT_VBASE, KCX, &upte);
	printf("\nkernel sp=%x, active udot=%x\n",
			 savessp, *(long *)&upte);

	warning_count = 0;
	save_nofault = nofault;
	if (fault_type = setjmp(jb)) {
		printf("\nFault: type=%d pc=%x addr=%x\n",
				 fault_type, u.u_ar0[PC],
				 u.u_pcb.pcb_aaddr);
	}
	nofault = jb;

	for (;;) {
		printf("debug: ");
		while ((ch = getchar()) < ' ')
			;
		switch (ch) {
		  case 'b':
			printf("boot (confirm) ");
			if (getchar() != 'y') {
				printf("no\n");
				continue;
			}
			printf("yes\n");
			doboot();
			/*NOTREACHED*/
#ifndef	KOPT_NOGL
		  case 'c':
			printf("changing consoles\n");
			if (!havegrconsole) {
				printf("ain't got no udder console\n");
				break;
			}
			if (con_getchar == dugetchar) {
				con_getchar = grgetchar;
				con_putchar = grputchar;
			} else {
				con_getchar = dugetchar;
				con_putchar = duputchar;
			}
			break;
#endif
		  case 'e':
			printf("dump a map\n");
			dump_amap();
			break;
		  case 'f':
			printf("display free memory\n");
			dump_cmap();
			break;
		  case 'g':
			printf("cylinder group dump\n");
			dump_cgs();
			break;
		  case '?':
		  case 'h':
			printf("help");
			do_help();
			break;
		  case 'k':
			printf("kernel memory dump");
			dump_memory(DUMP_KERNEL);
			break;
		  case 'm':
			printf("memory dump");
			dump_memory(DUMP_PHYSICAL);
			break;
		  case 'p':
			printf("page map");
			dump_pagemap();
			break;
		  case 'q':
			printf("quit (confirm) ");
			if (getchar() != 'y') {
				printf("no\n");
				continue;
			}
			printf("yes\n");
			nofault = save_nofault;
			return;
		  case 'r':
			printf("registers");
			dump_registers();
			break;
		  case 's':
			printf("stack trace");
			dump_stack();
			break;
		  case 't':
			printf("text table\n");
			dump_text();
			break;
		  case 'u':
			printf("change udot to ");
			switch_udot();
			break;
		  case 'B':
			printf("dump buffers");
			dump_buf();
			break;
#ifdef	notdef
		  case 'D':
			printf("do a dump\n");
			crashdump();
			break;
#endif
		  case 'M':
			printf("mbmap dump\n");
			dump_mbmap();
			break;
		  case 'P':
			printf("proc table dump");
			dump_proctable();
			break;
#ifdef	TRACE
		  case 'T':
			printf("trace dump");
			dump_trace();
			break;
#endif
		  case 'U':
			printf("update disk\n");
			(void) spl0();
			update();
			(void) spl7();
			break;
		  case 'X':
			printf("proc dump");
			dump_proc();
			break;
# ifdef DUMPC
		  case 'C':
			printf("clist dump");
			dump_clist();
			break;
# endif DUMPC
# ifdef DUMPT
		  case 'd':
			printf("tty dump");
			dump_tty();
			break;
# endif DUMPT
# ifdef DUMPI
		  case 'i':
			printf("inode dump");
			dump_inode();
			break;
# endif DUMPI
		  case 'z':
			printf("mount table dump");
			dump_mount();
			break;
		  default:
			if (warning_count++ < 5)
				printf("\nType \"h\" for help.\n");
			else
				printf("\n");
			break;
		}
	}
}

/*
 * Given a set of single character choices, return the index of the matched
 * choice, or -1 if no match
 */
choice(choices)
	register char *choices;
{
	register char ch;
	register int i;

	ch = getchar();
	i = 0;
	while (*choices) {
		if (*choices == ch)
			return (i);
		choices++;
		i++;
	}
	return (-1);
}

/*
 * htoi:
 *	- convert a hex string to integer
 */
u_long
htoi(s)
	register char *s;
{
	register u_long value;

	value = 0;
	while (*s) {
		if ((*s >= '0') && (*s <= '9'))
			value = (value << 4) + (*s - '0');
		else
		if ((*s >= 'a') && (*s <= 'z'))
			value = (value << 4) + (*s - 'a') + 10;
		else
		if ((*s >= 'A') && (*s <= 'Z'))
			value = (value << 4) + (*s - 'A') + 10;
		else
			break;
		s++;
	}
	return value;
}

do_help()
{
	printf("\nLegal commands are:\n");

	printf("a\talloc list dump\n");
	printf("b\tboot\n");
#ifndef	KOPT_NOGL
	printf("c\tchanging consoles\n");
#endif
#ifdef	DUMPT
	printf("d\ttty dump\n");
#endif
	printf("e\tdump a map\n");
	printf("f\tdisplay free memory\n");
	printf("g\tcylinder group dump\n");
	printf("h|?\thelp\n");
#ifdef	DUMPI
	printf("i\tinode dump\n");
#endif
	printf("k\tkernel memory dump\n");
	printf("m\tmemory dump\n");
	printf("p\tpage map\n");
	printf("q\tquit\n");
	printf("r\tregisters\n");
	printf("s\tstack trace\n");
	printf("t\ttext table\n");
	printf("u\tchange udot\n");
	printf("z\tmount table dump\n");
	printf("B\tdump buffer cache\n");
	printf("D\tdo a dump\n");
	printf("M\tmbmap dump\n");
	printf("P\tproc table dump\n");
#ifdef	TRACE
	printf("T\ttrace dump\n");
#endif
	printf("U\tupdate the disk\n");
	printf("X\tproc dump\n");
#ifdef	DUMPC
	printf("C\tclist dump\n");
#endif
}

/*
 * dbgfindproc:
 *	- ask user for a pid, then find that process
 */
struct proc *
dbgfindproc()
{
	register struct proc *p;
	char b[80];
	short pid;

	printf(" Pid ");
	gets(b);
	if (b[0]) {
		pid = atoi(b);
		for (p = &proc[0]; p < procNPROC; p++) {
			if (p->p_pid == pid)
				return p;
		}
		putchar('?');
	}
	return NULL;
}

/*
 * switch_udot:
 *	- prompt for a pid to switch to, and then remap the upage to point
 *	  to the new udot
 */
switch_udot()
{
	register struct proc *p;
	struct pte apte;

	if ((p = dbgfindproc()) == NULL)
		return;
	if (!(p->p_flag & SLOAD)) {
		printf("process is swapped out\n");
		return;
	}
	printf("New udot pte=%x\nSwitch? ", *(long *)(p->p_addr));
	if (getchar() == 'y') {
		printf("yes\n");
		apte = *(p->p_addr);
		*(long *)&apte |= PG_AS_OBRAM | PG_V;
		setpte((long) UDOT_VBASE, KCX, &apte);
		printf("Saved fp=%x pc=%x &rsave=%x\n",
			      u.u_rsave[10], u.u_rsave[12], &u.u_rsave[0]);
		return;
	}
	printf("no\n");
}

#ifdef	notdef
/*
 * dump_udot:
 *	- dump out portions of the user structure for a given pid
 */
dump_udot()
{
	register struct user *up;
	register struct proc *p;
	struct pte pg3pte, apte;

	if ((p = dbgfindproc()) == NULL)
		return;
	if (!(p->p_flag & SLOAD)) {
		printf("process is swapped out\n");
		return;
	}
	getpte(SCRPG3, KCX, &pg3pte);
	apte = *(p->p_addr);
	*(long *)&apte |= PG_AS_OBRAM | PG_V;
	setpte((long) SCRPG3, KCX, &apte);
	up = (struct user *)SCRPG3;

	printf("Udot for pid=%d\n", p->p_pid);
    printf("p0br=0x%x p0lr=0x%x p1br=0x%x p1lr=0x%x procp=0x%x loadaddr=%d\n",
			up->u_pcb.pcb_p0br, up->u_pcb.pcb_p0lr,
			up->u_pcb.pcb_p1br, up->u_pcb.pcb_p1lr,
			up->u_procp, up->u_loadaddr);
	printf("ar0=0x%x ap=0x%x error=%d rval1=0x%x rval2=0x%x\n",
		       up->u_ar0, up->u_ap, up->u_error,
		       up->u_rval1, up->u_rval2);
	printf("ts=%d ds=%d ss=%d\n", up->u_tsize, up->u_dsize, up->u_ssize);
	printf("uid=%d gid=%d ruid=%d rgid=%d\n", up->u_uid,
		       up->u_gid, up->u_ruid, up->u_rgid);
	printf("base=0x%x count=%d offset=0x%x segflg=%d\n",
			  up->u_base, up->u_count, up->u_offset, up->u_segflg);
	setpte((long) SCRPG3, KCX, &pg3pte);
}
#endif

/*
 * dump_stack:
 *	- print a stack trace
 *	- user supplies a starting frame pointer, which gets us going...
 */
dump_stack()
{
	register long *vaddr;
	register short i;
	char b[80];

	printf(" Virtual address=0x");
	gets(b);
	if (b[0] == 0)
		return;
	vaddr = (long *)htoi(b);
	if ((vaddr < (long *)UDOT_VBASE) ||
	    (vaddr >= (long *)(UDOT_VBASE + ctob(1)))) {
		printf("Invalid address\n");
		return;
	}

	printf("....fp.....pc..(args)\n");
	for (;;) {
		if ((vaddr < (long *)UDOT_VBASE) ||
		    (vaddr+ 1 >= (long *)(UDOT_VBASE + ctob(1))))
			break;
		printf("%6x %6x  (", *vaddr, *(vaddr + 1));
		for (i = 0; i < 8; i++) {
			if (((vaddr + i + 2) < (long *)UDOT_VBASE) ||
			    ((vaddr + i + 2) >= (long *)(UDOT_VBASE + ctob(1)))) {
				printf(")");
				goto done;
			}
			if (i < 7)
				printf("%x, ", *(vaddr + 2 + i));
			else
				printf("%x)\n", *(vaddr + 2 + i));
		}
		vaddr = (long *)*vaddr;
	}
done:
	printf("\n");
}

/*
 * dump_pagemap:
 *	- dump out the page map, in a nice way
 */
dump_pagemap()
{
	register short i, j;
	register u_long vaddr, save_vaddr;
	char b[80];
	struct grotpte xpte;

	printf(" Virtual address=0x");
	gets(b);
	if (b[0] == 0)
		return;
	vaddr = htoi(b);
	i = btop(vaddr) / 8;		/* round to 8 page boundary */
	vaddr = ctob(i * 8);		/* get rounded vaddr */

	for (; i < NPAGEMAP / 8; i++) {
		printf("\n%06x: ", vaddr);
		save_vaddr = vaddr;
		for (j = 0; j < 8; j++) {
			kgetpte((long)vaddr, 0, (struct pte *)&xpte);
			printf("%04x ", (u_short)xpte.pg_pfnum);
			vaddr += NBPG;
		}
		printf("\n        ");
		vaddr = save_vaddr;
		for (j = 0; j < 8; j++) {
			kgetpte((long)vaddr, 0, (struct pte *)&xpte);
			printf("%04x ", (u_short)xpte.pg_protbits);
			vaddr += NBPG;
		}
		if (getchar() == 'q') {
			printf("\n");
			return;
		}
	}
	printf("\n");
}

/*
 * kgetpte:
 *	- read the hardware page and protection mapping information and
 *	  translate it into a software pte
 */
kgetpte(vaddr, cx, pte)
	register long vaddr;
	register short cx;
	register struct pte *pte;
{
	register short page;

	page = btop(vaddr ^ (cx << 16));
	((struct grotpte *)pte)->pg_pfnum = *(((u_short *)PAGEBASE) + page);
	((struct grotpte *)pte)->pg_protbits = *(((u_short *)PROTBASE) + page);
}

/*
 * dump_memory:
 *	- dump out memory, in a nice way
 *	- dump at most one page
 */
dump_memory(how)
	short how;
{
	register short i;
	register u_short *vaddr, *limit;
	register char ch;
	u_long addr;
	char b[80];
	struct pte apte;

	printf(" Address=0x");
	gets(b);
	if (b[0] == 0)
		return;
	addr = htoi(b) & ~0xf;		/* convert and align address */

    /* figure out how to display memory */

	if (how == DUMP_KERNEL) {
		vaddr = (u_short *)addr;
		if ((addr < KERN_VBASE) || (addr > UDOT_VBASE + NBPG)) {
			printf("addr out of bounds [%x-%x]\n",
				     KERN_VBASE, UDOT_VBASE + NBPG);
			return;
		}
		limit = (u_short *) (UDOT_VBASE + NBPG);
	} else {
		vaddr = (u_short *)(SCRPG0_VBASE + (addr & 0x0FF0));
		*(long *)&apte = btop(addr) | PG_AS_OBRAM | PG_KW | PG_V;
		setpte((long) SCRPG0_VBASE, KCX, &apte);
		limit = (u_short *)(SCRPG0_VBASE + NBPG);
	}

    /* dump out memory */

	for (;;) {
		if (vaddr >= limit)
			break;
		for (i = 0; i < 16; i++) {
			ch = *(((u_char *)vaddr) + i);
			if ((ch < 32) || (ch >= 127))
				b[i] = '.';
			else
				b[i] = ch;
		}
		b[i] = 0;
		printf("\n%06x: %04x %04x %04x %04x %04x %04x %04x %04x %s ",
				addr,
				*(vaddr + 0), *(vaddr + 1),
				*(vaddr + 2), *(vaddr + 3),
				*(vaddr + 4), *(vaddr + 5),
				*(vaddr + 6), *(vaddr + 7),
				b);
		vaddr += 8;
		addr += 16;
		if (getchar() == 'q')
			break;
	}
	printf("\n");
}

/*
 * dump_registers:
 *	- display registers at the time of the panic
 */
dump_registers()
{
	register struct user *up;
	register short i;
	struct pte cpte;

	up = &u;
	getpte((long)up, KCX, &cpte);
	printf("\nSSP=%06x, active udot=%x, current=%x addr=%x\n",
			    savessp, *(long *)&upte, *(long *)&cpte,
			    u.u_pcb.pcb_aaddr);
	u.u_comm[DIRSIZ-1] = 0;
	printf("pc=%x sr=%04x u.u_procp=%x pid=%d exec='%s'\n",
		u.u_ar0[PC], u.u_ar0[RPS]&0xFFFF, u.u_procp,
		u.u_procp->p_pid, u.u_comm);
	for (i = 0; i < 16; i++) {
		printf("0x%x ", u.u_ar0[i]);
		if (i == 7 || i == 15)
			printf("\n");
	}
}

/*
 * proc_state:
 *	- return the character assosciated with the state of the process
 */
char
proc_state(state)
	register char state;
{
	switch (state) {
	  case SSLEEP:
		return 'S';
	  case SWAIT:
		return 'W';
	  case SRUN:
		return 'R';
	  case SIDL:
		return 'I';
	  case SZOMB:
		return 'Z';
	  case SSTOP:
		return 'T';
	  default:
		if (state < 10)
			return state + '0';
		else
			return '?';
	}
}

/*
 * dump_process:
 *	- display the contents of the process table
 */
dump_proctable()
{
	register struct proc *p;

	printf(
"\nPROCP   FLAGS S UID   PID  PPID PRI NIC   ADDR SIZE  WCHAN   TEXT   P0BR CX"
	       );
	for (p = &proc[0]; p < procNPROC; p++) {
		if (p->p_stat == 0)
			continue;
		printf("\n%6x %7x %c %3d %5d %5d %3d %3d %6x %4d %6x %6x %6x",
			      p, p->p_flag, proc_state(p->p_stat),
			      p->p_uid, p->p_pid, p->p_ppid, p->p_pri,
			      p->p_nice, p->p_addr, p->p_dsize + p->p_ssize,
			      p->p_wchan, p->p_textp, p->p_p0br);
		if (!(p->p_flag & SLOSTCX))
			printf(" %02x-%02x", p->p_cxnum, p->p_cxbsize);
	}
	printf("\n");
}

/*
 * dump_proc:
 *	- dump out a specific process
 */
dump_proc()
{
	register struct proc *p;
	register struct pte *pte;
	register struct user *up;
	register short i;
	struct pte apte;

	if ((p = dbgfindproc()) == NULL)
		return;
	printf("procp=%x flag=%x stat=%c pri=%d time=%d cpu=%d nice=%d\n",
			 p, p->p_flag, proc_state(p->p_stat),
			 p->p_pri, p->p_time, p->p_cpu, p->p_nice);
	printf("slptime=%d uid=%d suid=%d pgrp=%d pid=%d ppid=%d\n",
			   p->p_slptime, p->p_uid, p->p_suid,
			   p->p_pgrp, p->p_pid, p->p_ppid);
	printf("addr=%x poip=%d szpt=%d tsize=%d dsize=%d ssize=%d\n",
			p->p_addr, p->p_poip, p->p_szpt, p->p_tsize,
			p->p_dsize, p->p_ssize);
	printf("rssize=%d maxrss=%d swrss=%d swaddr=%d p0br=%x\n",
			  p->p_rssize, p->p_maxrss, p->p_swrss,
			  p->p_swaddr, p->p_p0br);
	printf("xlink=%x sig=%x wchan=%x textp=%x link=%x\n",
			 p->p_xlink, p->p_sig,
			 p->p_wchan, p->p_textp, p->p_link);
	printf("clktim=%d ndx=%d loadaddr=%x\n",
			  p->p_clktim, p->p_ndx, p->p_loadc);
	printf("cxnum=%x cxbsize=%x prev=%x next=%x\n",
			 p->p_cxnum, p->p_cxbsize, p->p_cxprev, p->p_cxnext);
	if (p->p_flag & SLOAD) {
		pte = tptopte(p, 0);
		if (p->p_textp) {
			printf("Text pages:");
			for (i = 0; i < p->p_tsize; i++) {
				if ((i & 7) == 0)
					printf("\n");
				printf("%08x ", *(long *)pte);
				pte++;
			}
		}
		pte = dptopte(p, 0);
		printf("\nData pages:");
		for (i = 0; i < p->p_dsize; i++) {
			if ((i & 7) == 0)
				printf("\n");
			printf("%08x ", *(long *)pte);
			pte++;
		}
		pte = sptopte(p, 0);
		printf("\nStack pages: (in reverse order!)");
		for (i = 0; i < p->p_ssize; i++) {
			if ((i & 7) == 0)
				printf("\n");
			printf("%08x ", *(long *)pte);
			pte--;
		}
		printf("\nUpage: %08x\n", *(long *)p->p_addr);

		up = (struct user *)SCRPG0_VBASE;
		apte = *(p->p_addr);
		*(long *)&apte |= PG_AS_OBRAM | PG_V;
		setpte((long) SCRPG0_VBASE, KCX, &apte);
		printf("Dmap: (size=%d alloc=%d)",
			      up->u_dmap.dm_size, up->u_dmap.dm_alloc);
		for (i = 0; i < NDMAP; i++) {
			if ((i & 7) == 0)
				printf("\n");
			printf("%5d ", up->u_dmap.dm_map[i]);
			if (up->u_dmap.dm_map[i] == 0)
				break;
		}
		printf("\nSmap: (size=%d alloc=%d)",
			      up->u_smap.dm_size, up->u_smap.dm_alloc);
		for (i = 0; i < NDMAP; i++) {
			if ((i & 7) == 0)
				printf("\n");
			printf("%5d ", up->u_smap.dm_map[i]);
			if (up->u_smap.dm_map[i] == 0)
				break;
		}
		printf("\n");
	}
}

/*
 * textflags:
 *	- return a string containing flag information
 */
char *
textflags(f, bp)
	register char f;
	register char *bp;
{
	if (f & XTRC)
		*bp++ = 'T';
	if (f & XWRIT)
		*bp++ = 'W';
	if (f & XLOAD)
		*bp++ = 'L';
	if (f & XLOCK)
		*bp++ = 'X';
	if (f & XWANT)
		*bp++ = 'W';
	if (f & XERROR)
		*bp++ = 'E';
	if (f & XSAVE)
		*bp++ = 'S';
	*bp = 0;
}

/*
 * dump_text:
 *	- print out pure text table in a nice way
 */
dump_text()
{
	register struct text *xp;
	char buf[10];

printf(
"\nNDX     XP    PT SIZ  CADDR INUM RSS SWRSS REF LOAD POIP SLP CMX FLG\n");
	for (xp = &text[0]; xp < textNTEXT; xp++) {
		if (xp->x_iptr == NULL)
			continue;
		textflags(xp->x_flag, buf);
	printf("%3d %06x %5d %3d %6x %4d %3d %5d %3d %4d %4d %3d %3d %s\n",
		    xp - text, xp, xp->x_ptdaddr, xp->x_size,
		    xp->x_caddr, xp->x_iptr->i_number, xp->x_rssize,
		    xp->x_swrss, xp->x_count, xp->x_ccount,
		    xp->x_poip, xp->x_slptime,
		    xp->x_cmx, buf);
#ifdef	notdef
		{
			register short i;

			for (i = 0; i < NXDAD; i++)
				printf("%3d ", xp->x_daddr[i]);
			printf("\n");
		}
#endif
	}
}

/*
 * dump_cmap:
 *	- dump the cmap and cmhash data structures
 */
dump_cmap()
{
	register short i;
	register struct cmap *c;

	printf("\nfreemem=%d maxmem=%d firstfree=%d maxfree=%d desfree=%d\n",
			     freemem, maxmem, firstfree, maxfree, desfree);
	printf("ncmap=%d ecmx=%d cmap=%x ecmap=%x\n",
			 ncmap, ecmx, cmap, ecmap);
printf("       NUM NEXT PREV LW PAGE IFGTY LOCKS NDX PF\n");
	c = cmap;
	for (i = 0; i < ncmap; i++, c++) {
	printf("%06x %3d %4d %4d %d%d %4x %d%d%d%2d %5d %3d %x\n",
		     c, i, c->c_next, c->c_prev, c->c_lock,
		     c->c_want, c->c_page, c->c_intrans,
		     c->c_free, c->c_gone, c->c_type, c->c_iolocks,
		     c->c_ndx, i ? cmtopg(i) : 0xffff);
		if (((i % 20) == 0) && i) {
			if (getchar() == 'q')
				break;
		}
	}
}

/*
 * dump_mbmap:
 *	- display the multibus map
 */
dump_mbmap()
{
	register short i;
	struct pte apte;

	printf("\nMultibus map:");
	for (i = 16; i < 256; i++) {
		if ((i & 7) == 0)
			printf("\n%02x: ", i);
		*(long *)&apte = (0x100 + i) | PG_AS_MBRAM | PG_KR | PG_V;
		setpte((long) MBUS_VBASE, KCX, &apte);
		printf("%4x ", *(u_short *)MBUS_VBASE);
	}
	printf("\n");
}

/*
 * dump_amap:
 *	- choose a resource map to dump, then dump it
 */
dump_amap()
{
	char b[20];
	extern struct map mbmap[];

    printf("\nMaps known about:\nswapmap=%x argmap=%x kernelmap=%x mbmap=%x\n",
		   swapmap, argmap, kernelmap, mbmap);
	printf("Address=0x");
	gets(b);
	if (b[0] == 0)
		return;
	dumpmap((struct mapent *)htoi(b));
}

/*
 * dumpmap:
 *	- dump out a map
 */
dumpmap(m)
	register struct mapent *m;
{
	register short i;

	printf("\nMap %s (size %d)\n Size Addr(16) Addr(10)\n",
		    ((struct map *)m)->m_name,
		    i = ((struct map *)m)->m_limit - (m + 1));
	while (i--) {
		m++;
		printf("%5d %8x %8d\n", m->m_size, m->m_addr, m->m_addr);
		if (m->m_size == 0)
			break;
	}
}

#ifdef	notdef

#define	getuvpfnum(vaddr) \
	* ( ((u_short *)PAGEBASE) + btop((int)(vaddr) ^ (cx.cx_user << 16)) )
#define	getuvprot(vaddr) \
	* ( ((u_short *)PROTBASE) + btop((int)(vaddr) ^ (cx.cx_user << 16)) )

/*
 * dumpupte:
 *	- dump out current users hardware ptes
 */
dumpupte()
{
	register struct proc *p = u.u_procp;
	register short npages, i;
	register long vaddr;

	printf("User page table for proc %d\n", p->p_pid);
	printf("page0=%x.%x upage=%x.%x\n",
			    getuvprot(0), getuvpfnum(0),
			    getuvprot(&u), getuvpfnum(&u));
	printf("data+text:");
	npages = p->p_tsize + p->p_dsize;
	i = 0;
	vaddr = u.u_loadaddr;
	while (npages--) {
		if ((i & 7) == 0)
			printf("\n");
		printf("%04x.%04x ", getuvprot(vaddr), getuvpfnum(vaddr));
		i++;
		vaddr += ctob(1);
	}
	printf("\nstack:");
	npages = p->p_ssize;
	i = 0;
	vaddr = USRSTACK - ctob(npages);
	while (npages--) {
		if ((i & 7) == 0)
			printf("\n");
		printf("%04x.%04x ", getuvprot(vaddr),
				   getuvpfnum(vaddr));
		i++;
		vaddr += ctob(1);
	}
	printf("\n");
}
#endif

#ifdef	TRACE
#define NTRACEBUF	1000

struct	tracebuf {
	char	*t_msg;				/* pointer to message */
	u_long	t_p0, t_p1, t_p2, t_p3;		/* some values */
	time_t	t_time;				/* time stamp */
} tracebuf[NTRACEBUF];

short	tindex;				/* current index into xtbuf[] */

/*
 * trace:
 *	- snapshot some values and a message into a circular trace buffer
 */
/*VARARGS1*/
trace(c, p0, p1, p2, p3)
	char *c;
	long p0, p1, p2, p3;
{
	register struct tracebuf *t;
	register int s;

	s = spl7();;
	t = &tracebuf[tindex++];
	if (tindex >= NTRACEBUF)
		tindex = 0;
	t->t_msg = c;
	t->t_p0 = p0;
	t->t_p1 = p1;
	t->t_p2 = p2;
	t->t_p3 = p3;
	t->t_time = lbolt;
	splx(s);
}

/*
 * dump_trace:
 *	- display results of trace
 */
dump_trace()
{
	register struct tracebuf *t;
	register short i, n, nout;
	register int mode;
	char b[50];
	char traceobuf[100];

	printf(" how (search|dump) ");
	switch (choice("sd")) {
	  case 0:			/* search */
		printf("search pattern=");
		gets(b);
		if (b[0] == 0)
			return;
		mode = 0;
		break;
	  case 1:			/* dump */
		printf("dump\n");
		mode = 1;
		break;
	  default:
		printf("?\n");
		return;
	}

	i = tindex - 1;
	if (i < 0)
		i = NTRACEBUF - 1;

	nout = 0;
	printf("\nTrace buffer (in reverse order!) (current index=%d)", i);
	for (n = 0; n < NTRACEBUF; n++) {
		t = &tracebuf[i];
		if (t->t_msg) {
			sprintf(traceobuf, t->t_msg, t->t_p0, t->t_p1,
					   t->t_p2, t->t_p3);
			if (mode || findpattern(b, traceobuf)) {
				printf("\n%8d\t%s", t->t_time, traceobuf);
				nout++;
				if (!more(nout))
					break;
			}
		}
		if (--i < 0)
			i = NTRACEBUF - 1;
	}
	printf("\n");
}

/*
 * Compare two strings, insuring that for at least the length of the shortest
 * string, that they match.  If the length of the pattern string is larger
 * than the text string, then assume no match.
 */
patmatch(text, pat)
	register char *text;
	register char *pat;
{
	if (strlen(text) < strlen(pat))
		return (0);

	while (*pat) {
		if (*text++ != *pat++)
			return (0);
	}
	return (1);
}

/*
 * Search for the given pattern in the given text, returning non-zero if
 * we have a match, zero otherwise.  Sorry, no regular expressions
 */
findpattern(pat, text)
	register char *pat;
	register char *text;
{
	register char *cp;
	extern char *strchr();

	while (*text) {
		cp = strchr(text, *pat);
		if (cp == NULL)
			return (0);		/* not even possibly there */
		if (patmatch(cp, pat))
			return (1);
		text = cp + 1;
	}
	return (0);
}
#endif

#ifdef	notdef
calldebug(m)
    char *m;
{
    register int s;

    s = spl7();
    clkstop();
    printf("calldebug: %s",m);
    debug();
    clkstart();
    splx(s);
}
#endif	notdef

# ifdef DUMPI
dump_ifree()
{
    register int nfree;
    register struct inode *ip;

    nfree = 0;
    printf("ifreepool $%x\n",&ifreepool);
    for( ip = ifreepool.il_fforw; ip != (struct inode *) &ifreepool;
	 ip = ip->i_fforw )
    {
	if( !(ip->i_fforw->i_fback == ip && ip == ip->i_fback->i_fforw) )
	    printf("broken chain! $%x - $%x - $%x\n"
		    ,ip->i_fforw->i_fback,ip,ip->i_fback->i_fforw);
	nfree++;
	printf(" $%x: $%x",ip,ip->i_flag);
    }
    printf(" -- %d free inodes\n",nfree);
}
dump_i()
{
    register struct inode *ip;
    register int nused;
    register int i;

    nused = 0;
    ip = inode + 0;
    for( i = ninode; --i >= 0; ip++ )
    if( ip->i_count != 0 )
    {
	nused++;
	printf(" $%x: $%x",ip,ip->i_flag);
    }
    printf(" -- %d used inodes\n",nused);
}
dump_inode()
{
    char b[100];
    register struct inode *ip;

    printf(" Address (or [AF]): ");
    gets(b);
    if( b[0] == 000 )
	return;
    if( b[0] == 'F' )
    {
	dump_ifree();
	return;
    }
    if( b[0] == 'A' )
    {
	dump_i();
	return;
    }
    ip = (struct inode*)htoi(b);
    dump1_inode(ip);
}
dump1_inode(ip)
	register struct inode *ip;
{
	printf("$%x: back=$%x forw=$%x\n", ip, ip->i_hback, ip->i_hforw);
	printf(" flag=$%x size=%d", ip->i_flag, ip->i_size);
	printf(" count=%d dev=$%x", ip->i_count, ip->i_dev);
	printf(" number %d",ip->i_number);
	printf(" fstyp %d\n",ip->i_fstyp);
	printf(" rdev $%x",ip->i_rdev);
	printf(" ftype %o",ip->i_ftype);
	printf(" nlink %d",ip->i_nlink);
	printf(" id %d.%d\n",ip->i_uid, ip->i_gid);
	printf(" fsptr $%x", ip->i_fsptr);
	printf("\n");
	if (ip->i_flag & IFIFO) {
		register struct pipe_inode *pi = pipe_fsptr(ip);

		printf(" mode %o\n", pi->pi_com.ci_mode);
		printf(" rb={flags=%x ptr=%d ref=%d pq={len=%d,proc=$%x}}\n",
			pi->pi_rb.pb_flags,
			pi->pi_rb.pb_ptr,
			pi->pi_rb.pb_ref,
			pi->pi_rb.pb_pq.pq_length,
			pi->pi_rb.pb_pq.pq_proc);
		printf(" wb={flags=%x ptr=%d ref=%d pq={len=%d,proc=$%x}}\n",
			pi->pi_wb.pb_flags,
			pi->pi_wb.pb_ptr,
			pi->pi_wb.pb_ref,
			pi->pi_wb.pb_pq.pq_length,
			pi->pi_wb.pb_pq.pq_proc);
	}
#if NEFS > 0
	else if (ip->i_mntdev->m_fstyp == efs_fstyp) {
		register int i;
		register struct efs_inode *iip;

		iip = efs_fsptr(ip);
		if (iip == NULL)
			return;
		printf(" mode %o",iip->ii_mode);
		if (ip->i_ftype == IFBLK || ip->i_ftype == IFCHR) {
			printf("\n");
			return;
		}
		printf(" #extents %d", iip->ii_numextents);
		if (iip->ii_numindirs > 0) {
			printf(" #indirect %d\n", iip->ii_numindirs);
			printf("\n # bn     len");
			for (i = 0; i < iip->ii_numindirs; i++) {
				printf("\n%2d %6d %3d",
				    i, iip->ii_indir[i].ex_bn,
				    iip->ii_indir[i].ex_length);
			}
			if (iip->ii_numextents > 0)
				printf("\ndirect:");
		}
		if (iip->ii_numextents > 0) {
			printf("\n# bn     len");
			for (i = 0; i < iip->ii_numextents; i++) {
				printf("\n%2d %6d %3d",
				    i, iip->ii_extents[i].ex_bn,
				    iip->ii_extents[i].ex_length);
			}
		}
		printf("\n");
	}
#endif
#if NNFS > 0
	else if (ip->i_fstyp == nfs_fstyp) {
		register struct rnode *rp;
		register struct nfsfattr *ap;
		register int flags, i;
		static struct flagmap {
			u_short	bit;
			char	*name;
		} flagmap[] = {
			RLOCKED,	"RLOCKED",
			RWANT,		"RWANT",
			RIOWAIT,	"RIOWAIT",
			REOF,		"REOF",
			RDIRTY,		"RDIRTY",
		};

		rp = vtor(ip);
		if (rp == NULL)
			return;
		printf(" rnode %d forw=$%x back=$%x ip=$%x\n",
		    rp->r_number, rp->r_forw, rp->r_back, rp->r_ip);
		printf(" fh={fsid=%x fno=%d gen=%x}",
		    rp->r_fh.fh_fsid, rp->r_fh.fh_fno, rp->r_fh.fh_fgen);
		printf(" open=%d flags=", rp->r_open);
		flags = rp->r_flags;
		for (i = 0; i < sizeof(flagmap) / sizeof(flagmap[0]); i++) {
			if (flagmap[i].bit & flags) {
				printf(flagmap[i].name);
				flags &= ~flagmap[i].bit;
				if (flags == 0)
					break;;
				putchar('|');
			}
		}
		printf("\n error=%d lastr=%d cred=%d.%d unlcred=%d.%dn",
		    rp->r_error, rp->r_lastr,
		    rp->r_cred.cr_uid, rp->r_cred.cr_gid,
		    rp->r_unlcred.cr_uid, rp->r_unlcred.cr_gid);
		printf("\n unlname=%s unldip=$%x iocount=%d ncap=%d",
		    rp->r_unlname ? rp->r_unlname : "NULL",
		    rp->r_unldip, rp->r_iocount, rp->r_ncap);
		ap = &rp->r_nfsattr;
		printf("\nnfsattr={\n");
		printf("\t type=%d mode=%o nlink=%d id=%d.%d\n",
		    ap->na_type, ap->na_mode, ap->na_nlink,
		    ap->na_uid, ap->na_gid);
		printf("\t size=%d blocksize=%d rdev=%x blocks=%x\n",
		    ap->na_size, ap->na_blocksize,
		    ap->na_rdev, ap->na_blocks);
		printf("\t fsid=%x nodeid=%d atime=%d.%d\n",
		    ap->na_fsid, ap->na_nodeid,
		    ap->na_atime.tv_sec, ap->na_atime.tv_usec);
		printf("\t mtime=%d.%d ctime=%d.%d\n",
		    ap->na_mtime.tv_sec, ap->na_mtime.tv_usec,
		    ap->na_ctime.tv_sec, ap->na_ctime.tv_usec);
		printf("\t nfsattrtime=%d\n }\n",
		    rp->r_nfsattrtime);
	}
#endif
}
#endif	DUMPI

# ifdef DUMPT
extern char *spybuf();
dump_tty()
{
# if	NNX > 0
    extern struct tty nx_tty[];
# endif	NNX > 0
#ifndef	KOPT_NOGL
    extern struct tty wntty[];
#endif
    char b[100];
    register struct tty *tp;
    register int i;

    printf(" Address (or [DWX#]): ");
    gets(b);
    if( *b == 000 )
	return;
    i = htoi(b+1);
#ifndef	KOPT_NOGL
    if( *b == 'W' )
	tp = wntty + i;
    else
#endif
# if	NNX > 0
    if( *b == 'X' )
	tp = nx_tty + i;
    else
# endif	NNX > 0
	tp = (struct tty *)i;

    dump1_tty(tp);
}

dump1_tty(tp)
    register struct tty *tp;
{
printf("$%x: index $%x state 0%o\n",tp,tp->t_index,tp->t_state);
printf("rawq [$%x:%d] canq [$%x:%d] outq [$%x:%d]\n"
,&tp->t_rawq,tp->t_rawq.c_cc
,&tp->t_canq,tp->t_canq.c_cc
,&tp->t_outq,tp->t_outq.c_cc);
printf("tbuf [%d/%d %s]\n"
,tp->t_tbuf.c_count,tp->t_tbuf.c_size
,spybuf(tp->t_tbuf.c_ptr,tp->t_tbuf.c_count));
printf("rbuf [%d/%d %s]\n"
,tp->t_rbuf.c_count,tp->t_rbuf.c_size
,spybuf(tp->t_rbuf.c_ptr,tp->t_rbuf.c_count));
printf(" ldisc %d delct %d",tp->t_line,tp->t_delct);
printf(" pgrp $%x \n",tp->t_pgrp);
}
# endif DUMPT

# ifdef DUMPC
dump_clist()
{
    char b[100];
    register int i;

    printf(" Address (or F): ");
    gets(b);
    if( *b == 000 )
	return;
    i = htoi(b);
    if( *b == 'F' )
    {
	extern struct chead cfreelist;
	i = (int)cfreelist.c_next;
	printf("cfreelist [$%x: next $%x size %d flag $%x]\n"
		,&cfreelist
		,i,cfreelist.c_size,cfreelist.c_flag);
	dumpcchain((struct cblock *)i,0);
	return;
    }
    dump1_clist((struct clist *)i);
}
dump1_clist(qp)
    struct clist *qp;
{
    printf("count %d first $%x last $%x\n" ,qp->c_cc,qp->c_cf,qp->c_cl);
    dumpcchain(qp->c_cf,1);
}
dumpcchain(cp,flag)
    register struct cblock *cp;
    int flag;
{
    register int n;

    n = 0;
    while( cp != 0 )
    {
	n++;
	printf(" [%d..%d %s]",cp->c_first,cp->c_last
		,spybuf(cp->c_data+cp->c_first,flag?cp->c_last-cp->c_first:0));
	cp = cp->c_next;
    }
    printf(" -- %d cblocks\n",n);
}

char *spybuf(buf,n)
    register char *buf;
    int n;
{
    static char spychars[100];
    register char *sp;
    register unsigned char c;

    if( buf == 0 )
	return "";

    if( n > sizeof spychars-1 ) n = sizeof spychars-1;

    sp = spychars;
    while( --n >= 0 )
    {
	c = *buf++;
	if (c < 040) {
	    *sp++ = '^';
	    *sp++ = (c & 037) + '@';
	} else
	if (c == 0177) {
	    *sp++ = '^';
	    *sp++ = '?';
	} else
	    *sp++ = c;
    }

    *sp++ = 000;
    return spychars;
}
# endif DUMPC

dump_mount()
{
	register struct mount *mp;

	printf("\n dev fstyp bsize ronly     fs mounted   root   next\n");
	mp = mount;
	while (mp) {
		printf("%4x %5d %5d %5d %6x %7x %6x %6x\n",
			    mp->m_dev, mp->m_fstyp, mp->m_bsize,
			    mp->m_readonly, mp->m_fs, mp->m_inodp,
			    mp->m_mount, mp->m_next);
		mp = mp->m_next;
	}
}

dump_buf()
{
	printf(" type (swap|cache) ");
	switch (choice("sc")) {
	  case 0:
		printf("swap\n");
		dump_buf_swap();
		break;
	  case 1:
		printf("cache\n");
		dump_buf_cache();
		break;
	  default:
		printf("?\n");
		return;
	}
}

dump_buf_cache()
{
	register int i;
	register struct buf *bp;

printf(
   "\n   buf    flags  dev blkno len   addr    mem bcount iobase"
);
	bp = buf;
	for (i = 0; i < nbuf; i++, bp++) {
		printf("\n%6x %8x %4x %5d %3d %6x %6x %6d %6x",
			      bp, bp->b_flags, (unsigned short)bp->b_dev,
			      bp->b_blkno, bp->b_length, bp->b_un.b_addr,
			      bp->b_memaddr, bp->b_bcount, bp->b_iobase);
		if (!more(i))
			break;
	}
	printf("\n");
}

dump_buf_swap()
{
	register int i;
	register struct buf *bp;

printf("swbuf=%x bclnlist=%x bswlist.av_forw=%x\n",
		 swbuf, bclnlist, bswlist.av_forw);
printf(
"\n   buf  flags  dev blkno   addr    mem   proc nc iobase  pf   forw"
);
	bp = swbuf;
	for (i = 0; i < nswbuf; i++, bp++) {
		printf("\n%6x %6x %4x %5d %6x %6x %6x %2d %6x %3x %6x",
			      bp, bp->b_flags, (unsigned short)bp->b_dev,
			      bp->b_blkno, bp->b_un.b_addr, bp->b_memaddr,
			      bp->b_proc, btoc(bp->b_bcount),
			      bp->b_iobase, bp->b_pfcent,
			      bp->av_forw);
		if (!more(i))
			break;
	}
	printf("\n");
}

/*
 * Return non-zero if user wants more printout
 */
more(x)
	int x;
{
	if (x && ((x % 20) == 0)) {
		printf(" (q/cr)? ");
		if (getchar() == 'q')
			return (0);
	}
	return (1);
}

dump_cgs()
{
	register struct mount *mp;
	char c;

	mp = mount;
	while (mp) {
		if (mp->m_fstyp == efs_fstyp) {
			printf("dev=%04x -- dump? ", mp->m_dev);
			c = getchar();
			if (c == 'y') {
				printf("yes\n");
				dump_efs((struct efs *) (mp->m_fs));
			} else
				printf("no\n");
		}
		mp = mp->m_next;
	}
}

dump_efs(fs)
	register struct efs *fs;
{
	register struct cg *cg;
	register int i;

	printf("size=%d firstcg=%d cgfsize=%d cgisize=%d sectors=%d\n",
			fs->fs_size, fs->fs_firstcg, fs->fs_cgfsize,
			fs->fs_cgisize, fs->fs_sectors);
	printf("heads=%d ncg=%d time=%d\n",
			 fs->fs_heads, fs->fs_ncg,
			 fs->fs_time);
	printf("magic=%x dirty=%d fname=%s fpack=%s prealloc=%d bmsize=%d\n",
			 fs->fs_magic, fs->fs_dirty, fs->fs_fname,
			 fs->fs_fpack, fs->fs_prealloc, fs->fs_bmsize);
	printf("tinode=%d tfree=%d checksum=%x\n",
			  fs->fs_tinode, fs->fs_tfree, fs->fs_checksum);

printf(" cg firstbn lastbn firstdbn firsti lasti fulli  lowi dfree nextd lastd\n");
	cg = &fs->fs_cgs[0];
	for (i = 0; i < fs->fs_ncg; i++, cg++) {
		printf("%3d %7d %6d %8d %6d %5d %5d %5d %5d %5d %5d ",
			    i, cg->cg_firstbn,
			    cg->cg_firstbn + fs->fs_cgfsize - 1,
			    cg->cg_firstdbn, cg->cg_firsti,
			    (i + 1) * fs->fs_ipcg - 1,
			    cg->cg_fulli, cg->cg_lowi,
			    cg->cg_dfree, cg->cg_nextd, cg->cg_lastd);
		printf(" dump? ");
		if (getchar() == 'y')
			dump_cg(fs, cg);
		else
			printf("\n");
	}
}

dump_cg(fs, cg)
	register struct efs *fs;
	register struct cg *cg;
{
	register int i, j;
	char l[81];
	int bn;

	printf("\nfirstdbn=%d firstdfree=%d\n",
			      cg->cg_firstdbn, cg->cg_firstdfree);
	if (cg->cg_nextd < cg->cg_lastd) {
		i = 0;
		printf("dsum:");
		while (i < cg->cg_lastd) {
			if (i == cg->cg_nextd)
				printf("!");
			printf("%d-%d ", cg->cg_dsum[i].d_bn,
				      cg->cg_dsum[i].d_length);
			i++;
		}
		printf("\n");
	}
	printf("bitmap:\n");
	bzero(l, sizeof(l));
	bn = cg->cg_firstdbn;
	j = 0;
	for (i = fs->fs_cgfsize; --i >= 0; bn++) {
		l[j] = '0';
		if (btst(fs->fs_dmap, bn))
			l[j] = '1';
		j++;
		if (j == 80) {
			j = 0;
			printf("%s", l);
		}
	}
	if (j) {
		l[j] = 0;
		printf("%s", l);
	}
	printf("\n");
}
