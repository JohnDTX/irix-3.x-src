/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/* #ident	"@(#)kern-3b2:os/flock.c	1.6" */
/* (IH)flock.c	1.1.1.4 */

/*
 *	Notes :
 *
 *		This file contains all of the file/record locking
 *		specific routines.
 *
 *		All record lock lists (referenced by a pointer in
 *		the inode) are ordered by starting position relative
 *		to the beginning of the file.
 *
 *		In this file the name "l_end" is a macro and is used
 *		in place of "l_len" because the end, not length, of
 *		the record lock is stored internally.
 *
 */

#include "../h/types.h"
#include "../h/param.h"
#ifdef	u3b2
#	include "sys/sbd.h"
#	include "sys/immu.h"
#	include "sys/region.h"
#	include "sys/psw.h"
#	include "sys/pcb.h"
#endif
#ifdef	u3b
#	include "sys/page.h"
#	include "sys/region.h"
#	include "sys/sema.h"
#	define	SLEEP(ptr, pri)		psema(&ptr->wakesem, pri)
#	define	WAKEUP(ptr)		if (ptr->stat.wakeflg) { \
						while (ptr->stat.wakeflg--) \
							vsema(&ptr->wakesem, 0); \
						++ptr->stat.wakeflg; \
					}
	sema_t	frlock_sem;
	sema_t	slplck_sem;
#else
#	define	SLEEP(ptr, pri)		sleep(ptr, pri)
#	define	WAKEUP(ptr)		if (ptr->stat.wakeflg) { \
						wakeup(ptr); \
						ptr->stat.wakeflg = 0 ; \
					}
#	define	initsema(sem, val)
#	define	appsema(sem, pri)
#	define	apvsema(sem, proc)
#endif
#include "../h/errno.h"
#include "../h/file.h"
#include "../h/inode.h"
#include "../h/dir.h"
#include "../h/signal.h"
#include "../h/proc.h"
#include "../h/user.h"
#include "../h/flock.h"

#define	p_epid	p_pid

/* region types */
#define	S_BEFORE	010
#define	S_START		020
#define	S_MIDDLE	030
#define	S_END		040
#define	S_AFTER		050
#define	E_BEFORE	001
#define	E_START		002
#define	E_MIDDLE	003
#define	E_END		004
#define	E_AFTER		005

extern	struct	flckinfo flckinfo;	/* configuration and acct info		*/
struct	filock	*frlock;		/* pointer to record lock free list	*/
struct	filock	*sleeplcks;		/* head of chain of sleeping locks	*/

#define	LOCALSYSID	0

/* build file lock free list
 */
flckinit()
{
	register i;

	initsema(&frlock_sem, 1);
	initsema(&slplck_sem, 1);

	for (i=0; i<flckinfo.recs; i++) {
		initsema(&flox[i].wakesem, 0);
		if (frlock == NULL) {
			flox[i].next = flox[i].prev = NULL;
			frlock = &flox[i];
		} else {
			flox[i].next = frlock;
			flox[i].prev = NULL;
			frlock = (frlock->prev = &flox[i]);
		}
	}
}

/* Insert lock (lckdat) after given lock (fl); If fl is NULL place the
 * new lock at the beginning of the list and update the head ptr to
 * list which is stored at the address given by lck_list. 
 */
struct filock *
insflck(lck_list, lckdat, fl)
struct	filock	**lck_list;
struct	filock	*fl;
struct	flock	*lckdat;
{
	register struct filock *new;

	appsema(&frlock_sem, PZERO);

	if ((new = frlock) != NULL) {
		++flckinfo.reccnt;
		++flckinfo.rectot;
		frlock = new->next;
		if (frlock != NULL)
			frlock->prev = NULL;
		apvsema(&frlock_sem, 0);
		new->set = *lckdat;
		new->set.l_pid = u.u_procp->p_epid;
		new->set.l_sysid = LOCALSYSID;
		new->stat.wakeflg = 0;
		if (fl == NULL) {
			new->next = *lck_list;
			if (new->next != NULL)
				new->next->prev = new;
			*lck_list = new;
		} else {
			new->next = fl->next;
			if (fl->next != NULL)
				fl->next->prev = new;
			fl->next = new;
		}
		new->prev = fl;
	} else {
		++flckinfo.recovf;
		apvsema(&frlock_sem, 0);
	}

	return (new);
}

/* Delete lock (fl) from the record lock list. If fl is the first lock
 * in the list, remove it and update the head ptr to the list which is
 * stored at the address given by lck_list.
 */
delflck(lck_list, fl)
struct filock  **lck_list;
struct filock  *fl;
{
	if (fl->prev != NULL)
		fl->prev->next = fl->next;
	else
		*lck_list = fl->next;
	if (fl->next != NULL)
		fl->next->prev = fl->prev;
	WAKEUP(fl);

	appsema(&frlock_sem, PZERO);
	--flckinfo.reccnt;
	if (frlock == NULL) {
		fl->next = fl->prev = NULL;
		frlock = fl;
	} else {
		fl->next = frlock;
		fl->prev = NULL;
		frlock = (frlock->prev = fl);
	}
	apvsema(&frlock_sem, 0);
}

/* regflck sets the type of span of this (un)lock relative to the specified
 * already existing locked section.
 * There are five regions:
 *
 *  S_BEFORE        S_START         S_MIDDLE         S_END          S_AFTER
 *     010            020             030             040             050
 *  E_BEFORE        E_START         E_MIDDLE         E_END          E_AFTER
 *      01             02              03              04              05
 * 			|-------------------------------|
 *
 * relative to the already locked section.  The type is two octal digits,
 * the 8's digit is the start type and the 1's digit is the end type.
 */
int
regflck(ld, flp)
struct flock *ld;
struct filock *flp;
{
	register int regntype;

	if (ld->l_start > flp->set.l_start) {
		if ((ld->l_start-1) == flp->set.l_end)
			return(S_END|E_AFTER);
		if (ld->l_start > flp->set.l_end)
			return(S_AFTER|E_AFTER);
		regntype = S_MIDDLE;
	} else if (ld->l_start == flp->set.l_start)
		regntype = S_START;
	else
		regntype = S_BEFORE;

	if (ld->l_end < flp->set.l_end) {
		if (ld->l_end == (flp->set.l_start-1))
			regntype |= E_START;
		else if (ld->l_end < flp->set.l_start)
			regntype |= E_BEFORE;
		else
			regntype |= E_MIDDLE;
	} else if (ld->l_end == flp->set.l_end)
		regntype |= E_END;
	else
		regntype |= E_AFTER;

	return (regntype);
}

/* Adjust file lock from region specified by 'ld', in the record
 * lock list indicated by the head ptr stored at the address given
 * by lck_list. Start updates at the lock given by 'insrtp'. It is 
 * assumed the list is ordered on starting position, relative to 
 * the beginning of the file, and no updating is required on any
 * locks in the list previous to the one pointed to by insrtp.
 * Insrtp is a result from the routine blocked().  Flckadj() scans
 * the list looking for locks owned by the process requesting the
 * new (un)lock :
 *
 * 	- If the new record (un)lock overlays an existing lock of
 * 	  a different type, the region overlaid is released.
 *
 * 	- If the new record (un)lock overlays or adjoins an exist-
 * 	  ing lock of the same type, the existing lock is deleted
 * 	  and its region is coalesced into the new (un)lock.
 *
 * When the list is sufficiently scanned and the new lock is not 
 * an unlock, the new lock is inserted into the appropriate
 * position in the list.
 */
flckadj(lck_list, insrtp, ld)
struct filock	**lck_list;
register struct filock *insrtp;
struct flock	*ld;
{
	register struct	filock	*flp, *nflp;
	int regtyp;

	nflp = (insrtp == NULL) ? *lck_list : insrtp;

	while (flp = nflp) {
		nflp = flp->next;
		if ((flp->set.l_pid == u.u_procp->p_epid)
		    && (flp->set.l_sysid == LOCALSYSID)) {

			/* release already locked region if necessary */

			switch (regtyp = regflck(ld, flp)) {
			case S_BEFORE|E_BEFORE:
				nflp = NULL;
				break;
			case S_BEFORE|E_START:
				if (ld->l_type == flp->set.l_type) {
					ld->l_end = flp->set.l_end;
					delflck(lck_list, flp);
				}
				nflp = NULL;
				break;
			case S_START|E_END:
				/* don't bother if this is in the middle of
				 * an already similarly set section.
				 */
				if (ld->l_type == flp->set.l_type)
					return(0);
			case S_START|E_AFTER:
				insrtp = flp->prev;
				delflck(lck_list, flp);
				break;
			case S_BEFORE|E_END:
				if (ld->l_type == flp->set.l_type)
					nflp = NULL;
			case S_BEFORE|E_AFTER:
				delflck(lck_list, flp);
				break;
			case S_START|E_MIDDLE:
				insrtp = flp->prev;
			case S_MIDDLE|E_MIDDLE:
				/* don't bother if this is in the middle of
				 * an already similarly set section.
				 */
				if (ld->l_type == flp->set.l_type)
					return(0);
			case S_BEFORE|E_MIDDLE:
				if (ld->l_type == flp->set.l_type)
					ld->l_end = flp->set.l_end;
				else {
					/* setup piece after end of (un)lock */
					register struct	filock *tdi, *tdp;
					struct flock td;

					td = flp->set;
					td.l_start = ld->l_end + 1;
					tdp = tdi = flp;
					do {
						if (tdp->set.l_start < td.l_start)
							tdi = tdp;
						else
							break;
					} while (tdp = tdp->next);
					if (insflck(lck_list, &td, tdi) == NULL)
						return(ENOLCK);
				}
				if (regtyp == (S_MIDDLE|E_MIDDLE)) {
					/* setup piece before (un)lock */
					flp->set.l_end = ld->l_start - 1;
					WAKEUP(flp);
					insrtp = flp;
				} else
					delflck(lck_list, flp);
				nflp = NULL;
				break;
			case S_MIDDLE|E_END:
				/* don't bother if this is in the middle of
				 * an already similarly set section.
				 */
				if (ld->l_type == flp->set.l_type)
					return(0);
				flp->set.l_end = ld->l_start - 1;
				WAKEUP(flp);
				insrtp = flp;
				break;
			case S_MIDDLE|E_AFTER:
				if (ld->l_type == flp->set.l_type) {
					ld->l_start = flp->set.l_start;
					insrtp = flp->prev;
					delflck(lck_list, flp);
				} else {
					flp->set.l_end = ld->l_start - 1;
					WAKEUP(flp);
					insrtp = flp;
				}
				break;
			case S_END|E_AFTER:
				if (ld->l_type == flp->set.l_type) {
					ld->l_start = flp->set.l_start;
					insrtp = flp->prev;
					delflck(lck_list, flp);
				}
				break;
			case S_AFTER|E_AFTER:
				insrtp = flp;
				break;
			}
		}
	}

	if (ld->l_type != F_UNLCK) {
		if (flp = insrtp) {
			do {
				if (flp->set.l_start < ld->l_start)
					insrtp = flp;
				else
					break;
			} while (flp = flp->next);
		}
		if (insflck(lck_list, ld, insrtp) == NULL)
			return(ENOLCK);
	}

	return (0);
}

/* blocked checks whether a new lock (lckdat) would be
 * blocked by a previously set lock owned by another process.
 * Insrt is set to point to the lock where lock list updating
 * should begin to place the new lock.
 */
struct filock *
blocked(flp, lckdat, insrt)
struct filock *flp;
struct flock  *lckdat;
struct filock **insrt;
{
	register struct filock *f;

	*insrt = NULL;
	for (f = flp; f != NULL; f = f->next) {
		if (f->set.l_start < lckdat->l_start)
			*insrt = f;
		else
			break;
		if ((f->set.l_pid == u.u_procp->p_epid)
		     && (f->set.l_sysid == LOCALSYSID)) {
			if ((lckdat->l_start-1) <= f->set.l_end)
				break;
		} else if ((lckdat->l_start <= f->set.l_end)
			    && (f->set.l_type == F_WRLCK
			        || (f->set.l_type == F_RDLCK
			            && lckdat->l_type == F_WRLCK)))
			return(f);
	}

	for (; f != NULL; f = f->next) {
		if (lckdat->l_end < f->set.l_start)
			break;
		if (lckdat->l_start <= f->set.l_end
		    && (f->set.l_pid != u.u_procp->p_epid
		        || f->set.l_sysid != LOCALSYSID)
		    && (f->set.l_type == F_WRLCK
		        || (f->set.l_type == F_RDLCK
		            && lckdat->l_type == F_WRLCK)))
			return(f);
	}

	return(NULL);
}

/*
 * get and set file/record locks
 *
 * cmd & SETFLCK indicates setting a lock.
 * cmd & SLPFLCK indicates waiting if there is a blocking lock.
 * cmd & INOFLCK indicates the inode associated with fp is locked.
 *
 */
reclock(ip, lckdat, cmd, flag, offset)
struct	inode  *ip;
struct	flock *lckdat;
int	cmd, flag;
off_t	offset;
{
	register struct filock  **lock_list, *sf;
	struct	filock *found, *insrt = NULL;
	int retval = 0;
	int contflg = 0;
	short whence;

	/* check access permissions */
	if ((cmd & SETFLCK)
	    && ((lckdat->l_type == F_RDLCK && (flag&FREAD)  == 0)
		|| (lckdat->l_type == F_WRLCK && (flag&FWRITE) == 0)))
		return (EBADF);
	
	/* Convert start to be relative to beginning of file */
	whence = lckdat->l_whence;
	if (retval = convoff(ip, lckdat, 0, offset))
		return (retval);

	/* Convert l_len to be the end of the rec lock l_end */
	if (lckdat->l_len < 0)
		return (EINVAL);
	if (lckdat->l_len == 0)
		lckdat->l_end = MAXEND;
	else
		lckdat->l_end += (lckdat->l_start - 1);

	/* check for arithmetic overflow */
	if (lckdat->l_start > lckdat->l_end)
		return (EINVAL);

	appsema(&ip->i_flcksem, PZERO);
	lock_list = (struct filock **)&ip->i_filocks;

	do {
		contflg = 0;
		switch (lckdat->l_type) {
		case F_RDLCK:
		case F_WRLCK:
			if ((found=blocked(*lock_list, lckdat, &insrt)) == NULL) {
				if (cmd & SETFLCK)
					retval = flckadj(lock_list, insrt, lckdat);
				else
					lckdat->l_type = F_UNLCK;
			} else if (cmd & SLPFLCK) {
				appsema(&slplck_sem, PZERO);
				/* do deadlock detection here */
				if (deadflck(found)) {
					apvsema(&slplck_sem, 0);
					retval = EDEADLK;
				} else if ((sf=insflck(&sleeplcks, lckdat, NULL)) == NULL) {
					apvsema(&slplck_sem, 0);
					retval = ENOLCK;
				} else {
					found->stat.wakeflg++;
					sf->stat.blk.pid = found->set.l_pid;
					sf->stat.blk.sysid = found->set.l_sysid;
					apvsema(&slplck_sem, 0);
					apvsema(&ip->i_flcksem, 0);
					if (cmd & INOFLCK)
						iunlock(ip);
					if (SLEEP(found, PCATCH|(PZERO+1)))
						retval = EINTR;
					else
						contflg = 1;
					appsema(&slplck_sem, PZERO);
					sf->stat.blk.pid = 0;
					sf->stat.blk.sysid = 0;
					delflck(&sleeplcks, sf);
					apvsema(&slplck_sem, 0);
					if (cmd & INOFLCK)
						ilock(ip);
					appsema(&ip->i_flcksem, PZERO);
				}
			} else if (cmd & SETFLCK) {
				retval = EAGAIN;
			} else {
				*lckdat = found->set;
			}
			break;
		case F_UNLCK:
			/* removing a file record lock */
			if (cmd & SETFLCK)
				retval = flckadj(lock_list, *lock_list, lckdat);
			break;
		default:
			/* invalid lock type */
			retval = EINVAL;
			break;
		}
	} while (contflg);

	apvsema(&ip->i_flcksem, 0);

	/* Restore l_len */
	if (lckdat->l_end == MAXEND)
		lckdat->l_len = 0;
	else
		lckdat->l_len -= (lckdat->l_start-1);
	convoff(ip, lckdat, whence, offset);

	return(retval);
}

/* convoff - converts the given data (start, whence) to the
 * given whence.
 */
int
convoff(ip, lckdat, whence, offset)
struct inode *ip;
struct flock *lckdat;
int whence;
off_t offset;
{
	if (lckdat->l_whence == 1)
		lckdat->l_start += offset;
	else if (lckdat->l_whence == 2)
		lckdat->l_start += ip->i_size;
	else if (lckdat->l_whence != 0)
		return (EINVAL);
	if (lckdat->l_start < 0)
		return (EINVAL);
	if (whence == 1)
		lckdat->l_start -= offset;
	else if (whence == 2)
		lckdat->l_start -= ip->i_size;
	else if (whence != 0)
		return (EINVAL);
	lckdat->l_whence = whence;
	return (0);
}

/* deadflck does the deadlock detection for the given record */
int
deadflck(flp)
struct filock *flp;
{
	register struct filock *blck, *sf;
	short blckpid, blcksysid;

	blck = flp;	/* current blocking lock pointer */
	blckpid = blck->set.l_pid;
	blcksysid = blck->set.l_sysid;
	do {
		if ((blckpid == u.u_procp->p_epid)
		    && (blcksysid == LOCALSYSID))
			return(1);
		/* if the blocking process is sleeping on a locked region,
		 * change the blocked lock to this one.
		 */
		for (sf = sleeplcks; sf != NULL; sf = sf->next) {
			if ((blckpid == sf->set.l_pid)
			    && (blcksysid == sf->set.l_sysid)) {
				blckpid = sf->stat.blk.pid;
				blcksysid = sf->stat.blk.sysid;
				break;
			}
		}
		blck = sf;
	} while (blck != NULL);
	return(0);
}

/* Clean up record locks left around by process (called in closef) */
cleanlocks(ip)
struct inode *ip;
{
	register struct filock *flp, *nflp, **lock_list;

	appsema(&ip->i_flcksem, PZERO);

	lock_list = (struct filock **)&ip->i_filocks;
	for (flp = *lock_list; flp!=NULL; flp=nflp) {
		nflp = flp->next;
		if ((flp->set.l_pid == u.u_procp->p_epid) &&
		    (flp->set.l_sysid == LOCALSYSID))
			delflck(lock_list, flp);
	};

	apvsema(&ip->i_flcksem, 0);
	return;
}
