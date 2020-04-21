/*
 * Xns protocol code specific to tty management
 */

#define  SYSTEMV
#include "../h/param.h"
#include "../h/types.h"
#include "../h/signal.h"
#include "../h/errno.h"
#include "../h/dir.h"
#include "../h/conf.h"
#include "../h/tty.h"
#include "../h/termio.h"
#ifdef SYSTEMV
#include "../h/file.h"
#endif SYSTEMV

#include "../xns/if_xns.h"

#define	ntohs(x)	(x)

# ifdef SYSTEMV
# define pgrpsignal(pgrp, sig)		signal(pgrp, sig)
# else  SYSTEMV
# define pgrpsignal(pgrp, sig)		gsignal(pgrp, sig)
# endif SYSTEMV

extern short xns_seqfix;

/*
 * Find a tty waiting for a connection on socket.
 */
struct tty *
xns_findtty(socket)
	register socket;
{
	register struct tty *tp, *stp;
	register d = 0;
	register sd;

	for(stp=NULL,tp=nx_tty; tp<&nx_tty[NDEV]; tp++,d++) {
		if (tp->t_state & WOPEN) {
			if ((stp == NULL) &&
			    (xnsmisc[d].x_socket == ALLSOCK)) {
		NXTRACE(("xns_findtty: found ALLSOCK tp=%x socket=%d d=%d",
				       tp, socket, d));
				stp = tp;
				sd = d;
			}
			if (xnsmisc[d].x_socket != (short)socket)
				continue;
			NXTRACE(("match SOCK tp=%x socket=%d d=%d",
					tp, socket, d));
			xns_enatty(tp);
			wakeup((caddr_t)&tp->t_rawq);
			return tp;
		}
	}
	/*
	 * Kludge: waiting on socket "-1" only works for socket numbers
	 * less than 100.
	 */
	if (stp && (socket < 100)) {
		NXTRACE(("xns_findtty: match ALLSOCK stp=%x socket=%d d=%d",
				       stp, socket, sd));
		xnsmisc[sd].x_socket = 0;		/* XXX for debug */
		xns_enatty(stp);
		wakeup((caddr_t)&stp->t_rawq);
		return stp;
	}
	NXTRACE(("xns_findtty: no match socket=%d", socket));
	return NULL;
}

/*
 * Connect tty to xns struct.
 */
xns_conntty(tp, conp)
	register struct tty *tp;
	register CONN conp;
{
	register struct xnsmisc *xp;
	register short m;

	m = XNSTTYINDEX(tp);
	xp = xnsmisc + m;
	xp->x_conp = conp;
	xp->x_state |= X_CONNECTED;
	conp->utp = tp;
}

xns_enatty(tp)
	register struct tty *tp;
{
	tp->t_state |= CARR_ON|ISOPEN;
	tp->t_state &= ~WOPEN;
}

/*
 * xns_close:
 *	- close down a connection
 */
xns_close(tp)
	register struct tty *tp;
{
	register struct xnsmisc *xp;
	register CONN conp;
	register int s;
	register short m;

	NXTRACE(("xns_close tp=%x", tp));
	if (tp==NULL)
		return;
	m = XNSTTYINDEX(tp);
	xp = xnsmisc + m;
	if (!(xp->x_state & X_CONNECTED))
		return;
/*
	if ((conp = xp->x_conp) == NULL)
		return;
	if ((xp->x_socket >= 0) &&
	    (xp->x_socket < SOCKBASE))
		return;
*/
	conp = xp->x_conp;
	if (conp->utp != tp)
		return;

	LOCK;
	if (tp->t_tbuf.c_ptr != NULL) {
		putcf(CMATCH(tp->t_tbuf.c_ptr));
		tp->t_tbuf.c_ptr = NULL;
	}
	if (tp->t_rbuf.c_ptr != NULL) {
		putcf(CMATCH(tp->t_rbuf.c_ptr));
		tp->t_rbuf.c_ptr = NULL;
	}
	xns_flush_ttyq(&tp->t_rawq);
	xns_flush_ttyq(&tp->t_canq);
	xns_flush_ttyq(&tp->t_outq);
	conp->utp = NULL;
	xp->x_state &= ~X_CONNECTED;
	xp->x_conp = NULL;
	if (conp->state==S_ACTIVE || conp->state==S_CALLOUT) {
		conp->state = S_CLOSING;
		conp->ntries = 0;
	}
	conp->ttime = 0;
	xns_doend(conp);
	UNLOCK;
}

xns_ttyeof(conp)
register CONN conp;
{
register struct tty *tp;
register struct xnsmisc *xp;
register short m;

	tp = conp->utp;
	m = XNSTTYINDEX(tp);
	if ((unsigned)m >= NDEV)
		return;
	xp = xnsmisc + m;
	xp->x_state |= X_EOFPEND;
	wakeup((caddr_t)&tp->t_rawq);
	tp->t_state &= ~CARR_ON;
	nxproc(tp, T_WFLUSH);
}

/*
 * Copy from network buffer to unix tty input line discipline:
 *	- do ^S/^Q processing here
 */
xns_ttyinput(s, len, conp)
	register char *s;
	register unsigned len;
	CONN conp;
{
	register struct tty *tp;
	register struct ccblock *cbp;
	register char c, ctmp;

	tp = conp->utp;
	cbp = &tp->t_rbuf;
	if (cbp->c_ptr == NULL)			/* !ISOPEN */
		return 0;
	if (len == 0)
		return 1;

	while (len--) {
		c = *s++;
		if (tp->t_iflag&IXON) {
			ctmp = c & 0177;
			if (tp->t_state&TTSTOP) {
				if (ctmp == CSTART || tp->t_iflag&IXANY)
					(*tp->t_proc)(tp, T_RESUME);
			} else {
				if (ctmp == CSTOP)
					(*tp->t_proc)(tp, T_SUSPEND);
			}
			if (ctmp == CSTART || ctmp == CSTOP)
				continue;
		}
		if (tp->t_iflag & ISTRIP)
			c &= 0x7f;

	    /* stash character in buffer */
		*cbp->c_ptr++ = c;
		if (--cbp->c_count == 0) {
			cbp->c_ptr -= cbp->c_size;
			(*linesw[tp->t_line].l_input)(tp);
		}
	}

    /* hand any left over characters up to the line discipline */
	if (cbp->c_size != cbp->c_count) {
		cbp->c_ptr -= cbp->c_size - cbp->c_count;
		(*linesw[tp->t_line].l_input)(tp);
	}

	return 1;
}


/*
 * send a signal.
 */
xns_ttysig(conp, sig)
register CONN conp;
{
register struct xnsmisc *xp;
register struct tty *tp;
register int m;

	if ((tp = conp->utp) == NULL)
		goto nokill;
	if ((unsigned)(m = XNSTTYINDEX(tp)) >= NDEV)
		goto nokill;

	xp = xnsmisc + m;

# ifdef SYSTEMV
	tp->t_state &= ~BUSY;
	ttyflush(tp, FWRITE|FREAD);
	tp->t_state &= ~(BUSY|CARR_ON);	/* UGLY */
# else  SYSTEMV
	ttyflush(tp, FWRITE|FREAD);
	tp->t_state &= ~TS_CARR_ON;
# endif SYSTEMV

	if (!(xp->x_state & X_PGRP))
		goto nokill;

	pgrpsignal((int)tp->t_pgrp, sig);
	return;

nokill:
	xns_eof(conp);
}



/*
 * tty interface to unix.
 *	xns_ttstart is called from a line discipline or the device driver.
 *	It moves data from the tty clist area to a netbuf.
 *      Returns true if the data was accepted (queued to be sent)
 */
#ifdef SYSTEMV
xns_ttstart(conp, nbp)
register CONN conp;
register NBP nbp;
{
	if (nbp==NULL)
		return 1;
	if (conp->state != S_ACTIVE) {
		freenbuf(nbp);			/* drop data */
		return 1;
	}
	NXTRACE(("xns_ttstart: conp=%x len=%d", conp, nbp->len));
	if (nxintflag==0) {
		xns_start(conp, nbp);
		return 1;
	}

	if (conp->lastseq == conp->ralloc) {
		NXTRACE(("fail ttstart flow lastseq=%d alloc=%d",
			       conp->lastseq, conp->ralloc));
		/*
		 * NOTE that this netbuf Must have been at the head
		 * of the pending queue, else it wouldn't have been
		 * started.  ALSO this routine is called at hi pri.
		 */
		nbinsert(nbp, &conp->pending);
		return 1;
	}

	nbp->seq = conp->lastseq = conp->nextseq++;
	if (xns_seqfix)
		conp->lastseq = conp->nextseq;

	nbp->quack = 0;
	nbappend(nbp, &conp->outq);
	/*LOCK;			redundent; nxintflag is checked above */
	if (conp->trans == NULL) {
		conp->trans = nbp;
		NXTRACE(("new trans seq=%d nbp=%x", nbp->seq, nbp));
		if (AVAIL(&conp->hbusy)) {
			NXTRACE(("ttstart: conp=%x hbusy=%d",
					   conp, conp->hbusy));
			xns_xdone(conp);
		}
	}
	/* UNLOCK;		see LOCK above */
	return 1;
}
#else

xns_ttstart(tp, conp)
struct tty *tp;
register CONN conp;
{
register NBP nbp;
register XSEQ xp;
register len;
register int s;

	if (conp->state!=S_ACTIVE)
		return;
	xp = &conp->header;
	NXTRACE(("xns_ttstart: conp=%x lenc=%d", conp, tp->t_outq.c_cc));

	if (nxintflag==0) {
		if (conp->blocked) {
			return 0;
		}

		if (tp->t_outq.c_cc > HLEN)
			nbp = getbnbuf();
		else
			nbp = getnbuf();
		nbp->dtype = DST_DATA;
		nbp->len = xns_ttyoutput(tp, nxmtod(nbp,caddr_t), nbp->len);
		NXTRACE(("new nbp=%x len=%d tplen=%d", nbp, nbp->len,
			      tp->t_outq.c_cc));
		xns_start(conp, nbp);
		return 1;
	}

	/* make sure xns flow control will allow data to be sent */
	if (conp->lastseq == conp->ralloc) {
		NXTRACE(("fail ttstart flow lastseq=%d alloc=%d",
			       conp->lastseq, conp->ralloc));
		return 1;
	}

	if (conp->blocked) {
		NXTRACE(("fail ttstart usr blocked"));
		return 0;
	}
	if (conp->pending.nbufs) {
		nbp = qfirst(&conp->pending);
	} else
	if ((nbp=igetnbuf())!=NULL) {
		nbp->len = xns_ttyoutput(tp, nxmtod(nbp,caddr_t), nbp->len);
		nbp->dtype = DST_DATA;
		NXTRACE(("new obuf nbp=%x seq=%d", nbp, nbp->seq));
	} else {
		NXTRACE(("fail ttstart igetnbuf"));
		return 0;
	}

	nbp->seq = conp->lastseq = conp->nextseq++;
	if (xns_seqfix)
		conp->lastseq = conp->nextseq;

	nbp->quack = 0;
	nbappend(nbp, &conp->outq);

	LOCK;
	if (conp->trans == NULL) {
		conp->trans = nbp;
		NXTRACE(("new trans seq=%d nbp=%x", nbp->seq, nbp));
		if (AVAIL(&conp->hbusy))
			xns_xdone(conp);
	}
	UNLOCK;
	return 1;
}
#endif

/*
 * xns_ioctl:
 *	- called to adjust xns state given user's ioctl to tty device
 *	- if we have no xns connection, or don't support the ioctl, then
 *	  just return -1 so that caller will ignore us
 *	- if we get an error, return it
 */
/*ARGSUSED*/
xns_ioctl(tp, conp, cmd, addr)
	struct tty *tp;
	register CONN conp;
	caddr_t addr;
{
/* XXX move this into device code: don't call xns_ioctl if we don't
       have a conp??? */
#ifndef	SYSTEMV
	/* None of these ioctl's apply to a non-connection */
	if (conp == NULL)
		return -2;		/* ignore */
#endif

	switch(cmd) {
	  case NXBLOCKOFF:
		conp->mode = 0;
		break;
	  case NXGETCONN:
		if (copyout((caddr_t)conp, addr, 64))
			return EFAULT;
		break;
	  case NXGETSOCK:
		if (copyout((caddr_t)&conp->sockin, addr,
			sizeof (conp->sockin)))
			return EFAULT;
		break;
	  case NXSETPGRP:
		break;
	  default:
#ifndef	SYSTEMV
		return -2;		/* note that we don't support this */
#else
		return -1;		/* note that we don't support this */
#endif
	}

	return 0;			/* everythings hunky dory */
}

xns_flush_ttyq(p)
	struct clist *p;
{
	register struct cblock *cp;

	while ((cp = getcb(p)) != NULL) {
		putcf(cp);
	}
	wakeup((caddr_t)p);
}
