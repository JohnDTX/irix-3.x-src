/* stream system call support functions
 * 
 * $Source: /d2/3.7/src/sys/streams/RCS/strcallsub.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:34:49 $
 */

#ifdef SVR3
#include "sys/sbd.h"
#include "sys/param.h"
#include "sys/types.h"
#include "sys/file.h"
#include "sys/inode.h"
#include "sys/immu.h"
#include "sys/region.h"
#include "sys/pcb.h"
#include "sys/proc.h"
#include "sys/signal.h"
#include "sys/fs/s5dir.h"
#include "sys/user.h"
#include "sys/stream.h"
#include "sys/stropts.h"

#else
#include "../h/param.h"
#include "../h/types.h"
#include "../h/file.h"
#include "../h/inode.h"
#include "../h/proc.h"
#include "../h/user.h"
#include "../streams/stream.h"
#include "../streams/stropts.h"
#include "../h/poll.h"		/* why not for SVR3 */

#include "ustrm.h"
#include "tcp.h"
#endif

#if NUSTRM > 0 || NTCP > 0
/*
 * Removes all event cells that refer to the current process in the
 * given stream's poll list.
 */
pollreset(stp)
register struct stdata *stp;
{
	register struct strevent *psep, *sep, *tmp;

	sep = stp->sd_pollist;
	psep = NULL;
	while (sep) {
		tmp = sep->se_next;
		if (sep->se_procp == u.u_procp) {
			if (psep)
				psep->se_next = tmp;
			else
				stp->sd_pollist = tmp;
			sefree(sep);
		}
		sep = tmp;
	}
	/*
	 * Recalculate pollflags
	 */
	stp->sd_pollflags = 0;
	for (sep = stp->sd_pollist; sep; sep = sep->se_next)
		stp->sd_pollflags |= sep->se_events;
}
#endif

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

int	nselcoll;
#define	selwait	pollwait
#define	splhigh	splmax

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
