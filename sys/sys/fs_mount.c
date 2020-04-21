#include "../h/param.h"
#include "../h/systm.h"
#include "../h/user.h"
#include "../h/fstyp.h"
#include "../h/file.h"
#include "../h/inode.h"
#include "../h/nami.h"
#include "../h/mount.h"

/* header for the mount table */
struct mount *mount;

enum mnt_type { MOUNT=0, UMOUNT=1 };

/*
 * the mount system call.
 */
smount()
{
	register struct a {
		char	*fspec;
		char	*freg;
		int	readonly;
	} *uap;
	register struct inode *ip;
	auto struct inode *devip;

	uap = (struct a *)u.u_ap;
	u.u_error = getmdev(MOUNT, &devip);
	if (u.u_error) {
		return;
	}
	u.u_dirp = (caddr_t)uap->freg;
	ip = namei(USERPATH, NI_LOOKUP, FOLLOWLINK);
	if (ip != NULL) {
		if (ip->i_ftype != IFDIR) {
			u.u_error = ENOTDIR;
		} else if (ip->i_count != 1 || ip->i_flag & IISROOT) {
			u.u_error = EBUSY;
		} else {
			u.u_error = mountfs(devip, uap->readonly, ip);
		}
		iput(ip);
	}
	iput(devip);
}

/*
 * mountdev:
 *	- mount dev's filesystem on ip
 *	- fake up an inode for dev and call mountfs
 *	- the caller is responsible for calling iput on ip
 */
int
mountdev(dev, readonly, ip)
	dev_t dev;
	int readonly;
	struct inode *ip;
{
	struct inode devin;

	devin.i_number = 42;
	devin.i_dev = devin.i_rdev = dev;
	devin.i_fstyp = 0;
	devin.i_fstypp = &fstypsw[0];
	devin.i_mntdev = NULL;
	devin.i_ftype = IFBLK;
	devin.i_nlink = 1;
	devin.i_uid = devin.i_gid = 0;
	devin.i_size = 0;
	devin.i_fsptr = NULL;

	return mountfs(&devin, readonly, ip);
}

/*
 * mountfs:
 *	- mount devip's filesystem on ip
 *	- the caller, who presumably got coveredip, is responsible for
 *	  putting coveredip
 */
int
mountfs(devip, readonly, coveredip)
	struct inode *devip;
	int readonly;
	register struct inode *coveredip;
{
	register dev_t dev;
	register struct mount *mp;
	register short i;

	dev = devip->i_rdev;
	/*
	 * Search mount list and make sure device isn't already mounted.
	 * If its already mounted, return an error.  If its not, then
	 * allocate a new mount structure.
	 */
	if (mount_find(dev) != NULL) {
		return EBUSY;
	}
	mp = (struct mount *) calloc(1, sizeof *mp);
	if (mp == NULL) {
		return ENOMEM;
	}

	/*
	 * Figure out what type of filesystem it is.  Go through the
	 * filesystem switch calling each entry's fs_mount function.
	 * When one succeeds, use that entry's index as an fstyp.
	 */
	for (i = 1; i < nfstyp; i++) {
		if (FS_MOUNT(i, devip, mp, readonly))
			break;
	}
	if (i == nfstyp) {
		free((char *) mp);
		return u.u_error ? u.u_error : EINVAL;
	}

	/*
	 * The fs_mount operation should have set all of the mount
	 * structure's members except m_inodp and m_next.  Set m_inodp
	 * to coveredip and link the new mount onto the mount list.
	 * The insert operation adds a reference to coveredip.
	 */
	ASSERT(mp->m_fstyp == i);
	mount_insert(mp, coveredip);
	return 0;
}

/*
 * the umount system call.
 */
sumount()
{
	register struct a {
		char	*fspec;
	};
	auto struct inode *devip;
	register dev_t dev;
	register struct mount **mpp;

	u.u_error = getmdev(UMOUNT, &devip);
	if (u.u_error)
		return;
	dev = devip->i_rdev;
	iput(devip);
	mpp = mount_find(dev);
	if (mpp != NULL) {
		u.u_error = umount(mpp);
	} else {
		u.u_error = EINVAL;
	}
}

/*
 * umount:
 *	- internal subroutine to unmount a device, given a pointer to
 *	  its mount pointer
 */
int
umount(mpp)
	register struct mount **mpp;
{
	register struct mount *mp;

	mp = *mpp;
	FS_UMOUNT(mp);
	if (u.u_error) {
		return u.u_error;
	}

	/*
	 * Unlink this mount struct from the mount list and release the
	 * directory inode which was covered by the mounted filesystem.
	 * Free the mount structure itself.
	 */
	mount_remove(mpp);
	free((char *)mp);
	return (0);
}

/*
 * umountall:
 *	- un-mount everything that's mounted
 *	- give up in case of error
 * XXX	add in code to nicely print device name
 */
umountall()
{
	printf("Unmounting: ");
	while (mount != NULL) {
		printf("%04x ", mount->m_dev);
		if (umount(&mount))
			break;
	}
	printf("\n");
}

/*
 * Common code for mount and umount.
 * Check that the user's argument is a reasonable thing on which to mount,
 * and return a pointer to the device's locked inode in *devipp.
 * Return an errno on failure.
 */
getmdev(mtype, devipp)
	enum mnt_type mtype;
	struct inode **devipp;
{
	register struct inode *ip;
	register int error = 0;

	/*
	 * If we're not the super user, no dice.
	 */
	if (!suser()) {
		return (u.u_error);
	}
	/*
	 * Attempt to open the device the user specified.  If it doesn't
	 * exist (or we can't open it) or its not a block device, puke.
	 */
	ip = namei(USERPATH, NI_LOOKUP, FOLLOWLINK);
	if (ip == NULL) {
		return (u.u_error);
	}

	/*
	 * If we are unmounting and ip is a root directory, set its
	 * rdev to that of its mount.  This enhancement integrates sun's
	 * nfs unmount functionality.  Check for block device and make
	 * sure that it is within the range of configured devices in
	 * the system.
	 */
	if (mtype == UMOUNT && ip->i_ftype == IFDIR) {
		if (ip->i_flag & IISROOT) {
			ip->i_rdev = ip->i_mntdev->m_dev;
		} else {
			error = EINVAL;
		}
	} else if (ip->i_ftype != IFBLK) {
		error = ENOTBLK;
	} else if (major(ip->i_rdev) >= bdevcnt) {
		error = ENXIO;
	}
	if (!error) {
		*devipp = ip;
		return (0);
	}

	iput(ip);
	return (error);
}

/*
 * Mount structure operations.
 */
struct mount **
mount_find(dev)
	register dev_t dev;
{
	register struct mount *mp, **mpp;

	mpp = &mount;
	for (mp = mount; mp != NULL; mp = mp->m_next) {
		if (mp->m_dev == dev) {
			return mpp;
		}
		mpp = &mp->m_next;
	}
	return NULL;
}

void
mount_insert(mp, coveredip)
	register struct mount *mp;
	register struct inode *coveredip;
{
	mp->m_inodp = coveredip;
	if (coveredip != NULL) {
		coveredip->i_flag |= IMOUNT;
		coveredip->i_mnton = mp;
		iuse(coveredip);
	}
	mp->m_next = mount;
	mount = mp;
}

void
mount_remove(mpp)
	register struct mount **mpp;
{
	register struct mount *mp;
	register struct inode *coveredip;

	ASSERT(mpp != NULL);
	mp = *mpp;
	ASSERT(mp != NULL);
	coveredip = mp->m_inodp;
	if (coveredip != NULL) {
		coveredip->i_flag &= ~IMOUNT;
		coveredip->i_mnton = NULL;
		iunuse(coveredip);
	}
	*mpp = mp->m_next;
}
