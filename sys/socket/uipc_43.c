/* these are miscellaneous functions used by the 4.3bsd uipc files, but
 *	found in other parts of 4.3
 *
 * $Source: /d2/3.7/src/sys/socket/RCS/uipc_43.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:34:34 $
 */

#ifdef SVR3
#include "../tcp-param.h"
#include "sys/sysmacros.h"
#include "sys/param.h"
#include "sys/systm.h"
#include "sys/signal.h"
#include "sys/sbd.h"
#include "sys/pcb.h"
#include "sys/immu.h"
#include "sys/region.h"
#include "sys/fs/s5dir.h"
#include "sys/user.h"
#include "sys/proc.h"
#include "sys/file.h"
#include "sys/inode.h"
#include "sys/time.h"
#include "sys/mbuf.h"
#include "sys/uio.h"
#include "sys/socketvar.h"
#include "sys/poll.h"
#include "sys/errno.h"
#include "sys/var.h"
#include "sys/pda.h"
#include "sys/debug.h"
#define UNTIMEOUT untimeout_func
#define splhigh() splhi()
#else
#include "../h/param.h"
#include "../h/systm.h"
#include "../h/user.h"
#include "../h/proc.h"
#include "../h/file.h"
#include "../h/inode.h"
#include "../bsd/time.h"
#include "../h/mbuf.h"
#include "../h/uio.h"
#include "../h/socketvar.h"
#include "../h/poll.h"
#define UNTIMEOUT untimeout
#define splhigh() splmax()
#endif
#include "../net/soioctl.h"
#include "values.h"

#define selwait pollwait		/* combine streams & select wait */
extern int selwait;

extern short soc_fstyp;

uiomove(cp, n, rw, uio)
	register caddr_t cp;
	register int n;
	enum uio_rw rw;
	register struct uio *uio;
{
	register struct iovec *iov;
	u_int cnt;
	int error = 0;

	while (n > 0 && uio->uio_resid) {
		iov = uio->uio_iov;
		cnt = iov->iov_len;
		if (cnt == 0) {
			uio->uio_iov++;
			uio->uio_iovcnt--;
			continue;
		}
		if (cnt > n)
			cnt = n;
		switch (uio->uio_segflg) {

		case UIO_USERSPACE:
		case UIO_USERISPACE:
			if (rw == UIO_READ)
				error = copyout(cp, iov->iov_base, cnt);
			else
				error = copyin(iov->iov_base, cp, cnt);
			if (error)
				return (error);
			break;

		case UIO_SYSSPACE:
			if (rw == UIO_READ)
				bcopy((caddr_t)cp, iov->iov_base, cnt);
			else
				bcopy(iov->iov_base, (caddr_t)cp, cnt);
			break;
		}
		iov->iov_base += cnt;
		iov->iov_len -= cnt;
		uio->uio_resid -= cnt;
		uio->uio_offset += cnt;
		cp += cnt;
		n -= cnt;
	}
	return (error);
}

#ifndef SVR3
struct proc *
pfind(pid)
	int pid;
{
	register struct proc *p;

	/* SLOW... */
	for (p = proc; p < procNPROC; p++)
		if (p->p_pid == pid)
			return (p);
	return ((struct proc *)0);
}
#endif


long	hostid;				/* XXX move this... */

gethostid()
{

	u.u_rval1 = hostid;
}

sethostid()
{
	struct a {
		int	hostid;
	} *uap = (struct a *)u.u_ap;

	if (suser())
		hostid = uap->hostid;
}



/* called as a time-out function to stop selecting
 *	Just checks to see if target process is (still) on the sleep queue.
 */
static
unselect(p)
register struct proc *p;
{
	selwakeup(p,0);
}


int	nselcoll;

/*
 * Select system call.
 *
 * TODO:
 *	Implement FS_POLL and clean up selscan()
 *	Use a semaphore and wsema in SVR3
 *	Move to os/poll.c (SVR3) and sys/sys2.c (SVR0)
 *	Revise streams to obviate strunscan()?
 */
select()
{
	register struct uap  {
		int	nd;
		fd_set	*in, *ou, *ex;
		struct	timeval *tv;
	} *uap = (struct uap *)u.u_ap;
	fd_set ibits[3], obits[3];
	struct timeval atv;
	time_t ticks;
	int s, ncoll, ni;

	bzero((caddr_t)ibits, sizeof(ibits));
	bzero((caddr_t)obits, sizeof(obits));
#ifdef SVR3
	if (uap->nd > v.v_nofiles)
		uap->nd = v.v_nofiles;	/* forgiving, if slightly wrong */
#else
	if (uap->nd > NOFILE)
		uap->nd = NOFILE;	/* forgiving, if slightly wrong */
#endif
	ni = howmany(uap->nd, NFDBITS);

#define	getbits(name, x) \
	if (uap->name) { \
		if (u.u_error = copyin((caddr_t)uap->name, (caddr_t)&ibits[x],\
			(unsigned)(ni * sizeof(fd_mask)))) return; \
	}
	getbits(in, 0);
	getbits(ou, 1);
	getbits(ex, 2);
#undef	getbits

	if (uap->tv) {
		u.u_error = copyin((caddr_t)uap->tv, (caddr_t)&atv,
			sizeof (atv));
		if (u.u_error)
			return;
		if (atv.tv_sec < 0
		    || atv.tv_sec >= MAXLONG/HZ-1
		    || atv.tv_usec < 0
		    || atv.tv_usec >= 1000000) {
			u.u_error = EINVAL;
			return;
		}
		ticks = atv.tv_sec*HZ + ((atv.tv_usec*HZ + (HZ/2))/1000000);
		if (ticks != 0) {
			s = splhigh();
			timeout(unselect, (caddr_t)u.u_procp, ticks);
			ticks += lbolt;
			splx(s);
		}
	} else {
		ticks = MAXLONG;
	}
#ifdef SVR3
	ASSERT(private.p_cpuid == 0);
#endif

	for (;;) {
		ncoll = nselcoll;
#ifdef SVR3
		lcksig(u.u_procp);
#endif
		u.u_procp->p_flag |= SSEL;
#ifdef SVR3
		unlcksig(u.u_procp);
#endif
		u.u_rval1 = selscan(ibits, obits, uap->nd);
		s = splhigh();
		if (u.u_error || u.u_rval1)
			break;
		if (ticks <= lbolt) 	/* quit when users time expires */
			break;
#ifdef SVR3
		spsema(u.u_procp->p_siglck);
#endif
		if ((u.u_procp->p_flag & SSEL) == 0 || nselcoll != ncoll) {
#ifdef SVR3
			svsema(u.u_procp->p_siglck);
#endif
			splx(s);
			continue;
		}
		u.u_procp->p_flag &= ~SSEL;
#ifdef SVR3
		svsema(u.u_procp->p_siglck);
#endif
		if (sleep((caddr_t)&selwait, (PZERO+1)|PCATCH)) {
			u.u_error = EINTR;
			break;
		}
		splx(s);
	}
	if (ticks < MAXLONG && ticks > lbolt)
		UNTIMEOUT(unselect, (caddr_t)u.u_procp);
	splx(s);

#define	putbits(name, x) \
	if (uap->name) { \
		register int error = copyout((caddr_t)&obits[x], \
					     (caddr_t)uap->name, \
					(unsigned)(ni * sizeof(fd_mask))); \
		if (error) \
			u.u_error = error; \
	}
	if (u.u_error == 0) {
		putbits(in, 0);
		putbits(ou, 1);
		putbits(ex, 2);
#undef putbits
	}

	strunscan(ibits, uap->nd);
}


/* tell the streams to stop worrying about polling
 */
static
strunscan(ibits, nfd)
fd_set *ibits;
int nfd;
{
	register int i, j;
	register fd_mask all;
	register struct file *fp;

	i = 0; j = 0;
	for (;;) {
		all = (ibits[0].fds_bits[i] | ibits[1].fds_bits[i] |
		       ibits[2].fds_bits[i]);
		for (;;) {
			if (j > nfd)
				return;

			if (all & 1) {
				register struct inode *ip;
				fp = u.u_ofile[j];
				if (NULL != fp
				    && NULL != (ip = fp->f_inode)->i_sptr
				    && IFCHR == ip->i_ftype)
					pollreset(ip->i_sptr);
			}
			all >>= 1;
			if (!all) {
				j = (++i)*NFDBITS;
				if (j >= nfd)
					return;
				break;
			}
			j++;
		}
	}
}

/* scan the files, looking for one that has had something happen to it
 *	This function can poll sockets or streams.  Unfortunately, they
 *	use different mechanisms and data structures.  One could have changed
 *	either to use the others, but...
 *
 *	Therefore, this function has changed substantially.  It scans the bit
 *	masks in parallel, rather than one after the other.
 */
static int
selscan(ibits, obits, nfd)
	fd_set *ibits, *obits;
	int nfd;
{
	register int i;			/* index among words */
	register int j;			/* index among all bits */
	fd_mask in, ou, ex;
	register fd_mask all, bit;
	register struct inode *ip;
	register struct file *fp;
	int n;				/* # of FDs found */
	extern dev_t queue_dev;

	n = 0;				/* start with nothing found */
	i = 0;				/* start scan at the 1st word */
	j = 0;				/* and at 1st bit among all words */
	do {
		in = ibits[0].fds_bits[i];
		ou = ibits[1].fds_bits[i];
		ex = ibits[2].fds_bits[i];
		for (all = in|ou|ex,	/* get all bits of interest in word */
		     bit = 1;		/* start at the 'right' */
		     all != 0		/* scan word until boring */
		       && j < nfd;	/* & until last valid bit */
		     all >>= 1,		/* advance to the next bit */
		     bit <<= 1,	
		     j++) {
			if ((all & 1) == 0)
				continue;
			fp = u.u_ofile[j];
			if (fp == NULL) {
				u.u_error = EBADF;
				return (-1);
			}
			ip = fp->f_inode;
			if (ip->i_fstyp == soc_fstyp) {
				register struct socket *so;
				so = soc_fsptr(ip);
				if ((bit & in)
				    && soc_select(so,FREAD)) {
					obits[0].fds_bits[i] |= bit;
					n++;
				}
				if ((bit & ou)
				    && soc_select(so,FWRITE)) {
					obits[1].fds_bits[i] |= bit;
					n++;
				}
				if ((bit & ex)
				    && soc_select(so,0)) {
					obits[2].fds_bits[i] |= bit;
					n++;
				}
				continue;
			}
			switch (ip->i_ftype) {
			  case IFCHR: {
				register int revents;
				register short events;

#if !defined(KOPT_NOGL) && !defined(GL1)
				/*
				 * Special case code for graphics queue device.
				 */
				if (ip->i_rdev == queue_dev) {
				    /*
				     * See if graphics queue for this
				     * process is not empty.  If so,
				     * then this descriptor returns true.
				     */
				    if (gr_queuenotempty(u.u_procp) > 0) {
					if (bit & in) {
						obits[0].fds_bits[i] |= bit;
						n++;
					}
					if (bit & ou) {
						obits[1].fds_bits[i] |= bit;
						n++;
					}
				    }
				    continue;
				}
#endif
				if (ip->i_sptr == 0)
					goto notsock;
				events = 0;
				if (bit & in)
					events |= POLLIN;
				if (bit & ou)
					events |= POLLOUT;
				revents = strpoll(ip->i_sptr, events, n);
				if (revents & POLLIN) {
					obits[0].fds_bits[i] |= bit;
					n++;
				}
				if (revents & POLLOUT) {
					obits[1].fds_bits[i] |= bit;
					n++;
				}
				if ((bit & ex)
				    && (revents & POLLHUP)) {
					obits[2].fds_bits[i] |= bit;
					n++;
				}
				if (u.u_error)
					return (-1);
				break;
			  }
			  case IFREG:
			  case IFDIR:
			  case IFLNK:
				if (bit & in) {
					obits[0].fds_bits[i] |= bit;
					n++;
				}
				if (bit & ou) {
					obits[1].fds_bits[i] |= bit;
					n++;
				}
				break;
			  case IFIFO: {
				auto short events = 0;

				if (bit & in)
					events = POLLIN;
				if (bit & ou)
					events |= POLLOUT;
				if (pipe_poll(ip, &events)) {
					if (events & POLLIN) {
						obits[0].fds_bits[i] |= bit;
						n++;
					}
					if (events & POLLOUT) {
						obits[1].fds_bits[i] |= bit;
						n++;
					}
					if ((bit & ex)
					    && (events & (POLLERR|POLLHUP))) {
						obits[2].fds_bits[i] |= bit;
						n++;
					}
				}
				break;
			  }

			  default:
notsock:
				u.u_error = ENOTSOCK;
				return (-1);
			}
		}
		j = (++i)*NFDBITS;	/* advance to 1st bit of next word */
	} while (j < nfd);		/* quit after last valid bit */
	return (n);
}

#ifdef NOTDEF
/*
 * GL2W3.6 caught the select/poll cleanup in media res, with standard pipe
 * code depending on optional TCP code (below).  The ad hoc fix which I
 * applied to 3.6 kernel source was to move this code to a standard file,
 * streams/strcallsub.c
 *	/be
 */
void
pollqueue(pq, p)
	register struct pollqueue *pq;
	register struct proc *p;
{
	register struct proc *head;

	head = pq->pq_proc;
	if (head == NULL
#ifdef SVR3
	    || head->p_w2chan
#else
	    || head->p_wchan
#endif
	    != (caddr_t)&pollwait) {
		pq->pq_proc = p;
	}
	pq->pq_length++;
}

void
pollwakeup(pq)
	register struct pollqueue *pq;
{
	if (pq->pq_proc != NULL) {
		selwakeup(pq->pq_proc, pq->pq_length > 1);
		pq->pq_proc = NULL;
		pq->pq_length = 0;
	}
}

selwakeup(p, coll)
	register struct proc *p;
	int coll;
{

	if (coll) {
		nselcoll++;
		wakeup((caddr_t)&selwait);
	}
	if (p) {
		int s = splhigh();
#ifdef SVR3
		if (p->p_w2chan
#else
		if (p->p_wchan
#endif
		    == (caddr_t)&selwait) {
			setrun(p);
		} else {
#ifdef SVR3
			spsema(p->p_siglck);
#endif
			if (p->p_flag & SSEL)
				p->p_flag &= ~SSEL;
#ifdef SVR3
			svsema(p->p_siglck);
#endif
		}
		splx(s);
	}
}
#endif
