/*
 * Excelan EXOS 204 Interface
 *
 *	George Powers
 *	Excelan Inc.
 *
 * Ported to SGI os by Kipp Hickman
 *
 * $Source: /d2/3.7/src/sys/multibusif/RCS/if_ex.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:32:02 $
 */
#include "ex.h"

#include "machine/pte.h"
#include "../h/param.h"
#include "../h/systm.h"
#include "../h/proc.h"
#include "../h/cmap.h"
#include "../h/mbuf.h"
#include "../h/protosw.h"
#include "../h/socket.h"
#include "../vm/vmmac.h"
#include "../net/soioctl.h"
#include "../h/errno.h"
#include "../net/if.h"
#include "../net/netisr.h"
#include "../net/route.h"

#ifdef	INET
#include "../netinet/in.h"
#include "../netinet/in_systm.h"
#include "../netinet/in_var.h"
#include "../netinet/ip.h"
#include "../netinet/if_ether.h"
#endif

#ifdef NS
#include "../netns/ns.h"
#include "../netns/ns_if.h"
#endif

#include "machine/cpureg.h"
#include "if_exreg.h"
#include "../multibusif/if_mb.h"
#include "../multibus/mbvar.h"

#undef	NOISE
char	XXXetheraddr[6];
char	XXXetheraddr_initialized;

/*
 * These constants are set to allow parallel execution between the
 * host and the exos board.  We have observed that 2 is better than
 * 1, but more is not.  Anyway, keeping these numbers small will
 * save memory.
 */
#define	XMITS	2		/* number of xmit buffers */
#define	RCVS	2		/* number of rcv buffers */

#define	NH2X (2 + 2 + RCVS)	/* a sufficient number is critical */
#define	NX2H (2 + 2 + RCVS)	/* this is pretty arbitrary */
#define	EXWATCHINTVL 10		/* call exwatch() every 10 seconds */

int	exinit(),exoutput(),exioctl(),exwatch();
int	exprobe(), exstart(), exintr();
struct	mb_device *exdinfo[NEX];
struct	mb_driver exdriver = {
	exprobe, (int (*)())0, (int (*)())0, exstart, exintr,
	(char *(*)())0, "ex", exdinfo,
};
struct ex_msg *exgetcbuf();

/* xs_flag's */
#define	EX_STATPENDING	0x01		/* stats rqst pending on EXOS */
#define	EX_EXISTS	0x02		/* this device exists */

#define	XS_SBUF_NONE	15		/* flag indicating sbuf not in use */
					/* must be > XMITS */

/*
 * Ethernet software status per interface.
 *
 * Each interface is referenced by a network interface structure,
 * xs_if, which the routing code uses to locate the interface.
 * This structure contains the output queue for the interface, its address, ...
 */
struct	ex_softc {
	long	xs_resets;		/* # of board resets */
	long	xs_xmit_frags;		/* total fragments sent */
	long	xs_xmit_fragcopies;	/* xmit fragments copied */
	long	xs_xmit_fragmaps;	/* xmit fragments maps */
	long	xs_rcv_copies;		/* rcv packets copied */
	long	xs_rcv_maps;		/* rcv packets mapped */
	long	xs_swaits;		/* delays for xmit buffer */
	long	xs_sbufs;		/* # of times static buf is used */

	char	xs_flags;		/* private flags */
	char	xs_xmits_inprogress;	/* # of xmits currently in progress */
	char	xs_sbufslot;		/* xmit slot using static buf */

	/* Transmit support stuff */
	struct	mbuf *xs_mbuf[XMITS];	/* xmit mbufs for freeing */
	short	xs_xmit_mbuspage[XMITS];/* base of reserved mb regs for xmit */

	/* Receive support stuff */
	short	xs_rcv_mbuspage;	/* base of reserved mb regs */
	caddr_t	xs_rcvbufs;		/* kernel virtual addr of buffers */
	struct	pte *xs_rcv_pt;		/* pointer to page table for buffers */

	struct	exdevice *xs_csraddr;	/* points to csr registers */
	struct	arpcom xs_ac;		/* Ethernet common part */

	struct	ex_msg *xs_h2xnext;	/* host pointer to request queue */
	struct	ex_msg *xs_x2hnext;	/* host pointer to reply queue */

	/*
	 * xs_mbvaddr is the base multibus virtual address
	 * of xs_h2xhdr and onward
	 */
	u_long	xs_mbvaddr;		/* multibus virtual address */

	/* the following structures are always mapped in */
	u_short	xs_h2xhdr;		/* EXOS's request queue header */
	u_short	xs_x2hhdr;		/* EXOS's reply queue header */
	struct	ex_msg xs_h2xent[NH2X];	/* request msg buffers */
	struct	ex_msg xs_x2hent[NX2H];	/* reply msg buffers */
	char	xs_xmit[EXMAXRBUF];	/* single xmit buffer */
	struct	stat_array xs_xsa;	/* EXOS writes stats here */
	/* end mapped area */
	short	xs_end;			/* place holder... */
} ex_softc[NEX];

#define	INCORE_BASE(p)		(((u_long)(&(p)->xs_h2xhdr)) & 0xFFFFFFF0)
#define	RVAL_OFF(xs,n)		((u_long)(&((xs)->n)) - INCORE_BASE(xs))
#define	H2XHDR_OFFSET(xs)	RVAL_OFF(xs, xs_h2xhdr)
#define	X2HHDR_OFFSET(xs)	RVAL_OFF(xs, xs_x2hhdr)
#define	H2XENT_OFFSET(xs)	RVAL_OFF(xs, xs_h2xent[0])
#define	X2HENT_OFFSET(xs)	RVAL_OFF(xs, xs_x2hent[0])
#define	SA_OFFSET(xs)		RVAL_OFF(xs, xs_xsa)
#define	CM_OFFSET(xs)		RVAL_OFF(xs, xs_xmit[0])
#define	XMIT_OFF(xs)		RVAL_OFF(xs, xs_xmit[0])
#define	INCORE_SIZE(xs)		RVAL_OFF(xs, xs_end)

#define	P_MBADDR(xs)	(((xs)->xs_mbvaddr) & 0x00FFFFF0)

/* use xmit buffer 0 to hold configuration message */
#define	XS_CM(xs)	((struct confmsg *)&(xs)->xs_xmit[0])

char	ex_ncall = 0;			/* counts calls to exprobe */

exprobe(reg)
	caddr_t reg;
{
	register struct exdevice *addr = (struct exdevice *)(MBIO_VBASE + reg);
	extern time_t time;
	time_t otime;
	int unit;
	char dummy;

	unit = ex_ncall++;
#ifdef NOISE
	iprintf("exprobe%d: addr = %x\n", unit, reg);
#endif
	ex_softc[unit].xs_csraddr = addr;

	/*
	 * Reset EXOS and run self-test (guaranteed to
	 * complete within 2 seconds).
	 */
	dummy = addr->xd_porta;		/* reading port resets it */
#ifdef	lint
	dummy = dummy;
#endif
	otime = time;
	while ((addr->xd_portb & EX_TESTOK) == 0) {
		/* if approximately 4 seconds have expired, then punt */
		if (time >= otime + 4)
			break;
	}
	if ((addr->xd_portb & EX_TESTOK) == 0)
		return (CONF_DEAD);
	return (exattach(unit));
}

/*
 * Interface exists: make available by filling in network interface
 * record.  System will initialize the interface when it is ready
 * to accept packets.  Board is temporarily configured and issues
 * a NET_ADDRS command, only to get the Ethernet address.
 *
 * This is coded, on purpose, to throw away resouces for a deranged board.
 * Much simpler, and not the common case.  The system will work just fine
 * without the extra resources.
 */
int
exattach(unit)
	int unit;
{
	register struct ex_softc *xs = &ex_softc[unit];
	register struct exdevice *addr = xs->xs_csraddr;
	register struct ifnet *ifp = &xs->xs_ac.ac_if;
	register struct ex_msg *bp;
	register int i;
	int kmx;

	ifp->if_unit = unit;
	ifp->if_name = "ex";
	ifp->if_mtu = ETHERMTU;

	/*
	 * Map queues in order to configure EXOS
	 */
	xs->xs_mbvaddr = (u_long) mbmapkget((caddr_t)INCORE_BASE(xs),
					    INCORE_SIZE(xs));
	exconfig(unit, 0);		/* without interrupts */
	if (XS_CM(xs)->cm_cc)
		goto badconf;

	bp = exgetcbuf(xs);
	bp->mb_rqst = LLNET_ADDRS;
	bp->mb_na.na_mask = READ_OBJ;
	bp->mb_na.na_slot = PHYSSLOT;
	bp->mb_status |= MH_EXOS;
	addr->xd_portb = EX_NTRUPT;
	bp = xs->xs_x2hnext;
	while ((bp->mb_status & MH_OWNER) == MH_EXOS)	/* poll for reply */
		;

	ifp->if_init = exinit;
	ifp->if_output = exoutput;
	ifp->if_ioctl = exioctl;
	ifp->if_reset = 0;
	ifp->if_flags = IFF_BROADCAST|IFF_NOTRAILERS;
	/*
	 * Allocate bus mapping resources for transmit packets.  During
	 * transmission, we use the mapping registers to point to each
	 * of the mbuf's, avoiding copying the data to a transmit
	 * packet buffer.  We allocate twice as many bus registers as
	 * needed, in case data must cross a page boundary.
	 */
	for (i = 0; i < XMITS; i++) {
		xs->xs_xmit_mbuspage[i] = mballoc(NFRAGMENTS * 2, 0);
		if (xs->xs_xmit_mbuspage[i] < 0)
			goto badconf;
	}
	/*
	 * Allocate bus mapping resources for receive packets.  We
	 * reserve exactly RCVS pages worth of mapping registers,
	 * so that when we page flip an incomming packet, we can
	 * at the same time flip the mapping register.
	 */
	xs->xs_rcv_mbuspage = mballoc(RCVS, 0);
	if (xs->xs_rcv_mbuspage < 0)
		goto badconf;
	/*
	 * Now allocate kernel map resources to map the packet buffers.
	 * Then allocate virtual memory for the buffers, and map the
	 * buffers in.  Also init the dma map.
	 */
	kmx = kmap_alloc(RCVS, 0);
	if (kmx == 0)
		goto toss;
	if (memall(&Usrptmap[kmx], RCVS, (struct proc *)0, CSYS) == 0) {
		printf("ex%d: NO MEMORY\n", unit);
toss:
		goto badconf;
	}
	xs->xs_rcv_pt = &Usrptmap[kmx];
	xs->xs_rcvbufs = (caddr_t) kmxtob(kmx);
	vmaccess(xs->xs_rcv_pt, xs->xs_rcvbufs, RCVS);
	for (i = 0; i < RCVS; i++) {
		(void) mbset((xs->xs_rcv_pt + i)->pg_pfnum,
			     xs->xs_rcv_mbuspage + i, 0);
	}

	/* now that EVERYTHING's configured, print out address */
	printf("(FW %c.%c, HW %c.%c) (%02x%02x.%02x%02x.%02x%02x) ",
		XS_CM(xs)->cm_vc[0], XS_CM(xs)->cm_vc[1],
		XS_CM(xs)->cm_vc[2], XS_CM(xs)->cm_vc[3],
		bp->mb_na.na_addrs[0], bp->mb_na.na_addrs[1],
		bp->mb_na.na_addrs[2], bp->mb_na.na_addrs[3],
		bp->mb_na.na_addrs[4], bp->mb_na.na_addrs[5]);
	bcopy((caddr_t)bp->mb_na.na_addrs, (caddr_t)xs->xs_ac.ac_enaddr,
			sizeof (xs->xs_ac.ac_enaddr));
	if (!XXXetheraddr_initialized) {
		bcopy((caddr_t)bp->mb_na.na_addrs, XXXetheraddr, 6);
		XXXetheraddr_initialized = 1;
	}
	if_attach(ifp);
	xs->xs_flags |= EX_EXISTS;
	return (CONF_ALIVE);

badconf:
	return (CONF_DEAD);
}

/*
 * Initialization of interface; clear recorded pending
 * operations, and reinitialize UNIBUS usage.
 * Called at boot time (with interrupts disabled?),
 * and at ifconfig time via exioctl, with interrupts disabled.
 */
exinit(unit)
	int unit;
{
	register struct ex_softc *xs = &ex_softc[unit];
	register struct exdevice *addr = xs->xs_csraddr;
	register struct ifnet *ifp = &xs->xs_ac.ac_if;
	register struct ex_msg *bp;
	register int i;
	int s;

	/* not yet, if address still unknown */
	if (ifp->if_addrlist == (struct ifaddr *)0)
		return;

	if (ifp->if_flags & IFF_RUNNING)
		return;

	/*
	 * Clean out any software state.
	 */
	for (i = 0; i < XMITS; i++) {
		if (xs->xs_mbuf[i]) {
			m_freem(xs->xs_mbuf[i]);
			xs->xs_mbuf[i] = 0;
		}
	}
	xs->xs_sbufslot = XS_SBUF_NONE;
	xs->xs_xmits_inprogress = 0;

	exconfig(unit, 3);		/* with level interrupts */
	if (XS_CM(xs)->cm_cc) {
		ifp->if_flags &= ~IFF_RUNNING;
		printf("ex%d: board hung, going off net\n", unit);
		return;
	}

	/*
	 * Put EXOS on the Ethernet, using NET_MODE command
	 */
	bp = exgetcbuf(xs);
	bp->mb_rqst = LLNET_MODE;
	bp->mb_nm.nm_mask = WRITE_OBJ;
	bp->mb_nm.nm_optn = 0;
	bp->mb_nm.nm_mode = MODE_PERF;
	bp->mb_status |= MH_EXOS;
	addr->xd_portb = EX_NTRUPT;
	bp = xs->xs_x2hnext;
	while ((bp->mb_status & MH_OWNER) == MH_EXOS)	/* poll for reply */
		;
	bp->mb_length = MBDATALEN;
	bp->mb_status |= MH_EXOS;		/* free up buffer */
	addr->xd_portb = EX_NTRUPT;		/* tell EXOS about it */
	xs->xs_x2hnext = xs->xs_x2hnext->mb_next;

	ifp->if_watchdog = exwatch;
	ifp->if_timer = EXWATCHINTVL;
	s = splimp();	/* are interrupts always disabled here, anyway? */
	/*
	 * Hang receive requests
	 */
	for (i = 0; i < RCVS; i++)
		exhangrcv(unit, i);
	exstart(unit);				/* start transmits */
	xs->xs_ac.ac_if.if_flags |= IFF_RUNNING;
	splx(s);
}

/*
 * Reset, test, and configure EXOS.  This routine assumes
 * that message queues, etc. have already been mapped into
 * the multibus.  It is called by exinit, and should also be
 * callable by exattach.
 */
exconfig(unit, itype)
	int unit;
	int itype;
{
	register struct ex_softc *xs;
	register struct exdevice *addr;
	register struct confmsg *cm;
	register struct ex_msg *bp;
	int i;
	u_long shiftreg;
	time_t otime;
	char dummy;

	xs = &ex_softc[unit];
	addr = xs->xs_csraddr;
	cm = XS_CM(xs);
	xs->xs_flags &= ~EX_STATPENDING;
	/*
	 * Reset EXOS, wait for self-test to complete
	 */
	dummy = addr->xd_porta;		/* reading port resets it */
#ifdef	lint
	dummy = dummy;
#endif
	while ((addr->xd_portb & EX_TESTOK) == 0)
		;
	/*
	 * Set up configuration message.
	 */
	cm->cm_1rsrv = 1;
	cm->cm_cc = 0xFF;
	cm->cm_opmode = 0;		/* link-level controller mode */
	cm->cm_dfo = 0x0101;		/* enable host data order conversion */
	cm->cm_dcn1 = 1;
	cm->cm_2rsrv[0] = 0;
	cm->cm_2rsrv[1] = 0;
	cm->cm_ham = 3;			/* absolute address mode */
	cm->cm_3rsrv = 0;
	cm->cm_mapsiz = 0;
	cm->cm_byteptrn[0] = 0x01;	/* EXOS deduces data order of host */
	cm->cm_byteptrn[1] = 0x03;	/*  by looking at this pattern */
	cm->cm_byteptrn[2] = 0x07;
	cm->cm_byteptrn[3] = 0x0F;
	cm->cm_wordptrn[0] = 0x0103;
	cm->cm_wordptrn[1] = 0x070F;
	cm->cm_lwordptrn = 0x0103070F;
	for (i=0; i<20; i++) cm->cm_rsrvd[i] = 0;
	cm->cm_mba = 0xFFFFFFFF;
	cm->cm_nproc = 0xFF;
	cm->cm_nmbox = 0xFF;
	cm->cm_nmcast = 0xFF;
	cm->cm_nhost = 1;
	cm->cm_h2xba = P_MBADDR(xs);
	cm->cm_h2xhdr = H2XHDR_OFFSET(xs);
	cm->cm_h2xtyp = 0;		/* should never wait for rqst buffer */
	cm->cm_x2hba = cm->cm_h2xba;
	cm->cm_x2hhdr = X2HHDR_OFFSET(xs);
	cm->cm_x2htyp = itype;		/* 0 for none, 3 for vectored */
	cm->cm_x2haddr = 0;
	/*
	 * Set up message queues and headers.
	 * First the request queue.
	 */
	for (bp = &xs->xs_h2xent[0]; bp < &xs->xs_h2xent[NH2X]; bp++) {
		bp->mb_link = (u_short)((char *)(bp+1)-INCORE_BASE(xs));
		bp->mb_rsrv = 0;
		bp->mb_length = MBDATALEN;
		bp->mb_status = MH_HOST;
		bp->mb_next = bp+1;
	}
	xs->xs_h2xhdr = (u_short)H2XENT_OFFSET(xs);
	xs->xs_h2xent[NH2X-1].mb_link = (u_short)H2XENT_OFFSET(xs);
	xs->xs_h2xnext = xs->xs_h2xent;
	xs->xs_h2xent[NH2X-1].mb_next = xs->xs_h2xent;

	/* Now the reply queue. */
	for (bp = &xs->xs_x2hent[0]; bp < &xs->xs_x2hent[NX2H]; bp++) {
		bp->mb_link = (u_short)((char *)(bp+1)-INCORE_BASE(xs));
		bp->mb_rsrv = 0;
		bp->mb_length = MBDATALEN;
		bp->mb_status = MH_EXOS;
		bp->mb_next = bp+1;
	}
	xs->xs_x2hhdr = (u_short)X2HENT_OFFSET(xs);
	xs->xs_x2hent[NX2H-1].mb_link = (u_short)X2HENT_OFFSET(xs);
	xs->xs_x2hnext = xs->xs_x2hent;
	xs->xs_x2hent[NX2H-1].mb_next = xs->xs_x2hent;

	/*
	 * Write config msg address to EXOS and wait for
	 * configuration to complete (guaranteed response
	 * within 2 seconds).
	 */
	shiftreg = (u_long)0x0000FFFF;
	for (i = 0; i < 8; i++) {
		if (i == 4)
			shiftreg = P_MBADDR(xs) + CM_OFFSET(xs);
		while (addr->xd_portb & EX_UNREADY)
			;
		addr->xd_portb = (u_char)(shiftreg & 0xFF);
		shiftreg >>= 8;
	}
	otime = time;
	while (cm->cm_cc == 0xFF) {
		if (time > otime + 4)
			break;
	}
}

/*
 * Start or re-start output on interface.
 * Get another datagram to send off of the interface queue,
 * and map it to the interface before starting the output.
 * This routine is called by exinit(), exoutput(), and exintr().
 * In all cases, interrupts by EXOS are disabled.
 */
exstart(unit)
	int unit;
{
	register struct ex_softc *xs = &ex_softc[unit];
	register struct mbuf *m;
	register struct buf_blk *bb;
	register int nfrags;
	register int bytes;
	register int mbuspage;
	register int slot;
	struct mbuf *m0;
	struct ex_msg *bp;
	struct exdevice *addr = xs->xs_csraddr;
	int oldnfrags;

	/* find xmit slot to use, if any */
	for (slot = 0; slot < XMITS; slot++) {
		if (xs->xs_mbuf[slot] == NULL)
			goto found;
	}
	return;

found:
	IF_DEQUEUE(&xs->xs_ac.ac_if.if_snd, m);
	if (m == 0)
		return;
	/*
	 * Quickly count number of buffers and number of bytes
	 * on mbuf chain.
	 */
	bytes = nfrags = 0;
	m0 = m;
	while (m) {
		nfrags++;
		bytes += m->m_len;
		m = m->m_next;
	}
	m = m0;
	ASSERT(bytes <= EXMAXRBUF);
	oldnfrags = nfrags;

	/*
	 * If mbuf chain is too fragmented to fit in the interfaces
	 * scatter/gather array then copy the first set of mbuf's into
	 * the reserved transmit buffer until the number of fragments
	 * will fit.
	 */
	if (nfrags > NFRAGMENTS) {
		register char *cp;
		register int mlen;

		if (xs->xs_sbufslot != XS_SBUF_NONE) {
			/*
			 * The single xmit buffer is busy.  Delay this
			 * xmit until later.  This should not happen
			 * often, since higher level code attempts to
			 * insure that a small number of mbufs are used for
			 * each output.  Since the xmit buffer is busy,
			 * we know that another xmit is outstanding and
			 * will free up the buffer with a completion interrupt.
			 */
			xs->xs_swaits++;
			IF_PREPEND(&xs->xs_ac.ac_if.if_snd, m);
			return;
		}

		bp = exgetcbuf(xs);
		bb = &bp->mb_et.et_blks[0];
		xs->xs_sbufs++;
		xs->xs_sbufslot = slot;
		cp = &xs->xs_xmit[0];
		bb->bb_len = 0;
		*(u_long *)bb->bb_addr = P_MBADDR(xs) + XMIT_OFF(xs);
		while (nfrags >= NFRAGMENTS) {
			xs->xs_xmit_fragcopies++;
			mlen = m->m_len;
			bcopy(mtod(m, caddr_t), cp, (unsigned)mlen);
			cp += mlen;
			bb->bb_len += mlen;
			m = m_free(m);
			nfrags--;
		}

		/* count static "fragment" */
		bb++;
		nfrags++;
		ASSERT(nfrags == NFRAGMENTS);
		ASSERT(m != NULL);
	} else {
		bp = exgetcbuf(xs);
		bb = &bp->mb_et.et_blks[0];
	}

	/*
	 * Choose transmit buffer "slot" to use.  This decides which
	 * set of multibus mapping registers to use.  Record the
	 * decision in the exos buffer, so that when the interrupt
	 * occurs we can release the correct mbuf.
	 */
	bp->mb_rqst = LLRTRANSMIT;
	bp->mb_mid = slot;
	mbuspage = xs->xs_xmit_mbuspage[slot];
	xs->xs_mbuf[slot] = m;
	xs->xs_xmits_inprogress++;
	xs->xs_xmit_frags += oldnfrags;

	/*
	 * Map remainder of mbuf chain.  Point each multibus mapping
	 * register at the mbuf data, loading up the software scatter/gather
	 * state at the same time.
	 */
	while (m) {
		register long kva;
		register int kmx;
		register int pf;

		kva = mtod(m, long);
		kmx = btokmx((struct pte *) kva);
		ASSERT(m->m_len > 0);
		ASSERT((0 <= kmx) && (kmx < USRPTSIZE));

		/*
		 * Mbuf data regions are not allowed to cross two
		 * page boundaries.  Since the maximum packet size
		 * is less than a page, this shouldn't be a problem.
		 */
		pf = Usrptmap[kmx].pg_pfnum;
		ASSERT(pf);
		bb->bb_len = m->m_len;
		*(u_long *)bb->bb_addr = mbset(pf, mbuspage++, kva);
		/*
		 * See if this mbuf cross a page boundary.  If it does,
		 * then setup the next mapping register.
		 */
		if ((kva & ~PGOFSET) != ((kva + m->m_len - 1) & ~PGOFSET)) {
			pf = Usrptmap[++kmx].pg_pfnum;
			(void) mbset(pf, mbuspage++, 0);
		}
		bb++;
		m = m->m_next;
		xs->xs_xmit_fragmaps++;
	}

	/*
	 * Insure that packet length meets minimum daily
	 * requirements by lengthening the last scatter/gather
	 * chunk.
	 */
	if (bytes - sizeof(struct ether_header) < ETHERMIN)
		(bb - 1)->bb_len +=
		    ETHERMIN - (bytes - sizeof(struct ether_header));
	ASSERT(nfrags <= NFRAGMENTS);
#ifdef	NOISE
	iprintf(">%d", bytes);
#endif
	bp->mb_et.et_nblock = nfrags;
	bp->mb_status |= MH_EXOS;
	addr->xd_portb = EX_NTRUPT;
}

/*
 * Command done interrupt.  Return non-zero if this routine did something
 * with the interrupt.  This supports sharing of the interrupt level.
 */
int
exintr()
{
	register struct ex_softc *xs;
	register struct ex_msg *bp;
	register struct exdevice *addr;
	register int unit;
	register int slot;
	int handled;

	handled = 0;
	xs = &ex_softc[0];
	for (unit = 0; unit < ex_ncall; unit++, xs++) {
		/*
		 * Avoid referring to non-existant devices.
		 */
		if ((xs->xs_flags & EX_EXISTS) == 0)
			continue;
		bp = xs->xs_x2hnext;
		addr = xs->xs_csraddr;
		while ((bp->mb_status & MH_OWNER) == MH_HOST) {
			switch (bp->mb_rqst) {
			case LLRECEIVE:
				exrecv(unit, bp);
				exhangrcv(unit, bp->mb_mid);
				break;
			case LLRTRANSMIT:
				if (xs->xs_xmits_inprogress == 0) {
					printf("ex%d: spurious interrupt\n",
						      unit);
					break;
				}
				xs->xs_ac.ac_if.if_opackets++;
				if (bp->mb_rply == LL_OK) {
					;
				} else if (bp->mb_rply & LLXM_1RTRY) {
					xs->xs_ac.ac_if.if_collisions++;
				} else if (bp->mb_rply & LLXM_RTRYS) {
					/* guess collision count */
					xs->xs_ac.ac_if.if_collisions += 2;
				} else if (bp->mb_rply & LLXM_ERROR) {
					xs->xs_ac.ac_if.if_oerrors++;
					ex_xmit_err(unit, bp->mb_rply);
				}
				/*
				 * If transmit used scatter gather resources,
				 * then the mbuf needs to be freed now that
				 * the transmit completed.
				 */
				slot = bp->mb_mid;
				if (xs->xs_sbufslot == slot)
					xs->xs_sbufslot = XS_SBUF_NONE;
				if (xs->xs_mbuf[slot]) {
					m_freem(xs->xs_mbuf[slot]);
					xs->xs_mbuf[slot] = 0;
				} else {
					printf("ex%d: weird xmit interrupt\n",
						      unit);
					break;
				}
				xs->xs_xmits_inprogress--;
				if (xs->xs_xmits_inprogress < XMITS)
					exstart(unit);
				break;
			case LLNET_STSTCS:
				xs->xs_ac.ac_if.if_ierrors = xs->xs_xsa.sa_crc;
				xs->xs_flags &= ~EX_STATPENDING;
				break;
			case LLNET_MODE:
			case LLNET_ADDRS:
				/*
				 * These interrupts can occur when a system
				 * has two interfaces, and the first
				 * generates an interrupt before the second
				 * has been initialized.  Just ignore the
				 * interrupt.
				 */
				break;

#ifdef	OS_DEBUG
			default:
				iprintf("ex%d: unknown reply %d\n",
					       unit, bp->mb_rqst);
				panic("exintr");
#endif
			}

			bp->mb_length = MBDATALEN;
			bp->mb_status |= MH_EXOS;	/* free up buffer */
			addr->xd_portb = EX_NTRUPT;	/* tell EXOS */
			bp = xs->xs_x2hnext = xs->xs_x2hnext->mb_next;
			handled = 1;
		}
		/*
		 * Clear interrupt, even if we didn't get a packet from
		 * it.  Doesn't hurt...
		 */
		addr->xd_porta = EX_CLRINT;
	}
	return (handled);
}

/*
 * Get a request buffer, fill in standard values, advance pointer.
 */
struct ex_msg *
exgetcbuf(xs)
	struct ex_softc *xs;
{
	register struct ex_msg *bp = xs->xs_h2xnext;

#ifdef OS_DEBUG
	if ((bp->mb_status & MH_OWNER) == MH_EXOS)
		panic("exgetcbuf(): EXOS owns message buffer");
#endif
	bp->mb_1rsrv = 0;
	bp->mb_length = MBDATALEN;
	xs->xs_h2xnext = xs->xs_h2xnext->mb_next;
	return bp;
}

/*
 * Process Ethernet receive completion:
 *	If input error just drop packet.
 *	Otherwise purge input buffered data path and examine 
 *	packet to determine type.  If can't determine length
 *	from type, then have to drop packet.  Otherwise decapsulate
 *	packet based on type and pass to type-specific higher-level
 *	input routine.
 */
exrecv(unit, bp)
	int unit;
	struct ex_msg *bp;
{
	register struct ex_softc *xs = &ex_softc[unit];
    	register struct mbuf *m0;
	register int len, off, resid;
	register u_short ether_type;
	register int kmx;
	register struct mbuf *m1;
	struct ether_header *eh;
	struct mbuf *m2, *mc;
	int slot;

	xs->xs_ac.ac_if.if_ipackets++;
	len = bp->mb_er.er_blks[0].bb_len - sizeof(struct ether_header) - 4;
	slot = bp->mb_mid;
	ASSERT(slot < RCVS);
	if (bp->mb_rply != LL_OK) {
		xs->xs_ac.ac_if.if_ierrors++;
		ex_rcv_err(unit, bp->mb_rply);
		return;
	}
	eh = (struct ether_header *)(xs->xs_rcvbufs + (slot << PGSHIFT));
	ether_type = ntohs((u_short)eh->ether_type);

	/*
	 * Deal with trailer protocol: if type is trailer
	 * get true type from first 16-bit word past data.
	 * Remember that type was trailer by setting off.
	 */
#define	exdataaddr(eh, off, type)	((type)(((caddr_t)((eh)+1)+(off))))
	if (ether_type >= ETHERTYPE_TRAIL &&
	    ether_type < ETHERTYPE_TRAIL+ETHERTYPE_NTRAILER) {
		off = (ether_type - ETHERTYPE_TRAIL) * 512;
		if (off >= ETHERMTU)
			return;		/* sanity */
		/*
		 * Fix up protocol type in ether header.
		 */
		ether_type = ntohs(*exdataaddr(eh, off, u_short *));
		resid = ntohs(*(exdataaddr(eh, off+sizeof(u_short), u_short *)));
		if (off + resid > len)
			return;		/* sanity */
		/*
		 * Remove 2 u_short's added into resid during output.
		 * Result is the length of the header.  We adjust len
		 * to be the total of the data length and the header
		 * length, not counting the two u_short's.
		 */
		resid -= 2 * sizeof(u_short);
		len = off + resid;	/* total len, minus two u_shorts */
	} else
		off = 0;
	if (len == 0)
		return;

	/*
	 * Get first mbuf ahead of time, since we need it for both
	 * decapsulation methods.
	 */
	m0 = m_get(M_DONTWAIT, MT_DATA);
	if (m0 == 0)
		return;

	/*
	 * See if receive packet is small.  If it will directly fit into
	 * a single mbuf, counting the ifp at the head, then just get an
	 * mbuf and copy into it.  Otherwise, do page flipping below.
	 */
	if (sizeof(&xs->xs_ac.ac_if) + len <= MLEN) {
		register char *cp;

		cp = mtod(m0, caddr_t);
		*(struct ifnet **)cp = &xs->xs_ac.ac_if;
		cp += sizeof(&xs->xs_ac.ac_if);
		if (off == 0)
			bcopy((caddr_t) (eh + 1), cp, len);
		else {
			/*
			 * Copy header first, then data.
			 */
			bcopy(exdataaddr(eh, off+2*sizeof(u_short), caddr_t),
					cp, resid);
			bcopy((caddr_t) (eh + 1), cp + resid, off);
		}
		m0->m_len = sizeof(&xs->xs_ac.ac_if) + len;
		xs->xs_rcv_copies++;
		goto decapsulated;
	}
	xs->xs_rcv_maps++;

	/*
	 * Allocate some mbufs.
	 * For normal (non-trailer) packets allocate two:
	 *	- one to hold the interface pointer,
	 *	  one to hold the header and the data.
	 *	- we allocate a cluster for the data, and page flip the
	 *	  receive buffer into it.
	 * For trailer packets, allocate three:
	 *	- one to hold the interface pointer,
	 *	- one to hold the header,
	 *	- and one to hold the data
	 *	- We allocate a cluster for the data and header, and page
	 *	  flip the receive buffer into it.  We need two
	 *	  mbufs to point at the same buffer, because one has
	 *	  to point at the header which follows the data, and the
	 *	  other has to point at the data at the beginning of
	 *	  the packet.
	 */
	m1 = m_get(M_DONTWAIT, MT_DATA);	/* header/header&data mbuf */
	if (m1 == 0)
		goto free_m0;
	mc = m_clget(M_DONTWAIT);		/* get cluster */
	if (mc == 0)
		goto free_m1_m0;
	kmx = mtocl(mc);
	if (off) {
		m2 = m_get(M_DONTWAIT, MT_DATA);
		if (m2 == 0) {
			(void) m_pgfree(mc);
free_m1_m0:
			(void) m_free(m1);
free_m0:
			(void) m_free(m0);
			return;
		}
	}
	/* copy interface pointer into m0 */
	m0->m_len = sizeof(&xs->xs_ac.ac_if);
	*(mtod(m0, struct ifnet **)) = &xs->xs_ac.ac_if;
	m0->m_next = m1;

/*
 * Macro to get an address into mc where the data&header will end up
 * after the page flip below.
 */
#define	mcdataaddr(off, type) \
	((type) ((char *)(mc) + sizeof(struct ether_header) + (off)))

	m1->m_freefunc = m_clfree;
	m1->m_dupfunc = m_cldup;
	if (off == 0) {
		/*
		 * Regular (non-trailer) packet.  Point m1 at data held
		 * in mc.
		 */
		m1->m_off = mcdataaddr(0, char *) - (char *)m1;
		m1->m_len = len;
	} else {
		/*
		 * Trailer packet.  Setup pointer to header.  Advance
		 * the clusters reference count, now that there are
		 * two pointers into it.
		 */
		m1->m_off = mcdataaddr(off + 2*sizeof(u_short), char *) -
				(char *)m1;
		m1->m_len = resid;
		m1->m_next = m2;

		/* setup pointer to data */
		m2->m_off = mcdataaddr(0, char *) - (char *)m2;
		m2->m_len = off;
		m2->m_freefunc = m_clfree;
		m2->m_dupfunc = m_cldup;
		mclrefcnt[kmx]++;
	}

	/*
	 * Page flip data between mc and ether buffer.  Prior to this,
	 * we saved any data we needed to look at in the ethernet buffer
	 * because we won't find it after the page flip.
	 */
	{
		struct pte *pte_rcv, *pte_mc;
		short pf;

		/*
		 * Stash pointers to both page tables.
		 */
		pte_rcv = xs->xs_rcv_pt + slot;
		pte_mc = &Usrptmap[kmx];
		ASSERT(pte_rcv->pg_pfnum && pte_mc->pg_pfnum);
		/*
		 * Exchange physical page between mc and rcv buf
		 */
		pf = pte_mc->pg_pfnum;
		pte_mc->pg_pfnum = pte_rcv->pg_pfnum;
		pte_rcv->pg_pfnum = pf;
		/*
		 * Remap kernel to observe the physical memory
		 * exchange.
		 */
		ptaccess(pte_rcv, (caddr_t)eh, 1);
		ptaccess(pte_mc, (caddr_t)mc, 1);
		/*
		 * Lastly, point rcv dma map at new page frame
		 */
		(void) mbset(pf, xs->xs_rcv_mbuspage + slot, 0);
	}

decapsulated:
	{
		register struct ifqueue *inq;

		switch (ether_type) {

#ifdef INET
		case ETHERTYPE_IP:
#ifdef	NOISE
			iprintf("<%d:IP", len);
#endif
			schednetisr(NETISR_IP);
			inq = &ipintrq;
			break;

		case ETHERTYPE_ARP:
#ifdef	NOISE
			iprintf("<%d:ARP", len);
#endif
			arpinput(&xs->xs_ac, m0);
			return;
#endif
#ifdef NS
		case ETHERTYPE_NS:
#ifdef	NOISE
			iprintf("<%d:NS", len_type);
#endif
			schednetisr(NETISR_NS);
			inq = &nsintrq;
			break;

#endif
		default:
			m_freem(m0);
			return;
		}
		if (IF_QFULL(inq)) {
			IF_DROP(inq);
			m_freem(m0);
		} else {
			IF_ENQUEUE(inq, m0);
		}
	}
}

/*
 * Send receive request to EXOS.
 * This routine is called by exinit and exintr,
 * with interrupts disabled in both cases.
 */
exhangrcv(unit, slot)
	int unit;
	int slot;
{
	register struct ex_softc *xs = &ex_softc[unit];
	register struct ex_msg *bp = exgetcbuf(xs);
	struct exdevice *addr = xs->xs_csraddr;;

	bp->mb_rqst = LLRECEIVE;
	bp->mb_er.er_nblock = 1;
	bp->mb_er.er_blks[0].bb_len = EXMAXRBUF;
	bp->mb_mid = slot;		/* record for later */
	*(u_long *)bp->mb_er.er_blks[0].bb_addr =
		(u_long) ptob(xs->xs_rcv_mbuspage + slot);
	bp->mb_status |= MH_EXOS;
	addr->xd_portb = EX_NTRUPT;
}

/*
 * Ethernet output routine.
 * Encapsulate a packet of type family for the local net.
 * Use trailer local net encapsulation if enough data in first
 * packet leaves a multiple of 512 bytes of data in remainder.
 */
exoutput(ifp, m0, dst)
	register struct ifnet *ifp;
	register struct mbuf *m0;
	struct sockaddr *dst;
{
	int type, s, error;
	u_char edst[6];
	struct in_addr idst;
	register struct ex_softc *xs = &ex_softc[ifp->if_unit];
	register struct mbuf *m = m0;
	register struct ether_header *eh;
	register int off;
	int usetrailers;

	if ((ifp->if_flags & (IFF_UP|IFF_RUNNING)) != (IFF_UP|IFF_RUNNING)) {
		error = ENETDOWN;
		goto bad;
	}
	switch (dst->sa_family) {

#ifdef INET
	case AF_INET:
		idst = ((struct sockaddr_in *)dst)->sin_addr;
		if (!arpresolve(&xs->xs_ac, m, &idst, edst, &usetrailers))
			return (0);	/* if not yet resolved */
		off = ntohs((u_short)mtod(m, struct ip *)->ip_len) - m->m_len;
		/* need per host negotiation */
		if (usetrailers && off > 0 && (off & 0x1ff) == 0 &&
		    m->m_off >= MMINOFF + 2 * sizeof (u_short)) {
			type = ETHERTYPE_TRAIL + (off>>9);
			m->m_off -= 2 * sizeof (u_short);
			m->m_len += 2 * sizeof (u_short);
			*mtod(m, u_short *) = htons((u_short)ETHERTYPE_IP);
			*(mtod(m, u_short *) + 1) = htons((u_short)m->m_len);
			goto gottrailertype;
		}
		type = ETHERTYPE_IP;
		off = 0;
		goto gottype;
#endif
#ifdef NS
	case AF_NS:
		type = ETHERTYPE_NS;
 		bcopy((caddr_t)&(((struct sockaddr_ns *)dst)->sns_addr.x_host),
		(caddr_t)edst, sizeof (edst));
		off = 0;
		goto gottype;
#endif

	case AF_UNSPEC:
		eh = (struct ether_header *)dst->sa_data;
		bcopy((caddr_t)eh->ether_dhost, (caddr_t)edst, sizeof (edst));
		type = eh->ether_type;
		goto gottype;

	default:
/*
		printf("ex%d: can't handle af%d\n", ifp->if_unit,
			dst->sa_family);
*/
		error = EAFNOSUPPORT;
		goto bad;
	}

gottrailertype:
	/*
	 * Packet to be sent as trailer: move first packet
	 * (control information) to end of chain.
	 */
	while (m->m_next)
		m = m->m_next;
	m->m_next = m0;
	m = m0->m_next;
	m0->m_next = 0;
	m0 = m;

gottype:
	/*
	 * Add local net header.  If no space in first mbuf,
	 * allocate another.
	 */
	if (m->m_off > MMAXOFF ||
	    MMINOFF + sizeof (struct ether_header) > m->m_off) {
		m = m_get(M_DONTWAIT, MT_HEADER);
		if (m == 0) {
			error = ENOBUFS;
			goto bad;
		}
		m->m_next = m0;
		m->m_off = MMINOFF;
		m->m_len = sizeof (struct ether_header);
	} else {
		m->m_off -= sizeof (struct ether_header);
		m->m_len += sizeof (struct ether_header);
	}
	eh = mtod(m, struct ether_header *);
	eh->ether_type = htons((u_short)type);
	bcopy((caddr_t)edst, (caddr_t)eh->ether_dhost, sizeof (edst));
	bcopy((caddr_t)xs->xs_ac.ac_enaddr, (caddr_t)eh->ether_shost, 6);

	/*
	 * Queue message on interface, and start output if interface
	 * not yet active.
	 */
	s = splimp();
	if (IF_QFULL(&ifp->if_snd)) {
		IF_DROP(&ifp->if_snd);
		splx(s);
		m_freem(m);
		return (ENOBUFS);
	}
	IF_ENQUEUE(&ifp->if_snd, m);

	/* if transmit request not already pending, then kick the back end */
	if (xs->xs_xmits_inprogress < XMITS)
		exstart(ifp->if_unit);
	splx(s);
	return (0);

bad:
	m_freem(m0);
	return (error);
}

/*
 * Watchdog routine - place stats request to EXOS
 * (This could be dispensed with, if you don't care
 *  about the if_ierrors count, or are willing to receive
 *  bad packets in order to derive it.)
 */
exwatch(unit)
	int unit;
{
	register struct ex_softc *xs = &ex_softc[unit];
	struct exdevice *addr = xs->xs_csraddr;
	register struct ex_msg *bp;
	int s = splimp();

	if (xs->xs_flags & EX_STATPENDING) {
		/* board died.  reset it */
#ifdef	OS_DEBUG
		printf("ex%d: board is hung, resetting\n", unit);
#endif
		xs->xs_ac.ac_if.if_flags &= ~IFF_RUNNING;
		exinit(unit);
		xs->xs_resets++;
		goto exspnd;
	}
	bp = exgetcbuf(xs);
	xs->xs_flags |= EX_STATPENDING;
	bp->mb_rqst = LLNET_STSTCS;
	bp->mb_ns.ns_mask = READ_OBJ;
	bp->mb_ns.ns_rsrv = 0;
	bp->mb_ns.ns_nobj = 8;		/* read all 8 stats objects */
	bp->mb_ns.ns_xobj = 0;		/* starting with the 1st one */
	bp->mb_ns.ns_bufp = P_MBADDR(xs) + SA_OFFSET(xs);
	bp->mb_status |= MH_EXOS;
	addr->xd_portb = EX_NTRUPT;
exspnd:
	splx(s);
	xs->xs_ac.ac_if.if_timer = EXWATCHINTVL;
}

/*
 * Process an ioctl request.
 */
exioctl(ifp, cmd, data)
	register struct ifnet *ifp;
	int cmd;
	caddr_t data;
{
	register struct ifaddr *ifa = (struct ifaddr *)data;
	int s;
	int error = 0;

	s = splimp();
	switch (cmd) {

	case SIOCSIFADDR:
                ifp->if_flags |= IFF_UP;
                exinit(ifp->if_unit);

                switch (ifa->ifa_addr.sa_family) {
#ifdef INET
		case AF_INET:
			((struct arpcom *)ifp)->ac_ipaddr =
				IA_SIN(ifa)->sin_addr;
			arpwhohas((struct arpcom *)ifp, &IA_SIN(ifa)->sin_addr);
			break;
#endif
#ifdef NS
		case AF_NS:
			IA_SNS(ifa)->sns_addr.x_host =
				* (union ns_host *) 
				     (ex_softc[ifp->if_unit].xs_ac.ac_enaddr);
			break;
#endif
		}
		break;

	default:
		error = EINVAL;
	}
	splx(s);
	return (error);
}

/*
 * Print out a transmit error, nicely
 */
ex_xmit_err(unit, rply)
	int unit;
	register int rply;
{
	printf("ex%d: transmit error: ", unit);
	if (rply & LLXM_NSQE)
		printf("<no sqe test signal>");
	if (rply & LLXM_CLSN)
		printf("<excessive collisions>");
	if (rply & LLXM_NCS)
		printf("<no carrier sense>");
	if (rply & LLXM_LNGTH)
		printf("<bad packet length>");
	printf("\n");
}

/*
 * Print out a receive error, nicely
 */
ex_rcv_err(unit, rply)
	int unit;
	register int rply;
{
	printf("ex%d: receive error: ", unit);
	if (rply & LLRC_TRUNC)
		printf("<packet larger than buffer>");
	if (rply & LLRC_ALIGN)
		printf("<alignment problem>");
	if (rply & LLRC_CRC)
		printf("<crc mismatch>");
	if (rply & LLRC_BUFLEN)
		printf("<packet buffer smaller than ethernet minimum>");
	printf("\n");
}
