/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
**	Inter-Process Communication Semaphore Facility.
*/
#include "../h/param.h"
#include "../h/systm.h"
#include "../h/map.h"
#include "../h/ipc.h"
#include "../h/sem.h"
#include "../h/user.h"
#include "../h/proc.h"
#include "../h/buf.h"

#ifdef pdp11
#define MOVE	sempimove
#else
#define	MOVE	iomove
#endif

extern struct semid_ds	sema[];		/* semaphore data structures */
extern struct sem	sem[];		/* semaphores */
extern struct map	semmap[];	/* sem allocation map */
extern struct sem_undo	*sem_undo[];	/* undo table pointers */
extern struct sem_undo	semu[];		/* operation adjust on exit table */
extern struct seminfo seminfo;		/* param information structure */
extern union {
	ushort			semvals[1]; /* set semaphore values */
	struct	semid_ds	ds;	/* set permission values */
	struct sembuf		semops[1];	/* operation holding area */
} semtmp;
struct sem_undo	*semunp;		/* ptr to head of undo chain */
struct sem_undo	*semfup;		/* ptr to head of free undo chain */

extern time_t	time;			/* system idea of date */

struct semid_ds	*ipcget(),
		*semconv();

/*
**	semaoe - Create or update adjust on exit entry.
*/

semaoe(val, id, num)
short	val,	/* operation value to be adjusted on exit */
	num;	/* semaphore # */
int	id;	/* semid */
{
	register struct undo		*uup,	/* ptr to entry to update */
					*uup2;	/* ptr to move entry */
	register struct sem_undo	*up,	/* ptr to process undo struct */
					*up2;	/* ptr to undo list */
	register int			i,	/* loop control */
					found;	/* matching entry found flag */

	if (val == 0)
		return(0);
	if (val > seminfo.semaem || val < -seminfo.semaem) {
		u.u_error = ERANGE;
		return(1);
	}
	if ((up = sem_undo[u.u_procp - proc]) == NULL)
		if (up = semfup) {
			semfup = up->un_np;
			up->un_np = NULL;
			sem_undo[u.u_procp - proc] = up;
		} else {
			u.u_error = ENOSPC;
			return(1);
		}
	for (uup = up->un_ent, found = i = 0;i < up->un_cnt;i++) {
		if (uup->un_id < id || (uup->un_id == id && uup->un_num < num)) {
			uup++;
			continue;
		}
		if (uup->un_id == id && uup->un_num == num)
			found = 1;
		break;
	}
	if (!found) {
		if (up->un_cnt >= seminfo.semume) {
			u.u_error = EINVAL;
			return(1);
		}
		if (up->un_cnt == 0) {
			up->un_np = semunp;
			semunp = up;
		}
		uup2 = &up->un_ent[up->un_cnt++];
		while (uup2-- > uup)
			*(uup2 + 1) = *uup2;
		uup->un_id = id;
		uup->un_num = num;
		uup->un_aoe = -val;
		return(0);
	}
	uup->un_aoe -= val;
	if (uup->un_aoe > seminfo.semaem || uup->un_aoe < -seminfo.semaem) {
		u.u_error = ERANGE;
		uup->un_aoe += val;
		return(1);
	}
	if (uup->un_aoe == 0) {
		uup2 = &up->un_ent[--(up->un_cnt)];
		while (uup++ < uup2)
			*(uup - 1) = *uup;
		if (up->un_cnt == 0) {

			/* Remove process from undo list. */
			if (semunp == up)
				semunp = up->un_np;
			else
				for (up2 = semunp;up2 != NULL;up2 = up2->un_np)
					if (up2->un_np == up) {
						up2->un_np = up->un_np;
						break;
					}
			up->un_np = NULL;
		}
	}
	return(0);
}

/*
**	semconv - Convert user supplied semid into a ptr to the associated
**		semaphore header.
*/

struct semid_ds *
semconv(s)
register int	s;	/* semid */
{
	register struct semid_ds	*sp;	/* ptr to associated header */

	if (s < 0) {
		u.u_error = EINVAL;
		return(NULL);
	}
	sp = &sema[s % seminfo.semmni];
	if ((sp->sem_perm.mode & IPC_ALLOC) == 0 ||
		s / seminfo.semmni != sp->sem_perm.seq) {
		u.u_error = EINVAL;
		return(NULL);
	}
	return(sp);
}

/*
**	semctl - Semctl system call.
*/

semctl()
{
	register struct a {
		int	semid;
		uint	semnum;
		int	cmd;
		int	arg;
	}	*uap = (struct a *)u.u_ap;
	register struct	semid_ds	*sp;	/* ptr to semaphore header */
	register struct sem		*p;	/* ptr to semaphore */
	register int			i;	/* loop control */

	if ((sp = semconv(uap->semid)) == NULL)
		return;
	u.u_rval1 = 0;
	switch (uap->cmd) {

	/* Remove semaphore set. */
	case IPC_RMID:
		if (u.u_uid != sp->sem_perm.uid && u.u_uid != sp->sem_perm.cuid
			&& !suser())
			return;
		semunrm(uap->semid, 0, sp->sem_nsems);
		for (i = sp->sem_nsems, p = sp->sem_base;i--;p++) {
			p->semval = p->sempid = 0;
			if (p->semncnt) {
				wakeup(&p->semncnt);
				p->semncnt = 0;
			}
			if (p->semzcnt) {
				wakeup(&p->semzcnt);
				p->semzcnt = 0;
			}
		}
		rmfree(semmap, (long)sp->sem_nsems,
			       (long) ((sp->sem_base - sem) + 1));
		if (uap->semid + seminfo.semmni < 0)
			sp->sem_perm.seq = 0;
		else
			sp->sem_perm.seq++;
		sp->sem_perm.mode = 0;
		return;

	/* Set ownership and permissions. */
	case IPC_SET:
		if (u.u_uid != sp->sem_perm.uid && u.u_uid != sp->sem_perm.cuid
			 && !suser())
			return;
		if (copyin(uap->arg, &semtmp.ds, sizeof(semtmp.ds))) {
			u.u_error = EFAULT;
			return;
		}
		sp->sem_perm.uid = semtmp.ds.sem_perm.uid;
		sp->sem_perm.gid = semtmp.ds.sem_perm.gid;
		sp->sem_perm.mode = semtmp.ds.sem_perm.mode & 0777 | IPC_ALLOC;
		sp->sem_ctime = time;
		return;

	/* Get semaphore data structure. */
	case IPC_STAT:
		if (ipcaccess(&sp->sem_perm, SEM_R))
			return;
		if (copyout(sp, uap->arg, sizeof(*sp))) {
			u.u_error = EFAULT;
			return;
		}
		return;

	/* Get # of processes sleeping for greater semval. */
	case GETNCNT:
		if (ipcaccess(&sp->sem_perm, SEM_R))
			return;
		if (uap->semnum >= sp->sem_nsems) {
			u.u_error = EINVAL;
			return;
		}
		u.u_rval1 = (sp->sem_base + uap->semnum)->semncnt;
		return;

	/* Get pid of last process to operate on semaphore. */
	case GETPID:
		if (ipcaccess(&sp->sem_perm, SEM_R))
			return;
		if (uap->semnum >= sp->sem_nsems) {
			u.u_error = EINVAL;
			return;
		}
		u.u_rval1 = (sp->sem_base + uap->semnum)->sempid;
		return;

	/* Get semval of one semaphore. */
	case GETVAL:
		if (ipcaccess(&sp->sem_perm, SEM_R))
			return;
		if (uap->semnum >= sp->sem_nsems) {
			u.u_error = EINVAL;
			return;
		}
		u.u_rval1 = (sp->sem_base + uap->semnum)->semval;
		return;

	/* Get all semvals in set. */
	case GETALL:
		if (ipcaccess(&sp->sem_perm, SEM_R))
			return;
		u.u_base = (caddr_t)uap->arg;
		u.u_offset = 0;
		u.u_segflg = 0;
		for (i = sp->sem_nsems, p = sp->sem_base;i--;p++) {
			MOVE((paddr_t)&p->semval, sizeof(p->semval), B_READ);
			if (u.u_error)
				return;
		}
		return;

	/* Get # of processes sleeping for semval to become zero. */
	case GETZCNT:
		if (ipcaccess(&sp->sem_perm, SEM_R))
			return;
		if (uap->semnum >= sp->sem_nsems) {
			u.u_error = EINVAL;
			return;
		}
		u.u_rval1 = (sp->sem_base + uap->semnum)->semzcnt;
		return;

	/* Set semval of one semaphore. */
	case SETVAL:
		if (ipcaccess(&sp->sem_perm, SEM_A))
			return;
		if (uap->semnum >= sp->sem_nsems) {
			u.u_error = EINVAL;
			return;
		}
		if ((unsigned)uap->arg > seminfo.semvmx) {
			u.u_error = ERANGE;
			return;
		}
		if ((p = sp->sem_base + uap->semnum)->semval = uap->arg) {
			if (p->semncnt) {
				p->semncnt = 0;
				wakeup(&p->semncnt);
			}
		} else
			if (p->semzcnt) {
				p->semzcnt = 0;
				wakeup(&p->semzcnt);
			}
		p->sempid = u.u_procp->p_pid;
		semunrm(uap->semid, uap->semnum, uap->semnum);
		return;

	/* Set semvals of all semaphores in set. */
	case SETALL:
		if (ipcaccess(&sp->sem_perm, SEM_A))
			return;
		u.u_base = (caddr_t)uap->arg;
		u.u_offset = 0;
		u.u_segflg = 0;
		MOVE((paddr_t)semtmp.semvals,
			sizeof(semtmp.semvals[0]) * sp->sem_nsems, B_WRITE);
		if (u.u_error)
			return;
		for (i = 0;i < sp->sem_nsems;)
			if (semtmp.semvals[i++] > seminfo.semvmx) {
				u.u_error = ERANGE;
				return;
			}
		semunrm(uap->semid, 0, sp->sem_nsems);
		for (i = 0, p = sp->sem_base;i < sp->sem_nsems;
			(p++)->sempid = u.u_procp->p_pid) {
			if (p->semval = semtmp.semvals[i++]) {
				if (p->semncnt) {
					p->semncnt = 0;
					wakeup(&p->semncnt);
				}
			} else
				if (p->semzcnt) {
					p->semzcnt = 0;
					wakeup(&p->semzcnt);
				}
		}
		return;
	default:
		u.u_error = EINVAL;
		return;
	}
}

/*
**	semexit - Called by exit(sys1.c) to clean up on process exit.
*/

semexit()
{
	register struct sem_undo	*up,	/* process undo struct ptr */
					*p;	/* undo struct ptr */
	register struct semid_ds	*sp;	/* semid being undone ptr */
	register int			i;	/* loop control */
	register long			v;	/* adjusted value */
	register struct sem		*semp;	/* semaphore ptr */

	if ((up = sem_undo[u.u_procp - proc]) == NULL)
		return;
	if (up->un_cnt == 0) {
		goto cleanup;
	}
	for (i = up->un_cnt;i--;) {
		if ((sp = semconv(up->un_ent[i].un_id)) == NULL)
			continue;
		v = (long)(semp = sp->sem_base + up->un_ent[i].un_num)->semval +
			up->un_ent[i].un_aoe;
		if (v < 0 || v > seminfo.semvmx)
			continue;
		semp->semval = v;
		if (v == 0 && semp->semzcnt) {
			semp->semzcnt = 0;
			wakeup(&semp->semzcnt);
		}
		if (up->un_ent[i].un_aoe > 0 && semp->semncnt) {
			semp->semncnt = 0;
			wakeup(&semp->semncnt);
		}
	}
	up->un_cnt = 0;
	if (semunp == up)
		semunp = up->un_np;
	else
		for (p = semunp;p != NULL;p = p->un_np)
			if (p->un_np == up) {
				p->un_np = up->un_np;
				break;
			}
cleanup:
	up->un_np = semfup;
	semfup = up;
	sem_undo[u.u_procp - proc] = NULL;
}

/*
**	semget - Semget system call.
*/

semget()
{
	register struct a {
		key_t	key;
		int	nsems;
		int	semflg;
	}	*uap = (struct a *)u.u_ap;
	register struct semid_ds	*sp;	/* semaphore header ptr */
	register unsigned int		i;	/* temp */
	int				s;	/* ipcget status return */

	if ((sp = ipcget(uap->key, uap->semflg, sema, seminfo.semmni, sizeof(*sp), &s))
		== NULL)
		return;
	if (s) {

		/* This is a new semaphore set.  Finish initialization. */
		if (uap->nsems <= 0 || uap->nsems > seminfo.semmsl) {
			u.u_error = EINVAL;
			sp->sem_perm.mode = 0;
			return;
		}
		if ((i = rmalloc(semmap, uap->nsems)) == NULL) {
			u.u_error = ENOSPC;
			sp->sem_perm.mode = 0;
			return;
		}
		sp->sem_base = sem + (i - 1);
		sp->sem_nsems = uap->nsems;
		sp->sem_ctime = time;
		sp->sem_otime = 0;
	} else
		if (uap->nsems && sp->sem_nsems < uap->nsems) {
			u.u_error = EINVAL;
			return;
		}
	u.u_rval1 = sp->sem_perm.seq * seminfo.semmni + (sp - sema);
}

/*
**	seminit - Called by main(main.c) to initialize the semaphore map.
*/

seminit()
{
	register i;

	rminit(semmap, (long)seminfo.semmns, (long)1, "semmap",
		       seminfo.semmap);

	semfup = semu;
	for (i = 0; i < seminfo.semmnu - 1; i++) {
		semfup->un_np = (struct sem_undo *)((uint)semfup+seminfo.semusz);
		semfup = semfup->un_np;
	}
	semfup->un_np = NULL;
	semfup = semu;
}

/*
**	semop - Semop system call.
*/

semop()
{
	register struct a {
		int		semid;
		struct sembuf	*sops;
		uint		nsops;
	}	*uap = (struct a *)u.u_ap;
	register struct sembuf		*op;	/* ptr to operation */
	register int			i;	/* loop control */
	register struct semid_ds	*sp;	/* ptr to associated header */
	register struct sem		*semp;	/* ptr to semaphore */
	int	again;

	if ((sp = semconv(uap->semid)) == NULL)
		return;
	if (uap->nsops > seminfo.semopm) {
		u.u_error = E2BIG;
		return;
	}
	u.u_base = (caddr_t)uap->sops;
	u.u_offset = 0;
	u.u_segflg = 0;
	MOVE((paddr_t)semtmp.semops, uap->nsops * sizeof(*op), B_WRITE);
	if (u.u_error)
		return;

	/* Verify that sem #s are in range and permissions are granted. */
	for (i = 0, op = semtmp.semops;i++ < uap->nsops;op++) {
		if (ipcaccess(&sp->sem_perm, op->sem_op ? SEM_A : SEM_R))
			return;
		if (op->sem_num >= sp->sem_nsems) {
			u.u_error = EFBIG;
			return;
		}
	}
	again = 0;
check:
	/* Loop waiting for the operations to be satisified atomically. */
	/* Actually, do the operations and undo them if a wait is needed
		or an error is detected. */
	if (again) {
		/* Verify that the semaphores haven't been removed. */
		if (semconv(uap->semid) == NULL) {
			u.u_error = EIDRM;
			return;
		}
		/* copy in user operation list after sleep */
		u.u_base = (caddr_t)uap->sops;
		u.u_offset = 0;
		u.u_segflg = 0;
		MOVE((paddr_t)semtmp.semops, uap->nsops * sizeof(*op), B_WRITE);
		if (u.u_error)
			return;
	}
	again = 1;

	for (i = 0, op = semtmp.semops;i < uap->nsops;i++, op++) {
		semp = sp->sem_base + op->sem_num;
		if (op->sem_op > 0) {
			if (op->sem_op + (long)semp->semval > seminfo.semvmx ||
				(op->sem_flg & SEM_UNDO &&
				semaoe(op->sem_op, uap->semid, op->sem_num))) {
				if (u.u_error == 0)
					u.u_error = ERANGE;
				if (i)
					semundo(semtmp.semops, i, uap->semid, sp);
				return;
			}
			semp->semval += op->sem_op;
			if (semp->semncnt) {
				semp->semncnt = 0;
				wakeup(&semp->semncnt);
			}
			if (semp->semzcnt && !semp->semval) {
				semp->semzcnt = 0;
				wakeup(&semp->semzcnt);
			}
			continue;
		}
		if (op->sem_op < 0) {
			if (semp->semval >= -op->sem_op) {
				if (op->sem_flg & SEM_UNDO &&
					semaoe(op->sem_op, uap->semid, op->sem_num)) {
					if (i)
						semundo(semtmp.semops, i, uap->semid, sp);
					return;
				}
				semp->semval += op->sem_op;
				if (semp->semzcnt && !semp->semval) {
					semp->semzcnt = 0;
					wakeup(&semp->semzcnt);
				}
				continue;
			}
			if (i)
				semundo(semtmp.semops, i, uap->semid, sp);
			if (op->sem_flg & IPC_NOWAIT) {
				u.u_error = EAGAIN;
				return;
			}
			semp->semncnt++;
			if (sleep(&semp->semncnt, PCATCH | PSEMN)) {
				if ((semp->semncnt)-- <= 1) {
					semp->semncnt = 0;
					wakeup(&semp->semncnt);
				}
				u.u_error = EINTR;
				return;
			}
			goto check;
		}
		if (semp->semval) {
			if (i)
				semundo(semtmp.semops, i, uap->semid, sp);
			if (op->sem_flg & IPC_NOWAIT) {
				u.u_error = EAGAIN;
				return;
			}
			semp->semzcnt++;
			if (sleep(&semp->semzcnt, PCATCH | PSEMZ)) {
				if ((semp->semzcnt)-- <= 1) {
					semp->semzcnt = 0;
					wakeup(&semp->semzcnt);
				}
				u.u_error = EINTR;
				return;
			}
			goto check;
		}
	}

	/* All operations succeeded.  Update sempid for accessed semaphores. */
	for (i = 0, op = semtmp.semops;i++ < uap->nsops;
		(sp->sem_base + (op++)->sem_num)->sempid = u.u_procp->p_pid);
	sp->sem_otime = time;
	u.u_rval1 = 0;
}

#ifdef pdp11
/*
**	sempimove - PDP 11 pimove interface for possibly large copies.
*/

sempimove(base, count, mode)
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
**	semsys - System entry point for semctl, semget, and semop system calls.
*/

semsys()
{
	int	semctl(),
		semget(),
		semop();
	static int	(*calls[])() = {semctl, semget, semop};
	register struct a {
		uint	id;	/* function code id */
	}	*uap = (struct a *)u.u_ap;

	if (uap->id > 2) {
		u.u_error = EINVAL;
		return;
	}
	u.u_ap = &u.u_arg[1];
	(*calls[uap->id])();
}

/*
**	semundo - Undo work done up to finding an operation that can't be done.
*/

semundo(op, n, id, sp)
register struct sembuf		*op;	/* first operation that was done ptr */
register int			n,	/* # of operations that were done */
				id;	/* semaphore id */
register struct semid_ds	*sp;	/* semaphore data structure ptr */
{
	register struct sem	*semp;	/* semaphore ptr */

	for (op += n - 1;n--;op--) {
		if (op->sem_op == 0)
			continue;
		semp = sp->sem_base + op->sem_num;
		semp->semval -= op->sem_op;
		if (op->sem_flg & SEM_UNDO)
			semaoe(-op->sem_op, id, op->sem_num);
	}
}

/*
**	semunrm - Undo entry remover.
**
**	This routine is called to clear all undo entries for a set of semaphores
**	that are being removed from the system or are being reset by SETVAL or
**	SETVALS commands to semctl.
*/

semunrm(id, low, high)
int	id;	/* semid */
ushort	low,	/* lowest semaphore being changed */
	high;	/* highest semaphore being changed */
{
	register struct sem_undo	*pp,	/* ptr to predecessor to p */
					*p;	/* ptr to current entry */
	register struct undo		*up;	/* ptr to undo entry */
	register int			i,	/* loop control */
					j;	/* loop control */

	pp = NULL;
	p = semunp;
	while (p != NULL) {

		/* Search through current structure for matching entries. */
		for (up = p->un_ent, i = 0;i < p->un_cnt;) {
			if (id < up->un_id)
				break;
			if (id > up->un_id || low > up->un_num) {
				up++;
				i++;
				continue;
			}
			if (high < up->un_num)
				break;
			for (j = i;++j < p->un_cnt;
				p->un_ent[j - 1] = p->un_ent[j]);
			p->un_cnt--;
		}

		/* Reset pointers for next round. */
		if (p->un_cnt == 0)

			/* Remove from linked list. */
			if (pp == NULL) {
				semunp = p->un_np;
				p->un_np = NULL;
				p = semunp;
			} else {
				pp->un_np = p->un_np;
				p->un_np = NULL;
				p = pp->un_np;
			}
		else {
			pp = p;
			p = p->un_np;
		}
	}
}
