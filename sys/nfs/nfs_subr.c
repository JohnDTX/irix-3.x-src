/*	@(#)nfs_subr.c 1.1 85/05/30 SMI	*/

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
# include "sys/buf.h"
# include "sys/fcntl.h"
# include "sys/conf.h"
# include "sys/fstyp.h"
# include "sys/inode.h"
# include "sys/mount.h"
# include "sys/nami.h"
# include "sys/immu.h"
# include "sys/region.h"
# include "sys/sbd.h"
# include "sys/proc.h"
# include "sys/stat.h"
# include "sys/var.h"
# include "netinet/in.h"
# include "sys/fs/nfs.h"
# include "sys/fs/nfs_clnt.h"
# include "sys/fs/nfs_export.h"
# include "sys/fs/nfs_stat.h"
# include "sys/fs/com_pncc.h"
# include "sys/fs/rnode.h"
# include "rpc/types.h"
# include "rpc/xdr.h"
# include "rpc/auth.h"
# include "rpc/clnt.h"
#else
# include "../h/param.h"
# include "../h/systm.h"
# include "../h/user.h"
# include "../h/buf.h"
# include "../h/fcntl.h"
# include "../h/fs.h"
# include "../h/fstyp.h"
# include "../h/inode.h"
# include "../h/mount.h"
# include "../h/nami.h"
# include "../h/proc.h"
# include "../h/stat.h"
# include "../netinet/in.h"
# include "../nfs/nfs.h"
# include "../nfs/nfs_clnt.h"
# include "../nfs/nfs_export.h"
# include "../nfs/nfs_stat.h"
# include "../nfs/rnode.h"
# include "../rpc/types.h"
# include "../rpc/xdr.h"
# include "../rpc/auth.h"
# include "../rpc/clnt.h"
#endif

#ifdef NFSDEBUG
#ifdef SVR3
extern int	nfsdebug;
#else
int nfsdebug = 0;
#endif
#endif

#if !defined(SVR3) || defined(SIMPLEX)

#define	SETMUSTRUN(was_running)
#define	RESTOREMUSTRUN(was_running)
#define	CLSTAT_LOCK()
#define	CLSTAT_UNLOCK()

#else

#define	SETMUSTRUN(was_running)		(was_running = setmustrun(0))
#define	RESTOREMUSTRUN(was_running)	restoremustrun(was_running)
lock_t	clstat_lock;

#define	CLSTAT_LOCK()			spsemahi(clstat_lock)
#define	CLSTAT_UNLOCK()			svsemax(clstat_lock)

#endif

/*
 * Client side utilities
 */

/*
 * client side statistics
 */
struct clstat clstat;

#define MAXCLIENTS	4
struct chtab {
	int	ch_timesused;
	bool_t	ch_inuse;
	bool_t	ch_hard;
	CLIENT	*ch_client;
} chtable[MAXCLIENTS];

#ifdef SVR3
sema_t	chtablewait;
#ifndef SIMPLEX
sema_t	chtablesem;
#endif SIMPLEX
#else !SVR3
int	clwanted = 0;
#endif SVR3

static CLIENT *
clget(mi, cred)
	register struct mntinfo *mi;
	struct ucred *cred;
{
	register struct chtab *ch;

	for (;;) {
		register bool_t allhard = TRUE;

#if defined(SVR3) && !defined (SIMPLEX)
		psema(&chtablesem, PZERO);
#endif
		clstat.nclgets++;	/* protected by psema above */
		for (ch = chtable; ch < &chtable[MAXCLIENTS]; ch++) {
			if (ch->ch_inuse) {
				if (!ch->ch_hard) {
					allhard = FALSE;
				}
			} else {
				if (mi->mi_hard && allhard
				    && ch == &chtable[MAXCLIENTS-1]) {
					/*
					 * Don't let hard RPCs tie up chtable.
					 */
					break;
				}
				ch->ch_inuse = TRUE;	/* 3.0 fix */
				ch->ch_hard = mi->mi_hard;
#if defined(SVR3) && !defined(SIMPLEX)
				vsema(&chtablesem);
#endif
				if (ch->ch_client == NULL) {
					ch->ch_client =
					    clntkudp_create(&mi->mi_addr,
					    NFS_PROGRAM, NFS_VERSION,
					    mi->mi_retrans, cred);
				} else {
					clntkudp_init(ch->ch_client,
					    &mi->mi_addr, mi->mi_retrans, cred);
				}
				if (ch->ch_client == NULL) {
					panic("clget: null client");
				}
				ch->ch_timesused++;
				return (ch->ch_client);
			}
		}
		/*
		 * If we got here there are no available handles
		 */
		clstat.nclsleeps++; /* protected by chtablesem */
#ifdef SVR3
#ifdef SIMPLEX
		psema(&chtablewait, PRIBIO);
#else !SIMPLEX
		vpsema(&chtablesem, &chtablewait, PRIBIO);
#endif SIMPLEX
#else !SVR3
		clwanted++;
		sleep((caddr_t)chtable, PRIBIO);
#endif SVR3
	}
}

static
clfree(cl)
	register CLIENT *cl;
{
	register struct chtab *ch;

#if defined(SVR3) && !defined(SIMPLEX)
	psema(&chtablesem, PZERO);
#endif
	for (ch = chtable; ch < &chtable[MAXCLIENTS]; ch++) {
		if (ch->ch_client == cl) {
			ch->ch_inuse = FALSE;
			break;
		}
	}
#ifdef SVR3
#ifndef SIMPLEX
	vsema(&chtablesem);
#endif SIMPLEX
	cvsema(&chtablewait);
#else !SVR3
	if (clwanted) {
		clwanted = 0;
		wakeup((caddr_t)chtable);
	}
#endif SVR3
}

char *rpcstatnames[] = {
	"SUCCESS", "CANT ENCODE ARGS", "CANT DECODE RES", "CANT SEND",
	"CANT RECV", "TIMED OUT", "VERS MISMATCH", "AUTH ERROR",
	"PROG UNAVAIL", "PROG VERS MISMATCH", "PROC UNAVAIL",
	"CANT DECODE ARGS", "UNKNOWN HOST", "UNKNOWN", "PMAP FAILURE",
	"PROG NOT REGISTERED", "SYSTEM ERROR", "FAILED",
	"INTERRUPTED"
#define	RPC_INTR	18
};
char *rfsnames[] = {
	"null", "getattr", "setattr", "unused", "lookup", "readlink", "read",
	"unused", "write", "create", "remove", "rename", "link", "symlink",
	"mkdir", "rmdir", "readdir", "fsstat"
};

/*
 * Back off for retransmission timeout, MAXTIMO is in 10ths of a sec
 */
#define MAXTIMO	300
#define backoff(tim)	((((tim) << 2) > MAXTIMO) ? MAXTIMO : ((tim) << 2))

int
rfscall(mi, which, xdrargs, argsp, xdrres, resp, cred)
	struct mntinfo *mi;
	int	 which;
	xdrproc_t xdrargs;
	caddr_t	argsp;
	xdrproc_t xdrres;
	caddr_t	resp;
	struct ucred *cred;
{
	CLIENT *client;
	register enum clnt_stat status;
	struct rpc_err rpcerr;
	struct timeval wait;
	struct ucred *newcred;
	int timeo;
	bool_t tryagain;

#ifdef NFSDEBUG
	dprint(nfsdebug, 6, "rfscall: %x, %d, %x, %x, %x, %x\n",
	    mi, which, xdrargs, argsp, xdrres, resp);
#endif
	rpcerr.re_errno = 0;
	CLSTAT_LOCK();
	clstat.ncalls++;
	clstat.reqs[which]++;
	CLSTAT_UNLOCK();
	newcred = NULL;
	timeo = mi->mi_timeo;
retry:
	client = clget(mi, cred);

	/*
	 * If hard mounted fs, retry call forever unless hard error occurs
	 */
	do {
		wait.tv_sec = timeo / 10;
		wait.tv_usec = 100000 * (timeo % 10);
		status = CLNT_CALL(client, which, xdrargs, argsp,
		    xdrres, resp, wait);
		switch (status) {
		case RPC_SUCCESS:
			tryagain = FALSE;
			break;

		/*
		 * Unrecoverable errors: give up immediately
		 */
		case RPC_AUTHERROR:
		case RPC_CANTENCODEARGS:
		case RPC_CANTDECODERES:
		case RPC_VERSMISMATCH:
		case RPC_PROGVERSMISMATCH:
		case RPC_CANTDECODEARGS:
			tryagain = FALSE;
			break;

		default:
			if (mi->mi_hard && isfatalsig(u.u_procp)) {
				status = (enum clnt_stat) RPC_INTR;
				rpcerr.re_status = RPC_SYSTEMERROR;
				rpcerr.re_errno = EINTR;
				tryagain = FALSE;
				break;
			}
			if (!mi->mi_printed) {
				mi->mi_printed = 1;
				printf("NFS server %s not responding",
				    mi->mi_hostname);
				if (mi->mi_hard) {
					printf(", still trying\n");
				} else {
					printf(", giving up\n");
				}
			}
			timeo = backoff(timeo);
			tryagain = TRUE;
		}
	} while (tryagain && mi->mi_hard);

	if (status != RPC_SUCCESS) {
		CLNT_GETERR(client, &rpcerr);
		CLSTAT_LOCK();
		clstat.nbadcalls++;
		CLSTAT_UNLOCK();
		printf("NFS %s failed for server %s: %s\n", rfsnames[which],
		    mi->mi_hostname, rpcstatnames[(int)status]);
	} else if (resp && *(int *)resp == EACCES &&
	    newcred == NULL && cred->cr_uid == 0 && cred->cr_ruid != 0) {
		/*
		 * Boy is this a kludge!  If the reply status is EACCES
		 * it may be because we are root (no root net access).
		 * Check the real uid, if it isn't root make that
		 * the uid instead and retry the call.
		 */
		newcred = crdup(cred);
		cred = newcred;
		cred->cr_uid = cred->cr_ruid;
		clfree(client);
		goto retry;
	} else if (mi->mi_printed && mi->mi_hard) {
		printf("NFS server %s ok\n", mi->mi_hostname);
	}
	mi->mi_printed = 0;

	clfree(client);
#ifdef NFSDEBUG
	dprint(nfsdebug, 7, "rfscall: returning %d\n", rpcerr.re_errno);
#endif
	if (newcred) {
		crfree(newcred);
	}
	return (rpcerr.re_errno);
}

/*
 * Timeout constraints chosen based upon how the last interval between change
 * and reference compared with a large threshold.  The idea is to minimize the
 * Poisson probability of choosing too long an attribute timeout, i.e. holding
 * stale attributes after the served file has been changed.
 */
#ifdef SVR3
extern int	nfsac_mintimeo;
extern int	nfsac_maxtimeo;
extern int	nfsac_threshold;
#else
int	nfsac_mintimeo = 3;
int	nfsac_maxtimeo = 30;
int	nfsac_threshold = 3000;
#endif

nfs_attrcache(vp, na, fflag)
	register struct inode *vp;
	register struct nfsfattr *na;
	enum staleflush fflag;
{
	register struct rnode *rp;
	register long now, prev, delta;

	rp = vtor(vp);
	/*
	 * check the new modify time against the old modify time
	 * to see if cached data is stale
	 */
	if (na->na_mtime.tv_sec != rp->r_nfsattr.na_mtime.tv_sec ||
	    na->na_mtime.tv_usec != rp->r_nfsattr.na_mtime.tv_usec) {
		/*
		 * The file has changed.
		 * If this was unexpected (fflag == SFLUSH),
		 * flush the delayed write blocks associated with this inode
		 * from the buffer cache and mark the cached blocks on the
		 * free list as invalid.
		 * If this is a text mark it invalid so that the next pagein
		 * from the file will fail.
		 * If the vnode is a directory, purge the directory name
		 * lookup cache.
		 */
		if (fflag == SFLUSH) {
			rinvalfree(vp);
		}
#ifdef SVR3
		if (vtor(vp)->r_flags & RMAPPED)
			if (vp->i_flag & ITEXT)
				flushpgch(vp);
			else
				nfs_freemap(vp);
#endif
		if (vp->i_flag & IXSAVED) {
			(void) xflush(vp);
		}
		if (vp->i_ftype == IFDIR) {
			rnewcap(vp);
		}
	}

	/*
	 * Approximate current ``file'' time on the server with
	 * MAX(attributed atime, attributed mtime)
	 */
	now = na->na_atime.tv_sec;
	if (now < na->na_mtime.tv_sec) {
		now = na->na_mtime.tv_sec;
	}
	/*
	 * Estimate the file's previous modification time with
	 * MIN(previous attributed mtime, attributed mtime)
	 */
	prev = rp->r_nfsattr.na_mtime.tv_sec;
	if (prev == 0 || prev > na->na_mtime.tv_sec) {
		prev = na->na_mtime.tv_sec;
	}
	/*
	 * Use (current time - previous mod time) as the estimated
	 * timeout, then threshold appropriately.
	 */
	delta = now - prev;
	if (delta < nfsac_threshold) {
		delta = nfsac_mintimeo;
	} else {
		delta = nfsac_maxtimeo;
	}

	rp->r_nfsattrtime = time + delta;
	rp->r_nfsattr = *na;
	rattr_to_iattr(vp);
}

/*
 * Externalize some of ip's rnode state.  We must decide whether to believe
 * ip->i_size or the rnode's attributed size.  
 *
 * If the rnode is closed, either we found it in the name cache or created it
 * based on an RFS_LOOKUP.  In the name cache hit case, the rnode's attributed
 * size has been refreshed by calling nfs_getattr().  In the RFS_LOOKUP case,
 * the rnode is new and has a zero ip->i_size.  Therefore in both cases we set
 * ip->i_size to the attributed size.
 *
 * If the rnode is open, then we may be writing it, so we set ip->i_size and
 * the attributed size to whichever is greater.  Sun relies on the fact that
 * non-directory rnodes are not cached to keep things more or less consistent:
 * after final close the rnode goes away, and upon next reference a new one is
 * created with fresh attributes.
 */
rattr_to_iattr(ip)
	register struct inode *ip;
{
	register struct rnode *rp;
	register struct nfsfattr *na;

	rp = vtor(ip);
	na = &rp->r_nfsattr;
	if (!rp->r_open || ip->i_size < na->na_size) {
		ip->i_size = na->na_size;
	} else {
		na->na_size = ip->i_size;
	}
	if ((short) na->na_nlink <= 0) {
		/*
		 * This node seems to be unlinked, yet the server did not
		 * return "stale file handle."  We must fix the link count so
		 * that generic AT&T kernel code (e.g. namei()) won't fail
		 * unrecoverably.
		 */
		na->na_nlink = 1;
		nfsattr_inval(ip);
	}
	ip->i_nlink = na->na_nlink;
	ip->i_uid = na->na_uid;
	ip->i_gid = na->na_gid;
	ip->i_rdev = na->na_rdev;
}

sattr_null(sa)
	register struct nfssattr *sa;
{

	sa->sa_mode = SATTR_NULL;
	sa->sa_uid = SATTR_NULL;
	sa->sa_gid = SATTR_NULL;
	sa->sa_size = SATTR_NULL;
	sa->sa_atime.tv_sec = SATTR_NULL;
	sa->sa_atime.tv_usec = SATTR_NULL;
	sa->sa_mtime.tv_sec = SATTR_NULL;
	sa->sa_mtime.tv_usec = SATTR_NULL;
}

setdiropargs(da, nm, dvp)
	struct nfsdiropargs *da;
	char *nm;
	struct inode *dvp;
{

	da->da_fhandle = *vtofh(dvp);
	da->da_name = nm;
}

#define	PREFIXLEN	4
static char prefix[PREFIXLEN+1] = ".nfs";

/*ARGSUSED*/
char *
newname(s)
	char *s;
{
	char *news;
	register char *s1, *s2;
	int id;

	id = lbolt;
	news = (char *)kmem_alloc((u_int)NFS_MAXNAMLEN);
	for (s1 = news, s2 = prefix; s2 < &prefix[PREFIXLEN]; ) {
		*s1++ = *s2++;
	}
	while (id) {
		*s1++ = "0123456789ABCDEF"[id & 0x0f];
		id = id >> 4;
	}
	*s1 = '\0';
	return (news);
}

/*
 * Server side utilities
 */

/*
 * Get attributes from the inode into nap.  Use FS_STATF to get the
 * inode mode and times, which are considered private under the 5.3
 * filesystem switch.
 */
int
vop_getattr(ip, nap)
	register struct inode *ip;
	register struct nfsfattr *nap;
{
	struct stat stb;
	enum nfsftype itype_to_ntype();

#ifdef NFSDEBUG
	dprint(nfsdebug, 6, "vop_getattr: ip 0x%x nap 0x%x\n", ip, nap);
#endif
	if (ip->i_flag & (IACC|IUPD|ICHG)) {
		FS_IUPDAT(ip, &time, &time);
		if (u.u_error) {
			return u.u_error;
		}
	}
 
	/*
	 * Copy attributes from inode into *nap.
	 */
	nap->na_type = itype_to_ntype(ip->i_ftype);
	nap->na_nlink = ip->i_nlink;
	nap->na_uid = ip->i_uid;
	nap->na_gid = ip->i_gid;
	nap->na_size = ip->i_size;
	switch (ip->i_ftype) {
	  case IFBLK:
	  case IFCHR:
		nap->na_blocksize = BLKDEV_IOSIZE;
		break;
	  default:
		nap->na_blocksize = ip->i_mntdev->m_bsize;
		break;
	}
	nap->na_rdev = ip->i_rdev;
	nap->na_blocks = (ip->i_size / ip->i_mntdev->m_bsize) + 1;
	nap->na_fsid = ip->i_dev;
	nap->na_nodeid = ip->i_number;
 
	/*
	 * Call FS_STATF to get private inode data (mode and times).
	 */
	FS_STATF(ip, &stb);
	if (u.u_error) {
		return u.u_error;
	}
	nap->na_mode = ip->i_ftype | (stb.st_mode & MODEMSK);
	nap->na_atime.tv_sec = stb.st_atime;
	nap->na_atime.tv_usec = 0;
	nap->na_mtime.tv_sec = stb.st_mtime;
	nap->na_mtime.tv_usec = 0;
	nap->na_ctime.tv_sec = stb.st_ctime;
	nap->na_ctime.tv_usec = 0;
 
#ifdef NFSDEBUG
	dprint(nfsdebug, 10, "\
vop_getattr: type 0x%x mode %o fsid 0x%x nodeid %d\n\
	     nlink %d uid %d gid %d size %d\n\
	     rdev 0x%x blocks %d blocksize %d\n\
vop_getattr: returning\n",
		nap->na_type,
		nap->na_mode,
		nap->na_fsid,
		nap->na_nodeid,
		nap->na_nlink,
		nap->na_uid,
		nap->na_gid,
		nap->na_size,
		nap->na_rdev,
		nap->na_blocks,
		nap->na_blocksize);
#endif
	return 0;
}

/*
 * Change mode, ownership, size, and time attributes of an inode.
 * Use FS_SETATTR to change mode and ownership.  Use the fs_itruncate
 * filesystem switch function to update size.  Use FS_STATF and
 * FS_IUPDAT to update timestamps.
 */
int
vop_setattr(ip, sap)
	register struct inode *ip;
	register struct nfssattr *sap;
{
	register bool_t changedtime = FALSE;
	register int error = 0;
	auto time_t atime, mtime;
	struct stat stb;
	struct argnamei nmarg;
#define OWNER(ip) \
	((u.u_uid == (ip)->i_uid || suser()) ? 0 : u.u_error)
 
	/*
	 * Change file access modes.  Must be owner or superuser.
	 * Server daemon takes on uid and gid of requester.
	 */
	if (sap->sa_mode != SATTR_NULL
	    && sap->sa_mode != (u_short)-1) {	/* XXX work-around Sun bug */
		error = OWNER(ip);
		if (error) {
			goto out;
		}
		nmarg.cmd = NI_CHMOD;
		nmarg.mode = sap->sa_mode & ~IFMT;
		if (u.u_uid != 0) {
			nmarg.mode &= ~ISVTX;
			/*
			 * Following line was ``if (!groupmember(ip->i_gid))''
			 */
			if (ip->i_gid != u.u_gid) {
				nmarg.mode &= ~ISGID;
			}
		}
		if (FS_SETATTR(ip, &nmarg) != 0) {
			ip->i_flag |= ICHG;
		}
	}
	/*
	 * Change file ownership.  Must be owner or su in att-land.
	 * (bsd: must be su).
	 */
	if (sap->sa_uid != SATTR_NULL || sap->sa_gid != SATTR_NULL) {
		error = OWNER(ip);
		if (error) {
			goto out;
		}
		nmarg.cmd = NI_CHOWN;
		nmarg.uid = (ushort)sap->sa_uid;
		nmarg.gid = (ushort)sap->sa_gid;
		if (FS_SETATTR(ip, &nmarg) != 0) {
			ip->i_flag |= ICHG;
		}
	}
	/*
	 * Truncate file.  Must be writeable and must not be a directory.
	 */
	if (sap->sa_size != SATTR_NULL) {
		if (ip->i_ftype == IFDIR) {
			error = EISDIR;
			goto out;
		}
		if (FS_ACCESS(ip, IWRITE)) {
			error = u.u_error;
			goto out;
		}
		FS_SETSIZE(ip, sap->sa_size);
	}
	/*
	 * Change file access or modified times.
	 */
	FS_STATF(ip, &stb);
	if (sap->sa_atime.tv_sec != SATTR_NULL) {
		error = OWNER(ip);
		if (error) {
			goto out;
		}
		atime = sap->sa_atime.tv_sec;
		changedtime = TRUE;
	} else {
		atime = stb.st_atime;
	}
	if (sap->sa_mtime.tv_sec != SATTR_NULL) {
		error = OWNER(ip);
		if (error) {
			goto out;
		}
		mtime = sap->sa_mtime.tv_sec;
		changedtime = TRUE;
	} else {	 
		mtime = stb.st_mtime;
	}
	if (changedtime) {
		ip->i_flag |= IACC|IUPD|ICHG|ISYN;
	}
out:
	/* should be asynchronous for performance */
	FS_IUPDAT(ip, &atime, &mtime);
	if (u.u_error) {
		error = u.u_error;
	}
	return (error);
}

/*
 * A wrapper for 5.3 FS_NAMEI which insists on a return value identical
 * either to the success argument or to NI_FAIL, and which returns an
 * error number or zero instead of the result code.
 */
int
vop_namei(ip, nxp, ap, success)
	struct inode *ip;
	struct nx *nxp;
	struct argnamei *ap;
	int success;
{
	register int result;

	ASSERT(ip->i_ftype == IFDIR);
	nxp->follow = DONTFOLLOW;
	result = FS_NAMEI(ip, nxp, ap);
	if (result == NI_FAIL) {
		return u.u_error;
	}
	ASSERT(result == success);
	return 0;
}

/*
 * Bmap and bread blocks from ip into *bpp, starting at the given offset
 * and extending at most count bytes.  If the request can't be mapped into
 * one logical block (i.e. buffer), return 0 with NULL *bpp.  Otherwise
 * return 0 and a buffer pointer via bpp.  Return an error number with
 * NULL *bpp if there was an i/o error.
 */
int
vop_bread(ip, offset, count, bpp)
	register struct inode *ip;
	off_t offset;
	register off_t count;
	struct buf **bpp;
{
	register struct user *uiop = &u;
	register int error;
	register struct buf *bp;
	struct bmapval ex, rex;

	uiop->u_offset = offset;
	uiop->u_count = count;
	error = FS_BMAP(ip, B_READ, &ex, &rex);
	if (!error) {
		if (uiop->u_pbsize != count) {
			*bpp = NULL;
			return 0;
		}
		if (rex.length) {
			bp = breada(ip->i_dev, ex.bn, ex.length, rex.bn,
				rex.length);
		} else {
			bp = bread(ip->i_dev, ex.bn, ex.length);
		}
		if (bp->b_flags & B_ERROR) {
			brelse(bp);
			bp = NULL;
			error = EIO;
		}
	}
	*bpp = bp;
	return error;
}

/*
 * Make an fhandle from an inode
 */
int
makefh(fh, ip)
	register fhandle_t *fh;
	register struct inode *ip;
{
	if (ip->i_fstyp == nfs_fstyp) {
		return (EREMOTE);
	}
	bzero((caddr_t)fh, NFS_FHSIZE);
	fh->fh_fsid = ip->i_dev;
	fh->fh_fno = ip->i_number;
	fh->fh_fgen = ip->i_gen;
	return (0);
}

/*
 * Convert an fhandle into a inode.
 * Uses the inode number in the fhandle (fh_fno) to get the locked inode.
 * WARNING: users of this routine must do an iput on the inode when they
 * are done with it.
 */
struct inode *
fhtoip(fh)
	register fhandle_t *fh;
{
	register struct mount *mp;
	register struct inode *ip;
	extern int nobody;

	mp = mount;
#ifdef SVR3
	appsema(&mnt_lck, PZERO);
	for (;;) {
		if (mp->m_flags & MINUSE && mp->m_dev == fh->fh_fsid)
			break;
		if (++mp >= (struct mount *) v.ve_mount)
			return NULL;
	}
	apvsema(&mnt_lck);
#else
	for (;;) {
		if (mp->m_dev == fh->fh_fsid)
			break;
		if ((mp = mp->m_next) == NULL)
			return NULL;
	}
#endif
	if (!(mp->m_flags & MEXPORTED)) {
		return NULL;
	}
	ip = iget(mp, fh->fh_fno);
	if (ip == NULL) {
#ifdef NFSDEBUG
		dprint(nfsdebug, 1, "fhtoip(%x, %x) couldn't iget\n",
		    fh->fh_fsid, fh->fh_fno);
#endif
		return (NULL);
	}
	if (ip->i_mntdev != mp) {
		/*
		 * If iget() crossed a mount-point, check whether the lower
		 * filesystem is exported and visible from above.  If not then
		 * recover the mounted-on inode.
		 */
		mp = ip->i_mntdev;
		if (!(mp->m_flags & MEXPORTED)
		    || !(mp->m_exflags & EX_SUBMOUNT)) {
			iput(ip);
			ip = mp->m_inodp;
			IHOLD(ip);
		}
	}
	if (ip->i_gen != fh->fh_fgen) {
#ifdef NFSDEBUG
		dprint(nfsdebug, 2, "NFS stale fhandle %x %d\n",
		    fh->fh_fsid, fh->fh_fno);
#endif
		iput(ip);
		return (NULL);
	}
	if (u.u_uid == (ushort) nobody
	    && (ip->i_mntdev->m_flags & MEXPORTED)) {
		/*
		 * We assume here that the uid is already set to "nobody"
		 * if the uid in the credentials with the call was 0.
		 */
#ifdef NFSDEBUG
		dprint(nfsdebug, 3, "fhtoip: root -> %d\n",
		    ip->i_mntdev->m_exroot);
#endif
		u.u_uid = ip->i_mntdev->m_exroot;
	}
	return (ip);
}

/*
 * Exportfs system call
 */
exportfs()
{
	register struct a {
		char	*dname;
		int	rootid;
		int	flags;
	} *uap = (struct a *) u.u_ap;
	register struct inode *ip;
	register struct mount *mp;
	register int was_running;

	ip = namei(USERPATH, NI_LOOKUP, FOLLOWLINK);
	if (ip == NULL) {
		return;
	}
	SETMUSTRUN(was_running);
	mp = ip->i_mntdev;
	mp->m_exroot = (u_short)uap->rootid;
	mp->m_exflags = (short)uap->flags;
	mp->m_flags |= MEXPORTED;
#ifdef NFSDEBUG
	dprint(nfsdebug, 3, "exportfs: root %d flags %x\n",
	    mp->m_exroot, mp->m_exflags);
#endif
	RESTOREMUSTRUN(was_running);
	iput(ip);
}

/*
 * Translate an inode file type into an nfs file type.
 */
enum nfsftype
itype_to_ntype(ftype)
	ushort ftype;
{
	switch (ftype) {
	case IFREG:
		return NFREG;
	case IFDIR:
		return NFDIR;
	case IFBLK:
		return NFBLK;
	case IFCHR:
		return NFCHR;
	case IFLNK:
		return NFLNK;
	default:
		return NFNON;
	};
}

/*
 * Translate an nfs file type into an inode file type.
 */
ushort
ntype_to_itype(rtype)
	enum nfsftype rtype;
{
	static ushort typemap[] = {
		0,	/* NFNON=0 */
		IFREG,	/* NFREG=1 - regular file */
		IFDIR,	/* NFDIR=2 - directory */
		IFBLK,	/* NFBLK=3 - block special */
		IFCHR,	/* NFCHR=4 - character special */
		IFLNK	/* NFLNK=5 - symbolic link */
	};
	register short i = (short) rtype;

	if ((short) NFNON <= i && i <= (short) NFLNK) {
		return typemap[i];
	}
	return 0;
}

/*
 * General utilities
 */
#ifdef SVR3
enum nfsstat nfs_statmap[] = {
	NFS_OK,			/* 0 */
	NFSERR_PERM,		/* EPERM = 1 */
	NFSERR_NOENT,		/* ENOENT = 2 */
	NFSERR_SRCH,		/* ESRCH = 3 */
	NFSERR_INTR,		/* EINTR = 4 */
	NFSERR_IO,		/* EIO = 5 */
	NFSERR_NXIO,		/* ENXIO = 6 */
	NFSERR_2BIG,		/* E2BIG = 7 */
	NFSERR_NOEXEC,		/* ENOEXEC = 8 */
	NFSERR_BADF,		/* EBADF = 9 */
	NFSERR_CHILD,		/* ECHILD = 10 */
	NFSERR_AGAIN,		/* EAGAIN = 11 */
	NFSERR_NOMEM,		/* ENOMEM = 12 */
	NFSERR_ACCES,		/* EACCES = 13 */
	NFSERR_FAULT,		/* EFAULT = 14 */
	NFSERR_NOTBLK,		/* ENOTBLK = 15 */
	NFSERR_BUSY,		/* EBUSY = 16 */
	NFSERR_EXIST,		/* EEXIST = 17 */
	NFSERR_XDEV,		/* EXDEV = 18 */
	NFSERR_NODEV,		/* ENODEV = 19 */
	NFSERR_NOTDIR,		/* ENOTDIR = 20 */
	NFSERR_ISDIR,		/* EISDIR = 21 */
	NFSERR_INVAL,		/* EINVAL = 22 */
	NFSERR_NFILE,		/* ENFILE = 23 */
	NFSERR_MFILE,		/* EMFILE = 24 */
	NFSERR_NOTTY,		/* ENOTTY = 25 */
	NFSERR_TXTBSY,		/* ETXTBSY = 26 */
	NFSERR_FBIG,		/* EFBIG = 27 */
	NFSERR_NOSPC,		/* ENOSPC = 28 */
	NFSERR_SPIPE,		/* ESPIPE = 29 */
	NFSERR_ROFS,		/* EROFS = 30 */
	NFSERR_MLINK,		/* EMLINK = 31 */
	NFSERR_PIPE,		/* EPIPE = 32 */
	NFSERR_DOM,		/* EDOM = 33 */
	NFSERR_RANGE,		/* ERANGE = 34 */
	NFS_OK,			/* ENOMSG = 35 */
	NFS_OK,			/* EIDRM = 36 */
	NFS_OK,			/* ECHRNG = 37 */
	NFS_OK,			/* EL2NSYNC = 38 */
	NFS_OK,			/* EL3HLT = 39 */
	NFS_OK,			/* EL3RST = 40 */
	NFS_OK,			/* ELNRNG = 41 */
	NFS_OK,			/* EUNATCH = 42 */
	NFS_OK,			/* ENOCSI = 43 */
	NFS_OK,			/* EL2HLT = 44 */
	NFS_OK,			/* EDEADLK = 45 */
	NFS_OK,			/* ENOLCK = 46 */
	NFS_OK,			/* 47 */
	NFS_OK,			/* 48 */
	NFS_OK,			/* 49 */
	NFS_OK,			/* EBADE = 50 */
	NFS_OK,			/* EBADR = 51 */
	NFS_OK,			/* EXFULL = 52 */
	NFS_OK,			/* ENOANO = 53 */
	NFS_OK,			/* EBADRQC = 54 */
	NFS_OK,			/* EBADSLT = 55 */
	NFS_OK,			/* EDEADLOCK = 56 */
	NFS_OK,			/* EBFONT = 57 */
	NFS_OK,			/* 58 */
	NFS_OK,			/* 59 */
	NFS_OK,			/* ENOSTR = 60 */
	NFS_OK,			/* ENODATA = 61 */
	NFS_OK,			/* ETIME = 62 */
	NFS_OK,			/* ENOSR = 63 */
	NFS_OK,			/* ENONET = 64 */
	NFS_OK,			/* ENOPKG = 65 */
	NFS_OK,			/* EREMOTE = 66 */
	NFS_OK,			/* ENOLINK = 67 */
	NFS_OK,			/* EADV = 68 */
	NFS_OK,			/* ESRMNT = 69 */
	NFS_OK,			/* ECOMM = 70 */
	NFS_OK,			/* EPROTO = 71 */
	NFS_OK,			/* 72 */
	NFS_OK,			/* 73 */
	NFS_OK,			/* EMULTIHOP = 74 */
	NFS_OK,			/* ELBIN = 75 */
	NFS_OK,			/* EDOTDOT = 76 */
	NFS_OK,			/* EBADMSG = 77 */
	NFS_OK,			/* 78 */
	NFS_OK,			/* 79 */
	NFS_OK,			/* ENOTUNIQ = 80 */
	NFS_OK,			/* EBADFD = 81 */
	NFS_OK,			/* EREMCHG = 82 */
	NFS_OK,			/* ELIBACC = 83 */
	NFS_OK,			/* ELIBBAD = 84 */
	NFS_OK,			/* ELIBSCN = 85 */
	NFS_OK,			/* ELIBMAX = 86 */
	NFS_OK,			/* ELIBEXEC = 87 */
	NFS_OK,			/* 88 */
	NFS_OK,			/* 89 */
	NFS_OK,			/* 90 */
	NFS_OK,			/* 91 */
	NFS_OK,			/* 92 */
	NFS_OK,			/* 93 */
	NFS_OK,			/* 94 */
	NFS_OK,			/* 95 */
	NFS_OK,			/* 96 */
	NFS_OK,			/* 97 */
	NFS_OK,			/* 98 */
	NFS_OK,			/* 99 */
	NFS_OK,			/* 100 */
	NFSERR_WOULDBLOCK,	/* EWOULDBLOCK = 101 */
	NFSERR_INPROGRESS,	/* EINPROGRESS = 102 */
	NFSERR_ALREADY,		/* EALREADY = 103 */
	NFSERR_NOTSOCK,		/* ENOTSOCK = 104 */
	NFSERR_DESTADDRREQ,	/* EDESTADDRREQ = 105 */
	NFSERR_MSGSIZE,		/* EMSGSIZE = 106 */
	NFSERR_PROTOTYPE,	/* EPROTOTYPE = 107 */
	NFSERR_NOPROTOOPT,	/* ENOPROTOOPT = 108 */
	NFSERR_PROTONOSUPPORT,	/* EPROTONOSUPPORT = 109 */
	NFSERR_SOCKTNOSUPPORT,	/* ESOCKTNOSUPPORT = 110 */
	NFSERR_OPNOTSUPP,	/* EOPNOTSUPP = 111 */
	NFSERR_PFNOSUPPORT,	/* EPFNOSUPPORT = 112 */
	NFSERR_AFNOSUPPORT,	/* EAFNOSUPPORT = 113 */
	NFSERR_ADDRINUSE,	/* EADDRINUSE = 114 */
	NFSERR_ADDRNOTAVAIL,	/* EADDRNOTAVAIL = 115 */
	NFSERR_NETDOWN,		/* ENETDOWN = 116 */
	NFSERR_NETUNREACH,	/* ENETUNREACH = 117 */
	NFSERR_NETRESET,	/* ENETRESET = 118 */
	NFSERR_CONNABORTED,	/* ECONNABORTED = 119 */
	NFSERR_CONNRESET,	/* ECONNRESET = 120 */
	NFSERR_NOBUFS,		/* ENOBUFS = 121 */
	NFSERR_ISCONN,		/* EISCONN = 122 */
	NFSERR_NOTCONN,		/* ENOTCONN = 123 */
	NFSERR_SHUTDOWN,	/* ESHUTDOWN = 124 */
	NFSERR_TOOMANYREFS,	/* ETOOMANYREFS = 125 */
	NFSERR_TIMEDOUT,	/* ETIMEDOUT = 126 */
	NFSERR_CONNREFUSED,	/* ECONNREFUSED = 127 */
	NFSERR_HOSTDOWN,	/* EHOSTDOWN = 128 */
	NFSERR_HOSTUNREACH,	/* EHOSTUNREACH = 129 */
	NFSERR_LOOP,		/* ELOOP = 130 */
	NFSERR_NAMETOOLONG,	/* ENAMETOOLONG = 131 */
	NFSERR_NOTEMPTY,	/* ENOTEMPTY = 132 */
	NFSERR_DQUOT,		/* EDQUOT = 133 */
	NFSERR_STALE,		/* ESTALE = 134 */
	NFSERR_REMOTE,		/* ENFSREMOTE = 135 */
};

short nfs_errmap[] = {
	0,		/* NFS_OK = 0 */
	EPERM,		/* NFSERR_PERM = 1 */
	ENOENT,		/* NFSERR_NOENT = 2 */
	ESRCH,		/* NFSERR_SRCH = 3 */
	EINTR,		/* NFSERR_INTR = 4 */
	EIO,		/* NFSERR_IO = 5 */
	ENXIO,		/* NFSERR_NXIO = 6 */
	E2BIG,		/* NFSERR_2BIG = 7 */
	ENOEXEC,	/* NFSERR_NOEXEC = 8 */
	EBADF,		/* NFSERR_BADF = 9 */
	ECHILD,		/* NFSERR_CHILD = 10 */
	EAGAIN,		/* NFSERR_AGAIN = 11 */
	ENOMEM,		/* NFSERR_NOMEM = 12 */
	EACCES,		/* NFSERR_ACCES = 13 */
	EFAULT,		/* NFSERR_FAULT = 14 */
	ENOTBLK,	/* NFSERR_NOTBLK = 15 */
	EBUSY,		/* NFSERR_BUSY = 16 */
	EEXIST,		/* NFSERR_EXIST = 17 */
	EXDEV,		/* NFSERR_XDEV = 18 */
	ENODEV,		/* NFSERR_NODEV = 19 */
	ENOTDIR,	/* NFSERR_NOTDIR = 20 */
	EISDIR,		/* NFSERR_ISDIR = 21 */
	EINVAL,		/* NFSERR_INVAL = 22 */
	ENFILE,		/* NFSERR_NFILE = 23 */
	EMFILE,		/* NFSERR_MFILE = 24 */
	ENOTTY,		/* NFSERR_NOTTY = 25 */
	ETXTBSY,	/* NFSERR_TXTBSY = 26 */
	EFBIG,		/* NFSERR_FBIG = 27 */
	ENOSPC,		/* NFSERR_NOSPC = 28 */
	ESPIPE,		/* NFSERR_SPIPE = 29 */
	EROFS,		/* NFSERR_ROFS = 30 */
	EMLINK,		/* NFSERR_MLINK = 31 */
	EPIPE,		/* NFSERR_PIPE = 32 */
	EDOM,		/* NFSERR_DOM = 33 */
	ERANGE,		/* NFSERR_RANGE = 34 */
	EWOULDBLOCK,	/* NFSERR_WOULDBLOCK = 35 */
	EINPROGRESS,	/* NFSERR_INPROGRESS = 36 */
	EALREADY,	/* NFSERR_ALREADY = 37 */
	ENOTSOCK,	/* NFSERR_NOTSOCK = 38 */
	EDESTADDRREQ,	/* NFSERR_DESTADDRREQ = 39 */
	EMSGSIZE,	/* NFSERR_MSGSIZE = 40 */
	EPROTOTYPE,	/* NFSERR_PROTOTYPE = 41 */
	ENOPROTOOPT,	/* NFSERR_NOPROTOOPT = 42 */
	EPROTONOSUPPORT,/* NFSERR_PROTONOSUPPORT = 43 */
	ESOCKTNOSUPPORT,/* NFSERR_SOCKTNOSUPPORT = 44 */
	EOPNOTSUPP,	/* NFSERR_OPNOTSUPP = 45 */
	EPFNOSUPPORT,	/* NFSERR_PFNOSUPPORT = 46 */
	EAFNOSUPPORT,	/* NFSERR_AFNOSUPPORT = 47 */
	EADDRINUSE,	/* NFSERR_ADDRINUSE = 48 */
	EADDRNOTAVAIL,	/* NFSERR_ADDRNOTAVAIL = 49 */
	ENETDOWN,	/* NFSERR_NETDOWN = 50 */
	ENETUNREACH,	/* NFSERR_NETUNREACH = 51 */
	ENETRESET,	/* NFSERR_NETRESET = 52 */
	ECONNABORTED,	/* NFSERR_CONNABORTED = 53 */
	ECONNRESET,	/* NFSERR_CONNRESET = 54 */
	ENOBUFS,	/* NFSERR_NOBUFS = 55 */
	EISCONN,	/* NFSERR_ISCONN = 56 */
	ENOTCONN,	/* NFSERR_NOTCONN = 57 */
	ESHUTDOWN,	/* NFSERR_SHUTDOWN = 58 */
	ETOOMANYREFS,	/* NFSERR_TOOMANYREFS = 59 */
	ETIMEDOUT,	/* NFSERR_TIMEDOUT = 60 */
	ECONNREFUSED,	/* NFSERR_CONNREFUSED = 61 */
	ELOOP,		/* NFSERR_LOOP = 62 */
	ENAMETOOLONG,	/* NFSERR_NAMETOOLONG = 63 */
	EHOSTDOWN,	/* NFSERR_HOSTDOWN = 64 */
	EHOSTUNREACH,	/* NFSERR_HOSTUNREACH = 65 */
	ENOTEMPTY,	/* NFSERR_NOTEMPTY = 66 */
	0,		/* NFSERR_PROCLIM = 67 */
	0,		/* NFSERR_USERS = 68 */
	EDQUOT,		/* NFSERR_DQUOT = 69 */
	ESTALE,		/* NFSERR_STALE = 70 */
	ENFSREMOTE,	/* NFSERR_REMOTE = 71 */
};
#endif

/*
 * Returns the prefered transfer size in bytes based on
 * what network interfaces are available.
 */
nfstsize()
{
	return NFS_MAXDATA;	/* XXX */
}

#ifdef NFSDEBUG
/*
 * Utilities used by both client and server
 * Standard levels:
 * 0) no debugging
 * 1) hard failures
 * 2) soft failures
 * 3) current test software
 * 4) main procedure entry points
 * 5) main procedure exit points
 * 6) utility procedure entry points
 * 7) utility procedure exit points
 * 8) obscure procedure entry points
 * 9) obscure procedure exit points
 * 10) random stuff
 * 11) all <= 1
 * 12) all <= 2
 * 13) all <= 3
 * ...
 */

/*VARARGS2*/
dprint(var, level, str, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12)
	int var;
	int level;
	char *str;
	int a1, a2, a3, a4, a5, a6, a7, a8, a9;
{
	if (var == level || (var > 10 && (var - 10) >= level))
		iprintf(str, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12);
}
#endif
