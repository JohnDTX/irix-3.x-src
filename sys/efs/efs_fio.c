/*
 * File control and access operations for the efs.
 *
 * $Source: /d2/3.7/src/sys/efs/RCS/efs_fio.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:27:54 $
 */
#include "efs.h"
#if NEFS > 0

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/user.h"
#include "../h/buf.h"
#include "../h/mount.h"
#include "../h/conf.h"
#include "../h/fstyp.h"
#include "../h/nami.h"
#include "../h/inode.h"
#include "../h/fcntl.h"
#include "../h/flock.h"

/*
 * efs_fcntl:
 *	- file control operations lock files and records and check
 *	  flag-setting permissions
 */
efs_fcntl(ip, cmd, arg, flag, offset)
	register struct inode *ip;
	register int cmd, arg, flag;
	off_t offset;
{
	register int i;
	struct flock bf;

	if (cmd != F_CHKFL && ip->i_ftype != IFREG) {
		u.u_error = EINVAL;	/* cannot lock irregular files */
		return;
	}
	switch (cmd) {
	case F_GETLK:
		/* get record lock */
		if (copyin((caddr_t) arg, (caddr_t) &bf, sizeof bf))
			u.u_error = EFAULT;
		else if ((i=reclock(ip, &bf, 0, flag, offset)) != 0) {
			if (i == EAGAIN)
				u.u_error = EACCES;
			else	
				u.u_error = i;
		}
		else if (copyout((caddr_t) &bf, (caddr_t) arg, sizeof bf))
			u.u_error = EFAULT;
		break;

	case F_SETLK:
		/* set record lock */
		if (copyin((caddr_t) arg, (caddr_t) &bf, sizeof bf))
			u.u_error = EFAULT;
		else if ((i=reclock(ip, &bf, SETFLCK, flag, offset)) != 0) {
			if (i == EAGAIN)
				u.u_error = EACCES;
			else
				u.u_error = i;
		}
		break;

	case F_SETLKW:
		/* set record lock and wait */
		if (copyin((caddr_t) arg, (caddr_t) &bf, sizeof bf))
			u.u_error = EFAULT;
		else if ((i=reclock(ip, &bf, SETFLCK|SLPFLCK, flag, offset))
		    != 0) {
			if (i == EAGAIN)
				u.u_error = EACCES;
			else
				u.u_error = i;
		}
		break;

	case F_CHKFL:
		break;

	case F_TRUNC:
		efs_itruncate(ip, offset);
		break;

	default:
		u.u_error = EINVAL;
	}
}

#endif
