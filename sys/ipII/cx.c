/*
 * Context allocation routines
 *
 * $Source: /d2/3.7/src/sys/ipII/RCS/cx.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:30:40 $
 */

#include "../h/param.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/proc.h"
#include "../vm/vm.h"
#include "../h/ipc.h"
#include "../h/shm.h"
#include "../ipII/cx.h"

/*
** maximum size of the largest context that we can use, while still
** being able to share the pagemap on the IP2 is 32mb
*/
#define	MAXCXSZ		btop(0x2000000)

/*
 * cxinit:
 *	- initialize the context data structures
 *	- mark all context's busy
 *	- mark all free lists as empty
 *	- pre-allocate various contexts
 */
cxinit()
{
	register short i;
	register u_short *cp;

	/*
	 * mark all contexts busy
	 */
	cp = cx.cx_map;
	for (i = NCX + 1; --i >= 0; )
		*cp++ = CXBUSY;

	/*
	 * mark the free lists as empty
	 */
	cp = cx.cx_free;
	for (i = NCXLOG2 + 1; --i >= 0; )
		*cp++ = NIL;

	/*
	 * now allocate the context for the kernel: begins at
	 * context D00 and runs to DFF (4mb worth of contexts)
	 */
	cp = &cx.cx_map[KCX];
	for (i = KCX + 0x100; --i >= KCX; )
		*cp++ = CXBUSY;
	/*
	 * initialize the contexts to the following:
	 * context	size
	 * 0		800
	 * 800		400
	 * c00		100
	 * e00		200
	 *
	 * This is determined by the initial layout of the page map, and
	 * by the following:
	 *
	 * (cx_free entry number is LOG2(size of context)
	 *  where context size is = # bytes/ (pagesize * PTEPCX)
	 *  see cx.h for how context is computed)
	 */
	cx.cx_free[ 8 ] = 0xc00;
	cx.cx_map[ 0xc00 ] = NIL;

	cx.cx_free[ 9 ] = 0xe00;
	cx.cx_map[ 0xe00 ] = NIL;

	cx.cx_free[ 10 ] = 0x800;
	cx.cx_map[ 0x800 ] = NIL;

	cx.cx_free[ 11 ] = 0x0;
	cx.cx_map[ 0 ] = NIL;
}

/*
 * cx_size:
 *	- return the maximum context size needed for the current process's
 *	  text/data in clicks
 */
size_t
cx_size()
{
	register int tdsize;
	register short caddr;
	register int i;

	tdsize = u.u_procp->p_loadc + u.u_tsize + u.u_dsize;

	/*
	 * See if shmem areas exist past the last data address.  Record
	 * the shmem region which is further out, address wise.
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

	return (tdsize);
}

/*
 * cxget:
 *	- get context space for the given proc
 *	- if we can't find something our size, steal somebody elses
 * XXX	this code won't work if the stack is real large, and the td segment
 * XXX	is small, and lots of other processes are using the map...who cares?
 */
cxget(p)
	register struct proc *p;
{
	register short cxsize, cxslot, size, ocxslot;
	register size_t npg;
	register short num;
	int	ssize = 0;

	npg = cx_size();

    /* steal map back from whoever has cx 0 */
	if (cx.cx_zeroinuse) {
#ifdef	ASSERT
		if (cx.cx_head) {
			printf("cx.cx_head=%x p=%x\n", cx.cx_head, p);
			debug("cxget");
		}
#endif
		cx.cx_zeroinuse = 0;
	}

    /* use entire map, if too big */
	if (npg > MAXCXSZ) {
#ifdef	ASSERT
		printf("npg=%x\n", npg);
#endif
		while (cx.cx_head)
			cxput(cx.cx_head, 1);
		p->p_cxsnum = p->p_cxtdnum = 0;
		p->p_cxssize = p->p_cxbsize = NCXLOG2;
		/* glue new process to head of active context list */
		p->p_cxprev = NULL;
		p->p_cxnext = cx.cx_head;
		cx.cx_head = p;
		cx.cx_zeroinuse = 1;
		return;
	}

loop:
    /*
     * convert npg to # of contexts needed; figure out which freelist to
     * look for the correctly sized context
     */
	npg = (npg + PTEPCX - 1) >> PPCXLOG2;
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
				/* working on text/data */
				if ( !ssize ) {
					p->p_cxtdnum = num;
					p->p_cxbsize = cxslot;
				/* glue proc to head of active context list */
					if (cx.cx_head)
						cx.cx_head->p_cxprev = p;
					p->p_cxprev = NULL;
					p->p_cxnext = cx.cx_head;
					cx.cx_head = p;
					p->p_flag |= HAVTDCX;
				}
				else {
					p->p_cxsnum = num;
					p->p_cxssize = cxslot;
					p->p_flag |= HAVSCX;
				}
				goto out;
			}
			size <<= 1;
		}
		cxslot = ocxslot;
		if (!cx.cx_head)
			panic("cxget");

		/*
		 * strange case - getting stack context, nothing left
		 *		  rip yourself off?
		 */
		if (ssize)
			panic("cxget1");

	    /* nothing usable; rip off everybodys else's */
		while (cx.cx_head)
			cxput(cx.cx_head, 1);
	}
out:
	/*
	 * if no stack size, set the stack size and go get the context.
	 * Assumes: if you have a context, your text/data context has
	 * you linked on the LRU list for contexts, so we do not link
	 * the stack contexts
	 */
	if ( ! ssize )
	{
		npg = u.u_ssize + HIGHPAGES;
		ssize++;
/* XXX is this the right thing to do????? */
		if ( !npg )
		{
			return;
		}
		goto loop;
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
	{
#ifdef	ASSERT
		if ( p->p_flag&(HAVTDCX+HAVSCX)) {
		    printf("no hardware cx! but td or stk cx present, p=%x\n",
			       p);
		    debug("cxput");
		}
#endif
		return;
	}
#ifdef	ASSERT
	if ( !((p->p_flag&HAVTDCX) || (p->p_flag&HAVSCX) )) {
		printf("have hardware cx! but no td/stk cx, p=%x\n", p);
		debug("cxput");
	}
#endif
	if (doglean)
		glean(p);		/* save modified bits */
	if (cx.cx_zeroinuse)
		cx.cx_zeroinuse = 0;
	else {
		if ( p->p_flag & HAVTDCX )
			cx_free(p->p_cxtdnum, p->p_cxbsize);/* text/data cx */
		if ( p->p_flag & HAVSCX )
			cx_free(p->p_cxsnum, p->p_cxssize );/* stack cx*/
	}

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
	p->p_flag &= ~(HAVTDCX+HAVSCX);
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
