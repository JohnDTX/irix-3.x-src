/*
 * Copyright (c) 1982 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)uipc_mbuf.c	6.8 (Berkeley) 9/16/85
 */

#include "machine/pte.h"

#include "../h/param.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/proc.h"
#include "../h/cmap.h"
#include "../vm/vm.h"
#include "../h/mbuf.h"

mbinit()
{
	struct mbuf *m;

	MGET(m, M_WAIT, MT_DATA);
	if (!m)
		panic("mbinit");
	(void) m_free(m);
}

/*
 * Free a page, which was pointed to by some random mbuf
 */
void
m_pgfree(m)
	struct mbuf *m;
{
	register long mbx;
	register int s;

	mbx = btokmx((struct pte *)m);
	ASSERT(0 != mclrefcnt[mbx]);

	s = spl7();			/* do not lower priority */
	if (--mclrefcnt[mbx] == 0) {
		memfree(&Usrptmap[mbx], 1, 1);
		kmfree(1, mbx);
		mbstat.m_clusters--;
	}
	splx(s);
}

/*
 * Free cluster data pointed to by m
 */
int
m_clfree(m)
	struct mbuf *m;
{
	m_pgfree((struct mbuf *) (mtod(m, int)&~PGOFSET));
}

/*
 * Allocate a page of memory.  Make it addressable by the kernel, and
 * then set its reference count to one.
 */
struct mbuf *
m_clget(canwait)
	int canwait;
{
	register long mbx;
	struct mbuf *m;
	int s;

	s = splimp();
	mbx = kmap_alloc((long)1, canwait);
	if (mbx == 0) {
		mbstat.m_drops++;
		splx(s);
		return ((struct mbuf *)0);
	}
	m = cltom(mbx);
	if (memall(&Usrptmap[mbx], 1, (struct proc *)0, CSYS) == 0) {
		kmfree((long)1, (long)mbx);
		mbstat.m_drops++;
		splx(s);
		return ((struct mbuf *)0);
	}
	vmaccess(&Usrptmap[mbx], (caddr_t)m, 1);
	m->m_off = 0;				/*XXX needed? */
	ASSERT(mclrefcnt[mbx] == 0);
	mclrefcnt[mbx] = 1;
	mbstat.m_clusters++;
	splx(s);
	return (m);
}

#ifdef	notdef
/*
 * Given an mbuf, allocate a page and point the argument mbuf at the page,
 * initializing the offset and free function.  Return 0 if we can't do
 * it for some reason, non-zero otherwise.
 */
caddr_t
m_pgget(m, canwait)
	register struct mbuf *m;
	int canwait;
{
	caddr_t buf;

	if (buf = (caddr_t) m_clget(canwait)) {
		m->m_off = (char *)buf - (char *)m;
		m->m_freefunc = m_clfree;
	}
	return (buf);
}
#endif

/* up reference count for a normal (HAH!) cluster
 */
/* ARGSUSED */
m_cldup(m)
register struct mbuf *m;
{
	register struct mbuf *p;
	register int i;
	register int s = splimp();

	p = mtod(m, struct mbuf *);
	i = mtocl(p);
	ASSERT(i >= 0);
	ASSERT(mclrefcnt[i] != 0);
	mclrefcnt[i]++;

	splx(s);
}

/*
 * Allocate an mbuf.
 */
struct mbuf *
m_get(canwait, type)
	int canwait, type;
{
	register struct mbuf *m;
	register int i;
	int ms;

	ASSERT(type != MT_FREE && type >= 0 && type < MT_MAX);

	ms = splimp();
	for (;;) {
		if (m = mfree) {
			mfree = m->m_next;
			m->m_next = 0;
			if (m->m_type != MT_FREE)
				panic("mget");
			m->m_type = type;
			mbstat.m_mtypes[MT_FREE]--;
			mbstat.m_mtypes[type]++;
			m->m_off = MMINOFF;
			break;
		}

		/*
		 * Have to allocate a new cluster of MBUFs
		 */
		m = m_clget(canwait);
		if (!m) {
			if (canwait) {
				m_want = 1;
				(void) sleep((caddr_t)&mfree, PZERO - 1);
			} else
				break;
		} else {
			/*
			 * Split cluster up into a bunch of MBUFs
			 */
			for (i = NMBPCL; --i >= 0; ) {
				m->m_next = 0;
				m->m_off = 0;
				m->m_type = MT_DATA;
				mbstat.m_mbufs++;
				mbstat.m_mtypes[MT_DATA]++;
				(void) m_free(m);
				m++;
			}
		}
	}

	if (!m) {
		mbstat.m_drops++;
#ifdef NOISE
		iprintf("m_get() failed canwait=%d\n",canwait);
#endif
	}
	splx(ms);
	return (m);
}

struct mbuf *
m_getclr(canwait, type)
	int canwait, type;
{
	register struct mbuf *m;

	MGET(m, canwait, type);
	if (m == 0)
		return (0);
	bzero(mtod(m, caddr_t), MLEN);
	return (m);
}

/*
 * Free an mbuf, returning the next mbuf in the mbuf chain.
 */
struct mbuf *
m_free(m)
	register struct mbuf *m;
{
	register struct mbuf *n;
	int ms;

	ms = splimp();
	ASSERT(m->m_type >= 0 && m->m_type <= MT_MAX);
	if (m->m_type == MT_FREE)
		panic("mfree");
	mbstat.m_mtypes[m->m_type]--;
	mbstat.m_mtypes[MT_FREE]++;
	m->m_type = MT_FREE;
	if (m->m_off >= MSIZE) {
		ASSERT(m->m_freefunc);
		(*m->m_freefunc)(m);
	}
	n = m->m_next;			/* save next mbuf in chain */
	m->m_off = 0;
	m->m_act = 0;
	m->m_next = mfree;		/* link mbuf onto freelist */
	mfree = m;
	splx(ms);

	if (m_want) {
		m_want = 0;
		wakeup((caddr_t)&mfree);
	}
	return (n);
}

m_freem(m)
	register struct mbuf *m;
{
	register int s;

	s = splimp();
	while (m)
		m = m_free(m);
	splx(s);
}

/*
 * Mbuffer utility routines.
 */

/*
 * Make a copy of an mbuf chain starting "off" bytes from the beginning,
 * continuing for "len" bytes.  If len is M_COPYALL, copy to end of mbuf.
 * Should get M_WAIT/M_DONTWAIT from caller.
 */
struct mbuf *
m_copy(m, off, len)
	register struct mbuf *m;
	int off;
	register int len;
{
	register struct mbuf *n, **np;
	struct mbuf *top;

	if (len == 0)
		return (0);
	if (off < 0 || len < 0)
		panic("m_copy");
	while (off > 0) {
		if (m == 0)
			panic("m_copy");
		if (off < m->m_len)
			break;
		off -= m->m_len;
		m = m->m_next;
	}
	np = &top;
	top = 0;
	while (len > 0) {
		if (m == 0) {
			if (len != M_COPYALL)
				panic("m_copy");
			break;
		}
		MGET(n, M_DONTWAIT, m->m_type);
		*np = n;
		if (n == 0)
			goto nospace;
		n->m_len = MIN(len, m->m_len - off);
		if (m->m_off > MMAXOFF) {
			n->m_off = (mtod(m,int) - (int)n) + off;
			n->m_u.m_us = m->m_u.m_us;
			ASSERT(0 != m->m_dupfunc);
			if (0 != m->m_dupfunc)
				(*m->m_dupfunc)(m);
		} else
			bcopy(mtod(m, caddr_t)+off, mtod(n, caddr_t),
			    (unsigned)n->m_len);
		if (len != M_COPYALL)
			len -= n->m_len;
		off = 0;
		m = m->m_next;
		np = &n->m_next;
	}
	return (top);
nospace:
	m_freem(top);
	return (0);
}

m_cat(m, n)
	register struct mbuf *m, *n;
{
	while (m->m_next)
		m = m->m_next;
	while (n) {
		if (m->m_off >= MMAXOFF ||
		    m->m_off + m->m_len + n->m_len > MMAXOFF) {
			/* just join the two chains */
			m->m_next = n;
			return;
		}
		/* splat the data from one into the other */
		bcopy(mtod(n, caddr_t), mtod(m, caddr_t) + m->m_len,
		    (u_int)n->m_len);
		m->m_len += n->m_len;
		n = m_free(n);
	}
}

m_adj(mp, len)
	struct mbuf *mp;
	register int len;
{
	register struct mbuf *m;
	register count;

	if ((m = mp) == NULL)
		return;
	if (len >= 0) {
		while (m != NULL && len > 0) {
			if (m->m_len <= len) {
				len -= m->m_len;
				m->m_len = 0;
				m = m->m_next;
			} else {
				m->m_len -= len;
				m->m_off += len;
				break;
			}
		}
	} else {
		/*
		 * Trim from tail.  Scan the mbuf chain,
		 * calculating its length and finding the last mbuf.
		 * If the adjustment only affects this mbuf, then just
		 * adjust and return.  Otherwise, rescan and truncate
		 * after the remaining size.
		 */
		len = -len;
		count = 0;
		for (;;) {
			count += m->m_len;
			if (m->m_next == (struct mbuf *)0)
				break;
			m = m->m_next;
		}
		if (m->m_len >= len) {
			m->m_len -= len;
			return;
		}
		count -= len;
		/*
		 * Correct length for chain is "count".
		 * Find the mbuf with last data, adjust its length,
		 * and toss data from remaining mbufs on chain.
		 */
		for (m = mp; m; m = m->m_next) {
			if (m->m_len >= count) {
				m->m_len = count;
				break;
			}
			count -= m->m_len;
		}
		while (m = m->m_next)
			m->m_len = 0;
	}
}

/*
 * Rearrange an mbuf chain so that len bytes are contiguous
 * and in the data area of an mbuf (so that mtod and dtom
 * will work for a structure of size len).  Returns the resulting
 * mbuf chain on success, frees it and returns null on failure.
 * If there is room, it will add up to MPULL_EXTRA bytes to the
 * contiguous region in an attempt to avoid being called next time.
 */
struct mbuf *
m_pullup(n, len)
	register struct mbuf *n;
	int len;
{
	register struct mbuf *m;
	register int count;
	int space;

	if (n->m_off + len <= MMAXOFF && n->m_next) {
		m = n;
		n = n->m_next;
		len -= m->m_len;
	} else {
		if (len > MLEN)
			goto bad;
		MGET(m, M_DONTWAIT, n->m_type);
		if (m == 0)
			goto bad;
		m->m_len = 0;
	}
	space = MMAXOFF - m->m_off;
	do {
		count = MIN(MIN(space - m->m_len, len + MPULL_EXTRA), n->m_len);
		bcopy(mtod(n, caddr_t), mtod(m, caddr_t)+m->m_len,
		  (unsigned)count);
		len -= count;
		m->m_len += count;
		n->m_len -= count;
		if (n->m_len)
			n->m_off += count;
		else
			n = m_free(n);
	} while (len > 0 && n);
	if (len > 0) {
		(void) m_free(m);
		goto bad;
	}
	m->m_next = n;
	return (m);
bad:
	m_freem(n);
	return (0);
}
