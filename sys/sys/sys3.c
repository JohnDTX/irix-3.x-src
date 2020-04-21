/*
 * $Source: /d2/3.7/src/sys/sys/RCS/sys3.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:35:37 $
 */

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/user.h"
#include "../h/fcntl.h"
#include "../h/file.h"
#include "../h/flock.h"
#include "../h/fstyp.h"
#include "../h/inode.h"
#include "../h/nami.h"
#include "../h/stat.h"
#include "../h/statfs.h"

/*
 * the fstat system call.
 */
fstat()
{
	register struct file *fp;
	register struct a {
		int	fdes;
		struct stat *sb;
	} *uap;

	uap = (struct a *)u.u_ap;
	fp = getf(uap->fdes);
	if(fp == NULL)
		return;
	stat1(fp->f_inode, uap->sb);
}

/*
 * the stat system call.
 */
stat()
{
	register struct inode *ip;
	register struct a {
		char	*fname;
		struct stat *sb;
	} *uap;

	uap = (struct a *)u.u_ap;
	ip = namei(USERPATH, NI_LOOKUP, FOLLOWLINK);
	if(ip == NULL)
		return;
	stat1(ip, uap->sb);
	iput(ip);
}

/*
 * the lstat system call.
 */
lstat()
{
	struct a {
		char *fname;
		struct stat *sb;
	};
	register struct inode *ip;
	register struct a *uap;

	uap = (struct a *)u.u_ap;
	ip = namei(USERPATH, NI_LOOKUP, DONTFOLLOW);
	if (ip == NULL)
		return;
	stat1(ip,uap->sb);
	iput(ip);
}

/*
 * The basic routine for fstat and stat:
 * get the inode and pass appropriate parts back.
 */
stat1(ip, ub)
	register struct inode *ip;
	struct stat *ub;
{
	struct stat ds;

	if (ip->i_flag & (IACC|IUPD|ICHG))
		FS_IUPDAT(ip, &time, &time);

	ds.st_dev = ip->i_dev;
	ds.st_ino = ip->i_number;
	ds.st_nlink = ip->i_nlink;
	ds.st_uid = ip->i_uid;
	ds.st_gid = ip->i_gid;
	ds.st_rdev = ip->i_rdev;
	ds.st_size = ip->i_size;
	/*
	 * Switch to filesystem implementation for mode and times.
	 * Note that the filesystem may override any of the above
	 * assignments.
	 */
	FS_STATF(ip, &ds);

	if (copyout((caddr_t)&ds, (caddr_t)ub, sizeof(ds)) < 0)
		u.u_error = EFAULT;
}

/*
 * the old, short-inumber, fstat system call.
 */
oldfstat()
{
	register struct file *fp;
	register struct a {
		int	fdes;
		struct oldstat *sb;
	} *uap;

	uap = (struct a *)u.u_ap;
	fp = getf(uap->fdes);
	if(fp == NULL)
		return;
	oldstat1(fp->f_inode, uap->sb);
}

/*
 * the old stat system call.
 */
oldstat()
{
	register struct inode *ip;
	register struct a {
		char	*fname;
		struct oldstat *sb;
	} *uap;

	uap = (struct a *)u.u_ap;
	ip = namei(USERPATH, NI_LOOKUP, FOLLOWLINK);
	if(ip == NULL)
		return;
	oldstat1(ip, uap->sb);
	iput(ip);
}

/*
 * the old lstat system call.
 */
oldlstat()
{
	struct a {
		char *fname;
		struct oldstat *sb;
	};
	register struct inode *ip;
	register struct a *uap;

	uap = (struct a *)u.u_ap;
	ip = namei(USERPATH, NI_LOOKUP, DONTFOLLOW);
	if (ip == NULL)
		return;
	oldstat1(ip,uap->sb);
	iput(ip);
}

/*
 * The basic routine for fstat and stat:
 * get the inode and pass appropriate parts back.
 */
oldstat1(ip, ub)
	register struct inode *ip;
	struct oldstat *ub;
{
	struct stat ns;
	struct oldstat os;

	if (ip->i_flag & (IACC|IUPD|ICHG))
		FS_IUPDAT(ip, &time, &time);

	ns.st_dev = ip->i_dev;
	ns.st_ino = ip->i_number;
	ns.st_nlink = ip->i_nlink;
	ns.st_uid = ip->i_uid;
	ns.st_gid = ip->i_gid;
	ns.st_rdev = ip->i_rdev;
	ns.st_size = ip->i_size;
	/*
	 * Switch to filesystem implementation for mode and times.
	 * Note that the filesystem may override any of the above
	 * assignments.
	 */
	FS_STATF(ip, &ns);

	os.st_dev = ns.st_dev;
	os.st_ino = ns.st_ino;
	os.st_mode = ns.st_mode;
	os.st_nlink = ns.st_nlink;
	os.st_uid = ns.st_uid;
	os.st_gid = ns.st_gid;
	os.st_rdev = ns.st_rdev;
	os.st_size = ns.st_size;
	os.st_atime = ns.st_atime;
	os.st_mtime = ns.st_mtime;
	os.st_ctime = ns.st_ctime;
	if (copyout((caddr_t)&os, (caddr_t)ub, sizeof(os)) < 0)
		u.u_error = EFAULT;
}

/*
 * the fstatfs system call.
 */
fstatfs()
{
	register struct file *fp;
	register struct inode *ip;
	register struct a {
		int	fdes;
		struct	statfs *sb;
		int	len;
		int	fstyp;
	} *uap = (struct a *)u.u_ap;

	if ((fp = getf(uap->fdes)) == NULL)
		return;
	ilock(ip = fp->f_inode);
	statfs1(ip, uap->sb, uap->len, uap->fstyp);
	iunlock(ip);
}

/*
 * the statfs system call.
 */
statfs()
{
	register struct inode *ip;
	register struct a {
		char	*fname;
		struct	statfs *sb;
		int	len;
		int	fstyp;
	} *uap = (struct a *)u.u_ap;

	ip = namei(USERPATH, NI_LOOKUP, FOLLOWLINK);
	if (ip == NULL)
		return;
	statfs1(ip, uap->sb, uap->len, uap->fstyp);
	iput(ip);
}

/*
 * Common routine for fstatfs and statfs.
 */
statfs1(ip, ub, len, fstyp)
register struct inode *ip;
struct statfs *ub;
int len, fstyp;
{
	struct statfs ds;

	if (len < 0 || len > sizeof(ds) || fstyp < 0 || fstyp >= nfstyp) {
		u.u_error = EINVAL;
		return;
	}
	if (fstyp && ip->i_ftype != IFCHR && ip->i_ftype != IFBLK) {
		u.u_error = EINVAL;
		return;
	}
	/*
	 * Get generic superblock from fs-dependent code.
	 */
	FS_STATFS(ip, &ds, fstyp);
	if (u.u_error == 0
	    && copyout((caddr_t)&ds, (caddr_t)ub, len) < 0) {
		u.u_error = EFAULT;
	}
}

/*
 * the dup system call.
 */
dup()
{
	register struct file *fp;
	int i;
	struct a {
		int	fdes;
	} *uap;

	uap = (struct a *)u.u_ap;
	fp = getf(uap->fdes);
	if(fp == NULL)
		return;
	if ((i = ufalloc(0)) < 0)
		return;
	u.u_ofile[i] = fp;
	fp->f_count++;
}

/*
 * the file control system call.
 */
fcntl()
{
	register struct file *fp;
	register struct a {
		int	fdes;
		int	cmd;
		int	arg;
	} *uap;
	register i;

	uap = (struct a *)u.u_ap;
	fp = getf(uap->fdes);
	if (fp == NULL)
		return;
	switch(uap->cmd) {
	case F_DUPFD:
		i = uap->arg;
		if (i < 0 || i > NOFILE) {
			u.u_error = EINVAL;
			return;
		}
		if ((i = ufalloc(i)) < 0)
			return;
		u.u_ofile[i] = fp;
		fp->f_count++;
		break;

	case F_GETFD:
		u.u_rval1 = u.u_pofile[uap->fdes];
		break;

	case F_SETFD:
		u.u_pofile[uap->fdes] = uap->arg;
		break;

	case F_GETFL:
		u.u_rval1 = fp->f_flag+FOPEN;
		break;

	case F_SETFL:
		FS_FCNTL(fp->f_inode, F_CHKFL, uap->arg, fp->f_flag,
			 fp->f_offset);
		if (u.u_error)
			return;
		uap->arg &= FMASK;
		fp->f_flag &= (FREAD|FWRITE);
		fp->f_flag |= (uap->arg-FOPEN) & ~(FREAD|FWRITE);
		break;

	default:
		FS_FCNTL(fp->f_inode, uap->cmd, uap->arg, fp->f_flag,
			 fp->f_offset);
		break;
	}
}

/*
 * character special i/o control
 */
ioctl()
{
	register struct file *fp;
	register struct a {
		int	fdes;
		int	cmd;
		int	arg;
	} *uap;

	uap = (struct a *)u.u_ap;
	if ((fp = getf(uap->fdes)) == NULL)
		return;
	FS_IOCTL(fp->f_inode, uap->cmd, uap->arg, fp->f_flag);
}
