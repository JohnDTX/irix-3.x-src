/*
 * Unix directory implementation.
 *
 * $Source: /d2/3.7/src/sys/bfs/RCS/bfs_dir.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:26:17 $
 */
#include "bfs.h"
#include "efs.h"
#if NBFS > 0 || NEFS > 0

#include "../h/param.h"
#include "../h/systm.h"
#include "../h/sysinfo.h"
#include "../h/user.h"
#include "../h/buf.h"
#include "../h/dir.h"
#include "../h/dirent.h"
#include "../h/file.h"
#include "../h/fstyp.h"
#include "../h/inode.h"
#include "../h/nami.h"
#include "../com/com_pncc.h"

/* the maximum buffer size to use for getdents() externalization */
#define	MAXBUFSIZE	4096

/*
 * An abstraction to make directory lookup code simpler and more maintainable.
 * The OPENDIR() macro initializes a directory entry stream while CLOSEDIR()
 * finalizes the stream after it's exhausted.
 */
struct dirstream {
	struct inode	*inode;		/* directory inode */
	off_t		offset;		/* offset of next read */
	off_t		count;		/* bytes unread in directory */
	struct buf	*buffer;	/* current buffer */
	struct direct	*next;		/* next entry to get */
	short		entries;	/* entries remaining */
	short		error;		/* error status */
};

#define	OPENDIR(dp, off, cnt, ds) \
	((ds)->inode = (dp), (ds)->offset = (off), (ds)->count = (cnt), \
	 (ds)->buffer = NULL, (ds)->next = NULL, (ds)->entries = 0, \
	 (ds)->error = 0)

#define	CLOSEDIR(ds) \
	(((ds)->buffer != NULL) ? brelse((ds)->buffer) : 0)

/*
 * Get a pointer to the next directory entry in ds.  Put back a dent that we
 * got but can't deal with now.
 */
#define	GETDENT(ds) \
	(--(ds)->entries >= 0 ? (ds)->next++ : getmoredents(ds))
#define	UNGETDENT(ds) \
	((ds)->entries++, --(ds)->next)

/*
 * Functions to fill a dirstream buffer with new entries.  To read, modify,
 * and write entries (possibly growing the directory), call getdentstoput(),
 * modify the entries via the returned pointer, and call putdents().
 */
struct direct	*getmoredents(/* ds */);
struct direct	*getdentstoput(/* dp, offset, numdents, ds */);
int		putdents(/* ds */);

/*
 * We make use of the fact that old-style directory entries are 16 bytes
 * long to perform reduction in strength (the compiler should do this).
 */
#define	DENTSHIFT		4	/* log2(sizeof(struct direct)) */
#define	DENTSIZE		(1<<DENTSHIFT)
#define	DENTMASK		(DENTSIZE-1)
#define	dentstobytes(nd)	((nd)<<DENTSHIFT)
#define	bytestodents(nb)	((nb)>>DENTSHIFT)

/*
 * Tell the offset of the last dent got and of the next one to be got.
 */
#define LASTDENTPOS(ds) \
	((ds)->offset - dentstobytes((ds)->entries + 1))
#define TELLDIR(ds) \
	((ds)->offset - dentstobytes((ds)->entries))

/*
 * Read dp's entries starting at u.u_offset and translate them into bufsize
 * bytes worth of struct dirents starting at buf.  Minimize bufsize against
 * MAXBUFSIZE first so geteblk doesn't hang on an impossibly large request.
 * Enhanced for NFS to check u.u_segflg to find buf's address space.
 */
int
bell_getdents(dp, buf, bufsize)
	struct inode *dp;
	caddr_t buf;
	register int bufsize;
{
	struct dirstream dir;
	register struct dirent *dep;	/* externalized dent ptr */
	register struct direct *ep;	/* efs-internal dent ptr */
	register int bytesleftinbuf;
	register struct buf *bp;

	/*
	 * Check that the user's i/o offset is aligned properly.
	 */
	if (u.u_offset % DENTSIZE != 0) {  
		u.u_error = EINVAL;
		return -1;
	}

	/*
	 * Get some memory to fill with fs-independent versions of the
	 * requested entries.
	 */
	if (u.u_segflg == 1) {		/* UIOSEG_KERNEL */
		bp = NULL;
		dep = (struct dirent *) buf;
	} else {
		if (bufsize > MAXBUFSIZE)
			bufsize = MAXBUFSIZE;
		bp = geteblk(BTOBB(bufsize));
		dep = (struct dirent *) bp->b_un.b_addr;
	}
	bytesleftinbuf = bufsize;

	/*
	 * Now instantiate a dirstream and read from it, translating its
	 * unix-style directory entries into dirents in dep.
	 */
	OPENDIR(dp, u.u_offset, dp->i_size - u.u_offset, &dir);
	while ((ep = GETDENT(&dir)) != NULL) {
		register long inum;
		register int len, reclen;

		inum = ep->d_ino;
		if (inum == 0)
			continue;	/* skip free entries */

		/*
		 * Calculate name length and, from it, record length.  Check
		 * whether this record will fit in buffer.
		 */
		len = (ep->d_name[DIRSIZ-1] == '\0') ? strlen(ep->d_name)
			: DIRSIZ;
		reclen = DIRENTSIZE(len);
		if (reclen > bytesleftinbuf) {
			/*
			 * Check whether we've copied out at least one entry.
			 * If not, tell the user that bufsize was too small.
			 */
			if (bytesleftinbuf == bufsize) {
				u.u_error = EINVAL;
				bufsize = -1;
			}
			UNGETDENT(&dir);
			break;	/* filled buffer */
		}

		/*
		 * There's room for this entry: fill it in and advance.
		 * Note that the name is 0-terminated, and the DIRENTSIZE
		 * macro takes account of this.
		 */
		dep->d_ino = inum;
#ifdef DIROFFBUG
		dep->d_off = LASTDENTPOS(&dir);
#else
		dep->d_off = TELLDIR(&dir);
#endif
		dep->d_reclen = reclen;
		bcopy(ep->d_name, dep->d_name, len);
		dep->d_name[len] = '\0';

		dep = (struct dirent *)((char *) dep + reclen);
		bytesleftinbuf -= reclen;
	}

	if (dir.error) {	/* there was an i/o error in GETDENT() */
		u.u_error = dir.error;
		bufsize = -1;
	} else if (bufsize >= 0) {
		u.u_offset = TELLDIR(&dir);
		bufsize -= bytesleftinbuf;
		ASSERT(bufsize >= 0);
		if (bufsize > 0 && bp != NULL	/* UIOSEG_USER */
		    && copyout(bp->b_un.b_addr, buf, bufsize)) {
			u.u_error = EFAULT;
			bufsize = -1;
		}
	}
	if (bp != NULL)		/* UIOSEG_USER */
		brelse(bp);
	CLOSEDIR(&dir);
	return bufsize;
}

/*
 * Look for an entry in dp with the given component name and return an
 * fs-independent descriptor for the entry.  If name was found, then
 * dlr->dlr_inum will be non-zero, otherwise zero.
 */
/* ARGSUSED */
int
bell_lookup(dp, name, namlen, startpos, dlr)
	struct inode *dp;
	register char *name;
	unsigned short namlen;
	register off_t startpos;
	register struct dirlookupres *dlr;
{
	struct dirstream dir;
	register struct direct *ep;
	register off_t freepos;

	/*
	 * Loop over the directory's entries.  If we find an entry for name,
	 * fill in *dlr and return.  As we scan, we keep track of the first
	 * free slot offset in freepos.
	 */
	startpos &= ~DENTMASK;
	OPENDIR(dp, startpos, dp->i_size, &dir);
	freepos = dp->i_size;
	while ((ep = GETDENT(&dir)) != NULL) {
		register off_t pos;

		if (ep->d_ino == 0) {
			pos = LASTDENTPOS(&dir);
			if (pos < freepos)
				freepos = pos;
			continue;
		}
		if (!strncmp(ep->d_name, name, DIRSIZ)) {
			pos = LASTDENTPOS(&dir);
			dlr->dlr_inum = ep->d_ino;
			dlr->dlr_offset = pos;
			CLOSEDIR(&dir);
			return 0;
		}
	}

	/*
	 * We couldn't find name in dp.  Set *dlr to describe the free slot
	 * found during this scan and return.
	 */
	dlr->dlr_inum = 0;
	dlr->dlr_offset = freepos;
	CLOSEDIR(&dir);
	return dir.error;
}

/*
 * Initialize dp with directory entries for itself and pinum, its parent's
 * i-number.  Used by mkdir and rename code in fs_namei.  Rename uses this
 * hook to re-write ".." in a directory being moved from one directory to
 * another.
 */
int
bell_dirinit(dp, pinum)
	register struct inode *dp;
	ino_t pinum;
{
	struct dirstream dir;
	register struct direct *ep;

	ep = getdentstoput(dp, 0, 2, &dir);
	if (ep == NULL)
		return dir.error;
	ASSERT(dir.entries == 2);
	ASSERT(dp->i_size == 0 || ep->d_ino == dp->i_number);
	ep->d_ino = dp->i_number;
	strncpy(ep->d_name, dot, DIRSIZ);
	ep++;
	ep->d_ino = pinum;
	strncpy(ep->d_name, dotdot, DIRSIZ);
	return putdents(&dir);
}

/*
 * Create a new entry described by vep in directory dp.
 */
/* ARGSUSED */
int
bell_enter(dp, name, namlen, dlr)
	register struct inode *dp;
	register char *name;
	unsigned short namlen;
	register struct dirlookupres *dlr;
{
	struct dirstream dir;
	register struct direct *ep;

	ep = getdentstoput(dp, dlr->dlr_offset, 1, &dir);
	if (ep == NULL)
		return dir.error;
	ASSERT(dir.entries == 1);
	if (ep->d_ino != 0 && strncmp(ep->d_name, name, DIRSIZ)) {
		printf("directory %x/%d corrupted: ", dp->i_dev, dp->i_number);
		printf("enter at offset %d of %s:%d on top of %s:%d\n",
			dlr->dlr_offset, name, dlr->dlr_inum, ep->d_name,
			ep->d_ino);
		CLOSEDIR(&dir);
		return EIO;
	}
	ep->d_ino = dlr->dlr_inum;
	strncpy(ep->d_name, name, DIRSIZ);
	return putdents(&dir);
}

/*
 * Remove the entry described by (name,namlen,*dlr) from directory dp.
 */
/* ARGSUSED */
int
bell_remove(dp, name, namlen, dlr)
	register struct inode *dp;
	char *name;
	unsigned short namlen;
	register struct dirlookupres *dlr;
{
	struct dirstream dir;
	register struct direct *ep;

	ep = getdentstoput(dp, dlr->dlr_offset, 1, &dir);
	if (ep == NULL)
		return dir.error;
	ASSERT(dir.entries == 1);
	if (ep->d_ino != dlr->dlr_inum) {
		printf("lost rename unlink race: %s:%d from directory %x/%d\n",
			name, dlr->dlr_inum, dp->i_dev, dp->i_number);
		CLOSEDIR(&dir);
		return ENOENT;
	}
	bzero((caddr_t) ep, sizeof *ep);
	return putdents(&dir);
}

/*
 * Check whether dp is empty, really empty, or really-really empty (has
 * both "." and "..", only one, or neither, respectively).  Return true
 * or false directly, emptiness flag thru flagp.  The check is loose;
 * it doesn't cope with directory corruption.
 */
int
bell_isempty(dp, flagp)
	register struct inode *dp;
	register dflag_t *flagp;
{
	struct dirstream dir;
	register struct direct *ep;

	OPENDIR(dp, 0, dp->i_size, &dir);
	while ((ep = GETDENT(&dir)) != NULL) {
		if (ep->d_ino == 0)
			continue;
		if (FAST_ISDOT(ep->d_name))
			*flagp |= DIR_HASDOT;
		else if (FAST_ISDOTDOT(ep->d_name))
			*flagp |= DIR_HASDOTDOT;
		else
			break;
	}
	CLOSEDIR(&dir);
	return ep == NULL;
}

/*
 * Bmap and bread the next logical block from ds, resetting the stream's
 * state for more GETDENT()s.  Get and return the first of the new entries.
 */
static struct direct *
getmoredents(ds)
	register struct dirstream *ds;
{
	if (!filldir(ds, B_READ))
		return NULL;	/* error or eof */
	return GETDENT(ds);
}

/*
 * Initialize ds to hold a buffer containing directory entries starting at
 * offset in dp, and not exceeding count bytes.  The number of entries actually
 * read is returned in ds->entries.
 */
static struct direct *
getdentstoput(dp, offset, numdents, ds)
	register struct inode *dp;
	off_t offset;
	register int numdents;
	register struct dirstream *ds;
{
	OPENDIR(dp, offset, dentstobytes(numdents), ds);
	if (!filldir(ds, B_WRITE)) {
		CLOSEDIR(ds);
		return NULL;	/* error or eof */
	}
	return ds->next;
}

/*
 * Fill ds's buffer with entries from ds->offset through the end of file,
 * end of logical block, or ds->offset + ds->count, whichever comes first.
 * Return 1 on success, 0 on eof or error.
 */
static int
filldir(ds, rw)
	register struct dirstream *ds;
	register int rw;
{
	register struct inode *ip;
	register struct buf *bp;
	register off_t newbytesinbuf;
	struct bmapval bv, rabv;

	if (ds->count <= 0) {	/* we've read thru the entire dir */
		ds->entries = 0;
		return 0;
	}

	/*
	 * Check to see whether ds's offset should wrap to the beginning of
	 * the directory.
	 */
	ip = ds->inode;
	if (rw == B_READ && ds->offset >= ip->i_size)
		ds->offset = 0;
	u.u_offset = ds->offset;

	/*
	 * Map a logical block at ds's offset and read it into the buffer
	 * cache.  The number of requested bytes that were actually read is
	 * left in u.u_pbsize; u.u_pboff holds the data's byte offset within
	 * the buffer.
	 */
	u.u_count = ds->count;
	ds->error = FS_BMAP(ip, rw, &bv, &rabv);
	if (ds->error)
		return 0;
	if (bv.length == 0) {		/* signal eof */
		ds->entries = 0;
		return 0;
	}
	sysinfo.dirblk += bv.length;

	/*
	 * Release the previous buffer and call the bio to read a new one.
	 */
	bp = ds->buffer;
	if (bp != NULL)
		brelse(bp);
	if (rw == B_READ && rabv.length != 0) {
		bp = breada(ip->i_dev, (daddr_t) bv.bn, (int) bv.length,
			(daddr_t) rabv.bn, (int) rabv.length);
	} else
		bp = bread(ip->i_dev, (daddr_t) bv.bn, (int) bv.length);
	if (bp->b_flags & B_ERROR) {
		ds->error = EIO;
		return 0;
	}
	ds->buffer = bp;

	/*
	 * Update and adjust stream state.  The directory inode that we've
	 * opened and are reading from must be locked in order for us to assert
	 * that ds->count never goes negative.
	 */
	newbytesinbuf = u.u_pbsize;
	ds->count -= newbytesinbuf;
	ASSERT(ds->count >= 0);
	ds->offset += newbytesinbuf;
	ds->next = (struct direct *) (bp->b_un.b_addr + u.u_pboff);
	ds->entries = bytestodents(newbytesinbuf);
	return 1;
}

/*
 * Write entries in a directory stream buffer.  Return an error number or zero
 * if everything's ok.
 */
static int
putdents(ds)
	register struct dirstream *ds;
{
	bwrite(ds->buffer);	/* returns error in u.u_error */
	if (u.u_error)
		ds->error = u.u_error;
	else {
		register struct inode *ip;

		ip = ds->inode;
		ip->i_flag |= IUPD;
		if (ds->offset > ip->i_size) {
			ip->i_flag |= ICHG;
			ip->i_size = ds->offset;
			FS_IUPDAT(ip, &time, &time);
			if (u.u_error)
				ds->error = u.u_error;
		}
	}
	return ds->error;
}

#endif
