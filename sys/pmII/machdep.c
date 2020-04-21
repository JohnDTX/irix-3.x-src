/*
 * pmII/machdep.c
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
#include "../h/text.h"
#include "../h/acct.h"
#include "../vm/vm.h"
#include "../h/setjmp.h"
#include "../h/printf.h"
#include "../h/kprof.h"
#include "../com/com_pncc.h"
#include "../pmII/psr.h"
#include "../pmII/cpureg.h"
#include "../pmII/reg.h"
#include "../pmII/cx.h"
#include "../pmII/pte.h"
#include "../multibus/mbvar.h"
#include "../pmII/frame.h"

#include "xns.h"

extern	int etext, end;
extern	short panicing;

extern	unsigned char *bio_bbcnt;
extern	struct pte *bio_pt;
extern	struct map *bio_memmap;

char	XXXetheraddr[6];	/* ethernet address, maybe */
short	physmem;		/* # of physical memory pages */
long	rebootvec;		/* holds address of reboot entry to proms */
long	prom_mouseintr;		/* holds address of prom level7 intr routine */
short	maxmem;
short	shmem_pa;		/* physical page # of shared memory */
#ifdef	GL1
short	shmem_pa1;		/* physical page # of shared memory */
#endif
u_int	wub_pf;			/* physical page # of wub */
char	debug_stack[512];	/* temporary stack */

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

#define	HALFMEG		(512*1024)
#define	ONEMEG		(2*HALFMEG)
#define	SIXTEENMEG	(16*ONEMEG)
short	realfirstpage;				/* XXX */

/*
 * kernel_vend contains the kernels virtual end, one byte past the last
 * used byte by the kernel.
 */
long	kernel_vend;

out_of_memory()
{
	printf("Kernel: out of memory\n");
	halt();
}

/*
 * Allocate a page of kernel memory at kernel_vend.
 */
static short
getpage()
{
	struct pte x;

	getpte(kernel_vend, KCX, &x);
	if (x.pg_pfnum == 0)
		out_of_memory();
	kernel_vend += NBPG;
	return (x.pg_pfnum);
}

/*
 * mmuinit:
 *	- intialize the mmu to a known state
 *	- map the kernel into context 0
 *	- switch to the context (just to make sure)
 *	- map the schedulers udot (at firstpage)
 *	- protect kernel text except for i/o vectors
 *	- ASSUMES UPAGES == 1
 */
mmuinit()
{
	register struct pte *uaddr0;
	struct pte x;
	register int i;
	register long vaddr;

	/*
	 * Map the kernel's text space as read-only to make it a
	 * little easier to debug.  Skip the first page of memory
	 * because we need to write in it in locore.c.  The low
	 * page of memory is where the interrupt vectors are stored.
	 */
	vaddr = KERN_VBASE + NBPG;
	for (i = 1; i < btop((long)&etext - KERN_VBASE); i++) {
		getpte(vaddr, KCX, &x);
		*(long *)&x = x.pg_pfnum | PG_KR | PG_AS_OBRAM | PG_V;
		setpte(vaddr, KCX, &x);
		vaddr += NBPG;
	}

	/*
	 * Map in the multibus i/o space.
	 */
	vaddr = MBIO_VBASE;
	for (i = 0; i < btop(65536); i++) {
		*(long *)&x = i | PG_KW | PG_AS_MBIO | PG_V;
		setpte(vaddr, KCX, &x);
		vaddr += NBPG;
	}

	/*
	 * Align the kernel virtual end to a page boundary.
	 */
	kernel_vend = (((long)&end + USIZE - 1) >> PGSHIFT) << PGSHIFT;

	/* setup proc 0's page table pte */
	uaddr0 = &Usrptmap[0];
	*(long *)uaddr0 = getpage() | PG_KW | PG_AS_OBRAM | PG_V;
	setpte((long) usrpt, KCX, uaddr0);

	/* setup proc 0's udot page */
	uaddr0 = &usrpt[NPTEPG - UPAGES];	/* == uaddr(&proc[0]) */
	*(long *)uaddr0 = getpage() | PG_KW | PG_AS_OBRAM | PG_V;
	setpte((long) UDOT_VBASE, KCX, uaddr0);
	u.u_pcb.pcb_p0br = usrpt;
	u.u_pcb.pcb_p0lr = 0;

	/* don't ask (see the vax locore.s....) */
	u.u_pcb.pcb_p1br = (struct pte *)
		((int)usrpt + NBPG - 4*UPAGES + 4*(-(P1PAGES-UPAGES)));
	u.u_pcb.pcb_p1lr = P1PAGES-UPAGES;

	/*
	 * Before things get too serious, rip off a couple of pages
	 * for the graphics shared memory...
	 */
	shmem_pa = getpage();
	*(long *)&x = shmem_pa | PG_AS_OBRAM | PG_KW | PG_V;
	setpte((long) SHMEM_VBASE, KCX, &x);
	clearseg((unsigned) shmem_pa);
#ifdef	GL1
	/*
	 * GL1 uses the second page as feedback space.  GL2 doesn't need this,
	 * so don't bother wasting a page.
	 */
	shmem_pa1 = getpage();
	*(long *)&x = shmem_pa1 | PG_AS_OBRAM | PG_KW | PG_V;
	setpte((long) (SHMEM_VBASE + NBPG), KCX, &x);
	clearseg((unsigned) shmem_pa1);
#endif	GL1

	/*
	 * Now steal one page for the idiotic multibus controllers that need
	 * fixed addressed multibus memory.  We map the multibus map to point
	 * to this page.  We only support fixed address devices at WUB_MBADDR.
	 * This page is also mapped accessible from the pmII at WUB_VBASE.
	 */
	wub_pf = getpage();
	clearseg(wub_pf);
	*(long *)&x = btoc(ONEMEG + WUB_MBADDR) | PG_AS_MBRAM | PG_KW | PG_V;
	setpte((long) MBUS_VBASE, KCX, &x);
	*(u_short *)MBUS_VBASE = wub_pf;
	*(long *)&x = wub_pf | PG_AS_OBRAM | PG_KW | PG_V;
	setpte((long) WUB_VBASE, KCX, &x);

	beprint = 1;
}

/*
 * premain:
 *	- this code runs just before main, and does an machine dependent
 *	  initialization thats needed:
 *
 *	- initialize the interrupt vectors to point into the dispatch
 *	  table in locore.s
 *	- note that the address's stored in the interrupt vectors
 *	  point into the vector base region, and the code the gets executed
 *	  upon an interrupt must run in relative-pc mode until it switches
 *	  contexts to the kernel context
 *	- once that is done, we can map the low pages of the kernel
 *	  memory read-only, so as to prevent text scribbling
 */
premain()
{
	struct pte x;

	rebootvec = *(long *)(0xF80004);	/* get prom start addr */

    /* re-map the interrupt vector pages read-only */
	*(long *)&x = 0 | PG_AS_OBRAM | PG_KR | PG_V;
	setpte((long) IVEC_VBASE, KCX, &x);
	setpte((long) KERN_VBASE, KCX, &x);

    /* now choose console */
	con_init();
}

/*
 * For each board of memory, we keep this data structure for translating
 * page frame #'s into cmap index's
 */
#define	NBOARDS	32
short	btocmx[NBOARDS+1];		/* +1 because last one is a marker */
long	memorymask;			/* bitmap of memory boards found */
long	totalpages;			/* total pages in machine */

/*
 * Size the systems memory.  Probe each 1/2meg memory boad to see if its
 * present, clearing memory not used by the kernel and building up a
 * bitvector in memorymask describing which boards were found.
 */
sizemem()
{
	register long maskspot;
	register short pf, lastpf;
	register int i;
	short firstfree;
	jmp_buf jb;
	int *saved_jb;
	extern int *nofault;
	struct pte x;

	/*
	 * Figure out what the first free physical page of memory is, using
	 * kernel_vend.
	 */
	getpte(kernel_vend, KCX, &x);
	firstfree = x.pg_pfnum;
	if (firstfree == 0)
		out_of_memory();

	/*
	 * Probe each memory board that the kernel isn't using.  Record
	 * in memorymask the boards that are found.
	 */
	totalpages = 0;
	memorymask = 0;
	maskspot = 1;
	for (pf = 0; pf < btop(SIXTEENMEG); pf += btop(HALFMEG)) {
		lastpf = pf + btop(HALFMEG);
		if ((pf == 0) ||
		    ((pf < firstfree) && (firstfree < lastpf))) {
			/*
			 * Kernel is using this board.  Note its presence.
			 * If this is the last board that the kernel is
			 * using, clear the extra pages not being used.
			 */
			memorymask |= maskspot;
			totalpages += btop(HALFMEG);
			for (i = firstfree; i < lastpf; i++)
				clearseg((unsigned) i);
			physmem = lastpf - firstfree;
		} else {
			/*
			 * Kernel isn't using this board.  Probe it and
			 * see if it will hold a value.
			 */
			*(long *)&x = pf | PG_AS_OBRAM | PG_KW | PG_V;
			setpte((long) SCRPG0_VBASE, KCX, &x);
			saved_jb = nofault;
			nofault = jb;
			if (setjmp(jb) == 0) {
				beprint = 0;
				*(long *)SCRPG0_VBASE = 0xa5a5a5a5;
				beprint = 1;
				if (*(long *)SCRPG0_VBASE == 0xa5a5a5a5) {
					memorymask |= maskspot;
					for (i = pf; i < lastpf; i++)
						clearseg((unsigned) i);
					physmem += btop(HALFMEG);
					totalpages += btop(HALFMEG);
				}
			}
			nofault = saved_jb;
		}
		maskspot <<= 1;
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

/*
 * startup:
 *	- determine memory size, and adjust to constraints
 *	- allocate system tables, and initialize some of the paging
 *	  data structures
 */
startup()
{
	register short i;
	register caddr_t v;
	register short *b, cmx;
	int maskspot;
	int nproctimes2;
	int ptes;
	long bytes_of_cache;
	int pages;

	sizemem();
	freemem = maxmem = physmem;
	printf("real = %d\n", ctob(totalpages));
	/*
	 * Biopages is the number of pages we will ever need for the
	 * buffer cache page table.  It is directly a function of the
	 * number of buffers and the maximum size each buffer can be.
	 * Because buffers in the cache may be smaller than efs_lbsize
	 * the buffer cache page table will become fragmented during
	 * odd times.  Accordingly, we need to have a larger page table
	 * space to account for the fragmentation.  The bio code is
	 * never allowed to wait for bio_pagetable space, so this is
	 * necessary.
	 */
	efs_setparams(totalpages);

	/*
	 * Allocate space for system data structures.
	 * The first available real memory address is in "firstaddr".
	 * As pages of memory are allocated, "firstaddr" is incremented.
	 * The first available kernel virtual address is in "v".
	 * As pages of kernel virtual memory are allocated, "v" is advanced.
	 */
	v = (caddr_t) kernel_vend;
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
	setprofvec();
	printf("%d bytes of profiling memory reserved at 0x%x-0x%x\n",
		   PROFSIZE, profbuf, (long)profbuf + PROFSIZE);
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

	init_malloc((char *) (KERN_VBASE + ctob(unixend)), (char *)KERN_VLIMIT);

    /* clear unused portion of pagemap not used by unix */
	for (i = unixend + btop(KERN_VBASE);
	       i < btop(KERN_VBASE + KERN_VSIZE); i++) {
		*((short *)PAGEBASE + (i ^ (KCX << 4))) = 0;
		*((short *)PROTBASE + (i ^ (KCX << 4))) = PR_INVALID;
	}

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
		maskspot = 2;			/* start at second board */
	} else {				/* second board is special */
		*b++ = -32767;			/* set to wacko value */
		/*
		 * Now figure out using the memorymask, what piece of
		 * physical memory we ended up using for the second
		 * halfmeg section of the kernel.  Scan the memorymask
		 * looking for the first non-hole after the first halfmeg
		 * spot.
		 */
		maskspot = 2;
		while (!(memorymask & maskspot)) {
			maskspot <<= 1;
			*b++ = -32767;		/* set to wacko value */
		}
		*b++ = -(unixend & 0x7f) + 1;
		cmx = 1 + btop(HALFMEG) - (unixend & 0x7f);
		maskspot <<= 1;			/* start at next board */
	}
	for (; b < &btocmx[NBOARDS]; b++) {
		if (memorymask & maskspot) {
			*b = cmx;
			cmx += btop(HALFMEG);
		}
		maskspot <<= 1;
	}
	maxmem = totalpages - unixend;

#ifdef	notdef
printf("memorymask=%x totalpages=%x\n", memorymask, totalpages);
printf("maxmem=%x unixend=%x physmem=%x\n", maxmem, unixend, physmem);
	for (i = 0; i < 33; i++)
		printf("%04x ", (unsigned short)btocmx[i]);
	printf("\n");
#endif

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
	init_cmap();
	init_mbmap();
	maxmem = freemem;
	if (freemem < KLMAX)
		out_of_memory();
	printf("kmem = %d\n", ctob(unixend));
	printf("user = %d\n", ctob(freemem));
	printf("bufs = %d (max=%dk)\n", nbuf * efs_lbsize, efs_lbsize / 1024);
	rminit(kernelmap, (long)USRPTSIZE-1, (long)1, "usrpt", nproctimes2);
	/*
	 * enable parity error detection
	 */
	*(short *)STATUS |= STS_PARITY | STS_GEUSE;
	/*
	 * see if the sky board is there
	 */
	beprint = 0;
	skyprobe();
	beprint = 1;
}

/*
 * Initialize core map
 */
init_cmap()
{
	register int i, nextpf, lastpf;
	register struct cmap *c;
	int cmapindex;
	register int unixsize;
	register int maskspot;

	firstfree = 0;
	freemem = maxfree = maxmem;
	ecmx = ecmap - cmap;
	if (ecmx < freemem)
		freemem = ecmx;

	maskspot = cmapindex = 1;
	unixsize = unixend;
	for (i = 0; i < (SIXTEENMEG / HALFMEG); i++, maskspot <<= 1) {
		/*
		 * If no memory board is present, skip to the next one.
		 */
		if (!(memorymask & maskspot))
			continue;
		/*
		 * Place each free page in this half-meg piece on the cmap.
		 * Check if page is being used by the kernel, and if so,
		 * don't free it, just keep going.
		 */
		nextpf = i * btop(HALFMEG);
		lastpf = nextpf + btop(HALFMEG);
		while (nextpf < lastpf) {
			if (unixsize) {
				unixsize--;
				nextpf++;
				continue;
			}
			cmap[cmapindex - 1].c_next = cmapindex;
			c = &cmap[cmapindex];
			c->c_prev = cmapindex - 1;
			c->c_free = 1;
			c->c_gone = 1;
			c->c_type = CSYS;
			c->c_pfnum = nextpf++;
			cmapindex++;
		}
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
 *	- tag the region with an i/o lock so that pageout will leave
 *	  it alone, but so that we will be able to access the page
 *	  if some other part of the kernel is doing i/o in the page
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
		return (NULL);				/* no good */
	}

	p = u.u_procp;
	oldphysio = p->p_flag & SPHYSIO;
	p->p_flag |= SPHYSIO;
	pte0 = pte = vtopte(p, btop(base));
	npf = btoc(count + ((int)base & PGOFSET));
	while (npf > 0) {
		if (pte->pg_v) {
			c = &cmap[(short) pgtocm(pte->pg_pfnum)];
#ifdef	OS_DEBUG
			/*
			 * If page has some iolocks but no lock, then somebody
			 * blew it.
			 */
			if (c->c_iolocks && !c->c_lock) {
				printf("pf=%x c_ndx=%d c_page=%d c_type=%d\n",
					      pte->pg_pfnum,
					      c->c_ndx, c->c_page, c->c_type);
				panic("iolock iolocks");
			}
#endif	OS_DEBUG
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
			c = &cmap[(short) pgtocm(pte->pg_pfnum)];
#ifdef	OS_DEBUG
			if (!c->c_lock) {
				printf("pf=%x c_ndx=%d c_page=%d c_type=%d\n",
					      pte->pg_pfnum,
					      c->c_ndx, c->c_page, c->c_type);
				panic("iolock pagein");
			}
#endif	OS_DEBUG
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
	return (pte0);
}

#ifdef	GL2
/* 
 * iochecklock:
 *	- pagein a virtual segment for copyin/copyout
 *	- this just insure's that iolock hasn't been frobbed with and that
 *	  the user has not stepped outside his feedback area
 * XXX	- there must be SOMETHING better to do here than panic????
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
		return (NULL);
	}

	pte0 = pte = vtopte(p, btop(base));
	npf = btoc(count + ((int)base & PGOFSET));
	while (npf > 0) {
		register struct cmap *c;

		if (!pte->pg_v) {
			printf("pte=%x\n", pte);
			debug("iochecklock !v");
		}
		c = &cmap[(short) pgtocm(pte->pg_pfnum)];
		if (!(c->c_lock && c->c_iolocks)) {
			printf("pte=%x *pte=%x\n", pte, *(long *)pte);
			debug("iochecklock");
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
	register int rw;
{
	register short npf;
	register struct cmap *c;

	npf = btoc(count + ((int)base & PGOFSET));
	while (npf > 0) {
#ifdef	ASSERT
		if ((pte->pg_pfnum == 0) || !(pte->pg_v) || pte->pg_fod) {
			printf("iounlock: pte=%x *pte=%x, improper munlock\n",
					  pte, *(long *)pte);
			debug("iounlock");
		}
#endif	ASSERT
		c = &cmap[pgtocm(pte->pg_pfnum)];
		/**** begin inline expansion of munlock() ****/
#ifdef	ASSERT
		if (c->c_lock == 0) {
			printf("c_ndx=%d c_type=%d c_page=%d pf=%x\n",
					 c->c_ndx, c->c_type, c->c_page,
					 pte->pg_pfnum);
			panic("dup page unlock (iounlock)");
		}
#endif
		/*
		 * If the page still has some i/o locks on it, leave
		 * it c_lock'd until the last iolock is removed.
		 */
		if (c->c_iolocks) {
			if (--c->c_iolocks == 0)
				c->c_lock = 0;
		} else
			c->c_lock = 0;
		/*
		 * If page is finally unlocked, wakeup anybody waiting for
		 * the page
		 */
		if (!c->c_lock && c->c_want) {
			wakeup((caddr_t)c);
			c->c_want = 0;
		}
		/**** end inline expansion of munlock() ****/

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
	register caddr_t to;
	register int count;
{
	struct pte *pte;		/* don't register */
	register u_short *pagemap, *protmap;
	int amount;			/* don't register */
	short pfs[2], prots[2];
	int error;
	jmp_buf jb;
	int *saved_jb;

	/*
	 * Save old scratch mapping information
	 */
	pagemap = ((u_short *)PAGEBASE) + btop(SCRPG0_VBASE ^ (KCX << 16));
	protmap = ((u_short *)PROTBASE) + btop(SCRPG0_VBASE ^ (KCX << 16));
	pfs[0] = *pagemap;
	prots[0] = *protmap;
	pfs[1] = *(pagemap + 1);
	prots[1] = *(protmap + 1);

	saved_jb = nofault;
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
		/*
		 * validate pages to be used during copy
		 */
		if ((pte = iolock(from, (long)amount)) == NULL) {
			error = -1;
			break;
		}
		*pagemap = pte->pg_pfnum;
		*protmap = ((PG_AS_OBRAM | PG_KW) >> 16) | KCX;
		*(pagemap + 1) = (pte + 1)->pg_pfnum;
		*(protmap + 1) = ((PG_AS_OBRAM | PG_KW) >> 16) | KCX;

		bcopy((caddr_t) (SCRPG0_VBASE + ((long)from & PGOFSET)),
		      to, amount);

		iounlock(pte, (unsigned)from, (long)amount, B_WRITE);
		from += amount;
		to += amount;
		count -= amount;
	}

	/*
	 * Restore old scratch page mapping
	 */
out:
	nofault = saved_jb;
	*pagemap++ = pfs[0];
	*protmap++ = prots[0];
	*pagemap = pfs[1];
	*protmap = prots[1];
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
	register u_short *pagemap, *protmap;
	int amount;			/* don't register */
	short pfs[2], prots[2];
	int error;
	jmp_buf jb;
	int *saved_jb;

	/*
	 * Save old scratch mapping information
	 */
	pagemap = ((u_short *)PAGEBASE) + btop(SCRPG0_VBASE ^ (KCX << 16));
	protmap = ((u_short *)PROTBASE) + btop(SCRPG0_VBASE ^ (KCX << 16));
	pfs[0] = *pagemap;
	prots[0] = *protmap;
	pfs[1] = *(pagemap + 1);
	prots[1] = *(protmap + 1);

	saved_jb = nofault;
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
		/*
		 * validate pages to be used during copy
		 */
		if ((pte = iolock(to, (long)amount)) == NULL) {
			error = -1;
			break;
		}

		*pagemap = pte->pg_pfnum;
		*protmap = ((PG_AS_OBRAM | PG_KW) >> 16) | KCX;
		*(pagemap + 1) = (pte + 1)->pg_pfnum;
		*(protmap + 1) = ((PG_AS_OBRAM | PG_KW) >> 16) | KCX;

		bcopy(from, (caddr_t) (SCRPG0_VBASE + ((long)to & PGOFSET)),
			    amount);
		iounlock(pte, (unsigned)to, (long)amount, B_READ);
		to += amount;
		from += amount;
		count -= amount;
	}

	/*
	 * Restore old scratch page mapping
	 */
out:
	*pagemap++ = pfs[0];
	*protmap++ = prots[0];
	*pagemap = pfs[1];
	*protmap = prots[1];
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
	register u_short *pagemap, *protmap;
	short pfs[2], prots[2];

    /* save old mapping */
	pagemap = ((u_short *)PAGEBASE) + btop(SCRPG0_VBASE ^ (KCX << 16));
	protmap = ((u_short *)PROTBASE) + btop(SCRPG0_VBASE ^ (KCX << 16));
	pfs[0] = *pagemap;
	prots[0] = *protmap;
	pfs[1] = *(pagemap + 1);
	prots[1] = *(protmap + 1);

    /* validate pages to be used during copy */
	if ((pte = iolock(from, (long)NBPG)) == NULL)
		panic("copyseg");

	*pagemap = pte->pg_pfnum;
	*protmap = ((PG_AS_OBRAM | PG_KW) >> 16) | KCX;
	*(pagemap + 1) = topf;
	*(protmap + 1) = ((PG_AS_OBRAM | PG_KW) >> 16) | KCX;

    /* copy the memory */
	bcopy((caddr_t)SCRPG0_VBASE, (caddr_t)SCRPG1_VBASE, NBPG);
/* XXX write bcopyPAGE... */
	iounlock(pte, (unsigned)from, (long)NBPG, B_WRITE);

    /* restore old mapping */
	*pagemap++ = pfs[0];
	*protmap++ = prots[0];
	*pagemap = pfs[1];
	*protmap = prots[1];
}

/*
 * clearseg - Clear one click's worth of data.
 */
clearseg(pfnum)
	unsigned pfnum;
{
	register u_short *pagemap, *protmap;
	short pfs, prots;

    /* save old SCRPG0 pte info */
	pagemap = ((u_short *)PAGEBASE) + btop(SCRPG0_VBASE ^ (KCX << 16));
	protmap = ((u_short *)PROTBASE) + btop(SCRPG0_VBASE ^ (KCX << 16));
	pfs = *pagemap;
	prots = *protmap;

    /* load up new info */
	*pagemap = pfnum;
	*protmap = ((PG_AS_OBRAM | PG_KW) >> 16) | KCX;

    /* zero out the page */
	bzeroPAGE((caddr_t)SCRPG0_VBASE);

    /* restore old mapping */
	*pagemap = pfs;
	*protmap = prots;
}

fubyte(v)
	caddr_t v;
{
	register struct pte *pte;
	register u_short *pagemap, *protmap;
	short pfs, prots;
	u_char x;

	pagemap = ((u_short *)PAGEBASE) + btop(SCRPG0_VBASE ^ (KCX << 16));
	protmap = ((u_short *)PROTBASE) + btop(SCRPG0_VBASE ^ (KCX << 16));
	pfs = *pagemap;
	prots = *protmap;

    /* validate page to be used during copy */
	if ((pte = iolock(v, (long)sizeof(x))) == NULL)
		return -1;

	*pagemap = pte->pg_pfnum;
	*protmap = ((PG_AS_OBRAM | PG_KW) >> 16) | KCX;

	x = *(char *)(SCRPG0_VBASE + ((long)v & PGOFSET));
	iounlock(pte, (unsigned)v, (long)sizeof(x), B_WRITE);

	*pagemap = pfs;
	*protmap = prots;
	return x;
}

subyte(v, x)
	caddr_t v;
	char x;
{
	register struct pte *pte;
	register u_short *pagemap, *protmap;
	short pfs, prots;

	pagemap = ((u_short *)PAGEBASE) + btop(SCRPG0_VBASE ^ (KCX << 16));
	protmap = ((u_short *)PROTBASE) + btop(SCRPG0_VBASE ^ (KCX << 16));
	pfs = *pagemap;
	prots = *protmap;

    /* validate page to be used during copy */
	if ((pte = iolock(v, (long)sizeof(x))) == NULL)
		return -1;

	*pagemap = pte->pg_pfnum;
	*protmap = ((PG_AS_OBRAM | PG_KW) >> 16) | KCX;

	*(char *)(SCRPG0_VBASE + ((long)v & PGOFSET)) = x;
	iounlock(pte, (unsigned)v, (long)sizeof(x), B_READ);

	*pagemap = pfs;
	*protmap = prots;
	return 0;
}

fuword(v)
	caddr_t v;
{
	register struct pte *pte;
	register u_short *pagemap, *protmap;
	short pfs[2], prots[2];
	int x;

	pagemap = ((u_short *)PAGEBASE) + btop(SCRPG0_VBASE ^ (KCX << 16));
	protmap = ((u_short *)PROTBASE) + btop(SCRPG0_VBASE ^ (KCX << 16));
	pfs[0] = *pagemap;
	prots[0] = *protmap;
	pfs[1] = *(pagemap + 1);
	prots[1] = *(protmap + 1);

    /* validate pages to be used during copy */
	if ((pte = iolock(v, (long)sizeof(x))) == NULL)
		return -1;

	*pagemap = pte->pg_pfnum;
	*protmap = ((PG_AS_OBRAM | PG_KW) >> 16) | KCX;
	*(pagemap + 1) = (pte + 1)->pg_pfnum;
	*(protmap + 1) = ((PG_AS_OBRAM | PG_KW) >> 16) | KCX;

	x = *(int *)(SCRPG0_VBASE + ((long)v & PGOFSET));
	iounlock(pte, (unsigned)v, (long)sizeof(x), B_WRITE);

	*pagemap++ = pfs[0];
	*protmap++ = prots[0];
	*pagemap = pfs[1];
	*protmap = prots[1];
	return x;
}

suword(v, x)
	caddr_t v;
	int x;
{
	register struct pte *pte;
	register u_short *pagemap, *protmap;
	short pfs[2], prots[2];

	pagemap = ((u_short *)PAGEBASE) + btop(SCRPG0_VBASE ^ (KCX << 16));
	protmap = ((u_short *)PROTBASE) + btop(SCRPG0_VBASE ^ (KCX << 16));
	pfs[0] = *pagemap;
	prots[0] = *protmap;
	pfs[1] = *(pagemap + 1);
	prots[1] = *(protmap + 1);

    /* validate pages to be used during copy */
	if ((pte = iolock(v, (long)sizeof(x))) == NULL)
		return -1;

	*pagemap = pte->pg_pfnum;
	*protmap = ((PG_AS_OBRAM | PG_KW) >> 16) | KCX;
	*(pagemap + 1) = (pte + 1)->pg_pfnum;
	*(protmap + 1) = ((PG_AS_OBRAM | PG_KW) >> 16) | KCX;

	*(int *)(SCRPG0_VBASE + ((long)v & PGOFSET)) = x;
	iounlock(pte, (unsigned)v, (long)sizeof(x), B_READ);

	*pagemap++ = pfs[0];
	*protmap++ = prots[0];
	*pagemap = pfs[1];
	*protmap = prots[1];
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
	register u_short *pagemap, *protmap;
	short pfs, prots;

/* XXX get rid of me once fast feedback is in the gl! */
	if ((v >= (caddr_t)0xEFC000) && (v + 2 <= (caddr_t)0xEFD000)) {
		*(short *)(SHMEM_VBASE + ((long)v & PGOFSET)) = x;
		return;
	}
	pagemap = ((u_short *)PAGEBASE) + btop(SCRPG0_VBASE ^ (KCX << 16));
	protmap = ((u_short *)PROTBASE) + btop(SCRPG0_VBASE ^ (KCX << 16));
	pfs = *pagemap;
	prots = *protmap;

    /* validate pages to be used during copy */
	pte = iochecklock(p, (unsigned)v, (long)sizeof(x));

	*pagemap = pte->pg_pfnum;
	*protmap = ((PG_AS_OBRAM | PG_KW) >> 16) | KCX;

	*(short *)(SCRPG0_VBASE + ((long)v & PGOFSET)) = x;

	*pagemap = pfs;
	*protmap = prots;
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
	register u_short *pagemap, *protmap;
	register struct pte *pte;
	short pfs[2], prots[2];

/* XXX get rid of me once fast feedback is in the gl! */
	if ((v >= (caddr_t)0xEFC000) && (v + n <= (caddr_t)0xEFD000)) {
		to = (short *)(SHMEM_VBASE + ((long)v & PGOFSET));
		while (n--)
			*to++ = *x;
		return;
	}
	pagemap = ((u_short *)PAGEBASE) + btop(SCRPG0_VBASE ^ (KCX << 16));
	protmap = ((u_short *)PROTBASE) + btop(SCRPG0_VBASE ^ (KCX << 16));
	pfs[0] = *pagemap;
	prots[0] = *protmap;
	pfs[1] = *(pagemap + 1);
	prots[1] = *(protmap + 1);

    /* validate pages to be used during copy */
	pte = iochecklock(p, (unsigned)v, (long)(n*sizeof(short)));

	*pagemap = pte->pg_pfnum;
	*protmap = ((PG_AS_OBRAM | PG_KW) >> 16) | KCX;
	*(pagemap + 1) = (pte + 1)->pg_pfnum;
	*(protmap + 1) = ((PG_AS_OBRAM | PG_KW) >> 16) | KCX;

    /* copy data from fbc to user memory */
	to = (short *)(SCRPG0_VBASE + ((long)v & PGOFSET));
	while (n--)
		*to++ = *x;

	*pagemap++ = pfs[0];
	*protmap++ = prots[0];
	*pagemap = pfs[1];
	*protmap = prots[1];
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
	register u_short *pagemap, *protmap;
	struct pte *pte;
	short pfs, prots;
	register char c;
	register int amount;

    /* check for end of page boundary */
	if ((amount = (NBPG - ((long)from & PGOFSET))) < maxspace)
		maxspace = amount;

    /* save old scratch page mappings */
	pagemap = ((u_short *)PAGEBASE) + btop(SCRPG0_VBASE ^ (KCX << 16));
	protmap = ((u_short *)PROTBASE) + btop(SCRPG0_VBASE ^ (KCX << 16));
	pfs = *pagemap;
	prots = *protmap;

    /* validate pages to be used during copy; setup scratch pages */
	if ((pte = iolock(from, (long)maxspace)) == NULL) {
		u.u_error = EFAULT;
		return 0;
	}
	*pagemap = pte->pg_pfnum;
	*protmap = ((PG_AS_OBRAM | PG_KW) >> 16) | KCX;

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
	*pagemap = pfs;
	*protmap = prots;

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
	register u_short *pagemap, *protmap;
	struct pte *pte;
	short pfs, prots;
	register char c;
	register int amount;

    /* check for end of page boundary */
	if ((amount = (NBPG - ((long)to & PGOFSET))) < maxspace)
		maxspace = amount;

    /* save old scratch page mappings */
	pagemap = ((u_short *)PAGEBASE) + btop(SCRPG0_VBASE ^ (KCX << 16));
	protmap = ((u_short *)PROTBASE) + btop(SCRPG0_VBASE ^ (KCX << 16));
	pfs = *pagemap;
	prots = *protmap;

    /* validate pages to be used during copy; setup scratch pages */
	if ((pte = iolock(to, (long)maxspace)) == NULL) {
		u.u_error = EFAULT;
		return 0;
	}
	*pagemap = pte->pg_pfnum;
	*protmap = ((PG_AS_OBRAM | PG_KW) >> 16) | KCX;

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
	*pagemap = pfs;
	*protmap = prots;

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
 * serial_poll:
 *	- this is a hack used to avoid problems when a routine which
 *	  is doing private tty processing takes too much time
 * XXX	GET RID OF THIS !@#%$#$%
 */
serial_poll()
{
	/* duintr(); */
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
 * parity:
 *	- process a parity error, and try to figure out where it actually
 *	  happened
 *	- due to a hardware botch in the pmII, the parity error is late
 *	  so we have to search around for the error
 */

int	paritysearching;		/* true when searching */

parity(sr)
	short sr;
{
	long i, offset, readvalue, newvalue;
	long *addr;
	short *pagemap, *protmap;
	jmp_buf jb;
	int numfound;
	short physpg;
	int j;

#ifdef	lint
	offset = 0;
	physpg = 0;
	numfound = 0;
#endif
	(void) spl7();
	setConsole(CONSOLE_NOT_ON_PTY);
	resetConsole();
	printf("%s parity error. Searching memory for error\n",
		   sr & PS_SUP ? "kernel" : "user");

	/*
	 * Clear parity error from hardware
	 */
	*(short *)STATUS &= ~STS_PARITY;
	*(short *)STATUS |= STS_PARITY;

	/*
	 * Setup fault handler to catch repeat parity errors
	 */
	nofault = jb;
	paritysearching = 1;
	if (setjmp(jb)) {
		*(short *)STATUS &= ~STS_PARITY;
		*(short *)STATUS |= STS_PARITY;
		printf("Error found at physical page=0x%x, offset=0x%x\n",
			      physpg, offset);
		numfound++;
		if ( ( numfound % 20 ) == 0 )
		{
			printf( "Continue (cr)? " );
			while ( getchar() != '\n' )
				;
		}
		goto keepgoing;
	}

	/*
	 * Search through every bloody physical page in the system
	 * and every byte in every page
	 */
	paritysearching = 1;
	numfound = 0;
	pagemap = (short *)PAGEBASE + btop(SCRPG0_VBASE ^ (KCX << 16));
	protmap = (short *)PROTBASE + btop(SCRPG0_VBASE ^ (KCX << 16));
	*protmap = ((PG_AS_OBRAM | PG_KW) >> 16) | KCX;
	for (i = 0; i < NBOARDS; i++) {
	    if ( ! btocmx[ i ] )
		    continue;
	    printf("Checking memory from 0x%x to 0x%x\n",
			     i * btop(HALFMEG),
			     i * btop(HALFMEG) + btop(HALFMEG) - 1);
	    for ( j = 0; j < btop( HALFMEG ); j++ ) {
		physpg = i * btop( HALFMEG ) + j;
		*pagemap = physpg;
		addr = (long *)SCRPG0_VBASE;
		for (offset = 0; offset < NBPG; offset += sizeof(long)) {
			/*
			 * If the parity error is at this location, we
			 * will get another bus error and catch it above.
			 * Note that the parity error will arrive late,
			 * which is why we are going through all this
			 * crud in the first place, so we put a few nop's
			 * in there just for fun...
			 */
			readvalue = *addr;
			asm("nop"); asm("nop");
			*addr = readvalue; 

			/*
			 * Check value we wrote with value we read in
			 * the first place.  If memory is REALLY broken
			 * then we might find something here.
			 */
			if ((newvalue = *addr) != readvalue) {
				printf("value changed while testing:");
				printf(" physical page=0x%x offset=0x%x\n",
					 physpg, offset);
				printf("read=0x%x re-read=0x%x\n",
						  readvalue, newvalue);
			}

keepgoing:
			addr++;
		}
	    }
	    printf("\n");
	}
	printf("%d parity errors found\n", numfound);
	panic("parity error");
	/* NOTREACHED */
}

/*
 * rtclockset:
 *	- update real time clock hardware with new setting
 *	- do any thing else that depends on real time
 */
rtclockset()
{
#ifdef	GL1
	extern long grlastupdate;

	grlastupdate = time;
#endif
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
			bp->b_flags |= B_DMA;   /* borrowed, actually */
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

#ifdef	SGIGSC
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
#ifdef	SGIGSC
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
