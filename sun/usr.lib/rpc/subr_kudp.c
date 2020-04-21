/* NFSSRC @(#)subr_kudp.c	2.4 86/05/13 */
#ifndef lint
static char sccsid[] = "@(#)subr_kudp.c 1.1 86/02/03 Copyr 1984 Sun Micro";
#endif

/*
 * subr_kudp.c
 * Subroutines to do UDP/IP sendto and recvfrom in the kernel
 *
 * Copyright (C) 1984, Sun Microsystems, Inc.
 */
#ifdef SVR3
# include "sys/debug.h"
# include "sys/sysmacros.h"
# include "sys/param.h"
# include "sys/socket.h"
# include "sys/socketvar.h"
# include "sys/mbuf.h"
# include "sys/errno.h"
# include "net/if.h"
# include "net/route.h"
# include "netinet/in.h"
# include "netinet/in_pcb.h"
# include "rpc/types.h"
# include "rpc/xdr.h"
# include "rpc/auth.h"
# include "rpc/clnt.h"
#else
# include "../h/param.h"
# include "../h/socket.h"
# include "../h/socketvar.h"
# include "../h/mbuf.h"
# include "../h/errno.h"
# include "../net/if.h"
# include "../net/route.h"
# include "../netinet/in.h"
# include "../netinet/in_pcb.h"
# include "../rpc/types.h"
# include "../rpc/xdr.h"
# include "../rpc/auth.h"
# include "../rpc/clnt.h"
#endif

struct mbuf     *mclgetx();

/*
 * General kernel udp stuff.
 * The routines below are used by both the client and the server side
 * rpc code.
 */

/*
 * Kernel recvfrom.
 * Pull address mbuf and data mbuf chain off socket receive queue.
 */
struct mbuf *
ku_recvfrom(so, from)
	register struct socket *so;
	struct sockaddr_in *from;
{
	register struct mbuf	*m;
	register struct mbuf	*m0;
	register int	len = 0;
#ifdef sgi
	register struct mbuf *next_dgram;
#endif

#ifdef RPCDEBUG
	rpc_debug(4, "ku_recvfrom so=%X\n", so);
#endif
	sblock(&so->so_rcv);
	m = so->so_rcv.sb_mb;
#if defined sgi && defined RPCDEBUG
{
	extern int rpcdebug;

	if (5 <= rpcdebug) {
		register struct mbuf *n;

		iprintf("Received m(%x)", m);
		for (n = m; n != NULL; n = n->m_next) {
			iprintf(" {type=%d, mtod=%x, len=%d, next=%x, act=%x}",
			    n->m_type, (caddr_t)n + n->m_off, n->m_len,
			    n->m_next, n->m_act);
		}
		iprintf(" from %x.%d\n", from->sin_addr.s_addr, from->sin_port);
	}
}
#endif
	if (m == NULL) {
		sbunlock(&so->so_rcv);
		return (m);
	}

	*from = *mtod(m, struct sockaddr_in *);
	sbfree(&so->so_rcv, m);
#ifdef sgi
	next_dgram = m->m_act;
	ASSERT(m->m_type == MT_SONAME);
#endif
	MFREE(m, m0);
	if (m0 == NULL) {
		printf("ku_recvfrom: no body!\n");
#ifdef sgi
		so->so_rcv.sb_mb = next_dgram;
#else
		so->so_rcv.sb_mb = m0;
#endif
		sbunlock(&so->so_rcv);
		return (m0);
	}

#ifdef sgi
	/*
	 * A socket receive buffer enqueues datagram mbufs thus:
	 *	sb_mb
	 *	  |
	 *	SONAME -(m_next)-> DATA -> ...
	 *	  |(m_act)
	 *	SONAME -(m_next)-> DATA -> ...
	 *	  |(m_act)
	 *	. . .
	 * The MFREE(m, m0) above has removed the socket address
	 * (SONAME) mbuf from the first datagram.  Now loop over the
	 * packet mbufs (DATA) linked via m_next, freeing the socket
	 * buffer space which they reserved and accumulating length.
	 * Then unlink the datagram from so->so_rcv by setting sb_mb
	 * to next_dgram (the SONAME mbuf's m_act, saved above).
	 */
	for (m = m0; m != NULL; m = m->m_next) {
		ASSERT(m->m_act == NULL);
		sbfree(&so->so_rcv, m);
		len += m->m_len;
	}

	so->so_rcv.sb_mb = next_dgram;
#else
	/*
	 * Walk down mbuf chain till m_act set (end of packet) or
	 * end of chain freeing socket buffer space as we go.
	 * After the loop m points to the last mbuf in the packet.
	 */
	m = m0;
	for (;;) {
		sbfree(&so->so_rcv, m);
		len += m->m_len;
		if (m->m_act || m->m_next == NULL) {
			break;
		}
		m = m->m_next;
	}

	so->so_rcv.sb_mb = m->m_next;
	m->m_next = NULL;
#endif
	if (len > UDPMSGSIZE) {
		printf("ku_recvfrom: len = %d\n", len);
	}

#ifdef RPCDEBUG
	rpc_debug(4, "ku_recvfrom %d from %X\n", len, from->sin_addr.s_addr);
#endif
	sbunlock(&so->so_rcv);
	return (m0);
}

int Sendtries = 0;
int Sendok = 0;
#ifdef sgi
int Fastok = 0;
#ifdef SVR3
extern short Dofast;
#else
short Dofast = 1;
#endif
#endif

/*
 * Kernel sendto.
 * Set addr and send off via UDP.
 * Use ku_fastsend if possible.
 */
int
ku_sendto_mbuf(so, m, addr)
	struct socket *so;
	struct mbuf *m;
	struct sockaddr_in *addr;
{
	register struct inpcb *inp = sotoinpcb(so);
	int error;
	int s;
	struct in_addr laddr;

#ifdef RPCDEBUG
	rpc_debug(4, "ku_sendto_mbuf %X\n", addr->sin_addr.s_addr);
#endif
#ifdef RPCDEBUG
{
	extern int rpcdebug;

	if (5 <= rpcdebug) {
		register struct mbuf *n;

		iprintf("Sending");
		for (n = m; n != NULL; n = n->m_next) {
			iprintf(" {type=%d, mtod=%x, len=%d, next=%x, act=%x}",
			    n->m_type, (caddr_t)n + n->m_off, n->m_len,
			    n->m_next, n->m_act);
		}
		iprintf(" to %x.%d\n", addr->sin_addr.s_addr, addr->sin_port);
	}
}
#endif
	Sendtries++;
#ifdef sgi
	if (Dofast) {
		if ((error = ku_fastsend(so, m, addr)) == 0) {
			Sendok++;
			Fastok++;
			return (0);
		}
	} else {
		error = -2;
	}
#else
	if ((error = ku_fastsend(so, m, addr)) == 0) {
		Sendok++;
		return (0);
	}
#endif
	/*
	 *  if ku_fastsend returns -2, then we can try to send m the
	 *  slow way.  else m was freed and we return ENOBUFS.
	 */
	if (error != -2) {
#ifdef RPCDEBUG
		rpc_debug(3, "ku_sendto_mbuf: fastsend failed\n");
#endif
		return (ENOBUFS);
	}
	s = splnet();
	laddr = inp->inp_laddr;
	if (error = in_pcbsetaddr(inp, addr)) {
		printf("pcbsetaddr failed %d\n", error);
		(void) splx(s);
		m_freem(m);
		return (error);
	}
	error = udp_output(inp, m);
	in_pcbdisconnect(inp);
	inp->inp_laddr = laddr;
#ifdef RPCDEBUG
	rpc_debug(4, "ku_sendto returning %d\n", error);
#endif
	Sendok++;
	(void) splx(s);
	return (error);
}

#ifdef RPCDEBUG
#ifdef SVR3
extern int rpcdebug;
#else
int rpcdebug = 0;
#endif

/*VARARGS2*/
rpc_debug(level, str, a1, a2, a3, a4, a5, a6, a7, a8, a9)
	int level;
	char *str;
	int a1, a2, a3, a4, a5, a6, a7, a8, a9;
{

	if (level <= rpcdebug)
		iprintf(str, a1, a2, a3, a4, a5, a6, a7, a8, a9);
}
#endif
