/*
 * $Source: /d2/3.7/src/sys/sys/RCS/fs_nami.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:35:17 $
 */
#include "../h/param.h"
#include "../h/systm.h"
#include "../h/user.h"
#include "../h/fstyp.h"
#include "../h/inode.h"
#include "../h/mount.h"
#include "../h/nami.h"

/* well-known string constants for general use */
char	dot[sizeof(long)] = ".";
char	dotdot[sizeof(long)] = "..";

/*
 * Lookup an inode by its pathname.
 *
 * The inode is returned locked.  Results are also returned in the u.
 * The pathname is made up of components separated by '/'s.  If it
 * begins with a '/' the search starts from the user's root directory,
 * otherwise from his current directory.  As each directory component is
 * isolated, its inode is passed through the filesystem switch to a
 * specific FS_NAMEI.  The final component's inode is passed with
 * action, which specifies either a particular command such as NI_DEL
 * for unlink, or lookup, with the value NI_LOOKUP.
 *
 * Each component's inode up to the final one must be either a directory
 * or a symbolic link.  A symbolic link has the (recursive) effect of
 * making the inode named by the link be the "current" inode.
 *
 * The input protocol is as follows:
 *	u.u_dirp	Pointer to the pathname string in user or system
 *			space.
 *	fetch		Function to get the pathname string from its
 *			address space.
 *	action		NI_LOOKUP for lookup or command to perform when
 *			inode is found.  Legal action->cmd values are:
 *			NI_RENAME	rename a file atomically
 *			NI_SYMLINK	make a symbolic link
 *			NI_LINK		make a hard link
 *			NI_MKNOD	make a non-directory node
 *			NI_CREAT	create a regular file
 *			NI_XCREAT	exclusive create
 *			NI_DEL		unlink this file
 *			NI_MKDIR	make a directory
 *			NI_RMDIR	remove a directory
 *	follow		Whether or not to follow a symbolic link at the
 *			end the pathname.
 *
 * Side effects and outputs are:
 *	(return)	A pointer to the inode named by u.u_dirp or
 *			NULL if none was found.
 *	u.u_error	An error number if non-zero.
 */
struct inode *
namei(fetch, action, follow)
	int (*fetch)();
	struct argnamei *action;
	enum pnfollow follow;
{
	struct pathname pn;
	auto struct inode *ip;

	u.u_error = pathname(u.u_dirp, fetch, &pn);
	if (u.u_error != 0) {
		return NULL;	/* couldn't get pathname */
	}
	u.u_error =
	    pn_lookup(&pn, action, follow, (struct inode **) NULL, &ip);
	pn_free(&pn);
	return ip;
}

/*
 * Translate pn to an inode and perform the given action on the inode.
 * If translation succeeds and ipp is non-NULL, *ipp will point to the inode
 * on return.  If the translation fails and ipp points somewhere, *ipp will
 * be NULL on return.  Likewise for result parameters dpp, the final pathname
 * component's directory inode pointer.  On return pn->pn_component points
 * to the pathname's final component.  This function returns an errno.
 */
int
pn_lookup(pn, action, follow, dpp, ipp)
	register struct pathname *pn;
	struct argnamei *action;
	enum pnfollow follow;
	struct inode **dpp;
	struct inode **ipp;
{
	register char *comp;		/* current component */
	register struct inode *ip;	/* and current inode */
	register struct inode *rootip;	/* root of absolute pathname */
	register int linkdepth;		/* symbolic link depth counter */
	register struct mount *mp;	/* ip's mount, used in NI_PASS */
	register struct inode *dp;	/* parent of symlink ip if non-NULL */
	register int error;		/* error number to return */
	struct argnamei *ap;		/* per-component action */
	struct nx nx;			/* what a grotty name */

	/*
	 * Start at the user's current directory.
	 */
	ip = u.u_cdir;
	IHOLD(ip);
	linkdepth = 0;
	dp = NULL;
	error = 0;
	nx.pn = pn;
	nx.follow = follow;
	if (dpp != NULL) {
		*dpp = NULL;
	}

restart:
	/*
	 * Start translation.  If pn is null, raise "no such file" error.
	 * Otherwise find the user's root, used for later "/.." checking.
	 * If the pathname is absolute, start at this root.  Symbolic link
	 * processing resumes at restart with ip pointing to the link's
	 * parent in case the link is relative.
	 */
	if (pn_endofpath(pn)) {
		error = ENOENT;
		goto out;
	}
	rootip = u.u_rdir;
	if (rootip == NULL) {
		rootip = rootdir;
	}
	if (pn_peekchar(pn) == '/') {
		PN_SKIPSLASH(pn);
		if (pn_endofpath(pn) && action != NI_LOOKUP) {
			error = ENOENT;
			goto out;
		}
		iput(ip);
		ip = rootip;
		IHOLD(ip);
	}

dirloop:
	/*
	 * At this point ip must be an existent directory.
	 */
	if (ip->i_ftype != IFDIR || ip->i_nlink <= 0) {
		error = ENOTDIR;
		goto out;
	}

	/*
	 * Check for permission to search this directory.
	 */
	if (FS_ACCESS(ip, ICDEXEC) != 0) {
		error = u.u_error;
		goto out;
	}

	if (pn_endofpath(pn)) {	/* handle "/" case */
		goto out;
	}
	PN_GETCOMPONENT(pn, comp);
	if (! pn_endofpath(pn)) {
		ap = NI_LOOKUP;		/* non-terminal lookup */
	} else {
		ap = action;		/* terminal component action */
		if (dpp != NULL) {
			ip->i_count++;	/* use so caller can keep */
			*dpp = ip;
		}
	}

	/*
	 * We don't need to go through the switch if comp is "."
	 * and we're just looking up
	 */
	if (ISDOT(comp, pn->pn_complen) && action == NI_LOOKUP) {
		goto dirloop;
	}

mntloop:
	nx.dp = ip;
	nx.comp = comp;
	nx.len = pn->pn_complen;
	nx.flags = 0;

	/*
	 * Call the filesystem-specific namei to handle the component
	 * described by ``nx''.
	 */
	switch (FS_NAMEI(ip, &nx, ap)) {

	  case NI_FAIL:		/* failure */
		error = u.u_error;
		ASSERT(error != 0);
		if (dp != NULL) {
			/*
			 * We failed to expand a symlink whose parent was
			 * held locked via dp.
			 */
			ASSERT(ip->i_ftype == IFLNK);
			ASSERT(dp != ip);
			iput(dp);
			dp = NULL;
		}
		/* FALL THROUGH */

	  case NI_NULL:		/* void operation complete */
		ip = NULL;
		goto out;

	  case NI_DONE:		/* success on final component */
		ip = nx.dp;
		ASSERT(error == 0);
		goto out;

	  case NI_PASS:		/* pass over this component */
		mp = ip->i_mntdev;
		if (ISDOTDOT(comp, pn->pn_complen)) {
			if (ip == rootip) {
				/*
				 * Nullify cds above user's root.
				 */
				if (nx.ip != ip) {
					iput(nx.ip);
					nx.ip = ip;
				}
			} else if (nx.flags & NX_ISROOT
				    && ip->i_flag & IISROOT
				    && mp->m_dev != rootdev) {
				/*
				 * Get the mounted-on directory.
				 */
				if (nx.ip != ip) {
					iput(nx.ip);
				}
				iput(ip);
				ip = mp->m_inodp;
				IHOLD(ip);
				goto mntloop;
			}
		}

		/*
		 * 5.3 symbolic link semantics (hah!) dictate that links
		 * be interpreted relative to u.u_cdir.  Not very useful.
		 * To implement a more useful relative symbolic link
		 * interpretation, we must save the link's directory till
		 * it's been expanded (NI_RESTART below), and then resume
		 * with (ip = dp) at restart.
		 */
		ASSERT(dp == NULL);
		dp = ip;
		ip = nx.ip;	/* next component */
		ASSERT(ip != NULL);
		ASSERT(ip->i_nlink > 0);
		if (ip->i_ftype == IFLNK
		    && (!pn_endofpath(pn) || follow == FOLLOWLINK)) {
			/*
			 * If this component is a symbolic link call FS_NAMEI
			 * again to expand its contents into the pathname
			 * buffer.  Don't expand if comp is terminal and we
			 * were told not to follow terminal links.
			 */
			if (linkdepth < MAXLINKDEPTH) {
				linkdepth++;
				goto mntloop;
			}
			error = ELOOP;
		}

		/*
		 * Since ip isn't an expandable symlink, release its parent.
		 */
		if (dp != ip) {
			iput(dp);
		}
		dp = NULL;
		if (error) {
			goto out;
		}
		if (pn_endofpath(pn)) {
			strncpy(u.u_dent.d_name, comp, sizeof u.u_dent.d_name);
			goto out;
		}
		goto dirloop;

	  case NI_RESTART:
		/*
		 * The pathname was revised to contain a symbolic link
		 * expansion.  Recover the last component's parent inode
		 * pointer as ip and start from the top.
		 */
		ASSERT(ip == nx.dp);
		ASSERT(dp != NULL);
		ASSERT(ip != dp);
		iput(ip);
		ip = dp;
		dp = NULL;
		goto restart;

#ifdef OS_ASSERT
	  default:
		panic("namei");
#endif
	}

out:
	ASSERT(dp == NULL);
	/*
	 * If the caller was interested in the parent but we have no parent
	 * or the child is identical to *dpp, then return an error since we
	 * don't have the real parent.
	 */
	if (dpp != NULL && (*dpp == NULL || *dpp == ip)) {
		if (ip != NULL && *dpp == ip)
			--ip->i_count;
		error = EINVAL;
	}
	if (error != 0 && ip != NULL) {
		iput(ip);
		ip = NULL;
	}
	if (ipp != NULL) {
		*ipp = ip;
	} else if (ip != NULL) {
		iput(ip);
	}
	return error;
}

#ifdef OS_METER
struct pnmeter {
	long	pathnames;	/* number of pathnames created */
	long	pathbytes;	/* total pathname bytes used */
	long	longpaths;	/* number of overlong pathnames */
	long	maxpath;	/* size of longest pathname */
};

static struct pnmeter pnmeter;
#endif

union pathbuf {
	union pathbuf	*next;
	char		buf[MAXPATHLEN];
};

static union pathbuf	*pn_freebuflist;
static unsigned short	pn_freebufs;
static unsigned short	pn_maxfreebufs = NBPG / MAXPATHLEN;

pn_free(pn)
	struct pathname *pn;
{
	register union pathbuf *pb;

	pb = (union pathbuf *) pn->pn_buf;
	if (pn_freebufs >= pn_maxfreebufs) {
		free((char *) pb);
	} else {
		pb->next = pn_freebuflist;
		pn_freebuflist = pb;
		pn_freebufs++;
	}
}

/*
 * Initialize pathname object *pn with the name pointed to by name, in the
 * address space indicated by fetch.  Fetch must point at either fustring or
 * fsstring (see machine/machdep.c).
 */
int
pathname(name, fetch, pn)
	register char *name;
	register int (*fetch)();
	register struct pathname *pn;
{
	register int size, maxsize;
	register union pathbuf *pb;
	register int error = 0;

	pb = pn_freebuflist;
	if (pb != NULL) {
		pn_freebuflist = pb->next;
		--pn_freebufs;
	} else {
		pb = (union pathbuf *) malloc(sizeof *pb);
		if (pb == NULL)
			panic("pathname");
	}
	pn->pn_buf = pb->buf;
	maxsize = MAXPATHLEN;
	size = 0;
	for (;;) {
		register int amount;

		amount = (*fetch)(name, pn->pn_buf + size, maxsize);
		if (u.u_error) {
			error = u.u_error;
			break;
		}
		if (amount <= 0) {	/* hit end of string */
			size += -amount;
			break;
		}
		maxsize -= amount;
		if (maxsize <= 0) {	/* overlong name */
			METER(pnmeter.longpaths++);
			error = ENAMETOOLONG;
			break;
		}
		name += amount;
		size += amount;
	}
	if (size <= 1) {		/* size includes end of string */
		error = ENOENT;
	}
	if (error) {
		pn_free(pn);
		return error;
	}
	pn->pn_component = pn->pn_nextchar = pn->pn_buf;
	pn->pn_maxsize = size;
#ifdef OS_METER
	--size;
	if (size > pnmeter.maxpath)
		pnmeter.maxpath = size;
	pnmeter.pathnames++;
	pnmeter.pathbytes += size;
#endif
	return 0;
}

/*
 * Read a symbolic link from ip into pn in front of the residual
 * pathname.  Return 0 or an error number.
 */
int
pn_putlink(pn, ip)
	register struct pathname *pn;
	register struct inode *ip;
{
	register char *endoflink;

	ASSERT(ip->i_ftype == IFLNK);
	ASSERT(pn->pn_nextchar > pn->pn_buf);
	/*
	 * Restore the slash potentially clobbered by PN_GETCOMPONENT
	 * and check whether there's enough room at the front of pn
	 * for the link.
	 */
	if (!pn_endofpath(pn)) {
		*--pn->pn_nextchar = '/';
	}
	endoflink = pn->pn_buf + ip->i_size;
	if (pn->pn_nextchar < endoflink) {
		register int rem, size;

		/*
		 * We must slide the remaining pathname up past endoflink,
		 * taking care to increase pn_maxsize by the slide distance.
		 * NB: bcopy() usage assumes overlap capability.
		 */
		rem = pn->pn_maxsize - (pn->pn_nextchar - pn->pn_buf);
		size = ip->i_size + rem;
		if (size > MAXPATHLEN) {
			METER(pnmeter.longpaths++);
			return ENAMETOOLONG;
		}
		if (rem > 0) {
			bcopy(pn->pn_nextchar, endoflink, rem);
		}
		pn->pn_nextchar = pn->pn_buf;
		pn->pn_maxsize = size;
		ASSERT(pn->pn_buf[size-1] == '\0');
	} else {
		/*
		 * pn_maxsize must reflect the number of bytes from pn_buf[0]
		 * through the terminating 0, so we don't change it.
		 */
		pn->pn_nextchar -= ip->i_size;
	}

	/*
	 * Read the link into pn in front of the remaining pathname.
	 */
	u.u_offset = 0;
	u.u_base = pn->pn_component = pn->pn_nextchar;
	u.u_count = ip->i_size;
	u.u_segflg = 1;		/* UIOSEG_KERNEL */
	FS_READI(ip);
	return u.u_error;
}
