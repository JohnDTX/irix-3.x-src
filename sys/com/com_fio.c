/*
 * $Source: /d2/3.7/src/sys/com/RCS/com_fio.c,v $
 * $Date: 89/03/27 17:26:39 $
 * $Revision: 1.1 $
 */
#include "../h/param.h"
#include "../h/systm.h"
#include "../h/conf.h"
#include "../h/fstyp.h"
#include "../h/inode.h"
#include "../h/nami.h"
#include "../h/mount.h"
#include "../h/user.h"
#include "../h/buf.h"
#include "../h/file.h"
#include "../h/stat.h"
#include "../h/fcntl.h"
#include "../h/flock.h"
#include "../com/com_inode.h"

/*
 * Check mode permission on inode pointer.
 * Mode is READ, WRITE or EXEC.
 * In the case of WRITE, the read-only status of the file
 * system is checked. Also in WRITE, prototype text
 * segments cannot be written.
 * The mode is shifted to select the owner/group/other fields.
 * The super user is granted all permissions.
 */
com_access(ip, mode)
	register struct inode *ip;
	register mode;
{
	register struct com_inode *ci;

	ci = com_fsptr(ip);
	ASSERT(ci != NULL);
	switch (mode) {

	case ISUID:
	case ISGID:
	case ISVTX:
		if ((ci->ci_mode&mode))
			return(0);
		return(1);

	case IMNDLCK:
		if (((ci->ci_mode&IFMT)==IFREG) && 
			(ci->ci_mode & (ISGID|(IEXEC>>3))) == ISGID) 
			return(0);
		return(1);

	case IOBJEXEC:
		if ((ci->ci_mode & IFMT) != IFREG ||
 		   (ci->ci_mode & (IEXEC|(IEXEC>>3)|(IEXEC>>6))) == 0) 
			goto fail;

	case ICDEXEC:
		mode = IEXEC;
		goto exec;

	case IWRITE:
		if (rdonlyfs(ip->i_mntdev)) {
			u.u_error = EROFS;
			return(1);
		}
		if (ip->i_flag&IXSAVED)
			(void) xflush(ip);
		if (ip->i_flag & ITEXT) {
			u.u_error = ETXTBSY;
			return(1);
		}
		/* FALL THROUGH */
	case IREAD:
	case IEXEC:
exec:
		if (u.u_uid == 0)
			return(0);
		if (u.u_uid != ip->i_uid) {
			mode >>= 3;
			if (u.u_gid != ip->i_gid)
				mode >>= 3;
		}
		if ((ci->ci_mode&mode) != 0)
			return(0);
		goto fail;

	}

fail:
	u.u_error = EACCES;
	return(1);
}

/*
 * com_openi called to allow handler of special files to initialize and
 * validate before actual IO.
 */
com_openi(ip, flag)
	register struct inode *ip;
{
	dev_t dev;
	register unsigned int maj;

	dev = (dev_t)ip->i_rdev;
	switch (ip->i_ftype) {

	case IFCHR:
		maj = major(dev);
		if (maj >= cdevcnt)
			goto bad;
		if (u.u_ttyp == NULL)
			u.u_ttyd = dev;
		if (cdevsw[maj].d_str)
			stropen(ip, flag);
		else
			(*cdevsw[maj].d_open)(dev, flag);
		break;

	case IFBLK:
		maj = bmajor(dev);
		if (maj >= bdevcnt)
			goto bad;
		(*bdevsw[maj].d_open)(dev, flag);
		break;

	case IFIFO:
		pipe_openi(ip, flag);
	}
	return;

bad:
	u.u_error = ENXIO;
}

/* ARGSUSED */
com_closei(ip, flag, count, offset)
	register struct inode *ip;
	int flag, count;
	off_t offset;
{
	register int (*cfunc)();
	register struct file *fp;
	register int fmt;
	register dev_t dev;

	cleanlocks(ip);
	if (ip->i_sptr && ip->i_ftype != IFDIR)
		strclean(ip);
	if ((unsigned)count > 1)
		return;

	fmt = ip->i_ftype;
	dev = ip->i_rdev;
	switch (fmt) {

	case IFCHR:
		cfunc = cdevsw[major(dev)].d_close;
		break;

	case IFBLK:
		cfunc = bdevsw[bmajor(dev)].d_close;
		break;

	case IFIFO:
		pipe_closei(ip, flag);
		/* FALL THROUGH */

	default:
		return;
	}

	/*
	 * Don't call device close routine if there is any other open 
	 * device inode referring to the same device.
	 */
	if (ip->i_count > 1) 
		goto out;

	for (fp = file; fp < fileNFILE; fp++) {
		register struct inode *tip;

		if (fp->f_count) {
			tip = fp->f_inode;
			if (tip->i_rdev == dev && tip->i_ftype == fmt
			  && tip != ip)
				goto out;
		}
	}
	if (save(u.u_qsave)) {	/* catch half-closes */
		ilock(ip);
		goto out;
	}
	if (fmt == IFBLK) {
		register struct mount *mp;

		for (mp = mount; mp != NULL; mp = mp->m_next)
			if (mp->m_dev == dev) {
				(*cfunc)(dev, flag);
				goto out;
			}
		bflush(dev);
		(*cfunc)(dev, flag);
		binval(dev);
	} else {
		iunlock(ip);
		if (cdevsw[major(dev)].d_str)
			strclose(ip, flag);
		else
			(*cfunc)(dev, flag);
		ilock(ip);
	}
out:
	return;
}

/* fcntl */

com_fcntl(ip, cmd, arg, flag, offset)
	register struct inode *ip;
	int cmd, arg, flag;
	off_t offset;
{
	struct flock bf;
	int i;

	switch (cmd) {
	case F_GETLK:
		/* get record lock */
		if (copyin((caddr_t)arg, &bf, sizeof bf))
			u.u_error = EFAULT;
		else if ((i = reclock(ip, &bf, 0, flag, offset)) != 0)
			u.u_error = i;
		else if (copyout(&bf, (caddr_t)arg, sizeof bf))
			u.u_error = EFAULT;
		break;

	case F_SETLK:
		/* set record lock */
		if (copyin((caddr_t)arg, &bf, sizeof bf))
			u.u_error = EFAULT;
		else if ((i = reclock(ip, &bf, SETFLCK, flag, offset)) != 0) {
			/* The following if statement is to maintain
			** backward compatibility. Note: the return
			** value of reclock was not changed to
			** EAGAIN because rdwr() also calls reclock(),
			** and we want rdwr() to meet the standards.
			*/
			if ( i == EAGAIN )
				u.u_error = EACCES;
			else
				u.u_error = i;
		}
		break;

	case F_SETLKW:
		/* set record lock and wait */
		if (copyin((caddr_t)arg, &bf, sizeof bf))
			u.u_error = EFAULT;
		else if ((i = reclock(ip, &bf,
		  SETFLCK | SLPFLCK, flag, offset)) != 0) {
				u.u_error = i;
		}
		break;

	case F_CHKFL:
		break;

	default:
		u.u_error = EINVAL;
	}
}

/* ioctl */
com_ioctl(ip, cmd, arg, flag)
	register struct inode *ip;
	int cmd, arg, flag;
{
	register dev_t dev;

	if (ip->i_ftype != IFCHR) {
		u.u_error = ENOTTY;
		return;
	}
	dev = ip->i_rdev;
	if (cdevsw[major(dev)].d_str)
		strioctl(ip, cmd, arg, flag);
	else
		(*cdevsw[major(dev)].d_ioctl)(dev, cmd, arg, flag);
}
