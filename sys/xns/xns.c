# define NBBCAT_FIX
# define SYSTEMV
# undef DROPTEST
/*
 * xns protocol code
 */

#include "../h/param.h"
#include "../h/types.h"
#include "../h/buf.h"
#include "../h/systm.h"
#include "../h/signal.h"
#include "../h/dir.h"
#include "../h/map.h"
#include "../h/user.h"
#include "../h/errno.h"
#include "../h/conf.h"
#include "../h/tty.h"
#include "../h/termio.h"
#include "../h/inode.h"
#include "../h/file.h"
#include "../h/ioctl.h"
#include "../xns/if_xns.h"


/* XXX move this stuff to a header file */
extern	struct tty *xns_findtty();
extern	NBP qfirst();

CONN xns_connbase;		/* base of the array of struct conn's */
Xhost xns_myaddr;		/* my physical (etheader) address */
Xhost xns_bcastaddr = { 0xFFFF, 0xFFFF, 0xFFFF };


/* various optional bugfixes */
short xns_seqfix = 0;		/* optionally fix seq bug */
short xns_tenforce = 0;		/* optionally enforce xns type */
short xns_wenforce = 0;		/* optionally enforce SPP window */
short xns_tjunk = 0;		/* optionally do bad timer stuff */


/*
 * xns initialization:
 * 	copy local physical and logical addresses into conn structs;
 *	link conn structs together for fast scanning;
 *	link buffers;
 *	link route table entries;
 *	assign local socket numbers;
 *	construct "random" starting number for connection id's;
 *	start timer process;
 *	initialize the bounce prototype message.
 */
xns_initialize(va, mbva, na)
	long va, mbva;
	struct physnet *na;
{
	register CONN conp, lastp, prevp, pconp;
	register struct xns_route *r1, *r2, *rlast;
	register socket, x;

	xns_myaddr = *(Xhost *)na;
	myinternet.host = xns_myaddr;

	xns_connbase = (CONN)va;

	/* set up connection structs */
	socket = SOCKBASE;
	conp = xns_connbase;
	lastp = conp + NDEV;
	pconp = (CONN)mbva;
	for(prevp=conp; conp<lastp; prevp=conp) {
		conp++;
		prevp->next = (conp==lastp) ? NULL : conp;

		prevp->paddr = (caddr_t)&pconp->header;
		prevp->header.isrc = myinternet;
		prevp->header.esrc = xns_myaddr;
		prevp->header.isrc.socket = socket++;
		prevp->header.checksum = NOCHECKSUM;
		prevp->header.idtype = SEQTYPE;
		prevp->state = S_FREE;
		pconp++;
	}

	/* thread route table entries */
	r1 = xns_rtab;
	rlast = r1 + RSIZE;
	for(r2=r1; r1<rlast; r2=r1) {
		r1++;
		r2->next = (r1==rlast) ? NULL : r1;
	}

	/* set up bounce message prototype */
	bounceproto.et.dst = xns_bcastaddr;
	bounceproto.et.src = xns_myaddr;
	bounceproto.et.etype = htons(SG_BOUNCE);
	bounceproto.cmd = SERV_HOSTNAME;
	bounceproto.colon = ':';
}

xns_go()
{
	if (xns_ready)
		return;

	/* get a "random" number for initial id */
	if ((idnumber=SYSTIME/127)==0)
		idnumber = 127;

	/* start timer */
	xns_timer();

	/* tunable parameters */
	xns_rqueuemax = RQUEUEMAX;
	xns_squeuemax = SQUEUEMAX;
	xns_rwindow = RWINDOW;

	/* ready to go */
	xns_ready++;
}

/*
 * All incoming xns packets examined here.
 * Only ECHO and SPP protocols are supported.
 */
xns_rcv(nbp)
	register NBP nbp;
{
	register XSEQ hp, remp;
	register CONN conp;
	register char *data;
	register int len, control;
	register Xsocket socket;

	if (!xns_ready)
		goto out;

	/*
	 * Find the XNS header.  Coincidentally, the initial part
	 * of most XNS headers is identical to that of SPP.
	 * Convert network-order shorts to host form (nop on a 680X0).
	 */
	remp = nxmtod(nbp, XSEQ);

	NTOHS(remp->length);
	NTOHS(remp->seqno);
	NTOHS(remp->ackno);
	NTOHS(remp->allocno);
	NTOHS(remp->idst.socket);

	control = remp->tcontrol;
	socket = remp->idst.socket;

	/*
	 * Check that this packet is addressed to Us,
	 * and is of a supported idtype.
	 */
	if (!EQNETHOST(remp->idst, myinternet))
		goto out;

	if (remp->idtype == ECHOTYPE) {
		xns_echo(nbp);
		goto out;
	}

	if (xns_tenforce)
	if (remp->idtype != SEQTYPE)
		goto out;

	/*
	 * Give packet to xns_login() if it looks like a connection
	 * setup request.
	 */
	if (control&SYSTEMPACKET && 0 < socket && socket < SOCKBASE) {
		(void)xns_login(remp);
		goto out;
	}

	/*
	 * Determine and check local socket number (actually an index
	 * into our array of struct conn's).  Our socket numbers begin
	 * at SOCKBASE to avoid "well-known" socket numbers.
	 */
	socket -= SOCKBASE;
	if (socket >= NDEV)
		goto out;

	conp = &xns_connbase[socket];
	hp = &conp->header;

	/*
	 * Some tracing code to show incoming packets.
	 */
	NXTRACE((" {{rcv conp=$%x state=%d id=<%d>%d",
			conp, conp->state, hp->srcid, hp->dstid));
	NXTRACE(("  rack=%d alc=%d",
			conp->rackno, conp->allocno));
	NXTRACE((" [nbp control=$%x id=<%d>%d",
			control, remp->srcid, remp->dstid));
	NXTRACE(("  seq=%d ack=%d alc=%d dtype=$%x]",
			remp->dtype,
			remp->seqno, remp->ackno, remp->allocno));

	/*
	 * Check addressing in header (the destination host and net
	 * numbers were checked above):
	 *	Ignore anything that is not correctly addressed.
	 *	Callouts are handled as a special case since the
	 *	 remote connection id and socket number aren't yet known.
	 */
	if (!(remp->dstid == hp->srcid
	 && remp->idst.socket == hp->isrc.socket))
		goto out;

	if (!EQNETHOST(remp->isrc, hp->idst))
		goto out;

	if (!(remp->srcid == hp->dstid)) {

		if (!(conp->state == S_CALLOUT && conp->ntries))
			goto out;

		NXTRACE((" CALLOUT reply id=%d socket=%d",
				remp->srcid, remp->isrc.socket));
		conp->state = S_ACTIVE;
		conp->ntries = 0;
		hp->dstid = remp->srcid;
		hp->idst.socket = remp->isrc.socket;
		conp->mode = 1;
		conp->dir = 1;
		if (conp->utp)
			xns_enatty(conp->utp);
		wakeup((caddr_t)conp);
	}

	if (!(remp->isrc.socket == hp->idst.socket))
		goto out;

	/*
	 * Check sequence number.
	 *	Update last-receive time on the connection.
	 *	Respond to SENDACK (even if sequence number is wrong).
	 *	Optionally enforce allocation window.
	 */
	conp->rtime = the_time;
	if (remp->seqno != conp->nextrseq) {
		NXTRACE((" BADSEQ %d want %d", remp->seqno, conp->nextrseq));
		goto reply;
	}
	if (remp->seqno == conp->allocno) {
		NXTRACE((" BADALC alc=seq=%d", remp->seqno));
		if (xns_wenforce)
		goto reply;
	}

	/*
	 * We've decided this is a valid packet.
	 *	Wakeup sleeper blocked on allocation.
	 *	Free up output buffers on acknowledgement.
	 *	Update received time.
	 */
	if (conp->ralloc != remp->allocno) {
		NXTRACE((" advance flow ralc=%d was %d",
				remp->allocno, conp->ralloc));
		conp->ralloc = remp->allocno;
		if (conp->blocked) {
			wakeup((caddr_t)&conp->blocked);
			conp->blocked = 0;
		} else {
			/*
			 * call unix tty interface to start output.
			 * clear tty BUSY bit (SYSTEMV) only.
			 * (XXX may add (if conp->mode==0) also).
			 */
			if (conp->utp && conp->state==S_ACTIVE) {
#ifndef	SYSTEMV
				ilttstart(conp->utp);
#else	SYSTEMV
				conp->utp->t_state &= ~BUSY;
				nxstart(conp->utp);		/* BLECH */
#endif	SYSTEMV
			}
		}
	}

	/*
	 * Since the output may block several times per allocation window
	 * (if XNS_SQMAX < window size), we have to unblock the output as
	 * output packets are ack'd.
	 */
	if (conp->blocked && conp->nextseq != conp->ralloc) {
		wakeup((caddr_t)&conp->blocked);
		conp->blocked = 0;
	}

	/*
	 * Call xsync if it looks like:
	 *	1) the ack number is different, or
	 *	2) a retransmission should be started.
	 */
	if (conp->rackno != remp->ackno || remp->ackno != conp->nextseq) {
		if (conp->state == S_ACTIVE || conp->state == S_CLOSING)
			xns_xsync(remp, conp);
	}

	/*
	 * Process system packet, or consume sequence number.
	 * Deal with end-of-connection packets.
	 */
	if (control&SYSTEMPACKET) {
		if (remp->dtype == DST_END) {
			NXTRACE((" rcv sys END"));
			(void) xns_xack(conp, DST_ENDREPLY);
			if (conp->state == S_ACTIVE)
				xns_eof(conp);
		}
		else
		if (remp->dtype == DST_ENDREPLY) {
			NXTRACE((" rcv sys ENDREPLY inpq=%x", conp->inpq.head));
			if (conp->inpq.head == 0)
				conp->state = S_CLEANUP;
			else
				conp->state = S_FLUSH;
		} 
		goto reply;
	}

	if (remp->dtype == DST_END) {
		xns_eof(conp);
		conp->state = S_ACTIVE;
		NXTRACE((" rcv data END"));
	}

	/*
	 * If the connection is not active, then throw away
	 * incoming data packets.
	 */
	if (conp->state != S_ACTIVE)
		goto out;

	/*
	 * Process data packet by appending to inpq (block data mode),
	 * or stuffing into typewrite queue (xns_ttyinput).  Improve flow by
	 * causing an ACK if there's no output queued (conp->trans).
	 * Increment allocation number only if we succeed in storing
	 * received data in queue.
	 */
	len = remp->length - sizeof (Xidheader) - sizeof (Xseqheader);
	data = nxmtod(nbp, caddr_t) + sizeof (Xseq);
	if (conp->mode
	 || !(remp->dtype == DST_DATA || remp->dtype == DST_OLDDATA)) {
# ifdef NBBCAT_FIX
		nbp->dtype = remp->dtype;
		nbp->control = remp->control;
		nbp->data = data;
		nbp->len = len;
		if (nbbcat(nbp, &conp->inpq)) {
			wakeup((caddr_t)&conp->inpq);
			conp->rseq = conp->nextrseq++;
			nbp = 0;/* note freenbuf(0) is harmless */
			goto reply;
		}
# else  NBBCAT_FIX
		if (nbcat(data, len, &conp->inpq,
				remp->dtype, remp->control)) {
			wakeup((caddr_t)&conp->inpq);
			conp->rseq = conp->nextrseq++;
			goto reply;
		}
# endif NBBCAT_FIX
		goto out;
	} 

	/*
	 * Bump flow numbers before calling unix if we're an interactive
	 * connection because the unix tty mess will call output routines.
	 * It is possible to get input truncation with this arrangement.
	 */
	conp->allocno++;
	conp->rseq = conp->nextrseq++;
	(void)xns_ttyinput(data, (unsigned)len, conp);
	if (conp->trans == 0 && !conp->hbusy)
		control |= SENDACK;

reply:
	if (control&SENDACK && conp->state == S_ACTIVE) {
		if (!xns_xack(conp, DST_SYSTEM))
			conp->asend++;
	}

out:
	if (nbp != 0)
		freenbuf(nbp);

	NXTRACE((" rcv}}"));
}

/*
 * protocol block read routine, called from device driver.
 */
#ifndef	SYSTEMV
xns_read(conp, uio, dtype, control)
#else
xns_read(conp, dtype, control)
#endif	SYSTEMV
register CONN conp;
char *dtype, *control;
{
	register x, error;

	x = (int)conp;
	if (x>=0 && x<SOCKBASE) 
		return(-1);			/* XXX clean this up */
#ifndef	SYSTEMV
	error = ilbread(&conp->inpq, uio, dtype, control);
#else
	ilbread(&conp->inpq, dtype, control);
	error = u.u_error;
#endif	SYSTEMV

	x = conp->inpq.nbufs;


	NXTRACE(("xns_read conp=%x allocno=%d +x=%d rseq=%d nbufs+%d",
			   conp, conp->allocno, x, conp->rseq,
			   conp->inpq.nbufs));
	if (x>xns_rqueuemax || x<0)
		return error;

	conp->allocno = conp->nextrseq + xns_rwindow;

	if (xns_xack(conp, DST_SYSTEM)==0) {
		conp->asend++;
	}
	return error;
}

/*
 * Send a dtype system packet on a connection.
 * used to send a new ackno.
 */
xns_xack(conp, dtype)
	register CONN conp;
	int dtype;
{
	register XSEQ xp;
	register int s;

	LOCK;
	/*
	 * We can't send an ack if either something is being sent right
	 * now, or the header is busy.
	 */
	if (conp->trans || !AVAIL(&conp->hbusy)) {
		NXTRACE(("xack: conp=%x trans=%x hbusy=%d nbufs=%d",
				conp, conp->trans, conp->hbusy,
				conp->outq.nbufs));
		UNLOCK;
		return 0;
	}

	NXTRACE(("xack: conp=%x hbusy=%d", conp, conp->hbusy));
	xp = &conp->header;
	xp->tcontrol = SYSTEMPACKET;
	xp->allocno = conp->allocno;
	xp->seqno = conp->nextseq;
	xp->ackno = conp->nextrseq;
	xp->length = sizeof (Xseq) - 14;
	xp->dtype = dtype;
	if (dtype & XACK_SENDACK)
		xp->tcontrol |= SENDACK;

	NXTRACE((" xack: seq=%d ack=%d alloc=%d lseq=%d dtype=%d",
			xp->seqno, xp->ackno, xp->allocno,
			conp->lastseq, dtype));

	if (nx_ifstart(conp, conp->paddr, sizeof (Xseq), (NBP)NULL)) {
		NXTRACE((" xack: conp=%x hbusy=%d", conp, conp->hbusy));
		UNLOCK;
		return 1;
	}

	/* transmit failed */
	conp->hbusy = 0;
	NXTRACE((" xack: oops, conp=%x hbusy=%d", conp, conp->hbusy));
	UNLOCK;
	return 0;
}

/*
 * set up xns connection structure
 */
xns_initconn(tp, conp, xp, id)
	register struct tty *tp;
	register CONN conp;
	register XSEQ xp;
	unsigned short id;
{
	register XSEQ hp;
	register short m;

	m = XNSTTYINDEX(tp);
	xns_conntty(tp, conp);

	conp->nextseq = 0;
	conp->lastseq = 0xffff;
	if (xns_seqfix)
		conp->lastseq = conp->nextseq;
	conp->ralloc = 0xffff;
	conp->rackno = 0;
	conp->rseq = 0xffff;
	conp->nextrseq = 0;
	conp->rtime = the_time;
	if (xnsmisc[m].x_state & X_BLOCK)		/* XXX */
		conp->mode |= 1;

	hp = &conp->header;
	hp->srcid = id;
	hp->etype = ntohs(IDETHERTYPE);
	hp->tcontrol = SYSTEMPACKET;
	hp->allocno = conp->allocno = 1;
	hp->seqno = hp->ackno = 0;
	hp->dtype = DST_DATA;
	hp->length = sizeof (Xseq) - 14;
	if (xp) {
		bcopy((caddr_t)&xp->isrc, (caddr_t)&hp->idst,
		      sizeof(Xaddr));
		bcopy((caddr_t)&xp->esrc, (caddr_t)&hp->edst,
		      sizeof(Xhost));
		hp->dstid = xp->srcid;
	}
}

/*
 * Make an SPP connection from local tty to destination 'x'.
 */
xns_connect(tp, x)
	struct tty *tp;
	register SETUP x;
{
	register CONN conp;
	register XSEQ hp;
	register ROUTE r;
	register tryagain = 0;
	register s;

	/*
 	 * First find a free connection struct.
	 * Return -1 on failure.
	 */
	for(conp=xns_connbase; conp; conp=conp->next) {
		hp = &conp->header;
		if (conp->state!=S_FREE)
			continue;
		if (hp->srcid==0 && conp->utp==NULL) {
			LOCK;
			hp->srcid = idnumber++;
			if (idnumber==0)
				idnumber++;
			conp->state = S_CALLOUT;
			conp->ntries = 0;
			UNLOCK;
			break;
		}
	}
	NXTRACE(("CONNECT conp=%x utp=%x", conp, conp->utp));
	if (conp==NULL) {
		u.u_error = ENOBUFS;
		return -1;
	}

	/*
	 * Fill in goodies in the SPP header.
	 */
	HTONS(hp->idst.socket);
	hp->dstid = 0;

	/*
	 * Find network address to use.
	 * Return -1 on failure and release the connection.
	 */
loop:
	xns_initconn(tp, conp, (XSEQ)NULL, (unsigned short)hp->srcid);
	if ((r=xns_getroute(conp, x))==0) {
		hp->srcid = 0;
		conp->utp = NULL;
		conp->state = S_FREE;
		NXTRACE(("CONNECT getroute failed"));
		u.u_error = EHOSTUNREACH;
		return -1;
	}

	/*
	 * Now we're ready to try the connection.
	 * Send first system packet to remote host.
	 */
	conp->ntries = 1;
	if (xns_xack(conp, DST_SYSTEM | XACK_SENDACK)) {
		NXTRACE(("CON REQ SENT conp=%x utp=%x id=%d",
			      conp, conp->utp, hp->srcid));
	}

	/*
	 * Sleep on the connection.  It is in S_CALLOUT state.
	 * Connection is established when xns_rcv changes conp->state
	 * to S_ACTIVE and fills in dstid.
	 * The xns_timer() routine is responsible for retrying
	 * the connection attempt.  The state goes to FAILCALL
	 * if the timer gives up.
	 */
	LOCK;
	while (conp->state != S_FAILCALL && hp->dstid==0)
		sleep((caddr_t)conp, TTOPRI);
	NXTRACE(("CONN WAKEUP conp=%x state=%d dstid=%d",
		       conp, conp->state, hp->dstid));
	UNLOCK;
	/*
	 * Connection failure: Try again if we used a cache entry.
	 * This handles the case where a network address has changed
	 * (perhaps by changing interface boards) and we have an
	 * incorrect address in the cache.
	 */
	if (conp->state == S_FAILCALL && tryagain==0) {
		tryagain++;
		xns_rtabflush(r);
		if (r->hit) {
			NXTRACE(("tryagain"));
			goto loop;
		}
	}
	/*
	 * success. free up cache entry.  mark time for LRU.
	 */
	if (hp->dstid && conp->state == S_ACTIVE) {
		r->age = SYSTIME;
		r->lock = 0;
/* XXX is this needed? */
		return 0;
	}
	/*
	 * Connection setup fails here.
	 * Put the connection in cleanup state.
	 * If we used a name cache entry (returned by getroute)
	 * zap the entry so it won't be used again.
	 */
	NXTRACE(("nobody answered: conp=%x r=%x", conp, r));
	conp->state = S_CLEANUP;
	xns_rtabflush(r);
	u.u_error = ECONNREFUSED;
	return -1;
}

#ifdef	XTRACE
long xns_n_hbusy_cleanup;
#endif	XTRACE

/*
 * xns_cleanup:
 *	- clean up a connection, freeing up any used resources
 */
xns_cleanup(conp)
register CONN conp;
{
	register int s;

	/*
	 * This should never happen.  Find out why it does.
	 */
#ifdef	XTRACE
	if (conp->hbusy) {
		NXTRACE(("hbusy in cleanup conp=%x", conp));
		xns_n_hbusy_cleanup++;
	}
#endif

	LOCK;
	NXTRACE(("xns_cleanup conp=%x", conp));
	xns_close(conp->utp);
	conp->trans = NULL;
	conp->header.srcid = 0;
	conp->header.dstid = 0;
	conp->header.idst.socket = 0;
	conp->header.idst.host.low = 0;
	conp->header.idst.host.mid = 0;
	conp->header.idst.host.high = 0;
	conp->blocked = 0;
	conp->state = S_FREE;
	conp->mode = 0;
	conp->dir = 0;
	conp->ntries = 0;
	conp->hbusy = 0;				/* XXX */
	conp->asend = 0;
 
	xns_qflush(&conp->pending);
	xns_qflush(&conp->inpq);
	xns_qflush(&conp->outq);

	UNLOCK;
}

/*
 * Output done routine
 */
xns_xdone(conp)
register CONN conp;
{
	register NBP t, nbp;
	register XSEQ xp;
	register s;

	/*
	 * Free buffers that have been released by xns_xsync().
	 */
	LOCK;
	while ((t = conp->release) != NULL) {
		NXTRACE(("xdone: release seq=%d nbp=%x conp=%x",
				 t->seq, t, conp));
		conp->release = t->next;
		t->next = NULL;
		freenbuf(t);
#ifndef	SYSTEMV
		xnsbufstats[Release]++;
#endif	SYSTEMV
	}
	UNLOCK;

	xp = &conp->header;
	NXTRACE(("xdone: conp=%x seq=%d ack=%d alloc=%d intflg=%d",
			 conp, xp->seqno, xp->ackno, xp->allocno, nxintflag));
	if (conp->state == S_CLEANUP) {
		conp->trans = NULL;
		goto out;
	}

	/*
	 * Start up next output transmission.
	 */
	if ((nbp = conp->trans) != NULL) {
		register NBP tnbp;

		xp->seqno = nbp->seq;
		xp->allocno = conp->allocno;
		xp->ackno = conp->nextrseq;
		if (nbp->dtype==0)
			nbp->dtype = DST_DATA;
		xp->dtype = nbp->dtype;
		conp->asend = 0;
		/*
		 * Turn on SENDACK bit if the buffer is marked.
		 */
		xp->tcontrol = nbp->control;
		if (nbp->quack)
			xp->tcontrol |= SENDACK;
		xp->length = (sizeof (Xseq)-14)+nbp->len;
		tnbp = (nbp->len==0) ? NULL : nbp;
		NXTRACE((" xdone: left conp=%x hbusy=%d", conp, conp->hbusy));
		if (nx_ifstart(conp, conp->paddr, sizeof(Xseq), tnbp)) {
			conp->trans = nbp->next;
			conp->ttime = the_time;
			return;
		}
	} else {
		/*
		 * Nothing to send.  See if a sendack was previously requested
		 * that we were unable to send.
		 */
		if (conp->asend) {
			conp->asend = 0;
			conp->hbusy = 0;
			(void) xns_xack(conp, DST_SYSTEM);
			return;
		}
	}
out:
	conp->hbusy = 0;
	NXTRACE(("xdone: conp=%x hbusy=%d", conp, conp->hbusy));
}

/*
 * Free up output buffers according to the acknowledge
 * number (xp->ackno) just received.
 * If the output side is quiescent (conp->hbusy==0) the buffers
 * can be freed here.  Otherwise they are put on the release list
 * (conp->release) and the output side actually does the freeing
 * in xns_xdone().  After the outq has been xsync'd, retransmission
 * is indicated if there are outstanding buffers.
 */
xns_xsync(xp, conp)
	XSEQ xp;
	register CONN conp;
{
	register NBP nbp;
	register short quackflag = 0;

	NXTRACE(("xsync: ackno=%d lastseq=%d conp=%x",
			 xp->ackno, conp->lastseq, conp));
#ifndef	SYSTEMV
	if (conp->hbusy==0 && conp->trans==NULL) {
#else
	if (conp->hbusy == 0) {
#endif	SYSTEMV
		while ((nbp=conp->outq.head)!=NULL) {
			if (nbp->seq==xp->ackno)
				break;
#ifdef  SYSTEMV
			if (nbp==conp->trans)
				conp->trans = nbp->next;
#endif	SYSTEMV
			NXTRACE((" free seq#=%d nbp=%x conp=%x",
				       nbp->seq, nbp, conp));
			if (nbp->quack)
				quackflag++;
			qfreenbuf(&conp->outq);
#ifndef	SYSTEMV
			xnsbufstats[Free]++;
#endif	SYSTEMV
		}
	} else {
		while ((nbp=conp->outq.head)!=NULL) {
			if (nbp->seq==xp->ackno)
				break;
			if (nbp==conp->trans)
				conp->trans = nbp->next;
			NXTRACE((" rel seq#=%d nbp=%x conp=%x",
				      nbp->seq, nbp, conp));
			if (nbp->quack)
				quackflag++;
			conp->outq.head = nbp->next;
			conp->outq.nbufs--;
			nbp->next = conp->release;
			conp->release = nbp;
		}
	}
	/*
	 * Save the received ackno.
	 * Test for retransmission only if
	 *	the received packet was not a polling packet,
	 *	we didn't just release a buffer that had SENDACK
	 *		turned on (quackflag).
	 */
	conp->rackno = xp->ackno;
	nbp = conp->outq.head;
	NXTRACE((" xsync nbp=%x quackflag=%d ttime=%d thetime=%d tc=%d",
		   nbp, quackflag, conp->ttime, the_time,
		   xp->tcontrol&SENDACK));
	if (nbp /*&& (xp->tcontrol & SENDACK)==0 */ && quackflag==0) {
#ifdef notdef
/* old */	if (xp->ackno!=conp->lastseq && conp->ttime!=the_time)
#endif
/* new */	if (xp->ackno!=conp->nextseq && conp->ttime!=the_time)
		{
	NXTRACE((" sequence off: ackno=%d lastseq=%d nbufs=%d hbusy=%d conp=%x",
		   xp->ackno, conp->lastseq, conp->outq.nbufs,
		   conp->hbusy, conp));
			if (conp->trans)
				return;
			conp->trans = nbp;
#ifndef	SYSTEMV
			xnsbufstats[AHA]++;
#endif	SYSTEMV
			if (conp->hbusy)
				return;
			conp->hbusy++;
			xns_xdone(conp);
		}
	}
	if (conp->state==S_CLOSING||conp->state==S_FLUSH)
		xns_doend(conp);
	NXTRACE((" xsync DONE"));
}

/*
 * xns_start:
 *	- start output on a given connection
 *	- add nbp, if non null, to the output queue
 *	- this is called at non-interrupt time only
 */
xns_start(conp, nbp)
	register CONN conp;
	register NBP nbp;
{
	register int s;

#ifdef XTRACE
	if (conp) 
		NXTRACE(("xns_start conp=%x nbp=%x len=%d state=%d",
				    conp, nbp, nbp->len, conp->state));
	else
		NXTRACE(("xns_start no conp=%x nbp=%x", conp, nbp));
#endif

	/*
	 * Don't do anything if we get on an inactive connection.
	 */
	if (conp==NULL || (conp->state != S_ACTIVE 
			&& conp->state != S_CLOSING)) {
		freenbuf(nbp);
		goto alldone;
	}

	/*
	 * Block on flow control.
	 *	Conditions for stopping are
	 *		1) used last sequence number in window
	 *		2) keeping more than squeuemax net buffers busy.
	 *
	 * The sleep happens on buffers that have data copied into them
	 * from the user but do not yet have a sequence number assigned.
	 * This way we are ready to start device output as soon as the
	 * blocking conditions are removed.  Because more than one
	 * process can be writing (and sleeping), pending output buffers
	 * go onto the pending queue and are taken off in fifo fashion
	 * via qfirst().
	 */
	LOCK;
	while (conp->lastseq==conp->ralloc || conp->outq.nbufs>xns_squeuemax) {
		if (nbp) {
			nbappend(nbp, &conp->pending);
			nbp = NULL;
		}
		NXTRACE((" BLK on lastseq=%d nbufs=%d pending=%d",
			      conp->lastseq, conp->outq.nbufs,
			      conp->pending.nbufs));
		conp->blocked++;
		sleep((caddr_t)&conp->blocked, TTOPRI);

		/*
		 * Handle the case where state changed while sleeping.
		 */
		if (conp->state!=S_ACTIVE && conp->state!=S_CLOSING) {
			goto out;
		}

		if (conp->pending.head == NULL)
			goto out;
#ifdef	XTRACE
		if (conp->blocked == 0)
			NXTRACE((" UNBLK on seq=%d", conp->lastseq));
#endif
	}
	if (nbp==NULL)
		nbp = qfirst(&conp->pending);
	if (nbp==NULL)
		goto out;
	/*
	 * The buffer can now be placed on the output queue.
	 * It consumes a sequence number, and the quack flag is set
	 * if we want the remote host to ack the packet.
	 * This happens if we will otherwise block on the subsequent packet.
	 */
	if ((nbp->control&SYSTEMPACKET)==0) {
		conp->lastseq = nbp->seq = conp->nextseq++;
		if (xns_seqfix)
			conp->lastseq = conp->nextseq;
		nbp->quack = (conp->lastseq==conp->ralloc) ||
			     (conp->outq.nbufs == xns_squeuemax);
	}

	nbappend(nbp, &conp->outq);
	NXTRACE((" xstart new conp=%x seq=%d nbp=%x quack=%d len=%d",
		   conp, nbp->seq, nbp, 
		   nbp->quack, nbp->len));
	if (conp->trans==NULL) {
		conp->trans = nbp;
		NXTRACE((" new trans seq=%d nbp=%x", nbp->seq, nbp));
		if (AVAIL(&conp->hbusy)) {
			NXTRACE(("xstart: conp=%x hbusy=%d",
					  conp, conp->hbusy));
			xns_xdone(conp);
		}
	}
out:
	UNLOCK;
alldone:
	NXTRACE((" xns_start DONE"));
}

xns_timer()
{
	register CONN conp;
	register unsigned xtime;
	register ROUTE r, t;
	register int s;

	xtime = ++the_time;
	nxintflag++;
	nx_timer();
#ifdef	SYSTEMV
	if (mwaiting) {
		wakeup((caddr_t)&mwaiting);
		mwaiting = 0;
	}
#endif	SYSTEMV
	for(conp=xns_connbase; conp; conp=conp->next) {
		/*
		 * Ignore connections that don't exist
		 */
		if (conp->state == S_FREE)
			continue;
		/* 
		 * Count outgoing connection attempts.
		 * Resend connect message until we get tired (4 tries).
		 */
		if (conp->state==S_CALLOUT) {
	NXTRACE(("time CALLOUT ntries=%d hbusy=%d trans=%d",
		       conp->ntries, conp->hbusy, conp->trans));
			if (conp->ntries == 0)
				continue;

			if (conp->ntries < 5) {
				if (xns_xack(conp, DST_SYSTEM | XACK_SENDACK)) {
					NXTRACE(("REQ SENT ntries=%d",
						      conp->ntries));
					conp->ntries++;
				}
				continue;
			}
			LOCK;
			conp->state = S_FAILCALL;
			conp->ntries = 0;
			UNLOCK;
			wakeup((caddr_t)conp);
			continue;
		}
		if (conp->state==S_FAILCALL) {
			conp->state = S_CLEANUP;
			continue;
		}

		/*
		 * Ignore idle connections.
		 */
		if (xns_tjunk)
		if (conp->header.srcid==0 || conp->header.idst.socket==0)
			continue;
		/*
		 * Shutdown progresses from CLOSING, to FLUSH, to CLEANUP.
		 *	CLOSING: try to send outstanding buffers.
		 *	FLUSH: try END/REPLY handshake with other side.
		 *	CLEANUP: free connection and any buffers.
		 */
		if (conp->state == S_CLOSING) {
			NXTRACE(("timer CLOSING conp=%x ntries=%d ",
					conp, conp->ntries));
			xns_doend(conp);
			continue;
		}
		if (conp->state==S_FLUSH) {
			NXTRACE(("timer FLUSH conp=%x tries=%d",
					conp, conp->ntries));
			xns_doend(conp);
			continue;
		}
		if (conp->state == S_CLEANUP) {
			xns_cleanup(conp);
			continue;
		}
		/*
		 * Generate eof if the connection looks dead for
		 * 10 time ticks.  Get nasty at 20.
		 */
		if ((xtime - conp->rtime) == 10) {
			xns_eof(conp);
			continue;
		}
		if ((xtime - conp->rtime) == 20) {
			xns_sig(conp, SIGHUP);
			conp->state = S_CLEANUP;
			continue;
		}
		/*
		 * Bounce a system packet off the other host if it looks
		 * like output flow control is blocked.
		 */
		if (conp->lastseq==conp->ralloc && (xtime-conp->ttime>1)) {
			NXTRACE(("XTIME conp=%x", conp));
			(void) xns_xack(conp, DST_ENQ | XACK_SENDACK);
			continue;
		}
		/*
		 * If we haven't received anything lately,
		 * bounce a system packet off the other side.
		 */
		if ((xtime - conp->rtime) > 2) {
			NXTRACE(("rtime xtime=%d rtime=%d conp=%x",
					xtime, conp->rtime, conp));
			(void) xns_xack(conp, DST_ENQ | XACK_SENDACK);
			if (conp->inpq.nbufs)
				wakeup((caddr_t)&conp->inpq);
			continue;
		}
	}
	/*
	 * Rebroadcast name server packets.
	 * After 4 tries, wakeup the sleeper and clobber the
	 * cache table entry.
	 */
	LOCK;
	for(r=xns_bcast; r; r=t) {
		t = r->active;
		if (r->busy>50) {
			xns_rtabflush(r);
			continue;
		}
		r->busy++;
		if (r->busy > 4) {
			wakeup((caddr_t)r);
			r->age = 0;
			r->name[0] = 0;
			r->name[1] = 0;
			continue;
		}
		if (r->name[0])
			xns_broute(r);
	}
	UNLOCK;
	readenable();
	nxintflag--;
	timeout(xns_timer, (caddr_t)0, HZ*2);
}

/*
 * Send unix signal to process group on the connection.
 * Wakeup sleepers.
 */
xns_sig(conp, sig)
register CONN conp;
{
	xns_ttysig(conp, sig);

	if (conp->blocked) {
		wakeup((caddr_t)&conp->blocked);
		conp->blocked = 0;
	}
	wakeup((caddr_t)&conp->inpq);
}

/*
 * Mark a connection for CLOSING.
 * This happens when:
 *	1) we receive DST_END from remote host,
 *	2) local timer decides to abort connection.
 */
xns_eof(conp)
register CONN conp;
{

	NXTRACE(("xns_eof conp=%x mode=%d", conp, conp->mode));

	conp->inpq.nbufs = -1;
	conp->state = S_CLOSING;
	conp->ntries = 0;
	wakeup((caddr_t)&conp->inpq);
	if (conp->blocked) {
		wakeup((caddr_t)&conp->blocked);
		conp->blocked = 0;
	}
	xns_ttyeof(conp);
}

/*
 * Stay in CLOSING state for a while hoping that outq will drain.
 * Go to FLUSH state after 4 ticks even if outq is non-null.
 * In FLUSH state try to do and END/ENDREPLY handshake.
 * Stay in FLUSH state sending END.  If ENDREPLY is received,
 * the xns_rcv routine will put the connection in CLEANUP state.
 */
xns_doend(conp)
register CONN conp;
{
	NXTRACE(("doend: state=%d conp=%x outq=%x inpq=%x",
			 conp->state, conp, conp->outq.head, conp->inpq.head));

	if (conp->state==S_CLOSING) {
		if (conp->ntries>4 || conp->outq.head==NULL) {
			conp->state = S_FLUSH;
			conp->ntries = 1;
		}
	}
	if (conp->state==S_FLUSH) {
		register struct tty *tp;

		NXTRACE((" doend sending END conp=%x head=%x",
				conp, conp->inpq.head));
		(void) xns_xack(conp, DST_END);
		wakeup((caddr_t)&conp->blocked);
		/*
		 * If there is data to read and a reader
		 * left to read it, don't advance state
		 * any further.  However, get nasty after
		 * a very long while.
		 */
		if ((tp = conp->utp) != NULL
		 && (conp->inpq.head || tp->t_canq.c_cc || tp->t_rawq.c_cc)) {
			if (conp->ntries > 200)
				conp->state = S_CLEANUP;
		}
		else {
			if (conp->ntries > 4)
				conp->state = S_CLEANUP;
		}
	}
	conp->ntries++;
}

#ifdef	notdef
/*
 * Test/Set on a bytes.
 * Could be replaced by (simpler) in-line assembly code.
 */
XBUSY(f)
register char *f;
{
	register s;

	LOCK;
	if (*f) {
		splx(s);
		return(1);
	}
	(*f)++;
	UNLOCK;
	return(0);
}
#endif

AVAIL(f)
register char *f;
{
	register s;

	LOCK;
	if (*f) {
		splx(s);
		return(0);
	}
	(*f)++;
	UNLOCK;
	return(1);
}

/*
 * kernel "hostname" server (called from device rcv interupt routine):
 *	returns 0 if m is not a SERV_HOSTNAME packet
 *	returns 1 if it is, but doesn't match local info
 *	returns m if it is, and matches.
 */
#ifndef	SYSTEMV

struct mbuf *
xns_hostname(m)
	register struct mbuf *m;
#else

NBP
xns_hostname(m)
	register NBP m;
#endif	SYSTEMV
{
	register BOUNCE b;
	register SETUP x;

	/*
	 * If somebody sent us an IDENT packet, see if we want
	 * to remember it.
	 */
	b = nxmtod(m, BOUNCE);
	if (b->cmd==SERV_IDENT) {
		xns_remember(b);
		return(NULL);
	}
	/*
	 * If the message is not a HOSTNAME message, then we're not
	 * interested (in the kernel, anyway).
	 */
	if (b->cmd != SERV_HOSTNAME) {
		return(NULL);
	}
	/*
	 * If the name in the message isn't our own, then nothing happens.
	 */
	if (!eqnstr(b->infostr, hostname, hostnamelen))
#ifndef	SYSTEMV
		return (struct mbuf *)1;
#else	SYSTEMV
		return (NBP)1;
#endif	SYSTEMV

	/*
	 * The name matches.  Rewrite the message header to return
	 * to the sender as type IDENT.  Fill in the address parts
	 * of the message with our local info.
	 */
	b->et.dst = b->et.src;
	b->et.src = xns_myaddr;
	b->et.etype = htons(SG_BOUNCE);
	b->cmd = SERV_IDENT;
	x = (SETUP)b->infostr;
	x->physaddr = xns_myaddr;
	x->internet = myinternet;
	bcopy(hostname, x->name, hostnamelen);
	return m;
}

/*
 * Find slot in routing table.
 *	returns a locked table entry, or NULL.
 */
ROUTE
xns_newroute(name)
	char *name;
{
	register ROUTE r, new;
	long nage;

	/*
	 * First see if there's an unlocked slot that matches name.
	 */
	nage = SYSTIME;
	for(r=xns_rtab; r; r=r->next) {
		if (r->age==0 || r->lock || r->busy)
			continue;
		if (eqnstr(r->name, name, NSIZE)) {
			r->hit = 1;
			NXTRACE(("name cache hit r=%x", r));
			goto out;
		}
	}
	/*
	 * No match.
	 * Find an empty slot (age==0), or LRU slot (smallest age).
	 * Returns NULL if all slots are busy.  This would happen
	 * if NRTAB connections were being set up simultaneously.
	 */
	new = NULL;
	nage = -4;
	for(r=xns_rtab; r; r=r->next) {
		if (r->lock==0 && r->age==0)
			goto out;
		if (r->busy)
			continue;
		if (r->age > 0) {
			if (new==NULL)
				new = r;
			if (r->age < new->age)
				new = r;
		}
	}
	r = new;
	if (r)
		r->hit = 0;
out:
	if (r) {
		r->age = nage;
		r->lock = 1;
	}
	return(r);
}

/*
 * Return route table entry to host identified
 * by x.  Has side-effect of copying route info into
 * connection struct.
 */
ROUTE
xns_getroute(conp, x)
	register CONN conp;
	register SETUP x;
{
	register XSEQ hp;
	register ROUTE r;
	register s;

	/*
	 * If the name field is null, then we "believe" the binary
	 * addresses that are passed in.
	 */
	hp = &conp->header;
	if (x->name[0]==0) {
		static struct xns_route r;

		bcopy((caddr_t)&x->physaddr, (caddr_t)&hp->edst,
		      sizeof (hp->edst));
		bcopy((caddr_t)&x->internet, (caddr_t)&hp->idst,
		      sizeof (hp->idst));
		return(&r);
	}
	/*
	 * If newroute returns a ROUTE entry with non-zero age,
	 * then it is a valid entry for x->name and we're done.
	 * Newroute can fail if ALL entries are marked "busy".
	 * This would happen if we're trying to set up NRTAB
	 * connections simultaneously (not likely).
	 */
	r = xns_newroute(x->name);
	if (r==NULL) {
		NXTRACE(("newroute failed"));
		return(r);
	}
	NXTRACE(("new route r=%x age=%d", r, r->age));
	if (r->age > 0)
		goto done;
	/*
	 * Destination name was not in the cache, so we broadcast 
	 * to find the address.  Thee are two broadcasts to increase the
	 * chance of getting through on a busy net.
	 */
	LOCK;
	bcopy(x->name, r->name, NSIZE);
	r->active = xns_bcast;
	xns_bcast = r;
	r->busy = 1;
	r->age = 0;
	xns_broute(r);
	xns_broute(r);
	for (;;) {
		if (r->age || r->busy>4)
			break;
		sleep((caddr_t)r, TTOPRI);
	}
	if (r->busy)
		xns_deactivate(r);
	UNLOCK;
	if (r->age==0) {
		xns_rtabflush(r);
		return(NULL);
	}
	
done:
	bcopy((caddr_t)&r->physaddr, (caddr_t)&hp->edst,
	      sizeof (r->physaddr));
	bcopy((caddr_t)&r->internet, (caddr_t)&hp->idst,
	      sizeof (hp->idst));
	hp->idst.socket = x->internet.socket;
	return(r);
}

/*
 * Remove r from the linked list
 * headed by xns_bcast.
 */
xns_deactivate(r)
	register ROUTE r;
{
	register ROUTE r1, r2;
	register s;

	LOCK;
	r->busy = 0;
	if (xns_bcast == r) {
		xns_bcast = r->active;
		goto out;
	}
	for(r1=xns_bcast; r1; r1=r1->active) {
		r2 = r1->active;
		if (r2 == r) {
			r1->active = r2->active;
			goto out;
		}
	}
out:
	r->active = NULL;
	UNLOCK;
}

/*
 * Invalidate a route table (cache) entry.
 * Take it off active list if busy.
 */
xns_rtabflush(r)
	register ROUTE r;
{
	register s;

	if (r==NULL)
		return;
	r->lock = 0;
	r->age = 0;
	r->name[0] = 0;
	LOCK;
	if (r->busy)
		xns_deactivate(r);
	UNLOCK;
}

/*
 * Broadcast a route/name request.
 * The BOUNCE prototype packet is copied into a netbuf
 * and the host name field is filled in.
 */
xns_broute(r)
register ROUTE r;
{
	register NBP nbp;
	register BOUNCE b;
	
#ifndef	SYSTEMV
	nbp = getnbuf();
#else
	if ((nbp = getnbuf(0)) == NULL)
		return;
#endif	SYSTEMV

	b = nxmtod(nbp, BOUNCE);
	bcopy((caddr_t)&bounceproto, (caddr_t)b, sizeof(bounceproto));
	bcopy(r->name, b->infostr, NSIZE);
	nbp->len = sizeof (struct bouncemsg);
	nx_send(nbp);
}

/*
 * Remember an IDENT message by putting it in the rtab/cache.
 */
xns_remember(b)
BOUNCE b;
{
	register SETUP x;
	register ROUTE r;

	/*
	 * make sure it's not a broadcast packet.
	 */
	if (!EQHOST(b->et.dst, xns_myaddr))
		return;
	x = (SETUP)b->infostr;

	/*
	 * Find a locked rtab entry that's waiting for the
	 * bounce msg (i.e. names match).  Copy in the info, leave it
	 * locked, and update the age to the current time.
	 */
	for(r=xns_bcast; r; r=r->active) {
		if (eqnstr(x->name, r->name, NSIZE)) {
			r->internet = x->internet;
			r->physaddr = b->et.src;
			r->age = SYSTIME;
			wakeup((caddr_t)r);
			xns_deactivate(r);
			NXTRACE(("name hit on r=%x", r));
			break;
		}
	}
}

eqnstr(a, b, len)
register char *a, *b;
register len;
{
	while (*a == *b) {
		if (*a == 0)
			return(1);
		if (len==0)
			return(0);
		a++; b++; len--;
	}
	if (*a==':' && *b==0)
		return(1);
	return(0);
}


#ifdef  SYSTEMV
#ifdef	XTRACE
nxtrace(c, p0, p1, p2, p3, p4)
char *c;
long p0, p1, p2, p3, p4;
{
	register struct xtrace *p;
	register int s;
	extern time_t time;

	LOCK;
	p = &xtbuf[xtp++];
	if (xtp>=NXCELLS)
		xtp = 0;
	p->code = c;
	p->p0 = p0;
	p->p1 = p1;
	p->p2 = p2;
	p->p3 = p3;
	p->p4 = p4;
	p->time = time;
	UNLOCK;
}
#endif  XTRACE


/*
 * Flush a netqueue.
 */
xns_qflush(q)
	register struct netqueue *q;
{
	while (q->head != NULL)
		qfreenbuf(q);
	q->nbufs = 0;
}
#endif SYSTEMV

/*
 * Search connection table for an entry.
 * `nconp' will point to the free entry.  Make sure the
 * connection is not already set up, i.e. that the current
 * message is a repeat.  If so, nconp will point to the already
 * set up connection.   A reply is generated in both cases.
 *
 * This is only called at interrupt time, so we don't need to block
 * out other connection allocations.
 */
xns_login(xp)
	register XSEQ xp;
{
	register XSEQ hp;
	register CONN conp, nconp;
	register struct tty *ntp;
	register ushort id;

	/*
	 * Find a connection struct.
	 */
	nconp = (CONN)NULL;
	ntp = (struct tty *)NULL;
	for (conp=xns_connbase; conp; conp=conp->next) {
		hp = &conp->header;
		if (conp->state!=S_FREE)
			continue;
		if (nconp==NULL && hp->srcid==0 && conp->utp==NULL) {
			nconp = conp;
			id = idnumber++;
			if (idnumber==0)
				idnumber++;
			continue;
		}
		if (hp->srcid==0 && conp->utp==NULL)
			continue;
		if (EQADDR(xp->isrc, hp->idst)) {
			if (hp->dstid!=xp->srcid)
				continue;
			nconp = (CONN)hp;
			ntp = nconp->utp;
			id = nconp->header.srcid;
			break;
		}
	}
	/*
	 * If we have a connection struct,
	 * find a special file waiting on the socket.
	 */
	if (nconp!=NULL && ntp==NULL)
		ntp = xns_findtty(xp->idst.socket);
	/*
	 * Fail if something didn't work.
	 */
	if (nconp==NULL || ntp==NULL) {
		NXTRACE(("fail nconp=%x ntp=%x", nconp, ntp));
		return;
	}
	/*
	 * Record the socket that the connection came in on.
	 */
	nconp->sockin = xp->idst.socket;

	/*
	 * hook up connection and tty.
	 */
	NXTRACE(("new connection: tp=%x nconp=%x conp=%x", ntp, nconp, conp));
	xns_initconn(ntp, nconp, xp, (unsigned short)id);
	nconp->state = S_ACTIVE;

	if (xns_xack(nconp, DST_SYSTEM))
		nconp->ttime = the_time;
}

/*
 * Return XNS echo packets.
 */
xns_echo(nbp)
	register NBP nbp;
{
	register XECHO ep;

	ep = nxmtod(nbp, XECHO);
	if (ep->operation != ECHOREQUEST)
		return;
	ep->operation = ECHOREPLY;

	if ((nbp = getnbuf(0)) == NULL)
		return;

	ep->edst = ep->esrc;
	ep->esrc = xns_myaddr;
	bcopy((caddr_t)ep, nxmtod(nbp, caddr_t), sizeof(Xecho));
	nx_send(nbp);
}

/*
 * move already-received block data from the
 * block inq to the tty inq.
 */
xns_blockoff_rcv(conp)
	register CONN conp;
{
	register NBP nbp, *finger;
	register int s;
	short foundany, foulup;

	foundany = foulup = 0;

	LOCK;
	for (finger = &conp->inpq.head; (nbp = *finger) != NULL;) {
		if (nbp->dtype == DST_DATA || nbp->dtype == DST_OLDDATA) {
			/*
			 * found a data buffer.  treat it more or
			 * less as xns_rcv() have were the connection
			 * not in block mode when the packet arrived.
			 * the xns header has already been removed.
			 */
			foundany ++;
			conp->allocno ++;

			if (!xns_ttyinput(nxmtod(nbp, caddr_t),
					nbp->len, conp))
				foulup --;

			/*
			 * delete the buffer from the block inq.
			 * then free it.
			 */
			conp->inpq.nbufs --;
			*finger = nbp->next;
			freenbuf(nbp);
			nbp = *finger;
		}
		else {
			finger = &nbp->next;
		}
	}

	/*
	 * if found any, advance flow control ala xns_rcv() .
	 */
	if (foundany) {
		if (xns_xack(conp, DST_SYSTEM) == 0)
			conp->asend++;
	}
	UNLOCK;

	return foulup;
}

/*
 * conp_restart() --
 * called from nx restart code.
 * after the nx is restarted, kick protocol code.
 */
conp_restart()
{
	register CONN conp;

	for (conp = xns_connbase; conp != NULL; conp = conp->next) {
		if (conp->hbusy)
			NXTRACE((" conp $%x: hbusy-->0",conp));
		conp->hbusy = 0;
		if (conp->outq.head != NULL) {
			conp->trans = conp->outq.head;
			xns_xdone(conp);
			if (conp->utp && conp->state==S_ACTIVE)
				nxstart(conp->utp);
		}
	}
}
