/*
 * $Source: /d2/3.7/src/sys/sys/RCS/sys2.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:35:36 $
 */
#include "../h/param.h"
#include "../h/systm.h"
#include "../h/user.h"
#include "../h/buf.h"
#include "../h/fcntl.h"
#include "../h/file.h"
#include "../h/flock.h"
#include "../h/fstyp.h"
#include "../h/inode.h"
#include "../h/nami.h"
#include "../h/proc.h"
#include "../h/sysinfo.h"

/*
 * read system call
 */
read()
{
	sysinfo.sysread++;
	rdwr(FREAD);
}

/*
 * write system call
 */
write()
{
	sysinfo.syswrite++;
	rdwr(FWRITE);
}

/*
 * common code for read and write calls:
 * check permissions, set base, count, and offset,
 * and switch out to readi/writei
 */
rdwr(mode)
	register int mode;
{
	register struct user *up;
	register struct file *fp;
	register struct inode *ip;
	register struct a {
		int	fdes;
		char	*cbuf;
		unsigned count;
	} *uap;
	register int type;
	register uint ulimit;

	up = &u;
	uap = (struct a *)up->u_ap;
	if ((fp = getf(uap->fdes)) == NULL)
		return;
	if ((fp->f_flag&mode) == 0) {
		up->u_error = EBADF;
		return;
	}
	up->u_base = (caddr_t)uap->cbuf;
	up->u_count = uap->count;
	up->u_segflg = 0;
	up->u_fmode = fp->f_flag;
	ip = fp->f_inode;
	type = ip->i_ftype;
	if ((type == IFREG) || (type == IFDIR)) {
		ILOCK(ip);
		if ((up->u_fmode & FAPPEND) && (mode == FWRITE))
			fp->f_offset = ip->i_size;

		/*	Make sure that the user can read all the way up
		**	to the ulimit value.
		*/

		ulimit = (uint)(u.u_limit << BBSHIFT) - fp->f_offset;
		if(type == IFREG  &&  mode == FWRITE
		   && ulimit < uap->count  &&  ulimit > 0)
			uap->count = u.u_count = ulimit;
	} else if (type == IFIFO) {
		ILOCK(ip);
		fp->f_offset = 0;
	}
	up->u_offset = fp->f_offset;
	if (mode == FREAD)
		FS_READI(ip);
	else
		FS_WRITEI(ip);
	if (u.u_error != 0) {
		if (type == IFREG || type == IFDIR || type == IFIFO)
			IUNLOCK(ip);
		return;
	}
	if (type==IFREG) {
		/* If synchronous write, update inode now. */
		if ((u.u_fmode&FSYNC) && (mode == FWRITE)) {
			ip->i_flag |= ISYN;
			FS_IUPDAT(ip, &time, &time);
		}
		IUNLOCK(ip);
	} else if (type==IFDIR || type==IFIFO)
		IUNLOCK(ip);
	fp->f_offset += uap->count - up->u_count;

	up->u_rval1 = uap->count - up->u_count;
	u.u_ioch += (unsigned)u.u_rval1;
	if (mode == FREAD)
		sysinfo.readch += (unsigned)up->u_rval1;
	else
		sysinfo.writech += (unsigned)up->u_rval1;
}

/*
 * open system call
 */
open()
{
	register struct a {
		char	*fname;
		int	mode;
		int	crtmode;
	} *uap;

	uap = (struct a *)u.u_ap;
	copen(uap->mode-FOPEN, uap->crtmode);
}

/*
 * creat system call
 */
creat()
{
	struct a {
		char	*fname;
		int	fmode;
	} *uap;

	uap = (struct a *)u.u_ap;
	copen(FWRITE|FCREAT|FTRUNC, uap->fmode);
}

/*
 * common code for open and creat.
 * Check permissions, allocate an open file structure,
 * and call the device open routine if any.
 */
copen(mode, arg)
register int mode, arg;
{
	register struct inode *ip;
	struct argnamei nmarg;

	arg &= ~ISVTX;				/* lose this bit from now on */
	if ((mode&(FREAD|FWRITE)) == 0) {
		u.u_error = EINVAL;
		return;
	}
	if (mode&FCREAT) {
		nmarg.cmd = (mode&FEXCL) ? NI_XCREAT : NI_CREAT;
		nmarg.mode = arg & MODEMSK;
		nmarg.ftype = 0;
		nmarg.idev = NODEV;
		ip = namei(USERPATH, &nmarg, FOLLOWLINK);
		if (ip == NULL) {
			return;
		}
		if (nmarg.rcode == FSN_FOUND) {
			mode &= ~FCREAT;
		} else {
			mode &= ~FTRUNC;
		}
	} else {
		ip = namei(USERPATH, NI_LOOKUP, FOLLOWLINK);
		if (ip == NULL) {
			return;
		}
	}
	copen1(ip, mode);
}

copen1(ip, mode)
register struct inode *ip;
register mode;
{
	register struct file *fp;
	register int i;

	/* flush any lingering saved text regions, just in case */
	if ((mode & (FWRITE | FTRUNC)) && (ip->i_flag & IXSAVED))
		(void) xflush(ip);
	if (!(mode&FCREAT)) {
		if (mode&FREAD)
			(void) FS_ACCESS(ip, IREAD);
		if (mode&(FWRITE|FTRUNC)) {
			(void) FS_ACCESS(ip, IWRITE);
			if (ip->i_ftype == IFDIR) {
				u.u_error = EISDIR;
			} else if ((mode&FTRUNC)
				    && (!FS_ACCESS(ip, IMNDLCK))
				    && (ip->i_filocks != NULL)) {
				u.u_error = EAGAIN;
			}
		}
	}
	if (u.u_error || (fp = falloc(ip, mode&FMASK)) == NULL) {
		iput(ip);
		return;
	}
	if (mode&FTRUNC) {
		FS_ITRUNC(ip);
	}
	iunlock(ip);
	i = u.u_rval1;
	if (save(u.u_qsave)) {	/* catch half-opens */
		if (u.u_error == 0) {
			u.u_error = EINTR;
		}
		u.u_ofile[i] = NULL;
		closef(fp);
	} else {
		if (!u.u_error) {
			FS_OPENI(ip, mode);
		}
		if (u.u_error) {
			u.u_ofile[i] = NULL;
			if (--fp->f_count <= 0) {
				fp->f_next = ffreelist;
				ffreelist = fp;
			}
			iunuse(ip);
		}
	}
}

/*
 * close system call
 */
close()
{
	register struct file *fp;
	register struct a {
		int	fdes;
	} *uap;

	uap = (struct a *)u.u_ap;
	fp = getf(uap->fdes);
	if (fp == NULL)
		return;
	u.u_ofile[uap->fdes] = NULL;
	closef(fp);
}

/*
 * seek system call
 */
seek()
{
	register struct file *fp;
	register struct inode *ip;
	struct argnotify noarg;
	register struct a {
		int	fdes;
		off_t	off;
		int	sbase;
	} *uap;

	uap = (struct a *)u.u_ap;
	fp = getf(uap->fdes);
	if (fp == NULL)
		return;
	ip = fp->f_inode;
	if (ip->i_ftype == IFIFO) {
		u.u_error = ESPIPE;
		return;
	}
	if (uap->sbase == 1)
		uap->off += fp->f_offset;
	else if (uap->sbase == 2)
		uap->off += fp->f_inode->i_size;
	else if (uap->sbase != 0) {
		u.u_error = EINVAL;
		psignal(u.u_procp, SIGSYS);
		return;
	}
	if (uap->off < 0) {
		u.u_error = EINVAL;
		return;
	}
	if (fsinfo[ip->i_fstyp].fs_notify & NO_SEEK) {
		noarg.cmd = NO_SEEK;
		noarg.data1 = uap->off;
		noarg.data2 = uap->sbase;
		uap->off = FS_NOTIFY(ip, &noarg);
		if (u.u_error)
			return;
	}
	fp->f_offset = uap->off;
	u.u_roff = uap->off;
}

/*
 * link system call
 */
link()
{
	register struct inode *ip;
	struct argnamei nmarg;
	register struct a {
		char	*target;
		char	*linkname;
	} *uap;

	uap = (struct a *)u.u_ap;
	ip = namei(USERPATH, NI_LOOKUP, DONTFOLLOW);
	if (ip == NULL)
		return;
	if (ip->i_ftype == IFDIR && !suser())
		goto out;

	iunlock(ip);
	u.u_dirp = (caddr_t)uap->linkname;
	nmarg.cmd = NI_LINK;
	nmarg.ip = ip;
	(void) namei(USERPATH, &nmarg, FOLLOWLINK);
	ilock(ip);
out:
	iput(ip);
}

/*
 * mknod system call
 */
mknod()
{
	register struct inode *ip;
	register struct a {
		char	*fname;
		int	fmode;
		int	dev;
	} *uap;
	struct argnamei nmarg;

	uap = (struct a *)u.u_ap;
	if ((uap->fmode&IFMT) != IFIFO && !suser())
		return;
	nmarg.cmd = NI_MKNOD;
	nmarg.mode = uap->fmode & MODEMSK;
	nmarg.ftype = uap->fmode & IFMT;
	nmarg.idev = (dev_t) uap->dev;
	ip = namei(USERPATH, &nmarg, FOLLOWLINK);
	if (ip != NULL) {
		iput(ip);
	}
}

/*
 * access system call
 */
saccess()
{
	register svuid, svgid;
	register struct inode *ip;
	register struct a {
		char	*fname;
		int	fmode;
	} *uap;

	uap = (struct a *)u.u_ap;
	svuid = u.u_uid;
	svgid = u.u_gid;
	u.u_uid = u.u_ruid;
	u.u_gid = u.u_rgid;
	ip = namei(USERPATH, NI_LOOKUP, FOLLOWLINK);
	if (ip != NULL) {
		if (uap->fmode&(IREAD>>6))
			(void) FS_ACCESS(ip, IREAD);
		if (uap->fmode&(IWRITE>>6))
			(void) FS_ACCESS(ip, IWRITE);
		if (uap->fmode&(IEXEC>>6))
			(void) FS_ACCESS(ip, IEXEC);
		iput(ip);
	}
	u.u_uid = svuid;
	u.u_gid = svgid;
}

/*
 * symlink - make a symbolic link
 */
symlink()
{
	struct a {
		char	*text;
		char	*linkname;
	};
	register struct a *uap;
	register char *tp;
	register c, nc;
	struct argnamei nmarg;

	uap = (struct a *)u.u_ap;
	tp = uap->text;
	nc = 0;
	while ((c = fubyte(tp)) != 0) {
		if (c < 0) {
			u.u_error = EFAULT;
			return;
		}
		tp++;
		nc++;
	}
	u.u_dirp = uap->linkname;
	nmarg.cmd = NI_SYMLINK;
	nmarg.mode = 0;		/* links have zero modes for now */
	nmarg.name = uap->text;
	nmarg.namlen = nc;
	nmarg.namseg = 0;	/* UIOSEG_USER */
	(void) namei(USERPATH, &nmarg, DONTFOLLOW);
}

/*
 * Return target name of a symbolic link
 */
readlink()
{
	struct a
	{
		char	*name;
		char	*buf;
		int	count;
	};
	register struct a *uap;
	register struct inode *ip;

	uap = (struct a*)u.u_ap;
	ip = namei(USERPATH, NI_LOOKUP, DONTFOLLOW);
	if (ip == NULL) {
		return;
	}
	switch (ip->i_ftype) {
	case IFLNK:
		break;
	default:
		iput(ip);
		u.u_error = ENXIO;
		return;
	}

	u.u_base = uap->buf;
	u.u_count = uap->count;
	u.u_offset = 0L;
	u.u_segflg = 0;
	FS_READI(ip);

	iput(ip);
	u.u_rval1 = uap->count - u.u_count;
}

/*
 * Truncate a file given its path name.  Transcribed from 4.2 by Herb
 * Kuta, June 1985.
 */
truncate()
{
	struct a {
		char	*fname;
		off_t	length;
	} *uap = (struct a *)u.u_ap;
	struct inode *ip;

	ip = namei(USERPATH, NI_LOOKUP, FOLLOWLINK);
	if (ip == NULL) {
		return;
	}
	if (FS_ACCESS(ip, IWRITE)) {
		goto bad;
	}
	if (ip->i_ftype == IFDIR) {
		u.u_error = EISDIR;
		goto bad;
	}
	if (uap->length < 0) {
		u.u_error = EINVAL;
	} else {
		FS_SETSIZE(ip, uap->length);
	}
bad:
	iput(ip);
}

/*
 * Truncate a file given a file descriptor.  Transcribed from 4.2
 * by Herb Kuta, June 1985.
 */
ftruncate()
{
	struct a {
		int	fd;
		off_t	length;
	} *uap = (struct a *)u.u_ap;
	struct inode *ip;
	struct file *fp;

	fp = getf(uap->fd);
	if (fp == NULL) {
		return;
	}
	if ((fp->f_flag&FWRITE) == 0) {
		u.u_error = EBADF;
		return;
	}

	ip = (struct inode *)fp->f_inode;
	ilock(ip);
	if (uap->length < 0) {
		u.u_error = EINVAL;
	} else {
		FS_SETSIZE(ip, uap->length);
	}
	iunlock(ip);
}

/*
 * mkdir system call
 */
mkdir()
{
	register struct a {
		char *fname;
		int fmode;
	} *uap;
	struct argnamei nmarg;

	uap = (struct a *)u.u_ap;
	nmarg.cmd = NI_MKDIR;
	nmarg.mode = (uap->fmode & PERMMSK);
	(void) namei(USERPATH, &nmarg, FOLLOWLINK);
}

/*
 * rmdir system call
 */
rmdir()
{	struct a {
		char *fname;
	};
	struct argnamei nmarg;

	nmarg.cmd = NI_RMDIR;
	(void) namei(USERPATH, &nmarg, DONTFOLLOW);
}

/*
 * getdents system call
 */
getdents()
{
	register struct inode *ip;
	register struct file *fp;
	int num;
	struct a {
		int fd;
		char *buf;
		int nbytes;
	} *uap;

	uap = (struct a *)u.u_ap;
	if ((fp = getf(uap->fd)) == NULL)
		return;
	ip = fp->f_inode;
	if (ip->i_ftype != IFDIR) {
		u.u_error = ENOTDIR;
		return;
	}
	u.u_offset = fp->f_offset;
	u.u_segflg = 0;		/* UIOSEG_USER */
	ilock(ip);
	num = FS_GETDENTS(ip, uap->buf, uap->nbytes);
	iunlock(ip);
	if (num > 0) {
		fp->f_offset = u.u_offset;
	}
	u.u_rval1 = num;
}

/*
 * Rename system call.
 */
rename()
{
	register struct args {
		char	*frompath;	/* source pathname */
		char	*topath;	/* target pathname */
	} *uap;
	struct pathname pn;
	auto struct inode *fdp;
	auto struct inode *fip;
	struct argnamei nmarg;

	uap = (struct args *) u.u_ap;
	u.u_error = pathname(uap->frompath, USERPATH, &pn);
	if (u.u_error != 0) {
		return;		/* error copying in pathname */
	}
	u.u_error = pn_lookup(&pn, NI_LOOKUP, DONTFOLLOW, &fdp, &fip);
	if (u.u_error == 0) {
		/*
		 * Unlock source inode so that both fip and its parent fdp
		 * are referenced but unlocked.
		 */
		iunlock(fip);
		nmarg.cmd = NI_RENAME;
		nmarg.dp = fdp;
		nmarg.name = pn.pn_component;
		nmarg.namlen = pn.pn_complen;
		nmarg.ip = fip;
		u.u_dirp = uap->topath;
		(void) namei(USERPATH, &nmarg, DONTFOLLOW);
		iunuse(fip);
		iunuse(fdp);
	}
	pn_free(&pn);
}
