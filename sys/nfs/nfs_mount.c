#ifndef lint
static	char	rcsid[] = "$Header: /d2/3.7/src/sys/nfs/RCS/nfs_mount.c,v 1.1 89/03/27 17:33:13 root Exp $";
#endif
/*
 * NFS mount and vfs-level operations.
 *
 * $Source: /d2/3.7/src/sys/nfs/RCS/nfs_mount.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:33:13 $
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
# include "sys/inode.h"
# include "sys/mount.h"
# define m_fs	m_bufp		/* ad-hoc compatibility */
# include "sys/nami.h"
# include "sys/statfs.h"
# include "sys/socket.h"
# include "netinet/in.h"
# include "sys/fs/nfs.h"
# include "sys/fs/nfs_clnt.h"
# include "sys/fs/nfs_export.h"
# include "sys/fstyp.h"
# include "sys/fs/com_pncc.h"
# include "sys/fs/rnode.h"
#else
# include "../h/param.h"
# include "../h/systm.h"
# include "../h/user.h"
# include "../h/file.h"
# include "../h/fs.h"		/* bio parameters */
# define bio_maxbsize	efs_lbsize
# include "../h/inode.h"
# include "../h/mount.h"
# include "../h/nami.h"
# include "../h/statfs.h"
# include "../h/socket.h"
# include "../netinet/in.h"
# include "../nfs/nfs.h"
# include "../nfs/nfs_clnt.h"
# include "../nfs/nfs_export.h"
# include "../nfs/rnode.h"
#endif

int nfs_mntno = 0;		/* fake minor dev number */

/*
 * NFS mount system call.
 *
 * This guy is a bit tricky.  Here is what we do:
 * 1) lookup the directory to be covered (uap->localdir).
 * 2) create an rpc conection to the server at uap->argp->addr.
 * 3) get the root inode of the server and make it the current dir.
 * 4) look up the inode of the served directory (uap->argp->fh).
 * 5) restore the original current directory.
 * 6) release the server root inode
 *
 * The mess about looking up the served directory starting at the
 * current directory (which is the server's root) allows the client
 * to mount a sub-tree of the server's filesystem.
 */
int
nfs_mount()
{
	struct a {
		struct nfs_args	*argp;
		char		*localdir;
		int		readonly;
	} *uap = (struct a *) u.u_ap;
	struct nfs_args args;		/* nfs-specific arguments */
	register struct nfs_args *ap;

	struct inode *hidvp = NULL;	/* local inode to be hidden */
	struct mntinfo *mi = NULL;	/* mount info, pointed at by mp */
	struct inode *rootvp = NULL;	/* the server's root */
	struct mount *mp = NULL;	/* mount structure (vfs) */
	struct nfsfattr na;		/* root inode attributes in nfs form */
	struct statfs sb;		/* server's file system stats */
	fhandle_t fh;			/* root fhandle */
	struct ucred cred;		/* user's credentials */
#ifdef SVR3
	struct mount *mount_alloc();
#endif

	/*
	 * Must be super user
	 */
	if (!suser()) {
		return;
	}

	/*
	 * Get inode to be covered.  Return immediately if error.  After
	 * this paragraph, all errors return by going to out.
	 */
	u.u_dirp = uap->localdir;
	hidvp = namei(USERPATH, NI_LOOKUP, FOLLOWLINK);
	if (hidvp == NULL) {
		return;
	}
	if (hidvp->i_ftype != IFDIR) {
		u.u_error = ENOTDIR;
		iput(hidvp);
		return;
	}

	/*
	 * Copyin nfs mount arguments.
	 */
	ap = &args;
	if (copyin((caddr_t)uap->argp, (caddr_t)ap, sizeof(*ap))) {
		u.u_error = EFAULT;
		goto out;
	}

	/*
	 * Create a mount record, and initialize all of its members
	 * except true block size.
	 */
	mi = (struct mntinfo *)kmem_alloc((u_int)sizeof(*mi));
	mi->mi_bsize = BLKDEV_IOSIZE;	/* lie temporarily */
	mi->mi_refct = 0;
	mi->mi_stsize = 0;	/* set below by nfs_statfs */
	mi->mi_hard = (ap->flags & NFSMNT_SOFT) == 0;
	if (ap->flags & NFSMNT_RETRANS) {
		mi->mi_retrans = ap->retrans;
		if (ap->retrans < 0) {
			u.u_error = EINVAL;
			goto out;
		}
	} else {
		mi->mi_retrans = NFS_RETRIES;
	}
	if (ap->flags & NFSMNT_TIMEO) {
		mi->mi_timeo = ap->timeo;
		if (ap->timeo <= 0) {
			u.u_error = EINVAL;
			goto out;
		}
	} else {
		mi->mi_timeo = NFS_TIMEO;
	}
	mi->mi_mntno = nfs_mntno;
	mi->mi_printed = 0;
	if (copyin((caddr_t)ap->addr, (caddr_t)&mi->mi_addr,
	    sizeof(mi->mi_addr))) {
		u.u_error = EFAULT;
		goto out;
	}
	/*
	 * For now we support just AF_INET
	 */
	if (mi->mi_addr.sin_family != AF_INET) {
		u.u_error = EPFNOSUPPORT;
		goto out;
	}
	if (ap->flags & NFSMNT_HOSTNAME) {
#ifdef SVR3
		switch (upath((caddr_t)ap->hostname,
		    (caddr_t)mi->mi_hostname, sizeof(mi->mi_hostname))) {
		case -2:	/* hostname overlong */
		case 0:		/* hostname empty */
			u.u_error = EINVAL;
			goto out;
		case -1:	/* bad address */
			u.u_error = EFAULT;
			goto out;
		}
#else
		if (fustring((caddr_t)ap->hostname, (caddr_t)mi->mi_hostname,
		    sizeof(mi->mi_hostname)) >= 0) {
			goto out;
		}
#endif
	} else {
		addr_to_str(&mi->mi_addr, mi->mi_hostname);
	}

	/*
	 * Allocate a mount structure and install the mount record in it.
	 * Initialize all of mp's members except the blocksize, root pointer,
	 * covered inode pointer, and next link fields.  We initialize mp
	 * partially here instead of below because rootvp needs mntinfo
	 * before we can do a getattr on it.
	 */
#ifdef SVR3
	mp = mount_alloc();
	if (mp == NULL) {
		u.u_error = EBUSY;	/* see os/sys3.c:smount() */
		return;
	}
	if (uap->readonly) {
		mp->m_flags |= MRDONLY;
	}
#else
	mp = (struct mount *) kmem_alloc(sizeof *mp);
	mp->m_flags = uap->readonly ? MRDONLY : 0;
#endif
	mp->m_dev = makedev(nfs_major, nfs_mntno);
	mp->m_fstyp = nfs_fstyp;
	mp->m_fs = (char *) mi;
	mp->m_exflags = 0;

	/*
	 * Make the root inode with null NFDIR attributes
	 */
	bzero((caddr_t) &na, sizeof na);
	na.na_type = NFDIR;
	if (copyin((caddr_t)ap->fh, (caddr_t)&fh, sizeof(fh))) {
		u.u_error = EFAULT;
		goto out;
	}
	u.u_error = nfs_igetbyfh(mp, &fh, &na, &rootvp);
	if (u.u_error) {
		goto out;	/* out of inodes or rnodes */
	}
	if (rootvp->i_flag & IISROOT) {
		u.u_error = EBUSY;
		goto out;
	}

	/*
	 * Get full attributes of the root inode
	 */
	crinit(&u, &cred);
	vtor(rootvp)->r_nfsattrtime = 0;
	if (time < 0) {
		printf("nfs_mount: fix system time with date(1)\n");
		u.u_error = EINVAL;
		goto out;
	}
	u.u_error = nfs_getattr(rootvp, &cred);
	if (u.u_error) {
		goto out;
	}
	rootvp->i_flag |= IISROOT;
	mi->mi_rootvp = rootvp;

	/*
	 * This call to nfs_statfs() sets mi_stsize, the server's
	 * maximum transfer size in bytes.
	 */
	nfs_statfs(rootvp, &sb, (short) 0);
	if (u.u_error) {
		goto out;
	}
	if (ap->flags & NFSMNT_WSIZE) {
		if (ap->wsize <= 0) {
			u.u_error = EINVAL;
			goto out;
		}
		mi->mi_stsize = MIN(mi->mi_stsize, ap->wsize);
	}

	/*
	 * Set filesystem block size to the smallest of: NFS_MAXDATA,
	 * server's filesystem block size, this client's transfer size,
	 * and maximum bio buffer size (for our system, bio_maxbsize).
	 * Set the mount structure's block size field.
	 */
	mi->mi_tsize = MIN(nfstsize(), NFS_MAXDATA);
	if (ap->flags & NFSMNT_RSIZE) {
		if (ap->rsize <= 0) {
			u.u_error = EINVAL;
			goto out;
		}
		mi->mi_tsize = MIN(mi->mi_tsize, ap->rsize);
	}
#ifdef NFSDEBUG
	dprint(nfsdebug, 1,
	    "nfs_mount: hard %d timeo %d retries %d wsize %d rsize %d\n",
	    mi->mi_hard, mi->mi_timeo, mi->mi_retrans, mi->mi_stsize,
	    mi->mi_tsize);
#endif
	/*
	 * Should set read-only here!
	 */

	/*
	 * Set the client's idea of the server's block size to at most
	 * the local bio's maximum buffer size and at least the
	 * bio's basic block size.
	 */
	mi->mi_bsize = vtor(rootvp)->r_nfsattr.na_blocksize;
	ASSERT(mi->mi_bsize);
	if (mi->mi_bsize % BBSIZE != 0) {
		mi->mi_bsize = BBSIZE;
	} else if (bio_maxbsize < mi->mi_bsize) {
		mi->mi_bsize = bio_maxbsize;
	}
	mp->m_bsize = mi->mi_bsize;

	/*
	 * Set the mount structure's root pointer and link it into the
	 * mount list, setting the covered inode as its ``mounted-on''
	 * pointer.  Bump nfs_mntno - the mount has succeeded.
	 */
	mp->m_mount = rootvp;
	iunlock(rootvp);
#ifdef SVR3
	hidvp->i_flag |= IMOUNT;
	hidvp->i_mnton = mp;
	iuse(hidvp);
	mp->m_inodp = hidvp;
#else
	mount_insert(mp, hidvp);
#endif
	nfs_mntno++;
	ASSERT(u.u_error == 0);

#ifdef NFSDEBUG
        dprint(nfsdebug, 10, "mp %x: inodecov = %x, data = %x\n",
            mp, mp->m_inodp, mp->m_fs);
        dprint(nfsdebug, 10, "hidvp %x: mnton %x mntdev %x\n",
            hidvp, hidvp->i_mnton, hidvp->i_mntdev);
        dprint(nfsdebug, 10, "rootvp %x: mnton %x mntdev %x\n",
            rootvp, rootvp->i_mnton, rootvp->i_mntdev);
#endif

out:
	/*
	 * Cleanup on error.  Release the hidden inode (if this mount
	 * succeeded, hidvp will be held unlocked after the iput() as
	 * mp->m_inodp).
	 */
	if (u.u_error) {
		if (rootvp) {
#ifdef SVR3
			ifreebusy(rootvp);
#else
			rootvp->i_flag |= IERROR;
			iput(rootvp);
#endif
		}
		if (mi) {
			kmem_free((caddr_t)mi, (u_int)sizeof(*mi));
		}
		if (mp) {
#ifdef SVR3
			mp->m_flags = MFREE;
#else
			kmem_free((caddr_t)mp, (u_int)sizeof(*mp));
#endif
		}
	}
	iput(hidvp);
}

#ifdef SVR3
#include "sys/var.h"

/*
 * XXX should unify with in-line mechanism in os/sys3.c
 */
static struct mount *
mount_alloc()
{
	register struct mount *mp;

	mp = mount;
	for (;;) {
		if (mp->m_flags == MFREE) {
			mp->m_flags = MINTER;
			break;
		}
		if (++mp >= (struct mount *)v.ve_mount) {
			/* no free slots */
			return NULL;
		}
	}

	mp->m_bcount = 0;
	u.u_mntindx = mp - mount;
#ifdef NOTDEF
	u.u_syscall = DULBMOUNT;
#endif
	mp->m_rflags = 0;
	mp->m_flags &= ~MINTER;
	mp->m_flags |= MINUSE;
	return mp;
}
#endif

static char *
itoa(n, str)
	u_short n;
	char *str;
{
	char prbuf[11];
	register char *cp;

	cp = prbuf;
	do {
		*cp++ = "0123456789"[n%10];
		n /= 10;
	} while (n);
	do {
		*str++ = *--cp;
	} while (cp > prbuf);
	return (str);
}

/*
 * Convert an INET address into a string for printing
 */
static
addr_to_str(addr, str)
	struct sockaddr_in *addr;
	char *str;
{
	struct in_addr42 {
		union {
			struct { u_char s_b1,s_b2,s_b3,s_b4; } S_un_b;
			u_long S_addr;
		} S_un;
#define	s_net	S_un.S_un_b.s_b1	/* network */
#define	s_host	S_un.S_un_b.s_b2	/* host on imp */
#define	s_lh	S_un.S_un_b.s_b3	/* logical host */
#define	s_impno	S_un.S_un_b.s_b4	/* imp # */
	};
	register struct in_addr42 *in = (struct in_addr42 *)&addr->sin_addr;

	str = itoa(in->s_net, str);
	*str++ = '.';
	str = itoa(in->s_host, str);
	*str++ = '.';
	str = itoa(in->s_lh, str);
	*str++ = '.';
	str = itoa(in->s_impno, str);
	*str = '\0';
#undef	s_host
#undef	s_net
#undef	s_impno
#undef	s_lh
}

nfs_umount(mp)
	register struct mount *mp;
{
	struct mntinfo *mi = vftomi(mp);
	register struct inode *rvp;

#ifdef NFSDEBUG
        dprint(nfsdebug, 4, "nfs_umount(%x) mi = %x\n", mp, mi);
#endif

	/*
	 * remove pathname component cache entries for this file system
	 */
	pncc_purgedev(mp->m_dev);

	/*
	 * Remove cached text segments for this mount structure.
	 */
	(void) xumount(mp);

	if (iflush(mp) < 0
	    || mi->mi_refct != 1 || mi->mi_rootvp->i_count != 1) {
		u.u_error = EBUSY;
		return;
	}

	/*
	 * Release root inode.
	 */
	rvp = mi->mi_rootvp;
#ifdef SVR3
	plock(rvp);
	ifreebusy(rvp);
	mp->m_mount = NULL;
#else
	rvp->i_count = 0;
	iuncache(rvp);
	runcache(rvp, mi);
#endif
	kmem_free((caddr_t)mi, (u_int)sizeof(*mi));
	return;
}

#if defined SVR3 || defined diskless
/*
 * Needed in V.3 because otherwise lboot will supply a stray stub and each
 * update will result in a printf.
 */
nfs_update(mp)
	struct mount *mp;
{
}
#endif

#ifdef diskless
time_t
nfs_gettime(mp)
	struct mount *mp;
{
}

nfs_updatetime(mp)
	struct mount *mp;
{
}
#endif

/* ARGSUSED */
nfs_statfs(ip, sbp, ufstyp)
	struct inode *ip;
	register struct statfs *sbp;
	register short ufstyp;
{
	struct ucred cred;
	register struct mntinfo *mi;
	register fhandle_t *fh;
	struct nfsstatfs fs;
	register int error;

	if (ufstyp != 0) {
		u.u_error = EINVAL;
		return;
	}
	crinit(&u, &cred);
	mi = vtomi(ip);
	fh = vtofh(ip);
#ifdef NFSDEBUG
        dprint(nfsdebug, 4, "nfs_statfs fh %x %d\n", fh->fh_fsid, fh->fh_fno);
#endif
	error = rfscall(mi, RFS_STATFS, xdr_fhandle,
	    (caddr_t)fh, xdr_statfs, (caddr_t)&fs, &cred);
	if (!error) {
		error = geterrno(fs.fs_status);
	}
	if (!error) {
		if (mi->mi_stsize) {
			mi->mi_stsize = MIN(mi->mi_stsize, fs.fs_tsize);
		} else {
			mi->mi_stsize = fs.fs_tsize;
		}
		sbp->f_fstyp = nfs_fstyp;
		sbp->f_bsize = fs.fs_bsize;
		sbp->f_frsize = 0;
		sbp->f_blocks = BTOBB(fs.fs_blocks * fs.fs_bsize);
		sbp->f_bfree = BTOBB(fs.fs_bfree * fs.fs_bsize);
		sbp->f_files = 0;
		sbp->f_ffree = 0;
		bcopy(mi->mi_hostname, sbp->f_fname,
		    sizeof sbp->f_fname);
		sbp->f_fpack[0] = '\0';
	}
#ifdef NFSDEBUG
        dprint(nfsdebug, 5, "nfs_statfs returning %d\n", error);
#endif
	u.u_error = error;
}
