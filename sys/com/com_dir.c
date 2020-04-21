/*
 * Directory implementation interface, with pathname component lookup cache
 * and directory scanning rotor.
 *
 * $Source: /d2/3.7/src/sys/com/RCS/com_dir.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:26:37 $
 */
#include "../h/param.h"
#include "../h/systm.h"
#include "../h/fstyp.h"
#include "../h/inode.h"
#include "../h/nami.h"
#include "../h/user.h"
#include "com_pncc.h"
#include "com_dir.h"	/* private interface */

#ifdef	OS_METER
static struct scanmeter	scanmeter;
#endif

int
dirlookup(dp, name, len, flags, ep)
	register struct inode *dp;
	register char *name;
	register unsigned short len;
	register int flags;
	register struct entry *ep;
{
	register int error;
	register off_t startoff;

	ep->e_ncstat = pncc_lookup(dp, name, len, &ep->e_ncent);
	if (ep->e_ncstat == NC_HIT) {
		error = 0;
		ASSERT(ep->e_inum != 0);
	} else {
		if ((flags & DLF_SCAN)
		    && u.u_scaninum == dp->i_number
		    && u.u_scandev == dp->i_dev) {
			METER(scanmeter.rotates++);
			startoff = u.u_scanpos;
		} else {
			startoff = 0;
		}
		error = DIR_LOOKUP(dp, name, len, startoff, &ep->e_dlres);
		if (error) {
			return error;
		}
	}

	if (flags & DLF_IGET) {
		/*
		 * Not just looking up an inumber, so we must get an inode.
		 */
		if (ep->e_inum == 0) {	/* ENOENT */
			ep->e_ip = NULL;
		} else if (ep->e_inum == dp->i_number) {
			ep->e_ip = dp;	/* "." or an alias thereto */
		} else {
			register char isdotdot = ISDOTDOT(name, len);

			/*
			 * Get the entry's inode without deadlocking.
			 * Lock parent then child; if child is "..", then
			 * release parent, lock grandparent, and re-lock
			 * parent.  Name cannot be ".".  Cyclic directory
			 * hard links other than ".." and its counterpart
			 * "from above" may result in deadlock.
			 */
			if (isdotdot) {
				iunlock(dp);
			}
			ep->e_ip = iget(dp->i_mntdev, ep->e_inum);
			if (ep->e_ip == NULL) {
				error = u.u_error;
			} else if (ep->e_ip->i_nlink <= 0) {
				printf("directory %x/%d corrupted: ",
					dp->i_dev, dp->i_number);
				printf("bad inumber %d for name %s\n",
					ep->e_inum, name);
				error = ENOENT;
				iput(ep->e_ip);
			}
			if (isdotdot) {
				ilock(dp);
			}
			if (error) {
				return error;
			}
		}
	}

	if (ep->e_inum != 0) {
		/*
		 * Update dp's scan rotor.  If name isn't cached under dp,
		 * enter it now.
		 */
		if (flags & DLF_SCAN) {
			METER(scanmeter.rotsaves++);
			u.u_scaninum = dp->i_number;
			u.u_scandev = dp->i_dev;
			u.u_scanpos = ep->e_offset;
		}
		if (ep->e_ncstat == NC_MISS) {
			ep->e_ncstat = pncc_enter(dp, name, len, &ep->e_ncent);
#ifdef OS_METER
			if ((flags & DLF_SCAN) && ep->e_offset < startoff)
				scanmeter.wraps++;
#endif
		}
	}

	ep->e_name = name;	/* fill in the rest of *ep */
	ep->e_namlen = len;
	return 0;
}

int
direnter(dp, ip, ep)
	register struct inode *dp;
	register struct inode *ip;
	register struct entry *ep;
{
	register int error;

	ASSERT(ep->e_ip == NULL);
	ep->e_dlres.dlr_inum = ip->i_number;
	error = DIR_ENTER(dp, ep->e_name, ep->e_namlen, &ep->e_dlres);
	if (!error && ep->e_ncstat == NC_MISS) {
		/*
		 * DIR_LOOKUP had set ep->e_dlres to describe empty space
		 * for a new entry.  The DIR_ENTER call just made will have
		 * modified ep->e_dlres.dlr_offset to be correct for the new
		 * entry.  Therefore we cache its inumber and offset in the
		 * hope that it will be re-referenced soon.
		 */
		ep->e_ncstat = pncc_enter(dp, ep->e_name, ep->e_namlen,
			&ep->e_ncent);
	}
	ep->e_ip = ip;
	return error;
}

int
dirremove(dp, ep)
	register struct inode *dp;
	register struct entry *ep;
{
	ASSERT(ep->e_ip);
	if (ep->e_ncstat == NC_HIT) {
		pncc_remove(&ep->e_ncent);
	}
	return DIR_REMOVE(dp, ep->e_name, ep->e_namlen, &ep->e_dlres);
}

int
dirinit(dp, pdp)
	struct inode *dp, *pdp;
{
	struct ncentry ncent;

	if (pncc_lookup(dp, dotdot, 2, &ncent) == NC_HIT
	    && pdp->i_number != ncent.nce_inum) {
		pncc_remove(&ncent);
	}
	return DIR_INIT(dp, pdp->i_number);
}

int
dirisempty(dp, dflagp)
	register struct inode *dp;
	register dflag_t *dflagp;
{
	if (DIR_ISEMPTY(dp, dflagp)) {
		struct ncentry ncent;

		if (*dflagp & DIR_HASDOTDOT) {
			if (pncc_lookup(dp, dotdot, 2, &ncent) == NC_HIT)
				pncc_remove(&ncent);
		}
		if (*dflagp & DIR_HASDOT) {
			if (pncc_lookup(dp, dot, 1, &ncent) == NC_HIT)
				pncc_remove(&ncent);
		}
		return 1;
	}
	return 0;
}
