/*
 *	NSC HYPERchannel with IKON Multibus Interface
 *
 *	Raw interface support for 64k transfers
 *
 * $Source: /d2/3.7/src/sys/multibusif/RCS/if_hy.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:32:05 $
 */

#include "hy.h"
#if NHY > 0

#include "machine/pte.h"
#include "machine/cpureg.h"

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/cmap.h"
#include "../h/buf.h"
#include "../h/errno.h"
#include "../h/ioctl.h"
#include "../h/mbuf.h"
#include "../h/protosw.h"
#include "../h/socket.h"
#include "../vm/vmmac.h"
#include "../h/user.h"
#include "../net/if.h"
#include "../net/netisr.h"
#include "../net/route.h"
#include "../net/soioctl.h"
#include "../netinet/in.h"
#include "../netinet/in_systm.h"
#include "../netinet/in_var.h"
#include "../netinet/ip.h"
#include "../netinet/ip_var.h"
#include "../multibus/mbvar.h"
#include "../multibusif/if_hyreg.h"
#include "../multibusif/if_hy.h"
#include "../multibusif/hyroute.h"

int	hyprobe(), hyintr();
int	hyinit(), hyoutput(), hyioctl(), hywatch();

struct mb_driver hydriver = {
	hyprobe,
	(int (*)()) 0,
	(int (*)()) 0,
	(int (*)()) 0,
	hyintr,
	(char *(*)()) 0,
	"hy",
	0,
};

struct hydebug {
	int		dbg_state;
	int		dbg_cmd;
	int		dbg_addr;
	int		dbg_cnt;
	int		dbg_csr;
};

#define	HYDEBUGMAX	32

/*
 *	Multibus memory window usage
 *
 *	17 page for transmit
 *
 *	1 page for receiving msgs and status
 *	16 page for receiving data
 */

#define	RECV_PAGES	17		/* allow for 64K transfers */
#define	SEND_PAGES	17		/* allow for 64K transfers */
#define	ERR_SIZE	(sizeof(struct hy_status) * HY_CHANNELS)

#define MAP_RECV_MSG(is,m) {					\
	register int kva_msg, kmx_msg;				\
	(m)->m_len = HY_MPROP_SIZE;				\
	kva_msg = mtod((m), long);				\
	kmx_msg = btokmx((struct pte *) kva_msg);		\
	ASSERT(0 <= kmx_msg && kmx_msg <= USRPTSIZE);		\
	(is)->hy_recv_mba = mbset(Usrptmap[kmx_msg].pg_pfnum,	\
			(is)->hy_recv_mbuspage, kva_msg);	\
}

#define MAP_SEND_MSG(is,m) {					\
	register int kva_msg, kmx_msg;				\
	kva_msg = mtod((m), int) + HY_HWOFFSET;			\
	kmx_msg = btokmx((struct pte *) kva_msg);		\
	ASSERT(0 <= kmx_msg && kmx_msg <= USRPTSIZE);		\
	(is)->hy_send_mba = mbset(Usrptmap[kmx_msg].pg_pfnum,	\
			(is)->hy_send_mbuspage, kva_msg);	\
}

#define MAP_SEND_DATA(is,m) {					\
	register int kva_msg, kmx_msg;				\
	kva_msg = mtod((m), long);				\
	kmx_msg = btokmx((struct pte *) kva_msg);		\
	ASSERT(0 <= kmx_msg && kmx_msg <= USRPTSIZE);		\
	(is)->hy_send_mba = mbset(Usrptmap[kmx_msg].pg_pfnum,	\
			(is)->hy_send_mbuspage, kva_msg);	\
}

#define MAP_SEND_REST(is,m,i) {					\
	register int kva_msg, kmx_msg;				\
	kva_msg = mtod((m), long);				\
	kmx_msg = btokmx((struct pte *) kva_msg);		\
	ASSERT(0 <= kmx_msg && kmx_msg <= USRPTSIZE);		\
	ASSERT((kva_msg & PGOFSET) == 0);			\
	mbset(Usrptmap[kmx_msg].pg_pfnum,			\
			(is)->hy_send_mbuspage+(i), 0);		\
}

#define MAP_ERR_STAT(is,chan) {					\
	register struct hy_status *kva;				\
	kva = is->hy_status = &is->hy_err_status[chan];		\
	(is)->hy_err_mba = (is)->hy_err_vbase +			\
		(((int) (kva))-((int) ((is)->hy_err_status)));	\
}

#define	RECV_DONE(is, chan) {					\
	if((chan) > 0 && (is)->hy_wait_read[(chan)]) {		\
		(is)->hy_wait_read[(chan)] = 0;			\
		wakeup(&(is)->hy_wait_read[(chan)]);		\
	}							\
}

#define	SEND_DONE(is, chan, err) {				\
	if((chan) > 0) {					\
		(is)->hy_write_err[(chan)] = (err);		\
		wakeup(&(is)->hy_wait_write[(chan)]);		\
	}							\
}

#define PRINT_MBUF(m, str) {					\
	struct mbuf *mx;					\
	int spl = splhy();					\
	mx = (m);						\
	iprintf(str);						\
	while(mx) {						\
		iprintf(" [%x,%d,%d]",mx,mx->m_off,mx->m_len);	\
		mx = mx->m_next;				\
	}							\
	iprintf("\n");						\
	splx(spl);						\
}

/*
 *	per device state information
 */
struct hy_softc {
	struct hydevice		*hy_addr;

	short			hy_state;
	short			hy_active;
	short			hy_alive;

	short			hy_host;
	short			hy_adaptor;
	short			hy_port;

	short			hy_stat_channel;
	short			hy_send_channel;
	short			hy_recv_len;
	short			hy_send_len;

	struct mbuf		*hy_recv_msg;	/* current receive mbuf */
	struct mbuf		*hy_send_msg;	/* send message proper */
	struct mbuf		*hy_send_next;	/* send data pointer */

	struct hy_status	*hy_status;

	short			hy_send_mbuspage;
	short			hy_recv_mbuspage;
	u_long			hy_recv_mba;
	u_long			hy_send_mba;
	u_long			hy_err_mba;

	/*
	 *	multibus mapping base address of the status array
	 */
	u_long			hy_err_vbase;

	/*
	 *	page table entries where we store the receive pages
	 */
	struct pte		*hy_recv_pt;
	int			hy_recv_kmx;

	/*
	 *	interface information
	 */
	struct ifnet		hy_if;

	/*
	 *	routing table
	 */
	struct hy_route		hy_route;

	/*
	 *	Information used by raw interface
	 */
	struct ifqueue		hy_rawq[HY_CHANNELS];
	char			hy_wait_read[HY_CHANNELS];
	char			hy_wait_write[HY_CHANNELS];
	char			hy_write_err[HY_CHANNELS];
	char			hy_open[HY_CHANNELS];
	struct hy_status	hy_err_status[HY_CHANNELS];

	struct hydebug		hy_debug[HYDEBUGMAX];
	int			hy_debug_ptr;
} hy_softc[NHY];

char hy_ndev = 0;

#define	splhy	spl2

/*
 *	HYPROBE
 *
 *	Called by the auto configure code to test for the existence
 *	of a given interface.  This must be called in order for
 *	hy0, hy1, hy2, ...
 */

hyprobe(reg)
caddr_t reg;
{
	register struct hydevice *dr = (struct hydevice *) (MBIO_VBASE + reg);
	register int unit;

	unit = hy_ndev++;
	hy_softc[unit].hy_addr = dr;

	dr->d_csr = CS_RESET;
	if(dr->d_csr & CS_READY) {
		return(hyattach(unit));
	}
	return(CONF_DEAD);
}

/*
 *	HYATTACH
 *
 *	This is only called by hyprobe if the interface registers
 *	are detected.
 *
 *	All resources used by the driver except for mbufs are
 *	preallocated in this routine.
 */

hyattach(unit)
int unit;
{
	register struct hy_softc *is = &hy_softc[unit];
	register struct hydevice *dr = is->hy_addr;
	register struct ifnet *ifp = &is->hy_if;
	static struct hy_status sp;
	static struct hy_statistics stp;
	int kmx;
	int i;

	is->hy_state = ST_STARTUP;

	/*
	 *	Initialize ifnet structure
	 */

	ifp->if_unit = unit;
	ifp->if_name = "hy";
	ifp->if_mtu = HY_MTU;

	ifp->if_init = hyinit;
	ifp->if_ioctl = hyioctl;
	ifp->if_output = hyoutput;
	ifp->if_reset = 0;
	ifp->if_watchdog = hywatch;
	ifp->if_timer = 0;

	/*
	 *	Initialize raw interface data structures
	 */

	for(i=1; i<HY_CHANNELS; i++) {
		is->hy_rawq[i].ifq_maxlen = HY_QSIZE;
	}

	/*
	 *	Allocate Multibus Mapping Registers
	 */
	if((is->hy_send_mbuspage = mballoc(SEND_PAGES, 0)) < 0) {
		printf("[cannot allocate send map] ", unit);
		goto badconf0;
	}

	if((is->hy_recv_mbuspage = mballoc(RECV_PAGES, 0)) < 0) {
		printf("[cannot allocate recv map] ", unit);
		goto badconf1;
	}

	if((is->hy_err_vbase = mbmapkget(is->hy_err_status, ERR_SIZE)) < 0) {
		printf("[cannot allocate err map] ", unit);
		goto badconf1a;
	}

	/*
	 *	Allocate kernel map resources for receive page flipping
	 */
	if((kmx = kmap_alloc(RECV_PAGES-1, 0)) < 0) {
		printf("[cannot allocate recv page table space] ", unit);
		goto badconf2;
	}

	if(memall(&Usrptmap[kmx], RECV_PAGES-1, (struct proc *)0, CSYS) == 0) {
		printf("[cannot allocate recv pages] ", unit);
		goto badconf3;
	}

	is->hy_recv_pt = &Usrptmap[kmx];
	is->hy_recv_kmx = kmx;
	vmaccess(is->hy_recv_pt, kmxtob(kmx), RECV_PAGES-1);

	/*
	 *	Set up the receive pages for associated data
	 */
	for(i=0; i<RECV_PAGES-1; i++) {
		mbset((is->hy_recv_pt+i)->pg_pfnum,
			is->hy_recv_mbuspage+i+1, 0);
	}

	/*
	 *	Master clear the adaptor
	 */
	dr->d_csr = CS_RESET;

	/*
	 *	Query the adaptor for our port number
	 */
	if(hycmd(CMD_STATUS, &sp, sizeof(struct hy_status), dr)) {
		printf("[cannot get adaptor status] ", unit);
		goto badconf4;
	}

	is->hy_port = PORT_NUMBER(&sp);

	/*
	 *	Query the adaptor for its unit number
	 */
	if(hycmd(CMD_GET_STAT, &stp, sizeof(struct hy_statistics), dr)) {
		printf("[cannot get adaptor statistics] ", unit);
		goto badconf4;
	}
	
	is->hy_adaptor = stp.st_adaptor;

	printf("ADAPTOR %d, PORT %d ", is->hy_adaptor, is->hy_port);

	is->hy_host = (is->hy_adaptor << 8) | is->hy_port;

	/*
	 *	Make our presence known to the higher levels of the TCP/IP code
	 */
	if_attach(ifp);
	ifp->if_snd.ifq_maxlen = HY_QSIZE;

	is->hy_alive = 1;

	return(CONF_ALIVE);

	/*
	 *	Free any resources allocated
	 */
badconf4:
	/*
	m_free(is->hy_recv_msg);
	*/
badconf3:
	kmfree(RECV_PAGES-1, kmx);
badconf2:
	mbmapkput(ptob(is->hy_recv_mbuspage), RECV_PAGES<<PGSHIFT);
badconf1a:
	mbmapkput(is->hy_err_vbase, ERR_SIZE);
badconf1:
	mbmapkput(ptob(is->hy_send_mbuspage), SEND_PAGES<<PGSHIFT);
badconf0:
	return(CONF_DEAD);
}

/*
 *	HYCMD
 *
 *	This is only called by the hyattach routine.
 *
 *	It does a simple busy wait I/O operation to the adaptor.
 *	It is only used to retrieve adaptor status/statistics
 *	so the actual I/O time is extremely short, hence the
 *	loop testing for completion has a short time out.
 */

hycmd(cmd, addr, cnt, dr)
int cmd;
int addr;
int cnt;
register struct hydevice *dr;
{
	register i;
	register int mbva;

	mbva = mbmapkget(addr, cnt);

	CLEAR_INTERRUPTS(dr);

	dr->d_dma_cntl = 0;
	dr->d_lo_cnt = (cnt>>1) - 1;
	dr->d_hi_cnt = 0;
	dr->d_lo_addr = (mbva >> 1) & 0xffff;
	dr->d_hi_addr = (mbva >> 17) & 0x007f;
	dr->d_cmd = cmd;
	dr->d_csr = CS_GO;

	for(i=0; i<1000; i++) {
		if(dr->d_csr & CS_READY) break;
	}

	mbmapkput(mbva, cnt);

	if(dr->d_csr & CS_READY)
		return(0);
	else
		return(1);
}

/*
 *	HYSTART
 *
 *	This routinue is used to issue commands to the adaptor
 */

hystart(unit, cmd, count, mbaddr)
int unit;
int cmd;
int count;
int mbaddr;
{
	register struct hy_softc *is = &hy_softc[unit];
	register struct hydevice *dr = is->hy_addr;

	CLEAR_INTERRUPTS(dr);

	ASSERT((mbaddr&0x01) == 0x00);
	ASSERT(!is->hy_active || cmd == CMD_CLR_WAIT);

	is->hy_debug[is->hy_debug_ptr].dbg_state = is->hy_state;
	is->hy_debug[is->hy_debug_ptr].dbg_cmd = cmd;
	is->hy_debug[is->hy_debug_ptr].dbg_addr = mbaddr;
	is->hy_debug[is->hy_debug_ptr].dbg_cnt = count;

	is->hy_if.if_timer = 0;

	/*
	 *	Set up a DMA transfer
	 *
	 *	See page 7 of the IKON Hardware/Software Manual
	 *	for an explanation.
	 */

	dr->d_dma_cntl = 0;

	if(count) {
		dr->d_lo_cnt = ((count+1)>>1) - 1;
		dr->d_hi_cnt = 0;
	}

	if(mbaddr) {
		dr->d_lo_addr = (mbaddr>>1) & 0xffff;
		dr->d_hi_addr = (mbaddr>>17) & 0x007f;
	}

	dr->d_cmd = cmd;

	is->hy_active = 1;
	is->hy_if.if_timer = HY_SCANINTERVAL;

	dr->d_csr = CS_IE | CS_IATTN | CS_GO;
}

/*
 *	HYINTR
 *
 *	called when a multibus interrupt occurs
 *	we must determine if the interrupt is ours
 */
hyintr(dev)
dev_t dev;
{
	register int unit;
	register int csr;
	int mine;

	mine = 0;

	for(unit=0; unit<hy_ndev; unit++) {
		register struct hy_softc *is = &hy_softc[unit];
		register struct hydevice *dr = is->hy_addr;

		if(!is->hy_alive)
			continue;

		csr = dr->d_csr;

		if((csr & CS_ATTF) == 0) {
			continue;
		}

		mine = 1;
		is->hy_active = 0;

		/*
		 *	Check too see if the adaptor is powered off
		 */
		if((csr & CS_NORMAL) && (csr & CS_ABNORMAL)) {
			CLEAR_INTERRUPTS(dr);
			dr->d_csr = CS_RESET;
			/*
			 *	the state machine will be handled by hywatch
			 */
			continue;
		}
		hyfsm(unit, csr);
	}

	return(mine);
}

/*
 *	HYFSM
 *
 *	Finite State Machine 		(well, close enough)
 *
 *	handle the host to adaptor protocol and the flow
 *	of packets to/from the network
 *
 *	This routine is responsible for clearing the interrupt
 *	condition or calling hystart before returning.
 */
hyfsm(unit, csr)
int unit;
int csr;
{
	register struct hy_softc *is = &hy_softc[unit];
	register struct hydevice *dr = is->hy_addr;
	register struct mbuf *m;
	register struct hy_hdr *hdr;
	struct mbuf *mp, *md;
	struct ifqueue *inq;
	int inx;
	int s;
	int len;
	int cmd;
	int chan;
	int off;
	int i, k;

	is->hy_debug[is->hy_debug_ptr].dbg_csr = csr;
	is->hy_debug_ptr += 1;
	if(is->hy_debug_ptr >= HYDEBUGMAX)
		is->hy_debug_ptr = 0;

	for(;;) {
		switch(is->hy_state) {
		case ST_DEAD:			/* Unexpected */
		case ST_STARTUP:
			CLEAR_INTERRUPTS(dr);
			return;

		case ST_IDLE:
			if(dr->d_csr & CS_MSGPEND) {
		input_msg:
				is->hy_state = ST_RECV_MSG;
				hystart(unit, CMD_RECV_MSG,
					HY_MPROP_SIZE, is->hy_recv_mba);
				return;
			}

			IF_DEQUEUE(&is->hy_if.if_snd, m);
			if(m != NULL) {
				is->hy_send_msg = m;
				MAP_SEND_MSG(is, m);

				hdr = &(mtod(m, struct hy_swhdr *))->sw_hdr;
				is->hy_send_channel = CHAN_NO(hdr->h_src);
				if(UNIT_NO(hdr->h_src)==UNIT_NO(hdr->h_dest)) {
					cmd = CMD_SEND_LMSG;
					is->hy_state = ST_SEND_LMSG;
				}
				else {
					cmd = CMD_SEND_RMSG;
					is->hy_state = ST_SEND_RMSG;
				}
				if(m->m_next == NULL) {
					is->hy_state = ST_SEND_MSG;
				}

				hystart(unit, cmd, HY_MPROP_SIZE,
					is->hy_send_mba);
				return;
			}

			is->hy_state = ST_SET_WAIT;
			hystart(unit, CMD_SET_WAIT, 0, 0);
			return;

		case ST_SET_WAIT:
		case ST_CLR_WAIT:
		case ST_END:
			is->hy_state = ST_IDLE;
			continue;

		case ST_STAT_MSG:
		case ST_STAT_DATA:
			m = is->hy_send_msg;
			switch(is->hy_status->s_error) {
				struct hy_swhdr *swhdr;

			default:
				/*
				 *	This was an unexpected error,
				 *	we print out the error code
				 *	for debugging purposes
				 */
				iprintf("HY%d: send error %d\n", unit,
					is->hy_status->s_error);
				/* Fall through into ... */
				i = is->hy_debug_ptr;
				do {
					iprintf("  state = %d cmd = 0x%x csr = 0x%x\n",
						is->hy_debug[i].dbg_state,
						is->hy_debug[i].dbg_cmd,
						is->hy_debug[i].dbg_csr);
					for(k=0; k<10000; k++)
						;
					if(++i >= HYDEBUGMAX)
						i = 0;
				} while(i != is->hy_debug_ptr); 

			case NSC_TRANS_ABORT:
			case NSC_ADAPTOR_RES:
			case NSC_RETRY_COUNT:
				swhdr = mtod(m, struct hy_swhdr *);
				if(swhdr->sw_retry++ >= HY_MAXRETRY) {
					IF_DROP(&is->hy_if.if_snd);
					m_freem(m);
					SEND_DONE(is, is->hy_stat_channel, 1);
					break;
				}
				IF_ENQUEUE(&is->hy_if.if_snd, m);
				break;

			case NSC_PORT_DOWN:
				/*
				 *	Some twit is messing with our mind
				 */
				iprintf("hy%d: PORT MARKED DOWN\n");
				IF_ENQUEUE(&is->hy_if.if_snd, m);
				break;

			case NSC_MSG_PEND:
				/*
				 *	place the packet at the end of the
				 *	queue as a cheap back off algorithm
				 *
				 *	this might make the queue too long
				 *	but we really don't care as we must
				 *	take a packet off of the queue in
				 *	order to get here.
				 */
				IF_ENQUEUE(&is->hy_if.if_snd, m);
				break;
			}

			if(is->hy_state == ST_STAT_DATA) {
				/*
				 *	reset the adaptor state machine
				 */
				is->hy_state = ST_END;
				hystart(unit, CMD_END, 0, 0);
				return;
			}
			is->hy_state = ST_IDLE;
			continue;
			
		case ST_STAT_END:
			is->hy_state = ST_END;
			hystart(unit, CMD_END, 0, 0);
			return;

		case ST_RECV_MSG:
			if(csr & CS_ABNORMAL) {
				MAP_ERR_STAT(is, 0);

				is->hy_if.if_ierrors++;
				is->hy_state = ST_STAT_END;

				hystart(unit, CMD_STATUS,
					sizeof(struct hy_status),
					is->hy_err_mba);
				return;
			}

			mp = is->hy_recv_msg;
			is->hy_recv_len = HY_MPROP_SIZE-((dr->d_lo_cnt+1)<<1);

			hdr = mtod(mp, struct hy_hdr *);
			if(hdr->h_ctl & C_ASSOC_DATA) {
				is->hy_state = ST_RECV_DATA;
				hystart(unit, CMD_RECV_DATA, HY_ASSOC_SIZE,
					ptob(is->hy_recv_mbuspage+1));
				return;
			}

			md = NULL;

			if((m = m_get(M_DONTWAIT, MT_HEADER)) == NULL) {
				goto recv_drop;
			}

			mp->m_next = md;

			/*
			 *	first remap a new mbuf for receive
			 */

			is->hy_recv_msg = m;
			MAP_RECV_MSG(is, m);

			is->hy_if.if_ipackets++;

			chan = CHAN_NO(hdr->h_dest);
			switch(chan) {
			case HY_IP_CHANNEL:
				if(hdr->h_offset > HY_OFFSET_MAX) {
					printf("hyfsm: recv off (%d)\n", off);
					m_freem(mp);
					is->hy_state = ST_IDLE;
					continue;
				}
				off = hdr->h_offset + sizeof(struct hy_hdr);
				mp->m_off += off;
				mp->m_len -= off;

				mp->m_off -= sizeof(struct ifnet *);
				mp->m_len += sizeof(struct ifnet *);
				*(mtod(mp, struct ifnet **)) = &is->hy_if;

				schednetisr(NETISR_IP);
				inq = &ipintrq;
				break;

			default:
				if(is->hy_open[chan] == 0) {
					m_freem(mp);
					is->hy_state = ST_IDLE;
					continue;
				}
				inq = &is->hy_rawq[chan];
				break;
			}

			if(IF_QFULL(inq)) {		/* drop older */
				IF_DROP(inq);
				IF_DEQUEUE(inq, m);
				m_freem(m);
			}
			IF_ENQUEUE(inq, mp);
			RECV_DONE(is, chan);
			is->hy_state = ST_IDLE;
			continue;

		recv_drop:
			if(md != NULL) {
				m_freem(md);
			}
			is->hy_state = ST_IDLE;
			continue;

		case ST_RECV_DATA:
			if(csr & CS_ABNORMAL) {
				MAP_ERR_STAT(is, 0);

				is->hy_if.if_ierrors++;
				is->hy_state = ST_STAT_END;

				hystart(unit, CMD_STATUS,
					sizeof(struct hy_status),
					is->hy_err_mba);
				return;
			}

			/*
			 *	repeat
			 *	   allocate a page and an mbuf
			 *	   flip the page into the receive window
			 *	   link the mbuf into the chain
			 *	until all pages of data have been flipped
			 *
			 *	if error free all
			 */

			len = HY_ASSOC_SIZE - ((dr->d_lo_cnt+1)<<1);
			inx = 0;
			md = (struct mbuf *) 0;

			while(len > 0) {
				struct mbuf *mc, *m1, *m2;
				struct pte *pte_rcv, *pte_mc;
				short pf;
				caddr_t kva;

				mc = m_clget(M_DONTWAIT);
				if(mc == NULL) {
					goto recv_err;
				}

				m2 = m_get(M_DONTWAIT, MT_DATA);
				if (m2 == 0) {
					(void) m_pgfree(mc);
					goto recv_err;
				}

				m2->m_freefunc = m_clfree;
				m2->m_off = (char *)mc - (char *)m2;
				m2->m_len = len > CLBYTES ? CLBYTES : len;

				if(md == NULL) {
					md = m2;
				}
				else {
					m1->m_next = m2;
				}
				m1 = m2;

				/*
				 * Stash pointers to both page tables.
				 */
				pte_rcv = is->hy_recv_pt + inx;
				pte_mc = &Usrptmap[mtocl(mc)];
				ASSERT(pte_rcv->pg_pfnum && pte_mc->pg_pfnum);

				s = spl7();

				/*
				 * Exchange phys page between mc and rcv buf
				 */
				pf = pte_mc->pg_pfnum;
				pte_mc->pg_pfnum = pte_rcv->pg_pfnum;
				pte_rcv->pg_pfnum = pf;

				/*
				 * Remap kernel to observe the physical memory
				 * exchange.
				 */
				kva = (caddr_t) kmxtob(is->hy_recv_kmx+inx);
				ptaccess(pte_rcv, kva, 1);
				ptaccess(pte_mc, (caddr_t)mc, 1);

				splx(s);

				/*
				 * Lastly, point rcv dma map at new page frame
				 */
				(void) mbset(pf,is->hy_recv_mbuspage+inx+1,0);

				len -= CLBYTES;
			}

			mp = is->hy_recv_msg;
			hdr = mtod(mp, struct hy_hdr *);

			if((m = m_get(M_DONTWAIT, MT_HEADER)) == NULL) {
				goto recv_drop;
			}

			mp->m_next = md;

			/*
			 *	first remap a new mbuf for receive
			 */

			is->hy_recv_msg = m;
			MAP_RECV_MSG(is, m);

			is->hy_if.if_ipackets++;

			chan = CHAN_NO(hdr->h_dest);
			switch(chan) {
			case HY_IP_CHANNEL:
				if(hdr->h_offset < 0
				|| hdr->h_offset > HY_OFFSET_MAX) {
					printf("hyfsm: recv off (%d)\n", off);
					m_freem(mp);
					is->hy_state = ST_IDLE;
					continue;
				}
				off = hdr->h_offset + sizeof(struct hy_hdr);
				mp->m_off += off;
				mp->m_len -= off;

				mp->m_off -= sizeof(struct ifnet *);
				mp->m_len += sizeof(struct ifnet *);
				*(mtod(mp, struct ifnet **)) = &is->hy_if;

				schednetisr(NETISR_IP);
				inq = &ipintrq;
				break;

			default:
				if(is->hy_open[chan] == 0) {
					m_freem(mp);
					is->hy_state = ST_IDLE;
					continue;
				}
				inq = &is->hy_rawq[chan];
				break;
			}

			if(IF_QFULL(inq)) {		/* drop older */
				IF_DROP(inq);
				IF_DEQUEUE(inq, m);
				m_freem(m);
			}
			IF_ENQUEUE(inq, mp);

			RECV_DONE(is, chan);

			is->hy_state = ST_IDLE;
			continue;

		recv_err:
			if(md != NULL) {
				m_freem(md);
			}
			is->hy_state = ST_IDLE;
			continue;

		case ST_SEND_LMSG:
#if 0
			/*
			 *	while len > CLBYTES send a DATA message
			 *	then send LAST message
			 */
			if(csr & CS_ABNORMAL) {
				goto send_msg_err;
			}

			is->hy_send_next = is->hy_send_msg->m_next;
			MAP_SEND_DATA(is, is->hy_send_next);
			if(is->hy_send_next->m_next == NULL) {
				is->hy_state = ST_SEND_LAST;
				cmd = CMD_SEND_LAST;
			}
			else {
				is->hy_state = ST_SEND_DATA;
				cmd = CMD_SEND_DATA;
			}
			len = is->hy_send_next->m_len;
			is->hy_send_next = is->hy_send_next->m_next;
			hystart(unit, cmd, len, is->hy_send_mba);
			return;
#endif
		case ST_SEND_RMSG:
			if(csr & CS_ABNORMAL) {
				goto send_msg_err;
			}

			/*
			 *	loop through the associated data mbufs
			 *	and map all of them.
			 */

			m = is->hy_send_msg->m_next;
			MAP_SEND_DATA(is, m);
			len = m->m_len;
			i = 1;
			while(m->m_next) {
				m = m->m_next;
				MAP_SEND_REST(is, m, i);
				len += m->m_len;
				i++;
			}

			is->hy_state = ST_SEND_LAST;
			hystart(unit, CMD_SEND_LAST, len, is->hy_send_mba);
			return;

		case ST_SEND_MSG:
			if(csr & CS_ABNORMAL) {
				struct hy_swhdr *swhdr;
		send_msg_err:
				if(dr->d_csr & CS_MSGPEND) {
					is->hy_if.if_collisions++;
					IF_PREPEND(&is->hy_if.if_snd,
						is->hy_send_msg);
					is->hy_send_msg = NULL;
					goto input_msg;
				}

				m = is->hy_send_msg;
				swhdr = mtod(m, struct hy_swhdr *);
				chan = CHAN_NO(swhdr->sw_hdr.h_src);

				MAP_ERR_STAT(is, chan);

				is->hy_if.if_oerrors++;
				is->hy_stat_channel = chan;
				is->hy_state = ST_STAT_MSG;
				hystart(unit, CMD_STATUS,
					sizeof(struct hy_status),
					is->hy_err_mba);
				return;
			}
			SEND_DONE(is, is->hy_send_channel, 0);
			is->hy_if.if_opackets++;
			m_freem(is->hy_send_msg);
			is->hy_state = ST_IDLE;
			continue;
#if 0
		case ST_SEND_DATA:
			if(csr & CS_ABNORMAL) {
				goto send_data_err;
			}

			MAP_SEND_DATA(is, is->hy_send_next);
			if(is->hy_send_next->m_next == NULL) {
				is->hy_state = ST_SEND_LAST;
				cmd = CMD_SEND_LAST;
			}
			else {
				is->hy_state = ST_SEND_DATA;
				cmd = CMD_SEND_DATA;
			}
			len = is->hy_send_next->m_len;
			is->hy_send_next = is->hy_send_next->m_next;
			hystart(unit, cmd, len, is->hy_send_mba);
			return;
#endif
		case ST_SEND_LAST:
			if(csr & CS_ABNORMAL) {
				struct hy_swhdr *swhdr;
		send_data_err:
				m = is->hy_send_msg;
				swhdr = mtod(m, struct hy_swhdr *);
				chan = CHAN_NO(swhdr->sw_hdr.h_src);

				MAP_ERR_STAT(is, chan);

				is->hy_if.if_oerrors++;
				is->hy_stat_channel = chan;
				is->hy_state = ST_STAT_DATA;
				hystart(unit, CMD_STATUS,
					sizeof(struct hy_status),
					is->hy_err_mba);
				return;
			}

			SEND_DONE(is, is->hy_send_channel, 0);
			is->hy_if.if_opackets++;
			m_freem(is->hy_send_msg);
			is->hy_state = ST_IDLE;
			continue;

		default:
			printf("hyfsm: bad state %d\n", is->hy_state);
			is->hy_state = ST_IDLE;
			continue;
		}
	}
}

/*
 *	Routing
 */

hyroute(ifp, dest, hdr)
register struct ifnet *ifp;
u_long dest;
register struct hy_hdr *hdr;
{
	register struct hy_softc *is = &hy_softc[ifp->if_unit];
	register struct hy_route *rt = &is->hy_route;
	register struct hyr_hash *rhash;
	register int i;


	hdr->h_param = 0;
	if (rt->hyr_lasttime != 0) {
		i = HYRHASH(dest);
		rhash = &rt->hyr_hash[i];
		i = 0;
		while (rhash->hyr_key != dest) {
			if (rhash->hyr_flags == 0 || i > HYRSIZE)
				return(-1);
			rhash++; 
			i++;
			if (rhash >= &rt->hyr_hash[HYRSIZE])
				rhash = &rt->hyr_hash[0];
		}
		hdr->h_ctl = rhash->hyr_ctl;
		hdr->h_access = rhash->hyr_access;
		hdr->h_dest = rhash->hyr_dst;
	}
	return(0);
}

/*
 *	Network Interface
 */

hyinit(unit)
int unit;
{
	register struct hy_softc *is = &hy_softc[unit];
	register struct ifnet *ifp = &is->hy_if;
	register struct mbuf *m;
	int s;

	if(ifp->if_addrlist == 0)		/* address still unknown */
		return;
	
	if(is->hy_recv_msg == (struct mbuf *) 0) {
		/*
		 *	Get an mbuf for receiving message propers
		 */
		if((is->hy_recv_msg = m_get(M_DONTWAIT, MT_HEADER)) == NULL) {
			printf("hy%d: cannot allocate recv mbuf\n", unit);
			return;
		}

		/*
		 * Map the mbuf for receiving message propers ahead of time
		 */
		MAP_RECV_MSG(is, is->hy_recv_msg);
	}

	ifp->if_flags |= IFF_RUNNING;

	s = splhy();

	do {
		IF_DEQUEUE(&ifp->if_snd, m);
		if(m != NULL)
			m_freem(m);
	} while(m != NULL);

	is->hy_state = ST_IDLE;
	hyfsm(unit, CS_NORMAL|CS_ATTF);

	splx(s);
}

hyoutput(ifp, m0, dst)
struct ifnet *ifp;
struct mbuf *m0;
struct sockaddr *dst;
{
	register struct hy_softc *is = &hy_softc[ifp->if_unit];
	register struct hydevice *dr = is->hy_addr;
	register struct hy_swhdr *swhdr;
	register struct mbuf *m;
	register struct mbuf *m1, *m2;
	int s;
	int i;
	u_long dest;
	register char *cp;

	/*
	 *	This driver only supports INET address family
	 */
	if(dst->sa_family != AF_INET) {
		m_freem(m0);
		return(EAFNOSUPPORT);
	}

	/*
	 *	allocate a new header, as we want to copy
	 *	additional data into the mbuf and this
	 *	way we are certain that we won't get
	 *	into trouble by going off of the end
	 */
	m = m_get(M_DONTWAIT, MT_HEADER);
	if (m == 0) {
		m_freem(m0);
		return(ENOBUFS);
	}
	m->m_next = m0;
	m->m_off = MMINOFF;
	m->m_len = sizeof(struct hy_swhdr);

	/*
	 *	get a pointer to the header that we just allocated
	 */
	swhdr = mtod(m, struct hy_swhdr *);
	bzero((caddr_t)swhdr, sizeof(struct hy_swhdr));

	/*
	 * Based on the interface and the internet local host addr,
	 * hyroute fills in trunks, to-addr, access (and params)
	 */
	dest = in_lnaof(((struct sockaddr_in *)dst)->sin_addr);
	i = hyroute(ifp, dest, &swhdr->sw_hdr);
	if (i < 0) {
		m_freem(m);
		return(EHOSTUNREACH);
	}

	/*
	 * set the offset field to point to the IP header
	 */
	swhdr->sw_retry = 0;
	swhdr->sw_hdr.h_offset = HY_OFFSET_IP;
	swhdr->sw_hdr.h_src = htons(is->hy_host) | (HY_IP_CHANNEL << 2);

	/*
	 * Put the message proper into a single mbuf
	 */
	m1 = m->m_next;
	cp = mtod(m, char *);
	cp += m->m_len;
	while(m->m_len + m1->m_len <= HY_SWMSG_SIZE) {
		bcopy(mtod(m1, caddr_t), cp, (unsigned)m1->m_len);
		m->m_len += m1->m_len;
		cp += m1->m_len;
		m->m_next = m1->m_next;
		if((m1 = m_free(m1)) == NULL)
			break;
	}

	/*
	 * if everything did not fit into an mbuf,
	 * place all the remaining data into page mapped mbufs
	 * if they are not already page mapped and aligned
	 */
	if(m1 != NULL) {
		int fillcnt;
		int fail;

		/*
		 * We put data into the message proper to pad it
		 * to 64 bytes so that cray does not have
		 * to adhere to any standards that they don't
		 * chose
		 */
		if(m->m_len < HY_SWMSG_SIZE) {
			fillcnt = HY_SWMSG_SIZE - m->m_len; 
			bcopy(mtod(m1, caddr_t), cp, fillcnt);
			m1->m_off += fillcnt;
			m1->m_len -= fillcnt;
			m->m_len += fillcnt;
			cp += fillcnt;
		}

		/*
		 * if all of the pages except the first are page aligned
		 * and all but the last are entire pages, we do not have
		 * to copy them.
		 */

		fail = 0;
		m2 = m1->m_next;
		while(m2) {
			if((mtod(m2, long)&PGOFSET) != 0) {
				fail = 1;
				break;
			}
			if(m2->m_next && m2->m_len != NBPG) {
				fail = 1;
				break;
			}
			m2 = m2->m_next;
		}

		/*
		 * the special case were the first mbuf contains
		 * an odd number of bytes forces copying of the
		 * entire mbuf chain
		 */

		if((m1->m_len & 0x01) != 0)
			fail = 1;

		if(!fail) {
			/*
			 * if the data ends on a page boundary
			 * and it is word aligned, we do not
			 * have to move it around
			 */
			 
			if(m1->m_next != NULL &&
			  ((mtod(m1, long)+m1->m_len)&PGOFSET) != 0)
				fail = 1;

			if((mtod(m1,int)&1) != 0)
				fail = 1;

			if(fail) {
				struct mbuf *mc, *ml;

				/*
				 * allocate a cluster and copy
				 * the first mbuf into the end of
				 * the new page.
				 */
				mc = m_clget(M_DONTWAIT);
				if(mc == NULL) {
					goto drop;
				}

				ml = m_get(M_DONTWAIT, MT_DATA);
				if (ml == 0) {
					(void) m_pgfree(mc);
					goto drop;
				}

				ml->m_freefunc = m_clfree;
				ml->m_off = (char *)mc - (char *)ml;
				ml->m_off += NBPG - m1->m_len;
				ml->m_len = m1->m_len;

				cp = mtod(ml, caddr_t);
				bcopy(mtod(m1, caddr_t), cp, m1->m_len);

				ml->m_next = m1->m_next;
				m->m_next = ml;

				m_free(m1);
			}
		}
		else {
			/*
			 * we must clean up the entire chain
			 */

			m2 = m;
			while(m1) {
				struct mbuf *mc, *ml;

				/*
				 * allocate a cluster and copy
				 * the first mbuf into the end of
				 * the new page.
				 */
				mc = m_clget(M_DONTWAIT);
				if(mc == NULL) {
					goto drop;
				}

				ml = m_get(M_DONTWAIT, MT_DATA);
				if (ml == 0) {
					(void) m_pgfree(mc);
					goto drop;
				}

				ml->m_freefunc = m_clfree;
				ml->m_off = (char *)mc - (char *)ml;
				ml->m_len = 0;
				m2->m_next = ml;

				cp = mtod(ml, caddr_t);
				while(m1 && (m1->m_len+ml->m_len) <= NBPG) {
					bcopy(mtod(m1, caddr_t), cp, m1->m_len);
					cp += m1->m_len;
					ml->m_len += m1->m_len;
					m1 = m_free(m1);
				}

				if(m1 && ml->m_len != NBPG) {
					int len = NBPG - ml->m_len;
					bcopy(mtod(m1, caddr_t), cp, len);
					ml->m_len += len;
					m1->m_len -= len;
					m1->m_off += len;
				}

				/*
				 * patch up the mbuf chain to reflect
				 * the changes we just made and keep
				 * a pointer to the last mbuf that has
				 * been fixed
				 */
				ml->m_next = m1;
				m2 = ml;
			}
		}
	}
	else if(m->m_len < HY_SWMSG_SIZE) {
		bzero(cp, HY_SWMSG_SIZE - m->m_len);
		m->m_len = HY_SWMSG_SIZE;
	}

	if (m->m_next != NULL)
		swhdr->sw_hdr.h_ctl |= C_ASSOC_DATA;
	else
		swhdr->sw_hdr.h_ctl &= ~C_ASSOC_DATA;

	s = splhy();

	if (IF_QFULL(&ifp->if_snd)) {
		IF_DROP(&ifp->if_snd);
		splx(s);
		goto drop;
	}

	IF_ENQUEUE(&ifp->if_snd, m);
	if (is->hy_state == ST_SET_WAIT) {
		is->hy_state = ST_CLR_WAIT;
		hystart(ifp->if_unit, CMD_CLR_WAIT, 0, 0);
		if(dr->d_csr&CS_MSGPEND) {
			is->hy_active = 0;
			hyfsm(ifp->if_unit, CS_ABNORMAL|CS_ATTF);
		}
	}
	splx(s);
	return (0);

drop:
	m_freem(m);
	return (ENOBUFS);
}

hyioctl(ifp, cmd, data)
register struct ifnet *ifp;
int cmd;
caddr_t	data;
{
	struct ifaddr *ifa = (struct ifaddr *) data;
	struct hyrsetget *sg = (struct hyrsetget *) data;
	struct hy_route *r = &hy_softc[ifp->if_unit].hy_route;
	int error = 0;
	int s = splhy();

	switch(cmd) {

	case SIOCSIFADDR:
		if (ifa->ifa_addr.sa_family != AF_INET)
			return (EINVAL);
		ifp->if_flags |= IFF_UP;
		if ((ifp->if_flags & IFF_RUNNING) == 0)
			hyinit(ifp->if_unit);
		break;

	case HYSETROUTE:
		if (!suser()) {
			error = EPERM;
			goto bad;
		}

		if (sg->hyrsg_len != sizeof(struct hy_route)) {
			error = EINVAL;
			goto bad;
		}
		if (copyin(sg->hyrsg_ptr, r, sg->hyrsg_len)) {
			/* disable further routing if trouble */
			r->hyr_lasttime = 0;
			error = EFAULT;
			goto bad;
		}
		r->hyr_lasttime = time;
		break;

	case HYGETROUTE:
		if (sg->hyrsg_len > sizeof(struct hy_route)) {
			error = EINVAL;
			goto bad;
		}
		if (copyout(r, sg->hyrsg_ptr, sg->hyrsg_len)) {
			error = EFAULT;
			goto bad;
		}
		break;

	default:
		error = EINVAL;
		break;
	}
bad:
	splx(s);
	return (error);
}

hywatch(unit)
int unit;
{
	register struct hy_softc *is = &hy_softc[unit];
	int s;

	s = splhy();
	switch(is->hy_state) {
	case  ST_STARTUP:
	case  ST_IDLE:
	case  ST_SET_WAIT:
		is->hy_if.if_timer = HY_MAXINTERVAL;
		break;
	default:
		iprintf("hywatch: %d\n", is->hy_state);
		hyfsm(unit, CS_ABNORMAL|CS_ATTF);
		break;
	}
	splx(s);
}

/*
 *	Raw Interface
 */

rhyopen(dev)
dev_t dev;
{
	register struct hy_softc *is = &hy_softc[minor(dev)>>6];
	register int chan = minor(dev) & 0x3f;

	if(chan < 1 || chan >= HY_CHANNELS) {
		u.u_error = ENODEV;
		return;
	}

	if(is->hy_open[chan]) {
		u.u_error = EBUSY;
		return;
	}
	is->hy_open[chan] = 1;
}

rhyclose(dev)
dev_t dev;
{
	register struct hy_softc *is = &hy_softc[minor(dev)>>6];
	register int chan = minor(dev) & 0x3f;
	register struct mbuf *m;
	register struct ifqueue *q = &is->hy_rawq[chan];
	int s;

	s = splhy();
	is->hy_open[chan] = 0;

	do {
		IF_DEQUEUE(q, m);
		if(m != NULL)
			m_freem(m);
	} while(m != NULL);

	splx(s);
}

rhyread(dev)
dev_t dev;
{
	register struct hy_softc *is = &hy_softc[minor(dev)>>6];
	register int chan = minor(dev) & 0x3f;
	register struct mbuf *m, *m1;
	int len;
	int s;

	s = splhy();
	for(;;) {
		IF_DEQUEUE(&is->hy_rawq[chan], m);
		if(m != NULL)
			break;
		is->hy_wait_read[chan] = 1;
		sleep(&is->hy_wait_read[chan], PPIPE);
	}
	splx(s);

	m1 = m;
	while(m1 && u.u_count > 0) {
		len = m1->m_len < u.u_count ? m1->m_len : u.u_count;
		if(copyout(mtod(m1, caddr_t), u.u_base, len)) {
			u.u_error = EFAULT;
			break;
		}
		u.u_count -= len;
		u.u_base += len;
		m1 = m1->m_next;
	}

	m_freem(m);
}

rhywrite(dev)
dev_t dev;
{
	register struct hy_softc *is = &hy_softc[minor(dev)>>6];
	register struct hydevice *dr = is->hy_addr;
	register int chan = minor(dev) & 0x3f;
	register struct mbuf *mp;	/* message proper */
	register struct mbuf *ad;	/* associated data */
	register struct mbuf *cl;	/* cluster for data */
	register struct hy_hdr *hdr;
	register struct hy_swhdr *swhdr;
	register struct mbuf *ml;
	int		mplen;
	int		len;
	int		s;

	if(u.u_count < HY_RAW_MIN || u.u_count > HY_RAW_MAX) {
		u.u_error = EIO;
		return;
	}

	mp = m_get(M_WAIT, MT_HEADER);
	mp->m_off = MMINOFF;

	mplen = HY_MPROP_SIZE < u.u_count ? HY_MPROP_SIZE : u.u_count;
	if(copyin(u.u_base, mtod(mp, caddr_t)+HY_HWOFFSET, mplen)) {
		u.u_error = EFAULT;
		m_free(mp);
		return;
	}

	mp->m_len = mplen;
	u.u_count -= mplen;
	u.u_base += mplen;

	swhdr = mtod(mp, struct hy_swhdr *);
	swhdr->sw_hdr.h_src = htons(is->hy_host) | (chan<<2);

	if(UNIT_NO(swhdr->sw_hdr.h_src)==UNIT_NO(swhdr->sw_hdr.h_dest)) {
		if(u.u_count > HY_LOCAL_SIZE) {
			u.u_error = EIO;
			m_free(mp);
			return;
		}
	}

	ml = mp;
	while(u.u_count > 0) {
		cl = m_clget(M_WAIT);
		ad = m_get(M_WAIT, MT_DATA);

		ad->m_freefunc = m_clfree;
		ad->m_off = (char *)cl - (char *)ad;

		ml->m_next = ad;
		ml = ad;

		len = u.u_count > NBPG ? NBPG : u.u_count;

		if(copyin(u.u_base, mtod(ad, caddr_t), len)) {
			u.u_error = EFAULT;
			m_freem(mp);
			return;
		}
		ad->m_len = len;
		u.u_base += len;
		u.u_count -= len;
	}

	s = splhy();
	if (IF_QFULL(&is->hy_if.if_snd)) {
		IF_DROP(&is->hy_if.if_snd);
		u.u_error = ENOBUFS;
		m_freem(mp);
		splx(s);
		return;
	}

	IF_ENQUEUE(&is->hy_if.if_snd, mp);
	if (is->hy_state == ST_SET_WAIT) {
		is->hy_state = ST_CLR_WAIT;
		hystart(minor(dev)>>6, CMD_CLR_WAIT, 0, 0);
		if(dr->d_csr&CS_MSGPEND) {
			is->hy_active = 0;
			hyfsm(minor(dev)>>6, CS_ABNORMAL|CS_ATTF);
		}
	}

	sleep(&is->hy_wait_write[chan], PPIPE);
	splx(s);

	if(is->hy_write_err[chan])
		u.u_error = EIO;
}

rhyioctl(dev, cmd, addr, flag)
dev_t dev;
int cmd;
caddr_t addr;
int flag;
{
	register struct hy_softc *is = &hy_softc[minor(dev)>>6];

	cmd &= 0xff;
	if (cmd == CMD_LSTAT) {
		if (copyout(&is->hy_err_status[minor(dev)&0x3f], addr, 8))
			u.u_error = EFAULT;
		return;
	}
	u.u_error = EINVAL;
}

#endif
