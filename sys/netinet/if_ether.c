/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)if_ether.c	7.1.1.1 (Berkeley) 2/27/87
 */

/*
 * Ethernet address resolution protocol.
 */

#ifdef mips
#include "../tcp-param.h"
#include "sys/param.h"
#include "sys/mbuf.h"
#include "sys/socket.h"
#include "sys/errno.h"
#else
#include "../h/param.h"
#include "../h/systm.h"
#include "../h/mbuf.h"
#include "../h/socket.h"
#include "../h/errno.h"
#endif

#define hz HZ

#include "../net/soioctl.h"
#include "../net/if.h"
#include "in.h"
#include "in_systm.h"
#include "ip.h"
#include "if_ether.h"

#ifdef sgi
#define	ARPTAB_BSIZ	8		/* bucket size */
#define	ARPTAB_NB	41		/* number of buckets */
#else
#define	ARPTAB_BSIZ	9		/* bucket size */
#define	ARPTAB_NB	19		/* number of buckets */
#endif
#define	ARPTAB_SIZE	(ARPTAB_BSIZ * ARPTAB_NB)
struct	arptab arptab[ARPTAB_SIZE];
int	arptab_size = ARPTAB_SIZE;	/* for arp command */

/*
 * ARP trailer negotiation.  Trailer protocol is not IP specific,
 * but ARP request/response use IP addresses.
 */
#define ETHERTYPE_IPTRAILERS ETHERTYPE_TRAIL

#define	ARPTAB_HASH(a) \
	((u_long)(a) % ARPTAB_NB)

#define	ARPTAB_LOOK(at,addr) { \
	register n; \
	at = &arptab[ARPTAB_HASH(addr) * ARPTAB_BSIZ]; \
	for (n = 0 ; n < ARPTAB_BSIZ ; n++,at++) \
		if (at->at_iaddr.s_addr == addr) \
			break; \
	if (n >= ARPTAB_BSIZ) \
		at = 0; \
}

/* timer values */
#define	ARPT_AGE	(60*1)	/* aging timer, 1 min. */
#define	ARPT_KILLC	20	/* kill completed entry in 20 mins. */
#define	ARPT_KILLI	3	/* kill incomplete entry in 3 minutes */

u_char	etherbroadcastaddr[6] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
extern struct ifnet loif;

/*
 * Timeout routine.  Age arp_tab entries once a minute.
 */
arptimer()
{
	register struct arptab *at;
	register i;

	timeout(arptimer, (caddr_t)0, ARPT_AGE * hz);
	at = &arptab[0];
	for (i = 0; i < ARPTAB_SIZE; i++, at++) {
		if (at->at_flags == 0 || (at->at_flags & ATF_PERM))
			continue;
		if (++at->at_timer < ((at->at_flags&ATF_COM) ?
		    ARPT_KILLC : ARPT_KILLI))
			continue;
		/* timer has expired, clear entry */
		arptfree(at);
	}
}

/*
 * Broadcast an ARP packet, asking who has addr on interface ac.
 */
arpwhohas(ac, addr)
	register struct arpcom *ac;
	struct in_addr *addr;
{
	register struct mbuf *m;
	register struct ether_header *eh;
	register struct ether_arp *ea;
	struct sockaddr sa;

	if ((m = m_get(M_DONTWAIT, MT_DATA)) == NULL)
		return;
	m->m_len = sizeof *ea;
	m->m_off = MMAXOFF - m->m_len;
	ea = mtod(m, struct ether_arp *);
	eh = (struct ether_header *)sa.sa_data;
	bzero((caddr_t)ea, sizeof (*ea));
	bcopy((caddr_t)etherbroadcastaddr, (caddr_t)eh->ether_dhost,
	    sizeof(eh->ether_dhost));
	eh->ether_type = ETHERTYPE_ARP;		/* if_output will swap */
	ea->arp_hrd = htons(ARPHRD_ETHER);
	ea->arp_pro = htons(ETHERTYPE_IP);
	ea->arp_hln = sizeof(ea->arp_sha);	/* hardware address length */
	ea->arp_pln = sizeof(ea->arp_spa);	/* protocol address length */
	ea->arp_op = htons(ARPOP_REQUEST);
	bcopy((caddr_t)ac->ac_enaddr, (caddr_t)ea->arp_sha,
	   sizeof(ea->arp_sha));
	bcopy((caddr_t)&ac->ac_ipaddr, (caddr_t)ea->arp_spa,
	   sizeof(ea->arp_spa));
	bcopy((caddr_t)addr, (caddr_t)ea->arp_tpa, sizeof(ea->arp_tpa));
	sa.sa_family = AF_UNSPEC;
	(*ac->ac_if.if_output)(&ac->ac_if, m, &sa);
}

#ifdef SVR3
extern int useloopback;		/* use loopback interface for local traffic */
#else
int	useloopback = 1;	/* use loopback interface for local traffic */
#endif

/*
 * Resolve an IP address into an ethernet address.  If success, 
 * desten is filled in.  If there is no entry in arptab,
 * set one up and broadcast a request for the IP address.
 * Hold onto this mbuf and resend it once the address
 * is finally resolved.  A return value of 1 indicates
 * that desten has been filled in and the packet should be sent
 * normally; a 0 return indicates that the packet has been
 * taken over here, either now or for later transmission.
 *
 * We do some (conservative) locking here at splimp, since
 * arptab is also altered from input interrupt service (ecintr/ilintr
 * calls arpinput when ETHERTYPE_ARP packets come in).
 */
arpresolve(ac, m, destip, desten, usetrailers)
	register struct arpcom *ac;
	struct mbuf *m;
	register struct in_addr *destip;
	register u_char *desten;
	int *usetrailers;
{
	register struct arptab *at;
	register struct ifnet *ifp;
	struct sockaddr_in sin;
	int s, lna;

	*usetrailers = 0;
	if (in_broadcast(*destip)) {	/* broadcast address */
		bcopy((caddr_t)etherbroadcastaddr, (caddr_t)desten,
		    sizeof(etherbroadcastaddr));
		return (1);
	}
	lna = in_lnaof(*destip);
	ifp = &ac->ac_if;
	/* if for us, use software loopback driver if up */
	if (destip->s_addr == ac->ac_ipaddr.s_addr) {
		if (useloopback) {
			sin.sin_family = AF_INET;
			sin.sin_addr = *destip;
			(void) looutput(&loif, m, (struct sockaddr *)&sin);
			/*
			 * The packet has already been sent and freed.
			 */
			return (0);
		} else {
			bcopy((caddr_t)ac->ac_enaddr, (caddr_t)desten,
			    sizeof(ac->ac_enaddr));
			return (1);
		}
	}
	s = splimp();
	ARPTAB_LOOK(at, destip->s_addr);
	if (at == 0) {			/* not found */
		if (ifp->if_flags & IFF_NOARP) {
			bcopy((caddr_t)ac->ac_enaddr, (caddr_t)desten, 3);
			desten[3] = (lna >> 16) & 0x7f;
			desten[4] = (lna >> 8) & 0xff;
			desten[5] = lna & 0xff;
			splx(s);
			return (1);
		} else {
			at = arptnew(destip);
			at->at_hold = m;
			arpwhohas(ac, destip);
			splx(s);
			return (0);
		}
	}
	at->at_timer = 0;		/* restart the timer */
	if (at->at_flags & ATF_COM) {	/* entry IS complete */
		bcopy((caddr_t)at->at_enaddr, (caddr_t)desten,
		    sizeof(at->at_enaddr));
		if (at->at_flags & ATF_USETRAILERS)
			*usetrailers = 1;
		splx(s);
		return (1);
	}
	/*
	 * There is an arptab entry, but no ethernet address
	 * response yet.  Replace the held mbuf with this
	 * latest one.
	 */
	if (at->at_hold)
		m_freem(at->at_hold);
	at->at_hold = m;
	arpwhohas(ac, destip);		/* ask again */
	splx(s);
	return (0);
}

/*
 * Called from 10 Mb/s Ethernet interrupt handlers
 * when ether packet type ETHERTYPE_ARP
 * is received.  Common length and type checks are done here,
 * then the protocol-specific routine is called.
 */
arpinput(ac, m)
	struct arpcom *ac;
	struct mbuf *m;
{
	register struct arphdr *ar;

	if (ac->ac_if.if_flags & IFF_NOARP)
		goto out;
	IF_ADJ(m);
	if (m->m_len < sizeof(struct arphdr))
		goto out;
	ar = mtod(m, struct arphdr *);
	if (ntohs(ar->ar_hrd) != ARPHRD_ETHER)
		goto out;
	if (m->m_len < sizeof(struct arphdr) + 2 * ar->ar_hln + 2 * ar->ar_pln)
		goto out;

	switch (ntohs(ar->ar_pro)) {

	case ETHERTYPE_IP:
	case ETHERTYPE_IPTRAILERS:
		in_arpinput(ac, m);
		return;

	default:
		break;
	}
out:
	m_freem(m);
}

/*
 * ARP for Internet protocols on 10 Mb/s Ethernet.
 * Algorithm is that given in RFC 826.
 * In addition, a sanity check is performed on the sender
 * protocol address, to catch impersonators.
 * We also handle negotiations for use of trailer protocol:
 * ARP replies for protocol type ETHERTYPE_TRAIL are sent
 * along with IP replies if we want trailers sent to us,
 * and also send them in response to IP replies.
 * This allows either end to announce the desire to receive
 * trailer packets.
 * We reply to requests for ETHERTYPE_TRAIL protocol as well,
 * but don't normally send requests.
 */
in_arpinput(ac, m)
	register struct arpcom *ac;
	struct mbuf *m;
{
	register struct ether_arp *ea;
	struct ether_header *eh;
	register struct arptab *at;  /* same as "merge" flag */
	struct mbuf *mcopy = 0;
	struct sockaddr_in sin;
	struct sockaddr sa;
	struct in_addr isaddr, itaddr, myaddr;
	int proto, op, s;

	myaddr = ac->ac_ipaddr;
	ea = mtod(m, struct ether_arp *);
	proto = ntohs(ea->arp_pro);
	op = ntohs(ea->arp_op);
#if defined(sgi) && defined(mips)
	bcopy((char*)&((struct in_addr*)ea->arp_spa)->s_addr,
	      (char*)&isaddr.s_addr,	/* not on a double word bndry */
	      sizeof(isaddr.s_addr));	/* so we must do it hard way */
	bcopy((char*)&((struct in_addr*)ea->arp_tpa)->s_addr,
	      (char*)&itaddr.s_addr,
	      sizeof(itaddr.s_addr));
#else
	isaddr.s_addr = ((struct in_addr *)ea->arp_spa)->s_addr;
	itaddr.s_addr = ((struct in_addr *)ea->arp_tpa)->s_addr;
#endif
	if (!bcmp((caddr_t)ea->arp_sha, (caddr_t)ac->ac_enaddr,
	  sizeof (ea->arp_sha)))
		goto out;	/* it's from me, ignore it. */
	if (!bcmp((caddr_t)ea->arp_sha, (caddr_t)etherbroadcastaddr,
	    sizeof (ea->arp_sha))) {
#ifdef sgi
		printf("arp: ether address is broadcast for IP address %x!\n",
		       ntohl(isaddr.s_addr));
#else
		log(LOG_ERR,
		    "arp: ether address is broadcast for IP address %x!\n",
		    ntohl(isaddr.s_addr));
#endif
		goto out;
	}
	if (isaddr.s_addr == myaddr.s_addr) {
#ifdef sgi
		printf("duplicate IP address!--sent from ethernet addr: %s\n",
		       ether_sprintf(ea->arp_sha));
#else
		log(LOG_ERR, "%s: %s\n",
			"duplicate IP address!! sent from ethernet address",
			ether_sprintf(ea->arp_sha));
#endif
		itaddr = myaddr;
		if (op == ARPOP_REQUEST)
			goto reply;
		goto out;
	}
	s = splimp();
	ARPTAB_LOOK(at, isaddr.s_addr);
	if (at) {
		bcopy((caddr_t)ea->arp_sha, (caddr_t)at->at_enaddr,
		    sizeof(ea->arp_sha));
		at->at_flags |= ATF_COM;
		if (at->at_hold) {
			sin.sin_family = AF_INET;
			sin.sin_addr = isaddr;
			(*ac->ac_if.if_output)(&ac->ac_if, 
			    at->at_hold, (struct sockaddr *)&sin);
			at->at_hold = 0;
		}
	}
	if (at == 0 && itaddr.s_addr == myaddr.s_addr) {
		/* ensure we have a table entry */
		at = arptnew(&isaddr);
		bcopy((caddr_t)ea->arp_sha, (caddr_t)at->at_enaddr,
		    sizeof(ea->arp_sha));
		at->at_flags |= ATF_COM;
	}
	splx(s);
reply:
	switch (proto) {

	case ETHERTYPE_IPTRAILERS:
		/* partner says trailers are OK */
		if (at)
			at->at_flags |= ATF_USETRAILERS;
		/*
		 * Reply to request iff we want trailers.
		 */
		if (op != ARPOP_REQUEST || ac->ac_if.if_flags & IFF_NOTRAILERS)
			goto out;
		break;

	case ETHERTYPE_IP:
		/*
		 * Reply if this is an IP request, or if we want to send
		 * a trailer response.
		 */
		if (op != ARPOP_REQUEST && ac->ac_if.if_flags & IFF_NOTRAILERS)
			goto out;
	}
	if (itaddr.s_addr == myaddr.s_addr) {
		/* I am the target */
		bcopy((caddr_t)ea->arp_sha, (caddr_t)ea->arp_tha,
		    sizeof(ea->arp_sha));
		bcopy((caddr_t)ac->ac_enaddr, (caddr_t)ea->arp_sha,
		    sizeof(ea->arp_sha));
	} else {
		ARPTAB_LOOK(at, itaddr.s_addr);
		if (at == NULL || (at->at_flags & ATF_PUBL) == 0)
			goto out;
		bcopy((caddr_t)ea->arp_sha, (caddr_t)ea->arp_tha,
		    sizeof(ea->arp_sha));
		bcopy((caddr_t)at->at_enaddr, (caddr_t)ea->arp_sha,
		    sizeof(ea->arp_sha));
	}

	bcopy((caddr_t)ea->arp_spa, (caddr_t)ea->arp_tpa,
	    sizeof(ea->arp_spa));
	bcopy((caddr_t)&itaddr, (caddr_t)ea->arp_spa,
	    sizeof(ea->arp_spa));
	ea->arp_op = htons(ARPOP_REPLY); 
	/*
	 * If incoming packet was an IP reply,
	 * we are sending a reply for type IPTRAILERS.
	 * If we are sending a reply for type IP
	 * and we want to receive trailers,
	 * send a trailer reply as well.
	 */
	if (op == ARPOP_REPLY)
		ea->arp_pro = htons(ETHERTYPE_IPTRAILERS);
	else if (proto == ETHERTYPE_IP &&
	    (ac->ac_if.if_flags & IFF_NOTRAILERS) == 0)
		mcopy = m_copy(m, 0, (int)M_COPYALL);
	eh = (struct ether_header *)sa.sa_data;
	bcopy((caddr_t)ea->arp_tha, (caddr_t)eh->ether_dhost,
	    sizeof(eh->ether_dhost));
	eh->ether_type = ETHERTYPE_ARP;
	sa.sa_family = AF_UNSPEC;
	(*ac->ac_if.if_output)(&ac->ac_if, m, &sa);
	if (mcopy) {
		ea = mtod(mcopy, struct ether_arp *);
		ea->arp_pro = htons(ETHERTYPE_IPTRAILERS);
		(*ac->ac_if.if_output)(&ac->ac_if, mcopy, &sa);
	}
	return;
out:
	m_freem(m);
	return;
}

/*
 * Free an arptab entry.
 */
arptfree(at)
	register struct arptab *at;
{
	int s = splimp();

	if (at->at_hold)
		m_freem(at->at_hold);
	at->at_hold = 0;
	at->at_timer = at->at_flags = 0;
	at->at_iaddr.s_addr = 0;
	splx(s);
}

/*
 * Enter a new address in arptab, pushing out the oldest entry 
 * from the bucket if there is no room.
 * This always succeeds since no bucket can be completely filled
 * with permanent entries (except from arpioctl when testing whether
 * another permanent entry will fit).
 */
struct arptab *
arptnew(addr)
	struct in_addr *addr;
{
	register n;
	int oldest = -1;
	register struct arptab *at, *ato = NULL;
	static int first = 1;

	if (first) {
		first = 0;
		timeout(arptimer, (caddr_t)0, hz);
	}
	at = &arptab[ARPTAB_HASH(addr->s_addr) * ARPTAB_BSIZ];
	for (n = 0; n < ARPTAB_BSIZ; n++,at++) {
		if (at->at_flags == 0)
			goto out;	 /* found an empty entry */
		if (at->at_flags & ATF_PERM)
			continue;
		if (at->at_timer > oldest) {
			oldest = at->at_timer;
			ato = at;
		}
	}
	if (ato == NULL)
		return (NULL);
	at = ato;
	arptfree(at);
out:
	at->at_iaddr = *addr;
	at->at_flags = ATF_INUSE;
	return (at);
}

arpioctl(cmd, data)
	int cmd;
	caddr_t data;
{
	register struct arpreq *ar = (struct arpreq *)data;
	register struct arptab *at;
	register struct sockaddr_in *sin;
	int s;

	if (ar->arp_pa.sa_family != AF_INET ||
	    ar->arp_ha.sa_family != AF_UNSPEC)
		return (EAFNOSUPPORT);
	sin = (struct sockaddr_in *)&ar->arp_pa;
	s = splimp();
	ARPTAB_LOOK(at, sin->sin_addr.s_addr);
	if (at == NULL) {		/* not found */
		if (cmd != SIOCSARP) {
			splx(s);
			return (ENXIO);
		}
		if (ifa_ifwithnet(&ar->arp_pa) == NULL) {
			splx(s);
			return (ENETUNREACH);
		}
	}
	switch (cmd) {

	case SIOCSARP:		/* set entry */
		if (at == NULL) {
			at = arptnew(&sin->sin_addr);
			if (ar->arp_flags & ATF_PERM) {
			/* never make all entries in a bucket permanent */
				register struct arptab *tat;
				
				/* try to re-allocate */
				tat = arptnew(&sin->sin_addr);
				if (tat == NULL) {
					arptfree(at);
					splx(s);
					return (EADDRNOTAVAIL);
				}
				arptfree(tat);
			}
		}
		bcopy((caddr_t)ar->arp_ha.sa_data, (caddr_t)at->at_enaddr,
		    sizeof(at->at_enaddr));
		at->at_flags = ATF_COM | ATF_INUSE |
			(ar->arp_flags & (ATF_PERM|ATF_PUBL));
		at->at_timer = 0;
		break;

	case SIOCDARP:		/* delete entry */
		arptfree(at);
		break;

	case SIOCGARP:		/* get entry */
		bcopy((caddr_t)at->at_enaddr, (caddr_t)ar->arp_ha.sa_data,
		    sizeof(at->at_enaddr));
		ar->arp_flags = at->at_flags;
		break;
	}
	splx(s);
	return (0);
}

/*
 * Convert Ethernet address to printable (loggable) representation.
 */
char *
ether_sprintf(ap)
	register u_char *ap;
{
	register i;
	static char etherbuf[18];
	register char *cp = etherbuf;
	static char digits[] = "0123456789abcdef";

	for (i = 0; i < 6; i++) {
		*cp++ = digits[*ap >> 4];
		*cp++ = digits[*ap++ & 0xf];
		*cp++ = ':';
	}
	*--cp = 0;
	return (etherbuf);
}
