/*
 * Multibus network interface support code.
 *
 * Written by: Kipp Hickman
 *
 * $Source: /d2/3.7/src/sys/multibusif/RCS/if_mb.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:32:09 $
 */
#include "../h/param.h"
#include "../net/mbuf.h"
#include "../net/socket.h"
#include "../net/if.h"

/*
 * Pull read data off a interface.
 * Len is length of data, with local net header stripped.
 * Off is non-zero if a trailer protocol was used, and
 * gives the offset of the trailer information.
 * We copy the trailer information and then all the normal
 * data into mbufs.  When full cluster sized units are present
 * on the interface on cluster boundaries we can get them more
 * easily by remapping, and take advantage of this here.
 * Prepend a pointer to the interface structure,
 * so that protocols can determine where incoming packets arrived.
 */
struct mbuf *
if_mbget(cp, totlen, off, ifp, hlen)
	char *cp;
	int totlen;
	int off;
	struct ifnet *ifp;
	int hlen;
{
	struct mbuf *top, **mp;
	register struct mbuf *m;
	register int len;
	register unsigned cplen;
	register caddr_t tgt;
	char *src;

	top = (struct mbuf *)0;
	mp = &top;
	src = cp + hlen + off;			/* copy trailer first */
	len = totlen - off;
	totlen -= len;
	for (;;) {
		if (!len) {
			if (!totlen)
				return (top);
			src = cp + hlen;
			len = totlen;
			totlen = 0;
		}

		MGET(m, M_DONTWAIT, MT_DATA);
		if (m == 0) {
			m_freem(top);
			return (0);
		}
		tgt = mtod(m, caddr_t);

		if (ifp) {
			/*
			 * Prepend interface pointer to first mbuf.
			 */
			*(struct ifnet **)tgt = ifp;
			ifp = (struct ifnet *)0;
			m->m_len = sizeof(ifp);
			tgt += sizeof(ifp);
			cplen = MLEN - sizeof(ifp);
		} else {
			m->m_len = 0;
			cplen = MLEN;
		}
		if (len < cplen) cplen = len;
		bcopy(src, tgt, cplen);
		src += cplen;
		len -= cplen;
		m->m_len += cplen;
		*mp = m;
		mp = &m->m_next;
	}
}

/*
 * Copy a chain of mbufs into a packet buffer.
 */
/*ARGSUSED*/
int
if_mbput(ifp, cp0, m)
	struct ifnet *ifp;
	caddr_t cp0;
	register struct mbuf *m;
{
	register struct mbuf *mp;
	register caddr_t cp;

	cp = cp0;
	while (m) {
		bcopy(mtod(m, caddr_t), cp, (unsigned)m->m_len);
		cp += m->m_len;
		MFREE(m, mp);
		m = mp;
	}
	return (cp - cp0);
}
