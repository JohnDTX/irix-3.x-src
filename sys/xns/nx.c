# define BLOCKOFF_FIX
# define BLOCKFIONREAD
# define SGI_METER
/*
 * Driver for Excelan ethernet controller as used by XNS
 *
 * $Source: /d2/3.7/src/sys/xns/RCS/nx.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:36:21 $
 */

#include "../h/param.h"
#include "../h/buf.h"
#include "../h/systm.h"
#include "../h/signal.h"
#include "../h/dir.h"
#include "../h/map.h"
#include "../h/user.h"
#include "../h/proc.h"
#include "../h/errno.h"
#include "../h/conf.h"
#include "../h/tty.h"
#include "../h/termio.h"
#include "../h/inode.h"
#include "../h/file.h"
#include "../xns/if_xns.h"

extern long nxeva;
#define NONBP ((NBP)nxeva)

#define pgrpsignal(pgrp, sig)	signal(pgrp, sig)


int	nxproc();

/* XXX move this into a header file */
extern	NBP xns_hostname();
extern	NBP qfirst();

/* index of next tty to look at for NXAVAIL */
static short lastavail;

struct stats_reply nx_cum_stats;/* cumulative stats */
long nx_n_stats;		/* # times stats done */
long nx_n_rcverr;		/* # rcv errors */

int nx_def_maxinpq = 8;		/* default maxinpq */

extern NBP watch_rcv();
struct xnsmisc *nx_watchmisc;/* pointer to misc of watcher */
static long watch_dropped;	/* number dropped while watching */

long nx_unwanted = 0;
long nx_tomulti = 0;
long nx_oflow = 0;
long nx_cloned = 0;
long nx_bounced = 0;
long nx_unbounce = 0;
long nx_kbounce = 0;

# undef METER
# ifdef SGI_METER
# define METER(x)	(x)
# else  SGI_METER
# define METER(x)
# endif SGI_METER


/*
 * nxopen is a "normal" character special device open except
 * that it always succeeds: i.e. it never waits for CARR_ON.
 *
 * The sleep for carrier detect is instead done in the first read or write.
 * The algorithm is:
 *	1. in nxopen, t_state has WOPEN set, CARR_ON cleared, and x_conp zero.
 *	2. nxread/nxwrite will sleep on t_rawq if WOPEN is set.
 *	3. the xns code clears WOPEN and sets CARR_ON when a connection is made.
 *	4. x_socket contains the waiting socket number.
 *	   x_socket of zero implies waiting on the LOGINSOCKET.
 */
nxopen(dev)
	dev_t dev;
{
	register struct tty *tp;
	register struct xnsmisc *xp;
	register short m;
	register s;

	m = NXDEV(dev);
	if (m >= NDEV || !nxpresent) {
		u.u_error = ENXIO;
		return;
	}

	xp = &xnsmisc[m];
	tp = &nx_tty[m];
	xp->x_ttyp = tp;

# ifdef NXAVOPEN
	if (m != 0) {
		xp->x_owner = u.u_procp->p_pid;
		/*
		if (xp->x_owner != u.u_procp->p_pid) {
		    u.u_error = EBUSY;
		    return;
		}
		 */
	}
# endif NXAVOPEN

	if ((tp->t_state&ISOPEN) == 0) {
		LOCK;
		tp->t_proc = nxproc;
		tp->t_index = m;
		xp->x_state = 0;
		xp->x_conp = NULL;
		xp->x_socket = 0;
		ttinit(tp);
		(*linesw[tp->t_line].l_open)(tp);
		tp->t_pgrp = u.u_procp->p_pgrp;		 /* force pgrp */

		tp->t_state = ISOPEN | WOPEN;	/* set to correct state */

		UNLOCK;
	}
	else {
		(*linesw[tp->t_line].l_open)(tp);
	}
}

/*
 * Normal unix close routine plus:
 *	free broadcast packet queue if the bounce daemon is exiting,
 *	disable multicast if the process had a slot enabled,
 *	free up any input queue (inpq) or hanging (x_pend) buffers,
 *	tell xns to disconnect.
 */
nxclose(dev)
	dev_t dev;
{
	register struct tty *tp;
	register struct xnsmisc *xp;
	register short m;
	register NBP nbp;

	m = NXDEV(dev);
	xp = &xnsmisc[m];
	tp = xp->x_ttyp;

	if (xp == nx_watchmisc)
		offwatch();	/* stop watching */

	if (xp->x_state & X_MULTI)
		nxsetrcv(m, 0);
	if ((nbp = xp->x_pend) != NULL) {
		freenbuf(nbp);
		xp->x_pend = NULL;
	}
	xp->x_uetype = 0;
	xns_close(tp);
	xns_qflush(&xp->x_inpq);
	xp->x_maxinpq = 0;
	tp->t_state = 0;
	(*linesw[tp->t_line].l_close)(tp);
	xp->x_state = 0;
	xp->x_conp = NULL;
	xp->x_socket = 0;
# ifdef NXAVOPEN
	xp->x_owner = 0;
# endif NXAVOPEN
}

/*
 * Interrupt Routine.
 * Process message buffers returned by the controller.
 * Pass incoming data to packet decoder.
 * Wake up processes sleeping on output message buffers.
 */
# ifdef IFXNS
nxns_intr(mp)
	NxMsg *mp;
# else  IFXNS
nxintr() 
# endif IFXNS
{
	register NxMsg *rmp; 	/* receive q message pointer */
	register NBP nbp;
	register CONN conp;
	extern char msgbufs_free;

	/*
	 * Bump software interrupt flag on the way in, clear going out.
	 * Clear device interrupt via PORTA on the way in.
	 * The id field points to a netbuf.
	 * `lastrmp' always points to the <next> msgbuf to be returned
	 * from the device to the host.  Returned msgbufs have status 0.
	 */
	nxintflag++;
# ifndef IFXNS
	*PORTA = 1;
	rmp = (NxMsg *)lastrmp;

	while ((rmp->status & (MX_EXOS|MX_BUSY)) == 0) {
		extern long nx_n_rmp;

		nx_n_rmp++;
# else   IFXNS
		rmp = mp;
# endif  IFXNS
		nbp = (NBP)rmp->id;
		NXTRACE(("nxintr: id=%x status=%x request=%x reply=%x",
				  rmp->id, rmp->status, rmp->request,
				  rmp->reply));
		switch (rmp->request) {
		  case NET_READ:
			nx_qpacket((READ)rmp, nbp);
			readenable();
			break;

		  case NET_XMIT:
			if (rmp->reply >= 0x10)
				iprintf("nx: write error, reply=%x\n",
					     rmp->reply);
			/*
			 * write completion. In this implementation it is
			 * either xns with a conp, or it must be a raw write.
			 * note that in the case of a conp, nbp might be
			 * a NONBP pointer.
			 */
			conp = (CONN)rmp->block[7].addr;
			if (conp) {
				xns_xdone(conp);
				if (conp->utp && conp->state==S_ACTIVE)
					nxstart(conp->utp);
			} else {
				NXTRACE(("wakeup nbp=%x", nbp));
				wakeup((caddr_t)nbp);
				nbp->btype |= N_DONE;
				freenbuf(nbp);
			}
			break;

		  case NET_RECV:
		  case NET_MODE:
		  case NET_ADDRS:
			/*
			 * other device requests.
			 */
			bcopy((caddr_t)rmp, nbp->data, EXOSMAX);
			wakeup((caddr_t)nbp);
			nbp->btype |= N_DONE;
			break;

		  case NET_STATS: {
			/* update statistics */
			nx_cum_stats = *nxmtod(nbp, struct stats_reply *);
			nx_n_stats ++;
			freenbuf(nbp);
			break;
		  }

		  default:
			iprintf("nx: unknown msg, request=%x reply=%x\n",
				     rmp->request, rmp->reply);
		}

		/*
		 * Restore msgbuf for reuse.
		 */
		msgbufs_free++;
		rmp->length = MSGLEN;
		rmp->block[6].addr = 0;
		rmp->block[7].addr = 0;
		rmp->status = MX_EXOS|MX_BUSY;
# ifndef IFXNS
		rmp = (NxMsg *)((MP)rmp)->next;
	}

	lastrmp = (MP)rmp;
# endif  IFXNS

	if (mwaiting) {
		mwaiting = 0;
		wakeup((caddr_t)&mwaiting);
		NXTRACE(("MWAIT WAKEUP"));
		XXTRACE(("MSGBUF WAIT WAKEUP, nxreads=%d", nxreads));
	}
	nxintflag--;
}

/*
 * nx_qpacket() --
 * put a packet on the right queue if any.
 * otherwise, free it.
 */
nx_qpacket(rmp, nbp)
	register READ rmp;
	register NBP nbp;
{
	register short etype;
	register struct xnsmisc *xp;
	register short x;

	/*
	 * if watch-ing, put it on the watch queue,
	 * but otherwise ignore packets not addressed to us.
	 * otherwise, if it's an XNS packet,
	 * pass it to the protocol receive routine.
	 * otherwise, if it's for a raw reader,
	 * pass it to the appropriate raw input queue.
	 * otherwise, ignore it.
	 */
	nxreads--;
	nbp->len = rmp->block[0].len - NX_TRAILER_SIZE;

	etype = nxmtod(nbp, etheader *)->etype;

# ifndef IFXNS
	/*
	 * if the packet came from the ex driver,
	 * watching is already taken care of.
	 */
	if (rmp->reply > 2) {
		/*
		iprintf("nx: rcv error, reply=%x id=%x\n",
				rmp->reply, (int)nbp);
		 */
		nx_n_rcverr++;
		nbp->control = rmp->reply;
		if (nx_watchmisc != 0)
			(void) watch_rcv(nbp, 0);
		else
			freenbuf(nbp);
		return;
	}

	if (nx_watchmisc != 0) {
		if ((nbp = watch_rcv(nbp, 1)) == NULL)
			return;
	}
# endif  IFXNS

	if (etype == (short)IDETHERTYPE) {
		xns_rcv(nbp);
		return;
	}
	if (etype == (short)SG_BOUNCE) {
		nxbounce(nbp);
		return;
	}

	x = rmp->slot&0xff;
	if (0 < x && x < NDEV
	 && (xp = (xnsmisc+x))->x_state & X_MULTI) {
		nxmulti(nbp, xp);
		return;
	}

# ifdef NXUPROTO
	for (xp = xnsmisc , x = NDEV; --x >= 0; xp++)
		if (xp->x_state & X_UPROTO
		 && xp->x_uetype == etype) {
			nxmulti(nbp, xp);
			return;
		}
# endif NXUPROTO

	METER(nx_unwanted++);
	freenbuf(nbp);
}

/*
 * readenable:
 *	- start some receive commands, so that the board will be able to
 *	  give received packets to us
 */
readenable()
{
# ifndef IFXNS
	register NBP nbp;
	register READ rdmp;

	while (nxreads < NREADS) {

		/*
		 * To start a read, we need both a netbuf and a msgbuf.
		 * If we can't get both, give up to avoid deadlock.
		 */
		if ((nbp = getnbuf(1)) == NULL) {
			XXTRACE(("READENABLE: NO NETBUFS, nxreads=%d",
					      nxreads));
			break;
		}
		nbp->btype |= B_RECV;
		if ((rdmp = (READ)getmsgbuf()) == NULL) {
			freenbuf(nbp);
			XXTRACE(("READENABLE: NO MSGBUFS, nxreads=%d",
					      nxreads));
			break;
		}

		/*
		 * Set up the various data structures.
		 */
		XXTRACE(("readenable: mp=%x nbp=%x nxreads=%d", rdmp,
				      nbp, nxreads));
		rdmp->request = NET_READ;
		rdmp->nblocks = 1;
		rdmp->block[0].len = MAXPACKET;
		rdmp->block[0].addr = nbp->nxpaddr;
		putmsg((NxMsg *)rdmp, nbp);
		nxreads++;
	}
# endif  IFXNS
}

nxns_readenable()
{
	nxintflag++;
	readenable();
	nxintflag--;
}


/*
 * process SGI bounce packet
 */
nxbounce(nbp)
	register NBP nbp;
{
	register NBP rnbp;
	register XMIT xp;

	/*
	 * xns_hostname examines the packet:
	 *	if NULL return, put packet on bounce server queue,
	 *	if it returns nbp, transmit the returned buffer,
	 *	otherwise ignore the packet.
	 */
	METER(nx_bounced++);
	rnbp = xns_hostname(nbp);

	if (rnbp == NULL && sgbouncemisc != NULL) {
		nxmulti(nbp, sgbouncemisc);
	} else
	if (rnbp == nbp) {
		METER(nx_kbounce++);
		if ((xp = (XMIT)getmsgbuf()) == NULL)
		 	freenbuf(nbp);
		else {
			xp->request = NET_XMIT;
			xp->nblocks = 1;
			xp->block[0].len = nbp->len;
			xp->block[0].addr = nbp->nxpaddr;
			xp->block[7].addr = NULL;
# ifdef IFXNS
			if (nbp->m != 0) {
				bcopy(nbp->data, nbp->info, nbp->len);
				UNMBUFIZE(nbp);
			}
# endif IFXNS
			nbp->btype = B_RAWXMIT;
			putmsg((NxMsg *)xp, nbp);
		}
	} else {
		METER(nx_unbounce++);
		freenbuf(nbp);
	}
}

nxmulti(nbp, xp)
	register NBP nbp;
	register struct xnsmisc *xp;
{
	METER(nx_tomulti++);
	/*
	 * Reject multicast packets generated by us.
	 */
	if (xp->x_state & X_DISCARD
	 && EQPHYSNET(&myaddr, &((etheader *)nbp->data)->src)) {
		freenbuf(nbp);
		return;
	}

	/*
	 * Append buffer to inpq.
	 * Discard if queue already contains the maximum
	 * number of packets.
	 */
	if (xp->x_inpq.nbufs < xp->x_maxinpq) {
		nbp->btype = B_INPQ;
		nbappend(nbp, &xp->x_inpq);	/* add to queue */
	}
	else {
		METER(nx_oflow++);
		freenbuf(nbp);			/* drop packet */
	}
	wakeup((caddr_t)&xp->x_inpq);		/* wakeup reader */
}

/*
 * Device read routine.
 * Check for bootsrv,
 * handle wait-for-connection blocking,
 * call line discipline for i/o.
 */
nxread(dev)
	dev_t dev;
{
	register struct tty *tp;
	register struct xnsmisc *xp;
	register short m = NXDEV(dev);
	unsigned transfer;

	NXTRACE(("nxread on m=%d x_state=%x", m, xnsmisc[m].x_state));
	xp = &xnsmisc[m];
	tp = xp->x_ttyp;

	if (xp == nx_watchmisc) {
		extern time_t lbolt;

		struct {
			time_t timestamp;	/* arrival time */
			char control;
			char dummy;
			short ndropped;		/* number dropped */
		} whead;
		register caddr_t timep;

		/* will fill in and copy after ilbread() */
		timep = u.u_base;
		u.u_base += sizeof whead;
		u.u_count -= sizeof whead;

		ilbread(&xp->x_inpq, (char *)NULL, &whead.control);

		whead.timestamp = lbolt;
		whead.ndropped = watch_dropped;
		if (copyout((caddr_t)&whead, timep, sizeof whead))
			u.u_error = EFAULT;

		return;
	}

	if (xp->x_state & (X_MULTI|X_UPROTO)) {
		ilbread(&xp->x_inpq, (char *)NULL, (char *)NULL);
		return;
	}

	/*
	 * blocking first read to complement non-blocking open.
	 */
	if (tp->t_state & WOPEN) {
		if (!(xp->x_state & X_SETSOCKET)) {
			xp->x_state |= X_SETSOCKET;
			xp->x_socket = LOGINSOCKET;
			NXTRACE(("nxread: login set, tp=%x conp=%x x_state=%x",
					  tp, xp->x_conp, xp->x_state));
		}
		while ((tp->t_state & CARR_ON) == 0)
			sleep((caddr_t)&tp->t_rawq, TTIPRI);
	}

	if (xp->x_state & X_EOF
	 && (!(xp->x_state & X_CONNECTED) || !(tp->t_state & CARR_ON))) {
		u.u_error = ECONNABORTED;
		nxsignal(xp);
		return;
	}
	if ((tp->t_state&ISOPEN) == 0 || !(xp->x_state & X_CONNECTED)) {
		NXTRACE(("nxread: not connected: tp=%x x_state=%x",
				  tp, xp->x_state));
		u.u_error = EIO;
		return;
	}

	transfer = u.u_count;
	if (xp->x_state & X_BLOCK) {
		if (xns_read(xp->x_conp, (char *)NULL, (char *)NULL))
			u.u_error = EIO;
		if (xp->x_state & X_EOFPEND && transfer == u.u_count)
			xp->x_state |= X_EOF;
	} else {
		(*linesw[tp->t_line].l_read)(tp);
		if (xp->x_state & X_EOFPEND && transfer == u.u_count
		 && tp->t_canq.c_cc + tp->t_rawq.c_cc <= 0)
			xp->x_state |= X_EOF;
	}
}

ilbread(q, dtype, control)
	register struct netqueue *q;
	char *dtype, *control;
{
	register NBP nbp;
	register len;
	register int s;
	char ndtype,ncontrol;

	LOCK;
	while ((nbp=q->head)==NULL) {
		if (q->nbufs<0)
			break;
		NXTRACE(("ilbread sleep on q=%x nbufs=%d nxreads=%d curxmp=%x",
				  q, q->nbufs, nxreads, curxmp));
		sleep((caddr_t)q, TTIPRI);
	}
	if (nbp==NULL) {
		NXTRACE(("ilbread exit with null nbp"));
		UNLOCK;
		return;
	}

	if (dtype)
		*dtype = nbp->dtype;
	if (control)
		*control = nbp->control;

	/* save dtype, control in case there are leftovers */
	ndtype = nbp->dtype;
	ncontrol = nbp->control;

	nbp->dtype = 0;
	nbp->control = 0;
	UNLOCK;
	len = MIN(nbp->len, u.u_count);
	iomove(nbp->data, len, B_READ);

	/*
	 * save leftovers for later use (UGH).
	 * restore dtype, control.
	 */
	if ((nbp->len -= len) > 0) {
		nbp->data += len;
		LOCK;
		nbp->dtype = ndtype;
		nbp->control = ncontrol;
		UNLOCK;
	} else
		qfreenbuf(q);
}

/*
 * device write routine.
 */
nxwrite(dev)
	dev_t dev;
{
	register NBP nbp;
	register short m = NXDEV(dev);
	register int cc;
	register XMIT mp;
	register struct tty *tp;
	register struct xnsmisc *xp;

	m = NXDEV(dev);
	xp = &xnsmisc[m];
	tp = xp->x_ttyp;

	/*
	 * perform raw output.
	 */
	if (xp->x_state & X_RAW) {
		if ((cc=u.u_count)>=MAXPACKET) {
			u.u_error = EINVAL;
			return;
		}
		nbp = getnbuf(0);
		nbp->btype |= B_RAWXMIT;
		nbp->len = cc;
		iomove(nbp->data, cc, B_WRITE);
		if (u.u_error) {			/* faulted */
			freenbuf(nbp);
			return;
		}

		mp = (XMIT)getmsgbuf();
		mp->request = NET_XMIT;
		mp->nblocks = 1;
		mp->block[0].len = cc;
		mp->block[0].addr = nbp->nxpaddr;
		mp->block[7].addr = NULL;

		wputmsg((NxMsg *)mp, nbp, PRIBIO);
		return;
	}
	/*
     	 * blocking first write to complement non-blocking open.
     	 */
	if (tp->t_state & WOPEN) {
		if (!(xp->x_state & X_SETSOCKET)) {
			xp->x_state |= X_SETSOCKET;
			xp->x_socket = LOGINSOCKET;
			NXTRACE(("nxwrite: login set, tp=%x conp=%x x_state=%x",
					  tp, xp->x_conp, xp->x_state));
		}
		while ((tp->t_state & CARR_ON) == 0)
			sleep((caddr_t)&tp->t_rawq, TTOPRI);
	}

	if (xp->x_state & X_WRERROR) {
		u.u_error = ECONNABORTED;
		nxsignal(xp);
		return;
	}

	/*
     	 * Fast mode means bypass typewriter line disciplines on output.
	 */
	if (xp->x_state & (X_BLOCK | X_FAST))
		ilbwrite(tp, 0, 0);
	else
		(*linesw[tp->t_line].l_write)(tp);

	/*
	 * If error, remember to deliver a signal next time.
	 */
	if (u.u_error)
		xp->x_state |= X_WRERROR;	/* |X_EOFPEND?*/
}

nxsignal(xp)
	register struct xnsmisc *xp;
{
	if (xp->x_state & X_PGRP)
		pgrpsignal((int)xp->x_ttyp->t_pgrp, SIGHUP);
	else
		psignal(u.u_procp, SIGHUP);
}

ilbwrite(tp, dtype, control)
	register struct tty *tp;
{
	register NBP nbp;
	register struct xnsmisc *xp;

	xp = &xnsmisc[tp->t_index];
	do {
		if (u.u_count==0 && control==0 && dtype==0)
			break;
		if (!(xp->x_state & X_CONNECTED)) {	/* no connection */
			u.u_error = EIO;
			return;
		}
		nbp = getnbuf(0);
		nbp->btype |= B_XMIT;
		nbp->len = MIN(1024, u.u_count);
		nbp->dtype = dtype;
		nbp->control = control;
		iomove(nxmtod(nbp, caddr_t), nbp->len, B_WRITE);
		if (u.u_error) {
			freenbuf(nbp);
			return;
		}
		xns_start(xp->x_conp, nbp);
	} while (u.u_count);
}

/*
 * process tty operations
 */
nxproc(tp, cmd)
	register struct tty *tp;
	register short cmd;
{
	extern ttrstrt();

	if (tp==0)
		return;
	switch (cmd) {
	case T_TIME:	/* restart output after a delay timeout */
		tp->t_state &= ~TIMEOUT;
		goto start;

	case T_WFLUSH:	/* flush write queue */
		tp->t_state &= ~BUSY;
		/* fall through */

	case T_RESUME:	/* resume stopped output */
		tp->t_state &= ~TTSTOP;

	case T_OUTPUT:
start:
		if (!(tp->t_state & (BUSY|TTSTOP)))
			nxstart(tp);
		break;

	case T_SUSPEND:	/* suspend (stop) output */
		tp->t_state &= ~BUSY;
		tp->t_state |= TTSTOP;
		break;

	case T_BLOCK:	/* output blocked */
		tp->t_state &= ~TTXON;
		tp->t_state |= TBLOCK;
		if (tp->t_state & BUSY)
			tp->t_state |= TTXOFF;
		break;

	case T_RFLUSH:	/* flush read */
		if (!(tp->t_state&TBLOCK))
			break;
		/* fall through */

	case T_UNBLOCK:	/* output unblocked */
		tp->t_state &= ~(TTXOFF|TBLOCK);
		if (tp->t_state & BUSY)
			tp->t_state |= TTXON;
		break;

	case T_BREAK:	/* send a break character */
		tp->t_state |= TIMEOUT;
		timeout(ttrstrt, (caddr_t)tp, hz>>2);
		break;
	}
}

/*
 * Start routine provided for typewriter line discipline output.
 * It may be called at interrupt time (character echo) or normally.
 */
nxstart(tp)
	register struct tty *tp;
{
	register struct ccblock *tbuf;
	register NBP nbp;
	register CONN conp;
	register struct xnsmisc *xp;
	register int s;
	register short amount_taken;
	char *cp;

	s = spl6();
	if (tp==0) {
		splx(s);
		return;
	}
	xp = &xnsmisc[tp->t_index];
	if (xp->x_state & X_FAST)
		goto out;

	/*
	 * If no connection, ditch data in clist's
	 * XXX why is this here
	 */
	if  (!(xp->x_state & X_CONNECTED)) {
		xns_flush_ttyq(&tp->t_outq);
		goto out;
	}
	conp = xp->x_conp;

	/* XXX this code is a repeat, more or less, of xns_ttstart... */
	/* XXX this flow of control stuff is a bit complicated. can we
	       clean it up */
	if (conp->blocked)
		goto out;		/* don't bother trying */
	if (conp->state != S_ACTIVE)
		goto out;
	if (conp->lastseq == conp->ralloc) {
		NXTRACE(("fail ttstart on flow: conp=%x lastseq=%d ralloc=%d",
			       conp, conp->lastseq, conp->ralloc));
		tp->t_state |= BUSY;
		goto out;
	}
	if (tp->t_state & TTSTOP)
		goto out;
	if (conp->pending.nbufs) {
		(void) xns_ttstart(conp, qfirst(&conp->pending));
		goto out;
	}

	nbp = getnbuf(0);
	if (nbp == NULL)
		goto out;
	nbp->btype |= B_XMIT;

    /* now we have a buffer to put data into; copy data from clists into buf */
	tbuf = &tp->t_tbuf;
	amount_taken = 0;
	cp = nxmtod(nbp, caddr_t);
	for (;;) {
		if (tbuf->c_ptr == 0 || tbuf->c_count == 0) {
			if (tbuf->c_ptr)
				tbuf->c_ptr -= tbuf->c_size - tbuf->c_count;
			if (!(CPRES &
			    (*linesw[(short)tp->t_line].l_output)(tp))) {
				break;
			}
		}
		if (amount_taken + tbuf->c_count >= 1024)
			break;

		/* XXX begin hack */
		/* XXX	s = spl6(); */
		/* XXX	serial_poll(); */
		/* XXX	splx(s); */
		/* XXX end hack */

		bcopy(tbuf->c_ptr, cp, tbuf->c_count);
		amount_taken += tbuf->c_count;
		cp += tbuf->c_count;
		tbuf->c_ptr += tbuf->c_count;
		tbuf->c_count = 0;
	}
	if (amount_taken) {
		nbp->dtype = 0;
		nbp->len = amount_taken;
		(void) xns_ttstart(conp, nbp);
	} else {
		freenbuf(nbp);
	}
	splx(s);
	return;
out:
	splx(s);
}

int nx_errmask = 0;
/*
 * Change device state to newmode.
 */
nxmode(newmode)
	int newmode;
{
	register MODE mp;
	register NBP nbp;
	register int s;
	register int timo;

	mp = (MODE)getmsgbuf();
	if ((nbp = getnbuf(1)) == NULL) {
		printf("no netbufs in nxmode!\n");
		return -1;
	}
	nbp->btype |= B_XMIT;
	mp->request = NET_MODE;
	mp->reqmask = WRITEREQ;
	mp->errmask = nx_errmask;		/* XXX */
	mp->mode = newmode;

	/* can't sleep here */
	LOCK;
	putmsg((NxMsg *)mp, nbp);

	for (timo = 2000; --timo >= 0;) {
		if (nbp->btype&N_DONE)
			break;
# ifdef IFXNS
		exintr();
# else  IFXNS
		nxintr();
# endif IFXNS
		msdelay(1);
	}

	UNLOCK;

	mp = (MODE)nbp->data;
	s = (mp->reply || timo < 0) ? -1 : 0;
	freenbuf(nbp);
	return s;
}

/*
 * read out device addr slot.
 */
nxgetslot(slotno, a)
	struct physnet *a;
{
	register ADDRS ap;
	register NBP nbp;

	ap = (ADDRS) getmsgbuf();
	ap->request = NET_ADDRS;
	ap->reqmask = READREQ;
	ap->slot = slotno;
	if ((nbp = getnbuf(1)) == NULL) {
		printf("no netbufs in nxgetslot!\n");
		return -1;
	}
	nbp->btype |= B_XMIT;

	wputmsg((NxMsg *)ap, nbp, PRIBIO);

	ap = (ADDRS)nbp->data;
	*a = ap->addr;
	if (ap->reply)
		u.u_error = EIO;
	freenbuf(nbp);
	return 0;
}

nxputslot(slotno, a)
	struct physnet *a;
{
	register ADDRS ap;
	register NBP nbp;

	ap = (ADDRS) getmsgbuf();
	ap->request = NET_ADDRS;
	ap->reqmask = WRITEREQ;
	ap->slot = slotno;
	nbp = getnbuf(0);
	nbp->btype |= B_XMIT;
	ap->addr = *a;

	wputmsg((NxMsg *)ap, nbp, PRIBIO);

	ap = (ADDRS)nbp->data;
	if (ap->reply)
		u.u_error = EIO;
	freenbuf(nbp);
}

nxsetrcv(slotno, mode)
	short slotno;
	int mode;
{
	register RECV ap;
	register NBP nbp;

	ap = (RECV) getmsgbuf();
	ap->request = NET_RECV;
	ap->reqmask = WRITEREQ|((mode)?04:0);
	ap->slot = slotno;
	nbp = getnbuf(0);
	nbp->btype |= B_XMIT;

	wputmsg((NxMsg *)ap, nbp, PRIBIO);

	ap = (RECV)nbp->data;
	if (ap->reply)
		u.u_error = ENXIO;
	freenbuf(nbp);
}

nxioctl(dev, cmd, arg, mode)
	dev_t dev;
	register int cmd;
	int mode;
	caddr_t arg;
{
	register int s;
	register struct tty *tp;
	register struct xnsmisc *xp;

	register short d = NXDEV(dev);
	register int error;
	register int v;

	if (!xns_ready)
		xns_go();

	xp = &xnsmisc[d];
	tp = xp->x_ttyp;

# ifdef BLOCKFIONREAD
	/*
	 * intercept and simulate FIONREAD for block mode.
	 */
	if (cmd == FIONREAD && xp->x_state & X_BLOCK) {
		extern off_t nxccount();

		off_t ccount;

		ccount = 0;
		if (!(xp->x_state & X_CONNECTED)
		 && xp->x_state & (X_MULTI|X_UPROTO))
			ccount = nxccount(&xp->x_inpq);
		else
		if (xp->x_conp != NULL)
			ccount = nxccount(&xp->x_conp->inpq);
		if (copyout((caddr_t)&ccount, arg, sizeof ccount))
			u.u_error = EFAULT;
		return;
	}
# endif BLOCKFIONREAD

	if ((cmd & IOCTYPE) != NXIOCTYPE) {
		(void) ttiocom(tp, cmd, (int)arg, mode);
		return;
	}

	/*
	 * error flag		=0 - no error
	 *			<0 - not supported
	 *			>0 - error code
	 */
	error = 0;

	switch (cmd) {
	/*
	 * Pop board with an interrupt to attempt an unwedge
	 */
	case NXBOINK: {
		extern short nx_play_dead;

		if (!suser())
			return;
		nx_play_dead = 1;
		break;
		}

	/*
	 * Read statistics from the board
	 */
	case NXSTATS: {
		if (copyout((caddr_t)&nx_cum_stats,
				arg, sizeof nx_cum_stats)) {
			error = EFAULT;
			break;
		}
		(void) nxgetstats();
		break;
		}

	/*
	 * process wants to receive SG Bounce packets.
	 */
	case NXIOBOUNCE: {
		if (sgbouncemisc != NULL) {
			error = EADDRINUSE;
			break;
		}
		sgbouncemisc = xp;
		xp->x_maxinpq = 99;	/* XXX */
		xp->x_uetype == SG_BOUNCE;
		xp->x_state |= X_UPROTO;
		break;
		}

	/*
	 * Bypass typewriter output routines.
	 */
	case NXIOFAST: {
		xp->x_state |= X_FAST;
		break;
		}

	case NXIOSLOW: {
		xp->x_state &= ~X_FAST;
		break;
		}

	case NXGETCONN: {
		break;
	}

	/*
	 * raw network output.
	 */
	case NXIORAW: {
		xp->x_state |= X_RAW;
		break;
		}
	/*
	 * set block mode.
	 */
	case NXBLOCKIO: {
		xp->x_state |= X_BLOCK;
		xp->x_state &= ~X_FAST;
		break;
		}

	case NXBLOCKOFF: {
		xp->x_state &= ~(X_BLOCK | X_FAST);
# ifdef BLOCKOFF_FIX
		if (xp->x_state & X_CONNECTED)
			xns_blockoff_rcv(xp->x_conp);
# endif BLOCKOFF_FIX
		break;
		}

	/*
	 * return interface board net address.
	 */
	case NXPHYSADDR: {
		if (copyout((caddr_t)&myaddr, arg, sizeof(myaddr)))
			error = EFAULT;
		break;
		}

	/*
	 * Set receive socket number.
	 *	Negative number interpreted as "any socket."
	 *	Socket must be between 0 and SOCKBASE.
	 */
	case NXSETSOCKWAIT:
	case NXSOCKET: {
		short socketno;

		/* make sure we aren't already connected */
		if (xp->x_state & (X_CONNECTED | X_SETSOCKET)) {
			error = EISCONN;
			break;
		}
		if (copyin(arg, (caddr_t)&socketno, sizeof(socketno))) {
			error = EFAULT;
			break;
		}
		v = (long)socketno;
		if (v==0 || v>=SOCKBASE) {
			error = EINVAL;
			break;
		}
		if (v<0)
			v = ALLSOCK;
		xp->x_socket = v;
		xp->x_state |= X_SETSOCKET;
		NXTRACE(("setsocket: tp=%x socket=%d x_state=%x t_state=%o",
				     tp, xp->x_socket, xp->x_state,
				     tp->t_state));
		if (cmd == NXSOCKET)
			break;
		/* FALLTHROUGH */
		}

	/*
	 * Wait for connection.  Must set socket first.
	 */
	case NXSOCKWAIT:
		if (!(xp->x_state & X_SETSOCKET)) {
			error = EINVAL;
			break;
		}
		NXTRACE(("sockwait: tp=%x socket=%d x_state=%x t_state=%o", tp,
				    xp->x_socket, xp->x_state, tp->t_state));
		LOCK;
		while ((tp->t_state & CARR_ON) == 0) {
			tp->t_state |= WOPEN;
			NXTRACE(("sockwait: tp=%x socket=%d",
					    tp, xp->x_socket));
			sleep((caddr_t)&tp->t_rawq, TTOPRI);
		}
		UNLOCK;
		break;

	/*
	 * Make outgoing net connection.
	 * Copy an xns header from user and pass it over
	 * to xns routines.
	 */
	case NXCONNECT: {
		struct xns_setup x;

		if (copyin(arg, (caddr_t)&x, sizeof(x))) {
			error = EFAULT;
			break;
		}
		NXTRACE(("nxconnect: tp=%x", tp));
		v = xns_connect(tp, &x);
		if (v == 0)
			xp->x_state |= X_FAST | X_BLOCK;
		else {
			xp->x_state &= ~(X_FAST | X_BLOCK);
			error = u.u_error;
		}
		NXTRACE(("nxconnect tp=%x v=%d state=%x", tp, v, xp->x_state));
		break;
		}

	/*
	 * See if special file is in use by more than 1 proc.
	 * This is done by checking the inode reference count.
	 */
	case NXIOTEST: {
		short count;
		register struct file *fp;

		if ((fp=getf(u.u_arg[0]))==NULL) {
			error = EBUSY;
			break;
		}
		count = fp->f_inode->i_count;
		if (copyout((caddr_t)&count, arg, sizeof count))
			error = EFAULT;
		break;
		}

	/*
	 * return minor device number of an available file.
	 */
	case NXAVAIL: {
		short count;

		LOCK;
		count = -1;
		for (d = NDEV; --d >= 0; ) {
			if (lastavail <= 0)
				lastavail = NDEV;
			lastavail--;
			if (nx_tty[lastavail].t_state == 0) {
				count = lastavail;
				break;
			}
		}
		UNLOCK;

		if (copyout((caddr_t)&count, arg, sizeof count))
			error = EFAULT;
		break;
		}

	/*
	 * Force pgrp of tty to match that of current process.
	 */
	case NXSETPGRP: {
		extern dev_t nx_dev;

		u.u_ttyp = &tp->t_pgrp;
		u.u_ttyd = makedev(nx_dev, minor(dev));
		tp->t_pgrp = u.u_procp->p_pgrp = u.u_procp->p_pid;
		xp->x_state |= X_PGRP;
		break;
		}

	/*
	 * ioctl write.
	 */
	case NXWRITE:  {
		struct xnsio io;

		if (copyin(arg, (caddr_t)&io, sizeof io)) {
			error = EFAULT;
			break;
		}
		u.u_base = io.addr;
		u.u_count = io.count;
		u.u_offset = 0;
		u.u_segflg = 0;
		ilbwrite(tp, io.dtype, io.control);
		if (copyout((caddr_t)&io, arg, sizeof io))
			error = EFAULT;
		break;
		}

	/*
	 * ioctl read.
	 */
	case NXREAD: {
		struct xnsio io;

		if (copyin(arg, (caddr_t)&io, sizeof io)) {
			error = EFAULT;
			break;
		}
		u.u_base = io.addr;
		u.u_count = io.count;
		u.u_offset = 0;
		u.u_segflg = 0;

		if (!(xp->x_state & X_CONNECTED) &&
		     (xp->x_state & (X_MULTI|X_UPROTO))) {
			if (xp->x_inpq.nbufs)
				ilbread(&xp->x_inpq, (char *)NULL,
						     (char *)NULL);
		} else {
			if (xns_read(xp->x_conp, &io.dtype,
						 &io.control))
				error = EIO;
		}
		io.count -= u.u_count;
		if (copyout((caddr_t)&io, arg, sizeof io))
			error = EFAULT;
		break;
		}

	case NXPUTSLOT: {
		struct physnet addr;

		if (copyin(arg, (caddr_t)&addr, sizeof addr))
			error = EFAULT;
		else
			nxputslot(d, &addr);
		break;
		}

	case NXGETSLOT: {
		struct physnet addr;

		nxgetslot(d, &addr);
		if (copyout((caddr_t)&addr, arg, sizeof addr))
			error = EFAULT;
		break;
		}

	case NXSETRCV: {
		if (xp->x_state & X_UPROTO) {
# ifdef NXERR_FIX
			error = EISCONN;
# else  NXERR_FIX
			error = ENXIO;
# endif NXERR_FIX
			break;
		}
		nxsetrcv(d, (int)arg);
		if (arg) {
			xp->x_maxinpq = nx_def_maxinpq;
			if ((int)arg>1)
				xp->x_state |= X_DISCARD;
			xp->x_state |= X_MULTI;
		} else
			xp->x_state &= ~X_MULTI;
		break;
		}

# ifdef NXAVOPEN
	/*
	 * find and open an available nx device.
	 * this mechanism "should" be the only way to
	 * open nx device files other than device 0.
	 * should be performed only on device 0.
	 * actually, in some cases it is necessary to
	 * treat the nx file as a (controlling) tty.
	 * in those cases it would make more sense to
	 * have the open do a chmod-chown like login.
	 */
	case NXAVOPEN: {
		register short n;

		/* set error now, clear later if ok */
		error = EBUSY;

		if (d != 0) break;	/* only allowed on device 0 */

		for (n = NDEV; --n > 0;)
		if (xnsmisc[n].x_owner == 0) {
			xnsmisc[n].x_owner = u.u_procp->p_pid;
			break;
		}

		if (n > 0) {
			nxiopen((unsigned)n, arg, 2);
			if (error = u.u_error)
				xnsmisc[n].x_owner = 0;
		}

		break;
		}
# endif NXAVOPEN

# ifdef NXUPROTO
	/*
	 * set up to receive all packets of given etype.
	 * allow at most one receiver per etype.
	 * don't allow interception of special etypes.
	 */
	case NXUPROTO: {
		short uetype;

		if (copyin(arg, (caddr_t)&uetype, sizeof uetype)) {
			error = EFAULT;
			break;
		}
		error = nxrcvtype(xp, (int)uetype);
		break;
		}
# endif NXUPROTO

	case NXWATCH: {
		if (!suser())
			return;
		if (xp->x_state & (X_MULTI|X_UPROTO|X_CONNECTED)) {
			error = EISCONN;
			break;
		}
		onwatch(xp);
		break;
		}

	  default: {
		error = -1;
		break;
		}
	}

	/*
	 * Some of the above ioctl's have xns parallels.
	 * Call the xns code to process the ioctl (ONLY
	 * if there was no error above and we have a conp).
	 */
	if (error <= 0 && xp->x_conp != NULL) {
		v = xns_ioctl(tp, xp->x_conp, cmd, arg);
		NXTRACE(("cmd=%x tp=%x conp=%x v=%d error=%d",
				 cmd, tp, xp->x_conp, v, error));
		if (v >= 0)
			error = v;
	}

	/* check for nobody supporting the ioctl, or an error from the ioctl */
	if (error < 0)
		error = ENXIO;

	NXTRACE(("cmd=%x tp=%x conp=%x error=%d", cmd, tp, xp->x_conp, error));
	u.u_error = error;
}

/*
 * nxgetstats:
 *	- query board for statistics (don't wait).
 *	  they may be copied to the user later.
 */
int
nxgetstats()
{
	register NBP nbp;
	register STATS mp;

	if ((nbp = getnbuf(0)) == NULL) {
		return -1;
	}
	if ((mp = (STATS)getmsgbuf()) == NULL) {
		freenbuf(nbp);
		return -1;
	}
	mp->request = NET_STATS;
	mp->reqmask = READREQ;		/* read stats */
	mp->nobj = sizeof (struct stats_reply)
		/ sizeof (long);/* read all objects */
	mp->index = 0;
	mp->baddr = nbp->nxpaddr;

	putmsg((NxMsg *)mp, nbp);
	return 0;
}



/*
 * nx_send:
 *	- take the given buffer, and place it on the nx interface's xmit
 *	  queue
 */
nx_send(nbp)
	register NBP nbp;
{
	register XMIT mp;

	nbp->btype |= B_RAWXMIT;
	NXTRACE(("nxsend: nbp=%x", nbp));
	if (nbp->len > 1024)
		iprintf("nx: (send) nbp=%x len=%d\n", nbp, nbp->len);
	if ((mp = (XMIT)getmsgbuf()) == NULL) {
		freenbuf(nbp);
		return;
	}
	mp->request = NET_XMIT;
	mp->nblocks = 1;
	mp->block[0].len = nbp->len;
	mp->block[0].addr = nbp->nxpaddr;
	mp->block[7].addr = NULL;

	putmsg((NxMsg *)mp, nbp);
}

/*
 * i/o start routine for protocol.
 * conp and pconp are the virtual and physical header addresses.
 * hsize is the header size.
 * nbp (which may be null) is the data.
 */
nx_ifstart(conp, pconp, hsize, nbp)
	CONN conp;
	caddr_t pconp;
	NBP nbp;
{
	register XMIT mp;
	register len;

	/*
	 * first obtain a device msgbuf.
	 */
	if ((mp = (XMIT) getmsgbuf()) == NULL)
		return 0;

	/*
	 * If there's a data segment, set up the second block
	 * of the device msgbuf.
	 */
	if (nbp) {
		if (nbp->len > 1024)
			iprintf("nx: (ifstart) nbp=%x len=%d\n", nbp, nbp->len);
		if (hsize+nbp->len < MINPACKET)
			len = MINPACKET-hsize;
		else
			len = nbp->len;
		/* insure ether packet is of even length */
		if (len & 1)
			len++;
		mp->block[1].len = len;
		mp->block[1].addr = nbp->nxpaddr;
		mp->nblocks = 2;
	} else {
		mp->nblocks = 1;
		mp->block[1].len = 0;
		hsize = MAX(hsize, MINPACKET);
		/*
		 * NEVER free this one!
		 * nxintr() recognizes this case by (conp != 0).
		 */
		nbp = NONBP;
	}
	mp->request = NET_XMIT;
	mp->block[0].len = hsize;
	mp->block[0].addr = (long)pconp;
	mp->block[7].addr = (long)conp;			/* stash */
	putmsg((NxMsg *)mp, nbp);
#ifdef	XTRACE
{
	register XSEQ xp;

	xp = &conp->header;
	NXTRACE(("devstart: conp=%x len[0]=%d len[1]=%d tcontrol=%x",
			    conp,  mp->block[0].len, mp->block[1].len,
			    xp->tcontrol));
}
#endif
	return 1;
}

# ifdef NXAVOPEN

# define NXDEVNAME	"/dev/ttyn"
# define NXDEVNAMELEN	9

/*
 * nxiopen() --
 * indirect open routine for nx device files.
 *
 * - build the name of the specified nx device.
 * - copy it back to the user's struct xnsio.
 * - use the system call open() routine on it.
 *
	nx_index		id (index) of nx device
	ioarg			pointer to user's struct xnsio arg
	open_flagarg		used by system call open() routine
 *
 */
int
nxiopen(nx_index, ioarg, open_flagarg)
	unsigned nx_index;
	caddr_t ioarg;
	int open_flagarg;
{
	struct a {
		char *path;
		int flag;
		int crmode;
	};
	register struct user *up;
	register struct a *ap;

	auto char devname[NXDEVNAMELEN+5];
	auto struct xnsio io;

	up = &u;

	if (copyin(ioarg, (char *)&io, sizeof io) < 0) {
		up->u_error = EFAULT;
		return;
	}

	if (io.count < sizeof devname) {
		up->u_error = EINVAL;
		return;
	}
	/*
	strncpy(devname, NXDEVNAME, sizeof devname);
	uitoa(nx_index, 10, 0, devname+NXDEVNAMELEN);
	 */
	sprintf(devname, "%s%u", NXDEVNAME, nx_index);
	if (copyout(devname, io.addr, sizeof devname)) {
		up->u_error = EFAULT;
		return;
	}

	ap = (struct a *)up->u_ap;
	ap->path = up->u_dirp = io.addr;
	ap->flag = open_flagarg;
	ap->crmode = 0;

	open();
}
# endif NXAVOPEN

offwatch()
{
	extern short nx_onnet;

	nx_watchmisc = 0;

	nx_onnet = 0;
	if (nxmode(NET_CONNECT) == 0)
		nx_onnet++;
	if (!nx_onnet)
		u.u_error = ENXIO;
}

onwatch(xp)
	struct xnsmisc *xp;
{
	extern short nx_onnet;

	if (nx_watchmisc != 0) {
		u.u_error = EADDRINUSE;
		return;
	}
	nx_watchmisc = xp;
	nx_watchmisc->x_maxinpq = NETBUFS - 8;

	nx_onnet = 0;
	if (nxmode(NET_PROMISCUOUS) == 0)
		nx_onnet++;
	if (!nx_onnet)
		u.u_error = ENXIO;
}

/*
 * watch_rcv() --
 * put a buf on the watcher's queue.
 * don't let the watch queue hog all the bufs.
 */
struct physnet bcastphysaddr = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
NBP
watch_rcv(nbp, ok)
	register NBP nbp;
	int ok;
{
	extern NBP nbclone();

	register NBP nnbp;
	char flunk;

	flunk = nx_watchmisc->x_inpq.nbufs >= nx_watchmisc->x_maxinpq;
	nnbp = NULL;
	if (!flunk && ok) {
		register etheader *ep;

		ep = nxmtod(nbp, etheader *);
		if (EQPHYSNET(&ep->dst, &bcastphysaddr)
		 || EQPHYSNET(&ep->dst, &myaddr))
			if ((nnbp = nbclone(nbp)) == NULL)
				flunk++;
	}
		
	if (flunk) {
		freenbuf(nbp);
		watch_dropped++;
		return NULL;
	}

	nxmulti(nbp, nx_watchmisc);
	return nnbp;
}

# ifdef IFXNS
/*
 * nxns_recv() --
 * handle an incoming packet from the ex driver.
 * the mbuf chain is either:
 *	1 mbuf containing a ptr followed by data;
 * 	2 mbufs, one containing a ptr, the other containing data.
 */
nxns_recv(rmp, m)
	register READ rmp;
	register struct mbuf *m;
{
	register NBP nbp;

	nxintflag++;
	if ((nbp = getnbuf(1)) != 0) {
		MBUFIZE(nbp, m);
		nx_qpacket(rmp, nbp);
	}
	nxintflag--;
}

/*
 * nxns_watchrecv() --
 * put a packet on the watch queue, recording its reply
 * in the netbuf control field.  if m is non-0, it is a
 * pointer to an mbuf chain which being given exclusively
 * to nxns.  if d is non-0, it is a pointer to data which
 * must be copied.  exactly one of these conditions must
 * always hold.
 */
int
nxns_watchrecv(rmp, m, d)
	register READ rmp;
	register struct mbuf *m;
	char *d;
{
	register NBP nbp;

	nxintflag++;
	if ((nbp = getnbuf(0)) == 0) {
		watch_dropped++;
		nxintflag--;
		return -1;
	}

	nbp->len = rmp->block[0].len - NX_TRAILER_SIZE;
	if (nbp->len > MAXPACKET)
		nbp->len = MAXPACKET;
	nbp->control = rmp->reply;
	if (m != 0) {
		MBUFIZE(nbp, m);
	}
	else {
		nbp->data = d;
		bcopy(nbp->data, nbp->info, nbp->len);
		UNMBUFIZE(nbp);
	}

	nxmulti(nbp, nx_watchmisc);
	nxintflag--;
	return 0;
}
# endif IFXNS

NBP
nbclone(nbp)
	register NBP nbp;
{
	register NBP nnbp;

	METER(nx_cloned++);
	if ((nnbp = getnbuf(0)) == NULL)
		return 0;

	nnbp->btype = nbp->btype;
	bcopy(nxmtod(nbp, caddr_t), nxmtod(nnbp, caddr_t), nbp->len);
	nnbp->len = nbp->len;
	nnbp->control = nbp->control;
	nnbp->dtype = nbp->dtype;
	nnbp->seq = nbp->seq;
	nnbp->quack = nbp->quack;
	return nnbp;
}

# ifdef BLOCKFIONREAD
/*
 * nxccount() --
 * return the number of characters in an input queue.
 * if at EOF, return -1.
 */
off_t
nxccount(qp)
	register NBQ qp;
{
	register int s;
	register off_t ccount;
	register NBP nbp;

	ccount = 0;
	LOCK;
	for (nbp = qp->head; nbp != NULL; nbp = nbp->next)
		ccount += nbp->len;
	if (ccount == 0 && qp->nbufs < 0)
		ccount--;
	UNLOCK;
	return ccount;
}
# endif BLOCKFIONREAD

int
nxrcvtype(xp, uetype)
	register struct xnsmisc *xp;
	int uetype;
{
	register int n;

	if (xp->x_state & (X_MULTI|X_UPROTO|X_CONNECTED))
		return EISCONN;

	if (uetype == (short)SG_BOUNCE || uetype == (short)IDETHERTYPE)
		return EADDRINUSE;

	for (n = NDEV; --n >= 0;)
		if (xnsmisc[n].x_state & X_UPROTO
		 && xnsmisc[n].x_uetype == uetype)
			return EADDRINUSE;

	xp->x_maxinpq = nx_def_maxinpq;
	xp->x_uetype = uetype;
	xp->x_state |= X_UPROTO;
	return 0;
}
