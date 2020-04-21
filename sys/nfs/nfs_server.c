/*      @(#)nfs_server.c 1.1 85/05/30 SMI      */

/*
 * Copyright (c) 1985 by Sun Microsystems, Inc.
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
# include "sys/buf.h"
# include "sys/file.h"
# include "sys/conf.h"
# include "sys/fstyp.h"
# include "sys/inode.h"
# include "sys/mount.h"
# include "sys/nami.h"
# include "sys/statfs.h"
# undef MFREE
# include "sys/mbuf.h"
# include "sys/fs/com_inode.h"
# include "sys/socketvar.h"
# include "sys/immu.h"
# include "sys/pda.h"
# include "netinet/in.h"
# include "sys/fs/nfs.h"
# include "sys/fs/nfs_stat.h"
# include "rpc/types.h"
# include "rpc/auth.h"
# include "rpc/auth_unix.h"
# include "rpc/svc.h"
# include "rpc/xdr.h"
#else
# include "../h/param.h"
# include "../h/systm.h"
# include "../h/user.h"
# include "../h/buf.h"
# include "../h/file.h"
# include "../h/fstyp.h"
# include "../h/inode.h"
# include "../h/mount.h"
# include "../h/nami.h"
# include "../h/statfs.h"
# include "../h/mbuf.h"
# include "../h/socketvar.h"
# include "../netinet/in.h"
# include "../nfs/nfs.h"
# include "../nfs/nfs_stat.h"
# include "../rpc/types.h"
# include "../rpc/auth.h"
# include "../rpc/auth_unix.h"
# include "../rpc/svc.h"
# include "../rpc/xdr.h"
#endif
 
/*
 * rpc service program version range supported
 */
#define	VERSIONMIN	2
#define	VERSIONMAX	2

/*
 * Returns true iff filesystem for a given fsid is exported read-only
 */
#define	rdonly(vp)	(((vp)->i_mntdev->m_flags & MEXPORTED) && \
			 ((vp)->i_mntdev->m_exflags & EX_RDONLY))

#if !defined(SVR3) || defined(SIMPLEX)

#define	SETMUSTRUN(was_running)
#define	RESTOREMUSTRUN(was_running)

#else

#define	SETMUSTRUN(was_running)		{ was_running = setmustrun(0); }
#define	RESTOREMUSTRUN(was_running)	{ restoremustrun(was_running); }

#endif

struct inode	*fhtoip();		/* returns inode held but unlocked */
struct socket	*getsock();		/* Sun/BSD returns (struct file *) */
void		svcerr_progvers();
void		rfs_dispatch();

#ifdef NFSDEBUG
extern int nfsdebug;
#endif

struct svstat svstat;

/*
 * NFS Server system call.
 * Does all of the work of running a NFS server.
 * sock is the fd of an open UDP socket.
 */
nfs_svc()
{
	register struct a {
		int     sock;
	} *uap = (struct a *) u.u_ap;
	register int was_running;

	struct inode	*rdir;
	struct inode	*cdir;
	struct socket   *so;
	SVCXPRT *xprt;
	u_long vers;
 
	so = getsock(uap->sock);
	if (so == (struct socket *) 0) {
		u.u_error = EBADF;
		return;
	}
	 
	SETMUSTRUN(was_running);

	/*
	 * Be sure that rdir (the server's root vnode) is set.
	 * Save the current directory and restore it again when
	 * the call terminates.  rfs_lookup uses u.u_cdir for lookupname.
	 */
	rdir = u.u_rdir;
	cdir = u.u_cdir;
	if (u.u_rdir == (struct inode *) 0) {
		u.u_rdir = u.u_cdir;
	}
	xprt = svckudp_create(so, NFS_PORT);
	for (vers = VERSIONMIN; vers <= VERSIONMAX; vers++) {
		(void) svc_register(xprt, NFS_PROGRAM, vers, rfs_dispatch,
		    FALSE);
	}
#ifdef SVR3
	if (setjmp(u.u_qsav)) {
#else
	if (save(u.u_qsave)) {
#endif
		for (vers = VERSIONMIN; vers <= VERSIONMAX; vers++) {
			svc_unregister(NFS_PROGRAM, vers);
		}
		SVC_DESTROY(xprt);
		RESTOREMUSTRUN(was_running);
		u.u_error = EINTR;
	} else {
		svc_run(xprt);  /* never returns */
	}
	u.u_rdir = rdir;
	u.u_cdir = cdir;
}


/*
 * Get file handle system call.
 * Takes open file descriptor and returns a file handle for it.
 */
nfs_getfh()
{
	register struct a {
		int		fdes;
		fhandle_t	*fhp;
	} *uap = (struct a *) u.u_ap;

	register struct file *fp;
	fhandle_t fh;
	struct inode *ip;
	struct file *getf();

	if (!suser()) {
		return;
	}
	fp = getf(uap->fdes);
	if (fp == NULL) {
		return;
	}
	ip = fp->f_inode;
	u.u_error = makefh(&fh, ip);
	if (!u.u_error) {
		if (copyout((caddr_t)&fh, (caddr_t)uap->fhp, sizeof(fh))) {
			u.u_error = EFAULT;
		}
	}
	return;
}

	
/*
 * These are the interface routines for the server side of the
 * Networked File System.  See the NFS protocol specification
 * for a description of this interface.
 */


/*
 * Get file attributes.
 * Returns the current attributes of the file with the given fhandle.
 */
static
rfs_getattr(fhp, ns)
	register fhandle_t *fhp;
	register struct nfsattrstat *ns;
{
	register int error;
	register struct inode *ip;

#ifdef NFSDEBUG
	dprint(nfsdebug, 4, "rfs_getattr fh %x %d\n",
	    fhp->fh_fsid, fhp->fh_fno);
#endif
	ip = fhtoip(fhp);
	if (ip == NULL) {
		ns->ns_status = NFSERR_STALE;
		return;
	}
	error = vop_getattr(ip, &ns->ns_attr);
	ns->ns_status = puterrno(error);
	iput(ip);
#ifdef NFSDEBUG
	dprint(nfsdebug, 5, "rfs_getattr: returning %d\n", error);
#endif
}

/*
 * Set file attributes.
 * Sets the attributes of the file with the given fhandle.  Returns
 * the new attributes.
 */
static
rfs_setattr(args, ns)
	register struct nfssaargs *args;
	register struct nfsattrstat *ns;
{
	register int error;
	register struct inode *ip;

#ifdef NFSDEBUG
	dprint(nfsdebug, 4, "rfs_setattr fh %x %d\n",
	    args->saa_fh.fh_fsid, args->saa_fh.fh_fno);
#endif
	ip = fhtoip(&args->saa_fh);
	if (ip == NULL) {
		ns->ns_status = NFSERR_STALE;
		return;
	}
	if (rdonly(ip)) {
		error = EROFS;
	} else {
		error = vop_setattr(ip, &args->saa_sa);
		if (!error) {
			error = vop_getattr(ip, &ns->ns_attr);
		}
	}
	ns->ns_status = puterrno(error);
	iput(ip);
#ifdef NFSDEBUG
	dprint(nfsdebug, 5, "rfs_setattr: returning %d\n", error);
#endif
}

/*
 * Directory lookup.
 * Returns a fhandle and file attributes for file name in a directory.
 */
static
rfs_lookup(da, dr)
	register struct nfsdiropargs *da;
	register struct nfsdiropres *dr;
{
	register int error;
	register struct inode *dp;
	struct nx nx;

#ifdef NFSDEBUG
	dprint(nfsdebug, 4, "rfs_lookup %s fh %x %d\n",
	    da->da_name, da->da_fhandle.fh_fsid, da->da_fhandle.fh_fno);
#endif
	dp = fhtoip(&da->da_fhandle);
	if (dp == NULL) {
		dr->dr_status = NFSERR_STALE;
		return;
	}

	/*
	 * do lookup.  XXX would be nice if there were a diropargs::da_namlen
	 */
mntloop:
	nx.dp = dp;
	nx.comp = da->da_name;
	nx.len = strlen(da->da_name);
	error = vop_namei(dp, &nx, NI_LOOKUP, NI_PASS);
	if (!error) {
#define	submnt(mp) \
	((mp)->m_flags & MEXPORTED && (mp)->m_exflags & EX_SUBMOUNT)
		register struct inode *ip;
		register struct mount *mp;

		ip = nx.ip;
		mp = dp->i_mntdev;
		if (submnt(mp)			/* sub-mount is visible */
		    && da->da_name[0] == '.'
		    && da->da_name[1] == '.'
		    && da->da_name[2] == '\0'	/* and name is ".." */
		    && nx.flags & NX_ISROOT	/* and ".." is a root */
		    && dp->i_flag & IISROOT	/* and "." is also root */
		    && mp->m_dev != rootdev) {	/* but not "/" */
			/*
			 * Get this sub-mount's mounted-on directory.
			 * If ip != dp, then dp is a filesystem root
			 * whose ".." entry is corrupted.
			 */
			if (ip != dp) {
				iput(ip);
			}
			iput(dp);
			dp = mp->m_inodp;
			IHOLD(dp);
			goto mntloop;
		}
		if (dp != ip) {
			/*
			 * Not looking up "." or an alias (e.g. ".." in
			 * a filesystem root), so release directory.
			 */
			iput(dp);
		}
		/*
		 * Check whether iget crossed a mount point.
		 */
		if (ip->i_mntdev != mp && !submnt(ip->i_mntdev)) {
			mp = ip->i_mntdev;
			iput(ip);
			ip = mp->m_inodp;
			IHOLD(ip);
		}
		error = vop_getattr(ip, &dr->dr_attr);
		if (!error) {
			error = makefh(&dr->dr_fhandle, ip);
		}
		iput(ip);
#undef submnt
	}
	dr->dr_status = puterrno(error);
#ifdef NFSDEBUG
	dprint(nfsdebug, 5, "rfs_lookup: returning %d\n", error);
#endif
}

/*
 * Read symbolic link.
 * Returns the string in the symbolic link at the given fhandle.
 */
static
rfs_readlink(fhp, rl)
	register fhandle_t *fhp;
	register struct nfsrdlnres *rl;
{
	register int error;
	register struct user *uiop = &u;
	register struct inode *ip;

#ifdef NFSDEBUG
	dprint(nfsdebug, 4, "rfs_readlink fh %x %d\n",
	    fhp->fh_fsid, fhp->fh_fno);
#endif
	ip = fhtoip(fhp);
	if (ip == NULL) {
		rl->rl_status = NFSERR_STALE;
		return;
	}

	/*
	 * Allocate data for pathname.  This will be freed by rfs_rlfree.
	 */
	rl->rl_data = (char *)kmem_alloc((u_int)NFS_MAXPATHLEN);

	/*
	 * Set up io vector to read sym link data
	 */
	uiop->u_base = rl->rl_data;
	uiop->u_count = NFS_MAXPATHLEN;
	uiop->u_segflg = 1;	/* UIOSEG_KERNEL */
	uiop->u_offset = 0;

	/*
	 * read link
	 */
	FS_READI(ip);
	error = uiop->u_error;

	/*
	 * Clean up
	 */
	if (error) {	
		kmem_free((caddr_t)rl->rl_data, (u_int)NFS_MAXPATHLEN);
		rl->rl_count = 0;
		rl->rl_data = NULL;
	} else {
		rl->rl_count = NFS_MAXPATHLEN - uiop->u_count;
	}
	rl->rl_status = puterrno(error);
	iput(ip);
#ifdef NFSDEBUG
	dprint(nfsdebug, 5, "rfs_readlink: returning '%s' %d\n",
	    rl->rl_data, error);
#endif
}

/*
 * Free data allocated by rfs_readlink
 */
static
rfs_rlfree(rl)
	struct nfsrdlnres *rl;
{
	if (rl->rl_data) {
		kmem_free((caddr_t)rl->rl_data, (u_int)NFS_MAXPATHLEN); 
	}
}

/*
 * Read data.
 * Returns some data read from the file at the given fhandle.
 */
static
rfs_read(ra, rr)
	register struct nfsreadargs *ra;
	register struct nfsrdresult *rr;
{
	register int error;
	register struct inode *ip;
	register struct user *uiop = &u;
	auto struct buf *bp;

#ifdef NFSDEBUG
	dprint(nfsdebug, 4, "rfs_read %d from fh %x %d\n",
	    ra->ra_count, ra->ra_fhandle.fh_fsid, ra->ra_fhandle.fh_fno);
#endif
	rr->rr_data = NULL;
	rr->rr_count = 0;
	ip = fhtoip(&ra->ra_fhandle);
	if (ip == NULL) {
		rr->rr_status = NFSERR_STALE;
		return;
	}
	error = vop_getattr(ip, &rr->rr_attr);
	if (error) {
		goto done;
	}
	/*
	 * This is a kludge to allow reading of files created
	 * with no read permission.  The owner of the file
	 * is always allowed to read it.
	 */
	if (u.u_uid != rr->rr_attr.na_uid) {
		if (FS_ACCESS(ip, IREAD)) {
			/*
			 * Exec is the same as read over the net because
			 * of demand loading.
			 */
			if (FS_ACCESS(ip, IEXEC)) {
				error = uiop->u_error;
			}
		}
		if (error) {
			goto done;
		}
	}

	/*
	 * Check whether we can do this with bread, which would
	 * save the copy through the uio.
	 */
	if (ra->ra_offset >= rr->rr_attr.na_size) {
		rr->rr_count = 0;
		goto done;
	}

	error = vop_bread(ip, ra->ra_offset, ra->ra_count, &bp);
	if (error) {
		printf("nfs read: failed, errno %d\n", error);
	} else if (bp != NULL) {
		rr->rr_data = bp->b_un.b_addr + uiop->u_pboff;
		rr->rr_count = uiop->u_pbsize;
#ifdef OS_ASSERT
		{
			u_int cnt =
			    MIN((u_int)(rr->rr_attr.na_size - ra->ra_offset),
				(u_int)ra->ra_count);

			if (uiop->u_pbsize != cnt) {
				iprintf("rfs_read: pbsize %d != cnt %d\n",
				    uiop->u_pbsize, cnt);
			}
		}
#endif
		rr->rr_bp = bp;
		goto done;
	}

	/*
	 * Allocate space for data.  This will be freed by xdr_rdresult
	 * when it is called with x_op = XDR_FREE.
	 */
	rr->rr_data = (char *)kmem_allocmbuf((u_int)ra->ra_count);
	rr->rr_bp = (struct buf *) 0;

	/*
	 * Set up i/o vector
	 */
	uiop->u_base = rr->rr_data;
	uiop->u_count = ra->ra_count;
	uiop->u_segflg = 1;	/* UIOSEG_KERNEL */
	uiop->u_offset = ra->ra_offset;
	/*
	 * for now we assume no append mode and ignore
	 * totcount (read ahead)
	 */
	FS_READI(ip);
	error = uiop->u_error;
	if (error) {
		kmem_freembuf((caddr_t)rr->rr_data, (u_int)ra->ra_count);
		rr->rr_data = NULL;
		rr->rr_count = 0;
		goto done;
	}
	rr->rr_count = ra->ra_count - uiop->u_count;
	/*
	 * Remember the read buffer's size so we can free below in
	 * rfs_rdfree().
	 */
	rr->rr_bsize = ra->ra_count;
done:
	rr->rr_status = puterrno(error);
	iput(ip);
#ifdef NFSDEBUG
	dprint(nfsdebug, 5, "rfs_read returning %d, count = %d\n",
	    error, rr->rr_count);
#endif
}

/*
 * Free data allocated by rfs_read.
 */
static
rfs_rdfree(rr)
	struct nfsrdresult *rr;
{
	if (rr->rr_bp == (struct buf *) 0 && rr->rr_data) {
		kmem_freembuf((caddr_t)rr->rr_data, (u_int)rr->rr_bsize);
	}
}

/*
 * Write data to file.
 * Returns attributes of a file after writing some data to it.
 */
static
rfs_write(wa, ns)
	register struct nfswriteargs *wa;
	register struct nfsattrstat *ns;
{
	register int error;
	register struct inode *ip;
	register struct user *uiop = &u;

#ifdef NFSDEBUG
	dprint(nfsdebug, 4, "rfs_write: %d bytes fh %x %d\n",
	    wa->wa_count, wa->wa_fhandle.fh_fsid, wa->wa_fhandle.fh_fno);
#endif
	ip = fhtoip(&wa->wa_fhandle);
	if (ip == NULL) {
		ns->ns_status = NFSERR_STALE;
		return;
	}
	if (rdonly(ip)) {
		error = EROFS;
	} else {
		error = vop_getattr(ip, &ns->ns_attr);
	}
	if (!error) {
		if (u.u_uid != ns->ns_attr.na_uid) {
			/*
			 * This is a kludge to allow writes of files created
			 * with read only permission.  The owner of the file
			 * is always allowed to write it.
			 */
			if (FS_ACCESS(ip, IWRITE)) {
				error = uiop->u_error;
			}
		}
		if (!error) {
			uiop->u_base = wa->wa_data;
			uiop->u_count = wa->wa_count;
			uiop->u_segflg = 1;	/* UIOSEG_KERNEL */
			uiop->u_offset = wa->wa_offset;
			/*
			 * for now we assume no append mode
			 */
			FS_WRITEI(ip);
			error = uiop->u_error;
		}
	}
	if (error) {
		printf("nfs write: failed, errno %d fh %x %d\n",
		    error, wa->wa_fhandle.fh_fsid, wa->wa_fhandle.fh_fno);
	} else {
		/*
		 * Get attributes again so we send the latest mod
		 * time to the client side for his cache.
		 */
		error = vop_getattr(ip, &ns->ns_attr);
	}
	ns->ns_status = puterrno(error);
	iput(ip);
#ifdef NFSDEBUG
	dprint(nfsdebug, 5, "rfs_write: returning %d\n", error);
#endif
}

/*
 * Create a file.
 * Creates a file with given attributes and returns those attributes
 * and an fhandle for the new file.
 */
static
rfs_create(args, dr, req)
	struct nfscreatargs *args;
	struct  nfsdiropres *dr;
	struct svc_req *req;
{
	register int error;
	register struct inode *ip;
	register struct user *uiop = &u;
	struct nx nx;
	struct argnamei nmarg;

#ifdef NFSDEBUG
	dprint(nfsdebug, 4, "rfs_create: %s dfh %x %d\n",
	    args->ca_da.da_name, args->ca_da.da_fhandle.fh_fsid,
	    args->ca_da.da_fhandle.fh_fno);
#endif
	/*
	 * XXX Should get exclusive flag and use it.
	 */
	ip = fhtoip(&args->ca_da.da_fhandle);
	if (ip == NULL) {
		dr->dr_status = NFSERR_STALE;
		return;
	}
	if (rdonly(ip)) {
		error = EROFS;
		iput(ip);
	} else {
		nx.dp = ip;
		nx.comp = args->ca_da.da_name;
		nx.len = strlen(args->ca_da.da_name);
		nmarg.cmd = NI_CREAT;
		nmarg.mode = args->ca_sa.sa_mode;
		nmarg.ftype = 0;
		nmarg.idev = NODEV;
		error = vop_namei(ip, &nx, &nmarg, NI_DONE);
		ip = error ? NULL : nx.dp;

		/*
		 * The call to svckudp_dup checks for a duplicate request.
		 */
		if (!error && nmarg.rcode == FSN_FOUND && !svckudp_dup(req)) { 
			/*
			 * If file already exists, check truncate permissions.
			 */
			FS_ACCESS(ip, IWRITE);
			error = uiop->u_error;
			if (!error && !FS_ACCESS(ip, IMNDLCK)
			    && ip->i_filocks != NULL) {
				error = uiop->u_error;
			}
			if (!error && ip->i_ftype == IFREG
			    && args->ca_sa.sa_size == 0) {
				FS_ITRUNC(ip);
				error = uiop->u_error;
			}
		}
		if (!error) {
			error = vop_getattr(ip, &dr->dr_attr);
			if (!error) {
				error = makefh(&dr->dr_fhandle, ip);
			}
		}
		if (ip != NULL) {
			iput(ip);
		}
	}
	dr->dr_status = puterrno(error);
	if (!error) {
		svckudp_dupsave(req);
	}
#ifdef NFSDEBUG
	dprint(nfsdebug, 5, "rfs_create: returning %d\n", error);
#endif
}

/*
 * Remove a file.
 * Remove named file from parent directory.
 */
static
rfs_remove(da, status, req)
	register struct nfsdiropargs *da;
	enum nfsstat *status;
	struct svc_req *req;
{
	register int error;
	register struct inode *ip;
	struct nx nx;
	struct argnamei nmarg;

#ifdef NFSDEBUG
	dprint(nfsdebug, 4, "rfs_remove %s dfh %x %d\n",
	    da->da_name, da->da_fhandle.fh_fsid, da->da_fhandle.fh_fno);
#endif
	ip = fhtoip(&da->da_fhandle);
	if (ip == NULL) {
		*status = NFSERR_STALE;
		return;
	}
	if (rdonly(ip)) {
		error = EROFS;
		iput(ip);
	} else {
		nx.dp = ip;
		nx.comp = da->da_name;
		nx.len = strlen(da->da_name);
		nmarg.cmd = NI_DEL;
		error = vop_namei(ip, &nx, &nmarg, NI_NULL);
	}
	if (error == ENOENT) {
		/*
		 * check for dup request
		 */
		if (svckudp_dup(req)) { 
			error = 0;
		}
	}
	*status = puterrno(error);
	if (!error) {
		svckudp_dupsave(req);
	}
#ifdef NFSDEBUG
	dprint(nfsdebug, 5, "rfs_remove: %s returning %d\n",
	    da->da_name, error);
#endif
}

/*
 * rename a file
 * Give a file (from) a new name (to).
 */
static
rfs_rename(args, status)
	register struct nfsrnmargs *args;
	enum nfsstat *status; 
{
	register int error;
	register struct inode *fromdp;
	register struct inode *fromip;
	register struct inode *todp;
	struct nx nx;
	struct argnamei nmarg;

#ifdef NFSDEBUG
	dprint(nfsdebug, 4, "rfs_rename %s ffh %x %d -> %s tfh %x %d\n",
	    args->rna_from.da_name,
	    args->rna_from.da_fhandle.fh_fsid,
	    args->rna_from.da_fhandle.fh_fno,
	    args->rna_to.da_name,
	    args->rna_to.da_fhandle.fh_fsid,
	    args->rna_to.da_fhandle.fh_fno);
#endif
	fromdp = fhtoip(&args->rna_from.da_fhandle);
	if (fromdp == NULL) {
		*status = NFSERR_STALE;
		return;
	}
	iunlock(fromdp);
	todp = fhtoip(&args->rna_to.da_fhandle);
	if (todp == NULL) {
		*status = NFSERR_STALE;
		iunuse(fromdp);
		return;
	}
	if (rdonly(fromdp) || rdonly(todp)) {
		error = EROFS;
		iunuse(fromdp);
		iput(todp);
	} else {
		/*
		 * Get source ("from") inode -- FS_NAMEI design botch.
		 */
		iunlock(todp);
		ilock(fromdp);
		nx.dp = fromdp;
		nx.comp = args->rna_from.da_name;
		nx.len = strlen(args->rna_from.da_name);
		error = vop_namei(fromdp, &nx, NI_LOOKUP, NI_PASS);
		if (error) {
			/* release all inodes held after vop_namei() */
			iunuse(todp);
		} else {
			/*
			 * Unlock the source inodes, re-lock the target
			 * directory, and do the rename.
			 */
			iunlock(fromdp);
			fromip = nx.ip;
			if (fromip == fromdp) {
				iuse(fromip);	/* need an extra reference */
			} else {
				iunlock(fromip);
			}
			ilock(todp);
			nmarg.cmd = NI_RENAME;
			nmarg.dp = fromdp;
			nmarg.ip = fromip;
			nmarg.name = args->rna_from.da_name;
			nmarg.namlen = nx.len;
			nx.dp = todp;
			nx.comp = args->rna_to.da_name;
			nx.len = strlen(args->rna_to.da_name);
			error = vop_namei(todp, &nx, &nmarg, NI_NULL);
			iunuse(fromip);
			iunuse(fromdp);
		}
	}
	*status = puterrno(error); 
#ifdef NFSDEBUG
	dprint(nfsdebug, 5, "rfs_rename: returning %d\n", error);
#endif
}

/*
 * Link to a file.
 * Create a file (to) which is a hard link to the given file (from).
 */
static
rfs_link(args, status, req) 
	register struct nfslinkargs *args;
	enum nfsstat *status;  
	struct svc_req *req;
{
	register int error;
	register struct inode *fromip;
	register struct inode *toip;
	struct nx nx;
	struct argnamei nmarg;

#ifdef NFSDEBUG
	dprint(nfsdebug, 4, "rfs_link ffh %x %d -> %s tfh %x %d\n",
	    args->la_from.fh_fsid, args->la_from.fh_fno,
	    args->la_to.da_name,
	    args->la_to.da_fhandle.fh_fsid, args->la_to.da_fhandle.fh_fno);
#endif
	fromip = fhtoip(&args->la_from);
	if (fromip == NULL) {
		*status = NFSERR_STALE;
		return;
	}
	iunlock(fromip);
	toip = fhtoip(&args->la_to.da_fhandle);
	if (toip == NULL) {
		*status = NFSERR_STALE;
		iunuse(fromip);
		return;
	}
	if (rdonly(toip)) {
		error = EROFS;
		iput(toip);
	} else {
		nx.dp = toip;
		nx.comp = args->la_to.da_name;
		nx.len = strlen(args->la_to.da_name);
		nmarg.cmd = NI_LINK;
		nmarg.ip = fromip;
		error = vop_namei(toip, &nx, &nmarg, NI_NULL);
		if (error == EEXIST) {
			/*
			 * check for dup request
			 */
			if (svckudp_dup(req)) { 
				error = 0;
			}
		}
	}
	*status = puterrno(error);
	iunuse(fromip);
	if (!error) {
		svckudp_dupsave(req);
	}
#ifdef NFSDEBUG
	dprint(nfsdebug, 5, "rfs_link: returning %d\n", error);
#endif
}
 
/*
 * Symbolicly link to a file.
 * Create a file (to) with the given attributes which is a symbolic link
 * to the given path name (to).
 */
static
rfs_symlink(args, status) 
	register struct nfsslargs *args;
	enum nfsstat *status;   
{		  
	register int error;
	register struct inode *ip;
	struct nx nx;
	struct argnamei nmarg;

#ifdef NFSDEBUG
	dprint(nfsdebug, 4, "rfs_symlink %s ffh %x %d -> %s\n",
	    args->sla_from.da_name,
	    args->sla_from.da_fhandle.fh_fsid,
	    args->sla_from.da_fhandle.fh_fno,
	    args->sla_tnm);
#endif
	ip = fhtoip(&args->sla_from.da_fhandle);
	if (ip == NULL) {
		*status = NFSERR_STALE;
		return;
	}
	if (rdonly(ip)) {
		error = EROFS;
		iput(ip);
	} else {
		nx.dp = ip;
		nx.comp = args->sla_from.da_name;
		nx.len = strlen(args->sla_from.da_name);
		nmarg.cmd = NI_SYMLINK;
		nmarg.mode = 0;
		nmarg.name = args->sla_tnm;
		nmarg.namlen = strlen(args->sla_tnm);
		nmarg.namseg = 1;	/* UIOSEG_KERNEL */
		error = vop_namei(ip, &nx, &nmarg, NI_NULL);
	}
	*status = puterrno(error);
#ifdef NFSDEBUG
	dprint(nfsdebug, 5, "rfs_symlink: returning %d\n", error);
#endif
}
  
/*
 * Make a directory.
 * Create a directory with the given name, parent directory, and attributes.
 * Returns a file handle and attributes for the new directory.
 */
static
rfs_mkdir(args, dr, req)
	register struct nfscreatargs *args;
	struct nfsdiropres *dr;
	struct svc_req *req;
{
	register int error;
	register struct inode *ip;
	struct nx nx;
	struct argnamei nmarg;

#ifdef NFSDEBUG
	dprint(nfsdebug, 4, "rfs_mkdir %s fh %x %d\n",
	    args->ca_da.da_name, args->ca_da.da_fhandle.fh_fsid,
	    args->ca_da.da_fhandle.fh_fno);
#endif
	/*
	 * Should get exclusive flag and pass it on here
	 */
	ip = fhtoip(&args->ca_da.da_fhandle);
	if (ip == NULL) {
		dr->dr_status = NFSERR_STALE;
		return;
	}
	if (rdonly(ip)) {
		error = EROFS;
		iput(ip);
	} else {
		nx.dp = ip;
		nx.comp = args->ca_da.da_name;
		nx.len = strlen(args->ca_da.da_name);
		nmarg.cmd = NI_MKDIR;
		nmarg.mode = args->ca_sa.sa_mode;
		iuse(ip);	/* FS_NAMEI design botch */
		error = vop_namei(ip, &nx, &nmarg, NI_NULL);
		/*
		 * If directory already exists, check for dup request.
		 * Otherwise release the reference added above (iuse)
		 * to ip, as FS_NAMEI has already dropped the original
		 * reference, added by fhtoip().
		 */
		if (error) {
			if (error == EEXIST && svckudp_dup(req)) {
				error = 0;
			} else {
				iunuse(ip);
			}
		}
	}
	if (!error) {
		/*
		 * The new directory's parent, ip, is held used but
		 * unlocked.  Look up the new dir in it in order to get
		 * its attributes and make an fhandle for it.  As above
		 * if FS_NAMEI has a lookup error, it releases ip.
		 */
		ilock(ip);
		nx.dp = ip;
		ASSERT(nx.comp == args->ca_da.da_name);
		error = vop_namei(ip, &nx, NI_LOOKUP, NI_PASS);
		if (!error) {
			register struct mount *mp;

			mp = ip->i_mntdev;
			iput(ip);
			ip = nx.ip;
			ASSERT(ip->i_ftype == IFDIR);
			error = vop_getattr(ip, &dr->dr_attr);
			if (!error) {
				error = makefh(&dr->dr_fhandle, ip);
			}
			iput(ip);
		}
	}
	dr->dr_status = puterrno(error);
	if (!error) {
		svckudp_dupsave(req);
	}
#ifdef NFSDEBUG
	dprint(nfsdebug, 5, "rfs_mkdir: returning %d\n", error);
#endif
}

/*
 * Remove a directory.
 * Remove the given directory name from the given parent directory.
 */
static
rfs_rmdir(da, status, req)
	register struct nfsdiropargs *da;
	enum nfsstat *status;
	struct svc_req *req;
{
	register int error;
	register struct inode *ip;
	struct nx nx;
	struct argnamei nmarg;

#ifdef NFSDEBUG
	dprint(nfsdebug, 4, "rfs_rmdir %s fh %x %d\n",
	    da->da_name, da->da_fhandle.fh_fsid, da->da_fhandle.fh_fno);
#endif
	ip = fhtoip(&da->da_fhandle);
	if (ip == NULL) {
		*status = NFSERR_STALE;
		return;
	}
	if (rdonly(ip)) {
		error = EROFS;
		iput(ip);
	} else {
		nx.dp = ip;
		nx.comp = da->da_name;
		nx.len = strlen(da->da_name);
		nmarg.cmd = NI_RMDIR;
		error = vop_namei(ip, &nx, &nmarg, NI_NULL);
		if (error == ENOENT) {
			/*
			 * check for dup request
			 */
			if (svckudp_dup(req)) { 
				error = 0;
			}
		}
	}
	*status = puterrno(error);
	if (!error) {
		svckudp_dupsave(req);
	}
#ifdef NFSDEBUG
	dprint(nfsdebug, 5, "nfs_rmdir returning %d\n", error);
#endif
}

static
rfs_readdir(rda, rd)
	struct nfsrddirargs *rda;
	register struct nfsrddirres *rd;
{
	register struct user *uiop = &u;
	register int nbytes;
	register struct inode *ip;
	register int error = 0;

#ifdef NFSDEBUG
	dprint(nfsdebug, 4, "rfs_readdir fh %x %d count %d\n",
	    rda->rda_fh.fh_fsid, rda->rda_fh.fh_fno, rda->rda_count);
#endif
	ip = fhtoip(&rda->rda_fh);
	if (ip == NULL) {
		rd->rd_status = NFSERR_STALE;
		return;
	}
#ifdef NOTDEF	/* XXX this check doesn't make sense, nor is it SVID */
	/*
	 * check cd access to dir.  we have to do this here because
	 * the opendir doesn't go over the wire.
	 */
	if (FS_ACCESS(ip, ICDEXEC)) {
		error = uiop->u_error;
		goto bad;
	}
#endif

	/*
	 * Allocate data for entries.  This will be freed by rfs_rdfree.
	 */
	if (rda->rda_count >= BBSIZE) {
		rd->rd_bp = geteblk(BTOBB(rda->rda_count));
		rd->rd_entries = (struct dirent *)rd->rd_bp->b_un.b_addr;
	} else {
		rd->rd_bp = (struct buf *) 0;
		rd->rd_entries =
		    (struct dirent *)kmem_alloc((u_int)rda->rda_count);
	}
	rd->rd_bufsize = rda->rda_count;
	rd->rd_offset = rda->rda_offset;

	/*
	 * Set up io vector to read directory data into kmem
	 */
	uiop->u_segflg = 1;	/* UIOSEG_KERNEL */
	uiop->u_offset = rd->rd_offset;

	/*
	 * read directory
	 */
	nbytes = FS_GETDENTS(ip, (caddr_t)rd->rd_entries, rda->rda_count);
	if (nbytes < 0) {
		rd->rd_size = 0;
		error = uiop->u_error;
	} else {
		rd->rd_size = nbytes;
		rd->rd_eof = (nbytes == 0);
		rd->rd_offset = uiop->u_offset;
	}

#ifdef NOTDEF
bad:
#endif
	rd->rd_status = puterrno(error);
	iput(ip);

#ifdef NFSDEBUG
	dprint(nfsdebug, 5, "rfs_readdir: returning %d\n", error);
#endif
}

static
rfs_rddirfree(rd)
	struct nfsrddirres *rd;
{

	if (rd->rd_bp) {
		ASSERT(rd->rd_entries && rd->rd_bufsize >= BBSIZE);
		brelse(rd->rd_bp);
	} else {
		kmem_free((caddr_t)rd->rd_entries, (u_int)rd->rd_bufsize);
	}
}

static
rfs_statfs(fh, fs)
	fhandle_t *fh;
	register struct nfsstatfs *fs;
{
	register int error;
	register struct inode *ip;
	struct statfs sb;

#ifdef NFSDEBUG
	dprint(nfsdebug, 4, "rfs_statfs fh %x %d\n", fh->fh_fsid, fh->fh_fno);
#endif
	ip = fhtoip(fh);
	if (ip == NULL) {
		fs->fs_status = NFSERR_STALE;
		return;
	}
	FS_STATFS(ip, &sb, 0);
	error = u.u_error;
	fs->fs_status = puterrno(error);
	if (!error) {
		fs->fs_tsize = nfstsize();
		fs->fs_bsize = sb.f_bsize;
		fs->fs_blocks = BBTOB(sb.f_blocks) / sb.f_bsize;
		fs->fs_bfree = BBTOB(sb.f_bfree) / sb.f_bsize;
		fs->fs_bavail = fs->fs_blocks - fs->fs_bfree;
	}
	iput(ip);
#ifdef NFSDEBUG
	dprint(nfsdebug, 5, "rfs_statfs returning %d\n", error);
#endif
}

/*ARGSUSED*/
static
rfs_null(argp, resp)
	caddr_t argp;
	caddr_t resp;
{
	/* do nothing */
	return (0);
}

/*ARGSUSED*/
static
rfs_error(argp, resp)
	caddr_t argp;
	caddr_t resp;
{
	return (EOPNOTSUPP);
}

int
nullfree()
{
}

/*
 * rfs dispatch table
 * Indexed by version,proc
 */

struct rfsdisp {
	int	  (*dis_proc)();	/* proc to call */
	xdrproc_t dis_xdrargs;		/* xdr routine to get args */
	int	  dis_argsz;		/* sizeof args */
	xdrproc_t dis_xdrres;		/* xdr routine to put results */
	int	  dis_ressz;		/* size of results */
	int	  (*dis_resfree)();	/* frees space allocated by proc */
} rfsdisptab[][RFS_NPROC]  = {
	{
	/*
	 * VERSION 2
	 * Changed rddirres to have eof at end instead of beginning
	 */
	/* RFS_NULL = 0 */
	{rfs_null, xdr_void, 0,
	    xdr_void, 0, nullfree},
	/* RFS_GETATTR = 1 */
	{rfs_getattr, xdr_fhandle, sizeof(fhandle_t),
	    xdr_attrstat, sizeof(struct nfsattrstat), nullfree},
	/* RFS_SETATTR = 2 */
	{rfs_setattr, xdr_saargs, sizeof(struct nfssaargs),
	    xdr_attrstat, sizeof(struct nfsattrstat), nullfree},
	/* RFS_ROOT = 3 *** NO LONGER SUPPORTED *** */
	{rfs_error, xdr_void, 0,
	    xdr_void, 0, nullfree},
	/* RFS_LOOKUP = 4 */
	{rfs_lookup, xdr_diropargs, sizeof(struct nfsdiropargs),
	    xdr_diropres, sizeof(struct nfsdiropres), nullfree},
	/* RFS_READLINK = 5 */
	{rfs_readlink, xdr_fhandle, sizeof(fhandle_t),
	    xdr_rdlnres, sizeof(struct nfsrdlnres), rfs_rlfree},
	/* RFS_READ = 6 */
	{rfs_read, xdr_readargs, sizeof(struct nfsreadargs),
	    xdr_rdresult, sizeof(struct nfsrdresult), rfs_rdfree},
	/* RFS_WRITECACHE = 7 *** NO LONGER SUPPORTED *** */
	{rfs_error, xdr_void, 0,
	    xdr_void, 0, nullfree},
	/* RFS_WRITE = 8 */
	{rfs_write, xdr_writeargs, sizeof(struct nfswriteargs),
	    xdr_attrstat, sizeof(struct nfsattrstat), nullfree},
	/* RFS_CREATE = 9 */
	{rfs_create, xdr_creatargs, sizeof(struct nfscreatargs),
	    xdr_diropres, sizeof(struct nfsdiropres), nullfree},
	/* RFS_REMOVE = 10 */
	{rfs_remove, xdr_diropargs, sizeof(struct nfsdiropargs), 
	    xdr_enum, sizeof(enum nfsstat), nullfree},
	/* RFS_RENAME = 11 */
	{rfs_rename, xdr_rnmargs, sizeof(struct nfsrnmargs), 
	    xdr_enum, sizeof(enum nfsstat), nullfree},
	/* RFS_LINK = 12 */
	{rfs_link, xdr_linkargs, sizeof(struct nfslinkargs), 
	    xdr_enum, sizeof(enum nfsstat), nullfree},
	/* RFS_SYMLINK = 13 */
	{rfs_symlink, xdr_slargs, sizeof(struct nfsslargs), 
	    xdr_enum, sizeof(enum nfsstat), nullfree},
	/* RFS_MKDIR = 14 */
	{rfs_mkdir, xdr_creatargs, sizeof(struct nfscreatargs),
	    xdr_diropres, sizeof(struct nfsdiropres), nullfree},
	/* RFS_RMDIR = 15 */
	{rfs_rmdir, xdr_diropargs, sizeof(struct nfsdiropargs), 
	    xdr_enum, sizeof(enum nfsstat), nullfree},
	/* RFS_READDIR = 16 */
	{rfs_readdir, xdr_rddirargs, sizeof(struct nfsrddirargs),
	    xdr_putrddirres, sizeof(struct nfsrddirres), rfs_rddirfree},
	/* RFS_STATFS = 17 */
	{rfs_statfs, xdr_fhandle, sizeof(fhandle_t),
	    xdr_statfs, sizeof(struct nfsstatfs), nullfree},
	}
};

struct rfsspace {
	struct rfsspace *rs_next;
	caddr_t		rs_dummy;
};

static struct rfsspace	*rfsfreesp = NULL;
static u_int		rfssize = 0;

caddr_t
rfsget()
{
	register int i;
	register struct rfsdisp *dis;
	register caddr_t ret;

	if (rfssize == 0) {
		for (i = 0; i < 1 + VERSIONMAX - VERSIONMIN; i++) {
			for (dis = &rfsdisptab[i][0];
			     dis < &rfsdisptab[i][RFS_NPROC];
			     dis++) {
				rfssize = MAX(rfssize, dis->dis_argsz);
				rfssize = MAX(rfssize, dis->dis_ressz);
			}
		}
		ASSERT(rfssize >= sizeof(struct rfsspace));
	}

	if (rfsfreesp) {
		ret = (caddr_t)rfsfreesp;
		rfsfreesp = rfsfreesp->rs_next;
	} else {
		ret = kmem_alloc((u_int)rfssize);
	}
	return (ret);
}

rfsput(rs)
	struct rfsspace *rs;
{
	rs->rs_next = rfsfreesp;
	rfsfreesp = rs;
}

#ifdef SVR3
extern int	nobody;
extern int	nfs_portmon;
#else
int nobody = -2;

/*
 * If nfs_portmon is set, then clients are required to use
 * privileged ports (ports < IPPORT_RESERVED) in order to get NFS services.
 */
int nfs_portmon = 0;
#endif

void
rfs_dispatch(req, xprt)
	register struct svc_req *req;
	register SVCXPRT *xprt;
{
	register int which;
	register int vers;
	caddr_t args = NULL;
	caddr_t res = NULL;
	register struct rfsdisp *disp;
	struct ucred tmpcr;
	struct ucred *newcr = NULL;
	register int error;

#ifdef SVR3
	ASSERT(private.p_cpuid == 0);
#endif
	svstat.ncalls++;
	error = 0;
	which = req->rq_proc;	/* XXX validate domain */
	if (which < 0 || which >= RFS_NPROC) {
#ifdef NFSDEBUG
		dprint(nfsdebug, 2,
		    "rfs_dispatch: bad proc %d\n", which);
#endif
		svcerr_noproc(req->rq_xprt);
		error++;
		goto done;
	}
	vers = req->rq_vers;
	if (vers < VERSIONMIN || vers > VERSIONMAX) {
#ifdef NFSDEBUG
		dprint(nfsdebug, 2,
		    "rfs_dispatch: bad vers %d low %d high %d\n",
		    vers, VERSIONMIN, VERSIONMAX);
#endif
		svcerr_progvers(req->rq_xprt, (u_long)VERSIONMIN,
		    (u_long)VERSIONMAX);
		error++;
		goto done;
	}
	vers -= VERSIONMIN;
	disp = &rfsdisptab[vers][which];

	/*
	 * Clean up as if a system call just started
	 */
	u.u_error = 0;

	/*
	 * Allocate args struct and deserialize into it.
	 */
	args = rfsget();
	bzero(args, rfssize);
	if (! SVC_GETARGS(xprt, disp->dis_xdrargs, args)) {
		svcerr_decode(xprt);
		error++;
		goto done;
	}

	/*
	 * Check for unix style credentials
	 */
	if (req->rq_cred.oa_flavor != AUTH_UNIX && which != RFS_NULL) {
		svcerr_weakauth(xprt);
		error++;
		goto done;
	}

	if (nfs_portmon) {
		/*
		* Check for privileged port number
		*/
		static int count = 0;

		if (ntohs(xprt->xp_raddr.sin_port) >= IPPORT_RESERVED) {
			svcerr_weakauth(xprt);
			if (count == 0) {
				printf("NFS request from unprivileged port, ");
				printf("source IP address = %x, port %d\n",
					xprt->xp_raddr.sin_addr.s_addr,
					xprt->xp_raddr.sin_port);
			}
			count++;
			count %= 256;
			error++;
			goto done;
		}
	}

	/*
	 * Set uid, gid, and gids to auth params
	 */
	if (which != RFS_NULL) {
		register struct authunix_parms *aup;
		register int *gp;

		aup = (struct authunix_parms *)req->rq_clntcred;
		newcr = crget();
		if (aup->aup_uid == 0) {
			/*
			 * root over the net becomes other on the server
			 * (uid -2)
			 */
			newcr->cr_uid = nobody;
		} else {
			newcr->cr_uid = aup->aup_uid;
		}
		newcr->cr_gid = aup->aup_gid;
		if (aup->aup_len > NGROUPS) {
			aup->aup_len = NGROUPS;
		}
		bcopy((caddr_t)aup->aup_gids, (caddr_t)newcr->cr_groups,
		    aup->aup_len * sizeof(newcr->cr_groups[0]));
		for (gp = &newcr->cr_groups[aup->aup_len];
		     gp < &newcr->cr_groups[NGROUPS];
		     gp++) {
			*gp = NOGROUP;
		}
		crinit(&u, &tmpcr);
		cruse(newcr, &u);
	}

	/*
	 * Allocate results struct.
	 */
	res = rfsget();
	bzero((caddr_t)res, rfssize);

	svstat.reqs[which]++;

	/*
	 * Call service routine with arg and results structs and the
	 * request pointer
	 */
	(*disp->dis_proc)(args, res, req);

done:
	if (args != NULL) {
		/*
		 * Free arguments struct
		 */
		if (! SVC_FREEARGS(xprt, disp->dis_xdrargs, args)) {
#ifdef NFSDEBUG
			dprint(nfsdebug, 1, "nfs: can't free arguments\n");
#endif
			error++;
		}
		rfsput((struct rfsspace *) args);
	}
	if (res != NULL) {
		/*
		 * Serialize results struct
		 */
		if (!error && !svc_sendreply(xprt, disp->dis_xdrres, res)) {
#ifdef NFSDEBUG
			dprint(nfsdebug, 1, "nfs: can't encode reply\n");
#endif
			error++;
		}

		/*
		 * Free results struct
		 */
		if (disp->dis_resfree != nullfree) {
			(*disp->dis_resfree)(res);
		}
		rfsput((struct rfsspace *) res);
	}

	/*
	 * restore original credentials
	 */
	if (newcr) {
		cruse(&tmpcr, &u);
		crfree(newcr);
	}
	svstat.nbadcalls += error;
}
