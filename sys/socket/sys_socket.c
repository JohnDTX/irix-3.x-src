/* file-system-to-socket glue
 *	4.3bsd sockets are connected to the rest of the system by making them
 *	live in a special file system.  This allows us to avoid having to put
 *	'file ops' in the file table or to have to modify all of the places in
 *	the system where a user might hand a socket descriptor to something
 *	expecting a file descriptor.
 *
 * $Source: /d2/3.7/src/sys/socket/RCS/sys_socket.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:34:33 $
 */


#ifdef SVR3
#include "../tcp-param.h"
#include "sys/param.h"
#include "sys/systm.h"
#include "sys/signal.h"
#include "sys/sbd.h"
#include "sys/pcb.h"
#include "sys/immu.h"
#include "sys/region.h"
#include "sys/user.h"
#include "sys/conf.h"
#include "sys/fstyp.h"
#include "sys/inode.h"
#include "sys/stat.h"
#include "sys/statfs.h"
#include "sys/socket.h"
#include "sys/socketvar.h"
#include "sys/uio.h"
#include "sys/protosw.h"
#include "sys/termio.h"
#include "../net/soioctl.h"
#include "sys/file.h"
#include "sys/fcntl.h"
#include "sys/errno.h"
#include "sys/debug.h"
#else 
#include "../h/param.h"
#include "../h/systm.h"
#include "../h/user.h"
#include "../h/conf.h"
#include "../h/fstyp.h"
#include "../h/inode.h"
#include "../h/stat.h"
#include "../h/statfs.h"
#include "../h/socket.h"
#include "../h/socketvar.h"
#include "../h/uio.h"
#include "../h/protosw.h"
#include "../h/termio.h"
#include "../net/soioctl.h"
#include "../h/file.h"
#include "../h/fcntl.h"
#endif


short	soc_fstyp = 0;

/*
 * Figure out my fstyp, which is the well-known global soc_fstyp.
 */
soc_init()
{
	soc_fstyp = findfstyp(soc_init);
}

/*
 * Close a socket
 */
/* ARGSUSED */
soc_closei(ip, mode, count, offset)
register struct inode *ip;
int mode;
unsigned count;
off_t offset;
{
	register int rv;
	
	if (count > 1)
		rv = 0;
	else {
		rv = soclose(soc_fsptr(ip));
		ip->i_fsptr = 0;
	}

	return rv;
}

/*
 * read or write to a socket
 */
static
soc_rdwr(ip,rw)
struct inode *ip;
enum uio_rw rw;
{
	register struct user *up = &u;
	struct uio uio;
	struct iovec iov;

	/*
	 * While System V views I/O counts as unsigned ints,
	 * the socket code views them as signed ints. Rather than
	 * change the socket code (and carefully check it for
	 * problems thus induced), we will just invalidate writes
	 * >= '-1'.
	 */
	if ((int)(up->u_count) < 0) {
		u.u_error = EFAULT;
		return;
	}

	/* we could (should?) use the IRIS u_io structure instead of
	 * a 4.3 uio.  However, we might have to change the socket code.
	 * We would at least have to check that the socket code does not use
	 * the iov structure.
	 */
	uio.uio_iov = &iov;
	uio.uio_iovcnt = 1;
	uio.uio_segflg = UIO_USERSPACE;
	uio.uio_resid = up->u_count;
	uio.uio_offset = 0;

	iov.iov_base = up->u_base;
	iov.iov_len = up->u_count;

	if (rw == UIO_READ)
		u.u_error = soreceive(soc_fsptr(ip), 0, &uio, 0, 0);
	else
		u.u_error = sosend(soc_fsptr(ip), 0, &uio, 0, 0);
	up->u_count = uio.uio_resid;
}

/*
 * called to read from a socket
 */
soc_readi(ip)
struct inode *ip;
{
	soc_rdwr(ip,UIO_READ);
}

/*
 * called to write to a socket
 */
soc_writei(ip)
struct inode *ip;
{
	soc_rdwr(ip,UIO_WRITE);
}

/*
 * find out about a file
 */
soc_statf(ip, st)
register struct inode *ip;
register struct stat *st;
{
	register struct com_inode *ci;

	ci = com_fsptr(ip);
	/* tell users we are a FIFO/pipe so that they will not think we are a
	 * regular file.  This is to make 'cat' not complain that its input
	 * and output files are the same. */
	st->st_mode = (ci->ci_mode & ~S_IFMT) | S_IFIFO;
	st->st_atime = ci->ci_atime;
	st->st_mtime = ci->ci_mtime;
	st->st_ctime = ci->ci_ctime;
}

/*
 * Get filesystem status.
 *
 * XXX For now, just punt.  Someday, this should be like the pipe statfs.
 */
/*ARGSUSED*/
soc_statfs(ip, statp, fstyp)
struct inode *ip;
struct statfs *statp;
short fstyp;
{
	if (ip->i_fstyp != soc_fstyp) {
		u.u_error = EINVAL;
		return;
	}
	statp->f_fstyp = soc_fstyp;
	statp->f_bsize = 0;		/* XXX what is a good value here? */
	statp->f_frsize = 0;
	statp->f_blocks = 0;		/* XXX this is wrong */
	statp->f_bfree = 0;
	statp->f_files = 0;		/* XXX wrong again */
	statp->f_ffree = 0;
	strncpy(&statp->f_fname[0], "socket", sizeof(statp->f_fname));
	strncpy(&statp->f_fpack[0], "core", sizeof(statp->f_fpack));
}

/*
 * do IOCTLs for sockets
 */
/* ARGSUSED */
soc_ioctl(ip, cmd, addr, flag)
register struct inode *ip;
int cmd;
caddr_t addr;
int flag;
{
	register struct socket *so = soc_fsptr(ip);
	register u_int size;
	char data[IOCPARM_MASK+1];

	/*
	 * Interpret high order word to find
	 * amount of data to be copied to/from the user address space.
	 */
	size = (cmd &~ (IOC_INOUT|IOC_VOID)) >> 16;
	if (size > sizeof (data)) {
		u.u_error = EFAULT;
		return;
	}
	if (cmd&IOC_IN) {
		if (size != 0) {
			u.u_error = copyin(addr, (caddr_t)data, (u_int)size);
			if (u.u_error)
				return;
		} else
			*(caddr_t *)data = addr;
	} else if ((cmd&IOC_OUT) && size != 0) {
		/*
		 * Zero the buffer on the stack so the user
		 * always gets back something deterministic.
		 */
		bzero((caddr_t)data, size);
	} else if (cmd&IOC_VOID) {
		*(caddr_t *)data = addr;
	}

	switch (cmd) {

	case FIONBIO:
		if (*(int *)data)
			so->so_state |= SS_NBIO;
		else
			so->so_state &= ~SS_NBIO;
		break;

	case FIOASYNC:
		if (*(int *)data)
			so->so_state |= SS_ASYNC;
		else
			so->so_state &= ~SS_ASYNC;
		break;

	case FIONREAD:
		*(int *)data = so->so_rcv.sb_cc;
		break;

	case FIOSETOWN:
	case SIOCSPGRP:
		so->so_pgrp = *(int *)data;
		break;

	case FIOGETOWN:
	case SIOCGPGRP:
		*(int *)data = so->so_pgrp;
		break;

	case SIOCATMARK:
		*(int *)data = (so->so_state&SS_RCVATMARK) != 0;
		break;

	default:
		/*
		 * Interface/routing/protocol specific ioctls:
		 * interface and routing ioctls should have a
		 * different entry since a socket is unnecessary
		 */
#define	cmdbyte(x)	(((x) >> 8) & 0xff)
		if (cmdbyte(cmd) == 'i')
			u.u_error = (ifioctl(so, cmd, data));
		else if (cmdbyte(cmd) == 'r')
			u.u_error = (rtioctl(cmd, data));
		else
			u.u_error = ((*so->so_proto->pr_usrreq)(so, PRU_CONTROL,
							  (struct mbuf *)cmd,
							  (struct mbuf *)data,
							  (struct mbuf *)0));
	}

	/*
	 * Copy any data to user, size was
	 * already set and checked above.
	 */
	if (u.u_error == 0 && (cmd&IOC_OUT) && size)
		u.u_error = copyout(data, addr, (u_int)size);
}


/*
 * Socket file control
 */
/* ARGSUSED */
soc_fcntl(ip, cmd, arg, flag, offset)
struct inode *ip;
int cmd, arg, flag;
off_t offset;
{
	register struct socket *so = soc_fsptr(ip);

	switch (cmd) {
	case F_CHKFL:
		break;

	case F_GETOWN:
		u.u_rval1 = so->so_pgrp;
		break;

	case F_SETOWN:
		so->so_pgrp = arg;
		break;

	default:
		u.u_error = EINVAL;
	}
}


int					/* 1=got something */
soc_select(so, which)
struct socket *so;
int which;
{
	register int s = splnet();

	switch (which) {

	case FREAD:
		if (soreadable(so)) {
			splx(s);
			return (1);
		}
		sbselqueue(&so->so_rcv);
		break;

	case FWRITE:
		if (sowriteable(so)) {
			splx(s);
			return (1);
		}
		sbselqueue(&so->so_snd);
		break;

	case 0:
		if (so->so_oobmark ||
		    (so->so_state & SS_RCVATMARK)) {
			splx(s);
			return (1);
		}
		sbselqueue(&so->so_rcv);
		break;
	}
	splx(s);
	return (0);
}


#ifdef SVR3
/* various file system stubs 
 */

#include "sys/nami.h"

soc_iput()
{
}

soc_iupdat(ip, ta, tm)
register struct inode *ip;
time_t *ta, *tm;
{
	com_iupdat(ip, ta, tm);
}

int
soc_access(ip, mode)
register struct inode *ip;
register int mode;
{
	return com_access(ip,mode);
}

long
soc_notify(ip, noargp)
register struct inode *ip;
register struct argnotify *noargp;
{
	ASSERT(noargp != NULL);
	switch ((int) noargp->cmd) {
	case NO_SEEK:
		if (noargp->data1 < 0)
			u.u_error = EINVAL;
		return noargp->data1;
		break;
	}
	return 0L;
}
#endif
