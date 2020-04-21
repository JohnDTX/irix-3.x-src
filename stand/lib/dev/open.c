/*
 * $Source: /d2/3.7/src/stand/lib/dev/RCS/open.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:14:42 $
 */
#include "stand.h"
#include "sysmacros.h"
#include "dprintf.h"

extern char *index();

open( strx, how )
char	*strx;
{
	register struct iob	*io;
	register struct devsw	*dp;
	register struct buf	*bp;
	register struct inode	*ip;
	int			fdesc;
	ino_t			n;
	char			*prfx,
				*ext,
				*name,
				*memp;

	/* check the args */
	if ( strx == NULL || how < 0 || how > 2 ) {
		errno = EINVAL;
		return (-1);
	}

	dprintf(("open of -%s-\n",strx));

	/* look for a empty slot to use */
	for ( fdesc = 0; fdesc < NFILES; fdesc++ )
		if ( iobuf[ fdesc ].i_flgs == 0 )
			goto gotfile;

	errno = EMFILE;
	return (-1);

gotfile:
	/*
	 * found an empty slot, set flags to say it is in use
	 */
	( io = &iobuf[ fdesc ] )->i_flgs |= F_ALLOC;
	ip = &inodes[ fdesc ];
	io->i_ip = ip;
	bp = &bufs[ fdesc ];
	io->i_bp = bp;
	io->i_flgs |= (how+1);		/* don't you love unix?		*/

	/*
	** rip the spec apart to get the prefix, extension and name
	*/
	splitspec( strx, &prfx, &ext, &name );

	/* try to match a device */
	for ( dp = devsw; dp->dv_name; dp++ )
	{
		if ( !strcmp( prfx, dp->dv_name ) )
			goto gotname;
	}

	io->i_flgs = 0;
	errno = ENXIO;
	return (-1);

gotname:
	/*
	** found a valid device, save index and device flags
	*/
	io->i_flgs |= dp->dv_flags;
	/* the device open will fix the minor */
	ip->i_dev = makedev( dp - devsw, 0 );

	if ( (io->i_flgs & F_DISK) && (name != NULL ) )
		ip->i_mode = IFREG;
	else
		ip->i_mode = IFCHR;

	if ( how != 0 ) {		/* XXX read-only for now	*/
		io->i_flgs = 0;
		errno = EACCES;
		return (-1);
	}

	/*
	 * check for a member name within the file name which
	 * indicates an element of a cpio file
	 */
	if ( ( memp = index( name, '(' ) ) != NULL ) {
		char *cp;

		cp = memp + 1;
		if ( ( cp = index( cp, ')' ) ) == NULL ) {
			printf("unmatched \"(\" in file name: %s\n", name);
			io->i_flgs = 0;
			return (-1);
		}
		/*
		 * separate the names of the cpio file and the member
		 * within the cpio file.
		 */
		*memp++ = '\0';
		*cp = '\0';
		io->i_flgs |= F_CPIO;
	}

	if ( _devopen( io, ext, name ) < 0 ) {
		errno = io->i_error;
		io->i_flgs = 0;
		return (-1);
	}

	/* this is for physical devices	*/
	if ( name == NULL || *name == '\0' ) {
		goto out;
	}

	if ( io->i_flgs & F_TAPE ) {
		io->i_flgs |= F_CPIO;
		memp = name;
	}
	
	if ( ip->i_mode & IFREG ) {	/* special for disk files	*/

		/*
		 * if we are looking for a named file we have to
		 * determine the type of file system. So "mount" the filesystem
		 * and run through the filesystem to find the inode.
		 * fsopen() does all this for us.
		 */
		if ( fsopen( io, name ) < 0 ) {
			_devclose( io );
			errno = io->i_error;
			io->i_flgs = 0;
			return (-1);
		}

	}

	/*
	 * if the file is a cpio file, do cpio open processing
	 */
	if ( io->i_flgs & F_CPIO ) {
		io->i_offset = 0;
		io->i_error = 0;
		if ( cpioopen( io, memp, 0 ) < 0 ) {
			_devclose( io );
			errno = io->i_error;
			io->i_flgs = 0;
			return (-1);
		}
		goto out1;
	}

out:
	/* set up default values for the IO block	*/

	io->i_offset = 0;
	io->i_error = 0;
out1:
	return (fdesc+3);
}
