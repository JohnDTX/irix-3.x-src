/*
 * $Source: /d2/3.7/src/sys/sys/RCS/acct.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:35:05 $
 */
#include "../h/param.h"
#include "../h/systm.h"
#include "../h/acct.h"
#include "../h/user.h"
#include "../h/inode.h"
#include "../h/nami.h"
#include "../h/fstyp.h"
#include "../h/file.h"
#include "../h/buf.h"

/*
 * Perform process accounting functions.
 */

sysacct()
{
	register struct inode *ip;
	register struct a {
		char	*fname;
	} *uap;
	static char ac_lock;

	uap = (struct a *)u.u_ap;
	if (ac_lock || !suser())
		return;
	ac_lock++;
	switch (uap->fname) {
	case NULL:
		if (ip = acctp) {
			ilock(ip);
			FS_CLOSEI(ip, FWRITE, 1, (off_t) 0);
			iput(ip);
			acctp = NULL;
		}
		break;
	default:
		if (acctp) {
			u.u_error = EBUSY;
			break;
		}
		ip = namei(USERPATH, NI_LOOKUP, FOLLOWLINK);
		if (ip == NULL)
			break;
		if (ip->i_ftype != IFREG)
			u.u_error = EACCES;
		else {
			(void) FS_ACCESS(ip, IWRITE);
			if (!u.u_error)
				FS_OPENI(ip, FWRITE);
		}
		if (u.u_error) {
			iput(ip);
			break;
		}
		acctp = ip;
		iunlock(ip);
	}
	ac_lock--;
}

/*
 * On exit, write a record on the accounting file.
 */
acct(st)
{
	register struct inode *ip;
	struct acct acctbuf;
	off_t siz;

	if ((ip=acctp) == NULL)
		return;
	ilock(ip);
	bcopy((caddr_t)u.u_comm, (caddr_t)acctbuf.ac_comm,
	      sizeof(acctbuf.ac_comm));
	acctbuf.ac_btime = u.u_start;
	acctbuf.ac_utime = compress(u.u_utime);
	acctbuf.ac_stime = compress(u.u_stime);
	acctbuf.ac_etime = compress(lbolt - u.u_ticks);
	acctbuf.ac_mem = compress(u.u_mem);
	acctbuf.ac_io = compress(u.u_ioch);
	acctbuf.ac_rw = compress(u.u_ior+u.u_iow);
	acctbuf.ac_uid = u.u_ruid;
	acctbuf.ac_gid = u.u_rgid;
	acctbuf.ac_tty = u.u_ttyp ? u.u_ttyd : NODEV;
	acctbuf.ac_stat = st;
	acctbuf.ac_flag = u.u_acflag;
	siz = ip->i_size;
	u.u_offset = siz;
	u.u_base = (caddr_t)&acctbuf;
	u.u_count = sizeof(acctbuf);
	u.u_segflg = 1;
	u.u_error = 0;
	u.u_limit = (daddr_t)5000;
	u.u_fmode = FWRITE;
	FS_WRITEI(ip);
	if (u.u_error)
		ip->i_size = siz;
	iunlock(ip);
}

/*
 * Produce a pseudo-floating point representation
 * with 3 bits base-8 exponent, 13 bits fraction.
 */
compress(t)
register time_t t;
{
	register exp = 0, round = 0;

	while (t >= 8192) {
		exp++;
		round = t&04;
		t >>= 3;
	}
	if (round) {
		t++;
		if (t >= 8192) {
			t >>= 3;
			exp++;
		}
	}
	return((exp<<13) + t);
}
