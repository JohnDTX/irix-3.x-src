#ifndef __nfs_compat__
#define	__nfs_compat__
/*
 * Sun nfs compatibility with bsd and with at&t.
 *
 * $Source: /d2/3.7/src/sys/nfs/RCS/nfs_compat.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:33:08 $
 */
#ifdef SVR3
# include "sys/cmn_err.h"
# define iprintf	printf
# define ilock(ip)	plock(ip)
# define iunlock(ip)	prele(ip)
# define iuse(ip)	((ip)->i_count++)
# define iunuse(ip)	{ plock(ip); iput(ip); }
# define IHOLD(ip)	{ iuse(ip); ilock(ip); }
# ifdef ILOCKED
#  undef ILOCKED
# endif
# define ILOCKED	ILOCK
# define IXSAVED	ITEXT
# define xflush(ip)	xrele(ip)
# define USERPATH	upath
# define BLKDEV_IOSIZE	4096	/* XXX see fs/efs/bmap.c */
# define howmany(x, y)	(((x)+((y)-1))/(y))
# define roundup(x, y)	((((x)+((y)-1))/(y))*(y))

/* AT&T-compatible inode private data pointer type */
typedef	int	*fsptr_t;
#else
/* a better inode private data pointer type */
typedef	char	*fsptr_t;
#endif

/*
 * Kernel memory allocation functions.  Use kmem_allocmbuf() and
 * kmem_freembuf() if the memory you're manipulating will be wrapped in
 * an mbuf and shipped to a network interface.
 *	char	*p;
 *	u_int	nbytes;
 */
char	*kmem_alloc(/* nbytes */);
#ifdef NOTDEF
char	*kmem_realloc(/* p, nbytes */);
#endif
void	kmem_free(/* p, nbytes */);

#ifdef SVR3
# define kmem_allocmbuf(nbytes)	 kmem_alloc(nbytes)
# define kmem_freembuf(p,nbytes) kmem_free(p,nbytes)
#else
char	*kmem_allocmbuf(/* nbytes */);
int	kmem_freembuf(/* p, nbytes */);
#endif

/* user credentials */
#define	NOGROUP	-1
#define	NGROUPS	1	/* XXX 8 in sun-land */

struct ucred {
	u_short	cr_ref;			/* reference count */
	short   cr_uid;			/* effective user id */
	short   cr_gid;			/* effective group id */
	int     cr_groups[NGROUPS];	/* groups, 0 terminated */
	short   cr_ruid;		/* real user id */
	short   cr_rgid;		/* real group id */
};

#ifdef KERNEL
extern int	nfsdebug;

struct ucred *crget();
struct ucred *crcopy();
struct ucred *crdup();
#endif

/*
 * Get an mbuf which points at non-mbuf memory.
 *	int	(*ffun)();	// free function
 *	long	farg;		// argument thereto
 *	int	(*dfun)();	// duplicate function
 *	long	darg;		// argument thereto
 *	caddr_t addr;		// non-mbuf memory base
 *	int	len;		// non-mbuf memory size
 *	int	wait;		// TRUE if we can sleep
 */
struct mbuf	*mclgetx(/* ffun, farg, dfun, darg, addr, len, wait */);

#endif	/* __nfs_compat__ */
