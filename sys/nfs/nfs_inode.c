#ifndef lint
static	char	rcsid[] = "$Header: /d2/3.7/src/sys/nfs/RCS/nfs_inode.c,v 1.1 89/03/27 17:33:12 root Exp $";
#endif
/*
 * NFS inode implementation.
 *
 * $Source: /d2/3.7/src/sys/nfs/RCS/nfs_inode.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:33:12 $
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
# include "sys/conf.h"
# include "sys/file.h"
# include "sys/inode.h"
# include "sys/mount.h"
# include "sys/stat.h"
# include "sys/fstyp.h"
# include "sys/fs/com_pncc.h"
# include "sys/fs/nfs.h"
# include "sys/fs/nfs_clnt.h"
# include "sys/fs/nfs_export.h"
# include "sys/fs/rnode.h"
# include "rpc/xdr.h"
# include "sys/var.h"
# define ninode		(v.v_inode)
#else
# include "../h/param.h"
# include "../h/systm.h"
# include "../h/user.h"
# include "../h/conf.h"
# include "../h/file.h"
# include "../h/inode.h"
# include "../h/mount.h"
# include "../h/stat.h"
# include "../com/com_pncc.h"
# include "../nfs/nfs.h"
# include "../nfs/nfs_clnt.h"
# include "../nfs/nfs_export.h"
# include "../nfs/rnode.h"
# include "../rpc/xdr.h"
#endif

short		nfs_fstyp = 0;		/* NFS filesystem type */
struct rnode	**nfs_rtable = NULL;	/* rnode pointer table base */
short		nfs_rtablesize = 0;	/* and logical length  */

/* special nfs_rtable[i] value to reserve slot i for an ralloc-in-progress */
#define	RESERVED	((struct rnode *) 1)

#ifdef SVR3
extern sema_t	async_wait;
extern sema_t	chtablewait;
extern lock_t	async_lock;
#ifndef SIMPLEX
extern lock_t	clstat_lock;
extern sema_t	chtablesem;
#endif SIMPLEX
#endif SVR3

/*
 * A low watermark rnode number tells the index of the probable first
 * free rnode slot in nfs_rtable; rfreecnt is initialized in nfs_init to
 * the rnode table size and tracks the number of free slots therein.
 */
static short	rlowfree = 0;
static short	rfreecnt;
#ifdef SVR3
sema_t		rtablesem;
#ifndef SIMPLEX
sema_t		igetbyfhsem;
#endif
#endif

/*
 * Rnode hashing keys are file handles.  XXX fh_fno on client side!!!
 */
#define	RHASHBITS	4
#define	RHASHSIZE	(1<<RHASHBITS)
#define	RHASHMASK	(RHASHSIZE-1)
#define	RBUCKET(fhp)	(&rhash[(fhp)->fh_fno & RHASHMASK])

struct rbucket {
	struct rnode	*r_forw;	/* hash chain linkage */
	struct rnode	*r_back;
};

static struct rbucket	rhash[RHASHSIZE];

#define	INSHASH(rp, rbp) { \
	(rp)->r_forw = (rbp)->r_forw; \
	(rp)->r_back = (struct rnode *) (rbp); \
	(rp)->r_forw->r_back = (rp)->r_back->r_forw = (rp); \
}
#define	REMHASH(rp) { \
	(rp)->r_forw->r_back = (rp)->r_back; \
	(rp)->r_back->r_forw = (rp)->r_forw; \
}
#define	NULLHASH(rp) \
	((rp)->r_forw = (rp)->r_back = (rp))

/*
 * Initialize NFS client state.
 */
nfs_init()
{
	register short i;
	register struct rbucket *rbp;

	/*
	 * Determine NFS's filesystem type.
	 */
	nfs_fstyp = findfstyp(nfs_init);
	ASSERT(nfs_fstyp > 0);

	/*
	 * Allocate the rnode table.  Don't let it be too big.
	 */
	i = nfs_rtablesize;
	if (i == 0 || i > NFS_MAXRTABLESIZE) {
		i = NFS_MAXRTABLESIZE;
	}
	if (i >= ninode) {
		i = ninode - 1;	/* or ninode for diskless */
	}
	rfreecnt = i;
	nfs_rtablesize = i;
	i *= sizeof(struct rnode *);
	nfs_rtable = (struct rnode **) kmem_alloc((u_int) i);
	bzero((caddr_t) nfs_rtable, (u_int) i);

	/*
	 * Initialize the rnode hash table.
	 */
	rbp = rhash;
	for (i = RHASHSIZE; i > 0; --i) {
		NULLHASH((struct rnode *) rbp);
		rbp++;
	}

#ifdef SVR3
	/* Initialize locks and semaphores */
	initnsema(&async_wait, 0, "async_wt");
	initnsema(&chtablewait, 0, "chtablew");
	initlock(&async_lock);
#ifndef SIMPLEX
	initlock(&clstat_lock);
	initnmutex(&chtablesem, 1, "chtables");
	initnmutex(&igetbyfhsem, 1, "igetbyfh");
	initnmutex(&rtablesem, 1, "rtable");
#endif SIMPLEX
#endif SVR3
}

/*
 * Given a pathname component cache entry, return the cached inode named
 * by the component.  If the cached component is stale, remove it and
 * return NULL in *ipp.  Otherwise see whether the cached attributes need
 * to be refreshed by calling nfs_getattr().
 */
int
nfs_igetbyname(dp, nce, cred, ipp)
	register struct inode *dp;
	register struct ncentry *nce;
	struct ucred *cred;
	register struct inode **ipp;
{
	register struct rnode *rp;
	register int error;

	ASSERT(vtor(dp));
	rp = nfs_rtable[nce->nce_inum];
	if (rp == NULL || rp == RESERVED
	    || rp->r_ncap != nce->nce_cap
	    || vtor(dp)->r_ncap != nce->nce_pcap) {
		/*
		 * A stale hit; turn it into a miss.
		 */
		pncc_remove(nce);
		*ipp = NULL;
		return 0;
	}
	ASSERT(rp->r_number == nce->nce_inum);
	*ipp = iget(dp->i_mntdev, nce->nce_inum);
	if (*ipp == NULL) {
		error = u.u_error;
		ASSERT(error);
	} else {
		error = nfs_getattr(*ipp, cred);
		if (error) {
			iput(*ipp);
			*ipp = NULL;
		}
	}
	return error;
}

/*
 * Return an inode for the given file handle, caching its attributes.
 * If no rnode exists for fhp then create one and install it in the
 * rnode hash table.
 */
int
nfs_igetbyfh(mp, fhp, attrp, ipp)
	struct mount *mp;
	register fhandle_t *fhp;
	struct nfsfattr *attrp;
	struct inode **ipp;
{
	register struct rnode *rp;
	register struct rbucket *rbp;
	register int newnode, error;
	struct rnode *ralloc();

	rbp = RBUCKET(fhp);
#ifdef SVR3
	appsema(&igetbyfhsem, PZERO); /* one at a time for now */
	appsema(&rtablesem, PZERO);
#endif
	for (rp = rbp->r_forw; rp != (struct rnode *) rbp;
	     rp = rp->r_forw) {
		ASSERT(rtov(rp));
		if (rtov(rp)->i_mntdev == mp
		    && !bcmp((caddr_t)rtofh(rp), (caddr_t)fhp, sizeof *fhp))
			break;
	}
	if (rp != (struct rnode *) rbp) {
		newnode = 0;	/* we hit the rnode cache */
	} else {
		rp = ralloc(vftomi(mp));
		if (rp == NULL) {	/* bail out with ENFILE */
			*ipp = NULL;
			return ENFILE;
		}
		INSHASH(rp, rbp);
		rp->r_fh = *fhp;
		rp->r_nfsattr = *attrp;
		newnode = 1;
	}
#ifdef SVR3
	/*
	 * Unfortunately, we have to give up rtablesem here in case
	 * iget calls nfs_iput(), which will call runcache().
	 */
	apvsema(&rtablesem);
#endif
	*ipp = iget(mp, rtorn(rp));
	if (*ipp == NULL) {
		error = u.u_error;
		ASSERT(error);
#ifdef SVR3
		appsema(&rtablesem, PZERO);
#endif
		rfree(rp, vftomi(mp));
#ifdef SVR3
		apvsema(&rtablesem);
#endif
	} else {
		error = 0;
		if (newnode) {		/* inode cache may leave stale size */
			(*ipp)->i_size = 0;
		}
		nfs_attrcache(*ipp, attrp, newnode ? NOFLUSH : SFLUSH);
	}
#ifdef SVR3
	apvsema(&igetbyfhsem);
#endif
	return error;
}

/*
 * Allocate an rnode, not on any hash chain.  If there are no free slots,
 * scan the table for a slot whose inode is cached with zero references.
 * Uncache the inode, freeing its rnode and rnode slot.  In either case
 * allocate a new rnode and stuff its address into the found slot.
 */
static struct rnode *
ralloc(mi)
	struct mntinfo *mi;
{
	register int i;
	register struct rnode **rpp;
	register struct rnode *rp;

#ifdef SVR3
	ASSERT(valusema(&rtablesem) <= 0);
#endif
	ASSERT(rfreecnt >= 0);
	if (rfreecnt > 0) {
		i = nfs_rtablesize - rlowfree;
		for (rpp = nfs_rtable + rlowfree; --i >= 0; rpp++) {
			if (*rpp == NULL)
				break;
		}
		ASSERT(i >= 0);
	} else {
		i = nfs_rtablesize;
		for (rpp = nfs_rtable; ; rpp++) {
			register struct inode *ip;

			if (--i < 0) {
				return NULL;
			}
			rp = *rpp;
			ASSERT(rp);
			if (rp == RESERVED) {
				continue;
			}
			ip = rtov(rp);
			ASSERT(ip && ip->i_number == rp->r_number);
			if (ip->i_count == 0
#ifdef SVR3
			    && cpsema(&ip->i_lock)) {
				ASSERT(ip->i_fstyp == nfs_fstyp);
				ASSERT(ip->i_forw && ip->i_back);
#else
			    && (ip->i_flag & ILOCKED) == 0) {
				ASSERT(ip->i_flag & IVALID);
#endif
#ifdef SVR3
				/*
				 * Unfortunately, have to give up rtablesem
				 * here so runcache()  can get it.
				 */
				apvsema(&rtablesem);
#endif
				iuncache(ip);
#ifdef SVR3
				appsema(&rtablesem, 0);
				prele(ip);
#endif
				ASSERT(rfreecnt == 1 && *rpp == NULL);
				break;
			}
		}
	}

	/* reserve the nfs_rtable slot in case kmem_alloc sleeps */
	*rpp = RESERVED;
	--rfreecnt;
	ASSERT(rfreecnt >= 0);

	/* create and initialize the rnode */
	rp = (struct rnode *)kmem_alloc(sizeof *rp);
	bzero((caddr_t) rp, sizeof *rp);
	NULLHASH(rp);
	rp->r_number = rpp - nfs_rtable;
	rp->r_ncap = pncc_newcap();

	/* install it in nfs_rtable */
	*rpp = rp;
	rlowfree = rp->r_number + 1;

	mi->mi_refct++;
	return rp;
}

static
rfree(rp, mi)
	register struct rnode *rp;
	struct mntinfo *mi;
{
	REMHASH(rp);	/* remove from cache before sleeping */

	/* free the rnode's slot, adjusting rlowfree */
	nfs_rtable[rp->r_number] = NULL;
	rfreecnt++;
	ASSERT(rfreecnt <= nfs_rtablesize);
	if (rp->r_number < rlowfree) {
		rlowfree = rp->r_number;
	}

	/* now free the rnode */
	kmem_free((caddr_t) rp, sizeof *rp);
	--mi->mi_refct;
}

/*
 * Invalidate ip's resources and remove its rnode from the rnode cache.
 * This primitive is called by nfs_iput when recycling an nfs inode and by
 * nfs_umount when disposing of the root inode (just before freeing vtomi(ip),
 * the mount structure's private data).
 */
runcache(ip, mi)
	register struct inode *ip;
	struct mntinfo *mi;
{
#ifdef SVR3
	appsema(&rtablesem, PZERO);
	ASSERT(valusema(&ip->i_lock) <= 0);
#endif
	rinvalfree(ip);	/* kill ip's buffers */
	rfree(vtor(ip), mi);
	ip->i_fsptr = NULL;
#ifdef SVR3
	apvsema(&rtablesem);
#endif
}

#ifdef NOTDEF
/*
 * Lock and unlock rnodes
 */
void
rlock(rp)
	register struct rnode *rp;
{
        while (rp->r_flags & RLOCKED) {
                rp->r_flags |= RWANT;
                sleep((caddr_t)rp, PINOD);
        }
        rp->r_flags |= RLOCKED;
}

void
runlock(rp)
	register struct rnode *rp;
{
        rp->r_flags &= ~RLOCKED;
        if (rp->r_flags & RWANT) {
                rp->r_flags &= ~RWANT;
                wakeup((caddr_t)rp);
        }
}
#endif

/*
 * NFS filesystem switch inode operations.
 */

/*
 * Release an rnode after its final use, and invalidate its resources
 * when it is about to be reused.  In the first case, handle the
 * open-unlink rename hack.  In the second, free the rnode table slot
 * so it can be reclaimed by ralloc, and invalidate any buffers which
 * are associated by pseudo-device with this rnode.
 */
nfs_iput(ip)
	register struct inode *ip;
{
	register struct rnode *rp;

#ifdef SVR3
	ASSERT(valusema(&ip->i_lock) <= 0);
#endif
	rp = vtor(ip);
	ASSERT(rp != NULL);
	/*
	 * The rnode must have been cleaned already by nfs_closei.
	 */
	ASSERT(!rp->r_open);
	ASSERT((rp->r_flags & RDIRTY) == 0);
	if (ip->i_count == 0) {
		runcache(ip, vtomi(ip));
		return;
	}
	ASSERT(ip->i_count == 1);

	/*
	 * Forget any asynchronous i/o errors which till now may have stopped
	 * further i/o.  Reset readahead detection on last close.
	 */
	rp->r_error = 0;
	rp->r_lastr = 0;

	/*
	 * Normal filesystems would check here for an unlinked, open
	 * file and remove it.  Since servers don't have state, NFS
	 * simulates open unlinks by renaming the file on the server.
	 * Thus we check here for such a rename and delete.
	 */
	if (rp->r_unlname != NULL) {
		struct nfsdiropargs da;
		auto enum nfsstat status;
		register int error;

		setdiropargs(&da, rp->r_unlname, rp->r_unldip);
		error =
		    rfscall(vtomi(rp->r_unldip), RFS_REMOVE, xdr_diropargs,
			(caddr_t) &da, xdr_enum, (caddr_t) &status,
			&rp->r_unlcred);
		if (error == 0) {
			error = geterrno(status);
		}
		iunuse(rp->r_unldip);
		rp->r_unlcred.cr_ref = 0;
		kmem_free((caddr_t)rp->r_unlname, NFS_MAXNAMLEN);
		rp->r_unlname = NULL;
	}
}

/*
 * Fill in st with status information about ip's underlying rnode.
 * The stat system call fills in everything but the mode and times, and
 * then calls FS_STATF.  For nfs we must fill in everything but the mode
 * and times from the rnode, because some of the inode's members may be
 * stale while others are not even maintained.
 */
nfs_statf(ip, st)
	register struct inode *ip;
	register struct stat *st;
{
	register int error;
	struct ucred cred;
	struct nfsfattr *attrp;

#ifdef SVR3
	ASSERT(valusema(&ip->i_lock) <= 0);
#endif
#ifdef	NFSDEBUG
	dprint(nfsdebug, 4, "nfs_statf %s %x %d\n",
	    vtomi(ip)->mi_hostname, vtor(ip)->r_nfsattr.na_fsid,
	    vtor(ip)->r_nfsattr.na_nodeid);
#endif
	crinit(&u, &cred);
	error = nfs_getattr(ip, &cred);
	if (error) {
		u.u_error = error;
		return;
	}
	attrp = &vtor(ip)->r_nfsattr;

	/*
	 * Use the inode's mount device name for this file's device.
	 * Use its node number on the server to uniquely identify it.
	 */
	st->st_dev = ip->i_mntdev->m_dev;
	st->st_ino = attrp->na_nodeid;

	st->st_mode = attrp->na_mode;
	st->st_nlink = attrp->na_nlink;
	st->st_uid = attrp->na_uid;
	st->st_gid = attrp->na_gid;
	st->st_rdev = attrp->na_rdev;
	st->st_size = ip->i_size;
	st->st_atime = attrp->na_atime.tv_sec;
	st->st_mtime = attrp->na_mtime.tv_sec;
	st->st_ctime = attrp->na_ctime.tv_sec;

	/*
	 * note that IFMT internal codes match S_IFMT external codes
	 */
	st->st_mode |= ntype_to_itype(attrp->na_type);
#ifdef	NFSDEBUG
	dprint(nfsdebug, 5, "nfs_statf: st_mode %o, st_size %d\n",
	    st->st_mode, st->st_size);
#endif
}

/*
 * Construct an NFS node.  The inode pointed at by ip has a private
 * rnode allocated by nfs_igetbyfh() prior to the call therein to iget(),
 * which in turn calls this function through the filesystem switch.
 * In this way we find the allocated rnode and attach it to ip.
 */
struct inode *
nfs_iread(ip)
	register struct inode *ip;
{
	register struct rnode *rp;

#ifdef SVR3
	ASSERT(valusema(&ip->i_lock) <= 0);
#endif
	ASSERT(ip->i_number < nfs_rtablesize);
	ASSERT(vtor(ip) == NULL);
	rp = rntor(ip->i_number);
	if (rp == NULL || rtov(rp) != NULL) {
		/*
		 * Someone called iget() with an inumber for an rnode that
		 * went away or got recycled.  What can we do without a file
		 * handle?
		 */
#ifdef NFSDEBUG
		dprint(nfsdebug, 9, "nfs_iread missed! %s %x %d\n",
		    vtomi(ip)->mi_hostname, rtofh(rp)->fh_fsid,
		    rtofh(rp)->fh_fno);
#endif
		u.u_error = ESTALE;
		return NULL;
	}
#ifdef NFSDEBUG
	dprint(nfsdebug, 4, "nfs_iread %s %x %d\n",
	    vtomi(ip)->mi_hostname, rtofh(rp)->fh_fsid,
	    rtofh(rp)->fh_fno);
#endif
	/*
	 * Unlike vanilla 5.3, we always call FS_IPUT when recycling an
	 * inode, whether or not its fstyp is changing.
	 */
	ip->i_fsptr = (fsptr_t) rp;
	rtov(rp) = ip;
	ip->i_dev = makedev(nfs_major, ip->i_number);
	ip->i_ftype = ntype_to_itype(rp->r_nfsattr.na_type);
	rattr_to_iattr(ip);

	return ip;
}

/*
 * Update an inode from its rnode's attributes.
 */
nfs_iupdat(ip, ta, tm)
	register struct inode *ip;
	time_t *ta, *tm;
{
#ifdef SVR3
	ASSERT(valusema(&ip->i_lock) <= 0);
#endif
#ifdef NFSDEBUG
	dprint(nfsdebug, 4, "nfs_iupdat %s %x %d\n",
	    vtomi(ip)->mi_hostname, vtor(ip)->r_nfsattr.na_fsid,
	    vtor(ip)->r_nfsattr.na_nodeid);
#endif
	/*
	 * Don't change the times on the server unless we were called by utime.
	 * How can we tell?  By checking whether (ta == &time && tm == &time).
	 * Pretty gross.
	 */
	if (ta != &time || tm != &time) {
		register int error;
		struct ucred cred;
		struct nfssattr sa;

		crinit(&u, &cred);
		sattr_null(&sa);
		sa.sa_atime.tv_sec = *ta;
		sa.sa_atime.tv_usec = 0;
		sa.sa_mtime.tv_sec = *tm;
		sa.sa_mtime.tv_usec = 0;
		/*
		 * nfs_setattr calls nfs_attrcache, which updates ip
		 */
		error = nfs_chattr(ip, &sa, &cred);
		if (error) {
			u.u_error = error;
		}
	}
	ip->i_flag &= ~(IACC|IUPD|ICHG|ISYN);
}

/*
 * Truncate an nfs inode to zero size.
 */
nfs_itrunc(ip)
	register struct inode *ip;
{
#ifdef SVR3
	ASSERT(valusema(&ip->i_lock) <= 0);
#endif
	nfs_setsize(ip, (off_t) 0);
}

/*
 * Truncate the inode by setting its rnode's size attribute to newsize.
 */
nfs_setsize(ip, newsize)
	register struct inode *ip;
	off_t newsize;
{
	register int error;
	struct ucred cred;
	struct nfssattr sa;

#ifdef NFSDEBUG
	dprint(nfsdebug, 4, "nfs_itrunc %s %x %d\n",
	    vtomi(ip)->mi_hostname, vtor(ip)->r_nfsattr.na_fsid,
	    vtor(ip)->r_nfsattr.na_nodeid);
#endif
	if (ip->i_ftype != IFREG && ip->i_ftype != IFLNK) {
		u.u_error = EINVAL;
		return;
	}
#ifdef SVR3
	if (vtor(ip)->r_flags & RMAPPED)
		nfs_freemap(ip);
#endif
	crinit(&u, &cred);
	sattr_null(&sa);
	sa.sa_size = newsize;
	error = nfs_chattr(ip, &sa, &cred);
	if (error) {
		u.u_error = error;
	}
}
