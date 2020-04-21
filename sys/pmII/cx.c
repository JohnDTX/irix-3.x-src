/*
 * Context allocation routines
 *
 * $Source: /d2/3.7/src/sys/pmII/RCS/cx.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:33:32 $
 */

#include "../h/param.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/proc.h"
#include "../vm/vm.h"
#include "../h/ipc.h"
#include "../h/shm.h"
#include "../pmII/cx.h"
#include "../pmII/cpureg.h"

#define	FOURMB		btop(0x400000)

/*
 * cxinit:
 *	- initialize the context data structures
 *	- mark all context's busy
 *	- put context 0 on the size 128 free list
 *	- pre-allocate the kernel context's
 */
cxinit()
{
	register short i;
	register u_char *cp;

	cp = cx.cx_map;
	for (i = NCX + 1; --i >= 0; )
		*cp++ = CXBUSY;
	cp = cx.cx_free;
	for (i = NCXLOG2 + 1; --i >= 0; )
		*cp++ = NIL;

    /* allocate context's that the kernel uses (0x20 - 0x3F) */
	cp = &cx.cx_map[KCX];
	for (i = 0x40; --i >= 0x20; )
		*cp++ = CXBUSY;
	cx.cx_free[5] = 0x00; cx.cx_map[0x00] = NIL;
	cx.cx_free[6] = 0x40; cx.cx_map[0x40] = NIL;
}

/*
 * cx_size:
 *	- return the maximum context size needed for the current process,
 *	  in clicks
 */
size_t
cx_size()
{
	register int tdsize, ssize;
	register short caddr;
	register int i;

	/*
	 * Remember last data address and last stack address
	 */
	tdsize = u.u_procp->p_loadc + u.u_tsize + u.u_dsize;
	ssize = u.u_ssize + HIGHPAGES;

	/*
	 * See if phys() areas exist past the last data address.  Record
	 * the phys region which is furthurst out, address wise.
	 */
	if (u.u_pcb.pcb_physused) {
		register struct phys *ph;

		for (ph = &u.u_pcb.pcb_phys[0]; ph < &u.u_pcb.pcb_phys[NPHYS];
			ph++) {
			if (ph->p_phsize) {
				caddr = btoc(ph->p_phladdr) + ph->p_phsize;
				if (caddr > tdsize)
					tdsize = caddr;
			}
		}
	}

	/*
	 * See if shmem areas exist past the last data address.  Record
	 * the shmem region which is furthurst out, address wise.
	 */
	if (u.u_procp->p_flag & SSHMEM) {
		register struct shmid_ds **slot;
		register struct shmpt_ds *map;

		i = u.u_procp->p_ndx * shminfo.shmseg;
		slot = &shm_shmem[i];
		map = &shm_ptbl[i];
		for (i = shminfo.shmseg; --i >= 0; slot++, map++) {
			if (*slot) {
				caddr = map->shm_segbeg +
					btoc((*slot)->shm_segsz);
				if (caddr > tdsize)
					tdsize = caddr;
			}
		}
	}

	if (tdsize > ssize)
		return (tdsize);
	return (ssize);
}

/*
 * cxget:
 *	- get context space for the given proc
 *	- if we can't find something our size, steal somebody elses
 */
cxget(p)
	register struct proc *p;
{
	register short cxsize, cxslot, size, ocxslot;
	register size_t npg;
	register short num;

	npg = cx_size();

    /* steal map back from whoever has cx 0 */
	if (cx.cx_zeroinuse) {
		while (cx.cx_head)
			cxput(cx.cx_head, 1);
		cx.cx_zeroinuse = 0;
	}

    /* use entire map, if too big */
	if (npg > FOURMB) {
		while (cx.cx_head)
			cxput(cx.cx_head, 1);
		p->p_cxnum = 0;
		p->p_cxbsize = NCXLOG2;
		/* glue new process to head of active context list */
		p->p_cxprev = NULL;
		p->p_cxnext = cx.cx_head;
		cx.cx_head = p;
		cx.cx_zeroinuse = 1;
		return;
	}

    /*
     * convert npg to # of contexts needed; figure out which freelist to
     * look for the correctly sized context
     */
	npg = (npg + 15) >> 4;		/* 16 pages per context */
	cxsize = 1;
	cxslot = 0;
	while (cxsize < npg) {
		cxsize <<= 1;
		cxslot++;
	}

    /* search for a good context to use */
	ocxslot = cxslot;
	for (;;) {
		size = cxsize;
		for (; cxslot < NCXLOG2; cxslot++) {
			if (cx.cx_free[cxslot] != NIL) {
				num = cx.cx_free[cxslot];
				cx.cx_free[cxslot] = cx.cx_map[num];

			    /* free extra space */
				while (size > cxsize) {
					size >>= 1;
					cxslot--;
					cx.cx_map[num + size] =
						cx.cx_free[cxslot];
					cx.cx_free[cxslot] = num + size;
				}
				cx.cx_map[num] = CXBUSY;
				p->p_cxnum = num;
				p->p_cxbsize = cxslot;
				/* glue proc to head of active context list */
				if (cx.cx_head)
					cx.cx_head->p_cxprev = p;
				p->p_cxprev = NULL;
				p->p_cxnext = cx.cx_head;
				cx.cx_head = p;
				return;
			}
			size <<= 1;
		}
		cxslot = ocxslot;
		if (!cx.cx_head)
			panic("cxget");

	    /* nothing usable; rip off everybodys else's */
		while (cx.cx_head)
			cxput(cx.cx_head, 1);
	}
}

/*
 * Clear out the nolonger used context
 */
void
cx_clear(p)
	register struct proc *p;
{
	register u_short *pagemap, *protmap;
	register short size;
	register long scx;
	register long vaddr;

	if (cx.cx_zeroinuse) {
		/* clear entire portion of map that user is using */
		pagemap = (u_short *)PAGEBASE;
		protmap = (u_short *)PROTBASE;
		size = MAXTDSIZ;
		while (--size >= 0) {
			*pagemap++ = 0;
			*protmap++ = 0;
		}
	} else {
		/* clear out text&data portion of context */
		scx = p->p_cxnum << 16;
		vaddr = btop(0 ^ scx);
		pagemap = ((u_short *)PAGEBASE) + vaddr;
		protmap = ((u_short *)PROTBASE) + vaddr;
		size = 16 << p->p_cxbsize;
		while (--size >= 0) {
			*pagemap++ = 0;
			*protmap++ = 0;
		}

		/* clear out stack portion of context */
		size = 16 << p->p_cxbsize;
		vaddr = btop((USRSTACK + ctob(HIGHPAGES) - ctob(size)) ^ scx);
		pagemap = ((u_short *)PAGEBASE) + vaddr;
		protmap = ((u_short *)PROTBASE) + vaddr;
		while (--size >= 0) {
			*pagemap++ = 0;
			*protmap++ = 0;
		}
	}
}

/*
 * cxput:
 *	- release the context proc "p" has
 */
cxput(p, doglean)
	register struct proc *p;
	short doglean;
{
	if (p->p_flag & SLOSTCX)
		return;
	if (doglean)
		glean(p);		/* save modified bits */
	cx_clear(p);
	if (cx.cx_zeroinuse)
		cx.cx_zeroinuse = 0;
	else
		cx_free(p->p_cxnum, p->p_cxbsize);

    /* unlink from lru */
	if (p != cx.cx_head) {		/* something before us */
		if (p->p_cxprev->p_cxnext = p->p_cxnext) {
			/* something after us */
			p->p_cxnext->p_cxprev = p->p_cxprev;
		}
	} else
		cx.cx_head = p->p_cxnext;
	p->p_cxnext = p->p_cxprev = NULL;
	p->p_flag |= SLOSTCX;
}

/*
 * cx_free:
 *	- free context "num" of size "size" (in pages)
 */
cx_free(num, bsize)
	register short num;
	register short bsize;
{
	register short buddy, prev;
	register size_t size;

	size = 1<<bsize;
	for (;;) {
		buddy = num ^ size; 
		if (cx.cx_map[buddy] != CXBUSY) {
			/*
			 * buddy is free, find buddy on its free list
			 */
			if (cx.cx_free[bsize] == buddy) {
				/* unlink from head */
				cx.cx_free[bsize] = cx.cx_map[buddy];
				cx.cx_map[buddy] = NIL;
				if (cx.cx_free[bsize] == CXBUSY)
					panic("cxfree");
			} else {
				/* unlink from middle */
				prev = cx.cx_free[bsize];
				while ((prev != NIL) &&
				       (cx.cx_map[prev] != buddy)) {
					if (prev == CXBUSY)
						panic("cxfree");
					prev = cx.cx_map[prev];
				}

				/* buddy is on some other free list */
				if (prev == NIL)
					goto somewhereelse;
				cx.cx_map[prev] = cx.cx_map[buddy];
			}

			/* set num to smaller buddy */
			if (buddy < num)
				num = buddy;

			/* advance to larger piece */
			size <<= 1;
			bsize++;
		} else {
			/* buddy is busy */
somewhereelse:
			cx.cx_map[num] = cx.cx_free[bsize];
			cx.cx_free[bsize] = num;
			return;
		}
	}
}
