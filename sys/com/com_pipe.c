/*
 * Pipe management.  There are two kinds of pipes:
 *	- those that are permanent on the disk (named pipes)
 *	- those that are created via the pipe() system call
 *
 * Both kinds use the filesystem switch to call functions in this module.
 * Named pipes are manipulated in disk filesystem implementations, accessed
 * via the switch.  Those implementations may call pipefstyp's fstypsw entries
 * to get and update the common pipe state (struct pipe_inode, cf. inode.h).
 * These named pipe operations indirect twice through the switch.
 *
 * Unnamed pipes are operated upon directly through pipefstyp's switch entries.
 *
 * $Source: /d2/3.7/src/sys/com/RCS/com_pipe.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:26:44 $
 */
#include "../h/param.h"
#include "../h/systm.h"
#include "../h/user.h"
#include "../h/conf.h"
#include "../h/fstyp.h"
#include "../h/inode.h"
#include "../h/mount.h"
#include "../h/nami.h"
#include "../h/proc.h"
#include "../h/buf.h"
#include "../h/file.h"
#include "../h/statfs.h"
#include "../com/pipe_inode.h"

/* # of basic blocks in use by pipes */
long	pipe_blocks = 0;

void	pbwait(), pbwakeup();

/*
 * Allocate an inode for a clone device or for an incore pipe (as opposed to
 * a named pipe).
 */
struct inode *
com_getinode(mp, cmd, rdev)
	register struct mount *mp;
	register int cmd;
	int rdev;
{
	register struct inode *ip;
	extern short pipe_files;

	ip = getinode(mp, com_fstyp, NODEV);
	if (ip == NULL)
		return NULL;
	ip->i_number = (ip - inode) + 10000;
	ip->i_size = 0;
	ASSERT((cmd == FSG_PIPE) || (cmd == FSG_CLONE));
	if (cmd == FSG_PIPE) {
		register struct pipe_inode *pi;

		pi = (struct pipe_inode *) calloc(1, sizeof *pi);
		if (!pi) {
			printf("out of memory making pipe");
			iput(ip);
			return NULL;
		}
		ip->i_fsptr = (char *)pi;
		com_isetup(ip, IFIFO);
		pi->pi_rb.pb_ref = 1;
		pi->pi_wb.pb_ref = 1;
		pi->pi_com.ci_magic = PIPE_MAGIC;
	} else if (cmd == FSG_CLONE) {
		register struct com_inode *ci;

		ci = (struct com_inode *) calloc(1, sizeof *ci);
		if (!ci) {
			printf("out of memory cloning");
			iput(ip);
			return NULL;
		}
		ip->i_fsptr = (char *)ci;
		com_isetup(ip, IFCHR);
		ci->ci_magic = COM_MAGIC;
		ip->i_rdev = rdev;
	}
	pipe_files++;
	return (ip);
}

/*
 * Open a pipe.  Check read and write counts, delay as necessary.
 */
void
pipe_openi(ip, mode)
	register struct inode *ip;
	register int mode;
{
	register struct pipe_inode *pi;

	ASSERT(ip->i_ftype == IFIFO);
	pi = pipe_fsptr(ip);
	ASSERT(pi->pi_com.ci_magic == PIPE_MAGIC);
	if (mode & FREAD) {
		if (pi->pi_rb.pb_ref++ == 0)
			wakeup((caddr_t)&pi->pi_rb.pb_ref);
	}
	if (mode & FWRITE) {
		if ((mode & FNDELAY) && (pi->pi_rb.pb_ref == 0)) {
			u.u_error = ENXIO;
			return;
		}
		if (pi->pi_wb.pb_ref++ == 0)
			wakeup((caddr_t)&pi->pi_wb.pb_ref);
	}
	if (mode & FREAD) {
		while (pi->pi_wb.pb_ref == 0) {
			if ((mode & FNDELAY) || ip->i_size)
				return;
			(void) sleep((caddr_t)&pi->pi_wb.pb_ref, PPIPE);
		}
	}
	if (mode & FWRITE) {
		while (pi->pi_rb.pb_ref == 0)
			(void) sleep((caddr_t)&pi->pi_rb.pb_ref, PPIPE);
	}
}

/*
 * Close a pipe.  Update counts and cleanup.
 */
void
pipe_closei(ip, mode)
	register struct inode *ip;
	register int mode;
{
	register struct pipe_inode *pi;

	ASSERT(ip->i_ftype == IFIFO);
	pi = pipe_fsptr(ip);
	ASSERT(pi->pi_com.ci_magic == PIPE_MAGIC);
	if ((mode & FREAD) && --pi->pi_rb.pb_ref == 0)
		pbwakeup(&pi->pi_rb);
	if ((mode & FWRITE) && --pi->pi_wb.pb_ref == 0)
		pbwakeup(&pi->pi_wb);
	if ((pi->pi_rb.pb_ref == 0) && (pi->pi_wb.pb_ref == 0)) {
		if (pi->pi_bp) {
			_RANGEOFP(pi->pi_bp, sizeof(*pi->pi_bp), buf,
			    nbuf, "buf");
			pipe_blocks -= pi->pi_bp->b_length;
			brelse(pi->pi_bp);
			pi->pi_bp = NULL;
		}
		pi->pi_rb.pb_ptr = 0;
		pi->pi_wb.pb_ptr = 0;
		ip->i_size = 0;
		ip->i_flag |= IUPD|ICHG;
	}
}

/*
 * Construct a pipe_inode and initialize its members.  Return true on
 * success, false on failure due to memory shortage.  Used by filesystems
 * which use the common filesystem.
 */
int
pipe_create(ip)
	struct inode *ip;
{
	register struct pipe_inode *pi;

	ASSERT(ip->i_fsptr == NULL);
	pi = (struct pipe_inode *) calloc(1, sizeof *pi);
	if (pi == NULL)
		return 0;
	pi->pi_com.ci_mode = IFIFO;
	pi->pi_com.ci_magic = PIPE_MAGIC;
	ip->i_fsptr = (char *) pi;
	ip->i_size = 0;
	return 1;
}

/*
 * pipe_readi:
 *	- called to read from a pipe or from a cloned IFCHR inode
 */
void
pipe_readi(ip)
	register struct inode *ip;
{
	register struct pipe_inode *pi;
	register struct user *up;
	register struct buf *bp;
	register unsigned n;

	ASSERT(ip->i_ftype == IFIFO);
	pi = pipe_fsptr(ip);
	ASSERT(pi->pi_com.ci_magic == PIPE_MAGIC);
	up = &u;

	/*
	 * Wait for data to arrive in the pipe from a writer
	 */
	while (ip->i_size == 0) {
		/*
		 * Make sure there is at least one writer.  Otherwise, return
		 * end of file.  If the user won't allow a delay, return
		 * immediately.
		 */
		if ((pi->pi_wb.pb_ref == 0) || (up->u_fmode & FNDELAY))
			return;
		pbwait(&pi->pi_wb, ip);
	}

	if ((bp = pi->pi_bp) == NULL) {
		register int length = BTOBB(PIPE_SIZE);

		pi->pi_bp = bp = geteblk(length);
		_RANGEOFP(bp, sizeof(*bp), buf, nbuf, "buf");
		pipe_blocks += length;
	} else {
		_RANGEOFP(bp, sizeof(*bp), buf, nbuf, "buf");
		bio_allockmap(bp);
	}

	do {
		/*
		 * Bound users i/o request against (1) the amount of data
		 * actually in the pipe, and (2) the amount we can read
		 * before wrapping around.
		 */
		n = up->u_count;
		if (n > ip->i_size)
			n = ip->i_size;
		if (n > PIPE_SIZE - pi->pi_rb.pb_ptr)
			n = PIPE_SIZE - pi->pi_rb.pb_ptr;
		if (n) {
			_RANGEOFP(bp, sizeof(*bp), buf, nbuf, "buf");
			iomove(bp->b_un.b_addr + pi->pi_rb.pb_ptr, (int)n, B_READ);
			ip->i_size -= n;
		}
		/*
		 * Wrap the read pointer around, if its past the
		 * end of the buffer.
		 */
		pi->pi_rb.pb_ptr += n;
		if (pi->pi_rb.pb_ptr >= PIPE_SIZE)
			pi->pi_rb.pb_ptr = 0;
		ip->i_flag |= IACC;
	} while (!up->u_error && up->u_count && n);

	_RANGEOFP(bp, sizeof(*bp), buf, nbuf, "buf");
	bio_freekmap(bp);
	pbwakeup(&pi->pi_rb);
}

/*
 * pipe_write:
 *	- called to write to a pipe or cloned IFCHR file
 */
void
pipe_writei(ip)
	register struct inode *ip;
{
	register struct user *up;
	register struct pipe_inode *pi;
	register struct buf *bp;
	register unsigned n;
	register unsigned int usave;

	ASSERT(ip->i_ftype == IFIFO);
	pi = pipe_fsptr(ip);
	ASSERT(pi->pi_com.ci_magic == PIPE_MAGIC);
	up = &u;

floop:
	/*
	 * Respect u_limit bound on inode size.
	 */
	if ((BTOBB(ip->i_size + up->u_count) >= up->u_limit)) {
		up->u_error = EFBIG;
		return;
	}
	/*
	 * Check and see if the users write request will fit in the pipe
	 * in one piece.  If not, wait until there is room.  If the request
	 * is too large to ever fit in the pipe (> PIPE_SIZE) then break
	 * the request in to smaller pieces.
	 */
	usave = 0;
	while ((up->u_count + ip->i_size) > PIPE_SIZE) {
		if (pi->pi_rb.pb_ref == 0)		/* insure at least 1 reader */
			break;
		/*
		 * If users i/o request is larger than amount we can buffer
		 * in the pipe and there is more room in the pipe, truncate
		 * the write request.  If we loop around before a reader reads,
		 * then we will hang up below and wait for the reader to read.
		 */
		if ((up->u_count > PIPE_SIZE) && (ip->i_size < PIPE_SIZE)) {
			usave = up->u_count;
			up->u_count = PIPE_SIZE - ip->i_size;
			usave -= up->u_count;
			break;
		}
		if (up->u_fmode & FNDELAY)
			return;
		/*
		 * Pipe can't fit the users i/o request (and its smaller
		 * than PIPE_SIZE).  Wait here until the pipe drains.
		 */
		pbwait(&pi->pi_rb, ip);
	}

	/*
	 * Make sure there is at least one reader.  If not, signal
	 * the writer with a SIGPIPE and return EPIPE.
	 */
	if (pi->pi_rb.pb_ref == 0) {
		psignal(up->u_procp, SIGPIPE);
		up->u_error = EPIPE;
		return;
	}

	if ((bp = pi->pi_bp) == NULL) {
		register int length = BTOBB(PIPE_SIZE);

		pi->pi_bp = bp = geteblk(length);
		_RANGEOFP(bp, sizeof(*bp), buf, nbuf, "buf");
		pipe_blocks += length;
	} else {
		_RANGEOFP(bp, sizeof(*bp), buf, nbuf, "buf");
		bio_allockmap(bp);
	}

	while (!up->u_error && up->u_count) {
		/*
		 * Bound users i/o request against (1) the amount of room
		 * left in the pipe, and (2) the amount we can write
		 * before wrapping around.
		 */
		n = up->u_count;
		if (n > PIPE_SIZE - pi->pi_wb.pb_ptr)
			n = PIPE_SIZE - pi->pi_wb.pb_ptr;
		if (n) {
#ifdef	OS_ASSERT
			if (pi->pi_wb.pb_ptr + n > PIPE_SIZE) {
				printf("ip=%x i_size=%d n=%d wptr=%d\n",
					      ip, ip->i_size, n, pi->pi_wb.pb_ptr);
				debug("pipe_writei");
			}
#endif
			_RANGEOFP(bp, sizeof(*bp), buf, nbuf, "buf");
			iomove(bp->b_un.b_addr + pi->pi_wb.pb_ptr, (int)n, B_WRITE);
			ip->i_size += n;
		}
		/*
		 * Wrap the write pointer back to the beginning
		 */
		pi->pi_wb.pb_ptr += n;
		if (pi->pi_wb.pb_ptr >= PIPE_SIZE)
			pi->pi_wb.pb_ptr = 0;
	}
	_RANGEOFP(bp, sizeof(*bp), buf, nbuf, "buf");
	bio_freekmap(bp);
	ip->i_flag |= IUPD|ICHG;
	pbwakeup(&pi->pi_wb);

	/*
	 * Continue writing in case user is reading a large amount
	 */
	if (!up->u_error && (usave != 0)) {
		up->u_count = usave;
		goto floop;
	}
}

int
pipe_poll(ip, eventp)
	register struct inode *ip;
	register short *eventp;
{
	register short events = *eventp;
	register struct pipe_inode *pi = pipe_fsptr(ip);

	events &= POLLIN|POLLOUT;
	if ((events & POLLIN) && ip->i_size == 0) {
		events &= ~POLLIN;
		pollqueue(&pi->pi_rb.pb_pq, u.u_procp);
	}
	if ((events & POLLOUT)
	    && (pi->pi_rb.pb_ref == 0 || ip->i_size == PIPE_SIZE)) {
		events &= ~POLLOUT;
		if (pi->pi_rb.pb_ref == 0) {
			events |= POLLHUP;
		}
		pollqueue(&pi->pi_wb.pb_pq, u.u_procp);
	}
	*eventp = events;
	return events != 0;
}

static void
pbwait(pb, ip)
	register struct pipe_buffer *pb;
	register struct inode *ip;
{
	pb->pb_flags |= PBWAIT;
	iunlock(ip);
	(void) sleep((caddr_t)pb, PPIPE);
	ilock(ip);
}

static void
pbwakeup(pb)
	register struct pipe_buffer *pb;
{
	pollwakeup(&pb->pb_pq);
	if (pb->pb_flags & PBWAIT) {
		pb->pb_flags &= ~PBWAIT;
		wakeup((caddr_t) pb);
	}
}

/* ARGSUSED */
com_statfs(ip, f, ufstyp)
	struct inode *ip;
	struct statfs *f;
	int ufstyp;
{
	extern short pipe_files;	/* # of pipes/clones in use */

	if (ip->i_fstyp != pipefstyp) {
		ASSERT(ufstyp == pipefstyp);
		u.u_error = EINVAL;	/* vectored via ufstyp */
		return;
	}
	f->f_fstyp = pipefstyp;
	f->f_bsize = PIPE_SIZE;
	f->f_frsize = 0;
	f->f_blocks = pipe_blocks;
	f->f_bfree = 0;
	f->f_files = pipe_files;
	f->f_ffree = 0;
	strncpy(f->f_fname, "pipe", sizeof f->f_fname);
	strncpy(f->f_fpack, "kmem", sizeof f->f_fpack);
}
