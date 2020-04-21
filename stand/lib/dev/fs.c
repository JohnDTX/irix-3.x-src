/*
 * $Source: /d2/3.7/src/stand/lib/dev/RCS/fs.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:14:29 $
 */
/* These routines are shared by all filesystems			*/

#include "stand.h"
#include "sys/dir.h"
#include "sys/stat.h"
#include "machine/param.h"
#include "dprintf.h"

extern struct iops bell_iops;
extern struct iops efs_iops;

#define ROOTINO	((ino_t)2)	/* XXX put somewhere else	*/

char *dirbuf;

/*
 * Open the file that has the name given by path.  Assume that the
 * physical device has been opened and that the dev in the iob is correct.
 * You first need to determine what type of file system is on the device.
 * After that you can run through namei to grap the inode for the file.
 */
fsopen(io, path)
struct iob *io;
char *path;
{
	struct inode *ip;

 	/* Like what type of file system do we have here anyway? */
	if ( mount( io ) < 0 ) {
		dprintf(("No filesystem on device\n"));
		return( -1 );
	}

	if (  namei( ROOTINO, io, path)  < 0 )
		return( -1 );
	else
		return( 0 );

}

/*
 * fsread - read data from a file on a local filesystem.  The file has
 * already been openned so you know what type of filesystem we are dealing 
 * with.  Read i_count bytes to i_base location.  Return the number of
 * bytes read.
 */
fsread(io)
struct iob *io;
{
	register int cnt;

	dprintf(("fsread "));
	return(readi(io));
}

/*
 * mount is called with a iob structure.  Determine what type
 * of filesystem is on that device.
 *
 * Returns: 0 - success
 *	   -1 - can't figure out what type of filesystem
 *
 * XXX TO DO - don't mount on each open.  keep track of dev and fs type
 */
mount(io)
register struct iob *io;
{

	dprintf(("mount\n"));

	if ( bell_mount( io ) == 0 ) {
		printf("Bell Filesystem\n");
		io->i_ip->i_ops = &bell_iops;
		return(0);
	}

	if ( efs_mount( io ) == 0 ) {
		printf("SGI Extent Filesystem\n");
		io->i_ip->i_ops = &efs_iops;
		return(0);
	}

	/* failure	*/
	dprintf(("mount:no filesystem on device\n"));
	if ( io->i_error == 0 )
		io->i_error = ENOENT;
	return(-1);
}

/*
 * turn a path name into an inode.
 * update the iob
 */
namei(curino,io,path)
register ino_t curino;
register struct iob *io;
char *path;
{
	register char *comp;
	register char *tp;
	register struct inode *ip;
	register struct direct *dp;
	struct inode *iread();
	struct direct *dir_read();
	char component[DIRSIZ+1];

	comp = path;

	if( *comp == '/' ) {
		curino = ROOTINO;
		while( *comp == '/' )
			comp++;
	}

	if( (ip = iread(io,curino)) <= 0 ) {
		dprintf(("couldn't read first inode\n"));
		goto error;
	}

	while( *comp != '\0' ) {
		/* get next part of component	*/
		tp = component;
		while( *comp != '\0' && *comp != '/' )
			*tp++ = *comp++;
		*tp = '\0';


		/* get next pathname component */
		if ( (ip->i_mode & IFMT) != IFDIR ) {
			dprintf(("not a directory"));
			io->i_error = ENOENT;
			goto error;
		}

		curino = 0;
		io->i_offset = 0;

		/* search the directory */
		while( (dp = dir_read(io)) != 0 ) {
			if( dp->d_ino != 0 ) {
				/*
				 * try to match directory entry. any
				 * characters beyond DIRSIZ are ignored
				 */
				if(strncmp(dp->d_name,component,DIRSIZ) == 0) {
					curino = dp->d_ino;
					break;
				}
			}
		}

		if( curino == 0 ) {
			io->i_error = ENOENT;
			goto error;
		}

		while( *comp == '/' )
			comp++;

		if( (ip = iread(io,curino)) <= 0 ) {
			dprintf(("couldn't read inode %d\n",curino));
			goto error;
		}
	}

	/*
	 * at this point we are at the end of the path and
	 * we found the inode.  Fix up data structures.
	 */
	return(0);

error:			/* badness */
	return(-1);
}


/*
 * iread() - Read in an inode
 */
struct inode *
iread(io,inum)
register struct iob *io;
register ino_t inum;
{
	register struct inode *ip;

	/* set up stuff in inode	*/
	ip = io->i_ip;
	ip->i_number = inum;

	return( (*ip->i_ops->iread)(io) );
}



struct {
	unsigned s_count;
	dev_t s_dev;
	daddr_t s_bn;
	char *s_buf;
} storage;


/*
 * readi() - Read in data from file system file pointed to by the inode
 */
readi(io)
register struct iob *io;
{
	register struct inode *ip;
	register struct buf *bp;
	unsigned size;		/* max size of request allowed by filesys */
	register offset;
	char *mbmalloc();

	ip = io->i_ip;
	bp = io->i_bp;

	if ( io->i_offset >= ip->i_size ) {
		return(0);
	}

	if ( io->i_count + io->i_offset > ip->i_size ) {
		io->i_count = ip->i_size - io->i_offset;
	}

	/* convert to logical block in file */
	bp->b_iobn = (*ip->i_ops->bmap)(io,&size);

	if ( bp->b_iobn == 0 ) {	/* hole */
		dprintf(("HOLE!\n"));
		size = min(io->i_count,size);
		bzero(io->i_offset,size);
		return(size);
	}

	if ( (io->i_offset & BBMASK) || (io->i_count < BBSIZE) ) {
		/*
		 * if offset not on block boundary or if count is less
		 * than then basic block size we need
		 * to buffer. Make surce that you copy to the correct
		 * offset and only copy the residual number of bytes.
		 */
		if ( storage.s_buf == 0 ) {
			storage.s_buf = mbmalloc(BBSIZE);
			storage.s_dev = -1;
		}

		if ( (storage.s_dev == ip->i_dev)
					&& (storage.s_bn == bp->b_iobn) ) {
			size = storage.s_count;
		} else {
			storage.s_dev = ip->i_dev;
			storage.s_bn = bp->b_iobn;
			bp->b_iobase = storage.s_buf;
			bp->b_bcount = BBSIZE;
			size = _devread(io);
			if ( size < 0 )
				return(0);
			storage.s_count = size;
		}
		/* now copy to user buffer */
		offset = io->i_offset % size;	/* get correct offset */
		size -= offset;			/* only xfer valid bytes */
		size = min(size, io->i_count);
		dprintf(("piece: base 0x%x offset 0x%x, size %d\n",
					io->i_base,offset, size));
		bcopy(storage.s_buf+offset, io->i_base, size);
		return(size);
		
	}

	/* request is on block boundary so read the sucker in */
	/* make sure that the count is a multiple of blocks */
	bp->b_iobase = io->i_base;
	bp->b_bcount = min( size, (io->i_count & ~BBMASK ) );
	
	if ( (size = _devread(io)) < 0 )
		return(0);
	else
		return(size);
}

struct direct *
dir_read(io)
register struct iob *io;
{
	register struct inode *ip;
	register off_t	index;		/* offset into dirbuf[] */
	char *mbmalloc();

	ip = io->i_ip;

	if ( dirbuf == 0 ) {
		if ( (dirbuf = mbmalloc(BBSIZE)) == 0 ) {
			printf("malloc failed\n");
			return(0);
		}
	}

	if ( io->i_offset >= ip->i_size ) {	/* read through all of dir */
		return(0);
	}

	index = io->i_offset % BBSIZE;

	if ( index == 0 ) { /* buffer empty */
		io->i_base = dirbuf;
		io->i_count = BBSIZE;
		if ( readi(io) < 0 )
			return(0);
	}

	/* now return pointer to directory */
	io->i_offset += sizeof(struct direct);
	return( (struct direct *)(dirbuf + index) );

}

nullop()
{

	printf("nullop!!!");
}

fstat( fd, sp)
register int fd;
register struct stat *sp;
{
	register struct iob *io;
	register struct inode *ip;


	fd -= 3;
	if ( fd < 0
	     || fd >= NFILES
	     || ( ( io = &iobuf[ fd ] )->i_flgs & F_ALLOC ) == 0
	     || ( io->i_flgs & F_READ ) == 0 ) {
		errno = EBADF;
		return (-1);
	}

	/* now copy shit over */
	ip = io->i_ip;


	sp->st_dev = ip->i_mode;
	sp->st_ino = ip->i_number;
	sp->st_mode = ip->i_mode;
	sp->st_nlink = ip->i_nlink;
	sp->st_uid = ip->i_uid;
	sp->st_gid = ip->i_gid;
	sp->st_rdev = ip->i_rdev;
	sp->st_size = ip->i_size;
	sp->st_atime = 0;
	sp->st_mtime = 0;
	sp->st_ctime = 0;

	return(0);
}
