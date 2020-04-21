/*
 * $Source: /d2/3.7/src/sys/sys/RCS/tty_clist.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:35:40 $
 */
#include "../h/param.h"
#include "../h/tty.h"

struct	chead cfreelist;

/*
 * Initialize clist by freeing all character blocks.
 */
init_clist()
{
	register short n;
	register struct cblock *cp;

	for(n = 0, cp = &cfree[0]; n < nclist; n++, cp++) {
		cp->c_next = cfreelist.c_next;
		cfreelist.c_next = cp;
	}
	cfreelist.c_size = CBSIZE;
}

getc(p)
register struct clist *p;
{
	register struct cblock *bp;
	register int c;
	register int s;

	s = spltty();
	if (p->c_cc > 0) {
		p->c_cc--;
		bp = p->c_cf;
		c = bp->c_data[bp->c_first++]&0377;
		if (bp->c_first == bp->c_last) {
			if ((p->c_cf = bp->c_next) == NULL)
				p->c_cl = NULL;
#ifdef	notdef
			bp->c_next = cfreelist.c_next;
			cfreelist.c_next = bp;
#else
			putcf(bp);
#endif
		}
	} else
		c = -1;
	splx(s);
	return(c);
}

putc(c, p)
register struct clist *p;
{
	register struct cblock *bp, *obp;
	register int s;

	s = spltty();
	if ((bp = p->c_cl) == NULL || bp->c_last == (char)cfreelist.c_size) {
		obp = bp;
		if ((bp = cfreelist.c_next) == NULL) {
			splx(s);
			return(-1);
		}
		cfreelist.c_next = bp->c_next;
		bp->c_next = NULL;
		bp->c_first = 0; bp->c_last = 0;
		if (obp == NULL)
			p->c_cf = bp;
		else
			obp->c_next = bp;
		p->c_cl = bp;
	}
	bp->c_data[bp->c_last++] = c;
	p->c_cc++;
	splx(s);
	return(0);
}

struct cblock *
getcf()
{
	register struct cblock *bp;
	register struct chead *cf;
	register int s;

	s = spltty();
	cf = &cfreelist;
	if ((bp = cf->c_next) != NULL) {
		cf->c_next = bp->c_next;
		bp->c_next = NULL;
		bp->c_first = 0;
		bp->c_last = cf->c_size;
	}
	splx(s);
	return(bp);
}

/*
 * wgetcf() --
 * get a cblock from the cfreelist,
 * waiting if necessary.
 */
struct cblock *
wgetcf()
{
	register struct cblock *bp;
	register struct chead *cf;
	register int s;

	s = spltty();
	cf = &cfreelist;

	while ((bp = cf->c_next) == NULL) {
		cfreelist.c_flag = 1;
		sleep((caddr_t)&cfreelist, TTOPRI);
	}

	cf->c_next = bp->c_next;
	bp->c_next = NULL;
	bp->c_first = 0;
	bp->c_last = cf->c_size;

	splx(s);
	return(bp);
}

putcf(bp)
register struct cblock *bp;
{
	register struct chead *cf;
	register int s;

	s = spltty();
	cf = &cfreelist;
	bp->c_next = cf->c_next;
	cf->c_next = bp;
	if (cf->c_flag) {
		cf->c_flag = 0;
		wakeup((caddr_t)cf);
	}
	splx(s);
}

struct cblock *
getcb(p)
register struct clist *p;
{
	register struct cblock *bp;
	register int s;

	s = spltty();
	if ((bp = p->c_cf) != NULL) {
		p->c_cc -= bp->c_last - bp->c_first;
		if ((p->c_cf = bp->c_next) == NULL)
			p->c_cl = NULL;
	}
	splx(s);
	return(bp);
}

putcb(bp, p)
register struct cblock *bp;
register struct clist *p;
{
	register int s;

	s = spltty();
	if (p->c_cl == NULL)
		p->c_cf = bp;
	else
		p->c_cl->c_next = bp;
	p->c_cl = bp;
	bp->c_next = NULL;
	p->c_cc += bp->c_last - bp->c_first;
	splx(s);
}

/*
 * aputcb() --
 * append one cblock's worth of data to a clist q.
 * combine with the q tail if possible.
 */
aputcb(bp, q)
register struct cblock *bp;
struct clist *q;
{
	register struct cblock *tail;
	register int room,cc;
	register int s;

	s = spltty();

	for (;;) {
		/*
		 * if the cblock is empty, just free it.
		 */
		if ((cc = bp->c_last - bp->c_first) <= 0) {
#ifdef	notdef
			bp->c_next = cfreelist.c_next;
			cfreelist.c_next = bp;
			if (cfreelist.c_flag) {
				cfreelist.c_flag = 0;
				wakeup((caddr_t)&cfreelist);
			}
#else
			putcf(bp);
#endif
			break;
		}

		/*
		 * top off the tail cblock.  if already
		 * full, just append the new cblock.
		 * else put in as much as will fit.
		 */
		if ((tail = q->c_cl) == NULL
		 || (room = cfreelist.c_size - tail->c_last) <= 0) {
			bp->c_next = NULL;
			if (q->c_cl == NULL)
				q->c_cf = bp;
			else
				q->c_cl->c_next = bp;
			q->c_cl = bp;
			q->c_cc += cc;
			break;
		}

		if (cc > room)
			cc = room;
		bcopy(bp->c_data+bp->c_first, tail->c_data+tail->c_last, cc);
		tail->c_last += cc;
		q->c_cc += cc;
		bp->c_first += cc;
	}

	splx(s);
}

#ifdef	notdef
/* This is the equivalent of q_to_b */
getcbp(p, cp, n)
	struct clist *p;
	register char *cp;
	register int n;
{
	register struct cblock *bp;
	register char *op;
	register int on;
	register char *acp = cp;

	while (n) {
		if ((bp = p->c_cf) == NULL)
			break;
		op = &bp->c_data[bp->c_first];
		on = bp->c_last - bp->c_first;
		if (n >= on) {
			bcopy((caddr_t)op, (caddr_t)cp, on);
			cp += on;
			n -= on;
			if ((p->c_cf = bp->c_next) == NULL)
				p->c_cl = NULL;
#ifdef	notdef
			bp->c_next = cfreelist.c_next;
			cfreelist.c_next = bp;
#else
			putcf(bp);
#endif
		} else {
			bcopy((caddr_t)op, (caddr_t)cp, n);
			bp->c_first += n;
			cp += n;
			n = 0;
			break;
		}
	}
	n = cp - acp;
	p->c_cc -= n;
	return(n);
}

putcbp(p, cp, n)
	struct clist *p;
	register char *cp;
	register int n;
{
	register struct cblock *bp, *obp;
	register char *op;
	register int on;
	register char *acp = cp;

	while (n) {
		if ((bp = p->c_cl) == NULL || bp->c_last == cfreelist.c_size) {
			obp = bp;
			if ((bp = cfreelist.c_next) == NULL)
				break;
			cfreelist.c_next = bp->c_next;
			bp->c_next = NULL;
			bp->c_first = 0; bp->c_last = 0;
			if (obp == NULL)
				p->c_cf = bp;
			else
				obp->c_next = bp;
			p->c_cl = bp;
		}
		op = &bp->c_data[bp->c_last];
		on = cfreelist.c_size - bp->c_last;
		if (n >= on) {
			bcopy((caddr_t)cp, (caddr_t)op, on);
			cp += on;
			bp->c_last += on;
			n -= on;
		} else {
			bcopy((caddr_t)cp, (caddr_t)op, n);
			cp += n;
			bp->c_last += n;
			n = 0;
			break;
		}
	}
	n = cp - acp;
	p->c_cc += n;
	return(n);
}
#endif
