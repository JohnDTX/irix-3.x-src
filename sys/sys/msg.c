/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
**	Inter-Process Communication Message Facility.
*/
#include "../h/param.h"
#include "../h/user.h"
#include "../h/proc.h"
#include "../h/map.h"
#include "../h/ipc.h"
#include "../h/msg.h"
#include "../h/systm.h"
#include "../h/buf.h"

#ifdef	sgi
char	msgmap_wanted;
#endif

extern struct map	msgmap[];	/* msg allocation map */
extern struct msqid_ds	msgque[];	/* msg queue headers */
extern struct msg	msgh[];		/* message headers */
extern struct msginfo	msginfo;	/* message parameters */
extern char		msglock[];
struct msg		*msgfp;	/* ptr to head of free header list */
paddr_t			msg;		/* base address of message buffer */
extern time_t		time;		/* system idea of date */

struct msqid_ds		*ipcget(),
			*msgconv();

/* Convert bytes to msg segments. */
#define	btoq(X)	((X + msginfo.msgssz - 1) / msginfo.msgssz)

/* Choose appropriate message copy routine. */
#ifdef pdp11
#define MOVE	msgpimove
#else
#define	MOVE	iomove
#endif

/*
**	msgconv - Convert a user supplied message queue id into a ptr to a
**		msqid_ds structure.
*/

struct msqid_ds *
msgconv(id)
register int	id;
{
	register struct msqid_ds	*qp;	/* ptr to associated q slot */
	register char			*lockp;	/* ptr to lock.		*/

	if (id < 0) {
		u.u_error = EINVAL;
		return(NULL);
	}
	qp = &msgque[id % msginfo.msgmni];
	lockp = MSGLOCK(qp);
	while (*lockp)
		sleep(lockp, PMSG);
	*lockp = 1;
	if ((qp->msg_perm.mode & IPC_ALLOC) == 0 ||
		id / msginfo.msgmni != qp->msg_perm.seq) {
		*lockp = 0;
		wakeup(lockp);
		u.u_error = EINVAL;
		return(NULL);
	}
	return(qp);
}

/*
**	msgctl - Msgctl system call.
*/

msgctl()
{
	register struct a {
		int		msgid,
				cmd;
		struct msqid_ds	*buf;
	}		*uap = (struct a *)u.u_ap;
	struct msqid_ds			ds;	/* queue work area */
	register struct msqid_ds	*qp;	/* ptr to associated q */
	register char			*lockp;

	if ((qp = msgconv(uap->msgid)) == NULL)
		return;
	lockp = MSGLOCK(qp);
	u.u_rval1 = 0;
	switch (uap->cmd) {
	case IPC_RMID:
		if (u.u_uid != qp->msg_perm.uid && u.u_uid != qp->msg_perm.cuid
			&& !suser())
			break;
		while (qp->msg_first)
			msgfree(qp, NULL, qp->msg_first);
		qp->msg_cbytes = 0;
		if (uap->msgid + msginfo.msgmni < 0)
			qp->msg_perm.seq = 0;
		else
			qp->msg_perm.seq++;
		if (qp->msg_perm.mode & MSG_RWAIT)
			wakeup(&qp->msg_qnum);
		if (qp->msg_perm.mode & MSG_WWAIT)
			wakeup(qp);
		qp->msg_perm.mode = 0;
		break;
	case IPC_SET:
		if (u.u_uid != qp->msg_perm.uid && u.u_uid != qp->msg_perm.cuid
			 && !suser())
			break;
		if (copyin(uap->buf, &ds, sizeof(ds))) {
			u.u_error = EFAULT;
			break;
		}
		if (ds.msg_qbytes > qp->msg_qbytes && !suser())
			break;
		qp->msg_perm.uid = ds.msg_perm.uid;
		qp->msg_perm.gid = ds.msg_perm.gid;
		qp->msg_perm.mode = (qp->msg_perm.mode & ~0777) |
			(ds.msg_perm.mode & 0777);
		qp->msg_qbytes = ds.msg_qbytes;
		qp->msg_ctime = time;
		break;
	case IPC_STAT:
		if (ipcaccess(&qp->msg_perm, MSG_R))
			break;
		if (copyout(qp, uap->buf, sizeof(*qp))) {
			u.u_error = EFAULT;
			break;
		}
		break;
	default:
		u.u_error = EINVAL;
		break;
	}

	*lockp = 0;
	wakeup(lockp);

}

/*
**	msgfree - Free up space and message header, relink pointers on q,
**	and wakeup anyone waiting for resources.
*/

msgfree(qp, pmp, mp)
register struct msqid_ds	*qp;	/* ptr to q of mesg being freed */
register struct msg		*mp,	/* ptr to msg being freed */
				*pmp;	/* ptr to mp's predecessor */
{
	/* Unlink message from the q. */
	if (pmp == NULL)
		qp->msg_first = mp->msg_next;
	else
		pmp->msg_next = mp->msg_next;
	if (mp->msg_next == NULL)
		qp->msg_last = pmp;
	qp->msg_qnum--;
	if (qp->msg_perm.mode & MSG_WWAIT) {
		qp->msg_perm.mode &= ~MSG_WWAIT;
		wakeup(qp);
	}

	/* Free up message text. */
	if (mp->msg_ts)
		rmfree(msgmap, (long) btoq(mp->msg_ts),
			       (long) (mp->msg_spot + 1));

	/* Free up header */
	mp->msg_next = msgfp;
	if (msgfp == NULL)
		wakeup(&msgfp);
	msgfp = mp;
}

/*
**	msgget - Msgget system call.
*/

msgget()
{
	register struct a {
		key_t	key;
		int	msgflg;
	}	*uap = (struct a *)u.u_ap;
	register struct msqid_ds	*qp;	/* ptr to associated q */
	int				s;	/* ipcget status return */

	if ((qp = ipcget(uap->key, uap->msgflg, msgque, msginfo.msgmni, sizeof(*qp), &s))
		== NULL)
		return;

	if (s) {
		/* This is a new queue.  Finish initialization. */
		qp->msg_first = qp->msg_last = NULL;
		qp->msg_qnum = 0;
		qp->msg_qbytes = msginfo.msgmnb;
		qp->msg_lspid = qp->msg_lrpid = 0;
		qp->msg_stime = qp->msg_rtime = 0;
		qp->msg_ctime = time;
	}
	u.u_rval1 = qp->msg_perm.seq * msginfo.msgmni + (qp - msgque);
}

/*
**	msginit - Called by main(main.c) to initialize message queues.
*/

msginit()
{
	register int		i;	/* loop control */
	register struct msg	*mp;	/* ptr to msg begin linked */

	/* Allocate physical memory for message buffer. */
	if ((msg = (paddr_t)
		 calloc(1, msginfo.msgseg * msginfo.msgssz)) == NULL) {
		printf("kernel: out of memory for msg's\n");
		msginfo.msgseg = 0;
	}
	rminit(msgmap, (long)msginfo.msgseg, (long)1, "msgmap", msginfo.msgmap);
	for (i = 0, mp = msgfp = msgh;++i < msginfo.msgtql;mp++)
		mp->msg_next = mp + 1;
}

#ifdef pdp11
/*
**	msgpimove - PDP 11 pimove interface for possibly large copies.
*/

msgpimove(base, count, mode)
paddr_t			base;	/* base address */
register unsigned	count;	/* byte count */
int			mode;	/* transfer mode */
{
	register unsigned	tcount;	/* current transfer count */

	while (u.u_error == 0 && count) {
		tcount = count > 8064 ? 8064 : count;
		pimove(base, tcount, mode);
		base += tcount;
		count -= tcount;
	}
}
#endif

/*
**	msgrcv - Msgrcv system call.
*/

msgrcv()
{
	register struct a {
		int		msqid;
		struct msgbuf	*msgp;
		int		msgsz;
		long		msgtyp;
		int		msgflg;
	}	*uap = (struct a *)u.u_ap;
	register struct msg		*mp,	/* ptr to msg on q */
					*pmp,	/* ptr to mp's predecessor */
					*smp,	/* ptr to best msg on q */
					*spmp;	/* ptr to smp's predecessor */
	register struct msqid_ds	*qp;	/* ptr to associated q */
	register char			*lockp;
	struct msqid_ds			*qp1;
	int				sz;	/* transfer byte count */

	if ((qp = msgconv(uap->msqid)) == NULL)
		return;
	lockp = MSGLOCK(qp);
	if (ipcaccess(&qp->msg_perm, MSG_R))
		goto msgrcv_out;
	if (uap->msgsz < 0) {
		u.u_error = EINVAL;
		goto msgrcv_out;
	}
	smp = spmp = NULL;
	*lockp = 0;
	wakeup(lockp);
findmsg:
	if ((qp1 = msgconv(uap->msqid)) == NULL) {
		u.u_error = EIDRM;
		return;
	}
	if (qp1 != qp) {
		u.u_error = EIDRM;
		lockp = MSGLOCK(qp1);
		*lockp = 0;
		wakeup(lockp);
		return;
	}
	pmp = NULL;
	mp = qp->msg_first;
	if (uap->msgtyp == 0)
		smp = mp;
	else
		for (;mp;pmp = mp, mp = mp->msg_next) {
			if (uap->msgtyp > 0) {
				if (uap->msgtyp != mp->msg_type)
					continue;
				smp = mp;
				spmp = pmp;
				break;
			}
			if (mp->msg_type <= -uap->msgtyp) {
				if (smp && smp->msg_type <= mp->msg_type)
					continue;
				smp = mp;
				spmp = pmp;
			}
		}
	if (smp) {
		if (uap->msgsz < smp->msg_ts)
			if (!(uap->msgflg & MSG_NOERROR)) {
				u.u_error = E2BIG;
				goto msgrcv_out;
			} else
				sz = uap->msgsz;
		else
			sz = smp->msg_ts;
		if (copyout(&smp->msg_type, uap->msgp, sizeof(smp->msg_type))) {
			u.u_error = EFAULT;
			goto msgrcv_out;
		}
		if (sz) {
			u.u_base = (caddr_t)uap->msgp + sizeof(smp->msg_type);
			u.u_segflg = 0;
			MOVE(msg + msginfo.msgssz * smp->msg_spot,
				sz, B_READ);
			if (u.u_error)
				goto msgrcv_out;
		}
		u.u_rval1 = sz;
		qp->msg_cbytes -= smp->msg_ts;
		qp->msg_lrpid = u.u_procp->p_pid;
		qp->msg_rtime = time;
		curpri = PMSG;
		msgfree(qp, spmp, smp);
		goto msgrcv_out;
	}
	if (uap->msgflg & IPC_NOWAIT) {
		u.u_error = ENOMSG;
		goto msgrcv_out;
	}
	qp->msg_perm.mode |= MSG_RWAIT;
	*lockp = 0;
	wakeup(lockp);
	if (sleep(&qp->msg_qnum, PMSG | PCATCH)) {
		u.u_error = EINTR;
		return;
	}
	goto findmsg;

msgrcv_out:

	*lockp = 0;
	wakeup(lockp);
}

/*
**	msgsnd - Msgsnd system call.
*/

msgsnd()
{
	register struct a {
		int		msqid;
		struct msgbuf	*msgp;
		int		msgsz;
		int		msgflg;
	}	*uap = (struct a *)u.u_ap;
	register struct msqid_ds	*qp;	/* ptr to associated q */
	register struct msg		*mp;	/* ptr to allocated msg hdr */
	register int			cnt,	/* byte count */
					spot;	/* msg pool allocation spot */
	register char			*lockp;
	struct msqid_ds			*qp1;
	long				type;	/* msg type */

	if ((qp = msgconv(uap->msqid)) == NULL)
		return;
	lockp = MSGLOCK(qp);
	if (ipcaccess(&qp->msg_perm, MSG_W))
		goto msgsnd_out;
	if ((cnt = uap->msgsz) < 0 || cnt > msginfo.msgmax) {
		u.u_error = EINVAL;
		goto msgsnd_out;
	}
	if (copyin(uap->msgp, &type, sizeof(type))) {
		u.u_error = EFAULT;
		goto msgsnd_out;
	}
	if (type < 1) {
		u.u_error = EINVAL;
		goto msgsnd_out;
	}
	*lockp = 0;
	wakeup(lockp);
getres:
	/* Be sure that q has not been removed. */

	if ((qp1 = msgconv(uap->msqid)) == NULL) {
		u.u_error = EIDRM;
		return;
	}
	if (qp1 != qp) {
		u.u_error = EIDRM;
		lockp = MSGLOCK(qp1);
		*lockp = 0;
		wakeup(lockp);
		return;
	}

	/* Allocate space on q, message header, & buffer space. */
	if (cnt + qp->msg_cbytes > qp->msg_qbytes) {
		if (uap->msgflg & IPC_NOWAIT) {
			u.u_error = EAGAIN;
			goto msgsnd_out;
		}
		qp->msg_perm.mode |= MSG_WWAIT;
		*lockp = 0;
		wakeup(lockp);
		if (sleep(qp, PMSG | PCATCH)) {
			u.u_error = EINTR;
			if ((qp1 = msgconv(uap->msqid)) == NULL)
				return;
			if (qp1 != qp) {
				lockp = MSGLOCK(qp1);
				*lockp = 0;
				wakeup(lockp);
				return;
			}
			qp->msg_perm.mode &= ~MSG_WWAIT;
			wakeup(qp);
			goto msgsnd_out;
		}
		goto getres;
	}
	if (msgfp == NULL) {
		if (uap->msgflg & IPC_NOWAIT) {
			u.u_error = EAGAIN;
			goto msgsnd_out;
		}
		*lockp = 0;
		wakeup(lockp);
		if (sleep(&msgfp, PMSG | PCATCH)) {
			u.u_error = EINTR;
			if ((qp1 = msgconv(uap->msqid)) == NULL)
				return;
			if (qp1 != qp) {
				lockp = MSGLOCK(qp1);
				*lockp = 0;
				wakeup(lockp);
				return;
			}
			goto msgsnd_out;
		}
		goto getres;
	}
	mp = msgfp;
	msgfp = mp->msg_next;
	if (cnt && (spot = rmalloc(msgmap, btoq(cnt))) == NULL) {
		if (uap->msgflg & IPC_NOWAIT) {
			u.u_error = EAGAIN;
			goto msgsnd_out1;
		}
		msgmap_wanted++;
		mp->msg_next = msgfp;
		if (msgfp == NULL)
			wakeup(&msgfp);
		msgfp = mp;
		*lockp = 0;
		wakeup(lockp);
		if (sleep(msgmap, PMSG | PCATCH)) {
			u.u_error = EINTR;
			if ((qp1 = msgconv(uap->msqid)) == NULL) {
				return;
			}
			if (qp1 != qp) {
				lockp = MSGLOCK(qp1);
				*lockp = 0;
				wakeup(lockp);
				return;
			}
			goto msgsnd_out;
		}
		goto getres;
	}

	/* Everything is available, copy in text and put msg on q. */
	if (cnt) {
		u.u_base = (caddr_t)uap->msgp + sizeof(type);
		u.u_segflg = 0;
		MOVE(msg + msginfo.msgssz * --spot, cnt, B_WRITE);
		if (u.u_error) {
			rmfree(msgmap, (long) btoq(cnt), (long) (spot + 1));
			if (msgmap_wanted) {
				msgmap_wanted = 0;
				wakeup((caddr_t)msgmap);
			}
			goto msgsnd_out1;
		}
	}
	qp->msg_qnum++;
	qp->msg_cbytes += cnt;
	qp->msg_lspid = u.u_procp->p_pid;
	qp->msg_stime = time;
	mp->msg_next = NULL;
	mp->msg_type = type;
	mp->msg_ts = cnt;
	mp->msg_spot = cnt ? spot : -1;
	if (qp->msg_last == NULL)
		qp->msg_first = qp->msg_last = mp;
	else {
		qp->msg_last->msg_next = mp;
		qp->msg_last = mp;
	}
	if (qp->msg_perm.mode & MSG_RWAIT) {
		qp->msg_perm.mode &= ~MSG_RWAIT;
		curpri = PMSG;
		wakeup(&qp->msg_qnum);
	}
	u.u_rval1 = 0;
	goto msgsnd_out;

msgsnd_out1:

	mp->msg_next = msgfp;
	if (msgfp == NULL)
		wakeup(&msgfp);
	msgfp = mp;

msgsnd_out:

	*lockp = 0;
	wakeup(lockp);
	return;
}

/*
**	msgsys - System entry point for msgctl, msgget, msgrcv, and msgsnd
**		system calls.
*/

msgsys()
{
	int		msgctl(),
			msgget(),
			msgrcv(),
			msgsnd();
	static int	(*calls[])() = { msgget, msgctl, msgrcv, msgsnd };
	register struct a {
		unsigned	id;	/* function code id */
		int		*ap;	/* arg pointer for recvmsg */
	}		*uap = (struct a *)u.u_ap;

	if (uap->id > 3) {
		u.u_error = EINVAL;
		return;
	}
	u.u_ap = &u.u_arg[1];
	(*calls[uap->id])();
}
