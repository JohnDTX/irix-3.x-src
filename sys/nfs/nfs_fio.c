/*
 * File i/o status and control operations for NFS.
 *
 * $Source: /d2/3.7/src/sys/nfs/RCS/nfs_fio.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:33:11 $
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
# include "sys/file.h"
# include "sys/inode.h"
# include "sys/mount.h"
# include "sys/nami.h"
# include "sys/fs/nfs.h"
# include "sys/fs/nfs_clnt.h"
# include "sys/fstyp.h"
# include "sys/fs/com_pncc.h"
# include "sys/fs/rnode.h"
#else
# include "../h/param.h"
# include "../h/systm.h"
# include "../h/user.h"
# include "../h/file.h"
# include "../h/inode.h"
# include "../h/mount.h"
# include "../h/nami.h"
# include "../nfs/nfs.h"
# include "../nfs/nfs_clnt.h"
# include "../nfs/rnode.h"
#endif

/* ARGSUSED */
nfs_openi(ip, flag)
	register struct inode *ip;
	int flag;
{
	int error;
	struct ucred cred;

#ifdef NFSDEBUG
	dprint(nfsdebug, 4, "nfs_openi %s %x %d flag %d\n",
	    vtomi(ip)->mi_hostname, vtor(ip)->r_nfsattr.na_fsid,
	    vtor(ip)->r_nfsattr.na_nodeid, flag);
#endif
	/*
	 * Validate cached data by getting the attributes.  Use nfsattr_inval()
	 * to guarantee fresh attributes (new in Sun 3.2 release).
	 */
	crinit(&u, &cred);
#ifdef SVR3
	plock(ip);
#endif
	nfsattr_inval(ip);
	error = nfs_getattr(ip, &cred);
	if (!error) {
		vtor(ip)->r_open++;
	} else {
		u.u_error = error;
	}
#ifdef SVR3
	prele(ip);
#endif
}

/* ARGSUSED */
nfs_closei(ip, flag, count, offset)
	register struct inode *ip;
	int flag;
	unsigned count;
	off_t offset;
{
	register struct rnode *rp;

	if (count > 1) {
		return;	/* vnode close is called on last ref only */
	}
#ifdef SVR3
	ASSERT(valusema(&ip->i_lock) <= 0);
#endif
#ifdef NFSDEBUG
	dprint(nfsdebug, 4, "nfs_closei %s %x %d flag %d\n",
	    vtomi(ip)->mi_hostname, vtor(ip)->r_nfsattr.na_fsid,
	    vtor(ip)->r_nfsattr.na_nodeid, flag);
#endif
	rp = vtor(ip);
	/*
	 * Flush all buffers on the async daemon list due to dangling
	 * inode pointer reference.  After the close, the inode will be
	 * unlocked and unreferenced, and ripe for recycling by ralloc().
	 */
	nfs_bflushw(rp, ip->i_dev);
	rp->r_flags &= ~RDIRTY;
	/*
	 * we want to be sure that ALL async (read/write) are done
	 * since binval (rinval) assumes that all have been done
	 * we use rinvalfree (binvalfree) instead which forces
	 * each buffer to be locked before STALing it.
	 */
	if (rp->r_unldip != NULL || rp->r_error)
		rinvalfree(ip);
	--rp->r_open;
	u.u_error = (flag & FWRITE) ? rp->r_error : 0;
}

/*ARGSUSED*/
int
nfs_fsync(vp)
	register struct inode *vp;
{
	register struct rnode *rp;

#ifdef SVR3
	ASSERT(valusema(&vp->i_lock) <= 0);
#endif
#ifdef NFSDEBUG
	dprint(nfsdebug, 4, "nfs_fsync %s %x %d\n",
	    vtomi(vp)->mi_hostname, vtor(vp)->r_nfsattr.na_fsid,
	    vtor(vp)->r_nfsattr.na_nodeid);
#endif
	rp = vtor(vp);
	if (rp->r_flags & RDIRTY) {
		ASSERT(rp->r_open);
		nfs_bflushw(rp, vp->i_dev);
		rp->r_flags &= ~RDIRTY;
	}
	return (rp->r_error);
}

/*
 * The following code is based on 5.3's s5access.  It's been cleaned
 * up to have a single return point and to use fewer gotos.  We also
 * respect the IXSAVED bit, which flags an inode having cached vm.
 * Notice that access may be denied without setting u.u_error.
 */
int
nfs_access(ip, mode)
	register struct inode *ip;
	register int mode;
{
	struct ucred cred;
	register struct ucred *credp;
	register struct nfsfattr *attrp;
	register int error, noaccess;

#ifdef SVR3
	ASSERT(valusema(&ip->i_lock) <= 0);
#endif
#ifdef NFSDEBUG
	dprint(nfsdebug, 4, "nfs_access %s %x %d mode %o uid %d\n",
	    vtomi(ip)->mi_hostname, vtor(ip)->r_nfsattr.na_fsid,
	    vtor(ip)->r_nfsattr.na_nodeid, mode, u.u_uid);
#endif
	credp = &cred;
	crinit(&u, credp);
	error = nfs_getattr(ip, credp);
	if (error) {
		u.u_error = error;
		noaccess = 1;
	} else {
		noaccess = 0;
		attrp = &vtor(ip)->r_nfsattr;
		switch (mode) {

		case ISUID:
		case ISGID:
		case ISVTX:
			noaccess = (mode & attrp->na_mode) == 0;
			break;
	
		case IMNDLCK:
			noaccess = 1;
			break;

		case IOBJEXEC:
#define	IANYEXEC	(IEXEC|(IEXEC>>3)|(IEXEC>>6))
			if (ip->i_ftype != IFREG
			    || (attrp->na_mode & IANYEXEC) == 0) {
				u.u_error = EACCES;
				noaccess = 1;
				break;
			}
			/* FALL THROUGH */
		case ICDEXEC:
			mode = IEXEC;
			goto exec;
	
		case IWRITE:
			if (rdonlyfs(ip->i_mntdev)) {
				u.u_error = EROFS;
				noaccess = 1;
				break;
			}
			if (ip->i_flag & IXSAVED) {
				(void) xflush(ip);
			}
			if (ip->i_flag & ITEXT) {
				u.u_error = ETXTBSY;
				noaccess = 1;
				break;
			}
			/* FALL THROUGH */
		case IREAD:
		case IEXEC:
exec:
			if (credp->cr_uid == 0) {
				break;
			}
			if (credp->cr_uid != ip->i_uid) {
				mode >>= 3;
				if (credp->cr_gid != ip->i_gid) {
					mode >>= 3;
				}
			}
			if ((mode & attrp->na_mode) == 0) {
				u.u_error = EACCES;
				noaccess = 1;
			}
			break;
		}
	}

#ifdef NFSDEBUG
	dprint(nfsdebug, 5, "nfs_access: error %d returning %d\n",
	    u.u_error, noaccess);
#endif
	return (noaccess);
}

/*
 * Perform chmod and chown over the wire.
 */
int
nfs_setattr(ip, ap)
	register struct inode *ip;
	register struct argnamei *ap;
{
	struct ucred cred;
	struct nfssattr sattr;
	register int error;

#ifdef SVR3
	ASSERT(valusema(&ip->i_lock) <= 0);
#endif
	if (u.u_uid != ip->i_uid && !suser()) {
		return (0);
	}
	if (rdonlyfs(ip->i_mntdev)) {
		u.u_error = EROFS;
		return (0);
	}

	crinit(&u, &cred);
	sattr_null(&sattr);

	switch (ap->cmd) {
	case NI_CHMOD:
		sattr.sa_mode = ap->mode & MODEMSK;
		break;

	case NI_CHOWN:
		sattr.sa_uid = ap->uid;
		sattr.sa_gid = ap->gid;
		break;
	}

	error = nfs_chattr(ip, &sattr, &cred);
	if (error) {
		u.u_error = error;
		return (0);
	}
	return (1);
}

/*
 * Get attributes from server into rnode's attribute cache.
 */
int
nfs_getattr(ip, cred)
	register struct inode *ip;
	register struct ucred *cred;
{
	register struct rnode *rp;
	register int error;
	struct nfsattrstat *ns;

#ifdef SVR3
	ASSERT(valusema(&ip->i_lock) <= 0);
#endif
#ifdef NFSDEBUG
	dprint(nfsdebug, 4, "nfs_getattr %s %x %d\n",
	    vtomi(ip)->mi_hostname, vtor(ip)->r_nfsattr.na_fsid,
	    vtor(ip)->r_nfsattr.na_nodeid);
#endif
	(void) nfs_fsync(ip);		/* sync so mod time is right */

	/*
	 * if cached attributes are stale refresh from across the net
	 */
	rp = vtor(ip);
	if (time < rp->r_nfsattrtime) {
		/*
		 * Use cached attributes.
		 */
		rattr_to_iattr(ip);
		return 0;
	}
#ifdef NFSDEBUG
	dprint(nfsdebug, 10, "nfs_getattr: refreshing stale attributes\n");
#endif
	ns = (struct nfsattrstat *)kmem_alloc((u_int)sizeof(*ns));
	error =
	    rfscall(vtomi(ip), RFS_GETATTR, xdr_fhandle,
		(caddr_t)vtofh(ip), xdr_attrstat,
		(caddr_t)ns, cred);
	if (!error) {
		error = geterrno(ns->ns_status);
		if (!error) {
			nfs_attrcache(ip, &ns->ns_attr, SFLUSH);
		}
	}
	kmem_free((caddr_t)ns, (u_int)sizeof(*ns));

#ifdef NFSDEBUG
	dprint(nfsdebug, 5, "nfs_getattr: returns %d\n", error);
#endif
	return (error);
}

/*
 * Send attributes in sap across the wire to a server.
 */
int
nfs_chattr(ip, sap, cred)
	register struct inode *ip;
	struct nfssattr *sap;
	register struct ucred *cred;
{
	register int error;
	struct nfssaargs args;
	struct nfsattrstat *ns;

#ifdef NFSDEBUG
	dprint(nfsdebug, 4, "nfs_setattr %s %x %d\n",
	    vtomi(ip)->mi_hostname, vtor(ip)->r_nfsattr.na_fsid,
	    vtor(ip)->r_nfsattr.na_nodeid);
#endif
	(void) nfs_fsync(ip);
	if (sap->sa_size != SATTR_NULL) {
		ip->i_size = sap->sa_size;
	}
	args.saa_sa = *sap;
	args.saa_fh = *vtofh(ip);
	ns = (struct nfsattrstat *)kmem_alloc((u_int)sizeof(*ns));
	error =
	    rfscall(vtomi(ip), RFS_SETATTR, xdr_saargs, (caddr_t)&args,
		xdr_attrstat, (caddr_t)ns, cred);
	if (!error) {
		error = geterrno(ns->ns_status);
		if (!error) {
			nfs_attrcache(ip, &ns->ns_attr, SFLUSH);
		}
	}

	kmem_free((caddr_t)ns, (u_int)sizeof(*ns));
#ifdef NFSDEBUG
	dprint(nfsdebug, 5, "nfs_setattr: returning %d\n", error);
#endif
	return (error);
}

/*
 * Remote file and i/o control calls are unsupported in NFS.
 */
/* ARGSUSED */
nfs_fcntl(ip, cmd, arg, flag, offset)
	struct inode *ip;
	int cmd, arg, flag;
	off_t offset;
{
	u.u_error = EINVAL;
}

/* ARGSUSED */
nfs_ioctl(ip, cmd, arg, flag)
	struct inode *ip;
	int cmd, arg, flag;
{
	u.u_error = ENOTTY;
}
