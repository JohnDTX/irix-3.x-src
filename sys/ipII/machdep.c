/*
 * ipII/machdep.c
 */
#include "../h/param.h"
#include "../h/systm.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/proc.h"
#include "../h/seg.h"
#include "../h/map.h"
#include "../h/fs.h"
#include "../h/inode.h"
#include "../h/tty.h"
#include "../h/termio.h"
#include "../h/file.h"
#include "../h/callout.h"
#include "../h/cmap.h"
#include "../h/buf.h"
#include "../com/com_pncc.h"
#include "../h/text.h"
#include "../h/acct.h"
#include "../vm/vm.h"
#include "../h/setjmp.h"
#include "../h/printf.h"
#include "../h/kprof.h"
#include "../ipII/psr.h"
#include "../ipII/cpureg.h"
#include "../ipII/reg.h"
#include "../ipII/cx.h"
#include "../ipII/pte.h"
#include "../ipII/tod.h"
#include "../ipII/common.h"
#include "../multibus/mbvar.h"
#include "../debug/debug.h"
#include "../ipII/frame.h"

#include "xns.h"

extern	int etext, end;
extern	short panicing;
extern  short havefpa;
#ifndef KOPT_NOGL
extern	char _dcrmodes;
extern	short _dcrflags;
#endif

extern	unsigned char *bio_bbcnt;
extern	struct pte *bio_pt;
extern	struct map *bio_memmap;

char	XXXetheraddr[6];	/* ethernet address, maybe */
short	unixend;		/* page # of last unix page */
short	physmem;		/* # of physical memory pages */
long	rebootvec = PROM_BASE;	/* holds address of reboot entry to proms */
short	beprint;
short	maxmem;
u_short	shmem_pa;		/* physical page # of shared memory */
u_int	wub_pf;			/* physical page # of wub */
int	mbwin;
char	wtimeout;

char	rev_A;			/* !=0 if this is a revision-A IP2 board */



/*
 * Icode is the bootstrap program used to exec init.
 *	- attempts to exec /etc/init, and if that fails, calls exit
 *	  so that the system will panic...
 */
#define LOW	(USRTEXT&0xFFFF)		/* Low user starting address */
#define HIGH	((USRTEXT>>16)&0xFFFF)		/* High user starting address */
short icode[] = {
					/*	 .=USTART		*/
/* 00 */	0x2E7C,	HIGH, LOW+0x400,/*	 movl	#USTART+0x400,sp*/
/* 06 */	0x227C,	HIGH, LOW+0x2C,	/*	 movl	#envp,a1	*/
/* 0c */	0x223C,	HIGH, LOW+0x28,	/*	 movl	#argp,d1	*/
/* 12 */	0x207C,	HIGH, LOW+0x30,	/*	 movl	#name,a0	*/
/* 18 */	0x42A7,			/*	 clrl	sp@-		*/
/* 1a */	0x303C,	0x3B,		/*	 movw	#59.,d0		*/
/* 1e */	0x4E40,			/*	 trap	#0		*/
/* 20 */	0x303C, 0x01,		/*	 movw	#1.,d0		*/
/* 24 */	0x4E40,			/*	 trap	#0		*/
/* 26 */	0x60FE,			/*	 bra	.		*/
/* 28 */	HIGH,	LOW+0x30,	/* argp: .long	name		*/
/* 2c */	0,	0,		/* envp: .long	0		*/
/* 30 */	0x2F65,	0x7463,	0x2F69,	/* name: .asciz	"/etc/init"	*/
		0x6E69,	0x7400
};
int szicode = sizeof(icode);

#ifdef	PROF
extern int proflevel7, mouseintr;

/*
 * Set and reset the mouse quadrate vector to point to the profiling routine
 * or the mouse interrupt handler
 */
setprofvec()
{
	setlevel7(&proflevel7);
}

unsetprofvec()
{
	setlevel7(&mouseintr);
}

setlevel7(addr)
	int *addr;
{
	struct pte apte;
	int s;

	s = splmax();				/* won't REALLY work... */
	initpte(&apte, 0, PG_V|PG_KW);
	setpte(KERN_VBASE, KCX, &apte);
	*((long *)KERN_VBASE + 87) = (long) addr;
	*((long *)KERN_VBASE + 86) = (long) addr;
	initpte(&apte, 0, PG_V|PG_KR);
	setpte(KERN_VBASE, KCX, &apte);
	splx(s);
}
#endif

/*
 * mmuinit:
 *	- intialize the mmu to a known state
 *	- map the kernel into context 0
 *	- switch to the context (just to make sure)
 *	- map the schedulers udot (at firstpage)
 *	- protect kernel text except for i/o vectors
 *	- ASSUMES UPAGES == 1
 */
mmuinit(firstpage)
	register short firstpage;
{
	register struct pte *uaddr0;
	struct pte x;
	register u_char	*xx;

	init_mbmap();

	/*
	 * Map the kernels text space as read only.  Map the remainder of
	 * the kernels address space as system-access-only (kernel rw).
	 */
	mapkernel((long) (KERN_VBASE),
		  (int) btop((long)&etext - KERN_VBASE), 0, (int) PG_KR);
	mapkernel((long) &etext,
		  (int) btop(KERN_VLIMIT - (long)&etext + NBPG - 1),
		  (int) btop((long)&etext - KERN_VBASE), (int) PG_KW);

    /* setup proc 0's page table pte */
	uaddr0 = &Usrptmap[0];
	*(long *)uaddr0 = firstpage+1 | PG_KW | PG_V;
	setpte((long)usrpt, KCX, uaddr0);

    /* setup proc 0's udot page */
	uaddr0 = &usrpt[NPTEPG - UPAGES];	/* == uaddr(&proc[0]) */
	*(long *)uaddr0 = firstpage | PG_KW | PG_V;
	u.u_pcb.pcb_p0br = usrpt;
	u.u_pcb.pcb_p0lr = 0;
	firstpage += 2;		/* first vallocable page	*/

    /* don't ask (see the vax locore.s....) */
    /* p1br must the address of the starting pte that is not used for
     * region p1 (remember region p1 grows from high to low)
     * p1lr is the length of the unused region. This is set up
     * for proc 0.
     * The following picture hopefully describes this:
     *
     * usrpt:
     *  +---------------+
     *	|		|
     *	|		|<- 4*(-(P1PAGES-UPAGES)), backs up to start of area.
     *	|		|
     *	|		|
     *	|		|<- 4*UPAGES (backs up the one used pte (udot)
     *	|		|<- usrpt + NBPG, end of p1 region
     *	|		|
     *	|		|
     *	|		|
     *	+---------------+
     *
     *  P1PAGES - UPAGES -> # pages not in use
     *  Multiplication by 4 represents the pte size, so we get byte offsets.
     */
	u.u_pcb.pcb_p1br = (struct pte *)
		((int)usrpt + NBPG - 4*UPAGES + 4*(-(P1PAGES-UPAGES)));
	u.u_pcb.pcb_p1lr = P1PAGES-UPAGES;

	/*
	 * Before things get too serious, rip off a page
	 * for the graphics shared memory...
	 */
	shmem_pa = firstpage;
	*(long *)&x = firstpage++ | PG_KW | PG_V;
	setpte(SHMEM_VBASE, KCX, &x);
	clearseg((unsigned) shmem_pa);

	/*
	 * Now steal one page for the idiotic multibus controllers that need
	 * fixed addressed multibus memory.  We map the multibus map to point
	 * to this page.  We only support fixed address devices at WUB_MBADDR.
	 * This page is also mapped accessible from the processor at WUB_VBASE.
	 */
	wub_pf = firstpage++;
	clearseg(wub_pf);
	xx = (u_char *)MBREG_VBASE + WUB_MBADDR;
	xx += mbwin;
	*(u_short *)xx = wub_pf;
	*(long *)&x = wub_pf | PG_KW | PG_V;
	setpte(WUB_VBASE, KCX, &x);
	beprint = 1;
	return (int)(firstpage);
}

/*
 * For each 1/2Mb of memory, we keep this data structure for translating
 * page frame #'s into cmap index's
 */
#define	NBOARDS	MAXMEMMB * 2		/* number of "1/2Mb" boards */
short	btocmx[NBOARDS+1];		/* +1 because last one is a marker */

#define NIM1BRDS	8		/* max number of IM1 memory boards  */
#define	IM1REG_VBASE	( MBIO_VBASE + 0x400 )
char	membdsz[ NIM1BRDS ];		/* holds sizes of memory boards	    */

/*
 * sizemem:
 *	- size the system's memory
 *	- The proms have already sized the memory and left that information
 *	   in the the
 *	   static 2kb area.
 *	- We clear memory, and set up the btocmx array.  We convert frm
 *	  the 1mb chunks in the 2kb ram area to 1/2mb chunks btocmx expects
 *	  to be in.  This is done in order to limit kernel changes.
 *	- "firstfree" is the first free page frame # it follows the sched page
 *	  table and udot.
 */
/* XXX REDO THIS! */
sizemem(firstfree)
	short firstfree;
{
	register short *b;
	register short i, j, pf;
	u_long	membits, memmb;

	/* known layout! */
	membits = *(u_long *)SRAM_BASE;
	memmb = *((u_long *)SRAM_BASE + 1);

	/* clear out remainder of first boards memory */
	for (i = firstfree; i < ONEMEGPG; i++)
		clearseg((unsigned) i);

	physmem = memmb << 8;	/* convert to pages */
	btocmx[1] = 1;		/* assume 1mb present	*/
	b = &btocmx[2];

	/* now clear rest of da memory */
	for ( pf = ONEMEGPG, i = 1; i < MAXMEMMB; i++, pf += ONEMEGPG )
	{
		if ( membits & ( 1 << i ) )
		{
			/* one megabyte exists, so clear dem pages */
			for ( j = 0; j < btop(ONEMEG); j++ )
				clearseg((unsigned)(pf+j));

			/* update btocmx array */
			*b++ = 1; *b++ = 1;
		}
		else
		{
			/* no memory here */
			*b++ = 0; *b++ = 0;
		}
	}
	*b++ = -1;

	/*
	** we must keep track of the size of each memory board, 2 or 4mb,
	** for later parity error reporting
	*/
	for ( j = 0, i = 0; i < MAXMEMMB; j++, i += 4 )
	{
		if ( membits & ( 0xF << i ) )
		{
			if ( membits & ( 5 << i ) )
				membdsz[ j ] = 1; 	/* 4mb board	*/
			else
				membdsz[ j ] = 0;	/* 2mb board	*/
		}
		else
			membdsz[ j ] = 0xFF;		/* board not present */
	}
}

/*
 * System page tables:
 *	- these are just SOFTWARE maintained pte info; the hardware map
 *	  is loaded when these are changed using vmaccess() and ptaccess()
 */ 
struct	pte Usrptmap[USRPTSIZE];
struct	pte Forkmap[UPAGES];
struct	pte Pushmap[UPAGES];
struct	pte Xswapmap[UPAGES];
struct	pte Xswap2map[UPAGES];
struct	pte Swapmap[UPAGES];

/*
 * The number of swap headers is three-quarters of the maximum number of
 * pushes which the pagedaemon may initiate in one second.
 */
short	nswbuf = (DISKRPM * 2 / 3) * 3 / 4;

startup()
{
	printf("real = %d\n", ctob(physmem));
	printf("kmem = %d\n", ctob(unixend));
	printf("user = %d\n", ctob(freemem));
	printf("bufs = %d (max=%dk)\n", nbuf * efs_lbsize, efs_lbsize / 1024);
#ifdef PROF
	printf("%d bytes of profiling memory reserved at 0x%x-0x%x\n",
		   PROFSIZE, profbuf, (long)profbuf + PROFSIZE);
#endif
}

/*
** mlsetup( firstaddr )
** short firstaddr;	first free memory page
**   determine memory size, and adjust to constraints
**   allocate system tables, and initialize some of the paging
**   data structures
**   NOTE: firstaddr must reside within the first 1megabyte of memory.
*/
mlsetup( firstaddr )
register short	firstaddr;
{
	register short		i;
	register caddr_t	v;
	register short		*b, cmx;
	register u_long		*pagemap;
	int totalpages;
	int nproctimes2;
	long bytes_of_cache;
	int pages;

	/*
	** set according to MASTER/SLAVE bit in the switch register.
	** Used in the VTOP() macro.
	*/
	if ( *SWTCH_REG & SW_MASTRSLV )
		mbwin = (ONEMEG << 1);
	else
		mbwin = 0;

	duinit();	/* initialize the duarts */
	todinit();	/* initialize the clock/tod */

	firstaddr = (short)mmuinit( firstaddr );	/* setup mmus */
	sizemem(firstaddr);
	totalpages = physmem;

	/* init efs based on memory info */
	efs_setparams(totalpages);

	/*
	 * Allocate space for system data structures.
	 * The first available real memory address is in "firstaddr".
	 * As pages of memory are allocated, "firstaddr" is incremented.
	 * The first available kernel virtual address is in "v".
	 * As pages of kernel virtual memory are allocated, "v" is incremented.
	 * An index into the kernel page table corresponding to the
	 * virtual memory address maintained in "v" is kept in "mapaddr".
	 */
	v = (caddr_t)(KERN_VBASE | (firstaddr * NBPG));
#define	valloc(name, type, num) \
	    (name) = (type *)(v); (v) = (caddr_t)((name)+(num))
#define	valloclim(name, type, num, lim) \
	    (name) = (type *)(v); (v) = (caddr_t)((lim) = ((name)+(num)))
	valloc(buf, struct buf, nbuf);
	valloc(swbuf, struct buf, nswbuf);
	valloclim(inode, struct inode, ninode, inodeNINODE);
	valloclim(pncc_base, struct ncblock, pncc_size, pncc_limit);
	valloclim(file, struct file, nfile, fileNFILE);
	valloclim(proc, struct proc, nproc, procNPROC);
	valloclim(text, struct text, ntext, textNTEXT);
#if NXNS > 0
	valloc(cfree, struct cblock, nclist);
#endif
	valloc(callout, struct callout, ncallout);
	nproctimes2 = nproc * 2;
	valloc(swapmap, struct map, nswapmap = nproctimes2);
	valloc(argmap, struct map, ARGMAPSIZE);
	valloc(kernelmap, struct map, nproctimes2);
#ifdef PROF
	valloc(profbuf, long, PROFSIZE/sizeof(long));
#endif

	/*
	 * Allocate space for bio memory management.  The bio stuff needs
	 * 3 data structures:
	 *	(a)	a "struct map" for allocating memory. The maximum
	 *		fragmentation for this structure is 100%, thus we
	 *		allocate 2 * nbuf slots.
	 *
	 *	(b)	a page table for each physical page.  The maximum
	 *		amount of physical memory fragmentation that this
	 *		policy will tolerate is 50% (old policy was 100%),
	 *		thus we allocate 1.5 times as much page tables as
	 *		we expect to use.
	 *
	 *	(c)	a reference count for each physical page.  Since the
	 *		underlying granularity of the bio memory allocation
	 *		is 512 bytes (BBSIZE), a count of the number of
	 *		references is needed to know when to free a page.
	 */
	bytes_of_cache = (nbuf * efs_lbsize * 3) / 2;
	pages = btoc(bytes_of_cache);
	i = nbuf + nbuf + 1;
	valloc(bio_memmap, struct map, i);
	valloc(bio_pt, struct pte, pages);
	valloc(bio_bbcnt, unsigned char, pages);
	rminit(bio_memmap, bytes_of_cache / BBSIZE, 1, "biomap", i);

	/*
	 * Now allocate space for core map:
	 * Allow space for all of phsical memory minus the amount 
	 * dedicated to the system including the cmap itself.  This
	 * is computed by dividing the remaining memory (before
	 * allocating the cmap), by the size of a cmap entry plus
	 * the size of the "core" it represents.  An extra entry
	 * with no associated "core" is tacked on as a header.
	 * Any leftover memory cannot be utilized in this scheme.
	 */
	ncmap = (ctob(totalpages) - ((long)v - KERN_VBASE + sizeof *cmap))
			/ (sizeof *cmap + NBPG) + 1;
	valloclim(cmap, struct cmap, ncmap, ecmap);
	unixend = totalpages - (ncmap - 1);

	init_malloc((char *)KERN_VBASE + ctob(unixend), (char *)KERN_VLIMIT);

	/* build board to cmap index conversion array */
	b = &btocmx[0];
	/*
	 * The pgtocm formula is:
	 *	(pf & 7f) + btocmx[pf >> 7].
	 * Thus we add in the base cmap index for a given board, to the
	 * offset into the board.  So if board 1 is located at cmap index
	 * 25, and we are using pf 0x83, we will refrence cmap index
	 *	25 + (0x83 & 0x7f) --> 28.
	 *
	 * Depending on the size of the kernel, btocmx[0] or btocmx[1] must
	 * be special.
	 *
	 * If the system uses less than 1/2meg of memory, then the
	 * offset into the first board will be added to btocmx[0]. Thus,
	 * btocmx[0] must be negative the amount of memory NOT available
	 * for usage in the first board.  This is done to counteract the
	 * addition of the first-usable-pages offset into the board.
	 * Example:  The system uses 0x60 pages of memory, from 0 to 0x5f.
	 * The first free page number is 0x60. Thus, in order for the cmap
	 * index of page 0x60 to be 1, we must set btocmx[0] to be -0x60 + 1.
	 * Thus the first board will use cmap index's 1 through 0x1f.
	 * The next memory board found, will then start at cmap index 0x20.
	 *
	 * If the system uses more than 1/2meg of memory, then the first
	 * btocmx slot should never be used, and thus it is set to garbage.
	 * The second slot, btocmx[1], should obviously be set to some negative
	 * value which for the first free page, will yield a cmap index of
	 * one.  The btcomx[1] value must necessaryily, because of the
	 * pgtocm formula, counteract the offset into the page.
	 * Example:  The system uses 0xc0 pages of memory, from 0 to 0xbf.
	 * The first free page number is 0xc0.  Thus in order for the cmap
	 * index of page 0xc0 to be 1, we must set btocmx[0] to
	 * -(0xc0 & 0x7f) + 1.  Thus, (0xc0 & 0x7f) + -0x40 + 1 --> 1.
	 */
	if (unixend < btop(HALFMEG)) {		/* first board is special */
		*b++ = -unixend + 1;
		cmx = 1 + btop(HALFMEG) - unixend;
	} else {				/* second board is special */
		*b++ = -32767;			/* set to wacko value */
		*b++ = -(unixend & 0x7f) + 1;
		cmx = 1 + btop(HALFMEG) - (unixend & 0x7f);
	}
	for (; b < &btocmx[NBOARDS]; b++) {
		if (*b) {
			*b = cmx;
			cmx += btop(HALFMEG);
		}
	}
	maxmem = physmem - unixend;

#ifdef	notdef
printf("&btocmx=%x\n", &btocmx[0]);
printf("maxmem=%x unixend=%x physmem=%x\n", maxmem, unixend, physmem);
	for (i = 0; i < 33; i++)
		printf("%04x ", (unsigned)btocmx[i]);
	printf("\n");
#endif

	/*
	 * Clear out unused portion of kernel pagemap
	 */
	pagemap = ((u_long *)PTMAP_BASE) + (KCX * PTEPCX) + unixend;
	for (i = (KCX * PTEPCX) + unixend;
	       i < (KCX * PTEPCX + btop(KERN_VSIZE)); i++) {
		*pagemap++ = 0;
	}

	/*
	 * Initialize callouts
	 */
	callfree = callout;
	for (i = 1; i < ncallout; i++)
		callout[i-1].c_next = &callout[i];

	/*
	 * Initialize memory allocator and swap
	 * and user page table maps.
	 *
	 * THE USER PAGE TABLE MAP IS CALLED ``kernelmap''
	 * WHICH IS A VERY UNDESCRIPTIVE AND INCONSISTENT NAME.
	 */
	meminit(maxmem, unixend);
	maxmem = freemem;
	if (freemem < KLMAX)
		panic("not enough memory");
	rminit(kernelmap, (long)USRPTSIZE-1, (long)1, "usrpt", nproctimes2);

	/*
	** Allow user access to the GE and FPA.  Enable external interrupts
	** from the multibus and other external interrupts.
	** Allow user access to the DC, UC, GF and UC DMA space 
	** 	as they say: "Power tools are dangerous"
	*/
	*STATUS_REG |= ST_IP2ACC | ST_EINTR | ST_EXINTR |
		       ST_GEUACC;
	*MBP_REG = (MBP_DCACC | MBP_UCACC | MBP_GFACC | MBP_DMACC); 

	/*
	** set info into the 50bytes non-volatle RAW within the clock/tod
	** device:
	**	Indicated we have been booted.
	**	Zero powerfail flag.
	**	Zero watchdog timeout flag.
	*/
	todsetflg( TD_BTFLG, (char)1 );	
	todsetflg( TD_PWRFLG, (char)0 );
	wtimeout = todsetflg( TD_WTIMFLG, (char)0 );

#ifndef KOPT_NOGL
	/* setup global variables that come from the PROMS	*/
	_dcrmodes = (char)_commdat->c_dcconfig;
	_dcrflags = (short)_commdat->c_flags;
#endif
	havefpa =  _commdat->c_havefpa;

	/*
	** if the fpa hardware is present, then we allow user access to
	** it
	*/
	if ( havefpa )
		*STATUS_REG |= ST_FPAUACC;

	con_init();
}

/*
 * Initialize core map
 */
meminit(npages, firstpf)
	register short npages;
	short firstpf;
{
	register int i, nextpf, amountleft;
	register struct cmap *c;
	register short *b;

	b = &btocmx[0];
	if ( firstpf > btop(HALFMEG) )
	{
		b++;
		amountleft = btop(ONEMEG) - firstpf;
	}
	else
		amountleft = btop(HALFMEG) - firstpf;
	nextpf = firstpf;

	firstfree = 0;				/* XXX */
	freemem = maxfree = npages;
	ecmx = ecmap - cmap;
	if (ecmx < freemem)
		freemem = ecmx;
	for (i = 1; i <= freemem; i++) {
		cmap[i-1].c_next = i;
		c = &cmap[i];
		c->c_prev = i-1;
		c->c_free = 1;
		c->c_gone = 1;
		c->c_type = CSYS;
		if (amountleft == 0) {
			do {
				b++;
			} while (*b == 0);
			if (b == &btocmx[NBOARDS])
				panic("meminit");
			amountleft = btop(HALFMEG);
			nextpf = (b - &btocmx[0]) << 7;
		}
		c->c_pfnum = nextpf++;
		amountleft--;
	}
	cmap[freemem].c_next = CMHEAD;
	cmap[CMHEAD].c_prev = freemem;
	cmap[CMHEAD].c_type = CSYS;
	avefree = freemem;
}

#ifdef	INET
/*
 * useracc:
 *	- see if we can access the user virtual space from vaddr through
 *	  vaddr + count
 *	- how is ``B_READ'' or ``B_WRITE''
 */
useracc(vaddr, count, how)
	register long vaddr;
	register long count;
	int how;
{
	register struct proc *p;

    /* first make sure its inside our software limits */
	p = u.u_procp;
	if ((vaddr < ctob(p->p_loadc)) ||
	    (vaddr + count > USRSTACK) ||
	    ((vaddr + count > ctob(p->p_loadc + u.u_tsize + u.u_dsize)) &&
	     (vaddr < (USRSTACK - ctob(u.u_ssize)))) || (vaddr >= USRSTACK))
		return (0);			/* no good */

    /* if this is a read check, we can return now */
	if (how == B_READ)
		return (1);			/* okay */

    /* make sure that vaddr is outside of text region for writes */
	if (vaddr < ctob(p->p_loadc + u.u_tsize))
		return (0);			/* no good */
	return (1);				/* okay */
}
#endif

/* 
 * iopagein:
 *	- pagein a virtual segment for copyin/copyout
 */
struct pte *
iolock(base, count)
	register caddr_t base;
	register long count;
{
	register struct pte *pte, *pte0;
	register struct proc *p;
	register short npf;
	register struct cmap *c;
	long oldphysio;

    /* verify user address */
	if ((base < (caddr_t)u.u_loadaddr) ||
	    (base + count > (caddr_t)USRSTACK) ||
	    ((base + count >
		   (caddr_t)u.u_loadaddr + ctob(u.u_tsize + u.u_dsize)) &&
	     (base < (caddr_t)(USRSTACK - ctob(u.u_ssize)))) ||
	    (base >= (caddr_t)USRSTACK) ||
	    (count < 0)) {
		return NULL;				/* no good */
	}

	p = u.u_procp;
	oldphysio = p->p_flag & SPHYSIO;
	p->p_flag |= SPHYSIO;
	pte0 = pte = vtopte(p, btop(base));
	npf = btoc(count + ((int)base & PGOFSET));
	while (npf > 0) {
		if (pte->pg_v) {
			c = &cmap[pgtocm(pte->pg_pfnum)];

			/*
			 * If page has some iolocks but no lock, then somebody
			 * blew it.
			 */
			if (c->c_iolocks && !c->c_lock)
				panic("iolock iolocks");
			/*
			 * If page is locked and has no i/o locks on it, then
			 * the pager must have it.  Wait for pager to let go
			 * of the lock.  Once the pager does let go, start over
			 * at the top, because the page may no longer be valid.
			 */
			if (c->c_lock && (c->c_iolocks == 0)) {
				c->c_want = 1;
				sleep((caddr_t)c, PSWP+1);
				continue;
			}
			c->c_lock = 1;
		} else {
			if (pagein((long)base, 1))
				panic("pagein v");	/* return it locked */
			c = &cmap[pgtocm(pte->pg_pfnum)];
		}
		/*
		 * Advance number of i/o locks on the page.  iounlock() will
		 * be called to unlock the pages, and will reduce the
		 * iolocks count.
		 */
		c->c_iolocks++;
		pte++;
		base += NBPG;
		npf--;
	}

    /* see if we lost our context during the pagein; get one if we did */
	if (p->p_flag & (SPTECHG|SLOSTCX))
		sureg(1);
	ASSERT(p->p_flag & SPHYSIO);
	if (!oldphysio)
		p->p_flag &= ~SPHYSIO;
	return pte0;
}

#ifdef	GL2
/* 
 * iochecklock:
 *	- pagein a virtual segment for copyin/copyout
 */
struct pte *
iochecklock(p, base, count)
	register struct proc *p;
	register unsigned base;
	register long count;
{
	register struct pte *pte, *pte0;
	register short npf;

	/* verify user address */
	if ((base < ctob(p->p_loadc)) || (base + count > USRSTACK) ||
	    ((base + count > ctob(p->p_loadc + p->p_tsize + p->p_dsize)) &&
	     (base < (USRSTACK - ctob(p->p_ssize)))) || (base >= USRSTACK)) {
		printf("pid=%d base=%x count=%d load=%d ts=%d ds=%d ss=%d\n",
			       p->p_pid, base, count, p->p_loadc,
			       p->p_tsize, p->p_dsize, p->p_ssize);
		return (0);
	}

	pte0 = pte = vtopte(p, btop(base));
	npf = btoc(count + ((int)base & PGOFSET));
	while (npf > 0) {
		register struct cmap *c;

		if (!pte->pg_v) {
			printf("pte=%x\n", pte);
			debug("iochecklock !v");
		}

		c = &cmap[pgtocm(pte->pg_pfnum)];
		if (!c->c_lock && c->c_iolocks) {
			printf("pte=%x *pte=%x\n", pte, *(long *)pte);
			debug("iochecklock !lock");
		}
		pte++;
		base += NBPG;
		npf--;
	}
	return (pte0);
}
#endif

/* 
 * iounlock:
 *	- unlock a virtual segment during i/o (like vsunlock, but used
 *	  to simulate kernel virtual i/o)
 */
iounlock(pte, base, count, rw)
	register struct pte *pte;
	unsigned base;
	long count;
	register short rw;
{
	register short npf;

	npf = btoc(count + ((int)base & PGOFSET));
	while (npf > 0) {
		/* XXX should expand munlock here */
		munlock(pte->pg_pfnum);
		if (rw == B_READ)	/* Reading from device writes memory */
			pte->pg_m = 1;
		pte++;
		npf--;
	}
}

/*
 * copyin:
 *	- read in count bytes of data from "from" (in the users address space)
 *	  to "to" (in the kernels address space)
 *	- pmem() in mem.c undestands that we only use SCRPG0 and SCRPG1
 */
int
copyin(from, to, count)
	caddr_t from;			/* don't register */
	caddr_t to;
	register int count;
{
	struct pte *pte;		/* don't register */
	register u_long *pagemap;
	int amount;			/* don't register */
	long ptes[2];
	int error;
	jmp_buf jb;
	int *saved_jb;

	/* save old info */
	pagemap = ((u_long *)PTMAP_BASE) + (btop(SCRPG0_VBASE & ~SEG_MSK) +
		  (KCX << PPCXLOG2));
	ptes[0] = *pagemap;
	ptes[1] = *(pagemap + 1);

	saved_jb = nofault;
#ifdef	lint
	amount = amount;
	pte = pte;
#endif
	if (setjmp(jb)) {
		/*
		 * If we get here, user gave us a bogus address to use
		 * that iolock didn't catch.  Unlock pages and
		 * return an error.
		 */
		iounlock(pte, (unsigned)from, (long)amount, B_WRITE);
		error = -1;
		goto out;
	} else
		nofault = jb;

	error = 0;
	while (count) {
		amount = count;
		if (amount > NBPG)
			amount = NBPG;

		/* validate pages to be used during copy */
		if ((pte = iolock(from, (long)amount)) == NULL) {
			error = -1;
			break;
		}

		*pagemap = pte->pg_pfnum | PG_KW;
		*(pagemap + 1) = ((pte + 1)->pg_pfnum) | PG_KW;

		bcopy((caddr_t) (SCRPG0_VBASE + ((long)from & PGOFSET)),
			to, amount);
		iounlock(pte, (unsigned)from, (long)amount, B_WRITE);
		to += amount;
		from += amount;
		count -= amount;
	}

out:
	/* restore info	*/
	nofault = saved_jb;
	*pagemap++ = ptes[0];
	*pagemap = ptes[1];
	return (error);
}

/*
 * copyout:
 *	- write out count bytes of data from "from" (in the kernels
 *	  address space) to "to" (in the users address space)
 *	- pmem() in mem.c undestands that we only use SCRPG0 and SCRPG1
 */
int
copyout(from, to, count)
	register caddr_t from;
	caddr_t to;			/* don't register */
	register int count;
{
	struct pte *pte;		/* don't register */
	register u_long *pagemap;
	int amount;			/* don't register */
	long ptes[2];
	int error;
	jmp_buf jb;
	int *saved_jb;

	/* save old scratch info */
	pagemap = ((u_long *)PTMAP_BASE) + (btop(SCRPG0_VBASE & ~SEG_MSK) +
		  (KCX << PPCXLOG2));
	ptes[0] = *pagemap;
	ptes[1] = *(pagemap + 1);

	saved_jb = nofault;
#ifdef	lint
	amount = amount;
	pte = pte;
#endif
	if (setjmp(jb)) {
		/*
		 * If we get here, user gave us a bogus address to use
		 * that iolock didn't catch.  Unlock pages and
		 * return an error.
		 */
		iounlock(pte, (unsigned)to, (long)amount, B_READ);
		error = -1;
		goto out;
	} else
		nofault = jb;

	error = 0;
	while (count) {
		amount = count;
		if (amount > NBPG)
			amount = NBPG;

		/* validate pages to be used during copy */
		if ((pte = iolock(to, (long)amount)) == NULL) {
			error = -1;
			break;
		}

		*pagemap = pte->pg_pfnum | PG_KW;
		*(pagemap + 1) = ((pte + 1)->pg_pfnum) | PG_KW;

		bcopy(from, (caddr_t) (SCRPG0_VBASE + ((long)to & PGOFSET)),
				amount);
		iounlock(pte, (unsigned)to, (long)amount, B_READ);
		to += amount;
		from += amount;
		count -= amount;
	}

out:
	nofault = saved_jb;
	*pagemap++ = ptes[0];
	*pagemap = ptes[1];
	return (error);
}

/*
 * copyseg:
 *	- copy one page of data from user vaddr "fromv" to page frame "topf"
 *	- this is used by vmdup() only! (and this code understands that!)
 */
copyseg(from, topf)
	caddr_t from;
	u_int topf;
{
	register struct pte *pte;
	register u_long *pagemap;
	long ptes[2];

	/* save old mapping */
	pagemap = ((u_long *)PTMAP_BASE) + (btop(SCRPG0_VBASE & ~SEG_MSK) +
		  (KCX << PPCXLOG2));
	ptes[0] = *pagemap;
	ptes[1] = *(pagemap + 1);

	/* validate pages to be used during copy */
	if ((pte = iolock(from, (long)NBPG)) == NULL)
		panic("copyseg");

	*pagemap = pte->pg_pfnum | PG_KW;
	*(pagemap + 1) = (topf) | PG_KW;

	/* copy the memory */
	bcopyPAGE((caddr_t)SCRPG0_VBASE, (caddr_t)SCRPG1_VBASE);
	iounlock(pte, (unsigned)from, (long)NBPG, B_WRITE);

	/* restore old mapping */
	*pagemap++ = ptes[0];
	*pagemap = ptes[1];
}

/*
 * clearseg - Clear one click's worth of data.
 */
clearseg(pfnum)
	unsigned pfnum;
{
	register u_long *pagemap;
	long ptes;

    /* save old SCRPG0 pte info */
	pagemap = ((u_long *)PTMAP_BASE) + (btop(SCRPG0_VBASE & ~SEG_MSK) +
		  (KCX << PPCXLOG2));
	ptes = *pagemap;

    /* load up new info */
	*pagemap = pfnum | PG_KW;

    /* zero out the page */
	bzeroPAGE((caddr_t)SCRPG0_VBASE);

    /* restore old mapping */
	*pagemap = ptes;
}

fubyte(v)
	caddr_t v;
{
	register struct pte *pte;
	register u_long *pagemap;
	long ptes;
	u_char x;

	pagemap = ((u_long *)PTMAP_BASE) + (btop(SCRPG0_VBASE & ~SEG_MSK) +
		  (KCX << PPCXLOG2));
	ptes = *pagemap;

    /* validate page to be used during copy */
	if ((pte = iolock(v, (long)sizeof(x))) == NULL)
		return -1;

	*pagemap = pte->pg_pfnum | PG_KW;

	x = *(char *)(SCRPG0_VBASE + ((long)v & PGOFSET));
	iounlock(pte, (unsigned)v, (long)sizeof(x), B_WRITE);

	*pagemap = ptes;
	return x;
}

subyte(v, x)
	caddr_t v;
	char x;
{
	register struct pte *pte;
	register u_long *pagemap;
	long ptes;

	pagemap = ((u_long *)PTMAP_BASE) + (btop(SCRPG0_VBASE & ~SEG_MSK) +
		  (KCX << PPCXLOG2));
	ptes = *pagemap;

    /* validate page to be used during copy */
	if ((pte = iolock(v, (long)sizeof(x))) == NULL)
		return -1;

	*pagemap = pte->pg_pfnum | PG_KW;

	*(char *)(SCRPG0_VBASE + ((long)v & PGOFSET)) = x;
	iounlock(pte, (unsigned)v, (long)sizeof(x), B_READ);

	*pagemap = ptes;
	return 0;
}

fuword(v)
	caddr_t v;
{
	register struct pte *pte;
	register u_long *pagemap;
	long ptes[2];
	int x;

	pagemap = ((u_long *)PTMAP_BASE) + (btop(SCRPG0_VBASE & ~SEG_MSK) +
		  (KCX << PPCXLOG2));
	ptes[0] = *pagemap;
	ptes[1] = *(pagemap + 1);

    /* validate pages to be used during copy */
	if ((pte = iolock(v, (long)sizeof(x))) == NULL)
		return -1;

	*pagemap = pte->pg_pfnum | PG_KW;
	*(pagemap + 1) = ((pte + 1)->pg_pfnum) | PG_KW;

	x = *(int *)(SCRPG0_VBASE + ((long)v & PGOFSET));
	iounlock(pte, (unsigned)v, (long)sizeof(x), B_WRITE);

	*pagemap++ = ptes[0];
	*pagemap = ptes[1];
	return x;
}

suword(v, x)
	caddr_t v;
	int x;
{
	register struct pte *pte;
	register u_long *pagemap;
	long ptes[2];

	pagemap = ((u_long *)PTMAP_BASE) + (btop(SCRPG0_VBASE & ~SEG_MSK) +
		  (KCX << PPCXLOG2));
	ptes[0] = *pagemap;
	ptes[1] = *(pagemap + 1);

    /* validate pages to be used during copy */
	if ((pte = iolock(v, (long)sizeof(x))) == NULL)
		return -1;

	*pagemap = pte->pg_pfnum | PG_KW;
	*(pagemap + 1) = ((pte + 1)->pg_pfnum) | PG_KW;

	*(int *)(SCRPG0_VBASE + ((long)v & PGOFSET)) = x;
	iounlock(pte, (unsigned)v, (long)sizeof(x), B_READ);

	*pagemap++ = ptes[0];
	*pagemap = ptes[1];
	return 0;
}

#ifdef	GL2
void
sushort(p, v, x)
	struct proc *p;
	caddr_t v;
	short x;
{
	register struct pte *pte;
	register u_long *pagemap;
	long ptes;

	pagemap = ((u_long *)PTMAP_BASE) + (btop(SCRPG0_VBASE & ~SEG_MSK) +
		  (KCX << PPCXLOG2));
	ptes = *pagemap;

    /* validate pages to be used during copy */
	pte = iochecklock(p, (unsigned)v, (long)sizeof(x));

	*pagemap = pte->pg_pfnum | PG_KW;

	*(short *)(SCRPG0_VBASE + ((long)v & PGOFSET)) = x;

	*pagemap = ptes;
}

/*
 * sustuff:
 *	- store a bunch of stuff from the fbc into user memory
 *	  for process "p" at virtual address "v" of "n" shorts
 *	  and with the fbc at address "x"
 */
sustuff(p, v, n, x)
	struct proc *p;
	caddr_t v;
	short n;
	short *x;
{
	register short *to;
	register u_long *pagemap;
	register struct pte *pte;
	long ptes[2];

	pagemap = ((u_long *)PTMAP_BASE) + (btop(SCRPG0_VBASE & ~SEG_MSK) +
		  (KCX << PPCXLOG2));
	ptes[0] = *pagemap;
	ptes[1] = *(pagemap + 1);

    /* validate pages to be used during copy */
	pte = iochecklock(p, (unsigned)v, (long)(n*sizeof(short)));

	*pagemap = pte->pg_pfnum | PG_KW;
	*(pagemap + 1) = ((pte + 1)->pg_pfnum) | PG_KW;

    /* copy data from fbc to user memory */
	to = (short *)(SCRPG0_VBASE + ((long)v & PGOFSET));
	while (n--)
		*to++ = *x;

	*pagemap++ = ptes[0];
	*pagemap = ptes[1];
}
#endif	GL2

/*
 * fustring:
 *	- read in up to maxspace bytes of data from "from", in the users
 *	  address space, to "to" in the kernels address space
 *	- pmem() in mem.c undestands that we only use SCRPG0 and SCRPG1
 *	- this is used by exece to read a users string in
 *	- we return negative of the amount of characters copied if we hit
 *	  a null in the string; otherwise, we just return the number of
 *	  characters copied. Means we used up maxspace, or ran off end of
 *	  a page.
 */
fustring(from, to, maxspace)
	caddr_t from;
	register char *to;
	register int maxspace;
{
	register char *fromp;
	register u_long *pagemap;
	struct pte *pte;
	long ptes;
	register char c;
	register int amount;

    /* check for end of page boundary */
	if ((amount = (NBPG - ((long)from & PGOFSET))) < maxspace)
		maxspace = amount;

    /* save old scratch page mappings */
	pagemap = ((u_long *)PTMAP_BASE) + (btop(SCRPG0_VBASE & ~SEG_MSK) +
		  (KCX << PPCXLOG2));
	ptes = *pagemap;

    /* validate pages to be used during copy; setup scratch pages */
	if ((pte = iolock(from, (long)maxspace)) == NULL) {
		u.u_error = EFAULT;
		return 0;
	}
	*pagemap = pte->pg_pfnum | PG_KW;

	amount = 0;
	fromp = (char *)SCRPG0_VBASE + ((long)from & PGOFSET);
	for (;;) {
		c = *fromp++;
		*to++ = c;
		amount++;
		if (c == 0) {
			amount = -amount;
			break;
		}
		if (amount >= maxspace)
			break;
	}

    /* release page locks; restore scratch page mapping */
	iounlock(pte, (unsigned)from, (long)maxspace, B_WRITE);
	*pagemap = ptes;

	return amount;
}

/*
 * sustring:
 *	- write out up to maxspace bytes of data from "from", in the kernels
 *	  address space, to "to" in the users address space
 *	- pmem() in mem.c undestands that we only use SCRPG0 and SCRPG1
 *	- this is used by exece to read a users string in
 *	- we return negative of the amount of characters copied if we hit
 *	  a null in the string; otherwise, we just return the number of
 *	  characters copied. Means we used up maxspace, or ran off end of
 *	  a page.
 */
sustring(from, to, maxspace)
	register char *from;
	caddr_t to;
	register int maxspace;
{
	register char *top;
	register u_long *pagemap;
	struct pte *pte;
	long ptes;
	register char c;
	register int amount;

    /* check for end of page boundary */
	if ((amount = (NBPG - ((long)to & PGOFSET))) < maxspace)
		maxspace = amount;

    /* save old scratch page mappings */
	pagemap = ((u_long *)PTMAP_BASE) + (btop(SCRPG0_VBASE & ~SEG_MSK) +
		  (KCX << PPCXLOG2));
	ptes = *pagemap;

    /* validate pages to be used during copy; setup scratch pages */
	if ((pte = iolock(to, (long)maxspace)) == NULL) {
		u.u_error = EFAULT;
		return 0;
	}
	*pagemap = pte->pg_pfnum | PG_KW;

	amount = 0;
	top = (char *)SCRPG0_VBASE + ((long)to & PGOFSET);
	for (;;) {
		c = *from++;
		*top++ = c;
		amount++;
		if (c == 0) {
			amount = -amount;
			break;
		}
		if (amount >= maxspace)
			break;
	}

    /* release page locks; restore scratch page mapping */
	iounlock(pte, (unsigned)to, (long)maxspace, B_READ);
	*pagemap = ptes;

	return amount;
}

/*
 * fsstring:
 *	- like fustring, except copy from system space to system space
 */
fsstring(from, to, maxspace)
	register char *from;
	register char *to;
	register int maxspace;
{
	register char c;
	register int amount;

	amount = 0;
	for (;;) {
		c = *from++;
		*to++ = c;
		amount++;
		if (c == 0) {
			amount = -amount;
			break;
		}
		if (amount >= maxspace)
			break;
	}

	return amount;
}

/*
 * addupc - Take a profile sample.
 */
addupc(userpc, p, incr)
unsigned userpc;
register struct {
	short	*pr_base;
	unsigned pr_size;
	unsigned pr_off;
	unsigned pr_scale;
} *p;
{
	union {
		int w_form;		/* this is 32 bits on 68000 */
		ushort s_form[2];
	} word;
	register caddr_t slot;
	union {
		long	longword;
		ushort	shortword[2];
	} pc, scale;
	register int temp;

	/*
	 * Copy users pc and scale into union's defined above.  We do this
	 * so that an extended precision multiply can be done.  Please note
	 * that the short order is machine dependent.
	 */
	pc.longword = userpc - p->pr_off;
	scale.longword = p->pr_scale;
	temp = ((pc.shortword[1] * scale.shortword[1]) >> 16) +
		(pc.shortword[0] * scale.shortword[1]) +
		(pc.shortword[1] * scale.shortword[0]);

	/*
	 * Compute address in users profile buffer of the histogram entry
	 * (which is a ushort).
	 */
#ifdef	notdef
	slot = (caddr_t) &p->pr_base[((((pc - p->pr_off) * p->pr_scale) >> 16) + 1)>>1];
#else
	slot = (caddr_t) &p->pr_base[(temp + 1) >> 1];
#endif
	if ((slot >= (caddr_t)p->pr_base) &&
	    (slot < (caddr_t)p->pr_base + p->pr_size)) {
		if ((word.w_form = fuword(slot)) == -1)
			u.u_prof.pr_scale = 0;	/* turn off */
		else {
			word.s_form[0] += (ushort)incr;
			(void) suword(slot, word.w_form);
		}
	}
}

/*
 * sendsig - Simulate an interrupt.
 */
/* ARGSUSED */
sendsig(p, signo)
caddr_t p;
{
	register caddr_t newsp;
	register int *regp;
	struct {
		short	sr;
		long	pc;
	} f;

	regp = u.u_ar0;

	/*
	 * If user stack is not large enough for this signal frame,
	 * grow the stack.
	 */
	newsp = (caddr_t) (regp[SP] - 6);
	if (newsp <= (caddr_t) (USRSTACK - ctob(u.u_ssize))) {
		if (grow((unsigned)newsp) == 0) {
		    uprintf("can't send signal to pid %d - can't grow stack\n",
				   u.u_procp->p_pid);
		}
	}

	f.sr = regp[RPS];
	f.pc = regp[PC];
	if (copyout((caddr_t)&f, newsp, sizeof(f))) {
		/*
		 * Can't send the signal to the user.  Process must have
		 * clobbered its stack.  Re-enable the illegal instruction
		 * signal, and then give it one.
		 */
		u.u_signal[SIGILL-1] = (int) (SIG_DFL);
		psignal(u.u_procp, SIGILL);
		return;
	}
	regp[SP] = (int)newsp;
	regp[RPS] &= ~PS_T;
	regp[PC] = (int)p;
}

/*
 * Clear registers on exec
 */
setregs()
{
	register char *cp;
	register int *rp;
	register struct file **fpp;

	for (rp = &u.u_signal[0]; rp < &u.u_signal[NSIG]; rp++)
		if ((*rp & 1) == 0)
			*rp = 0;
	for (cp = &regloc[0]; cp < &regloc[15]; )
		u.u_ar0[*cp++] = 0;
	u.u_ar0[PC] = u.u_exdata.ux_entloc;

	fpp = &u.u_ofile[0];
	for (cp = &u.u_pofile[0]; cp < &u.u_pofile[NOFILE];) {
		if ((*cp++ & EXCLOSE) && *fpp) {
			closef(*fpp);
			*fpp = NULL;
		}
		fpp++;
	}

	/*
	 * Remember file name for accounting.
	 */
	u.u_acflag &= ~AFORK;
	bcopy((caddr_t)u.u_dent.d_name, (caddr_t)u.u_comm, DIRSIZ);
}

/*
 * IM1 memory board registers.  Each IM1 memory board has these registers
 * in multibus i/o space.
 */
struct memregs {
	ushort	mr_ras;		/* ras register */
	ushort	mr_cas;		/* cas register */
	ushort	mr_parity;	/* parity register */
	ushort	mr_clear;	/* clear's error when read */
};

#define	PARITY_STRIPNOISE	0x01FF		/* mask to remove noise bits */
#define	PARITY_RAS_PADDR	0x01FF
#define	PARITY_CAS_PADDR_2MB	0x01FF
#define	PARITY_CAS_PADDR_4MB	0x01FE
#define	PARITY_CAS_BIT21	0x0001
#define	PARITY_PAR_BIT11	0x0001		/* active low */
#define	PARITY_PAR_BIT20	0x0002		/* active low */
#define	PARITY_PAR_BYTE0	0x0080		/* active low */
#define	PARITY_PAR_BYTE1	0x0040		/* active low */
#define	PARITY_PAR_BYTE2	0x0020		/* active low */
#define	PARITY_PAR_BYTE3	0x0010		/* active low */
#define	PARITY_PAR_PROCESSOR	0x0100

/*
 * This points to a jmp_buf when some code wants to catch parity errors
 * in a non-default manner.
 */
int *parityFault;

int parityErrorRecoverable;

void
logParityError(paddr)
	long paddr;
{
}

/*
 * Print out to both the serial port and the console port.  If the
 * console port is the serial port, only print once.
 */
void
pprintf(fmt, x1)
	char *fmt;
	int x1;
{
	doprnt(duputchar, fmt, &x1);
	if (con_putchar != duputchar)
		doprnt(con_putchar, fmt, &x1);
}

/*
 * Halt.
 */
void
parityHalt()
{
	pprintf("halting...\n");
	halt();
}

/*
 * Gather information from a parity error from a particular memory
 * board.  If the board has an error registered, return 1.  If no
 * error has occured on this board, return 0.
 */
int
parityGather(board, physaddr, ras, cas, par)
	int board;
	long *physaddr;
	ushort *ras, *cas, *par;
{
	register struct memregs *memreg;
	register long addr;

	if ((membdsz[board] & 0xFF) == 0xFF) {
		/* no memory here */
		return 0;
	}

	memreg = ((struct memregs *) IM1REG_VBASE) + board;
	if ((memreg->mr_parity & 0x4) == 0) {
		/* no error on this board */
		return 0;
	}

	/* get info from hardware registers */
	*ras = memreg->mr_ras & PARITY_STRIPNOISE;
	*cas = memreg->mr_cas & PARITY_STRIPNOISE;
	*par = memreg->mr_parity & PARITY_STRIPNOISE;

	/*
	 * Compute the physical address.  The ras register contains the
	 * address bits 2-10.
	 */
	addr = (*ras & PARITY_RAS_PADDR) << 2;
	if ((*par & PARITY_PAR_BIT20) == 0)
		addr |= 1<<20;
	if (membdsz[board] == 0) {
		/* 2mb board */
		addr |= (*cas & PARITY_CAS_PADDR_2MB) << 11;
	} else {
		/* 4mb board */
		addr |= (*cas & PARITY_CAS_PADDR_4MB) << 11;
		if (*cas & PARITY_CAS_BIT21)
			addr |= 1<<21;
		if ((*par & PARITY_PAR_BIT11) == 0)
			addr |= 1<<11;
	}
	addr += board * (4 * ONEMEG);
	*physaddr = addr;
	addr = memreg->mr_clear;		/* clear error */
	return 1;
}

/*
 * Given the value of the mr_parity register, convert it to an ascii
 * description of which dram chips are affected.
 */
void
convertToChips(buf, par)
	register char *buf;
	register ushort par;
{
	/*
	 * The values in these arrays are the starting chip number for
	 * each bank of rams.  The bit11and20 value is used to select which
	 * chip range to use.  The parity register bits BYTE0-BYTE3 are
	 * used to select which array to use (note that multiple bytes
	 * may have errors).  The chips are numbered sequentially starting
	 * from the value in the array to the value plus 8, inclusive
	 * (one extra bit for parity).
	 */
	static short byte0[4] = { 162, 120, 76, 34 };
	static short byte1[4] = { 151, 109, 67, 23 };
	static short byte2[4] = { 184, 140, 98, 56 };
	static short byte3[4] = { 173, 131, 87, 45 };
	char temp[20];
	register int bit11and20;
	register int b;

	/*
	 * Bits 11 and 20 of the physical address are used as megabyte
	 * selections on the 4mb board.
	 *	bit 11		bit 20		Megabyte
	 *	------		------		--------
	 *	   0		   0		    0
	 *	   0		   1		    1
	 *	   1		   0		    2
	 *	   1		   1		    3
	 */
	bit11and20 = ((~par & PARITY_PAR_BIT11) << 1) |
		     ((~par & PARITY_PAR_BIT20) >> 1);
	buf[0] = 0;
	if ((par & PARITY_PAR_BYTE0) == 0) {
		b = byte0[bit11and20];
		sprintf(temp, " U%d-%d", b, b + 8);
		strcat(buf, temp);
	}
	if ((par & PARITY_PAR_BYTE1) == 0) {
		b = byte1[bit11and20];
		sprintf(temp, " U%d-%d", b, b + 8);
		strcat(buf, temp);
	}
	if ((par & PARITY_PAR_BYTE2) == 0) {
		b = byte2[bit11and20];
		sprintf(temp, " U%d-%d", b, b + 8);
		strcat(buf, temp);
	}
	if ((par & PARITY_PAR_BYTE3) == 0) {
		b = byte3[bit11and20];
		sprintf(temp, " U%d-%d", b, b + 8);
		strcat(buf, temp);
	}
}

/*
 * Print a well formatted parity error message
 */
void
parityPrint(board, ras, cas, par, paddr)
	int board;
	ushort ras, cas, par;
	long paddr;
{
	char buf[4*20];
	register ulong *pagemap;
	long oldPagemapValue;
	long value;

	/*
	 * Get value that physical address points to so that we
	 * can print it.  We know that we can't get another parity error
	 * at this point because such errors are disabled.
	 */
	pagemap = ((ulong *)PTMAP_BASE) + (btop(SCRPG0_VBASE & ~SEG_MSK) +
		  (KCX << PPCXLOG2));
	oldPagemapValue = *pagemap;
	*pagemap = btop(paddr) | PG_KR;
	value = *(long *)(SCRPG0_VBASE + (paddr & PGOFSET));
	*pagemap = oldPagemapValue;

	convertToChips(buf, par);
	pprintf("Board %d: ras=%04x cas=%04x parity=%04x chips=%s\n",
		       board, ras, cas, par, buf);
	pprintf("Board %d: physical address=%08x, contents=%08x,  byte(s):",
		       board, paddr, value);
	if ((par & PARITY_PAR_BYTE0) == 0) pprintf(" 0");
	if ((par & PARITY_PAR_BYTE1) == 0) pprintf(" 1");
	if ((par & PARITY_PAR_BYTE2) == 0) pprintf(" 2");
	if ((par & PARITY_PAR_BYTE3) == 0) pprintf(" 3");
	if ((par & PARITY_PAR_PROCESSOR) == 0)
		pprintf(" - multibus cycle");
	pprintf("\n");
}

/*
 * Scan a page table, looking for a particular page frame.
 */
int
pageInPageTable(pf, pt, n)
	register long pf;
	register struct pte *pt;
	register int n;
{
	while (--n >= 0) {
		if (pt->pg_pfnum == pf)
			return 1;
	}
	return 0;
}

/*
 * Find user of a page thats in the Usrptmap.  Because the bio page
 * table has already been scanned, we don't need to look there.
 */
void
parityFindUserOfUsrptmap(kmx)
	int kmx;
{
	register struct proc *p;
	register long vaddr;

	vaddr = (long) kmxtob(kmx);
	/*
	 * Scan process table, looking for a process that is using
	 * this page for its page tables.
	 */
	for (p = proc; p < procNPROC; p++) {
		if (p->p_stat) {
			if ((vaddr >= (long)p->p_p0br) &&
			    (vaddr < (long)p->p_p0br + ctob(p->p_szpt))) {
				pprintf("used by page tables for pid %d\n",
					      p->p_pid);
				return;
			}
		}
	}
	pprintf("unknown usage (mbuf?)\n");
}

/*
 * Find the user of a given physical page, and print it out.
 * Root around in kernel page tables.
 */
void
parityFindUserOfPage(board, paddr)
	int board;
	long paddr;
{
	register long pf;
	register struct cmap *c;
	register int i;
	register struct proc *p;
	extern struct pte *kheap_pte;

	pprintf("Board %d: Page is ", board);
	pf = btop(paddr);
	if (pf < cmap[1].c_pfnum) {
		pprintf("used by the kernel text/data\n");
		return;
	}
	if (pf > cmap[maxfree].c_pfnum) {
		pprintf("outside legal memory range!\n");
		return;
	}

	c = &cmap[pgtocm(pf)];
	switch (c->c_type) {
	  case CTEXT:
		pprintf("used by user text\n");
		return;
	  case CDATA:
	  case CSTACK:
		pprintf("used by process %d\n", proc[c->c_ndx].p_pid);
		return;
	}

	if (pageInPageTable(pf, bio_pt, btoc((nbuf * efs_lbsize * 3) / 2))) {
		pprintf("used by the buffer cache\n");
		return;
	}
	if (pageInPageTable(pf, kheap_pte, 128)) {
		pprintf("used by kern_malloc\n");
		return;
	}

	/*
	 * Scan process table, looking to see if the page is being used
	 * by a process's udot.
	 */
	for (p = proc; p < procNPROC; p++) {
		if (p->p_stat) {
			if (p->p_addr->pg_pfnum == pf) {
				pprintf("used by udot for process %d\n",
					      p->p_pid);
				return;
			}
		}
	}

	/*
	 * Scan Usrptmap.  If we find the page in the Usrptmap, the
	 * figure out which consumer of the Usrptmap is using the
	 * page.
	 */
	for (i = 0; i < USRPTSIZE; i++) {
		if (Usrptmap[i].pg_pfnum == pf) {
			pprintf("in Usrptmap: ");
			parityFindUserOfUsrptmap(i);
			return;
		}
	}
	pprintf("used by kernel somewhere\n");
}

/*
 * Kill the process p.  Print a message on the console and the serial
 * console.
 */
void
parityKillProcess(p)
	register struct proc *p;
{
	char name[DIRSIZ+1];

	/*
	 * Find name of process
	 */
	if (p->p_addr && p->p_addr->pg_pfnum) {
	    register ulong *pagemap;
	    ulong oldPagemapValue;

	    pagemap = ((ulong *)PTMAP_BASE) + (btop(SCRPG0_VBASE & ~SEG_MSK) +
		      (KCX << PPCXLOG2));
	    oldPagemapValue = *pagemap;
	    *pagemap = p->p_addr->pg_pfnum | PG_KR;
	    bcopy(((struct user *)SCRPG0_VBASE)->u_comm, name, DIRSIZ);
	    name[DIRSIZ] = 0;
	    *pagemap = oldPagemapValue;
	} else {
	    name[0] = 0;
	}
	psignal(p, SIGKILL);
	pprintf("Process %d (%s) killed due to parity error in its memory.\n",
			 p->p_pid, name);
}

/*
 * Find out what's using the page.  If its a user, kill the process.  If
 * its the kernel, note that the error is unrecoverable.
 */
void
checkForPageUsedByKernel(paddr)
	long paddr;
{
	register long pf;
	register struct cmap *c;

	pf = btop(paddr);
	if (pf < cmap[1].c_pfnum) {
		parityErrorRecoverable = 0;
		return;
	}
	if (pf > cmap[maxfree].c_pfnum) {
		setConsole(CONSOLE_NOT_ON_PTY);
		resetConsole();
		pprintf("\nParity error: Physical address is outside range of memory!\n");
		parityHalt();
	}
	c = &cmap[pgtocm(pf)];
	switch (c->c_type) {
	  case CSYS:
		parityErrorRecoverable = 0;
		break;
	  case CDATA:
	  case CSTACK:
		parityKillProcess(&proc[c->c_ndx]);
		clearseg(pf);
		break;
	  case CTEXT:
		{
			struct text *xp;
			struct proc *p;

			xp = &text[c->c_ndx];
			p = xp->x_caddr;
			while (p) {
				parityKillProcess(p);
				p = p->p_xlink;
			}
		}
		clearseg(pf);
		break;
	}
}

/*
 * Scan a page of physical memory at vbase for parity errors.
 * Return a count of the number of errors found.
 */
int
parityScanPage(board, pageNumber)
	int board;
	long pageNumber;
{
	ushort ras, cas, par;
	long paddr;
	long errors;			/* must not be in a register */
	long *cell;			/* must not be in a register */
	jmp_buf jb;
	long dummy;

	errors = 0;
	if (setjmp(jb)) {
		*PARCTL_REG = 0;
		errors++;
		if (!parityGather(board, &paddr, &ras, &cas, &par)) {
			pprintf("Parity error occured, but for wrong board!\n");
			parityHalt();
		}
		parityPrint(board, ras, cas, par, paddr);
		parityFindUserOfPage(board, paddr);
		if (btop(paddr) != pageNumber) {
			pprintf("Parity error occured, but for wrong page!\n");
			parityHalt();
		}
		if ((paddr & PGOFSET) != ((long)cell & PGOFSET)) {
			long physaddr;
			physaddr = ((long)cell & PGOFSET) |
				   (pageNumber << PGSHIFT);
			pprintf("Board %d: cell=%x != paddr=%x\n",
				       board, physaddr, paddr);
			parityHalt();
		}
		/*
		 * Reset cell to point to the address that we just got
		 * a parity error from.  This allows the loop to start
		 * up right where it left off.
		 */
		cell = (long *) ((paddr & PGOFSET) | (((long)cell) & ~PGOFSET));
		*PARCTL_REG = PAR_KR;
		*STATUS_REG |= ST_EINTR;
		goto next_loop;
	} else {
		/* point parity handler at saved context */
		parityFault = jb;
	}
	*PARCTL_REG = PAR_KR;		/* enable parity error detection */
	*STATUS_REG |= ST_EINTR;	/* enable interrupts */

	/* read each long word in the page */
	for (cell = (long *) SCRPG0_VBASE;
		  cell < (long *) (SCRPG0_VBASE + NBPG); ) {
		dummy = *cell;
		asm("nop"); asm("nop"); asm("nop"); asm("nop");
next_loop:
		cell++;
	}

	/* give parity circuitry a chance to interrupt... */
	asm("nop"); asm("nop"); asm("nop"); asm("nop");

	*STATUS_REG &= ~ST_EINTR;	/* disable interrupts */
	*PARCTL_REG = 0;		/* disable parity error detection */
	parityFault = 0;		/* reset to no parity handler */
	if (errors) {
		checkForPageUsedByKernel(pageNumber << PGSHIFT);
	}
	return errors;
}

/*
 * Scan a memory board for parity errors within the board.  Count all
 * the errors found and print out the errors.
 */
int
parityScanBoard(board)
	register int board;
{
	register long pageNumber;
	register int boardErrors;
	register ulong *pagemap;
	register int i;
	register int pages;
	long oldPagemapValue;

	/*
	 * Get a pointer into the page map for a scrach page's mapping
	 * register.  In our inner loop we will set the mapping register
	 * to point to the physical page we are interested in reading.
	 */
	pagemap = ((ulong *)PTMAP_BASE) + (btop(SCRPG0_VBASE & ~SEG_MSK) +
		  (KCX << PPCXLOG2));
	oldPagemapValue = *pagemap;

	/*
	 * Scan each page in the board.
	 */
	pageNumber = board * 4 * btop(ONEMEG);
	boardErrors = 0;
	pages = ((membdsz[board] == 0) ? 2 : 4) * btop(ONEMEG);
	for (i = pages; --i >= 0; pageNumber++) {
		*pagemap = pageNumber | PG_KW;
		boardErrors += parityScanPage(board, pageNumber);
	}
	*pagemap = oldPagemapValue;
	return boardErrors;
}

/*
 * Handle a parity error.  Invoked by the trap code when a parity error
 * interrupt occurs.  Scan each memory board for an error, and if the
 * board has an error, decode its state and print out the memory address
 * of the error(s).
 */
void
parity(sr)
	short sr;
{
	register int i;
	ushort ras, cas, par;
	long paddr;
	int errorsFound;
	char buf[4*20];
	short oldParityControlReg;

	oldParityControlReg = *PARCTL_REG;
	*PARCTL_REG = 0;	/* disable future parity interrupts */
	errorsFound = 0;
	setConsole(CONSOLE_NOT_ON_PTY);
	resetConsole();
	pprintf("\nParity error:\n");
	parityErrorRecoverable = 1;

	/*
	 * Scan each memory board looking for boards that have errors
	 * to report.  For boards that report an error, scan that
	 * boards physical memory, counting the number of errors
	 * discovered.
	 */
	for (i = 0; i < NIM1BRDS; i++) {
		int boardErrors;

		if (parityGather(i, &paddr, &ras, &cas, &par)) {
			parityPrint(i, ras, cas, par, paddr);
			parityFindUserOfPage(i, paddr);
			logParityError(paddr);
			pprintf("Board %d: scanning for more errors\n", i);
			boardErrors = parityScanBoard(i);
			if ((boardErrors == 0) || (boardErrors > 100)) {
				parityErrorRecoverable = 0;
			}
			checkForPageUsedByKernel(paddr);
			errorsFound++;
		}
	}
	if (errorsFound == 0) {
		pprintf("Parity error address could not be determined!\n");
		parityHalt();
	}
	if (!parityErrorRecoverable) {
		pprintf("Unrecoverable error.\n");
		parityHalt();
	}

	/*
	 * Re-enable all kinds of interrupts. Set runrun to force an immediate
	 * processor reschedule so that current process gets a chance to be
	 * blasted to pieces.
	 */
	pprintf("Parity error is recoverable; continuing.\n");

	/*
	 * Because we had to reset the graphics display to print the error,
	 * blow away all graphics processes immediately.
	 */
	{
		register struct proc *p;

		for (p = &proc[0]; p < procNPROC; p++) {
			if (p->p_stat && (p->p_flag & SGR)) {
				psignal(p, SIGKILL);
			}
		}
	}
	*PARCTL_REG = oldParityControlReg;
	*STATUS_REG |= ST_EINTR;
	runrun++;
}

clkstart()
{
	todclkstart();
}

/*
 * rtclockset:
 *	- update real time clock hardware with new setting
 *	- do any thing else that depends on real time
 */
rtclockset()
{
	todsettim( time );
}

/*
 * physstrat:
 *	- perform physical i/o on the given buffer
 */
physstrat(bp, strat, prio)
	register struct buf *bp;
	int (*strat)(), prio;
{
	register int s;
	register struct proc *rp;
	register struct buf *ebp;
	register struct pte *pte;
	int count;

	rp = (bp->b_flags & B_DIRTY) ? &proc[2] : bp->b_proc;

	/*
	 * Look at bp->b_flags and see what kind of i/o this is
	 */
	if (bp->b_flags & B_PAGET) {
		/*
		 * A special hack is done here because page table reads
		 * have to be truncated to the requested size (we can't
		 * just read the whole page).  So, read the page table
		 * into a temporary buffer, then copy the portion requested
		 * into the destination page table.
		 */
		count = bp->b_bcount;
		if (bp->b_flags & B_READ) {
			/*
			 * Get an empty buffer from the buffer cache.  Use
			 * the empty buffers dma mapping to do dma into,
			 * effectively using its memory for dma into.
			 */
			ebp = getdmablk(BTOBB(count));
			bp->b_bcount = ebp->b_bcount;
			bp->b_iobase = ebp->b_iobase;
			bp->b_iolen = ebp->b_iolen;
			bp->b_flags |= B_DMA;    /* borrowed, actually */
		} else {
			/*
			 * Insure that the byte count is rounded up to
			 * the nearest bb size
			 */
			bp->b_bcount = BBTOB(BTOBB(count));
			pte = &Usrptmap[btokmx((struct pte *)bp->b_un.b_addr)];
		}
		DBG((DBG_PHYSIO, "page table swap%s, bcount=%d\n",
				 bp->b_flags & B_READ ? "in" : "out",
				 bp->b_bcount));
	} else
	if (bp->b_flags & B_UAREA)
		pte = rp->p_addr;
	else
		pte = vtopte(rp, btop(bp->b_un.b_addr));

	/*
	 * now allocate the multibus map space needed and start the
	 * i/o request
	 */
	bp->b_length = BTOBB(bp->b_bcount);
	if ((bp->b_flags & B_DMA) == 0) {
		extern caddr_t mbmapalloc();

		/*
		 * DMA I/O could have block (record) lengths not divisible
		 * by 512.  Pass the actual count (in bp->b_iolen) to
		 * mbmapalloc instead of BTOBB(bp->b_bcount).  Otherwise,
		 * panics will occasionally occur in bio_dma_free.
		 */
		bp->b_iolen = bp->b_bcount;
		bp->b_iobase = mbmapalloc(pte,
			(long)bp->b_un.b_addr & PGOFSET, bp->b_iolen);
		bp->b_flags |= B_DMA;
	}
	(*strat)(bp);

	/* pageout daemon doesn't wait for pushed pages */
	if (bp->b_flags & B_DIRTY)
		return;

	s = spl6();
	while ((bp->b_flags & B_DONE) == 0)
		sleep((caddr_t)bp, prio);
	splx(s);

	/*
	 * Copy just the requested portion of the page table.
	 */
	if ((bp->b_flags & (B_READ|B_PAGET)) == (B_READ|B_PAGET)) {
		/*
		 * Because the i/o has completed on bp, we know that the
		 * dma region it has borrowed has been released.  Clean
		 * out ebp's notion of dma space, so that it doesn't get
		 * released twice.
		 */
		ASSERT(!(bp->b_flags & B_DMA));
		ebp->b_flags &= ~B_DMA;
		ebp->b_iobase = (caddr_t)NULL;
		ebp->b_iolen = 0;
		/*
		 * Since dma was done into ebp's buffer, we now have
		 * to copy the requested data into the raw buffer.
		 */
		bcopy(ebp->b_un.b_addr, bp->b_un.b_addr, count);
		brelse(ebp);
	}
}

#include "sgigsc.h"
#if NSGIGSC > 0
#include "../h/sgigsc.h"
#endif

/*
 * exitmisc:
 *	- handle machine dependent exit
 */
/*ARGSUSED*/
exitmisc(p)
	register struct proc *p;
{
#ifndef KOPT_NOGL
	if (p->p_flag & SGR) {
#if NSGIGSC > 0
		struct sgigsc_qentry qe;

		qe.event = SGE_PDEATH;
		qe.ev.pid = p->p_pid;
		sgqenter(&qe);
#endif
		gr_free((long)p);
	}
#endif
}

/*ARGSUSED*/
qdopen(dev, flag)
	dev_t dev;
	int flag;
{
#ifdef	KOPT_NOGL
	u.u_error = ENXIO;
#else
	if (!(u.u_procp->p_flag & SGR))
		u.u_error = EINVAL;
#endif
}
