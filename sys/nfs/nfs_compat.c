/*
 * Sun 4.2 NFS compatibility routines.
 */
#ifdef SVR3
# include "sys/debug.h"
# include "sys/types.h"
# include "sys/sysmacros.h"
# include "sys/param.h"
# include "sys/systm.h"
# include "sys/signal.h"
# include "sys/errno.h"
# include "sys/psw.h"
# include "sys/pcb.h"
# include "sys/user.h"
# include "sys/mbuf.h"
# include "sys/fs/nfs_compat.h"
# define malloc(nbytes)	kern_malloc(nbytes)
# define free(p)	kern_free(p)
#else
# include "../h/param.h"
# include "../h/systm.h"
# include "../h/user.h"
# include "../h/cmap.h"
# include "../vm/vmmac.h"
# include "machine/pte.h"
# include "../h/mbuf.h"
# include "../nfs/nfs_compat.h"
#endif

#ifdef NFSDEBUG
unsigned long kmem_used = 0;
#endif

/*
 * Kernel memory allocation functions.
 */
char *
kmem_alloc(nbytes)
	u_int nbytes;
{
	register char *p;

	p = malloc(nbytes);
	if (p == NULL) {
		panic("kmem_alloc: out of memory\n");
	}
#ifdef NFSDEBUG
	dprint(nfsdebug, 6, "kmem_alloc(%x, %d)\n", p, nbytes);
	kmem_used += nbytes;
#endif
	return p;
}

#ifdef NOTDEF
char *
kmem_realloc(p, nbytes)
	register char *p;
	u_int nbytes;
{
#ifdef NFSDEBUG
	dprint(nfsdebug, 6, "kmem_realloc(%x, %d)\n", (u_int) p, nbytes);
#endif
	p = realloc(p, nbytes);
	if (p == NULL) {
		panic("kmem_realloc: out of memory\n");
	}
	return p;
}
#endif

void
kmem_free(p, nbytes)
	register char *p;
	u_int nbytes;
{
#ifdef NFSDEBUG
	dprint(nfsdebug, 6, "kmem_free(%x, %d)\n", (u_int) p, nbytes);
	kmem_used -= nbytes;
#endif
	if (nbytes != 0) {	/* Sun-compatible hackery */
		free(p);
	}
}

#ifndef SVR3
caddr_t
kmem_allocmbuf(nbytes)
	u_int nbytes;
{
	register u_int clicks;
	register int kmx;
	register caddr_t p;

	clicks = btoc(nbytes);
	kmx = kmap_alloc(clicks, M_WAIT);
	ASSERT(kmx > 0);
	p = (caddr_t) kmxtob(kmx);
	if (vmemall(&Usrptmap[kmx], clicks, (struct proc *)0, CSYS) == 0) {
		kmfree(clicks, kmx);
		return 0;
	}
	vmaccess(&Usrptmap[kmx], (caddr_t) p, clicks);
	return p;
}

kmem_freembuf(p, nbytes)
	caddr_t p;
	u_int nbytes;
{
	register u_int clicks;
	register int kmx;

	clicks = btoc(nbytes);
	kmx = btokmx((struct pte *) p);
	memfree(&Usrptmap[kmx], clicks, 0);
	kmfree(clicks, kmx);
}
#endif

#ifdef NOTDEF
kmem_access(p, nbytes)
	register caddr_t p;
	u_int nbytes;
{
	register int kmx;
	register int pf;

	kmx = btokmx((struct pte *) p);
	ASSERT((0 <= kmx) && (kmx < USRPTSIZE));
	/* Do something with Usrptmap[kmx] */
}
#endif

/*
 * Routines to allocate and free credentials structures
 */
#ifdef SVR3
extern int	crdebug;
#else
int crdebug = 0;
#endif
int cractive = 0;
#define	caller()	0	/* asm.sed expands to 16(fp) on vax */

/*
 * Initialize a cred structure from a user structure.  Set the reference
 * count to one, indicating valid credentials.
 */
crinit(up, cr)
	register struct user *up;
	register struct ucred *cr;
{
	cr->cr_uid = up->u_uid;
	cr->cr_gid = up->u_gid;
	cr->cr_ruid = up->u_ruid;
	cr->cr_rgid = up->u_rgid;
	cr->cr_ref = 1;
}

/*
 * Set a user's credentials from a given cred structure.
 */
cruse(cr, up)
	register struct ucred *cr;
	register struct user *up;
{
	ASSERT(cr->cr_ref > 0);
	up->u_uid = cr->cr_uid;
	up->u_gid = cr->cr_gid;
	up->u_ruid = cr->cr_ruid;
	up->u_rgid = cr->cr_rgid;
}

/*
 * Hold a cred structure.
 */
crhold(cr)
	struct ucred *cr;
{
	if (crdebug) {
		printf("crhold %x %d %d %x\n",
		    cr, cr->cr_uid, cr->cr_ref, caller());
	}
	cr->cr_ref++;
}

/*
 * Allocate a zeroed cred structure and crhold it.
 */
struct ucred *
crget()
{
	struct ucred *cr;

	cr = (struct ucred *)kmem_alloc(sizeof(*cr));
	bzero(cr, sizeof(*cr));
	crhold(cr);
	cractive++;
	return(cr);
}

/*
 * Free a cred structure.
 * Throws away space when ref count gets to 0.
 */
crfree(cr)
	struct ucred *cr;
{
	int	s = spl6();

	if (crdebug)
		printf("crfree %x %d %d %x\n",
		    cr, cr->cr_uid, cr->cr_ref, caller());
	if (--cr->cr_ref > 0) {
		splx(s);
		return;
	}
	kmem_free(cr, sizeof(*cr));
	cractive--;
	splx(s);
}

#ifdef NOTDEF
/*
 * Copy cred structure to a new one and free the old one.
 */
struct ucred *
crcopy(cr)
	struct ucred *cr;
{
	struct ucred *newcr;

	newcr = crget();
	*newcr = *cr;
	crfree(cr);
	newcr->cr_ref = 1;
/*	printf("crcopy: old %x %d %d new %x\n", cr, cr->cr_uid,
	    cr->cr_ref, newcr);*/
	return(newcr);
}
#endif

/*
 * Dup cred struct to a new held one.
 */
struct ucred *
crdup(cr)
	struct ucred *cr;
{
	struct ucred *newcr;

	newcr = crget();
	*newcr = *cr;
	newcr->cr_ref = 1;
/*	printf("crdup: old %x %d %d new %x\n", cr, cr->cr_uid,
	    cr->cr_ref, newcr);*/
	return(newcr);
}

/*
 * Get a data mbuf and point it at memory starting at addr and running
 * len bytes.  Set the mbuf free function and argument to fun and arg.
 */
/* ARGSUSED */
struct mbuf *
mclgetx(ffun, farg, dfun, darg, addr, len, wait)
	int (*ffun)(), (*dfun)(), len, wait;
	long farg, darg;
	caddr_t addr;
{
	register struct mbuf *m;

	MGET(m, wait, MT_DATA);
	if (m == 0)
		return (0);
	m->m_off = (int)addr - (int)m;
	m->m_len = len;
	m->m_freefunc = ffun;
	m->m_farg = farg;
	m->m_dupfunc = dfun;
	m->m_darg = darg;
	return (m);
}
