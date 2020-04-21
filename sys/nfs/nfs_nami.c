#ifndef lint
static	char	rcsid[] = "$Header: /d2/3.7/src/sys/nfs/RCS/nfs_nami.c,v 1.1 89/03/27 17:33:14 root Exp $";
#endif
/*
 * NFS-dependent namei implementation.
 *
 * $Source: /d2/3.7/src/sys/nfs/RCS/nfs_nami.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:33:14 $
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
# include "sys/fstyp.h"
# include "sys/fs/com_pncc.h"
# include "sys/fs/nfs.h"
# include "sys/fs/nfs_clnt.h"
# include "sys/fs/nfs_export.h"
# include "sys/fs/rnode.h"
# include "rpc/xdr.h"
#else
# include "../h/param.h"
# include "../h/user.h"
# include "../h/file.h"
# include "../h/inode.h"
# include "../h/mount.h"
# include "../h/nami.h"
# include "../com/com_pncc.h"
# include "../nfs/nfs.h"
# include "../nfs/nfs_clnt.h"
# include "../nfs/nfs_export.h"
# include "../nfs/rnode.h"
# include "../rpc/xdr.h"
#endif

/*
 * We must avoid a deadlock in lookup between a process looking "down" from
 * a directory to a sub-directory and another looking "up" from below.  The
 * lookup operation takes a locked directory inode and a name, and yields the
 * entry's inode.  If the name is "." or an equivalent, lookup returns the
 * directory inode straight away.  If the entry name does denotes neither the
 * directory nor its parent (".."), the entry inode is returned referenced and
 * locked.  If the entry is "..", then we unlock the directory before getting
 * its parent.  This opens a window through which may race any number of
 * processes trying to get the parent.
 *
 * The locking protocol here follows that used in the Berkeley filesystem:
 * we lock parent-child pairs searching down the tree, but unlock the parent
 * before getting the child when the child is "..".  Cycles of directory hard
 * links other than ".." in a sub-directory and the sub-directory's name may
 * result in deadlock.
 */

int
nfs_namei(nxp, ap)
	register struct nx *nxp;
	register struct argnamei *ap;
{
	register char *comp;
	register struct inode *dp;
	register int cmd;
	register int error;
	register int result;
	struct ucred cred;

#ifdef NFSDEBUG
	dprint(nfsdebug, 8, "nfs_namei %s %x %d '%s'\n",
	    vtomi(nxp->dp)->mi_hostname, vtor(nxp->dp)->r_nfsattr.na_fsid,
	    vtor(nxp->dp)->r_nfsattr.na_nodeid, nxp->comp);
#endif
	/*
	 * Check for a symbolic link.  If one is at hand, expand its
	 * contents into the pathname buffer and restart generic namei
	 * processing.  The link's contents are read using FS_READI by
	 * pn_putlink.  FS_READI calls nfs_readi to read the link.
	 */
	dp = nxp->dp;
#ifdef SVR3
	ASSERT(valusema(&dp->i_lock) <= 0);
#endif
	if (dp->i_ftype == IFLNK) {
		u.u_error = pn_putlink(nxp->pn, dp);
		if (u.u_error != 0) {
			iput(dp);
			return NI_FAIL;
		}
		return NI_RESTART;
	}
	ASSERT(dp->i_ftype == IFDIR);

	cmd = (ap == NI_LOOKUP) ? NI_LOOKUP : ap->cmd;
	comp = nxp->comp;
	crinit(&u, &cred);

	/*
	 * Switch out to nfs inode operations, hacked to conform to the
	 * filesystem switch's namei interface.
	 */
	switch (cmd) {

	case NI_LOOKUP: {
		register struct inode *ip;

		error = nfs_lookup(dp, comp, nxp->len, &nxp->ip, &cred);
		if (error) {
			ASSERT(nxp->ip == NULL);
			break;	/* not found or error */
		}
		ip = nxp->ip;
		if (ip->i_flag & IISROOT) {
			nxp->flags |= NX_ISROOT;
			ASSERT(ip->i_mntdev->m_mount == ip);
		}
		result = NI_PASS;
		break;
	}

	case NI_RENAME: {
		if (ap->dp->i_mntdev != dp->i_mntdev) {
			error = EXDEV;
			break;
		}
#ifdef SVR3
		if (ap->dp != dp)
			ilock2(ap->dp, dp);
#endif
		if (nfs_access(ap->dp, IWRITE) || nfs_access(dp, IWRITE)) {
			error = u.u_error;
			break;
		}
		error = nfs_rename(ap->dp, ap->name, dp, comp, &cred);
#ifdef SVR3
		if (ap->dp != dp)
			prele(ap->dp);
#endif
		result = NI_NULL;
		break;
	}

	case NI_SYMLINK: {
		struct nfssattr sa;
		char *targetname;

		if (nfs_access(dp, IWRITE)) {
			error = u.u_error;
			break;
		}
		targetname = (char *) kmem_alloc(ap->namlen + 1);
		if (copyin((caddr_t) ap->name, (caddr_t) targetname, 
		    ap->namlen)) {
			error = EFAULT;
			break;
		}
		targetname[ap->namlen] = '\0';
		sattr_null(&sa);
		sa.sa_mode = ap->mode;
		error = nfs_symlink(dp, comp, &sa, targetname, &cred);
		kmem_free(targetname, ap->namlen + 1);
		result = NI_NULL;
		break;
	}

	case NI_LINK: {
		register struct inode *ip;

		if (nfs_access(dp, IWRITE)) {
			error = u.u_error;
			break;
		}
		ip = ap->ip;
		if (ip->i_mntdev != dp->i_mntdev) {
			error = EXDEV;	/* archaic error number */
			break;
		}
#ifdef SVR3
		if (ip != dp)
			ilock2(ip, dp);
#endif
		error = nfs_link(ip, dp, comp, &cred);
#ifdef SVR3
		if (ip != dp)
			prele(ip);
#endif
		result = NI_NULL;
		break;
	}

	case NI_MKNOD: {
		error = EINVAL;
		break;
	}

	case NI_XCREAT:
	case NI_CREAT: {
		auto struct inode *ip;

		error = nfs_lookup(dp, comp, nxp->len, &ip, &cred);
		result = NI_DONE;
		if (!error) {
			ASSERT(ip != NULL);
			/*
			 * Create and exclusive-create are *not* the only namei
			 * operations which follow terminal symlinks, but due
			 * to the way AT&T split symlink expansion across the
			 * switch, it is inefficient and extremely awkward to
			 * unify code in nfs_namei() to do non-lookup terminal
			 * link expansion.  XXX
			 */
			ASSERT(nxp->follow == FOLLOWLINK);
			if (ip->i_ftype == IFLNK) {
				nxp->ip = ip;
				result = NI_PASS;
			} else if (cmd == NI_XCREAT) {
				error = EEXIST;
				if (ip != dp)
					iput(ip);
			} else {
				ap->rcode = FSN_FOUND;
				if (ip != dp)
					nxp->dp = ip;
			}
		} else if (error == ENOENT) {
			struct nfssattr sa;

			ASSERT(ip == NULL);
			if (nfs_access(dp, IWRITE)) {
				error = u.u_error;
				break;
			}
			ap->mode &= ~u.u_cmask & (MODEMSK & ~ISVTX);
			sattr_null(&sa);
			sa.sa_mode = ap->mode;
			error = nfs_create(dp, comp, &sa, &ip, &cred);
			if (!error) {
				ap->rcode = FSN_NOTFOUND;
				ASSERT(ip != NULL);
				nxp->dp = ip;
			}
		}
		break;
	}

	case NI_DEL: {
		auto struct inode *aip = NULL;
		register struct inode *ip;

		error = nfs_lookup(dp, comp, nxp->len, &aip, &cred);
		ip = aip;

		/*
		 * First check for lookup failure and for "." alias.
		 */
		if (error) {
			ASSERT(ip == NULL);
			error = ENOENT;
		} else if (ip == dp) {
			error = EISDIR;
		} else {
			/*
			 * Check for directory unlink without superuser
			 * status, unlink across a mount point, and busy
			 * text unlink.  Remove the entry and drop ip's
			 * link count.  In any event put ip.
			 */
			if (nfs_access(dp, IWRITE)) {
				error = u.u_error;
			} else if (ip->i_ftype == IFDIR && !suser()) {
				error = EPERM;
			} else if (dp->i_mntdev != ip->i_mntdev) {
				error = EBUSY;
			} else {
				if (ip->i_flag & IXSAVED) {
					(void) xflush(ip);
				}
				if (ip->i_flag & ITEXT && ip->i_nlink == 1) {
					error = ETXTBSY;
				}
			}
			if (!error) {
				error = nfs_remove(dp, comp, ip, &cred);
				result = NI_NULL;
			}
			iput(ip);
		}
		break;
	}

	case NI_MKDIR: {
		auto struct inode *cdp;
		struct nfssattr sa;

		if (nfs_access(dp, IWRITE)) {
			error = u.u_error;
			break;
		}
		sattr_null(&sa);
		sa.sa_mode = ap->mode & (PERMMSK & ~u.u_cmask);
		error = nfs_mkdir(dp, comp, &sa, &cdp, &cred);
		if (!error) {
			iput(cdp);
		}
		result = NI_NULL;
		break;
	}

	case NI_RMDIR: {
		auto struct inode *acdp = NULL;
		register struct inode *cdp;

		error = nfs_lookup(dp, comp, nxp->len, &acdp, &cred);
		cdp = acdp;
		if (error) {
			error = ENOENT;
		} else if (nfs_access(dp, IWRITE)) {
			error = u.u_error;
		} else if (cdp == dp || ISDOTDOT(comp, nxp->len)) {
			error = EINVAL;
		} else if (cdp->i_ftype != IFDIR) {	/* not a directory */
			error = ENOTDIR;
		} else if (dp->i_mntdev != cdp->i_mntdev) {
			error = EBUSY;
		} else if (cdp == u.u_cdir) {		/* can't rmdir cwd */
			error = EINVAL;
		}
		if (cdp && cdp != dp) {
			iput(cdp);
		}
		if (!error) {
			error = nfs_rmdir(dp, comp, &cred);
			result = NI_NULL;
		}
		break;
	}

#ifdef OS_ASSERT
	default:
		panic("nfs_namei\n");
#endif
	}

	/*
	 * Now figure out how to return.
	 */
	if (error) {
		result = NI_FAIL;
		u.u_error = error;
	}
	switch (result) {
	case NI_PASS:	/* caller (pn_lookup) will put nxp->dp */
		break;

	case NI_DONE:	/* nxp->dp points to created inode */
		if (nxp->dp != dp) {
			iput(dp);
		}
		break;

	case NI_FAIL:	/* always release dp on failure/error */
	case NI_NULL:	/* void namei's return by releasing dp */
		iput(dp);
		break;
	}
#ifdef NFSDEBUG
	dprint(nfsdebug, 9, "nfs_namei returning %d error = %d\n",
	    result, error);
#endif
	return result;
}

static int
nfs_lookup(dvp, name, namlen, vpp, cred)
	register struct inode *dvp;
	char *name;
	unsigned short namlen;
	register struct inode **vpp;
	struct ucred *cred;
{
	register int error;
	struct ncentry ncent;
	enum ncstat stat;
	struct nfsdiropargs da;
	struct nfsdiropres *dr;
	register int isdotdot;

#ifdef NFSDEBUG
	dprint(nfsdebug, 4, "nfs_lookup %s %x %d '%s'\n",
	    vtomi(dvp)->mi_hostname, vtor(dvp)->r_nfsattr.na_fsid,
	    vtor(dvp)->r_nfsattr.na_nodeid, name);
#endif
	/*
	 * Before checking pncc, call getattr to be sure the node
	 * hasn't changed.  getattr will purge pncc of dvp references
	 * if a change has occurred.
	 */
	if (error = nfs_getattr(dvp, cred)) {
		return (error);
	}

	/*
	 * "." == ".." for root - pn_lookup has already locked the parent,
	 * so we would hang if we tried an iget().
	 */
	isdotdot = ISDOTDOT(name, namlen);
	if (ISDOT(name, namlen) || isdotdot && (dvp->i_flag & IISROOT)) {
#ifdef SVR3
		ASSERT(valusema(&dvp->i_lock) <= 0);
#else
		ASSERT(dvp->i_flag & ILOCKED);
#endif
		*vpp = dvp;
		return(0);
	}
	if (isdotdot) {
		/* going up the tree? unlock dvp first */
		iunlock(dvp);
	}

	/*
	 * Check the pathname component cache for a capable entry.
	 */
	stat = pncc_lookup(dvp, name, namlen, &ncent);
	if (stat == NC_HIT) {
		error = nfs_igetbyname(dvp, &ncent, cred, vpp);
		if (error) {
			if (isdotdot) {
				ilock(dvp);
			}
			return error;
		}
	} else {
		*vpp = NULL;
	}

	/*
	 * Have to go over the wire and search the directory.
	 */
	if (*vpp == NULL) {
		dr = (struct  nfsdiropres *)kmem_alloc((u_int)sizeof(*dr));
		setdiropargs(&da, name, dvp);
		error =
		    rfscall(vtomi(dvp), RFS_LOOKUP, xdr_diropargs, (caddr_t)&da,
			xdr_diropres, (caddr_t)dr, cred);
		if (!error) {
			error = geterrno(dr->dr_status);
		}
		if (!error) {
			error = nfs_igetbyfh(dvp->i_mntdev, &dr->dr_fhandle,
				&dr->dr_attr, vpp);
			if (!error) {
				ASSERT(*vpp);
				if (stat == NC_MISS) {
					ncent.nce_inum = (*vpp)->i_number;
					ncent.nce_pcap = rnamecap(dvp);
					ncent.nce_cap = rnamecap(*vpp);
					(void) pncc_enter(dvp, name, namlen,
						&ncent);
				}
			}
		} else {
			*vpp = (struct inode *)0;
		}
		kmem_free((caddr_t)dr, (u_int)sizeof(*dr));
	}

	if (isdotdot) {
		ilock(dvp);
	}

#ifdef NFSDEBUG
	dprint(nfsdebug, 5, "nfs_lookup returning %d vp = %x\n", error, *vpp);
#endif
	return (error);
}

static int
nfs_rename(odvp, onm, ndvp, nnm, cred)
	struct inode *odvp;
	char *onm;
	struct inode *ndvp;
	char *nnm;
	struct ucred *cred;
{
	register int error;
	enum nfsstat status;
	struct nfsrnmargs args;

#ifdef SVR3
	ASSERT(valusema(&odvp->i_lock) <= 0);
#endif
#ifdef NFSDEBUG
	dprint(nfsdebug, 4, "nfs_rename from %s %x %d '%s' to %s %x %d '%s'\n",
	    vtomi(odvp)->mi_hostname, vtor(odvp)->r_nfsattr.na_fsid,
	    vtor(odvp)->r_nfsattr.na_nodeid, onm,
	    vtomi(ndvp)->mi_hostname, vtor(ndvp)->r_nfsattr.na_fsid,
	    vtor(ndvp)->r_nfsattr.na_nodeid, nnm);
#endif
	if (!strcmp(onm, ".") || !strcmp(onm, "..") || !strcmp(nnm, ".")
	    || !strcmp (nnm, "..")) {
		error = EINVAL;
	} else {
		rnewcap(odvp);		/* the old name is going away */
		rnewcap(ndvp);		/* in case the target exists */
		setdiropargs(&args.rna_from, onm, odvp);
		setdiropargs(&args.rna_to, nnm, ndvp);
		error = rfscall(vtomi(odvp), RFS_RENAME,
		    xdr_rnmargs, (caddr_t)&args,
		    xdr_enum, (caddr_t)&status, cred);
		nfsattr_inval(odvp);	/* directory was modified */
		nfsattr_inval(ndvp);	/* directory was modified */
		if (!error) {
			error = geterrno(status);
		}
	}
#ifdef NFSDEBUG
	dprint(nfsdebug, 5, "nfs_rename returning %d\n", error);
#endif
	return (error);
}

static int
nfs_symlink(dvp, lnm, tsa, tnm, cred)
	struct inode *dvp;
	char *lnm;		/* link name */
	struct nfssattr *tsa;	/* target attributes (degenerate) */
	char *tnm;		/* target name */
	struct ucred *cred;
{
	int error;
	struct nfsslargs args;
	enum nfsstat status;

#ifdef NFSDEBUG
	dprint(nfsdebug, 4, "nfs_symlink %s %x %d '%s' to '%s'\n",
	    vtomi(dvp)->mi_hostname, vtor(dvp)->r_nfsattr.na_fsid,
	    vtor(dvp)->r_nfsattr.na_nodeid, lnm, tnm);
#endif
	setdiropargs(&args.sla_from, lnm, dvp);
	args.sla_sa = *tsa;
	args.sla_tnm = tnm;
	error = rfscall(vtomi(dvp), RFS_SYMLINK, xdr_slargs, (caddr_t)&args,
	    xdr_enum, (caddr_t)&status, cred);
	nfsattr_inval(dvp);
	if (!error) {
		error = geterrno(status);
	}
#ifdef NFSDEBUG
	dprint(nfsdebug, 5, "nfs_sysmlink: returning %d\n", error);
#endif
	return (error);
}

static int
nfs_link(vp, tdvp, tnm, cred)
	struct inode *vp;
	struct inode *tdvp;
	char *tnm;
	struct ucred *cred;
{
	int error;
	struct nfslinkargs args;
	enum nfsstat status;

#ifdef SVR3
	ASSERT(valusema(&vp->i_lock) <= 0 && valusema(&tdvp->i_lock) <= 0);
#endif
#ifdef NFSDEBUG
	dprint(nfsdebug, 4, "nfs_link from %s %x %d to %s %x %d '%s'\n",
	    vtomi(vp)->mi_hostname,
	    vtor(vp)->r_nfsattr.na_fsid, vtor(vp)->r_nfsattr.na_nodeid,
	    vtomi(tdvp)->mi_hostname,
	    vtor(tdvp)->r_nfsattr.na_fsid, vtor(tdvp)->r_nfsattr.na_nodeid,
	    tnm);
#endif
	args.la_from = *vtofh(vp);
	setdiropargs(&args.la_to, tnm, tdvp);
	error = rfscall(vtomi(vp), RFS_LINK, xdr_linkargs, (caddr_t)&args,
	    xdr_enum, (caddr_t)&status, cred);
	nfsattr_inval(tdvp);	/* directory was modified */
	nfsattr_inval(vp);	/* link count was modified */
	if (!error) {
		error = geterrno(status);
	}
#ifdef NFSDEBUG
	dprint(nfsdebug, 5, "nfs_link returning %d\n", error);
#endif
	return (error);
}

/*ARGSUSED*/
static int
nfs_create(dvp, nm, sa, vpp, cred)
	struct inode *dvp;
	char *nm;
	struct nfssattr *sa;
	struct inode **vpp;
	struct ucred *cred;
{
	int error;
	struct nfscreatargs args;
	struct  nfsdiropres *dr;

#ifdef NFSDEBUG
	dprint(nfsdebug, 4, "nfs_create %s %x %d '%s', mode %o\n",
	    vtomi(dvp)->mi_hostname,
	    vtor(dvp)->r_nfsattr.na_fsid, vtor(dvp)->r_nfsattr.na_nodeid,
	    nm, sa->sa_mode);
#endif
	*vpp = (struct inode *) 0;
	dr = (struct  nfsdiropres *)kmem_alloc((u_int)sizeof(*dr));
	setdiropargs(&args.ca_da, nm, dvp);
	args.ca_sa = *sa;
	error = rfscall(vtomi(dvp), RFS_CREATE, xdr_creatargs, (caddr_t)&args,
	    xdr_diropres, (caddr_t)dr, cred);
	nfsattr_inval(dvp);	/* directory was modified */
	if (!error) {
		error = geterrno(dr->dr_status);
		if (!error) {
			error = nfs_igetbyfh(dvp->i_mntdev, &dr->dr_fhandle,
				&dr->dr_attr, vpp);
			if (!error) {
				ASSERT(*vpp);
				(*vpp)->i_size = 0;
			}
		}
	}
	kmem_free((caddr_t)dr, (u_int)sizeof(*dr));
#ifdef NFSDEBUG
	dprint(nfsdebug, 5, "nfs_create returning %d\n", error);
#endif
	return (error);
}

/*
 * Weirdness: if the inode to be removed is open
 * we rename it instead of removing it and nfs_inactive
 * will remove the new name.
 */
static int
nfs_remove(dvp, nm, vp, cred)
	register struct inode *dvp;
	char *nm;
	register struct inode *vp;
	register struct ucred *cred;
{
	auto enum nfsstat status;
	register struct rnode *rp;
	register int error;
	struct nfsdiropargs da;
	char *tmpname, *newname();

#ifdef NFSDEBUG
	dprint(nfsdebug, 4, "nfs_remove %s %x %d '%s'\n",
	    vtomi(dvp)->mi_hostname, vtor(dvp)->r_nfsattr.na_fsid,
	    vtor(dvp)->r_nfsattr.na_nodeid, nm);
#endif
	status = NFS_OK;
	rnewcap(vp);
	rp = vtor(vp);
	if (rp->r_open && rp->r_unlname == NULL) {
		tmpname = newname(nm);
		error = nfs_rename(dvp, nm, dvp, tmpname, cred);
		if (error) {
			kmem_free((caddr_t)tmpname, (u_int)NFS_MAXNAMLEN);
		} else {
			rp->r_unldip = dvp;
			rp->r_unlname = tmpname;
			rp->r_unlcred = *cred;
			iuse(dvp);
		}
	} else {
		setdiropargs(&da, nm, dvp);
		error =
		    rfscall(vtomi(dvp), RFS_REMOVE, xdr_diropargs,
			(caddr_t)&da, xdr_enum, (caddr_t)&status,
			cred);
		nfsattr_inval(dvp);	/* directory was modified */
		nfsattr_inval(vp);		/* link count was modified */
	}
	rflush(vp);
	if (!error) {
		error = geterrno(status);
	}
#ifdef NFSDEBUG
	dprint(nfsdebug, 5, "nfs_remove returning %d\n", error);
#endif
	return (error);
}

static int
nfs_mkdir(dvp, nm, sa, vpp, cred)
	struct inode *dvp;
	char *nm;
	register struct nfssattr *sa;
	struct inode **vpp;
	struct ucred *cred;
{
	int error;
	struct nfscreatargs args;
	struct  nfsdiropres *dr;

#ifdef NFSDEBUG
	dprint(nfsdebug, 4, "nfs_mkdir %s %x %d '%s'\n",
	    vtomi(dvp)->mi_hostname, vtor(dvp)->r_nfsattr.na_fsid,
	    vtor(dvp)->r_nfsattr.na_nodeid, nm);
#endif
	dr = (struct  nfsdiropres *)kmem_alloc((u_int)sizeof(*dr));
	setdiropargs(&args.ca_da, nm, dvp);
	args.ca_sa = *sa;
	error = rfscall(vtomi(dvp), RFS_MKDIR, xdr_creatargs, (caddr_t)&args,
	    xdr_diropres, (caddr_t)dr, cred);
	nfsattr_inval(dvp);
	if (!error) {
		error = geterrno(dr->dr_status);
	}
	if (!error) {
		error = nfs_igetbyfh(dvp->i_mntdev, &dr->dr_fhandle,
			&dr->dr_attr, vpp);
		if (!error) {
			ASSERT(*vpp);
			/*
			 * XXX workaround for Sun server bug where bad size
			 * XXX is returned in new directory's attributes
			 */
			nfsattr_inval(*vpp);
		}
	} else {
		*vpp = (struct inode *)0;
	}
	kmem_free((caddr_t)dr, (u_int)sizeof(*dr));
#ifdef NFSDEBUG
	dprint(nfsdebug, 5, "nfs_mkdir returning %d\n", error);
#endif
	return (error);
}

static int
nfs_rmdir(dvp, nm, cred)
	struct inode *dvp;
	char *nm;
	struct ucred *cred;
{
	int error;
	enum nfsstat status;
	struct nfsdiropargs da;

#ifdef NFSDEBUG
	dprint(nfsdebug, 4, "nfs_rmdir %s %x %d '%s'\n",
	    vtomi(dvp)->mi_hostname, vtor(dvp)->r_nfsattr.na_fsid,
	    vtor(dvp)->r_nfsattr.na_nodeid, nm);
#endif
	setdiropargs(&da, nm, dvp);
	rnewcap(dvp);
	error = rfscall(vtomi(dvp), RFS_RMDIR, xdr_diropargs, (caddr_t)&da,
	    xdr_enum, (caddr_t)&status, cred);
	nfsattr_inval(dvp);
	if (!error) {
		error = geterrno(status);
	}
#ifdef NFSDEBUG
	dprint(nfsdebug, 5, "nfs_rmdir returning %d\n", error);
#endif
	return (error);
}
