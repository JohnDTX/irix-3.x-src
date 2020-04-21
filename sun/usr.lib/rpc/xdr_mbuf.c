/* NFSSRC @(#)xdr_mbuf.c	2.1 86/04/14 */
#ifndef lint
static char sccsid[] = "@(#)xdr_mbuf.c 1.1 86/02/03 Copyr 1984 Sun Micro";
#endif

/*
 * xdr_mbuf.c, XDR implementation on kernel mbufs.
 *
 * 
 *
 * Copyright (C) 1984, Sun Microsystems, Inc.
 */

#ifdef SVR3
# include "sys/debug.h"
# include "sys/param.h"
# include "sys/mbuf.h"
# include "rpc/types.h"
# include "rpc/xdr.h"
# include "netinet/in.h"
#else
# include "../h/param.h"
# include "../h/mbuf.h"
# include "../rpc/types.h"
# include "../rpc/xdr.h"
# include "../netinet/in.h"
#endif

bool_t	xdrmbuf_getlong(), xdrmbuf_putlong();
bool_t	xdrmbuf_getbytes(), xdrmbuf_putbytes();
u_int	xdrmbuf_getpos();
bool_t	xdrmbuf_setpos();
long *	xdrmbuf_inline();
void	xdrmbuf_destroy();

/*
 * Xdr on mbufs operations vector.
 */
struct	xdr_ops xdrmbuf_ops = {
	xdrmbuf_getlong,
	xdrmbuf_putlong,
	xdrmbuf_getbytes,
	xdrmbuf_putbytes,
	xdrmbuf_getpos,
	xdrmbuf_setpos,
	xdrmbuf_inline,
	xdrmbuf_destroy
};

/*
 * Sun uses the x_handy member to count bytes remaining in the mbuf.
 * The x_private member points at the next byte to get/put.
 */
#define	x_resid	x_handy
#define	x_next	x_private

#define	xtom(xdrs)	((struct mbuf *) (xdrs)->x_base)

/*
 * Initialize xdr stream.
 */
void
xdrmbuf_init(xdrs, m, op)
	register XDR *xdrs;
	register struct mbuf *m;
	enum xdr_op op;
{

	xdrs->x_op = op;
	xdrs->x_ops = &xdrmbuf_ops;
	xdrs->x_base = (caddr_t)m;
	xdrs->x_next = mtod(m, caddr_t);
	xdrs->x_resid = m->m_len;
	xdrs->x_public = (caddr_t)0;
}

/* ARGSUSED */
void
xdrmbuf_destroy(xdrs)
	XDR *xdrs;
{
	/* do nothing */
}

bool_t
xdrmbuf_getlong(xdrs, lp)
	register XDR *xdrs;
	long *lp;
{

	if ((xdrs->x_resid -= sizeof(long)) < 0) {
		if (xdrs->x_resid != -sizeof(long))
			printf("xdr_mbuf: long crosses mbufs!\n");
		if (xdrs->x_base) {
			register struct mbuf *m = xtom(xdrs)->m_next;

			xdrs->x_base = (caddr_t)m;
			if (m == NULL)
				return (FALSE);
			xdrs->x_next = mtod(m, caddr_t);
			xdrs->x_resid = m->m_len - sizeof(long);
		} else {
			return (FALSE);
		}
	}
	*lp = ntohl(*((long *)(xdrs->x_next)));
	xdrs->x_next += sizeof(long);
	return (TRUE);
}

bool_t
xdrmbuf_putlong(xdrs, lp)
	register XDR *xdrs;
	long *lp;
{

	if ((xdrs->x_resid -= sizeof(long)) < 0) {
		if (xdrs->x_resid != -sizeof(long))
			printf("xdr_mbuf: putlong, long crosses mbufs!\n");
		if (xdrs->x_base) {
			register struct mbuf *m = xtom(xdrs)->m_next;

			xdrs->x_base = (caddr_t)m;
			if (m == NULL)
				return (FALSE);
			xdrs->x_next = mtod(m, caddr_t);
			xdrs->x_resid = m->m_len - sizeof(long);
		} else {
			return (FALSE);
		}
	}
	*(long *)xdrs->x_next = htonl(*lp);
	xdrs->x_next += sizeof(long);
	return (TRUE);
}

bool_t
xdrmbuf_getbytes(xdrs, addr, len)
	register XDR *xdrs;
	caddr_t addr;
	register u_int len;
{

	while ((xdrs->x_resid -= len) < 0) {
		if ((xdrs->x_resid += len) > 0) {
			bcopy(xdrs->x_next, addr, (u_int)xdrs->x_resid);
			addr += xdrs->x_resid;
			len -= xdrs->x_resid;
		}
		if (xdrs->x_base) {
			register struct mbuf *m = xtom(xdrs)->m_next;

			xdrs->x_base = (caddr_t)m;
			if (m == NULL)
				return (FALSE);
			xdrs->x_next = mtod(m, caddr_t);
			xdrs->x_resid = m->m_len;
		} else {
			return (FALSE);
		}
	}
	bcopy(xdrs->x_next, addr, (u_int)len);
	xdrs->x_next += len;
	return (TRUE);
}

bool_t
xdrmbuf_putbytes(xdrs, addr, len)
	register XDR *xdrs;
	caddr_t addr;
	register u_int len;
{

	while ((xdrs->x_resid -= len) < 0) {
		if ((xdrs->x_resid += len) > 0) {
			bcopy(addr, xdrs->x_next, (u_int)xdrs->x_resid);
			addr += xdrs->x_resid;
			len -= xdrs->x_resid;
		}
		if (xdrs->x_base) {
			register struct mbuf *m = xtom(xdrs)->m_next;

			xdrs->x_base = (caddr_t)m;
			if (m == NULL)
				return (FALSE);
			xdrs->x_next = mtod(m, caddr_t);
			xdrs->x_resid = m->m_len;
		} else {
			return (FALSE);
		}
	}
	bcopy(addr, xdrs->x_next, len);
	xdrs->x_next += len;
	return (TRUE);
}

/*
 * Like putbytes, only we avoid the copy by pointing a type 2
 * mbuf at the buffer.  Not safe if the buffer goes away before
 * the mbuf chain is deallocated.
 */
bool_t
#ifdef sgi
xdrmbuf_putbuf(xdrs, addr, len, ffunc, farg, dfunc, darg)
#else
xdrmbuf_putbuf(xdrs, addr, len, func, arg)
#endif
	register XDR *xdrs;
	caddr_t addr;
	u_int len;
#ifdef sgi
	int (*ffunc)(), (*dfunc)();
	long farg, darg;
#else
	int (*func)();
	int arg;
#endif
{
	register struct mbuf *m;
	struct mbuf *mclgetx();
	long llen = len;

	xdrmbuf_putlong(xdrs, &llen);
	/*
	 * XXX actually we could memory fault - need to put out zeros
	 */
	len = (len + 3) & ~3;
	xtom(xdrs)->m_len -= xdrs->x_resid;
#ifdef sgi
	m = mclgetx(ffunc, farg, dfunc, darg, addr, (int)len, M_WAIT);
#else
	m = mclgetx(func, arg, addr, (int)len, M_WAIT);
#endif
	if (m == NULL) {
		printf("xdrmbuf_putbuf: mclgetx failed\n");
		return (FALSE);
	}
	xtom(xdrs)->m_next = m;
	xdrs->x_resid = 0;
	return (TRUE);
}

u_int
xdrmbuf_getpos(xdrs)
	register XDR *xdrs;
{

	return xdrs->x_next - mtod(xtom(xdrs), caddr_t);
}

bool_t
xdrmbuf_setpos(xdrs, pos)
	register XDR *xdrs;
	u_int pos;
{
	register caddr_t newaddr, lastaddr;

	newaddr = mtod(xtom(xdrs), caddr_t) + pos;
	lastaddr = xdrs->x_next + xdrs->x_resid;
	if ((int)newaddr > (int)lastaddr)
		return (FALSE);
	xdrs->x_next = newaddr;
	xdrs->x_resid = (int)lastaddr - (int)newaddr;
	return (TRUE);
}

long *
xdrmbuf_inline(xdrs, len)
	register XDR *xdrs;
	int len;
{
	long *buf = 0;

	if (xdrs->x_resid >= len) {
		xdrs->x_resid -= len;
		buf = (long *) xdrs->x_next;
		xdrs->x_next += len;
	}
	return (buf);
}
