/* NFSSRC @(#)kudp_fastsend.c	2.2 86/05/13 */
#ifndef lint
static	char sccsid[] = "@(#)kudp_fastsend.c 1.1 86/02/03 Copyr 1985 Sun Micro";
#endif

/*
 * Copyright (c) 1985 by Sun Microsystems, Inc.
 */

#ifdef SVR3
# include "sys/debug.h"
# include "sys/sysmacros.h"
# include "sys/types.h"
# include "sys/param.h"
# include "sys/mbuf.h"
# include "sys/socket.h"
# include "sys/socketvar.h"
# include "sys/file.h"
# include "net/if.h"
# include "net/route.h"
# include "netinet/in.h"
# include "netinet/if_ether.h"
# include "netinet/in_pcb.h"
# include "netinet/in_systm.h"
# include "netinet/ip.h"
# include "netinet/ip_var.h"
# include "netinet/udp.h"
#else
# include "../h/param.h"
# include "../h/mbuf.h"
# include "../h/socket.h"
# include "../h/socketvar.h"
# include "../h/file.h"
# include "../net/if.h"
# include "../net/route.h"
# include "../netinet/in.h"
# include "../netinet/if_ether.h"
# include "../netinet/in_pcb.h"
# include "../netinet/in_systm.h"
# include "../netinet/ip.h"
# include "../netinet/ip_var.h"
# include "../netinet/udp.h"
#endif

#ifndef sgi
static
buffree(m)
	register struct mbuf *m;
{
#ifdef RPCDEBUG
	rpc_debug(5, "ku_fastsend buffree: m=%x, mtod=%x\n",
	    m, mtod(m, caddr_t));
#endif
}
#endif

/*
 *  Take the mbuf chain at am, add ip/udp headers, and fragment it
 *  for the interface output routine.  Once a chain is sent to the
 *  interface, it is freed upon return.  It is possible for one
 *  fragment to succeed, and another to fail.  If this happens
 *  we must free the remaining mbuf chain (at am).  The caller
 *  must assume that the entire send failed if one fragment failed.
 *  If we get an error before a fregment is sent, then the original
 *  chain is intact and the caller may take other action.
 *
 *  Return codes:
 *	 0:  send ok
 *	-1:  send failed; mchain freed
 *	-2:  send failed; mchain intact
 */

ku_fastsend(so, am, to)
	struct socket *so;		/* socket data is sent from */
	register struct mbuf *am;	/* data to be sent */
	struct sockaddr_in *to;		/* destination data is sent to */
{
	register int datalen;		/* length of all data in packet */
	register int maxlen;		/* max length of fragment */
	register int curlen;		/* data fragment length */
	register int fragoff;		/* data fragment offset */
	register int sum;		/* ip header checksum */
	register int grablen;		/* number of mbuf bytes to grab */
	register struct ip *ip;		/* ip header */
	register struct mbuf *m;	/* ip header mbuf */
	struct ifnet *ifp;		/* interface */
	struct mbuf *lam;		/* last mbuf in chain to be sent */
	struct sockaddr	*dst;		/* packet destination */
	struct inpcb *inp;		/* inpcb for binding */
	struct ip *nextip;		/* ip header for next fragment */
	struct route route;		/* route to send packet */
	static struct route zero_route;	/* to initialize route */
	int err;			/* error number */
	int sentpck = 0;		/* set if we try to send a pck */

	/*
	 * Determine length of data.
	 * This should be passed in as a parameter.
	 */
	datalen = 0;
	for (m = am; m; m = m->m_next) {
		datalen += m->m_len;
	}
	/*
	 * Routing.
	 * We worry about routing early so we get the right ifp.
	 */
	{
		register struct route *ro;

		route = zero_route;
		ro = &route;
		ro->ro_dst.sa_family = AF_INET;
		((struct sockaddr_in *)&ro->ro_dst)->sin_addr = to->sin_addr;
		rtalloc(ro);
		if (ro->ro_rt == 0 || (ifp = ro->ro_rt->rt_ifp) == 0) {
			return(-2);
		}
		ro->ro_rt->rt_use++;
		if (ro->ro_rt->rt_flags & RTF_GATEWAY) {
			dst = &ro->ro_rt->rt_gateway;
		} else {
			dst = &ro->ro_dst;
		}
	}
	/*
	 * Get mbuf for ip, udp headers.
	 */
	MGET(m, M_WAIT, MT_HEADER);
	if (m == NULL) {
		printf("ku_fastsend: ip/udp hdr MGET failed\n");
		return(-2);
	}
	m->m_off = MMINOFF + sizeof (struct ether_header);
	m->m_len = sizeof (struct ip) + sizeof (struct udphdr);
	/*
	 * Create IP header.
	 */
	ip = mtod(m, struct ip *);
	ip->ip_hl = sizeof (struct ip) >> 2;
	ip->ip_v = IPVERSION;
	ip->ip_tos = 0;
	ip->ip_id = ip_id++;
	ip->ip_off = 0;
	ip->ip_ttl = MAXTTL;
	ip->ip_p = IPPROTO_UDP;
	ip->ip_sum = 0;			/* is this necessary? */
#ifdef sgi
	{	/* XXX can't we do better than this loop? */
		register struct ifaddr *ifa;
#define	ifatosin(ifa)	((struct sockaddr_in *) &(ifa)->ifa_addr)

		for (ifa = ifp->if_addrlist; ifa; ifa = ifa->ifa_next) {
	  		if (ifa->ifa_addr.sa_family == AF_INET) {
				ip->ip_src = ifatosin(ifa)->sin_addr;
				break;
			}
		}
		ASSERT(ifa != NULL);
#undef	ifatosin
	}
#else
	ip->ip_src = ((struct sockaddr_in *)&ifp->if_addr)->sin_addr;
#endif
	ip->ip_dst = to->sin_addr;
	/*
	 * Bind port, if necessary.
	 * Is this really necessary?
	 */
	inp = sotoinpcb(so);
	if (inp->inp_laddr.s_addr == INADDR_ANY && inp->inp_lport==0) {
		(void) in_pcbbind(inp, (struct mbuf *)0);
	}
	/*
	 * Create UDP header.
	 */
	{
		register struct udphdr *udp;

		udp = (struct udphdr *)(ip + 1);
		udp->uh_sport = inp->inp_lport;
		udp->uh_dport = to->sin_port;
		udp->uh_ulen = htons(sizeof (struct udphdr) + datalen);
		udp->uh_sum = 0;
	}
	/*
	 * Fragment the data into packets big enough for the
	 * interface, prepend the header, and send them off.
	 */
	maxlen = (ifp->if_mtu - sizeof (struct ip)) & ~7;
	curlen = sizeof (struct udphdr);
	fragoff = 0;
	for (;;) {
		register struct mbuf *mm;

		m->m_next = am;
		lam = m;
		while (am->m_len + curlen <= maxlen) {
			curlen += am->m_len;
			lam = am;
			am = am->m_next;
			if (am == 0) {
				ip->ip_off = htons((u_short) (fragoff >> 3));
				goto send;
			}
		}
		if (curlen == maxlen) {
			/*
			 * Incredible luck: last mbuf exactly
			 * filled out the packet.
			 */
			lam->m_next = 0;
		} else {
			/*
			 * We can squeeze part of the next
			 * mbuf into this packet, so we
			 * get a type 2 mbuf and point it at
			 * this data fragment.
			 */
#ifdef sgi
			grablen = maxlen - curlen;
			mm = m_copy(am, 0, grablen);
#else
			MGET(mm, M_WAIT, MT_DATA);
#endif
			if (mm == NULL) {
				/*
				 *  if already sent a pck, free entire
				 *  chain; else just free the ip/udp hdr.
				 */
				if (sentpck)
					m_freem(m);
				else
					(void) m_free(m);
				mm = dtom(ip);
				if (mm != m) {
					(void) m_free(mm);
				}
				printf("ku_fastsend: frag MGET failed\n");
				return (sentpck? -1 : -2);
			}
#ifndef sgi
			grablen = maxlen - curlen;
			mm->m_off = mtod(am, int) - (int) mm;
			mm->m_len = grablen;
			mm->m_cltype = 2;
			mm->m_clfun = buffree;
			mm->m_clswp = NULL;
#endif
			lam->m_next = mm;
			am->m_len -= grablen;
			am->m_off += grablen;
			curlen = maxlen;
		}
		/*
		 * m now points to the head of an mbuf chain which
		 * contains the max amount that can be sent in a packet.
		 */
		ip->ip_off = htons((u_short) ((fragoff >> 3) | IP_MF));
		/*
		 * There are more frags, so we save
		 * a copy of the ip hdr for the next
		 * frag.
		 */
		MGET(mm, M_WAIT, MT_HEADER);
		if (mm == 0) {
			printf("ku_fastsend: dup ip/udp hdr MGET failed\n");
			/* free entire chain if sent any frag */
			if (sentpck) {
				m_freem(m);	/* includes ip hdr */
				return (-1);
			} else {
				(void) m_free(dtom(ip));  /* just the hdr */
				return (-2);
			}
		}
		mm->m_off = MMINOFF + sizeof (struct ether_header);
		mm->m_len = sizeof (struct ip);
		nextip = mtod(mm, struct ip *);
		*nextip = *ip;
send:
		/*
		 * Set ip_len and calculate the ip header checksum.
		 */
		ip->ip_len = htons(sizeof (struct ip) + curlen);
#define	ips ((u_short *) ip)
		sum = ips[0] + ips[1] + ips[2] + ips[3] + ips[4] + ips[6] +
			ips[7] + ips[8] + ips[9];
		ip->ip_sum = ~(sum + (sum >> 16));
#undef ips
		/*
		 * At last, we send it off to the ethernet.
		 */
#ifdef RPCDEBUG
		rpc_debug(5, "ku_fastsend: am=%x, sending %x\n", am, m);
#endif
		if (err = (*ifp->if_output)(ifp, m, dst)) {
			/*
			 * mbuf chain m has been freed at this point.
			 * am and nextip (if nonnull) must be freed here
			 */
			printf("\
ku_fastsend: if_output failed: error=%d, sentpck=%d, am=%x\n",err,sentpck,am);
			if (am) {
				m_free(dtom(nextip));
				m_freem(am);
			}
			return (-1);
		}
		if (am == 0) {
			return (0);
		}
		sentpck = 1;
		ip = nextip;
		m = dtom(ip);
		fragoff += curlen;
		curlen = 0;
	}
}

#ifdef DEBUG
pr_mbuf(p, m)
	char *p;
	struct mbuf *m;
{
	register char *cp, *cp2;
	register struct ip *ip;
	register int len;

	len = 28;
	printf("%s: ", p);
	if (m && m->m_len >= 20) {
		ip = mtod(m, struct ip *);
		printf("hl %d v %d tos %d len %d id %d mf %d off %d ttl %d p %d sum %d src %x dst %x\n",
			ip->ip_hl, ip->ip_v, ip->ip_tos, ip->ip_len,
			ip->ip_id, ip->ip_off >> 13, ip->ip_off & 0x1fff,
			ip->ip_ttl, ip->ip_p, ip->ip_sum, ip->ip_src.s_addr,
			ip->ip_dst.s_addr);
		len = 0;
		printf("m %x addr %x len %d\n", m, mtod(m, caddr_t), m->m_len);
		m = m->m_next;
	} else if (m) {
		printf("pr_mbuf: m_len %d\n", m->m_len);
	} else {
		printf("pr_mbuf: zero m\n");
	}
	while (m) {
		printf("m %x addr %x len %d\n", m, mtod(m, caddr_t), m->m_len);
		cp = mtod(m, caddr_t);
		cp2 = cp + m->m_len;
		while (cp < cp2) {
			if (len-- < 0) {
				break;
			}
			printf("%x ", *cp & 0xFF);
			cp++;
		}
		m = m->m_next;
		printf("\n");
	}
}
#endif DEBUG
