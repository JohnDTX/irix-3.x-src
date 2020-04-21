/*
 * Filesystem type-independent inode routines.
 *
 * An LRU list and hash table form the inode cache.  Between public operations
 * defined herein, an inode may be in only one of the following states:
 *
 *	1.  Free - the inode has been destroyed or has never been referenced
 *		ip is on the freelist
 *		ip is not on a hash chain
 *		ip->i_mntdev == NULL
 *		ip->i_flag == 0
 *		ip->i_count == 0
 *
 *	2.  Checked-out - the inode has been allocated via getinode() by a
 *	    filesystem which does not cache inodes.
 *		ip is not on the freelist
 *		ip is on a null hash chain (chained to itself)
 *		ip->i_mntdev and ip->i_flag are indeterminate
 *		ip->i_count > 0
 *
 *	3.  Busy - the inode is not on the freelist, but *is* hashed
 *		ip is not on the freelist
 *		ip is on a hash chain
 *		ip->i_mntdev != NULL
 *		(ip->i_flag & IVALID) == 0
 *		ip->i_count > 0
 *
 *	4.  Cached - the inode was once active, but has no references now
 *		ip is on the freelist
 *		ip is on a hash chain
 *		ip->i_mntdev != NULL
 *		(ip->i_flag & IVALID)
 *		ip->i_count == 0
 *
 * An inode not on the freelist has ip->i_fforw == NULL && ip->i_fback == NULL.
 * An inode not on a hash chain has ip->i_hforw == NULL && ip->i_hback == NULL.
 * A Checked-out inode has ip->i_hforw == NULL && ip->i_hback == ip.
 *
 * Thus the following transitions on the named operations are possible:
 *
 *		getinode	iget		iput/iunuse	iuncache/iflush
 *-----------------------------------------------------------------------------
 * Free	      |	Checked-out	Busy
 * Checked-out|					Free		Free
 * Busy       |					Free/Cached*
 * Cached     |	Checked-out	Busy				Free
 *
 * (*)	An inode in the Busy state goes to Cached if its filesystem type caches
 *	inodes, otherwise to Free.
 *
 * $Source: /d2/3.7/src/sys/sys/RCS/fs_inode.c,v $
 * $Date: 89/03/27 17:35:15 $
 * $Revision: 1.1 $
 */
#include "../h/param.h"
#include "../h/systm.h"
#include "../h/user.h"
#include "../h/file.h"
#include "../h/fstyp.h"
#include "../h/mount.h"
#include "../h/inode.h"
#include "../h/buf.h"
#include "../h/sysinfo.h"

/*
 * The inode table, from which inodes are allocated.  ifreepool is the
 * circular freelist header of inode structures for allocation.
 */
struct inode	*inode;
struct inode	*inodeNINODE;
struct ilinks	ifreepool;

#ifdef	OS_METER
struct {
	long	hits;		/* inode cache hits */
	long	misses;		/* cache misses */
	long	dirhits;	/* directory inode hits */
	long	discards;	/* valid inodes bumped from cache */
	long	hashwaits;	/* igets which waited on hash lock */
} inometer;
#endif	/* OS_METER */

/*
 * Inode hashing is used for quick lookup of an inode given its mount
 * structure and number.  IHASHSIZE is the number of hash bucket headers.
 */
#define	IHASHBITS		7
#define	IHASHSIZE		(1<<IHASHBITS)

struct ihash {
	struct inode	*ih_chain;	/* dereferenced as i_hforw */
};

static struct ihash	ihash[IHASHSIZE];
static unsigned char	ihashflags[IHASHSIZE];

#define	IHASHLOCK	0x1	/* bucket is locked for iget search */
#define	IHASHWANT	0x2	/* someone wants to lock */

/* hash bucket locking */
#define	LOCK_HASH(hf) { \
	while (*(hf) & IHASHLOCK) { \
		METER(inometer.hashwaits++); \
		*(hf) |= IHASHWANT; \
		sleep((caddr_t) hf, PINOD); \
	} \
	*(hf) |= IHASHLOCK; \
}
#define	UNLOCK_HASH(hf) { \
	if (*(hf) & IHASHWANT) { \
		*(hf) &= ~IHASHWANT; \
		wakeup((caddr_t) hf); \
	} \
	*(hf) &= ~IHASHLOCK; \
}

/* hash link management */
#define	INS_HASH(ip, ih) { \
	if (((ip)->i_hforw = (ih)->ih_chain) != NULL) \
		(ip)->i_hforw->i_hback = (ip); \
	(ip)->i_hback = (struct inode *)(ih); \
	(ih)->ih_chain = (ip); \
}
#define	REM_HASH(ip) { \
	ASSERT((ip)->i_hback); \
	if (((ip)->i_hback->i_hforw = (ip)->i_hforw) != NULL) { \
		ASSERT((ip)->i_hforw); \
		(ip)->i_hforw->i_hback = (ip)->i_hback; \
	} \
	(ip)->i_hforw = (ip)->i_hback = NULL; \
}
#define	NULL_HASH(ip) \
	((ip)->i_hforw = NULL, (ip)->i_hback = (ip))

/* freelist link management */
#define	INS_FREE(ip, il) { \
	(ip)->i_fforw = ((struct inode *) (il))->i_fforw; \
	(ip)->i_fback = (struct inode *) (il); \
	(ip)->i_fforw->i_fback = (ip)->i_fback->i_fforw = (ip); \
}
#define	REM_FREE(ip) { \
	ASSERT((ip)->i_fforw && (ip)->i_fback); \
	(ip)->i_fforw->i_fback = (ip)->i_fback; \
	(ip)->i_fback->i_fforw = (ip)->i_fforw; \
	(ip)->i_fforw = (ip)->i_fback = NULL; \
}
#define	NULL_FREE(il) \
	((il)->il_fforw = (il)->il_fback = (struct inode *) (il))

/*
 * Initialize the inode freelist to contain all inodes.  We assume that kernel
 * memory has been cleared correctly.
 */
init_inodes()
{
	register struct inode *ip;
	register struct ilinks *ifp;
	register short i;

	ifp = &ifreepool;
	NULL_FREE(ifp);
	ip = inode;
	for (i = ninode; i > 0; --i) {
		INS_FREE(ip, ifp);
		ip++;
	}
	ASSERT(ip == inodeNINODE);
}

/*
 * Allocate an inode for mp and fstyp from the freelist.  Return the inode
 * with a reference count of 1.
 */
static struct inode *
ialloc(mp, fstyp, dev)
	register struct mount *mp;
	register short fstyp;
	register dev_t dev;
{
	register struct inode *ip;

	/*
	 * Take from head of ifreelist.  If valid, remove from old hash chain
	 * in preparation for re-use.
	 */
	ip = ifreepool.il_fforw;
	if (ip == (struct inode *) &ifreepool) {
		return NULL;
	}
	REM_FREE(ip);
	if (ip->i_flag & IVALID) {
		ASSERT(ip->i_mntdev);
		REM_HASH(ip);

		/*
		 * Call FS_IPUT to release filesystem-private resources.
		 */
		FS_IPUT(ip);
		ip->i_flag &= ~IVALID;
		METER(inometer.discards++);
	} else {
		ASSERT(ip->i_mntdev == NULL);
		ASSERT(ip->i_flag == 0);
		ASSERT(ip->i_hforw == NULL && ip->i_hback == NULL);
	}
	ASSERT(ip->i_fsptr == NULL);

	/*
	 * Initialize some inode state.  Note that we initialize only those
	 * fields for which our caller passed initial values and those fields
	 * which are filesystem-independent.
	 */
	ip->i_flag = ILOCKED;
	ip->i_mntdev = mp;
	ip->i_mnton = NULL;
	ip->i_fstyp = fstyp;
	ip->i_fstypp = &fstypsw[fstyp];
	ip->i_dev = dev;
	ASSERT(ip->i_count == 0);
	iuse(ip);
	return (ip);
}

/*
 * Allocate an inode for mp and fstyp, and put it on a null hash chain.
 * Such an inode may be freely used by filesystems which do not cache inodes.
 */
struct inode *
getinode(mp, fstyp, dev)
	register struct mount *mp;
	register short fstyp;
	register dev_t dev;
{
	register struct inode *ip;

	ip = ialloc(mp, fstyp, dev);
	if (ip) {
		NULL_HASH(ip);
	}
	return ip;
}

/*
 * Look up an inode by its mount structure and number.
 * If it is in core (in the inode structure), honor the locking protocol.
 * If it is not in core, read it in from the specified device.
 * If the inode is mounted on, perform the indicated indirection.
 * In all cases, a pointer to a locked inode structure is returned.
 *
 * Printf warning: no inodes -- if the inode structure is full
 * panic: no imt -- if the mounted filesystem is not in the mount table.
 *	"cannot happen"
 */
struct inode *
iget(mp, inum)
	register struct mount *mp;
	register ino_t inum;
{
	register int hash;
	register struct ihash *ih;
	register unsigned char *hf;
	register struct inode *ip;

	sysinfo.iget++;
	/*
	 * Look for inode on hash chains first.
	 */
	hash = (int)inum & (IHASHSIZE-1);
	ih = &ihash[hash];
	hf = &ihashflags[hash];
	for (;;) {
		LOCK_HASH(hf);
		for (ip = ih->ih_chain; ; ip = ip->i_hforw) {
			if (ip == NULL) {
				/*
				 * Cache miss: allocate a new inode, link it
				 * onto its hash chain, and fill it in.
				 */
				METER(inometer.misses++);
				ip = ialloc(mp, mp->m_fstyp, mp->m_dev);
				if (ip == NULL) {
					u.u_error = ENFILE;
					UNLOCK_HASH(hf);
					syserr.procovf++;
					return NULL;
				}
				ip->i_number = inum;
				INS_HASH(ip, ih);	/* ih is locked */
				UNLOCK_HASH(hf);
				return FS_IREAD(ip);	/* ip is locked */
			}

			if (ip->i_number == inum && ip->i_mntdev == mp) {
				/*
				 * Cache hit: if valid, delete from freelist
				 * and leave on old hash chain.
				 */
				if (ip->i_flag & IVALID) {
					ASSERT(ip->i_fforw && ip->i_fback);
					ASSERT(ip->i_count == 0);
#ifdef OS_METER
					if (ip->i_ftype == IFDIR) {
						inometer.dirhits++;
					}
#endif
					METER(inometer.hits++);
					REM_FREE(ip);
					ip->i_flag &= ~IVALID;
				}
				break;
			}
		}

		/*
		 * We found a cache entry for inum, so let's release the
		 * hash lock and wait for the inode to be unlocked.
		 */
		UNLOCK_HASH(hf);
		if ((ip->i_flag & ILOCKED) == 0) {
			break;		/* it's ours */
		}
		ip->i_flag |= IWANT;
		(void) sleep((caddr_t)ip, PINOD);
	}

	/*
	 * If ip has a filesystem mounted on it, get the filesystem which
	 * surmounts ip.
	 */
	if ((ip->i_flag & IMOUNT) != 0) {
		if (ip->i_mnton == NULL) {
			panic("iget: no imt");
		}
		ip = ip->i_mnton->m_mount;
		ASSERT(ip);
		ASSERT((ip->i_flag & IVALID) == 0);
		IHOLD(ip);	/* use and lock */
		return ip;
	}

	/*
	 * We have the desired inode, unlocked.  Lock it, note the new
	 * reference to it, and return it.
	 */
	ip->i_flag |= ILOCKED;
	iuse(ip);
	return ip;
}

/*
 * Dereference a locked inode.  Call FS_IPUT on last dereference.
 */
iput(ip)
	register struct inode *ip;
{
	ASSERT(ip->i_flag & ILOCKED);
	ASSERT(ip->i_fforw == NULL && ip->i_fback == NULL);
	ASSERT(ip->i_hback);
	ASSERT((ip->i_flag & IVALID) == 0);
	ASSERT(ip->i_count > 0);
	if (ip->i_count > 1) {
		--ip->i_count;
		IUNLOCK(ip);
		return;
	}

	/*
	 * Last reference: sever stream head connection, call filesystem
	 * implementation, and unlock.
	 */
	if (ip->i_ftype == IFCHR) {
		ip->i_sptr = NULL;
	}
	ip->i_flag |= IFLUX;
	FS_IPUT(ip);

	/*
	 * NB: the placement of iunlock() assumes no sleep()s before return
	 * from this function or removal from hash chain by iuncache().
	 */
	iunlock(ip);
	ip->i_count = 0;

	/*
	 * If the underlying filesystem implementation doesn't want inodes to
	 * be cached, uncache here.
	 */
	if (fsinfo[ip->i_fstyp].fs_flags & FS_NOICACHE) {
		ASSERT((ip->i_flag & IVALID) == 0);
		iuncache(ip);
		return;
	}
	ASSERT(ip->i_mntdev);

	/*
	 * Caching: put at tail of freelist.  Leave on hash chain marked as
	 * valid.  Take care to throw erroneous inodes out of the cache.
	 */
	INS_FREE(ip, ifreepool.il_fback);
	ip->i_flag |= IVALID;
	if (ip->i_flag & IERROR) {
		iuncache(ip);
	}
}

/*
 * Dereference an inode which the current process has unlocked.  Another
 * process may have locked it.
 */
iunuse(ip)
	struct inode *ip;
{
	ilock(ip);
	iput(ip);
}

/*
 * Remove an inode from the cache.
 */
iuncache(ip)
	register struct inode *ip;
{
	ASSERT((ip->i_flag & ILOCKED) == 0);
	ASSERT(ip->i_count == 0);

	/*
	 * Remove from hash chain if cached.  If this inode is checked-out,
	 * then it is on its own private hash chain, and the REM_HASH merely
	 * clears its hash links.
	 */
	REM_HASH(ip);
	if (ip->i_flag & IVALID) {
		/*
		 * Remove from freelist and recycle ip's filesystem-dependent
		 * resources via FS_IPUT.
		 */
		ASSERT(ip->i_mntdev);
		REM_FREE(ip);
		ASSERT((fsinfo[ip->i_fstyp].fs_flags & FS_NOICACHE) == 0);
		FS_IPUT(ip);
	}
	INS_FREE(ip, &ifreepool);

	/*
	 * Invalidate inode so getinode doesn't try to FS_IPUT it again and
	 * so iflush can't find it.
	 */
	ip->i_flag = 0;
	ip->i_mntdev = NULL;
}

/*
 * Search inodes for those on mp.  Purge any cached inodes.
 * Returns -1 if an active inode (besides root) is found, otherwise 0.
 * Warning: This code assumes that it runs on a simplex machine.
 * That is, it assumes that the inode table will not change while
 * it is being scanned.  Let's hope that no drivers are changing 
 * the inode table at interrupt level!
 */
iflush(mp)
	register struct mount *mp;
{
	register struct inode *rootip, *ip;
	register int i;

	rootip = mp->m_mount;
	ASSERT(rootip);
	do {
		for (i = ninode, ip = inode; --i >= 0; ip++) {
			if (ip->i_mntdev != mp
			    || ip == rootip && ip->i_count == 1) {
				continue;
			}
			if (ip->i_count == 0
			    && (ip->i_flag & ILOCKED) == 0) {
				iuncache(ip);
				break;
			}
			return -1;
		}
	} while (i > 0);
	return 0;
}

/*
 * Common code for inode creation.
 */
struct inode *
icreate(ip, ftype, nlink, rdev)
	register struct inode *ip;
	ushort ftype;
	short nlink;
	dev_t rdev;
{
	ip->i_ftype = ftype;
	ip->i_nlink = nlink;
	ip->i_rdev = rdev;
	ip->i_flag |= IACC | IUPD | ICHG | IEXT | ISYN;
	ip->i_uid = u.u_uid;
	ip->i_gid = u.u_gid;
	ip->i_size = 0;
	FS_IUPDAT(ip, &time, &time);
	return (ip);
}

/*
 * Lock an inode.  If it's already locked, set the WANT bit and sleep.
 */
ilock(ip)
	register struct inode *ip;
{
	ASSERT(ip->i_count > 0);
	ILOCK(ip);
	ASSERT(ip->i_count > 0);
}

/*
 * Unlock an inode.  If WANT bit is on, wakeup.
 */
iunlock(ip)
	register struct inode *ip;
{
	ASSERT(ip->i_flag & ILOCKED);
	ASSERT(ip->i_count > 0);
	IUNLOCK(ip);
	ASSERT(ip->i_count > 0);
}

/*
 * Given unlocked inode ip and locked inode ip2, lock ip without deadlocking.
 *
 * NB: Users of this function must order their inode locking so that two
 * processes never try to ilock2() the same two inodes.  If process A had ip
 * locked and process B has ip2 locked, and if both called ilock2() to try to
 * lock one another's inodes, A and B might dynamically deadlock, i.e. starve
 * one another forever (depending upon scheduling vagaries, of course).
 *
 * An easy protocol consists of defining some ordering on ip and ip2 such as
 * (ip < ip2), having processes which want to lock ip before ip2 do so with
 * ilock(), and requiring that processes which want to lock ip2 before ip use
 * ilock2().  That is, processes A and B should use the following schema to
 * acquire two locked inodes:
 *	if (ip < ip2)
 *		lock ip, lock ip2;
 *	else
 *		lock ip2, ilock2(ip, ip2);
 */
ilock2(ip, ip2)
	register struct inode *ip, *ip2;
{
	ASSERT(ip != ip2);
	ASSERT(ip2->i_flag & ILOCKED);
	for (;;) {
		if ((ip->i_flag & ILOCKED) == 0) {
			ilock(ip);
			break;
		}
		iunlock(ip2); ilock(ip);
		if ((ip2->i_flag & ILOCKED) == 0) {
			ilock(ip2);
			break;
		}
		iunlock(ip); ilock(ip2);
	}
}
