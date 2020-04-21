/*
 * Common filesystem namei().
 *
 * A note on updating directories and inodes: we always write directories
 * synchronously, and update inodes asynchronously.  When creating or deleting
 * a directory entry, we write the directory first, and update inode link
 * counts second.  This order is based on the following considerations:
 *	(1) fsck can detect too many or too few directory entries for a given
 *	    inode, and adjust that inode's link count.
 *	(2) The alternative to directory-then-inode order might leave dangling
 *	    inodes which fsck would link under lost+found.  In such cases the
 *	    inodes would be either newly-created and therefore uninteresting,
 *	    or else hard to correlate with their intended parent directories.
 *	(3) If there is a directory write error when creating a new node, then
 *	    we can unlink (and thus deallocate) the node and leave a clean
 *	    filesystem.
 *
 * A note on the structure of com_namei(): the SVR3 FSS namei interface is
 * fraught with side effects, non-orthogonality, and obscurity.  Moreover, it
 * does not extend gracefully.  Nevertheless, we have added rename and symlink.
 * In order that rename code can share link and unlink (com_del) code, the
 * various namei commands are implemented as vectored functions.  Dispatching
 * to an abstract name operator has proved cleaner than using a switch
 * statement (as an earlier implementation did), or using in-line code and
 * goto's (as SVR3 s5namei() does).
 *
 * $Source: /d2/3.7/src/sys/com/RCS/com_nami.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:26:42 $
 */
#include "../h/param.h"
#include "../h/systm.h"
#include "../h/conf.h"
#include "../h/fstyp.h"
#include "../h/inode.h"
#include "../h/mount.h"
#include "../h/nami.h"
#include "../h/buf.h"
#include "../h/file.h"
#include "../h/user.h"
#include "../com/com_inode.h"
#include "../com/com_dir.h"	/* private interface */

int	com_lookup(), com_del(), com_creat(), com_xcreat(), com_link(),
	com_mkdir(), com_rmdir(), com_symlink(), com_rename();

struct nameop {
	int	(*func)();	/* vectored namei() operation */
	int	success;	/* result returned unless error */
};

static struct nameop nameops[] = {
	com_lookup,	NI_PASS,	/* NI_LOOKUP	0x0 */
	com_del,	NI_NULL,	/* NI_DEL	0x1 */
	com_creat,	NI_DONE,	/* NI_CREAT	0x2 */
	com_xcreat,	NI_DONE,	/* NI_XCREAT	0x3 */
	com_link,	NI_NULL,	/* NI_LINK	0x4 */
	com_mkdir,	NI_NULL,	/* NI_MKDIR	0x5 */
	com_rmdir,	NI_NULL,	/* NI_RMDIR	0x6 */
	com_xcreat,	NI_DONE,	/* NI_MKNOD	0x7 */
	com_symlink,	NI_NULL,	/* NI_SYMLINK	0x8 */
	com_rename,	NI_NULL,	/* NI_RENAME	0x9 */
};

com_namei(np, ap)
	register struct nx *np;
	register struct argnamei *ap;
{
	register struct inode *dp;
	register int error;
	register int result;
	struct entry ent;

	/*
	 * dp must be either a directory or a symbolic link.
	 */
	dp = np->dp;
	if (dp->i_ftype == IFLNK) {
		error = pn_putlink(np->pn, dp);
		result = NI_RESTART;
	} else {
		register int cmd = (ap != NULL) ? ap->cmd : NI_LOOKUP;

		ASSERT(np->comp[0]);
		ASSERT(dp->i_ftype == IFDIR);
		error =
		    dirlookup(dp, np->comp, np->len,
			(ap == NI_LOOKUP) ? DLF_IGET|DLF_SCAN : DLF_IGET,
			&ent);
		if (!error) {
			register struct nameop *op;

			if (cmd != NI_LOOKUP && np->follow == FOLLOWLINK
			    && ent.e_ip && ent.e_ip->i_ftype == IFLNK) {
				ASSERT(ent.e_inum == ent.e_ip->i_number);
				cmd = NI_LOOKUP;	/* XXX botch */
			}
			op = &nameops[cmd];
			error = (*op->func)(dp, ap, &ent, np);
			result = op->success;
		}
	}
	if (error) {
		result = NI_FAIL;
	}
	if (result != NI_PASS && result != NI_RESTART) {
		iput(dp);	/* XXX botched design */
	}
	u.u_error = error;
	return result;
}

/*
 * Utility functions and macros.
 */
static int
canenter(dp, ep)
	register struct inode *dp;
	register struct entry *ep;
{
	if (ep->e_ip != NULL) {
		if (ep->e_ip != dp)
			iput(ep->e_ip);	/* caller beware */
		return EEXIST;
	}
	if (FS_ACCESS(dp, IWRITE)) {
		return u.u_error;
	}
	return 0;
}

static int
canremove(dp, ep)
	register struct inode *dp;
	register struct entry *ep;
{
	if (ep->e_ip == NULL) {
		return ENOENT;
	}
	if (FS_ACCESS(dp, IWRITE)) {
		if (ep->e_ip != dp)
			iput(ep->e_ip);	/* caller beware */
		return u.u_error;
	}
	return 0;
}

/*
 * Update an inode's last-changed time, returning an error number or 0.
 * Don't call FS_IUPDAT if ip has only one reference; FS_IPUT will soon
 * be called to do the update and possibly to deallocate the disk inode.
 * Otherwise, update now since ip may be the cwd of a daemon, and might
 * never be iput().
 */
static int
ichange(ip)
	register struct inode *ip;
{
	ip->i_flag |= ICHG;
	if (ip->i_count > 1) {
		(void) FS_IUPDAT(ip, &time, &time);
		return u.u_error;
	}
	return 0;
}

/*
 * Test whether ip can be linked or unlinked, returning an error in
 * u.u_error if not.
 */
#define	linkable(ip, ap) \
	((ip)->i_ftype != IFDIR || (ap)->cmd == NI_RENAME || suser())

/* ARGSUSED */
static int
com_lookup(dp, ap, ep, np)
	struct inode *dp;
	struct argnamei *ap;
	register struct entry *ep;
	register struct nx *np;
{
	register struct inode *ip;

	ip = ep->e_ip;
	if (ip == NULL)
		return ENOENT;
	ASSERT(ip->i_nlink > 0);
	np->ip = ip;
	if (ip->i_flag & IISROOT)
		np->flags |= NX_ISROOT;
	return 0;
}

static int
com_del(dp, ap, ep)
	register struct inode *dp;
	register struct argnamei *ap;
	register struct entry *ep;
{
	register int error;
	register struct inode *ip;

	error = canremove(dp, ep);
	if (error)
		return error;

	ip = ep->e_ip;
	if (ip->i_mntdev != dp->i_mntdev) {
		error = EBUSY;
	} else if (!linkable(ip, ap)) {
		error = u.u_error;
	} else {
		if (ip->i_flag&IXSAVED) {
			(void) xflush(ip);	/* try to free busy text */
		}
		if (ip->i_flag&ITEXT && ip->i_nlink == 1) {
			error = ETXTBSY;
		} else {
			error = dirremove(dp, ep);
			if (!error) {
				ip->i_nlink--;
				error = ichange(ip);
			}
		}
	}
	if (ip != dp)
		iput(ip);
	return error;
}

static int
com_creat(dp, ap, ep, np)
	register struct inode *dp;
	register struct argnamei *ap;
	register struct entry *ep;
	struct nx *np;
{
	register struct inode *ip;
	register int error;

	ip = ep->e_ip;
	if (ip != NULL) {
		ap->rcode = FSN_FOUND;
		if (dp == ip) {		/* cannot creat "." */
			error = EISDIR;
		} else {
			error = 0;
		}
	} else {
		ap->rcode = FSN_NOTFOUND;
		if (FS_ACCESS(dp, IWRITE)) {
			error = u.u_error;
		} else if (ap->ftype & (ushort)(~IFMT)) {
			error = EINVAL;
		} else {
			if (ap->ftype == 0)
				ap->ftype = IFREG;
			ap->mode &= (MODEMSK & ~u.u_cmask);
			if (ap->cmd != NI_MKNOD)
				ap->mode &= ~ISVTX;
			ap->mode |= ap->ftype;

			ip = FS_IALLOC(dp, ap->mode, 1, ap->idev);
			if (ip == NULL) {
				error = u.u_error;
			} else {
				error = direnter(dp, ip, ep);
				if (error) {
					ip->i_nlink = 0;
				}
			}
		}
	}
	if (error && ip != NULL) {
		if (ip != dp)
			iput(ip);
		ip = NULL;
	}
	np->dp = ip;
	return error;
}

static int
com_xcreat(dp, ap, ep, np)
	struct inode *dp;
	struct argnamei *ap;
	register struct entry *ep;
	struct nx *np;
{
	if (ep->e_ip != NULL) {
		if (ep->e_ip != dp)
			iput(ep->e_ip);
		return EEXIST;
	}
	return com_creat(dp, ap, ep, np);
}

static int
com_link(dp, ap, ep)
	register struct inode *dp;
	register struct argnamei *ap;
	struct entry *ep;
{
	register int error;
	register struct inode *ip;

	error = canenter(dp, ep);
	if (error)
		return error;
	ip = ap->ip;
	if (dp->i_mntdev != ip->i_mntdev)
		return EXDEV;

	if (ip != dp)
		ilock2(ip, dp);
	if (!linkable(ip, ap)) {
		error = u.u_error;
	} else if (ip->i_nlink >= MAXLINK) {
		error = EMLINK;
	} else {
		error = direnter(dp, ip, ep);
		if (!error) {
			ip->i_nlink++;
			error = ichange(ip);
		}
	}
	if (ip != dp)
		iunlock(ip);
	return error;
}

/* ARGSUSED */
static int
com_mkdir(dp, ap, ep)
	register struct inode *dp;
	register struct argnamei *ap;
	register struct entry *ep;
{
	register int error;
	register struct inode *cdp;	/* new child directory */

	error = canenter(dp, ep);
	if (error)
		return error;
	if (dp->i_nlink >= MAXLINK) {
		return EMLINK;
	}
	ap->mode &= (PERMMSK & ~u.u_cmask);
	ap->mode |= IFDIR;
	cdp = FS_IALLOC(dp, ap->mode, 2, ap->idev);
	if (cdp == NULL) {
		return u.u_error;
	}

	error = dirinit(cdp, dp);	/* create . and .. entries */
	if (!error) {
		error = direnter(dp, cdp, ep);
	}
	if (error) {
		cdp->i_nlink = 0;	/* this will cause inode to be freed */
	} else {
		dp->i_nlink++;		/* update link count for .. */
		error = ichange(dp);
	}
	iput(cdp);
	return error;
}

/* ARGSUSED */
static int
com_rmdir(dp, ap, ep)
	register struct inode *dp;
	struct argnamei *ap;
	register struct entry *ep;
{
	register int error;
	register struct inode *cdp;
	auto dflag_t dflag;

	error = canremove(dp, ep);
	if (error)
		return error;

	cdp = ep->e_ip;
	if (cdp == dp || cdp == u.u_cdir) {
		error = EINVAL;
	} else if (cdp->i_mntdev != dp->i_mntdev) {
		error = EBUSY;
	} else if (cdp->i_ftype != IFDIR) {
		error = ENOTDIR;
	} else if (!dirisempty(cdp, &dflag)) {
		error = EEXIST;	/* XXX ENOTEMPTY */
	} else {
		error = dirremove(dp, ep);
		if (!error) {
			if (dflag & DIR_HASDOTDOT) {
				dp->i_nlink--;
				(void) ichange(dp);
			}
			if (dflag & DIR_HASDOT)
				cdp->i_nlink -= 2;
			else
				cdp->i_nlink--;
			error = ichange(cdp);
		}
	}
	if (cdp != dp)
		iput(cdp);
	return error;
}

static int
com_symlink(dp, ap, ep)
	register struct inode *dp;
	register struct argnamei *ap;
	register struct entry *ep;
{
	register int error;
	register struct inode *ip;

	error = canenter(dp, ep);
	if (error)
		return error;
	ap->mode &= (PERMMSK & ~u.u_cmask);
	ap->mode |= IFLNK;
	ip = FS_IALLOC(dp, ap->mode, 1, (dev_t) 0);
	if (ip == NULL) {
		return u.u_error;
	}

	u.u_base = ap->name;
	u.u_offset = 0;
	u.u_count = ap->namlen;
	u.u_segflg = ap->namseg;
	FS_WRITEI(ip);
	error = u.u_error;
	if (!error) {
		error = direnter(dp, ip, ep);
	}
	if (error) {
		ip->i_nlink = 0;
	}
	iput(ip);
	return error;
}

static int
com_rename(tdp, ap, tep)
	register struct inode *tdp;	/* target (``to'') directory */
	struct argnamei *ap;		/* namei grot including target info */
	register struct entry *tep;	/* target entry */
{
	register int error;
	register struct inode *tip;	/* target inode if existent */
	register struct inode *fdp;	/* source (``from'') parent */
	register struct inode *fip;	/* source (``from'') inode */
	register char *fname;		/* source component name */
	struct entry fent;		/* source entry */

	fdp = ap->dp;
	fip = ap->ip;
	tip = tep->e_ip;
	fname = ap->name;
	if (fdp->i_mntdev != tdp->i_mntdev) {
		/* archaic error number for cross-filesystem rename attempt */
		error = EXDEV;
	} else if (ISDOT(fname, ap->namlen)
		|| ISDOTDOT(fname, ap->namlen)
		|| fip == fdp
		|| ISDOT(tep->e_name, tep->e_namlen)
		|| ISDOTDOT(tep->e_name, tep->e_namlen)
		|| tdp == tip) {
		/* rename sources such as "." and ".." are illegal */
		error = EINVAL;
	} else if (fip == tip) {
		/* source and target are identical - there's nothing to do */
		error = 0;
	} else if (fip->i_ftype != IFDIR
		|| (error = notancestor(fip, tdp)) == 0) {
		/*
		 * We know that either the source is not a directory or it is
		 * not an ancestor of the target directory (i.e. the rename
		 * won't cut the filesystem tree into disconnected components).
		 */
		error = dirlookup(fdp, fname, ap->namlen, 0, &fent);
		if (!error) {
			/*
			 * Lookup may return an empty or a recycled entry in
			 * fent because the source inodes are not locked.
			 */
			fent.e_ip = fip;
			error = dorename(fdp, &fent, tdp, tep, ap);
		}
	}
	if (tip != NULL) {
		iput(tip);
	}
	return error;
}

/*
 * Early returns from this function are ok - they may leave the inode being
 * renamed linked in both the source and target directories, but only if some
 * hard i/o error has occurred.  Leaving an extra link seems more prudent than
 * trying to carry on in the face of error and inviting link inconsistency.
 */
static int
dorename(fdp, fep, tdp, tep, ap)
	register struct inode *fdp;	/* source directory */
	struct entry *fep;		/* source entry */
	register struct inode *tdp;	/* target directory */
	register struct entry *tep;	/* target entry */
	struct argnamei *ap;		/* 5.3 namei grot */
{
	register struct inode *fip;	/* source inode */
	register struct inode *tip;	/* target inode if existent */
	register char renamingdir;	/* directory rename flag */
	register int error;

	fip = fep->e_ip;
	tip = tep->e_ip;
	renamingdir = fip->i_ftype == IFDIR;

	/*
	 * If the target exists and it's a directory, check that both target
	 * and source are directories and that target can be destroyed, or
	 * that neither is a directory.  If hard links to tip other than "."
	 * and ".." were permitted, then the name cache entry for ".." would
	 * have to be purged.
	 */
	if (tip != NULL) {
		dirpurgename(tep);
		if (tip->i_ftype == IFDIR) {
			auto dflag_t dflag;	/* NOTUSED */
	
			if (!renamingdir) {
				return ENOTDIR;
			}
			if (!dirisempty(tip, &dflag)
			    || tip->i_nlink > 2) {
				return EEXIST;	/* XXX ENOTEMPTY */
			}
		} else if (renamingdir) {
			return ENOTDIR;
		}
	}

	/*
	 * Link the source inode into the target entry.  This can be done
	 * ``atomically'', but if the source inode is a directory and if the
	 * rename isn't local to a directory, its ".." entry will be
	 * inconsistent till the dirinit() below.  Note that com_link() checks
	 * for write-permission and existence; we must therefore clear the
	 * target entry inode pointer.
	 */
	tep->e_ip = NULL;
	ap->ip = fip;			/* unlocked source inode */
	error = com_link(tdp, ap, tep);
	if (error) {
		return error;
	}

	/*
	 * Now that the target entry has been rewritten, drop the old target's
	 * directory reference count.
	 */
	if (tip != NULL) {
		--tip->i_nlink;
		tip->i_flag |= ICHG;
	}

	/*
	 * If the source is a directory and the target existed prior to the
	 * rename, drop the old target's link count again because it lost its
	 * self-link.  If the rename is not local to a directory, rewrite ".."
	 * in the renamed directory (and, using dirinit, in the name cache) to
	 * point to the target's parent, tdp.
	 */
	if (renamingdir) {
		if (tip != NULL) {
			--tip->i_nlink;
		}
		if (fdp != tdp) {
			error = dirinit(fip, tdp);
			if (error) {
				return error;
			}
			tdp->i_nlink++;		/* recall tdp is locked */
			(void) ichange(tdp);
			ilock(fdp);
			--fdp->i_nlink;
			(void) ichange(fdp);
			iunlock(fdp);
		}
	}

	/*
	 * Finally, remove the source.  Since fip has been unlocked, someone
	 * else may have already unlinked it, so ignore ENOENT.
	 */
	ASSERT(fip != fdp);
	iunlock(tdp);
	if (tip != NULL)
		iunlock(tip);
	ilock(fdp);		/* lock parent */
	ilock(fip);		/* THEN child */
	fip->i_count++;		/* because com_del will iput(fip) */
	error = com_del(fdp, ap, fep);
	if (error == ENOENT) {
		error = 0;
	}

	/* unlock the source directory and re-lock the target node(s) */
	ASSERT((fip->i_flag & ILOCKED) == 0);
	iunlock(fdp);
	ilock(tdp);
	if (tip != NULL)
		ilock(tip);
	return error;
}

/*
 * Return 0 if adp is not an ancestor of descendent ddp within a filesystem
 * directory hierarchy.  We use ancestor to mean the reflexive, transitive
 * closure of the parent relation on directory inodes, so (adp == ddp)
 * falsifies the predicate.  Return EINVAL if adp is an ancestor of ddp;
 * return a hard error number if corruption is detected.
 */
static int
notancestor(adp, ddp)
	register struct inode *adp, *ddp;
{
	register int error;
	register struct inode *ip;

	/*
	 * If two calls to notancestor overlapped execution, the partially
	 * completed work of one call could be invalidated by the rename
	 * which activated the other.  To avoid this, notancestor calls in
	 * the system are serialized.
	 */
#define	DONE		0x0	/* no calls currently active */
#define	IN_PROGRESS	0x1	/* one call active */
#define	WAITING		0x2	/* one or more calls waiting */
	static unsigned int state = DONE;

	while (state & IN_PROGRESS) {
		state |= WAITING;
		sleep((caddr_t) &state, PINOD);
	}
	state = IN_PROGRESS;

	/*
	 * Loop over ddp's ancestor line, stopping at adp or the filesystem's
	 * root.  If a ".." is missing, raise ENOENT.
	 */
	error = 0;
	ip = ddp;
	while ((ip->i_flag & IISROOT) == 0) {
		struct entry parent;

		if (ip == adp) {
			error = EINVAL;
			break;
		}
		error = dirlookup(ip, dotdot, 2, 0, &parent);
		if (error) {
			break;
		}
		if (parent.e_inum == ip->i_number) {
			printf("%x/%d: bad parent link\n", ip->i_dev,
			    ip->i_number);
			error = ENOENT;
			break;
		}
		if (ip == ddp)
			iunlock(ip);	/* XXX */
		else
			iput(ip);
		/*
		 * This is a race to get "..".
		 */
		ip = iget(ddp->i_mntdev, parent.e_inum);
		if (ip == NULL) {
			error = u.u_error;
			break;
		}
		if (ip->i_ftype != IFDIR || ip->i_nlink <= 0) {
			printf("%x/%d: not a directory\n", ip->i_dev,
			    ip->i_number);
			error = ENOTDIR;
			break;
		}
	}

	if (ip != NULL && ip != ddp) {
		iput(ip);
		/*
		 * Relock the rename target and make sure it has not gone
		 * away while it was unlocked.
		 */
		ilock(ddp);
		if (error == 0 && ddp->i_nlink == 0) {
			error = ENOENT;
		}
	}

	/*
	 * Unserialize notancestor calls.
	 */
	if (state & WAITING) {
		wakeup((caddr_t) &state);
	}
	state = DONE;
	return error;
}

/*
 * Set attributes in inode
 */
com_setattr(ip, ap)
	register struct inode *ip;
	register struct argnamei *ap;
{
	register struct com_inode *ci;

	ci = com_fsptr(ip);
	ASSERT(ci != NULL);

	if (u.u_uid != ip->i_uid && !suser())
		return 0;
	if (rdonlyfs(ip->i_mntdev)) {
		u.u_error = EROFS;
		return 0;
	}

	switch (ap->cmd) {
	  case NI_CHMOD:
		ci->ci_mode &= ~MODEMSK;
		if (u.u_uid) {
			ap->mode &= ~ISVTX;
			if (u.u_gid != ip->i_gid)
				ap->mode &= ~ISGID;
		}
		ci->ci_mode |= ap->mode&MODEMSK;
		return 1;

	  case NI_CHOWN:
		if (u.u_uid != 0)
			ci->ci_mode &= ~(ISUID|ISGID);
		ip->i_uid = ap->uid;
		ip->i_gid = ap->gid;
		return 1;

	}
	return 0;
}

/* ARGSUSED */
long
com_notify(ip, noargp)
	struct inode *ip;
	register struct argnotify *noargp;
{
	ASSERT(noargp != NULL);
	switch ((int) noargp->cmd) {
	  case NO_SEEK:
		if (noargp->data1 < 0)
			u.u_error = EINVAL;
		return noargp->data1;
		break;
	}
	return 0L;
}
