#include "../h/param.h"
#include "../h/user.h"
#include "../h/buf.h"

/*
 * Move n bytes at byte location cp to/from (rw) the
 * user/kernel (u_segflg) area starting at u_base.
 * Update all the arguments by the number
 * of bytes moved.
 */
iomove(cp, n, rw)
	register caddr_t cp;
	register int n;
	int rw;
{
	register struct user *up;
	register int t;

	if (n == 0)
		return;
	up = &u;
	if (up->u_segflg != 1)  {
		if (rw == B_WRITE)
			t = copyin(up->u_base, (caddr_t)cp, n);
		else
			t = copyout((caddr_t)cp, up->u_base, n);
		if (t) {
			up->u_error = EFAULT;
			return;
		}
	} else {
		if (rw == B_WRITE)
			bcopy(up->u_base, (caddr_t)cp, n);
		else
			bcopy((caddr_t)cp, up->u_base, n);
	}
	up->u_base += n;
	up->u_offset += n;
	up->u_count -= n;
}

/*
 * Pass back  c  to the user at his location u_base;
 * update u_base, u_count, and u_offset.  Return -1
 * on the last character of the user's read.
 * u_base is in the user data space.
 */
passc(c)
register c;
{
	if (subyte(u.u_base, c) < 0) {
		u.u_error = EFAULT;
		return(-1);
	}
	u.u_count--;
	u.u_offset++;
	u.u_base++;
	return(u.u_count == 0? -1: 0);
}

#if 0
/*
 * Pick up and return the next character from the user's
 * write call at location u_base;
 * update u_base, u_count, and u_offset.  Return -1
 * when u_count is exhausted.
 * u_base is in the user data space.
 */
cpass()
{
	register c;

	if (u.u_count == 0)
		return(-1);
	if ((c = fubyte(u.u_base)) < 0) {
		u.u_error = EFAULT;
		return(-1);
	}
	u.u_count--;
	u.u_offset++;
	u.u_base++;
	return(c);
}
#endif

/*
 * Routine which sets a user error; placed in
 * illegal entries in the bdevsw and cdevsw tables.
 */
nodev()
{

	u.u_error = ENODEV;
}

/*
 * Null routine; placed in insignificant entries
 * in the bdevsw and cdevsw tables.
 */
nulldev()
{
}
