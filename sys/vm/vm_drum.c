/*
 * $Source: /d2/3.7/src/sys/vm/RCS/vm_drum.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:35:56 $
 */
#include "../h/param.h"
#include "../h/systm.h"
#include "../h/user.h"
#include "../h/proc.h"
#include "../h/text.h"
#include "../h/map.h"
#include "../h/buf.h"
#include "../vm/vm.h"
#include "../h/cmap.h"
#include "machine/pte.h"

/*
 * Expand the swap area for both the data and stack segments.
 * If space is not available for both, retract and return 0.
 */
swpexpand(ds, ss, dmp, smp)
	int ds, ss;
	register struct dmap *dmp, *smp;
{
	register struct dmap *tmp;
	register int ts;
	swblk_t ods;

	/*
	 * If dmap isn't growing, do smap first.
	 * This avoids anomalies if smap will try to grow and
	 * fail, which otherwise would shrink ds without expanding
	 * ss, a rather curious side effect!
	 */
	if (dmp->dm_alloc > ctod(ds)) {
		tmp = dmp; ts = ds;
		dmp = smp; ds = ss;
		smp = tmp; ss = ts;
	}
	ods = dmp->dm_size;
	if (vsexpand(ds, dmp, 0) == 0)
		goto bad;
	if (vsexpand(ss, smp, 0) == 0) {
		(void) vsexpand(dtoc(ods), dmp, 1);
		goto bad;
	}
	return (1);

bad:
	u.u_error = ENOMEM;
	return (0);
}

/*
 * Expand or contract the virtual swap segment mapped
 * by the argument diskmap so as to just allow the given size.
 *
 * FOR NOW CANT RELEASE UNLESS SHRINKING TO ZERO, SINCE PAGEOUTS MAY
 * BE IN PROGRESS... TYPICALLY NEVER SHRINK ANYWAYS, SO DOESNT MATTER MUCH
 */
vsexpand(vssize, dmp, canshrink)
	register int vssize;
	register struct dmap *dmp;
{
	register long blk = DMMIN;
	register swblk_t vsbase = 0;
	register swblk_t *ip = dmp->dm_map;
	swblk_t oldsize = dmp->dm_size;
	swblk_t oldalloc = dmp->dm_alloc;

	vssize = ctod(vssize);
	while (vsbase < oldalloc || vsbase < vssize) {
		if (ip - dmp->dm_map >= NDMAP)
			panic("vmdrum NDMAP");
		if (vsbase >= oldalloc) {
			*ip = rmalloc(swapmap, blk);
			if (*ip == 0) {
				dmp->dm_size = vsbase;
				if (vsexpand(dtoc(oldsize), dmp, 1) == 0)
					panic("vsexpand");
				return (0);
			}
			dmp->dm_alloc += blk;
		} else
		if ((vssize == 0) || (vsbase >= vssize && canshrink)) {
			rmfree(swapmap, blk, *ip);
			*ip = 0;
			dmp->dm_alloc -= blk;
		}
		vsbase += blk;
		if (blk < DMMAX)
			blk *= 2;
		ip++;
	}
	dmp->dm_size = vssize;
	return (1);
}

/*
 * Allocate swap space for a text segment,
 * in chunks of at most dmtext pages.
 */
vsxalloc(xp)
	register struct text *xp;
{
	register long blk;
	register swblk_t *dp;
	register swblk_t vsbase;
	register int xsize;

	xsize = ctod(xp->x_size);
	if (xsize > NXDAD * DMTEXT)
		return (0);
	dp = xp->x_daddr;
	for (vsbase = 0; vsbase < xsize; vsbase += DMTEXT) {
		blk = xsize - vsbase;
		if (blk > DMTEXT)
			blk = DMTEXT;
		if ((*dp++ = rmalloc(swapmap, blk)) == 0) {
			vsxfree(xp, dtoc(vsbase));
			return (0);
		}
	}
	xp->x_ptdaddr = rmalloc(swapmap, (long)ctod(ctopt(xp->x_size)));
	if (xp->x_ptdaddr == 0) {
		vsxfree(xp, (long)xp->x_size);
		return (0);
	}
	return (1);
}

/*
 * Free the swap space of a text segment which
 * has been allocated ts pages.
 */
vsxfree(xp, ts)
	register struct text *xp;
	register long ts;
{
	register long blk;
	register swblk_t *dp;
	register swblk_t vsbase;

	ts = ctod(ts);
	dp = xp->x_daddr;
	for (vsbase = 0; vsbase < ts; vsbase += DMTEXT) {
		blk = ts - vsbase;
		if (blk > DMTEXT)
			blk = DMTEXT;
		rmfree(swapmap, blk, *dp);
		*dp++ = 0;
	}
	if (xp->x_ptdaddr) {
		rmfree(swapmap, (long)ctod(ctopt(xp->x_size)),
				xp->x_ptdaddr);
		xp->x_ptdaddr = 0;
	}
}

/*
 * Swap a segment of virtual memory to disk,
 * by locating the contiguous dirty pte's
 * and calling vschunk with each chunk.
 */
vsswap(p, pte, type, vsbase, vscount, dmp)
	struct proc *p;
	register struct pte *pte;
	int type;
	register int vsbase, vscount;
	struct dmap *dmp;
{
	register int size = 0;

	ASSERT(p->p_flag & SLOSTCX);
	for (;;) {
		/*
		 * This check here relys on the fact that dirty(pte) insures
		 * that a not-dirty page can be either
		 *	(1) not dirty
		 *	(2) not a real page (pg_pfnum == 0)
		 *	(3) fill on demand
		 * Otherwise, the page must be real (pg_pfnum != 0) and not
		 * be fill on demand (!pg_fod).
		 */
		if (vscount == 0 || !dirty(pte)) {
			if (size) {
				vschunk(p, vsbase, size, type, dmp);
				vsbase += size;
				size = 0;
			}
			if (vscount == 0)
				return;
			vsbase++;
			if (pte->pg_fod == 0 && pte->pg_pfnum)
				if (type == CTEXT)
					p->p_textp->x_rssize -= vmemfree(pte, 1);
				else
					p->p_rssize -= vmemfree(pte, 1);
		} else {
			size++;
			ASSERT(pte->pg_pfnum && !pte->pg_fod);
			mlock(pte->pg_pfnum);
			munlock(pte->pg_pfnum);
		}
		vscount--;
		if (type == CSTACK)
			pte--;
		else
			pte++;
	}
}

vschunk(p, base, size, type, dmp)
	register struct proc *p;
	register int base, size;
	int type;
	struct dmap *dmp;
{
	register struct pte *pte;
	struct dblock db;
	unsigned v;

	base = ctod(base);
	size = ctod(size);
	if (type == CTEXT) {
		while (size > 0) {
			db.db_size = DMTEXT - base % DMTEXT;
			if (db.db_size > size)
				db.db_size = size;
			ASSERT((base % ctod(1)) == 0);
			ASSERT((db.db_size % ctod(1)) == 0);
			swap(p, p->p_textp->x_daddr[base/DMTEXT] + base%DMTEXT,
			    ptob(tptov(p, dtoc(base))), (int)dtob(db.db_size),
			    B_WRITE, 0, swapdev, 0);
			pte = tptopte(p, dtoc(base));
			p->p_textp->x_rssize -=
			    vmemfree(pte, (int)dtoc(db.db_size));
			base += db.db_size;
			size -= db.db_size;
		}
		return;
	}
	do {
		vstodb(base, size, dmp, &db, type == CSTACK);
		v = type==CSTACK ?
		    sptov(p, dtoc(base+db.db_size)-1) :
		    dptov(p, dtoc(base));
		swap(p, db.db_base, ptob(v), (int)dtob(db.db_size),
		    B_WRITE, 0, swapdev, 0);
		pte = type==CSTACK ?
		    sptopte(p, dtoc(base+db.db_size)-1) :
		    dptopte(p, dtoc(base));
		p->p_rssize -= vmemfree(pte, (int)dtoc(db.db_size));
		base += db.db_size;
		size -= db.db_size;
	} while (size != 0);
}

/*
 * Given a base/size pair in virtual swap area,
 * return a physical base/size pair which is the
 * (largest) initial, physically contiguous block.
 */
vstodb(vsbase, vssize, dmp, dbp, rev)
	register int vsbase, vssize;
	struct dmap *dmp;
	register struct dblock *dbp;
{
	register int blk = DMMIN;
	register swblk_t *ip = dmp->dm_map;

	if ((vsbase < 0) || (vssize < 0) || (vsbase + vssize > dmp->dm_size))
		panic("vstodb");
	while (vsbase >= blk) {
		vsbase -= blk;
		if (blk < DMMAX)
			blk *= 2;
		ip++;
	}
	if (*ip + blk > swplo + nswap)
		panic("vstodb *ip");
	dbp->db_size = MIN(vssize, blk - vsbase);
	dbp->db_base = *ip + (rev ? blk - (vsbase + dbp->db_size) : vsbase);
}

/*
 * Convert a virtual page number 
 * to its corresponding disk block number.
 * Used in pagein/pageout to initiate single page transfers.
 */
swblk_t
vtod(p, v, dmap, smap)
	register struct proc *p;
	register int v;
	struct dmap *dmap, *smap;
{
	struct dblock db;
	int tp;

	if (isatsv(p, v)) {
		tp = ctod(vtotp(p, v));
		return (p->p_textp->x_daddr[tp/DMTEXT] + tp%DMTEXT);
	}
	if (isassv(p, v))
		vstodb(ctod(vtosp(p, v)), ctod(1), smap, &db, 1);
	else
		vstodb(ctod(vtodp(p, v)), ctod(1), dmap, &db, 0);
	return (db.db_base);
}
