/*
 * System V shared memory
 *
 * $Source: /d2/3.7/src/sys/sys/RCS/shm.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:35:30 $
 */
#include "../h/types.h"
#include "../h/param.h"
#include "../h/sysmacros.h"
#include "../h/dir.h"
#include "../h/errno.h"
#include "../h/signal.h"
#include "../h/user.h"
#include "../h/ipc.h"
#include "../h/shm.h"
#include "../h/seg.h"
#include "../h/proc.h"
#include "../h/systm.h"
#include "../vm/vm.h"
#include "../h/cmap.h"
#include "machine/pte.h"

#undef	SHM_NOISE
#undef	SHM_DEBUG

/* XXX */
/*
 * 1. Fix it so that the segments can be paged
 * 2. Fix the sbrk() code so that we insure that we don't sbrk() into the
 *    shared or phys segments
 * 3. Lift most of the aribtray limits in param.c
 */
#define	MAXADDR		0x400000	/* 4 megabytes */


struct	shmid_ds *ipcget(), *shmconv();

int	shmtot;

/*
**	shmat - Shmat system call.
*/
shmat()
{
	register struct a {
		int	shmid;
		char	*addr;
		int	flag;
	} *uap = (struct a *)u.u_ap;
	register struct shmid_ds *sp;
	register struct shmid_ds **slot;
	register struct shmpt_ds *map;
	register int i;
	register int size;
	register int segbeg;

	if ((sp = shmconv(uap->shmid, SHM_DEST)) == NULL)
		return;
	if (ipcaccess(&sp->shm_perm, SHM_R))
		return;
	if ((uap->flag & SHM_RDONLY) == 0)
		if (ipcaccess(&sp->shm_perm, SHM_W))
			return;
	/*
	 * Find first free slot for user to attach segment to
	 */
	i = u.u_procp->p_ndx * shminfo.shmseg;
	slot = &shm_shmem[i];
	map = &shm_ptbl[i];
	for (i = shminfo.shmseg; --i >= 0; slot++, map++) {
		if (*slot == NULL)
			goto gotit;
	}
	u.u_error = EMFILE;
	return;

gotit:
	if (uap->flag & SHM_RND)
		uap->addr = (char *)((uint)uap->addr & ~(SHMLBA - 1));

	/*
	 * Check for page alignment and containment within P0
	 */
	if (((uint)uap->addr & (ctob(1) - 1)) ||
	    ((uint)uap->addr > MAXADDR) ||
	    ((uint)uap->addr + sp->shm_segsz > MAXADDR)) {
		u.u_error = EINVAL;
		return;
	}

	/*
	 * Record generic info
	 */
	size = btoc(sp->shm_segsz);
	*slot = sp;
	map->shm_sflg = RW;
	if (uap->flag & SHM_RDONLY)
		map->shm_sflg = RO;

	/*
	 * An address of 0 places the shared memory into a first
	 * fit location
	 */
	if (uap->addr == NULL) {
		if (u.u_procp->p_smend)
			segbeg = u.u_procp->p_smend;
		else
			segbeg = u.u_procp->p_loadc + u.u_tsize + u.u_dsize +
				shminfo.shmbrk;
		for (;;) {
			map->shm_segbeg = segbeg;
			if (ckoverlap() == 0) {
#ifdef	SHM_NOISE
printf("shmat: picked addr %x\n", ctob(segbeg));
#endif
				break;
			}
			/*
			 * Sigh.  That didn't work.  Try furthur out.
			 */
			segbeg += shminfo.shmbrk;
			if (segbeg > btoc(MAXADDR)) {
				/*
				 * Oh well.  No place to do it...
				 */
				u.u_error = ENOMEM;
				*slot = NULL;
			}
		}
	} else {
		/*
		 * Check to make sure segment does not overlay any
		 * other valid segments
		 */
		map->shm_segbeg = btoc(uap->addr);
		if (ckoverlap()) {
			u.u_error = ENOMEM;
			*slot = NULL;
			return;
		}
	}

	/*
	 * Blow away current page mapping, forcing new page mapping
	 * to be acquired before the user runs again.
	 */
	cxput(u.u_procp, 1);

	/*
	 * Clear segment on first attach
	 */
	if (sp->shm_perm.mode & SHM_CLEAR) {
		register struct pte *pte;

		pte = sp->shm_ptbl;
		i = size;
		while (--i >= 0) {
			clearseg(pte->pg_pfnum);
			pte++;
		}
		sp->shm_perm.mode &= ~SHM_CLEAR;
	}
	sp->shm_nattch++;
	sp->shm_cnattch++;
	sp->shm_atime = time;
	sp->shm_lpid = u.u_procp->p_pid;
	u.u_procp->p_flag |= SSHMEM;
	if (map->shm_segbeg + size > u.u_procp->p_smend)
		u.u_procp->p_smend = map->shm_segbeg + size;

	/* return user address of segment */
	u.u_rval1 = ctob(map->shm_segbeg);
}

/*
**	shmconv - Convert user supplied shmid into a ptr to the associated
**		shared memory header.
*/
struct shmid_ds *
shmconv(shmid, flg)
	register int shmid;
	int flg;
{
	register struct shmid_ds *sp;

	sp = &shmem[shmid % shminfo.shmmni];
	if(!(sp->shm_perm.mode & IPC_ALLOC) || (sp->shm_perm.mode & flg) ||
	    (shmid / shminfo.shmmni != sp->shm_perm.seq)) {
#ifdef	SHM_DEBUG
printf("shmconv failure: mode=%o flg=%o shmid=%d shminfo.shmmni=%d seq=%d\n",
		sp->shm_perm.mode, flg, shmid, shminfo.shmmni,
		sp->shm_perm.seq);
#endif
		u.u_error = EINVAL;
		return (NULL);
	}
	return (sp);
}

/*
** shmctl:
**	- Shmctl system call.
*/
shmctl()
{
	register struct a {
		int	shmid;
		int	cmd;
		struct	shmid_ds *arg;
	} *uap = (struct a *)u.u_ap;
	register struct shmid_ds *sp;
	struct shmid_ds ds;

	if ((sp = shmconv(uap->shmid,
			  (uap->cmd == IPC_STAT) ? 0 : SHM_DEST)) == NULL)
		return;
	u.u_rval1 = 0;
	switch(uap->cmd) {
	    /* Remove shared memory identifier. */
	    case IPC_RMID:
		if ((u.u_uid != sp->shm_perm.uid) &&
		    (u.u_uid != sp->shm_perm.cuid) && !suser())
			return;
		sp->shm_ctime = time;
		sp->shm_perm.mode |= SHM_DEST;

		/* Change key to private so old key can be reused without
			waiting for last detach.  Only allowed accesses to
			this segment now are shmdt() and shmctl(IPC_STAT).
			All others will give bad shmid. */
		sp->shm_perm.key = IPC_PRIVATE;

		/* Adjust counts to counter shmfree decrements. */
		sp->shm_nattch++;
		sp->shm_cnattch++;
		shmfree(sp);
		return;

	    /* Set ownership and permissions. */
	    case IPC_SET:
		if ((u.u_uid != sp->shm_perm.uid) &&
		    (u.u_uid != sp->shm_perm.cuid) && !suser())
			return;
		if (copyin((caddr_t)uap->arg, (caddr_t)&ds, sizeof(ds))) {
			u.u_error = EFAULT;
			return;
		}
		sp->shm_perm.uid = ds.shm_perm.uid;
		sp->shm_perm.gid = ds.shm_perm.gid;
		sp->shm_perm.mode = (ds.shm_perm.mode & 0777) |
			(sp->shm_perm.mode & ~0777);
		sp->shm_ctime = time;
		return;

	    /* Get shared memory data structure. */
	    case IPC_STAT:
		if (ipcaccess(&sp->shm_perm, SHM_R))
			return;
		if (copyout((caddr_t)sp, (caddr_t)uap->arg, sizeof(*sp)))
			u.u_error = EFAULT;
		return;

	default:
		u.u_error = EINVAL;
		return;
	}
}

/*
** shmdt:
**	- Shmdt system call
**	- detach the given segment from the users address space
*/
shmdt()
{
	struct a {
		char	*addr;
	} *uap = (struct a *)u.u_ap;
	register struct shmid_ds **slot;
	register struct shmid_ds *sp;
	register struct shmpt_ds *map;
	register int i;
	register int segbeg;

	/*
	 * Check address for validity
	 */
	if ((int)uap->addr & (ctob(1) - 1) ||
	    (segbeg = btoc(uap->addr)) == 0) {
		u.u_error = EINVAL;
		return;
	}

	/*
	 * find segment
	 */
	i = u.u_procp->p_ndx * shminfo.shmseg;
	slot = &shm_shmem[i];
	map = &shm_ptbl[i];
	for (i = shminfo.shmseg; --i >= 0; slot++, map++) {
		sp = *slot;
		if (sp && (map->shm_segbeg == segbeg)) {
			/*
			 * Get rid of hardware mapping, forcing user to get it
			 * back when this system call completes.  This is how
			 * the mapping for the shared memory segment is removed
			 * from the users address space.
			 */
			cxput(u.u_procp, 1);
			shmfree(sp);
			*slot = NULL;
			sp->shm_dtime = time;
			sp->shm_lpid = u.u_procp->p_pid;
			return;
		}
	}
	u.u_error = EINVAL;
}

/*
** shmexec:
**	- Called by getxfile() to handle shared memory exec processing
**	- during an exec, the shared memory slots which are attached are
**	  detached
*/
shmexec()
{
	register struct shmid_ds **slot;
	register int i;

	if ((u.u_procp->p_flag & SSHMEM) == 0)
		return;

	/* detach any shared memory segments */
	slot = &shm_shmem[u.u_procp->p_ndx * shminfo.shmseg];
	for (i = shminfo.shmseg; --i >= 0; slot++) {
		if (*slot) {
			shmfree(*slot);
			*slot = NULL;
		}
	}
	cxput(u.u_procp, 1);
	u.u_procp->p_flag &= ~SSHMEM;
	u.u_procp->p_smend = 0;
}

/*
** shmexit:
**	- Called by exit to clean up on process exit
*/
shmexit()
{
	shmexec();
}

/*
** shmfork:
**	- Called by newproc() to handle shared memory fork
**	  processing
**	- child gets access to parents shared segments
*/
shmfork(childp, parentp)
	struct proc *childp, *parentp;
{
	register int i;
	register struct shmid_ds *sp;
	register struct shmid_ds **parent_slot;
	register struct shmid_ds **child_slot;
	register struct shmpt_ds *child_map, *parent_map;
	register int spot;

	spot = childp->p_ndx * shminfo.shmseg;
	child_slot = &shm_shmem[spot];
	child_map = &shm_ptbl[spot];

	spot = parentp->p_ndx * shminfo.shmseg;
	parent_slot = &shm_shmem[spot];
	parent_map = &shm_ptbl[spot];

	for (i = shminfo.shmseg; --i >= 0; ) {
		sp = *parent_slot;
		if (sp) {
			sp->shm_nattch++;
			sp->shm_cnattch++;
			*child_slot = sp;
			*child_map = *parent_map;
		}
		parent_slot++;
		parent_map++;
		child_slot++;
		child_map++;
	}
	if (parentp->p_flag & SSHMEM) {
		childp->p_smend = parentp->p_smend;
		childp->p_flag |= SSHMEM;
	}
}

/*
** shmfree:
**	- Decrement counts
**	- Free segment and page tables if indicated
*/
shmfree(sp)
	register struct shmid_ds *sp;
{
	register int size;

	if (sp == NULL)
		return;
	sp->shm_nattch--;
	sp->shm_cnattch--;
#ifdef	ASSERT
	if ((sp->shm_cnattch < 0) || (sp->shm_nattch < 0)) {
		printf("sp=%x nattch=%d cnattch=%d\n",
			      sp, sp->shm_nattch, sp->shm_cnattch);
		debug("shmfree");
	}
#endif
	if ((sp->shm_cnattch <= 0) && (sp->shm_perm.mode & SHM_DEST)) {
#ifdef	SHM_NOISE
printf("shmfree: %d\n", sp->shm_perm.seq * shminfo.shmmni + (sp - shmem));
#endif	SHM_NOISE
		/*
		 * Free page table and memory attached to page table
		 */
		size = btoc(sp->shm_segsz);
		(void) vmemfree(sp->shm_ptbl, size);
		free((caddr_t) sp->shm_ptbl);
		sp->shm_perm.mode = 0;
		shmtot -= size;
		if (((int)(++(sp->shm_perm.seq) * shminfo.shmmni
			     + (sp - shmem))) < 0)
			sp->shm_perm.seq = 0;
	}
}

/*
** shmget:
**	- Shmget system call.
*/
shmget()
{
	register struct a {
		key_t	key;
		int	size,
			shmflg;
	} *uap = (struct a *)u.u_ap;
	register struct shmid_ds *sp;
	int s;
	int size;

	if ((sp = ipcget(uap->key, uap->shmflg, shmem, shminfo.shmmni,
				   sizeof(*sp), &s)) == NULL)
		return;

	if (s) {
		/*
		 * This is a new shared memory segment.  Allocate memory and
		 * finish initialization
		 */
		if ((uap->size < shminfo.shmmin) ||
		    (uap->size > shminfo.shmmax)) {
			u.u_error = EINVAL;
			sp->shm_perm.mode = 0;
			return;
		}
		size = btoc(uap->size);
		if (shmtot + size > shminfo.shmall) {
			u.u_error = ENOMEM;
			sp->shm_perm.mode = 0;
			return;
		}
		sp->shm_segsz = uap->size;
		sp->shm_ptbl = (struct pte *)calloc(size, sizeof(struct pte));
/* XXX calling vmemall here is not quite right! */
		if ((sp->shm_ptbl == NULL) ||
		    (vmemall(sp->shm_ptbl, size, u.u_procp, CSYS) == 0)) {
			u.u_error = ENOMEM;
			sp->shm_perm.mode = 0;
			if (sp->shm_ptbl)
				free((caddr_t) sp->shm_ptbl);
			return;
		}

		/* adjust maxmem for the segment and page table */
		shmtot += size;

		sp->shm_perm.mode |= SHM_CLEAR;
		sp->shm_nattch = sp->shm_cnattch = 0;
		sp->shm_atime = sp->shm_dtime = 0;
		sp->shm_ctime = time;
		sp->shm_lpid = 0;
		sp->shm_cpid = u.u_procp->p_pid;
	} else {
		if (uap->size && (uap->size != sp->shm_segsz)) {
			u.u_error = EINVAL;
			return;
		}
	}
	u.u_rval1 = sp->shm_perm.seq * shminfo.shmmni + (sp - shmem);
#ifdef	SHM_NOISE
printf("shmget: %d\n", u.u_rval1);
#endif	SHM_NOISE
}

/*
** shmsys:
**	- system entry point for shmat, shmctl, shmdt, and shmget
**	  system calls
*/
shmsys()
{
	register struct a {
		uint	id;
	} *uap = (struct a *)u.u_ap;
	static int (*calls[])() = {shmat, shmctl, shmdt, shmget};

	if (uap->id > 3) {
		u.u_error = EINVAL;
		return;
	}
	u.u_ap = &u.u_arg[1];
	(*calls[uap->id])();
}

#ifdef	notdef
shm_dumptable(pte, n)
	register struct pte *pte;
	register int n;
{
	while (--n >= 0) {
		printf("%4x ", pte->pg_pfnum);
		pte++;
	}
	printf("\n");
}
#endif	notdef
