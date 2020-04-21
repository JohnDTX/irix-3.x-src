/*
 * $Source: /d2/3.7/src/sys/sys/RCS/tty_subr.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:35:45 $
 */
#include "../h/param.h"
#include "../h/systm.h"
#include "../h/user.h"
#include "../h/tty.h"
#include "../h/proc.h"
#include "../h/file.h"
#include "../h/conf.h"
#include "../h/termio.h"
#include "../h/sysinfo.h"

/*
 * general TTY subroutines
 */

extern int sspeed;
extern int tthiwat[];
extern int ttlowat[];
extern char ttcchar[];

/* null clist header */
struct clist ttnulq;

/*
 * Input mapping table-- if an entry is non-zero, when the
 * corresponding character is typed preceded by "\" the escape
 * sequence is replaced by the table value.  Mostly used for
 * upper-case only terminals.
 */
char	maptab[] = {
	000,000,000,000,000,000,000,000,
	000,000,000,000,000,000,000,000,
	000,000,000,000,000,000,000,000,
	000,000,000,000,000,000,000,000,
	000,'|',000,000,000,000,000,'`',
	'{','}',000,000,000,000,000,000,
	000,000,000,000,000,000,000,000,
	000,000,000,000,000,000,000,000,
	000,000,000,000,000,000,000,000,
	000,000,000,000,000,000,000,000,
	000,000,000,000,000,000,000,000,
	000,000,000,000,000,000,'~',000,
	000,'A','B','C','D','E','F','G',
	'H','I','J','K','L','M','N','O',
	'P','Q','R','S','T','U','V','W',
	'X','Y','Z',000,000,000,000,000,
};

/*
 * common ioctl tty code
 */
ttiocom(tp, cmd, arg, mode)
register struct tty *tp;
{
	register short flag;
	register struct termio *cbp;
	struct termio cb;

	switch(cmd) {
	case IOCTYPE:
		u.u_rval1 = TIOC;
		break;

	case TCSETAW:
	case TCSETAF:
		ttywait(tp);
		if (cmd == TCSETAF)
			ttyflush(tp, (FREAD|FWRITE));
	case TCSETA:
		cbp = &cb;
		if (copyin((caddr_t)arg, (caddr_t)cbp, sizeof cb)) {
			u.u_error = EFAULT;
			break;
		}
		if (tp->t_line != cbp->c_line) {
			if (cbp->c_line < 0 || cbp->c_line >= linecnt) {
				u.u_error = EINVAL;
				break;
			}
			(*linesw[tp->t_line].l_ioctl)(tp, LDCLOSE, 0, mode);
		}
		flag = tp->t_lflag;
		tp->t_iflag = cbp->c_iflag;
		tp->t_oflag = cbp->c_oflag;
		tp->t_cflag = cbp->c_cflag;
		tp->t_lflag = cbp->c_lflag;
		bcopy((caddr_t)cbp->c_cc, (caddr_t)tp->t_cc, NCC);
		if (tp->t_line != cbp->c_line) {
			tp->t_line = cbp->c_line;
			(*linesw[tp->t_line].l_ioctl)(tp, LDOPEN, 0, mode);
		} else if (tp->t_lflag != flag) {
			(*linesw[tp->t_line].l_ioctl)(tp, LDCHG, flag, mode);
		}
		return(1);

	case TCGETA:
		cbp = &cb;
		cbp->c_iflag = tp->t_iflag;
		cbp->c_oflag = tp->t_oflag;
		cbp->c_cflag = tp->t_cflag;
		cbp->c_lflag = tp->t_lflag;
		cbp->c_line = tp->t_line;
		bcopy((caddr_t)tp->t_cc, (caddr_t)cbp->c_cc, NCC);
		if (copyout((caddr_t)cbp, (caddr_t)arg, sizeof cb))
			u.u_error = EFAULT;
		break;

	case TCSBRK:
		ttywait(tp);
		if (arg == 0)
			(*tp->t_proc)(tp, T_BREAK);
		break;

	case TCXONC:
		switch (arg) {
		case 0:
			(*tp->t_proc)(tp, T_SUSPEND);
			break;
		case 1:
			(*tp->t_proc)(tp, T_RESUME);
			break;
		case 2:
			(*tp->t_proc)(tp, T_BLOCK);
			break;
		case 3:
			(*tp->t_proc)(tp, T_UNBLOCK);
			break;
		default:
			u.u_error = EINVAL;
		}
		break;

	case TCFLSH:
		switch (arg) {
		case 0:
		case 1:
		case 2:
			ttyflush(tp, (arg - FOPEN)&(FREAD|FWRITE));
			break;

		default:
			u.u_error = EINVAL;
		}
		break;

	/*
	 * The following ioctls were added by UniSoft
	 */

	/*
	 * Return number of characters immediately available.
	 */
	case oFIONREAD:
	case FIONREAD: {
		off_t nread;

		spl6();
		while (tp->t_rawq.c_cc && tp->t_delct)
			canon(tp);
		spl0();
		nread = tp->t_canq.c_cc;
		if (!(tp->t_lflag &ICANON))
			nread += tp->t_rawq.c_cc;
		if (copyout((caddr_t)&nread, (caddr_t)arg, sizeof (off_t)))
			u.u_error = EFAULT;
		break;
		}

	default:
		if ((cmd&IOCTYPE) == LDIOC)
			(*linesw[tp->t_line].l_ioctl)(tp, cmd, arg, mode);
		else
			u.u_error = EINVAL;
		break;
	}
	return(0);
}

ttinit(tp)
	register struct tty *tp;
{
	tp->t_line = 0;
	tp->t_iflag = ICRNL | ISTRIP;
	tp->t_oflag = OPOST | ONLCR | TAB3;
	tp->t_lflag = ISIG | ICANON | ECHO | ECHOK;
	tp->t_cflag = SSPEED | CS8 | CREAD | HUPCL;
	bcopy((caddr_t)ttcchar, (caddr_t)tp->t_cc, NCC);
}

ttywait(tp)
register struct tty *tp;
{
	spltty();
	while (tp->t_outq.c_cc || (tp->t_state&(BUSY|TIMEOUT))) {
		tp->t_state |= TTIOW;
		(void) sleep((caddr_t)&tp->t_oflag, TTOPRI);
	}
	spl0();
	delay(hz>>4);
}

/*
 * flush TTY queues
 */
ttyflush(tp, cmd)
register struct tty *tp;
{
	register struct cblock *cp;
	register s;

	if (cmd&FWRITE) {
		while ((cp = getcb(&tp->t_outq)) != NULL)
			putcf(cp);
		(*tp->t_proc)(tp, T_WFLUSH);
		if (tp->t_state&OASLP) {
			tp->t_state &= ~OASLP;
			wakeup((caddr_t)&tp->t_outq);
		}
		if (tp->t_state&TTIOW) {
			tp->t_state &= ~TTIOW;
			wakeup((caddr_t)&tp->t_oflag);
		}
	}
	if (cmd&FREAD) {
		while ((cp = getcb(&tp->t_canq)) != NULL)
			putcf(cp);
		s = spltty();
		while ((cp = getcb(&tp->t_rawq)) != NULL)
			putcf(cp);
		tp->t_delct = 0;
		splx(s);
		(*tp->t_proc)(tp, T_RFLUSH);
		if (tp->t_state&IASLP) {
			tp->t_state &= ~IASLP;
			wakeup((caddr_t)&tp->t_rawq);
		}
	}
}

/*
 * Transfer raw input list to canonical list,
 * doing erase-kill processing and handling escapes.
 */
canon(tp)
register struct tty *tp;
{
	register char *bp;
	register c, esc;
	char	canonb[CANBSIZ];		/* canon buffer */


	spltty();
	if (tp->t_rawq.c_cc == 0)
		tp->t_delct = 0;
	while (tp->t_delct == 0) {
		if (!(tp->t_state&CARR_ON) || (u.u_fmode&FNDELAY)) {
			spl0();
			return;
		}
		if (!(tp->t_lflag&ICANON) && tp->t_cc[VMIN]==0) {
			if (tp->t_cc[VTIME]==0)
				break;
			tp->t_state &= ~RTO;
			if (!(tp->t_state&TACT))
				tttimeo(tp);
		}
		tp->t_state |= IASLP;
		(void) sleep((caddr_t)&tp->t_rawq, TTIPRI);
	}
	if (!(tp->t_lflag&ICANON)) {
		if (tp->t_canq.c_cc == 0) {
			tp->t_canq = tp->t_rawq;
			tp->t_rawq = ttnulq;
		} else
			while (tp->t_rawq.c_cc)
				(void) putc(getc(&tp->t_rawq), &tp->t_canq);
		tp->t_delct = 0;
		spl0();
		return;
	}
	spl0();
	bp = canonb;
	esc = 0;
	while ((c=getc(&tp->t_rawq)) >= 0) {
		if (!esc) {
			if (c == '\\') {
				esc++;
			} else if (c == tp->t_cc[VERASE]) {
				if (bp > canonb)
					bp--;
				continue;
			} else if (c == tp->t_cc[VKILL]) {
				bp = canonb;
				continue;
			} else if (c == tp->t_cc[VEOF]) {
				break;
			}
		} else {
			esc = 0;
			if (c == tp->t_cc[VERASE] ||
			    c == tp->t_cc[VKILL] ||
			    c == tp->t_cc[VEOF])
				bp--;
			else if (tp->t_lflag&XCASE) {
				if ((c < 0200) && maptab[c]) {
					bp--;
					c = maptab[c];
				} else if (c == '\\')
					continue;
			} else if (c == '\\')
				esc++;
		}
		*bp++ = c;
		if (c == '\n' || c == tp->t_cc[VEOL] || c == tp->t_cc[VEOL2])
			break;
		if (bp >= &canonb[CANBSIZ])
			bp--;
	}
	tp->t_delct--;
	c = bp - canonb;
	sysinfo.canch += c;
	bp = canonb;
/* faster copy ? */
	while (c--)
		(void) putc(*bp++, &tp->t_canq);
	return;
}

/*
 * Restart typewriter output following a delay timeout.
 * The name of the routine is passed to the timeout
 * subroutine and it is called during a clock interrupt.
 */
ttrstrt(tp)
register struct tty *tp;
{
	(*tp->t_proc)(tp, T_TIME);
}
