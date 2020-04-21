/*
 * $Source: /d2/3.7/src/sys/sys/RCS/fio.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:35:10 $
 */
#include "../h/param.h"
#include "../h/systm.h"
#include "../h/user.h"
#include "../h/acct.h"
#include "../h/file.h"
#include "../h/fstyp.h"
#include "../h/inode.h"
#include "../h/nami.h"
#include "../h/mount.h"
#include "../h/sysinfo.h"

struct	file *ffreelist;		/* free list for falloc() */

/*
 * init_file:
 *	- init the file table
 */
init_file()
{
	register struct file *fp;
	register short i;

	ffreelist = fp = &file[0];
	i = nfile - 1;
	do {
		fp->f_next = fp+1;
		fp++;
	} while (--i != 0);
}

/*
 * Convert a user supplied file descriptor into a pointer
 * to a file structure.
 * Only task is to check range of the descriptor.
 */
struct file *
getf(f)
register int f;
{
	register struct file *fp;

	if (0 <= f && f < NOFILE) {
		fp = u.u_ofile[f];
		if (fp != NULL)
			return(fp);
	}
	u.u_error = EBADF;
	return(NULL);
}

/*
 * closeall:
 *	- closes all open files
 *	- this assumes that no other processes can run while this is
 *	  running (i.e., that all other processes have been killed)
 */
closeall()
{
	register struct file *fp;

	for (fp = &file[0]; fp < fileNFILE; fp++) {
		if (fp->f_count) {
			/*
			 * If inode is locked, unlock it now.  This is done
			 * regardless of what had been going on.  Closef()
			 * will derefrence the inode making it safe for
			 * updating to disk.
			 */
			if (fp->f_inode->i_flag & ILOCKED)
				fp->f_inode->i_flag &= ~ILOCKED;
			closef(fp);
		}
	}
}

/*
 * Internal form of close.
 * Decrement reference count on file structure.
 * Also make sure the pipe protocol does not constipate.
 *
 * Decrement reference count on the inode following
 * removal to the referencing file structure.
 * On the last close switch out to the device handler for
 * special files.  Note that the handler is called
 * on every open but only the last close.
 */
closef(fp)
	register struct file *fp;
{
	register struct inode *ip;

	if (fp == NULL || (fp->f_count) <= 0 )
		return;
	ip = fp->f_inode;
	ilock(ip);
	FS_CLOSEI(ip, fp->f_flag, fp->f_count, fp->f_offset);
	if ((unsigned)fp->f_count > 1) {
		fp->f_count--;
		iunlock(ip);
		return;
	}
	fp->f_count = 0;
	fp->f_next = ffreelist;
	ffreelist = fp;
	iput(ip);
}

/*
 * Look up a pathname and test if the resultant inode is owned by the
 * current user. If not, try for super-user.
 * If permission is granted, return inode pointer.
 */
struct inode *
owner()
{
	register struct inode *ip;

	ip = namei(USERPATH, NI_LOOKUP, FOLLOWLINK);
	if (ip == NULL)
		return(NULL);
	if (u.u_uid == ip->i_uid || suser()) {
		if (rdonlyfs(ip->i_mntdev))
			u.u_error = EROFS;
	}
	if (!u.u_error)
		return(ip);
	iput(ip);
	return(NULL);
}

/*
 * Test if the current user is the super user.
 */
suser()
{
	if (u.u_uid == 0) {
		u.u_acflag |= ASU;
		return(1);
	}
	u.u_error = EPERM;
	return(0);
}

/*
 * Allocate a user file descriptor.
 */
ufalloc(i)
	register int i;
{
	for(; i<NOFILE; i++)
		if (u.u_ofile[i] == NULL) {
			u.u_rval1 = i;
			u.u_pofile[i] = 0;
			return(i);
		}
	u.u_error = EMFILE;
	return(-1);
}

/*
 * Allocate a user file descriptor and a file structure.
 * Initialize the descriptor to point at the file structure.
 *
 * no file -- if there are no available file structures.
 */
struct file *
falloc(ip, flag)
	struct inode *ip;
{
	register struct file *fp;
	register i;

	i = ufalloc(0);
	if (i < 0)
		return(NULL);
	if ((fp=ffreelist) == NULL) {
		uprintf("File table overflow\n");
		syserr.fileovf++;
		u.u_error = ENFILE;
		return(NULL);
	}
	ffreelist = fp->f_next;
	u.u_ofile[i] = fp;
	fp->f_count++;
	fp->f_inode = ip;
	fp->f_flag = flag;
	fp->f_offset = 0;
	return(fp);
}
