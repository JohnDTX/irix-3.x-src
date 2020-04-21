/*
 * Copyright (c) 1982 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)uipc_syscalls.c	6.10 (Berkeley) 9/16/85
 */

#ifdef SVR3
#include "../tcp-param.h"
#include "sys/sbd.h"
#include "sys/param.h"
#include "sys/systm.h"
#include "sys/signal.h"
#include "sys/pcb.h"
#include "sys/immu.h"
#include "sys/region.h"
#include "sys/fs/s5dir.h"
#include "sys/user.h"
#include "sys/proc.h"
#include "sys/file.h"
#include "sys/inode.h"
#include "sys/mount.h"
#undef MFREE
#include "sys/buf.h"
#include "sys/mbuf.h"
#include "sys/protosw.h"
#include "sys/socket.h"
#include "sys/socketvar.h"
#include "sys/uio.h"
#include "sys/cmn_err.h"
#include "sys/errno.h"
#include "sys/fstyp.h"
#include "sys/conf.h"
#include "sys/debug.h"

/* AT&T-compatible inode private data pointer type */
typedef	int	*fsptr_t;
#else
#include "../h/param.h"
#include "../h/systm.h"
#include "../h/dir.h"
#include "../h/user.h"
#include "../h/proc.h"
#include "../h/file.h"
#include "../h/inode.h"
#include "../h/buf.h"
#include "../h/mbuf.h"
#include "../h/protosw.h"
#include "../h/socket.h"
#include "../h/socketvar.h"
#include "../h/uio.h"

/* a better inode private data pointer type */
typedef	char	*fsptr_t;
#endif

/*
 * System call interface to the socket abstraction.
 */

extern short soc_fstyp;

struct	socket *getsock();

/* make a new 'socket/file/inode'
 */
static struct file *
so_file()
{
	register struct inode *ip;
	register struct file *fp;
	struct inode *getinode();

	ip = getinode((struct mount *) NULL, soc_fstyp, NODEV);
	if (ip == NULL) {
		u.u_error = ENOBUFS;
		return NULL;
	}
	ASSERT(ip->i_fsptr == NULL);

	ip->i_number = ip - inode;	/* number, 1-to-1 with cache index */
	ip->i_size = 0;			/* number of bytes in file */
	ip->i_rdev = NODEV;		/* underlying device if IFBLK/IFCHR */
	ip->i_ftype = IFCHR;
	ip->i_nlink = 1;		/* number of links to file */
	ip->i_uid = u.u_uid;		/* owner user id */
	ip->i_gid = u.u_gid;      	/* owner group id */

	if ((fp = falloc(ip, FREAD|FWRITE)) == NULL) {
		iput(ip);
		return NULL;
	}
	fp->f_flag = FREAD|FWRITE;
	fp->f_inode = ip;
#ifdef SVR3
	prele(ip);			/* return with inode unlocked */
#else
	iunlock(ip);
#endif

	return fp;
}


socket()
{
	register struct a {
		int	domain;
		int	type;
		int	protocol;
	} *uap = (struct a *)u.u_ap;
	struct socket *so;
	register struct file *fp;
	register struct inode *ip;
	register int i;

	fp = so_file();
	if (!fp)
		return;
	ip = fp->f_inode;
	i = u.u_rval1;
	
#ifdef SVR3
	if (setjmp(u.u_qsav)) {
#else
	if (save(u.u_qsave)) {	/* catch half-opens */
#endif
		if (u.u_error == 0) {
			u.u_error = EINTR;
		}
		u.u_ofile[i] = NULL;
		closef(fp);
	} else {
		u.u_error =
		    socreate(uap->domain, &so, uap->type, uap->protocol);
		if (u.u_error == 0) {
			so->so_com.ci_ctime = time; 
			so->so_com.ci_atime = time; 
			so->so_com.ci_mtime = time; 
			ip->i_fsptr = (fsptr_t) so;
			return;
		}
		u.u_ofile[i] = NULL;
		if (--fp->f_count <= 0) {
			fp->f_next = ffreelist;
			ffreelist = fp;
		}
#ifdef SVR3
		plock(ip);
		iput(ip);
#else
		iunuse(ip);
#endif
	}
}

bind()
{
	register struct a {
		int	s;
		caddr_t	name;
		int	namelen;
	} *uap = (struct a *)u.u_ap;
	register struct socket *so;
	struct mbuf *nam;

	so = getsock(uap->s);
	if (!so)
		return;
	u.u_error = sockargs(&nam, uap->name, uap->namelen, MT_SONAME);
	if (u.u_error)
		return;
	u.u_error = sobind(so, nam);
	m_freem(nam);
}

listen()
{
	register struct a {
		int	s;
		int	backlog;
	} *uap = (struct a *)u.u_ap;
	register struct socket *so;

	so = getsock(uap->s);
	if (so == 0)
		return;
	u.u_error = solisten(so, uap->backlog);
}

accept()
{
	register struct a {
		int	s;
		caddr_t	name;
		int	*anamelen;
	} *uap = (struct a *)u.u_ap;
	register struct file *fp;
#ifndef sgi
	register struct inode *ip;
#endif
	struct mbuf *nam;
	int namelen;
	int s;
	register struct socket *so;

	if (uap->name == 0)
		goto noname;
	u.u_error = copyin((caddr_t)uap->anamelen, (caddr_t)&namelen,
		sizeof (namelen));
	if (u.u_error)
		return;
	if (useracc((caddr_t)uap->name, (u_int)namelen, B_WRITE) == 0) {
		u.u_error = EFAULT;
		return;
	}
noname:
	so = getsock(uap->s);
	if (so == 0)
		return;
	s = splnet();
	if ((so->so_options & SO_ACCEPTCONN) == 0) {
		u.u_error = EINVAL;
		splx(s);
		return;
	}
	if ((so->so_state & SS_NBIO) && so->so_qlen == 0) {
		u.u_error = EWOULDBLOCK;
		splx(s);
		return;
	}
	while (so->so_qlen == 0 && so->so_error == 0) {
		if (so->so_state & SS_CANTRCVMORE) {
			so->so_error = ECONNABORTED;
			break;
		}
		sleep((caddr_t)&so->so_timeo, PZERO+1);
	}
	if (so->so_error) {
		u.u_error = so->so_error;
		so->so_error = 0;
		splx(s);
		return;
	}
	{ struct socket *aso = so->so_q;
	  if (soqremque(aso, 1) == 0)
		panic("accept");
	  so = aso;
	}

	fp = so_file();
	if (!fp) {
		splx(s);
		return;
	}
	so->so_com.ci_ctime = time; 
	so->so_com.ci_atime = time; 
	so->so_com.ci_mtime = time; 
	fp->f_inode->i_fsptr = (fsptr_t)so;

	nam = m_get(M_WAIT, MT_SONAME);
	(void) soaccept(so, nam);
	if (uap->name) {
		if (namelen > nam->m_len)
			namelen = nam->m_len;
		/* SHOULD COPY OUT A CHAIN HERE */
		(void) copyout(mtod(nam, caddr_t), (caddr_t)uap->name,
		    (u_int)namelen);
		(void) copyout((caddr_t)&namelen, (caddr_t)uap->anamelen,
		    sizeof (*uap->anamelen));
	}
	m_freem(nam);
	splx(s);
}

connect()
{
	register struct a {
		int	s;
		caddr_t	name;
		int	namelen;
	} *uap = (struct a *)u.u_ap;
	register struct socket *so;
	struct mbuf *nam;
	int s;

	so = getsock(uap->s);
	if (so == 0)
		return;
	if ((so->so_state & SS_NBIO) &&
	    (so->so_state & SS_ISCONNECTING)) {
		u.u_error = EALREADY;
		return;
	}
	u.u_error = sockargs(&nam, uap->name, uap->namelen, MT_SONAME);
	if (u.u_error)
		return;
	u.u_error = soconnect(so, nam);
	if (u.u_error)
		goto bad;
	if ((so->so_state & SS_NBIO) &&
	    (so->so_state & SS_ISCONNECTING)) {
		u.u_error = EINPROGRESS;
		m_freem(nam);
		return;
	}
	s = splnet();
	while ((so->so_state & SS_ISCONNECTING) && so->so_error == 0) {
		if (sleep((caddr_t)&so->so_timeo, (PZERO+1)|PCATCH)) {
			u.u_error = EINTR;
			goto bad2;
		}
	}
	u.u_error = so->so_error;
	so->so_error = 0;
bad2:
	splx(s);
bad:
	so->so_state &= ~SS_ISCONNECTING;
	m_freem(nam);
}

socketpair()
{
#ifdef	NOTDEF
	register struct a {
		int	domain;
		int	type;
		int	protocol;
		int	*rsv;
	} *uap = (struct a *)u.u_ap;
	register struct file *fp1, *fp2;
	struct socket *so1, *so2;
	int sv[2];

	if (useracc((caddr_t)uap->rsv, 2 * sizeof (int), B_WRITE) == 0) {
		u.u_error = EFAULT;
		return;
	}
	u.u_error = socreate(uap->domain, &so1, uap->type, uap->protocol);
	if (u.u_error)
		return;
	u.u_error = socreate(uap->domain, &so2, uap->type, uap->protocol);
	if (u.u_error)
		goto free;
	fp1 = so_file();
	if (fp1 == NULL)
		goto free2;
	sv[0] = u.u_rval1;
	fp1->f_flag = FREAD|FWRITE;
	fp1->f_type = DTYPE_SOCKET;
	fp1->f_ops = &socketops;
	fp1->f_data = (caddr_t)so1;
	fp2 = falloc();
	if (fp2 == NULL)
		goto free3;
	fp2->f_flag = FREAD|FWRITE;
	fp2->f_type = DTYPE_SOCKET;
	fp2->f_ops = &socketops;
	fp2->f_data = (caddr_t)so2;
	sv[1] = u.u_r.r_val1;
	u.u_error = soconnect2(so1, so2);
	if (u.u_error)
		goto free4;
	if (uap->type == SOCK_DGRAM) {
		/*
		 * Datagram socket connection is asymmetric.
		 */
		 u.u_error = soconnect2(so2, so1);
		 if (u.u_error)
			goto free4;
	}
	u.u_r.r_val1 = 0;
	(void) copyout((caddr_t)sv, (caddr_t)uap->rsv, 2 * sizeof (int));
	return;
free4:
	fp2->f_count = 0;
	u.u_ofile[sv[1]] = 0;
free3:
	fp1->f_count = 0;
	u.u_ofile[sv[0]] = 0;
free2:
	(void)soclose(so2);
free:
	(void)soclose(so1);
#endif
	u.u_error = EOPNOTSUPP;
}

sendto()
{
	register struct a {
		int	s;
		caddr_t	buf;
		int	len;
		int	flags;
		caddr_t	to;
		int	tolen;
	} *uap = (struct a *)u.u_ap;
	struct msghdr msg;
	struct iovec aiov;

	msg.msg_name = uap->to;
	msg.msg_namelen = uap->tolen;
	msg.msg_iov = &aiov;
	msg.msg_iovlen = 1;
	aiov.iov_base = uap->buf;
	aiov.iov_len = uap->len;
	msg.msg_accrights = 0;
	msg.msg_accrightslen = 0;
	sendit(uap->s, &msg, uap->flags);
}

send()
{
	register struct a {
		int	s;
		caddr_t	buf;
		int	len;
		int	flags;
	} *uap = (struct a *)u.u_ap;
	struct msghdr msg;
	struct iovec aiov;

	msg.msg_name = 0;
	msg.msg_namelen = 0;
	msg.msg_iov = &aiov;
	msg.msg_iovlen = 1;
	aiov.iov_base = uap->buf;
	aiov.iov_len = uap->len;
	msg.msg_accrights = 0;
	msg.msg_accrightslen = 0;
	sendit(uap->s, &msg, uap->flags);
}

sendmsg()
{
	register struct a {
		int	s;
		caddr_t	msg;
		int	flags;
	} *uap = (struct a *)u.u_ap;
	struct msghdr msg;
	struct iovec aiov[MSG_MAXIOVLEN];

	u.u_error = copyin(uap->msg, (caddr_t)&msg, sizeof (msg));
	if (u.u_error)
		return;
	if ((u_int)msg.msg_iovlen >= sizeof (aiov) / sizeof (aiov[0])) {
		u.u_error = EMSGSIZE;
		return;
	}
	u.u_error =
	    copyin((caddr_t)msg.msg_iov, (caddr_t)aiov,
		(unsigned)(msg.msg_iovlen * sizeof (aiov[0])));
	if (u.u_error)
		return;
	msg.msg_iov = aiov;
	sendit(uap->s, &msg, uap->flags);
}

sendit(s, mp, flags)
	int s;
	register struct msghdr *mp;
	int flags;
{
	register struct socket *so;
	struct uio auio;
	register struct iovec *iov;
	register int i;
	struct mbuf *to, *rights;
	int len;
	
	so = getsock(s);
	if (so == 0)
		return;
	auio.uio_iov = mp->msg_iov;
	auio.uio_iovcnt = mp->msg_iovlen;
	auio.uio_segflg = UIO_USERSPACE;
	auio.uio_offset = 0;			/* XXX */
	auio.uio_resid = 0;
	iov = mp->msg_iov;
	for (i = 0; i < mp->msg_iovlen; i++, iov++) {
		if (iov->iov_len < 0) {
			u.u_error = EINVAL;
			return;
		}
		if (iov->iov_len == 0)
			continue;
		if (useracc(iov->iov_base, (u_int)iov->iov_len, B_READ) == 0) {
			u.u_error = EFAULT;
			return;
		}
		auio.uio_resid += iov->iov_len;
	}
	if (mp->msg_name) {
		u.u_error =
		    sockargs(&to, mp->msg_name, mp->msg_namelen, MT_SONAME);
		if (u.u_error)
			return;
	} else
		to = 0;
	if (mp->msg_accrights) {
		u.u_error =
		    sockargs(&rights, mp->msg_accrights, mp->msg_accrightslen,
		    MT_RIGHTS);
		if (u.u_error)
			goto bad;
	} else
		rights = 0;
	len = auio.uio_resid;
	u.u_error = sosend(so, to, &auio, flags, rights);
	u.u_rval1 = len - auio.uio_resid;
	if (rights)
		m_freem(rights);
bad:
	if (to)
		m_freem(to);
}

recvfrom()
{
	register struct a {
		int	s;
		caddr_t	buf;
		int	len;
		int	flags;
		caddr_t	from;
		int	*fromlenaddr;
	} *uap = (struct a *)u.u_ap;
	struct msghdr msg;
	struct iovec aiov;
	int len;

	u.u_error = copyin((caddr_t)uap->fromlenaddr, (caddr_t)&len,
	   sizeof (len));
	if (u.u_error)
		return;
	msg.msg_name = uap->from;
	msg.msg_namelen = len;
	msg.msg_iov = &aiov;
	msg.msg_iovlen = 1;
	aiov.iov_base = uap->buf;
	aiov.iov_len = uap->len;
	msg.msg_accrights = 0;
	msg.msg_accrightslen = 0;
	recvit(uap->s, &msg, uap->flags, (caddr_t)uap->fromlenaddr, (caddr_t)0);
}

recv()
{
	register struct a {
		int	s;
		caddr_t	buf;
		int	len;
		int	flags;
	} *uap = (struct a *)u.u_ap;
	struct msghdr msg;
	struct iovec aiov;

	msg.msg_name = 0;
	msg.msg_namelen = 0;
	msg.msg_iov = &aiov;
	msg.msg_iovlen = 1;
	aiov.iov_base = uap->buf;
	aiov.iov_len = uap->len;
	msg.msg_accrights = 0;
	msg.msg_accrightslen = 0;
	recvit(uap->s, &msg, uap->flags, (caddr_t)0, (caddr_t)0);
}

recvmsg()
{
	register struct a {
		int	s;
		struct	msghdr *msg;
		int	flags;
	} *uap = (struct a *)u.u_ap;
	struct msghdr msg;
	struct iovec aiov[MSG_MAXIOVLEN];

	u.u_error = copyin((caddr_t)uap->msg, (caddr_t)&msg, sizeof (msg));
	if (u.u_error)
		return;
	if ((u_int)msg.msg_iovlen >= sizeof (aiov) / sizeof (aiov[0])) {
		u.u_error = EMSGSIZE;
		return;
	}
	u.u_error =
	    copyin((caddr_t)msg.msg_iov, (caddr_t)aiov,
		(unsigned)(msg.msg_iovlen * sizeof (aiov[0])));
	if (u.u_error)
		return;
	msg.msg_iov = aiov;
	if (msg.msg_accrights)
		if (useracc((caddr_t)msg.msg_accrights,
		    (unsigned)msg.msg_accrightslen, B_WRITE) == 0) {
			u.u_error = EFAULT;
			return;
		}
	recvit(uap->s, &msg, uap->flags,
	    (caddr_t)&uap->msg->msg_namelen,
	    (caddr_t)&uap->msg->msg_accrightslen);
}

recvit(s, mp, flags, namelenp, rightslenp)
	int s;
	register struct msghdr *mp;
	int flags;
	caddr_t namelenp, rightslenp;
{
	register struct socket *so;
	struct uio auio;
	register struct iovec *iov;
	register int i;
	struct mbuf *from, *rights;
	int len;
	
	so = getsock(s);
	if (so == 0)
		return;
	auio.uio_iov = mp->msg_iov;
	auio.uio_iovcnt = mp->msg_iovlen;
	auio.uio_segflg = UIO_USERSPACE;
	auio.uio_offset = 0;			/* XXX */
	auio.uio_resid = 0;
	iov = mp->msg_iov;
	for (i = 0; i < mp->msg_iovlen; i++, iov++) {
		if (iov->iov_len < 0) {
			u.u_error = EINVAL;
			return;
		}
		if (iov->iov_len == 0)
			continue;
		if (useracc(iov->iov_base, (u_int)iov->iov_len, B_WRITE) == 0) {
			u.u_error = EFAULT;
			return;
		}
		auio.uio_resid += iov->iov_len;
	}
	len = auio.uio_resid;
	u.u_error = soreceive(so, &from, &auio, flags, &rights);
	u.u_rval1 = len - auio.uio_resid;
	if (mp->msg_name) {
		len = mp->msg_namelen;
		if (len <= 0 || from == 0)
			len = 0;
		else {
			if (len > from->m_len)
				len = from->m_len;
			(void) copyout((caddr_t)mtod(from, caddr_t),
			    (caddr_t)mp->msg_name, (unsigned)len);
		}
		(void) copyout((caddr_t)&len, namelenp, sizeof (int));
	}
	if (mp->msg_accrights) {
		len = mp->msg_accrightslen;
		if (len <= 0 || rights == 0)
			len = 0;
		else {
			if (len > rights->m_len)
				len = rights->m_len;
			(void) copyout((caddr_t)mtod(rights, caddr_t),
			    (caddr_t)mp->msg_accrights, (unsigned)len);
		}
		(void) copyout((caddr_t)&len, rightslenp, sizeof (int));
	}
	if (rights)
		m_freem(rights);
	if (from)
		m_freem(from);
}

shutdown()
{
	struct a {
		int	s;
		int	how;
	} *uap = (struct a *)u.u_ap;
	register struct socket *so;

	so = getsock(uap->s);
	if (so == 0)
		return;
	u.u_error = soshutdown(so, uap->how);
}

setsockopt()
{
	struct a {
		int	s;
		int	level;
		int	name;
		caddr_t	val;
		int	valsize;
	} *uap = (struct a *)u.u_ap;
	struct socket *so;
	struct mbuf *m = NULL;

	so = getsock(uap->s);
	if (so == 0)
		return;
	if (uap->valsize > MLEN) {
		u.u_error = EINVAL;
		return;
	}
	if (uap->val) {
		m = m_get(M_WAIT, MT_SOOPTS);
		if (m == NULL) {
			u.u_error = ENOBUFS;
			return;
		}
		u.u_error =
		    copyin(uap->val, mtod(m, caddr_t), (u_int)uap->valsize);
		if (u.u_error) {
			(void) m_free(m);
			return;
		}
		m->m_len = uap->valsize;
	}
	u.u_error = sosetopt(so, uap->level, uap->name, m);
}

getsockopt()
{
	struct a {
		int	s;
		int	level;
		int	name;
		caddr_t	val;
		int	*avalsize;
	} *uap = (struct a *)u.u_ap;
	struct socket *so;
	struct mbuf *m = NULL;
	int valsize;

	so = getsock(uap->s);
	if (so == 0)
		return;
	if (uap->val) {
		u.u_error = copyin((caddr_t)uap->avalsize, (caddr_t)&valsize,
			sizeof (valsize));
		if (u.u_error)
			return;
	} else
		valsize = 0;
	u.u_error = sogetopt(so, uap->level, uap->name, &m);
	if (u.u_error)
		goto bad;
	if (uap->val && valsize && m != NULL) {
		if (valsize > m->m_len)
			valsize = m->m_len;
		u.u_error = copyout(mtod(m, caddr_t), uap->val, (u_int)valsize);
		if (u.u_error)
			goto bad;
		u.u_error = copyout((caddr_t)&valsize, (caddr_t)uap->avalsize,
		    sizeof (valsize));
	}
bad:
	if (m != NULL)
		(void) m_free(m);
}

/*
 * Get socket name.
 */
getsockname()
{
	register struct a {
		int	fdes;
		caddr_t	asa;
		int	*alen;
	} *uap = (struct a *)u.u_ap;
	register struct socket *so;
	struct mbuf *m;
	int len;

	so = getsock(uap->fdes);
	if (so == 0)
		return;
	u.u_error = copyin((caddr_t)uap->alen, (caddr_t)&len, sizeof (len));
	if (u.u_error)
		return;
	m = m_getclr(M_WAIT, MT_SONAME);
	if (m == NULL) {
		u.u_error = ENOBUFS;
		return;
	}
	u.u_error = (*so->so_proto->pr_usrreq)(so, PRU_SOCKADDR, 0, m, 0);
	if (u.u_error)
		goto bad;
	if (len > m->m_len)
		len = m->m_len;
	u.u_error = copyout(mtod(m, caddr_t), (caddr_t)uap->asa, (u_int)len);
	if (u.u_error)
		goto bad;
	u.u_error = copyout((caddr_t)&len, (caddr_t)uap->alen, sizeof (len));
bad:
	m_freem(m);
}

/*
 * Get name of peer for connected socket.
 */
getpeername()
{
	register struct a {
		int	fdes;
		caddr_t	asa;
		int	*alen;
	} *uap = (struct a *)u.u_ap;
	register struct socket *so;
	struct mbuf *m;
	int len;

	so = getsock(uap->fdes);
	if (so == 0)
		return;
	if ((so->so_state & SS_ISCONNECTED) == 0) {
		u.u_error = ENOTCONN;
		return;
	}
	m = m_getclr(M_WAIT, MT_SONAME);
	if (m == NULL) {
		u.u_error = ENOBUFS;
		return;
	}
	u.u_error = copyin((caddr_t)uap->alen, (caddr_t)&len, sizeof (len));
	if (u.u_error)
		return;
	u.u_error = (*so->so_proto->pr_usrreq)(so, PRU_PEERADDR, 0, m, 0);
	if (u.u_error)
		goto bad;
	if (len > m->m_len)
		len = m->m_len;
	u.u_error = copyout(mtod(m, caddr_t), (caddr_t)uap->asa, (u_int)len);
	if (u.u_error)
		goto bad;
	u.u_error = copyout((caddr_t)&len, (caddr_t)uap->alen, sizeof (len));
bad:
	m_freem(m);
}

sockargs(aname, name, namelen, type)
	struct mbuf **aname;
	caddr_t name;
	int namelen, type;
{
	register struct mbuf *m;
	int error;

	if (namelen > MLEN)
		return (EINVAL);
	m = m_get(M_WAIT, type);
	if (m == NULL)
		return (ENOBUFS);
	m->m_len = namelen;
	error = copyin(name, mtod(m, caddr_t), (u_int)namelen);
	if (error)
		(void) m_free(m);
	else
		*aname = m;
	return (error);
}

struct socket *
getsock(fdes)
	int fdes;
{
	register struct file *fp;
	register struct inode *ip;

	extern struct file *getf();

	fp = getf(fdes);
	if (fp == NULL)
		return (0);
	ip = fp->f_inode;
	if (ip->i_fstyp != soc_fstyp) {
		u.u_error = ENOTSOCK;
		return (0);
	}
	return soc_fsptr(ip);
}


/* hack to make S5 copyin/out look like bsd */
#undef copyin
int					/* return a good errno */
bsd_copyin(from,to,len)
char *from, *to;
int len;
{
	return (copyin(from,to,len) ? EFAULT : 0);
}

#undef copyout
int					/* return a good errno */
bsd_copyout(from,to,len)
char *from, *to;
int len;
{
	return (copyout(from,to,len) ? EFAULT : 0);
}
