/*
 * $Source: /d2/3.7/src/sys/sys/RCS/utssys.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:35:47 $
 */
#include "../h/param.h"
#include "../h/user.h"
#include "../h/utsname.h"
#include "../h/fstyp.h"
#include "../h/inode.h"
#include "../h/mount.h"
#include "../h/statfs.h"
#include "../h/ustat.h"

utssys()
{
	register struct a {
		char	*cbuf;
		int	mv;
		int	type;
	} *uap;
	register struct mount *mp;

	uap = (struct a *)u.u_ap;
	switch(uap->type) {
	  case 0:			/* uname */
		if (copyout((caddr_t)&utsname, uap->cbuf,
			    sizeof(struct utsname)))
			u.u_error = EFAULT;
		return;
	  case 2:			/* ustat */
		mp = mount;
		while (mp) {
			if (mp->m_dev == uap->mv) {
				register struct inode *ip;
				struct statfs stfs;
				struct ustat ust;

				ASSERT(mp->m_mount != NULL);
				ip = iget(mp, mp->m_mount->i_number);
				if (ip == NULL) {
					if (u.u_error == 0)
						u.u_error = ENOENT;
					return;
				}
				FS_STATFS(ip, &stfs, 0);
				if (u.u_error) {
					iput(ip);
					return;
				}
				ust.f_tfree = (daddr_t) stfs.f_bfree;
				ust.f_tinode = (ino_t) stfs.f_ffree;
				bcopy(stfs.f_fpack, ust.f_fpack, 
				    sizeof(ust.f_fpack));
				bcopy(stfs.f_fname, ust.f_fname, 
				    sizeof(ust.f_fname));

				if (copyout((caddr_t) &ust, uap->cbuf,
				    sizeof(ust))) {
					u.u_error = EFAULT;
				}
				iput(ip);
				return;
			}
			mp = mp->m_next;
		}
		u.u_error = EINVAL;
		return;

	default:
		u.u_error = EINVAL;
	}
}
