/*
 * SGI system calls specific to graphics
 *
 * $Source: /d2/3.7/src/sys/gl1/RCS/grsys.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:28:24 $
 */

#include "../h/param.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/proc.h"
#include "machine/cpureg.h"
#include "../gl1/kgl.h"
#include "../gl1/shmem.h"
#include "../gl1/kbd.h"
#include "../gl1/grioctl.h"
#include "../gl1/device.h"
#include "../gl1/textport.h"
#include "../gl1/font.h"

extern unsigned short int wncurtty;

/*
 * grsys:
 *	- handle graphics commands
 */
grsys()
{
	register struct a {
		int	cmd;
		caddr_t	addr;
	} *uap = (struct a *)u.u_ap;
	register struct shmem *sh = (struct shmem *)SHMEM_VBASE;
	register struct textport *tx;
	register int s;
	register ushort i;
	short data[2];
	short cdata[3];
	struct kbd nkbd;
	struct grmapcolor mc;
	struct grsetvaluator sv;
	struct grblinkcolor *pbc, *blink_alloc(), blinkbuf;/* peter 8/21/84 */
	char dialbuf[8];

    /* if no graphics hardware, then return an error */
	if (fbcalive == 0) {
		u.u_error = EIO;
		return;
	}

	switch (uap->cmd) {
	  case GR_ALLOC:
		if (grproc) {
			u.u_error = EGRBUSY;
/* XXX this is just for now... */
			psignal(u.u_procp, SIGKILL);
			return;
		}
		if (fbcalive == 0) {
			u.u_error = ENOGR;
			return;
		}
		grproc = u.u_procp;
		u.u_procp->p_flag |= SGR;
		cxput(u.u_procp, 1);

	    /* init shared memory */
		kgr |= KGR_BUSY;
		sh->GE_found = gefound;
		sh->GE_mask = gemask;
		fbcshmem();
		kgr &= ~KGR_BUSY;
		break;
	  case GR_RESET:			/* reset pipe */
		if (u.u_procp == grproc)
			(void) gereset();
		else
			u.u_error = EGRBUSY;
		break;
	  case GR_FREE:				/* free graphics */
		gr_free();
		u.u_procp->p_flag &= ~SGR;
		break;
	  case GR_QENTER:			/* enter data in queue */
		if (copyin(uap->addr, (caddr_t)data, sizeof(data))) {
			u.u_error = EFAULT;
			break;
		}
		u.u_rval1 = qenter(data[0], data[1]);
		u.u_rval2 = 0;
		break;
	  case GR_QRESET:			/* zero out the queue */
		s = spl6();
		nqueued = 0;
		queuein = queueout = queue;
		splx(s);
		break;
	  case GR_QTEST:			/* test for queue data */
		s = spl6();
		if (qtest(&data[0])) {
			if (copyout((caddr_t)data, uap->addr, sizeof(short))) {
				u.u_error = EFAULT;
				splx(s);
				break;
			}
			u.u_rval1 = 1;
		} else
			u.u_rval1 = 0;
		u.u_rval2 = 0;
		splx(s);
		break;
	  case GR_ATTACHCURSOR:
		if (copyin(uap->addr, (caddr_t)data, sizeof(data))) {
			u.u_error = EFAULT;
			break;
		}
		data[0] -= VALOFFSET;
		data[1] -= VALOFFSET;
		if ((data[0] < 0) || (data[0] >= VALCOUNT) ||
		    (data[1] < 0) || (data[1] >= VALCOUNT)) {
			u.u_error = EINVAL;
			break;
		}
		sh->cursorxvaluator = data[0];
		sh->cursoryvaluator = data[1];
		/* FALL THROUGH */
	  case GR_RESETCURSOR:
		cursorx = sh->valuators[sh->cursorxvaluator].value;
		cursory = sh->valuators[sh->cursoryvaluator].value;
		s = spl3();			/* block fbcintr */
		mousebusy = 1;
		if (sh->cursorxvaluator == (MOUSEX - VALOFFSET))
			_mousex = cursorx;
		if (sh->cursoryvaluator == (MOUSEY - VALOFFSET))
			_mousey = cursory;
		mousebusy = 0;
		mouse();
		mouse();			/* for luck */
		splx(s);
		break;
	  case GR_SETVALUATOR:
		if (copyin(uap->addr, (caddr_t)&sv, sizeof(sv))) {
			u.u_error = EFAULT;
			break;
		}
		if ((sv.gr_valuator >= (DIAL0 - VALOFFSET)) &&
		    (sv.gr_valuator <= (DIAL7 - VALOFFSET)) ||
		    (sv.gr_valuator == LPENX) ||
		    (sv.gr_valuator == LPENY))
			dial_reset(sv.gr_valuator, sv.gr_initialvalue);
		else
			u.u_error = EINVAL;
		break;
	  case GR_QREAD:
		qread(&data[0], &data[1]);
		if (copyout((caddr_t)data, uap->addr, sizeof(data))) {
			u.u_error = EFAULT;
			break;
		}
		break;
	  case GR_SETKBD:
		if (copyin(uap->addr, (caddr_t)&nkbd, sizeof(nkbd))) {
			u.u_error = EFAULT;
			break;
		}
		if ((nkbd.k_beepmask & KBD_CLICK) !=
		    (kbd.k_beepmask & KBD_CLICK))
			kb_putc(nkbd.k_beepmask &
					~(KBD_SHORTBEEP|KBD_LONGBEEP));
		kb_putc(nkbd.k_ledmask);
		kbd.k_beepmask = nkbd.k_beepmask;
		kbd.k_ledmask = nkbd.k_ledmask;
		break;
	  case GR_GETKBD:
		if (copyout((caddr_t)&kbd, uap->addr, sizeof(kbd)))
			u.u_error = EFAULT;
		break;
	  case GR_MAPCOLOR:
		if (copyin(uap->addr, (caddr_t)&mc, sizeof(mc))) {
			u.u_error = EFAULT;
			break;
		}
		mapcolor(mc.gr_index, mc.gr_red, mc.gr_green, mc.gr_blue);
		break;

	/* All grioctl's below are specific to window management */

	  case GR_WATTACH:
		i = (ushort)uap->addr;
		if (i > NTEXTPORT)
			u.u_error = EINVAL;
		else if (txport[i].tx_state & TX_OPEN)
			wncurtty = i;
		else
			u.u_error = EINVAL;
		break;
	  case GR_GETWIN:
		tx = &txport[0];
		for (i = 0; i < NTEXTPORT; i++, tx++) {
			if ((tx->tx_state & TX_OPEN) == 0) {
				tx_init(tx);
				tx->tx_state |= TX_OPEN;
				u.u_rval1 = i;
				u.u_rval2 = 0;
				return;
			}
		}
		u.u_error = EBUSY;
		break;
	  case GR_PUTWIN:
		i = (ushort)uap->addr;
		if (i > NTEXTPORT)
			u.u_error = EINVAL;
		else {
			tx = &txport[i];
			tx->tx_state &= ~TX_OPEN;
		}
		break;
	  case GR_ENABLEWIN:
	  case GR_DISABLEWIN:
		i = (ushort)uap->addr;
		if (i > NTEXTPORT)
			u.u_error = EINVAL;
		else {
			tx = &txport[i];
			if (uap->cmd == GR_ENABLEWIN) {
				tx->tx_state |= TX_ON | TX_SCROLLED |
					TX_REDISPLAY;
				gr_tpblank = 0;
				wnenable(i);
			} else
				tx->tx_state &= ~TX_ON;
		}
		break;
	  case GR_BORDERON:
	  case GR_BORDEROFF:
		i = (ushort)uap->addr;
		if (i > NTEXTPORT)
			u.u_error = EINVAL;
		else {
			tx = &txport[i];
			if (uap->cmd == GR_BORDERON)
				tx->tx_state |= (TX_BORDER | TX_DRAWBORDER);
			else
				tx->tx_state &= ~(TX_BORDER | TX_DRAWBORDER);
		}
		break;
	  case GR_SETPIECE:
	  case GR_SETPIECE1:
		s = spltty();
		u.u_error = setpieces(uap->addr, uap->cmd);
		splx(s);
		break;
	  case GR_GETPIECE:
		u.u_error = getpieces(uap->addr);
		break;
	  case GR_FLUSH:
		wnrepaint();
		break;

	/* all of these grioctl's are of questionable taste */

	  case GR_TPBLANK:
		gr_tpblank = 1;
		txport[0].tx_state &= ~TX_ON;
		break;
	  case GR_TEXTCOLOR:
	  case GR_PAGECOLOR:
		tx = &txport[0];
		tx->tx_state |= TX_SCROLLED | TX_DRAWBORDER;
		if (uap->cmd == GR_TEXTCOLOR)
			tx->tx_borderon = tx->tx_foreground = (char)uap->addr;
		else
			tx->tx_borderoff = tx->tx_background = (char)uap->addr;
		tx->tx_curcolor = tx->tx_foreground;
		tx_redisplay(tx);
		break;
	  case GR_RINGBELL:
		kbbell();
		break;

	/* more junk added to make-things-work */
	  case GR_DIALINIT:
		dial_init();
		break;
	  case GR_DIALTEXT:
		if (copyin(uap->addr, dialbuf, sizeof(dialbuf)))
			u.u_error = EFAULT;
		else
			dial_text(dialbuf);
		break;
	  case GR_DIALLEDS:
		dial_leds((u_long)uap->addr);
		break;

	/* blink code added by 				peter 8/21/84 */
	  case GR_ADDBLINK:
		if (copyin(uap->addr, (caddr_t)&blinkbuf, sizeof(blinkbuf)))
			u.u_error = EFAULT;
		else
			if(pbc = blink_alloc(blinkbuf.gr_index)) {
				*pbc = blinkbuf;
			} else {
				u.u_error = EINVAL;
			}
		break;
	  case GR_DELBLINK:
		if (copyin(uap->addr, (caddr_t)&blinkbuf, sizeof(blinkbuf)))
			u.u_error = EFAULT;
		else
			del_blink(blinkbuf.gr_index);
		break;

	/* font management grioctls */
# ifdef GR_GETCHARINFO
	  case GR_GETCHARINFO:
		cdata[0] = WIDTH;
		cdata[1] = HEIGHT;
		cdata[2] = DESCENDER;
		if (copyout((caddr_t)cdata, uap->addr, sizeof cdata))
			u.u_error = EFAULT;
		break;
# endif GR_GETCHARINFO
# ifdef GR_SETCHARINFO
	  case GR_SETCHARINFO:
		if (copyin(uap->addr, (caddr_t)cdata, sizeof cdata)) {
			u.u_error = EFAULT;
			break;
		}
		if (cdata[0]) 
			WIDTH = cdata[0];
		if (cdata[1]) 
			HEIGHT = cdata[1];
	        DESCENDER = cdata[2];
		tx_resetconsole(&txport[0]);
		tx_redisplay(&txport[0]);
		break;
# endif GR_SETCHARINFO
# ifdef GR_GETCHAROFFSETS
	  case GR_GETCHAROFFSETS:
		if (copyout((char *)defont_font, uap->addr,
				GLYPHSPERFONT*sizeof *defont_font))
			u.u_error = EFAULT;	
		break;
# endif GR_GETCHAROFFSETS
# ifdef GR_SETCHAROFFSETS
	  case GR_SETCHAROFFSETS:
		if (copyin(uap->addr, (char *)defont_font,
				GLYPHSPERFONT*sizeof *defont_font)) 
			u.u_error = EFAULT;	
		break;
# endif GR_SETCHAROFFSETS

	  default:
		u.u_error = EINVAL;
		break;
	}
}

/*
 * setpieces:
 *	- read a piece list from the user
 *	- certain simple checks are made for data integrity; however, the
 *	  pieces themselves are unchecked
 *	- pieces are assumed to be contiguous in piece array
 */
setpieces(uaddr, cmd)
	register char *uaddr;
{
	register struct textport *tx;
	register struct piece *p;
	register struct grpiecehdr *ph;
	register short npieces, i;
	register short error = 0;
	struct grpiecehdr piecehdr;
	struct grpiece piece;

	if (copyin(uaddr, (caddr_t)&piecehdr, sizeof(piecehdr)))
		return EFAULT;
	uaddr += sizeof(piecehdr);

    /* make sure # of pieces is ok */
	npieces = piecehdr.gr_pieces;
	if (npieces > MAXPIECES)
		return EINVAL;

    /* make sure window # is ok, and in use */
	if (piecehdr.gr_win >= NTEXTPORT)
		return EINVAL;
	tx = &txport[piecehdr.gr_win];
	if ((tx->tx_state & TX_OPEN) == 0)
		return EINVAL;

    /* check that new textport location is on the screen, and sized ok */
	ph = &piecehdr;
	if ((ph->gr_xlen < MINCOLS) || (ph->gr_xlen > MAXCOLS) ||
	    (ph->gr_ylen < MINROWS) || (ph->gr_ylen > MAXROWS) ||
	    (ph->gr_llx + XBORDER > XMAXSCREEN) ||
	    (ph->gr_lly + YBORDER > YMAXSCREEN))
		return EINVAL;

    /* update textports screen location */
	tx->tx_state |= TX_BUSY | TX_SCROLLED | TX_REDISPLAY | TX_DRAWBORDER;
	tx->tx_w.w_llx = ph->gr_llx;
	tx->tx_w.w_lly = ph->gr_lly;
	tx->tx_w.w_xlen = ph->gr_xlen;
	tx->tx_w.w_ylen = ph->gr_ylen;

    /* read in each piece, verifying it as we go */
	p = &tx->tx_w.w_piece[0];
	for (i = 0; i < npieces; i++, p++) {
		if (copyin(uaddr, (caddr_t)&piece, sizeof(piece))) {
			error = EFAULT;
			break;
		}
		uaddr += sizeof(piece);
		p->p_state = 1;
		p->p_xmin = piece.gr_xmin;
		p->p_ymin = piece.gr_ymin;
		p->p_xmax = piece.gr_xmax;
		p->p_ymax = piece.gr_ymax;
		p->p_ly = piece.gr_ly;
		p->p_uy = piece.gr_uy;
	}
	p->p_state = 0;
	tx_fixcursor(tx);
	tx->tx_state &= ~TX_BUSY;
	if (cmd == GR_SETPIECE)
		tx_redisplay(tx);
	return error;
}

/*
 * getpieces:
 *	- write a piece list to the user
 *	- pieces are assumed to be contiguous in piece array
 */
getpieces(uaddr)
	register char *uaddr;
{
	register struct textport *tx;
	register struct piece *p;
	register struct grpiecehdr *ph;
	register short npieces, i;
	struct grpiecehdr piecehdr;
	struct grpiece piece;

    /* read in users piece hdr to find npieces and window # */
	if (copyin(uaddr, (caddr_t)&piecehdr, sizeof(piecehdr)))
		return EFAULT;

    /* validate */
	if ((piecehdr.gr_pieces > MAXPIECES) || (piecehdr.gr_win > NTEXTPORT))
		return EINVAL;
	tx = &txport[piecehdr.gr_win];
	if ((tx->tx_state & TX_OPEN) == 0)
		return EINVAL;

    /* build up piechdr for user, then copy it out */
	ph = &piecehdr;
	ph->gr_llx = tx->tx_w.w_llx;
	ph->gr_lly = tx->tx_w.w_lly;
	ph->gr_xlen = tx->tx_w.w_xlen;
	ph->gr_ylen = tx->tx_w.w_ylen;
	npieces = 0;
	for (p = &tx->tx_w.w_piece[0]; p->p_state != 0; p++)
		npieces++;
	ph->gr_pieces = npieces;
	if (copyout((caddr_t)&piecehdr, uaddr, sizeof(piecehdr)))
		return EFAULT;
	uaddr += sizeof(piecehdr);

    /* write out each piece */
	p = &tx->tx_w.w_piece[0];
	for (i = 0; i < npieces; i++, p++) {
		piece.gr_xmin = p->p_xmin;
		piece.gr_ymin = p->p_ymin;
		piece.gr_xmax = p->p_xmax;
		piece.gr_ymax = p->p_ymax;
		if (copyout((caddr_t)&piece, uaddr, sizeof(piece)))
			return EFAULT;
		uaddr += sizeof(piece);
	}
	return 0;
}
