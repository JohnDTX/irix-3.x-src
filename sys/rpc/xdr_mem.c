#ifndef lint
static char sccsid[] = "@(#)xdr_mem.c 1.1 85/05/30 Copyr 1984 Sun Micro";
#endif

/*
 * xdr_mem.h, XDR implementation using memory buffers.
 *
 * Copyright (C) 1984, Sun Microsystems, Inc.
 *
 * If you have some data to be interpreted as external data representation
 * or to be converted to external data representation in a memory buffer,
 * then this is the package for you.
 *
 */

#ifdef KERNEL
# ifdef SVR3
#  include "sys/param.h"
#  include "sys/sysmacros.h"
#  include "rpc/types.h"
#  include "rpc/xdr.h"
#  include "netinet/in.h"
# else
#  include "../h/param.h"
#  include "../rpc/types.h"
#  include "../rpc/xdr.h"
#  include "../netinet/in.h"
# endif
#else
#include "types.h"	/* <> */
#include "xdr.h"	/* <> */
#include <netinet/in.h>
#endif

static bool_t	xdrmem_getlong();
static bool_t	xdrmem_putlong();
static bool_t	xdrmem_getbytes();
static bool_t	xdrmem_putbytes();
static u_int	xdrmem_getpos();
static bool_t	xdrmem_setpos();
static long *	xdrmem_inline();
static void	xdrmem_destroy();

static struct	xdr_ops xdrmem_ops = {
	xdrmem_getlong,
	xdrmem_putlong,
	xdrmem_getbytes,
	xdrmem_putbytes,
	xdrmem_getpos,
	xdrmem_setpos,
	xdrmem_inline,
	xdrmem_destroy
};

/*
 * The procedure xdrmem_create initializes a stream descriptor for a
 * memory buffer.  
 */
void
xdrmem_create(xdrs, addr, size, op)
	register XDR *xdrs;
	caddr_t addr;
	u_int size;
	enum xdr_op op;
{

	xdrs->x_op = op;
	xdrs->x_ops = &xdrmem_ops;
	xdrs->x_private = xdrs->x_base = addr;
	xdrs->x_handy = size;
}

static void
xdrmem_destroy(/*xdrs*/)
	/*XDR *xdrs;*/
{
}

static bool_t
xdrmem_getlong(xdrs, lp)
	register XDR *xdrs;
	long *lp;
{

	if ((xdrs->x_handy -= sizeof(long)) < 0)
		return (FALSE);
	*lp = ntohl(*((long *)(xdrs->x_private)));
	xdrs->x_private += sizeof(long);
	return (TRUE);
}

static bool_t
xdrmem_putlong(xdrs, lp)
	register XDR *xdrs;
	long *lp;
{

	if ((xdrs->x_handy -= sizeof(long)) < 0)
		return (FALSE);
	*(long *)xdrs->x_private = htonl(*lp);
	xdrs->x_private += sizeof(long);
	return (TRUE);
}

static bool_t
xdrmem_getbytes(xdrs, addr, len)
	register XDR *xdrs;
	caddr_t addr;
	register u_int len;
{

	if ((xdrs->x_handy -= len) < 0)
		return (FALSE);
	bcopy(xdrs->x_private, addr, len);
	xdrs->x_private += len;
	return (TRUE);
}

static bool_t
xdrmem_putbytes(xdrs, addr, len)
	register XDR *xdrs;
	caddr_t addr;
	register u_int len;
{

	if ((xdrs->x_handy -= len) < 0)
		return (FALSE);
	bcopy(addr, xdrs->x_private, len);
	xdrs->x_private += len;
	return (TRUE);
}

static u_int
xdrmem_getpos(xdrs)
	register XDR *xdrs;
{

	return ((u_int)xdrs->x_private - (u_int)xdrs->x_base);
}

static bool_t
xdrmem_setpos(xdrs, pos)
	register XDR *xdrs;
	u_int pos;
{
	register caddr_t newaddr = xdrs->x_base + pos;
	register caddr_t lastaddr = xdrs->x_private + xdrs->x_handy;

	if ((long)newaddr > (long)lastaddr)
		return (FALSE);
	xdrs->x_private = newaddr;
	xdrs->x_handy = (int)lastaddr - (int)newaddr;
	return (TRUE);
}

static long *
xdrmem_inline(xdrs, len)
	register XDR *xdrs;
	int len;
{
	long *buf = 0;

	if (xdrs->x_handy >= len) {
		xdrs->x_handy -= len;
		buf = (long *) xdrs->x_private;
		xdrs->x_private += len;
	}
	return (buf);
}
