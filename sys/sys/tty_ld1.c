/*
 * $Source: /d2/3.7/src/sys/sys/RCS/tty_ld1.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:35:41 $
 */
#include "../h/param.h"
#include "../h/systm.h"
#include "../h/conf.h"
#include "../h/user.h"
#include "../h/proc.h"
#include "../h/file.h"
#include "../h/tty.h"
#include "../h/termio.h"
#include "../h/dk.h"
#include "../h/sysinfo.h"

extern char partab[];

/*
 * routine called on first teletype open.
 * establishes a process group for distribution
 * of quits and interrupts from the tty.
 */
ttopen(tp)
register struct tty *tp;
{
	register struct proc *pp;

	pp = u.u_procp;
	if ((pp->p_pid == pp->p_pgrp)
	 && (u.u_ttyp == NULL)
	 && (tp->t_pgrp == 0)) {
		u.u_ttyp = &tp->t_pgrp;
		tp->t_pgrp = pp->p_pgrp;
	}
	ttioctl(tp, LDOPEN, 0, 0);
	tp->t_state &= ~WOPEN;
	tp->t_state |= ISOPEN;
}

ttclose(tp)
register struct tty *tp;
{
	if ((tp->t_state&ISOPEN) == 0)
		return;
	tp->t_state &= ~ISOPEN;
	tp->t_pgrp = 0;
	ttioctl(tp, LDCLOSE, 0, 0);
}

/*
 * Called from device's read routine after it has
 * calculated the tty-structure given as argument.
 */
ttread(tp)
register struct tty *tp;
{
	register struct clist *tq;

	tq = &tp->t_canq;
	if (tq->c_cc == 0)
		canon(tp);
	while (u.u_count!=0 && u.u_error==0) {
		if (u.u_count >= CBSIZE) {
			register n;
			register struct cblock *cp;

			if ((cp = getcb(tq)) == NULL)
				break;
			n = MIN(u.u_count,
				(unsigned)(cp->c_last - cp->c_first));
			if (copyout((caddr_t)&cp->c_data[cp->c_first],
				(caddr_t)u.u_base, n))
				u.u_error = EFAULT;
			putcf((struct cblock *)cp);
			u.u_base += n;
			u.u_count -= n;
		} else {
			register c;

			if ((c = getc(tq)) < 0)
				break;
			if (subyte(u.u_base++, c))
				u.u_error = EFAULT;
			u.u_count--;
		}
	}
	if (tp->t_state&TBLOCK) {
		if (tp->t_rawq.c_cc<TTXOLO) {
			(*tp->t_proc)(tp, T_UNBLOCK);
		}
	}
}

/*
 * Called from device's write routine after it has
 * calculated the tty-structure given as argument.
 */
ttwrite(tp)
	register struct tty *tp;
{
	register struct cblock *cp;
	register int a;
	register int n;

	if (!(tp->t_state&CARR_ON))
		return;
	a = tthiwat[tp->t_cflag & CBAUD];
	while (u.u_count) {
		/*
		 * If hi water mark is reached, try to start output; then wait
		 * for output to drain to low water mark
		 */
		if (tp->t_outq.c_cc > a) {
			(void) spltty();
			(*tp->t_proc)(tp, T_OUTPUT);
			while (tp->t_outq.c_cc > tthiwat[tp->t_cflag&CBAUD]) {
				tp->t_state |= OASLP;
				sleep((caddr_t)&tp->t_outq, TTOPRI);
			}
			(void) spl0();
		}
		/*
		 * Get a cblock to put the data into.  Wait, if we can't get
		 * one.  This is done since most user code doesn't check
		 * for return values from write.
		 */
		cp = wgetcf();
		n = MIN(u.u_count, cp->c_last);
		if (copyin(u.u_base, cp->c_data, n)) {
			u.u_error = EFAULT;
			putcf(cp);
			break;
		}
		u.u_base += n;
		u.u_count -= n;
		cp->c_last = n;
		ttxput(tp, cp, n);
	}
	(void) spltty();
	(*tp->t_proc)(tp, T_OUTPUT);
	(void) spl0();
}

/*
 * ttuwrite:
 *	- called by the kernel to write to a given users tty
 *	- we ignore high water marks here
 * XXX	it might be a bad idea to sleep here?
 */
ttuwrite(tp, c)
	register struct tty *tp;
	char c;
{
	register struct cblock *cp;
	register int s;

	if (!(tp->t_state&CARR_ON))
		return;
	/*
	 * Get a cblock to put the data into.  Wait, if we can't get
	 * one.  This is done since most user code doesn't check
	 * for return values from write.
	 */
	cp = wgetcf();
	cp->c_data[0] = c;
	cp->c_last = 1;
	ttxput(tp, cp, 1);

	s = spltty();
	(*tp->t_proc)(tp, T_OUTPUT);
	splx(s);
}

/*
 * Place a character on raw TTY input queue, putting in delimiters
 * and waking up top half as needed.
 * Also echo if required.
 */
#define	LCLESC	0400

ttin(tp)
register struct tty *tp;
{
	register c;
	register flg;
	register char *cp;
	ushort nchar, nc;

	nchar = tp->t_rbuf.c_size - tp->t_rbuf.c_count;
	/* reinit rx control block */
	tp->t_rbuf.c_count = tp->t_rbuf.c_size;
	if (nchar==0)
		return;
	flg = tp->t_iflag;
	nc = nchar;
	cp = tp->t_rbuf.c_ptr;
	if (nc < cfreelist.c_size || (flg & (INLCR|IGNCR|ICRNL|IUCLC))) {
			/* must do per character processing */
		for ( ;nc--; cp++) {
			c = *cp;
			if (c == '\n' && flg&INLCR)
				*cp = c = '\r';
			else if (c == '\r')
				if (flg&IGNCR)
					continue;
				else if (flg&ICRNL)
					*cp = c = '\n';
			if (flg&IUCLC && 'A' <= c && c <= 'Z')
				c += 'a' - 'A';
			if (putc(c, &tp->t_rawq))
				continue;
			sysinfo.rawch++;
		}
		cp = tp->t_rbuf.c_ptr;
	} else {
		/* may do block processing */
		putcb(CMATCH((struct cblock *)cp), &tp->t_rawq);
		sysinfo.rawch += nc;
		/* allocate new rx buffer */
		if ((tp->t_rbuf.c_ptr = getcf()->c_data)
			== ((struct cblock *)NULL)->c_data) {
			tp->t_rbuf.c_ptr = NULL;
			return;
		}
		tp->t_rbuf.c_count = cfreelist.c_size;
		tp->t_rbuf.c_size = cfreelist.c_size;
	}


	if (tp->t_rawq.c_cc > TTXOHI) {
		if (flg&IXOFF && !(tp->t_state&TBLOCK))
			(*tp->t_proc)(tp, T_BLOCK);
		if (tp->t_rawq.c_cc > TTYHOG) {
			ttyflush(tp, FREAD);
			return;
		}
	}
	flg = lobyte(tp->t_lflag);
	if (tp->t_outq.c_cc > (tthiwat[tp->t_cflag&CBAUD] + TTECHI))
		flg &= ~(ECHO|ECHOK|ECHONL|ECHOE);
	if (flg) while (nchar--) {
		c = *cp++;
		if (flg&ISIG) {
			if (c == tp->t_cc[VINTR]) {
				signal(tp->t_pgrp, SIGINT);
				if (!(flg&NOFLSH))
					ttyflush(tp, (FREAD|FWRITE));
				continue;
			}
			if (c == tp->t_cc[VQUIT]) {
				signal(tp->t_pgrp, SIGQUIT);
				if (!(flg&NOFLSH))
					ttyflush(tp, (FREAD|FWRITE));
				continue;
			}
		}
		if (flg&ICANON) {
			if (c == '\n') {
				if (flg&ECHONL)
					flg |= ECHO;
				tp->t_delct++;
			} else if (c == tp->t_cc[VEOL] || c == tp->t_cc[VEOL2])
				tp->t_delct++;
			if (!(tp->t_state&CLESC)) {
				if (c == '\\')
					tp->t_state |= CLESC;
				if (c == tp->t_cc[VERASE] && flg&ECHOE) {
					if (flg&ECHO)
						ttxput(tp, '\b', 0);
					flg |= ECHO;
					ttxput(tp, ' ', 0);
					c = '\b';
				} else if (c == tp->t_cc[VKILL] && flg&ECHOK) {
					if (flg&ECHO)
						ttxput(tp, c, 0);
					flg |= ECHO;
					c = '\n';
				} else if (c == tp->t_cc[VEOF]) {
					flg &= ~ECHO;
					tp->t_delct++;
				}
			} else {
				if (c != '\\' || (flg&XCASE))
					tp->t_state &= ~CLESC;
			}
		}
		if (flg&ECHO) {
			ttxput(tp, c, 0);
			(*tp->t_proc)(tp, T_OUTPUT);
		}
	}
	if (!(flg&ICANON)) {
		tp->t_state &= ~RTO;
		if (tp->t_rawq.c_cc >= tp->t_cc[VMIN])
			tp->t_delct = 1;
		else if (tp->t_cc[VTIME]) {
			if (!(tp->t_state&TACT))
				tttimeo(tp);
		}
	}
	if (tp->t_delct && (tp->t_state&IASLP)) {
		tp->t_state &= ~IASLP;
		wakeup((caddr_t)&tp->t_rawq);
	}
}

/*
 * Scan a list of characters and assure that they require no
 * post processing
 */
ttxchk(ncode, cp)
register short ncode;
register unsigned char *cp;
{
	register c, n;

	n = 0;
	ncode--;
	do {
		c = *cp++;
		if (c & 0200)
			return(-1);
		c = partab[c] & 077;
		if (c == 0)
			n++;
		else if (c != 1)
			return(-1);
	} while (--ncode != -1);
	return(n);
}

/*
 * Put character(s) on TTY output queue, adding delays,
 * expanding tabs, and handling the CR/NL bit.
 * It is called both from the base level for output, and from
 * interrupt level for echoing.
 */
/* VARARGS1 */
ttxput(tp, ucp, ncode)
register struct tty *tp;
register ncode;
union {
	struct ch {		/*  machine dependent union */
		char dum[3];
		unsigned char theaddr;
	} ch;
	int thechar;
	struct cblock *ptr;
} ucp;
{
	register struct clist *outqp;
	register unsigned char *cp;
	register c, flg, ctype;
	register char *colp;
	struct cblock *scf;
	int cs;

	flg = tp->t_oflag;
	outqp = &tp->t_outq;
	if (ncode == 0) {
		if (!(flg&OPOST)) {
			sysinfo.outch++;
			(void) putc(ucp.thechar, outqp);
			return;
		}
		ncode++;
		cp = (unsigned char *)&ucp.ch.theaddr;
		scf = NULL;
	} else {
		if (!(flg&OPOST)) {
			sysinfo.outch += ncode;
#ifdef	notdef
			putcb(ucp.ptr, outqp);
#else
			aputcb(ucp.ptr, outqp);
#endif
			return;
		}
		cp = (unsigned char *)&ucp.ptr->c_data[ucp.ptr->c_first];
		scf = ucp.ptr;
	}
	if ((tp->t_lflag&XCASE)==0 && (flg&OLCUC)==0) {
		colp = &tp->t_col;
		if (ncode > 1 && (c = ttxchk(ncode, cp)) >= 0) {
			(*colp) += c;
			sysinfo.outch += ncode;
			putcb(ucp.ptr, outqp);
			return;
		}
		while (ncode--) {
			ctype = partab[c = *cp++] & 077;
			if (ctype==0) {
				sysinfo.outch++;
				(*colp)++;
				(void) putc(c, outqp);
				continue;
			}
			else if (ctype==1) {
				sysinfo.outch++;
				(void) putc(c, outqp);
				continue;
			}
			if (c >= 0200) {
				sysinfo.outch++;
				if (c == QESC)
					(void) putc(QESC, outqp);
				(void) putc(c, outqp);
				continue;
			}
			cs = c;
			/*
			 * Calculate delays.
			 * The numbers here represent clock ticks
			 * and are not necessarily optimal for all terminals.
			 * The delays are indicated by characters above 0200.
			 */
			c = 0;
			switch (ctype) {

			case 0:	/* ordinary */
				(*colp)++;

			case 1:	/* non-printing */
				break;

			case 2:	/* backspace */
				if (flg&BSDLY)
					c = 2;
				if (*colp)
					(*colp)--;
				break;

			case 3:	/* line feed */
				if (flg&ONLRET)
					goto qcr;
				if (flg&ONLCR) {
					if (!(flg&ONOCR && *colp==0)) {
						sysinfo.outch++;
						(void) putc('\r', outqp);
					}
					goto qcr;
				}
			qnl:
				if (flg&NLDLY)
					c = 5;
				break;

			case 4:	/* tab */
				c = 8 - ((*colp)&07);
				*colp += c;
				ctype = flg&TABDLY;
				if (ctype == TAB0) {
					c = 0;
				} else if (ctype == TAB1) {
					if (c < 5)
						c = 0;
				} else if (ctype == TAB2) {
					c = 2;
				} else if (ctype == TAB3) {
					sysinfo.outch += c;
					do
						(void) putc(' ', outqp);
					while (--c);
					continue;
				}
				break;

			case 5:	/* vertical tab */
				if (flg&VTDLY)
					c = 0177;
				break;

			case 6:	/* carriage return */
				if (flg&OCRNL) {
					cs = '\n';
					goto qnl;
				}
				if (flg&ONOCR && *colp == 0)
					continue;
			qcr:
				ctype = flg&CRDLY;
				if (ctype == CR1) {
					if (*colp)
						c = MAX((unsigned)((*colp>>4) + 3), 6);
				} else if (ctype == CR2) {
					c = 5;
				} else if (ctype == CR3) {
					c = 9;
				}
				*colp = 0;
				break;

			case 7:	/* form feed */
				if (flg&FFDLY)
					c = 0177;
				break;
			}
			sysinfo.outch++;
			(void) putc(cs, outqp);
			if (c) {
				if ((c < 32) && flg&OFILL) {
					if (flg&OFDEL)
						cs = 0177;
					else
						cs = 0;
					(void) putc(cs, outqp);
					if (c > 3)
						(void) putc(cs, outqp);
				} else {
					(void) putc(QESC, outqp);
					(void) putc(c|0200, outqp);
				}
			}
		}
	} else
	while (ncode--) {
		c = *cp++;
		if (c >= 0200) {
	/* spl5-0 */
			if (c == QESC)
				(void) putc(QESC, outqp);
			sysinfo.outch++;
			(void) putc(c, outqp);
			continue;
		}
		/*
		 * Generate escapes for upper-case-only terminals.
		 */
		if (tp->t_lflag&XCASE) {
			colp = "({)}!|^~'`\\\\";
			while(*colp++)
				if (c == *colp++) {
					ttxput(tp, '\\'|0200, 0);
					c = colp[-2];
					break;
				}
			if ('A' <= c && c <= 'Z')
				ttxput(tp, '\\'|0200, 0);
		}
		if (flg&OLCUC && 'a' <= c && c <= 'z')
			c += 'A' - 'a';
		cs = c;
		/*
		 * Calculate delays.
		 * The numbers here represent clock ticks
		 * and are not necessarily optimal for all terminals.
		 * The delays are indicated by characters above 0200.
		 */
		ctype = partab[c];
		colp = &tp->t_col;
		c = 0;
		switch (ctype&077) {

		case 0:	/* ordinary */
			(*colp)++;

		case 1:	/* non-printing */
			break;

		case 2:	/* backspace */
			if (flg&BSDLY)
				c = 2;
			if (*colp)
				(*colp)--;
			break;

		case 3:	/* line feed */
			if (flg&ONLRET)
				goto cr;
			if (flg&ONLCR) {
				if (!(flg&ONOCR && *colp==0)) {
					sysinfo.outch++;
					(void) putc('\r', outqp);
				}
				goto cr;
			}
		nl:
			if (flg&NLDLY)
				c = 5;
			break;

		case 4:	/* tab */
			c = 8 - ((*colp)&07);
			*colp += c;
			ctype = flg&TABDLY;
			if (ctype == TAB0) {
				c = 0;
			} else if (ctype == TAB1) {
				if (c < 5)
					c = 0;
			} else if (ctype == TAB2) {
				c = 2;
			} else if (ctype == TAB3) {
				sysinfo.outch += c;
				do
					(void) putc(' ', outqp);
				while (--c);
				continue;
			}
			break;

		case 5:	/* vertical tab */
			if (flg&VTDLY)
				c = 0177;
			break;

		case 6:	/* carriage return */
			if (flg&OCRNL) {
				cs = '\n';
				goto nl;
			}
			if (flg&ONOCR && *colp == 0)
				continue;
		cr:
			ctype = flg&CRDLY;
			if (ctype == CR1) {
				if (*colp)
					c = MAX((unsigned)((*colp>>4) + 3), 6);
			} else if (ctype == CR2) {
				c = 5;
			} else if (ctype == CR3) {
				c = 9;
			}
			*colp = 0;
			break;

		case 7:	/* form feed */
			if (flg&FFDLY)
				c = 0177;
			break;
		}
		sysinfo.outch++;
		(void) putc(cs, outqp);
		if (c) {
			if ((c < 32) && flg&OFILL) {
				if (flg&OFDEL)
					cs = 0177;
				else
					cs = 0;
				(void) putc(cs, outqp);
				if (c > 3)
					(void) putc(cs, outqp);
			} else {
				(void) putc(QESC, outqp);
				(void) putc(c|0200, outqp);
			}
		}

	}
	if (scf != NULL)
		putcf(scf);
}

/*
 * Get next packet from output queue.
 * Called from xmit interrupt complete.
 */

ttout(tp)
register struct tty *tp;
{
	register struct ccblock *tbuf;
	register c;
	register char *cptr;
	register retval;
	register struct clist *outqp;
	extern ttrstrt();

	outqp = &tp->t_outq;
	if (tp->t_state&TTIOW && outqp->c_cc==0) {
		tp->t_state &= ~TTIOW;
		wakeup((caddr_t)&tp->t_oflag);
	}
delay:
	tbuf = &tp->t_tbuf;
	if (hibyte(tp->t_lflag)) {
		if (tbuf->c_ptr) {
			putcf(CMATCH((struct cblock *)tbuf->c_ptr));
			tbuf->c_ptr = NULL;
		}
		tp->t_state |= TIMEOUT;
		timeout(ttrstrt, (caddr_t)tp,
			(int)((hibyte(tp->t_lflag)&0177)+6));
		hibyte(tp->t_lflag) = 0;
		return(0);
	}
	retval = 0;
	if (!(tp->t_oflag&OPOST)) {
		if (tbuf->c_ptr)
			putcf(CMATCH((struct cblock *)tbuf->c_ptr));
		if ((tbuf->c_ptr = (char *)getcb(outqp)) == NULL)
			goto out;
			/* return(0); */
		tbuf->c_count = ((struct cblock *)tbuf->c_ptr)->c_last -
				((struct cblock *)tbuf->c_ptr)->c_first;
		tbuf->c_size = tbuf->c_count;
		tbuf->c_ptr = &((struct cblock *)tbuf->c_ptr)->c_data
				[((struct cblock *)tbuf->c_ptr)->c_first];
		retval = CPRES;
	} else {		/* watch for timing	*/
		if (tbuf->c_ptr == NULL) {
			if ((tbuf->c_ptr = getcf()->c_data)
				== ((struct cblock *)NULL)->c_data) {
				tbuf->c_ptr = NULL;
				goto out;
				/* return(0);	/* Add restart? */
			}
		}
		tbuf->c_count = 0;
		cptr = tbuf->c_ptr;
		while ((c=getc(outqp)) >= 0) {
			if (c == QESC) {
				if ((c = getc(outqp)) < 0)
					break;
				if (c > 0200) {
					hibyte(tp->t_lflag) = c;
					if (!retval)
						goto delay;
					break;
				}
			}
			retval = CPRES;
			*cptr++ = c;
			tbuf->c_count++;
			/*
			 * don't spend too long at high priority --
			 * will miss serial interrupts -- try dividing
			 * limit by 4  CSK 5/22/85
			 */
			if (tbuf->c_count >= cfreelist.c_size >> 2)
				break;		
		}
		tbuf->c_size = tbuf->c_count;
	}
out:
	if (tp->t_state&OASLP &&
		outqp->c_cc<=ttlowat[tp->t_cflag&CBAUD]) {
		tp->t_state &= ~OASLP;
		wakeup((caddr_t)outqp);
	}
	return(retval);
}

tttimeo(tp)
register struct tty *tp;
{
	tp->t_state &= ~TACT;
	if (tp->t_lflag&ICANON || tp->t_cc[VTIME] == 0)
		return;
	if (tp->t_rawq.c_cc == 0 && tp->t_cc[VMIN])
		return;
	if (tp->t_state&RTO) {
		tp->t_delct = 1;
		if (tp->t_state&IASLP) {
			tp->t_state &= ~IASLP;
			wakeup((caddr_t)&tp->t_rawq);
		}
	} else {
		tp->t_state |= RTO|TACT;
		timeout(tttimeo, (caddr_t)tp,
			(int)(tp->t_cc[VTIME]*(short)((short)hz/10)));
	}
}

/*
 * I/O control interface
 */
/* ARGSUSED */
ttioctl(tp, cmd, arg, mode)
register struct tty *tp;
{
	ushort	chg;

	switch(cmd) {
	case LDOPEN:
		if (tp->t_rbuf.c_ptr == NULL) {
			/* allocate RX buffer */
			tp->t_rbuf.c_ptr = wgetcf()->c_data;
			tp->t_rbuf.c_count = cfreelist.c_size;
			tp->t_rbuf.c_size  = cfreelist.c_size;
			(*tp->t_proc)(tp, T_INPUT);
		}
		break;

	case LDCLOSE:
		spltty();
		(*tp->t_proc)(tp, T_RESUME);
		spl0();
		ttywait(tp);
		ttyflush(tp, (FREAD|FWRITE));
		if (tp->t_tbuf.c_ptr) {
			putcf(CMATCH((struct cblock *)tp->t_tbuf.c_ptr));
			tp->t_tbuf.c_ptr = NULL;
			tp->t_tbuf.c_count = 0;
			tp->t_tbuf.c_size = 0;
		}
		if (tp->t_rbuf.c_ptr) {
			putcf(CMATCH((struct cblock *)tp->t_rbuf.c_ptr));
			tp->t_rbuf.c_ptr = NULL;
			tp->t_rbuf.c_count = 0;
			tp->t_rbuf.c_size = 0;
		}
		tp->t_tmflag = 0;
		break;

	case LDCHG:
		chg = tp->t_lflag^arg;
		if (!(chg&ICANON))
			break;
		spltty();
		if (tp->t_canq.c_cc) {
			if (tp->t_rawq.c_cc) {
				tp->t_canq.c_cc += tp->t_rawq.c_cc;
				tp->t_canq.c_cl->c_next = tp->t_rawq.c_cf;
				tp->t_canq.c_cl = tp->t_rawq.c_cl;
			}
			tp->t_rawq = tp->t_canq;
			tp->t_canq = ttnulq;
		}
		tp->t_delct = tp->t_rawq.c_cc;
		spl0();
		break;

	default:
		break;
	}
}
